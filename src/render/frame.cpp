#include "render/frame.h"
#include "render/backdrop_layers.h"
#include "render/backdrop_wrap.h"
#include "render/player_sprite.h"

namespace frame {
namespace {

// --- the layers, one function each ----------------------------------------
//
// V11 turned the run of draw calls this file held at V17 into the table at the
// bottom. **The bodies below are the V17 lines unchanged** apart from three
// things, each of which is one of V11's bullets: the parallax offsets are asked
// of the camera instead of assembled from three of its outputs at the draw
// site, the factors are read from the generated backdrop_layers.h instead of
// from constants that had a copy in the Python, and there is a mid-ground band
// where there was nothing. Everything else - the order, the rects, the
// arithmetic - is what it was, and the golden checksum is what says so.

// Multiply one channel by a grade channel, the way SDL's own texture colour mod
// does it: `v * m / 255`, truncating. Matching SDL rather than rounding half-up
// matters because both paths are live in the same frame - a graded rectangle and
// a graded texture sitting side by side must not land on different values for
// the same grade. 255 is exactly identity either way, which is what the golden
// checksum leans on.
constexpr uint8_t graded(uint8_t v, uint8_t m) {
    return static_cast<uint8_t>(v * m / 255);
}

// Apply a layer's grade to a texture, and **always apply it, including when it
// is identity.** Colour mod is state that lives on the texture, not on the draw
// call, so a layer that sets it only when it has something to say leaves the
// previous layer's mod on a texture it shares - and the props, the cell texture
// and the player sheet all outlive the frame. Setting it unconditionally costs
// nothing and makes each layer's grade a property of that layer rather than of
// whatever ran before it.
void apply_grade(SDL_Texture* tex, const Grade& g) {
    if (tex) SDL_SetTextureColorMod(tex, g.r, g.g, g.b);
}

// **The clear is load-bearing now in a way it was not before.** V1's
// 64-band gradient filled the whole window every frame, so it doubled
// as a clear and nothing here ever needed one. The authored sky that
// replaced it is drawn behind an `if` - a missing or unreadable BMP
// leaves the framebuffer holding whatever was in it, which on a
// double-buffered renderer is two-frames-ago garbage rather than a
// plain background. Clearing to the palette's darkest sky tone means
// the failure mode is "the backdrop is flat" instead of "the window
// is full of noise".
void draw_clear(SDL_Renderer* renderer, const Params&, const Grade& g) {
    // `sky_horizon`, tools/pixel_art.py - hand-copied, and the copy went stale
    // once. **This read `0x14, 0x10, 0x22` with the comment `sky_deep` until V21
    // (2026-08-16), and both halves of that were wrong by then.** V20 raised the
    // backdrop group wholesale and did not carry the change here, so the clear
    // sat at luminance 18 while the sky above it had moved to 62-95 - and V20
    // had *also* inverted the sky ramp, which made `sky_deep` the brighter of
    // the pair and `sky_horizon` the darkest sky tone this comment says to use.
    // The value was invisible in the ordinary frame, because the sky texture
    // covers the window, which is exactly why nothing caught it.
    //
    // It is still hand-copied. `.claude/rules/assets-and-formats.md` is blunt
    // that two copies of a constant with a comment between them is not
    // enforcement, and this is now the second duplicated backdrop constant to go
    // stale silently. Generating it into `backdrop_layers.h` alongside the
    // parallax factors is the fix and is written up in ENGINEERING_NOTES.md
    // rather than done here, because V21 is a retune and a retune that quietly
    // grows a code path is how a measurement stops being bracketed.
    SDL_SetRenderDrawColor(renderer, graded(0x38, g.r), graded(0x2C, g.g),
                           graded(0x57, g.b), 255);
    SDL_RenderClear(renderer);
}

// The backdrop layers (V8, replacing V1's gradient placeholder now that there
// is authored art to show). Static textures, each shifted by the camera's
// continuous view position scaled by SCALE and the layer's own parallax factor
// - `Camera::parallax_origin_x/y`, which is where that arithmetic lives as of
// V11. Full-texture draws with a negative destination offset rather than a
// cropped source rect: the art is static, so there is nothing to re-upload per
// frame, only where it is drawn needs to move.
void draw_backdrop_layer(SDL_Renderer* renderer, const Camera& camera,
                         SDL_Texture* tex, int w, int h,
                         const backdrop_layers::Layer& layer, const Grade& g) {
    if (!tex) return;
    apply_grade(tex, g);
    const SDL_FRect dst{
        camera.parallax_origin_x(layer.parallax_x),
        camera.parallax_origin_y(layer.parallax_y),
        static_cast<float>(w), static_cast<float>(h)
    };
    SDL_RenderCopyF(renderer, tex, nullptr, &dst);
}

void draw_sky(SDL_Renderer* renderer, const Params& p, const Grade& g) {
    draw_backdrop_layer(renderer, *p.camera, p.backdrop.sky,
                        p.backdrop.sky_w, p.backdrop.sky_h, backdrop_layers::SKY, g);
}

void draw_mountains(SDL_Renderer* renderer, const Params& p, const Grade& g) {
    draw_backdrop_layer(renderer, *p.camera, p.backdrop.mountains,
                        p.backdrop.mountain_w, p.backdrop.mountain_h,
                        backdrop_layers::MOUNTAINS, g);
}

// **A mid-ground band was built here, deleted, and the deletion's own reopen
// trigger then fired. Read all three parts before touching this area.**
//
// V11 built one - a slot at parallax 0.40 between the mountains and the world -
// because notes/reference_observations.txt entry 4 found that band doing most of
// the depth work in five of eight reference frames, and our stack went from 0.15
// straight to 1.00 with nothing in between. **Entry 4 also wrote down what would
// disprove it**: our world is 1080 cells tall while the camera sees 270 of them,
// so a *simulated* world might fill that band with terrain by itself where a
// hand-painted one cannot. Checked against a played frame the same day, the
// terrain already occupied it, and the row came back out. The reopen trigger
// recorded then was **a location whose terrain does not fill that band.**
//
// **On 2026-08-16 V19 fired that trigger by screenshot, which is what admits the
// layer below.** resources/game_screenshots/visual_rework_1.png, measured in
// Rec. 709 luminance: the mid band is 70% bare sky, and the region below the
// terrain's skyline is a *flat fill* - luminance spread exactly 0.0 across 400
// rows and 37% of the frame, sitting at 22.7 against the upper sky's 22.3. Four
// tenths of a level out of 255 between the nearest surface in the frame and one
// of the most distant. The deletion was right for the band it was about and the
// trigger was right about what would change it; both stand.
//
// **What goes in is not what came out.** The deleted band was a silhouette at
// one factor. The ground plane below is a *surface* at a range of factors, drawn
// behind the world, and the near silhouette in front of it stays the simulated
// terrain - deliberately, because a painted band in front of the world would
// occlude the one verb the game has. The near ridge and the shore treeline,
// which are the rows that genuinely land back in the deleted band, are the next
// three commits and not this one.

// --- V19's ground plane ----------------------------------------------------
//
// **The one piece of new rendering V19 costs, and the reason it is new is that
// a receding plane has no single depth.** Drawn flat at one parallax factor it
// reads as a wall standing behind the world; drawn as N strips between two
// factors it reads as ground going away. The arithmetic - which strip is at
// which depth, what it samples, and where the wrapping copies go - is in
// render/backdrop_wrap.h, tested headless in tests/test_backdrop.cpp, and this
// function does nothing but turn its answers into SDL_RenderCopy calls. That
// split is the same one render/player_anim.cpp has with the sprite sheet.
//
// **Two constants live here rather than in the generated header, because they
// are composition and not parallax.** The factors come from the header, which is
// generated from the same table that sizes the tile; these two are where the
// band sits in the frame and how finely it is cut, and both are TUNING.md rows.
//
// STRIPS is the cost knob and it is a real one: the plane issues STRIPS *
// (copies per strip) draw calls every frame, against one for every other
// backdrop layer. 24 was chosen as the point where the factor stepping between
// adjacent strips stops being visible as banding at 1920x1080; it is not
// measured against a frame budget, because grid_bench times the simulation and
// cannot see a draw call at all. The honest instrument for it is the frame rate
// in the running game, which is checklist work.
constexpr int GROUND_STRIPS = 24;

// **Where the plane's far edge sits used to be a fraction of the window height,
// and V20 replaced it with a row of the mountains BMP. The replacement is the
// fix for "mountains are not visible just the plane" (playtest session 6).**
//
// The old constant was `GROUND_HORIZON_FRACTION = 0.55f`, justified as "where
// the played frame's terrain skyline already sits". That justification named the
// *terrain* and the layer it collides with is the *mountains*, which were
// authored independently, in their own image's coordinates, by a different
// script. The two were contradictory at every camera position the world reaches:
// the plane's horizon ran between screen rows 594 and 238 while the mountain
// silhouette began at 604 and ran to 1124. The plane is opaque RGB with no
// colour key and its row in LAYERS below is drawn *after* the mountains', so it
// covered the band completely. Nothing was faint and no grade was
// over-correcting - measured, the mountains against the sky were the largest
// contrast anywhere in the frame.
//
// **A fraction of the window was the right shape of constant and the wrong
// space to state it in.** The reason a fraction was chosen still holds: the
// window is switchable at runtime (1920x1080, 2560x1440, 3440x1440) and a
// horizon in window pixels would sit at three different heights. But the plane's
// far edge is not a fact about the window at all - it is the place where the
// ground meets the mountains, which is a fact about the mountains. Stated there,
// it is resolution-independent for free and it cannot contradict the art,
// because it *is* the art: backdrop_layers::MOUNTAINS_SKYLINE_MAX_ROW is
// generated from the same seeded walk that draws the silhouette.
//
// The deepest row of the skyline, and not the highest, so the whole jagged edge
// stands clear above the plane and only the solid body below it is covered.
//
// `mountain_h` is the loaded texture's height rather than the generated one, so
// the horizon follows the art that is actually on screen. That matters for more
// than tidiness: the golden-frame fixture builds a 300-row synthetic mountain
// texture where the shipped BMP is 1642, and against an absolute row index the
// plane fell off the bottom of the fixture's window and stopped being in the
// checksum at all.
float ground_horizon_y(const Camera& camera, int mountain_h) {
    return camera.parallax_origin_y(backdrop_layers::MOUNTAINS.parallax_y) +
           static_cast<float>(mountain_h) * backdrop_layers::MOUNTAINS_SKYLINE_MAX;
}

void draw_ground(SDL_Renderer* renderer, const Params& p, const Grade& g) {
    SDL_Texture* tex = p.backdrop.ground;
    if (!tex || p.backdrop.ground_w <= 0 || p.backdrop.ground_h <= 0) return;
    apply_grade(tex, g);

    const Camera& camera = *p.camera;
    const int window_w = p.padded_w * Camera::SCALE;
    const int window_h = p.padded_h * Camera::SCALE;

    // The horizon moves with the camera at the **mountains'** vertical factor
    // and not the plane's own, which is the second half of the same correction.
    // A receding plane's far edge is the most distant thing in the frame - it is
    // at infinity by construction - so its parallax factor has to be the
    // *smallest* in the scene, not the plane's near-edge one. Given 0.11 it slid
    // upward at nearly twice the rate of the band it recedes toward, which is
    // the motion half of the same defect: even a correctly placed horizon would
    // have climbed past the mountains within a few hundred cells of descent.
    //
    // The band then runs from wherever that lands to the bottom of the window,
    // so it always meets the near edge exactly.
    const backdrop_wrap::Plane plane{
        ground_horizon_y(camera, p.backdrop.mountain_h),
        static_cast<float>(window_h),
        backdrop_layers::GROUND.parallax_x,
        backdrop_layers::GROUND_NEAR_X,
        p.backdrop.ground_h
    };

    for (int i = 0; i < GROUND_STRIPS; ++i) {
        const backdrop_wrap::Strip s = backdrop_wrap::plane_strip(plane, i, GROUND_STRIPS);
        if (s.dst_h <= 0.0f || s.src_h <= 0.0f) continue;
        // Strips above the window happen whenever the camera is low enough to
        // push the horizon off the top, which is most of the world's height.
        // Skipping them is the difference between 24 draw calls and 24 draw
        // calls plus their tiling, for rows nobody can see.
        if (s.dst_y + s.dst_h <= 0.0f || s.dst_y >= static_cast<float>(window_h)) continue;

        const backdrop_wrap::Tiling t = backdrop_wrap::wrap_axis(
            camera.parallax_origin_x(s.factor), p.backdrop.ground_w, window_w);

        // The source rect is integer, so it rounds; the destination stays float,
        // the way every other layer's does. Rounding the destination is the A1
        // defect coming back on a new layer - the plane would jerk in whole
        // pixels while the world under it scrolls smoothly.
        //
        // **The two rows come from plane_src_row, one boundary at a time, and
        // not from rounding this strip's start and height independently.** The
        // latter is what shipped and it is half of the black banding the tester
        // saw: two neighbouring strips rounded in isolation do not meet, so the
        // texture repeats a row at some boundaries and skips one at others. The
        // argument is at plane_src_row, and the other half of the defect was in
        // the tile's art.
        //
        // A strip whose two boundaries round to the same row is one whose depth
        // range has collapsed below a single texel, which happens at the horizon
        // end where the compression is steepest. It gets one row rather than
        // being skipped: a skipped strip is a transparent gap in the destination
        // band, which is the defect being fixed, only worse.
        const int row0 = backdrop_wrap::plane_src_row(plane, i, GROUND_STRIPS);
        const int row1 = backdrop_wrap::plane_src_row(plane, i + 1, GROUND_STRIPS);
        SDL_Rect src{0, row0, p.backdrop.ground_w, row1 - row0 > 0 ? row1 - row0 : 1};
        if (src.y + src.h > p.backdrop.ground_h) src.h = p.backdrop.ground_h - src.y;
        if (src.h <= 0) continue;

        for (int c = 0; c < t.count; ++c) {
            const SDL_FRect dst{
                t.first + static_cast<float>(c) * p.backdrop.ground_w,
                s.dst_y, static_cast<float>(p.backdrop.ground_w), s.dst_h
            };
            SDL_RenderCopyF(renderer, tex, &src, &dst);
        }
    }
}

// V4's props. Drawn before the cell texture on purpose - see the
// Prop comment in frame.h - so a trunk that overlaps authored
// terrain gets buried by it with no depth test and no new code path,
// exactly the way the cell texture already occludes the backdrop
// wherever a cell is not Empty.
void draw_props(SDL_Renderer* renderer, const Params& p, const Grade& g) {
    if (!p.props) return;
    const Camera& camera = *p.camera;
    for (const Prop& prop : *p.props) {
        if (!prop.texture) continue;
        apply_grade(prop.texture, g);
        const SDL_FRect dst{
            camera.world_to_screen_x(prop.anchor_x - prop.w / 2.0f),
            camera.world_to_screen_y(prop.anchor_y - static_cast<float>(prop.h)),
            static_cast<float>(camera.scale_length(prop.w)),
            static_cast<float>(camera.scale_length(prop.h))
        };
        SDL_RenderCopyF(renderer, prop.texture, nullptr, &dst);
    }
}

// Drawn shifted by the camera's sub-cell remainder, which is the half of
// A1 that smoothing the player alone would not have fixed: the view is
// unclamped wherever the player usually is, so the player sits near
// screen centre and it is the *world* that scrolls. In whole cells that
// is a 4-pixel jerk of everything on screen at once.
void draw_cells(SDL_Renderer* renderer, const Params& p, const Grade& g) {
    const Camera& camera = *p.camera;
    apply_grade(p.cells, g);
    const SDL_FRect world_dst{
        -camera.frac_x() * Camera::SCALE,
        -camera.frac_y() * Camera::SCALE,
        static_cast<float>(p.padded_w * Camera::SCALE),
        static_cast<float>(p.padded_h * Camera::SCALE)
    };
    SDL_RenderCopyF(renderer, p.cells, nullptr, &world_dst);
}

// --- S0's objective marker ---
//
// **Drawn in world cells, not screen pixels**, which is the opposite
// choice from the reticle and for the opposite reason: the reticle
// is a cursor and has to keep its legibility at any scale, while this is
// a thing that is *somewhere* and has to sit still in the world as the
// camera moves over it. A screen-space marker would slide against the
// terrain it is standing on.
//
// Drawn after the world and before the player, so the body passes in
// front of it - reaching an objective you are standing on top of should
// not leave you hidden behind it - and before the light pass, which is
// additive and therefore leaves an unlit marker at exactly the colour
// written here rather than dimming it into the terrain.
//
// Three concentric squares rather than a sprite: no new asset, no
// manifest entry, and the dark ring is what stops it disappearing
// against sand for the same reason the reticle has an outline.
void draw_objective(SDL_Renderer* renderer, const Params& p, const Grade& g) {
    if (!p.has_objective) return;
    const Camera& camera = *p.camera;
    const float gx = static_cast<float>(p.objective_x);
    const float gy = static_cast<float>(p.objective_y);
    struct Ring { int cells; uint8_t r, g, b; };
    const Ring rings[3] = {
        // A near-black outline, and **deliberately not tied to the backdrop.**
        // This read "sky_deep, the same dark the frame clears to" until V21
        // (2026-08-16), by which point both halves were false: V20 had moved
        // `sky_deep` and V21 moved the clear to `sky_horizon`. **The value is
        // kept exactly where it was, because the comment was describing a
        // coincidence as if it were a constraint** - this ring's job is to stay
        // legible against *sand*, the same job the reticle's outline has, and
        // nothing about it wants to track the sky. Retuning it to chase a
        // backdrop change would have been the actual defect.
        { 12, 0x14, 0x10, 0x22 },
        { 10, 0xF0, 0xC0, 0x40 },
        {  4, 0xFF, 0xFF, 0xFF },
    };
    for (const Ring& ring : rings) {
        SDL_SetRenderDrawColor(renderer, graded(ring.r, g.r), graded(ring.g, g.g),
                               graded(ring.b, g.b), 255);
        const SDL_FRect box{
            camera.world_to_screen_x(gx - ring.cells / 2.0f),
            camera.world_to_screen_y(gy - ring.cells / 2.0f),
            static_cast<float>(camera.scale_length(ring.cells)),
            static_cast<float>(camera.scale_length(ring.cells))
        };
        SDL_RenderFillRectF(renderer, &box);
    }
}

// The player is not a cell, so it is not in the pixel buffer either -
// it is drawn on top of the world as its own sprite. Float rect and
// float position: rounding either one here is the defect coming back.
//
// Positioned by subtracting the offsets from the *box's* corner, which
// is what "anchored to the box's bottom-centre" works out to - the
// sprite's baseline lands on the box's baseline and its extra width is
// split evenly either side.
void draw_player(SDL_Renderer* renderer, const Params& p, const Grade& g) {
    const Camera& camera = *p.camera;
    const SDL_FRect body{
        camera.world_to_screen_x(p.player_x - player_sprite::OFFSET_X),
        camera.world_to_screen_y(p.player_y - player_sprite::OFFSET_Y),
        static_cast<float>(camera.scale_length(player_sprite::FRAME_W)),
        static_cast<float>(camera.scale_length(player_sprite::FRAME_H))
    };
    if (p.player_tex) {
        apply_grade(p.player_tex, g);
        const SDL_RendererFlip flip =
            p.facing_left ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

        // The source rect is the whole of "this is a sheet rather than a
        // sprite" on this side of the boundary. Which cell it names is
        // decided in render/player_anim.cpp, which is SDL-free and tested.
        const SDL_Rect src{
            p.sheet_col * player_sprite::FRAME_W,
            p.sheet_row * player_sprite::FRAME_H,
            player_sprite::FRAME_W, player_sprite::FRAME_H
        };
        SDL_RenderCopyExF(renderer, p.player_tex, &src, &body, 0.0, nullptr, flip);
    } else {
        // The pre-V3 rectangle, kept as the fallback rather than deleted.
        // A missing asset should degrade to a visible player, not to an
        // invisible one - load_art_texture already printed why it failed,
        // and a game you can still move around in is a better diagnostic
        // than a world with nothing in it.
        SDL_SetRenderDrawColor(renderer, graded(235, g.r), graded(235, g.g),
                               graded(245, g.b), 255);
        const SDL_FRect box{
            camera.world_to_screen_x(p.player_x),
            camera.world_to_screen_y(p.player_y),
            static_cast<float>(camera.scale_length(p.player_box_w)),
            static_cast<float>(camera.scale_length(p.player_box_h))
        };
        SDL_RenderFillRectF(renderer, &box);
    }
}

// --- V11's fourth bullet: the pass that can darken -------------------------
//
// One full-screen rectangle in SDL_BLENDMODE_MOD, which is `dst = dst * src`.
// That is the operation the renderer has been missing since V7: everything
// before it could only ever add, so no biome, no time of day and no depth band
// could be darker than the art as authored.
//
// **It sits before the light pass and that ordering is the design, not an
// accident of where it got typed.** Multiply first, add second, so a fire at
// night burns at the brightness V7 computed for it instead of being graded down
// with the rock it is standing on. The reverse order dims the one thing in the
// frame that is supposed to survive a dark grade, and it would read as "the
// lighting stopped working" rather than as an ordering mistake.
//
// **No blend mode is composed for this and none is needed.** SDL_BLENDMODE_MOD
// ships in 2.30.0 and is supported by both the accelerated and the software
// backends - the second matters, because the golden frame test rasterises in
// software and a custom blend mode is exactly the kind of thing it would have
// been blind to. `SDL_ComposeCustomBlendMode` stays unspent; the note in
// .claude/rules/assets-and-formats.md about every scheduled item having a route
// through the renderer as it stands survives this item intact.
//
// **The alpha channel is deliberately 255 and MOD ignores it.** MOD multiplies
// colour and leaves destination alpha alone, which is why this cannot be used to
// fade anything - a fade is V12's alpha work, not this.
void draw_grade(SDL_Renderer* renderer, const Params& p, const Grade&) {
    if (p.world_grade.identity()) return;   // the common case: not one draw call

    SDL_BlendMode prev = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prev);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MOD);
    SDL_SetRenderDrawColor(renderer, p.world_grade.r, p.world_grade.g,
                           p.world_grade.b, 255);
    SDL_RenderFillRect(renderer, nullptr);  // null rect is the whole target
    SDL_SetRenderDrawBlendMode(renderer, prev);
}

// V7's one extra RenderCopy - the whole cost of the feature on the GPU
// side, which is what the architecture note budgeted.
//
// **Drawn after the world and after the player, and before the reticle
// and HUD.** Everything in the world is a surface that light lands on,
// including the player, who otherwise stays flatly, evenly lit while
// standing inside a fire. Everything after it is UI, which is not in the
// world and must not be tinted by it - a reticle that goes orange near a
// flame is the exact defect B1 was about. That ordering is now declared as
// well as observed: this is the table's one `Lighting::Light` entry, and the
// static_asserts below hold every `Lit` layer in front of it.
//
// The destination is the *block* extent, not the padded cell extent, and
// that is what aligns the stretch. `cols()*BLOCK` is the padded width
// rounded up to a whole block, so each texel's centre lands on the centre
// of the four-by-four cells it was computed from. Sized to the padded
// extent instead, every texel would sit up to half a block off and the
// glow would trail behind the flame that cast it.
void draw_light(SDL_Renderer* renderer, const Params& p, const Grade& g) {
    if (!p.light || !p.light->any_light()) return;
    const Camera& camera = *p.camera;
    apply_grade(p.light_texture, g);
    const SDL_FRect light_dst{
        -camera.frac_x() * Camera::SCALE,
        -camera.frac_y() * Camera::SCALE,
        static_cast<float>(p.light->cols() * LightField::BLOCK * Camera::SCALE),
        static_cast<float>(p.light->rows() * LightField::BLOCK * Camera::SCALE)
    };
    SDL_RenderCopyF(renderer, p.light_texture, nullptr, &light_dst);
}

// --- the ordered list -----------------------------------------------------
//
// **This is V11's first bullet, and the order in this table is the feature.**
// Reading it top to bottom is reading the frame back to front. A band is a row
// here; before V11 it was an insertion between two comments in a 175-line
// function in main.cpp, where the only record of what came before what was the
// order the lines happened to be in.
//
// **The claim was "adding a band is one row rather than surgery", and the first
// thing that actually happened was a band being *removed* in one row** - the
// mid-ground layer went in and came back out the same day, on the played-frame
// check above, touching this table, one draw function and one factor. That is a
// better demonstration of what this item bought than adding one would have been,
// because the expensive direction was always going to be changing your mind.
constexpr Grade PLAIN{};   // 255,255,255 - drawn as authored

// **The mountains are multiplied down to 60% and that is step 3's one measured
// change to the frame.** notes/reference_observations.txt entry 2 said the depth
// bands do not separate by value; measured in luminance on 2026-08-16 the claim
// is worse than the entry's channel ranges made it look. The sky averages 26 and
// the mountains are *flat 28* - p05 and p95 both 28, no internal variation at
// all - so the two most distant bands in the frame are separated by two levels
// out of 255, and the far one is the *brighter* of the pair. At 0.60 the pair
// becomes 26 against 16 and the mountains read as a silhouette against the sky,
// which is the arrangement the reference gets its depth from.
//
// **Darker with nearness, not lighter, because the sky is the only light in the
// frame.** Daylight aerial perspective washes distant things *toward* the sky,
// and that instinct is the wrong one here: at night the sky is the bright thing
// and everything in front of it is a cut-out.
//
// **A number here and not darker mountain art, and the difference is worth
// stating because today the two are numerically identical.** Regenerating the
// BMP darker would produce the same pixels this frame. It stops being the same
// the moment `world_grade` is non-identity - baked art cannot respond to a night
// grade, a per-layer multiply composes with it. It is also 20 MB of asset
// regenerated for what is a composition decision, and V11's whole argument is
// that a direction change should be a number in a table.
//
// TUNING.md carries this row. The other layers are `PLAIN` on purpose rather
// than for want of trying: the sky is the reference the rest is judged against,
// and the world's own spread (Oil 38 to Sand 171) is already the widest in the
// frame - grading it would compress the one band that does not need help.
constexpr Layer TABLE[] = {
    {"clear",     Lighting::Lit,   PLAIN,           draw_clear},
    {"sky",       Lighting::Lit,   PLAIN,           draw_sky},
    {"mountains", Lighting::Lit,   {153, 153, 153}, draw_mountains},
    {"ground",    Lighting::Lit,   {135, 135, 135}, draw_ground},
    {"props",     Lighting::Lit,   PLAIN,           draw_props},
    {"cells",     Lighting::Lit,   PLAIN,           draw_cells},
    {"objective", Lighting::Lit,   PLAIN,           draw_objective},
    {"player",    Lighting::Lit,   PLAIN,           draw_player},
    {"grade",     Lighting::Grade, PLAIN,           draw_grade},
    {"light",     Lighting::Light, PLAIN,           draw_light},
    // Nothing `Unlit` yet, and that is not an omission: the UI drawn after the
    // light pass lives in main.cpp and stays there. The value exists so that
    // the first thing to cross the boundary declares which side it is on
    // instead of inheriting a position.
    //
    // **`grade` is a layer with no caller** - nothing sets `Params::world_grade`,
    // so it returns before its first draw call on every frame the game currently
    // composes. That is a state this project is right to distrust, so the terms
    // are written down: it ships because the *per-layer* half of the same
    // mechanism is live on the mountains row, which is what proves the multiply
    // works at all, and because the pass and its ordering argument are the part
    // that is expensive to add later. **It becomes a defect the day it is still
    // unset and the ordering claim above has stopped being checked by anything.**
    // The trigger for spending it is V8's time-of-day or a second biome; if
    // neither arrives, delete the row rather than leave it as decoration.
};
constexpr int TABLE_COUNT = static_cast<int>(sizeof(TABLE) / sizeof(TABLE[0]));

// --- the invariant, held by the compiler ----------------------------------
//
// `Lighting` is a claim about where a layer sits relative to the light pass,
// and a claim the code does not enforce is the failure mode this project's
// first rule is about. So it is enforced, at compile time, in the file that
// would have to be edited to break it - the same guard CMakeLists.txt's source
// sets give the simulation/rendering boundary.

// How many entries carry a given value.
constexpr int count_of(Lighting want) {
    int n = 0;
    for (int i = 0; i < TABLE_COUNT; ++i) if (TABLE[i].lighting == want) ++n;
    return n;
}

static_assert(count_of(Lighting::Light) == 1,
              "the layer table needs exactly one Lighting::Light entry - it is the "
              "boundary the other values are defined against, so zero of them makes "
              "Lit and Unlit meaningless and two of them makes them ambiguous");

// **At most one, not exactly one.** Two grade quads would multiply into a third
// grade that nothing declares; zero is legal, because a build with no world-wide
// grade is a coherent build - it is the one this project shipped up to step 3.
static_assert(count_of(Lighting::Grade) <= 1,
              "at most one Lighting::Grade entry - two full-screen multiplies compose "
              "into a third grade that nothing declares, and the second would be tuned "
              "against the first without either row saying so");

// **This was a boundary index at step 2 and is a rank at step 3, and the change
// is the point.** With one boundary the check was "before it or after it"; with a
// grade pass there are three positions in a fixed order, and the invariant is
// that the table never goes backwards through them. Written as a rank rather than
// as pairwise comparisons so the next value inserted into `Lighting` is one line
// here instead of a case analysis.
constexpr int rank(Lighting l) {
    switch (l) {
        case Lighting::Lit:   return 0;
        case Lighting::Grade: return 1;
        case Lighting::Light: return 2;
        case Lighting::Unlit: return 3;
    }
    return -1;
}

constexpr bool lighting_matches_order() {
    for (int i = 1; i < TABLE_COUNT; ++i) {
        if (rank(TABLE[i].lighting) < rank(TABLE[i - 1].lighting)) return false;
    }
    return true;
}
static_assert(lighting_matches_order(),
              "a layer's declared Lighting disagrees with where it sits in the table. "
              "The order is Lit, then Grade, then Light, then Unlit, and it is not a "
              "convention: the grade multiplies and the light pass adds, so putting the "
              "grade after the light is the difference between a fire that survives "
              "nightfall and one that gets dimmed by it");

// The two mechanisms are separate fields and this is the guard on that. A grade
// quad carrying a per-layer grade of its own would multiply twice - once as a
// property of the layer and once as its whole purpose - and the second one is
// invisible in the table, because it looks exactly like every other row.
constexpr bool grade_layer_is_plain() {
    for (int i = 0; i < TABLE_COUNT; ++i) {
        if (TABLE[i].lighting == Lighting::Grade && !TABLE[i].grade.identity())
            return false;
    }
    return true;
}
static_assert(grade_layer_is_plain(),
              "the Lighting::Grade row must carry an identity per-layer grade - it "
              "reads Params::world_grade and applies that. A grade on the grade row "
              "multiplies twice and neither number says so");

} // namespace

const Layer* const LAYERS = TABLE;
const int LAYER_COUNT = TABLE_COUNT;

void compose(SDL_Renderer* renderer, const Params& p) {
    for (int i = 0; i < TABLE_COUNT; ++i) TABLE[i].draw(renderer, p, TABLE[i].grade);
}

} // namespace frame
