"""Generates the two backdrop parallax layers (V8): sky+stars and a mountain
silhouette. Layer 1 and 2 of notes/art_direction.txt's four-layer model.

Sizes and parallax factors here are derived from main.cpp's own constants
(GRID_WIDTH/HEIGHT, the DisplayMode table, Camera::SCALE) and MUST stay in
sync with the PARALLAX_* constants main.cpp draws these layers with - if
either changes, the other has to move with it, or a layer runs out of image
before the camera runs out of world and a seam appears at the pan limit.
That coupling is written down here rather than computed automatically
because these are two different languages and this project has no build
step that could enforce it either way; grep main.cpp for PARALLAX_SKY /
PARALLAX_MOUNTAIN before changing a number on either side.

**One asset set covers all three display modes, and which mode sizes it is
the counter-intuitive part.** The window is now switchable at runtime
(1920x1080, 2560x1440, 3440x1440) at a fixed 4 screen pixels per cell, so a
wider window sees *more* cells and therefore has *less* world left to pan
across. The largest pan range - the case a layer can run out of image in -
belongs to the *smallest* window, and the widest window needs the largest
window-sized base. So the layers are sized from the smallest viewport and
the largest window dimensions, taken independently, and every other mode
uses a sub-rectangle of the same file.

Run from the repo root:
    python tools/generate_backdrop.py
"""
import random
from pixel_art import PALETTE, COLOR_KEY, dither_mix, write_bmp

# --- must match main.cpp -----------------------------------------------
GRID_WIDTH, GRID_HEIGHT = 1920, 1080
SCALE = 4                                    # Camera::SCALE

# main.cpp's DisplayMode table, in window pixels.
MODES = [(1920, 1080), (2560, 1440), (3440, 1440)]

PARALLAX_SKY = (0.04, 0.02)                 # (x, y) factors
PARALLAX_MOUNTAIN = (0.15, 0.06)
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


if __name__ == '__main__':
    generate_sky()
    generate_mountains()
