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
# **The backdrop group was raised wholesale on 2026-08-16 (V20) and the reason
# is the one mistake that produced every symptom playtest session 6 reported.**
# The value ladder was built *downward from a floor*. `sky_deep` was authored at
# luminance 18 out of 255 and every band added since was a per-layer Grade, which
# is a multiply and can therefore only darken - so each new band was pushed
# toward zero from a sky that was already at the bottom. Measured on the played
# frame, the whole composition occupied **L 15.5 to 24.5**: nine levels, against
# the reference's 51.6 to 173.6, and smaller than the reference's *smallest
# single band join* (14 levels, at the horizon). The reference's **night** frame
# has its sky at L 163.
#
# So the numbers below are stated as post-grade luminance targets, since that is
# what reaches the screen, and the RGB is whatever hits the target on the hue
# the group already had:
#
#     sky top        76      mountain rim     57    (grade 0.60)
#     sky horizon    50      mountain body    35    (grade 0.60)
#     ground far     24      ground near      85    (grade 0.53)
#
# **`ground near` was 62 until V22 part 4 (2026-08-23) and is the one number in
# that table the ceiling move no longer governs** - see the ground group below
# for why it moved and why nothing around it did.
#
# **Those are V21's numbers (2026-08-16). V20's were 95/62, 71/44, 30/78 and
# playtest session 7 answered item 1 "too bright".** V21 multiplies the whole
# group by **0.80** and changes nothing else: every ratio between bands is
# preserved exactly, both grades are untouched, and this is therefore a ceiling
# move of the second kind in `.claude/rules/assets-and-formats.md` - the kind no
# grade can perform, and the kind whose whole virtue is that it cannot separate
# or merge two bands by accident.
#
# **The reason the factor is 0.80 and not lower is the thing to read before
# taking it down again.** A ceiling move is a multiply, so it scales every
# *absolute* separation by the same factor - and absolute separation is the exact
# quantity V20 was created to buy, after matching the reference plane by ratio
# bought 9.8 levels. The mountain/ground horizon join is the tightest one and it
# is the reference's own signature: 14 levels there. At 0.80 it goes 14 -> 11. At
# 0.72 it would be 10, and at 0.60 it would be 8, back under the reference's
# smallest single band join and most of the way back to the flat frame session 6
# reported. **So the ceiling is nearly out of downward room.** If session 8 still
# reports "too bright", the next move is *not* a smaller factor - it is hue and
# saturation (this group is a strongly saturated violet and saturation reads as
# brightness), or the grades, and either is a different item with a different
# argument.
#
# `star` is deliberately *not* scaled. It is a point accent rather than a band,
# so it carries no ratio to preserve, and the reference's night frame keeps its
# stars and moon at full value while everything around them descends.
#
# Two relationships in that table are the point of it rather than side effects.
# **`ground_far` is the darkest value in the frame** - entry 7's mechanism 2, the
# horizon as the frame's dark pinch - and it sits 14 below the mountains above
# it, which is the reference's own horizon join. And **the sky now darkens
# downward**, top to horizon, where it used to brighten downward; the old
# direction put the frame's brightest row immediately above the row that is
# supposed to be its darkest.
PALETTE = {
    # sky (V8, layer 1) - cool, deep, no warm colour anywhere in this group.
    # `sky_deep` is the top of the frame and `sky_horizon` the bottom of the sky;
    # deep is now the brighter of the two - see the note above.
    'sky_deep':     (0x53, 0x43, 0x8E),
    'sky_horizon':  (0x38, 0x2C, 0x57),
    'star':         (0xE8, 0xE4, 0xFF),

    # mountains (V8, layer 2) - one flat silhouette tone plus a peak rim.
    # **The rim is authored to land *brighter than the sky behind it*** (71
    # against the sky's 62 at that height), because it is the one lit edge in the
    # band and a rim at or below the sky is a rim nobody can see.
    'mountain':     (0x42, 0x34, 0x63),
    'mountain_rim': (0x69, 0x55, 0x9E),

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
    # V11 argues a direction change should be.
    #
    # **This comment used to end "ratio near/far here is 1.83 against the
    # reference plane's 1.78", and that sentence is the whole V20 error in one
    # line.** It is arithmetically true and it was the wrong quantity: entry 7
    # records the reference plane as 77.5 -> 138.2, which is a ratio of 1.78
    # *and* a difference of 61 levels. Matched as a ratio down at this end of the
    # scale, 1.83 bought a ramp of 18 levels before the grade and **9.8 after**,
    # and the tester's report was "the effect isn't very convincing". Kept rather
    # than deleted because the shape recurs: **when a mechanism is absolute
    # contrast, matching its ratio is not matching it.** The pair below is now
    # stated as a difference - 30 to 78 post-grade, 48 levels - and the ratio it
    # happens to have is not the thing being held.
    #
    # **V22 part 4 (2026-08-23) scaled `ground_near` and `ground_mark` by 1.35
    # and left `ground_far` alone, and the reason it is the near end that moved
    # is a geometry finding rather than a taste one.** The paragraph above set
    # the target as a *difference* and named the reference's 61 levels; the pair
    # was then authored to 38 and the gap was never closed, because five
    # sittings of tuning aimed at the tile's near edge - which at the spawn is
    # **never on screen**. The plane is drawn from the horizon to a near edge
    # 640 px below it, the world's surface sits at 65% of that band, and
    # `plane_src_at`'s inverse-depth mapping puts that contact at **tile row 198
    # of 256**. So the row the eye actually reads against the terrain was
    # carrying 0.778 of a ramp that was already 23 levels short, and the join
    # came out at 51 against the world's 73 - a lit shelf in front of a darker
    # backdrop, which is what six playtests in a row reported.
    #
    # At 1.35 the two readings agree, which is the only reason to trust either:
    # the post-grade span becomes **60.9 levels against the reference's 60.7**,
    # and the value at row 198 becomes 71.5 against the world's 73.4. **The
    # target was hit by fixing where it was measured, not by moving it.**
    #
    # **What did not move, and each is load-bearing.** `ground_far` is untouched,
    # so the horizon stays the frame's dark pinch (entry 7 mechanism 2) and stays
    # under the graded mountains - a uniform grade could not have done this, which
    # is why it is here and not in the layer table. `ground_mark` is scaled by the
    # *same* factor so its contrast against the ramp is unchanged as a ratio and
    # grows as a difference toward the near edge, which is the detail-energy ramp
    # entry 13 measures at 12x in the reference and had at 5x and backwards here.
    #
    # **This is the one band V21's 0.80 ceiling no longer covers, and that is a
    # debt not a licence.** 0.80 x 1.35 puts this band 8% above V20, and V20's
    # ground_near is part of what playtest session 7 called "too bright". The
    # defence is that it is *one* band's near end rather than the ceiling, that
    # the visible part of it tops out at 71.5 rather than 85, and that the world's
    # lit surface at 73.4 stays the brightest thing next to the player - which is
    # the reference's arrangement, not a departure from it. If session 14 says
    # "too bright" again, this scale factor is the first thing to look at.
    'ground_far':   (0x32, 0x29, 0x4E),
    'ground_near':  (0xB2, 0x97, 0xCD),
    'ground_mark':  (0xC8, 0xA9, 0xDB),

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

    # **The lit end of the same four, added by V22 part 3 (2026-08-22), and the
    # `*_fill` values above are now the *deep* end of a ramp rather than the
    # material's one tone.** Nothing above changed value; what changed is that a
    # cell near its own surface no longer gets the deep tone.
    #
    # The defect these answer, measured: `plane_probe`'s band ladder at the
    # spawn read the receding plane's near end at luminance **65.7** and the
    # world standing in front of it at **23.3** - the frame's brightest band
    # directly behind its darkest one, at the one junction the eye is asked to
    # read as continuous. Every reference frame does the opposite: the surface
    # the character stands on is the *brightest* band in the frame (72.9-79.9
    # across four WnC frames) and the frame darkens both upward and downward
    # from it (reference_observations ENTRY 12). That inversion is the whole of
    # playtest session 12's "a separate shelf sitting in front of a painted
    # backdrop", and it is why three attempts to fix that report by grading a
    # layer could not have worked: **a Grade is uniform, so a ramp inside one
    # band is always the art's job** (.claude/rules/assets-and-formats.md).
    #
    # So the terrain layer is now authored the way the ground plane already is:
    # **the ramp is in the art and the level is in the grade.** These are the
    # near/lit end of it, and they are targets rather than tastes -
    #
    #     wall lit        67      the plane's near end is 65.7; the world is
    #                             nearer than the plane, so it is not darker
    #     sand lit        78      the top of the reference's lit band (64-81)
    #     wood lit        53      a beam is an object on the surface, not the
    #                             surface; it stays the darker thing on it
    #
    # Hue follows the daylight finding in art_direction section 9 item 1, which
    # holds for any lit surface: **lit surfaces are warm and get warmer as they
    # brighten** (+3 red-over-blue at luminance 0-20, rising to +33 at 90-130).
    # At luminance ~68 that is about +20, and these carry +17 (rock, greyer) to
    # +41 (sand, the warmest thing in the layer). The `*_fill` end keeps its own
    # +10, so the ramp warms as it brightens rather than merely lightening.
    # **Every rung is named, and that is the constraint that shaped the set.**
    # The first version of this ramp interpolated between the lit tone and the
    # fill and dithered between the results, which put nine colours into
    # assets/test_albedo.bmp that appear nowhere in this file -
    # `tools/validate_palette.py` reported 25,480 off-palette pixels and it was
    # right to. Every other generated pass here dithers between two *named*
    # colours and therefore cannot leave the set: the rim light does, the ground
    # tile's far-to-near ramp does. So the ramp is a ladder of palette entries
    # and `apply_depth_ramp` only ever picks between two adjacent rungs.
    #
    # Even steps in luminance, four rungs per material counting the fill:
    #
    #     wall   66.9 -> 52.1 -> 37.7 -> 22.9 (wall_fill)
    #     sand   77.9 -> 61.4 -> 45.4 -> 28.8 (sand_fill)
    #     wood   52.8 -> 41.6 -> 30.3 -> 18.9 (wood_fill)
    'wall_lit':     (0x49, 0x42, 0x38),  # greyer than dirt at the same level: rock
    'wall_mid':     (0x3A, 0x33, 0x2B),
    'wall_shade':   (0x2A, 0x25, 0x1E),
    'sand_lit':     (0x5C, 0x4C, 0x33),
    'sand_mid':     (0x49, 0x3C, 0x27),
    'sand_shade':   (0x37, 0x2C, 0x1C),
    'wood_lit':     (0x3E, 0x33, 0x27),
    'wood_mid':     (0x31, 0x28, 0x1E),
    'wood_shade':   (0x24, 0x1D, 0x16),

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


# --- the depth ramp ------------------------------------------------------
#
# The terrain layer's own far-to-near ramp, and the sibling of `apply_rim_light`
# above: same shape of pass, same place in a generator, one question different.
# The rim asks *is this cell at a surface*; this asks *how far below one is it*.
#
# **Why it is a pass over the albedo and not a Grade in frame.cpp.** A Grade is
# a uniform multiply over a whole layer, so it can place a band on the value
# ladder but can never make one end of a band brighter than the other. The
# ground plane hit this first and resolved it the same way - "the ramp is in the
# art and the level is in the grade" - and the rule was written down after it:
# **a ramp within a band is always the art's job**
# (.claude/rules/assets-and-formats.md). The terrain layer needed exactly that
# and did not have it, which is why it read as one flat near-black mass sitting
# in front of a plane that recedes.
#
# **What it costs, stated because it is the same cost the rim light already
# pays and neither is free.** The tone is baked per cell, so a cell carries the
# depth it was *authored* at, not the depth it currently sits at: dig a lit
# surface cell out and drop it down a shaft and it stays lit. That is
# notes/art_direction.txt section 7's travelled-highlight acceptance, taken
# knowingly there for the rim and taken again here. The freshly exposed face of
# a dig gets no lit band either, for the same reason. If either ever reads
# wrongly in play, the fix is a renderer pass with the grid as its input, and
# that is a different item with a much larger argument to make first.
#
# The steps are quantised and then dithered between, rather than interpolated
# smoothly, for the reason section 3 gives: a transition is *stepped* through
# the Bayer matrix, never blended into tones no editor's palette would contain.
# `steps` is therefore how many distinct tones the ramp adds, and it is a small
# number on purpose - the reference's lit masses carry 5.9-21.5 distinct tones
# per 100 pixels (section 9 item 2) and ours carried 0.1.
def apply_depth_ramp(mat, alb, width, height, empty_marker, ramp,
                     lit_depth=4, fade_depth=24):
    """Returns a new albedo buffer, brightened toward each cell's own surface.

    For every filled cell, the depth to the nearest empty cell **straight up**
    is measured, and the albedo is set from a ladder of colours: `ramp[0]` for
    the first `lit_depth` cells, then the rest of the ladder in order, reaching
    whatever the buffer already held at `lit_depth + fade_depth`. Deeper than
    that the buffer is untouched, so a material's `*_fill` keeps meaning exactly
    what it meant - the deep tone, and the ladder's last rung.

    `ramp` is a sequence of (r, g, b) from the surface downward, or a callable
    `(x, y) -> sequence | None` - the same contract `apply_rim_light` uses, so a
    caller with several materials in one buffer gives each its own ladder and
    returns None for any that should stay flat.

    **Only ever picks between two adjacent rungs, and never averages them.** The
    ladder is palette entries, so the output is too; see the note at the ramp
    colours in PALETTE for the audit that made this the contract.

    Straight up, and not a distance field: the light in this scene comes from
    the sky, so a vertical measure is the one that matches the mechanism. It
    also makes an overhang's underside deep, which is correct, where a distance
    field would light it like a floor.

    Run it **before** `apply_rim_light`, so the rim's dithered hand-off lands on
    the lit tone rather than on the deep one - which is the join the whole pass
    exists to make continuous.
    """
    ramp_fn = ramp if callable(ramp) else (lambda x, y: ramp)

    def filled(x, y):
        if not (0 <= x < width and 0 <= y < height):
            return False
        return mat[y * width + x] != empty_marker

    out = list(alb)
    total = lit_depth + fade_depth
    for x in range(width):
        depth = 0
        for y in range(height):
            if not filled(x, y):
                depth = 0
                continue
            if depth >= total:
                depth += 1
                continue
            rungs = ramp_fn(x, y)
            if not rungs:
                depth += 1
                continue
            idx = y * width + x
            if depth < lit_depth:
                out[idx] = rungs[0]
            else:
                # The fill already in the buffer is the ladder's last rung, so
                # there are len(rungs) segments to cross over fade_depth cells.
                t = (depth - lit_depth) / float(fade_depth)
                level = t * len(rungs)
                k = min(int(level), len(rungs) - 1)
                lower = rungs[k + 1] if k + 1 < len(rungs) else alb[idx]
                out[idx] = dither_mix(x, y, rungs[k], lower, level - k)
            depth += 1
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
