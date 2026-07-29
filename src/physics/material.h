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
    Rubble,
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

    // What this material breaks into when it loses structural support.
    // ElementType::Count means "never collapses" and is the right answer for
    // everything that already falls on its own.
    ElementType debris;
};

// Indexed by ElementType. Keep rows in the same order as the enum.
inline constexpr Material MATERIALS[] = {
    // name      colour       jitter  move              density  spread  debris
    {  "Empty",  0xFF000000,      0,  MoveKind::Static,       0,      0,  ElementType::Count  },
    {  "Sand",   0xFFEEDD82,     18,  MoveKind::Powder,     150,      0,  ElementType::Count  },
    {  "Water",  0xFF4444FF,     10,  MoveKind::Liquid,     100,      5,  ElementType::Count  },
    // The two structural materials, and the only two that can collapse. Both
    // break into the same Rubble: a second debris type is one more table row
    // whenever splintered wood is worth telling apart from broken stone.
    {  "Wall",   0xFF888888,     12,  MoveKind::Static,   32000,      0,  ElementType::Rubble },
    {  "Wood",   0xFF6B4423,     14,  MoveKind::Static,   32000,      0,  ElementType::Rubble },
    {  "Oil",    0xFF3A2E22,      8,  MoveKind::Liquid,      60,      3,  ElementType::Count  },
    {  "Steam",  0xFFBFD8E8,      6,  MoveKind::Gas,        -20,      3,  ElementType::Count  },
    // Denser (less negative) than Steam so flame stays under a steam layer
    // instead of punching through it - visually reads as fire boiling water.
    {  "Fire",   0xFFFF6A00,     24,  MoveKind::Gas,        -10,      4,  ElementType::Count  },
    // Denser than Sand so a collapse settles through a sand pile rather than
    // perching on top of it. Debris is Count: rubble has already fallen apart,
    // and rubble that could collapse again would be a loop.
    {  "Rubble", 0xFF7A6E62,     16,  MoveKind::Powder,     160,      0,  ElementType::Count  },
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
// True if this material holds itself up and can therefore fall down when it
// stops being held up. Derived from the debris column so that, like solidity,
// it stays one table rather than a second list kept in sync with the first.
inline constexpr bool is_collapsible(ElementType type) {
    return material_of(type).debris != ElementType::Count;
}

inline constexpr bool is_solid(ElementType type) {
    if (type == ElementType::Empty) return false;
    const MoveKind move = material_of(type).move;
    return move == MoveKind::Static || move == MoveKind::Powder;
}
