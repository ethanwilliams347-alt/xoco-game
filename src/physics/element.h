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

    // ARGB8888, base colour plus per-cell jitter.
    //
    // **Transparent, because the default type is `Empty` and the two have to
    // agree.** This was `0xFF000000` - opaque black - which was invisible until
    // V1 made Empty transparent and put a backdrop behind it, and then wrong
    // everywhere at once: `swap_elements` moves whole Elements, so a grain
    // leaving a cell swaps a *default-constructed* Empty into its place, and
    // that Empty carried opaque black into `pixels`. Every cell anything had
    // ever moved through was painted black permanently, so a falling stream cut
    // a black wake through the backdrop and a settling pile left a black shadow
    // in the shape of everywhere it had been (PLAYTEST_LOG.md session 1, A8).
    //
    // The colour a cell renders as belongs to `MATERIALS`, and this default is
    // the one place a colour was written down outside it. The static_assert
    // below is what stops the two drifting apart again.
    uint32_t color = 0x00000000;

    // The frame tag of the step this cell was last visited in. A cell is skipped
    // when this equals the grid's current tag, which stops it moving twice in one
    // sweep. Storing the tag rather than a bool means there is no per-step pass to
    // reset it, which matters once most of the world is asleep.
    uint8_t updated_tag = 0;

    // **A per-cell step counter whose meaning is set by the cell's role.** Two
    // things need one and neither can afford a byte of its own:
    //
    //  - For a *structural* cell it is steps spent in unbroken free fall, which
    //    is what a falling piece's speed and its fracture threshold are read
    //    from. It lives on the cell rather than on the piece because a piece has
    //    no identity between steps - it is re-discovered by flood fill every
    //    time, so the cells are the only place to keep anything.
    //  - For a *Fire* cell it is fuel remaining: steps of burning still owed by
    //    whatever this flame came from, seeded at ignition out of the burning
    //    material's `burn_duration` (PLAYTEST_LOG.md session 1, A3/A4).
    //
    // Sharing is safe because the two roles cannot occur in the same cell: Fire
    // is a Gas and is never structural, and that is asserted below rather than
    // trusted. Every transition between the roles goes through `place()`, which
    // builds a fresh Element and so zeroes this - a Wood cell that has been
    // falling does not carry its fall ticks over as fuel when it ignites.
    //
    // Zero for everything else. Powders and fluids move one cell per step by
    // their own rules and have no use for a clock.
    uint8_t ticks = 0;

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

// ticks and temperature are meant to be free - they sit in padding the
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
// ticks(1) + temperature(1) + piece_tag(1) is 12 with nothing spare, so the
// next field added here is the first one that actually costs something - 500 KB
// at the target resolution and a wider stride through the hot loop. Anything
// after this either earns that outright or waits for P1 to split the array,
// which is the change that makes the question a different one.
static_assert(sizeof(Element) <= 12, "Element grew - price the extra memory traffic before accepting it");

// **A default-constructed cell must render as what its default type renders as.**
//
// The two are written down in different files, by different kinds of edit - one
// is a member initialiser here, the other is a row in `MATERIALS` - and nothing
// about touching either one suggests reading the other. They disagreed for as
// long as V1 had existed: Empty's row went transparent so a backdrop could show
// through it, and this default stayed opaque black, so every cell that anything
// moved through was painted black by `swap_elements` and the backdrop was
// destroyed wherever the world had been alive.
//
// Exactly the hazard the correctness pass named - data-driven design moves the
// danger into the relationships between rows, and those relationships have no
// compiler behind them unless one is written. This is that compiler, and it is
// the same shape as the spawn-temperature check at the bottom of `reaction.h`.
static_assert(Element{}.color == material_of(ElementType::Empty).color,
              "a default-constructed Element must carry Empty's colour from MATERIALS");

// **Fire may not be structural, because `ticks` means two things.**
//
// The byte is fall time for a structural cell and fuel for a Fire cell, and the
// whole argument that one byte can serve both is that no cell is ever both at
// once. That is true today for a reason no reader would think to check - Fire is
// a Gas - and it is a single word in a MATERIALS row away from stopping being
// true. If it ever did, a burning cell in free fall would spend its fuel on the
// fracture threshold and its fall time on burning out, and neither system would
// report anything wrong; the fire would simply last a random length of time.
static_assert(!material_of(ElementType::Fire).structural,
              "Fire shares Element::ticks with the structural fall clock and must not be structural");
