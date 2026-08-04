// Player physics tests.
//
// The player reads the grid but never writes it, so these run headlessly for
// the same reason the grid tests do. Every scenario steps the grid alongside
// the player rather than freezing it, because a player standing on terrain that
// is still settling is the normal case, not the exception.
//
// **Every dimension below is derived from Player::WIDTH / HEIGHT /
// MAX_STEP_HEIGHT rather than written as a number.** These were literals until
// the body was scaled to match the reference art (4x8 cells to 8x20), and the
// literals did not merely need updating - some of them silently changed what
// was being tested. The old "four-cell ledge" case asserted that a ledge taller
// than MAX_STEP_HEIGHT stops the player, and at a step height of 5 a four-cell
// ledge is one the player is *supposed* to walk up: the test would have kept
// its name, flipped its meaning, and failed for the right reason by accident.
// Worse, the standing spawns put the body's feet below the floor at the new
// height, so several scenarios would have begun buried and passed through
// resolve_overlap() instead of through the physics they name. Derived geometry
// is what stops the next resize from doing that quietly.

#include "physics/player.h"
#include "test_util.h"
#include <string>

namespace {

constexpr float DT = 1.0f / 60.0f;

// A world roomy enough for the body at any plausible size: tall enough for a
// long fall above the floor, wide enough that a second of walking at
// MOVE_SPEED never reaches the far border.
constexpr int WORLD_W = 8 * Player::WIDTH + 240;
constexpr int WORLD_H = 5 * Player::HEIGHT;
constexpr int FLOOR_Y = WORLD_H - Player::HEIGHT;

// Spawn heights. STAND_Y drops the body a short way onto the floor - enough
// that it is genuinely falling, not so far that it is still in the air after
// the settle steps. AIR_Y is the long-drop spawn.
constexpr int STAND_Y = FLOOR_Y - Player::HEIGHT - Player::HEIGHT / 2;
constexpr int AIR_Y = 2;

// Where obstacles go: far enough right that the player is at full speed when
// it arrives, with room beyond to keep walking afterwards.
constexpr int OBSTACLE_X = WORLD_W / 2;

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
        Grid g = make_world(WORLD_W, WORLD_H, FLOOR_Y);
        Player p(10, AIR_Y);

        const int start_y = p.cell_y();
        step(g, p, NOTHING, 10);
        check("player falls under gravity", p.cell_y() > start_y, feet_detail(p));

        step(g, p, NOTHING, 200);
        check("player lands exactly on the floor surface", feet(p) == FLOOR_Y, feet_detail(p));
        check("player reports on_ground once landed", p.is_on_ground());
        check("a landed player is not inside terrain", !p.overlaps_solid(g, p.cell_x(), p.cell_y()));
    }

    // --- no tunnelling, even at terminal velocity through a one-cell floor ---
    // Movement resolves one cell at a time, so this is meant to be impossible
    // by construction rather than by being fast enough. The long drop is there
    // to guarantee the player is at MAX_FALL_SPEED when it arrives -- so the
    // drop is stated in terms of MAX_FALL_SPEED rather than as a fixed height,
    // which is what keeps it a terminal-velocity test after a retune.
    {
        const int drop = static_cast<int>(Player::MAX_FALL_SPEED) * 2;
        const int thin_floor_y = drop + Player::HEIGHT;
        Grid g(WORLD_W, thin_floor_y + 4);
        for (int x = 0; x < WORLD_W; ++x) g.set_element(x, thin_floor_y, ElementType::Wall);

        Player p(10, 0);
        step(g, p, NOTHING, 400);
        check("player does not tunnel through a one-cell floor", feet(p) == thin_floor_y,
              feet_detail(p));
    }

    // --- walking ---
    {
        Grid g = make_world(WORLD_W, WORLD_H, FLOOR_Y);
        Player p(10, STAND_Y);
        step(g, p, NOTHING, 60);

        const int start_x = p.cell_x();
        step(g, p, held_right(), 60); // one second of walking

        // Most of a second's worth of MOVE_SPEED, not a fixed cell count: the
        // slack absorbs the steps spent landing, and the claim ("it actually
        // travels at roughly the speed it is configured for") survives a
        // retune of MOVE_SPEED, which a literal would not.
        const int expected = static_cast<int>(Player::MOVE_SPEED * 0.66f);
        check("player walks along flat ground", p.cell_x() - start_x > expected,
              "moved=" + std::to_string(p.cell_x() - start_x) + " cells, wanted >" +
                  std::to_string(expected));
    }

    // --- a wall stops the player, and is not passed through ---
    {
        Grid g = make_world(WORLD_W, WORLD_H, FLOOR_Y);
        for (int y = FLOOR_Y - Player::HEIGHT - 2; y < FLOOR_Y; ++y)
            g.set_element(OBSTACLE_X, y, ElementType::Wall);

        Player p(10, STAND_Y);
        step(g, p, NOTHING, 60);
        step(g, p, held_right(), 300);

        check("player stops flush against a wall", p.cell_x() + Player::WIDTH == OBSTACLE_X,
              feet_detail(p));
        check("player does not pass through a wall", !p.overlaps_solid(g, p.cell_x(), p.cell_y()));
    }

    // --- a one-cell lip is a step, not a wall ---
    {
        Grid g = make_world(WORLD_W, WORLD_H, FLOOR_Y);
        g.set_element(OBSTACLE_X, FLOOR_Y - 1, ElementType::Wall);
        g.set_element(OBSTACLE_X + 1, FLOOR_Y - 1, ElementType::Wall);

        Player p(10, STAND_Y);
        step(g, p, NOTHING, 60);
        step(g, p, held_right(), 300);

        check("player steps up over a one-cell lip", p.cell_x() > OBSTACLE_X + 2, feet_detail(p));
    }

    // --- a ledge of exactly MAX_STEP_HEIGHT is still a step ---
    // The upper edge of the promise. Without this, MAX_STEP_HEIGHT could be
    // quietly off by one in the blocking direction and only the test below
    // would notice, which would still pass.
    {
        Grid g = make_world(WORLD_W, WORLD_H, FLOOR_Y);
        for (int y = FLOOR_Y - Player::MAX_STEP_HEIGHT; y < FLOOR_Y; ++y)
            for (int x = OBSTACLE_X; x < OBSTACLE_X + 4; ++x)
                g.set_element(x, y, ElementType::Wall);

        Player p(10, STAND_Y);
        step(g, p, NOTHING, 60);
        step(g, p, held_right(), 300);

        check("player steps up a ledge of exactly MAX_STEP_HEIGHT",
              p.cell_x() > OBSTACLE_X + 4, feet_detail(p));
    }

    // --- but a ledge one cell taller than MAX_STEP_HEIGHT is not ---
    {
        Grid g = make_world(WORLD_W, WORLD_H, FLOOR_Y);
        for (int y = FLOOR_Y - Player::MAX_STEP_HEIGHT - 1; y < FLOOR_Y; ++y)
            g.set_element(OBSTACLE_X, y, ElementType::Wall);

        Player p(10, STAND_Y);
        step(g, p, NOTHING, 60);
        step(g, p, held_right(), 300);

        check("player cannot step up a ledge taller than MAX_STEP_HEIGHT",
              p.cell_x() + Player::WIDTH == OBSTACLE_X, feet_detail(p));
    }

    // --- jumping ---
    {
        Grid g = make_world(WORLD_W, WORLD_H, FLOOR_Y);
        Player p(10, STAND_Y);
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

        // One body height of clearance. JUMP_SPEED against GRAVITY gives about
        // one and a half, so this asserts the jump is useful rather than
        // asserting the exact arc, which is tuning and not a promise.
        check("jump lifts the player clear of the ground", rest_y - highest >= Player::HEIGHT,
              "peak=" + std::to_string(rest_y - highest) + " cells");
        check("player lands back on the ground after a jump",
              p.cell_y() == rest_y && p.is_on_ground(), feet_detail(p));
    }

    // --- liquids are not solid: the player sinks through a pool ---
    // The pool is a full body deep, so this is a submerged body rather than a
    // shallow puddle the feet happen to clear.
    {
        Grid g = make_world(WORLD_W, WORLD_H, FLOOR_Y);
        for (int y = FLOOR_Y - Player::HEIGHT; y < FLOOR_Y; ++y)
            for (int x = 0; x < WORLD_W; ++x)
                g.set_element(x, y, ElementType::Water);

        Player p(WORLD_W / 2, AIR_Y);
        step(g, p, NOTHING, 300);
        check("player falls through water rather than standing on it",
              feet(p) == FLOOR_Y && p.is_on_ground(), feet_detail(p));
    }

    // --- powder is solid: the player walks over it instead of through it ---
    // The slab is left to slump into its own settled slope first, so this is a
    // walk over real uneven powder rather than over a flat block that happens
    // to be made of sand. Sized to the body: a pile shorter than the step
    // height is one the player steps over without ever climbing it, which
    // would pass this test without testing anything.
    {
        Grid g = make_world(WORLD_W, WORLD_H, FLOOR_Y);
        const int pile_h = Player::MAX_STEP_HEIGHT + 3;
        const int pile_end = OBSTACLE_X + 6 * Player::WIDTH;
        for (int y = FLOOR_Y - pile_h; y < FLOOR_Y; ++y)
            for (int x = OBSTACLE_X; x < pile_end; ++x)
                g.set_element(x, y, ElementType::Sand);

        Player p(10, STAND_Y);
        step(g, p, NOTHING, 200); // player lands, sand settles

        bool walked_above_floor = false;
        const PlayerInput right = held_right();
        for (int i = 0; i < 300; ++i) {
            step(g, p, right, 1);
            if (feet(p) < FLOOR_Y) walked_above_floor = true;
        }

        check("player walks over settled powder, not through it", walked_above_floor,
              feet_detail(p));
        check("player crosses the powder and keeps going", p.cell_x() > pile_end, feet_detail(p));
        check("player does not end up inside the powder",
              !p.overlaps_solid(g, p.cell_x(), p.cell_y()));
    }

    // --- a buried player digs itself out instead of freezing ---
    // The grid does not know the player exists, so terrain can and will end up
    // inside the body. Without resolve_overlap() every direction is blocked and
    // the player is stuck for the rest of the run.
    //
    // The body is buried deeper than MAX_UNSTUCK_RADIUS can see on purpose:
    // that is the grind-upward path at the end of resolve_overlap(), and it is
    // the one that has to hold when a collapse drops a hillside on the player.
    // The step count has to cover that grind, one cell per step.
    {
        const int surface = Player::HEIGHT;
        Grid g(WORLD_W, WORLD_H);
        for (int y = surface; y < WORLD_H; ++y)
            for (int x = 0; x < WORLD_W; ++x)
                g.set_element(x, y, ElementType::Wall);

        Player p(10, surface + 2 * Player::HEIGHT); // fully inside the block
        check("a buried player starts out overlapping terrain",
              p.overlaps_solid(g, p.cell_x(), p.cell_y()));

        step(g, p, NOTHING, 4 * Player::HEIGHT + 60);
        check("a buried player pushes itself back out of terrain",
              !p.overlaps_solid(g, p.cell_x(), p.cell_y()), feet_detail(p));
    }

    // --- the sub-cell remainder must not carry the body into terrain ---
    //
    // Regression tests for PLAYTEST_LOG.md session 1, defect A1. These assert on
    // visual_x()/visual_y(), which is the *only* thing in the suites that does
    // and is meant to stay that way -- but the bug they cover is a simulation
    // bug, not a rendering one. `rem_y` was pending motion that had been decided
    // against and never cleared, so a standing player sank a full cell into the
    // floor over ~18 steps and snapped back. Every whole-cell assertion above
    // passed throughout, because the body's *cell* never moved. Nothing that
    // rounded to whole cells could have caught this, which is exactly why the
    // check is written against the fractional position.
    {
        Grid g = make_world(WORLD_W, WORLD_H, FLOOR_Y);
        Player p(10, AIR_Y);
        step(g, p, NOTHING, 120); // land and settle

        const float rest_y = p.visual_y();
        float lowest = rest_y;
        for (int i = 0; i < 120; ++i) {
            step(g, p, NOTHING, 1);
            if (p.visual_y() > lowest) lowest = p.visual_y();
        }

        check("a standing player does not sink into the floor",
              lowest - rest_y < 0.001f,
              "sank " + std::to_string(lowest - rest_y) + " cells below rest");
        check("a standing player does not drift off the floor",
              p.visual_y() == rest_y, feet_detail(p));
    }

    {
        Grid g = make_world(WORLD_W, WORLD_H, FLOOR_Y);
        for (int y = FLOOR_Y - Player::HEIGHT - 2; y < FLOOR_Y; ++y)
            g.set_element(OBSTACLE_X, y, ElementType::Wall);

        Player p(OBSTACLE_X - 4 * Player::WIDTH, STAND_Y);
        step(g, p, held_right(), 120); // walk into the wall and keep pushing

        // The body's right edge must never cross the wall's near face, not even
        // fractionally. Held against it, `rem_x` used to reach 0.75 of a cell
        // before a whole-cell step was attempted, which drew the player inside
        // the wall it was flush against.
        const float right_edge = p.visual_x() + Player::WIDTH;
        check("a player held against a wall does not phase into it",
              right_edge <= static_cast<float>(OBSTACLE_X),
              "right edge at " + std::to_string(right_edge) + ", wall face at " +
                  std::to_string(OBSTACLE_X));
    }

    return report();
}
