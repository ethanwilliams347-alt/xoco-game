"""Turns a Gemini pixel-art render of a hand drawing into a single player-sheet
frame: same grid (14x26 cells, matching FRAME_W/FRAME_H in tools/player_sheet.py
and src/render/player_sprite.h) and the same 1-pixel-per-world-cell convention
every other asset in assets/ uses, so the result drops in for a side-by-side
comparison against the real sheet with no separate art scale to reason about.

Colours are kept as Gemini rendered them - no snap to tools/pixel_art.py's
PALETTE. This is for comparing candidate sprites, not shipping one; run
tools/validate_palette.py (or requantize by hand) once a candidate is picked.

Gemini's output is a full-resolution PNG, not an exact low-res grid, so this
does the two things a raw image can't do on its own: flood-fills the flat
backdrop out to this project's transparency colour-key, then box-downsamples
the remaining (non-background) pixels into the target WxH grid.

Assumes the source PNG has a roughly flat, solid-colour background - ask
Gemini for exactly that ("pixel art sprite on a plain solid white background,
no shadow, no texture on the backdrop"). A busy or gradient background will
flood-fill badly; re-export rather than fight it here.

    python tools/gemini_to_player_frame.py render.png assets/candidate_a.bmp
    python tools/gemini_to_player_frame.py render.png out.bmp --width 14 --height 26
    python tools/bmp_to_png.py assets/candidate_a.bmp preview.png   # to eyeball it

Only 8-bit, non-interlaced RGB/RGBA PNGs are supported (re-export as PNG-24 if
your source is a palette or 16-bit PNG - covers what Gemini and every normal
"save image" dialog produce). No third-party dependency, same reasoning as
every other file in tools/: this project has none by policy.
"""
import sys
import zlib
import struct
import argparse
from collections import deque

from pixel_art import COLOR_KEY, write_bmp
from player_sheet import FRAME_W as PLAYER_FRAME_W, FRAME_H as PLAYER_FRAME_H


# --- minimal PNG decoder ----------------------------------------------------

def _paeth(a, b, c):
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    return b if pb <= pc else c


def read_png(path):
    """Returns (width, height, pixels_rgb, alpha) - pixels_rgb is row-major
    (r, g, b); alpha is a same-length list of 0-255 ints, or None if the PNG
    had no alpha channel. This pipeline's own flood-fill (below) is what
    determines transparency for a fresh Gemini export, since that's not
    guaranteed to carry alpha - but tools/png_to_bmp.py, the other reader of
    this function, needs the real channel back for round-tripping art that
    was already edited (and properly erased) in a sprite editor."""
    with open(path, 'rb') as f:
        data = f.read()

    if data[:8] != b'\x89PNG\r\n\x1a\n':
        raise ValueError(f'{path}: not a PNG')

    width = height = bit_depth = color_type = interlace = None
    idat = bytearray()
    pos = 8
    while pos < len(data):
        length = struct.unpack_from('>I', data, pos)[0]
        ctype = data[pos + 4:pos + 8]
        payload = data[pos + 8:pos + 8 + length]
        if ctype == b'IHDR':
            (width, height, bit_depth, color_type, _comp, _filt, interlace) = \
                struct.unpack('>IIBBBBB', payload)
        elif ctype == b'IDAT':
            idat += payload
        elif ctype == b'PLTE' and color_type == 3:
            raise ValueError(f'{path}: palette PNGs are not supported - '
                              f're-export as a truecolor PNG-24')
        pos += 8 + length + 4  # length + type + data + crc

    if bit_depth != 8:
        raise ValueError(f'{path}: only 8-bit PNGs are supported (got {bit_depth}-bit)')
    if interlace:
        raise ValueError(f'{path}: interlaced PNGs are not supported - re-export non-interlaced')
    if color_type not in (2, 6):
        raise ValueError(f'{path}: only RGB (2) or RGBA (6) PNGs are supported '
                          f'(got color type {color_type})')

    channels = 3 if color_type == 2 else 4
    raw = zlib.decompress(bytes(idat))
    stride = width * channels
    recon = bytearray(stride * height)

    pos = 0
    for y in range(height):
        filt = raw[pos]
        pos += 1
        row_start = y * stride
        prev_start = row_start - stride
        for x in range(stride):
            byte = raw[pos + x]
            a = recon[row_start + x - channels] if x >= channels else 0
            b = recon[prev_start + x] if y > 0 else 0
            c = recon[prev_start + x - channels] if y > 0 and x >= channels else 0
            if filt == 0:
                val = byte
            elif filt == 1:
                val = (byte + a) & 0xFF
            elif filt == 2:
                val = (byte + b) & 0xFF
            elif filt == 3:
                val = (byte + (a + b) // 2) & 0xFF
            elif filt == 4:
                val = (byte + _paeth(a, b, c)) & 0xFF
            else:
                raise ValueError(f'{path}: unknown PNG filter type {filt} on row {y}')
            recon[row_start + x] = val
        pos += stride

    pixels = [None] * (width * height)
    alpha = [None] * (width * height) if channels == 4 else None
    for i in range(width * height):
        o = i * channels
        pixels[i] = (recon[o], recon[o + 1], recon[o + 2])
        if alpha is not None:
            alpha[i] = recon[o + 3]
    return width, height, pixels, alpha


# --- background flood-fill --------------------------------------------------

def _dist2(a, b):
    return (a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2 + (a[2] - b[2]) ** 2


def flood_fill_background(width, height, pixels, threshold):
    """Returns a set of pixel indices judged to be background: connected to
    the image border and within `threshold` colour distance of the border's
    own dominant colour. Assumes one roughly flat backdrop colour, per this
    file's docstring."""
    border = ([pixels[x] for x in range(width)] +
              [pixels[(height - 1) * width + x] for x in range(width)] +
              [pixels[y * width] for y in range(height)] +
              [pixels[y * width + width - 1] for y in range(height)])
    bg_ref = max(set(border), key=border.count)
    thresh2 = threshold * threshold

    is_bg = bytearray(width * height)
    q = deque()
    for x in range(width):
        for y in (0, height - 1):
            idx = y * width + x
            if not is_bg[idx] and _dist2(pixels[idx], bg_ref) <= thresh2:
                is_bg[idx] = 1
                q.append(idx)
    for y in range(height):
        for x in (0, width - 1):
            idx = y * width + x
            if not is_bg[idx] and _dist2(pixels[idx], bg_ref) <= thresh2:
                is_bg[idx] = 1
                q.append(idx)

    while q:
        idx = q.popleft()
        x, y = idx % width, idx // width
        for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
            if 0 <= nx < width and 0 <= ny < height:
                nidx = ny * width + nx
                if not is_bg[nidx] and _dist2(pixels[nidx], bg_ref) <= thresh2:
                    is_bg[nidx] = 1
                    q.append(nidx)

    return {i for i in range(width * height) if is_bg[i]}, bg_ref


# --- box downsample ----------------------------------------------------------

def downsample(width, height, pixels, bg_idx, target_w, target_h):
    out = [COLOR_KEY] * (target_w * target_h)
    empty_cells = 0
    for ty in range(target_h):
        y0 = height * ty // target_h
        y1 = max(y0 + 1, height * (ty + 1) // target_h)
        for tx in range(target_w):
            x0 = width * tx // target_w
            x1 = max(x0 + 1, width * (tx + 1) // target_w)
            r = g = b = n = 0
            for y in range(y0, y1):
                row = y * width
                for x in range(x0, x1):
                    idx = row + x
                    if idx in bg_idx:
                        continue
                    px = pixels[idx]
                    r += px[0]; g += px[1]; b += px[2]; n += 1
            if n == 0:
                empty_cells += 1
                continue
            color = (r // n, g // n, b // n)
            if color == COLOR_KEY:  # would accidentally read as transparent
                color = (color[0], color[1], 254)
            out[ty * target_w + tx] = color
    return out, empty_cells


def main():
    p = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('input_png')
    p.add_argument('output_bmp')
    p.add_argument('--width', type=int, default=PLAYER_FRAME_W,
                    help=f'target width in cells (default {PLAYER_FRAME_W}, the player frame width)')
    p.add_argument('--height', type=int, default=PLAYER_FRAME_H,
                    help=f'target height in cells (default {PLAYER_FRAME_H}, the player frame height)')
    p.add_argument('--bg-threshold', type=float, default=24.0,
                    help='colour distance (0-441) under which a pixel connected to the '
                         'border counts as background (default 24)')
    args = p.parse_args()

    width, height, pixels, _alpha = read_png(args.input_png)
    bg_idx, bg_ref = flood_fill_background(width, height, pixels, args.bg_threshold)
    out, empty_cells = downsample(width, height, pixels, bg_idx,
                                   args.width, args.height)
    write_bmp(args.output_bmp, args.width, args.height, out)

    bg_frac = len(bg_idx) / (width * height)
    print(f'read {args.input_png}: {width}x{height}, '
          f'background ~{bg_frac:.0%} (ref #{bg_ref[0]:02X}{bg_ref[1]:02X}{bg_ref[2]:02X})')
    print(f'wrote {args.output_bmp}: {args.width}x{args.height}')
    if args.width == PLAYER_FRAME_W and args.height == PLAYER_FRAME_H:
        print(f'matches the player frame grid (FRAME_W/FRAME_H in tools/player_sheet.py)')
    if empty_cells:
        print(f'note: {empty_cells} target cell(s) had no foreground pixels and are '
              f'colour-keyed transparent - check the silhouette isn\'t clipped')
    print('colours are kept as Gemini rendered them, not snapped to PALETTE - '
          'preview with: python tools/bmp_to_png.py ' + args.output_bmp + ' preview.png')


if __name__ == '__main__':
    sys.exit(main())
