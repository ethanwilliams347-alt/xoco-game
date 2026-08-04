// Digging tests.
//
// The tool writes to the grid, which the player never does, so these are the
// first tests in the project covering a deliberate mutation of the world from
// outside the simulation step. Most scenarios freeze the grid (no update()
// call) so that what is asserted is the dig itself rather than the settling
// that follows it -- except the one test where the settling is the point.

#include "physics/tool.h"
#include "test_util.h"
#include <string>

namespace {

// Fills a rectangle, inclusive of both corners.
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

// Digs once, ignoring the cooldown, by using a fresh tool each time.
bool dig_once(Grid& g, int from_x, int from_y, int aim_x, int aim_y) {
    DigTool tool;
    return tool.update(g, true, from_x, from_y, aim_x, aim_y);
}

} // namespace

int main() {
    // --- a dig removes cells around the impact point ---
    {
        Grid g(120, 80);
        fill(g, 60, 0, 119, 79, ElementType::Wall);
        const int before = count_of(g, ElementType::Wall);

        // 15 cells short of the slab, comfortably inside RANGE (60).
        const bool hit = dig_once(g, 45, 40, 110, 40);
        const int after = count_of(g, ElementType::Wall);

        check("digging into a slab reports a hit", hit);
        check("digging into a slab removes cells", after < before,
              "removed " + std::to_string(before - after));
        check("the hole is at the near face of the slab",
              g.get_element(60, 40).type == ElementType::Empty);
    }

    // --- range is a hard limit ---
    {
        Grid g(200, 80);
        // Slab starts well beyond RANGE (60) from the origin at x=10.
        fill(g, 100, 0, 199, 79, ElementType::Wall);
        const int before = count_of(g, ElementType::Wall);

        const bool hit = dig_once(g, 10, 40, 150, 40);

        check("a target beyond range is not hit", !hit);
        check("a target beyond range is untouched",
              count_of(g, ElementType::Wall) == before);
    }

    // --- the ray stops at the FIRST solid it meets ---
    // The anti-tunnelling assertion. A ray that samples every Nth cell can pass
    // through a thin wall and dig the terrain behind it, which is exactly the
    // wall the player was sheltering behind.
    {
        Grid g(120, 80);
        fill(g, 30, 0, 30, 79, ElementType::Wall); // near wall, 1 cell thick
        fill(g, 60, 0, 60, 79, ElementType::Wall); // far wall, 1 cell thick

        dig_once(g, 10, 40, 110, 40);

        check("the near wall is cut", g.get_element(30, 40).type == ElementType::Empty);
        check("the far wall is untouched behind it",
              g.get_element(60, 40).type == ElementType::Wall);
    }

    // --- liquids and gases do not stop the ray ---
    {
        Grid g(120, 80);
        fill(g, 20, 30, 25, 50, ElementType::Water);
        fill(g, 28, 30, 30, 50, ElementType::Fire);
        fill(g, 34, 0, 34, 79, ElementType::Wall);

        dig_once(g, 10, 40, 110, 40);

        check("the ray passes through water and fire to the wall behind",
              g.get_element(34, 40).type == ElementType::Empty);
    }

    // --- digging under a settled pile makes it fall ---
    // The one test that steps the grid. It is really an assertion about
    // set_element() waking the 3x3 neighbourhood: a dig that wrote cells[]
    // directly would leave the sand hanging over the hole.
    {
        Grid g(120, 80);
        fill(g, 0, 60, 119, 79, ElementType::Wall); // floor
        fill(g, 50, 40, 70, 59, ElementType::Sand); // pile resting on it

        const int placed = count_of(g, ElementType::Sand);
        for (int i = 0; i < 200; ++i) g.update(); // let it settle
        const int settled = count_of(g, ElementType::Sand);

        // Guards the setup itself. If settling loses grains, every assertion
        // below it is measuring the wrong thing.
        check("settling the pile conserves sand", placed == settled,
              std::to_string(placed) + " -> " + std::to_string(settled));
        check("the pile is resting on the floor",
              g.get_element(60, 59).type == ElementType::Sand);

        // Dig horizontally into the base of the pile, from inside RANGE. The
        // pile's near face is ~15 cells away, so this stays a near-range dig
        // even though RANGE itself grew with the body.
        const int dug_from_x = 35;
        dig_once(g, dug_from_x, 55, 119, 55);

        const int after_dig = count_of(g, ElementType::Sand);
        check("digging opens a hole in the pile", after_dig < settled,
              "removed " + std::to_string(settled - after_dig));

        for (int i = 0; i < 200; ++i) g.update();
        const int after_collapse = count_of(g, ElementType::Sand);

        check("the collapse that follows creates and destroys nothing",
              after_dig == after_collapse,
              std::to_string(after_dig) + " -> " + std::to_string(after_collapse));

        // The point of the whole block: the grains over the hole must not still
        // be hanging where they were. They can only know to fall because
        // set_element() woke their neighbourhood.
        check("sand above the hole did not stay hanging in mid-air",
              g.get_element(50, 59).type == ElementType::Sand);
    }

    // --- the cooldown gates repeat digs ---
    // Asserted against the tool's own ready state rather than by counting digs
    // over time: a slab recedes as it is dug, so a count would be bounded by
    // geometry running out of range as much as by the cooldown, and would pass
    // for the wrong reason.
    {
        Grid g(120, 80);
        fill(g, 40, 0, 119, 79, ElementType::Wall);

        DigTool tool;
        check("a fresh tool is ready", tool.is_ready());
        check("the first held step digs", tool.update(g, true, 30, 40, 110, 40));
        check("a tool that just dug is not ready", !tool.is_ready());

        for (int i = 0; i < DigTool::COOLDOWN_STEPS - 1; ++i)
            tool.update(g, false, 30, 40, 110, 40);
        check("the tool is still cooling one step short", !tool.is_ready());

        tool.update(g, false, 30, 40, 110, 40);
        check("the tool is ready again after the full cooldown", tool.is_ready());
    }

    // --- holding the button does not dig every step ---
    {
        Grid g(400, 80);
        fill(g, 0, 0, 399, 79, ElementType::Wall);

        DigTool tool;
        int digs = 0;
        const int steps = 60;
        for (int i = 0; i < steps; ++i)
            if (tool.update(g, true, 200, 40, 399, 40)) digs++;

        check("holding dig is rate-limited", digs > 0 && digs < steps,
              std::to_string(digs) + " digs in " + std::to_string(steps) + " steps");
    }

    // --- missing costs no cooldown ---
    {
        Grid g(120, 80);
        DigTool tool;
        for (int i = 0; i < 30; ++i) tool.update(g, true, 20, 40, 30, 40); // empty air
        check("digging thin air leaves the tool ready", tool.is_ready());
    }

    // --- the world border survives being dug at ---
    {
        Grid g(60, 60);
        fill(g, 0, 50, 59, 59, ElementType::Wall);
        const int before = count_of(g, ElementType::Wall);

        // Straight at the border from close range, and then past the corner.
        dig_once(g, 5, 55, -40, 55);
        dig_once(g, 5, 5, -40, -40);
        dig_once(g, 55, 55, 200, 200);

        check("digging at the border does not crash and leaves the world sealed",
              g.get_element(-1, 55).type == ElementType::Wall &&
              g.get_element(60, 55).type == ElementType::Wall);
        check("digging at the border still removes in-bounds cells",
              count_of(g, ElementType::Wall) < before);
    }

    // --- aiming at your own position does nothing ---
    {
        Grid g(60, 60);
        fill(g, 0, 0, 59, 59, ElementType::Wall);
        const int before = count_of(g, ElementType::Wall);
        const bool hit = dig_once(g, 30, 30, 30, 30);
        check("a zero-length aim is a no-op",
              !hit && count_of(g, ElementType::Wall) == before);
    }

    return report();
}
