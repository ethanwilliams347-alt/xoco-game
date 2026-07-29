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

    // Largest connected structure the support check will judge. Past this, the
    // structure is assumed supported and left alone.
    //
    // The asymmetry is deliberate. A missed collapse is invisible - a slab that
    // should have fallen simply doesn't. A wrong collapse turns a level into
    // rubble. When the answer is too expensive to compute, guess the harmless
    // way.
    static constexpr int MAX_SUPPORT_CELLS = 4096;

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

    // True if any of the 8 cells surrounding (x, y) is of type t. Reads cells[]
    // directly rather than going through get_element(), which treats
    // out-of-bounds as Wall - correct for physics sealing, wrong here since it
    // would make every world edge silently act as a Wall catalyst.
    bool has_neighbor(int x, int y, ElementType t) const;

    // Checks (x, y) against the REACTIONS table and converts it via
    // set_element() on success. Returns true if the cell was converted, in
    // which case step_cell skips movement for it this frame.
    bool try_react(int x, int y);

    // --- structures and falling ---
    //
    // Static materials hold their shape, which means they can also hold it
    // somewhere they have no business holding it: dig the ground out from under
    // a stone slab and it hangs in mid-air. Sand next to it falls correctly, so
    // the inconsistency is visible side by side.
    //
    // An unsupported structure falls **as one rigid piece**, keeping its shape
    // the whole way down. It is not converted into loose grains: a stone slab
    // that dissolves the moment it comes free reads as a bug of a different
    // kind, and the shape is the thing that makes it look like masonry rather
    // than gravel. The piece stays in the cell grid while it falls, which is
    // what keeps rendering, player collision and digging working on it
    // unchanged - it is a rigid body only in how it moves, not in where it
    // lives.
    //
    // Support is checked on **disturbance only**, never as a global truth. A
    // sweep of the whole world every step would cost more than the simulation
    // it is attached to, and a world as authored is assumed to be standing up
    // on purpose. So a structure nobody has touched is never questioned; the
    // moment part of one is removed, what was leaning on it gets re-examined.

    // True if (x, y) is held up from directly below - by the floor of the world,
    // or by something solid that is not part of the same structure. Powders
    // bear load; liquids and gases do not.
    bool is_grounded(int x, int y) const;

    // Adds (x, y) to the list to re-examine, if it is a structure cell at all.
    void queue_support_check(int x, int y);

    // Works through that list. Called once per step, before the sweep.
    void resolve_support();

    // Flood-fills the structure containing (x, y). If no cell in it is grounded,
    // the whole piece moves down exactly one cell, shape intact.
    void fall_if_unsupported(int x, int y);

    // Translates the piece currently in support_component down by one cell and
    // re-queues it so it keeps falling on the next step.
    void drop_component();

    std::vector<int> pending_support;
    std::vector<int> support_stack;     // scratch, reused across fills
    std::vector<int> support_component; // scratch, reused across fills

    // Per-cell "seen this step" marker. One byte and an epoch counter rather
    // than a bool array that would need clearing every time; the whole thing is
    // cleared once every 255 steps, when the epoch wraps.
    //
    // The epoch advances once per *step*, not once per fill, and that is what
    // stops one piece being dropped twice in a step when several of its cells
    // were queued: a flood fill only ever reaches cells of one piece, so a
    // second seed finding itself already marked is proof it belongs to a piece
    // that has already been dealt with.
    std::vector<uint8_t> support_visit;
    uint8_t support_epoch = 0;

    // Falling writes cells, and those writes would otherwise queue support
    // checks for the fall that is already in progress.
    bool resolving_support = false;

    uint32_t jittered_color(const Material& mat);

    // Random generator for scattering and colour variation
    std::mt19937 rng;
};
