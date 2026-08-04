"""Generates the tree sprites for V4's props layer (layer 3 of notes/
art_direction.txt's model): non-simulated, colour-keyed, drawn at roughly
the terrain's own depth rather than in the far backdrop.

Sized in world cells, not screen pixels - one BMP pixel is one world cell,
the same convention every other authored asset in assets/ already uses, so
a sprite drops into main.cpp's existing Camera::SCALE scaling with no
separate art scale to track.

Run from the repo root:
    python tools/generate_props.py
"""
import random
from pixel_art import PALETTE, COLOR_KEY, dither_mix, write_bmp


def generate_tree(path, w, h, seed, tiers=4):
    # The seed is what makes three trees three trees rather than one shape at
    # three scales. It was accepted and never used in the first version, which
    # is worse than not having it: the call sites passed three distinct values
    # and read as though they varied something.
    #
    # What it varies is deliberately small - a per-tier width wobble and a
    # slight vertical stagger - because the silhouette is doing the work at
    # this size and a tree that varies too much stops reading as the same
    # species as the one beside it.
    rng = random.Random(seed)
    tier_jitter = [rng.uniform(0.82, 1.06) for _ in range(tiers)]
    tier_lift = [rng.randint(0, 2) for _ in range(tiers)]
    pixels = [COLOR_KEY] * (w * h)
    cx = w // 2

    trunk_h = max(3, h // 6)
    trunk_w = max(2, w // 10)
    for y in range(h - trunk_h, h):
        for x in range(cx - trunk_w // 2, cx + trunk_w // 2 + 1):
            if 0 <= x < w:
                pixels[y * w + x] = PALETTE['trunk']

    canopy_bottom = h - trunk_h + 2  # slight overlap so the trunk isn't detached
    tier_height = canopy_bottom // tiers
    max_half = w / 2.0 - 1

    for t in range(tiers):
        tier_bottom_y = canopy_bottom - t * tier_height - tier_lift[t]
        tier_top_y = tier_bottom_y - tier_height - (2 if t == 0 else 0)
        bottom_half = max_half * (tiers - t) / tiers * tier_jitter[t]
        top_half = max(1.0, max_half * (tiers - t - 1) / tiers * 0.4 + 1)

        for y in range(max(tier_top_y, 0), min(tier_bottom_y, h)):
            span = max(1, tier_bottom_y - tier_top_y)
            frac = (y - tier_top_y) / span
            half = top_half + (bottom_half - top_half) * frac
            half = max(1.0, half)

            for dx in range(-int(half), int(half) + 1):
                x = cx + dx
                if not (0 <= x < w):
                    continue
                # Silhouette-first shading: dark on the left, lit on the
                # right, one dithered hand-off between each flat band -
                # the same rule the terrain's rim uses, applied to a sprite
                # instead of a filled region.
                edge = dx / half if half > 0 else 0.0
                if edge < -0.5:
                    color = PALETTE['tree_shadow']
                elif edge > 0.55:
                    color = dither_mix(x, y, PALETTE['tree_mid'],
                                        PALETTE['tree_lit'], (edge - 0.55) / 0.45)
                else:
                    color = dither_mix(x, y, PALETTE['tree_shadow'],
                                        PALETTE['tree_mid'], (edge + 0.5) / 1.05)
                pixels[y * w + x] = color

    write_bmp(path, w, h, pixels)
    print(f'wrote {path} ({w}x{h})')


if __name__ == '__main__':
    generate_tree('assets/tree_a.bmp', 18, 32, seed=11, tiers=4)
    generate_tree('assets/tree_b.bmp', 14, 24, seed=17, tiers=3)
    generate_tree('assets/tree_c.bmp', 22, 38, seed=23, tiers=5)
