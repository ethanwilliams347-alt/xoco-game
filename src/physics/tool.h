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

    // **A swing, not a cooldown, and the difference is the whole of D1.** This
    // used to be `COOLDOWN_STEPS = 6` - a rate limit with no notion of a swing
    // at all, which left the swing's length owned by the *animation* table
    // (3 frames x wait 8 = 24 steps) while the digs it was supposed to portray
    // arrived every 6. Two clocks, in two files, that nobody ever compared: the
    // animation was restarted four times before it could reach its second
    // frame, so a held dig sat frozen on frame 0 and played its one swing only
    // once the button came up.
    //
    // There is now one clock and it lives here, on the fixed step, because
    // ENGINEERING_NOTES.md refuses frame-tied gameplay events outright -
    // rendering must never drive simulation. The animation is *told* where in
    // the swing the tool is (see `swing_progress`) and owns none of the timing.
    //
    // 36 steps is deliberately longer than the walk cycle's 30, which is the
    // request from session 5 - a swing should read as heavier than a stride.
    // It takes digging from ~10/second to under 2, a gameplay change accepted
    // when this shape was chosen; **the dig radius is the knob if it reads as
    // feeble, not this.**
    static constexpr int SWING_STEPS = 36;

    // **The hole comes out on the first step of the swing, and the rest of the
    // swing is follow-through.** An earlier build put the impact two thirds in,
    // where a real swing's arc bottoms out, and it is the more literal reading
    // of the motion - but it costs the player half a second between pressing
    // the button and the world changing, on a tool used constantly, and a dig
    // that lands late reads as input lag rather than as weight. The frames
    // after the first are the recovery from a blow that has already landed.
    //
    // Consequences worth knowing, because both are the *opposite* of what the
    // deferred-impact version needed. The aim used is the aim at the moment of
    // the press, which is exactly right when there is no time in between for it
    // to have moved - so there is no second ray march, and no window in which
    // the world can change under the swing. And the ray that decides whether a
    // swing starts at all is the same ray that digs, rather than a probe that
    // has to be repeated later.

    // Advances the swing and digs from (from_x, from_y) toward (aim_x, aim_y).
    // Returns true on the step the hole is taken out, which is the step a swing
    // begins - so at most once per `SWING_STEPS`.
    //
    // Call once per fixed step, whether or not the button is held: the swing
    // only advances when this is called, so skipping the call while not digging
    // would leave a swing suspended halfway through.
    bool update(Grid& grid, bool held, int from_x, int from_y, int aim_x, int aim_y);

    // Where the swing is, as a fraction from 0 (just started) up to but never
    // reaching 1, or -1 when no swing is in progress.
    //
    // **A fraction rather than a step count, so that the ratio D1 was is not
    // reconstructible.** A consumer given "step 14 of 36" has to hold 36 to
    // make sense of it, and the moment two files hold the swing's length one of
    // them can be retuned alone - which is exactly how the animation and the
    // cooldown drifted apart. A fraction is complete on its own, and the
    // animation divides it by its own frame count without ever learning this
    // clock's units.
    float swing_progress() const {
        return swing < 0 ? -1.0f : static_cast<float>(swing) / SWING_STEPS;
    }

    // Where the current aim would land, for drawing a cursor. Reports the
    // impact point if the ray hits something, otherwise the end of its range.
    // Read-only: takes a const Grid& and shares the march with update().
    void aim_point(const Grid& grid, int from_x, int from_y, int aim_x, int aim_y,
                   int& out_x, int& out_y) const;

    // Ready means "not mid-swing". Same meaning it always had - the tool will
    // act on the button this step - now that the swing is what occupies it.
    bool is_ready() const { return swing < 0; }

private:
    // The step reached within the current swing, or -1 for idle.
    int swing = -1;

    // Marches one cell at a time from the origin toward the aim and reports
    // where it stops. One cell per step for exactly the reason the player's
    // movement sub-steps: a ray that skips cells digs *through* a wall into
    // whatever is behind it, and the wall it skipped is the one the player was
    // standing behind for cover.
    void march(const Grid& grid, int from_x, int from_y, int aim_x, int aim_y,
               int& out_x, int& out_y, bool& out_hit) const;
};
