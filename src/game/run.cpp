#include "run.h"

Run::Run(int width, int height, uint64_t seed)
    : grid(width, height, seed)
    , player(width / 2, height / 4)
{
}
