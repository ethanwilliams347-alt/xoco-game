"""Wraps an authored BMP into a PNG so it can actually be looked at without
a pixel-art editor installed - the same zlib/struct technique
tools/rawpng.py already uses for preview_light's raw dumps, applied to the
other end of this pipeline. No third-party dependency, same reasoning as
the rest of tools/.

    python tools/bmp_to_png.py assets/backdrop_mountains.bmp out.png
"""
import sys
import zlib
import struct
from pixel_art import read_bmp

bmp_path, png_path = sys.argv[1], sys.argv[2]
w, h, pixels = read_bmp(bmp_path)

scan = bytearray()
for y in range(h):
    scan.append(0)
    for x in range(w):
        scan += bytes(pixels[y * w + x])


def chunk(tag, payload):
    return (struct.pack('>I', len(payload)) + tag + payload +
            struct.pack('>I', zlib.crc32(tag + payload) & 0xFFFFFFFF))


png = b'\x89PNG\r\n\x1a\n'
png += chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
png += chunk(b'IDAT', zlib.compress(bytes(scan), 6))
png += chunk(b'IEND', b'')
open(png_path, 'wb').write(png)
print('wrote', png_path)
