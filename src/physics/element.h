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

    // Steps this cell has spent in unbroken free fall, which is what a falling
    // structure's speed is read from. It lives on the cell rather than on the
    // piece because a piece has no identity between steps: it is re-discovered
    // by flood fill every time, so the only place to keep anything is in the
    // cells themselves. Moving the cell carries it along for free, since
    // swap_elements() moves whole Elements.
    //
    // Zero for everything else. Only structural materials are ever in free fall
    // as a body; powders and fluids move one cell per step by their own rules.
    uint8_t fall_ticks = 0;
};

// fall_ticks is meant to be free - it sits in padding the struct already had
// after updated_tag. Worth pinning down, because it would stop being free the
// moment it pushed the struct over an alignment boundary: this array is one
// entry per cell of the world, and every step walks all of the awake part of
// it, so a byte here is 500 KB and a slower sweep at the target resolution.
static_assert(sizeof(Element) <= 12, "Element grew - price the extra memory traffic before accepting it");
