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

} // namespace

int main() {
    std::printf("\nGrid %dx%d (%d cells), %d steps per scenario\n\n",
                BENCH_WIDTH, BENCH_HEIGHT, BENCH_WIDTH * BENCH_HEIGHT, BENCH_STEPS);

    run("settled", build_settled, 120);
    run("sparse", build_sparse, 0);
    run("churning", build_churning, 0);
    run("cascading", build_settled, 120, cascade);

    std::printf("\n");
    return 0;
}
