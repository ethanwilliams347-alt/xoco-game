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

// **Where the plane's far edge goes, and the reason it is a row of the
// mountains BMP rather than a fraction of the window.** V19 authored the
// horizon at 0.55 of the window height, independently of where the mountain
// silhouette actually sat, and the two were contradictory at every camera
// position the world reaches: the plane is opaque and is drawn after the
// mountains, so it covered the entire band and the tester reported the
// mountains as missing. Deriving the horizon from the silhouette makes that
// contradiction unrepresentable instead of unlikely.
//
// This is the *deepest* row the skyline reaches, so the whole jagged edge is
// above the plane and the band below it - which is solid mountain across the
// full width - is what the plane is allowed to cover.
//
// It is generated because it is a fact about the art: mountain_skyline() is a
// pure function of random.Random(3) and the two composition fractions, so a
// change to either moves this number without anyone having to remember to.
//
// **A fraction of the layer's height and not a row index, which is not
// cosmetic.** The renderer multiplies it by whatever mountains texture was
// actually loaded, so the horizon lands on the silhouette for any mountain
// image - including the small synthetic one the golden-frame fixture builds,
// which is 300 rows against the shipped BMP's 1642. Stated as a row it
// was a number only one image could satisfy, and the fixture answered by
// pushing the whole plane off the bottom of its window and quietly losing
// the layer from the checksum.
// Shipped BMP: 1642 rows, skyline 285..623.
inline constexpr float MOUNTAINS_SKYLINE_MAX = 0.379415f;

// The inputs the sizes above are derived from, so a reader can tell whether
// a mismatch is a stale asset or a changed display table.
inline constexpr int GRID_WIDTH = 1920;
inline constexpr int GRID_HEIGHT = 1080;
inline constexpr int SCALE = 4;

} // namespace backdrop_layers
