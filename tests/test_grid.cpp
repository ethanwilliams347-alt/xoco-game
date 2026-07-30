#include "physics/grid.h"
#include "physics/random.h"
#include "test_util.h"
#include <cstdint>
#include <set>
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

// A scene that touches every part of the engine that consumes randomness:
// colour jitter at placement, the per-row sweep direction, the powder and fluid
// direction picks, and the reaction roll. A determinism test built on a quieter
// scene would pass while most of the randomness in the engine sat unexercised.
static void build_mixed(Grid& g) {
    for (int y = 40; y < 44; ++y)
        for (int x = 0; x < g.get_width(); ++x) g.set_element(x, y, ElementType::Water);
    for (int y = 5; y < 15; ++y)
        for (int x = 10; x < 40; ++x) g.set_element(x, y, ElementType::Sand);
    for (int y = 20; y < 30; ++y)
        for (int x = 50; x < 70; ++x) g.set_element(x, y, ElementType::Wood);
    g.set_element(55, 19, ElementType::Fire);

    // Structure placed with the brush is assumed to be standing on purpose, so
    // the slab has to be knocked loose before it will fall. Put a cell under it
    // and take it straight back out. This pulls the falling-structure state -
    // support marks and fall_ticks - into the comparison as well.
    g.set_element(50, 30, ElementType::Wall);
    g.set_element(50, 30, ElementType::Empty);
}

// Compared field by field rather than with memcmp: Element carries padding, and
// a difference in padding bytes is not a difference in the world.
static bool worlds_match(const Grid& a, const Grid& b) {
    if (a.get_pixels() != b.get_pixels()) return false;
    for (int y = 0; y < a.get_height(); ++y) {
        for (int x = 0; x < a.get_width(); ++x) {
            const Element ea = a.get_element(x, y);
            const Element eb = b.get_element(x, y);
            if (ea.type != eb.type || ea.color != eb.color ||
                ea.updated_tag != eb.updated_tag || ea.fall_ticks != eb.fall_ticks)
                return false;
        }
    }
    return true;
}

// A small Wall-sealed box holding exactly two touching cells, `a` at (1,1) and
// `b` at (2,1). Fire is a Gas and rises away from whatever it is next to
// within a frame or two, so an open-field "place them touching" test measures
// how long two things happen to stay adjacent, not the reaction's real odds.
// Walling them in removes every legal move, pinning them in contact so the
// reaction gets its full, repeated chance to fire.
static Grid make_sealed_pair(ElementType a, ElementType b, uint64_t seed = Grid::DEFAULT_SEED) {
    Grid g(4, 3, seed);
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
//
// Each trial needs its own seed, and that is not decoration. These grids are
// identical down to the cell, so trials sharing a seed are not thirty samples of
// a 2:1 race - they are one sample counted thirty times, and the result can only
// ever be 0/30 or 30/30. That is what this looked like between F1.1 (which made
// the seed fixed) and F1.4 (which changed the numbers it produces, turning a
// silent 30/30 into a loud 0/30). The test was wrong the whole time; only its
// answer changed.
static int count_ignitions(ElementType target, int steps, int trials) {
    int ignited = 0;
    for (int i = 0; i < trials; ++i) {
        Grid g = make_sealed_pair(target, ElementType::Fire, 9000 + static_cast<uint64_t>(i));
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

    // --- the hash behind the randomness ---
    // Tested directly, and not only through the world, because the failure mode
    // that matters is invisible from the outside. A mixer that is merely *poor* -
    // biased, or correlated between streams, or repeating for a cell across steps
    // - still produces a world that settles, stratifies and burns exactly as
    // every other test in this file expects. It would just look subtly wrong in
    // motion, which is not a thing an assertion can notice. So the mixer is
    // checked on its own terms rather than through its consequences.
    {
        using namespace sim_random;
        const uint64_t seed = 0xC0FFEEull;

        check("the hash is a function of its inputs",
              bits(seed, 7, 99, Stream::Reaction) == bits(seed, 7, 99, Stream::Reaction));

        // Balance across neighbouring cells on one step, and across consecutive
        // steps for one cell. Both matter and they fail differently: the first
        // going wrong looks like diagonal banding in falling powder, the second
        // looks like a cell that has made its mind up and stopped rerolling.
        int across_cells = 0, across_steps = 0, stream_disagreements = 0;
        for (uint64_t i = 0; i < 10000; ++i) {
            if (coin(seed, 1, i, Stream::PowderDirection)) across_cells++;
            if (coin(seed, i, 1, Stream::PowderDirection)) across_steps++;
            if (coin(seed, 1, i, Stream::PowderDirection) != coin(seed, 1, i, Stream::FluidDirection))
                stream_disagreements++;
        }
        check("neighbouring cells get unrelated values", across_cells > 4500 && across_cells < 5500,
              std::to_string(across_cells) + "/10000");
        check("consecutive steps get unrelated values", across_steps > 4500 && across_steps < 5500,
              std::to_string(across_steps) + "/10000");

        // Two streams reading the same cell on the same step must disagree about
        // half the time. Sharing a value here would not look random-ish and wrong,
        // it would look like a permanent correlation between two unrelated rules.
        check("separate streams do not track each other",
              stream_disagreements > 4500 && stream_disagreements < 5500,
              std::to_string(stream_disagreements) + "/10000");

        check("chance(0) never fires", !chance(0, seed, 3, 4, Stream::Reaction));
        check("chance(100) always fires", chance(100, seed, 3, 4, Stream::Reaction));

        int hits = 0;
        for (uint64_t i = 0; i < 10000; ++i)
            if (chance(25, seed, 1, i, Stream::Reaction)) hits++;
        check("chance(pct) fires at about the rate asked for", hits > 2300 && hits < 2700,
              std::to_string(hits) + "/10000, wanted ~2500");

        // The reserved world-generation streams (F1.5). random.h already asserts
        // at compile time that no two stream values are equal, but that is the
        // weaker half of what is needed: two tags one bit apart are distinct and
        // still correlated, which is the same permanent-correlation failure the
        // check above exists to rule out. So the minted streams are held to the
        // same standard as the declared ones - against the simulation, which they
        // must never influence, and against each other, since the generator will
        // mint several and use them on the same cells.
        int gen_vs_sim = 0, gen_vs_gen = 0;
        for (uint64_t i = 0; i < 10000; ++i) {
            if (coin(seed, 0, i, worldgen(0)) != coin(seed, 0, i, Stream::PowderDirection)) gen_vs_sim++;
            if (coin(seed, 0, i, worldgen(0)) != coin(seed, 0, i, worldgen(1))) gen_vs_gen++;
        }
        check("generation streams do not track the simulation",
              gen_vs_sim > 4500 && gen_vs_sim < 5500, std::to_string(gen_vs_sim) + "/10000");
        check("generation streams do not track each other",
              gen_vs_gen > 4500 && gen_vs_gen < 5500, std::to_string(gen_vs_gen) + "/10000");
    }

    // --- the step clock ---
    // Nothing reads this yet; it exists so the hash that replaces the generator
    // has a wide time input, and so a save file can say where a run had got to.
    // Checked anyway, because a counter nothing observes is a counter that can be
    // quietly wrong right up until the thing depending on it is built - at which
    // point the bug looks like it is in the new code.
    {
        Grid g(32, 32);
        check("a fresh grid has taken no steps", g.steps() == 0,
              "steps=" + std::to_string(g.steps()));
        step(g, 5);
        check("the step clock counts every update", g.steps() == 5,
              "steps=" + std::to_string(g.steps()));

        // A step that does no work still happened. The world here is empty, so
        // every chunk is asleep and the sweep touches nothing - the clock must
        // advance regardless, or it would measure activity rather than time and
        // two runs paused for different lengths would disagree about "when" it is.
        const uint64_t before = g.steps();
        step(g, 3);
        check("an idle step still advances the clock", g.steps() == before + 3,
              "steps=" + std::to_string(g.steps()));
    }

    // --- colour jitter ---
    // Two checks because F1.4 deliberately changed what a player sees, and an
    // untested behaviour change is indistinguishable from a bug. Jitter is now
    // hashed on position with no step input, so it is fixed to the spot rather
    // than drawn fresh on every write.
    {
        Grid g(32, 32, 777);

        // Still live. The cheapest way to break jitter while passing every other
        // test in this file is to flatten it to the table colour, which nothing
        // about the physics would notice.
        std::set<uint32_t> shades;
        for (int x = 0; x < 32; ++x) {
            g.set_element(x, 0, ElementType::Sand);
            shades.insert(g.get_element(x, 0).color);
        }
        check("jitter varies between neighbouring cells", shades.size() >= 8,
              std::to_string(shades.size()) + " distinct shades across 32 cells");

    }
    {
        // The change itself, pinned down. A fresh grid so the falling sand above
        // cannot wander into the cell under test. Steps run between the two
        // writes so the clock has moved on: if the step number were still
        // reaching the jitter hash, the repainted cell would come back a
        // different colour and this would fail.
        Grid g(32, 32, 777);
        g.set_element(5, 5, ElementType::Sand);
        const uint32_t first = g.get_element(5, 5).color;
        g.set_element(5, 5, ElementType::Empty);
        step(g, 7);
        g.set_element(5, 5, ElementType::Sand);
        check("a cell repainted in the same spot comes back the same shade",
              g.get_element(5, 5).color == first);
    }

    // --- determinism: the same seed produces the same world ---
    // Three checks, and the middle one is what makes the first mean anything.
    // Two worlds also match when nothing random ever happened, so an equality
    // test on its own would pass against an engine with its randomness wired to
    // a constant, or against an empty grid. Requiring that a *different* seed
    // diverges is what pins down that the seed is actually reaching the work.
    {
        Grid a(80, 60, 4242); build_mixed(a); step(a, 200);
        Grid b(80, 60, 4242); build_mixed(b); step(b, 200);
        check("the same seed produces the same world", worlds_match(a, b));

        Grid c(80, 60, 4243); build_mixed(c); step(c, 200);
        check("a different seed produces a different world", !worlds_match(a, c));

        // The high half of the seed must survive. std::mt19937 seeds from 32
        // bits, so a seed handed straight to it would drop everything above bit
        // 31 and these two worlds would come out identical.
        Grid d(80, 60, 1ull); build_mixed(d); step(d, 200);
        Grid e(80, 60, 1ull | (1ull << 40)); build_mixed(e); step(e, 200);
        check("the whole 64-bit seed is used", !worlds_match(d, e));
    }

    // --- Grid::reset ---
    {
        // A queued-but-not-yet-resolved support check, built without ever
        // calling update(): a floating Wood block, then one cell knocked out
        // of it. Removal queues a check for the 3x3 neighbourhood immediately;
        // resolve_support() would not run until the next step, which never
        // happens here. This is the one piece of state reset() can only prove
        // it cleared by looking at directly - nothing about an Empty world
        // with zero awake chunks would tell you the queue was still holding
        // stale entries.
        Grid g(20, 20, 42);
        for (int y = 5; y <= 7; ++y)
            for (int x = 5; x <= 7; ++x)
                g.set_element(x, y, ElementType::Wood);
        g.set_element(6, 6, ElementType::Empty);
        check("building the scene actually queues a support check",
              g.has_pending_support_checks());
        check("building the scene actually wakes chunks",
              g.active_chunk_count() > 0);

        g.reset(42);

        bool all_empty = true;
        for (int y = 0; y < g.get_height() && all_empty; ++y)
            for (int x = 0; x < g.get_width() && all_empty; ++x)
                if (g.get_element(x, y).type != ElementType::Empty) all_empty = false;
        check("reset clears every cell back to Empty", all_empty);
        check("reset puts every chunk back to sleep", g.active_chunk_count() == 0);
        check("reset clears the queued support check", !g.has_pending_support_checks());
    }
    {
        // The one check that can actually catch a member left out of the wipe.
        // A stale step_count is the example the roadmap names: it would not
        // show up as a non-Empty cell or an awake chunk, only as a divergence
        // once the reset grid starts rolling randomness again from the wrong
        // step. So the two worlds are not just reset and compared - they are
        // reset, driven through the same scripted scene as a fresh grid, and
        // then compared, which is what gives a forgotten field somewhere to
        // actually show up.
        Grid a(80, 60, 9090);
        build_mixed(a);
        step(a, 100);
        a.reset(9090);
        build_mixed(a);
        step(a, 100);

        Grid b(80, 60, 9090);
        build_mixed(b);
        step(b, 100);

        check("a reset run matches a fresh run built with the same seed", worlds_match(a, b));
    }

    // --- Grid::paint ---
    {
        Grid g(20, 20, 123);
        check("grid starts with 0 active chunks", g.active_chunk_count() == 0);
        
        for (int y = 9; y <= 11; ++y)
            for (int x = 9; x <= 11; ++x)
                g.paint(x, y, ElementType::Wood, 0xFF123456);
        
        Element el = g.get_element(10, 10);
        check("paint sets exactly the specified color without jitter", el.color == 0xFF123456);
        check("paint sets the specified element type", el.type == ElementType::Wood);
        check("paint wakes the chunk", g.active_chunk_count() > 0);
        
        g.paint(10, 10, ElementType::Empty, 0x00000000);
        check("paint over structure with non-structure queues support checks", g.has_pending_support_checks());
    }

    // A painted cell with nothing under it has to fall, per F4.1's verify note
    // - the actual proof that paint's write wakes the chunk, rather than the
    // proxy of merely observing active_chunk_count() above. Sand, not Wood:
    // a powder's movement is evaluated every awake step on its own, with no
    // queued check involved, so it is the shape that isolates "did the write
    // wake the chunk" from structural falling's separate disturbance-only
    // trigger (see the comment on Grid::place - a freshly placed structural
    // cell is deliberately left standing, precisely so a scene can paint a
    // platform without it collapsing on load).
    {
        Grid g(20, 20, 123);
        g.paint(10, 10, ElementType::Sand, 0xFFABCDEF);

        check("a freshly painted cell starts exactly where it was painted",
              g.get_element(10, 10).type == ElementType::Sand);

        step(g, 40);

        check("a painted cell with nothing under it falls",
              g.get_element(10, 10).type == ElementType::Empty);
        check("...landing at the bottom of the world, keeping its colour",
              g.get_element(10, 19).type == ElementType::Sand &&
              g.get_element(10, 19).color == 0xFFABCDEF);
    }

    return report();
}
