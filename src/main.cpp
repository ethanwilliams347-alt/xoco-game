#include <SDL.h>
#include <algorithm>
#include <cstdio>
#include <random>
#include <string>
#include <vector>
#include "game/camera.h"
#include "game/run.h"
#include "render/light.h"
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

// One extra cell on each axis, uploaded and drawn but never quite fully on
// screen. The camera scrolls in fractions of a cell (defect A1), so the world
// texture is drawn shifted by up to one cell left and up - which uncovers a
// sliver along the right and bottom edges that this margin fills. Everything
// downstream uses the padded size; the camera is told its viewport is the
// padded size too, so its clamp keeps the upload inside the grid.
const int PADDED_WIDTH = VIEWPORT_WIDTH + 1;
const int PADDED_HEIGHT = VIEWPORT_HEIGHT + 1;

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

// Loads a plain (non-scene) authored BMP as a texture - the backdrop and
// prop art V8/V4 add on top of F4's scene loader above. `colorkey` marks
// pixel_art.COLOR_KEY (magenta, 0xFF00FF - see tools/pixel_art.py) as
// transparent before the surface becomes a texture, which is how a sprite
// with an irregular silhouette gets transparency out of a 24-bit BMP with
// no alpha channel at all: SDL_CreateTextureFromSurface bakes a colour-keyed
// surface's key into the resulting texture's alpha, so nothing downstream
// has to know the trick happened. Backdrop layers (opaque, full-rect) pass
// colorkey=false and get a plain opaque texture.
SDL_Texture* load_art_texture(SDL_Renderer* renderer, const char* path, bool colorkey) {
    SDL_Surface* surf = SDL_LoadBMP(path);
    if (!surf) {
        std::fprintf(stderr, "Failed to load %s: %s\n", path, SDL_GetError());
        return nullptr;
    }
    if (colorkey) {
        const Uint32 key = SDL_MapRGB(surf->format, 0xFF, 0x00, 0xFF);
        SDL_SetColorKey(surf, SDL_TRUE, key);
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!tex) {
        std::fprintf(stderr, "Failed to create texture from %s: %s\n", path, SDL_GetError());
        return nullptr;
    }
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}

// A non-simulated sprite anchored to a world position - V4's props layer.
// Exercises no system and is never dug, ignited or displaced; see notes/
// art_direction.txt for why trees specifically are drawn *before* the cell
// texture rather than after; it is that ordering, not anything in this
// struct, that gives a planted trunk its occlusion for free.
struct Prop {
    SDL_Texture* texture;
    int w, h;         // native size, in world cells (1 BMP pixel = 1 cell)
    float anchor_x;   // world cell, bottom-centre of the sprite
    float anchor_y;
};

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
        PADDED_WIDTH, PADDED_HEIGHT
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

    // V7's light texture: one texel per LightField::BLOCK cells, stretched over
    // the world and composited additively. Three properties of it are load-
    // bearing and none is obvious from the create call.
    //
    // **ADD, not BLEND.** Light is something the scene gains, not something laid
    // over it: an alpha blend towards orange would wash the terrain's colour out
    // towards the flame's, where addition brightens whatever is already there and
    // leaves a lit grey wall reading as a grey wall. It also means black is
    // free - an unlit block adds nothing - which is what lets the same texture
    // cover the whole viewport rather than needing a mask.
    //
    // **Linear filtering is the entire reason a downsampled grid is acceptable.**
    // At nearest-neighbour this is 4x4-cell squares of flat colour, which is
    // worse than no lighting at all. Set per-texture rather than through
    // SDL_HINT_RENDER_SCALE_QUALITY, which is read at *creation* time and is
    // global - routing this texture's filtering through a global would make the
    // cell texture's sharpness depend on the order the two are created in, and
    // the day that bit rots every cell in the world goes soft at once.
    LightField light(PADDED_WIDTH, PADDED_HEIGHT);
    SDL_Texture* light_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        light.cols(), light.rows()
    );
    if (!light_texture) {
        std::fprintf(stderr, "Light texture could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_SetTextureBlendMode(light_texture, SDL_BLENDMODE_ADD);
    SDL_SetTextureScaleMode(light_texture, SDL_ScaleModeLinear);

    // V8's backdrop: two static parallax layers, replacing V1's 64-band
    // gradient placeholder now that there is authored art to show instead.
    // Generated by tools/generate_backdrop.py, which sizes each layer to
    // cover the camera's full pan range at its own factor - see that
    // script's header for why the two files have to change together.
    // Layer order and the "trees draw before terrain" rule are notes/
    // art_direction.txt's four-layer model.
    SDL_Texture* backdrop_sky = load_art_texture(renderer, "assets/backdrop_sky.bmp", false);
    SDL_Texture* backdrop_mountains = load_art_texture(renderer, "assets/backdrop_mountains.bmp", true);
    constexpr float PARALLAX_SKY_X = 0.04f, PARALLAX_SKY_Y = 0.02f;
    constexpr float PARALLAX_MOUNTAIN_X = 0.15f, PARALLAX_MOUNTAIN_Y = 0.06f;
    int sky_w = 0, sky_h = 0, mountain_w = 0, mountain_h = 0;
    if (backdrop_sky) SDL_QueryTexture(backdrop_sky, nullptr, nullptr, &sky_w, &sky_h);
    if (backdrop_mountains) SDL_QueryTexture(backdrop_mountains, nullptr, nullptr, &mountain_w, &mountain_h);

    // V4's props: a handful of trees from tools/generate_props.py, placed on
    // the F4 fixture's flat ground either side of the pit (297-328) and the
    // water channel (400-561) so none of them anchor over open air.
    SDL_Texture* tree_a = load_art_texture(renderer, "assets/tree_a.bmp", true);
    SDL_Texture* tree_b = load_art_texture(renderer, "assets/tree_b.bmp", true);
    SDL_Texture* tree_c = load_art_texture(renderer, "assets/tree_c.bmp", true);
    auto tree_size = [](SDL_Texture* t, int& w, int& h) {
        if (t) SDL_QueryTexture(t, nullptr, nullptr, &w, &h);
    };
    int taw = 0, tah = 0, tbw = 0, tbh = 0, tcw = 0, tch = 0;
    tree_size(tree_a, taw, tah);
    tree_size(tree_b, tbw, tbh);
    tree_size(tree_c, tcw, tch);

    // Anchors are x-only; the y below is a fallback that is overwritten by the
    // terrain scan after the scene loads (see `snap_prop_to_terrain`). Writing
    // a y here at all is what put three of these trees inside the snowbank:
    // FLOOR_TOP is the floor slab's top, and the authored sand slope rises up
    // to 60 cells above it, so "the ground" is not one number.
    constexpr float GROUND_Y = 380.0f; // FLOOR_TOP in generate_test_scene.py
    std::vector<Prop> props = {
        { tree_c, tcw, tch, 40.0f,  GROUND_Y },
        { tree_a, taw, tah, 58.0f,  GROUND_Y },
        { tree_b, tbw, tbh, 75.0f,  GROUND_Y },
        { tree_a, taw, tah, 255.0f, GROUND_Y },
        { tree_c, tcw, tch, 270.0f, GROUND_Y },
        { tree_b, tbw, tbh, 575.0f, GROUND_Y },
        { tree_a, taw, tah, 592.0f, GROUND_Y },
        { tree_c, tcw, tch, 610.0f, GROUND_Y },
        { tree_b, tbw, tbh, 625.0f, GROUND_Y },
    };

    // The reticle *is* the cursor now, so the OS one would be a second pointer
    // sitting on top of it. SDL scopes this to its own window rather than
    // globally, so the desktop pointer is untouched the moment the mouse leaves
    // - which is why this can be set once here instead of tracked per frame.
    SDL_ShowCursor(SDL_DISABLE);

    // A streaming texture is created with undefined contents, and the upload
    // below only ever writes the rect the grid actually covers. Those are the
    // same rect in every world this project currently builds - but a world
    // smaller than the viewport on either axis leaves the remainder holding
    // whatever the driver's allocation happened to contain, and it would show
    // as garbage along the edge rather than as the backdrop. One clear at
    // startup rather than a per-frame guard, since it can only ever be wrong
    // once.
    {
        const std::vector<uint32_t> blank(static_cast<size_t>(PADDED_WIDTH) * PADDED_HEIGHT, 0);
        SDL_UpdateTexture(texture, nullptr, blank.data(), PADDED_WIDTH * sizeof(uint32_t));
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

    // Plant each prop on the terrain that is actually under it, rather than on
    // a hardcoded ground line. **This is a fix for a class of bug, not for the
    // three trees that had it:** props were authored at `FLOOR_TOP`, which is
    // true of the floor slab and false of everything standing on it, so the
    // three trees over the authored sand slope were 26%, 43% and 83% buried -
    // invisibly, because they sit off-screen at spawn and the screenshot that
    // "confirmed" the feature was of the other side of the world.
    //
    // Scans the prop's own footprint rather than its centre column and takes
    // the *lowest* surface found, so a tree on a slope is planted at its
    // downhill edge and leans into the hill instead of floating off its uphill
    // one. Runs once, after the scene is stamped and before the first frame:
    // props are not simulated, so terrain that moves later does not drag them
    // with it - which is correct for a tree and is the same "exercises no
    // system" line that put them in this layer at all.
    for (Prop& prop : props) {
        if (!prop.texture) continue;
        const int x0 = static_cast<int>(prop.anchor_x - prop.w / 2.0f);
        int lowest_surface = -1;
        for (int x = x0; x < x0 + prop.w; ++x) {
            if (x < 0 || x >= GRID_WIDTH) continue;
            for (int y = 0; y < GRID_HEIGHT; ++y) {
                if (is_solid(run.grid.get_element(x, y).type)) {
                    if (y > lowest_surface) lowest_surface = y;
                    break;
                }
            }
        }
        // No solid ground anywhere under it is a scene-authoring mistake, not
        // something to paper over with a default - a prop hanging in the air
        // is exactly as wrong as one buried, and silently placing it at the
        // fallback is how the first version of this hid its own bug.
        if (lowest_surface < 0) {
            std::fprintf(stderr, "WARNING: prop at x=%.0f has no ground under it; left at y=%.0f\n",
                         prop.anchor_x, prop.anchor_y);
            continue;
        }
        prop.anchor_y = static_cast<float>(lowest_surface);
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
    int fps_display = 0;

    // **Rebuilt every frame, and the once-a-second version this replaces was
    // defect A2** (PLAYTEST_LOG.md session 1). The old comment here argued that
    // a string this cheap was not worth reformatting 60x/sec, which was true
    // about the cost and wrong about the consequence: the brush name and the
    // awake-chunk count are not periodic readouts like the frame rate, they are
    // answers to "what did that key just do" and "has the world settled yet".
    // Cached for a second, pressing 2 left the HUD reading SAND for up to a
    // full second, and a playtester correctly read that as the *input* being
    // sluggish. Only the frame rate is genuinely a once-a-second quantity, so
    // only the frame rate is cached now.
    //
    // The lesson worth keeping: this is an instrument, and an instrument that
    // lags makes every measurement taken with it suspect. It was the first
    // thing fixed after the playtest for that reason.
    std::string hud_text;

    // Previous frame's sub-cell player position, for render interpolation. The
    // simulation steps at 60 Hz and the display was running at 165, so each
    // simulated position was being shown for about 2.75 frames and the motion
    // stepped visibly even once the sub-cell remainder was accounted for.
    // Updated inside the step loop so it always holds the state one step behind
    // the current one - including on frames where no step runs at all, which is
    // most of them at this refresh rate.
    float prev_player_x = run.player.visual_x();
    float prev_player_y = run.player.visual_y();

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
        camera.follow(static_cast<float>(run.player.center_x()), static_cast<float>(run.player.center_y()),
                      PADDED_WIDTH, PADDED_HEIGHT, GRID_WIDTH, GRID_HEIGHT);

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
            prev_player_x = run.player.visual_x();
            prev_player_y = run.player.visual_y();
            run.step(input);
            accumulator -= Run::FIXED_DT;
        }

        // How far between the last simulated state and the current one this
        // frame falls. Zero steps this frame is the normal case at 165 Hz, and
        // it is why `prev_*` lives outside the loop: alpha keeps climbing
        // towards 1 and the draw keeps easing towards the state already
        // computed, rather than freezing until the next step lands.
        const float alpha = static_cast<float>(accumulator / Run::FIXED_DT);
        float draw_player_x = prev_player_x + (run.player.visual_x() - prev_player_x) * alpha;
        float draw_player_y = prev_player_y + (run.player.visual_y() - prev_player_y) * alpha;

        // Teleports must not be interpolated. `resolve_overlap` can shift the
        // body several cells at once to push it out of terrain, and easing
        // across that draws the player skating through the wall it was just
        // rescued from. Anything larger than a stride is a jump, not motion.
        constexpr float MAX_INTERPOLATED_CELLS = 4.0f;
        if (std::abs(run.player.visual_x() - prev_player_x) > MAX_INTERPOLATED_CELLS ||
            std::abs(run.player.visual_y() - prev_player_y) > MAX_INTERPOLATED_CELLS) {
            draw_player_x = run.player.visual_x();
            draw_player_y = run.player.visual_y();
        }

        // Re-aimed at the interpolated position before drawing. The call above
        // is what the mouse-to-world conversion needed and runs before the
        // step; this one is what the *render* needs, and using the pre-step
        // camera for it would reintroduce exactly the whole-frame lag between
        // the player and the world that this defect is about.
        camera.follow(draw_player_x + Player::WIDTH / 2.0f, draw_player_y + Player::HEIGHT / 2.0f,
                      PADDED_WIDTH, PADDED_HEIGHT, GRID_WIDTH, GRID_HEIGHT);

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
        const int visible_w = std::min(PADDED_WIDTH, GRID_WIDTH);
        const int visible_h = std::min(PADDED_HEIGHT, GRID_HEIGHT);
        const SDL_Rect visible_rect{0, 0, visible_w, visible_h};
        const uint32_t* visible_pixels = pixels.data() + camera.view_y() * GRID_WIDTH + camera.view_x();
        SDL_UpdateTexture(texture, &visible_rect, visible_pixels, GRID_WIDTH * sizeof(uint32_t));

        // V7. Computed against the same view origin the cell upload just used,
        // and after `camera.follow` for the same reason that upload is: a light
        // field built from last frame's view would slide against the world it is
        // lighting, which is defect A1 reintroduced through a new feature.
        //
        // Recomputed from scratch every frame rather than carried between them.
        // It is affordable (the scan is the padded viewport, the propagation is
        // ~2,000 blocks) and the alternative is a cache keyed on both the camera
        // and every temperature in view - which is a correctness problem in
        // exchange for saving something already too cheap to measure.
        light.update(run.grid, camera.view_x(), camera.view_y());
        if (light.any_light()) {
            SDL_UpdateTexture(light_texture, nullptr, light.pixels().data(),
                              light.cols() * sizeof(uint32_t));
        }

        // The backdrop layer (V8, replacing V1's gradient placeholder now
        // that there is authored art to show). Two static textures, each
        // shifted by the camera's continuous view position - view_x()/
        // view_y() plus the sub-cell frac_x()/frac_y() the world texture
        // below already uses for its own smooth scroll - scaled by SCALE and
        // the layer's own parallax factor. Full-texture draws with a
        // negative destination offset rather than a cropped source rect: the
        // art is static, so there is nothing to re-upload per frame, only
        // where it is drawn needs to move.
        const float cont_view_x = static_cast<float>(camera.view_x()) + camera.frac_x();
        const float cont_view_y = static_cast<float>(camera.view_y()) + camera.frac_y();

        // **The clear is load-bearing now in a way it was not before.** V1's
        // 64-band gradient filled the whole window every frame, so it doubled
        // as a clear and nothing here ever needed one. The authored sky that
        // replaced it is drawn behind an `if` - a missing or unreadable BMP
        // leaves the framebuffer holding whatever was in it, which on a
        // double-buffered renderer is two-frames-ago garbage rather than a
        // plain background. Clearing to the palette's darkest sky tone means
        // the failure mode is "the backdrop is flat" instead of "the window
        // is full of noise".
        SDL_SetRenderDrawColor(renderer, 0x14, 0x10, 0x22, 255); // sky_deep, tools/pixel_art.py
        SDL_RenderClear(renderer);

        if (backdrop_sky) {
            const SDL_FRect dst{
                -cont_view_x * Camera::SCALE * PARALLAX_SKY_X,
                -cont_view_y * Camera::SCALE * PARALLAX_SKY_Y,
                static_cast<float>(sky_w), static_cast<float>(sky_h)
            };
            SDL_RenderCopyF(renderer, backdrop_sky, nullptr, &dst);
        }
        if (backdrop_mountains) {
            const SDL_FRect dst{
                -cont_view_x * Camera::SCALE * PARALLAX_MOUNTAIN_X,
                -cont_view_y * Camera::SCALE * PARALLAX_MOUNTAIN_Y,
                static_cast<float>(mountain_w), static_cast<float>(mountain_h)
            };
            SDL_RenderCopyF(renderer, backdrop_mountains, nullptr, &dst);
        }

        // V4's props. Drawn before the cell texture on purpose - see the
        // Prop comment above main() - so a trunk that overlaps authored
        // terrain gets buried by it with no depth test and no new code path,
        // exactly the way the cell texture already occludes the backdrop
        // wherever a cell is not Empty.
        for (const Prop& prop : props) {
            if (!prop.texture) continue;
            const SDL_FRect dst{
                camera.world_to_screen_x(prop.anchor_x - prop.w / 2.0f),
                camera.world_to_screen_y(prop.anchor_y - static_cast<float>(prop.h)),
                static_cast<float>(camera.scale_length(prop.w)),
                static_cast<float>(camera.scale_length(prop.h))
            };
            SDL_RenderCopyF(renderer, prop.texture, nullptr, &dst);
        }

        // Drawn shifted by the camera's sub-cell remainder, which is the half of
        // A1 that smoothing the player alone would not have fixed: the view is
        // unclamped wherever the player usually is, so the player sits near
        // screen centre and it is the *world* that scrolls. In whole cells that
        // is a 4-pixel jerk of everything on screen at once.
        const SDL_FRect world_dst{
            -camera.frac_x() * Camera::SCALE,
            -camera.frac_y() * Camera::SCALE,
            static_cast<float>(PADDED_WIDTH * Camera::SCALE),
            static_cast<float>(PADDED_HEIGHT * Camera::SCALE)
        };
        SDL_RenderCopyF(renderer, texture, nullptr, &world_dst);

        // The player is not a cell, so it is not in the pixel buffer either -
        // it is drawn on top of the world as its own rectangle. Float rect and
        // float position: rounding either one here is the defect coming back.
        SDL_SetRenderDrawColor(renderer, 235, 235, 245, 255);
        const SDL_FRect body{
            camera.world_to_screen_x(draw_player_x),
            camera.world_to_screen_y(draw_player_y),
            static_cast<float>(camera.scale_length(Player::WIDTH)),
            static_cast<float>(camera.scale_length(Player::HEIGHT))
        };
        SDL_RenderFillRectF(renderer, &body);

        // V7's one extra RenderCopy - the whole cost of the feature on the GPU
        // side, which is what the architecture note budgeted.
        //
        // **Drawn after the world and after the player, and before the reticle
        // and HUD.** Everything in the world is a surface that light lands on,
        // including the player, who otherwise stays a flat white rectangle while
        // standing inside a fire. Everything after it is UI, which is not in the
        // world and must not be tinted by it - a reticle that goes orange near a
        // flame is the exact defect B1 was about.
        //
        // The destination is the *block* extent, not the padded cell extent, and
        // that is what aligns the stretch. `cols()*BLOCK` is the padded width
        // rounded up to a whole block, so each texel's centre lands on the centre
        // of the four-by-four cells it was computed from. Sized to the padded
        // extent instead, every texel would sit up to half a block off and the
        // glow would trail behind the flame that cast it.
        if (light.any_light()) {
            const SDL_FRect light_dst{
                -camera.frac_x() * Camera::SCALE,
                -camera.frac_y() * Camera::SCALE,
                static_cast<float>(light.cols() * LightField::BLOCK * Camera::SCALE),
                static_cast<float>(light.rows() * LightField::BLOCK * Camera::SCALE)
            };
            SDL_RenderCopyF(renderer, light_texture, nullptr, &light_dst);
        }

        // The reticle (V10/B1). Three things it has to do, and the old one-cell
        // filled square did none of them well enough to survive a playtest.
        //
        // **It is drawn in screen pixels, not cells.** A cell is 4 screen pixels
        // at the current scale, so a cell-sized marker is four pixels across and
        // any shape drawn inside one is not a shape. Screen-space also means it
        // keeps its size and legibility if the scale is ever changed.
        //
        // **It is not orange any more.** The defect behind this item was that the
        // marker sat in the same colour family as Fire and disappeared against
        // the one thing you most want to aim at. Colour now carries range only -
        // full white in reach, dim white out of it - and the outline below is
        // what carries legibility, so neither job depends on the background.
        //
        // **Each arm is drawn over a dark outline.** A white reticle would vanish
        // against snow and steam exactly as the orange one vanished against fire;
        // an outlined one cannot vanish against anything, because whatever the
        // background is, one of the two contrasts with it.
        const int dx_cells = gridX - run.player.center_x();
        const int dy_cells = gridY - run.player.center_y();
        const bool in_range = (dx_cells * dx_cells + dy_cells * dy_cells) <= DigTool::RANGE * DigTool::RANGE;

        // A gap wider than the arms are long would read as four separate marks
        // rather than one reticle; the point of leaving the centre open is that
        // the cell being aimed at stays visible, so the gap is sized to the cell.
        constexpr int TICK_GAP = 4;
        constexpr int TICK_LEN = 7;
        constexpr int TICK_THICK = 2;

        const SDL_Rect arms[4] = {
            { mouseX - TICK_THICK / 2, mouseY - TICK_GAP - TICK_LEN, TICK_THICK, TICK_LEN }, // N
            { mouseX - TICK_THICK / 2, mouseY + TICK_GAP,            TICK_THICK, TICK_LEN }, // S
            { mouseX - TICK_GAP - TICK_LEN, mouseY - TICK_THICK / 2, TICK_LEN, TICK_THICK }, // W
            { mouseX + TICK_GAP,            mouseY - TICK_THICK / 2, TICK_LEN, TICK_THICK }, // E
        };

        // Only while the pointer is actually over this window. `SDL_GetMouseState`
        // keeps reporting the last position inside the window after the mouse
        // leaves, so without this the reticle sticks to the edge and reads as a
        // frozen UI element rather than as a cursor that has gone elsewhere.
        if (SDL_GetMouseFocus() == window) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, in_range ? 170 : 120);
            for (const SDL_Rect& arm : arms) {
                const SDL_Rect outline{arm.x - 1, arm.y - 1, arm.w + 2, arm.h + 2};
                SDL_RenderFillRect(renderer, &outline);
            }

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, in_range ? 255 : 115);
            for (const SDL_Rect& arm : arms) SDL_RenderFillRect(renderer, &arm);

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        // The UI layer decision (ENGINEERING_NOTES.md): drawn here, in the
        // game window, rather than left to the OS title bar - a solid backing
        // rect first so the text stays readable over whatever the simulation
        // is doing underneath it.
        hud_text = "FPS:" + std::to_string(fps_display) +
                   " BRUSH:" + material_of(current_brush).name + "(" + std::to_string(brush_size) + ")" +
                   " CHUNKS:" + std::to_string(run.grid.active_chunk_count());

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
            // Only the frame rate is cached - it is a rate, and a rate needs an
            // interval to be measured over. The brush name and the awake-chunk
            // count are read straight from live state where the string is built
            // above; awake chunks in particular are shown because they explain
            // the frame rate, and a count that lags by a second cannot.
            fps_display = frames_this_second;
            frames_this_second = 0;
            title_timer = 0.0;
        }
    }

    if (tree_c) SDL_DestroyTexture(tree_c);
    if (tree_b) SDL_DestroyTexture(tree_b);
    if (tree_a) SDL_DestroyTexture(tree_a);
    if (backdrop_mountains) SDL_DestroyTexture(backdrop_mountains);
    if (backdrop_sky) SDL_DestroyTexture(backdrop_sky);
    SDL_DestroyTexture(light_texture);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
