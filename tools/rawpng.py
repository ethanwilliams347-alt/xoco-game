"""Wrap a raw RGB dump from preview_light into a PNG.

    build/Release/preview_light frame.raw 90
    python tools/rawpng.py frame.raw frame.png 804 604

Written against zlib and struct rather than Pillow on purpose: this project has
no third-party dependencies by policy (the same rule that chose BMP over PNG for
assets and immediate-mode over Dear ImGui), and a debugging tool is a bad reason
to be the first. A PNG is a header, one deflate stream of filtered scanlines, and
three CRCs - about fifteen lines, all of them here.
"""
import sys, zlib, struct

raw_path, png_path, w, h = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
data = open(raw_path, 'rb').read()
assert len(data) == w * h * 3, (len(data), w * h * 3)

scan = bytearray()
for y in range(h):
    scan.append(0)
    scan += data[y * w * 3:(y + 1) * w * 3]


def chunk(tag, payload):
    return (struct.pack('>I', len(payload)) + tag + payload +
            struct.pack('>I', zlib.crc32(tag + payload) & 0xFFFFFFFF))


png = b'\x89PNG\r\n\x1a\n'
png += chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
png += chunk(b'IDAT', zlib.compress(bytes(scan), 6))
png += chunk(b'IEND', b'')
open(png_path, 'wb').write(png)
print('wrote', png_path)
