// V23: the vertical anchor, and the dig framing that moves it.
//
// Links no engine sources at all - `camera.h` and `camera_bias.h` are both
// SDL-free headers - which is the narrowest source set in the build after
// `backdrop_test`, and the reason the decision was put in a header instead of
// in the SDL frame loop where it would have been unreachable.

#include "game/camera.h"
#include "game/camera_bias.h"
#include "test_util.h"

#include <cmath>

namespace {

constexpr int VIEWPORT_W = 480;
constexpr int VIEWPORT_H = 270;   // 1080 / Camera::SCALE, the shipped mode
constexpr int WORLD_W = 1920;
constexpr int WORLD_H = 1080;

bool near(float a, float b, float tol = 0.001f) { return std::fabs(a - b) <= tol; }

// Where the followed point lands on screen, as a fraction of viewport height.
// This is the quantity the whole item is about, so the tests assert on it
// rather than on `view_y()`, which is the same claim one step further from what
// a player sees.
float on_screen_fraction(const Camera& camera, float center_y) {
    return (center_y - (static_cast<float>(camera.view_y()) + camera.frac_y())) /
           static_cast<float>(VIEWPORT_H);
}

} // namespace

int main() {
    // ---------------------------------------------------------------- Camera
    // The default is exactly the pre-V23 expression. This is the check that
    // let the mechanism ship against the standing golden checksum before
    // anything moved, and it is kept because that guarantee is what any future
    // no-op half will be run against too.
    {
        Camera centred;
        check("anchor defaults to centre", near(centred.vertical_anchor(), 0.5f), "");

        // Somewhere the view is unclamped on both axes, so the anchor is free
        // to act - at a clamp the world edge decides and the anchor cannot.
        const float cy = 540.0f;
        centred.follow(960.0f, cy, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        check("centred camera draws the player at mid screen",
              near(on_screen_fraction(centred, cy), 0.5f), "");

        Camera low;
        low.set_vertical_anchor(CameraBias::SURFACE_ANCHOR);
        low.follow(960.0f, cy, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        check("surface anchor draws the player low",
              near(on_screen_fraction(low, cy), CameraBias::SURFACE_ANCHOR), "");

        // The point of the item, stated as the number that failed: a centred
        // camera cannot put the player past half the screen, so the receding
        // plane's band below them capped at ~50% by construction.
        check("the anchor is what lifts the plane's share past half",
              on_screen_fraction(low, cy) > 0.5f, "");

        // The anchor must never defeat the world-edge clamp, or the view walks
        // off the world and the renderer uploads from outside the grid.
        Camera at_floor;
        at_floor.set_vertical_anchor(CameraBias::SURFACE_ANCHOR);
        at_floor.follow(960.0f, 1075.0f, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        check("anchor still clamps at the world's bottom edge",
              at_floor.view_y() == WORLD_H - VIEWPORT_H,
              std::to_string(at_floor.view_y()));

        Camera at_ceiling;
        at_ceiling.set_vertical_anchor(CameraBias::DIG_ANCHOR);
        at_ceiling.follow(960.0f, 4.0f, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        check("anchor still clamps at the world's top edge", at_ceiling.view_y() == 0, "");

        // The horizontal axis is deliberately untouched by V23, and this says
        // so: a vertical anchor that quietly moved x would shift every parallax
        // layer sideways, which is the seam V11 exists to prevent.
        Camera a, b;
        b.set_vertical_anchor(CameraBias::DIG_ANCHOR);
        a.follow(960.0f, cy, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        b.follow(960.0f, cy, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
        check("the anchor is vertical only", a.view_x() == b.view_x() && near(a.frac_x(), b.frac_x()), "");
    }

    // ------------------------------------------------------------ the target
    {
        check("not digging holds the surface framing",
              near(CameraBias::target(CameraBias::SURFACE_ANCHOR, false, 0, 200, VIEWPORT_H),
                   CameraBias::SURFACE_ANCHOR),
              "a steep aim with the button up must not move the camera");

        // Straight down, hard. The `+ viewport_h * (anchor - 0.5)` term means
        // the aim has to clear the anchor's own displacement before it reads as
        // downward at all, so this uses a genuinely deep target.
        check("digging straight down reaches the dig framing",
              near(CameraBias::target(CameraBias::SURFACE_ANCHOR, true, 0, 400, VIEWPORT_H),
                   CameraBias::DIG_ANCHOR),
              "");

        check("digging upward keeps the surface framing",
              near(CameraBias::target(CameraBias::SURFACE_ANCHOR, true, 0, -400, VIEWPORT_H),
                   CameraBias::SURFACE_ANCHOR),
              "the volume worth showing is not underneath the player");

        // The framing is bounded by the two reference readings and never
        // wanders outside them - a target between them is fine, one beyond
        // either is a sign the downward fraction stopped being clamped.
        for (int dy = -400; dy <= 400; dy += 25) {
            for (int dx = -300; dx <= 300; dx += 100) {
                const float t = CameraBias::target(CameraBias::SURFACE_ANCHOR, true, dx, dy, VIEWPORT_H);
                if (t < CameraBias::DIG_ANCHOR - 0.001f || t > CameraBias::SURFACE_ANCHOR + 0.001f) {
                    check("target stays between the two reference framings", false,
                          "dx=" + std::to_string(dx) + " dy=" + std::to_string(dy) +
                          " t=" + std::to_string(t));
                }
            }
        }
        check("target stays between the two reference framings", true, "");
    }

    // --------------------------------------------------- the feedback cut
    // The trap this class is mostly comment about. The cursor's world cell is
    // resolved through the moving view, so a **stationary hand** reports a
    // different `aim_dy` at every anchor. If the target were computed off that
    // raw offset, digging downward would raise the view, which would make the
    // same hand position read as a steeper dig, which would raise it further.
    //
    // Simulated exactly: hold one screen pixel, resolve it through cameras at
    // two different anchors, and check the framing asked for does not depend on
    // which camera did the resolving.
    {
        const float player_y = 540.0f;
        const int held_mouse_y = 700;   // one fixed screen pixel, in the lower half

        auto framing_asked_for = [&](float anchor) {
            Camera camera;
            camera.set_vertical_anchor(anchor);
            camera.follow(960.0f, player_y, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
            const int cursor_world_y = camera.screen_to_world_y(held_mouse_y);
            const int aim_dy = cursor_world_y - static_cast<int>(player_y);
            return CameraBias::target(anchor, true, 0, aim_dy, VIEWPORT_H);
        };

        const float from_surface = framing_asked_for(CameraBias::SURFACE_ANCHOR);
        const float from_dig = framing_asked_for(CameraBias::DIG_ANCHOR);
        check("a still hand asks for the same framing from either anchor",
              near(from_surface, from_dig, 0.02f),
              "surface=" + std::to_string(from_surface) + " dig=" + std::to_string(from_dig));

        // And the same check with the correction removed, to prove the test can
        // see the defect it exists for. This is the raw offset the naive
        // version would have used; the two framings must disagree, or the
        // check above passes for a reason nobody has verified.
        auto naive_framing = [&](float anchor) {
            Camera camera;
            camera.set_vertical_anchor(anchor);
            camera.follow(960.0f, player_y, VIEWPORT_W, VIEWPORT_H, WORLD_W, WORLD_H);
            const int aim_dy = camera.screen_to_world_y(held_mouse_y) - static_cast<int>(player_y);
            return CameraBias::target(0.5f, true, 0, aim_dy, VIEWPORT_H);
        };
        check("the still-hand check can actually fail",
              !near(naive_framing(CameraBias::SURFACE_ANCHOR), naive_framing(CameraBias::DIG_ANCHOR), 0.02f),
              "if these agree the correction above is not what is being tested");
    }

    // -------------------------------------------------------------- the ease
    {
        check("ease closes the gap and stops exactly",
              near(CameraBias::ease(0.30f, 0.80f, 10.0f), 0.80f),
              "a huge dt must land on the target, not overshoot past it");
        check("ease moves down as well as up",
              near(CameraBias::ease(0.80f, 0.30f, 10.0f), 0.30f), "");

        const float one_frame = CameraBias::ease(CameraBias::SURFACE_ANCHOR, CameraBias::DIG_ANCHOR, 1.0f / 60.0f);
        check("ease is gradual at a frame's dt",
              one_frame < CameraBias::SURFACE_ANCHOR && one_frame > CameraBias::DIG_ANCHOR,
              std::to_string(one_frame));

        // Frame-rate independence: the same simulated second must land in the
        // same place whether it arrives in 60 slices or 15. This is the defect
        // class F2.3 removed from the simulation, checked here so it does not
        // come back through the view.
        CameraBias fast, slow;
        for (int i = 0; i < 60; ++i) fast.update(1.0f / 60.0f, true, 0, 400, VIEWPORT_H);
        for (int i = 0; i < 15; ++i) slow.update(1.0f / 15.0f, true, 0, 400, VIEWPORT_H);
        check("a second of easing is a second at any frame rate",
              near(fast.anchor(), slow.anchor(), 0.01f),
              "fast=" + std::to_string(fast.anchor()) + " slow=" + std::to_string(slow.anchor()));
    }

    // ------------------------------------------------------- the whole cycle
    {
        CameraBias bias;
        check("a run starts already composed, not easing in from centre",
              near(bias.anchor(), CameraBias::SURFACE_ANCHOR),
              "the first frame of a run is a frame the player sees");

        for (int i = 0; i < 120; ++i) bias.update(1.0f / 60.0f, true, 0, 400, VIEWPORT_H);
        check("digging down arrives at the dig framing",
              near(bias.anchor(), CameraBias::DIG_ANCHOR, 0.01f), std::to_string(bias.anchor()));

        for (int i = 0; i < 120; ++i) bias.update(1.0f / 60.0f, false, 0, 400, VIEWPORT_H);
        check("releasing returns to the surface framing",
              near(bias.anchor(), CameraBias::SURFACE_ANCHOR, 0.01f), std::to_string(bias.anchor()));
    }

    return report();
}
