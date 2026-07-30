#pragma once
#include "physics/element.h"
#include "physics/grid.h"
#include <vector>
#include <cstdint>

struct Scene {
    int width = 0;
    int height = 0;
    std::vector<ElementType> materials;
    std::vector<uint32_t> albedo;
};

void load_scene(Grid& grid, const Scene& scene, int offset_x = 0, int offset_y = 0);
