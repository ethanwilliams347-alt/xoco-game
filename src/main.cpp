#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <string>
#include <vector>
#include "game/boot.h"
#include "game/camera.h"
#include "game/debug_view.h"
#include "game/display.h"
#include "game/input_log.h"
#include "game/pacer.h"
#include "game/run.h"
#include "game/settings_menu.h"
#include "render/backdrop_layers.h"
#include "render/backdrop_wrap.h"
#include "render/frame.h"
#include "render/light.h"
#include "render/overlay.h"
#include "render/player_anim.h"
#include "render/surface_plane.h"
#include "scene/bmp.h"
#include "scene/props.h"
#include "scene/scene.h"
#include "scene/sprites.h"
#include "ui/text.h"
#include "ui/hotbar.h"

// **The world's size, S0's objective column and the terrain scans that plant
// things on the ground all moved to `game/boot.h` (W5, 2026-08-17)**, which is
// SDL-free and has a suite. What is left in this file is the SDL half: a
// window, a renderer, textures, the pump and the draw. Aliased here because
// eighteen lines below read better as `GRID_WIDTH` than as a qualified name,
// and because these two are what every camera and upload call is stated
// against.
constexpr int GRID_WIDTH = boot::GRID_WIDTH;
constexpr int GRID_HEIGHT = boot::GRID_HEIGHT;

// SDL_RenderCopy below stretches the whole viewport-sized texture across the
// whole window (two null rects). **This used to warn that a grid not matching
// the window's proportions renders squashed or cropped, and that stopped being
// true when F3.3 sized the texture to the viewport rather than to the grid**:
// the texture is the padded viewport and the window is exactly Camera::SCALE
// times that, so the blit is 1:1 whatever size the world is - and, now, at
// whatever size the window is. The warning outlived the problem and read as a
// known defect in code that is correct. Camera (F3.2) owns every
// screen-to-world and world-to-screen conversion, and (F3.4) the viewport's
// position in the world, so mouse/render coordinates are correct at any grid
// size and any camera offset. The texture is sized to the viewport rather
// than the whole grid (F3.3), so upload cost does not scale with world size,
// and the viewport now follows the player and clamps at the world's edges
// (F3.4) rather than staying pinned at the origin - the two together are what
// turn "the whole world, squashed" into a real view of part of a larger one.
// The stall clamp moved to `game/pacer.h` with the rest of the pacer (W5).

// **The scene loader moved to `src/scene/bmp.cpp` (P4) and this is all that is
// left of it here.** It used to be 90 lines of `SDL_LoadBMP` and surface
// conversion in this file, which meant nothing headless could stamp the world
// the game actually plays in - and P4's replayed benchmark row has to, or it
// measures a session replayed into a world it was not recorded in. The
// unmatched-colour reporting went with it unchanged; see scene/bmp.h for why
// there is one reader rather than a headless one beside this one.
Scene load_scene_from_bmp(const char* material_path, const char* albedo_path) {
    std::string error, warning;
    Scene scene = bmp::load(material_path, albedo_path, &error, &warning);
    if (!error.empty()) std::fprintf(stderr, "Failed to load scene: %s\n", error.c_str());
    if (!warning.empty()) std::fprintf(stderr, "WARNING: %s\n", warning.c_str());
    return scene;
}


// Loads a plain (non-scene) authored BMP as a texture - the backdrop and
// prop art V8/V4 add on top of F4's scene loader above. `colorkey` marks
// pixel_art.COLOR_KEY (magenta, 0xFF00FF - see tools/pixel_art.py) as
// transparent before the surface becomes a texture, which is how a sprite
// with an irregular silhouette gets transparency out of a 24-bit BMP with
// no alpha channel at all: SDL_CreateTextureFromSurface bakes a colour-keyed
// surface's key into the resulting texture's alpha, so nothing downstream
// has to know the trick happened. Backdrop layers (opaque, full-rect) pass
// colorkey=false and get a plain opaque texture.
SDL_Texture* load_art_texture(SDL_Renderer* renderer, const char* path, bool colorkey) {
    SDL_Surface* surf = SDL_LoadBMP(path);
    if (!surf) {
        std::fprintf(stderr, "Failed to load %s: %s\n", path, SDL_GetError());
        return nullptr;
    }
    if (colorkey) {
        const Uint32 key = SDL_MapRGB(surf->format, 0xFF, 0x00, 0xFF);
        SDL_SetColorKey(surf, SDL_TRUE, key);
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!tex) {
        std::fprintf(stderr, "Failed to create texture from %s: %s\n", path, SDL_GetError());
        return nullptr;
    }
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}

// V4's props layer, defined in render/frame.h with the layer ordering it is
// part of. Aliased rather than qualified everywhere below: the planting scan
// further down is scene setup, not rendering, and reads no better for being
// told twice which namespace a tree is in.
using frame::Prop;

// Everything whose size is a function of the display mode, and nothing else.
//
// The list is short on purpose, and the reason it is short is F3.2-F3.4: the
// pixel buffer is grid-sized rather than viewport-sized, and Camera is told the
// viewport's size as an argument to follow() rather than storing it. Neither
// had anything to do with resolution switching when they were written; both are
// why switching is a matter of rebuilding two textures instead of rebuilding
// the renderer.
struct RenderTargets {
    SDL_Texture* cells = nullptr;      // the world's ARGB streaming texture
    SDL_Texture* light_texture = nullptr;
    LightField light{1, 1};            // replaced wholesale on every mode change
};

// Builds the render targets for `mode` and, on success, swaps them in and
// resizes the window.
//
// **Constructs everything new before releasing anything old, and that is the
// whole reason this returns a bool instead of exiting.** At startup a failed
// texture allocation can reasonably end the process; a hundred frames into a
// session it cannot, because the player still has a world open and the only
// thing that went wrong is a setting they can change back. On failure nothing
// has been destroyed and nothing has been reassigned, so the caller keeps
// playing at the mode it already had.
bool apply_mode(SDL_Window* window, SDL_Renderer* renderer,
                const DisplayMode& mode, RenderTargets& targets) {
    SDL_Texture* cells = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        mode.padded_w(), mode.padded_h());
    if (!cells) {
        std::fprintf(stderr, "Could not create a %dx%d cell texture: %s\n",
                     mode.padded_w(), mode.padded_h(), SDL_GetError());
        return false;
    }

    LightField light(mode.padded_w(), mode.padded_h());
    SDL_Texture* light_texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        light.cols(), light.rows());
    if (!light_texture) {
        std::fprintf(stderr, "Could not create a %dx%d light texture: %s\n",
                     light.cols(), light.rows(), SDL_GetError());
        SDL_DestroyTexture(cells);
        return false;
    }

    // **Both blend modes and the light's filtering are re-applied here, not set
    // once at startup, because they are properties of a texture and these are
    // new textures.** Each of the three is load-bearing and none is obvious
    // from the create call:
    //
    // **BLEND on the cells.** Empty is 0x00000000 in MATERIALS, so an empty
    // cell is transparent rather than black - but only if the texture is
    // composited rather than blitted. Without this line the alpha is carried
    // all the way to the screen and then ignored, which looks exactly like the
    // opaque black the table used to hold.
    //
    // **ADD, not BLEND, on the light.** Light is something the scene gains, not
    // something laid over it: an alpha blend towards orange would wash the
    // terrain's colour out towards the flame's, where addition brightens
    // whatever is already there and leaves a lit grey wall reading as a grey
    // wall. It also means black is free - an unlit block adds nothing - which
    // is what lets the same texture cover the whole viewport rather than
    // needing a mask.
    //
    // **Linear filtering is the entire reason a downsampled light grid is
    // acceptable.** At nearest-neighbour it is BLOCK-sized squares of flat
    // colour, which is worse than no lighting at all. Set per-texture rather
    // than through SDL_HINT_RENDER_SCALE_QUALITY, which is read at *creation*
    // time and is global - routing this through a global would make the cell
    // texture's sharpness depend on the order the two are created in, and the
    // day that bit rots every cell in the world goes soft at once.
    SDL_SetTextureBlendMode(cells, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(light_texture, SDL_BLENDMODE_ADD);
    SDL_SetTextureScaleMode(light_texture, SDL_ScaleModeLinear);

    // A streaming texture is created with undefined contents, and the upload
    // each frame only ever writes the rect the grid actually covers. Those are
    // the same rect in every world this project currently builds - but a world
    // smaller than the viewport on either axis leaves the remainder holding
    // whatever the driver's allocation happened to contain, and it would show
    // as garbage along the edge rather than as the backdrop. Once per texture,
    // since it can only ever be wrong once - which is why it lives here rather
    // than at startup now that textures are created more than once.
    {
        const std::vector<uint32_t> blank(
            static_cast<size_t>(mode.padded_w()) * mode.padded_h(), 0);
        SDL_UpdateTexture(cells, nullptr, blank.data(), mode.padded_w() * sizeof(uint32_t));
    }

    if (targets.cells) SDL_DestroyTexture(targets.cells);
    if (targets.light_texture) SDL_DestroyTexture(targets.light_texture);
    targets.cells = cells;
    targets.light_texture = light_texture;
    targets.light = std::move(light);

    SDL_SetWindowSize(window, mode.window_w, mode.window_h);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    return true;
}

// Whether a mode's window actually fits on the display the window is on.
//
// 3440x1440 on a 1080p monitor is a window larger than the desktop: SDL will
// create it, the bottom and right of the game are then off-screen, and the
// settings menu the player would use to change it back is one of the things
// that has gone off-screen with them. Usable bounds rather than raw bounds, so
// a taskbar counts.
bool mode_fits(const DisplayMode& mode, int display_index) {
    SDL_Rect usable;
    if (SDL_GetDisplayUsableBounds(display_index, &usable) != 0) {
        // No answer is not the same as "no". A driver that cannot report its
        // own bounds should not be able to hide every mode in the menu.
        return true;
    }
    return mode.window_w <= usable.w && mode.window_h <= usable.h;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Which modes this display can actually show, and which of them to open at.
    // **Only the first of those two needs SDL, and W5 split them on that line:**
    // `mode_fits` asks the display, `choose_display_mode` decides, and the
    // deciding is the half with the wrong answers in it - a stored mode that no
    // longer fits, and a desktop smaller than every mode in the table. Both are
    // now in `display_test` rather than in a checklist step nobody can run
    // without two monitors.
    bool mode_available[DISPLAY_MODE_COUNT];
    for (int i = 0; i < DISPLAY_MODE_COUNT; ++i)
        mode_available[i] = mode_fits(DISPLAY_MODES[i], 0);

    const int stored_mode = load_display_mode();
    const ModeChoice choice =
        choose_display_mode(mode_available, DISPLAY_MODE_COUNT, stored_mode);
    int mode_index = choice.index;
    switch (choice.why) {
        case ModeChoice::Why::NothingFits:
            std::fprintf(stderr, "No display mode fits this desktop; opening at %dx%d anyway.\n",
                         DISPLAY_MODES[mode_index].window_w, DISPLAY_MODES[mode_index].window_h);
            break;
        case ModeChoice::Why::StoredTooBig:
            std::fprintf(stderr, "Stored mode %dx%d does not fit this display; ignoring it.\n",
                         DISPLAY_MODES[stored_mode].window_w, DISPLAY_MODES[stored_mode].window_h);
            break;
        case ModeChoice::Why::Stored:
        case ModeChoice::Why::Largest:
            break;
    }
    DisplayMode mode = DISPLAY_MODES[mode_index];

    // Printed for the same reason the seed below is: a mode chosen by a
    // fallback path is invisible otherwise, and "it opened smaller than I asked
    // for" is not something a player can debug from the window alone.
    std::printf("Display: %dx%d (%dx%d cells at %dx)\n", mode.window_w, mode.window_h,
                mode.viewport_w(), mode.viewport_h(), Camera::SCALE);

    SDL_Window* window = SDL_CreateWindow(
        "SLOP Pixel Physics",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        mode.window_w, mode.window_h,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // The world's ARGB8888 streaming texture and V7's light texture, both sized
    // to the viewport (F3.3) rather than to the whole grid, and both rebuilt by
    // this same call whenever the mode changes. Here a failure is fatal, unlike
    // in the menu: there is no earlier mode to fall back to.
    RenderTargets targets;
    if (!apply_mode(window, renderer, mode, targets)) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // V8's backdrop: two static parallax layers, replacing V1's 64-band
    // gradient placeholder now that there is authored art to show instead.
    // Generated by tools/generate_backdrop.py, which sizes each layer to
    // cover the camera's full pan range at its own factor - see that
    // script's header for why the two files have to change together.
    // Layer order and the "trees draw before terrain" rule are notes/
    // art_direction.txt's four-layer model.
    // Which BMP each of those keys actually resolves to is data now - see
    // src/scene/sprites.h. The literals below are the fallback, so this file
    // still says what ships; the manifest is how a drawing dropped into assets/
    // gets in front of them without a code change. A malformed manifest is
    // reported and then ignored wholesale rather than half-applied, because a
    // half-applied rebinding table is a scene where some art moved and some did
    // not, which is far harder to read than the art you already had.
    std::string sprite_manifest_error;
    SpriteManifest sprites = load_sprite_manifest("assets/sprites.txt", &sprite_manifest_error);
    if (!sprite_manifest_error.empty()) {
        std::fprintf(stderr, "WARNING: %s\n", sprite_manifest_error.c_str());
        std::fprintf(stderr, "         every sprite falls back to its shipped file.\n");
    }

    // The parallax factors are in the generated render/backdrop_layers.h as of
    // V11, written by `python tools/generate_backdrop.py --header` from the same
    // table that sizes these images. What is left here is the loading, which is
    // startup rather than composition.
    frame::Backdrop backdrop;
    backdrop.sky = load_art_texture(
        renderer, sprites.path_for("backdrop_sky", "backdrop_sky.bmp").c_str(), false);
    backdrop.mountains = load_art_texture(
        renderer, sprites.path_for("backdrop_mountains", "backdrop_mountains.bmp").c_str(), true);
    // **V19's ground plane, and it loads with no colour key** - it is an opaque
    // surface rather than a silhouette, so there is nothing in it to key out and
    // a key would punch holes in the plane wherever the ramp happened to land on
    // the key colour.
    backdrop.ground = load_art_texture(
        renderer, sprites.path_for("backdrop_ground", "backdrop_ground.bmp").c_str(), false);

    if (backdrop.sky)
        SDL_QueryTexture(backdrop.sky, nullptr, nullptr, &backdrop.sky_w, &backdrop.sky_h);
    if (backdrop.mountains)
        SDL_QueryTexture(backdrop.mountains, nullptr, nullptr,
                         &backdrop.mountain_w, &backdrop.mountain_h);
    if (backdrop.ground)
        SDL_QueryTexture(backdrop.ground, nullptr, nullptr,
                         &backdrop.ground_w, &backdrop.ground_h);

    // **The seam at the pan limit, turned into a printed line.** A backdrop
    // layer has to be large enough to cover the window plus the camera's whole
    // pan range at that layer's factor; if it is not, the layer runs out of
    // image before the world runs out of ground and an edge of raw clear colour
    // appears - at the far edge of the map, which is the last place anybody
    // looks. That was previously guarded by a comment in two files asking a
    // human to keep four numbers in step (ASSETS.md's "Change one side and you
    // must change the other"). The header now carries the size the generator
    // would produce, so the disagreement is checkable, and this says so at
    // startup rather than at the map's edge.
    //
    // A warning and not a failure: an undersized backdrop is a cosmetic defect
    // at one extreme of the world, and refusing to launch over it would be
    // worse than the seam. It joins the seed and scene counts as a launch
    // check - a line here means the art and the header disagree, which is
    // either a stale BMP (rerun the generator) or a hand-edited header.
    auto check_layer_size = [](const char* name, SDL_Texture* tex, int w, int h,
                               const backdrop_layers::Layer& expected) {
        if (!tex) return;
        if (w < expected.width || h < expected.height) {
            std::fprintf(stderr,
                         "WARNING: backdrop %s is %dx%d but parallax %.2f/%.2f needs at "
                         "least %dx%d - expect a seam at the pan limit.\n"
                         "         rerun: python tools/generate_backdrop.py\n",
                         name, w, h, expected.parallax_x, expected.parallax_y,
                         expected.width, expected.height);
        }
    };
    check_layer_size("sky", backdrop.sky, backdrop.sky_w, backdrop.sky_h,
                     backdrop_layers::SKY);
    check_layer_size("mountains", backdrop.mountains, backdrop.mountain_w,
                     backdrop.mountain_h, backdrop_layers::MOUNTAINS);
    // **The ground plane is checked by the same lambda and the number means
    // something different for it**, which is worth a line because the warning
    // text talks about a seam at the pan limit and a wrapping layer cannot have
    // one. For a tile, undersized means the texture repeats sooner than the art
    // was drawn for, and the vertical half is sharper still: the tile's rows are
    // the plane's depth, so a short tile is a plane missing part of its
    // recession. Same direction, same fix, different symptom.
    check_layer_size("ground", backdrop.ground, backdrop.ground_w,
                     backdrop.ground_h, backdrop_layers::GROUND);

    // **V25: the same tile again, on the CPU, reduced to one colour per row.**
    // `render/surface_plane.cpp` blends the near terrain toward the plane's own
    // value at its own depth, and it cannot read an `SDL_Texture` - so the BMP is
    // read a second time, through `bmp::read`, which is the reader main.cpp
    // already uses for the scene.
    //
    // **Read again rather than kept from the texture load, and that is a
    // deliberate small waste.** `load_art_texture` returns a texture and throws
    // the pixels away; threading a copy out of it would put a second output
    // parameter on a function four other layers call and do not want it. This is
    // one 0.2 MB file, once, at startup.
    //
    // A failure here is not fatal: `ground_rows` stays empty, `TileRows::count`
    // is 0, and the pass copies the window through unchanged - which is exactly
    // the frame the game composed before V25 existed. The warning is printed
    // because a silently-disabled depth pass is the kind of thing that gets
    // noticed six sessions later.
    std::vector<uint8_t> ground_rows;
    {
        bmp::Image ground_img;
        std::string err;
        const std::string path = sprites.path_for("backdrop_ground", "backdrop_ground.bmp");
        if (bmp::read(path.c_str(), ground_img, &err)) {
            surface_plane::average_rows(ground_img.pixels.data(), ground_img.width,
                                        ground_img.height, ground_rows);
        } else {
            std::printf("WARNING: ground plane rows unavailable (%s) - "
                        "the near terrain will not take the plane's value\n", err.c_str());
        }
    }

    // The `ground` row's grade, read out of frame.cpp's table rather than
    // written here a second time. **A constant copied into two files with a
    // comment asking a human to keep them in step is this project's most
    // repeated defect** - it is what V11 removed from the parallax factors and
    // what `.claude/rules/assets-and-formats.md` records as the shape to
    // recognise. The blend has to match the plane *as composited*, so it needs
    // this number; the table stays the one place it is decided.
    frame::Grade ground_grade{};
    for (int i = 0; i < frame::LAYER_COUNT; ++i) {
        if (std::string(frame::LAYERS[i].name) == "ground") {
            ground_grade = frame::LAYERS[i].grade;
            break;
        }
    }

    // Scratch for V25's pass. **Resized at the upload site rather than sized
    // here**, because `mode` is not fixed for the run - the settings menu
    // switches between 1920x1080, 2560x1440 and 3440x1440 and `apply_mode`
    // rebuilds every target when it does. A buffer sized once at boot is the
    // shape of defect that survives every test and fails on the one machine
    // whose owner opens the menu.
    std::vector<uint32_t> cell_window;
    std::vector<int> plane_src_row_for;
    std::vector<int> plane_row_scale;
    std::vector<int> cell_depth;

    // V4's props: sprites from tools/generate_props.py, positioned by
    // assets/test_props.txt rather than by a list in this file. **That
    // indirection is the whole of V4's second half** - the nine trees below
    // used to be a hardcoded `std::vector<Prop>` here, positioned by eye
    // against the F4 fixture's known geometry, which meant a second scene had
    // no way to have props at all. See src/scene/props.h for why the format is
    // a text list and not a second BMP.
    //
    // One texture per distinct sprite name, not one per record: nine trees are
    // three images, and the cache is what keeps that true as a scene grows.
    // Keyed by name so `prop_textures` is also the destroy list at shutdown.
    std::vector<std::pair<std::string, SDL_Texture*>> prop_textures;
    auto prop_texture = [&](const std::string& sprite) -> SDL_Texture* {
        for (auto& entry : prop_textures)
            if (entry.first == sprite) return entry.second;
        // A prop's name is already its own key, so the manifest can rebind one
        // without the scene file changing: the record still says `tree_a`, and
        // which drawing that is is now a swap you can make from the command
        // line.
        const std::string path = sprites.path_for(sprite, sprite + ".bmp");
        SDL_Texture* tex = load_art_texture(renderer, path.c_str(), true);
        if (!tex) {
            std::fprintf(stderr, "WARNING: prop sprite '%s' did not load; "
                                 "every prop naming '%s' is skipped.\n",
                         path.c_str(), sprite.c_str());
        }
        prop_textures.emplace_back(sprite, tex);
        return tex;
    };

    // V3.1's player sheet, from tools/player_sheet.py.
    // Colour-keyed like the props, and like them one BMP pixel is one world
    // cell. Every number about the sheet's layout comes from the generated
    // header rather than being retyped here - see player_sprite.h for why.
    //
    // **The sprite is deliberately larger than the collision box and the
    // generated offsets are the whole of that decoupling.** Player::WIDTH/HEIGHT
    // stay 8x20 - eighteen tests and every movement constant are tuned to them -
    // and the 14x26 frame is drawn anchored to the box's bottom-centre, so the
    // masked head overhangs upward into space that collides with nothing (the
    // allowance player.h reserves by name) and the sleeves hang outside the
    // box's width. A sleeve visually overlapping a wall is correct: it is art,
    // not body.
    //
    // **The aiming arm is not drawn, and that is why nothing here reads a
    // hotspot.** Decomposing the limb out of the sheet is the finding V3.1 was
    // built around - Noita's wizard gets most of its expressiveness from a
    // freely rotating wand arm over a short body loop - but the arm is pulled
    // for now and the sheet stands on its own. What it costs to bring back is
    // the hotspot image and the rotate-about-the-shoulder draw, both of which
    // are described in ROADMAP.md's V3.1 entry rather than left as dead code
    // here.
    //
    // **The path is a manifest lookup and the layout is not.** Which BMP this
    // is can be changed from the command line (tools/load_sprite.py); how many
    // frames it holds and what they mean cannot, because that is the animation
    // table in tools/player_sheet.py and the header it generates. So the one
    // thing worth checking here is that the image actually fits the layout the
    // code is compiled against - a sheet with the wrong frame size loads fine
    // and draws fine, it just draws the wrong rectangles, and the symptom is
    // sliced-looking art rather than anything that mentions a size.
    const std::string player_sheet_path =
        sprites.path_for("player_sheet", "player_sheet_fly.bmp");
    SDL_Texture* player_tex = load_art_texture(renderer, player_sheet_path.c_str(), true);
    if (player_tex) {
        int sheet_w = 0, sheet_h = 0;
        SDL_QueryTexture(player_tex, nullptr, nullptr, &sheet_w, &sheet_h);
        const int need_w = player_sprite::SHEET_COLS * player_sprite::FRAME_W;
        const int need_h = player_sprite::SHEET_ROWS * player_sprite::FRAME_H;
        if (sheet_w != need_w || sheet_h != need_h) {
            std::fprintf(stderr,
                         "WARNING: %s is %dx%d, but the animation table expects %dx%d "
                         "(%d cols x %d rows of %dx%d).\n",
                         player_sheet_path.c_str(), sheet_w, sheet_h, need_w, need_h,
                         player_sprite::SHEET_COLS, player_sprite::SHEET_ROWS,
                         player_sprite::FRAME_W, player_sprite::FRAME_H);
            std::fprintf(stderr,
                         "         The figure will draw sliced. Either the sheet is the "
                         "wrong art, or ANIMATIONS in tools/player_sheet.py needs "
                         "updating and --header re-running.\n");
        }
    }
    player_anim::State anim_state;

    // Which way the figure faces. Tracked here rather than on Player because
    // Player is simulation and this is presentation - F3.5's rule that
    // rendering does not feed the simulation runs in this direction too, and a
    // facing flag on the body would be state the determinism tests would then
    // have to account for. Sampled off the same key state the input struct is
    // built from, and *sticky*: it holds the last direction actually pressed,
    // so a player standing still keeps facing where they were going rather than
    // snapping to a default the moment the key comes up.
    bool facing_left = false;

    // Anchors are x-only. There is no authored y at all now - the file does not
    // carry one and the terrain scan below is the only thing that sets it. That
    // is not a shortcut: writing a y is what put three of these trees inside
    // the snowbank, because FLOOR_TOP is the floor slab's top and the authored
    // sand slope rises up to 150 cells above it, so "the ground" is not one
    // number and never was. `anchor_y` sits at 0 until the scan runs.
    //
    // **A malformed prop list is loud and costs every prop, not the bad line.**
    // The alternative - skip the row that would not parse - is the same silent
    // failure as V2's blank world and the first props pass's buried trees: a
    // scene that renders, renders wrong, and says nothing. props.h has the
    // argument in full.
    std::string prop_error;
    const std::vector<PropDef> prop_defs = load_prop_list("assets/test_props.txt", &prop_error);
    if (!prop_error.empty()) {
        std::fprintf(stderr, "ERROR: %s\n", prop_error.c_str());
        std::fprintf(stderr, "       No props are drawn. Fix the line above and re-run.\n");
    }

    // One entry per record, parallel to `prop_defs`, holding what only a window
    // can answer: the sprite's texture and its native size in world cells. A
    // record whose sprite did not load keeps its slot with a null texture and a
    // width of 0, which is how `boot::plant_props` - which knows no SDL - is
    // told about it without being handed one.
    std::vector<SDL_Texture*> prop_tex(prop_defs.size(), nullptr);
    std::vector<int> prop_w(prop_defs.size(), 0);
    std::vector<int> prop_h(prop_defs.size(), 0);
    for (size_t i = 0; i < prop_defs.size(); ++i) {
        SDL_Texture* tex = prop_texture(prop_defs[i].sprite);
        if (!tex) continue; // already warned by prop_texture, once per name
        prop_tex[i] = tex;
        SDL_QueryTexture(tex, nullptr, nullptr, &prop_w[i], &prop_h[i]);
    }
    // The count is NOT printed here. A prop that has a texture is not yet a prop
    // that got placed - the terrain scan below drops any with no ground under
    // them - so a count taken at this point reports texture loads while saying
    // "placed". It read `9 of 9` on a run that drew 8. See below.
    std::vector<Prop> props;

    // The reticle *is* the cursor now, so the OS one would be a second pointer
    // sitting on top of it. SDL scopes this to its own window rather than
    // globally, so the desktop pointer is untouched the moment the mouse leaves
    // - which is why this can be set once here instead of tracked per frame.
    SDL_ShowCursor(SDL_DISABLE);

    // This is the only nondeterministic line in the project, and it is here
    // rather than inside Grid on purpose: the simulation is a pure function of
    // its seed, and exactly one place gets to choose that seed. Two draws
    // because std::random_device yields 32 bits at a time.
    //
    // Printed because a seed you cannot read is only half of determinism - this
    // is the number that turns "it collapsed weirdly" into something that can be
    // reproduced. It will belong in the title bar or a debug overlay once there
    // is a UI to put it in; stdout will do until then.
    std::random_device rd;
    const uint64_t world_seed = (static_cast<uint64_t>(rd()) << 32) | rd();
    std::printf("World seed: %llu\n", static_cast<unsigned long long>(world_seed));

    Run run(GRID_WIDTH, GRID_HEIGHT, world_seed); // starts fully Empty, player mid-air
                                                  // - boot::stand_player_on_ground below
                                                  // is what puts it down, once there is
                                                  // terrain to put it on
    
    // Load F4 test scene. A scene that resolves to no cells at all is reported
    // rather than shrugged off: README's launch check is "terrain is visible
    // immediately", and a blank world is exactly what a broken legend, a
    // missing file and an empty file all look like from here.
    Scene scene = load_scene_from_bmp("assets/test_material.bmp", "assets/test_albedo.bmp");
    int scene_cells = 0;
    if (scene.width > 0) {
        scene_cells = load_scene(run.grid, scene, 0, 0);
        std::printf("Scene: %dx%d, %d cells placed\n", scene.width, scene.height, scene_cells);
        if (scene_cells == 0) {
            std::fprintf(stderr, "WARNING: the scene named no material anywhere - the world is empty.\n");
        }
    }

    // The body starts mid-air because `Run` is built before the scene is; now
    // that there is terrain, stand it on the terrain. Playtest session 12
    // reported the fall this removes. The scan is in `game/boot.h` so a suite
    // can reach it; what is left here is the warning, for the same reason the
    // objective's is here - a body that could not find ground is something the
    // player is about to experience and cannot otherwise account for.
    if (!boot::stand_player_on_ground(run).placed) {
        std::fprintf(stderr, "WARNING: no ground under the spawn column; "
                             "the player starts in mid-air and will fall.\n");
    }

    // S0's objective, planted on whatever terrain is actually at
    // `boot::OBJECTIVE_X` the same way a prop is. The decision moved to
    // `game/boot.h` with W5; what is left here is the warning, because a run
    // that cannot be won is a thing the *player* has to be told about and
    // stderr is where this shell says such things.
    auto place_objective = [&]() {
        if (!boot::place_objective(run).placed) {
            std::fprintf(stderr, "WARNING: no ground under the objective column x=%d; "
                                 "this run has no objective and cannot be won.\n",
                         boot::OBJECTIVE_X);
        }
    };
    place_objective();

    // Printed for the same reason the seed and the scene count are: an objective
    // that silently failed to place is a run that cannot be won, and that is not
    // something a player can tell apart from one they have not found yet.
    if (run.has_objective()) {
        std::printf("Objective: (%d, %d)\n", run.objective_x(), run.objective_y());
    }

    // --- P4: the session recorder ---
    //
    // **Recording is always on, and F9 saves what has been recorded so far.**
    // The alternative - F9 starts recording - was tried on paper and does not
    // work: a log has to begin at a world state the replay can rebuild, and the
    // only such state is the one right here, before the first step. A recording
    // started two minutes in would replay from the fixture scene into inputs
    // that assume two minutes of dug tunnels and poured water, which is the
    // "silently measures nothing" failure P4 exists to remove rather than
    // introduce.
    //
    // The cost of always-on is one 24-byte `Input` per fixed step - about 1.4 MB
    // per hour - and no work per step beyond the copy. The cap below is what
    // keeps that a fact rather than a hope; it stops recording rather than
    // dropping the oldest steps, because a log missing its middle is not a
    // session and there is no honest way to replay one.
    input_log::Log recording;
    recording.header.grid_w = GRID_WIDTH;
    recording.header.grid_h = GRID_HEIGHT;
    recording.header.seed = world_seed;
    recording.header.scene_cells = scene_cells;
    recording.header.start_fingerprint = input_log::fingerprint(run.grid);
    constexpr size_t MAX_RECORDED_STEPS = 60 * 60 * 30; // half an hour of play
    bool recording_full = false;

    // Shown under the HUD for a few seconds after F9, because a save that
    // reports only on stdout is a save a player in a fullscreen window cannot
    // see happen.
    std::string record_notice;
    double record_notice_timer = 0.0;
    int saved_logs = 0;

    // Plant each prop on the terrain that is actually under it, rather than on
    // a hardcoded ground line. **The scan moved to `boot::plant_props` with W5
    // and its argument went with it**; it runs here, after the scene is stamped
    // and before the first frame, because props are not simulated and terrain
    // that moves later must not drag them with it.
    //
    // What is left in this file is turning the report into draw records and
    // saying out loud what was dropped - the two things that need a texture and
    // a stderr respectively.
    const boot::PlantingReport planting =
        boot::plant_props(run.grid, prop_defs, prop_w);
    for (int i : planting.no_ground) {
        std::fprintf(stderr, "WARNING: assets/test_props.txt: prop at x=%.1f has no ground "
                             "under it and is not drawn.\n", prop_defs[i].x);
    }
    props.reserve(planting.planted.size());
    for (const boot::Planted& p : planting.planted) {
        props.push_back(Prop{ prop_tex[p.def_index], prop_w[p.def_index], prop_h[p.def_index],
                              prop_defs[p.def_index].x, static_cast<float>(p.anchor_y) });
    }

    // Printed *after* planting, because this is the line README's launch check
    // reads and it has to count props that will actually be drawn. It used to
    // print immediately after the textures loaded, 59 lines above and before
    // either way a prop can be dropped - so a run with an unplantable prop
    // reported `10 of 10 placed` on stdout while warning on stderr that one of
    // them was not drawn. The warning was right and the number contradicted it,
    // which is worse than no number: a check that asserts the wrong thing fails
    // silently, because it passes. Both drops are in the count now - a sprite
    // that would not load, and a prop with no ground under it.
    std::printf("Props: %d of %d placed\n", static_cast<int>(props.size()),
                static_cast<int>(prop_defs.size()));

    // Centred, and deliberately not configurable from here. V23 gave this a
    // moving vertical anchor and session 9 asked for the centring back; the
    // whole mechanism went with it, so there is nothing left to hold. See
    // ROADMAP.md's V23b entry before adding a second framing.
    Camera camera;

    bool running = true;
    SDL_Event e;

    ElementType current_brush = ElementType::Sand;
    int brush_size = 3;

    // The settings menu is a *state*, not an overlay with a flag: while it is
    // open the fixed-step loop below does not run, so the world is frozen
    // rather than continuing to simulate behind a screen the player cannot act
    // through. It still renders, because a settings screen over a black void
    // gives no way to judge a resolution change against the thing being
    // resized.
    //
    // **ESC now opens this instead of quitting.** That is a change to an
    // existing binding and worth stating: a key that ends the session without
    // confirmation is the wrong key to leave next to a menu, so quitting moved
    // into the menu as an item, where it takes two deliberate presses.
    enum class Screen { Playing, Settings };
    Screen screen = Screen::Playing;

    // **The navigation and selection moved to `game/settings_menu.h` (W5,
    // 2026-08-17)** and has a suite. What is left here is the keysyms and the
    // two calls that need a window. The cursor still lives in a local because
    // the drawing code below reads it.
    const int MENU_QUIT = menu::quit_index(DISPLAY_MODE_COUNT);
    menu::State menu_state;
    menu::open(menu_state, mode_index);
    int& menu_cursor = menu_state.cursor;

    // Shown under the menu for a few seconds after a switch is attempted, so a
    // refused mode says so instead of looking like a dead key.
    std::string menu_notice;
    double menu_notice_timer = 0.0;

    uint64_t prev_counter = SDL_GetPerformanceCounter();
    const double counter_freq = static_cast<double>(SDL_GetPerformanceFrequency());

    // The accumulator, the freeze rule and the interpolation alpha moved to
    // `game/pacer.h` (W5, 2026-08-17). Reading the clock stays here; deciding
    // what the elapsed seconds *mean* does not.
    pacer::Pacer frame_pacer;

    int frames_this_second = 0;
    double title_timer = 0.0;
    int fps_display = 0;

    // **Rebuilt every frame, and the once-a-second version this replaces was
    // defect A2** (PLAYTEST_LOG.md session 1). The old comment here argued that
    // a string this cheap was not worth reformatting 60x/sec, which was true
    // about the cost and wrong about the consequence: the brush name and the
    // awake-chunk count are not periodic readouts like the frame rate, they are
    // answers to "what did that key just do" and "has the world settled yet".
    // Cached for a second, pressing 2 left the HUD reading SAND for up to a
    // full second, and a playtester correctly read that as the *input* being
    // sluggish. Only the frame rate is genuinely a once-a-second quantity, so
    // only the frame rate is cached now.
    //
    // The lesson worth keeping: this is an instrument, and an instrument that
    // lags makes every measurement taken with it suspect. It was the first
    // thing fixed after the playtest for that reason.
    std::string hud_text;

    // Previous frame's sub-cell player position, for render interpolation. The
    // simulation steps at 60 Hz and the display was running at 165, so each
    // simulated position was being shown for about 2.75 frames and the motion
    // stepped visibly even once the sub-cell remainder was accounted for.
    // Updated inside the step loop so it always holds the state one step behind
    // the current one - including on frames where no step runs at all, which is
    // most of them at this refresh rate.
    float prev_player_x = run.player.visual_x();
    float prev_player_y = run.player.visual_y();

    // --- S0: starting the run over ---
    //
    // **One path, `Run::reset(seed)`, and not a second set of code that puts
    // things back.** A win and a loss both come here, and so would a debug
    // reset hotkey when T1 builds one.
    //
    // The scene has to be re-stamped because `Run::reset` wipes the grid - the
    // run does not own the level, `main.cpp` does - and the objective has to be
    // re-derived because the terrain it was scanned off has just been rebuilt.
    // Both are cheap and both happen while the world is frozen.
    //
    // **The recording starts over too, and that is what keeps P4's guarantee
    // intact.** A session log replays by rebuilding the world from the seed and
    // the scene and replaying the inputs into it; a log that spanned a reset
    // would replay into a world two minutes of play deep and the bench could not
    // tell that from a stale log. Resetting on the *same seed* and re-stamping
    // the *same scene* reproduces the world the recording started in exactly -
    // `run_test` asserts that against the fingerprint - so the new log is as
    // valid as the first, and what is lost is only the part of the session
    // before the restart. `saved_logs` is deliberately not reset, so a second
    // F9 still writes to a new file rather than over the first.
    auto restart_run = [&]() {
        run.reset(world_seed);
        if (scene.width > 0) load_scene(run.grid, scene, 0, 0);
        place_objective();

        recording.steps.clear();
        recording.header.start_fingerprint = input_log::fingerprint(run.grid);
        recording_full = false;

        // The body is somewhere else entirely now, so last step's drawn
        // position is not something to ease away from. The interpolation clamp
        // below would catch this on its own; setting them is the honest version
        // of relying on that.
        prev_player_x = run.player.visual_x();
        prev_player_y = run.player.visual_y();
        anim_state = player_anim::State{};
        frame_pacer.accumulator = 0.0;
    };

    // --- T1: the debug tooling ---------------------------------------------
    //
    // The state and every decision in it are in `game/debug_view.h`, which is
    // SDL-free and has a suite; what is left down here is the key bindings and
    // the drawing.
    DebugView debug;

    // Exactly one fixed step, and the only place one happens.
    //
    // **Extracted so that pause's single-step and the frame pacer run the same
    // code rather than two copies of it.** The alternative - a second copy under
    // the `.` key - is how the recorder would end up being fed by one path and
    // not the other, and a session log missing the steps taken while paused is
    // not a session: it replays into a world those steps had changed. This is
    // the same argument F2.2 made for `Run` itself, one level up.
    //
    // The accumulator is deliberately *not* touched here. Time is the pacer's
    // business, and a single-step is a step that no time was spent on.
    auto advance_one_step = [&](const Input& step_input) {
        prev_player_x = run.player.visual_x();
        prev_player_y = run.player.visual_y();

        // Captured here, inside the one place a step happens, and this placement
        // is the whole of what makes the log replayable. One record per *step*,
        // never per rendered frame: the same session played at 60 and at 165 fps
        // produces the same list of records, because there is no sampling left
        // in it. Recording up in the input-building block instead would store
        // one record per frame and put the frame rate back into the measurement
        // - which is F1 and F2.3 undone by the instrument built to check them.
        if (recording.steps.size() < MAX_RECORDED_STEPS) {
            recording.steps.push_back(step_input);
        } else if (!recording_full) {
            recording_full = true;
            std::fprintf(stderr, "Session recording stopped at %d steps (half an hour); "
                                 "F9 still writes what was recorded up to that point.\n",
                         static_cast<int>(MAX_RECORDED_STEPS));
        }

        run.step(step_input);

        // The animation clock. Advanced here, inside the one place a step
        // happens, and nowhere else - see the timing note at the top of
        // render/player_anim.h. Driving it from the render loop would make the
        // walk cycle's speed a function of frame rate, which is the class of bug
        // F1 and F2.3 spent two sections retiring and which would present as an
        // art problem.
        player_anim::Conditions cond;
        cond.on_ground = run.player.is_on_ground();
        // `!= 0` rather than an epsilon, which F5 made correct rather than
        // merely tidy: horizontal velocity is now exactly zero or exactly
        // +/-MOVE_SPEED, with no float noise for the epsilon to absorb.
        cond.moving = run.player.velocity_x() != 0;
        cond.vel_y = run.player.velocity_y();  // sign only; see Conditions
        // Read off the tool rather than off the step's return value, and that is
        // the D1 fix arriving at the call site. The old line took the one step a
        // dig *landed* on and restarted the swing there, which pinned the figure
        // on frame 0 whenever the button was held: the tool landed a dig every 6
        // steps and the swing needed 24.
        //
        // The swing is now a duration the simulation owns and this only reports
        // where in it the tool is. **The direction still holds** -
        // ENGINEERING_NOTES.md refuses rendering that drives simulation, and
        // this is a read, on the fixed step, of a value the tool would have
        // computed with no window attached.
        cond.dig_progress = run.dig_tool.swing_progress();
        cond.flapped = run.player.flapped();
        player_anim::update(anim_state, cond, 1);
    };

    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            else if (screen == Screen::Settings && e.type == SDL_KEYDOWN) {
                // Keyboard-driven rather than click-driven, and that is a
                // scope decision rather than a taste one: this project has no
                // widget layer, no hit-testing and no focus model, and adding
                // three of them to change a resolution would be a larger
                // feature than the one being built. Arrow keys need none of it.
                //
                // **This switch is now only the binding** - which keysyms mean
                // which of the menu's four verbs. Every branch that used to
                // follow (what a confirm does on a mode that does not fit, what
                // the cursor does when a switch fails, whether a failed *save*
                // still closes the screen) is `menu::key` in
                // game/settings_menu.h, where `shell_test` can reach it.
                menu::Key mk = menu::Key::Back;
                bool bound = true;
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_w: mk = menu::Key::Prev; break;
                    case SDLK_DOWN:
                    case SDLK_s: mk = menu::Key::Next; break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                    case SDLK_SPACE: mk = menu::Key::Confirm; break;
                    case SDLK_ESCAPE: mk = menu::Key::Back; break;
                    default: bound = false; break;
                }
                if (bound) {
                    menu::Outcome out = menu::key(menu_state, mk, mode_available,
                                                  DISPLAY_MODE_COUNT, mode_index);
                    // The one thing the state machine cannot do for itself: try
                    // the switch, on a window, and report which way it went.
                    if (out.act == menu::Act::ApplyMode) {
                        const DisplayMode& wanted = DISPLAY_MODES[out.mode];
                        if (apply_mode(window, renderer, wanted, targets)) {
                            mode = wanted;
                            mode_index = out.mode;
                            // Persisted at the point it takes effect, not on the
                            // way out of the menu: a crash between the two would
                            // otherwise lose a setting the player watched
                            // succeed.
                            out = menu::mode_applied(menu_state, out.mode,
                                                     save_display_mode(mode));
                        } else {
                            out = menu::mode_refused(menu_state, mode_index);
                        }
                    }
                    if (out.notice) {
                        menu_notice = out.notice;
                        menu_notice_timer = out.notice_seconds;
                    }
                    if (out.act == menu::Act::Close) screen = Screen::Playing;
                    else if (out.act == menu::Act::Quit) running = false;
                }
            }
            else if (e.type == SDL_MOUSEWHEEL) {
                brush_size += e.wheel.y;
                if (brush_size < 1) brush_size = 1;
                if (brush_size > 32) brush_size = 32;
            }
            else if (e.type == SDL_KEYDOWN) {
                // The material keys used to be a switch here. They are a loop
                // over ui::HOTBAR now for one reason: the row drawn at the
                // bottom of the screen has to be telling the truth about what
                // each key does, and the only way to guarantee that is for the
                // binding and the icon to come out of the same table.
                for (int i = 0; i < ui::HOTBAR_COUNT; ++i) {
                    if (e.key.keysym.sym == ui::HOTBAR[i].key) {
                        current_brush = ui::HOTBAR[i].type;
                        break;
                    }
                }
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    screen = Screen::Settings;
                    menu::open(menu_state, mode_index);
                }
                // S0. **Only while the run is over**, so `R` is inert during
                // play rather than a key that throws a session away by
                // mis-hitting it - which is the same argument that moved
                // quitting off ESC and into the settings menu.
                //
                // **T1 adds the unconditional version and does not merge the
                // two, and that is left as a decision rather than closed.** The
                // debug reset works mid-run, which is exactly the mis-hit `R`
                // refuses, so it takes a modifier: Ctrl+R is not a key anybody
                // reaches for while aiming for sand, and the two meanings stay
                // apart without `R` acquiring a second one. Whether they should
                // eventually be one key is still open in ROADMAP_ITEMS.md.
                //
                // `else if`, because with the run over both branches match and a
                // reset that ran twice would be invisible - the second one
                // produces exactly the world the first one did.
                // **Held keys repeat, and every binding below except `.` is a
                // toggle or a one-shot.** SDL sends a KEYDOWN per repeat, so
                // without this a held `P` flickers the pause on and off at the
                // OS repeat rate and a held `Ctrl`+`R` resets the world dozens
                // of times a second - both of which read as the key not working
                // rather than as working too well. `.` is the exception and
                // wants the repeats: stepping at the repeat rate is how you
                // scrub through a collapse.
                const bool repeat = e.key.repeat != 0;
                const bool ctrl_held = (e.key.keysym.mod & KMOD_CTRL) != 0;
                if (e.key.keysym.sym == SDLK_r && ctrl_held && !repeat) {
                    // **The same seed, not a fresh one, and there are two
                    // reasons.** A debugging session is worth nothing if the
                    // world it is being debugged in changes underneath it. And
                    // the recorder's header seed is written once at startup, so
                    // a reset onto a new seed would silently make every session
                    // saved afterwards replay into the wrong world - a log that
                    // is wrong rather than absent, which is the failure P4
                    // exists to prevent rather than introduce. A reseeding reset
                    // would have to rewrite that header, and nothing in the plan
                    // asks for one.
                    restart_run();
                    record_notice = "WORLD RESET";
                    record_notice_timer = 2.0;
                } else if (e.key.keysym.sym == SDLK_r && !repeat &&
                           run.outcome() != Run::Outcome::Playing) {
                    restart_run();
                }

                // --- T1: pause, single-step, the free camera, the inspector ---
                if (e.key.keysym.sym == SDLK_p && !repeat) debug.toggle_pause();
                if (e.key.keysym.sym == SDLK_PERIOD) debug.request_single_step();
                if (e.key.keysym.sym == SDLK_i && !repeat) debug.inspector = !debug.inspector;
                if (e.key.keysym.sym == SDLK_f && !repeat) {
                    if (debug.free_camera) {
                        debug.attach_camera();
                    } else {
                        // Handed the *player's* centre rather than the camera's
                        // own, which are the same view: `Camera::follow` was
                        // given this exact number last frame and clamped it, and
                        // `detach_camera` clamps it the same way. Reading it back
                        // off the camera would mean deriving a centre from a
                        // clamped view, which is the one arithmetic here that
                        // could put the view somewhere it was not.
                        debug.detach_camera(static_cast<float>(run.player.center_x()),
                                            static_cast<float>(run.player.center_y()),
                                            mode.padded_w(), mode.padded_h(),
                                            GRID_WIDTH, GRID_HEIGHT);
                    }
                }
                // F9 writes everything played so far to a session log (P4).
                // Deliberately not bound to a letter: every letter within reach
                // of the movement keys is a hotbar slot, and a key that saves a
                // file is a bad thing to hit while reaching for sand.
                if (e.key.keysym.sym == SDLK_F9) {
                    // The end state is captured at the moment of writing, not at
                    // the end of the session, because this *is* the end of the
                    // recording being written - the replay has to check against
                    // the world the last recorded step produced.
                    recording.header.end_fingerprint = input_log::fingerprint(run.grid);
                    recording.header.end_player_x = run.player.cell_x();
                    recording.header.end_player_y = run.player.cell_y();

                    // A second save in one session does not overwrite the first:
                    // two takes of a session are two measurements, and the
                    // interesting one is often the earlier.
                    const std::string path = saved_logs == 0
                        ? std::string("session.rec")
                        : "session_" + std::to_string(saved_logs + 1) + ".rec";

                    std::string log_error;
                    if (input_log::write(path.c_str(), recording, &log_error)) {
                        saved_logs++;
                        record_notice = "SAVED " + path + "  " +
                                        std::to_string(recording.steps.size()) + " STEPS";
                        std::printf("Recorded session written to %s: %d steps, seed %llu\n",
                                    path.c_str(), static_cast<int>(recording.steps.size()),
                                    static_cast<unsigned long long>(world_seed));
                    } else {
                        record_notice = "COULD NOT SAVE THE SESSION LOG";
                        std::fprintf(stderr, "ERROR: %s\n", log_error.c_str());
                    }
                    record_notice_timer = 4.0;
                }
            }
        }

        // **Moved above the camera by T1**, because the free camera pans in real
        // time and has to have panned before the view is aimed - otherwise the
        // pan lands a frame late and, worse, the mouse-to-world conversion below
        // it resolves the cursor against last frame's view, which is a cell of
        // aiming error at every pan speed and several at a fast one.
        const uint64_t now_counter = SDL_GetPerformanceCounter();
        double frame_time = static_cast<double>(now_counter - prev_counter) / counter_freq;
        prev_counter = now_counter;
        frame_time = pacer::clamp_frame_time(frame_time);

        if (menu_notice_timer > 0.0) menu_notice_timer -= frame_time;
        if (record_notice_timer > 0.0) record_notice_timer -= frame_time;

        // Movement is read from live key state rather than key events, so
        // holding a key keeps moving instead of firing once and repeating on the
        // OS key-repeat delay. Sampled here, once, and used for both the free
        // camera's pan and the body's input below.
        const uint8_t* keys = SDL_GetKeyboardState(nullptr);

        // --- T1: panning the free camera ---
        //
        // The same keys that move the body, because while the camera is detached
        // the body is not being driven - see the input block below. Two sets of
        // movement keys, one of them dead depending on a mode, is how you end up
        // pressing the wrong one for a whole session.
        //
        // Real seconds rather than fixed steps: the camera is presentation, it
        // moves while the world is paused, and a pan measured in simulated time
        // would stop dead exactly when you paused to look at something.
        if (debug.free_camera && screen == Screen::Playing) {
            float pan_x = 0.0f, pan_y = 0.0f;
            if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])  pan_x -= 1.0f;
            if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) pan_x += 1.0f;
            if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])    pan_y -= 1.0f;
            if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])  pan_y += 1.0f;
            const bool fast = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
            const float speed = DebugView::PAN_CELLS_PER_SECOND *
                                (fast ? DebugView::PAN_FAST_MULTIPLIER : 1.0f) *
                                static_cast<float>(frame_time);
            debug.pan(pan_x * speed, pan_y * speed, mode.padded_w(), mode.padded_h(),
                      GRID_WIDTH, GRID_HEIGHT);
        }

        // The viewport follows the player, clamped at the world's edges
        // (F3.4). Recomputed once per rendered frame, same as the mouse and
        // keyboard samples below it - "the player's position this frame" has
        // exactly the same one-sample-per-frame character as those do.
        //
        // Or follows T1's free camera, which is the whole of what detaching
        // means: `Camera` is unchanged and still owns every conversion and the
        // clamp, it is simply handed a different centre.
        if (debug.free_camera) {
            camera.follow(debug.cam_x, debug.cam_y,
                          mode.padded_w(), mode.padded_h(), GRID_WIDTH, GRID_HEIGHT);
        } else {
            camera.follow(static_cast<float>(run.player.center_x()), static_cast<float>(run.player.center_y()),
                          mode.padded_w(), mode.padded_h(), GRID_WIDTH, GRID_HEIGHT);
        }

        // Handle continuous mouse pressing
        int mouseX, mouseY;
        const uint32_t mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        const int gridX = camera.screen_to_world_x(mouseX);
        const int gridY = camera.screen_to_world_y(mouseY);

        // Sampled once per rendered frame, same as before - a real mouse and
        // keyboard cannot be sampled at the simulation's fixed rate, since
        // there is no such thing as "the input for a step that has not
        // happened yet". What changed is what happens to this sample: every
        // fixed step this frame accumulates gets its own call to run.step()
        // with it, rather than the brush being painted once up here before
        // the loop even starts.
        //
        // **The free camera drives the pan keys instead of the body, and the
        // suppression happens here rather than inside `Run`.** Two consequences
        // worth stating. The body genuinely stands still while you look around,
        // rather than walking off the far side of the world unwatched. And what
        // the recorder stores is what the simulation was given, so a session
        // with debug camera work in it still replays exactly - the alternative,
        // suppressing after the record, would write a log of keys the world
        // never saw.
        Input input;
        input.left  = !debug.free_camera && (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]);
        input.right = !debug.free_camera && (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]);
        input.jump  = !debug.free_camera &&
                      (keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]);
        // Sticky facing, updated only when a direction is actually held. Both
        // keys down at once keeps the current facing rather than picking one,
        // which matches what the body does - Player cancels the two against
        // each other and stands still.
        if (input.left != input.right) facing_left = input.left;
        input.cursor_x = gridX;
        input.cursor_y = gridY;
        input.dig   = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
        // Right-click, not left. Digging is the game's action and gets the
        // primary button; the material brush is a development tool and moved
        // out of its way.
        input.brush_active = (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
        input.brush_type = current_brush;
        input.brush_size = brush_size;

        // V23's anchor update stood here and is gone (V23b). The camera is
        // centred and stays centred; nothing per-frame is left to do to it
        // besides the `follow` further down.
        //
        // Worth keeping the note it leaves behind: the anchor was updated
        // *after* the cursor was resolved, on purpose, so that the aim was read
        // through the frame the player was actually looking at. Any later
        // feature that moves the view from player input inherits that ordering
        // problem and the positive feedback loop behind it - the argument is in
        // ROADMAP.md's V23 entry, which is the only place it survives now.

        // **The freeze rule is `pacer::world_advances` now (W5), and it is one
        // function because it is one mechanism.** The menu froze the world
        // first, a finished run followed it in S0, T1's pause was the third, and
        // the whole argument - that freezing means *not accumulating time*,
        // never skipping the step loop with the accumulator still filling - is
        // written at the function rather than here, because that is where the
        // fourth caller will read it. The rejected version banks every frozen
        // second and spends it in one catch-up burst at the stall clamp, which
        // is a visible lurch; it matters most for a finished run, where the last
        // thing that happened is the thing the player has to be able to look at.
        const bool run_over = run.outcome() != Run::Outcome::Playing;
        const int steps = frame_pacer.steps(
            frame_time,
            pacer::world_advances(screen == Screen::Settings, run_over, debug.paused));
        for (int i = 0; i < steps; ++i) advance_one_step(input);

        // T1's single-step. Outside the pacer's loop and spending none of its
        // time, which is what makes `.` advance the world by exactly one step
        // and leave the accumulator where the pause left it.
        while (debug.consume_single_step()) advance_one_step(input);

        // Where between the last two simulated states this frame falls, and the
        // teleport clamp that stops `resolve_overlap`'s several-cell shove being
        // eased across. Both are `game/pacer.h` (W5) - the reasoning for the
        // pin-to-1-while-paused and for the clamp is at each of them there.
        // `prev_*` still lives outside the loop, which is what lets alpha keep
        // climbing on the frames that buy no step at all.
        const float alpha = frame_pacer.alpha(debug.paused);
        const pacer::Interpolated draw_player =
            pacer::interpolate(prev_player_x, prev_player_y,
                               run.player.visual_x(), run.player.visual_y(), alpha);
        float draw_player_x = draw_player.x;
        float draw_player_y = draw_player.y;

        // Re-aimed at the interpolated position before drawing. The call above
        // is what the mouse-to-world conversion needed and runs before the
        // step; this one is what the *render* needs, and using the pre-step
        // camera for it would reintroduce exactly the whole-frame lag between
        // the player and the world that this defect is about.
        //
        // The detached camera has nothing to re-aim - it is not tracking
        // anything that moved - so this whole correction is the attached case's,
        // and applying it to a free camera would drag the view towards the
        // player it was detached from.
        if (!debug.free_camera) {
            camera.follow(draw_player_x + Player::WIDTH / 2.0f, draw_player_y + Player::HEIGHT / 2.0f,
                          mode.padded_w(), mode.padded_h(), GRID_WIDTH, GRID_HEIGHT);
        }

        // Upload only the visible rect (F3.3), not the whole grid, starting
        // from the camera's current view (F3.4) rather than always (0, 0).
        // Clamped to the grid's own size so this stays correct if the grid is
        // ever smaller than the viewport; the case this step exists for is the
        // opposite one, a grid larger than the viewport, where the clamp is a
        // no-op and the rect is the full viewport every frame.
        //
        // **This comment used to end "one call, no intermediate buffer", and
        // V25 spent that.** The zero-copy upload worked by handing SDL the
        // grid's own buffer with a pitch of GRID_WIDTH, so SDL read the right
        // columns out of each row and skipped the rest. `surface_plane::apply`
        // has to *change* some of those pixels - see decision 1 in its header,
        // which is that the near terrain has to get brighter than it is and a
        // colour multiply can only darken - so there is now a viewport-sized
        // copy between the grid and the texture. It costs 480x270 words at
        // 1920x1080, half a megabyte, once a frame. **The old arrangement is
        // still reachable and still used**: a window row that is not on the
        // plane, which is every row above the horizon, is a straight copy, and
        // when the tile failed to load the whole pass is one.
        const std::vector<uint32_t>& pixels = run.grid.get_pixels();
        const int visible_w = std::min(mode.padded_w(), GRID_WIDTH);
        const int visible_h = std::min(mode.padded_h(), GRID_HEIGHT);
        const SDL_Rect visible_rect{0, 0, visible_w, visible_h};

        // Which tile row each window row of cells is looking at, or -1 for the
        // rows that are not on the plane at all.
        //
        // **The near edge clamps to the tile's last row rather than falling off
        // to -1**, which is the same choice frame.cpp makes in its fill below
        // the plane's near edge, and for the same reason stated there: what is
        // past the near end of a receding plane is ground nearer still, so the
        // tile's nearest row continues rather than the effect stopping. Dropping
        // to -1 here would put a horizontal line across the terrain at whatever
        // row the plane's near edge happened to reach.
        const backdrop_wrap::Plane plane = backdrop_wrap::plane_geometry(
            frame::ground_horizon_y(camera, backdrop.mountain_h),
            backdrop.ground_h,
            backdrop_layers::GROUND.parallax_x,
            backdrop_layers::GROUND_NEAR_X);
        const float band = plane.bottom_y - plane.horizon_y;
        plane_src_row_for.assign(static_cast<size_t>(visible_h), -1);
        plane_row_scale.assign(static_cast<size_t>(visible_h), 0);
        for (int wy = 0; wy < visible_h; ++wy) {
            // The centre of the cell row, in screen pixels, including the
            // camera's sub-cell remainder - the same shift draw_cells applies to
            // the texture this feeds. Sampling the top edge instead would bias
            // every row half a cell toward the horizon.
            const float screen_y =
                (static_cast<float>(wy) + 0.5f - camera.frac_y()) * Camera::SCALE;
            if (band <= 0.0f || screen_y < plane.horizon_y) continue;
            const float t = (screen_y - plane.horizon_y) / band;
            const int scale = surface_plane::row_scale_at(t);
            if (scale <= 0) continue;
            int row = backdrop.ground_h - 1;
            if (t < 1.0f) {
                row = static_cast<int>(backdrop_wrap::plane_src_at(plane, t) + 0.5f);
                if (row < 0) row = 0;
                if (row > backdrop.ground_h - 1) row = backdrop.ground_h - 1;
            }
            plane_src_row_for[static_cast<size_t>(wy)] = row;
            plane_row_scale[static_cast<size_t>(wy)] = scale;
        }

        const size_t window_cells =
            static_cast<size_t>(visible_w) * static_cast<size_t>(visible_h);
        if (cell_window.size() != window_cells) cell_window.assign(window_cells, 0u);
        if (cell_depth.size() != window_cells) cell_depth.assign(window_cells, -1);

        const surface_plane::View view{camera.view_x(), camera.view_y(), visible_w, visible_h};
        const surface_plane::TileRows tile_rows{
            ground_rows.empty() ? nullptr : ground_rows.data(),
            static_cast<int>(ground_rows.size() / 3)
        };
        surface_plane::apply(pixels.data(), GRID_WIDTH, GRID_HEIGHT, view,
                             tile_rows, plane_src_row_for.data(), plane_row_scale.data(),
                             ground_grade.r, ground_grade.g, ground_grade.b,
                             cell_depth.data(), cell_window.data());
        SDL_UpdateTexture(targets.cells, &visible_rect, cell_window.data(),
                          visible_w * sizeof(uint32_t));

        // V7. Computed against the same view origin the cell upload just used,
        // and after `camera.follow` for the same reason that upload is: a light
        // field built from last frame's view would slide against the world it is
        // lighting, which is defect A1 reintroduced through a new feature.
        //
        // Recomputed from scratch every frame rather than carried between them.
        // It is affordable (the scan is the padded viewport, the propagation is
        // ~2,000 blocks) and the alternative is a cache keyed on both the camera
        // and every temperature in view - which is a correctness problem in
        // exchange for saving something already too cheap to measure.
        targets.light.update(run.grid, camera.view_x(), camera.view_y());
        if (targets.light.any_light()) {
            SDL_UpdateTexture(targets.light_texture, nullptr, targets.light.pixels().data(),
                              targets.light.cols() * sizeof(uint32_t));
        }

        // --- the world layers (V17) ---
        //
        // Everything from the clear to V7's light pass now lives in
        // render/frame.cpp, moved there verbatim, and `golden_frame_test`
        // checksums the result so the move can be shown to have changed
        // nothing. **What stays below this call is UI**, and that split is not
        // filing: the light pass is the last thing in the world, and anything
        // drawn after it is deliberately not lit.
        frame::Params fp;
        fp.camera = &camera;
        fp.padded_w = mode.padded_w();
        fp.padded_h = mode.padded_h();
        fp.backdrop = backdrop;
        fp.props = &props;
        fp.cells = targets.cells;
        fp.has_objective = run.has_objective();
        fp.objective_x = run.objective_x();
        fp.objective_y = run.objective_y();
        fp.player_tex = player_tex;
        fp.player_x = draw_player_x;
        fp.player_y = draw_player_y;
        fp.facing_left = facing_left;
        fp.sheet_col = anim_state.sheet_col();
        fp.sheet_row = anim_state.sheet_row();
        fp.player_box_w = Player::WIDTH;
        fp.player_box_h = Player::HEIGHT;
        fp.light = &targets.light;
        fp.light_texture = targets.light_texture;
        frame::compose(renderer, fp);

        // --- the screen-space layer (W5 part 3) ---
        //
        // Everything drawn after `frame::compose` is UI, and all of it now
        // lives in render/overlay.cpp behind one call. What stays here is the
        // half that reads simulation state - whether the cursor is in reach,
        // what the readout says, which lines T1 wants under it, which hotbar
        // slot the brush is in - because nothing under src/render/ may ask a
        // Run or a Grid anything. overlay.h says why the split is drawn there
        // and why this is a second file rather than rows in the layer table.
        const int dx_cells = gridX - run.player.center_x();
        const int dy_cells = gridY - run.player.center_y();

        overlay::Params op;
        op.window_w = mode.window_w;
        op.window_h = mode.window_h;
        op.ui_scale = mode.ui_scale();

        // Only while the pointer is actually over this window.
        // `SDL_GetMouseState` keeps reporting the last position inside the
        // window after the mouse leaves, so without this the reticle sticks to
        // the edge and reads as a frozen UI element rather than as a cursor
        // that has gone elsewhere.
        op.show_reticle = (SDL_GetMouseFocus() == window);
        op.mouse_x = mouseX;
        op.mouse_y = mouseY;
        op.in_range =
            (dx_cells * dx_cells + dy_cells * dy_cells) <= DigTool::RANGE * DigTool::RANGE;

        hud_text = "FPS:" + std::to_string(fps_display) +
                   " BRUSH:" + material_of(current_brush).name + "(" + std::to_string(brush_size) + ")" +
                   " CHUNKS:" + std::to_string(run.grid.active_chunk_count());
        // **`CHUNKS:0` does not mean the world has stopped**, and this suffix is
        // the correction arriving on screen. A falling structural piece is
        // carried by the support queue rather than by the chunk rects, so a slab
        // can fall the height of the world with this counter reading zero the
        // whole way - found on 2026-08-14 with T1's own inspector, corrected at
        // `Grid::active_chunk_count()` and pinned in `test_debug.cpp`. Shown as
        // a flag rather than a count because the queue's length is a number of
        // seeds, not of pieces, and would read as far more work than it is.
        if (run.grid.has_pending_support_checks()) hud_text += "+FALLING";

        // --- S0's readout ---
        //
        // **`HP` goes first, ahead of the three diagnostics.** The rest of this
        // line is an instrument for whoever is building the engine; this is the
        // one thing on it a player is playing against, and reading it should not
        // mean scanning past a frame rate.
        //
        // **`GOAL` is a bearing, and it is text on the line that already exists
        // rather than an arrow at the screen edge**, which keeps S0's "no UI
        // beyond the readout" limit intact while answering the question the
        // objective actually raises: it is ~740 cells from the spawn and the
        // viewport is 480 cells wide, so it starts off-screen. Without this the
        // run is "walk east until you find it", which is not a difficulty, it is
        // a missing instrument. Distance is to the body's centre, in cells.
        std::string status = "HP:" + std::to_string(run.player.health());
        if (run.has_objective()) {
            const int gdx = run.objective_x() - run.player.center_x();
            const int gdy = run.objective_y() - run.player.center_y();
            const int gdist = static_cast<int>(std::sqrt(
                static_cast<double>(gdx) * gdx + static_cast<double>(gdy) * gdy));
            status += "  GOAL:" + std::to_string(gdist) + (gdx < 0 ? "W" : "E");
        }
        op.hud_text = status + "  " + hud_text;

        // The lines under the readout, pushed in the order they are drawn.
        // A list rather than three calls at three sites, for the reason the
        // cursor in overlay.cpp exists: T1 makes all of these conditional, and
        // independently-computed offsets are how two end up drawn on top of
        // each other in whichever combination nobody tried.

        // The recorder's line, drawn only while it has something to say (P4).
        // A permanent "REC" indicator was the other option and is worse: this
        // records every session, so an always-on marker would be furniture
        // within a minute and invisible by the time it mattered.
        if (record_notice_timer > 0.0 && !record_notice.empty()) {
            op.hud_lines.push_back({record_notice, 0xFFFFC080});
        }

        // --- T1's two lines ---
        //
        // The mode line first, because it is the one that explains why the game
        // is not responding the way it usually does, and a player who has hit
        // `P` by accident has to be told before anything else.
        {
            const std::string modes = debug_status(debug);
            if (!modes.empty()) op.hud_lines.push_back({modes, 0xFF80D0FF});
        }
        if (debug.inspector) {
            // Reads the cell the cursor names, which the free camera can now put
            // outside the world - `describe_cell` says so rather than clamping,
            // because "there is no cell there" and "there is an empty cell
            // there" are different answers and a debug tool that conflates them
            // is the same failure as a legend resolving an unknown colour to
            // Empty.
            op.hud_lines.push_back({describe_cell(run.grid, gridX, gridY), 0xFFE0E0E0});
        }

        // --- the material hotbar (V10) ---
        //
        // The selected slot is looked up rather than stored, so `current_brush`
        // stays the single fact about what is selected. A second index kept
        // beside it is exactly how a highlight ends up on the wrong box after
        // some later code path sets the brush without going through a key.
        op.hotbar_selected = -1;
        for (int i = 0; i < ui::HOTBAR_COUNT; ++i) {
            if (ui::HOTBAR[i].type == current_brush) op.hotbar_selected = i;
        }

        op.run_over = run_over;
        op.won = run.outcome() == Run::Outcome::Won;

        op.settings_open = (screen == Screen::Settings);
        op.cursor = menu_cursor;
        op.mode_count = DISPLAY_MODE_COUNT;
        op.current_mode = mode_index;
        op.modes = DISPLAY_MODES;
        op.available = mode_available;
        // An expired notice is handed over as no notice at all. The timer is a
        // fact about this loop, not about the drawing, and passing it would put
        // a second copy of "is it still showing" on the far side of the seam.
        if (menu_notice_timer > 0.0) op.notice = menu_notice;

        overlay::draw(renderer, op);

        SDL_RenderPresent(renderer);

        // Surface the frame rate so performance regressions are visible while working.
        frames_this_second++;
        title_timer += frame_time;
        if (title_timer >= 1.0) {
            // Only the frame rate is cached - it is a rate, and a rate needs an
            // interval to be measured over. The brush name and the awake-chunk
            // count are read straight from live state where the string is built
            // above; awake chunks in particular are shown because they explain
            // the frame rate, and a count that lags by a second cannot.
            fps_display = frames_this_second;
            frames_this_second = 0;
            title_timer = 0.0;
        }
    }

    if (player_tex) SDL_DestroyTexture(player_tex);
    // The cache is the destroy list - one entry per distinct sprite name, so a
    // scene with fifty trees of three kinds still frees exactly three textures
    // and `props` holding several borrowed copies of each is not a double free.
    for (auto& entry : prop_textures)
        if (entry.second) SDL_DestroyTexture(entry.second);
    if (backdrop.ground) SDL_DestroyTexture(backdrop.ground);
    if (backdrop.mountains) SDL_DestroyTexture(backdrop.mountains);
    if (backdrop.sky) SDL_DestroyTexture(backdrop.sky);
    SDL_DestroyTexture(targets.light_texture);
    SDL_DestroyTexture(targets.cells);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
