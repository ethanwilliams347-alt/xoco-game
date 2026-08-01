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

    // Temperature window the target cell must be inside for this row to be
    // eligible at all, inclusive on both ends. 0-255 is "any temperature", and
    // it is what the rows that are genuinely about contact rather than heat use.
    //
    // **This is where the ignition point lives, rather than as a column on
    // MATERIALS.** A threshold is a property of a transformation, not of a
    // substance: the same Water row that boils at 100 would need a second
    // number the moment anything else about water were temperature-gated, and
    // Fire is already both a target and a catalyst with no threshold of its own.
    // Keeping it here also means the whole rule stays readable on one line.
    uint8_t min_temp;
    uint8_t max_temp;
};

// Heat, not luck, is what spreads a fire now. Wood and Oil ignite when they get
// hot enough, full stop - conduction from a neighbouring flame is what gets
// them there, and how long that takes is set by their conductivity in
// MATERIALS. The dice are left on exactly one row, Fire's own burnout, which is
// a lifetime rather than a threshold and has nothing to be gated on.
inline constexpr Reaction REACTIONS[] = {
    // Dousing keeps its place at the top and its chance, and is deliberately
    // *not* temperature-gated: water hitting a flame puts it out because it is
    // water, and gating it on heat would mean a cold splash did nothing.
    { ElementType::Water, ElementType::Fire,  90, ElementType::Steam,   0, 255 },
    { ElementType::Count, ElementType::Wood, 100, ElementType::Fire,  120, 255 },
    { ElementType::Count, ElementType::Oil,  100, ElementType::Fire,   90, 255 },
    { ElementType::Count, ElementType::Water,100, ElementType::Steam, 100, 255 },
    { ElementType::Count, ElementType::Steam,100, ElementType::Water,   0,  80 },
    { ElementType::Count, ElementType::Fire,   6, ElementType::Empty,   0, 255 },
};
