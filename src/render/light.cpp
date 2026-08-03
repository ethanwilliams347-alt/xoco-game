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
// solid figure is what makes a wall a wall.
//
// **0.72, down from 0.86, and the old figure is most of why the first build was
// blown out.** 0.86 per four cells is 0.963 per cell, which still leaves half
// brightness twenty cells from the flame and a tenth of it sixty cells out - so
// a fire of any size washed the entire screen. Light does not carry that far;
// the number reads as gentle and compounds into something enormous. At 0.72 the
// same glow is at a third by twenty cells and gone by forty.
constexpr float TRANSMIT_CLEAR = 0.72f;
constexpr float TRANSMIT_SOLID = 0.20f;

// Peak emission from a block that is entirely on fire, before tone mapping.
//
// **Was 1.7, chosen as deliberate headroom so the brightest cells would clip to
// white.** That reasoning was wrong twice over: this layer is composited
// *additively* over a scene that is already bright, so the clipping happens in
// the blend whatever this value is, and deliberately driving a signal past its
// ceiling destroys every gradient above the ceiling rather than only the peak.
// The screenshot that prompted this had a flat white plateau where the fire was.
constexpr float MAX_EMISSION = 1.0f;

// **A block emits according to how much of it is burning, not only how hot its
// hottest cell is.** Taking the maximum temperature is right for deciding
// *whether* a block is a light source - a single flame cell in a block of air
// must not be averaged away - but using it for brightness too means one stray
// flame lights as hard as a solid wall of fire. Combined with max-propagation
// below, that is what turned a ragged, mostly-empty flame front into a solid
// glowing slab: every block the fire's edge touched emitted at full strength.
//
// The floor keeps a lone flame clearly visible; the rest scales with coverage.
constexpr float COVERAGE_FLOOR = 0.30f;

// Light below one step of an 8-bit channel is not light, it is arithmetic. Cut
// to zero during propagation rather than at the end, so a glow has a definite
// edge instead of an exponential tail that is invisible but still costs
// iterations - and so the reach of a light is bounded by its own falloff rather
// than only by ITERATIONS.
constexpr float MIN_VISIBLE = 1.0f / 255.0f;

// An iteration that moves no channel by more than this has converged, and the
// remaining iterations would be arithmetic nobody can see. Well below one step
// of an 8-bit channel.
constexpr float CONVERGED = 1.0f / 512.0f;

// **Tone mapping, and the reason a hard clamp was not enough.** Clamping maps
// every value at or above 1.0 to the same white, so the whole interior of a fire
// - which is exactly where the most light is - collapses into one flat shape
// with no structure in it. That is what "blown out and overexposed" describes.
//
// Reinhard's curve compresses instead: it is close to linear while dim, rolls
// off smoothly as it brightens, and approaches 1 without ever reaching it, so
// there is always some difference between two bright values. It costs one divide
// per channel per block.
float tone_map(float v) {
    return v <= 0.0f ? 0.0f : v / (1.0f + v);
}

} // namespace

LightField::LightField(int region_cells_w, int region_cells_h)
    : block_cols(std::max(1, (region_cells_w + BLOCK - 1) / BLOCK)),
      block_rows(std::max(1, (region_cells_h + BLOCK - 1) / BLOCK)) {
    const size_t count = static_cast<size_t>(block_cols) * block_rows;
    emission.assign(count, Rgb{});
    front.assign(count, Rgb{});
    back.assign(count, Rgb{});
    transmit.assign(count, TRANSMIT_CLEAR);
    transmit_diag.assign(count, TRANSMIT_CLEAR);
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
            int glowing = 0;
            for (int cy = 0; cy < BLOCK; ++cy) {
                for (int cx = 0; cx < BLOCK; ++cx) {
                    const Element cell = grid.get_element(origin_x + bx * BLOCK + cx,
                                                          origin_y + by * BLOCK + cy);
                    hottest = std::max(hottest, cell.temperature);
                    occluded += occlusion_of(cell.type);
                    if (cell.temperature > GLOW_THRESHOLD) ++glowing;
                }
            }

            const float opacity = occluded / CELLS_PER_BLOCK;
            const float k = TRANSMIT_CLEAR + (TRANSMIT_SOLID - TRANSMIT_CLEAR) * opacity;
            transmit[i] = k;
            // k raised to the diagonal's length. sqrt(2) is 1.414 and k*sqrt(k)
            // is k^1.5 - within a couple of percent across the whole range this
            // takes, and a sqrt rather than a pow in a per-block loop.
            transmit_diag[i] = k * std::sqrt(k);

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
            const float coverage = static_cast<float>(glowing) / CELLS_PER_BLOCK;
            const float strength = MAX_EMISSION * t * std::sqrt(t) *
                                   (COVERAGE_FLOOR + (1.0f - COVERAGE_FLOOR) * coverage);
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
                // Orthogonal and diagonal neighbours are gathered separately
                // because they are paid for at different rates - see
                // `transmit_diag`. Folded in as two maxima rather than eight so
                // the two costs stay distinguishable.
                const bool l = bx > 0, r = bx < block_cols - 1;
                const bool u = by > 0, d = by < block_rows - 1;
                const int W = block_cols;

                Rgb best{};
                const auto fold = [&best, this](size_t j) {
                    best.r = std::max(best.r, front[j].r);
                    best.g = std::max(best.g, front[j].g);
                    best.b = std::max(best.b, front[j].b);
                };
                if (l) fold(i - 1);
                if (r) fold(i + 1);
                if (u) fold(i - W);
                if (d) fold(i + W);

                Rgb diag{};
                const auto fold_diag = [&diag, this](size_t j) {
                    diag.r = std::max(diag.r, front[j].r);
                    diag.g = std::max(diag.g, front[j].g);
                    diag.b = std::max(diag.b, front[j].b);
                };
                if (l && u) fold_diag(i - W - 1);
                if (r && u) fold_diag(i - W + 1);
                if (l && d) fold_diag(i + W - 1);
                if (r && d) fold_diag(i + W + 1);

                const float k = transmit[i];
                const float kd = transmit_diag[i];
                const auto carry = [k, kd](float e, float n, float nd) {
                    const float v = std::max(e, std::max(n * k, nd * kd));
                    return v < MIN_VISIBLE ? 0.0f : v;
                };
                const Rgb next{
                    carry(emission[i].r, best.r, diag.r),
                    carry(emission[i].g, best.g, diag.g),
                    carry(emission[i].b, best.b, diag.b),
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
            return static_cast<uint32_t>(std::clamp(tone_map(v), 0.0f, 1.0f) * 255.0f + 0.5f);
        };
        texels[i] = 0xFF000000u | (channel(front[i].r) << 16) |
                    (channel(front[i].g) << 8) | channel(front[i].b);
    }
}
