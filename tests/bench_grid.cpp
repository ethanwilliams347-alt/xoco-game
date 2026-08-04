// Headless simulation benchmark.
//
// This exists so that optimisation work is measured rather than guessed at. The
// grid is sized to the resolution the project actually wants to hit (1920x1080
// at a 2px scale = 960x540 cells), not the small prototype window, because the
// whole point is to find out whether that target is reachable.
//
// Not registered with CTest: a benchmark that fails the build on a slow machine
// is a nuisance, and timings are for reading, not for gating.

#include "physics/grid.h"
#include "render/light.h"
#include "game/display.h"
#include <chrono>
#include <cstdio>

namespace {

// 1920x1080 at PIXEL_SCALE 2 - the resolution the "runs on low-end PCs" goal
// has to survive.
constexpr int BENCH_WIDTH = 960;
constexpr int BENCH_HEIGHT = 540;
constexpr int BENCH_STEPS = 300;

// A step must fit in 16.67 ms alongside rendering and input, so the simulation
// alone needs to be a fraction of that.
constexpr double FRAME_BUDGET_MS = 1000.0 / 60.0;

// A world that has come to rest. Nothing can move, so this is a direct measure
// of how much the engine pays for cells that are doing nothing - which is most
// of the world, most of the time, in a real game.
void build_settled(Grid& g) {
    for (int y = BENCH_HEIGHT / 2; y < BENCH_HEIGHT; ++y)
        for (int x = 0; x < BENCH_WIDTH; ++x)
            g.set_element(x, y, ElementType::Sand);
}

// The opposite extreme: bands of sand suspended over bands of water, so the
// whole lower world is sinking and displacing at once. Worst case by design.
void build_churning(Grid& g) {
    for (int y = BENCH_HEIGHT / 3; y < BENCH_HEIGHT; ++y) {
        const ElementType t = ((y / 8) % 2 == 0) ? ElementType::Sand : ElementType::Water;
        for (int x = 0; x < BENCH_WIDTH; ++x)
            g.set_element(x, y, t);
    }
}

// What an ordinary gameplay frame looks like: a large mostly-static world with a
// small patch of action in it.
void build_sparse(Grid& g) {
    for (int x = 0; x < BENCH_WIDTH; ++x)
        for (int y = BENCH_HEIGHT - 20; y < BENCH_HEIGHT; ++y)
            g.set_element(x, y, ElementType::Wall);

    for (int y = 40; y < 80; ++y)
        for (int x = 460; x < 500; ++x)
            g.set_element(x, y, ElementType::Sand);
}

// Every scenario above eventually comes to rest, which flatters its average. This
// one cannot: a row is scraped off the floor and a row is poured in at the
// ceiling every step, so mass stays constant and the whole column between them is
// permanently cascading. It is the sustained worst case - terrain collapsing
// continuously, nothing ever settling - and it is the number the 60 Hz budget
// should actually be judged against.
void cascade(Grid& g) {
    for (int x = 0; x < BENCH_WIDTH; ++x) {
        g.set_element(x, BENCH_HEIGHT - 1, ElementType::Empty);
        g.set_element(x, 0, ElementType::Sand);
    }
}

// A large Wood slab, permanently on fire. Reactions add per-cell work to
// exactly the cells that chunking cannot help - the active ones - so this is
// the scenario that has to be measured, not assumed, per the roadmap note on
// this item. A drip of fresh Fire along the top edge every step keeps
// ignition, spread, and decay all happening at once for the whole run, so the
// slab never fully burns down to Empty inside the measurement window.
void build_burning(Grid& g) {
    for (int y = BENCH_HEIGHT / 2; y < BENCH_HEIGHT; ++y)
        for (int x = 0; x < BENCH_WIDTH; ++x)
            g.set_element(x, y, ElementType::Wood);
}

void feed_fire(Grid& g) {
    for (int x = 0; x < BENCH_WIDTH; x += 4)
        g.set_element(x, BENCH_HEIGHT / 2, ElementType::Fire);
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
// as too big to judge and never fall at all.
constexpr int SLAB_W = 100;
constexpr int SLAB_H = 4;
constexpr int SLAB_SLOTS = 8;
constexpr int DRAIN_TOP = 400; // slabs are removed here rather than being allowed to land

void spawn_slab(Grid& g, int slot) {
    const int x0 = slot * (BENCH_WIDTH / SLAB_SLOTS) + 4;
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
// measures nothing. This drains them at DRAIN_TOP instead and feeds a
// replacement in at the ceiling, so eight are always in the air. The band is
// deeper than MAX_FALL_SPEED because a slab at full speed would step over a
// thinner one. Reading before writing keeps the drain nearly free while it is
// empty, which is most of it, most of the time.
void churn_slabs(Grid& g) {
    static int tick = 0;

    for (int y = DRAIN_TOP; y < DRAIN_TOP + 12; ++y)
        for (int x = 0; x < BENCH_WIDTH; ++x)
            if (g.get_element(x, y).type != ElementType::Empty)
                g.set_element(x, y, ElementType::Empty);

    // One new slab every 8 steps, cycling the slots. A slab needs 64 steps to
    // accelerate its way down to the drain, which is exactly how long a slot
    // waits its turn, so a slot is always clear when its next slab arrives.
    if (tick % 8 == 0) spawn_slab(g, (tick / 8) % SLAB_SLOTS);
    tick++;
}

void build_collapsing(Grid&) {} // the hook does all of it

// `collapsing` prices pieces *in the air*, and it says so: it drains them at
// DRAIN_TOP "rather than being allowed to land". That is the right design for
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
constexpr int SHATTER_FLOOR = 300;

void build_shattering(Grid& g) {
    // A sawtooth: steps eight cells deep every thirty-two across, so a 100-wide
    // slab always spans several of them.
    for (int x = 0; x < BENCH_WIDTH; ++x) {
        const int top = SHATTER_FLOOR + ((x / 32) % 2 == 0 ? 0 : 8);
        for (int y = top; y < BENCH_HEIGHT; ++y) g.set_element(x, y, ElementType::Wall);
    }
}

void shatter_slabs(Grid& g) {
    static int tick = 0;

    // Clear the heap every 60 steps, floor included, and rebuild the sawtooth.
    // Cheaper and more honest than draining a band: it puts the scenario back to
    // the state its name describes instead of letting it drift into "a deep pile
    // of rubble settling", which is a different measurement.
    if (tick % 60 == 59) {
        for (int y = SHATTER_FLOOR - 40; y < BENCH_HEIGHT; ++y)
            for (int x = 0; x < BENCH_WIDTH; ++x)
                if (g.get_element(x, y).type != ElementType::Empty)
                    g.set_element(x, y, ElementType::Empty);
        build_shattering(g);
    } else if (tick % 8 == 0) {
        spawn_slab(g, (tick / 8) % SLAB_SLOTS);
    }
    tick++;
}

void run(const char* name, void (*build)(Grid&), int settle_steps, void (*on_step)(Grid&) = nullptr) {
    Grid g(BENCH_WIDTH, BENCH_HEIGHT);
    build(g);

    // Let the scenario reach its steady state before the clock starts, so the
    // number describes the state named in the label rather than the setup.
    for (int i = 0; i < settle_steps; ++i) {
        if (on_step) on_step(g);
        g.update();
    }

    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < BENCH_STEPS; ++i) {
        if (on_step) on_step(g);
        g.update();
    }
    const auto end = std::chrono::steady_clock::now();

    const double total_ms = std::chrono::duration<double, std::milli>(end - start).count();
    const double per_step = total_ms / BENCH_STEPS;

    // Reported alongside the timing because it is the number that explains it:
    // if a scenario is slow while barely any chunks are awake, the cost is not
    // where chunking can reach it.
    const int total_chunks = ((BENCH_WIDTH + Grid::CHUNK_SIZE - 1) / Grid::CHUNK_SIZE) *
                             ((BENCH_HEIGHT + Grid::CHUNK_SIZE - 1) / Grid::CHUNK_SIZE);

    std::printf("  %-10s %9.4f ms/step  %6.1f%% of a 60 Hz frame   %4d/%d chunks awake%s\n",
                name, per_step, 100.0 * per_step / FRAME_BUDGET_MS,
                g.active_chunk_count(), total_chunks,
                per_step < FRAME_BUDGET_MS ? "" : "  <-- OVER BUDGET");
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
// Sized to the **viewport**, not to BENCH_WIDTH/HEIGHT. The light field covers
// what is on screen and nothing else, so it does not scale with world size - and
// building it at 960x540 would measure a configuration the game never runs.
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

void run_light(const char* name, void (*build)(Grid&), void (*on_step)(Grid&)) {
    Grid g(BENCH_WIDTH, BENCH_HEIGHT);
    build(g);
    for (int i = 0; i < 60; ++i) {
        if (on_step) on_step(g);
        g.update();
    }

    // Aimed at the band where the fire actually is. A light field pointed at
    // empty sky early-outs on `any_light()` and would measure the scan alone -
    // which is a real number, but not the one this scenario is named for.
    const int origin_x = BENCH_WIDTH / 2 - LIGHT_VIEW_W / 2;
    const int origin_y = BENCH_HEIGHT / 2 - LIGHT_VIEW_H / 2;

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
        if (on_step) on_step(g);
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

} // namespace

int main() {
    std::printf("\nGrid %dx%d (%d cells), %d steps per scenario\n\n",
                BENCH_WIDTH, BENCH_HEIGHT, BENCH_WIDTH * BENCH_HEIGHT, BENCH_STEPS);

    run("settled", build_settled, 120);
    run("sparse", build_sparse, 0);
    run("churning", build_churning, 0);
    run("cascading", build_settled, 120, cascade);
    run("burning", build_burning, 60, feed_fire);
    run("collapsing", build_collapsing, 70, churn_slabs);      // settle: fill the pipeline first
    run("shattering", build_shattering, 70, shatter_slabs);    // the same slabs, allowed to land

    // Rendering, not simulation - a different instrument and a different unit
    // (per frame, not per step), so it is printed under its own heading rather
    // than as another row in a table that means something else.
    std::printf("\nRender-side, viewport %dx%d cells, %d cells per light block\n\n",
                LIGHT_VIEW_W, LIGHT_VIEW_H, LightField::BLOCK);
    run_light("light/fire", build_burning, feed_fire);
    run_light("light/dark", build_settled, nullptr);

    std::printf("\n");
    return 0;
}
