#pragma once

#include <cmath>

// V16's wrapping layers, pulled forward into V19 because V19 cannot afford
// them any other way.
//
// **The size relationship this retires is the reason it exists.** A
// non-wrapping backdrop layer must be as wide as the window plus the whole pan
// range at its own parallax factor - see layer_size() in
// tools/generate_backdrop.py - so **the nearer the band, the bigger its file**.
// The sky is 16 MB at 0.04 and the mountains 20 MB at 0.15; a ground plane sits
// nearer than either, and V19 wants five new bands. Priced flat, that is
// roughly three times the whole of assets/ for art that is, by construction,
// one repeating texture. A wrapping layer has no size relationship to the pan
// range at all: it is one tile, drawn as many times as the window needs.
//
// **This header is arithmetic and nothing else, and that split is deliberate.**
// The draw call that uses it lives in render/frame.cpp and needs SDL; the
// question of *where the copies go* does not, so it is here, where a headless
// suite can reach it. Same arrangement render/player_anim.cpp has with the
// sprite sheet, and for the same reason: the part that is easy to get wrong is
// the part that does not need a window to check.
//
// Nothing in src/physics/ may include this. It is renderer-side by the same
// rule the light field is - see the ENGINE_SOURCES / RENDER_SOURCES split in
// CMakeLists.txt, which is what makes that a build error rather than a habit.

namespace backdrop_wrap {

// Where the first copy of a tile goes, and how many copies cover the window.
//
// `first` is always in (-tile, 0]: the leftmost copy starts at or before the
// window's edge, never after it, so there is no uncovered strip on that side.
struct Tiling {
    float first;
    int count;
};

// One axis of it. `origin` is the layer's unwrapped parallax origin - what
// Camera::parallax_origin_x/y returns, which runs negative as the camera moves
// forward and is unbounded in both directions over a long session.
//
// **The whole job is turning that unbounded number into a bounded one**, and
// doing it without ever letting the visible result depend on how far the camera
// has travelled. std::fmod keeps the sign of its left operand, so a negative
// origin gives a result in (-tile, 0] directly and a positive one needs a
// single subtraction to land in the same half-open interval. Both branches must
// produce a `first` that is <= 0, because a `first` of exactly +0.001 leaves a
// one-pixel column of whatever was behind the layer showing at the window edge,
// on one frame, at one camera position - the class of defect that gets seen
// once, disbelieved, and not reproduced.
//
// The interval is half-open at 0 and not at -tile on purpose: `first == 0` is a
// legitimate aligned state and must not be pushed to -tile, which would draw an
// entire redundant copy off-screen on every frame the layer happens to align.
inline Tiling wrap_axis(float origin, int tile, int window) {
    if (tile <= 0) return Tiling{0.0f, 0};   // a zero-width tile tiles nothing

    const float t = static_cast<float>(tile);
    float first = std::fmod(origin, t);
    if (first > 0.0f) first -= t;

    // Copies needed to reach the far edge. `first` is <= 0, so (window - first)
    // is the full span that has to be covered measured from the first copy's
    // own left edge, and the ceiling of that over the tile width is the count.
    // Computed as an integer ceiling rather than std::ceil so that a span that
    // divides exactly does not gain a copy from a float that landed one ulp
    // high - the visible symptom of which would be nothing at all, which is
    // exactly why it would never get fixed.
    const int span = static_cast<int>(window - first) + 1;
    const int count = (span + tile - 1) / tile;
    return Tiling{first, count};
}

} // namespace backdrop_wrap
