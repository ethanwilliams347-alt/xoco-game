"""Converts the authored Background_1 layer set into engine BMPs in assets/bg1/.

**A thin wrapper over tools/png_to_bmp.py and not a second converter.** It calls
that module's `convert` for every layer rather than reimplementing the alpha
rule, because the COLOR_KEY nudge in there (an opaque pixel that happens to be
exactly the transparency colour gets moved by 1 so it does not vanish) is the
kind of detail a second copy loses silently. What is here that is not there is
the *list* - which nine files, in which order, and which of them is opaque.

**The `keyed` column is measured, not declared.** The art's own README calls
layers 8 and 9 "Receding Plane" and "Fully Opaque"; measured against the PNGs on
2026-08-25, layer 9 is 100% opaque and layer 8 is 56%, so the plane keys like
every silhouette above it and only the sky does not. Written the other way the
plane would have been stamped as a solid rectangle over the whole sky band -
which is V19's mountains-are-missing bug arriving a second time, from the asset
side instead of the horizon side.

The sky must stay opaque for a different reason: it is the backmost layer and
there is nothing behind it but the clear colour, so a keyed sky is a hole.

It also writes the scene's material and albedo maps - see `build_scene_maps`
below for why a backdrop set needs a floor at all.

    python tools/convert_background_layers.py
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from pixel_art import write_bmp
from gemini_to_player_frame import read_png
from png_to_bmp import convert

# (source, destination, keys transparency). Listed front to back, the order the
# art's README uses; the *draw* order is main.cpp's and is the reverse.
LAYERS = [
    ("01_foreground_rocks.png",  "bg1_01_fg_rocks.bmp",      True),
    ("02_hills_near.png",        "bg1_02_hills_near.bmp",    True),
    ("03_hills_midnear.png",     "bg1_03_hills_midnear.bmp", True),
    ("04_hills_mid.png",         "bg1_04_hills_mid.bmp",     True),
    ("05_hills_midfar.png",      "bg1_05_hills_midfar.bmp",  True),
    ("06_hills_far.png",         "bg1_06_hills_far.bmp",     True),
    ("07_distant_mountains.png", "bg1_07_mountains.bmp",     True),
    ("08_ground_plane.png",      "bg1_08_ground.bmp",        True),
    ("09_sky.png",               "bg1_09_sky.bmp",           False),
]

SRC_DIR = os.path.join("art_src", "Background_1")
DST_DIR = os.path.join("assets", "bg1")

# The set's native size. Every layer is this, and a layer that is not is a
# problem: main.cpp scales the whole stack by one integer factor, so a differing
# source size would silently land at a different on-screen scale than its
# neighbours - a depth ladder whose rungs are different lengths.
NATIVE_W, NATIVE_H = 344, 144


# --- V28: the scene maps -----------------------------------------------------
#
# `bg1` is not only a backdrop. It is a world 344x144 *cells* - one cell per art
# pixel - and the player has to stand somewhere in it. A `floor` spawn would put
# the body on the world's bottom border, which the camera's vertical clamp then
# pins to the very bottom edge of the window at every resolution; the reference
# has the feet well above that. So the scene gets a material map with a real
# floor in it, and `spawn terrain` finds it by scanning.
#
# **FLOOR_ROW is measured, not derived, and the distinction matters.** It is read
# off the tester's reference IMG_0195.PNG: the player's sprite occupies rows
# 106..131 there, so the feet plane is row 132. What *is* derived is the
# consequence - `player_sprite.h` has OFFSET_Y == FRAME_H - Player::HEIGHT, so a
# body whose sprite bottom is 132 has pos_y 112 and a collision box of rows
# 112..131. That the engine's own terrain scan lands on exactly the reference's
# number is the check that this row is right; see section 1.4 of
# gemini_plans/bg1_world_scale_and_geometry_plan.md.
FLOOR_ROW = 132

# From src/scene/legend.h, which is frozen - these are looked up there, never
# invented. Empty is the artist's "nothing here"; Wall is the one inert solid.
LEGEND_EMPTY = (0x00, 0x00, 0x00)
LEGEND_WALL = (0x88, 0x88, 0x88)

# Back to front. **This is the numeric-descending order of the filenames**, which
# is what the art README's front-to-back table says read the other way round, and
# it is confirmed by pixels: compositing in this order reproduces IMG_0195.PNG
# exactly everywhere the player is not. Any other order does not.
DRAW_ORDER = [
    "09_sky.png",
    "08_ground_plane.png",
    "07_distant_mountains.png",
    "06_hills_far.png",
    "05_hills_midfar.png",
    "04_hills_mid.png",
    "03_hills_midnear.png",
    "02_hills_near.png",
    "01_foreground_rocks.png",
]

MATERIAL_PATH = os.path.join("assets", "bg1_material.bmp")
ALBEDO_PATH = os.path.join("assets", "bg1_albedo.bmp")

# How opaque a source pixel has to be to count as part of the composite. The same
# threshold png_to_bmp uses for the colour key, imported rather than repeated so
# the albedo cannot disagree with the layer BMPs about what is solid.
from png_to_bmp import ALPHA_THRESHOLD


def build_scene_maps():
    """Writes the material and albedo BMPs that give `bg1` a floor.

    The albedo is the whole frame rather than only the floor band. Only rows
    FLOOR_ROW.. are ever read - an Empty cell places nothing, so its albedo pixel
    is never looked at - but `src/scene/bmp.cpp` requires the two maps to be the
    same size, and a 344x144 image of mostly-unread pixels is 148 KB. Cropping it
    would save nothing and would put a second geometry in the pipeline.
    """
    composite = [[(0, 0, 0)] * NATIVE_W for _ in range(NATIVE_H)]
    for name in DRAW_ORDER:
        path = os.path.join(SRC_DIR, name)
        if not os.path.exists(path):
            print(f"error: {path} not found")
            return 1
        width, height, pixels, alpha = read_png(path)
        if (width, height) != (NATIVE_W, NATIVE_H):
            print(f"error: {name} is {width}x{height}, expected {NATIVE_W}x{NATIVE_H}")
            return 1
        for y in range(NATIVE_H):
            row = y * NATIVE_W
            for x in range(NATIVE_W):
                i = row + x
                if alpha is None or alpha[i] >= ALPHA_THRESHOLD:
                    composite[y][x] = pixels[i]

    material = []
    albedo = []
    solid = 0
    for y in range(NATIVE_H):
        for x in range(NATIVE_W):
            if y >= FLOOR_ROW:
                material.append(LEGEND_WALL)
                solid += 1
            else:
                material.append(LEGEND_EMPTY)
            albedo.append(composite[y][x])

    write_bmp(MATERIAL_PATH, NATIVE_W, NATIVE_H, material)
    write_bmp(ALBEDO_PATH, NATIVE_W, NATIVE_H, albedo)
    print(f"scene maps -> {MATERIAL_PATH}, {ALBEDO_PATH}  "
          f"({NATIVE_W}x{NATIVE_H}, floor at row {FLOOR_ROW}, {solid} solid cells)")
    return 0


def main():
    if not os.path.isdir(SRC_DIR):
        print(f"error: {SRC_DIR} not found - run this from the repo root (code/)")
        return 1

    os.makedirs(DST_DIR, exist_ok=True)
    failures = 0
    for src_name, dst_name, keyed in LAYERS:
        src_path = os.path.join(SRC_DIR, src_name)
        dst_path = os.path.join(DST_DIR, dst_name)
        if not os.path.exists(src_path):
            print(f"error: {src_path} not found")
            failures += 1
            continue
        width, height, pixels, alpha = read_png(src_path)
        if (width, height) != (NATIVE_W, NATIVE_H):
            print(f"error: {src_name} is {width}x{height}, expected "
                  f"{NATIVE_W}x{NATIVE_H} - the stack is scaled as one set")
            failures += 1
            continue
        out = convert(width, height, pixels, alpha if keyed else None)
        write_bmp(dst_path, width, height, out)
        print(f"{src_name} -> {dst_path}  ({width}x{height}, "
              f"{'keyed' if keyed else 'opaque'})")

    if failures:
        print(f"{failures} layer(s) failed")
        return 1

    return build_scene_maps()


if __name__ == "__main__":
    sys.exit(main())
