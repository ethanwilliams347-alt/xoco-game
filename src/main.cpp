#include <SDL.h>
#include <cstdio>
#include <random>
#include <string>
#include "game/run.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PIXEL_SCALE = 4; // Each physics pixel is 4x4 screen pixels
const int GRID_WIDTH = WINDOW_WIDTH / PIXEL_SCALE;
const int GRID_HEIGHT = WINDOW_HEIGHT / PIXEL_SCALE;

const double MAX_FRAME_TIME = 0.25; // clamp after a stall so we don't spiral

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

    // ARGB8888 texture for direct pixel manipulation
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        GRID_WIDTH, GRID_HEIGHT
    );
    if (!texture) {
        std::fprintf(stderr, "Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
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

    bool running = true;
    SDL_Event e;

    ElementType current_brush = ElementType::Sand;
    int brush_size = 3;

    uint64_t prev_counter = SDL_GetPerformanceCounter();
    const double counter_freq = static_cast<double>(SDL_GetPerformanceFrequency());
    double accumulator = 0.0;

    int frames_this_second = 0;
    double title_timer = 0.0;

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

        // Handle continuous mouse pressing
        int mouseX, mouseY;
        const uint32_t mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        const int gridX = mouseX / PIXEL_SCALE;
        const int gridY = mouseY / PIXEL_SCALE;

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

        // Update texture with new pixels
        const std::vector<uint32_t>& pixels = run.grid.get_pixels();
        SDL_UpdateTexture(texture, nullptr, pixels.data(), GRID_WIDTH * sizeof(uint32_t));

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        // The player is not a cell, so it is not in the pixel buffer either -
        // it is drawn on top of the world as its own rectangle.
        SDL_SetRenderDrawColor(renderer, 235, 235, 245, 255);
        const SDL_Rect body{
            run.player.cell_x() * PIXEL_SCALE,
            run.player.cell_y() * PIXEL_SCALE,
            Player::WIDTH * PIXEL_SCALE,
            Player::HEIGHT * PIXEL_SCALE
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
            mark_x * PIXEL_SCALE, mark_y * PIXEL_SCALE, PIXEL_SCALE, PIXEL_SCALE
        };
        SDL_RenderFillRect(renderer, &mark);

        SDL_RenderPresent(renderer);

        // Surface the frame rate so performance regressions are visible while working.
        frames_this_second++;
        title_timer += frame_time;
        if (title_timer >= 1.0) {
            // Awake chunks are shown because they explain the frame rate: if the
            // world is idle and the count is not near zero, culling has a bug.
            const std::string title = "SLOP Pixel Physics  |  " + std::to_string(frames_this_second) + " fps  |  " +
                                      material_of(current_brush).name + "  (brush " + std::to_string(brush_size) + ")" +
                                      "  |  " + std::to_string(run.grid.active_chunk_count()) + " chunks awake";
            SDL_SetWindowTitle(window, title.c_str());
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
