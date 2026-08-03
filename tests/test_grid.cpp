#include "physics/grid.h"
#include "physics/random.h"
#include "test_util.h"
#include <cstdint>
#include <cstdlib>
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
    // support marks and ticks - into the comparison as well.
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
                ea.updated_tag != eb.updated_tag || ea.ticks != eb.ticks ||
                ea.temperature != eb.temperature || ea.piece_tag != eb.piece_tag)
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

// Steps a sealed pair takes to ignite `target` from the Fire beside it, or
// `limit` if it never does.
//
// **This used to be a statistical test and no longer is, and that is the point
// of E2 rather than a convenience.** Ignition was a 12%-per-step dice roll
// racing Fire's own 6% burnout, so the honest assertion was thirty independent
// seeds and a bar at 40% - a test that could correctly fail. It is now a
// threshold: heat conducts into the wood at a rate the table sets, the wood
// crosses its ignition point, and it catches. Same seed, same answer, every
// time, and the *number of steps* is now a meaningful quantity to assert on,
// which is exactly the state between "wood" and "on fire" that was missing.
//
// The per-trial seeds are gone with the statistics. They were load-bearing when
// this measured a race - trials sharing a seed are one sample counted thirty
// times, and this test was silently 30/30 between F1.1 and F1.4 for that reason.
// With no roll left to sample there is nothing for them to vary.
// The flame is re-placed every step so that it is a source rather than
// something with a lifetime of its own. Without that, this measures Fire's 6%
// burnout as much as it measures ignition, and it fails outright for any seed
// whose ember happens to die during the handful of steps the target needs to
// come up to temperature - which is what it did on the first run of this test,
// with Wood at 60/60 and Oil at 4. That is not a bug in either material; it is
// the last piece of dice in this path being measured by accident.
static int steps_to_ignite(ElementType target, int limit) {
    Grid g = make_sealed_pair(target, ElementType::Fire);
    for (int i = 1; i <= limit; ++i) {
        g.set_element(2, 1, ElementType::Fire);
        g.update();
        if (g.get_element(1, 1).type != target) return i;
    }
    return limit;
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

    // --- a U-tube equalizes (E1) ---
    //
    // The one scene that could not work before liquids were allowed to rise:
    // the two arms are joined only at the bottom, so the short arm can only gain
    // a cell by pushing one up against gravity. Before E1 the right arm stayed
    // empty forever no matter how long it ran.
    //
    // Paired with the conservation count below, which is the check that actually
    // matters here: the obvious way to make water level is to invent some, and a
    // rule that equalizes by creating cells passes the level test and fails the
    // engine. Both, or neither counts.
    {
        Grid g(40, 40);

        // Two 1-cell-wide arms at x=10 and x=20, joined by a tunnel along y=38.
        //
        // Every wall cell has to belong to one connected piece that reaches the
        // world's bottom row, which is why there is a lid: the divider between
        // the arms sits over the tunnel, so on its own it is a slab hanging in
        // mid-air, and the first swap of water underneath it queues the support
        // check that drops it a cell onto the floor. That is the collapse rule
        // working exactly as specified - the fixture was wrong, not the engine -
        // but a container that rearranges itself mid-test proves nothing about
        // water.
        for (int x = 9; x <= 21; ++x) { g.set_element(x, 18, ElementType::Wall);   // lid
                                        g.set_element(x, 39, ElementType::Wall); } // floor
        for (int y = 18; y <= 39; ++y) { g.set_element(9, y, ElementType::Wall);
                                         g.set_element(21, y, ElementType::Wall); }
        for (int y = 19; y <= 37; ++y)
            for (int x = 11; x <= 19; ++x) g.set_element(x, y, ElementType::Wall); // divider

        // All the water on the left: the tunnel plus fourteen cells of column.
        for (int x = 10; x <= 20; ++x) g.set_element(x, 38, ElementType::Water);
        for (int y = 24; y <= 37; ++y) g.set_element(10, y, ElementType::Water);
        const int placed = count_of(g, ElementType::Water);

        step(g, 200);

        // Topmost water in each arm. 40 means the arm is empty, which is what
        // this test failed with before the rule existed.
        const auto surface = [&](int x) {
            for (int y = 0; y < 40; ++y) if (g.get_element(x, y).type == ElementType::Water) return y;
            return 40;
        };
        const int left = surface(10), right = surface(20);

        check("a U-tube equalizes", left < 40 && right < 40 && std::abs(left - right) <= 1,
              "left surface row=" + std::to_string(left) + " right=" + std::to_string(right));
        check("a U-tube conserves water", count_of(g, ElementType::Water) == placed,
              "placed=" + std::to_string(placed) +
              " after=" + std::to_string(count_of(g, ElementType::Water)));
        // Level is only half of it: a rule that equalizes and then keeps
        // trading cells back and forth across the join is level on average and
        // costs full price forever. MIN_PRESSURE_HEAD is what this checks.
        check("a U-tube stops once it is level", g.active_chunk_count() == 0,
              "awake=" + std::to_string(g.active_chunk_count()));
    }

    // --- a level pool stays put, and stays asleep ---
    //
    // The negative case, and the one that stops the rule above from being a
    // machine for jitter: every surface cell of a settled pool asks the pressure
    // question every step it is awake, and has to keep answering no. If it ever
    // says yes, the pool never sleeps - which is both a visible shimmer and a
    // chunk that costs full price forever.
    {
        // Full width and a whole number of rows, so the pool is already level
        // and already at rest. The partial-top-row case is the test immediately
        // below, and it used to be excluded here rather than tested.
        Grid g(40, 40);
        for (int y = 30; y < 40; ++y)
            for (int x = 0; x < 40; ++x) g.set_element(x, y, ElementType::Water);
        step(g, 200);
        const int before = count_of(g, ElementType::Water);
        const int top = [&] {
            for (int y = 0; y < 40; ++y)
                for (int x = 0; x < 40; ++x)
                    if (g.get_element(x, y).type == ElementType::Water) return y;
            return 40;
        }();
        step(g, 100);
        int top_after = 40;
        for (int y = 0; y < 40 && top_after == 40; ++y)
            for (int x = 0; x < 40; ++x)
                if (g.get_element(x, y).type == ElementType::Water) { top_after = y; break; }

        check("a level pool does not climb", top_after == top,
              "top=" + std::to_string(top) + " after=" + std::to_string(top_after));
        check("a level pool still conserves water", count_of(g, ElementType::Water) == before);
        check("a level pool goes back to sleep", g.active_chunk_count() == 0,
              "awake=" + std::to_string(g.active_chunk_count()));
    }

    // --- a pool with a PARTIAL top row also settles, and also sleeps ---
    //
    // The case the test above used to dodge, and it was not a corner: a body of
    // water only had a whole number of full rows if its cell count happened to
    // divide by its container's width, so almost every real puddle landed here.
    // Those leftover cells slid back and forth across their own flat surface
    // forever - a tank filled to an exact multiple slept, one cell more and it
    // never did, with two chunks awake and about thirty cells changing places
    // every step on water that had visibly finished moving.
    //
    // The rule that fixed it is in step_fluid: a lateral move has to land
    // somewhere it can rest or descend from, so a cell perched on more of its
    // own liquid with nowhere to go stays put. What that rule costs is checked
    // here too, because the fix has an obvious wrong version - refuse those
    // moves outright and a poured column settles into a permanent heap, since
    // the same sideways walk was also how cells got off the top of a mound.
    // Hence the flatness assertion below, which the wrong version fails at a
    // spread of 5 while still passing every sleep check.
    {
        Grid g(40, 40);
        for (int y = 30; y < 40; ++y)
            for (int x = 0; x < 40; ++x) g.set_element(x, y, ElementType::Water);
        // The 17 cells that make this untidy.
        for (int x = 0; x < 17; ++x) g.set_element(x, 29, ElementType::Water);
        const int before = count_of(g, ElementType::Water);

        step(g, 1500);

        check("a pool with a partial top row conserves water",
              count_of(g, ElementType::Water) == before,
              "before=" + std::to_string(before) +
              " after=" + std::to_string(count_of(g, ElementType::Water)));

        // Level to within one cell: every column is the same depth give or take
        // the single leftover row. Exactly the tolerance MIN_PRESSURE_HEAD
        // already documents - what is new is that the surface is *still* at that
        // tolerance rather than merely level on average.
        int min_depth = 41, max_depth = 0;
        for (int x = 0; x < 40; ++x) {
            int d = 0;
            for (int y = 0; y < 40; ++y) if (g.get_element(x, y).type == ElementType::Water) d++;
            min_depth = d < min_depth ? d : min_depth;
            max_depth = d > max_depth ? d : max_depth;
        }
        check("a partial top row does not leave a permanent heap", max_depth - min_depth <= 1,
              "min=" + std::to_string(min_depth) + " max=" + std::to_string(max_depth));

        // The one that was failing. Asserted after a further run so it is
        // "asleep and staying asleep" rather than "asleep for one step".
        check("a pool with a partial top row goes to sleep", g.active_chunk_count() == 0,
              "awake=" + std::to_string(g.active_chunk_count()));

        const int settled = count_of(g, ElementType::Water);
        step(g, 200);
        check("and nothing moves once it is asleep",
              g.active_chunk_count() == 0 && count_of(g, ElementType::Water) == settled);
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
        // 45 steps, not 300, and not the 100 this used to say. Steam spawns hot
        // and cools, so once it has condensed back to water there is no steam
        // left to measure the height of - which is E2 working, not this test
        // failing, and the condensing behaviour gets its own check below.
        //
        // The window is ~60 steps now rather than ~140, because Steam's spawn
        // temperature had to come down below the coldest ignition point in
        // REACTIONS (see its row in material.h) and its life is exactly that
        // span. 45 is comfortably inside it and comfortably past the 35 steps
        // the puff needs to climb from row 35 to the ceiling at one cell a step.
        step(g, 45);
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

    // --- reactions: fire ignites adjacent wood and oil, by heat ---
    // Both now catch every time rather than most of the time, and oil still
    // catches sooner than wood - which used to be a difference of odds (40%
    // against 12%) and is now a difference of ignition point (90 against 120).
    // The upper bound matters as much as the fact of ignition: it is what says
    // the flame front actually advances rather than eventually getting there.
    {
        const int wood = steps_to_ignite(ElementType::Wood, 60);
        const int oil = steps_to_ignite(ElementType::Oil, 60);
        check("fire ignites adjacent wood", wood < 60, "steps=" + std::to_string(wood));
        check("fire ignites adjacent oil", oil < 60, "steps=" + std::to_string(oil));
        check("oil ignites sooner than wood", oil < wood,
              "oil=" + std::to_string(oil) + " wood=" + std::to_string(wood));
    }

    // --- reactions: water extinguishes fire into steam ---
    {
        // Asserted as "it passed through Steam" rather than "it is Steam after
        // N steps", and E2 is why the distinction now matters. A puff of steam
        // pinned against cold stone and cold water dumps its heat into both in
        // a handful of steps and condenses, so there is no fixed N at which the
        // old form of this check is reliable - it read Steam at 30 steps before
        // heat existed and Water at 5 steps after. Watching the transition is
        // the thing this test was ever actually about.
        Grid g = make_sealed_pair(ElementType::Fire, ElementType::Water);
        bool steamed = false;
        for (int i = 0; i < 30 && !steamed; ++i) {
            step(g, 1);
            steamed = g.get_element(1, 1).type == ElementType::Steam;
        }
        check("water extinguishes fire into steam", steamed,
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

    // --- A3: wood smoulders for seconds; flame is a moment ---
    //
    // **The two numbers this asserts are on different cells, which is the whole
    // of E9's rebuild.** The first version of these tests measured one number on
    // one cell, because wood *became* the flame; reference footage says the fuel
    // burns for seconds while the flame it throws is replaced entirely within a
    // dozen steps. Each is checked in isolation below, with the other sealed
    // away.
    {
        // Charred's lifetime is a decay chance, so a single cell is one draw
        // from a geometric distribution and could legitimately be anything.
        // Sampled across many independent cells instead, and the bar is set on
        // the mean with room either side - this must not break every time the
        // row is retuned, only when the row stops being connected to anything.
        //
        // Each cell is sealed in Wall on all eight sides, which does double duty:
        // it isolates decay from emission (no empty neighbour, so no flame is
        // ever thrown) and it stops the cells igniting each other.
        //
        // **The world is tiled exactly, with no spare row anywhere.** Sized with
        // a margin, the Wall lattice is an unsupported structure: it falls, and
        // a scan of fixed coordinates then reads cells the world has moved out
        // from under it. That reads as "the cells vanished instantly" - the
        // first run of this test reported a 2-step mean lifetime - while the
        // census showed every cell still present. Third time a floating scene
        // has answered a question that was not asked; see the note on measuring
        // A4 in PLAYTEST_LOG.md.
        const int COLS = 12, ROWS = 12, N = COLS * ROWS;
        Grid g(COLS * 3, ROWS * 3);
        for (int j = 0; j < ROWS; ++j)
            for (int i = 0; i < COLS; ++i) {
                const int cx = i * 3 + 1, cy = j * 3 + 1;
                for (int dy = -1; dy <= 1; ++dy)
                    for (int dx = -1; dx <= 1; ++dx)
                        if (dx != 0 || dy != 0) g.set_element(cx + dx, cy + dy, ElementType::Wall);
                g.set_element(cx, cy, ElementType::Charred);
            }

        long long total = 0;
        int died = 0;
        for (int s = 1; s <= 3000 && died < N; ++s) {
            g.update();
            for (int j = 0; j < ROWS; ++j)
                for (int i = 0; i < COLS; ++i) {
                    const int cx = i * 3 + 1, cy = j * 3 + 1;
                    if (g.get_element(cx, cy).type != ElementType::Empty) continue;
                    // Count each cell once, by sealing it the moment it dies.
                    g.set_element(cx, cy, ElementType::Wall);
                    total += s;
                    ++died;
                }
        }
        const int mean = died ? static_cast<int>(total / died) : 0;
        check("A3: every charred cell eventually burns out", died == N,
              "died=" + std::to_string(died) + "/" + std::to_string(N));
        // Charred's row is 34 per myriad, a mean of ~294 steps closed-form and
        // 345 measured here - higher than the closed form because a sealed cell
        // reaches equilibrium with its wall, sleeps, and misses rolls it would
        // have lost. The bar is wide because this is a sampled mean, and it is a
        // bar rather than an equality because the point is that the row is
        // connected, not what the number is.
        //
        // **The top of it was 320 and session 3 walked straight through it**,
        // retuning 46 to 34. That is the bar doing the wrong job: an upper bound
        // one retune above the current value is an equality check wearing a
        // range's clothes, and it fails for the ordinary reason that somebody
        // tuned the thing it is watching. Nine hundred is fifteen seconds, which
        // is long enough to be a genuine statement - wood that smoulders for a
        // quarter of a minute is a defect - and far enough away that a taste
        // change does not have to come here first.
        check("A3: wood smoulders for seconds, not a fraction of one", mean > 90 && mean < 900,
              "mean lifetime=" + std::to_string(mean) + " steps");

        // Emission is what would break the isolation above, so assert the
        // isolation held rather than trusting it. This is also finding 6 from
        // the reference footage: a buried cell does not visibly burn.
        check("A3: a fully buried charred cell throws no flame", count_of(g, ElementType::Fire) == 0,
              "fire=" + std::to_string(count_of(g, ElementType::Fire)));
    }

    // --- A3: a flame is a moment, and its lifetime is its own ---
    // The one lifetime in the engine that is a countdown rather than a roll,
    // because the colour ramp has to read the flame's age.
    {
        Grid g(4, 3);
        for (int y = 0; y < 3; ++y)
            for (int x = 0; x < 4; ++x) g.set_element(x, y, ElementType::Wall);
        g.set_element(1, 1, ElementType::Fire);
        int burned = 0;
        for (int i = 1; i <= 200 && burned == 0; ++i) {
            g.update();
            if (g.get_element(1, 1).type != ElementType::Fire) burned = i;
        }
        check("A3: a flame lives about a dozen steps", burned > 5 && burned < 22,
              "steps=" + std::to_string(burned));
    }

    // --- session 2: flames do not all live the same length of time ---
    //
    // **The fix for two playtest notes that were one defect** - "a hard height
    // cutoff that looks unnatural" and "the uniform height of the flames does not
    // look natural". A flame rose one cell per step and lived exactly twelve
    // steps, so every flame in the world died exactly twelve cells above its
    // fuel: a straight horizontal line across the top of a fire.
    //
    // Asserted on the *spread* rather than on any one lifetime, because a single
    // sample cannot tell a jittered lifetime from a fixed one - which is the
    // trivially-passing shape session 1 got caught by.
    {
        int shortest = 999, longest = -1;
        for (int trial = 0; trial < 60; ++trial) {
            Grid g(4, 3, 500 + trial);
            for (int y = 0; y < 3; ++y)
                for (int x = 0; x < 4; ++x) g.set_element(x, y, ElementType::Wall);
            // Placed by the engine's own emission path, not by the brush: a
            // brush-placed flame is given the mean life on purpose, so seeding
            // this with set_element would measure the one case that is still
            // uniform and report no spread at all.
            g.set_element(1, 1, ElementType::Charred);
            g.set_element(2, 1, ElementType::Empty);

            // **The source is walled off the moment it lights, and without that
            // this measures the wrong thing.** Charred re-emits into the only
            // empty cell it has, and a flame can die and be replaced within a
            // single sweep - so an observer watching one cell sees unbroken Fire
            // across two flames and reports one life of thirty steps. The first
            // version of this test did exactly that and read 6..30 for a quantity
            // bounded at 8..18, which looked like a bug in the engine and was a
            // bug in the measurement.
            int life = -1;
            for (int i = 1; i <= 400; ++i) {
                g.update();
                if (g.get_element(2, 1).type != ElementType::Fire) continue;

                g.set_element(1, 1, ElementType::Wall); // no second flame possible
                life = 0;
                for (int k = 1; k <= 60; ++k) {
                    g.update();
                    if (g.get_element(2, 1).type != ElementType::Fire) { life = k; break; }
                }
                break;
            }
            if (life > 0) {
                if (life < shortest) shortest = life;
                if (life > longest) longest = life;
            }
        }
        check("session 2: flame lifetimes vary between flames", longest > shortest,
              "shortest=" + std::to_string(shortest) + " longest=" + std::to_string(longest));
        // Bounds written as literals rather than read from Grid, because
        // FLAME_LIFETIME_MEAN and _SPREAD are private and widening the engine's
        // public surface for a test is the more expensive of the two options.
        // 8-18 is mean 13 +/- spread 5, plus a step of slack either side for the
        // sampling. If those constants move, this is meant to fail.
        check("session 2: flame lifetimes stay inside their declared bounds",
              shortest >= 7 && longest <= 19,
              "shortest=" + std::to_string(shortest) + " longest=" + std::to_string(longest));
    }

    // --- session 2: a flame rises slower than one cell per step ---
    //
    // "The flames move about 10 percent too fast." A gas moves a whole cell or
    // none, so 0.9 cells per step is a skipped step rather than a smaller one.
    // Averaged over many flames because one flame lives ~13 steps and so resolves
    // the rate no finer than 1/13 - far too coarse to tell 0.9 from 1.0, which is
    // the entire quantity under test.
    {
        int moves = 0, steps = 0;
        for (int trial = 0; trial < 200; ++trial) {
            Grid g(40, 60, 1000 + trial);
            g.set_element(20, 50, ElementType::Fire);
            int prev = 50;
            for (int i = 0; i < 40; ++i) {
                g.update();
                int found = -1;
                for (int y = 0; y < 60 && found < 0; ++y)
                    for (int x = 0; x < 40; ++x)
                        if (g.get_element(x, y).type == ElementType::Fire) { found = y; break; }
                if (found < 0) break;
                ++steps;
                if (found != prev) ++moves;
                prev = found;
            }
        }
        const int pct = steps ? moves * 100 / steps : 0;
        check("session 2: a flame rises on about 9 steps in 10", pct >= 85 && pct <= 95,
              "moved on " + std::to_string(pct) + "% of steps");
        // The control. Without this the test above is passed by a flame that
        // never moves at all, and by one that has stopped being emitted.
        check("session 2: flames are still rising", moves > 0 && steps > 500,
              "moves=" + std::to_string(moves) + " steps=" + std::to_string(steps));
    }

    // --- session 2: burnt wood is charcoal, not a hole in the backdrop ---
    //
    // A near-black cell over V1's dark blue backdrop reads as absence rather than
    // as material. Guarded as a floor on the darkest channel rather than as an
    // exact colour, so the palette can still be tuned without editing a test.
    {
        const uint32_t c = material_of(ElementType::Charred).color;
        const int r = (c >> 16) & 0xFF, gg = (c >> 8) & 0xFF, b = c & 0xFF;
        const int darkest = std::min(r, std::min(gg, b));
        check("session 2: charred reads as charcoal rather than jet black", darkest >= 40,
              "darkest channel=" + std::to_string(darkest));
        // And still clearly darker than the flame it throws, or the fuel and the
        // fire become one shape again - the confusion Charred exists to resolve.
        check("session 2: charred stays far darker than flame",
              r < ((material_of(ElementType::Fire).color >> 16) & 0xFF) / 2,
              "charred r=" + std::to_string(r));
    }

    // --- E9: the fuel is not the flame ---
    //
    // The single check that would have caught the first build of this. Wood that
    // catches must still be *there* - solid, in its own cell, still holding up
    // whatever it was holding up - with flame in the air beside it, rather than
    // having been replaced by the flame.
    {
        Grid g(40, 40);
        for (int i = 0; i < 12; ++i) g.set_element(10 + i, 39, ElementType::Wood);
        for (int i = 0; i < 40; ++i) { g.set_element(10, 38, ElementType::Fire); g.update(); }

        int charred = 0, fire_in_air = 0;
        for (int y = 0; y < 40; ++y)
            for (int x = 0; x < 40; ++x) {
                if (g.get_element(x, y).type == ElementType::Charred) ++charred;
                if (g.get_element(x, y).type == ElementType::Fire && y < 39) ++fire_in_air;
            }
        check("E9: burning wood is still a solid cell of its own", charred > 0,
              "charred=" + std::to_string(charred));
        check("E9: burning wood is still structural", is_structural(ElementType::Charred));
        check("E9: flame appears in the air, not only where the fuel was", fire_in_air > 0,
              "flame cells=" + std::to_string(fire_in_air));
    }

    // --- A4: fire propagates along a beam and consumes it ---
    //
    // **Measured as cells gone, not as cells changed.** The earlier form of this
    // counted anything that was no longer Wood, which `Charred` satisfies the
    // instant it catches - so after the rebuild it passed without the fuel ever
    // being consumed, and would have gone on passing if propagation had broken
    // entirely. A metric that a refactor can satisfy trivially is not a test.
    {
        const auto burnt_run = [](bool horizontal, int steps) {
            // **Both beams rest on the floor, and that is load-bearing.** The
            // first version of this hung them in mid-air, where burning the end
            // off one leaves the remainder unsupported: the collapse system then
            // drops the unburnt wood out from under the flame and propagation
            // stops for a reason that has nothing to do with A4. It stalled at
            // 11 of 20 cells through 4800 steps, which reads exactly like a
            // propagation bug.
            Grid g(40, 40);
            const int FLOOR = 39;
            for (int i = 0; i < 20; ++i) {
                if (horizontal) g.set_element(10 + i, FLOOR, ElementType::Wood);
                else            g.set_element(20, FLOOR - i, ElementType::Wood);
            }
            // A match held to one end for 30 steps and then taken away. It has
            // to be *held*: a lone flame is a free gas with a twelve-step life,
            // so a single placed cell rises off the beam and dies long before
            // the wood is anywhere near its threshold.
            const int mx = horizontal ? 10 : 20;
            const int my = horizontal ? FLOOR : FLOOR - 19;
            for (int i = 0; i < 30; ++i) {
                g.set_element(mx, my - 1, ElementType::Fire); // beside the end, not on it
                g.update();
            }
            step(g, steps);

            int consumed = 0;
            for (int i = 0; i < 20; ++i) {
                const ElementType t = horizontal ? g.get_element(10 + i, FLOOR).type
                                                 : g.get_element(20, FLOOR - i).type;
                if (t == ElementType::Empty) ++consumed;
            }
            return consumed;
        };
        const int h = burnt_run(true, 2000);
        const int v = burnt_run(false, 2000);
        check("A4: fire consumes a horizontal beam", h >= 15, "consumed=" + std::to_string(h));
        check("A4: fire consumes a vertical beam", v >= 15, "consumed=" + std::to_string(v));
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

    // --- heat: the byte is free, and a fresh world is cold ---
    // The size is asserted at compile time in element.h too. It is repeated here
    // because a static_assert that quietly stops being tight is invisible, and
    // because 12 is the number the whole argument for spending a seventh axis
    // rested on: temperature had to land in padding the struct already carried.
    {
        check("temperature costs no memory", sizeof(Element) == 12,
              "sizeof(Element)=" + std::to_string(sizeof(Element)));

        Grid g(20, 20);
        bool all_ambient = true;
        for (int y = 0; y < 20; ++y)
            for (int x = 0; x < 20; ++x)
                if (g.get_element(x, y).temperature != AMBIENT_TEMPERATURE) all_ambient = false;
        check("a fresh world starts at ambient", all_ambient);
    }

    // --- heat: conduction carries heat away from a flame and runs out ---
    // The flame is re-placed every step so it is a source rather than a thing
    // with a lifetime; this is the same trick bench_grid.cpp uses to keep its
    // `burning` scenario burning, and without it the test would be measuring
    // Fire's 6% burnout instead of conduction.
    //
    // A Wall bar, not Wood: Wall has no ignition row, so what is measured is
    // heat moving and nothing else. That heat *stops* is as much the point as
    // that it moves - the bleed back to ambient is the only thing removing heat
    // from the world, and without it a single candle eventually cooks the map.
    {
        // On the world's bottom row, not floating in the middle of it. Wall is
        // structural, and a bar with nothing under it is an unsupported piece
        // that collapses one row on the first disturbance - the same fixture
        // mistake E1's U-tube made twice, and it would have left this test
        // measuring the temperature of the empty cell the bar used to be in.
        Grid g(40, 10);
        for (int x = 0; x < 40; ++x) g.set_element(x, 9, ElementType::Wall);
        for (int i = 0; i < 300; ++i) {
            g.set_element(1, 8, ElementType::Fire);
            g.update();
        }
        const int near = g.get_element(2, 9).temperature;
        const int far = g.get_element(30, 9).temperature;
        check("heat conducts out of a flame into what it touches", near > AMBIENT_TEMPERATURE + 5,
              "near=" + std::to_string(near));
        check("heat falls off with distance", near > far,
              "near=" + std::to_string(near) + " far=" + std::to_string(far));
        check("heat does not reach the far end of the bar", far <= AMBIENT_TEMPERATURE + 1,
              "far=" + std::to_string(far));
    }

    // --- heat: a flame burns *through* a beam ---
    // The observation E2 exists to answer, asserted directly: the near end of a
    // wooden beam is consumed and the far end is untouched, so there is a front
    // that advances rather than a beam that lights up all over at once. The
    // negative half is the one that would catch a runaway conduction constant.
    {
        Grid g(60, 10);
        for (int x = 0; x < 60; ++x) g.set_element(x, 9, ElementType::Wood);
        for (int i = 0; i < 200; ++i) {
            g.set_element(1, 8, ElementType::Fire);
            g.update();
        }
        check("a beam burns away at the end the flame is on",
              g.get_element(2, 9).type != ElementType::Wood,
              "type=" + std::string(material_of(g.get_element(2, 9).type).name));
        check("the far end of the beam is untouched",
              g.get_element(55, 9).type == ElementType::Wood &&
              g.get_element(55, 9).temperature <= AMBIENT_TEMPERATURE + 1,
              "type=" + std::string(material_of(g.get_element(55, 9).type).name) +
              " temp=" + std::to_string(g.get_element(55, 9).temperature));
    }

    // --- heat: water boils ---
    // Water has no contact reaction with Fire that produces Steam *from the
    // water's side* - the dousing row transforms the Fire cell, not this one -
    // so the only route from Water to Steam here is the temperature-gated boil
    // row. Sealed so neither cell can move away from the other.
    {
        Grid g(5, 3);
        for (int y = 0; y < 3; ++y)
            for (int x = 0; x < 5; ++x) g.set_element(x, y, ElementType::Wall);
        g.set_element(2, 1, ElementType::Water);
        int boiled = 0;
        for (int i = 1; i <= 60; ++i) {
            g.set_element(1, 1, ElementType::Fire);
            g.update();
            if (boiled == 0 && g.get_element(2, 1).type == ElementType::Steam) boiled = i;
        }
        check("water heated by a flame boils into steam", boiled > 0 && boiled < 60,
              "step=" + std::to_string(boiled));
    }

    // --- heat: steam condenses as it cools ---
    // The other end of the same cycle, and the reason Steam spawns hot rather
    // than at ambient: steam that is not above condensing point is water, so a
    // puff created at room temperature would turn back on the step it was made.
    //
    // In open air rather than sealed in a Wall box, which is the fixture the
    // first version of this test used and the wrong one. Empty has conductivity
    // zero, so steam surrounded by air cools only by the slow bleed to ambient
    // and lives a couple of hundred steps; steam packed against stone dumps its
    // heat into the stone and is gone in ten. Both are correct, and the one
    // worth asserting on is the one a player ever sees.
    //
    // Probed at 30 steps rather than 60. Steam's life is exactly the span
    // between its spawn temperature and its condensing point, and both moved
    // when steam stopped being hot enough to set fire to things (see Steam's
    // row in material.h): ~60 steps now rather than ~140. A 60-step probe would
    // still pass, two degrees clear of condensing, but it would be asserting
    // "steam has not *quite* gone yet" while reading as "steam is not
    // instantaneous", and the next tweak to either number would break it for a
    // reason nobody could see from here.
    {
        Grid g(20, 40);
        for (int x = 8; x < 12; ++x) g.set_element(x, 35, ElementType::Steam);
        step(g, 30);
        check("steam does not condense the moment it is made",
              count_of(g, ElementType::Steam) == 4,
              "steam=" + std::to_string(count_of(g, ElementType::Steam)));
        step(g, 370);
        check("steam condenses once it has cooled",
              count_of(g, ElementType::Steam) == 0, // and matter is conserved across the change
              "steam=" + std::to_string(count_of(g, ElementType::Steam)) +
              " water=" + std::to_string(count_of(g, ElementType::Water)));
        check("condensing steam conserves matter", count_of(g, ElementType::Water) == 4,
              "water=" + std::to_string(count_of(g, ElementType::Water)));
    }

    // --- heat: steam is not a fire-starter ---
    //
    // Steam used to spawn at 220, which is 100 degrees over Wood's ignition
    // point and 130 over Oil's, so it lit them on contact with no flame in the
    // world at all. Both routes into steam produced it - boiling, and water
    // dousing a flame - which made putting a fire out a way of spreading it.
    //
    // **Confined on purpose.** In open air steam rises away and cools before it
    // does any damage, which is why every existing steam test missed this and
    // why the fixture matters more than the assertion: sealed under a wooden
    // ceiling is the shape authored terrain produces and an open grid never
    // does. The pocket below burned 17 of its 20 wood cells before the fix.
    {
        Grid g(40, 40);
        for (int x = 10; x < 30; ++x) g.set_element(x, 20, ElementType::Wood);  // ceiling
        for (int x = 10; x < 30; ++x) g.set_element(x, 24, ElementType::Wall);  // floor
        for (int y = 21; y < 24; ++y) {
            g.set_element(10, y, ElementType::Wall);
            g.set_element(29, y, ElementType::Wall);
        }
        for (int x = 11; x < 29; ++x)
            for (int y = 21; y < 24; ++y) g.set_element(x, y, ElementType::Steam);

        const int wood_before = count_of(g, ElementType::Wood);
        step(g, 400);

        check("steam does not ignite the wood it is sealed against",
              count_of(g, ElementType::Wood) == wood_before,
              "wood " + std::to_string(wood_before) + " -> " +
              std::to_string(count_of(g, ElementType::Wood)) +
              ", fire=" + std::to_string(count_of(g, ElementType::Fire)));
        check("and no fire appeared from nowhere", count_of(g, ElementType::Fire) == 0);
    }

    // --- heat: dousing a fire does not start a bigger one ---
    //
    // The gameplay half of the same bug, and the reason it was worth fixing
    // rather than documenting: notes/art_pipeline.txt schedules a scene built
    // around exactly this move ("sleepers beside the water -> ignite wood,
    // watch water douse it to steam").
    {
        Grid g(40, 40);
        for (int x = 12; x < 28; ++x) g.set_element(x, 30, ElementType::Wood);   // wooden floor
        for (int y = 24; y < 30; ++y) {                                          // walls to trap the steam
            g.set_element(12, y, ElementType::Wall);
            g.set_element(27, y, ElementType::Wall);
        }
        for (int x = 13; x < 27; ++x) g.set_element(x, 29, ElementType::Fire);
        for (int x = 13; x < 27; ++x)
            for (int y = 26; y < 29; ++y) g.set_element(x, y, ElementType::Water);

        const int wood_before = count_of(g, ElementType::Wood);
        step(g, 500);

        // The fire may take some of the floor before the water reaches it -
        // what must not happen is the steam carrying on burning after the
        // flames are out.
        check("dousing a fire puts it out", count_of(g, ElementType::Fire) == 0,
              "fire=" + std::to_string(count_of(g, ElementType::Fire)));

        const int wood_after_dousing = count_of(g, ElementType::Wood);
        step(g, 400);
        check("and the steam left behind does not keep burning the floor",
              count_of(g, ElementType::Wood) == wood_after_dousing,
              "wood " + std::to_string(wood_before) + " -> " +
              std::to_string(wood_after_dousing) + " -> " +
              std::to_string(count_of(g, ElementType::Wood)));
    }

    // --- heat: a burnt-out world cools back to ambient and sleeps ---
    // The check the whole axis stands or falls on. Heat that never settles is
    // heat that keeps a chunk awake forever, which would hand back the entire
    // saving the chunk system exists for - and it would do it silently, since a
    // world that is merely warm looks identical to one that is not.
    //
    // Ambient+1, not ambient: the dead band in heat_flow stops an exchange once
    // two temperatures are within one of each other, which is precisely what
    // makes this terminate at all. "Level to within a cell" was the same trade
    // in E1, for the same reason.
    {
        Grid g(20, 20);
        g.set_element(10, 10, ElementType::Fire);
        step(g, 600);
        int hottest = 0;
        for (int y = 0; y < 20; ++y)
            for (int x = 0; x < 20; ++x)
                hottest = std::max(hottest, static_cast<int>(g.get_element(x, y).temperature));
        check("a burnt-out world cools back to ambient", hottest <= AMBIENT_TEMPERATURE + 1,
              "hottest=" + std::to_string(hottest));
        check("a burnt-out world goes back to sleep", g.active_chunk_count() == 0,
              "awake=" + std::to_string(g.active_chunk_count()));
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

    // --- a pouring stream must not throw cells sideways through open air ---
    //
    // Regression test for PLAYTEST_LOG.md session 1, defect A7. `spread` is a
    // distance a fluid may cover *along a surface*; it was being spent crossing
    // empty space too, because "I could fall from there" satisfied the
    // can-I-stop-here test and every point in mid-air satisfies that. A cell
    // inside a falling stream cannot descend - its own kind is below it - so it
    // reached the lateral scan, found five cells of nothing beside it, and
    // relocated to the far end of them.
    //
    // Measured as the longest horizontal run of liquid with nothing underneath
    // it. One cell of overhang is legitimate and is what flowing off a lip looks
    // like, and two adjacent columns may each take that step in the same frame;
    // beyond that the fluid is crossing air it should not cross. Was 4 on this
    // scene before the fix.
    {
        Grid g(120, 90, 12345);
        for (int x = 0; x < 120; ++x)
            for (int y = 80; y < 90; ++y) g.set_element(x, y, ElementType::Wall);

        int worst = 0;
        for (int s = 0; s < 400; ++s) {
            if (s < 200)
                for (int x = 58; x < 62; ++x) g.set_element(x, 5, ElementType::Water);
            g.update();

            for (int y = 7; y < 89; ++y) { // from below the source row down
                int run = 0;
                for (int x = 0; x < 120; ++x) {
                    const bool hanging = g.get_element(x, y).type == ElementType::Water &&
                                         g.get_element(x, y + 1).type == ElementType::Empty &&
                                         g.get_element(x, y - 1).type != ElementType::Water;
                    run = hanging ? run + 1 : 0;
                    if (run > worst) worst = run;
                }
            }
        }

        check("a pouring stream throws no lateral spikes", worst <= 2,
              "longest mid-air overhang was " + std::to_string(worst) + " cells");
    }

    // --- a pouring powder must not fringe itself with horizontal shelves ---
    //
    // Regression test for PLAYTEST_LOG.md session 1, defect A7 (powder half),
    // which the screenshots in resources/video_screenshots/ are of. The liquid
    // fix above does not cover this: Sand is a Powder with `spread` 0 and never
    // reaches the lateral scan at all.
    //
    // `swap_elements` tags only the two cells it touches, so an entire row could
    // cascade diagonally within one sweep - grain at x to (x+1, y+1), grain at
    // x+1 to (x+2, y+1), all the way along - landing a one-cell-thick shelf nine
    // cells proud of the pile with nothing beneath it. Measured as the longest
    // horizontal run of powder with Empty below and no powder above, at any
    // point during the pour rather than after it settles: the defect is a
    // transient that regenerates every step, so a settled world shows nothing.
    // Was 9 before the fix.
    {
        Grid g(200, 150, 999);
        for (int x = 0; x < 200; ++x)
            for (int y = 140; y < 150; ++y) g.set_element(x, y, ElementType::Wall);

        int worst = 0;
        for (int s = 0; s < 600; ++s) {
            if (s < 400) {
                const int cx = 100 + (s / 40) % 5 - 2;
                for (int dy = -6; dy <= 6; ++dy)
                    for (int dx = -6; dx <= 6; ++dx)
                        if (dx * dx + dy * dy <= 36)
                            g.set_element(cx + dx, 20 + dy, ElementType::Sand);
            }
            g.update();

            for (int y = 1; y < 149; ++y) {
                int run = 0;
                for (int x = 0; x < 200; ++x) {
                    const bool shelf = g.get_element(x, y).type == ElementType::Sand &&
                                       g.get_element(x, y + 1).type == ElementType::Empty &&
                                       g.get_element(x, y - 1).type != ElementType::Sand;
                    run = shelf ? run + 1 : 0;
                    if (run > worst) worst = run;
                }
            }
        }

        check("a pouring powder throws no horizontal shelves", worst <= 5,
              "longest mid-air shelf was " + std::to_string(worst) + " cells");

        // **The other half of the same defect, and it is here because a fix for
        // the shelves caused it.** Refusing the diagonal roll unless its
        // destination was already supported removed the shelves and stopped
        // settled grains relaxing down a face, so piles grew straight up and
        // held vertical columns. Shelves and columns are opposite failures of
        // one rule, so a test for either alone can be passed by breaking the
        // other - which is exactly what happened, twice.
        int quiet = 0;
        for (int s = 0; s < 4000 && quiet < 30; ++s) {
            g.update();
            quiet = g.active_chunk_count() == 0 ? quiet + 1 : 0;
        }

        int surface[200];
        for (int x = 0; x < 200; ++x) {
            surface[x] = 140;
            for (int y = 0; y < 140; ++y)
                if (g.get_element(x, y).type == ElementType::Sand) { surface[x] = y; break; }
        }
        int steepest = 0;
        for (int x = 1; x < 200; ++x) {
            if (surface[x] >= 140 || surface[x - 1] >= 140) continue; // off the pile
            const int drop = std::abs(surface[x] - surface[x - 1]);
            if (drop > steepest) steepest = drop;
        }

        check("a settled powder pile has no vertical faces", steepest <= 2,
              "steepest adjacent column drop was " + std::to_string(steepest) + " cells");
    }

    return report();
}
