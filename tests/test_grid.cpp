#include "physics/grid.h"
#include "test_util.h"
#include <string>

static void step(Grid& g, int n) {
    for (int i = 0; i < n; ++i) g.update();
}

static int count_of(Grid& g, ElementType t) {
    int n = 0;
    for (int y = 0; y < g.get_height(); ++y)
        for (int x = 0; x < g.get_width(); ++x)
            if (g.get_element(x, y).type == t) n++;
    return n;
}

// Average row index of a material; lower = higher on screen.
static double mean_row(Grid& g, ElementType t) {
    double sum = 0; int n = 0;
    for (int y = 0; y < g.get_height(); ++y)
        for (int x = 0; x < g.get_width(); ++x)
            if (g.get_element(x, y).type == t) { sum += y; n++; }
    return n ? sum / n : -1.0;
}

// A small Wall-sealed box holding exactly two touching cells, `a` at (1,1) and
// `b` at (2,1). Fire is a Gas and rises away from whatever it is next to
// within a frame or two, so an open-field "place them touching" test measures
// how long two things happen to stay adjacent, not the reaction's real odds.
// Walling them in removes every legal move, pinning them in contact so the
// reaction gets its full, repeated chance to fire.
static Grid make_sealed_pair(ElementType a, ElementType b) {
    Grid g(4, 3);
    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < 4; ++x)
            g.set_element(x, y, ElementType::Wall);
    g.set_element(1, 1, a);
    g.set_element(2, 1, b);
    return g;
}

// Ignition is a race, not a certainty: a lone ember has some chance of
// self-extinguishing (Fire's 6% spontaneous decay) before it manages to
// ignite whatever it is touching, so a single sealed pair is not reliable
// enough on its own - it can genuinely, correctly fail. Run many independent
// pairs instead and require a safe fraction to ignite.
static int count_ignitions(ElementType target, int steps, int trials) {
    int ignited = 0;
    for (int i = 0; i < trials; ++i) {
        Grid g = make_sealed_pair(target, ElementType::Fire);
        step(g, steps);
        if (g.get_element(1, 1).type != target) ignited++;
    }
    return ignited;
}

// A settled powder should never have a gap directly beneath it. This is the
// invariant that chunked updates break if a write fails to wake its neighbours.
static bool no_floating_powder(Grid& g) {
    for (int y = 0; y < g.get_height() - 1; ++y)
        for (int x = 0; x < g.get_width(); ++x)
            if (g.get_element(x, y).type == ElementType::Sand &&
                g.get_element(x, y + 1).type == ElementType::Empty)
                return false;
    return true;
}

int main() {
    // --- sand falls to the floor and is conserved ---
    {
        Grid g(40, 40);
        for (int i = 0; i < 10; ++i) g.set_element(20, i, ElementType::Sand);
        step(g, 200);
        check("sand is conserved while falling", count_of(g, ElementType::Sand) == 10,
              "count=" + std::to_string(count_of(g, ElementType::Sand)));
        check("sand settles on the floor", g.get_element(20, 39).type == ElementType::Sand);
    }

    // --- static materials never move ---
    {
        Grid g(20, 20);
        g.set_element(10, 5, ElementType::Wall);
        g.set_element(11, 5, ElementType::Wood);
        step(g, 100);
        check("wall is static", g.get_element(10, 5).type == ElementType::Wall);
        check("wood is static", g.get_element(11, 5).type == ElementType::Wood);
    }

    // --- water spreads out instead of forming a column ---
    {
        Grid g(40, 40);
        for (int i = 0; i < 20; ++i) g.set_element(20, i, ElementType::Water);
        step(g, 300);
        int widest = 0;
        for (int y = 0; y < 40; ++y) {
            int row = 0;
            for (int x = 0; x < 40; ++x) if (g.get_element(x, y).type == ElementType::Water) row++;
            widest = row > widest ? row : widest;
        }
        check("water spreads horizontally", widest > 5, "widest row=" + std::to_string(widest));
        check("water is conserved", count_of(g, ElementType::Water) == 20);
    }

    // --- sand sinks through water (denser) ---
    {
        Grid g(20, 40);
        for (int y = 30; y < 40; ++y)
            for (int x = 0; x < 20; ++x) g.set_element(x, y, ElementType::Water);
        for (int x = 8; x < 12; ++x) g.set_element(x, 5, ElementType::Sand);
        step(g, 400);
        const double sand = mean_row(g, ElementType::Sand);
        const double water = mean_row(g, ElementType::Water);
        check("sand sinks below water", sand > water,
              "sand row=" + std::to_string(sand) + " water row=" + std::to_string(water));
    }

    // --- oil floats on water (less dense) ---
    {
        Grid g(20, 40);
        for (int y = 20; y < 40; ++y)
            for (int x = 0; x < 20; ++x) g.set_element(x, y, ElementType::Oil);
        for (int x = 0; x < 20; ++x) g.set_element(x, 5, ElementType::Water);
        step(g, 600);
        const double oil = mean_row(g, ElementType::Oil);
        const double water = mean_row(g, ElementType::Water);
        check("oil floats above water", oil < water,
              "oil row=" + std::to_string(oil) + " water row=" + std::to_string(water));
    }

    // --- steam rises to the ceiling ---
    {
        Grid g(20, 40);
        for (int x = 8; x < 12; ++x) g.set_element(x, 35, ElementType::Steam);
        step(g, 300);
        const double steam = mean_row(g, ElementType::Steam);
        check("steam rises", steam >= 0.0 && steam < 5.0, "steam row=" + std::to_string(steam));
        check("steam is conserved", count_of(g, ElementType::Steam) == 4);
    }

    // --- nothing escapes the sealed border ---
    {
        Grid g(30, 30);
        for (int x = 0; x < 30; ++x)
            for (int y = 0; y < 3; ++y) g.set_element(x, y, ElementType::Water);
        step(g, 400);
        check("no material leaks out of bounds", count_of(g, ElementType::Water) == 90,
              "count=" + std::to_string(count_of(g, ElementType::Water)));
    }

    // --- chunked updates: a resting world sleeps, and wakes when disturbed ---
    {
        const int W = Grid::CHUNK_SIZE * 3;
        const int H = Grid::CHUNK_SIZE * 3;
        Grid g(W, H);

        // Wall-to-wall sand, so the block is stable the moment it is placed.
        const int top = H - Grid::CHUNK_SIZE;
        for (int y = top; y < H; ++y)
            for (int x = 0; x < W; ++x) g.set_element(x, y, ElementType::Sand);
        const int placed = (H - top) * W;

        step(g, 60);
        check("a settled world goes fully to sleep", g.active_chunk_count() == 0,
              "active chunks=" + std::to_string(g.active_chunk_count()));

        // Dig a single grain out from under the middle of the block. Only that
        // one cell is written, so everything above it must be woken indirectly.
        g.set_element(W / 2, H - 1, ElementType::Empty);
        check("disturbing a sleeping world wakes it", g.active_chunk_count() > 0,
              "active chunks=" + std::to_string(g.active_chunk_count()));

        step(g, 300);
        check("sand does not float over a hole dug beneath it", no_floating_powder(g));
        check("sand is conserved through the collapse",
              count_of(g, ElementType::Sand) == placed - 1,
              "count=" + std::to_string(count_of(g, ElementType::Sand)));
        check("the world settles back to sleep", g.active_chunk_count() == 0,
              "active chunks=" + std::to_string(g.active_chunk_count()));
    }

    // --- chunked updates: no seams along the invisible chunk borders ---
    {
        const int W = Grid::CHUNK_SIZE * 3;
        const int H = Grid::CHUNK_SIZE * 3;
        Grid g(W, H);

        // Dropped exactly on a vertical chunk border, and falling far enough to
        // cross every horizontal one on the way down.
        const int border_x = Grid::CHUNK_SIZE;
        for (int i = 0; i < 5; ++i) g.set_element(border_x, i, ElementType::Sand);

        step(g, 400);
        check("sand falls across chunk borders", g.get_element(border_x, H - 1).type == ElementType::Sand);
        check("sand is conserved across chunk borders", count_of(g, ElementType::Sand) == 5,
              "count=" + std::to_string(count_of(g, ElementType::Sand)));
    }

    // --- chunked updates: liquid spreads through a chunk border ---
    {
        const int W = Grid::CHUNK_SIZE * 3;
        Grid g(W, 40);
        for (int i = 0; i < 30; ++i) g.set_element(Grid::CHUNK_SIZE - 1, i, ElementType::Water);

        step(g, 400);
        bool crossed = false;
        for (int y = 0; y < 40; ++y)
            for (int x = Grid::CHUNK_SIZE; x < W; ++x)
                if (g.get_element(x, y).type == ElementType::Water) crossed = true;

        check("water spreads past a chunk border", crossed);
        check("water is conserved across a chunk border", count_of(g, ElementType::Water) == 30,
              "count=" + std::to_string(count_of(g, ElementType::Water)));
    }

    // --- reactions: fire ignites adjacent wood ---
    // Wood's 12%/step ignition chance races against Fire's 6%/step self-decay
    // (roughly a 2:1 in wood's favour), so ~30 independent trials at a >=40%
    // bar is the honest way to assert this rather than expecting near-certain
    // single-trial success, which the odds do not actually support.
    {
        const int trials = 30;
        const int ignited = count_ignitions(ElementType::Wood, 60, trials);
        check("fire ignites adjacent wood", ignited >= 12,
              "ignited=" + std::to_string(ignited) + "/" + std::to_string(trials));
    }

    // --- reactions: fire ignites adjacent oil ---
    // Oil's 40%/step chance dominates the same 6% race far more comfortably
    // (~87% per trial), so the pass bar can sit much closer to the mean.
    {
        const int trials = 30;
        const int ignited = count_ignitions(ElementType::Oil, 40, trials);
        check("fire ignites adjacent oil", ignited >= 18,
              "ignited=" + std::to_string(ignited) + "/" + std::to_string(trials));
    }

    // --- reactions: water extinguishes fire into steam ---
    {
        Grid g = make_sealed_pair(ElementType::Fire, ElementType::Water);
        step(g, 30);
        check("water extinguishes fire into steam", g.get_element(1, 1).type == ElementType::Steam,
              "type=" + std::string(material_of(g.get_element(1, 1).type).name));
    }

    // --- reactions: fire burns out on its own with no catalyst nearby ---
    {
        Grid g(20, 20);
        g.set_element(10, 10, ElementType::Fire);
        step(g, 150);
        check("fire burns out with no catalyst nearby", g.get_element(10, 10).type == ElementType::Empty,
              "type=" + std::string(material_of(g.get_element(10, 10).type).name));
    }

    // --- reactions: a trapped fire still ticks instead of freezing ---
    // Regression test for the chunked-updates interaction: a spontaneous
    // reaction (fire's self-decay) has no movement to piggyback a wake on, so
    // try_react must self-mark every frame or a fire with nowhere to move
    // freezes the instant its chunk goes back to sleep. Sealed in Wall (not
    // Wood) to isolate pure self-decay from ignition, and run as 30 independent
    // trials rather than one: a single trial would still get one free, fully
    // -woken frame from its own placement, which is not a reliable regression
    // signal on its own.
    {
        const int COLS = 6, ROWS = 5;
        Grid g(COLS * 3 + 2, ROWS * 3 + 2);
        for (int j = 0; j < ROWS; ++j) {
            for (int i = 0; i < COLS; ++i) {
                const int cx = i * 3 + 1;
                const int cy = j * 3 + 1;
                for (int dy = -1; dy <= 1; ++dy)
                    for (int dx = -1; dx <= 1; ++dx)
                        if (dx != 0 || dy != 0) g.set_element(cx + dx, cy + dy, ElementType::Wall);
                g.set_element(cx, cy, ElementType::Fire);
            }
        }

        step(g, 200);

        int decayed = 0;
        for (int j = 0; j < ROWS; ++j)
            for (int i = 0; i < COLS; ++i)
                if (g.get_element(i * 3 + 1, j * 3 + 1).type == ElementType::Empty) decayed++;

        check("a trapped fire still burns out instead of freezing", decayed >= 25,
              "decayed=" + std::to_string(decayed) + "/" + std::to_string(COLS * ROWS));
    }

    return report();
}
