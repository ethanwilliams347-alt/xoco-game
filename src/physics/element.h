#pragma once
#include <cstdint>
#include "material.h"

// What an undisturbed cell sits at, and what the whole world starts at. The
// scale is chosen to read as degrees Celsius so the numbers in MATERIALS and
// REACTIONS mean something to a person: 20 is room temperature, water boils at
// 100, and 255 - the top of the byte - is a little above what an open flame
// holds. That is a naming convenience, not a physical claim; nothing here
// models energy, only a number that flows downhill between neighbours.
inline constexpr uint8_t AMBIENT_TEMPERATURE = 20;

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

    // How hot this cell is. Ambient unless something has heated it, and it
    // decays back to ambient on its own, so a world nobody has set fire to
    // holds one value everywhere and generates no thermal work at all.
    //
    // On the cell rather than on the material for the obvious reason - it is
    // the one thermal quantity that varies per cell - and it moves with the
    // cell for free, since swap_elements moves whole Elements. A grain of sand
    // pulled out of a fire carries its heat with it.
    uint8_t temperature = AMBIENT_TEMPERATURE;

    // Which broken-off piece of structure this cell belongs to. Zero for
    // everything that has never been fractured, which is almost everything.
    //
    // The support flood fill only crosses between two structural cells whose
    // tags match, so a crack is *stored as a disagreement between two cells*
    // rather than as a line somewhere between them. That is the whole reason
    // this is a per-cell field and not a side table of edges: cracks have to
    // survive the piece moving, and a whole Element is what swap_elements
    // carries, so a tag rides along for free where an edge would have to be
    // rewritten every cell of every fall.
    //
    // Fracture is the only thing that ever writes a non-zero value here, and it
    // never writes one back to zero: two pieces that have come apart do not
    // re-weld by touching. The counter is a byte and wraps, so two unrelated
    // pieces can end up sharing a tag and welding if they happen to meet. That
    // is a *missed* fracture, which is the harmless direction, and it is the
    // same asymmetry MAX_SUPPORT_CELLS is chosen on.
    uint8_t piece_tag = 0;
};

// fall_ticks and temperature are meant to be free - they sit in padding the
// struct already had after updated_tag. Worth pinning down, because they would
// stop being free the moment they pushed the struct over an alignment
// boundary: this array is one entry per cell of the world, and every step walks
// all of the awake part of it, so a byte here is 500 KB and a slower sweep at
// the target resolution.
//
// E2 added temperature on the strength of that padding and E3 added piece_tag,
// and `ENGINEERING_NOTES.md` is the reason this is asserted rather than counted:
// its own 8-vs-12-byte claim was wrong for several revisions, from counting
// fields instead of compiling one.
//
// **That was the last free byte.** type(1) + 3 pad + color(4) + updated_tag(1) +
// fall_ticks(1) + temperature(1) + piece_tag(1) is 12 with nothing spare, so the
// next field added here is the first one that actually costs something - 500 KB
// at the target resolution and a wider stride through the hot loop. Anything
// after this either earns that outright or waits for P1 to split the array,
// which is the change that makes the question a different one.
static_assert(sizeof(Element) <= 12, "Element grew - price the extra memory traffic before accepting it");
