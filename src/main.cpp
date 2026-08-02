#include <SDL.h>
#include <algorithm>
#include <cstdio>
#include <random>
#include <string>
#include <vector>
#include "game/camera.h"
#include "game/run.h"
#include "scene/legend.h"
#include "scene/scene.h"
#include "ui/text.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// The simulated world's size, in cells - independent of the window since F3.1.
// No longer equal to WINDOW_WIDTH/HEIGHT divided by Camera::SCALE: that was
// only ever a coincidence of nothing having needed them to differ yet, and F4's
// scene is authored at 640x400 (notes/art_pipeline.txt's reference resolution),
// bigger than the 200x150 viewport below. That is deliberate, not a mismatch to
// fix - it is what exercises the panning half of the camera (F3.4) rather than
// just the decoupling half (F3.1-F3.3), which a world equal to the viewport
// never did.
//
// SDL_RenderCopy below stretches the whole viewport-sized texture across the
// whole window (two null rects). **This used to warn that a grid not matching
// the window's proportions renders squashed or cropped, and that stopped being
// true when F3.3 sized the texture to the viewport rather than to the grid**:
// the texture is VIEWPORT_WIDTH x VIEWPORT_HEIGHT and the window is exactly
// Camera::SCALE times that, so the blit is 1:1 whatever size the world is. The
// warning outlived the problem and read as a known defect in code that is
// correct. Camera (F3.2) owns every
// screen-to-world and world-to-screen conversion, and (F3.4) the viewport's
// position in the world, so mouse/render coordinates are correct at any grid
// size and any camera offset. The texture is sized to the viewport rather
// than the whole grid (F3.3), so upload cost does not scale with world size,
// and the viewport now follows the player and clamps at the world's edges
// (F3.4) rather than staying pinned at the origin - the two together are what
// turn "the whole world, squashed" into a real view of part of a larger one.
const int GRID_WIDTH = 640;
const int GRID_HEIGHT = 400;

// How many world cells actually fit on screen at once - independent of
// GRID_WIDTH/HEIGHT, which is the whole point of F3.3: a world bigger than
// this no longer costs more to upload just for existing off-screen.
const int VIEWPORT_WIDTH = WINDOW_WIDTH / Camera::SCALE;
const int VIEWPORT_HEIGHT = WINDOW_HEIGHT / Camera::SCALE;

const double MAX_FRAME_TIME = 0.25; // clamp after a stall so we don't spiral

Scene load_scene_from_bmp(const char* material_path, const char* albedo_path) {
    Scene scene;
    SDL_Surface* mat_surf = SDL_LoadBMP(material_path);
    SDL_Surface* alb_surf = SDL_LoadBMP(albedo_path);

    if (!mat_surf || !alb_surf) {
        if (mat_surf) SDL_FreeSurface(mat_surf);
        if (alb_surf) SDL_FreeSurface(alb_surf);
        std::fprintf(stderr, "Failed to load scene BMPs\n");
        return scene;
    }

    if (mat_surf->w != alb_surf->w || mat_surf->h != alb_surf->h) {
        SDL_FreeSurface(mat_surf);
        SDL_FreeSurface(alb_surf);
        std::fprintf(stderr, "Scene BMP dimensions do not match\n");
        return scene;
    }

    scene.width = mat_surf->w;
    scene.height = mat_surf->h;
    scene.materials.resize(scene.width * scene.height, ElementType::Empty);
    scene.albedo.resize(scene.width * scene.height, 0);

    // Assuming 24-bit or 32-bit BMPs.
    // We should lock surfaces if needed, but SDL_LoadBMP gives 24-bit or 8-bit.
    // For simplicity, we convert both to 32-bit ARGB.
    SDL_Surface* mat_32 = SDL_ConvertSurfaceFormat(mat_surf, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_Surface* alb_32 = SDL_ConvertSurfaceFormat(alb_surf, SDL_PIXELFORMAT_ARGB8888, 0);

    SDL_FreeSurface(mat_surf);
    SDL_FreeSurface(alb_surf);

    if (!mat_32 || !alb_32) {
        if (mat_32) SDL_FreeSurface(mat_32);
        if (alb_32) SDL_FreeSurface(alb_32);
        return scene;
    }

    // Indexed via pitch rather than width * 4: SDL_ConvertSurfaceFormat is free
    // to pad each row for alignment, and reading straight across the buffer as
    // if pitch == width * 4 would drift a row further off with every line on
    // any width where that assumption doesn't hold.
    const uint8_t* mat_base = static_cast<const uint8_t*>(mat_32->pixels);
    const uint8_t* alb_base = static_cast<const uint8_t*>(alb_32->pixels);

    // A pixel that names no material is a *fault in the scene file*, not an
    // empty cell, and the two used to be indistinguishable here - which is how
    // a palette change silently emptied the whole world. Counted, reported, and
    // the first few offenders named, because "3 unmatched" sends you looking
    // and "#4444FF" tells you what happened.
    int unmatched = 0;
    uint32_t first_unmatched[4] = {0, 0, 0, 0};
    int first_unmatched_n = 0;

    for (int y = 0; y < scene.height; ++y) {
        const uint32_t* mat_row = reinterpret_cast<const uint32_t*>(mat_base + y * mat_32->pitch);
        const uint32_t* alb_row = reinterpret_cast<const uint32_t*>(alb_base + y * alb_32->pitch);
        for (int x = 0; x < scene.width; ++x) {
            int idx = y * scene.width + x;
            uint32_t m_col = mat_row[x];
            uint32_t a_col = alb_row[x];

            // The legend is its own frozen table (scene/legend.h), deliberately
            // not the render palette - see there for what binding the two cost.
            ElementType type = ElementType::Empty;
            if (!element_from_legend(m_col, type)) {
                const uint32_t rgb = m_col & 0xFFFFFF;
                bool seen = false;
                for (int i = 0; i < first_unmatched_n; ++i) if (first_unmatched[i] == rgb) seen = true;
                if (!seen && first_unmatched_n < 4) first_unmatched[first_unmatched_n++] = rgb;
                ++unmatched;
                type = ElementType::Empty;
            }

            scene.materials[idx] = type;
            scene.albedo[idx] = a_col | 0xFF000000; // force alpha
        }
    }

    if (unmatched > 0) {
        std::fprintf(stderr,
                     "WARNING: %s has %d pixel(s) whose colour is in no legend entry; they loaded as Empty.\n"
                     "         Unrecognised colours include:", material_path, unmatched);
        for (int i = 0; i < first_unmatched_n; ++i) std::fprintf(stderr, " #%06X", first_unmatched[i]);
        std::fprintf(stderr, "\n         The legend is src/scene/legend.h and is frozen; the render palette"
                             " in material.h is not the legend.\n");
    }

    SDL_FreeSurface(mat_32);
    SDL_FreeSurface(alb_32);

    return scene;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SLOP Pixel Physics",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // ARGB8888 texture for direct pixel manipulation, sized to the viewport
    // (F3.3) rather than the whole grid - see VIEWPORT_WIDTH/HEIGHT above.
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        VIEWPORT_WIDTH, VIEWPORT_HEIGHT
    );
    if (!texture) {
        std::fprintf(stderr, "Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // V1: Empty is 0x00000000 in MATERIALS, so an empty cell is transparent
    // rather than black - but only if the texture is composited rather than
    // blitted. Without this line the alpha is carried all the way to the screen
    // and then ignored, which looks exactly like the opaque black the table
    // used to hold, so the two changes are only meaningful together.
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    // A streaming texture is created with undefined contents, and the upload
    // below only ever writes the rect the grid actually covers. Those are the
    // same rect in every world this project currently builds - but a world
    // smaller than the viewport on either axis leaves the remainder holding
    // whatever the driver's allocation happened to contain, and it would show
    // as garbage along the edge rather than as the backdrop. One clear at
    // startup rather than a per-frame guard, since it can only ever be wrong
    // once.
    {
        const std::vector<uint32_t> blank(static_cast<size_t>(VIEWPORT_WIDTH) * VIEWPORT_HEIGHT, 0);
        SDL_UpdateTexture(texture, nullptr, blank.data(), VIEWPORT_WIDTH * sizeof(uint32_t));
    }

    // This is the only nondeterministic line in the project, and it is here
    // rather than inside Grid on purpose: the simulation is a pure function of
    // its seed, and exactly one place gets to choose that seed. Two draws
    // because std::random_device yields 32 bits at a time.
    //
    // Printed because a seed you cannot read is only half of determinism - this
    // is the number that turns "it collapsed weirdly" into something that can be
    // reproduced. It will belong in the title bar or a debug overlay once there
    // is a UI to put it in; stdout will do until then.
    std::random_device rd;
    const uint64_t world_seed = (static_cast<uint64_t>(rd()) << 32) | rd();
    std::printf("World seed: %llu\n", static_cast<unsigned long long>(world_seed));

    Run run(GRID_WIDTH, GRID_HEIGHT, world_seed); // starts fully Empty, player mid-air
    
    // Load F4 test scene. A scene that resolves to no cells at all is reported
    // rather than shrugged off: README's launch check is "terrain is visible
    // immediately", and a blank world is exactly what a broken legend, a
    // missing file and an empty file all look like from here.
    Scene scene = load_scene_from_bmp("assets/test_material.bmp", "assets/test_albedo.bmp");
    if (scene.width > 0) {
        const int placed = load_scene(run.grid, scene, 0, 0);
        std::printf("Scene: %dx%d, %d cells placed\n", scene.width, scene.height, placed);
        if (placed == 0) {
            std::fprintf(stderr, "WARNING: the scene named no material anywhere - the world is empty.\n");
        }
    }

    Camera camera;

    bool running = true;
    SDL_Event e;

    ElementType current_brush = ElementType::Sand;
    int brush_size = 3;

    uint64_t prev_counter = SDL_GetPerformanceCounter();
    const double counter_freq = static_cast<double>(SDL_GetPerformanceFrequency());
    double accumulator = 0.0;

    int frames_this_second = 0;
    double title_timer = 0.0;

    // Recomputed once a second, same as the title bar readout it replaces,
    // but drawn every frame - a string this cheap to format is not worth
    // reformatting 60x/sec, and it costs nothing to redraw from the cached
    // copy in between.
    std::string hud_text = "FPS:0 BRUSH:SAND(3) CHUNKS:0";

    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            else if (e.type == SDL_MOUSEWHEEL) {
                brush_size += e.wheel.y;
                if (brush_size < 1) brush_size = 1;
                if (brush_size > 32) brush_size = 32;
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_1: current_brush = ElementType::Sand;  break;
                    case SDLK_2: current_brush = ElementType::Water; break;
                    case SDLK_3: current_brush = ElementType::Wall;  break;
                    case SDLK_4: current_brush = ElementType::Empty; break; // Eraser
                    case SDLK_5: current_brush = ElementType::Wood;  break;
                    case SDLK_6: current_brush = ElementType::Oil;   break;
                    case SDLK_7: current_brush = ElementType::Steam; break;
                    case SDLK_8: current_brush = ElementType::Fire;  break;
                    case SDLK_ESCAPE: running = false; break;
                }
            }
        }

        // The viewport follows the player, clamped at the world's edges
        // (F3.4). Recomputed once per rendered frame, same as the mouse and
        // keyboard samples below it - "the player's position this frame" has
        // exactly the same one-sample-per-frame character as those do.
        camera.follow(run.player.center_x(), run.player.center_y(), VIEWPORT_WIDTH, VIEWPORT_HEIGHT, GRID_WIDTH, GRID_HEIGHT);

        // Handle continuous mouse pressing
        int mouseX, mouseY;
        const uint32_t mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        const int gridX = camera.screen_to_world_x(mouseX);
        const int gridY = camera.screen_to_world_y(mouseY);

        // Sampled once per rendered frame, same as before - a real mouse and
        // keyboard cannot be sampled at the simulation's fixed rate, since
        // there is no such thing as "the input for a step that has not
        // happened yet". What changed is what happens to this sample: every
        // fixed step this frame accumulates gets its own call to run.step()
        // with it, rather than the brush being painted once up here before
        // the loop even starts. Movement is read from live key state rather
        // than key events, so holding a key keeps moving instead of firing
        // once and repeating on the OS key-repeat delay.
        const uint8_t* keys = SDL_GetKeyboardState(nullptr);
        Input input;
        input.left  = keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT];
        input.right = keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT];
        input.jump  = keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP];
        input.cursor_x = gridX;
        input.cursor_y = gridY;
        input.dig   = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
        // Right-click, not left. Digging is the game's action and gets the
        // primary button; the material brush is a development tool and moved
        // out of its way.
        input.brush_active = (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
        input.brush_type = current_brush;
        input.brush_size = brush_size;

        const uint64_t now_counter = SDL_GetPerformanceCounter();
        double frame_time = static_cast<double>(now_counter - prev_counter) / counter_freq;
        prev_counter = now_counter;
        if (frame_time > MAX_FRAME_TIME) frame_time = MAX_FRAME_TIME;

        accumulator += frame_time;
        while (accumulator >= Run::FIXED_DT) {
            run.step(input);
            accumulator -= Run::FIXED_DT;
        }

        // Upload only the visible rect (F3.3), not the whole grid, starting
        // from the camera's current view (F3.4) rather than always (0, 0).
        // The source pitch stays GRID_WIDTH wide even though the rect is
        // narrower, so SDL reads the right columns out of each grid row and
        // skips the rest - one call, no intermediate buffer. Clamped to the
        // grid's own size so this stays correct if the grid is ever smaller
        // than the viewport; the case this step exists for is the opposite
        // one, a grid larger than the viewport, where the clamp is a no-op
        // and the rect is the full viewport every frame.
        const std::vector<uint32_t>& pixels = run.grid.get_pixels();
        const int visible_w = std::min(VIEWPORT_WIDTH, GRID_WIDTH);
        const int visible_h = std::min(VIEWPORT_HEIGHT, GRID_HEIGHT);
        const SDL_Rect visible_rect{0, 0, visible_w, visible_h};
        const uint32_t* visible_pixels = pixels.data() + camera.view_y() * GRID_WIDTH + camera.view_x();
        SDL_UpdateTexture(texture, &visible_rect, visible_pixels, GRID_WIDTH * sizeof(uint32_t));

        // The backdrop layer (V1). Drawn before the cell texture, which now
        // composites over it wherever a cell is Empty. It is a placeholder
        // gradient rather than authored art on purpose: what this step delivers
        // is the *layer* - somewhere for a painted background to go and a
        // guarantee that it is not painted over - and the art that eventually
        // fills it is the art pipeline's job, not this one's.
        //
        // Bands rather than a per-pixel gradient because the whole point is
        // that this costs nothing: sixty-four filled rects a frame against a
        // renderer that is already vsync-bound. Sixteen was tried first and the
        // steps between bands were visible on screen, which reads as a
        // rendering bug rather than as a placeholder.
        constexpr int BACKDROP_BANDS = 64;
        const int band_height = (WINDOW_HEIGHT + BACKDROP_BANDS - 1) / BACKDROP_BANDS;
        for (int band = 0; band < BACKDROP_BANDS; ++band) {
            const int t = band * 255 / (BACKDROP_BANDS - 1);
            SDL_SetRenderDrawColor(
                renderer,
                static_cast<Uint8>(18 + (46 - 18) * t / 255),
                static_cast<Uint8>(20 + (52 - 20) * t / 255),
                static_cast<Uint8>(34 + (74 - 34) * t / 255),
                255
            );
            const SDL_Rect band_rect{0, band * band_height, WINDOW_WIDTH, band_height};
            SDL_RenderFillRect(renderer, &band_rect);
        }

        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        // The player is not a cell, so it is not in the pixel buffer either -
        // it is drawn on top of the world as its own rectangle.
        SDL_SetRenderDrawColor(renderer, 235, 235, 245, 255);
        const SDL_Rect body{
            camera.world_to_screen_x(run.player.cell_x()),
            camera.world_to_screen_y(run.player.cell_y()),
            camera.scale_length(Player::WIDTH),
            camera.scale_length(Player::HEIGHT)
        };
        SDL_RenderFillRect(renderer, &body);

        // Where a dig would actually land. Without this the range limit is
        // invisible and a shot that stopped short reads as the tool being
        // broken rather than as the target being too far away.
        int mark_x = 0;
        int mark_y = 0;
        run.dig_tool.aim_point(run.grid, run.player.center_x(), run.player.center_y(), gridX, gridY, mark_x, mark_y);
        SDL_SetRenderDrawColor(renderer, 255, 106, 0, 255);
        const SDL_Rect mark{
            camera.world_to_screen_x(mark_x), camera.world_to_screen_y(mark_y),
            camera.cell_size(), camera.cell_size()
        };
        SDL_RenderFillRect(renderer, &mark);

        // The UI layer decision (ENGINEERING_NOTES.md): drawn here, in the
        // game window, rather than left to the OS title bar - a solid backing
        // rect first so the text stays readable over whatever the simulation
        // is doing underneath it.
        const int hud_scale = 2;
        const int hud_x = 8, hud_y = 8;
        const SDL_Rect hud_backing{
            hud_x - 4, hud_y - 4,
            ui::text_width(hud_text, hud_scale) + 8, ui::GLYPH_HEIGHT * hud_scale + 8
        };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &hud_backing);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        ui::draw_text(renderer, hud_x, hud_y, hud_scale, hud_text, 0xFFE0E0E0);

        SDL_RenderPresent(renderer);

        // Surface the frame rate so performance regressions are visible while working.
        frames_this_second++;
        title_timer += frame_time;
        if (title_timer >= 1.0) {
            // Awake chunks are shown because they explain the frame rate: if the
            // world is idle and the count is not near zero, culling has a bug.
            hud_text = "FPS:" + std::to_string(frames_this_second) +
                       " BRUSH:" + material_of(current_brush).name + "(" + std::to_string(brush_size) + ")" +
                       " CHUNKS:" + std::to_string(run.grid.active_chunk_count());
            frames_this_second = 0;
            title_timer = 0.0;
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
