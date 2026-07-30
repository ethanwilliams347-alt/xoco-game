#pragma once
#include "physics/grid.h"
#include "physics/player.h"
#include "physics/tool.h"

// Everything one play session needs, held as a single object instead of three
// locals `main.cpp` had to thread through by hand. SDL-free for the same
// reason `src/physics/` is: a run that needs a window cannot be driven by a
// test.
//
// This step (F2.1) only moves the three locals `main.cpp` already had into
// one place - no behaviour change. `Grid::reset()` / `Run::reset(seed)`
// (F2.2) and `Run::step(const Input&)` (F2.3) are what make this class earn
// its name.
class Run {
public:
    // Matches how `main.cpp` built these three before this step existed: the
    // player spawns in mid-air over the middle of the world. There is no
    // terrain yet, but the world border reads as solid, so it falls to the
    // bottom edge rather than out of existence.
    Run(int width, int height, uint64_t seed = Grid::DEFAULT_SEED);

    Grid grid;
    Player player;
    DigTool dig_tool;
};
