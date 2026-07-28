#include <SDL.h>
#include <iostream>
#include "physics/grid.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PIXEL_SCALE = 4; // Each physics pixel is 4x4 screen pixels
const int GRID_WIDTH = WINDOW_WIDTH / PIXEL_SCALE;
const int GRID_HEIGHT = WINDOW_HEIGHT / PIXEL_SCALE;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SLOP Pixel Physics", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        WINDOW_WIDTH, WINDOW_HEIGHT, 
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // ARGB8888 texture for direct pixel manipulation
    SDL_Texture* texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ARGB8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        GRID_WIDTH, GRID_HEIGHT
    );

    Grid grid(GRID_WIDTH, GRID_HEIGHT);

    // Initialize all to empty explicitly
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            grid.set_element(x, y, ElementType::Empty);
        }
    }

    bool running = true;
    SDL_Event e;
    
    ElementType current_brush = ElementType::Sand;
    int brush_size = 3;

    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            // Switch elements with keys
            else if (e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    case SDLK_1: current_brush = ElementType::Sand; break;
                    case SDLK_2: current_brush = ElementType::Water; break;
                    case SDLK_3: current_brush = ElementType::Wall; break;
                    case SDLK_4: current_brush = ElementType::Empty; break; // Eraser
                    case SDLK_ESCAPE: running = false; break;
                }
            }
        }

        // Handle continuous mouse pressing
        int mouseX, mouseY;
        uint32_t mouseState = SDL_GetMouseState(&mouseX, &mouseY);
        
        if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            int gridX = mouseX / PIXEL_SCALE;
            int gridY = mouseY / PIXEL_SCALE;
            
            // Draw a brush circle
            for (int dy = -brush_size; dy <= brush_size; dy++) {
                for (int dx = -brush_size; dx <= brush_size; dx++) {
                    if (dx*dx + dy*dy <= brush_size*brush_size) {
                        grid.set_element(gridX + dx, gridY + dy, current_brush);
                    }
                }
            }
        }

        // Update physics
        grid.update();

        // Update texture with new pixels
        const std::vector<uint32_t>& pixels = grid.get_pixels();
        SDL_UpdateTexture(texture, nullptr, pixels.data(), GRID_WIDTH * sizeof(uint32_t));

        // Render
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
