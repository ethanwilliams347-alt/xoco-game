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
#include <chrono>
#include <cstdio>

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

void run(const char* name, const WorldSize& size,
         void (*build)(Grid&, Bench&), int settle_steps,
         void (*on_step)(Grid&, Bench&) = nullptr,
         int (*witness)(Grid&, Bench&) = nullptr, const char* witness_label = nullptr) {
    Bench b{size.w, size.h};
    Grid g(b.w, b.h);
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

int main() {
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

    std::printf("\n");
    return 0;
}
