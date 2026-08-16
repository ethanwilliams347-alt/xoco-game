"""Generates the backdrop parallax layers (V8): sky+stars and a mountain
silhouette. Layers 1 and 2 of notes/art_direction.txt's depth-band model
(section 6).

**This file is the single source of the parallax factors as of V11
(2026-08-16), and `--header` is how the C++ side gets them.** It used to say
that the numbers below "MUST stay in sync with the PARALLAX_* constants
main.cpp draws these layers with", that the coupling was "written down here
rather than computed automatically because these are two different languages
and this project has no build step that could enforce it either way", and
that you should grep before changing a number on either side. Every one of
those sentences was true and none of them was enforcement - the failure they
described is a seam at the pan limit, which is invisible until somebody pans
to the edge of the world. There is now exactly one copy:

    python tools/generate_backdrop.py --header

writes src/render/backdrop_layers.h from the table below, the same way
tools/player_sheet.py --header writes src/render/player_sprite.h. **Do not
edit that header** - it is overwritten work. The header carries each layer's
generated size as well as its factors, so main.cpp can compare what it
actually loaded against what this script would have produced and say so at
startup, which is the seam becoming a printed line instead of a pixel nobody
reaches.

**One asset set covers all three display modes, and which mode sizes it is
the counter-intuitive part.** The window is now switchable at runtime
(1920x1080, 2560x1440, 3440x1440) at a fixed 4 screen pixels per cell, so a
wider window sees *more* cells and therefore has *less* world left to pan
across. The largest pan range - the case a layer can run out of image in -
belongs to the *smallest* window, and the widest window needs the largest
window-sized base. So the layers are sized from the smallest viewport and
the largest window dimensions, taken independently, and every other mode
uses a sub-rectangle of the same file.

**A consequence of that formula worth knowing before adding a band: the
nearer the band, the bigger its file.** Size grows with the parallax factor,
so a near band is the most expensive layer in the stack and not the cheapest -
a mid-ground at 0.40 would have been 32 MB, more than the sky and mountains
together. `--sizes` prints the table. V16's wrapping layers retire the
relationship entirely.

Run from the repo root:
    python tools/generate_backdrop.py            # the shipped layers
    python tools/generate_backdrop.py --header   # regenerate the C++ header
    python tools/generate_backdrop.py --sizes    # what each layer would cost
"""
import random
import sys
from pixel_art import PALETTE, COLOR_KEY, dither_mix, write_bmp

# --- must match main.cpp -----------------------------------------------
GRID_WIDTH, GRID_HEIGHT = 1920, 1080
SCALE = 4                                    # Camera::SCALE

# main.cpp's DisplayMode table, in window pixels.
MODES = [(1920, 1080), (2560, 1440), (3440, 1440)]

PARALLAX_SKY = (0.04, 0.02)                 # (x, y) factors
PARALLAX_MOUNTAIN = (0.15, 0.06)

# **There was briefly a third band here and there is a reason there is not
# now.** V11 added a mid-ground layer at 0.40/0.16 on 2026-08-16, between the
# mountains and the world, because notes/reference_observations.txt entry 4
# found that band carrying most of the depth in five of eight reference frames.
# Entry 4 also wrote down its own disproof condition - a *simulated* world might
# already be filling that band with terrain, where a hand-painted one cannot -
# and the played-frame check it asked for came back the same day saying exactly
# that. The band came out.
#
# Do not re-add one without checking the same way. The reopen trigger is a
# location whose terrain does *not* fill the band: a flatter scene with a lower
# horizon than F4's snowbank, or a zoomed-out camera once Camera::SCALE is a
# runtime value. Neither exists today. The full argument is at V11 in ROADMAP.md
# and at the layer table in src/render/frame.cpp.
# -------------------------------------------------------------------------

# Padded viewport, in cells: VIEWPORT + 1 on each axis, matching main.cpp's
# PADDED_WIDTH/HEIGHT. The smallest of each is what maximises the pan range.
MIN_VIEWPORT_W = min(w // SCALE for w, _ in MODES) + 1
MIN_VIEWPORT_H = min(h // SCALE for _, h in MODES) + 1

# The largest window is what sets the base size every layer must at least
# cover; the smallest viewport is what sets how far it then has to scroll.
WINDOW_W = max(w for w, _ in MODES)
WINDOW_H = max(h for _, h in MODES)

MAX_SHIFT_X = (GRID_WIDTH - MIN_VIEWPORT_W) * SCALE
MAX_SHIFT_Y = (GRID_HEIGHT - MIN_VIEWPORT_H) * SCALE


def layer_size(factor):
    fx, fy = factor
    # +8px margin beyond the exact shift range so a float rounding error
    # never exposes a one-pixel gap at the pan limit.
    w = WINDOW_W + int(MAX_SHIFT_X * fx) + 8
    h = WINDOW_H + int(MAX_SHIFT_Y * fy) + 8
    return w, h


# --- sky: banded gradient, ordered-dithered transitions, sparse stars ------
def generate_sky():
    w, h = layer_size(PARALLAX_SKY)
    top = PALETTE['sky_deep']
    bottom = PALETTE['sky_horizon']

    # Steps by whole bands (V5's dithering rule: transitions between flat
    # tones, not a per-pixel smooth blend) - dither only at each band
    # boundary, not across the whole gradient.
    bands = 10
    pixels = [None] * (w * h)
    for y in range(h):
        band = min(y * bands // h, bands - 1)
        t_this = band / (bands - 1)
        for x in range(w):
            # Within 2px of the next band's start, dither toward it, so the
            # seam is a stepped hand-off instead of a hard line.
            next_band_start = (band + 1) * h // bands
            if band < bands - 1 and next_band_start - y <= 2:
                t_next = (band + 1) / (bands - 1)
                pixels[y * w + x] = dither_mix(x, y, top, bottom,
                                                t_this + (t_next - t_this) *
                                                (1 - (next_band_start - y) / 3))
            else:
                pixels[y * w + x] = dither_mix(x, y, top, bottom, t_this)

    rng = random.Random(7)
    star = PALETTE['star']
    for _ in range(70):
        sx = rng.randrange(w)
        sy = rng.randrange(int(h * 0.65))  # stars stay above the skyline band
        pixels[sy * w + sx] = star

    write_bmp('assets/backdrop_sky.bmp', w, h, pixels)
    print(f'wrote assets/backdrop_sky.bmp ({w}x{h})')


# --- mountains: jagged silhouette, colour-keyed, drawn onto the sky's tone -
def generate_mountains():
    w, h = layer_size(PARALLAX_MOUNTAIN)
    mountain = PALETTE['mountain']
    rim = PALETTE['mountain_rim']

    # A jagged skyline as a random walk between segment points, the same
    # "seeded RNG, blocky rather than smooth" texture generate_test_scene.py
    # already uses for the snowbank slope - deliberate variation, not noise.
    rng = random.Random(3)
    base_y = int(h * 0.58)
    peak_span = int(h * 0.22)
    segment_w = 48

    points = []
    x = 0
    while x <= w:
        points.append((x, base_y + rng.randint(-peak_span, peak_span // 2)))
        x += segment_w + rng.randint(-10, 10)
    points.append((w, points[-1][1]))

    def skyline_y(px):
        for i in range(len(points) - 1):
            x0, y0 = points[i]
            x1, y1 = points[i + 1]
            if x0 <= px <= x1:
                t = 0 if x1 == x0 else (px - x0) / (x1 - x0)
                return y0 + (y1 - y0) * t
        return points[-1][1]

    pixels = [COLOR_KEY] * (w * h)
    for px in range(w):
        sy = int(round(skyline_y(px)))
        for py in range(max(sy, 0), h):
            pixels[py * w + px] = rim if py < sy + 2 else mountain

    write_bmp('assets/backdrop_mountains.bmp', w, h, pixels)
    print(f'wrote assets/backdrop_mountains.bmp ({w}x{h})')


# --- the generated header --------------------------------------------------
LAYERS = [
    ('SKY', 'backdrop_sky', PARALLAX_SKY),
    ('MOUNTAINS', 'backdrop_mountains', PARALLAX_MOUNTAIN),
]


def print_sizes():
    print(f'{"layer":<12}{"factor":<14}{"size":<14}{"BMP":>10}')
    for name, _key, factor in LAYERS:
        w, h = layer_size(factor)
        mb = w * h * 3 / 1024 / 1024
        print(f'{name.lower():<12}{str(factor):<14}{f"{w}x{h}":<14}{mb:>9.1f}M')


def generate_header(path='src/render/backdrop_layers.h'):
    out = []
    out.append('#pragma once')
    out.append('')
    out.append('// GENERATED FILE - do not edit.')
    out.append('//   python tools/generate_backdrop.py --header')
    out.append('//')
    out.append('// V11 retiring the parallax duplication. These four numbers per layer used')
    out.append('// to exist twice - once in the C++ that draws the layer and once in the')
    out.append('// Python that sizes its image - with a comment in each asking a human to')
    out.append('// keep them in step. The failure mode of that arrangement is a seam at the')
    out.append('// pan limit: a layer runs out of image before the camera runs out of world,')
    out.append('// and nothing says so until somebody walks to the edge of the map.')
    out.append('//')
    out.append('// tools/generate_backdrop.py is the source. It derives both the factors and')
    out.append('// the sizes, so a change on that side cannot leave this side stale - the')
    out.append('// same arrangement tools/player_sheet.py --header has with player_sprite.h,')
    out.append('// and for the same reason.')
    out.append('//')
    out.append('// `width`/`height` are what the generator *would* write for this layer at')
    out.append('// these factors. main.cpp compares them against what it actually loaded and')
    out.append('// warns on a mismatch, which is what turns the seam from a pixel nobody')
    out.append('// reaches into a line at startup.')
    out.append('namespace backdrop_layers {')
    out.append('')
    out.append('struct Layer {')
    out.append('    float parallax_x;')
    out.append('    float parallax_y;')
    out.append('    int width;   // the BMP size generate_backdrop.py produces at these factors')
    out.append('    int height;')
    out.append('};')
    out.append('')
    for name, key, factor in LAYERS:
        w, h = layer_size(factor)
        out.append(f'// assets/{key}.bmp')
        out.append(f'inline constexpr Layer {name}{{{factor[0]}f, {factor[1]}f, {w}, {h}}};')
    out.append('')
    out.append('// The inputs the sizes above are derived from, so a reader can tell whether')
    out.append('// a mismatch is a stale asset or a changed display table.')
    out.append(f'inline constexpr int GRID_WIDTH = {GRID_WIDTH};')
    out.append(f'inline constexpr int GRID_HEIGHT = {GRID_HEIGHT};')
    out.append(f'inline constexpr int SCALE = {SCALE};')
    out.append('')
    out.append('} // namespace backdrop_layers')
    out.append('')
    text = '\n'.join(out)
    with open(path, 'w', newline='\n') as f:
        f.write(text)
    print(f'wrote {path}')


if __name__ == '__main__':
    args = sys.argv[1:]
    if '--header' in args:
        generate_header()
    elif '--sizes' in args:
        print_sizes()
    else:
        generate_sky()
        generate_mountains()
