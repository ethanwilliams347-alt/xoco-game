#pragma once
#include <cstdint>
#include <vector>
#include "physics/grid.h"

// V7 - per-cell emissive lighting, at the resolution ROADMAP.md's architecture
// note settled on: a **downsampled** light grid, propagated over a handful of
// iterations, uploaded as a small texture and stretched over the world with
// linear filtering. One extra SDL_RenderCopy, no shader path, no new dependency,
// and a resolution knob (BLOCK) to trade quality against cost.
//
// **This lives in src/render/ and not in src/physics/, and that is the item's
// stated trap rather than a filing preference.** F3.5 settled that rendering
// does not feed the simulation, and gave the reason that matters: two players on
// the same seed and the same input log must not diverge because of what was on
// screen. So the dependency runs one way only - this file includes grid.h, and
// nothing under src/physics/ may ever include this one. Everything here reads
// the grid `const` and writes only its own buffers, which is what makes that
// rule checkable by reading a single `#include` line.
//
// It is deliberately SDL-free so it can be tested headlessly like the
// simulation is, even though it is rendering code. The texture upload is
// main.cpp's job; what this produces is a plain ARGB buffer.
class LightField {
public:
    // Cells per light block on each axis. The knob the architecture note names.
    // Four is small enough that a single flame cell lands in a block of its own
    // rather than being averaged into a wall, and large enough that the
    // propagation below is a rounding error against the physics step - at the
    // padded viewport that is ~51x38 blocks, not 201x151 cells.
    static constexpr int BLOCK = 4;

    // How far light carries, in propagation steps. Each step moves light one
    // block - BLOCK cells - so this is a reach of ITERATIONS*BLOCK = 64 cells,
    // which is the number the reference footage argues for: the flame occupies a
    // modest band and walls tens of cells out pick up orange. Attenuation makes
    // the true reach shorter than the bound; this is the ceiling, not the look.
    static constexpr int ITERATIONS = 16;

    // Below this a cell contributes nothing. Ambient is 20 and Wood ignites at
    // 120, so this sits under the coldest ignition point and above anything a
    // world that is merely warm will reach - a cell this hot has either caught
    // or is about to, and both should glow. Without a floor here every conducted
    // degree would be a light source and the whole scene would haze.
    static constexpr uint8_t GLOW_THRESHOLD = 100;

    // Sized in cells; the block dimensions are derived and rounded up, so the
    // last block may hang off the edge of the region. That is intentional and it
    // is what keeps the stretch aligned - see `pixels()`.
    LightField(int region_cells_w, int region_cells_h);

    // Recomputes the whole field from the grid region whose top-left cell is
    // (origin_x, origin_y). Reads the grid and mutates nothing outside this
    // object.
    void update(const Grid& grid, int origin_x, int origin_y);

    // ARGB8888, one texel per block, row-major, `cols()` wide. Alpha is 255
    // throughout and carries no meaning: this texture is composited additively,
    // so black is "no light" and the alpha channel is never read.
    //
    // **The texture covers cols()*BLOCK by rows()*BLOCK cells, not the region it
    // was asked for**, and the caller must stretch it over exactly that larger
    // area. That is what puts each texel's centre on the centre of the cells it
    // was computed from, which is the whole of getting linear filtering to land
    // where the light actually is instead of half a block off it.
    const std::vector<uint32_t>& pixels() const { return texels; }

    int cols() const { return block_cols; }
    int rows() const { return block_rows; }

    // Whether anything at all is emitting. Zero light is the overwhelmingly
    // common case - a world nobody has set fire to - and the caller skips the
    // upload and the draw entirely when this is false, so the cost of the
    // feature in an unlit scene is the scan and nothing else.
    bool any_light() const { return lit; }

private:
    int block_cols;
    int block_rows;

    // Floats rather than bytes because sixteen multiplications by an attenuation
    // under 1 quantises to nothing in 8 bits - the far half of every falloff
    // would be a hard edge at the point the integer hit zero.
    struct Rgb { float r = 0.0f, g = 0.0f, b = 0.0f; };

    // What the hot cells themselves put out, kept separate from the propagated
    // result because every iteration re-imposes it as a floor. A source that
    // could be dimmed by its own falloff would fade out over the iterations
    // instead of settling.
    std::vector<Rgb> emission;
    std::vector<Rgb> front;
    std::vector<Rgb> back;

    // How much light survives crossing each block, 0-1. Derived from how much of
    // the block is solid, so terrain shadows itself without anything tracing a
    // ray.
    std::vector<float> transmit;

    std::vector<uint32_t> texels;
    bool lit = false;
};
