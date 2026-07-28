#include <SDL.h>
#include <cstdio>
#include <string>
#include "physics/grid.h"
#include "physics/player.h"
#include "physics/tool.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PIXEL_SCALE = 4; // Each physics pixel is 4x4 screen pixels
const int GRID_WIDTH = WINDOW_WIDTH / PIXEL_SCALE;
const int GRID_HEIGHT = WINDOW_HEIGHT / PIXEL_SCALE;

// The simulation advances in fixed steps so that sand falls at the same rate on
// a 60 Hz and a 144 Hz display. Rendering still runs as fast as the display allows.
const double FIXED_DT = 1.0 / 60.0;
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

    Grid grid(GRID_WIDTH, GRID_HEIGHT); // starts fully Empty

    // Spawned in mid-air over an empty world. There is no terrain to land on
    // yet, but the world border reads as solid, so the player falls to the
    // bottom edge rather than out of existence.
    Player player(GRID_WIDTH / 2, GRID_HEIGHT / 4);
    DigTool dig_tool;

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

        // Right-click, not left. Digging is the game's action and gets the
        // primary button; the material brush is a development tool and moved
        // out of its way.
        if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            // Draw a brush circle
            for (int dy = -brush_size; dy <= brush_size; dy++) {
                for (int dx = -brush_size; dx <= brush_size; dx++) {
                    if (dx * dx + dy * dy <= brush_size * brush_size) {
                        grid.set_element(gridX + dx, gridY + dy, current_brush);
                    }
                }
            }
        }

        // Movement is read from the live key state rather than from key events,
        // so holding a key keeps moving instead of firing once and repeating on
        // the OS key-repeat delay.
        const uint8_t* keys = SDL_GetKeyboardState(nullptr);
        PlayerInput input;
        input.left  = keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT];
        input.right = keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT];
        input.jump  = keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP];
        input.aim_x = gridX;
        input.aim_y = gridY;
        input.dig   = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

        const uint64_t now_counter = SDL_GetPerformanceCounter();
        double frame_time = static_cast<double>(now_counter - prev_counter) / counter_freq;
        prev_counter = now_counter;
        if (frame_time > MAX_FRAME_TIME) frame_time = MAX_FRAME_TIME;

        accumulator += frame_time;
        while (accumulator >= FIXED_DT) {
            grid.update();
            // After the grid, so the player collides against the world as it
            // now is rather than as it was a step ago.
            player.update(grid, input, static_cast<float>(FIXED_DT));
            // Last, so the dig is aimed from where the body actually ended up
            // this step. Called every step whether or not the button is held,
            // because that is what advances the tool's cooldown.
            dig_tool.update(grid, input.dig, player.center_x(), player.center_y(),
                            input.aim_x, input.aim_y);
            accumulator -= FIXED_DT;
        }

        // Update texture with new pixels
        const std::vector<uint32_t>& pixels = grid.get_pixels();
        SDL_UpdateTexture(texture, nullptr, pixels.data(), GRID_WIDTH * sizeof(uint32_t));

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        // The player is not a cell, so it is not in the pixel buffer either -
        // it is drawn on top of the world as its own rectangle.
        SDL_SetRenderDrawColor(renderer, 235, 235, 245, 255);
        const SDL_Rect body{
            player.cell_x() * PIXEL_SCALE,
            player.cell_y() * PIXEL_SCALE,
            Player::WIDTH * PIXEL_SCALE,
            Player::HEIGHT * PIXEL_SCALE
        };
        SDL_RenderFillRect(renderer, &body);

        // Where a dig would actually land. Without this the range limit is
        // invisible and a shot that stopped short reads as the tool being
        // broken rather than as the target being too far away.
        int mark_x = 0;
        int mark_y = 0;
        dig_tool.aim_point(grid, player.center_x(), player.center_y(), gridX, gridY, mark_x, mark_y);
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
                                      "  |  " + std::to_string(grid.active_chunk_count()) + " chunks awake";
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
