#pragma once
#include "fixed.h"
#include "grid.h"

// What the player is being told to do this step.
//
// Deliberately plain bools rather than SDL key codes: the simulation stays free
// of any SDL dependency, so the player is testable headlessly for the same
// reason the grid is, and main.cpp remains the only file that knows a keyboard
// exists.
struct PlayerInput {
    bool left = false;
    bool right = false;
    bool jump = false;

    // Where the player is aiming, in absolute grid cells. Grid coordinates
    // rather than a direction vector because the caller already has the cursor
    // in world space and converting to an angle here would only throw away
    // information the tool wants back.
    int aim_x = 0;
    int aim_y = 0;
    bool dig = false;
};

// A rigid body that lives *outside* the cell grid.
//
// Everything else in this engine is a cell that gets stepped in place. The
// player is not, and that is a deliberate architectural split: a cell can only
// move one step per frame in one of eight directions, which is fine for sand
// and useless for a character that needs sub-cell speed, a jump arc, and a
// body several cells tall that must stay in one piece.
//
// So the player is an axis-aligned box with its own position and velocity that
// only ever *reads* the grid, asking one question of it: "is this cell solid?".
// It never writes cells, which means it cannot break the engine's rule that all
// writes go through set_element / swap_elements.
class Player {
public:
    // Body size in cells. Matched to Noita's wizard measured off
    // resources/video_screenshots/test_location.jpg: at that game's 4 screen
    // pixels per cell the body is ~8 cells across and ~20 tall below the hat,
    // and Camera::SCALE is 4 here too, so these numbers put a character of the
    // same apparent size on screen *built from the same number of cells*. That
    // second half is the point. The body was 4x8 before, which looked close
    // enough at a glance but resolved the character roughly 2.5x more coarsely
    // than the reference: a two-cell step lip was a quarter of the body's
    // height rather than a tenth, so every interaction with uneven terrain read
    // chunkier than the art it was imitating.
    //
    // A hat, if one is ever drawn, overhangs as sprite only. It is not part of
    // the collision box and must not be added to HEIGHT - the reference's own
    // hat is ~3 further cells that clearly do not collide with anything.
    static constexpr int WIDTH = 8;
    static constexpr int HEIGHT = 20;

    // Cells per second, and cells per second squared, in `fx` fixed point.
    // Real units rather than per-step amounts, so the tuning still means the
    // same thing if the fixed step ever changes.
    //
    // **Written as exact rationals, never as a float cast.** `from_ratio(225,
    // 2)` is 112.5 and is the same 7372800 on every compiler; `fx::v(112.5f *
    // 65536)` is a float expression a compiler is free to fold its own way, and
    // the whole point of F5 is that no such freedom exists anywhere on the path
    // from a constant to a cell. Read the second number as the denominator of
    // the value in TUNING.md - `from_int(500)` and `from_ratio(1000, 2)` are
    // the same thing, and the first is the one to write.
    //
    // All five of these are lengths per unit time, so they are all 2.5x what
    // they were before the body grew by 2.5x. That factor is not a coincidence
    // to be tidied away: a speed in cells only means something relative to the
    // size of the thing moving, and holding body-lengths-per-second fixed is
    // what makes the character feel unchanged at the new scale rather than
    // merely be the same size. Tune from here by feel; do not tune from the
    // pre-scale numbers, which now describe a much slower character.
    static constexpr fx::v MOVE_SPEED = fx::from_ratio(225, 2);  // 112.5
    static constexpr fx::v JUMP_SPEED = fx::from_int(175);  // ~30 cells, still ~1.5 bodies
    static constexpr fx::v GRAVITY = fx::from_int(500);
    static constexpr fx::v MAX_FALL_SPEED = fx::from_int(400);

    // --- flight ---------------------------------------------------------
    //
    // The character is a bird, so holding the key beats wings rather than
    // doing nothing until the feet are back on the ground. Three constants
    // and an interval, and the relationship between them is the whole feel:
    //
    // A flap is an **impulse, not a velocity set**. Setting `vel_y` outright
    // would let one beat cancel a terminal-speed dive, which is the opposite
    // of weight - a bird arresting a stoop takes several beats and visibly
    // loses height doing it. Subtracting a fixed amount means a fall has to
    // be *worked* out of, and the number of beats it costs scales with how
    // fast the body was already going down.
    //
    // `FLAP_MAX_CLIMB` is what stops that impulse compounding. Without a cap,
    // each beat leaves the body a little faster upward than the last and the
    // climb accelerates without bound - holding the key would eventually
    // outrun the world. The cap is deliberately *far* below JUMP_SPEED: the
    // launch off the ground is one big downstroke and is allowed to exceed
    // it, so a standing jump still clears more than a body height while
    // sustained flight is a slow laboured grind.
    //
    // Net altitude per beat is the balance of two numbers:
    // FLAP_IMPULSE against GRAVITY * (FLAP_INTERVAL_STEPS / 60), the gravity a
    // single beat has to pay for. That toll is ~117 cells/s and it is fixed
    // here - there is no separate flight gravity - so **the felt weight of
    // flight is how much of the impulse is left after paying it.**
    //
    // It was 130 against ~117: a margin of 13, about a body height per two
    // seconds, which playtested as roughly 40% too heavy. Both constants are
    // retuned to take 40% out of that weight, and they have to move together:
    //
    //   FLAP_IMPULSE 177 leaves a margin of ~60 rather than ~13, which is the
    //   same climb the character would get if gravity's toll were 40% lighter
    //   (117 * 0.6 = 70; 130 - 70 = 60). This is the number that decides how
    //   many beats it costs to arrest a dive, which is most of what "heavy"
    //   means moment to moment.
    //
    //   FLAP_MAX_CLIMB 98 is the old 70 plus the same 40%, and without it the
    //   extra impulse would be spent entirely on recovering from falls while
    //   sustained climbing stayed at exactly its old speed - the cap, not the
    //   impulse, is what sets that. Raising one and not the other is the most
    //   likely way to retune this and still be told it feels heavy.
    //
    // The cap is still deliberately far below JUMP_SPEED: the launch off the
    // ground is one big downstroke and is allowed to exceed it, so a standing
    // jump stays the strongest single upward move the character has. "Climbs
    // under its own power, and is not a helicopter" is the design; it is a
    // property of the margin rather than of any one constant, so tune the pair
    // together or the character becomes one or the other.
    static constexpr fx::v FLAP_IMPULSE = fx::from_int(177);    // cells/s removed from vel_y per beat
    static constexpr fx::v FLAP_MAX_CLIMB = fx::from_int(98);   // cells/s ceiling on upward speed
    static constexpr int FLAP_INTERVAL_STEPS = 14;             // fixed steps between beats

    // Tallest lip the player walks over without jumping. This is the whole of
    // "walking over uneven powder": a settled sand slope is a staircase of
    // one-cell steps, and without this the player would have to jump over every
    // single grain.
    //
    // 5 rather than the old 2 for the same reason as the speeds: the slope is
    // still made of one-cell steps, but the leg climbing it is 2.5x longer, and
    // a lip that stopped the old body at a quarter of its height would stop
    // this one at a tenth.
    //
    // **2026-08-12: 3 rather than 5 (D7).** The reasoning above scaled the old
    // ratio and never checked it - 5 against a 20-cell body is a quarter of the
    // player's own height climbed instantly and with no animation, which
    // playtesting read as "walking into a settled pile does nothing, the player
    // walks over it". That is not a missing interaction, it is this number. 3
    // is about a curb rather than a table, and it is still three times the
    // one-cell steps a settled slope is actually made of, so powder stays
    // walkable - the test below the cliff cases in tests/test_player.cpp exists
    // to hold that, because it is the failure this trades towards. Not 2: the
    // leg really is 2.5x longer than the body that number was chosen for, so
    // going back to it would be undoing the scaling rather than correcting it.
    static constexpr int MAX_STEP_HEIGHT = 3;

    // How far the unstuck search looks for open space when the player ends up
    // inside terrain. See resolve_overlap() for why that happens at all. Scaled
    // with the body: the search has to be able to clear a body-width of
    // material, or a burial that used to be escapable no longer is.
    static constexpr int MAX_UNSTUCK_RADIUS = 20;

    Player(int start_x, int start_y);

    // Advances the player by one fixed step. Call this at the same fixed rate
    // as Grid::update(), and after it, so collision is tested against the world
    // as it now is rather than as it was.
    //
    // **There is no `dt` parameter, and its absence is the point.** It used to
    // take one, and every caller passed the same compile-time constant into it -
    // which made the timestep look like something that varies and is not. A
    // parameter nobody varies is an invitation to vary it, and the first caller
    // that passed a real frame time would have made the simulation a function
    // of framerate again, which is the exact defect F1 spent a whole item
    // closing. The rate is `fx::STEPS_PER_SECOND`, it is known at compile time,
    // and every per-step amount below is therefore folded by the compiler
    // rather than multiplied at runtime.
    void update(const Grid& grid, const PlayerInput& input);

    // Top-left corner of the body, in cells.
    int cell_x() const { return pos_x; }
    int cell_y() const { return pos_y; }

    // Centre of the body, in cells. Where tools originate from -- firing from
    // the top-left corner would let the player dig through a wall its own body
    // is flush against on the other side.
    int center_x() const { return pos_x + WIDTH / 2; }
    int center_y() const { return pos_y + HEIGHT / 2; }

    // The body's position including the sub-cell remainder. **For rendering
    // only** - nothing in `src/physics/` may read these, and no test asserts on
    // them, because a fractional position is exactly the float-edge
    // representation the integer scheme below exists to keep out of collision.
    //
    // They exist because discarding the remainder at draw time was a visible
    // defect (PLAYTEST_LOG.md session 1, A1) rather than a rounding detail:
    // MOVE_SPEED was 45 cells/s at the time against a 60 Hz step, which is 0.75
    // cells per step, so `cell_x()` advanced on three steps out of four and
    // stalled on the fourth. (It is 112.5 now, or 1.875 cells per step - the
    // defect is the same shape at any speed with a fraction in it.) The
    // simulation was right the whole time - the renderer was throwing away the
    // part that made the motion smooth.
    //
    // Float here rather than fixed point, and only here: this is the one number
    // the player hands to the renderer, which multiplies it by a screen scale
    // and rounds it to a pixel. Converting at the boundary keeps the float on
    // the render side of it, where a last-bit difference between two machines
    // is a pixel that was going to be rounded anyway.
    float visual_x() const { return static_cast<float>(pos_x) + fx::to_float(rem_x); }
    float visual_y() const { return static_cast<float>(pos_y) + fx::to_float(rem_y); }

    bool is_on_ground() const { return on_ground; }

    // Downward speed in **fx cells per second** - compare it against the
    // constants above, or against `fx::from_int(n)`, not against a plain number.
    // `fx::trunc()` gives whole cells per second for a readout.
    //
    // S0's fall damage is the first consumer that will read this for anything
    // other than a sign, and it is the reason this is fixed point rather than a
    // float: a damage threshold is a *rule*, and a run that kills the player on
    // one machine and not another is a worse bug than a wrong threshold.
    fx::v velocity_y() const { return vel_y; }

    // True on the step a wing beat actually fired - not while the key is
    // held. Added for the animation selector, which needs the *event* rather
    // than the input: a flap animation driven off the held key would play
    // continuously between beats, and the whole read of flight at this size
    // is the rhythm of discrete downstrokes. Same shape, and the same
    // reasoning, as DigTool reporting the step a dig connected.
    bool flapped() const { return did_flap; }

    // Read-only, and added for the animation selector rather than for anything
    // in here. It is what distinguishes "the walk key is held" from "the body
    // is actually travelling" - input held against a wall leaves the first true
    // and the second false, and a walk cycle that plays on the input walks on
    // the spot against every wall in the game. No new state: this is the same
    // field move_x already keeps.
    // fx cells per second, like velocity_y(). It is exactly zero or exactly
    // +/-MOVE_SPEED, so a caller asking "is the body travelling" compares
    // against 0 rather than against an epsilon - which the float version could
    // not do, and which is one small place where the conversion made a check
    // correct rather than merely portable.
    fx::v velocity_x() const { return vel_x; }

    // True if a body placed with its top-left at (px, py) would overlap any
    // solid cell. Public because "the player is not inside a wall" is the
    // single most useful thing for a test to assert.
    bool overlaps_solid(const Grid& grid, int px, int py) const;

private:
    // Position is an integer cell plus a sub-cell remainder rather than a plain
    // float. Collision then only ever compares whole cells, so a resting player
    // sits at an exact cell instead of a hair inside the floor, and there is no
    // class of float-edge bugs where a box is "0.0001 into" a wall. The
    // remainder carries the fractional part of a move into the next step, which
    // is what keeps motion smooth at speeds below one cell per step.
    //
    // The remainder and the velocities are `fx` rather than `float` (F5). The
    // integer cell was always the half that mattered for collision; making the
    // other half integer too is what turns "deterministic on this binary" into
    // "deterministic anywhere", which is what `tests/test_run.cpp`'s replay
    // check has been quietly assumed to prove and did not.
    int pos_x;
    int pos_y;
    fx::v rem_x = 0;
    fx::v rem_y = 0;

    fx::v vel_x = 0;
    fx::v vel_y = 0;
    bool on_ground = false;

    // Steps remaining before the next wing beat is allowed, and whether one
    // fired this step. Counted in fixed steps rather than seconds for the
    // same reason DigTool's cooldown is: the beat rate has to be identical
    // on every machine.
    int flap_timer = 0;
    bool did_flap = false;

    // How far the body would have to be lifted to move one cell towards `sign`,
    // or -1 if that direction is a wall rather than a step. Zero means the way
    // is already clear. Shared by move_x, which needs the height, and by
    // update(), which only needs to know whether the sub-cell remainder is
    // allowed to keep accumulating that way.
    int climb_for(const Grid& grid, int sign) const;

    // Both move one cell at a time and stop at the first blocked cell, so the
    // body cannot tunnel through thin terrain no matter how fast it is going.
    void move_x(const Grid& grid, int amount);
    void move_y(const Grid& grid, int amount);

    // How many of the body's cells are inside solid material at a position.
    // `overlaps_solid` answers whether the body is stuck; this answers how
    // badly, which is what makes one escape comparable to another.
    int overlap_depth(const Grid& grid, int px, int py) const;

    // Whether the body can actually travel `dx, dy` to reach an open position,
    // rather than only fit once it is there. See resolve_overlap().
    bool escape_is_reachable(const Grid& grid, int dx, int dy) const;

    // Returns true if the body was overlapping terrain this step, in which case
    // the caller should skip normal physics.
    bool resolve_overlap(const Grid& grid);
};

// Three relationships the constants above have to hold, asserted rather than
// written down - the same move `element.h` and `reaction.h` make about their
// data tables, and for the same reason: TUNING.md invites these numbers to be
// changed by feel, and a prose sentence about how two of them relate is a
// sentence that goes stale the first time one is retuned alone.

// Headroom. `rem_y` holds at most a whole cell plus one step of the fastest
// motion in the game before the whole part is taken out of it, so the ceiling
// is nowhere near int32_t's - but "nowhere near" is a claim about MAX_FALL_SPEED,
// and MAX_FALL_SPEED is a knob. A retune that overflowed this would show up as
// a body teleporting on the frame it hit terminal velocity.
static_assert(fx::ONE + fx::per_step(Player::MAX_FALL_SPEED) < INT32_MAX / 256,
              "MAX_FALL_SPEED has been raised far enough to threaten the fixed-point "
              "range; widen fx::v before raising it further");

// Flight climbs at all. A beat has to pay GRAVITY for the whole interval it
// covers, and what is left over is the climb - so raising FLAP_INTERVAL_STEPS
// or GRAVITY without touching FLAP_IMPULSE eventually makes the wingbeat a
// slower fall rather than a climb. That is a legitimate design choice and this
// is not blocking it; it is refusing to let it happen *silently*, which is how
// it would otherwise be discovered.
static_assert(Player::FLAP_IMPULSE > fx::per_step(Player::GRAVITY) * Player::FLAP_INTERVAL_STEPS,
              "a wingbeat no longer outweighs the gravity it has to pay for, so holding "
              "the key would sink rather than climb");

// The standing jump stays the strongest single upward move. The whole reason
// the ground beat sets vel_y outright and the air beat is capped.
static_assert(Player::FLAP_MAX_CLIMB < Player::JUMP_SPEED,
              "sustained flight is now at least as fast as a standing jump, which makes "
              "the character a helicopter - see the flight comment above");
