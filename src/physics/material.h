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
};

// Indexed by ElementType. Keep rows in the same order as the enum.
inline constexpr Material MATERIALS[] = {
    // name      colour       jitter  move              density  spread  structural  cond  spawn  source
    {  "Empty",  0xFF000000,      0,  MoveKind::Static,       0,      0,  false,        0,    20,      0 },
    {  "Sand",   0xFFEEDD82,     18,  MoveKind::Powder,     150,      0,  false,       30,     0,      0 },
    {  "Water",  0xFF4444FF,     10,  MoveKind::Liquid,     100,      5,  false,      120,     0,      0 },
    // The two structural materials. Density is what lets an unsupported slab
    // sink through any fluid it lands in rather than perching on top of it.
    // Wall conducts poorly on purpose: a good conductor here would make every
    // wall a heat sink large enough to quench any fire touching it, since a
    // wall is usually the biggest connected body in a scene.
    {  "Wall",   0xFF888888,     12,  MoveKind::Static,   32000,      0,  true,        30,     0,      0 },
    {  "Wood",   0xFF6B4423,     14,  MoveKind::Static,   32000,      0,  true,        90,     0,      0 },
    {  "Oil",    0xFF3A2E22,      8,  MoveKind::Liquid,      60,      3,  false,       70,     0,      0 },
    // Spawns hot, holds no heat of its own, and conducts slowly - so a puff of
    // steam cools over a couple of hundred steps and condenses back to water
    // rather than lasting forever or vanishing on the step it is made.
    {  "Steam",  0xFFBFD8E8,      6,  MoveKind::Gas,        -20,      3,  false,       40,   220,      0 },
    // Denser (less negative) than Steam so flame stays under a steam layer
    // instead of punching through it - visually reads as fire boiling water.
    // The only heat source in the table.
    {  "Fire",   0xFFFF6A00,     24,  MoveKind::Gas,        -10,      4,  false,      200,   250,    250 },
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
