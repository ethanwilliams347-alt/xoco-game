#pragma once
#include "physics/grid.h"
#include "physics/player.h"
#include "physics/tool.h"

// Everything outside the run that one fixed step needs - filled from SDL by
// `main.cpp`, and filled by hand in `tests/test_run.cpp`. This is the
// boundary F1 could not close from inside `Grid`: the simulation was already
// a pure function of its seed, but input still arrived by sampling the
// keyboard once per *rendered* frame and replaying that one sample into every
// fixed step the frame happened to contain - so the framerate was quietly an
// input to the simulation. One `Input` now drives exactly one fixed step, so
// a recorded sequence of them replays to the same result regardless of how
// the original session was paced across frames.
//
// A plain struct of button and cursor state rather than SDL types, for the
// same reason `PlayerInput` already was: it keeps everything under
// `src/game/` and `src/physics/` testable without a window.
struct Input {
    bool left = false;
    bool right = false;
    bool jump = false;
    bool dig = false;

    // Cursor position in grid cells, shared by the dig tool's aim and by
    // where the brush paints - both come from the same mouse position in
    // `main.cpp`, so one pair of coordinates covers both.
    int cursor_x = 0;
    int cursor_y = 0;

    // The world-editing brush. Folded in here rather than kept as the special
    // case it used to be: it was painted once per rendered frame, before the
    // fixed-step loop even started, which made the amount of material laid
    // down while the button was held depend on render framerate rather than
    // on how much simulated time actually passed. It is now just three more
    // fields on the struct that drives a step, painted once per step like
    // everything else.
    bool brush_active = false;
    ElementType brush_type = ElementType::Sand;
    int brush_size = 1;
};

// Everything one play session needs, held as a single object instead of three
// locals `main.cpp` had to thread through by hand. SDL-free for the same
// reason `src/physics/` is: a run that needs a window cannot be driven by a
// test.
class Run {
public:
    // S0: how this run ended, or that it has not.
    //
    // **`Playing` is a real state and not the absence of the other two**, which
    // is the whole reason this is an enum rather than a pair of bools: a run
    // that is somehow both won and lost is unrepresentable, and the "neither"
    // case has a name that reads correctly at the call site.
    enum class Outcome : uint8_t { Playing, Won, Lost };

    // The simulation advances in fixed steps so that sand falls at the same
    // rate on a 60 Hz and a 144 Hz display; rendering runs as fast as the
    // display allows and calls `step()` as many times as have accumulated.
    // Owned here rather than by `main.cpp` now that `Run` is what actually
    // advances by this amount each call.
    //
    // **This is the frame pacer's copy of the rate, not the simulation's.**
    // `main.cpp` accumulates real elapsed seconds against it and interpolates
    // the drawn position by the leftover; nothing inside a step reads it. The
    // physics uses `fx::STEPS_PER_SECOND` instead, as an integer, because a
    // step's worth of gravity has to be the same number on every machine and a
    // double reciprocal is not (F5). So it is *derived* from the integer rather
    // than written as `1.0 / 60.0` beside it: two spellings of one rate is two
    // chances to change one of them, and a pacer running at a rate the physics
    // does not believe in fails as a character that moves at the wrong speed -
    // a bug that looks like a tuning complaint and gets answered as one.
    static constexpr double FIXED_DT = 1.0 / fx::STEPS_PER_SECOND;

    // Matches how `main.cpp` built these three before this step existed: the
    // player spawns in mid-air over the middle of the world. There is no
    // terrain yet, but the world border reads as solid, so it falls to the
    // bottom edge rather than out of existence.
    Run(int width, int height, uint64_t seed = Grid::DEFAULT_SEED);

    // Puts the run back to what a fresh `Run(width, height, seed)` would be:
    // `grid` goes through `Grid::reset(seed)` rather than being reallocated,
    // and `player`/`dig_tool` are replaced outright since neither owns anything
    // a wipe would need to preserve. Grid size cannot change through a reset,
    // for the same reason `Grid::reset()` cannot change it - see there.
    //
    // **Two things it deliberately does not restore, and both are named rather
    // than left to be discovered.** The objective survives, for the reason at
    // the field below - it is a property of the level and not of the run. And
    // **the world's terrain does not come back**, because `Run` does not own it:
    // the scene is loaded from two BMPs by `main.cpp`, so a caller that wants
    // the level it started with has to re-stamp it after calling this. That is
    // a real sharp edge and it is the caller's, not this class's; the argument
    // for leaving it there rather than giving `Run` the scene is in
    // ENGINEERING_NOTES.md, along with what would make it come due.
    // **The size arguments are `V26`, 2026-08-24, and they are the one way a
    // grid's dimensions can change at all.** `Grid::reset` cannot resize and
    // deliberately says so; a scene list whose rows name different world sizes
    // needs *some* path that can, and the choice is between this and tearing the
    // whole `Run` down at the call site. It is here because the call site is
    // `main.cpp`, which holds a `Run&` threaded through several lambdas - a
    // rebuild there would leave every one of them pointing at a dead object.
    //
    // 0 means "keep the size", not "a world of no cells", so every existing
    // caller and every recorded session is unaffected: `reset(seed)` still
    // wipes in place through `Grid::reset` and reallocates nothing.
    //
    // **A resize is a heavier reset than a wipe and the difference is
    // observable.** `Grid::reset`'s documented exception - `vent_radius`
    // surviving, because it is configuration rather than world state - does not
    // survive a resize, because the grid is a new object. That is stated rather
    // than fixed: carrying it across would mean the new grid is not a fresh
    // `Grid(w, h, seed)`, which is the one thing the reset contract promises.
    // A caller that has configured a vent radius re-applies it after changing
    // scene, the same way it re-stamps the terrain.
    void reset(uint64_t seed, int new_width = 0, int new_height = 0);

    // Advances grid, player and dig tool by exactly one fixed step, in that
    // order - matching the order `main.cpp` ran them in before this step
    // existed. The brush paints first, before the grid steps, for the same
    // reason `main.cpp` used to paint before physics ran: a cell should not
    // move on the same step it was placed.
    //
    // Returns whether the dig tool actually removed material this step -
    // `DigTool::update`'s own answer, forwarded rather than recomputed.
    //
    // **Nothing currently reads it.** It existed to drive the dig animation,
    // and since D1 the animation reads `DigTool::swing_progress()` instead: an
    // impact is one step and a swing is a duration, and it was the duration
    // that needed portraying. Kept because "a blow landed this step" is a real
    // event that a hit-reaction, a sound or a screen shake will each want, and
    // because it costs a forwarded bool. **Do not reintroduce it as an
    // animation trigger** - that is the defect, and ROADMAP.md's wave 4 has the
    // arithmetic.
    bool step(const Input& input);

    // --- S0: the objective, and how a run ends -----------------------------
    //
    // **The objective is a point in the world, not a cell**, and that is a
    // deliberate limit on the thin half of the item. A cell would mean a row in
    // MATERIALS, which would mean answering what happens when it is dug, burnt,
    // displaced or buried - four questions the full "Objective + Extraction"
    // item in ROADMAP.md exists to answer and which a spike has no business
    // deciding by accident. A point is reached or it is not.
    //
    // Placed by the caller rather than by a generator, because there is no
    // generator; `main.cpp` scans the fixture's terrain for a surface the same
    // way it plants props, which is the closest thing to "in the level" that a
    // hard-coded objective can honestly be.
    void set_objective(int x, int y);

    // Removes the objective outright.
    //
    // **This is the escape hatch for the exception recorded at `objective_set`
    // below, and it exists because that exception met a level it was wrong
    // for.** Surviving `reset()` is right when the caller re-stamps *the same*
    // scene, which was the only thing that could happen until scenes became a
    // list. Switching to a scene with no terrain leaves the previous level's
    // objective hanging in empty space - placed, unreachable, and claiming the
    // run is winnable. So the rule is unchanged and narrowed: the objective
    // survives a reset, and a caller that changes the *level* says so.
    void clear_objective();
    bool has_objective() const { return objective_set; }
    int objective_x() const { return goal_x; }
    int objective_y() const { return goal_y; }

    // How close the body has to get, in cells, measured from the objective to
    // the nearest point of the collision box rather than to the body's centre -
    // so a 8x20 body reaches the same objective from the same distance whether
    // it is standing beside it or under it.
    //
    // 6 is a little under half a body height. Small enough that it reads as
    // touching the marker, large enough that it does not need pixel-accurate
    // positioning against a marker drawn at four screen pixels per cell.
    static constexpr int OBJECTIVE_REACH = 6;

    Outcome outcome() const { return run_outcome; }

    Grid grid;
    Player player;
    DigTool dig_tool;

private:
    // **The outcome latches.** Once a run is over it stays over, even though
    // `step()` keeps simulating: freezing the world is a presentation decision
    // and `main.cpp` makes it by not accumulating time, the same way it already
    // does for the settings menu. A `Run` driven headlessly past its own ending
    // - which every test here does - must not have the answer flicker back.
    Outcome run_outcome = Outcome::Playing;

    // **The objective survives `reset()`, and this is the second documented
    // exception to "reset restores a fresh Run", after `Grid::vent_radius`.**
    // The argument is the same shape: it is a property of the *level*, not of
    // the run played through it, and the caller re-stamps the same scene on
    // reset, so a cleared objective would be one the caller has to remember to
    // place again. Forgetting would produce a run that is unwinnable and says
    // nothing, which is the worst available failure for this particular field.
    bool objective_set = false;
    int goal_x = 0;
    int goal_y = 0;
};
