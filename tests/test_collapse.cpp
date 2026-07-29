// Structural falling tests.
//
// Wall and Wood hold their shape, which means they can hold it somewhere they
// should not: dig the ground out from under a slab and it hangs in mid-air.
// An unsupported piece falls as one rigid body, shape intact, which is what
// makes it read as masonry rather than as gravel.
//
// These cover that, and -- just as importantly -- the cases where nothing must
// move, because a piece that falls when it shouldn't destroys a level.

#include "physics/grid.h"
#include "test_util.h"
#include <string>

namespace {

void fill(Grid& g, int x0, int y0, int x1, int y1, ElementType t) {
    for (int y = y0; y <= y1; ++y)
        for (int x = x0; x <= x1; ++x)
            g.set_element(x, y, t);
}

int count_of(const Grid& g, ElementType t) {
    int n = 0;
    for (int y = 0; y < g.get_height(); ++y)
        for (int x = 0; x < g.get_width(); ++x)
            if (g.get_element(x, y).type == t) n++;
    return n;
}

void step(Grid& g, int n) {
    for (int i = 0; i < n; ++i) g.update();
}

// True if a solid rectangle of `t` sits exactly at (x0,y0)-(x1,y1) and nowhere
// in the ring of cells immediately around it. Asserting the negative space is
// the point: it is what proves the piece kept its shape rather than smearing.
bool rect_exactly_at(const Grid& g, int x0, int y0, int x1, int y1, ElementType t) {
    for (int y = y0 - 1; y <= y1 + 1; ++y) {
        for (int x = x0 - 1; x <= x1 + 1; ++x) {
            const bool inside = (x >= x0 && x <= x1 && y >= y0 && y <= y1);
            if (g.get_element(x, y).type == t) {
                if (!inside) return false;
            } else if (inside) {
                return false;
            }
        }
    }
    return true;
}

// Top row of the topmost cell of `t`, or -1.
int top_row_of(const Grid& g, ElementType t) {
    for (int y = 0; y < g.get_height(); ++y)
        for (int x = 0; x < g.get_width(); ++x)
            if (g.get_element(x, y).type == t) return y;
    return -1;
}

} // namespace

int main() {
    // --- an undisturbed structure is never questioned ---
    // The single most important test here. Support is checked on disturbance,
    // never as a global truth, so a platform placed on purpose stays put.
    // test_grid.cpp depends on this too: it floats a lone Wall in empty space
    // and asserts it never moves.
    {
        Grid g(60, 60);
        fill(g, 20, 20, 40, 24, ElementType::Wall);
        step(g, 200);
        check("an untouched floating slab does not move",
              rect_exactly_at(g, 20, 20, 40, 24, ElementType::Wall));
    }

    // --- digging a hole in a grounded structure does not drop it ---
    {
        Grid g(60, 60);
        fill(g, 0, 50, 59, 59, ElementType::Wall);
        fill(g, 20, 40, 30, 49, ElementType::Wall);

        g.set_element(25, 45, ElementType::Empty);
        step(g, 60);

        check("a grounded structure stays put when dug",
              g.get_element(20, 40).type == ElementType::Wall &&
              g.get_element(30, 49).type == ElementType::Wall &&
              g.get_element(25, 45).type == ElementType::Empty);
    }

    // --- cutting a slab free drops it, shape intact ---
    {
        Grid g(60, 60);
        fill(g, 0, 50, 59, 59, ElementType::Wall);  // floor
        fill(g, 20, 30, 30, 34, ElementType::Wall); // slab, floating
        fill(g, 25, 35, 25, 49, ElementType::Wall); // the one column holding it up

        const int wall_before = count_of(g, ElementType::Wall);
        step(g, 5);
        check("the propped slab is stable while its support stands",
              rect_exactly_at(g, 20, 30, 30, 34, ElementType::Wall) == false ||
              g.get_element(25, 35).type == ElementType::Wall);

        // Cut the prop out from under it.
        fill(g, 25, 35, 25, 49, ElementType::Empty);

        step(g, 1);
        check("one step after being cut free, the slab has moved down exactly one cell",
              rect_exactly_at(g, 20, 31, 30, 35, ElementType::Wall));

        step(g, 1);
        check("it keeps falling on its own without being disturbed again",
              rect_exactly_at(g, 20, 32, 30, 36, ElementType::Wall));

        // Not rect_exactly_at() here: the piece has landed on a Wall floor, so
        // there is legitimately Wall directly under it. Check its own cells, and
        // clear air above and to both sides.
        step(g, 200);
        bool resting = true;
        for (int y = 45; y <= 49; ++y) {
            for (int x = 20; x <= 30; ++x)
                if (g.get_element(x, y).type != ElementType::Wall) resting = false;
            if (g.get_element(19, y).type != ElementType::Empty) resting = false;
            if (g.get_element(31, y).type != ElementType::Empty) resting = false;
        }
        for (int x = 20; x <= 30; ++x)
            if (g.get_element(x, 44).type != ElementType::Empty) resting = false;
        check("it comes to rest as a slab sitting on the floor", resting);

        check("nothing was created or destroyed on the way down",
              count_of(g, ElementType::Wall) == wall_before - 15,
              std::to_string(count_of(g, ElementType::Wall)) + " wall");
    }

    // --- a falling piece keeps a non-rectangular shape ---
    // A rectangle would still look right if each column fell independently.
    // An L has to keep its corner to prove the whole piece moves as one.
    {
        Grid g(60, 60);
        fill(g, 0, 50, 59, 59, ElementType::Wall); // floor
        fill(g, 20, 20, 22, 30, ElementType::Wood); // vertical arm
        fill(g, 23, 28, 30, 30, ElementType::Wood); // foot of the L

        g.set_element(20, 20, ElementType::Empty); // disturb it; nothing is grounded
        step(g, 1);

        const bool arm = g.get_element(21, 21).type == ElementType::Wood &&
                         g.get_element(22, 31).type == ElementType::Wood;
        const bool foot = g.get_element(30, 31).type == ElementType::Wood &&
                          g.get_element(23, 29).type == ElementType::Wood;
        const bool corner_empty = g.get_element(30, 28).type == ElementType::Empty;
        check("an L-shaped piece falls with its corner intact",
              arm && foot && corner_empty);
    }

    // --- Wall and Wood hold each other up ---
    {
        Grid g(60, 60);
        fill(g, 0, 50, 59, 59, ElementType::Wall);
        fill(g, 20, 45, 20, 49, ElementType::Wood); // post standing on the floor
        fill(g, 15, 40, 25, 44, ElementType::Wall); // slab resting on the post

        g.set_element(25, 40, ElementType::Empty);
        step(g, 30);

        check("a stone slab on a wooden post is supported",
              g.get_element(15, 44).type == ElementType::Wall &&
              g.get_element(20, 45).type == ElementType::Wood);
    }

    // --- powder bears load, liquid does not ---
    {
        Grid g(60, 60);
        fill(g, 0, 50, 59, 59, ElementType::Wall);
        fill(g, 20, 45, 30, 49, ElementType::Sand);
        fill(g, 20, 40, 30, 44, ElementType::Wall);
        step(g, 60); // settle the sand first

        g.set_element(30, 40, ElementType::Empty);
        step(g, 5);

        check("a slab resting on packed sand is supported",
              g.get_element(20, 44).type == ElementType::Wall);
    }
    {
        Grid g(60, 60);
        fill(g, 0, 55, 59, 59, ElementType::Wall);
        fill(g, 20, 45, 30, 54, ElementType::Water);
        fill(g, 20, 40, 30, 44, ElementType::Wall);

        g.set_element(30, 40, ElementType::Empty);
        step(g, 40);

        check("a slab is not held up by water, and sinks through it",
              top_row_of(g, ElementType::Wall) > 40,
              "top wall row " + std::to_string(top_row_of(g, ElementType::Wall)));
        check("the water it displaced was not destroyed",
              count_of(g, ElementType::Water) == 11 * 10,
              std::to_string(count_of(g, ElementType::Water)) + " water");
    }

    // --- a slab drops once the sand under it falls away ---
    // The swap_elements trigger rather than the set_element one: nothing is
    // removed and the slab is never touched, the sand simply leaves.
    {
        Grid g(80, 60);
        fill(g, 0, 50, 79, 59, ElementType::Wall);
        fill(g, 33, 50, 37, 59, ElementType::Empty); // a shaft through the floor
        fill(g, 35, 46, 35, 49, ElementType::Sand);  // sand pillar over the shaft
        fill(g, 30, 40, 40, 45, ElementType::Wall);  // slab held up by that alone

        check("the slab is up while the sand holds it",
              g.get_element(30, 45).type == ElementType::Wall);

        step(g, 200);

        check("the slab comes down once the sand under it falls away",
              top_row_of(g, ElementType::Wall) > 40,
              "top wall row " + std::to_string(top_row_of(g, ElementType::Wall)));
    }

    // --- a piece too large to judge is left alone ---
    // Guards the budget's failure direction: over the cap, assume supported.
    // A missed fall is invisible; a wrong one destroys the level.
    {
        const int w = 200, h = 200; // 40,000 cells, far over MAX_SUPPORT_CELLS
        Grid g(w, h);
        fill(g, 0, 0, w - 1, h - 2, ElementType::Wall); // huge, and not grounded
        const int before = count_of(g, ElementType::Wall);

        g.set_element(100, 100, ElementType::Empty);
        step(g, 2);

        check("a piece over the size budget is assumed supported",
              count_of(g, ElementType::Wall) == before - 1 &&
              g.get_element(0, 0).type == ElementType::Wall);
    }

    return report();
}
