#pragma once

// The one place that knows how a world cell maps to a screen pixel, in both
// directions, and (F3.4) which part of the world is currently in view. Before
// F3.2 this conversion was scattered across main.cpp; before F3.4 the
// viewport's top-left was always the world's origin, so a world bigger than
// the window could only ever show its top-left corner.
class Camera {
public:
    static constexpr int SCALE = 4; // each world cell is SCALE x SCALE screen pixels

    // Centers the viewport on (center_x, center_y) - normally the player -
    // clamped so it never scrolls past the world's edges. A world no bigger
    // than the viewport on an axis clamps to 0 on that axis, i.e. no scrolling
    // at all on that axis.
    // Takes a fractional centre, because a camera that can only sit on whole
    // cells is the larger half of defect A1 (PLAYTEST_LOG.md session 1). The
    // world (1920x1080) is larger than every viewport in DISPLAY_MODES, so the
    // view is unclamped wherever the player usually is, which pins the player
    // near screen centre and scrolls *the world* instead. Rounded to whole cells, that world moves
    // in 4-pixel jerks at the simulation's irregular 0.75-cells-per-step
    // cadence while the eye tracks it smoothly - which is what reads as
    // ghosting, and it is not fixed by smoothing the player alone.
    //
    // The split: `view_x()` stays an integer cell because it indexes the pixel
    // buffer the texture is uploaded from, and there is no such thing as a
    // fractional array index. The leftover is handed to the renderer as
    // `frac_x()` and paid out in screen pixels when the texture is drawn.
    void follow(float center_x, float center_y, int viewport_w, int viewport_h, int world_w, int world_h) {
        view_fx_ = clamp_view(center_x - static_cast<float>(viewport_w) / 2.0f, viewport_w, world_w);
        view_fy_ = clamp_view(center_y - static_cast<float>(viewport_h) * anchor_y_, viewport_h, world_h);
    }

    // Where on the screen the followed point sits, as a fraction of the
    // viewport's height: 0.5 is dead centre and was the only behaviour before
    // V23. Larger values draw the player lower on the screen, which is the
    // direction the reference frames want - `Camera::follow` centring strictly
    // is what capped the receding plane's visible share at ~50% of its band by
    // construction, and no scene edit could reach past that, because the scene
    // does not decide where on screen the player is drawn.
    //
    // **A fraction rather than a count of cells, and that is not a detail.**
    // The thing being matched is a *composition* - the reference puts its
    // subject about four fifths down the frame - and a fixed cell offset
    // expresses that only at the one viewport height it was tuned at. Every
    // other entry in DISPLAY_MODES would get a different picture from the same
    // constant.
    //
    // Held here rather than applied by the caller because `follow` is called
    // **twice per rendered frame** (once on the stepped position, once re-aimed
    // at the interpolated draw position - the A1 correction below), and an
    // anchor applied at one call site and not the other tears the backdrop
    // against the world every frame.
    //
    // Deliberately not clamped: a value outside [0, 1] aims the camera at a
    // point off-screen, which is meaningless rather than dangerous, and
    // `clamp_view` still keeps the view inside the world.
    void set_vertical_anchor(float fraction) { anchor_y_ = fraction; }
    float vertical_anchor() const { return anchor_y_; }

    // Float in, float out: a world position drawn against a fractional view has
    // a fractional screen position, and rounding it here would put the jitter
    // straight back for anything drawn through it. Callers that need a pixel
    // hand these to SDL's float-rect calls.
    float world_to_screen_x(float world_x) const { return (world_x - view_fx_) * SCALE; }
    float world_to_screen_y(float world_y) const { return (world_y - view_fy_) * SCALE; }

    // Still integers, and deliberately: this direction answers "which cell is
    // under the mouse", and there is no fractional answer to that question.
    // Floored rather than truncated, since a negative intermediate would
    // otherwise round towards zero and pick the cell on the wrong side.
    int screen_to_world_x(int screen_x) const { return floor_to_int(static_cast<float>(screen_x) / SCALE + view_fx_); }
    int screen_to_world_y(int screen_y) const { return floor_to_int(static_cast<float>(screen_y) / SCALE + view_fy_); }

    // Where a parallax layer's top-left corner goes, in screen pixels, for a
    // layer that moves at `factor` times the camera's speed (V11). A factor of
    // 1.0 gives a layer locked to the world; 0.0 gives one pinned to the
    // window.
    //
    // **Here rather than at the draw site because it is a camera question.**
    // The offset is a function of the view position and nothing else, and it
    // was previously computed inline in the composition from `view_x()`,
    // `frac_x()` and `SCALE` - three of this class's outputs reassembled by a
    // caller, which is the shape F3.2 already moved every other coordinate
    // conversion in here to end.
    //
    // The continuous view is rebuilt as `view_x() + frac_x()` rather than read
    // off `view_fx_` directly, and that is not redundancy: it is the exact
    // expression the composition used before V11, kept so the restructure is a
    // no-op the golden checksum can confirm rather than a claim.
    float parallax_origin_x(float factor) const {
        return -(static_cast<float>(view_x()) + frac_x()) * SCALE * factor;
    }
    float parallax_origin_y(float factor) const {
        return -(static_cast<float>(view_y()) + frac_y()) * SCALE * factor;
    }

    // A length, not a position, so it is never shifted by the viewport's
    // offset - only ever scaled. Also covers the on-screen size of one world
    // cell, as scale_length(1).
    int scale_length(int world_length) const { return world_length * SCALE; }

    // World-cell coordinates of the viewport's top-left corner - what the
    // texture upload in main.cpp reads the visible rect's pixels from. Floored,
    // so the cell named here is always the one the fractional view sits inside
    // rather than the nearest one, which is what makes frac_x() below
    // non-negative and the render offset a shift in one direction only.
    int view_x() const { return floor_to_int(view_fx_); }
    int view_y() const { return floor_to_int(view_fy_); }

    // How far into that cell the view actually is, 0 to just under 1. The
    // renderer draws the world texture shifted left/up by this much of a cell
    // so the world scrolls smoothly between whole-cell uploads. main.cpp
    // uploads one extra cell of margin to cover the sliver this exposes.
    float frac_x() const { return view_fx_ - static_cast<float>(view_x()); }
    float frac_y() const { return view_fy_ - static_cast<float>(view_y()); }

private:
    static int floor_to_int(float v) {
        const int truncated = static_cast<int>(v);
        return (v < 0.0f && static_cast<float>(truncated) != v) ? truncated - 1 : truncated;
    }

    static float clamp_view(float desired, int viewport_len, int world_len) {
        const float max_view = world_len > viewport_len ? static_cast<float>(world_len - viewport_len) : 0.0f;
        if (desired < 0.0f) return 0.0f;
        if (desired > max_view) return max_view;
        return desired;
    }

    float view_fx_ = 0.0f;
    float view_fy_ = 0.0f;

    // 0.5 is exactly the pre-V23 expression, and it is the default so that the
    // anchor mechanism could be built, linked and run against the standing
    // golden checksum before anything moved - the house rule for a change with
    // a no-op half.
    float anchor_y_ = 0.5f;
};
