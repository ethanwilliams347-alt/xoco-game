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
};

// Indexed by ElementType. Keep rows in the same order as the enum.
inline constexpr Material MATERIALS[] = {
    // name      colour       jitter  move              density  spread
    {  "Empty",  0xFF000000,      0,  MoveKind::Static,       0,      0 },
    {  "Sand",   0xFFEEDD82,     18,  MoveKind::Powder,     150,      0 },
    {  "Water",  0xFF4444FF,     10,  MoveKind::Liquid,     100,      5 },
    {  "Wall",   0xFF888888,     12,  MoveKind::Static,   32000,      0 },
    {  "Wood",   0xFF6B4423,     14,  MoveKind::Static,   32000,      0 },
    {  "Oil",    0xFF3A2E22,      8,  MoveKind::Liquid,      60,      3 },
    {  "Steam",  0xFFBFD8E8,      6,  MoveKind::Gas,        -20,      3 },
};

static_assert(sizeof(MATERIALS) / sizeof(MATERIALS[0]) == static_cast<size_t>(ElementType::Count),
              "MATERIALS must have exactly one row per ElementType");

inline constexpr const Material& material_of(ElementType type) {
    return MATERIALS[static_cast<size_t>(type)];
}
