#include "grid.h"
#include <algorithm>

Grid::Grid(int width, int height) : width(width), height(height) {
    cells.resize(width * height, Element{});
    pixels.resize(width * height, 0xFF000000); // ARGB Black
    std::random_device rd;
    rng.seed(rd());
}

bool Grid::is_within_bounds(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

int Grid::get_index(int x, int y) const {
    return y * width + x;
}

Element Grid::get_element(int x, int y) const {
    if (!is_within_bounds(x, y)) return Element{ElementType::Wall, 0xFF444444, false}; // Treat OOB as wall
    return cells[get_index(x, y)];
}

void Grid::set_element(int x, int y, ElementType type) {
    if (!is_within_bounds(x, y)) return;
    int idx = get_index(x, y);
    
    Element el;
    el.type = type;
    el.has_updated = true;

    // Simple color assignment based on type
    switch (type) {
        case ElementType::Empty: el.color = 0xFF000000; break; // Black background
        case ElementType::Sand:  
            // Add a tiny bit of random variation to sand color for texture
            el.color = 0xFFEEDD82; 
            break; 
        case ElementType::Water: el.color = 0xFF4444FF; break; // Blue
        case ElementType::Wall:  el.color = 0xFF888888; break; // Gray
    }
    
    cells[idx] = el;
    pixels[idx] = el.color;
}

void Grid::swap_elements(int x1, int y1, int x2, int y2) {
    int idx1 = get_index(x1, y1);
    int idx2 = get_index(x2, y2);

    std::swap(cells[idx1], cells[idx2]);
    pixels[idx1] = cells[idx1].color;
    pixels[idx2] = cells[idx2].color;
    
    // Mark as updated so they don't move again this frame
    cells[idx1].has_updated = true;
    cells[idx2].has_updated = true;
}

void Grid::update() {
    // Reset updated flags
    for (auto& cell : cells) {
        cell.has_updated = false;
    }

    // Iterate from bottom to top
    for (int y = height - 1; y >= 0; --y) {
        // Randomize x direction to prevent bias (sand piling to one side)
        int start_x = 0;
        int end_x = width;
        int step = 1;
        if (rng() % 2 == 0) {
            start_x = width - 1;
            end_x = -1;
            step = -1;
        }

        for (int x = start_x; x != end_x; x += step) {
            int idx = get_index(x, y);
            Element& current = cells[idx];

            if (current.has_updated || current.type == ElementType::Empty || current.type == ElementType::Wall) {
                continue;
            }

            if (current.type == ElementType::Sand) {
                // Try straight down
                if (is_within_bounds(x, y + 1) && cells[get_index(x, y + 1)].type == ElementType::Empty) {
                    swap_elements(x, y, x, y + 1);
                }
                // Try down-left or down-right
                else {
                    int dir = (rng() % 2 == 0) ? -1 : 1;
                    if (is_within_bounds(x + dir, y + 1) && cells[get_index(x + dir, y + 1)].type == ElementType::Empty) {
                        swap_elements(x, y, x + dir, y + 1);
                    } else if (is_within_bounds(x - dir, y + 1) && cells[get_index(x - dir, y + 1)].type == ElementType::Empty) {
                        swap_elements(x, y, x - dir, y + 1);
                    } else if (is_within_bounds(x, y + 1) && cells[get_index(x, y + 1)].type == ElementType::Water) {
                        // Sand is heavier, it sinks in water
                        swap_elements(x, y, x, y + 1);
                    }
                }
            }
            else if (current.type == ElementType::Water) {
                // Try straight down
                if (is_within_bounds(x, y + 1) && cells[get_index(x, y + 1)].type == ElementType::Empty) {
                    swap_elements(x, y, x, y + 1);
                }
                // Try down-left or down-right
                else {
                    int dir = (rng() % 2 == 0) ? -1 : 1;
                    if (is_within_bounds(x + dir, y + 1) && cells[get_index(x + dir, y + 1)].type == ElementType::Empty) {
                        swap_elements(x, y, x + dir, y + 1);
                    } else if (is_within_bounds(x - dir, y + 1) && cells[get_index(x - dir, y + 1)].type == ElementType::Empty) {
                        swap_elements(x, y, x - dir, y + 1);
                    }
                    // If blocked below, try left or right to spread out
                    else if (is_within_bounds(x + dir, y) && cells[get_index(x + dir, y)].type == ElementType::Empty) {
                        swap_elements(x, y, x + dir, y);
                    } else if (is_within_bounds(x - dir, y) && cells[get_index(x - dir, y)].type == ElementType::Empty) {
                        swap_elements(x, y, x - dir, y);
                    }
                }
            }
        }
    }
}
