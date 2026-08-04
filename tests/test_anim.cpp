#include <cstdio>
#include <string>
#include "render/player_anim.h"
#include "test_util.h"

// V3.1's animation selector. Headless because `player_anim` is deliberately
// SDL-free - what it produces is a sheet row and column, and turning that into
// a source rect is main.cpp's job - so the part with the logic in it is
// testable the same way the simulation is.
//
// **What these tests are actually guarding.** The selector's failure modes are
// all silent and all look like art problems rather than logic problems, which
// is why they are worth pinning in code rather than in the manual checklist:
// a cycle that restarts every step reads as "the walk animation is bad", a
// one-shot that never completes reads as "the dig sprite is stuck", and a
// clock that advances per rendered frame reads as "the walk is too fast on my
// machine". None of those would send anyone to this file.

using namespace player_anim;
namespace ps = player_sprite;

namespace {

Conditions standing() { Conditions c; c.on_ground = true;  c.moving = false; return c; }
Conditions walking()  { Conditions c; c.on_ground = true;  c.moving = true;  return c; }
Conditions rising()   { Conditions c; c.on_ground = false; c.vel_y = -5.0f;  return c; }
Conditions falling()  { Conditions c; c.on_ground = false; c.vel_y =  5.0f;  return c; }

void run(State& s, const Conditions& c, int steps) {
    for (int i = 0; i < steps; ++i) update(s, c, 1);
}

// --- selection --------------------------------------------------------------

void test_selection() {
    check("standing selects idle", &select(standing()) == &ps::IDLE);
    check("moving on ground selects walk", &select(walking()) == &ps::WALK);
    check("rising selects rise", &select(rising()) == &ps::RISE);
    check("falling selects fall", &select(falling()) == &ps::FALL);

    // The apex of a jump passes through zero. If zero picked `fall`, the pose
    // would flip for exactly one step on the way up and flip back - a
    // one-frame flicker at the top of every single jump.
    Conditions apex; apex.on_ground = false; apex.vel_y = 0.0f;
    check("apex of a jump holds the rising pose", &select(apex) == &ps::RISE);

    // Input held against a wall is the case `moving` exists for. Selecting on
    // the input instead would walk on the spot against every wall in the game.
    Conditions blocked; blocked.on_ground = true; blocked.moving = false;
    check("blocked against a wall does not walk", &select(blocked) == &ps::IDLE);
}

// --- the shared row ---------------------------------------------------------

void test_shared_row() {
    // rise and fall are one sheet row holding two poses. If `col` were ignored
    // both would draw the same drawing, and the bug would look like missing
    // art rather than like an indexing mistake.
    State s;
    update(s, rising(), 1);
    const int rise_row = s.sheet_row(), rise_col = s.sheet_col();
    update(s, falling(), 1);
    check("rise and fall share a sheet row", s.sheet_row() == rise_row);
    check("rise and fall are different columns", s.sheet_col() != rise_col,
          "rise col " + std::to_string(rise_col) + ", fall col " + std::to_string(s.sheet_col()));
}

// --- the clock --------------------------------------------------------------

void test_walk_cycle() {
    State s;
    update(s, walking(), 1);
    check("walk starts at frame 0", s.frame == 0);

    // One frame's worth of steps advances exactly one frame. This is the
    // property that makes the cycle's speed a function of simulated time
    // rather than of frame rate.
    run(s, walking(), ps::WALK.wait - 1);
    check("walk holds its frame for `wait` steps", s.frame == 1,
          "frame " + std::to_string(s.frame));

    // And a whole cycle returns to the start rather than running off the end
    // of the row into whatever is drawn beside it.
    State t;
    update(t, walking(), 1);
    run(t, walking(), ps::WALK.wait * ps::WALK.frames - 1);
    check("walk loops back to frame 0", t.frame == 0, "frame " + std::to_string(t.frame));
}

void test_no_restart_on_reselect() {
    // The failure this pins: re-selecting the animation already playing resets
    // it, so every step restarts the cycle and the figure stands still with
    // one leg twitching. It is the most likely single bug in this file and it
    // is invisible in a screenshot.
    State s;
    update(s, walking(), 1);
    run(s, walking(), ps::WALK.wait * 2);
    check("a continuing walk is not restarted each step", s.frame == 2,
          "frame " + std::to_string(s.frame));
}

// --- the one-shot -----------------------------------------------------------

void test_dig_oneshot() {
    State s;
    Conditions c = standing();
    c.dig_fired = true;
    update(s, c, 1);
    check("dig latches on the step it fires", s.anim == &ps::DIG && s.frame == 0);

    // It must hold through conditions that would otherwise select something
    // else, or the swing is cancelled by the player walking away mid-dig.
    run(s, walking(), 1);
    check("dig survives a change of conditions", s.anim == &ps::DIG);

    // And it must actually end. A one-shot that never clears is a figure
    // frozen mid-swing for the rest of the session.
    run(s, standing(), ps::DIG.wait * ps::DIG.frames);
    check("dig completes and returns to the selector", s.anim == &ps::IDLE,
          "still on " + std::string(s.anim == &ps::DIG ? "dig" : "something else"));
}

void test_dig_retrigger() {
    State s;
    Conditions c = standing();
    c.dig_fired = true;
    update(s, c, 1);
    run(s, standing(), ps::DIG.wait);
    check("dig has advanced before retrigger", s.frame == 1);
    update(s, c, 1);
    check("a second dig restarts the swing", s.anim == &ps::DIG && s.frame == 0);
}

// --- the fixed-step contract ------------------------------------------------

void test_batch_equivalence() {
    // Advancing by N is the same as advancing by 1 N times. This is what makes
    // it sound to drive the clock off a count of fixed steps: a frame that
    // happens to contain three steps must land where three frames of one step
    // each would have.
    State a, b;
    const int n = ps::WALK.wait * ps::WALK.frames + 3;
    update(a, walking(), n);
    run(b, walking(), n);
    check("advancing N steps equals N single steps",
          a.anim == b.anim && a.frame == b.frame && a.elapsed == b.elapsed,
          "batch frame " + std::to_string(a.frame) + " vs stepped " + std::to_string(b.frame));
}

void test_static_pose_does_not_spin() {
    // rise/fall have wait 0. A naive advance would divide by it or loop
    // forever; this asserts it simply holds.
    State s;
    run(s, falling(), 500);
    check("a wait-0 pose holds rather than spinning", s.frame == 0 && s.anim == &ps::FALL);
}

}  // namespace

int main() {
    test_selection();
    test_shared_row();
    test_walk_cycle();
    test_no_restart_on_reselect();
    test_dig_oneshot();
    test_dig_retrigger();
    test_batch_equivalence();
    test_static_pose_does_not_spin();
    return report();
}
