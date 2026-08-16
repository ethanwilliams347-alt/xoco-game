// V17 - the golden frame.
//
// **What this exists to catch:** the frame is composed by a run of draw calls
// whose *order* is the whole feature. Every suite in this project passes on a
// frame nobody can see through - session 2's lighting blowout proved that, and
// `preview_light` was built because of it, but `preview_light` asserts nothing.
// This is the assertion: one fixed scene, composed by the real code, hashed.
//
// **It drives `frame::compose`, not a copy of it, and that is the point rather
// than a convenience.** V17's own stated failure mode is a parallel software
// compositor that reimplements the layer order - such a thing passes happily
// while the shipped renderer is broken, because the two are different programs.
// So the only thing this file builds is the *inputs*: a grid, a camera, four
// textures. The ordering under test is read out of src/render/frame.cpp.
//
// **`SDL_CreateSoftwareRenderer`, because a GPU frame is not reproducible.**
// Drivers differ in filtering, in rounding and in how they clip a float rect,
// so an accelerated checksum would fail on a second machine and mean nothing.
// The cost is stated plainly: this checks the *software* rasterisation of the
// real draw calls. It cannot see a defect that exists only on the GPU path, and
// nothing here should be read as claiming otherwise. What it does see is every
// change to which layer is drawn, in what order, at what position, at what
// size - which is the entire content of the V11 restructure it is here to
// guard.
//
// **The textures are generated, not loaded from assets/.** A checksum over the
// shipped backdrop art would fail the moment anyone redraws a mountain, which
// is a thing they are supposed to be able to do; it would also make this suite
// depend on the build having staged assets. The patterns below are chosen only
// to be busy enough that a misplaced layer moves the hash.
// Before SDL.h, and it is load-bearing on Windows: SDL otherwise `#define`s
// `main` to `SDL_main` and expects SDL2main to supply the real one. This is a
// console program with a plain `main()` and does not link SDL2main, so without
// this the link fails on an unresolved `main` - which reads as a build-file
// mistake rather than as the one-line macro it is.
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <cstdint>
#include <cstdio>
#include <vector>
#include "game/camera.h"
#include "physics/grid.h"
#include "render/frame.h"
#include "render/light.h"
#include "test_util.h"

namespace {

// The viewport, in cells. Small on purpose - large enough to hold terrain, a
// fire, a prop and the body at once, small enough that the whole frame is a
// 480x272 surface. `padded_h` is not a multiple of LightField::BLOCK, which is
// deliberate: the light texture's block extent then genuinely overhangs the
// viewport, which is the alignment case the light pass's destination rect
// exists for.
constexpr int PADDED_W = 120;
constexpr int PADDED_H = 68;

constexpr int WORLD_W = 400;
constexpr int WORLD_H = 200;

// A fractional camera centre, so `frac_x()`/`frac_y()` are both non-zero and
// every sub-cell offset in the composition is actually exercised. A whole-cell
// camera would hash identically with all of them dropped.
constexpr float CAM_X = 183.37f;
constexpr float CAM_Y = 96.62f;

// FNV-1a over the whole surface. Not cryptographic and does not need to be -
// what it has to do is change when any pixel does.
uint64_t hash_surface(SDL_Surface* surf) {
    uint64_t h = 1469598103934665603ull;
    for (int y = 0; y < surf->h; ++y) {
        const uint8_t* row = static_cast<const uint8_t*>(surf->pixels) + y * surf->pitch;
        for (int i = 0; i < surf->w * 4; ++i) {
            h ^= row[i];
            h *= 1099511628211ull;
        }
    }
    return h;
}

// --- measuring a frame, as opposed to fingerprinting it --------------------
//
// V11's grade needs a different instrument from the checksum above, and the
// distinction is worth keeping straight because they look similar and answer
// opposite questions. A hash says "this frame is not the frame I expected" and
// nothing more; it is deliberately blind to *how* a frame differs, which is what
// makes it a good regression guard and a useless one for a claim like "the
// multiply darkens". These two say how much light is on screen.
//
// Rec. 709 weights, the same ones used to measure the depth bands' overlap when
// step 3 was scoped. Doubles rather than integers because the whole point is a
// ratio between two frames, and the surface is BGRA in memory.
double luminance_at(const uint8_t* px) {
    return 0.2126 * px[2] + 0.7152 * px[1] + 0.0722 * px[0];
}

double mean_luminance(SDL_Surface* surf) {
    double total = 0.0;
    for (int y = 0; y < surf->h; ++y) {
        const uint8_t* row = static_cast<const uint8_t*>(surf->pixels) + y * surf->pitch;
        for (int x = 0; x < surf->w; ++x) total += luminance_at(row + x * 4);
    }
    return total / (static_cast<double>(surf->w) * surf->h);
}

double max_luminance(SDL_Surface* surf) {
    double peak = 0.0;
    for (int y = 0; y < surf->h; ++y) {
        const uint8_t* row = static_cast<const uint8_t*>(surf->pixels) + y * surf->pitch;
        for (int x = 0; x < surf->w; ++x) {
            const double l = luminance_at(row + x * 4);
            if (l > peak) peak = l;
        }
    }
    return peak;
}

// A static texture filled from a generated pattern. `f` returns ARGB8888 for a
// pixel. Blend mode matches load_art_texture's, which is what the game gives
// every one of these.
SDL_Texture* pattern_texture(SDL_Renderer* r, int w, int h,
                             uint32_t (*f)(int x, int y)) {
    SDL_Texture* tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STATIC, w, h);
    if (!tex) return nullptr;
    std::vector<uint32_t> buf(static_cast<size_t>(w) * h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) buf[static_cast<size_t>(y) * w + x] = f(x, y);
    SDL_UpdateTexture(tex, nullptr, buf.data(), w * static_cast<int>(sizeof(uint32_t)));
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}

uint32_t sky_pattern(int x, int y) {
    return 0xFF000000u | (static_cast<uint32_t>(0x14 + (y / 7)) << 16) |
           (static_cast<uint32_t>(0x10 + (x / 23)) << 8) | 0x22u;
}

// Transparent above a jagged ridge, opaque below it - the silhouette a
// parallaxed mountain layer actually has, so a wrong y offset shows up in the
// hash rather than being hidden by a full-rect fill.
uint32_t mountain_pattern(int x, int y) {
    const int ridge = 30 + ((x * 7) % 23);
    return y < ridge ? 0x00000000u : 0xFF2B2438u;
}

// V19's ground plane tile. **Arted rather than left null, and the rule it is
// obeying is written down in .claude/rules/simulation.md**: a null texture draws
// nothing, so a checksum over a configuration with one covers that layer's
// *absence* and says nothing about its position, its order or its strip
// arithmetic. The pattern is a vertical ramp with sparse horizontal dashes -
// the same two things the real tile carries, because both are what the strip
// loop transforms. A flat fill would hash identically with every strip's source
// rect wrong.
uint32_t ground_pattern(int x, int y) {
    const uint32_t ramp = static_cast<uint32_t>(0x18 + y / 12);
    if ((x * 5 + y * 13) % 97 < 6) return 0xFF3C3452u;   // a mark
    return 0xFF000000u | (ramp << 16) | ((ramp - 4) << 8) | (ramp + 26);
}

uint32_t prop_pattern(int x, int y) {
    return (x + y) % 5 == 0 ? 0x00000000u : 0xFF1C3320u;
}

uint32_t player_pattern(int x, int y) {
    return (x * 3 + y) % 7 == 0 ? 0x00000000u : 0xFF6E7C99u;
}

// The scene. Deterministic by construction - a fixed seed and hand-placed
// cells, no `update()` at all, because what is under test is the composition
// and a stepped world would make this suite fail whenever the simulation
// legitimately changed.
void stamp_world(Grid& grid) {
    for (int x = 0; x < WORLD_W; ++x) {
        const int surface = 120 + (x % 17) - (x / 40);
        for (int y = surface; y < WORLD_H; ++y)
            grid.set_element(x, y, y < surface + 6 ? ElementType::Sand : ElementType::Wall);
    }
    // Water in a dip, and a fire so the light pass has something to do - an
    // unlit frame skips the light draw entirely, so a checksum taken without
    // one would not cover it.
    for (int x = 150; x < 190; ++x)
        for (int y = 112; y < 120; ++y) grid.set_element(x, y, ElementType::Water);
    for (int x = 205; x < 209; ++x)
        for (int y = 104; y < 108; ++y) grid.set_element(x, y, ElementType::Fire);
}

} // namespace

int main() {
    // No SDL_Init(SDL_INIT_VIDEO): a software renderer over a surface needs no
    // display, which is what lets this run headless on a build machine.
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
        0, PADDED_W * Camera::SCALE, PADDED_H * Camera::SCALE, 32,
        SDL_PIXELFORMAT_ARGB8888);
    check("surface created", surface != nullptr, SDL_GetError());
    if (!surface) return report();

    SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);
    check("software renderer created", renderer != nullptr, SDL_GetError());
    if (!renderer) return report();

    Grid grid(WORLD_W, WORLD_H, 1234567u);
    stamp_world(grid);

    Camera camera;
    camera.follow(CAM_X, CAM_Y, PADDED_W, PADDED_H, WORLD_W, WORLD_H);

    // The cell texture, uploaded exactly the way main.cpp uploads it - the
    // visible rect out of the grid's own pixel buffer, at the grid's pitch.
    SDL_Texture* cells = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STREAMING, PADDED_W, PADDED_H);
    check("cell texture created", cells != nullptr, SDL_GetError());
    SDL_SetTextureBlendMode(cells, SDL_BLENDMODE_BLEND);
    {
        const std::vector<uint32_t>& pixels = grid.get_pixels();
        const SDL_Rect visible{0, 0, PADDED_W, PADDED_H};
        const uint32_t* src = pixels.data() + camera.view_y() * WORLD_W + camera.view_x();
        SDL_UpdateTexture(cells, &visible, src, WORLD_W * static_cast<int>(sizeof(uint32_t)));
    }

    LightField light(PADDED_W, PADDED_H);
    light.update(grid, camera.view_x(), camera.view_y());
    check("the fixture scene is lit", light.any_light(),
          "no light means the light pass is not in the checksum at all");
    SDL_Texture* light_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                               SDL_TEXTUREACCESS_STREAMING,
                                               light.cols(), light.rows());
    SDL_SetTextureBlendMode(light_tex, SDL_BLENDMODE_ADD);
    SDL_SetTextureScaleMode(light_tex, SDL_ScaleModeLinear);
    SDL_UpdateTexture(light_tex, nullptr, light.pixels().data(),
                      light.cols() * static_cast<int>(sizeof(uint32_t)));

    frame::Params p;
    p.camera = &camera;
    p.padded_w = PADDED_W;
    p.padded_h = PADDED_H;
    p.backdrop.sky = pattern_texture(renderer, 700, 400, sky_pattern);
    p.backdrop.sky_w = 700;
    p.backdrop.sky_h = 400;
    p.backdrop.mountains = pattern_texture(renderer, 700, 300, mountain_pattern);
    p.backdrop.mountain_w = 700;
    p.backdrop.mountain_h = 300;
    // The plane's tile, at the size the generator actually writes. **Not scaled
    // down to match the small fixture window**, because the tile size is what
    // decides how many wrapping copies a window needs, and a tile shrunk to fit
    // would exercise a copy count the game never issues.
    p.backdrop.ground = pattern_texture(renderer, 256, 256, ground_pattern);
    p.backdrop.ground_w = 256;
    p.backdrop.ground_h = 256;
    p.cells = cells;

    SDL_Texture* prop_tex = pattern_texture(renderer, 24, 40, prop_pattern);
    const std::vector<frame::Prop> props = {
        frame::Prop{prop_tex, 24, 40, 160.0f, 121.0f},
        frame::Prop{prop_tex, 24, 40, 215.0f, 118.0f},
        // A null texture, because main.cpp's list can hold one and the skip is
        // a branch in the code under test.
        frame::Prop{nullptr, 24, 40, 240.0f, 118.0f},
    };
    p.props = &props;

    p.has_objective = true;
    p.objective_x = 230;
    p.objective_y = 108;

    p.player_tex = pattern_texture(renderer, 56, 52, player_pattern); // 4 cols x 2 rows
    p.player_x = 181.4f;
    p.player_y = 100.8f;
    p.facing_left = true;
    p.sheet_col = 2;
    p.sheet_row = 1;
    p.player_box_w = 8;
    p.player_box_h = 20;

    p.light = &light;
    p.light_texture = light_tex;

    check("every fixture texture created",
          p.backdrop.sky && p.backdrop.mountains && p.backdrop.ground && prop_tex && p.player_tex && light_tex,
          SDL_GetError());

    // **The layer table's shape, asserted here rather than only in the
    // compiler.** frame.cpp holds the Lit/Light/Unlit invariant with
    // static_asserts, which is the stronger guard and is where it belongs. What
    // that cannot catch is the table silently becoming empty or losing the
    // light pass through some future edit that still compiles - so the count
    // and the one boundary entry are checked from the outside too, against the
    // list frame.h exports for exactly this.
    int lit = 0, graded = 0, boundary = 0, unlit = 0;
    for (int i = 0; i < frame::LAYER_COUNT; ++i) {
        switch (frame::LAYERS[i].lighting) {
            case frame::Lighting::Lit: ++lit; break;
            case frame::Lighting::Grade: ++graded; break;
            case frame::Lighting::Light: ++boundary; break;
            case frame::Lighting::Unlit: ++unlit; break;
        }
    }
    check("the layer table has exactly one light pass", boundary == 1,
          "Lit and Unlit are defined against it; without it they mean nothing");
    check("at most one world-wide grade", graded <= 1,
          "two full-screen multiplies compose into a third grade nothing declares");
    check("every world layer is lit", lit == frame::LAYER_COUNT - 1 - graded && unlit == 0,
          "UI belongs in main.cpp, after the light pass - a layer arriving here "
          "unlit means the world/UI boundary has moved and B1 is reachable again");

    // **Step 3's grade pass is exercised by this frame only through the
    // mountains' per-layer multiply, and that is worth being explicit about.**
    // `Params::world_grade` defaults to identity and `draw_grade` returns before
    // its first draw call when it is, so the fixture below composes with the
    // quad *skipped*. The checksum therefore covers the per-layer half of V11's
    // multiply and not the world-wide half.
    //
    // So the world-wide half gets its own check rather than riding on the
    // golden number: compose once more with a non-identity grade and require
    // the frame to move. That is the same instrument as "the checksum moves
    // when the frame does" below, pointed at the one mechanism the main fixture
    // cannot reach - without it, `draw_grade` could early-return unconditionally
    // and every check in this file would still pass.
    check("the grade pass is skipped at identity", p.world_grade.identity(),
          "the fixture is supposed to compose without the quad, so that the golden "
          "checksum keeps meaning what it meant before step 3");

    frame::compose(renderer, p);
    const uint64_t first = hash_surface(surface);

    // **The composed frame's checksum.** Updating this number is a legitimate
    // thing to do and is exactly what the suite is for: it must be updated in
    // the *same commit* as the change that moved it, so `git log -S` on this
    // line lists every frame the renderer has ever produced. Updating it in a
    // separate tidy-up commit destroys that, which is the only way to misuse
    // this test.
    //
    // It moved once, at V17, from "no checksum" to this one - the frame at that
    // point being byte-identical to the one main.cpp had composed inline since
    // V8, since the extraction changed nothing but where the lines live.
    //
    // **V11 moved it twice on 2026-08-16 and moved it back, and the number
    // below is V17's, unchanged. That is not a no-op commit - it is the
    // strongest reading this test has ever produced.**
    //
    // V11 restructured the composition into an ordered layer table, moved the
    // parallax arithmetic onto Camera, generated the factors into a header, and
    // added a mid-ground band. The first three must change no pixel, so they
    // were built and run first against the V17 number, which held. The band
    // then moved it to 0x06f6412da7af6607. The band was then removed - the
    // played-frame check that notes/reference_observations.txt entry 4 asked
    // for came back saying our terrain already fills that part of the frame -
    // and the checksum returned to 0x3d729ad7fbcaa839 exactly.
    //
    // **Returning to the identical value is a second, independent proof that
    // everything else in V11 was a no-op**, and it is stronger than the first
    // one: the first rested on running the restructure alone before the band
    // existed, while this one says the whole of V11 - table, camera method,
    // generated header, a layer added and removed - composes the same frame the
    // inline code did at V8. A hash collision is the only other explanation and
    // it is not a serious one.
    //
    // Do not read this as "the frame has never changed". `git log -S` on this
    // constant is still the right instrument, and it will show three commits
    // where a naive reading of the file's history shows one.
    //
    // **Step 3 moved it for the first time, on 2026-08-16, to the value below.**
    // The same procedure was used and it is the one to copy: the whole grade
    // mechanism - the `Grade` struct, the per-layer field, the widened draw
    // signatures, the `Lighting::Grade` row and the quad itself - was built with
    // every grade at identity and run against 0x3d729ad7fbcaa839 first, which
    // held. So none of that machinery touches a pixel by existing, and this
    // number moved for exactly one reason: the mountains layer carries a 0.60
    // multiply. **That is the first commit in this project's history where the
    // composed frame changed on purpose**, and the whole point of running the
    // no-op half first is that the diff cannot hide a second cause inside it.
    //
    // **V19's ground plane moved it again on 2026-08-16, and the house
    // procedure had to be *adapted* rather than applied, which is the part
    // worth reading.** The rule is "ship the no-op half against the old
    // checksum first, so the diff cannot hide a second cause". Every previous
    // use had a no-op half available: the V11 restructure moved lines, and the
    // grade mechanism existed at identity. **A new band that draws pixels has
    // no such half** - a layer that composes to the old checksum is a layer
    // that is not in the frame, and leaving its texture null to arrange that is
    // the exact anti-pattern .claude/rules/simulation.md names.
    //
    // So the separation was bought with two numbers instead of one. The plane
    // was built and run with its per-layer grade at identity, which gave
    // **0xfd8e2f04b7037278**; only then did the grade go in, giving the number
    // below. The first number is the geometry - the strip loop, the wrapping
    // copies, the source mapping, the position in the table - and the step from
    // it to the second is the grade and nothing else. Both are stated here
    // because the intermediate is not recoverable from the file afterwards, and
    // it is the one that says the two causes were never mixed.
    //
    // Fifth move. `git log -S` on this constant remains the instrument.
    constexpr uint64_t GOLDEN = 0x24eb769681836a0eull;
    char detail[128];
    std::snprintf(detail, sizeof(detail), "got 0x%016llx, expected 0x%016llx",
                  static_cast<unsigned long long>(first),
                  static_cast<unsigned long long>(GOLDEN));
    check("the composed frame matches the golden checksum", first == GOLDEN, detail);

    // Composing the same inputs twice must give the same frame. This is not
    // redundant with the check above: it separates "the renderer changed" from
    // "this suite is reading uninitialised memory", and only one of those is
    // worth anybody's afternoon.
    frame::compose(renderer, p);
    check("composition is repeatable", hash_surface(surface) == first,
          "the same inputs produced two different frames");

    // And the checksum has to be *sensitive*, or a passing golden test is
    // worth nothing. One cell of camera movement is the smallest change the
    // composition can be asked to notice, and it moves every layer at once -
    // each by a different amount, which is what the parallax factors are.
    camera.follow(CAM_X + 1.0f, CAM_Y, PADDED_W, PADDED_H, WORLD_W, WORLD_H);
    frame::compose(renderer, p);
    check("the checksum moves when the frame does", hash_surface(surface) != first,
          "a one-cell camera move left the frame identical - the checksum is not "
          "watching what it claims to");

    // --- V11's fourth bullet: the pass has to actually darken ----------------
    //
    // Two claims, and neither is covered by anything above. `world_grade` is
    // identity in the fixture, so `draw_grade` returns immediately and the
    // golden checksum has never seen the quad; an unconditional early return
    // would pass every other check in this file.
    //
    // **Measured as mean luminance rather than as a checksum**, because "the
    // frame changed" is not the claim - the claim is that it got *darker*, which
    // is the whole of what V7's additive pass could not do. A hash cannot tell
    // those apart, and a grade that brightened would satisfy one and not the
    // other.
    camera.follow(CAM_X, CAM_Y, PADDED_W, PADDED_H, WORLD_W, WORLD_H);
    frame::compose(renderer, p);
    const double plain_mean = mean_luminance(surface);

    p.world_grade = frame::Grade{128, 128, 128};
    frame::compose(renderer, p);
    const double graded_mean = mean_luminance(surface);

    char graded_detail[192];
    std::snprintf(graded_detail, sizeof(graded_detail),
                  "mean luminance %.2f ungraded, %.2f at half grade",
                  plain_mean, graded_mean);
    check("a half grade darkens the composed frame", graded_mean < plain_mean * 0.75,
          graded_detail);

    // **And the ordering, as a number rather than as a comment.**
    //
    // The first attempt at this compared peak luminance and it could not do the
    // job: the fixture's fire already saturates at 255 ungraded, so the number a
    // correct ordering produces and the number a reversed one produces are both
    // squeezed against the clip and land 12% apart. That is not a margin worth
    // building a check on, and it is recorded here because the check *looked*
    // right - "the brightest pixel stays bright" is a true statement about the
    // design and a bad instrument for it.
    //
    // What discriminates cleanly is the light pass's *contribution*: compose
    // with and without it and subtract. Drawn before the light, as declared, the
    // grade cannot touch that difference and it comes out the same in both
    // frames. Drawn after it, the difference is multiplied along with everything
    // else and comes out at half. The threshold sits at 0.8, between 1.0 and
    // 0.5, so it separates the two orderings and not two tunings.
    //
    // **The static_assert in frame.cpp catches a reversed table first**, and was
    // verified against a reversed one before this was written - so this is not
    // the primary guard and is not pretending to be. It is the guard on the case
    // the compiler cannot see: `rank()` and the table edited together, which is
    // exactly what someone would do to "fix the build" after moving a row.
    const LightField* saved_light = p.light;

    p.light = nullptr;                       // draw_light returns before drawing
    frame::compose(renderer, p);
    const double graded_unlit = mean_luminance(surface);
    p.world_grade = frame::Grade{};
    frame::compose(renderer, p);
    const double plain_unlit = mean_luminance(surface);
    p.light = saved_light;

    const double plain_contribution = plain_mean - plain_unlit;
    const double graded_contribution = graded_mean - graded_unlit;

    std::snprintf(graded_detail, sizeof(graded_detail),
                  "the light pass adds %.3f ungraded and %.3f at half grade; drawn "
                  "after the grade instead it would add about %.3f",
                  plain_contribution, graded_contribution, plain_contribution * 0.5);
    check("the light pass carries a fixture the grade can be measured against",
          plain_contribution > 0.05, graded_detail);
    check("the light pass is not dimmed by the grade",
          graded_contribution > plain_contribution * 0.8, graded_detail);

    SDL_DestroyTexture(p.player_tex);
    SDL_DestroyTexture(prop_tex);
    SDL_DestroyTexture(p.backdrop.ground);
    SDL_DestroyTexture(p.backdrop.mountains);
    SDL_DestroyTexture(p.backdrop.sky);
    SDL_DestroyTexture(light_tex);
    SDL_DestroyTexture(cells);
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    return report();
}
