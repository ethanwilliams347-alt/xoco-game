// Structural collapse tests.
//
// Wall and Wood hold their shape, which means they can hold it somewhere they
// should not: dig the ground out from under a slab and it hangs in mid-air.
// These cover the support check that fixes that, and -- just as importantly --
// the cases where it must NOT fire, because a collapse that happens when it
// shouldn't destroys a level.

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

} // namespace

int main() {
    // --- an undisturbed structure is never questioned ---
    // The single most important test here. Support is checked on disturbance,
    // never as a global truth, and a floating platform placed on purpose has to
    // stay where it was put. test_grid.cpp depends on this too: it floats a lone
    // Wall in empty space and asserts it never moves.
    {
        Grid g(60, 60);
        fill(g, 20, 20, 40, 24, ElementType::Wall); // floating, touching nothing
        step(g, 200);
        check("an untouched floating slab does not collapse",
              count_of(g, ElementType::Wall) == 5 * 21 &&
              count_of(g, ElementType::Rubble) == 0);
    }

    // --- a slab standing on the world floor stays put when dug ---
    {
        Grid g(60, 60);
        fill(g, 0, 50, 59, 59, ElementType::Wall);
        fill(g, 20, 40, 30, 49, ElementType::Wall); // a block sitting on that floor

        g.set_element(25, 45, ElementType::Empty); // poke a hole in the middle
        step(g, 60);

        check("digging a hole in a grounded structure does not collapse it",
              count_of(g, ElementType::Rubble) == 0,
              std::to_string(count_of(g, ElementType::Rubble)) + " rubble");
    }

    // --- cutting a slab free makes the whole slab fall ---
    {
        Grid g(60, 60);
        fill(g, 0, 50, 59, 59, ElementType::Wall);  // floor
        fill(g, 20, 30, 30, 44, ElementType::Wall); // slab, floating above it
        fill(g, 25, 45, 25, 49, ElementType::Wall); // the one column holding it up

        const int slab = count_of(g, ElementType::Wall);
        step(g, 5);
        check("the propped slab is stable while its support stands",
              count_of(g, ElementType::Wall) == slab);

        // Cut the prop.
        fill(g, 25, 45, 25, 49, ElementType::Empty);
        step(g, 1);

        check("cutting the last support collapses the slab into rubble",
              count_of(g, ElementType::Rubble) == 11 * 15,
              std::to_string(count_of(g, ElementType::Rubble)) + " rubble");

        step(g, 300);
        check("the rubble falls and settles on the floor",
              g.get_element(25, 49).type == ElementType::Rubble);
    }

    // --- collapse conserves cell count ---
    {
        Grid g(60, 60);
        fill(g, 10, 10, 30, 20, ElementType::Wall);
        const int before = count_of(g, ElementType::Wall);

        g.set_element(10, 10, ElementType::Empty); // disturb it; nothing is grounded
        step(g, 1);

        const int after = count_of(g, ElementType::Rubble);
        check("every cell of a collapsed structure becomes one cell of debris",
              after == before - 1,
              std::to_string(before - 1) + " -> " + std::to_string(after));
    }

    // --- Wood collapses on the same rule as Wall ---
    {
        Grid g(60, 60);
        fill(g, 20, 20, 30, 25, ElementType::Wood);
        g.set_element(20, 20, ElementType::Empty);
        step(g, 1);
        check("wood is structural too", count_of(g, ElementType::Wood) == 0 &&
                                        count_of(g, ElementType::Rubble) > 0);
    }

    // --- Wall and Wood hold each other up ---
    {
        Grid g(60, 60);
        fill(g, 0, 50, 59, 59, ElementType::Wall); // floor
        fill(g, 20, 45, 20, 49, ElementType::Wood); // wooden post on the floor
        fill(g, 15, 40, 25, 44, ElementType::Wall); // stone slab on the post

        g.set_element(25, 40, ElementType::Empty); // disturb the slab
        step(g, 30);

        check("a stone slab resting on a wooden post is supported",
              count_of(g, ElementType::Rubble) == 0);
    }

    // --- powder bears load, liquid does not ---
    {
        Grid g(60, 60);
        fill(g, 0, 50, 59, 59, ElementType::Wall);
        fill(g, 20, 45, 30, 49, ElementType::Sand); // sand pile on the floor
        fill(g, 20, 40, 30, 44, ElementType::Wall); // slab resting on the sand
        step(g, 60);                                // let the sand settle first

        g.set_element(30, 40, ElementType::Empty); // disturb the slab
        step(g, 5);

        check("a slab resting on packed sand is supported",
              count_of(g, ElementType::Rubble) == 0,
              std::to_string(count_of(g, ElementType::Rubble)) + " rubble");
    }
    {
        Grid g(60, 60);
        fill(g, 0, 50, 59, 59, ElementType::Wall);
        fill(g, 20, 45, 30, 49, ElementType::Water);
        fill(g, 20, 40, 30, 44, ElementType::Wall);

        g.set_element(30, 40, ElementType::Empty);
        step(g, 2);

        check("a slab resting on water is not supported",
              count_of(g, ElementType::Rubble) > 0);
    }

    // --- digging sand out from under a slab drops it ---
    // The swap_elements trigger rather than the set_element one: nothing is
    // removed from under the slab, the sand simply flows away.
    {
        Grid g(80, 60);
        fill(g, 0, 50, 79, 59, ElementType::Wall);  // floor
        fill(g, 33, 50, 37, 59, ElementType::Empty); // ...with a shaft through it
        fill(g, 35, 46, 35, 49, ElementType::Sand); // a sand pillar over the shaft
        fill(g, 30, 40, 40, 45, ElementType::Wall); // slab, held up by that pillar alone

        check("the slab is up while the sand holds it",
              count_of(g, ElementType::Rubble) == 0);

        // Nothing is removed here and the slab is not touched. The sand simply
        // falls down the shaft, which is the swap_elements trigger rather than
        // the set_element one.
        step(g, 200);

        check("the slab comes down once the sand under it falls away",
              count_of(g, ElementType::Rubble) == 11 * 6,
              std::to_string(count_of(g, ElementType::Rubble)) + " rubble");
    }

    // --- rubble does not collapse again ---
    {
        Grid g(60, 60);
        fill(g, 20, 20, 30, 25, ElementType::Rubble);
        const int before = count_of(g, ElementType::Rubble);
        step(g, 300);
        check("rubble is not itself structural", count_of(g, ElementType::Rubble) == before,
              std::to_string(before) + " -> " + std::to_string(count_of(g, ElementType::Rubble)));
    }

    // --- a structure too large to judge is left alone ---
    // Guards the budget's failure direction: over the cap, assume supported.
    // A missed collapse is invisible; a wrong one destroys the level.
    {
        const int w = 200, h = 200; // 40,000 cells, far over MAX_SUPPORT_CELLS
        Grid g(w, h);
        fill(g, 0, 0, w - 1, h - 2, ElementType::Wall); // huge, and not grounded
        const int before = count_of(g, ElementType::Wall);

        g.set_element(100, 100, ElementType::Empty);
        step(g, 2);

        check("a structure over the size budget is assumed supported",
              count_of(g, ElementType::Rubble) == 0 &&
              count_of(g, ElementType::Wall) == before - 1);
    }

    return report();
}
