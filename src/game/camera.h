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
    // than the viewport on an axis clamps to 0 on that axis, i.e. no
    // scrolling at all, which is still true of every world in the project
    // today (F3.1's coincidence: GRID_WIDTH/HEIGHT equal VIEWPORT_WIDTH/HEIGHT).
    void follow(int center_x, int center_y, int viewport_w, int viewport_h, int world_w, int world_h) {
        view_x_ = clamp_view(center_x - viewport_w / 2, viewport_w, world_w);
        view_y_ = clamp_view(center_y - viewport_h / 2, viewport_h, world_h);
    }

    int world_to_screen_x(int world_x) const { return (world_x - view_x_) * SCALE; }
    int world_to_screen_y(int world_y) const { return (world_y - view_y_) * SCALE; }
    int screen_to_world_x(int screen_x) const { return screen_x / SCALE + view_x_; }
    int screen_to_world_y(int screen_y) const { return screen_y / SCALE + view_y_; }

    // A length, not a position, so it is never shifted by the viewport's
    // offset - only ever scaled. Also covers the on-screen size of one world
    // cell, where scale_length(1) would work but reads oddly as cell_size().
    int scale_length(int world_length) const { return world_length * SCALE; }
    int cell_size() const { return SCALE; }

    // World-cell coordinates of the viewport's top-left corner - what the
    // texture upload in main.cpp reads the visible rect's pixels from.
    int view_x() const { return view_x_; }
    int view_y() const { return view_y_; }

private:
    static int clamp_view(int desired, int viewport_len, int world_len) {
        const int max_view = world_len > viewport_len ? world_len - viewport_len : 0;
        if (desired < 0) return 0;
        if (desired > max_view) return max_view;
        return desired;
    }

    int view_x_ = 0;
    int view_y_ = 0;
};
