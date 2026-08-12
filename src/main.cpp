#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <string>
#include <vector>
#include "game/camera.h"
#include "game/display.h"
#include "game/run.h"
#include "render/light.h"
#include "render/player_anim.h"
#include "scene/legend.h"
#include "scene/props.h"
#include "scene/scene.h"
#include "scene/sprites.h"
#include "ui/text.h"
#include "ui/hotbar.h"

// The simulated world's size, in cells - independent of the window since F3.1.
// Not equal to any window size divided by Camera::SCALE: that was only ever a
// coincidence of nothing having needed them to differ yet, and F4's scene is
// authored at 1920x1080 cells (the fixture's original 640x400 rescaled with
// the player body - see generate_test_scene.py), bigger than every viewport in
// the display table. That is deliberate, not a mismatch to fix - it exercises the
// panning half of the camera (F3.4) rather than just the decoupling half
// (F3.1-F3.3), which a world equal to the viewport never did.
//
// It also has to clear the *largest* viewport, not the one the game happens to
// launch at: 3440x1440 sees 861x361 padded cells, so a world sized to the
// smallest mode would leave the widest one with nothing to pan across.
//
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
const int GRID_WIDTH = 1920;
const int GRID_HEIGHT = 1080;

const double MAX_FRAME_TIME = 0.25; // clamp after a stall so we don't spiral

Scene load_scene_from_bmp(const char* material_path, const char* albedo_path) {
    Scene scene;
    SDL_Surface* mat_surf = SDL_LoadBMP(material_path);
    SDL_Surface* alb_surf = SDL_LoadBMP(albedo_path);

    if (!mat_surf || !alb_surf) {
        if (mat_surf) SDL_FreeSurface(mat_surf);
        if (alb_surf) SDL_FreeSurface(alb_surf);
        std::fprintf(stderr, "Failed to load scene BMPs\n");
        return scene;
    }

    if (mat_surf->w != alb_surf->w || mat_surf->h != alb_surf->h) {
        SDL_FreeSurface(mat_surf);
        SDL_FreeSurface(alb_surf);
        std::fprintf(stderr, "Scene BMP dimensions do not match\n");
        return scene;
    }

    scene.width = mat_surf->w;
    scene.height = mat_surf->h;
    scene.materials.resize(scene.width * scene.height, ElementType::Empty);
    scene.albedo.resize(scene.width * scene.height, 0);

    // Assuming 24-bit or 32-bit BMPs.
    // We should lock surfaces if needed, but SDL_LoadBMP gives 24-bit or 8-bit.
    // For simplicity, we convert both to 32-bit ARGB.
    SDL_Surface* mat_32 = SDL_ConvertSurfaceFormat(mat_surf, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_Surface* alb_32 = SDL_ConvertSurfaceFormat(alb_surf, SDL_PIXELFORMAT_ARGB8888, 0);

    SDL_FreeSurface(mat_surf);
    SDL_FreeSurface(alb_surf);

    if (!mat_32 || !alb_32) {
        if (mat_32) SDL_FreeSurface(mat_32);
        if (alb_32) SDL_FreeSurface(alb_32);
        return scene;
    }

    // Indexed via pitch rather than width * 4: SDL_ConvertSurfaceFormat is free
    // to pad each row for alignment, and reading straight across the buffer as
    // if pitch == width * 4 would drift a row further off with every line on
    // any width where that assumption doesn't hold.
    const uint8_t* mat_base = static_cast<const uint8_t*>(mat_32->pixels);
    const uint8_t* alb_base = static_cast<const uint8_t*>(alb_32->pixels);

    // A pixel that names no material is a *fault in the scene file*, not an
    // empty cell, and the two used to be indistinguishable here - which is how
    // a palette change silently emptied the whole world. Counted, reported, and
    // the first few offenders named, because "3 unmatched" sends you looking
    // and "#4444FF" tells you what happened.
    int unmatched = 0;
    uint32_t first_unmatched[4] = {0, 0, 0, 0};
    int first_unmatched_n = 0;

    for (int y = 0; y < scene.height; ++y) {
        const uint32_t* mat_row = reinterpret_cast<const uint32_t*>(mat_base + y * mat_32->pitch);
        const uint32_t* alb_row = reinterpret_cast<const uint32_t*>(alb_base + y * alb_32->pitch);
        for (int x = 0; x < scene.width; ++x) {
            int idx = y * scene.width + x;
            uint32_t m_col = mat_row[x];
            uint32_t a_col = alb_row[x];

            // The legend is its own frozen table (scene/legend.h), deliberately
            // not the render palette - see there for what binding the two cost.
            ElementType type = ElementType::Empty;
            if (!element_from_legend(m_col, type)) {
                const uint32_t rgb = m_col & 0xFFFFFF;
                bool seen = false;
                for (int i = 0; i < first_unmatched_n; ++i) if (first_unmatched[i] == rgb) seen = true;
                if (!seen && first_unmatched_n < 4) first_unmatched[first_unmatched_n++] = rgb;
                ++unmatched;
                type = ElementType::Empty;
            }

            scene.materials[idx] = type;
            scene.albedo[idx] = a_col | 0xFF000000; // force alpha
        }
    }

    if (unmatched > 0) {
        std::fprintf(stderr,
                     "WARNING: %s has %d pixel(s) whose colour is in no legend entry; they loaded as Empty.\n"
                     "         Unrecognised colours include:", material_path, unmatched);
        for (int i = 0; i < first_unmatched_n; ++i) std::fprintf(stderr, " #%06X", first_unmatched[i]);
        std::fprintf(stderr, "\n         The legend is src/scene/legend.h and is frozen; the render palette"
                             " in material.h is not the legend.\n");
    }

    SDL_FreeSurface(mat_32);
    SDL_FreeSurface(alb_32);

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

// A non-simulated sprite anchored to a world position - V4's props layer.
// Exercises no system and is never dug, ignited or displaced; see notes/
// art_direction.txt for why trees specifically are drawn *before* the cell
// texture rather than after; it is that ordering, not anything in this
// struct, that gives a planted trunk its occlusion for free.
struct Prop {
    SDL_Texture* texture;
    int w, h;         // native size, in world cells (1 BMP pixel = 1 cell)
    float anchor_x;   // world cell, bottom-centre of the sprite
    float anchor_y;
};

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
    //
    // The stored setting wins if it is still valid *and* still fits - a monitor
    // can change between runs, and a saved 3440x1440 on a laptop screen would
    // otherwise open a window nobody can reach the settings menu inside. Failing
    // that, the largest mode that fits, which is the one that shows the most
    // world.
    bool mode_available[DISPLAY_MODE_COUNT];
    int mode_index = -1;
    for (int i = 0; i < DISPLAY_MODE_COUNT; ++i) {
        mode_available[i] = mode_fits(DISPLAY_MODES[i], 0);
        if (mode_available[i]) mode_index = i;
    }
    if (mode_index < 0) {
        // Every mode is larger than the desktop. Open at the smallest anyway
        // rather than refusing to start: an oversized window is a bad session,
        // and no window at all is no session.
        std::fprintf(stderr, "No display mode fits this desktop; opening at %dx%d anyway.\n",
                     DISPLAY_MODES[0].window_w, DISPLAY_MODES[0].window_h);
        mode_index = 0;
    }
    {
        const int stored = load_display_mode();
        if (stored >= 0 && mode_available[stored]) {
            mode_index = stored;
        } else if (stored >= 0) {
            std::fprintf(stderr, "Stored mode %dx%d does not fit this display; ignoring it.\n",
                         DISPLAY_MODES[stored].window_w, DISPLAY_MODES[stored].window_h);
        }
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

    SDL_Texture* backdrop_sky = load_art_texture(
        renderer, sprites.path_for("backdrop_sky", "backdrop_sky.bmp").c_str(), false);
    SDL_Texture* backdrop_mountains = load_art_texture(
        renderer, sprites.path_for("backdrop_mountains", "backdrop_mountains.bmp").c_str(), true);
    constexpr float PARALLAX_SKY_X = 0.04f, PARALLAX_SKY_Y = 0.02f;
    constexpr float PARALLAX_MOUNTAIN_X = 0.15f, PARALLAX_MOUNTAIN_Y = 0.06f;
    int sky_w = 0, sky_h = 0, mountain_w = 0, mountain_h = 0;
    if (backdrop_sky) SDL_QueryTexture(backdrop_sky, nullptr, nullptr, &sky_w, &sky_h);
    if (backdrop_mountains) SDL_QueryTexture(backdrop_mountains, nullptr, nullptr, &mountain_w, &mountain_h);

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

    std::vector<Prop> props;
    props.reserve(prop_defs.size());
    for (const PropDef& def : prop_defs) {
        SDL_Texture* tex = prop_texture(def.sprite);
        if (!tex) continue; // already warned by prop_texture, once per name
        int w = 0, h = 0;
        SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
        props.push_back(Prop{ tex, w, h, def.x, 0.0f });
    }
    // The count is NOT printed here. A prop that has a texture is not yet a prop
    // that got placed - the terrain scan below drops any with no ground under
    // them - so a count taken at this point reports texture loads while saying
    // "placed". It read `9 of 9` on a run that drew 8. See below.

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
    
    // Load F4 test scene. A scene that resolves to no cells at all is reported
    // rather than shrugged off: README's launch check is "terrain is visible
    // immediately", and a blank world is exactly what a broken legend, a
    // missing file and an empty file all look like from here.
    Scene scene = load_scene_from_bmp("assets/test_material.bmp", "assets/test_albedo.bmp");
    if (scene.width > 0) {
        const int placed = load_scene(run.grid, scene, 0, 0);
        std::printf("Scene: %dx%d, %d cells placed\n", scene.width, scene.height, placed);
        if (placed == 0) {
            std::fprintf(stderr, "WARNING: the scene named no material anywhere - the world is empty.\n");
        }
    }

    // Plant each prop on the terrain that is actually under it, rather than on
    // a hardcoded ground line. **This is a fix for a class of bug, not for the
    // three trees that had it:** props were authored at `FLOOR_TOP`, which is
    // true of the floor slab and false of everything standing on it, so the
    // three trees over the authored sand slope were 26%, 43% and 83% buried -
    // invisibly, because they sit off-screen at spawn and the screenshot that
    // "confirmed" the feature was of the other side of the world.
    //
    // Scans the prop's own footprint rather than its centre column and takes
    // the *lowest* surface found, so a tree on a slope is planted at its
    // downhill edge and leans into the hill instead of floating off its uphill
    // one. Runs once, after the scene is stamped and before the first frame:
    // props are not simulated, so terrain that moves later does not drag them
    // with it - which is correct for a tree and is the same "exercises no
    // system" line that put them in this layer at all.
    //
    // **A prop with no ground under it is now dropped rather than left at a
    // fallback**, which the authored-y version could not do: it had a y to fall
    // back to, and falling back to it is exactly how the buried trees hid. With
    // no authored y the only fallback available is 0 - the top of the world -
    // so an unplantable prop would hang in the sky. Removing it and saying so
    // is the honest version of the same warning.
    std::vector<Prop> planted;
    planted.reserve(props.size());
    for (Prop& prop : props) {
        if (!prop.texture) continue;
        const int x0 = static_cast<int>(prop.anchor_x - prop.w / 2.0f);
        int lowest_surface = -1;
        for (int x = x0; x < x0 + prop.w; ++x) {
            if (x < 0 || x >= GRID_WIDTH) continue;
            for (int y = 0; y < GRID_HEIGHT; ++y) {
                if (is_solid(run.grid.get_element(x, y).type)) {
                    if (y > lowest_surface) lowest_surface = y;
                    break;
                }
            }
        }
        // No solid ground anywhere under it is a scene-authoring mistake, not
        // something to paper over with a default - a prop hanging in the air
        // is exactly as wrong as one buried, and silently placing it at the
        // fallback is how the first version of this hid its own bug.
        if (lowest_surface < 0) {
            std::fprintf(stderr, "WARNING: assets/test_props.txt: prop at x=%.1f has no ground "
                                 "under it and is not drawn.\n", prop.anchor_x);
            continue;
        }
        prop.anchor_y = static_cast<float>(lowest_surface);
        planted.push_back(prop);
    }
    props = std::move(planted);

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

    // Menu items: one per display mode, then Quit. The index is into this
    // combined list rather than into DISPLAY_MODES, so navigation does not need
    // to know that the last row is special.
    const int MENU_QUIT = DISPLAY_MODE_COUNT;
    int menu_cursor = mode_index;

    // Shown under the menu for a few seconds after a switch is attempted, so a
    // refused mode says so instead of looking like a dead key.
    std::string menu_notice;
    double menu_notice_timer = 0.0;

    uint64_t prev_counter = SDL_GetPerformanceCounter();
    const double counter_freq = static_cast<double>(SDL_GetPerformanceFrequency());
    double accumulator = 0.0;

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
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_w:
                        menu_cursor = (menu_cursor + MENU_QUIT) % (MENU_QUIT + 1);
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        menu_cursor = (menu_cursor + 1) % (MENU_QUIT + 1);
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                    case SDLK_SPACE:
                        if (menu_cursor == MENU_QUIT) {
                            running = false;
                        } else if (!mode_available[menu_cursor]) {
                            menu_notice = "That mode is larger than this display.";
                            menu_notice_timer = 4.0;
                        } else if (menu_cursor == mode_index) {
                            screen = Screen::Playing;
                        } else {
                            const DisplayMode& wanted = DISPLAY_MODES[menu_cursor];
                            if (apply_mode(window, renderer, wanted, targets)) {
                                mode = wanted;
                                mode_index = menu_cursor;
                                // Persisted at the point it takes effect, not
                                // on the way out of the menu: a crash between
                                // the two would otherwise lose a setting the
                                // player watched succeed.
                                if (!save_display_mode(mode)) {
                                    menu_notice = "Mode changed, but settings.txt could not be written.";
                                    menu_notice_timer = 6.0;
                                } else {
                                    screen = Screen::Playing;
                                }
                            } else {
                                // apply_mode left everything as it was, so
                                // there is nothing to roll back - only to say.
                                menu_cursor = mode_index;
                                menu_notice = "Could not create render targets at that size; mode unchanged.";
                                menu_notice_timer = 6.0;
                            }
                        }
                        break;
                    case SDLK_ESCAPE:
                        screen = Screen::Playing;
                        break;
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
                    menu_cursor = mode_index;
                }
            }
        }

        // The viewport follows the player, clamped at the world's edges
        // (F3.4). Recomputed once per rendered frame, same as the mouse and
        // keyboard samples below it - "the player's position this frame" has
        // exactly the same one-sample-per-frame character as those do.
        camera.follow(static_cast<float>(run.player.center_x()), static_cast<float>(run.player.center_y()),
                      mode.padded_w(), mode.padded_h(), GRID_WIDTH, GRID_HEIGHT);

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
        // the loop even starts. Movement is read from live key state rather
        // than key events, so holding a key keeps moving instead of firing
        // once and repeating on the OS key-repeat delay.
        const uint8_t* keys = SDL_GetKeyboardState(nullptr);
        Input input;
        input.left  = keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT];
        input.right = keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT];
        input.jump  = keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP];
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

        const uint64_t now_counter = SDL_GetPerformanceCounter();
        double frame_time = static_cast<double>(now_counter - prev_counter) / counter_freq;
        prev_counter = now_counter;
        if (frame_time > MAX_FRAME_TIME) frame_time = MAX_FRAME_TIME;

        if (menu_notice_timer > 0.0) menu_notice_timer -= frame_time;

        // The menu freezes the simulation by not accumulating time, rather than
        // by skipping the step loop with the accumulator still filling: the
        // second version banks every second spent in the menu and spends it in
        // one burst of catch-up steps on the way out, which at MAX_FRAME_TIME's
        // quarter-second clamp is a visible lurch. Not accumulating means the
        // world is exactly where it was left.
        if (screen == Screen::Playing) accumulator += frame_time;
        while (accumulator >= Run::FIXED_DT) {
            prev_player_x = run.player.visual_x();
            prev_player_y = run.player.visual_y();

            run.step(input);

            // The animation clock. Advanced here, inside the fixed-step loop,
            // and nowhere else - see the timing note at the top of
            // render/player_anim.h. Driving it from the render loop would make
            // the walk cycle's speed a function of frame rate, which is the
            // class of bug F1 and F2.3 spent two sections retiring and which
            // would present as an art problem.
            player_anim::Conditions cond;
            cond.on_ground = run.player.is_on_ground();
            cond.moving = std::abs(run.player.velocity_x()) > 0.01f;
            cond.vel_y = run.player.velocity_y();
            // Read off the tool rather than off the step's return value, and
            // that is the D1 fix arriving at the call site. The old line took
            // the one step a dig *landed* on and restarted the swing there,
            // which pinned the figure on frame 0 whenever the button was held:
            // the tool landed a dig every 6 steps and the swing needed 24.
            //
            // The swing is now a duration the simulation owns and this only
            // reports where in it the tool is. **The direction still holds** -
            // ENGINEERING_NOTES.md refuses rendering that drives simulation,
            // and this is a read, on the fixed step, of a value the tool would
            // have computed with no window attached.
            cond.dig_progress = run.dig_tool.swing_progress();
            cond.flapped = run.player.flapped();
            player_anim::update(anim_state, cond, 1);

            accumulator -= Run::FIXED_DT;
        }

        // How far between the last simulated state and the current one this
        // frame falls. Zero steps this frame is the normal case at 165 Hz, and
        // it is why `prev_*` lives outside the loop: alpha keeps climbing
        // towards 1 and the draw keeps easing towards the state already
        // computed, rather than freezing until the next step lands.
        const float alpha = static_cast<float>(accumulator / Run::FIXED_DT);
        float draw_player_x = prev_player_x + (run.player.visual_x() - prev_player_x) * alpha;
        float draw_player_y = prev_player_y + (run.player.visual_y() - prev_player_y) * alpha;

        // Teleports must not be interpolated. `resolve_overlap` can shift the
        // body several cells at once to push it out of terrain, and easing
        // across that draws the player skating through the wall it was just
        // rescued from. Anything larger than a stride is a jump, not motion.
        constexpr float MAX_INTERPOLATED_CELLS = 4.0f;
        if (std::abs(run.player.visual_x() - prev_player_x) > MAX_INTERPOLATED_CELLS ||
            std::abs(run.player.visual_y() - prev_player_y) > MAX_INTERPOLATED_CELLS) {
            draw_player_x = run.player.visual_x();
            draw_player_y = run.player.visual_y();
        }

        // Re-aimed at the interpolated position before drawing. The call above
        // is what the mouse-to-world conversion needed and runs before the
        // step; this one is what the *render* needs, and using the pre-step
        // camera for it would reintroduce exactly the whole-frame lag between
        // the player and the world that this defect is about.
        camera.follow(draw_player_x + Player::WIDTH / 2.0f, draw_player_y + Player::HEIGHT / 2.0f,
                      mode.padded_w(), mode.padded_h(), GRID_WIDTH, GRID_HEIGHT);

        // Upload only the visible rect (F3.3), not the whole grid, starting
        // from the camera's current view (F3.4) rather than always (0, 0).
        // The source pitch stays GRID_WIDTH wide even though the rect is
        // narrower, so SDL reads the right columns out of each grid row and
        // skips the rest - one call, no intermediate buffer. Clamped to the
        // grid's own size so this stays correct if the grid is ever smaller
        // than the viewport; the case this step exists for is the opposite
        // one, a grid larger than the viewport, where the clamp is a no-op
        // and the rect is the full viewport every frame.
        const std::vector<uint32_t>& pixels = run.grid.get_pixels();
        const int visible_w = std::min(mode.padded_w(), GRID_WIDTH);
        const int visible_h = std::min(mode.padded_h(), GRID_HEIGHT);
        const SDL_Rect visible_rect{0, 0, visible_w, visible_h};
        const uint32_t* visible_pixels = pixels.data() + camera.view_y() * GRID_WIDTH + camera.view_x();
        SDL_UpdateTexture(targets.cells, &visible_rect, visible_pixels, GRID_WIDTH * sizeof(uint32_t));

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

        // The backdrop layer (V8, replacing V1's gradient placeholder now
        // that there is authored art to show). Two static textures, each
        // shifted by the camera's continuous view position - view_x()/
        // view_y() plus the sub-cell frac_x()/frac_y() the world texture
        // below already uses for its own smooth scroll - scaled by SCALE and
        // the layer's own parallax factor. Full-texture draws with a
        // negative destination offset rather than a cropped source rect: the
        // art is static, so there is nothing to re-upload per frame, only
        // where it is drawn needs to move.
        const float cont_view_x = static_cast<float>(camera.view_x()) + camera.frac_x();
        const float cont_view_y = static_cast<float>(camera.view_y()) + camera.frac_y();

        // **The clear is load-bearing now in a way it was not before.** V1's
        // 64-band gradient filled the whole window every frame, so it doubled
        // as a clear and nothing here ever needed one. The authored sky that
        // replaced it is drawn behind an `if` - a missing or unreadable BMP
        // leaves the framebuffer holding whatever was in it, which on a
        // double-buffered renderer is two-frames-ago garbage rather than a
        // plain background. Clearing to the palette's darkest sky tone means
        // the failure mode is "the backdrop is flat" instead of "the window
        // is full of noise".
        SDL_SetRenderDrawColor(renderer, 0x14, 0x10, 0x22, 255); // sky_deep, tools/pixel_art.py
        SDL_RenderClear(renderer);

        if (backdrop_sky) {
            const SDL_FRect dst{
                -cont_view_x * Camera::SCALE * PARALLAX_SKY_X,
                -cont_view_y * Camera::SCALE * PARALLAX_SKY_Y,
                static_cast<float>(sky_w), static_cast<float>(sky_h)
            };
            SDL_RenderCopyF(renderer, backdrop_sky, nullptr, &dst);
        }
        if (backdrop_mountains) {
            const SDL_FRect dst{
                -cont_view_x * Camera::SCALE * PARALLAX_MOUNTAIN_X,
                -cont_view_y * Camera::SCALE * PARALLAX_MOUNTAIN_Y,
                static_cast<float>(mountain_w), static_cast<float>(mountain_h)
            };
            SDL_RenderCopyF(renderer, backdrop_mountains, nullptr, &dst);
        }

        // V4's props. Drawn before the cell texture on purpose - see the
        // Prop comment above main() - so a trunk that overlaps authored
        // terrain gets buried by it with no depth test and no new code path,
        // exactly the way the cell texture already occludes the backdrop
        // wherever a cell is not Empty.
        for (const Prop& prop : props) {
            if (!prop.texture) continue;
            const SDL_FRect dst{
                camera.world_to_screen_x(prop.anchor_x - prop.w / 2.0f),
                camera.world_to_screen_y(prop.anchor_y - static_cast<float>(prop.h)),
                static_cast<float>(camera.scale_length(prop.w)),
                static_cast<float>(camera.scale_length(prop.h))
            };
            SDL_RenderCopyF(renderer, prop.texture, nullptr, &dst);
        }

        // Drawn shifted by the camera's sub-cell remainder, which is the half of
        // A1 that smoothing the player alone would not have fixed: the view is
        // unclamped wherever the player usually is, so the player sits near
        // screen centre and it is the *world* that scrolls. In whole cells that
        // is a 4-pixel jerk of everything on screen at once.
        const SDL_FRect world_dst{
            -camera.frac_x() * Camera::SCALE,
            -camera.frac_y() * Camera::SCALE,
            static_cast<float>(mode.padded_w() * Camera::SCALE),
            static_cast<float>(mode.padded_h() * Camera::SCALE)
        };
        SDL_RenderCopyF(renderer, targets.cells, nullptr, &world_dst);

        // The player is not a cell, so it is not in the pixel buffer either -
        // it is drawn on top of the world as its own sprite. Float rect and
        // float position: rounding either one here is the defect coming back.
        //
        // Positioned by subtracting the offsets from the *box's* corner, which
        // is what "anchored to the box's bottom-centre" works out to - the
        // sprite's baseline lands on the box's baseline and its extra width is
        // split evenly either side.
        const SDL_FRect body{
            camera.world_to_screen_x(draw_player_x - player_sprite::OFFSET_X),
            camera.world_to_screen_y(draw_player_y - player_sprite::OFFSET_Y),
            static_cast<float>(camera.scale_length(player_sprite::FRAME_W)),
            static_cast<float>(camera.scale_length(player_sprite::FRAME_H))
        };
        if (player_tex) {
            const SDL_RendererFlip flip =
                facing_left ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

            // The source rect is the whole of "this is a sheet rather than a
            // sprite" on this side of the boundary. Which cell it names is
            // decided in render/player_anim.cpp, which is SDL-free and tested.
            const SDL_Rect src{
                anim_state.sheet_col() * player_sprite::FRAME_W,
                anim_state.sheet_row() * player_sprite::FRAME_H,
                player_sprite::FRAME_W, player_sprite::FRAME_H
            };
            SDL_RenderCopyExF(renderer, player_tex, &src, &body, 0.0, nullptr, flip);
        } else {
            // The pre-V3 rectangle, kept as the fallback rather than deleted.
            // A missing asset should degrade to a visible player, not to an
            // invisible one - load_art_texture already printed why it failed,
            // and a game you can still move around in is a better diagnostic
            // than a world with nothing in it.
            SDL_SetRenderDrawColor(renderer, 235, 235, 245, 255);
            const SDL_FRect box{
                camera.world_to_screen_x(draw_player_x),
                camera.world_to_screen_y(draw_player_y),
                static_cast<float>(camera.scale_length(Player::WIDTH)),
                static_cast<float>(camera.scale_length(Player::HEIGHT))
            };
            SDL_RenderFillRectF(renderer, &box);
        }

        // V7's one extra RenderCopy - the whole cost of the feature on the GPU
        // side, which is what the architecture note budgeted.
        //
        // **Drawn after the world and after the player, and before the reticle
        // and HUD.** Everything in the world is a surface that light lands on,
        // including the player, who otherwise stays flatly, evenly lit while
        // standing inside a fire. Everything after it is UI, which is not in the
        // world and must not be tinted by it - a reticle that goes orange near a
        // flame is the exact defect B1 was about.
        //
        // The destination is the *block* extent, not the padded cell extent, and
        // that is what aligns the stretch. `cols()*BLOCK` is the padded width
        // rounded up to a whole block, so each texel's centre lands on the centre
        // of the four-by-four cells it was computed from. Sized to the padded
        // extent instead, every texel would sit up to half a block off and the
        // glow would trail behind the flame that cast it.
        if (targets.light.any_light()) {
            const SDL_FRect light_dst{
                -camera.frac_x() * Camera::SCALE,
                -camera.frac_y() * Camera::SCALE,
                static_cast<float>(targets.light.cols() * LightField::BLOCK * Camera::SCALE),
                static_cast<float>(targets.light.rows() * LightField::BLOCK * Camera::SCALE)
            };
            SDL_RenderCopyF(renderer, targets.light_texture, nullptr, &light_dst);
        }

        // The reticle (V10/B1). Three things it has to do, and the old one-cell
        // filled square did none of them well enough to survive a playtest.
        //
        // **It is drawn in screen pixels, not cells.** A cell is 4 screen pixels
        // at the current scale, so a cell-sized marker is four pixels across and
        // any shape drawn inside one is not a shape. Screen-space also means it
        // keeps its size and legibility if the scale is ever changed.
        //
        // **It is not orange any more.** The defect behind this item was that the
        // marker sat in the same colour family as Fire and disappeared against
        // the one thing you most want to aim at. Colour now carries range only -
        // full white in reach, dim white out of it - and the outline below is
        // what carries legibility, so neither job depends on the background.
        //
        // **Each arm is drawn over a dark outline.** A white reticle would vanish
        // against snow and steam exactly as the orange one vanished against fire;
        // an outlined one cannot vanish against anything, because whatever the
        // background is, one of the two contrasts with it.
        const int dx_cells = gridX - run.player.center_x();
        const int dy_cells = gridY - run.player.center_y();
        const bool in_range = (dx_cells * dx_cells + dy_cells * dy_cells) <= DigTool::RANGE * DigTool::RANGE;

        // A gap wider than the arms are long would read as four separate marks
        // rather than one reticle; the point of leaving the centre open is that
        // the cell being aimed at stays visible, so the gap is sized to the cell.
        constexpr int TICK_GAP = 4;
        constexpr int TICK_LEN = 7;
        constexpr int TICK_THICK = 2;

        const SDL_Rect arms[4] = {
            { mouseX - TICK_THICK / 2, mouseY - TICK_GAP - TICK_LEN, TICK_THICK, TICK_LEN }, // N
            { mouseX - TICK_THICK / 2, mouseY + TICK_GAP,            TICK_THICK, TICK_LEN }, // S
            { mouseX - TICK_GAP - TICK_LEN, mouseY - TICK_THICK / 2, TICK_LEN, TICK_THICK }, // W
            { mouseX + TICK_GAP,            mouseY - TICK_THICK / 2, TICK_LEN, TICK_THICK }, // E
        };

        // Only while the pointer is actually over this window. `SDL_GetMouseState`
        // keeps reporting the last position inside the window after the mouse
        // leaves, so without this the reticle sticks to the edge and reads as a
        // frozen UI element rather than as a cursor that has gone elsewhere.
        if (SDL_GetMouseFocus() == window) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, in_range ? 170 : 120);
            for (const SDL_Rect& arm : arms) {
                const SDL_Rect outline{arm.x - 1, arm.y - 1, arm.w + 2, arm.h + 2};
                SDL_RenderFillRect(renderer, &outline);
            }

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, in_range ? 255 : 115);
            for (const SDL_Rect& arm : arms) SDL_RenderFillRect(renderer, &arm);

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        // The UI layer decision (ENGINEERING_NOTES.md): drawn here, in the
        // game window, rather than left to the OS title bar - a solid backing
        // rect first so the text stays readable over whatever the simulation
        // is doing underneath it.
        hud_text = "FPS:" + std::to_string(fps_display) +
                   " BRUSH:" + material_of(current_brush).name + "(" + std::to_string(brush_size) + ")" +
                   " CHUNKS:" + std::to_string(run.grid.active_chunk_count());

        // Scaled with the window rather than fixed at 2 (see
        // DisplayMode::ui_scale): a HUD that keeps its pixel size keeps
        // shrinking as a fraction of the screen every time a wider mode is
        // added, and this one is an instrument - defect A2 is about it being
        // trusted to answer "what did that key just do".
        const int hud_scale = mode.ui_scale();
        const int hud_x = 8, hud_y = 8;
        const SDL_Rect hud_backing{
            hud_x - 4, hud_y - 4,
            ui::text_width(hud_text, hud_scale) + 8, ui::GLYPH_HEIGHT * hud_scale + 8
        };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &hud_backing);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        ui::draw_text(renderer, hud_x, hud_y, hud_scale, hud_text, 0xFFE0E0E0);

        // --- the material hotbar (V10) ---
        //
        // The selected slot is looked up rather than stored, so `current_brush`
        // stays the single fact about what is selected. A second index kept
        // beside it is exactly how a highlight ends up on the wrong box after
        // some later code path sets the brush without going through a key.
        int hotbar_selected = -1;
        for (int i = 0; i < ui::HOTBAR_COUNT; ++i) {
            if (ui::HOTBAR[i].type == current_brush) hotbar_selected = i;
        }
        ui::draw_hotbar(renderer, mode.window_w, mode.window_h, mode.ui_scale(), hotbar_selected);

        // --- the settings menu ---
        //
        // Drawn last, over a dimming wash rather than over a solid panel, so
        // the world stays visible behind it. That is not decoration: the only
        // way to judge "is this the resolution I want" is to see how much of
        // the world it shows, and a menu that hides the world hides the thing
        // the setting changes.
        if (screen == Screen::Settings) {
            const int scale = mode.ui_scale();
            const int line_h = (ui::GLYPH_HEIGHT + 3) * scale;

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
            const SDL_Rect wash{0, 0, mode.window_w, mode.window_h};
            SDL_RenderFillRect(renderer, &wash);

            const int menu_x = mode.window_w / 2 - 30 * ui::GLYPH_WIDTH * scale / 2;
            int y = mode.window_h / 2 - (MENU_QUIT + 4) * line_h / 2;

            ui::draw_text(renderer, menu_x, y, scale, "SETTINGS", 0xFFFFFFFF);
            y += line_h * 2;

            for (int i = 0; i <= MENU_QUIT; ++i) {
                const bool selected = (i == menu_cursor);
                std::string label;
                uint32_t colour;

                if (i == MENU_QUIT) {
                    label = "QUIT";
                    colour = selected ? 0xFFFFFFFF : 0xFFA0A0A0;
                } else {
                    const DisplayMode& m = DISPLAY_MODES[i];
                    label = std::to_string(m.window_w) + "X" + std::to_string(m.window_h) +
                            "  " + std::to_string(m.viewport_w()) + "X" +
                            std::to_string(m.viewport_h()) + " CELLS";
                    if (i == mode_index) label += "  *";
                    // Unavailable modes are shown greyed rather than hidden.
                    // A menu that silently omits a mode on one machine and
                    // lists it on another is a menu a player cannot learn.
                    if (!mode_available[i]) {
                        label += "  (TOO LARGE)";
                        colour = selected ? 0xFF806060 : 0xFF605050;
                    } else {
                        colour = selected ? 0xFFFFFFFF : 0xFFA0A0A0;
                    }
                }

                if (selected) {
                    ui::draw_text(renderer, menu_x - 2 * ui::GLYPH_WIDTH * scale, y, scale, ">",
                                  colour);
                }
                ui::draw_text(renderer, menu_x, y, scale, label, colour);
                y += line_h;
            }

            y += line_h;
            ui::draw_text(renderer, menu_x, y, scale,
                          "ARROWS  ENTER  ESC RESUMES", 0xFF808080);
            if (menu_notice_timer > 0.0 && !menu_notice.empty()) {
                y += line_h;
                ui::draw_text(renderer, menu_x, y, scale, menu_notice, 0xFFFFC080);
            }
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

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
    if (backdrop_mountains) SDL_DestroyTexture(backdrop_mountains);
    if (backdrop_sky) SDL_DestroyTexture(backdrop_sky);
    SDL_DestroyTexture(targets.light_texture);
    SDL_DestroyTexture(targets.cells);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
