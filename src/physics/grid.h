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

    // A falling structure speeds up. It moves one cell on the step it comes
    // loose and gains a cell per step for every TICKS_PER_SPEEDUP steps it
    // stays in the air, up to MAX_FALL_SPEED.
    //
    // Speed is repetition, not a longer stride: a piece never travels a cell
    // without first re-deriving what is holding it up, so moving at 8 costs
    // eight flood fills in a step. That is the whole reason there is a ceiling.
    // It also means a fast piece cannot pass through a floor thinner than its
    // speed, which a "move N cells then look" implementation would have to
    // guard against explicitly.
    //
    // Read against 1 cell = 1 cm: at 60 Hz, +1 cell/step every 4 steps is
    // ~15 m/s^2 and the ceiling is 4.8 m/s. Heavier than real gravity, which
    // suits masonry, and it reaches the ceiling in about half a second.
    static constexpr int MAX_FALL_SPEED = 8;
    static constexpr int TICKS_PER_SPEEDUP = 4;

    // Cells per step for a piece that has been falling for `ticks` steps.
    static int fall_speed(uint8_t ticks);

    // Used when a caller does not supply a seed. Any fixed value would do - the
    // point is only that it is fixed, so that every test is reproducible without
    // having to name a seed it does not care about.
    static constexpr uint64_t DEFAULT_SEED = 1;

    // The grid takes its seed rather than finding one. Nothing in here reads the
    // clock or the OS entropy pool, so a given (seed, sequence of writes, number
    // of steps) always produces the same world - which is what makes a save file,
    // a shared level and a reproducible bug report possible. Deciding where the
    // seed comes from is the caller's job, and `main.cpp` is the only place in
    // the project that is allowed to be nondeterministic about it.
    Grid(int width, int height, uint64_t seed = DEFAULT_SEED);
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

    // Readable so it can be written to a save file, shown to a player, or quoted
    // in a bug report. A seed that cannot be read back is only half of
    // determinism: reproducing a run needs the number, not just the guarantee
    // that one exists.
    uint64_t seed() const { return world_seed; }

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

    // Works through that list. Called once per step, before the sweep, and
    // internally runs up to MAX_FALL_SPEED passes over it -- one cell of travel
    // each -- so that pieces already up to speed get further in the step than
    // ones that have just come loose.
    void resolve_support();

    // Flood-fills the structure containing (x, y). If no cell in it is grounded,
    // the whole piece moves down exactly one cell, shape intact.
    void fall_if_unsupported(int x, int y);

    // Translates the piece currently in support_component down by one cell and
    // re-queues it so it keeps falling on the next step.
    void drop_component();

    // What a fill concluded about a cell. Only meaningful while that cell's
    // support_visit stamp holds the current epoch.
    //
    // Recording the conclusion and not merely "seen" is load-bearing. A fill
    // stops the instant it finds one grounded cell, so it leaves a partial
    // trail of marks across a piece it has just decided is held up. Without a
    // reason attached, a later seed from that same piece starts its own fill,
    // cannot cross that trail, never reaches the grounded cell, and drops
    // whatever subset it could reach - a structure standing on solid ground
    // sheds chunks of itself for no visible reason.
    enum class SupportState : uint8_t {
        Pending,   // reached by the fill running right now
        Supported, // its piece will not fall this step
        Moved,     // its piece already fell this step
    };

    // Stamps every cell this fill has marked with `state`, plus `extra` if it is
    // not negative (the cell in hand, popped but not yet filed anywhere).
    // Settling on Supported also puts those cells back at rest, since that is
    // the only way a piece ever stops falling.
    void settle_marks(SupportState state, int extra);

    std::vector<int> pending_support;
    std::vector<int> support_stack;     // scratch, reused across fills
    std::vector<int> support_component; // scratch, reused across fills

    // Per-cell "seen this pass" marker. One byte and an epoch counter rather
    // than a bool array that would need clearing every time; the whole thing is
    // cleared once every 255 passes, when the epoch wraps.
    //
    // The epoch advances once per *pass*, not once per fill. Marks therefore
    // outlive the fill that made them, which is what lets one fill answer for
    // the next: a fill that runs into a cell an earlier one already settled
    // adopts that answer instead of re-deriving it, because two connected cells
    // are by definition the same piece. It is a memo, so it also makes the
    // common case - hundreds of seeds queued off one piece - cheap.
    //
    // They must *not* outlive the pass: the world has moved by then, so an
    // answer from the previous cell of travel is about a world that no longer
    // exists.
    std::vector<uint8_t> support_visit;
    std::vector<uint8_t> support_state;
    uint8_t support_epoch = 0;

    // Falling writes cells, and those writes would otherwise queue support
    // checks for the fall that is already in progress.
    bool resolving_support = false;

    uint32_t jittered_color(const Material& mat);

    uint64_t world_seed;

    // Random generator for scattering and colour variation.
    //
    // Scheduled for removal (Foundations F1.6): a stateless hash of position and
    // step number gives the same numbers without 2.5 KB of state that a save file
    // would otherwise have to carry, and is faster in the inner loop besides. The
    // seed is already owned above, so that change is confined to the five call
    // sites and does not reach the constructor again.
    std::mt19937 rng;
};
