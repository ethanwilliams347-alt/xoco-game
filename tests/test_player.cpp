// Player physics tests.
//
// The player reads the grid but never writes it, so these run headlessly for
// the same reason the grid tests do. Every scenario steps the grid alongside
// the player rather than freezing it, because a player standing on terrain that
// is still settling is the normal case, not the exception.

#include "physics/player.h"
#include "test_util.h"
#include <string>

namespace {

constexpr float DT = 1.0f / 60.0f;

void step(Grid& g, Player& p, const PlayerInput& in, int n) {
    for (int i = 0; i < n; ++i) {
        g.update();
        p.update(g, in, DT);
    }
}

// A world with a solid floor whose top surface is row `floor_y`.
Grid make_world(int w, int h, int floor_y) {
    Grid g(w, h);
    for (int y = floor_y; y < h; ++y)
        for (int x = 0; x < w; ++x)
            g.set_element(x, y, ElementType::Wall);
    return g;
}

// Where the player's feet are, as a row index. Equal to floor_y when standing
// on that floor, which makes the resting position exactly assertable -- the
// point of storing position as whole cells plus a remainder.
int feet(const Player& p) { return p.cell_y() + Player::HEIGHT; }

std::string feet_detail(const Player& p) {
    return "feet=" + std::to_string(feet(p)) + " x=" + std::to_string(p.cell_x());
}

const PlayerInput NOTHING{};

PlayerInput held_right() {
    PlayerInput in;
    in.right = true;
    return in;
}

} // namespace

int main() {
    // --- gravity, and coming to rest exactly on the surface ---
    {
        Grid g = make_world(60, 40, 35);
        Player p(10, 2);

        const int start_y = p.cell_y();
        step(g, p, NOTHING, 10);
        check("player falls under gravity", p.cell_y() > start_y, feet_detail(p));

        step(g, p, NOTHING, 200);
        check("player lands exactly on the floor surface", feet(p) == 35, feet_detail(p));
        check("player reports on_ground once landed", p.is_on_ground());
        check("a landed player is not inside terrain", !p.overlaps_solid(g, p.cell_x(), p.cell_y()));
    }

    // --- no tunnelling, even at terminal velocity through a one-cell floor ---
    // Movement resolves one cell at a time, so this is meant to be impossible
    // by construction rather than by being fast enough. The long drop is there
    // to guarantee the player is at MAX_FALL_SPEED when it arrives.
    {
        Grid g(60, 200);
        for (int x = 0; x < 60; ++x) g.set_element(x, 190, ElementType::Wall);

        Player p(10, 0);
        step(g, p, NOTHING, 400);
        check("player does not tunnel through a one-cell floor", feet(p) == 190, feet_detail(p));
    }

    // --- walking ---
    {
        Grid g = make_world(120, 40, 35);
        Player p(10, 20);
        step(g, p, NOTHING, 60);

        const int start_x = p.cell_x();
        step(g, p, held_right(), 60); // one second of walking
        check("player walks along flat ground", p.cell_x() - start_x > 30,
              "moved=" + std::to_string(p.cell_x() - start_x) + " cells");
    }

    // --- a wall stops the player, and is not passed through ---
    {
        Grid g = make_world(120, 40, 35);
        for (int y = 25; y < 35; ++y) g.set_element(60, y, ElementType::Wall);

        Player p(10, 20);
        step(g, p, NOTHING, 60);
        step(g, p, held_right(), 300);

        check("player stops flush against a wall", p.cell_x() + Player::WIDTH == 60, feet_detail(p));
        check("player does not pass through a wall", !p.overlaps_solid(g, p.cell_x(), p.cell_y()));
    }

    // --- a one-cell lip is a step, not a wall ---
    {
        Grid g = make_world(120, 40, 35);
        g.set_element(60, 34, ElementType::Wall);
        g.set_element(61, 34, ElementType::Wall);

        Player p(10, 20);
        step(g, p, NOTHING, 60);
        step(g, p, held_right(), 300);

        check("player steps up over a one-cell lip", p.cell_x() > 62, feet_detail(p));
    }

    // --- but a ledge taller than MAX_STEP_HEIGHT is not ---
    {
        Grid g = make_world(120, 40, 35);
        for (int y = 31; y < 35; ++y) g.set_element(60, y, ElementType::Wall);

        Player p(10, 20);
        step(g, p, NOTHING, 60);
        step(g, p, held_right(), 300);

        check("player cannot step up a four-cell ledge", p.cell_x() + Player::WIDTH == 60,
              feet_detail(p));
    }

    // --- jumping ---
    {
        Grid g = make_world(60, 40, 35);
        Player p(10, 20);
        step(g, p, NOTHING, 60);

        const int rest_y = p.cell_y();

        PlayerInput jump;
        jump.jump = true;
        step(g, p, jump, 1);

        int highest = p.cell_y();
        for (int i = 0; i < 120; ++i) {
            step(g, p, NOTHING, 1);
            if (p.cell_y() < highest) highest = p.cell_y();
        }

        check("jump lifts the player clear of the ground", rest_y - highest >= 8,
              "peak=" + std::to_string(rest_y - highest) + " cells");
        check("player lands back on the ground after a jump",
              p.cell_y() == rest_y && p.is_on_ground(), feet_detail(p));
    }

    // --- liquids are not solid: the player sinks through a pool ---
    {
        Grid g = make_world(60, 40, 35);
        for (int y = 27; y < 35; ++y)
            for (int x = 0; x < 60; ++x)
                g.set_element(x, y, ElementType::Water);

        Player p(28, 2);
        step(g, p, NOTHING, 300);
        check("player falls through water rather than standing on it",
              feet(p) == 35 && p.is_on_ground(), feet_detail(p));
    }

    // --- powder is solid: the player walks over it instead of through it ---
    // The slab is left to slump into its own settled slope first, so this is a
    // walk over real uneven powder rather than over a flat block that happens
    // to be made of sand.
    {
        Grid g = make_world(120, 40, 35);
        for (int y = 33; y < 35; ++y)
            for (int x = 60; x < 90; ++x)
                g.set_element(x, y, ElementType::Sand);

        Player p(10, 20);
        step(g, p, NOTHING, 200); // player lands, sand settles

        bool walked_above_floor = false;
        const PlayerInput right = held_right();
        for (int i = 0; i < 300; ++i) {
            step(g, p, right, 1);
            if (feet(p) < 35) walked_above_floor = true;
        }

        check("player walks over settled powder, not through it", walked_above_floor,
              feet_detail(p));
        check("player crosses the powder and keeps going", p.cell_x() > 90, feet_detail(p));
        check("player does not end up inside the powder",
              !p.overlaps_solid(g, p.cell_x(), p.cell_y()));
    }

    // --- a buried player digs itself out instead of freezing ---
    // The grid does not know the player exists, so terrain can and will end up
    // inside the body. Without resolve_overlap() every direction is blocked and
    // the player is stuck for the rest of the run.
    {
        Grid g(60, 40);
        for (int y = 20; y < 40; ++y)
            for (int x = 0; x < 60; ++x)
                g.set_element(x, y, ElementType::Wall);

        Player p(10, 25); // fully inside the block
        check("a buried player starts out overlapping terrain",
              p.overlaps_solid(g, p.cell_x(), p.cell_y()));

        step(g, p, NOTHING, 60);
        check("a buried player pushes itself back out of terrain",
              !p.overlaps_solid(g, p.cell_x(), p.cell_y()), feet_detail(p));
    }

    return report();
}
