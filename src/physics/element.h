#pragma once
#include <cstdint>

enum class ElementType : uint8_t {
    Empty = 0,
    Sand,
    Water,
    Wall
};

struct Element {
    ElementType type = ElementType::Empty;
    uint32_t color = 0x00000000; // ARGB8888 format
    bool has_updated = false;    // Used to prevent updating the same pixel twice in one frame
};
