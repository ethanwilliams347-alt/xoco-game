// Headless simulation benchmark.
//
// This exists so that optimisation work is measured rather than guessed at.
//
// **Every scenario is run at two world sizes** (P2). The bench used to run one,
// 960x540, justified by a comment that said "1920x1080 at a 2px scale = 960x540
// cells". That arithmetic described a world that no longer exists in two
// separate ways: `Camera::SCALE` is 4 rather than 2, and - more to the point -
// since F3.1 the world's size is not a window size divided by anything at all.
// `GRID_WIDTH`/`GRID_HEIGHT` in main.cpp are a cell count the camera pans
// across, and no scale factor applies to them. The comment stayed true-looking
// while the thing it described was replaced underneath it, so the bench went on
// measuring a quarter of the played world and every budget on record was quoted
// against that quarter.
//
// Both sizes are run rather than the small one being replaced, because the
// 960x540 series is what every historical number in PERFORMANCE.md was measured
// against and a re-baseline that deletes its own baseline cannot be checked.
// The scenario geometry is threaded through `Bench` rather than read from
// file-scope constants so that the same builder can produce both, and every
// derived constant below is written so that it reproduces its old literal value
// **exactly** at 960x540 - if the historical row does not reproduce, the new row
// is not trustworthy either.
//
// Not registered with CTest: a benchmark that fails the build on a slow machine
// is a nuisance, and timings are for reading, not for gating.

#include "physics/grid.h"
#include "render/light.h"
#include "game/display.h"
#include "game/input_log.h"
#include "game/run.h"
#include "scene/bmp.h"
#include "scene/scene.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

namespace {

constexpr int BENCH_STEPS = 300;

// A step must fit in 16.67 ms alongside rendering and input, so the simulation
// alone needs to be a fraction of that.
constexpr double FRAME_BUDGET_MS = 1000.0 / 60.0;

// The world sizes every scenario is measured at.
//
// The second is `GRID_WIDTH` x `GRID_HEIGHT` from main.cpp, i.e. the world the
// game actually simulates. The first is kept because it is the series on record.
struct WorldSize {
    int w;
    int h;
    const char* note;
};
constexpr WorldSize SIZES[] = {
    { 960,  540,  "the historical series - every number in PERFORMANCE.md before P2" },
    { 1920, 1080, "what main.cpp actually runs (GRID_WIDTH x GRID_HEIGHT)" },
};
constexpr int SIZE_COUNT = static_cast<int>(sizeof(SIZES) / sizeof(SIZES[0]));

// Everything a scenario needs to know about the world it is being built into.
//
// **The step counter lives here rather than in a `static` local, and that is a
// correctness fix rather than tidying.** The two stepped scenarios used to keep
// their cadence in a function-scope `static int tick`, which was fine while each
// scenario ran exactly once per process. Running the suite at two sizes makes
// the second run inherit wherever the first one left its counter, which shifts
// both the spawn cadence and the clear cycle - so the 1920x1080 row would not be
// the same scenario as the 960x540 row, and neither would reproduce on a re-run.
// One `Bench` is constructed per run, so the counter starts at zero every time.
struct Bench {
    int w;
    int h;
    int tick = 0;

    // Slab slots are spaced by a fixed pitch rather than by a fixed count, so a
    // wider world gets *more* falling structure rather than the same eight
    // slabs spread thinner. That distinction is the whole point of the item: a
    // scenario that keeps its absolute workload while the world grows measures
    // the allocation and nothing else, and would read as "falling structure is
    // free at 1920x1080".
    //
    // 120 is `960 / 8` - the pitch the eight-slot version already had - so this
    // is exactly 8 at 960 and 16 at 1920. Clamped to at least one slot so a
    // hypothetical narrow world cannot divide by zero here.
    static constexpr int SLOT_PITCH = 120;
    int slab_slots() const { return w / SLOT_PITCH > 0 ? w / SLOT_PITCH : 1; }

    // Depth-proportional, and written as a scaled fraction of the original 540
    // so that both land on their historical literals at 540: 400 and 300.
    int drain_top() const { return h * 400 / 540; }
    int shatter_floor() const { return h * 300 / 540; }
};

// A world that has come to rest. Nothing can move, so this is a direct measure
// of how much the engine pays for cells that are doing nothing - which is most
// of the world, most of the time, in a real game.
void build_settled(Grid& g, Bench& b) {
    for (int y = b.h / 2; y < b.h; ++y)
        for (int x = 0; x < b.w; ++x)
            g.set_element(x, y, ElementType::Sand);
}

// The opposite extreme: bands of sand suspended over bands of water, so the
// whole lower world is sinking and displacing at once. Worst case by design.
void build_churning(Grid& g, Bench& b) {
    for (int y = b.h / 3; y < b.h; ++y) {
        const ElementType t = ((y / 8) % 2 == 0) ? ElementType::Sand : ElementType::Water;
        for (int x = 0; x < b.w; ++x)
            g.set_element(x, y, t);
    }
}

// What an ordinary gameplay frame looks like: a large mostly-static world with a
// small patch of action in it.
//
// The patch stays 40x40 cells and is re-centred rather than scaled with the
// world. A "small patch of action" that grows with the world is not the thing
// this scenario is named for - the real claim being tested is that a mostly
// asleep world costs almost nothing however big it is, and quadrupling the
// sleeping part while holding the awake part fixed is exactly the measurement
// that answers it. The floor slab stays 20 cells deep for the same reason.
void build_sparse(Grid& g, Bench& b) {
    for (int x = 0; x < b.w; ++x)
        for (int y = b.h - 20; y < b.h; ++y)
            g.set_element(x, y, ElementType::Wall);

    for (int y = 40; y < 80; ++y)
        for (int x = b.w / 2 - 20; x < b.w / 2 + 20; ++x)
            g.set_element(x, y, ElementType::Sand);
}

// Every scenario above eventually comes to rest, which flatters its average. This
// one cannot: a row is scraped off the floor and a row is poured in at the
// ceiling every step, so mass stays constant and the whole column between them is
// permanently cascading. It is the sustained worst case - terrain collapsing
// continuously, nothing ever settling - and it is the number the 60 Hz budget
// should actually be judged against.
void cascade(Grid& g, Bench& b) {
    for (int x = 0; x < b.w; ++x) {
        g.set_element(x, b.h - 1, ElementType::Empty);
        g.set_element(x, 0, ElementType::Sand);
    }
}

// A large Wood slab, permanently on fire. Reactions add per-cell work to
// exactly the cells that chunking cannot help - the active ones - so this is
// the scenario that has to be measured, not assumed, per the roadmap note on
// this item. A drip of fresh Fire along the top edge every step keeps
// ignition, spread, and decay all happening at once for the whole run, so the
// slab never fully burns down to Empty inside the measurement window.
void build_burning(Grid& g, Bench& b) {
    for (int y = b.h / 2; y < b.h; ++y)
        for (int x = 0; x < b.w; ++x)
            g.set_element(x, y, ElementType::Wood);
}

void feed_fire(Grid& g, Bench& b) {
    for (int x = 0; x < b.w; x += 4)
        g.set_element(x, b.h / 2, ElementType::Fire);
}

// Rigid pieces in permanent free fall.
//
// Structural material is the only thing in the engine that gets re-derived
// rather than simply stepped: a piece at MAX_FALL_SPEED runs eight flood fills
// over the whole of itself every step, one per cell of travel. No other
// scenario in this file contains a falling structure at all, so none of them
// can see that cost -- which is exactly why this one exists.
//
// Slabs are kept well under MAX_SUPPORT_CELLS, or they would be waved through
// as too big to judge and never fall at all. Slab *geometry* is absolute - a
// slab is a physical object, and a world twice as wide should contain twice as
// many of them, not the same ones stretched.
constexpr int SLAB_W = 100;
constexpr int SLAB_H = 4;

void spawn_slab(Grid& g, const Bench& b, int slot) {
    const int x0 = slot * (b.w / b.slab_slots()) + 4;
    for (int y = 2; y < 2 + SLAB_H; ++y)
        for (int x = x0; x < x0 + SLAB_W; ++x)
            g.set_element(x, y, ElementType::Wall);

    // Placing structure deliberately never queues a support check - that is what
    // lets the brush draw a floating platform on purpose - so the slab has to be
    // knocked loose. Put a cell under it and take it straight back out.
    g.set_element(x0, 2 + SLAB_H, ElementType::Wall);
    g.set_element(x0, 2 + SLAB_H, ElementType::Empty);
}

// A slab that lands goes quiet, and a scenario whose subject falls asleep
// measures nothing. This drains them at drain_top() instead and feeds a
// replacement in at the ceiling, so the air is always full. The band is deeper
// than MAX_FALL_SPEED because a slab at full speed would step over a thinner
// one. Reading before writing keeps the drain nearly free while it is empty,
// which is most of it, most of the time.
//
// **The cadence invariant, re-checked at both sizes.** One new slab every 8
// steps cycling the slots means a slot comes round every `8 * slab_slots()`
// steps, and a slot must be clear before its next slab arrives. Fall time
// follows MAX_FALL_SPEED 8 and TICKS_PER_SPEEDUP 4: a piece covers 144 cells in
// its first 32 steps and 8 per step after that.
//   960x540:   drain 400, fall ~394 -> 64 steps against a 64-step cycle.
//   1920x1080: drain 800, fall ~794 -> 114 steps against a 128-step cycle.
// The first is the exact equality the original comment recorded; the second has
// more slack, so scaling the drain with depth does not break the invariant.
void churn_slabs(Grid& g, Bench& b) {
    const int drain = b.drain_top();
    for (int y = drain; y < drain + 12; ++y)
        for (int x = 0; x < b.w; ++x)
            if (g.get_element(x, y).type != ElementType::Empty)
                g.set_element(x, y, ElementType::Empty);

    if (b.tick % 8 == 0) spawn_slab(g, b, (b.tick / 8) % b.slab_slots());
    b.tick++;
}

void build_collapsing(Grid&, Bench&) {} // the hook does all of it

// `collapsing` prices pieces *in the air*, and it says so: it drains them at
// drain_top() "rather than being allowed to land". That is the right design for
// what it measures and it is exactly why it cannot price E3 - fracture fires on
// landing and on nothing else, so a bracketed A/B against `collapsing` runs the
// feature zero times and returns a confidently flat number. Which it did.
//
// This scenario is the landing. Same slabs, no drain, and a deliberately jagged
// floor so that every one of them comes down across a support boundary and
// therefore actually breaks - a flat floor would land them evenly and skip the
// crack entirely, which is correct behaviour and useless as a measurement. The
// wreckage is cleared on a cycle so the pile does not grow until it reaches the
// ceiling and the scenario quietly becomes a different one.
//
// The sawtooth's own geometry is absolute (8 deep every 32 across) because it is
// sized against SLAB_W, which is also absolute: what has to stay true is that a
// 100-wide slab always spans several steps of it, and that does not depend on
// how wide the world is.
void build_shattering(Grid& g, Bench& b) {
    const int floor_y = b.shatter_floor();
    for (int x = 0; x < b.w; ++x) {
        const int top = floor_y + ((x / 32) % 2 == 0 ? 0 : 8);
        for (int y = top; y < b.h; ++y) g.set_element(x, y, ElementType::Wall);
    }
}

void shatter_slabs(Grid& g, Bench& b) {
    // Clear the heap every 60 steps, floor included, and rebuild the sawtooth.
    // Cheaper and more honest than draining a band: it puts the scenario back to
    // the state its name describes instead of letting it drift into "a deep pile
    // of rubble settling", which is a different measurement.
    if (b.tick % 60 == 59) {
        for (int y = b.shatter_floor() - 40; y < b.h; ++y)
            for (int x = 0; x < b.w; ++x)
                if (g.get_element(x, y).type != ElementType::Empty)
                    g.set_element(x, y, ElementType::Empty);
        build_shattering(g, b);
    } else if (b.tick % 8 == 0) {
        spawn_slab(g, b, (b.tick / 8) % b.slab_slots());
    }
    b.tick++;
}

// Proof that `shattering` still shatters.
//
// **This is here because of the specific way E3's first measurement failed**, and
// the failure is one that scaling the world makes *more* likely rather than
// less. Fracture fires on landing; at 1920x1080 a slab falls twice as far while
// the clear cycle stays at 60 steps, so "the slabs are now wiped mid-flight and
// nothing ever lands" is a real way for this row to become a confident number
// about a feature that ran zero times. PERFORMANCE.md's rule is that a null
// result is only evidence if the feature ran, and `run_light` already carries
// the same idea as its `lit_frames` count.
//
// `piece_tag` is written by fracture and by nothing else, and is never written
// back to zero, so a non-zero tag anywhere is a landing that broke. Counted over
// 60 untimed steps after the clock has stopped, and reported as a peak rather
// than a final reading, because a scan that happened to land on the step after a
// clear would read zero from a scenario that had been fracturing the whole way.
int peak_fractured_cells(Grid& g, Bench& b) {
    int peak = 0;
    for (int i = 0; i < 60; ++i) {
        shatter_slabs(g, b);
        g.update();
        int tagged = 0;
        for (int y = b.shatter_floor() - 40; y < b.h; ++y)
            for (int x = 0; x < b.w; ++x)
                if (g.get_element(x, y).piece_tag != 0) ++tagged;
        if (tagged > peak) peak = tagged;
    }
    return peak;
}

// The last parameter is the only one with a default that is a *setting* rather
// than an absence: every row above the vent sweep passes nothing and therefore
// runs the shipped simulation, which is what keeps those rows comparable with
// the ones on record. It is threaded through here rather than set by the caller
// on a grid it does not own, because `run` is what constructs the grid, and the
// radius has to be in place before `build` puts sand on top of water.
void run(const char* name, const WorldSize& size,
         void (*build)(Grid&, Bench&), int settle_steps,
         void (*on_step)(Grid&, Bench&) = nullptr,
         int (*witness)(Grid&, Bench&) = nullptr, const char* witness_label = nullptr,
         int vent_radius = Grid::DEFAULT_VENT_RADIUS,
         bool seek_level = true, bool room_above = true) {
    Bench b{size.w, size.h};
    Grid g(b.w, b.h);
    g.set_vent_radius(vent_radius);
    g.set_seek_level_enabled(seek_level);
    g.set_room_above_enabled(room_above);
    build(g, b);

    // Let the scenario reach its steady state before the clock starts, so the
    // number describes the state named in the label rather than the setup.
    for (int i = 0; i < settle_steps; ++i) {
        if (on_step) on_step(g, b);
        g.update();
    }

    // Sampled either side of the clock rather than only after it, and that is a
    // P2 fix rather than extra detail.
    //
    // A single end-of-run count cannot distinguish "this scenario stayed busy
    // for the whole window" from "this scenario went to sleep a third of the way
    // in and the remaining two thirds averaged its cost down". Both print
    // `0 awake`. That difference was invisible while the bench ran one size, and
    // it stops being harmless the moment two sizes are divided by each other:
    // `churning` settles inside the 300-step window at 960x540 and does not at
    // 1920x1080, so the ratio between those two rows is partly a measure of how
    // much of each window was spent asleep. Printing both ends puts that in the
    // output instead of leaving it to be inferred from a suspicious number.
    const int awake_before = g.active_chunk_count();

    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < BENCH_STEPS; ++i) {
        if (on_step) on_step(g, b);
        g.update();
    }
    const auto end = std::chrono::steady_clock::now();

    const double total_ms = std::chrono::duration<double, std::milli>(end - start).count();
    const double per_step = total_ms / BENCH_STEPS;

    // Reported alongside the timing because it is the number that explains it:
    // if a scenario is slow while barely any chunks are awake, the cost is not
    // where chunking can reach it.
    const int total_chunks = ((b.w + Grid::CHUNK_SIZE - 1) / Grid::CHUNK_SIZE) *
                             ((b.h + Grid::CHUNK_SIZE - 1) / Grid::CHUNK_SIZE);

    std::printf("  %-10s %9.4f ms/step  %6.1f%% of a 60 Hz frame   %4d->%-4d/%d chunks awake%s\n",
                name, per_step, 100.0 * per_step / FRAME_BUDGET_MS,
                awake_before, g.active_chunk_count(), total_chunks,
                per_step < FRAME_BUDGET_MS ? "" : "  <-- OVER BUDGET");

    // Run strictly after the clock has stopped, so proving the scenario ran
    // cannot change what it cost.
    if (witness) {
        const int n = witness(g, b);
        std::printf("  %-10s %s: %d%s\n", "", witness_label, n,
                    n == 0 ? "   <-- FEATURE NEVER FIRED; the row above is not about it" : "");
    }
}

// V7's light field, which every other scenario in this file is blind to.
//
// **`grid_bench` measures `Grid::update`, and the light field is not in it** -
// it is per-frame render work, computed in main.cpp between the simulation step
// and the present. So none of the scenarios above can reach it, and running the
// A/B this document normally asks for would produce flat numbers that agreed
// beautifully about nothing. That is the third rule in PERFORMANCE.md, and it
// has already cost this project one worthless measurement (E3's first attempt).
// The instrument has to change, not the scenario.
//
// What this measures and what it does not, stated plainly because the gap is the
// interesting part:
//
//  - **Measured:** the whole CPU cost of a frame's lighting - the scan over the
//    viewport's cells, the propagation, and the pack to ARGB. That is the part
//    this project controls and the part that scales with BLOCK and ITERATIONS.
//  - **Not measured:** the texture upload and the one extra `SDL_RenderCopy`.
//    Those are GPU-side and this project has no headless instrument that can see
//    them. They are not assumed to be free; they are unmeasured, and saying so is
//    what E9's entry in PERFORMANCE.md established as the alternative to writing
//    down a number the method does not support.
//
// Sized to the **viewport**, not to the world. The light field covers what is on
// screen and nothing else, so it does not scale with world size. **That is also
// why this section is run once rather than once per world size**, unlike every
// scenario above: the field scans a fixed window centred on the same burning
// band in both worlds, so a second size would produce a second identical number
// and invite someone to read the pair as evidence about world size. It is run at
// the played size, which is the world the viewport is a window onto.
//
// **The widest display mode, not the narrowest, and not the 201x151 this used
// to hold.** That figure was the padded viewport of an 800x600 window, and the
// window had already grown past it before the display modes existed - so this
// was measuring a configuration the game had stopped running, which is the
// failure PERFORMANCE.md's third rule is about. The viewport is now a runtime
// choice between three sizes, and the one worth having a number for is the
// largest: 3440x1440 puts 861x361 padded cells on screen against 1920x1080's
// 481x271, which is 2.4x the area to scan, propagate and pack every frame.
// Whatever this reads is the cost the widest mode pays; every other mode pays
// less.
constexpr DisplayMode WIDEST = DISPLAY_MODES[DISPLAY_MODE_COUNT - 1];
constexpr int LIGHT_VIEW_W = WIDEST.padded_w();
constexpr int LIGHT_VIEW_H = WIDEST.padded_h();
constexpr int LIGHT_FRAMES = 300;

void run_light(const char* name, const WorldSize& size,
               void (*build)(Grid&, Bench&), void (*on_step)(Grid&, Bench&)) {
    Bench b{size.w, size.h};
    Grid g(b.w, b.h);
    build(g, b);
    for (int i = 0; i < 60; ++i) {
        if (on_step) on_step(g, b);
        g.update();
    }

    // Aimed at the band where the fire actually is. A light field pointed at
    // empty sky early-outs on `any_light()` and would measure the scan alone -
    // which is a real number, but not the one this scenario is named for.
    const int origin_x = b.w / 2 - LIGHT_VIEW_W / 2;
    const int origin_y = b.h / 2 - LIGHT_VIEW_H / 2;

    LightField light(LIGHT_VIEW_W, LIGHT_VIEW_H);

    // **Only the `light.update` calls are inside the clock**, accumulated one
    // frame at a time, with the simulation step deliberately outside it.
    //
    // The first version of this timed the whole loop and subtracted a second,
    // light-free loop from it. That reads as more like the rest of this file -
    // it is shaped like an A/B - and it is much worse here: the two loops are
    // about two seconds each and the quantity being recovered is fifty
    // milliseconds, so a one-percent difference in machine state between them is
    // the same size as the answer. Differencing large numbers to find a small one
    // is the arithmetic version of the unbracketed pair PERFORMANCE.md's first
    // rule is about. Timing the call directly needs no control and no bracket,
    // because nothing is being compared.
    //
    // The grid is still stepped between measurements, so the field is looking at
    // a scene that changes exactly as it does in play - a frozen grid would let
    // the convergence early-out settle into a best case that never happens.
    double light_ms = 0.0;
    int lit_frames = 0;
    for (int i = 0; i < LIGHT_FRAMES; ++i) {
        if (on_step) on_step(g, b);
        g.update();

        const auto start = std::chrono::steady_clock::now();
        light.update(g, origin_x, origin_y);
        const auto end = std::chrono::steady_clock::now();
        light_ms += std::chrono::duration<double, std::milli>(end - start).count();

        if (light.any_light()) lit_frames++;
    }

    const double per_frame = light_ms / LIGHT_FRAMES;

    // **`lit_frames` is the count that makes a flat result mean anything**, and
    // it is here because of the rule that a null result is only evidence if the
    // feature ran. Zero lit frames means the propagation never executed and the
    // number below is the cost of the scan and the early-out, whatever the label
    // says.
    std::printf("  %-10s %9.4f ms/frame  %6.1f%% of a 60 Hz frame   %3d/%d frames lit\n",
                name, per_frame, 100.0 * per_frame / FRAME_BUDGET_MS,
                lit_frames, LIGHT_FRAMES);
}

// What a recorded session actually contained.
//
// **This exists because the first recorded session (2026-08-13) produced a row
// nobody could interpret.** It read 0 of 24,437 steps over budget, and the
// obvious next question - was that because the engine is fast, or because the
// session never did anything expensive? - had no answer anywhere in the output.
// Worse, `PERFORMANCE.md` owes E9 a measurement of steam under a ceiling, and a
// replayed row could not say whether a session contained a single steam cell.
// **A row that cannot say what it measured is a number without its conditions**,
// which is the one thing that file refuses to print.
//
// Two halves, and they have very different standing:
//
//  - **What the player did is exact and free.** It is a count over the log's
//    inputs, which are the session, so nothing is inferred and nothing sampled.
//  - **What the world contained is sampled, once a second, and is therefore
//    evidence of presence and not of absence.** A fire that lit and burned out
//    inside one second can fall between two samples. The output says "seen in N
//    of M samples" for exactly that reason, and a material reported as never
//    seen means *not seen*, which is a weaker claim than *not there*. Read it
//    the way the "absent is not zero" rule reads a missing row.
//
// **Why it is a second replay pass rather than a sample taken inside the timed
// one, which is the part worth keeping if this is ever rewritten.** A census
// walks all 2 M cells, which is ~25 MB - it evicts the entire cache, so the
// step immediately after each sample would pay cold-cache costs it would never
// pay in the game. At one sample a second that is ~1.7% of steps perturbed,
// landing squarely on the two statistics the budget rule reads (p99 and worst).
// **An instrument that changes what it measures is the failure this whole file
// is about**, so the timed pass is left byte-identical to what it was before
// this function existed, and the census gets its own untimed pass over the same
// log. That costs one more replay, a few seconds, and it is exact rather than
// approximate for a reason this project paid for up front: the simulation is
// deterministic, so the second pass sees precisely the worlds the first one
// timed. F2.3's replayability is what makes measuring twice free.
// One sample a second of play. Chosen against what the samples are read for
// rather than for a round number: the two questions on record are "did this
// session contain steam under a ceiling" (E9) and "was fire ever burning while
// sand fell into water" (checklist step 9), and both describe states that last
// seconds. Sampling faster costs a linear scan of 25 MB each time and buys
// resolution on transients neither question asks about.
constexpr int CENSUS_INTERVAL = 60;

struct SessionContents {
    static constexpr int TYPES = static_cast<int>(ElementType::Count);

    // Sampled, untimed.
    int samples = 0;
    int peak_cells[TYPES] = {};
    int samples_present[TYPES] = {};
    int peak_awake = 0;

    // Peak awake chunks *excluding* the pre-first-step sample, and it is the
    // number to read. Loading the fixture scene wakes a large part of the world
    // at once and it settles within the first second, so the baseline sample is
    // always the largest and always says the same thing about every session.
    // The first session peaked at 129 including it and **16 of 510 after it** -
    // two figures that support completely different sentences about how busy
    // the session was, and only the second one is about the session.
    int peak_awake_after_start = 0;

    // The world before the first step, taken as its own sample. **Without it
    // "peak cells" is unreadable**, because the fixture scene already contains
    // Sand, Water, Wall and Wood - so a session that touched none of them still
    // reports them present in every sample, and a peak means nothing until you
    // know what it started from. It also keeps the sampled peak-awake figure
    // consistent with the timed row's opening count, which is taken at the same
    // instant; the first session read 129 there and 16 here, purely because the
    // scene's initial settle finishes inside the first sampling interval.
    int start_cells[TYPES] = {};

    // Exact, counted over the log's inputs.
    int dig_steps = 0;
    int brush_steps = 0;
    int brush_by_type[TYPES] = {};
    int move_steps = 0;
    int jump_steps = 0;

    // The checklist step 9 question - "digging near falling sand near fire near
    // water" - asked as a co-occurrence rather than as four separate counts,
    // because four materials each present at some point in seven minutes says
    // nothing about whether they were ever present together.
    int mixed_samples = 0;
    int mixed_dig_samples = 0;
};

// One sample. Untimed by construction: every caller is outside a clock.
void take_sample(const Grid& g, SessionContents& c, bool dug_recently, bool is_baseline = false) {
    int count[SessionContents::TYPES] = {};
    const int w = g.get_width(), h = g.get_height();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            count[static_cast<int>(g.get_element(x, y).type)]++;
        }
    }

    c.samples++;
    for (int t = 0; t < SessionContents::TYPES; ++t) {
        if (count[t] > c.peak_cells[t]) c.peak_cells[t] = count[t];
        if (count[t] > 0) c.samples_present[t]++;
    }

    const int awake = g.active_chunk_count();
    if (awake > c.peak_awake) c.peak_awake = awake;
    if (!is_baseline && awake > c.peak_awake_after_start) c.peak_awake_after_start = awake;

    const bool mixed = count[static_cast<int>(ElementType::Fire)] > 0 &&
                       count[static_cast<int>(ElementType::Water)] > 0 &&
                       count[static_cast<int>(ElementType::Sand)] > 0;
    if (mixed) {
        c.mixed_samples++;
        if (dug_recently) c.mixed_dig_samples++;
    }
}

// P4: a scenario that is a real frame, replayed from a recorded session.
//
// **Every other row in this file is hand-built, and this project has now twice
// had to argue about which of them counts as realistic** - most recently over
// whether `churning` at 211% of a frame is a problem or an artifact. That
// argument cannot be won from a taxonomy of synthetic scenarios, and it does
// not have to be had: F2.3 made a run a seed plus a replayable list of inputs,
// and `test_run.cpp` proves such a list replays byte-identically. This row is
// what a player actually did, so it is a played frame by construction rather
// than by assertion, and it is the row the frame-budget rule in
// ROADMAP_ITEMS.md is aimed at.
//
// **Three things make this row different from every row above, and all three
// are deliberate:**
//
//  1. **It times `Run::step`, not `Grid::update`.** A played frame contains the
//     player's own step, the dig tool and the brush, and a budget quoted
//     against the grid alone is a budget for part of a frame. The two units are
//     therefore not directly comparable with the rows above - which is stated
//     here rather than left for someone to discover by dividing them.
//  2. **It runs at one size, the recorded one.** A session recorded in a
//     1920x1080 world is not a session in any other world; replaying it into a
//     960x540 grid would put every cursor coordinate somewhere else. Same
//     reasoning as `run_light` above, for a different reason.
//  3. **It reports a worst step and a share over budget, not just a mean.** A
//     mean is the wrong statistic for a budget: a session that spends 95% of
//     its steps asleep and 5% of them at 40 ms is a session that stutters, and
//     its mean looks excellent.
//
// The world is rebuilt exactly as `main.cpp` builds it - same seed, same
// fixture scene, through the same loader (`scene/bmp.cpp`, which exists so that
// sentence can be true). The two fingerprints in the log are what turn "exactly"
// from a claim into a check.
void run_replay(const char* log_path) {
    input_log::Log log;
    std::string error;
    if (!input_log::read(log_path, log, &error)) {
        std::printf("  replay    not run: %s\n", error.c_str());
        std::printf("            Record one by playing and pressing F9, then run this from `code/`.\n"
                    "            **This row is absent, not zero** - do not read the rows above as if\n"
                    "            the played frame had been measured and found cheap.\n");
        return;
    }

    Run run(log.header.grid_w, log.header.grid_h, log.header.seed);

    std::string scene_error, scene_warning;
    const Scene scene = bmp::load("assets/test_material.bmp", "assets/test_albedo.bmp",
                                  &scene_error, &scene_warning);
    if (!scene_error.empty()) {
        std::printf("  replay    not run: %s\n", scene_error.c_str());
        std::printf("            Run this from `code/`, where assets/ is.\n");
        return;
    }
    if (!scene_warning.empty()) std::printf("  replay    WARNING: %s\n", scene_warning.c_str());

    const int placed = load_scene(run.grid, scene, 0, 0);
    const uint64_t start = input_log::fingerprint(run.grid);

    // **The staleness check, and it is the reason P4 is an item rather than an
    // afternoon.** A log replayed into a world it was not recorded in produces
    // a number with nothing wrong-looking about it. Refused rather than warned:
    // there is no partial version of "this is the same world".
    if (placed != log.header.scene_cells || start != log.header.start_fingerprint) {
        std::printf("  replay    not run: the fixture scene has changed since this log was recorded\n"
                    "            (%d cells placed now, %d when recorded; start fingerprints %s).\n"
                    "            Re-record the session - see ROADMAP_ITEMS.md, P4.\n",
                    placed, log.header.scene_cells,
                    start == log.header.start_fingerprint ? "agree" : "differ");
        return;
    }

    std::printf("  replay    %s: %d steps (%.1f s of play), seed %llu, %d scene cells\n",
                log_path, static_cast<int>(log.steps.size()),
                log.steps.size() / 60.0,
                static_cast<unsigned long long>(log.header.seed), placed);

    const int awake_before = run.grid.active_chunk_count();

    std::vector<double> step_ms;
    step_ms.reserve(log.steps.size());
    for (const Input& in : log.steps) {
        const auto t0 = std::chrono::steady_clock::now();
        run.step(in);
        const auto t1 = std::chrono::steady_clock::now();
        step_ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }

    if (step_ms.empty()) {
        std::printf("  replay    not run: the log contains no steps.\n");
        return;
    }

    double total = 0.0;
    int over_budget = 0;
    for (double ms : step_ms) {
        total += ms;
        if (ms > FRAME_BUDGET_MS) over_budget++;
    }
    const double mean = total / step_ms.size();

    std::vector<double> sorted = step_ms;
    std::sort(sorted.begin(), sorted.end());
    const double p99 = sorted[static_cast<size_t>(sorted.size() * 99 / 100)];
    const double worst = sorted.back();

    std::printf("  %-10s %9.4f ms/step  %6.1f%% of a 60 Hz frame   %4d->%-4d chunks awake\n",
                "replay", mean, 100.0 * mean / FRAME_BUDGET_MS,
                awake_before, run.grid.active_chunk_count());
    std::printf("  %-10s p99 %.4f ms (%.1f%%), worst %.4f ms (%.1f%%), %d of %d steps over budget\n",
                "", p99, 100.0 * p99 / FRAME_BUDGET_MS, worst, 100.0 * worst / FRAME_BUDGET_MS,
                over_budget, static_cast<int>(step_ms.size()));

    // **Reported, not enforced, and the distinction carries the item's whole
    // argument about what invalidates a log.** A changed fixture scene or
    // changed input handling makes the row meaningless and is refused above. A
    // changed *simulation* also moves the end state - and that is an E-track
    // item working as intended, with the row still perfectly valid. The two are
    // indistinguishable from here, so this says what it saw and lets the reader
    // decide, rather than pretending to know which happened.
    const uint64_t end = input_log::fingerprint(run.grid);
    const bool same_world = end == log.header.end_fingerprint;
    const bool same_player = run.player.cell_x() == log.header.end_player_x &&
                             run.player.cell_y() == log.header.end_player_y;
    if (same_world && same_player) {
        std::printf("  %-10s replayed to the recorded end state exactly.\n", "");
    } else {
        std::printf("  %-10s end state DIFFERS from the recording (%s, player %s: %d,%d now vs %d,%d then).\n",
                    "", same_world ? "world matches" : "world differs",
                    same_player ? "matches" : "differs",
                    run.player.cell_x(), run.player.cell_y(),
                    log.header.end_player_x, log.header.end_player_y);
        std::printf("  %-10s That is expected after a change to the simulation, and means the log is\n"
                    "  %-10s stale if nothing about the simulation changed. The timing above is still\n"
                    "  %-10s a real session's inputs; what it is no longer is the same session.\n",
                    "", "", "");
    }

    // Everything from here down is untimed and runs after the clock has stopped,
    // for the reason set out at `SessionContents`: a census evicts the cache, so
    // it gets its own pass rather than perturbing the two statistics the budget
    // rule reads. Nothing above this line changed when this was added.
    SessionContents contents;

    // The input half is exact - these *are* the session, not a sample of it.
    for (const Input& in : log.steps) {
        if (in.dig) contents.dig_steps++;
        if (in.left || in.right) contents.move_steps++;
        if (in.jump) contents.jump_steps++;
        if (in.brush_active) {
            contents.brush_steps++;
            contents.brush_by_type[static_cast<int>(in.brush_type)]++;
        }
    }

    Run census_run(log.header.grid_w, log.header.grid_h, log.header.seed);
    load_scene(census_run.grid, scene, 0, 0);

    // Sample the world before the first step, and keep those counts as the
    // baseline the peaks are read against. See `start_cells`.
    take_sample(census_run.grid, contents, false, /*is_baseline=*/true);
    for (int t = 0; t < SessionContents::TYPES; ++t) {
        contents.start_cells[t] = contents.peak_cells[t];
    }

    bool dug_since_sample = false;
    for (size_t i = 0; i < log.steps.size(); ++i) {
        census_run.step(log.steps[i]);
        if (log.steps[i].dig) dug_since_sample = true;
        if ((i + 1) % CENSUS_INTERVAL == 0) {
            take_sample(census_run.grid, contents, dug_since_sample);
            dug_since_sample = false;
        }
    }
    std::printf("\n  %-10s what the session contained - inputs exact, world sampled every %d steps\n",
                "contents", CENSUS_INTERVAL);
    std::printf("  %-10s dig %d steps, brush %d, moving %d, jumping %d (of %d)\n",
                "", contents.dig_steps, contents.brush_steps, contents.move_steps,
                contents.jump_steps, static_cast<int>(log.steps.size()));

    std::printf("  %-10s %-9s %10s %11s %11s %13s\n",
                "", "material", "painted", "at start", "peak", "samples seen");
    for (int t = 1; t < SessionContents::TYPES; ++t) {
        const bool seen = contents.samples_present[t] > 0;
        const int painted = contents.brush_by_type[t];
        if (!seen && painted == 0) {
            std::printf("  %-10s %-9s %10s %11s %11s %13s\n",
                        "", MATERIALS[t].name, "-", "-", "-", "never seen");
            continue;
        }
        std::printf("  %-10s %-9s %10d %11d %11d %9d/%-4d\n",
                    "", MATERIALS[t].name, painted, contents.start_cells[t],
                    contents.peak_cells[t], contents.samples_present[t], contents.samples);
    }

    // Two figures rather than one, because the larger is about the fixture scene
    // and the smaller is about the session. See `peak_awake_after_start`.
    std::printf("  %-10s peak %d of %d chunks awake once the scene had settled (%d in the opening sample)\n",
                "", contents.peak_awake_after_start,
                ((log.header.grid_w + Grid::CHUNK_SIZE - 1) / Grid::CHUNK_SIZE) *
                    ((log.header.grid_h + Grid::CHUNK_SIZE - 1) / Grid::CHUNK_SIZE),
                contents.peak_awake);
    std::printf("  %-10s Fire+Water+Sand all present in %d of %d samples, digging in %d of those\n",
                "", contents.mixed_samples, contents.samples, contents.mixed_dig_samples);
    std::printf("  %-10s Sampled, so this is evidence of presence and not of absence: a material\n"
                "  %-10s that lit and burned out between two samples reads as never seen.\n",
                "", "");
}

// The radii the sweep visits. 0 is venting off, 3 is what ships, and 2 and 4 are
// the two neighbours the original sweep chose between.
//
// **0 is in here because the original sweep's baseline could not be.** The
// number it was priced against - "`churning` at 3.13 ms/step with no venting at
// all" - came from a build with the call edited out, so the baseline and the
// three readings above it were four different binaries. At runtime the radius
// collapses the search box to the fluid's own cell, which is never Empty, so
// `vent_fluid` returns false and every caller takes the plain swap: the same
// behaviour, on the same instrument as everything it is being compared with.
constexpr int VENT_RADII[] = { 0, 2, 3, 4 };
constexpr int VENT_RADII_COUNT = static_cast<int>(sizeof(VENT_RADII) / sizeof(VENT_RADII[0]));

// `churning` at each radius. This is the original sweep's scenario, re-run the
// way the original sweep should have been run.
//
// Run at both world sizes for the reason P2 gives generally, and for one extra
// reason here: the numbers on record were taken at 960x540, so the small block
// is the only place the *shape* of the old sweep - how much each step of radius
// cost relative to the last - can be held against the new one. The absolute
// times cannot be compared across those sittings and are not being.
void run_vent_sweep_synthetic(const WorldSize& size) {
    std::printf("\n  `churning` at each radius, %dx%d - one binary, one sitting\n\n",
                size.w, size.h);
    for (int i = 0; i < VENT_RADII_COUNT; ++i) {
        char label[16];
        std::snprintf(label, sizeof(label), "vent r=%d", VENT_RADII[i]);
        run(label, size, build_churning, 0, nullptr, nullptr, nullptr, VENT_RADII[i]);
    }
}

// The same sweep against the recorded session, which is the half that answers
// what the knob costs *in play* rather than under a worst case built to hurt.
//
// **Deliberately a separate function from `run_replay` rather than a refactor of
// it.** `run_replay` prints the row the frame-budget rule reads, and the cheapest
// way to make that row mean something slightly different is to reorganise the
// code around it - one more world built before the clock starts changes what is
// in cache when the first step runs. The staleness check below is therefore a
// copy rather than a shared helper, and that is the trade being made knowingly:
// two copies that can drift, against not touching the authority row to add an
// instrument beside it.
//
// **Every row but r=3 will report a diverged end state, and that is the
// measurement rather than a fault.** A different venting radius is a different
// simulation; identical inputs are supposed to produce a different world. The
// column is printed anyway because its *r=3* row is a real check - if the
// shipped radius stopped replaying exactly, the conversion to a runtime value
// changed the simulation, and that is the one outcome this whole exercise must
// not have.
// One engine configuration to replay the session under. `shipped` marks the row
// whose end state has to come back `exact`.
struct ReplayConfig {
    const char* label;
    int vent_radius;
    bool seek_level;
    bool room_above;
    bool shipped;
};

void run_replay_configs(const char* log_path, const char* what,
                        const ReplayConfig* configs, int count) {
    input_log::Log log;
    std::string error;
    if (!input_log::read(log_path, log, &error)) {
        std::printf("  %-9s not run: %s\n", what, error.c_str());
        std::printf("            **Absent, not zero.** The synthetic table above is a worst case,\n"
                    "            not a played frame, and cannot stand in for this.\n");
        return;
    }

    std::string scene_error, scene_warning;
    const Scene scene = bmp::load("assets/test_material.bmp", "assets/test_albedo.bmp",
                                  &scene_error, &scene_warning);
    if (!scene_error.empty()) {
        std::printf("  %-9s not run: %s\n", what, scene_error.c_str());
        std::printf("            Run this from `code/`, where assets/ is.\n");
        return;
    }

    std::printf("\n  %s, %s: %d steps (%.1f s of play), seed %llu\n\n",
                log_path, what, static_cast<int>(log.steps.size()), log.steps.size() / 60.0,
                static_cast<unsigned long long>(log.header.seed));
    std::printf("  %-10s %-12s %10s %10s %14s %14s\n",
                "", "config", "mean ms", "p99 ms", "worst ms", "end state");

    for (int i = 0; i < count; ++i) {
        const ReplayConfig& cfg = configs[i];

        Run run(log.header.grid_w, log.header.grid_h, log.header.seed);

        // Before the scene is loaded, so that the world the clock sees was built
        // under the setting being measured from its very first cell.
        run.grid.set_vent_radius(cfg.vent_radius);
        run.grid.set_seek_level_enabled(cfg.seek_level);
        run.grid.set_room_above_enabled(cfg.room_above);

        const int placed = load_scene(run.grid, scene, 0, 0);
        if (placed != log.header.scene_cells ||
            input_log::fingerprint(run.grid) != log.header.start_fingerprint) {
            std::printf("  %-9s not run: the fixture scene has changed since this log was\n"
                        "            recorded. Same refusal, and the same reason, as the row above.\n",
                        what);
            return;
        }

        std::vector<double> step_ms;
        step_ms.reserve(log.steps.size());
        for (const Input& in : log.steps) {
            const auto t0 = std::chrono::steady_clock::now();
            run.step(in);
            const auto t1 = std::chrono::steady_clock::now();
            step_ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
        }
        if (step_ms.empty()) {
            std::printf("  %-9s not run: the log contains no steps.\n", what);
            return;
        }

        double total = 0.0;
        int over_budget = 0;
        for (double ms : step_ms) {
            total += ms;
            if (ms > FRAME_BUDGET_MS) over_budget++;
        }
        std::vector<double> sorted = step_ms;
        std::sort(sorted.begin(), sorted.end());

        const bool exact = input_log::fingerprint(run.grid) == log.header.end_fingerprint &&
                           run.player.cell_x() == log.header.end_player_x &&
                           run.player.cell_y() == log.header.end_player_y;

        char label[16];
        std::snprintf(label, sizeof(label), "%s%s", cfg.label, cfg.shipped ? "*" : "");
        std::printf("  %-10s %-12s %10.4f %10.4f %14.4f %14s   %d over budget\n",
                    "", label, total / step_ms.size(),
                    sorted[static_cast<size_t>(sorted.size() * 99 / 100)], sorted.back(),
                    exact ? "exact" : "diverged", over_budget);
    }

    std::printf("\n  %-10s * is the shipped configuration, and its row is the one that has to\n"
                "  %-10s read `exact` - anything else means the switches changed the simulation\n"
                "  %-10s rather than only measuring it. Every other row diverging is the point.\n",
                "", "", "");
}

const ReplayConfig VENT_CONFIGS[] = {
    { "r=0",  0, true, true, false },
    { "r=2",  2, true, true, false },
    { "r=3",  3, true, true, true  },
    { "r=4",  4, true, true, false },
};

void run_vent_sweep_replay(const char* log_path) {
    run_replay_configs(log_path, "vent radius", VENT_CONFIGS, 4);
}

// **The fluid breakdown - what the replayed row could not say about itself.**
//
// D4 is the loudest finding on the record, its plausible fix is E5b, and E5b's
// case is that these three displacement rules should be replaced wholesale. The
// replayed row times a whole `Run::step`, so it can say the budget is intact and
// nothing at all about which rule inside it is expensive. **Ablation is the only
// method available that does not need one build per data point**, which is the
// method this project has already been burned by once.
//
// What each row removes:
//
//   no vent    `vent_fluid`, the 7x7 box scan per powder-touching-fluid per tick
//   no seek    `seek_level`, and with it `find_lower_surface`'s flood fill of up
//              to MAX_PRESSURE_CELLS per awake surface cell per tick
//   no lift    `make_room_above`, the walk of up to MAX_DISPLACE_RISE cells that
//              the brush pays per painted cell
//   none       all three - **the whole of what E5b proposes to retire**, which
//              is the number that item has never had
//
// **The rows do not have to add up to `all`, and reading them as a partition is
// the mistake this comment exists to prevent.** Each is a separate simulation:
// removing a rule changes what the world does, so every later step in that run
// is doing different work, and the difference from `all` is that rule's share of
// *this scenario* rather than its share of a step. Overlap and interference are
// both possible and neither is a defect in the instrument.
const ReplayConfig FLUID_CONFIGS[] = {
    { "all",     Grid::DEFAULT_VENT_RADIUS, true,  true,  true  },
    { "no vent", 0,                         true,  true,  false },
    { "no seek", Grid::DEFAULT_VENT_RADIUS, false, true,  false },
    { "no lift", Grid::DEFAULT_VENT_RADIUS, true,  false, false },
    { "none",    0,                         false, false, false },
};
constexpr int FLUID_CONFIG_COUNT = static_cast<int>(sizeof(FLUID_CONFIGS) / sizeof(FLUID_CONFIGS[0]));

// The same five configurations against `churning`, which is the scenario built
// to hurt exactly here.
//
// **Run at the played size only.** The breakdown is about where played time
// goes, and `churning` at 960x540 settles inside the window while 1920x1080 does
// not - so the small world would answer a slightly different question at 15
// seconds a row. The radius sweep above already covers both sizes.
//
// **`no lift` is a null control and should read as zero.** `make_room_above`
// only fires on a brush write, and `churning` never paints - it builds its world
// once and then steps it. A row that removes a rule which cannot run measures
// the instrument, not the rule: whatever it reads is this table's noise floor,
// and any other row smaller than it means nothing.
void run_fluid_breakdown_synthetic(const WorldSize& size) {
    std::printf("\n  `churning` with each displacement rule removed, %dx%d\n\n", size.w, size.h);
    for (int i = 0; i < FLUID_CONFIG_COUNT; ++i) {
        const ReplayConfig& cfg = FLUID_CONFIGS[i];
        run(cfg.label, size, build_churning, 0, nullptr, nullptr, nullptr,
            cfg.vent_radius, cfg.seek_level, cfg.room_above);
    }
}

void run_all(const WorldSize& size) {
    std::printf("\nGrid %dx%d (%d cells), %d steps per scenario\n  %s\n\n",
                size.w, size.h, size.w * size.h, BENCH_STEPS, size.note);

    run("settled", size, build_settled, 120);
    run("sparse", size, build_sparse, 0);
    run("churning", size, build_churning, 0);
    run("cascading", size, build_settled, 120, cascade);
    run("burning", size, build_burning, 60, feed_fire);
    run("collapsing", size, build_collapsing, 70, churn_slabs);   // settle: fill the pipeline first
    run("shattering", size, build_shattering, 70, shatter_slabs,  // the same slabs, allowed to land
        peak_fractured_cells, "peak fractured cells (untimed, after the clock)");
}

} // namespace

int main(int argc, char** argv) {
    for (int i = 0; i < SIZE_COUNT; ++i) run_all(SIZES[i]);

    // Rendering, not simulation - a different instrument and a different unit
    // (per frame, not per step), so it is printed under its own heading rather
    // than as another row in a table that means something else. Run once, at the
    // played size; see the note on run_light for why a second size would be a
    // number about nothing.
    const WorldSize& played = SIZES[SIZE_COUNT - 1];
    std::printf("\nRender-side, viewport %dx%d cells, %d cells per light block, world %dx%d\n\n",
                LIGHT_VIEW_W, LIGHT_VIEW_H, LightField::BLOCK, played.w, played.h);
    run_light("light/fire", played, build_burning, feed_fire);
    run_light("light/dark", played, build_settled, nullptr);

    // P4's row, printed last and under its own heading for the same reason the
    // light rows are: a different unit (a whole `Run::step`, not `Grid::update`)
    // and a world it does not get to choose. The path is an argument so a second
    // recorded session can be measured without rebuilding.
    const char* log_path = argc > 1 ? argv[1] : "session.rec";
    std::printf("\nA replayed session - the row the frame-budget rule is aimed at (P4)\n\n");
    run_replay(log_path);

    // Last, and after everything that was here before it, so that no output any
    // document quotes moved when this was added.
    std::printf("\n\nThe VENT_RADIUS sweep, re-run in one binary\n");
    std::printf("  The radius is a cost knob and was swept across three builds, which is the\n"
                "  method PERFORMANCE.md's E1 entry records producing a confident 28%% out of\n"
                "  the compiler. Every row below comes from this process, this sitting, and -\n"
                "  for the replay - the same recorded input stream.\n");
    for (int i = 0; i < SIZE_COUNT; ++i) run_vent_sweep_synthetic(SIZES[i]);
    run_vent_sweep_replay(log_path);

    // The fluid spike's breakdown, last because it is the longest block and the
    // one most likely to be skipped when someone only wants the budget row.
    std::printf("\n\nWhere fluid time goes, by ablation (the fluid spike)\n");
    std::printf("  Each row removes one displacement rule. **They are separate simulations,\n"
                "  not a partition of a step** - removing a rule changes what the world does,\n"
                "  so a row's gap from `all` is that rule's share of this scenario. `no lift`\n"
                "  on `churning` is a null control: nothing paints there, so it prices the\n"
                "  instrument. `none` is the whole of what E5b proposes to retire.\n");
    run_fluid_breakdown_synthetic(SIZES[SIZE_COUNT - 1]);
    run_replay_configs(log_path, "displacement rules", FLUID_CONFIGS, FLUID_CONFIG_COUNT);

    std::printf("\n");
    return 0;
}
