#pragma once
#include "element.h"
#include "random.h"
#include <vector>
#include <cstdint>

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

    // Puts the grid back to exactly the state a fresh `Grid(width, height,
    // seed)` would be in, without reallocating `cells`/`pixels`/the chunk and
    // support vectors - only clearing what they hold. Written as an explicit
    // member-by-member wipe rather than `*this = Grid(width, height, seed)` on
    // purpose: the compiler-generated assignment that shortcut would use cannot
    // forget a member, which would make the whole point of the test below - "a
    // field left out of the wipe" - impossible to demonstrate. A hand-written
    // wipe can forget one, so the test that catches that is worth keeping honest.
    //
    // Takes a seed rather than keeping the old one, because `Run::reset(seed)`
    // needs to hand the player a new world, and because the seed has to be one
    // of the things this clears for "reset run == fresh run with the same seed"
    // to mean anything - compare `seed()` unchanged with a fresh grid built on
    // a *different* seed and the two would trivially fail to match for a reason
    // that has nothing to do with whether the wipe worked.
    void reset(uint64_t seed);

    // Advances the simulation by exactly one fixed step. The caller is
    // responsible for calling this at a fixed rate, not once per rendered frame.
    void update();

    void set_element(int x, int y, ElementType type);
    void paint(int x, int y, ElementType type, uint32_t color);
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

    // Steps completed since construction - and, while a step is in progress, the
    // 1-based index of that step. Readable for the same reason the seed is: a
    // save file needs both numbers to say where a run had got to, and together
    // they are the whole of what the simulation's randomness will depend on once
    // the generator is gone (F1.3).
    uint64_t steps() const { return step_count; }

    // Number of chunks that will be simulated on the next step. Zero means the
    // world has come completely to rest. Exposed for tests and for the on-screen
    // diagnostic, so "is anything actually sleeping?" is observable rather than
    // assumed.
    int active_chunk_count() const;

    // Whether a structure removal is still waiting to be re-examined by
    // `resolve_support()`. Exposed for the same reason `active_chunk_count()`
    // is: it is the one piece of internal state a reset test needs to see to
    // prove the queue was actually cleared, not just that it looked cleared
    // because nothing was stepping it.
    bool has_pending_support_checks() const { return !pending_support.empty(); }

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

    // The body shared by set_element and paint once each has resolved its own
    // colour and confirmed the write is in bounds. See the definition for why
    // this is one function rather than two copies.
    void place(int x, int y, ElementType type, uint32_t color);

    // Runs the one cell at (x, y) through its material's behaviour.
    void step_cell(int x, int y);

    // Incremented once per step and stamped onto every cell visited. Wraps at
    // 256, which is harmless: a cell that has been asleep for an exact multiple
    // of 256 steps is skipped for a single step and runs the next one.
    uint8_t frame_tag = 0;

    // The simulation's clock. Distinct from `frame_tag` above despite both being
    // counters, and the two must not be merged into one.
    //
    // `frame_tag` is a skip flag - its only question is "has this cell already
    // moved this step" - and one byte is the right size for it, since a cell
    // asleep for an exact multiple of 256 steps simply waits one extra step. This
    // is a clock, and it will be the time input to the hash that replaces `rng`
    // (F1.3), where a byte would be actively wrong: randomness for a given cell
    // would repeat every 256 steps, which is visible periodicity rather than
    // noise.
    //
    // Incremented at the very top of update(), before resolve_support(), so that
    // everything happening within one step agrees on which step it is. Note that
    // `frame_tag` deliberately does *not* move up there with it - it is stamped
    // after support resolves so that cells a falling structure just moved are not
    // marked as already updated, and still get their turn in the sweep.
    uint64_t step_count = 0;

    // True if the cell at (x, y) may move into (tx, ty). Vertical moves are
    // allowed only when gravitationally favourable: a mover travelling down
    // must be denser than its target, one travelling up must be lighter.
    bool can_displace(const Material& mover, int tx, int ty, int dy) const;

    // Per-behaviour steps. Each returns true if the cell moved.
    bool step_powder(int x, int y, const Material& mat);
    bool step_fluid(int x, int y, const Material& mat, int dy);

    // --- liquids find their level ---
    //
    // can_displace refuses every upward move unless the mover is lighter than
    // its target, and Empty has density 0, so a liquid can *never* rise under
    // any circumstances. It falls and it spreads sideways into Empty, which is a
    // powder that happens to flow, not a fluid: a U-bend cannot equalize,
    // because the short arm has no way to gain a cell.
    //
    // **The rule below moves the tall arm's surface cell down onto the short
    // arm's surface, rather than making the short arm rise.** The substitution
    // is the design decision and not an approximation of one, so it is worth
    // being clear about what was tried first: letting a cell rise into the Empty
    // above it. That version worked, briefly, and then stopped. Rising is a
    // swap, so it leaves a bubble of Empty inside the body, and the transfer is
    // only finished once the ordinary fall and spread rules have walked that
    // bubble back down the arm, along the join and up the far side -- twenty-odd
    // steps, during which the bubble itself cuts the body in two, so the search
    // below transiently answers "no head" and the cells stop marking themselves
    // dirty. The chunk sleeps, and the U-tube parks itself two cells out of
    // level with nothing left awake to notice. Every fix for that is a way of
    // keeping a body awake while it is unlevel, which is a standing cost paid by
    // every pool in the world to serve the case that is out of level.
    //
    // Moving the tall cell instead makes each transfer a single atomic swap, so
    // there is no journey to stay awake for, and the wake-up is local and
    // automatic: the vacated cell's 3x3 mark is exactly the cell below it, which
    // is the next surface cell and the next one to transfer. A body equalizes at
    // one cell per step and then sleeps, with no self-wake rule of its own.
    //
    // The visible cost is that a cell can travel further than a cell should in
    // one step. It is bounded by the search below, it only ever happens between
    // two points of one connected body of the same liquid, and what it looks
    // like is one side dropping while the other rises -- which is what a U-tube
    // does. Conservation is what keeps this honest, and it is exactly the swap
    // that gives it: the obvious way to make water level is to invent some.

    // How far the connected-liquid search may look before giving up. Bounded for
    // the usual reason -- a settled pool's every surface cell asks this question
    // on every step it is awake, so the cost has to be a constant rather than
    // the size of the pool. Giving up too early is the harmless direction: a
    // body wider than this equalizes in several hops instead of one.
    //
    // **Raised from 64 when the lateral spread stopped doing this job by
    // accident.** The old spread rule let a surface cell walk sideways into any
    // Empty at all, which jittered forever on a flat surface -- but that same
    // walk was what carried cells off the top of a mound and out to the thin
    // edges of a pool, several cells a step, for free. Refusing the pointless
    // half of it (see step_fluid) also refused the useful half, and a poured
    // column settled into a permanent 6-cell-high heap that then went to sleep:
    // level transport had been resting on the jitter the whole time.
    //
    // 64 is far shorter than it reads. The search is breadth-first through the
    // *body*, not along its surface, and it spends its budget in both directions
    // at once, so in a pool five cells deep 64 cells buys about six columns of
    // reach either way. The heap above needed seven. Priced against the thing
    // that made it affordable: settled bodies now sleep instead of asking this
    // question forever, so the budget is spent while a pool is actually moving
    // and not as a standing cost -- which is the opposite of the trade the
    // original 64 was picked under.
    static constexpr int MAX_PRESSURE_CELLS = 512;

    // How much lower the receiving surface has to be before a cell will move to
    // it, in cells. Two, not one, and that is hysteresis rather than a tuning
    // preference: each transfer moves the two surfaces one cell towards each
    // other, so at a threshold of one a body one cell out of level would swap
    // which side was high, forever. Two settles, at the cost of "level" meaning
    // level to within a cell.
    static constexpr int MIN_PRESSURE_HEAD = 2;

    // Searches the body of `type` connected to (x, y) for a surface cell -- one
    // with Empty directly above it -- at least MIN_PRESSURE_HEAD rows lower.
    // Returns its index, or -1. Breadth-first and orthogonally connected:
    // nearest-first is what makes the cap bite evenly in every direction rather
    // than all down one arm, and a diagonal step would let two pools that merely
    // touch at a corner equalize into each other.
    int find_lower_surface(int x, int y, ElementType type);

    // The move itself: one cell from (x, y) onto whatever lower surface the
    // search found. Returns true if the cell moved.
    bool seek_level(int x, int y);

    std::vector<int> pressure_queue;     // scratch, reused across searches

    // A per-cell "seen this pass" marker, on the same epoch trick as
    // support_visit - and **shared by two unrelated searches**, which is why it
    // is named for what it is rather than for either of them. It was called
    // `pressure_visit` while `fracture_landing()` was quietly its second user,
    // which is the kind of name that turns a deliberate sharing into a
    // surprise.
    //
    // The two never overlap, and the reason is an ordering rather than
    // anything local: `fracture_landing()` runs only inside `resolve_support()`,
    // `find_lower_surface()` runs only inside the cell sweep, and support
    // resolves in full before the sweep starts. They also advance the epoch on
    // different schedules - once per support *pass* so one landing costs one
    // fill, once per *call* because two adjacent surface cells are asking
    // genuinely different questions.
    //
    // **That invariant is not enforced by anything, so a third user is not free.**
    // Anything that wants these marks from inside the sweep and inside support
    // resolution both, or that adds a pass between the two, needs its own array
    // rather than a third schedule on this one.
    std::vector<uint8_t> scratch_visit;
    uint8_t scratch_epoch = 0;

    // --- heat ---
    //
    // Fire used to spread by dice: a Wood cell touching a flame rolled 12% a
    // step and either caught or did not, which is why it never looked like it
    // was burning *through* anything - there was no state between "wood" and
    // "on fire" for the eye to follow. Nothing in the world had a temperature,
    // so nothing could warm, char, or cool.
    //
    // Heat is the seventh axis, and Foundations was explicitly forbidden from
    // adding one. This is the considered decision to spend it, and what keeps
    // it affordable is that it stays a *number on a cell that flows downhill*
    // and nothing more: no energy, no mass, no phase state, no second pass over
    // the world. It rides the existing sweep, one visit per awake cell, and it
    // reaches equilibrium and stops, which is the property everything else here
    // depends on.
    //
    // **Integer arithmetic only.** Floating-point diffusion would put
    // cross-platform nondeterminism back into `Grid`, which is exactly the
    // property F1 spent seven steps establishing and F1.7 wrote down as an
    // invariant. There is no float anywhere below and there must not be one.

    // The two rates, as reciprocals: heat moved per step is
    // `(difference * conductivity) / divisor`. Conduction between neighbours is
    // eight times faster than the bleed back to ambient, which is what lets a
    // flame push heat into a wall faster than the wall can shed it - the other
    // way round and nothing would ever get hot enough to ignite.
    static constexpr int CONDUCTION_DIVISOR = 1024;
    static constexpr int AMBIENT_DIVISOR = 8192;

    // Runs conduction, the ambient bleed and the heat-source clamp for one cell,
    // and marks it dirty if any of the three changed a temperature. Returns
    // whether anything changed.
    //
    // **A cell whose temperature is still changing has to stay awake**, and that
    // mark is the whole of how. It is the write rule and the wake rule in new
    // clothes: heat that diffuses into a sleeping chunk and stops is the same
    // bug as material frozen in mid-air, and the same bug as the boxed-in Fire
    // cell that froze forever before its self-wake fix. What makes it terminate
    // rather than keeping the world awake for good is the dead band in
    // `heat_flow` - two neighbours within one degree exchange nothing, so a warm
    // world settles and sleeps instead of trading a unit back and forth forever.
    bool step_thermal(int x, int y, const Material& mat);

    // True if any of the 8 cells surrounding (x, y) is of type t. Reads cells[]
    // directly rather than going through get_element(), which treats
    // out-of-bounds as Wall - correct for physics sealing, wrong here since it
    // would make every world edge silently act as a Wall catalyst.
    bool has_neighbor(int x, int y, ElementType t) const;

    // Checks (x, y) against the REACTIONS table and converts it via
    // set_element() on success. Returns true if the cell was converted, in
    // which case step_cell skips movement for it this frame.
    bool try_react(int x, int y);

    // How many steps a flame lives, and the only lifetime in the engine that is
    // a countdown rather than a decay chance.
    //
    // The reason is the colour ramp: a flame is drawn from white-hot at its
    // source through orange to dim red as it dies, which needs the cell to know
    // how old it is, and a dice roll has no age to read. Fire is a Gas and never
    // structural, so it is the one material that can spend `Element::ticks` on
    // this without colliding with the free-fall clock - see element.h.
    //
    // Around a fifth of a second. That is short on purpose and it is a
    // measurement, not a taste: reference footage of a burning scene replaces
    // the entire contents of a flame between frames roughly 10-20 steps apart,
    // while the fuel underneath takes seconds to be consumed.
    //
    // **It is a range and not a constant, and that is the fix for two separate
    // playtest notes at once** (session 2: "a hard height cutoff that looks
    // unnatural", "the uniform height of the flames does not look natural").
    // Both were the same defect, and neither was really about height: a flame
    // rose exactly one cell per step and lived exactly twelve steps, so *every*
    // flame died exactly twelve cells above the fuel that threw it. The result is
    // a razor-straight horizontal line across the top of a fire - the one shape
    // nothing in nature makes. Nothing needed to change about how flames move to
    // fix it; what was missing is that real flames do not all live equally long.
    //
    // 13 rather than 12 because flames also now rise slower (see
    // FLAME_RISE_SKIP_PERCENT), and the note asking for that asked about *speed*,
    // not height. 13 steps at 0.9 cells per step is the same reach 12 steps at
    // 1.0 was, so slowing the motion does not silently shorten the fire as well.
    static constexpr uint8_t FLAME_LIFETIME_MEAN = 13;

    // Half-width of the jitter, so a flame lives 8-18 steps. Wide enough that the
    // top of a fire is visibly ragged rather than merely soft, and bounded so the
    // longest-lived flame is still decoration rather than something that outlives
    // what threw it.
    static constexpr int FLAME_LIFETIME_SPREAD = 5;
    static constexpr uint8_t FLAME_LIFETIME_MAX =
        static_cast<uint8_t>(FLAME_LIFETIME_MEAN + FLAME_LIFETIME_SPREAD);

    // **Flames rise on 9 steps out of 10** (session 2: "the flames move about 10
    // percent too fast"). A gas moves a whole cell or none at all, so 0.9 cells
    // per step cannot be a smaller step - it has to be a skipped one. Rolled per
    // cell per step rather than counted, because a flame has no spare byte to
    // count in: `Element::ticks` is its lifetime, and `element.h` records that the
    // struct has no room left. The roll is drawn from the deterministic stream
    // like every other decision, so this stays reproducible from the seed.
    //
    // The skip is per cell, so neighbouring flames fall out of step with each
    // other, which breaks up the flat rising front as a side effect. That is a
    // bonus and not the reason - the reason is that the motion was 10% too fast.
    static constexpr int FLAME_RISE_SKIP_PERCENT = 10;

    // Throws a flame from a burning cell into a randomly chosen empty neighbour,
    // as named by the `emits` column. Returns true if one was placed.
    //
    // **This is the mechanism that makes flame a separate thing from fuel**, and
    // it is the only genuinely new rule E9 adds - everything else in the rebuild
    // is a row or a column. A burning cell stays put, keeps its structural role
    // and heats what it touches; what the player sees rising off it is
    // manufactured here and dead within FLAME_LIFETIME steps.
    bool emit_flame(int x, int y, const Material& mat);

    // Ages a flame by one step, ramps its colour to match, and kills it at zero.
    // Returns true if the cell is gone, in which case the caller must not go on
    // to move it.
    bool step_fire(int x, int y);

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

    // --- fracture ---
    //
    // `drop_component` translates an unsupported piece straight down with its
    // shape perfectly intact, so masonry descends like an elevator. Fracture is
    // the answer rather than rotation, and that substitution is the decision:
    // true rigid-body rotation on a cell grid means resampling the piece every
    // step it turns, which destroys the exact authored pixels
    // notes/art_pipeline.txt calls the entire visual pillar. Masonry mostly
    // breaks rather than tips anyway.
    //
    // **A piece breaks when it lands, not while it falls**, and that timing is
    // what makes the whole feature safe. Every "nothing must move here" test in
    // the suite is about a piece at rest, and a piece at rest has `ticks`
    // of zero and never reaches this code at all. Fracture cannot start a
    // collapse; it can only let a collapse that was already happening finish
    // unevenly. `Grid`'s comment on MAX_SUPPORT_CELLS says a missed collapse is
    // invisible while a wrong one turns a level into rubble, and this item was
    // flagged in ROADMAP.md as the single change most able to violate that -
    // tying the trigger to landing speed is how the asymmetry is kept by
    // construction rather than by care.
    //
    // **Splitting a falling piece would have been a no-op, and that is worth
    // recording because it was the obvious design.** Break a piece in mid-air
    // into two and both halves are unsupported, so both fall by exactly one cell
    // on exactly the same steps: identical to not having split it. Worse, the
    // fill re-discovers them as one component next step, because they are still
    // adjacent. **Fracture without persistent state is impossible on this
    // representation** - the crack has to survive into the next fill or it never
    // had any effect - which is what `Element::piece_tag` is and why a byte was
    // spent on it.

    // Smallest piece worth breaking. Below this, a break produces two fragments
    // small enough that the result reads as gravel rather than as masonry
    // coming apart, which is the failure mode the rigid drop was chosen over in
    // the first place. It also terminates the recursion in practice: each break
    // halves the piece, so a slab fragments a bounded number of times and then
    // stops being eligible.
    static constexpr int MIN_FRACTURE_CELLS = 20;

    // How long a piece must have been falling before landing can break it. Four
    // steps is one TICKS_PER_SPEEDUP interval, so this reads as "it was moving
    // at more than the speed it comes loose at". A piece that tips off a ledge
    // and drops one cell settles intact; one that has been accelerating breaks.
    static constexpr int FRACTURE_MIN_TICKS = 4;

    // Splits the piece that has just landed at (x, y) along a vertical seam,
    // giving everything on one side a fresh tag. Runs its own flood fill rather
    // than reusing the one in progress: the fill that detected the landing
    // stopped at the first grounded cell and holds only a partial trail, and a
    // partial piece is exactly the input that would put a crack in the wrong
    // place. Costs a full fill, once, per landing.
    void fracture_landing(int x, int y);

    // Tags handed out by fracture. Starts at 1 because 0 means "never broken",
    // and wraps past 255 back to 1 rather than to 0 - see Element::piece_tag for
    // what a collision costs.
    uint8_t next_piece_tag = 1;

    std::vector<int> fracture_component; // scratch, reused across breaks

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

    // Randomness for the step in progress. These fold in the seed and the step
    // count so that a call site names only what varies: where it is asking from,
    // and which decision it is making. Defined in the header rather than the
    // .cpp because the remaining call sites (F1.4) run per active cell, and an
    // out-of-line call would cost more than the mix itself.
    bool coin(uint64_t index, sim_random::Stream s) const {
        return sim_random::coin(world_seed, step_count, index, s);
    }
    bool chance(int pct, uint64_t index, sim_random::Stream s) const {
        return sim_random::chance(pct, world_seed, step_count, index, s);
    }
    bool chance_per_myriad(int per_myriad, uint64_t index, sim_random::Stream s) const {
        return sim_random::chance_per_myriad(per_myriad, world_seed, step_count, index, s);
    }
    int pick(int n, uint64_t index, sim_random::Stream s) const {
        return sim_random::pick(n, world_seed, step_count, index, s);
    }
    // A symmetric offset that *does* take the clock, unlike authored_spread
    // below. Flame lifetime needs this: pinned at step 0 the draw is a property
    // of the cell rather than of the flame, so the same spots would always throw
    // the long flames and the ragged top of a fire would be frozen in place
    // instead of moving - which is a more obviously wrong version of the
    // uniformity it was added to fix.
    int spread(int range, uint64_t index, sim_random::Stream s) const {
        return sim_random::spread(range, world_seed, step_count, index, s);
    }

    // The exception among these three: no step input, pinned at 0 instead.
    //
    // Colour jitter is not a decision the cell retakes every step, it is a
    // property of the material at that spot, so the clock has no business in it.
    // Feeding the step in would repaint the whole world every frame.
    int authored_spread(int range, uint64_t index, sim_random::Stream s) const {
        return sim_random::spread(range, world_seed, 0, index, s);
    }

    uint32_t jittered_color(const Material& mat, uint64_t index) const;

    // The seed and the step count are the whole of what randomness depends on.
    // There is no generator here any more, which is what makes that sentence
    // true: nothing carries state between draws, so a save file that records
    // these two numbers records everything.
    uint64_t world_seed;
};
