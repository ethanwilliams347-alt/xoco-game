#pragma once
#include "grid.h"
#include "player.h" // RANGE and RADIUS are stated as ratios of the body size

// The player's verbs — the things it does *to* the world, as opposed to the
// things the world does to it.
//
// Deliberately not methods on Player. The body and the verb are different
// concerns, and keeping Player free of any grid write is what makes it
// trivially correct against the engine's rule that every mutation goes through
// set_element / swap_elements: a class that holds only a `const Grid&` cannot
// break that rule by accident. Tools take a mutable `Grid&` and are the only
// player-side code that does.
class DigTool {
public:
    // How far the dig reaches from the player's centre, in cells. Roughly three
    // body-heights: far enough to clear a path ahead, short enough that the
    // player has to commit to a position rather than deleting the level from
    // across the screen. Stated as a multiple of Player::HEIGHT rather than as
    // a bare 60 so the ratio the comment claims is the one the code enforces -
    // this was 24 against a 8-cell body and had to be found by hand when the
    // body grew.
    static constexpr int RANGE = 3 * Player::HEIGHT;

    // Radius of the hole taken out at the impact point. Three quarters of the
    // body's width, so the bite is one and a half bodies across: clearly
    // visible at a 4x pixel scale without being an explosion. Same ratio the
    // old 3-against-a-4-wide-body had, kept by expressing it as the ratio.
    static constexpr int RADIUS = 3 * Player::WIDTH / 4;

    // Fixed steps between digs. Counted in steps rather than seconds so the
    // rate is identical on every machine, for the same reason the simulation
    // itself is fixed-step.
    static constexpr int COOLDOWN_STEPS = 6;

    // Digs from (from_x, from_y) toward (aim_x, aim_y) if the cooldown allows.
    // Returns true if a hole was actually taken out this step.
    //
    // Call once per fixed step, whether or not the button is held: the cooldown
    // only advances when this is called, so skipping the call while not digging
    // would let the timer freeze and every first shot fire instantly.
    bool update(Grid& grid, bool held, int from_x, int from_y, int aim_x, int aim_y);

    // Where the current aim would land, for drawing a cursor. Reports the
    // impact point if the ray hits something, otherwise the end of its range.
    // Read-only: takes a const Grid& and shares the march with update().
    void aim_point(const Grid& grid, int from_x, int from_y, int aim_x, int aim_y,
                   int& out_x, int& out_y) const;

    bool is_ready() const { return cooldown == 0; }

private:
    int cooldown = 0;

    // Marches one cell at a time from the origin toward the aim and reports
    // where it stops. One cell per step for exactly the reason the player's
    // movement sub-steps: a ray that skips cells digs *through* a wall into
    // whatever is behind it, and the wall it skipped is the one the player was
    // standing behind for cover.
    void march(const Grid& grid, int from_x, int from_y, int aim_x, int aim_y,
               int& out_x, int& out_y, bool& out_hit) const;
};
