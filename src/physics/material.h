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
};

// Indexed by ElementType. Keep rows in the same order as the enum.
inline constexpr Material MATERIALS[] = {
    // name      colour       jitter  move              density  spread  structural
    {  "Empty",  0xFF000000,      0,  MoveKind::Static,       0,      0,  false },
    {  "Sand",   0xFFEEDD82,     18,  MoveKind::Powder,     150,      0,  false },
    {  "Water",  0xFF4444FF,     10,  MoveKind::Liquid,     100,      5,  false },
    // The two structural materials. Density is what lets an unsupported slab
    // sink through any fluid it lands in rather than perching on top of it.
    {  "Wall",   0xFF888888,     12,  MoveKind::Static,   32000,      0,  true  },
    {  "Wood",   0xFF6B4423,     14,  MoveKind::Static,   32000,      0,  true  },
    {  "Oil",    0xFF3A2E22,      8,  MoveKind::Liquid,      60,      3,  false },
    {  "Steam",  0xFFBFD8E8,      6,  MoveKind::Gas,        -20,      3,  false },
    // Denser (less negative) than Steam so flame stays under a steam layer
    // instead of punching through it - visually reads as fire boiling water.
    {  "Fire",   0xFFFF6A00,     24,  MoveKind::Gas,        -10,      4,  false },
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
