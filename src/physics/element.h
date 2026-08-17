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
    //  - For a *Steam* cell it is steps until it condenses (E9's steam half).
    //    The same role as Fire's and deliberately not a third one: both are
    //    gases whose whole existence is a countdown, and neither can ever be
    //    structural. See `TickRole` below, which is what says so to the
    //    compiler now that the byte has more than one claimant of that kind.
    //
    // Sharing is safe because the roles cannot occur in the same cell: a
    // lifetime belongs to a Gas and a Gas is never structural, and that is
    // asserted below rather than trusted. Every transition between the roles
    // goes through `place()`, which builds a fresh Element and so zeroes this -
    // a Wood cell that has been falling does not carry its fall ticks over as
    // fuel when it ignites.
    //
    // **A third role was proposed for this byte and rejected on 2026-08-13.**
    // E10 and E5a both planned to claim it as a packed velocity - four bits of
    // signed dx and four of dy - and the instrumentation sitting measured that
    // representation instead of arguing about it. Four bits of whole cells per
    // step cannot hold a gravity increment at all: one step of Player::GRAVITY is
    // 5/36 of a cell per step, which truncates to zero in an integer, so a thrown
    // grain flies in a straight line forever. `velocity_probe` flies exactly that
    // and it never comes back down. Velocity went to the three free bytes at
    // offsets 1-3 instead, so **this byte keeps the two roles it has** and E5a
    // does not touch it.
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
// **"That was the last free byte" was written here and is wrong. There are three
// more, and they are in the sentence that says so.**
//
// The claim read: *type(1) + 3 pad + color(4) + updated_tag(1) + ticks(1) +
// temperature(1) + piece_tag(1) is 12 with nothing spare, so the next field
// added here is the first one that actually costs something - 500 KB at the
// target resolution and a wider stride through the hot loop.* It is left above
// rather than deleted because the error is worth recognising again: **the "3 pad"
// is counted in the arithmetic and then not counted as space.** `type` is one
// byte and `color` needs four-byte alignment, so offsets 1, 2 and 3 are a hole
// that no field occupies, and a field *declared between them* goes in for free.
// The tail hole after `updated_tag` is what E2 and E3 spent, and it is the only
// one anybody was looking at.
//
// Measured, not counted, at the instrumentation sitting on 2026-08-13 - which is
// the point, since this struct's byte count has now been got wrong twice by
// counting fields. `velocity_probe` prints `sizeof` and `offsetof` for the
// candidates: **+1 byte after `piece_tag` is 16 bytes; +1, +2 or +3 bytes after
// `type` is still 12; +4 is 16.** Note also that "a thirteenth byte" was never an
// available option - alignment rounds 13 up to 16, so appending costs *four*
// bytes per cell, 7.9 MB at 1920x1080 and +33% memory traffic. See PERFORMANCE.md,
// which also records that the 16-byte struct measured 10% *faster* on `cascading`
// and that this weakens the "wider stride" half of the claim above rather than
// licensing anyone to spend the memory.
//
// **So the correct statement is: three bytes remain, at offsets 1-3, and they are
// spoken for.** E5a's per-cell velocity is what goes there - a signed 4.4 byte
// per axis plus a nibble per axis of sub-cell remainder is exactly three - and
// the decision that put them there is in ROADMAP.md and ENGINEERING_NOTES.md.
// (It said ROADMAP_ITEMS.md until W4 merged that file away on 2026-08-17.)
// After that the struct really is full, and the next field genuinely costs four
// bytes rather than one.
//
// The assertion below does not move either way. It was the guard when the
// arithmetic above was believed and it is the guard now that it is not, which is
// the argument for having written it as an assertion rather than as a comment:
// it is also what makes the front hole *safe* to use on a machine whose ABI packs
// this struct differently, since a layout with no hole fails the build here
// instead of silently growing.
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

// **What `Element::ticks` means, per material, as something a compiler can
// read.**
//
// This used to be a single `static_assert` naming Fire, and it was right for as
// long as Fire was the only cell with a lifetime. E9's steam half is the second,
// which is the point at which "no cell is ever both at once" stops being a fact
// about one row and becomes a rule about the table. A rule about a table wants a
// lookup and an assertion over every row, for the reason
// `.claude/rules/simulation.md` gives about data-driven design: the danger lives
// in the relationships between rows, and no compiler sees it unless one is
// written.
//
// **Deliberately a function over `ElementType` rather than a column on
// MATERIALS.** A column would be authored per row and could disagree with
// `structural` on the very same line; this cannot, because `structural` is where
// it reads the answer from. The cost is that a material with a lifetime is named
// here, which is a real one and is the reason `Lifetime` is written as the
// exception list rather than as the default.
enum class TickRole : uint8_t {
    None,      // powders and fluids: no clock, and the byte is unused
    FallClock, // structural cells: steps of unbroken free fall
    Lifetime,  // gases that expire: steps left before they are gone
};

inline constexpr TickRole tick_role(ElementType type) {
    if (material_of(type).structural) return TickRole::FallClock;
    if (type == ElementType::Fire || type == ElementType::Steam) return TickRole::Lifetime;
    return TickRole::None;
}

// **A cell with a lifetime may not be structural, because `ticks` means two
// things.**
//
// The byte is fall time for a structural cell and a countdown for a gas, and the
// whole argument that one byte serves both is that no cell is ever both at once.
// That is true today for a reason no reader would think to check - Fire and
// Steam are Gases - and it is a single word in a MATERIALS row away from
// stopping being true. If it ever did, a burning cell in free fall would spend
// its fuel on the fracture threshold and its fall time on burning out, and
// neither system would report anything wrong; the fire would simply last a
// random length of time.
//
// The check is now over every row rather than over Fire's, so the next material
// given a lifetime is covered by it without anyone remembering to widen it -
// which is the half of the original that did not survive contact with a second
// claimant.
namespace detail {
constexpr bool lifetimes_are_never_structural() {
    for (int i = 0; i < static_cast<int>(ElementType::Count); ++i) {
        const ElementType t = static_cast<ElementType>(i);
        if (tick_role(t) == TickRole::Lifetime && material_of(t).structural) return false;
    }
    return true;
}
} // namespace detail

static_assert(detail::lifetimes_are_never_structural(),
              "a material with a ticks lifetime is structural and would collide with the fall clock");
