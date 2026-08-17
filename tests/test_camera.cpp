// The camera's view arithmetic: where the followed point lands on screen, and
// what happens at the world's edges.
//
// Links no engine sources at all - `camera.h` is an SDL-free header - which is
// the narrowest source set in the build after `backdrop_test`.
//
// **This file was `test_camera_bias.cpp` until V23b** (2026-08-17), which
// retired the moving vertical anchor after session 9 asked for the centred
// framing back. What is kept here is the half that was always about `Camera`
// itself; the framing checks went with the mechanism they described. The
// centring check below is the one that matters most now, because it is the
// whole of the shipped composition rule and nothing else asserts it.

#include "game/camera.h"
#include "test_util.h"

#include <cmath>

namespace {

constexpr int VIEWPORT_W = 480;
constexpr int VIEWPORT_H = 270;   // 1080 / Camera::SCALE, the shipped mode
constexpr int WORLD_W = 1920;
constexpr int WORLD_H = 1080;

bool near(float a, float b, float tol = 0.001f) { return std::fabs(a - b) <= tol; }

// Where the followed point lands on screen, as a fraction of viewport height.
// The framing is asserted on this rather than on `view_y()`, which is the same
// claim one step further from what a player actually sees - and V23a is the
// reason it is worth the indirection: a framing that the world-edge clamp
// cannot honour looks correct in `view_y()` terms and wrong on screen.
float on_screen_fraction(const Camera& camera, float center_y) {
    return (center_y - (static_cast<float>(camera.view_y()) + camera.frac_y())) /
           static_cast<float>(VIEWPORT_H);
}

} // namespace

int main() {
    // Somewhere the view is unclamped on both axes, so the framing is the
    // camera's decision - at a clamp the world edge decides instead.
    const float cy = 540.0f;

    {
        Camera camera;
        camera.follow(960.0f, cy, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        check("the player is drawn at mid screen", near(on_screen_fraction(camera, cy), 0.5f), "");
        check("and centred horizontally too",
              near((960.0f - (static_cast<float>(camera.view_x()) + camera.frac_x())) /
                       static_cast<float>(VIEWPORT_W),
                   0.5f),
              "");
    }

    // The clamps. The view must never walk off the world, or the renderer
    // uploads from outside the grid - and the clamp is also what made V23's dig
    // framing undeliverable near the floor, so it is worth pinning at both
    // edges rather than assuming symmetry.
    {
        Camera at_floor;
        at_floor.follow(960.0f, 1075.0f, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        check("the view clamps at the world's bottom edge",
              at_floor.view_y() == WORLD_H - VIEWPORT_H, std::to_string(at_floor.view_y()));

        Camera at_ceiling;
        at_ceiling.follow(960.0f, 4.0f, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        check("the view clamps at the world's top edge", at_ceiling.view_y() == 0, "");

        Camera at_left;
        at_left.follow(4.0f, cy, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        check("the view clamps at the world's left edge", at_left.view_x() == 0, "");

        Camera at_right;
        at_right.follow(1916.0f, cy, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        check("the view clamps at the world's right edge",
              at_right.view_x() == WORLD_W - VIEWPORT_W, std::to_string(at_right.view_x()));

        // A world no larger than the viewport does not scroll at all, rather
        // than clamping to a negative view.
        Camera tiny;
        tiny.follow(100.0f, 100.0f, VIEWPORT_W, VIEWPORT_H, 200, 200);
        check("a world smaller than the viewport pins at the origin",
              tiny.view_x() == 0 && tiny.view_y() == 0, "");
    }

    // The A1 split: the whole view is fractional and only the *index* is
    // floored, because a camera that can sit only on whole cells is what read
    // as ghosting. Floored and not truncated, so a negative intermediate picks
    // the cell on the correct side.
    {
        Camera camera;
        camera.follow(960.25f, cy, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        check("the leftover sub-cell offset survives as frac", near(camera.frac_x(), 0.25f),
              std::to_string(camera.frac_x()));
        check("frac is never negative", camera.frac_x() >= 0.0f && camera.frac_y() >= 0.0f, "");

        // Round trip through both conversions, at a fractional view: a screen
        // pixel resolves to the cell it is drawn from.
        const int cell = camera.screen_to_world_x(0);
        check("screen_to_world floors into the view's own cell", cell == camera.view_x(),
              std::to_string(cell) + " vs " + std::to_string(camera.view_x()));
    }

    // Parallax origins are a function of the view and nothing else (V11). The
    // check that matters is the pair of endpoints: a factor of 1 locks a layer
    // to the world, and 0 pins it to the window.
    {
        Camera camera;
        camera.follow(960.0f, cy, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        check("a factor of 0 pins a layer to the window",
              near(camera.parallax_origin_x(0.0f), 0.0f) &&
                  near(camera.parallax_origin_y(0.0f), 0.0f),
              "");
        // A factor of 1 puts the layer's origin exactly where world cell 0
        // lands, which is what "locked to the world" means and is the same
        // expression the composition used before V11 moved it in here.
        check("a factor of 1 locks a layer to the world",
              near(camera.parallax_origin_x(1.0f), camera.world_to_screen_x(0.0f)) &&
                  near(camera.parallax_origin_y(1.0f), camera.world_to_screen_y(0.0f)),
              std::to_string(camera.parallax_origin_x(1.0f)) + " vs " +
                  std::to_string(camera.world_to_screen_x(0.0f)));
    }

    return report();
}
