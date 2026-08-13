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
    void reset(uint64_t seed);

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

    Grid grid;
    Player player;
    DigTool dig_tool;
};
