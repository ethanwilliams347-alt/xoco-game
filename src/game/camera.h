#pragma once

// The one place that knows how a world cell maps to a screen pixel, in both
// directions, and (F3.4) which part of the world is currently in view. Before
// F3.2 this conversion was scattered across main.cpp; before F3.4 the
// viewport's top-left was always the world's origin, so a world bigger than
// the window could only ever show its top-left corner.
class Camera {
public:
    // **The default, not a constant - and the distinction is V28's whole
    // structural change.** Four screen pixels per cell is still the art
    // direction for every generated-backdrop scene and every recorded session,
    // and a default-constructed `Camera` is still exactly the old one, which is
    // what keeps `test_golden_frame`'s checksum meaningful rather than lucky.
    //
    // What changed is that a *scene* may now state its own. `bg1` is authored at
    // one art pixel to one world cell (344x144 of art, 344x144 of world), and at
    // 4 px/cell that world would be 1376x576 screen pixels - smaller than the
    // window, so the window could not be a view *into* it. It needs 10. The
    // argument is in gemini_plans/bg1_world_scale_and_geometry_plan.md section 2,
    // including why the alternative - moving the global 4 to 10 - was refused.
    //
    // `src/physics/` must never learn this exists. The scale is a rendering
    // property; the simulation is in cells and always was.
    static constexpr int DEFAULT_SCALE = 4;

    int scale() const { return scale_; }

    // Set once per scene activation, before the viewport is computed from it.
    // Refuses a non-positive value rather than propagating a division by zero
    // into every coordinate conversion in the project.
    void set_scale(int scale) { scale_ = scale > 0 ? scale : DEFAULT_SCALE; }

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
        follow_mode(center_x, center_y, viewport_w, viewport_h, world_w, world_h, false);
    }

    // The same framing, told which of `scene_list::SceneMode`'s two paradigms
    // the world is - and it takes a `bool` rather than the enum on purpose,
    // because `src/game/` must not start depending on the scene loader to
    // position a viewport. The caller reads `SceneDef::is_infinite()` and hands
    // over the answer.
    //
    // **Only the horizontal clamp is dropped, and never the vertical one.** An
    // infinite scene is unbounded in travel, not in depth: the world still has
    // a floor and a ceiling, and the texture upload in `main.cpp` reads rows
    // out of the grid, so a view that scrolled off the top would be reading
    // outside it. The horizontal case is different only because the upload
    // already clamps its own source rect and the backdrop tiles.
    //
    // `follow` keeps its old signature and delegates here with `false`, so
    // every existing caller and every recorded session frames identically -
    // the bounded path below is the pre-existing two lines, unchanged.
    void follow_mode(float center_x, float center_y,
                     int viewport_w, int viewport_h,
                     int world_w, int world_h,
                     bool is_infinite) {
        const float desired_y = center_y - static_cast<float>(viewport_h) * VERTICAL_ANCHOR;
        if (is_infinite) {
            view_fx_ = center_x - static_cast<float>(viewport_w) / 2.0f;
        } else {
            view_fx_ = clamp_view(center_x - static_cast<float>(viewport_w) / 2.0f, viewport_w, world_w);
        }
        view_fy_ = clamp_view(desired_y, viewport_h, world_h);
    }

    // **The vertical framing is not centred, and it is a constant rather than
    // a knob** (V22, 2026-08-18). The player sits VERTICAL_ANCHOR of the way
    // down the viewport wherever the view is unclamped; 0.5 would be the
    // centring V23b restored, and this is deliberately not that.
    //
    // **This reopens V23b at the tester's direction and it is not V23.** V23
    // made the anchor *move* - 0.80 at the surface easing to 0.50 while
    // digging - and that mechanism was rejected twice by playing: session 8
    // called the delivered framing upside down and session 9 asked for plain
    // centring. None of it comes back. What comes back is only the fixed
    // offset, because V22 needs one number and not a behaviour: the receding
    // plane cannot take more than ~50% of the band below a centred player,
    // and the reference reading V22 is authored against is two thirds
    // (notes/reference_observations.txt entry 9). **A moving anchor is a
    // rejected feel; a fixed anchor is a composition.** Do not re-derive the
    // easing from this constant.
    //
    // Both of the things V23b said to carry forward are honoured here, and
    // they are the reason this is four lines rather than a file. It lives
    // **here** and not at the caller, because `follow` is called twice per
    // rendered frame (once on the stepped position, once re-aimed at the
    // interpolated draw position - the A1 correction below) and an anchor
    // applied at one call site and not the other tears the backdrop against
    // the world every frame. And it is a **fraction of the viewport**, not a
    // count of cells: the ~80 cells the V22 entry computes is the shipped
    // 270-cell viewport only, and DISPLAY_MODES has several. The rest of the
    // argument is in ROADMAP.md under V22, and under V23 / V23a / V23b for
    // what was tried before it.
    //
    // **The world's bottom clamp still wins**, which is what V23a was really
    // about: near the floor `clamp_view` refuses the framing outright and the
    // player rides up the screen. That is correct - the alternative is
    // uploading from outside the grid - but it means this constant states an
    // intent, not a guarantee, and the spawn is close enough to the floor for
    // the difference to be visible. `test_camera` pins both.
    static constexpr float VERTICAL_ANCHOR = 0.80f;

    // Float in, float out: a world position drawn against a fractional view has
    // a fractional screen position, and rounding it here would put the jitter
    // straight back for anything drawn through it. Callers that need a pixel
    // hand these to SDL's float-rect calls.
    float world_to_screen_x(float world_x) const { return (world_x - view_fx_) * static_cast<float>(scale_); }
    float world_to_screen_y(float world_y) const { return (world_y - view_fy_) * static_cast<float>(scale_); }

    // Still integers, and deliberately: this direction answers "which cell is
    // under the mouse", and there is no fractional answer to that question.
    // Floored rather than truncated, since a negative intermediate would
    // otherwise round towards zero and pick the cell on the wrong side.
    int screen_to_world_x(int screen_x) const { return floor_to_int(static_cast<float>(screen_x) / static_cast<float>(scale_) + view_fx_); }
    int screen_to_world_y(int screen_y) const { return floor_to_int(static_cast<float>(screen_y) / static_cast<float>(scale_) + view_fy_); }

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
        return -(static_cast<float>(view_x()) + frac_x()) * static_cast<float>(scale_) * factor;
    }
    float parallax_origin_y(float factor) const {
        return -(static_cast<float>(view_y()) + frac_y()) * static_cast<float>(scale_) * factor;
    }

    // A length, not a position, so it is never shifted by the viewport's
    // offset - only ever scaled. Also covers the on-screen size of one world
    // cell, as scale_length(1).
    int scale_length(int world_length) const { return world_length * scale_; }

    // World-cell coordinates of the viewport's top-left corner - what the
    // texture upload in main.cpp reads the visible rect's pixels from. Floored,
    // so the cell named here is always the one the fractional view sits inside
    // rather than the nearest one, which is what makes frac_x() below
    // non-negative and the render offset a shift in one direction only.
    // The continuous view position, which is what a parallax layer that has to
    // be *placed* rather than *offset* needs - the fixed-scene backdrop pan in
    // frame.cpp divides it by the pan range. Exposed rather than reassembled by
    // the caller from `view_x() + frac_x()`, for the reason
    // `parallax_origin_x` already gives.
    float view_fx() const { return view_fx_; }
    float view_fy() const { return view_fy_; }

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

    int scale_ = DEFAULT_SCALE;
    float view_fx_ = 0.0f;
    float view_fy_ = 0.0f;
};
