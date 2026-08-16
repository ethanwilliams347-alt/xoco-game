"""The pixel-art framework: one shared palette, one BMP codec, one dithering
rule, one rim-light helper. Every generator in tools/ imports this rather
than rolling its own — that is the entire mechanism behind V6's "shared
palette both sides index" and the reason a validator is possible at all.

**Status: this is a working set, not a locked one.** V6 called it locked and
that claim has gone false, so it is corrected here rather than quietly
rewritten. Two halves aged differently:

  - The backdrop, tree and terrain groups are real and in force. Every pixel
    of that art is *generated* from these names by the tools below, so it
    conforms by construction and cannot drift.
  - The char_* group is a placeholder that no shipped art has ever used. Not
    one pixel of any of the four player sheets in assets/ is on-palette, and
    the current fly poses are warm olive where char_* is cool blue-grey. The
    art moved and the constant did not.

So nothing is *enforced* against hand-drawn art right now, on purpose: the
style is still being found and there is only one entity to find it from.
Draw freely. See ENGINEERING_NOTES.md, "The palette is deferred, not lost",
for what ends that and what to keep while it lasts.

Written against nothing but the standard library, on purpose: this project
has no third-party dependencies by policy (BMP over PNG, immediate-mode over
Dear ImGui - see ENGINEERING_NOTES.md), and an
authoring tool is a bad reason to be the first exception.

PALETTE is the set notes/art_direction.txt (V5) describes and reasons about;
this file is where the actual values live. Every colour below is an
original value chosen to match the *character* a reference sample measured
(dark, narrow value range, cool sky / warm ground) - see the dated entry in
notes/reference_observations.txt - not a colour copied out of that
reference. resources/ is gitignored specifically so that distinction stays
real: nothing in this project's asset pipeline ever reads a pixel from
footage that is not ours.

Adding a colour: add it here with a one-line reason, regenerate
assets/palette.gpl (tools/export_palette_gpl.py), and every generator picks
it up. Removing or changing one in the *generated* groups still invalidates
every BMP built from it, the same way changing a scene/legend.h value
invalidates every material map - treat those with that weight. The char_*
group carries no such cost today, because nothing was ever built from it.
"""

import struct

# --- the palette ------------------------------------------------------------
#
# Grouped by layer (notes/art_direction.txt's four-layer model), not
# alphabetically - the groups are the thing worth reading, the names inside
# them are arbitrary. Backdrop stays cool with zero warm colour in it, which
# is what keeps it reading as *behind* everything drawn over it. The terrain
# fill is warm near-black rather than cool near-black so unlit dirt still
# reads as earth instead of void; the rim is the one bright, warm accent in
# the whole terrain layer.
PALETTE = {
    # sky (V8, layer 1) - cool, deep, no warm colour anywhere in this group
    'sky_deep':     (0x14, 0x10, 0x22),
    'sky_horizon':  (0x28, 0x1F, 0x3E),
    'star':         (0xC7, 0xC2, 0xDE),

    # mountains (V8, layer 2) - one flat silhouette tone plus a faint peak rim
    'mountain':     (0x20, 0x19, 0x30),
    'mountain_rim': (0x30, 0x27, 0x48),

    # the ground plane (V19, drawn behind the world) - the one band in the
    # frame that recedes *within itself*, so it is authored as a ramp rather
    # than as a tone. `ground_far` is its horizon edge and `ground_near` its
    # near edge; the tile dithers between them top to bottom and the strip
    # loop in render/frame.cpp turns that into distance.
    #
    # **The ramp is in the art and the level is in the grade, and that split is
    # deliberate.** A per-layer Grade multiplies uniformly, so it cannot make a
    # surface brighten toward the viewer - only the tile can. What the grade
    # then does is place the whole band on the value ladder, which is the knob
    # V11 argues a direction change should be. Ratio near/far here is 1.83
    # against the reference plane's 1.78 (notes/reference_observations.txt
    # entry 7, 77.5 -> 138.2).
    'ground_far':   (0x18, 0x14, 0x26),
    'ground_near':  (0x2C, 0x25, 0x40),
    'ground_mark':  (0x3C, 0x34, 0x52),

    # trees (V4 props, layer 3) - desaturated green, warmer than the sky,
    # cooler than the terrain rim, so it sits legibly between the two
    'tree_shadow':  (0x18, 0x20, 0x16),
    'tree_mid':     (0x2A, 0x38, 0x24),
    'tree_lit':     (0x47, 0x59, 0x39),
    'trunk':        (0x22, 0x1A, 0x13),

    # terrain (simulated grid, layer 4) - warm near-black fill per material,
    # one shared rim colour, one dithered hand-off band
    'dirt_fill':    (0x1B, 0x16, 0x11),
    'dirt_mid':     (0x29, 0x20, 0x16),
    'wall_fill':    (0x1B, 0x16, 0x11),  # same family as dirt: reads as rock
    'sand_fill':    (0x24, 0x1C, 0x10),
    'wood_fill':    (0x17, 0x12, 0x0D),
    'rim_grass':    (0x69, 0x78, 0x3A),  # the one bright accent in the layer

    # player (V3, drawn between the props and the terrain's own layer - it is
    # not a cell and not a prop, it is the one sprite that moves under input).
    #
    # The separation problem this group solves: every other group above is
    # either cool-and-dark (sky, mountains) or warm-and-dark (terrain), and the
    # trees sit between them in green. A character painted in any of those
    # families disappears into whichever one it happens to be standing against.
    # So the robe is *cool blue-grey* - the one hue family the terrain layer
    # never uses - which reads against warm dirt and against green foliage
    # without being brighter than either. Value range stays inside the locked
    # set's: char_light is 0x3F-0x5E, below tree_lit's 0x59 top end.
    'char_base':    (0x1C, 0x20, 0x29),  # robe shadow; the darkest character tone
    'char_mid':     (0x2A, 0x32, 0x40),  # main robe tone, cool enough to clear the foliage green
    'char_light':   (0x3F, 0x4A, 0x5E),  # shoulder and hood highlight - the silhouette's edge
    'char_belt':    (0x4A, 0x3B, 0x2A),  # rope belt and boots; warm brown, ties the figure to the ground layer
    'char_mask':    (0x11, 0x11, 0x11),  # the mask's interior void, darker than any terrain fill
    'char_accent':  (0x94, 0x51, 0x28),  # dull copper on the mask only - see the note below

    # water is the deliberate exception (notes/art_direction.txt): it keeps
    # more saturation than anything else in the terrain layer because it has
    # to read as water up close, not just in silhouette
    'water_fill':   (0x1A, 0x29, 0x32),
    'water_rim':    (0x2E, 0x49, 0x55),
}

# char_accent is the brightest value in this entire palette - 0x94 against
# rim_grass's 0x69, which was previously "the one bright accent in the whole
# terrain layer". That is deliberate and it is also the one entry here most
# likely to need revisiting, so the tension is written down rather than left
# to be rediscovered:
#
#   - It is six pixels. The whole justification is that it is the *only*
#     saturated thing on a figure otherwise painted in the locked dark range,
#     so it functions as a fixation point rather than as a light source. At a
#     larger area this value would be wrong.
#   - It sits in the warm-orange family that notes/art_direction.txt reserves
#     for Fire. Six pixels on a moving figure is not going to be mistaken for
#     a flame, but if the character ever gains more copper - or if V7's
#     emissive pass makes warm pixels glow - this is where that reads as fire
#     first. Check it against a burning scene before adding any more.
#
# Reserved marker for sprite transparency (props, mountains) - SDL_SetColorKey
# treats this exact colour as "no pixel here". Magenta, for the same reason
# scene/legend.h picked it for its own unused slots: nobody paints real art
# in pure magenta by accident, so a leak is obvious on sight rather than a
# colour that could plausibly be intentional.
COLOR_KEY = (0xFF, 0x00, 0xFF)


def color_of(name):
    """Palette lookup that fails loudly. A typo'd name silently falling back
    to some default would be exactly the kind of off-palette pixel V6 exists
    to make impossible."""
    if name not in PALETTE:
        raise KeyError(f"'{name}' is not in PALETTE (tools/pixel_art.py) - "
                        f"add it there first - a generator names colours, "
                        f"it does not hardcode them")
    return PALETTE[name]


# --- ordered dithering --------------------------------------------------
#
# V2 already found the negative case: random per-cell jitter fights
# hand-placed dither instead of adding to it, which is why colour_jitter
# stays out of everything this pipeline authors. This is the positive
# case - a transition between two flat tones is stepped through a Bayer
# matrix threshold, never smoothly interpolated and never a hard edge with
# nothing between. See notes/art_direction.txt's "the dithering rule".
_BAYER_4X4 = (
    (0,  8,  2, 10),
    (12, 4, 14,  6),
    (3, 11,  1,  9),
    (15, 7, 13,  5),
)


def bayer_threshold(x, y):
    """0..1, deterministic in (x, y). Used as a per-pixel coin flip whose
    outcomes fall into the classic 4x4 ordered-dither grid instead of static."""
    return (_BAYER_4X4[y % 4][x % 4] + 0.5) / 16.0


def dither_mix(x, y, color_a, color_b, t):
    """Ordered-dithered pick between two flat colours, not a blend. t is the
    fraction of color_b: t=0 is all color_a, t=1 is all color_b, and values
    between land a proportional share of color_b's pixels in the Bayer
    pattern rather than averaging the two into a third colour no editor's
    palette would contain."""
    return color_b if bayer_threshold(x, y) < t else color_a


# --- rim light -----------------------------------------------------------
#
# The technique the reference sample argued for (notes/reference_observations
# .txt): a filled region reads as shadowed mass with almost no internal
# texture, and the one to two cells facing open air carry a bright, warm
# highlight. That is authored per scene, not computed by the engine -
# MATERIALS' colours are untouched, same as V6 requires - so this is a
# pre-process over the (material, albedo) buffers a scene generator already
# builds, not new engine code.
def apply_rim_light(mat, alb, width, height, empty_marker, rim_color,
                     rim_depth=2):
    """Returns a new albedo buffer. For every filled cell (mat != empty_marker)
    whose cell directly above is empty or off-grid, paints it a rim colour.
    The next (rim_depth - 1) cells downward are ordered-dithered from the rim
    colour toward whatever albedo already held there, so the hand-off is a
    dithered band rather than a hard line - the one dithering case notes/
    art_direction.txt calls out by name.

    `rim_color` is either a flat (r, g, b) applied everywhere, or a callable
    `(x, y) -> (r, g, b) | None` so a caller with more than one material in
    the buffer can give water a different rim than dirt - returning None
    skips that cell (e.g. a material that should stay unlit at its surface).

    Deliberately shape-agnostic otherwise: it only asks "is the cell above me
    filled", so it rims a slope, a ceiling-less pit wall or a standalone
    platform identically, with no per-region special-casing in the caller.
    """
    color_fn = rim_color if callable(rim_color) else (lambda x, y: rim_color)

    def filled(x, y):
        if not (0 <= x < width and 0 <= y < height):
            return False
        return mat[y * width + x] != empty_marker

    out = list(alb)
    for y in range(height):
        for x in range(width):
            if not filled(x, y) or filled(x, y - 1):
                continue
            top_color = color_fn(x, y)
            if top_color is None:
                continue
            for d in range(rim_depth):
                yy = y + d
                if not filled(x, yy):
                    break
                idx = yy * width + x
                if d == 0:
                    out[idx] = top_color
                else:
                    out[idx] = dither_mix(x, yy, alb[idx], top_color,
                                           1.0 - d / rim_depth)
    return out


# --- BMP codec -------------------------------------------------------------
#
# 24-bit uncompressed only, matching every BMP this project already writes
# (generate_test_scene.py) and reads (main.cpp's load_scene_from_bmp). No
# alpha channel - SDL_LoadBMP does not give one back reliably across
# platforms, which is why transparency here is COLOR_KEY plus
# SDL_SetColorKey on the C++ side rather than an alpha byte.
def write_bmp(filename, width, height, pixels_rgb):
    """pixels_rgb: row-major list of (r, g, b) tuples, top row first."""
    row_size = (width * 3 + 3) & ~3
    pixel_data_size = row_size * height
    file_size = 54 + pixel_data_size

    with open(filename, 'wb') as f:
        f.write(b'BM')
        f.write(struct.pack('<I', file_size))
        f.write(struct.pack('<H', 0))
        f.write(struct.pack('<H', 0))
        f.write(struct.pack('<I', 54))

        f.write(struct.pack('<I', 40))
        f.write(struct.pack('<i', width))
        f.write(struct.pack('<i', -height))  # negative: top-down, matches the reader below
        f.write(struct.pack('<H', 1))
        f.write(struct.pack('<H', 24))
        f.write(struct.pack('<I', 0))
        f.write(struct.pack('<I', pixel_data_size))
        f.write(struct.pack('<i', 2835))
        f.write(struct.pack('<i', 2835))
        f.write(struct.pack('<I', 0))
        f.write(struct.pack('<I', 0))

        for y in range(height):
            for x in range(width):
                r, g, b = pixels_rgb[y * width + x]
                f.write(struct.pack('<BBB', b, g, r))
            f.write(b'\x00' * (row_size - width * 3))


def read_bmp(filename):
    """Returns (width, height, pixels_rgb) for a 24-bit uncompressed BMP,
    top row first regardless of whether the file is stored top-down or
    bottom-up. Exists for tools/validate_palette.py - reading a format this
    project already writes with the standard library rather than reaching
    for Pillow, same reasoning as the rest of this file."""
    with open(filename, 'rb') as f:
        data = f.read()

    if data[0:2] != b'BM':
        raise ValueError(f"{filename}: not a BMP")

    pixel_offset = struct.unpack_from('<I', data, 10)[0]
    header_size = struct.unpack_from('<I', data, 14)[0]
    width = struct.unpack_from('<i', data, 18)[0]
    height_raw = struct.unpack_from('<i', data, 22)[0]
    bpp = struct.unpack_from('<H', data, 28)[0]
    compression = struct.unpack_from('<I', data, 30)[0]

    if bpp != 24 or compression != 0:
        raise ValueError(f"{filename}: only 24-bit uncompressed BMP is "
                          f"supported (got {bpp}-bit, compression {compression})")

    top_down = height_raw < 0
    height = abs(height_raw)
    row_size = (width * 3 + 3) & ~3

    pixels = [None] * (width * height)
    for row in range(height):
        file_row = row if top_down else (height - 1 - row)
        offset = pixel_offset + file_row * row_size
        for x in range(width):
            b, g, r = data[offset + x * 3:offset + x * 3 + 3]
            pixels[row * width + x] = (r, g, b)

    return width, height, pixels
