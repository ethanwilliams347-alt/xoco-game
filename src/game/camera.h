#pragma once

// The one place that knows how a world cell maps to a screen pixel, in both
// directions. Before this, main.cpp divided the mouse by PIXEL_SCALE to read
// the brush/aim cell and multiplied by it again at three separate render
// sites - the same number, spelled out five different ways, with nowhere a
// future zoom or independent x/y scale could change it in one place. Now
// there is exactly one, and PIXEL_SCALE itself lives only here.
class Camera {
public:
    static constexpr int SCALE = 4; // each world cell is SCALE x SCALE screen pixels

    int world_to_screen(int world) const { return world * SCALE; }
    int screen_to_world(int screen) const { return screen / SCALE; }

    // The on-screen size of one world cell - used for the width/height of a
    // one-cell rect, where world_to_screen(1) would work but reads oddly.
    int cell_size() const { return SCALE; }
};
