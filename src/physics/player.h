#pragma once
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

    // Cells per second, and cells per second squared. Real units rather than
    // per-step amounts, so the tuning still means the same thing if the fixed
    // step ever changes.
    //
    // All five of these are lengths per unit time, so they are all 2.5x what
    // they were before the body grew by 2.5x. That factor is not a coincidence
    // to be tidied away: a speed in cells only means something relative to the
    // size of the thing moving, and holding body-lengths-per-second fixed is
    // what makes the character feel unchanged at the new scale rather than
    // merely be the same size. Tune from here by feel; do not tune from the
    // pre-scale numbers, which now describe a much slower character.
    static constexpr float MOVE_SPEED = 112.5f;
    static constexpr float JUMP_SPEED = 175.0f;  // ~30 cells, still ~1.5 bodies
    static constexpr float GRAVITY = 500.0f;
    static constexpr float MAX_FALL_SPEED = 400.0f;

    // Tallest lip the player walks over without jumping. This is the whole of
    // "walking over uneven powder": a settled sand slope is a staircase of
    // one-cell steps, and without this the player would have to jump over every
    // single grain.
    //
    // 5 rather than the old 2 for the same reason as the speeds: the slope is
    // still made of one-cell steps, but the leg climbing it is 2.5x longer, and
    // a lip that stopped the old body at a quarter of its height would stop
    // this one at a tenth.
    static constexpr int MAX_STEP_HEIGHT = 5;

    // How far the unstuck search looks for open space when the player ends up
    // inside terrain. See resolve_overlap() for why that happens at all. Scaled
    // with the body: the search has to be able to clear a body-width of
    // material, or a burial that used to be escapable no longer is.
    static constexpr int MAX_UNSTUCK_RADIUS = 20;

    Player(int start_x, int start_y);

    // Advances the player by one fixed step. Call this at the same fixed rate
    // as Grid::update(), and after it, so collision is tested against the world
    // as it now is rather than as it was.
    void update(const Grid& grid, const PlayerInput& input, float dt);

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
    // MOVE_SPEED is 45 cells/s against a 60 Hz step, which is 0.75 cells per
    // step, so `cell_x()` advances on three steps out of four and stalls on the
    // fourth. The simulation was right the whole time - the renderer was
    // throwing away the part that made the motion smooth.
    float visual_x() const { return static_cast<float>(pos_x) + rem_x; }
    float visual_y() const { return static_cast<float>(pos_y) + rem_y; }

    bool is_on_ground() const { return on_ground; }
    float velocity_y() const { return vel_y; }

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
    int pos_x;
    int pos_y;
    float rem_x = 0.0f;
    float rem_y = 0.0f;

    float vel_x = 0.0f;
    float vel_y = 0.0f;
    bool on_ground = false;

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

    // Returns true if the body was overlapping terrain this step, in which case
    // the caller should skip normal physics.
    bool resolve_overlap(const Grid& grid);
};
