"""The player sprite sheet (V3.1): the animation table, the validator, and the
generated header that keeps this file and main.cpp from disagreeing.

Replaces the earlier generator, which authored one pose as an ASCII grid.
That was the right form for a single frame - a one-pixel change reviewed as a
one-character change - and it is the wrong form for four animations of six
frames, which is twenty-four such blocks. The sheet is hand-drawn instead;
what lives here is everything *about* the sheet that code has to agree on.

The model is Noita's, deliberately and with the parts we don't need left out.
Noita pairs a sprite sheet with a metadata XML: each row of the sheet is one
animation, and a RectAnimation entry names it and gives frame count, frame
size and frame wait. Its player sheet carries ~50 animations; four is the
whole of what this game can currently distinguish without tracking new state.
The part deliberately *not* built yet is the one that makes Noita's wizard
read as alive at this size: a separately drawn limb that rotates toward the
cursor over the body loop, positioned by a per-frame hotspot image so the
attachment tracks the body's bob rather than a coordinate somebody typed in.
That was built and then pulled - see ROADMAP.md's V3.1 entry. Bringing it
back means a second image the same size as the sheet, one marker pixel per
drawn frame, and a SHOULDER table emitted beside ANIMATIONS below; none of
it is left half-present here, because a validator that fails on a missing
marker for a feature the game does not draw is a tax on every new frame.

Three commands, all run from the repo root:

    python tools/player_sheet.py --starter    # bootstrap the BMPs, once
    python tools/player_sheet.py --validate   # check the drawn art
    python tools/player_sheet.py --header     # (re)emit src/render/player_sprite.h

--validate and --header are what you run after editing the art. --starter
refuses to overwrite existing files, because the whole point of it is to be
run before there is anything to lose.

--validate reports off-palette colour as a **warning**, never an error, and
holds every other check strict. `--strict-palette` promotes it back to an
error for the day a palette is actually chosen.

That default is inverted from how this started, and the reason is on the
record rather than rewritten away. V6 froze a palette and this check enforced
it. It was the character group that turned out to be provisional: no player
sheet ever authored - not one of the four in assets/ - has ever had a single
on-palette pixel, so the check was red on every run from the day the first
real art landed. A validator that is known to be red is a validator nobody
reads, which costs the *baseline* checks that only this script can make, and
those are the ones catching real bugs: a frame hovering a cell above every
floor, a hole through a silhouette, a declared frame nobody drew.

So the palette is not a gate while the visual style is still being found. It
becomes one again when there is a roster to choose it against - see
ENGINEERING_NOTES.md, "The palette is deferred, not lost".
"""
import sys
from pixel_art import PALETTE, COLOR_KEY, read_bmp, write_bmp

# --- the layout contract ---------------------------------------------------
#
# One BMP pixel is one world cell, as everywhere else in assets/. The frame is
# 14x26 against Player::WIDTH/HEIGHT of 8x20, anchored bottom-centre: the
# masked head overhangs upward into space that collides with nothing (the
# allowance player.h reserves by name) and the sleeves hang outside the box's
# width. A sleeve overlapping a wall is correct - it is art, not body.
#
# These numbers are emitted into src/render/player_sprite.h rather than
# retyped in main.cpp. V3 duplicated four of them across two files with
# nothing enforcing agreement, which is the trap V8's parallax factors have;
# a sheet turns four numbers into several dozen, so the duplication is
# removed rather than documented.
FRAME_W, FRAME_H = 14, 26
BOX_W, BOX_H = 8, 20
OFFSET_X = (FRAME_W - BOX_W) // 2   # 3
OFFSET_Y = FRAME_H - BOX_H          # 6

MAX_FRAMES = 6      # columns in the sheet; the longest animation sets this
SHEET_COLS = MAX_FRAMES


class Anim:
    """One animation. `wait` is in **fixed simulation steps**, not seconds and
    not rendered frames - see the timing note in src/render/player_anim.h for
    why that distinction is the whole reason this field is an integer.

    `col` is the starting column, which exists so two animations can share one
    row of the sheet. That is Noita's model rather than an economy: its docs
    are explicit that the same row can appear under several names, and `rise`
    and `fall` below are exactly that case - two single-frame poses that are
    selected by a condition rather than advanced by a clock, and that have no
    business each occupying six columns of a sheet to hold one drawing.

    `airborne` says this animation only ever plays with no ground under the
    feet, and it exists purely to tell --validate which baseline rule applies.
    It is not emitted into the header: nothing in the game reads it, because
    the selector already knows when the player is off the ground."""
    def __init__(self, name, row, col, frames, wait, loop, airborne=False):
        self.name, self.row, self.col = name, row, col
        self.frames, self.wait, self.loop = frames, wait, loop
        self.airborne = airborne


# Row order is the sheet's top-to-bottom order. Frame counts are what the art
# is expected to fill; --validate checks the drawn sheet against them, so
# lowering a count here is how you ship a shorter animation rather than
# leaving half-drawn frames in the file.
ANIMATIONS = [
    #    name     row  col  frames  wait  loop
    Anim('idle',   0,   0,     2,    30,  True),
    Anim('walk',   1,   0,     6,     5,  True),
    # Two poses, one row. Neither advances on a clock: which one shows is
    # decided by the sign of the player's vertical velocity, so a long fall
    # holds the falling pose instead of cycling through a rise it is not doing.
    Anim('rise',   2,   0,     1,     0,  True),
    Anim('fall',   2,   1,     1,     0,  True),
    # Slowed from 4 to 8 - the swing now runs 24 steps (0.4s) rather than 12.
    # Note this widens the gap with DigTool::COOLDOWN_STEPS (6): a held dig
    # re-fires and restarts the swing long before it finishes, so frames 1 and
    # 2 are only ever seen on an isolated click. That is a known consequence,
    # not an oversight - see the dig notes in src/physics/tool.h.
    Anim('dig',    3,   0,     3,     8,  False),   # one-shot; see player_anim.h
    # One wing beat, latched by Player::flapped() the way `dig` is latched by
    # the tool firing. **`wait` 3 against Player::FLAP_INTERVAL_STEPS of 14 is
    # the whole design of this row and the two numbers have to be read
    # together.** Six frames at 3 steps is 18, longer than the beat interval,
    # so the next flap always re-latches this animation before it would have
    # ended - which is what keeps the wings locked to the rhythm the physics is
    # actually producing, with no gap where it would fall back to the rise/fall
    # poses mid-climb. Let the interval and this wait drift apart and you get
    # either a stutter (wait too short) or a beat that visibly outlives the
    # impulse that caused it (wait too long).
    #
    # The consequence of 18 against 14 is that sustained flight re-latches part
    # way through frame 4 and frame 5 is never seen while the key is held. That
    # is deliberate rather than a miscount: frames 0-3 are the downstroke and
    # the edge-on turn, which is the part that has to stay in phase, and 4-5 are
    # the recovery back to the top. They play when the player *stops* flapping,
    # which is why frame 5 is drawn to read as a still - it is the last thing
    # shown before the falling pose takes over. Dropping to wait 2 would fit all
    # six into 12 steps and re-introduce exactly the two-step gap this row's
    # length exists to close.
    Anim('fly',    4,   0,     6,     3,  False,  airborne=True),
]

SHEET_ROWS = 1 + max(a.row for a in ANIMATIONS)
SHEET_W, SHEET_H = SHEET_COLS * FRAME_W, SHEET_ROWS * FRAME_H

# SHEET_ROWS is derived from the table above; SHEET_COLS is not, because the
# sheet's width is authored art rather than a consequence. So the one thing
# that can disagree is checked here, at import, rather than at the point it
# would surface: an animation running past the last column reads a frame that
# is not in the image, which the validator would report as a blank frame in
# the middle of a working animation rather than as a table that is too wide.
_widest = max(a.col + a.frames for a in ANIMATIONS)
assert SHEET_COLS >= _widest, (
    f'MAX_FRAMES is {SHEET_COLS} but {_widest} columns are needed - widen the '
    f'sheet before adding frames to the table')

# The one file both the tools and the game read - and it is now read from the
# same place the game reads it from, rather than typed here to match.
#
# This used to be a literal with a note saying "keep these two in step by hand",
# because main.cpp had its own literal. Pointing one somewhere the other was not
# looking validated one file while the game loaded another, which cost this
# project several rounds of "I changed the art and nothing happened". Both sides
# now resolve `player_sheet` through assets/sprites.txt, so they cannot drift:
# tools/load_sprite.py rebinds the key and this follows it on the next run.
#
# The fallback is the same one main.cpp compiles in, for the same reason - a
# missing or unreadable manifest gets you the shipped art rather than a crash.
_DEFAULT_SHEET = 'assets/player_sheet_fly.bmp'


def _sheet_path_from_manifest(key='player_sheet', default=_DEFAULT_SHEET):
    try:
        with open('assets/sprites.txt', encoding='utf-8') as f:
            for line in f:
                text = line.split('#', 1)[0].split()
                if len(text) >= 2 and text[0] == key:
                    return 'assets/' + text[1]
    except OSError:
        pass
    return default


SHEET_PATH = _sheet_path_from_manifest()
HEADER_PATH = 'src/render/player_sprite.h'


# --- reading the sheet ------------------------------------------------------

def frame_pixels(pixels, width, row, col):
    """The (row, col) frame lifted out of a full-sheet pixel list."""
    ox, oy = col * FRAME_W, row * FRAME_H
    return [pixels[(oy + y) * width + (ox + x)]
            for y in range(FRAME_H) for x in range(FRAME_W)]


def validate(strict_palette=False):
    """Everything about the art that code downstream assumes and cannot check.

    The checks are the ones the single-pose generator made before this file
    replaced it, applied per frame, plus the one the sheet format adds. They
    are not stylistic - each corresponds to a specific silent failure:

      - a frame whose bottom row is empty draws a figure hovering one cell
        above every floor it stands on, forever, and the symptom reads as a
        physics bug rather than an art one;
      - an empty row inside the collision box is a horizontal gap through the
        silhouette;
      - a frame the animation table declares but the art leaves blank shows as
        the player vanishing for a few steps mid-cycle, which reads as a
        flicker rather than as a frame nobody drew.

    The first two are about *standing on something*, so they are checked only
    on the rows that can be drawn with a floor under them. An `airborne` row
    (`fly`) is exempt from them and gets the same two failures restated
    against its own painted extent instead: the frame must not be blank, and
    it must not have a hole through the middle of the figure. Lifting the body
    off the bottom of the frame is what flight *is* here - the fly art raises
    the torso and lets the legs trail, up to four rows clear of the baseline on
    the upstroke - and there is no floor to look wrong against, because the
    animation is latched by a wing beat and cancelled on landing (see the
    on_ground branches in src/render/player_anim.cpp).
    """
    errors = []

    def load(path, expect_w, expect_h):
        try:
            w, h, px = read_bmp(path)
        except FileNotFoundError:
            errors.append(f'{path}: missing - run --starter first')
            return None
        if (w, h) != (expect_w, expect_h):
            errors.append(f'{path}: is {w}x{h}, expected {expect_w}x{expect_h}')
            return None
        return px

    sheet = load(SHEET_PATH, SHEET_W, SHEET_H)

    if sheet is not None:
        allowed = set(PALETTE.values()) | {COLOR_KEY}
        off = {p for p in sheet if p not in allowed}
        if off:
            sample = ', '.join('#%02X%02X%02X' % p for p in sorted(off)[:5])
            message = f'{SHEET_PATH}: {len(off)} off-palette colour(s): {sample}'
            if strict_palette:
                errors.append(message)
            else:
                print(f'NOTE  {message}')
                print('      (no palette is chosen yet - this is not a failure. '
                      '--strict-palette makes it one.)')

        for anim in ANIMATIONS:
            for col in range(anim.col, anim.col + anim.frames):
                fp = frame_pixels(sheet, SHEET_W, anim.row, col)
                painted = [any(fp[y * FRAME_W + x] != COLOR_KEY for x in range(FRAME_W))
                           for y in range(FRAME_H)]
                where = f'{anim.name} frame {col}'
                if anim.airborne:
                    if not any(painted):
                        errors.append(f'{where}: nothing drawn - the player would vanish mid-beat')
                        continue
                    top, bottom = painted.index(True), FRAME_H - 1 - painted[::-1].index(True)
                    for y in range(top, bottom):
                        if not painted[y]:
                            errors.append(f'{where}: row {y} is a gap through the figure')
                            break
                    continue
                if not painted[FRAME_H - 1]:
                    errors.append(f'{where}: bottom row empty - would hover above the floor')
                for y in range(OFFSET_Y, FRAME_H):
                    if not painted[y]:
                        errors.append(f'{where}: row {y} is inside the collision box but empty')
                        break

    for e in errors:
        print(f'FAIL  {e}')
    if errors:
        return False
    print(f'OK    {SHEET_PATH}: {SHEET_COLS}x{SHEET_ROWS} frames of {FRAME_W}x{FRAME_H}, '
          f'{sum(a.frames for a in ANIMATIONS)} drawn, baselines clean')
    return True


# --- the generated header ---------------------------------------------------

def emit_header():
    lines = []
    w = lines.append

    w('// GENERATED by tools/player_sheet.py - do not edit.')
    w('//')
    w('// Regenerate with:  python tools/player_sheet.py --header')
    w('//')
    w(f'// Everything main.cpp needs to know about {SHEET_PATH}, emitted')
    w('// from the one file that also validates it. V3 shipped these numbers typed')
    w('// into two places with nothing enforcing agreement - the same trap V8\'s')
    w('// parallax factors have - and a sheet turns four numbers into several dozen,')
    w('// so the duplication is removed rather than documented.')
    w('#pragma once')
    w('#include "physics/player.h"')
    w('')
    w('namespace player_sprite {')
    w('')
    w('// One animation. `wait` is in fixed simulation steps - see')
    w('// render/player_anim.h for why that is not a duration in seconds.')
    w('//')
    w('// `col` lets two animations share one row, which is Noita\'s model rather')
    w('// than an economy - RISE and FALL below are one row holding two poses that')
    w('// are chosen by a condition instead of advanced by a clock.')
    w('struct Anim {')
    w('    int row;')
    w('    int col;')
    w('    int frames;')
    w('    int wait;')
    w('    bool loop;')
    w('};')
    w('')
    w(f'inline constexpr int FRAME_W = {FRAME_W};')
    w(f'inline constexpr int FRAME_H = {FRAME_H};')
    w(f'inline constexpr int SHEET_COLS = {SHEET_COLS};')
    w(f'inline constexpr int SHEET_ROWS = {SHEET_ROWS};')
    w('')
    w('// The sprite is anchored to the collision box\'s bottom-centre: subtract')
    w('// these from the box\'s top-left corner to get the frame\'s top-left.')
    w(f'inline constexpr int OFFSET_X = {OFFSET_X};')
    w(f'inline constexpr int OFFSET_Y = {OFFSET_Y};')
    w('')
    w('static_assert(FRAME_W >= Player::WIDTH && FRAME_H >= Player::HEIGHT,')
    w('              "the player sprite must be at least as large as the collision box "')
    w('              "it is anchored to; a smaller frame cannot express the offset");')
    w('static_assert(OFFSET_X == (FRAME_W - Player::WIDTH) / 2 &&')
    w('              OFFSET_Y == FRAME_H - Player::HEIGHT,')
    w('              "generated offsets disagree with Player\'s box - the sheet was "')
    w('              "generated against a different body size; rerun the generator");')
    w('')
    w('// `inline` is load-bearing rather than stylistic, and a test caught it.')
    w('// A plain `constexpr` variable at namespace scope in a header has internal')
    w('// linkage, so every translation unit gets its *own* object at its own')
    w('// address - and render/player_anim.cpp identifies the current animation by')
    w("// pointer. Without inline, main.cpp's IDLE and player_anim.cpp's IDLE are")
    w('// different addresses, every comparison is false, and the animation restarts')
    w('// on every step: a figure that stands still twitching. It compiles, links')
    w('// and runs.')
    for a in ANIMATIONS:
        w(f'inline constexpr Anim {a.name.upper()}{{{a.row}, {a.col}, {a.frames}, {a.wait}, '
          f'{"true" if a.loop else "false"}}};')
    w('')
    w('// No hotspot table: the aiming arm this sheet was designed around is not')
    w('// drawn at the moment. It attached at a per-frame marker pixel carried in')
    w('// a second image the size of the sheet, which is the idea worth keeping -')
    w('// see ROADMAP.md V3.1 - but nothing is emitted for it here, because a')
    w('// table main.cpp does not read is a table nobody notices going stale.')
    w('')
    w('}  // namespace player_sprite')
    w('')

    with open(HEADER_PATH, 'w', newline='\n') as f:
        f.write('\n'.join(lines))
    print(f'wrote {HEADER_PATH}')


# --- starter art ------------------------------------------------------------
#
# The point of this is that you edit frames rather than start from a blank
# canvas: V3's authored pose is stamped into every frame slot the animation
# table declares, so the game runs end to end from the first minute and each
# frame you actually redraw is an improvement over a working baseline rather
# than a prerequisite for one.

_STARTER_POSE = """\
..............
.....LLL......
....LMMMB.....
....MAKAM.....
...LAKKKAB....
....MAKAM.....
...LMMMMBB....
..LLMMMMBBB...
.BLLMMMMMBBB..
.BMMMMMMMMBB..
.BMMMMMMMMBB..
.BMMMMMMMMBB..
.BMMTTTTMMBB..
.BMMBMMBMMBB..
.BMMBMMBMMBB..
.BMMBMMBMMBB..
.BMMBMMBMMBB..
..BMBMMBMMB...
..BMBMMBMMB...
...MBMMBMB....
...MBMMBMB....
...MBMMBMB....
...MBMMBMB....
....B....B....
....T....T....
...TT...TT....
"""

_LEGEND = {
    '.': COLOR_KEY,
    'B': PALETTE['char_base'],
    'M': PALETTE['char_mid'],
    'L': PALETTE['char_light'],
    'T': PALETTE['char_belt'],
    'K': PALETTE['char_mask'],
    'A': PALETTE['char_accent'],
}


def _pose_pixels():
    rows = _STARTER_POSE.rstrip('\n').split('\n')
    assert len(rows) == FRAME_H and all(len(r) == FRAME_W for r in rows)
    return [_LEGEND[ch] for r in rows for ch in r]


def starter():
    import os
    existing = [p for p in (SHEET_PATH,) if os.path.exists(p)]
    if existing:
        print('refusing to overwrite: ' + ', '.join(existing))
        print('--starter is a bootstrap; delete these by hand if you really mean it.')
        return False

    pose = _pose_pixels()
    sheet = [COLOR_KEY] * (SHEET_W * SHEET_H)

    for anim in ANIMATIONS:
        for col in range(anim.col, anim.col + anim.frames):
            ox, oy = col * FRAME_W, anim.row * FRAME_H
            for y in range(FRAME_H):
                for x in range(FRAME_W):
                    sheet[(oy + y) * SHEET_W + (ox + x)] = pose[y * FRAME_W + x]

    write_bmp(SHEET_PATH, SHEET_W, SHEET_H, sheet)

    print(f'wrote {SHEET_PATH} ({SHEET_W}x{SHEET_H})')
    print('Every frame is V3\'s pose. Load assets/palette.gpl in your editor and redraw.')
    return True


if __name__ == '__main__':
    args = set(sys.argv[1:])
    if not args:
        print(__doc__)
        sys.exit(2)
    ok = True
    if '--starter' in args:
        ok = starter() and ok
    if '--validate' in args:
        # `--allow-off-palette` is now the default and survives only as a
        # no-op, so the command written down in ASSETS.md and in older notes
        # keeps working instead of failing on an unknown flag.
        ok = validate('--strict-palette' in args) and ok
    if '--header' in args:
        emit_header()
    sys.exit(0 if ok else 1)
