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

// --- V19's ground plane: one band drawn as N strips ------------------------
//
// **A receding plane has no single depth, so it has no single parallax
// factor.** Drawn flat at one factor it reads as a wall standing behind the
// world rather than as ground going away, which is the failure this whole
// construction exists to avoid - and it is the one piece of new rendering V19
// costs. The plane is cut into horizontal strips, each strip is a depth, and
// each strip therefore scrolls at its own rate and samples its own rows of the
// tile.
//
// **The relation, and it is one relation rather than two.** For a plane seen
// edge-on, screen distance below the horizon goes as inverse world distance, so
// the scroll factor is linear in that distance:
//
//     f(t) = f_far + (f_near - f_far) * t,   t = 0 at the horizon, 1 at the near edge
//
// and world distance is d(t) = 1/f(t). The texture is laid out *in world
// distance* - its top row is the far edge and its bottom row the near one - so
// the source row for a strip falls out of the same d(t) with no second curve to
// author and nothing to keep in step:
//
//     v(t) = (d(t) - d_near) / (d_far - d_near),  src_y = (1 - v) * tile_h
//
// **This is the "texture gradient" mechanism and it comes out of the geometry
// rather than out of the art.** Marks near the viewer are magnified and marks
// near the horizon are compressed, because a far strip spans a huge range of
// world distance in a handful of screen rows. A correction to ROADMAP.md's V19
// entry belongs here rather than only there: that entry says "source row height
// shrinking with distance", and the relation above does the opposite - `src_h`
// *grows* with distance. The mechanism the entry names in the same sentence
// (the reference's marks widening 1.3x-3.0x toward the viewer) is what is built,
// and it is the sentence that was wrong, not the mechanism.
//
// **Only the vertical is scaled, and the horizontal tile width is constant
// across every strip.** A true plane shrinks a mark in both axes, so this is a
// stated cheapness rather than an oversight. Scaling the tile's width per strip
// costs nothing arithmetically and looks worse: adjacent strips would then tile
// at different widths, their phases would diverge, and the mark pattern would
// stair-step at every strip boundary - twenty-four visible seams bought to fix a
// foreshortening nobody can see on a night-dark texture. The vertical
// compression alone carries the gradient.
struct Plane {
    float horizon_y;    // screen y of the far edge
    float bottom_y;     // screen y of the near edge - normally the window's bottom
    float far_factor;   // parallax at the horizon
    float near_factor;  // parallax at the near edge
    int tile_h;         // the tile's height in pixels; its rows are the plane's depth
};

// One strip: where it lands, what it samples, and how fast it scrolls.
struct Strip {
    float factor;         // this strip's parallax factor, for wrap_axis
    float dst_y, dst_h;   // destination, in screen pixels
    float src_y, src_h;   // source, in tile rows
};

// Strip `i` of `n`, counted from the horizon down.
//
// The factor is taken at the strip's *midpoint* rather than at its top edge: a
// strip is one depth standing in for a range of them, and its middle is the
// depth it is least wrong about at both of its own edges. Taking the top edge
// biases every strip toward the horizon and leaves the near edge of the plane
// scrolling slower than `near_factor` says it does.
//
// The source rows come from the strip's actual edges and not from its midpoint,
// because those have to *meet*: strip i's bottom source row is strip i+1's top
// source row, or the texture repeats or skips a row at every boundary.
inline Strip plane_strip(const Plane& p, int i, int n) {
    if (n <= 0 || p.tile_h <= 0 || p.far_factor <= 0.0f || p.near_factor <= 0.0f)
        return Strip{0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    const float t0 = static_cast<float>(i) / static_cast<float>(n);
    const float t1 = static_cast<float>(i + 1) / static_cast<float>(n);
    const float band = p.bottom_y - p.horizon_y;

    const float f_mid = p.far_factor +
                        (p.near_factor - p.far_factor) * 0.5f * (t0 + t1);

    // v(t), written out. d_far and d_near are the reciprocals of the two
    // factors; the subtraction below is (1/f - 1/f_near) / (1/f_far - 1/f_near),
    // which is v with both reciprocals left in place rather than expanded -
    // legible beats clever here, and this runs n times a frame, not per pixel.
    const float d_far = 1.0f / p.far_factor;
    const float d_near = 1.0f / p.near_factor;
    const float span = d_far - d_near;
    const float th = static_cast<float>(p.tile_h);

    auto src_at = [&](float t) {
        if (span <= 0.0f) return t * th;   // degenerate: far and near are one depth
        const float d = 1.0f / (p.far_factor + (p.near_factor - p.far_factor) * t);
        const float v = (d - d_near) / span;
        return (1.0f - v) * th;
    };

    const float s0 = src_at(t0);
    const float s1 = src_at(t1);

    return Strip{f_mid,
                 p.horizon_y + band * t0, band * (t1 - t0),
                 s0, s1 - s0};
}

} // namespace backdrop_wrap
