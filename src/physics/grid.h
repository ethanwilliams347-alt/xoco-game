#pragma once
#include "element.h"
#include <vector>
#include <cstdint>
#include <random>

class Grid {
public:
    Grid(int width, int height);
    ~Grid() = default;

    void update();
    void set_element(int x, int y, ElementType type);
    Element get_element(int x, int y) const;
    
    // Get raw pixel colors for rendering to SDL Texture
    const std::vector<uint32_t>& get_pixels() const { return pixels; }

    int get_width() const { return width; }
    int get_height() const { return height; }

private:
    int width;
    int height;
    
    // We maintain two arrays: one for the physics logic (elements)
    // and one raw color buffer (pixels) that SDL reads directly.
    std::vector<Element> cells;
    std::vector<uint32_t> pixels;

    bool is_within_bounds(int x, int y) const;
    int get_index(int x, int y) const;
    void swap_elements(int x1, int y1, int x2, int y2);
    
    // Random generator for things like sand scattering
    std::mt19937 rng;
};
