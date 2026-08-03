#include "render/light.h"
#include <algorithm>
#include <cmath>

namespace {

// How much of a block a cell of each kind fills in, for the purpose of stopping
// light. Solid terrain blocks outright; fluids dim what passes through them;
// gases barely count, which is what lets a flame light the smoke above it
// instead of being smothered by it. Empty stops nothing at all - air is not
// simulated here, and it is not rendered as though it were.
float occlusion_of(ElementType type) {
    if (type == ElementType::Empty) return 0.0f;
    switch (material_of(type).move) {
        case MoveKind::Gas:    return 0.15f;
        case MoveKind::Liquid: return 0.50f;
        default:               return 1.00f; // Static and Powder
    }
}

// Light surviving one block of travel, as a function of how full that block is.
// The clear-air figure is what sets the falloff of an unobstructed glow, and the
// solid figure is what makes a wall a wall: at 0.24 a light dies within about
// three blocks of rock, so a fire in a pit spills out of the pit's mouth rather
// than through its sides.
constexpr float TRANSMIT_CLEAR = 0.86f;
constexpr float TRANSMIT_SOLID = 0.24f;

// Headroom above 1.0 so that the cells nearest a flame clip to white rather than
// merely reaching full orange. Fire's own colour is already near the top of the
// channel, and a glow that never oversaturates reads as a coloured wash laid
// over the scene instead of as something bright enough to look at.
constexpr float MAX_EMISSION = 1.7f;

// An iteration that moves no channel by more than this has converged, and the
// remaining iterations would be arithmetic nobody can see. Well below one step
// of an 8-bit channel.
constexpr float CONVERGED = 1.0f / 512.0f;

} // namespace

LightField::LightField(int region_cells_w, int region_cells_h)
    : block_cols(std::max(1, (region_cells_w + BLOCK - 1) / BLOCK)),
      block_rows(std::max(1, (region_cells_h + BLOCK - 1) / BLOCK)) {
    const size_t count = static_cast<size_t>(block_cols) * block_rows;
    emission.assign(count, Rgb{});
    front.assign(count, Rgb{});
    back.assign(count, Rgb{});
    transmit.assign(count, TRANSMIT_CLEAR);
    texels.assign(count, 0xFF000000u);
}

void LightField::update(const Grid& grid, int origin_x, int origin_y) {
    const size_t count = static_cast<size_t>(block_cols) * block_rows;
    constexpr float CELLS_PER_BLOCK = static_cast<float>(BLOCK * BLOCK);

    // --- gather ---
    //
    // One pass over the region's cells, accumulating two things per block: the
    // hottest cell in it, and how much of it is solid. **The hottest and not the
    // average**, because a single flame cell in a block of air is a light source
    // and averaging it against its fifteen empty neighbours would divide it away
    // - which is exactly the case this feature exists for.
    lit = false;
    for (int by = 0; by < block_rows; ++by) {
        for (int bx = 0; bx < block_cols; ++bx) {
            const size_t i = static_cast<size_t>(by) * block_cols + bx;

            uint8_t hottest = 0;
            float occluded = 0.0f;
            for (int cy = 0; cy < BLOCK; ++cy) {
                for (int cx = 0; cx < BLOCK; ++cx) {
                    const Element cell = grid.get_element(origin_x + bx * BLOCK + cx,
                                                          origin_y + by * BLOCK + cy);
                    hottest = std::max(hottest, cell.temperature);
                    occluded += occlusion_of(cell.type);
                }
            }

            const float opacity = occluded / CELLS_PER_BLOCK;
            transmit[i] = TRANSMIT_CLEAR + (TRANSMIT_SOLID - TRANSMIT_CLEAR) * opacity;

            if (hottest <= GLOW_THRESHOLD) {
                emission[i] = Rgb{};
                continue;
            }
            lit = true;

            // Temperature to colour. The ramp is the one a fire actually walks -
            // dull red where something has only just caught, orange through the
            // body of the flame, pale towards white at the top of the byte - and
            // it is computed from temperature rather than read off the cell's
            // own colour on purpose. A Charred cell is nearly black by design, so
            // its rendered colour is the worst possible guide to what it emits;
            // heat is the thing that both the dark ember and the bright flame
            // agree on, and E2 already tracks it.
            const float t = static_cast<float>(hottest - GLOW_THRESHOLD) /
                            static_cast<float>(255 - GLOW_THRESHOLD);
            const float strength = MAX_EMISSION * t * std::sqrt(t);
            emission[i] = Rgb{
                strength,
                strength * (0.34f + 0.46f * t),
                strength * (0.08f + 0.26f * t * t),
            };
        }
    }

    if (!lit) {
        std::fill(texels.begin(), texels.end(), 0xFF000000u);
        return;
    }

    // --- propagate ---
    //
    // Light spreads by taking the brightest of its four neighbours and paying
    // the crossing cost, with its own emission as a floor. **Max rather than a
    // sum or an average**, which is what keeps this stable at any iteration
    // count: a diffusion that adds neighbours together grows without bound if it
    // is run longer, so the look would be a function of ITERATIONS rather than of
    // the scene, and tuning either one would move the other.
    //
    // Double-buffered because an in-place sweep propagates further in the
    // direction it happens to walk, and a fire lighting further to its right than
    // its left is the kind of artefact that is obvious on screen and invisible in
    // the code that caused it.
    front = emission;
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        float largest_change = 0.0f;
        for (int by = 0; by < block_rows; ++by) {
            for (int bx = 0; bx < block_cols; ++bx) {
                const size_t i = static_cast<size_t>(by) * block_cols + bx;

                // Outside the region contributes nothing. The grid reads as Wall
                // past its own edge, so the world's border already stops light
                // on its own; this only matters at the edges of the *view*,
                // where light from off-screen is light the player cannot see the
                // source of anyway.
                Rgb best{};
                if (bx > 0)               best = Rgb{std::max(best.r, front[i - 1].r), std::max(best.g, front[i - 1].g), std::max(best.b, front[i - 1].b)};
                if (bx < block_cols - 1)  best = Rgb{std::max(best.r, front[i + 1].r), std::max(best.g, front[i + 1].g), std::max(best.b, front[i + 1].b)};
                if (by > 0)               best = Rgb{std::max(best.r, front[i - block_cols].r), std::max(best.g, front[i - block_cols].g), std::max(best.b, front[i - block_cols].b)};
                if (by < block_rows - 1)  best = Rgb{std::max(best.r, front[i + block_cols].r), std::max(best.g, front[i + block_cols].g), std::max(best.b, front[i + block_cols].b)};

                const float k = transmit[i];
                const Rgb next{
                    std::max(emission[i].r, best.r * k),
                    std::max(emission[i].g, best.g * k),
                    std::max(emission[i].b, best.b * k),
                };
                largest_change = std::max(largest_change, std::abs(next.r - front[i].r));
                largest_change = std::max(largest_change, std::abs(next.g - front[i].g));
                largest_change = std::max(largest_change, std::abs(next.b - front[i].b));
                back[i] = next;
            }
        }
        front.swap(back);
        if (largest_change < CONVERGED) break;
    }

    // --- pack ---
    for (size_t i = 0; i < count; ++i) {
        const auto channel = [](float v) -> uint32_t {
            return static_cast<uint32_t>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f);
        };
        texels[i] = 0xFF000000u | (channel(front[i].r) << 16) |
                    (channel(front[i].g) << 8) | channel(front[i].b);
    }
}
