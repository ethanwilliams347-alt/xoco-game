#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "physics/grid.h"
#include "render/light.h"
#include "test_util.h"

// V7's light field. Headless because `LightField` is deliberately SDL-free -
// what it produces is a plain ARGB buffer, and the texture upload is main.cpp's
// job - so the thing worth testing is testable the same way the simulation is.
//
// **These tests are written against a specific failure from session 1.** The A4
// test passed trivially after the fire rebuild and would have kept passing if
// propagation had broken completely, because it only ever asserted that the
// thing it was watching was non-zero. So every claim here that light *reaches*
// somewhere is paired with a control asserting it does not reach somewhere else.
// A test that only checks for brightness is passed by a field that lights the
// entire world, which is the most likely way this file breaks.

namespace {

// Channel accessors on the packed ARGB texels, so the tests read the same bytes
// the GPU will.
int red(uint32_t argb)   { return (argb >> 16) & 0xFF; }
int green(uint32_t argb) { return (argb >> 8) & 0xFF; }
int blue(uint32_t argb)  { return argb & 0xFF; }

// Rough perceptual brightness is not the point here - the tests compare light to
// light, so the sum of the channels is enough and is monotonic in all three.
int brightness(uint32_t argb) { return red(argb) + green(argb) + blue(argb); }

// Bounds-checked on purpose. The obvious indexing expression wraps a
// past-the-end column onto the *next row's* first block, which reads as a
// plausible brightness rather than as an error - a probe meant to be far from
// the fire came back lit because it had silently landed near it. Anything out of
// range is a broken test, so it aborts rather than returning a number.
uint32_t at(const LightField& light, int bx, int by) {
    if (bx < 0 || by < 0 || bx >= light.cols() || by >= light.rows()) {
        std::printf("[FAIL] probe (%d,%d) is outside a %dx%d field\n",
                    bx, by, light.cols(), light.rows());
        std::exit(1);
    }
    return light.pixels()[static_cast<size_t>(by) * light.cols() + bx];
}

// A world with nothing in it but air, so a test can place exactly what it means
// to and nothing else is emitting or occluding.
Grid empty_world(int w, int h) {
    return Grid(w, h, 1234);
}

} // namespace

int main() {
    constexpr int W = 128, H = 128;
    constexpr int B = LightField::BLOCK;

    // --- the region-to-block derivation ---
    {
        LightField light(10, 10);
        // Rounded up, so the field always covers at least the region asked for.
        // Rounding down would leave an unlit strip along the right and bottom of
        // the view, which is the edge the camera is usually panning towards.
        check("blocks cover the whole region",
              light.cols() * B >= 10 && light.rows() * B >= 10,
              std::to_string(light.cols()) + "x" + std::to_string(light.rows()) + " blocks");
        check("blocks do not overshoot by a whole block",
              (light.cols() - 1) * B < 10 && (light.rows() - 1) * B < 10);
    }

    // --- a world nobody has set fire to costs nothing and shows nothing ---
    {
        Grid grid = empty_world(W, H);
        for (int y = 64; y < H; ++y)
            for (int x = 0; x < W; ++x) grid.set_element(x, y, ElementType::Wall);

        LightField light(64, 64);
        light.update(grid, 0, 0);

        bool all_black = true;
        for (uint32_t t : light.pixels()) if (brightness(t) != 0) all_black = false;

        check("an unlit world reports no light", !light.any_light());
        check("an unlit world produces a black field", all_black);
    }

    // --- a hot cell lights its own block ---
    {
        Grid grid = empty_world(W, H);
        grid.set_element(32, 32, ElementType::Fire); // spawns at 250

        LightField light(64, 64);
        light.update(grid, 0, 0);

        check("a fire cell reports light", light.any_light());
        check("the block containing the fire is lit", brightness(at(light, 32 / B, 32 / B)) > 0);
    }

    // --- propagation reaches, and stops ---
    //
    // The paired assertion. `near` proves light leaves the block it was emitted
    // in; `far` proves it does not simply fill the buffer. A field that lit
    // everything, or one that lit only the source block, fails exactly one of
    // these, and the previous session's trivially-passing test is why both are
    // here.
    {
        Grid grid = empty_world(W, H);
        grid.set_element(32, 32, ElementType::Fire);

        LightField light(128, 128);
        light.update(grid, 0, 0);

        const int src_bx = 32 / B, src_by = 32 / B;
        const int source = brightness(at(light, src_bx, src_by));
        const int near   = brightness(at(light, src_bx + 2, src_by));
        // Past ITERATIONS blocks of reach, and still inside the field - the
        // first version of this probe was outside it and wrapped onto the next
        // row, which is why `at` now refuses to index out of range.
        const int far    = brightness(at(light, src_bx + LightField::ITERATIONS + 2, src_by));

        check("light carries into open air beyond its own block", near > 0,
              "near=" + std::to_string(near));
        check("light is dimmer further from the source", near < source,
              "source=" + std::to_string(source) + " near=" + std::to_string(near));
        check("light does not fill the whole field", far == 0,
              "far=" + std::to_string(far));
    }

    // --- the falloff is not lopsided ---
    //
    // **What this catches is an asymmetric neighbour gather** - a dropped or
    // mistyped branch in the four-way max, which is easy to write and produces a
    // glow visibly off-centre from the flame casting it.
    //
    // **What it does not catch, checked rather than assumed, is the in-place
    // sweep** that the double buffering in `update` exists to prevent. Removing
    // the double buffer was tried against this file: an in-place sweep carries
    // much further in the direction it walks, but both directions still converge
    // on the same value within a few blocks of the source, so every probe here
    // agrees and this passes. The assertion that actually fails on that mutation
    // is the reach bound above - `far == 0` - because the extra distance is the
    // whole of what an in-place sweep changes near the source.
    //
    // Recorded because a test whose stated purpose is not what it detects is
    // worse than no test: it retires a worry that is still live. The pairing to
    // keep is that the double buffer is covered by the reach bound, not here.
    {
        Grid grid = empty_world(W, H);
        grid.set_element(64, 64, ElementType::Fire);

        LightField light(128, 128);
        light.update(grid, 0, 0);

        const int bx = 64 / B, by = 64 / B;
        bool symmetric = true;
        for (int d = 1; d <= 6; ++d) {
            if (at(light, bx - d, by) != at(light, bx + d, by)) symmetric = false;
            if (at(light, bx, by - d) != at(light, bx, by + d)) symmetric = false;
        }
        check("the falloff is symmetric about the source", symmetric);
    }

    // --- terrain shadows itself ---
    //
    // The claim V7 is actually about: a flame in a pit lights nothing behind the
    // pit's wall. Two probes the same distance from the same source, one across
    // open air and one across rock, so distance is held constant and the only
    // difference between them is what is in the way.
    {
        Grid grid = empty_world(W, H);
        grid.set_element(64, 64, ElementType::Fire);
        // A wall to the right of the fire, spanning the probe's path. Nothing to
        // the left, which is the control.
        for (int x = 68; x < 80; ++x)
            for (int y = 40; y < 90; ++y) grid.set_element(x, y, ElementType::Wall);

        LightField light(128, 128);
        light.update(grid, 0, 0);

        const int bx = 64 / B, by = 64 / B;
        const int through_rock = brightness(at(light, bx + 5, by));
        const int through_air  = brightness(at(light, bx - 5, by));

        check("light is stopped by solid terrain", through_rock < through_air,
              "rock=" + std::to_string(through_rock) + " air=" + std::to_string(through_air));
        check("the open side is genuinely lit", through_air > 0);
    }

    // --- what glows is heat, not the material ---
    //
    // In a grid of its own, with no fire anywhere. Sharing one grid with the
    // flame above put this probe just inside that flame's reach, so it measured
    // propagated light and called it emission - the two have to be separated by
    // the scene, because the field itself cannot tell them apart.
    {
        Grid grid = empty_world(W, H);
        grid.set_element(32, 32, ElementType::Charred); // burnable, but nothing has lit it

        LightField cold(64, 64);
        cold.update(grid, 0, 0);
        check("a cold cell of a burnable material does not glow", !cold.any_light());
    }

    // --- the ramp stays warm ---
    {
        Grid grid = empty_world(W, H);
        grid.set_element(32, 32, ElementType::Fire); // 250

        LightField hot(64, 64);
        hot.update(grid, 0, 0);

        const uint32_t flame = at(hot, 32 / B, 32 / B);
        check("emission is warm - red leads, blue trails",
              red(flame) >= green(flame) && green(flame) >= blue(flame),
              std::to_string(red(flame)) + "," + std::to_string(green(flame)) + "," +
                  std::to_string(blue(flame)));
    }

    // --- the same input gives the same field ---
    //
    // Not determinism for the simulation's sake - this never feeds the
    // simulation, which is the point of it living in src/render/ - but because a
    // field that drifted between identical frames would flicker on a still
    // scene.
    {
        Grid grid = empty_world(W, H);
        grid.set_element(50, 50, ElementType::Fire);

        LightField a(64, 64), b(64, 64);
        a.update(grid, 0, 0);
        b.update(grid, 0, 0);
        // And the same object updated twice, which is the case that actually runs
        // every frame: `update` reuses its buffers, so state left over from the
        // previous frame is the plausible way this breaks.
        a.update(grid, 0, 0);

        check("the field is a pure function of the grid and the origin",
              a.pixels() == b.pixels());
    }

    // --- a field re-aimed away from the fire goes dark ---
    //
    // The origin argument is what makes this camera-relative, and getting it
    // wrong would light the wrong part of the world - the same class of defect
    // as A1, arriving through a new feature.
    {
        Grid grid = empty_world(W, H);
        grid.set_element(10, 10, ElementType::Fire);

        LightField light(32, 32);
        light.update(grid, 0, 0);
        check("the view containing the fire is lit", light.any_light());

        light.update(grid, 90, 90);
        check("a view far from the fire is dark", !light.any_light());
    }

    return report();
}
