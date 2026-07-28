#pragma once
#include "material.h"
#include <cstdint>

// A + optional-neighbor -> result, rolled once per eligible cell per step. Rows
// are checked in order; the first row whose target and catalyst condition are
// both satisfied is the only one considered that frame (win or lose the roll,
// no falling through to a later row) - this is what makes water-dousing take
// priority over Fire's natural burnout below without any special-casing.
struct Reaction {
    ElementType catalyst; // required 8-neighbor; ElementType::Count = spontaneous, no neighbor needed
    ElementType target;   // the cell type this row may transform
    uint8_t chance_pct;   // 0-100, rolled once per eligible cell per step
    ElementType result;
};

inline constexpr Reaction REACTIONS[] = {
    { ElementType::Fire,  ElementType::Wood,  12, ElementType::Fire  },
    { ElementType::Fire,  ElementType::Oil,   40, ElementType::Fire  },
    { ElementType::Water, ElementType::Fire,  90, ElementType::Steam },
    { ElementType::Count, ElementType::Fire,   6, ElementType::Empty },
};
