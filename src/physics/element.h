#pragma once
#include <cstdint>
#include "material.h"

struct Element {
    ElementType type = ElementType::Empty;
    uint32_t color = 0xFF000000; // ARGB8888, base colour plus per-cell jitter

    // The frame tag of the step this cell was last visited in. A cell is skipped
    // when this equals the grid's current tag, which stops it moving twice in one
    // sweep. Storing the tag rather than a bool means there is no per-step pass to
    // reset it, which matters once most of the world is asleep.
    uint8_t updated_tag = 0;
};
