#include "scene.h"

void load_scene(Grid& grid, const Scene& scene, int offset_x, int offset_y) {
    if (scene.width <= 0 || scene.height <= 0) return;
    if (scene.materials.size() != static_cast<size_t>(scene.width * scene.height)) return;
    if (scene.albedo.size() != static_cast<size_t>(scene.width * scene.height)) return;

    for (int y = 0; y < scene.height; ++y) {
        for (int x = 0; x < scene.width; ++x) {
            int scene_idx = y * scene.width + x;
            int grid_x = x + offset_x;
            int grid_y = y + offset_y;
            
            ElementType type = scene.materials[scene_idx];
            uint32_t color = scene.albedo[scene_idx];
            
            grid.paint(grid_x, grid_y, type, color);
        }
    }
}
