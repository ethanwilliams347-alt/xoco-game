#pragma once
#include "element.h"
#include <vector>
#include <cstdint>
#include <random>

class Grid {
public:
    // Side length of one simulation chunk, in cells. Each chunk tracks the
    // bounds of the cells inside it that might still move, and a chunk with
    // nothing moving is skipped entirely.
    //
    // Swept over 16/32/64/128 with tests/bench_grid.cpp: the active scenarios are
    // identical to within run-to-run noise, and the sleeping ones get slightly
    // cheaper as the chunk grows. 64 is the largest size that still gives useful
    // culling granularity when several small things are moving in different parts
    // of the world.
    static constexpr int CHUNK_SIZE = 64;

    Grid(int width, int height);
    ~Grid() = default;

    // Advances the simulation by exactly one fixed step. The caller is
    // responsible for calling this at a fixed rate, not once per rendered frame.
    void update();

    void set_element(int x, int y, ElementType type);
    Element get_element(int x, int y) const;

    // Get raw pixel colors for rendering to SDL Texture
    const std::vector<uint32_t>& get_pixels() const { return pixels; }

    int get_width() const { return width; }
    int get_height() const { return height; }

    // Number of chunks that will be simulated on the next step. Zero means the
    // world has come completely to rest. Exposed for tests and for the on-screen
    // diagnostic, so "is anything actually sleeping?" is observable rather than
    // assumed.
    int active_chunk_count() const;

private:
    // Bounds of the cells within one chunk that may still move, in world
    // coordinates, inclusive on both ends. max < min means the chunk is asleep.
    struct DirtyRect {
        int min_x = 0, min_y = 0, max_x = -1, max_y = -1;

        bool is_empty() const { return max_x < min_x; }
        void clear() { min_x = 0; min_y = 0; max_x = -1; max_y = -1; }

        void include(int x, int y) {
            if (is_empty()) {
                min_x = max_x = x;
                min_y = max_y = y;
                return;
            }
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
        }
    };

    int width;
    int height;

    int chunks_x;
    int chunks_y;

    // Two sets of bounds, not one. Growing the rect that is currently being
    // iterated would change the loop bounds underneath the loop; instead every
    // write during a step lands in `chunk_next`, which is swapped in at the
    // start of the following step.
    std::vector<DirtyRect> chunk_current;
    std::vector<DirtyRect> chunk_next;

    // Wakes the cell at (x, y) and its neighbours for the next step.
    void mark_dirty(int x, int y);

    // We maintain two arrays: one for the physics logic (elements)
    // and one raw color buffer (pixels) that SDL reads directly.
    std::vector<Element> cells;
    std::vector<uint32_t> pixels;

    bool is_within_bounds(int x, int y) const;
    int get_index(int x, int y) const;
    void swap_elements(int x1, int y1, int x2, int y2);

    // Runs the one cell at (x, y) through its material's behaviour.
    void step_cell(int x, int y);

    // Incremented once per step and stamped onto every cell visited. Wraps at
    // 256, which is harmless: a cell that has been asleep for an exact multiple
    // of 256 steps is skipped for a single step and runs the next one.
    uint8_t frame_tag = 0;

    // True if the cell at (x, y) may move into (tx, ty). Vertical moves are
    // allowed only when gravitationally favourable: a mover travelling down
    // must be denser than its target, one travelling up must be lighter.
    bool can_displace(const Material& mover, int tx, int ty, int dy) const;

    // Per-behaviour steps. Each returns true if the cell moved.
    bool step_powder(int x, int y, const Material& mat);
    bool step_fluid(int x, int y, const Material& mat, int dy);

    uint32_t jittered_color(const Material& mat);

    // Random generator for scattering and colour variation
    std::mt19937 rng;
};
