#pragma once
#include "render/player_sprite.h"

// Which frame of the player sheet to draw, and when to move to the next one.
//
// **Deliberately free of SDL**, exactly like V7's LightField and for the same
// reason: what this produces is a row and a column, and turning that into a
// source rect is main.cpp's job. So the part with the logic in it is testable
// the way the simulation is - see tests/test_anim.cpp.
//
// **And deliberately not part of the simulation.** F3.5 settled that rendering
// does not feed the simulation, and animation state on `Player` would be state
// the determinism tests then have to account for, in exchange for something
// that only ever picks a source rect. Nothing under src/physics/ includes this,
// the same arrangement (and the same CMakeLists variable) that keeps light out.
//
// ---------------------------------------------------------------------------
// THE CLOCK, which is the one thing here that is easy to get wrong
// ---------------------------------------------------------------------------
//
// `Anim::wait` counts **fixed simulation steps**, and `update()` is called once
// per fixed step - never once per rendered frame, and never with a delta time.
//
// ROADMAP.md names this trap against screen shake and hit-stop: an effect
// driven off the rendered frame while the simulation runs on a fixed step
// changes with frame rate, which is the class of bug F1 and F2.3 spent two
// sections retiring. A walk cycle has exactly that shape - at 165 Hz it would
// run nearly three times the speed it does at 60, and it would look like a
// tuning problem rather than a clock problem.
//
// The fix is not to move animation into the simulation. It is to drive it off
// the count of steps the simulation has actually taken, which main.cpp already
// computes and which is framerate-independent by construction.
namespace player_anim {

// Everything the selector needs, as plain data. No Player reference: the
// dependency direction is that presentation reads the body, and passing four
// values keeps this callable from a test with no Grid and no Run at all.
struct Conditions {
    bool on_ground = false;

    // Actually translating, not merely being asked to. Walking into a wall
    // leaves the input held while the body does not move, and a walk cycle
    // playing against a wall is the single most common way this kind of
    // selector looks broken.
    bool moving = false;

    // Sign is what matters: it chooses between the two poses sharing the
    // airborne row. Down is positive, matching the grid's y axis.
    float vel_y = 0.0f;

    // True on the step a dig actually fired - not while the button is held.
    // A held button fires on a cooldown, and animating the button rather than
    // the tool would play a swing on steps where nothing happened.
    bool dig_fired = false;
};

struct State {
    const player_sprite::Anim* anim = &player_sprite::IDLE;
    int frame = 0;      // index within the animation, not the sheet column
    int elapsed = 0;    // fixed steps spent on the current frame
    bool oneshot = false;

    // The sheet column to draw. Adding `col` is what lets two animations share
    // a row - see the note on Anim in the generated header.
    int sheet_col() const { return anim->col + frame; }
    int sheet_row() const { return anim->row; }
};

// The animation the conditions alone imply. Pure, and separate from update()
// so a test can pin the mapping without also driving a clock through it.
const player_sprite::Anim& select(const Conditions& c);

// Advance by `steps` fixed steps.
//
// A one-shot (`dig`) latches on the step it is triggered, plays to its last
// frame, and then hands control back to `select`. That is the whole reason this
// is a state machine rather than `frame = step_count % frames`: a modulo has no
// way to express "play this once and then stop asking me".
void update(State& state, const Conditions& c, int steps = 1);

}  // namespace player_anim
