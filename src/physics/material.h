#pragma once
#include <cstdint>
#include <cstddef>

enum class ElementType : uint8_t {
    Empty = 0,
    Sand,
    Water,
    Wall,
    Wood,
    Oil,
    Steam,
    Fire,
    // Wood that has caught. **A state of the fuel, not a kind of fire**, and the
    // distinction is the whole of E9's rebuild: this cell is what holds the burn
    // duration, stays structural, keeps heating its neighbours, and throws
    // short-lived Fire into the air around it. The flame is the thing you see;
    // this is the thing that is actually burning.
    Charred,
    Count
};

// How a material moves. The physics step is written against these four
// behaviours rather than against individual materials, so adding a material
// is a new row in MATERIALS below, not a new branch in the update loop.
enum class MoveKind : uint8_t {
    Static, // never moves (wall, wood)
    Powder, // falls straight down, piles into a slope (sand)
    Liquid, // falls, then spreads out to find its level (water, oil)
    Gas     // rises, then spreads (steam)
};

struct Material {
    const char* name;
    uint32_t color;       // ARGB8888 base colour
    uint8_t color_jitter; // per-cell brightness variation, 0 = flat
    MoveKind move;
    int16_t density;      // denser sinks through lighter; Empty is 0, gases negative
    uint8_t spread;       // cells a liquid/gas may travel sideways per step

    // Whether this material holds itself and its neighbours up, and therefore
    // has to fall as one piece when nothing holds *it* up. True for the
    // structural materials only; everything that already falls on its own is
    // false.
    bool structural;

    // --- thermal ---

    // How readily heat moves through this material, 0-255. Used for both
    // conduction between neighbours (the pair moves heat at the *lower* of the
    // two, so an insulator stops a conductor) and for the bleed back to
    // ambient, so one number sets both how fast a material heats and how fast
    // it forgets. Zero means the material takes no part in heat at all, which
    // is what `Empty` is: air is not simulated, so heat travels through matter
    // in contact and nowhere else. That is a deliberate simplification and it
    // is the reason the thermal pass is affordable - a settled pool of water is
    // hundreds of cells, the air above it is tens of thousands.
    uint8_t conductivity;

    // Temperature a freshly placed cell of this material gets. Zero means "keep
    // whatever the spot was already at", which is the right default: heat
    // belongs to the place, so material dug out of a hot wall arrives hot.
    // Non-zero is for materials that are hot *by definition* - there is no such
    // thing as cold Fire, and Steam that is not above condensing point is
    // water. `Empty` names ambient explicitly so that erasing a cell also
    // clears the heat that was in it.
    uint8_t spawn_temperature;

    // Temperature this material holds itself at, every step, regardless of what
    // it is losing to its surroundings. Zero for everything that is merely warm.
    // Fire is the one thing in the table that is a source rather than a
    // conductor - it is what heats the world, not something the world heats -
    // and this is what stops a flame being quenched by the cold wall it is
    // sitting against.
    uint8_t heat_source;

    // How many steps of Fire one cell of this material is worth when it burns.
    // Read at the moment of ignition and stored on the resulting flame, so this
    // is a property of the *fuel* and not of the fire - which is what makes
    // wood and oil burn differently at all. Zero for everything that is not
    // combustible, and zero for Fire itself: a flame conjured with the brush has
    // nothing behind it and dies on the burnout roll, the way it always did.
    //
    // What a burning cell of this material throws into the empty space beside
    // it, and how often. `Count` means "nothing", which is every row but one.
    //
    // **This is a column rather than a rule about Charred, and that is the whole
    // reason fire stays data-driven.** Emission is the mechanism that makes
    // flame a separate thing from fuel - the fuel sits and burns, the flame is
    // manufactured beside it and dies within a dozen steps - and if it were
    // written as "Charred throws Fire" then the next flammable material would
    // need engine work rather than a row.
    //
    // It also buys a behaviour nobody has to write: emission targets *Empty*
    // neighbours, so a buried cell has nowhere to emit and does not visibly
    // burn. Fire being a layer on exposed surfaces rather than something that
    // fills a volume is not a rule in this engine, it is what this column does
    // when a cell has no air next to it.
    ElementType emits;
    uint8_t emit_chance; // percent, per step, per burning cell
};

// Indexed by ElementType. Keep rows in the same order as the enum.
//
// The colours are chosen as a *set*, not row by row (V2). Two things force
// that. Reaction products take their colour from this table, so wood burns into
// whatever Fire's row says and nothing about the burning wood carries over -
// rows that were picked independently show that seam plainly. And every
// material is now composited over V1's backdrop rather than over black, so what
// a row reads as depends on what is behind it: the backdrop is a cool dark
// blue, so the world sits warm and desaturated against it and the two things
// that must never be missed on screen - Fire and Water - are the only rows that
// keep any real saturation.
//
// Jitter is small everywhere it is not zero. It was 8-24 across the table,
// which is per-cell random noise, and random noise does not combine with the
// hand-placed ordered dithering the art is going to use - it fights it, and the
// result reads as static rather than as texture. What is left is enough to stop
// a flat fill looking like a flat fill and no more. Fire keeps the widest range
// of anything, because there its variation is the thing being drawn.
inline constexpr Material MATERIALS[] = {
    // name      colour       jitter  move              density  spread  structural  cond  spawn  source  emits                 emit%
    // Empty is the one row whose colour is fully transparent rather than a
    // colour at all (V1): nothing is there, so whatever is drawn behind the
    // cell texture shows through. Every other row is opaque - alpha is a
    // property of *absence* here, not a per-material effect, and a translucent
    // material would need the pixel buffer to composite rather than overwrite.
    {  "Empty",  0x00000000,      0,  MoveKind::Static,       0,      0,  false,        0,    20,      0,  ElementType::Count,      0 },
    {  "Sand",   0xFFC6A970,      6,  MoveKind::Powder,     150,      0,  false,       30,     0,      0,  ElementType::Count,      0 },
    // Water is teal rather than blue, and that is a compositing decision rather
    // than a taste one: the backdrop behind it is a cool blue, and a blue liquid
    // seen through a gap in the terrain read as a hole in the world.
    {  "Water",  0xFF2E7F96,      5,  MoveKind::Liquid,     100,      5,  false,      120,     0,      0,  ElementType::Count,      0 },
    // The two structural materials. Density is what lets an unsupported slab
    // sink through any fluid it lands in rather than perching on top of it.
    // Wall conducts poorly on purpose: a good conductor here would make every
    // wall a heat sink large enough to quench any fire touching it, since a
    // wall is usually the biggest connected body in a scene.
    {  "Wall",   0xFF6F6A63,      5,  MoveKind::Static,   32000,      0,  true,        30,     0,      0,  ElementType::Count,      0 },
    // Wood is fuel, and what it turns into when lit is `Charred` rather than
    // `Fire` - the burning is a state of this cell, and the flame is something
    // thrown off it. How long it burns is the decay chance on Charred's row in
    // REACTIONS, not a column here.
    {  "Wood",   0xFF6B4E33,      5,  MoveKind::Static,   32000,      0,  true,        90,     0,      0,  ElementType::Count,      0 },
    // **Oil flashes straight to Fire and does not smoulder, and that boundary is
    // deliberate.** A burning cell holds its state in the cell, and Oil is a
    // Liquid - a smouldering oil cell would carry that state through
    // `swap_elements` and across every fluid rule, which is a much larger
    // question than the one E9 is answering. Only Static materials get a burning
    // state; movable fuels ignite and are gone. It also reads correctly: a spill
    // goes up all at once in a sheet, where a beam smoulders.
    {  "Oil",    0xFF2C2620,      3,  MoveKind::Liquid,      60,      3,  false,       70,     0,      0,  ElementType::Count,      0 },
    // **Spawns at 88, below the coldest ignition point in REACTIONS, and that
    // bound is enforced at the bottom of reaction.h rather than remembered.**
    //
    // It used to spawn at 220, picked for lifetime alone - steam's life is
    // exactly the span between this number and its condensing point, so a hot
    // spawn was the only way to make a puff last. The side effect was that
    // steam sat 100 degrees above Wood's ignition point and 130 above Oil's,
    // which made it a *fire-starter*: a pocket of steam under a wooden ceiling
    // burned 17 of 20 cells with no flame anywhere in the world. Both ways of
    // making steam - boiling, and water dousing a flame - produced it, so
    // putting a fire out could start a bigger one. It needed steam to be
    // confined to show up, which is why open-air tests never caught it and
    // authored terrain would have.
    //
    // Conductivity cannot fix that, only temperature can: heat_flow has a floor
    // of one unit per step, so any gap of two or more transfers in full sooner
    // or later. A conductor that is hotter than an ignition point *is* an
    // ignition source, eventually.
    //
    // The cost is that a puff now lives ~60 steps rather than ~140, since that
    // span is all there is. The gain beyond the bug: water boiling at 100 into
    // steam at 88 now *absorbs* heat instead of creating 120 units of it per
    // cell out of nothing, which is both what a latent heat of vaporisation
    // does and the reason a kettle on a fire no longer warms the room faster
    // than the fire does.
    {  "Steam",  0xFFC4D2D8,      4,  MoveKind::Gas,        -20,      3,  false,       40,    88,      0,  ElementType::Count,      0 },
    // Denser (less negative) than Steam so flame stays under a steam layer
    // instead of punching through it - visually reads as fire boiling water.
    // The only heat source in the table.
    {  "Fire",   0xFFF07A22,     16,  MoveKind::Gas,        -10,      4,  false,      200,   250,    250,  ElementType::Count,      0 },
    // **The cell that is actually on fire.** Structural, because a burning
    // ceiling that drops the instant it catches is worse than one that burns
    // through first, and because it is still wood. That single `true` is what
    // forces its lifetime to be a decay chance rather than a countdown: a
    // structural cell already spends `Element::ticks` on the free-fall clock,
    // `Element` has no spare byte, and the `static_assert` at the bottom of
    // `element.h` is what turns that collision into a compile error rather than
    // a fire that lasts a random length of time. The variance that falls out of
    // a chance is a gain, not a consolation - a plank's cells stop winking out
    // in lockstep.
    //
    // Heat source at 200 rather than a mere conductor, and that is what actually
    // spreads fire now: it sits above Wood's 120 ignition point, so a charred
    // cell brings its neighbours up to catching regardless of what is trying to
    // cool it. Flame does not do this job any more - flame is thrown off and
    // dies within a dozen steps, far too briefly to heat anything through.
    //
    // Colour is wood with the life taken out of it, and it has to read as *dark*
    // against the flame it is emitting, or the fire and the thing burning become
    // one shape again - which is the exact confusion this material exists to
    // resolve.
    {  "Charred",0xFF2A211B,      6,  MoveKind::Static,   32000,      0,  true,        90,     0,    200,  ElementType::Fire,      25 },
};

static_assert(sizeof(MATERIALS) / sizeof(MATERIALS[0]) == static_cast<size_t>(ElementType::Count),
              "MATERIALS must have exactly one row per ElementType");

inline constexpr const Material& material_of(ElementType type) {
    return MATERIALS[static_cast<size_t>(type)];
}

// What the player character collides with.
//
// Derived from the movement behaviour rather than listed per material, for the
// same reason movement itself is: a new MATERIALS row then gets correct
// collision for free instead of needing a second table kept in sync with the
// first. Anything that holds its shape is solid -- static terrain, and powders,
// which pile up and can be stood on. Liquids and gases are things you fall
// through. Empty is Static in the table only because it never moves, so it has
// to be excluded explicitly.
// True if this material is part of a structure: it holds its neighbours up, and
// when nothing holds it up in turn, the whole connected piece falls together
// rather than each cell falling on its own.
inline constexpr bool is_structural(ElementType type) {
    return material_of(type).structural;
}

inline constexpr bool is_solid(ElementType type) {
    if (type == ElementType::Empty) return false;
    const MoveKind move = material_of(type).move;
    return move == MoveKind::Static || move == MoveKind::Powder;
}
