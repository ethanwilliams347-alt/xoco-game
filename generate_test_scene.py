import random
import struct

# Generates the F4 fixture scene (assets/test_material.bmp, test_albedo.bmp).
# Layout and intent come from notes/art_pipeline.txt's "test fixture wearing
# art" section - each region below exists to exercise one named system, not
# to look good. Sized to GRID_WIDTH/HEIGHT in main.cpp (640x400); the two
# must move together.

WIDTH = 640
HEIGHT = 400

# Legend colours must match src/physics/material.h's MATERIALS table exactly
# (RGB only - main.cpp's loader masks off alpha before comparing), or a
# region silently loads as Empty instead of the intended material.
WALL = (0x88, 0x88, 0x88)
SAND = (0xEE, 0xDD, 0x82)
WOOD = (0x6B, 0x44, 0x23)
WATER = (0x44, 0x44, 0xFF)
EMPTY = (0x00, 0x00, 0x00)

# Albedo shades close to but distinguishable from the legend colours - a
# stand-in for authored art. Retuning these into an actual palette is F4's
# deferred phase 4 (notes/art_pipeline.txt), not this step's job.
WALL_ALB = (0x77, 0x77, 0x77)
SAND_ALB = SAND
WOOD_ALB = (0x55, 0x33, 0x11)
WATER_ALB = (0x33, 0x33, 0xEE)

FLOOR_TOP = 380  # first row of the floor slab; everything above is open


def write_bmp(filename, width, height, pixels_rgb):
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
        f.write(struct.pack('<i', -height))
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
            for _ in range(row_size - width * 3):
                f.write(b'\x00')


mat = [EMPTY] * (WIDTH * HEIGHT)
alb = [EMPTY] * (WIDTH * HEIGHT)


def fill_rect(x0, x1, y0, y1, material, albedo):
    for y in range(y0, y1):
        for x in range(x0, x1):
            if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                idx = y * WIDTH + x
                mat[idx] = material
                alb[idx] = albedo


# --- Floor, with a pit left open for the bridge (see below) ---
PIT_X0, PIT_X1 = 297, 328
fill_rect(0, PIT_X0, FLOOR_TOP, HEIGHT, WALL, WALL_ALB)
fill_rect(PIT_X1, WIDTH, FLOOR_TOP, HEIGHT, WALL, WALL_ALB)

# --- Snowbank: an uneven Sand slope for powder step-up ---
# Blocky "stairs" of random 1-3 cell tread height, not a smooth ramp - a
# smooth ramp doesn't exercise the player's step-up over uneven powder any
# differently than flat ground does.
rng = random.Random(1)
slope_top = FLOOR_TOP
x = 20
while x < 145:
    tread = rng.randint(6, 10)
    slope_top -= rng.randint(1, 3)
    fill_rect(x, min(x + tread, 145), max(slope_top, FLOOR_TOP - 60), FLOOR_TOP, SAND, SAND_ALB)
    x += tread

# --- Fence posts: standalone Wood columns for dig-the-base collapse ---
for post_x, post_top in ((170, 340), (200, 350), (230, 330)):
    fill_rect(post_x, post_x + 2, post_top, FLOOR_TOP, WOOD, WOOD_ALB)

# --- Bridge: two grounded pillars and a beam spanning the pit ---
# The two pillar/beam joints are the "L-piece keeping its corner" case: dig
# out one pillar and the support flood fill has to carry the beam and the
# far corner down together as one piece, not shed the corner it's attached
# to. The pit is real (no floor under it, see above), so this is the "gap
# underneath" case, not a beam resting on solid ground.
fill_rect(295, 297, 350, FLOOR_TOP, WOOD, WOOD_ALB)   # left pillar
fill_rect(PIT_X1, PIT_X1 + 2, 350, FLOOR_TOP, WOOD, WOOD_ALB)  # right pillar
fill_rect(295, PIT_X1 + 2, 348, 350, WOOD, WOOD_ALB)  # spanning beam

# --- Jump ledges at mixed heights, for jump arc tuning ---
for ledge_x, ledge_y in ((350, 330), (365, 300), (380, 270), (345, 240)):
    fill_rect(ledge_x, ledge_x + 8, ledge_y, ledge_y + 3, WALL, WALL_ALB)

# --- Water channel: walled trench, open at the top so the player can fall in ---
CHANNEL_X0, CHANNEL_X1 = 400, 561
fill_rect(CHANNEL_X0 - 2, CHANNEL_X0, 250, FLOOR_TOP, WALL, WALL_ALB)  # left wall
fill_rect(CHANNEL_X1, CHANNEL_X1 + 2, 250, FLOOR_TOP, WALL, WALL_ALB)  # right wall
fill_rect(CHANNEL_X0, CHANNEL_X1, 310, FLOOR_TOP, WATER, WATER_ALB)
# Diving ledge above the channel's near edge - step off it to fall through
# open air and then through the water, exercising density/displacement.
fill_rect(CHANNEL_X0, CHANNEL_X0 + 20, 230, 233, WALL, WALL_ALB)

# --- Sleepers beside the water: ignite with the Fire brush, then breach the
# channel wall so the spilled Water douses them into Steam ---
sleeper_x = CHANNEL_X1 + 4
while sleeper_x + 8 <= WIDTH - 4:
    fill_rect(sleeper_x, sleeper_x + 8, FLOOR_TOP - 3, FLOOR_TOP, WOOD, WOOD_ALB)
    sleeper_x += 14

write_bmp('assets/test_material.bmp', WIDTH, HEIGHT, mat)
write_bmp('assets/test_albedo.bmp', WIDTH, HEIGHT, alb)
