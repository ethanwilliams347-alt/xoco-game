#pragma once

// GENERATED FILE - do not edit.
//   python tools/generate_backdrop.py --header
//
// V11 retiring the parallax duplication. These four numbers per layer used
// to exist twice - once in the C++ that draws the layer and once in the
// Python that sizes its image - with a comment in each asking a human to
// keep them in step. The failure mode of that arrangement is a seam at the
// pan limit: a layer runs out of image before the camera runs out of world,
// and nothing says so until somebody walks to the edge of the map.
//
// tools/generate_backdrop.py is the source. It derives both the factors and
// the sizes, so a change on that side cannot leave this side stale - the
// same arrangement tools/player_sheet.py --header has with player_sprite.h,
// and for the same reason.
//
// `width`/`height` are what the generator *would* write for this layer at
// these factors. main.cpp compares them against what it actually loaded and
// warns on a mismatch, which is what turns the seam from a pixel nobody
// reaches into a line at startup.
namespace backdrop_layers {

// **A wrapping layer's width/height is its *tile* size and is exact, where a
// pan-sized layer's is a minimum.** main.cpp's warning reads it the second
// way for both, which is the right direction for the case that matters: a
// tile smaller than generated repeats sooner than the art was drawn for.
struct Layer {
    float parallax_x;
    float parallax_y;
    int width;   // the BMP size generate_backdrop.py produces at these factors
    int height;
};

// assets/backdrop_sky.bmp
inline constexpr Layer SKY{0.04f, 0.02f, 3678, 1512};
// assets/backdrop_mountains.bmp
inline constexpr Layer MOUNTAINS{0.15f, 0.06f, 4311, 1642};
// assets/backdrop_ground.bmp  (a tile - this layer wraps)
inline constexpr Layer GROUND{0.28f, 0.11f, 256, 256};

// V19's ground plane is drawn as strips between two depths, so it needs a
// second x factor that no other layer has. GROUND above carries the far edge
// (the horizon); this is the near one. Both are stated derivations off the
// geometric ladder - see the comment in tools/generate_backdrop.py, which is
// the only place the argument lives.
inline constexpr float GROUND_NEAR_X = 0.52f;

// The inputs the sizes above are derived from, so a reader can tell whether
// a mismatch is a stale asset or a changed display table.
inline constexpr int GRID_WIDTH = 1920;
inline constexpr int GRID_HEIGHT = 1080;
inline constexpr int SCALE = 4;

} // namespace backdrop_layers
