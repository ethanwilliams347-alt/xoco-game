#include "run.h"

Run::Run(int width, int height, uint64_t seed)
    : grid(width, height, seed)
    , player(width / 2, height / 4)
{
}

void Run::reset(uint64_t seed) {
    grid.reset(seed);
    player = Player(grid.get_width() / 2, grid.get_height() / 4);
    dig_tool = DigTool();
}
