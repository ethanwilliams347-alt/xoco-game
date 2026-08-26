# V28 — `bg1` world scale and scene geometry

**Status: plan only, nothing implemented. Written 2026-08-25 after the tester
rejected V27's shipped backdrop a second time ("still looks wrong ... also there
are weird issues with boundaries").**

The reference the tester supplied is `code/IMG_0195.PNG`. Everything in the
"Measured facts" section below was read off that file and off `assets/bg1/`
with a decoder, not estimated from the thumbnail. **Each number here is
reproducible; re-derive them before trusting this document rather than after a
change fails.**

---

## 1. Measured facts

These are the findings. They come first because three of them contradict things
currently written down in the repo, and those contradictions are the whole
reason the shipped frame is wrong.

### 1.1 The reference is the art at exactly 10x, ungraded

`IMG_0195.PNG` is 3440x1440. The art is 344x144. That is 10.0x on both axes, and
every band boundary in a vertical column scan lands on a multiple of 10.

Compositing the nine BMPs at 10x with magenta colour-key and **no grade at all**
reproduces the reference exactly except for one rectangle. Diff bounding box:

    x 1930..2049, y 1060..1319   (120 x 260 px)

That rectangle is the player. Nothing else differs — so the reference carries
**no per-layer darkening**. V27 ships nine `Grade` values (255/220/200/190/180/
170/160/255/255); the reference says all nine are 255.

### 1.2 The depth order is strict numeric-descending filename order

The composite only matches when the order is

    09_sky, 08_ground, 07_mountains, 06_hills_far, 05_hills_midfar,
    04_hills_mid, 03_hills_midnear, 02_hills_near, 01_fg_rocks

which is what `art_src/Background_1/README.md` says when its table is read as
what it is labelled: **"Layer Order & Depth Breakdown (Front to Back)"**, 01
front through 09 back.

**V27 ships `08_ground` second-from-front, between `02_hills_near` and
`01_fg_rocks`.** That is the largest visual error after the scale: the ground
plane fills art rows 63..143 at 56% coverage, so placed near the front it paints
over the entire hill range. The grey mass in the lower half of the frame the
tester has been looking at is this.

### 1.3 Per-layer vertical extents

    bg1_01_fg_rocks       rows 126..143    3.8% fill
    bg1_02_hills_near     rows  65.. 89    3.7%
    bg1_03_hills_midnear  rows  46.. 82    5.0%
    bg1_04_hills_mid      rows  45.. 78    7.5%
    bg1_05_hills_midfar   rows  33.. 73    8.0%
    bg1_06_hills_far      rows  22.. 67   10.1%
    bg1_07_mountains      rows   0.. 62   29.8%
    bg1_08_ground         rows  63..143   56.2%
    bg1_09_sky            rows   0..143  100.0%

`08_ground` and `07_mountains` do not overlap (63..143 against 0..62), which is
why swapping only those two leaves the composite identical — do not read that as
licence to reorder them. The README states the order, and the other seven
positions are pinned by pixels.

### 1.4 The player sits at art row 132, and that number is derivable

The player's diff box is 120x260 px = 12x26 art pixels at 10x.
`player_sprite.h` has `FRAME_W = 14`, `FRAME_H = 26` — so the sprite is drawn at
**one screen pixel per art pixel per world cell**, 10 screen px each, and 12 of
its 14 columns are opaque.

Sprite rows 106..131, so the feet plane is row **132**. `player_sprite.h` has
`OFFSET_Y == FRAME_H - Player::HEIGHT` = 6, and `Player::HEIGHT` = 20, so the
sprite bottom is `pos_y + 20`. Feet at 132 gives `pos_y = 112` and a collision
box of rows 112..131.

**This is not a hand-placed mockup coordinate. It is the exact position a body
of the engine's own dimensions takes when it stands on a floor at row 132.**

### 1.5 The art does not tile, and V27 tiles it

Rows where column 0 equals column 343:

    bg1_01_fg_rocks       144/144      bg1_06_hills_far      129/144
    bg1_02_hills_near     140/144      bg1_07_mountains      116/144
    bg1_03_hills_midnear  111/144      bg1_08_ground         129/144
    bg1_04_hills_mid      113/144      bg1_09_sky            141/144
    bg1_05_hills_midfar   122/144

`03_hills_midnear` mismatches on 33 of 144 rows. `bg1` is declared `infinite`,
so `draw_backdrop_layer` takes the `wrap_axis` branch and repeats every layer
horizontally — putting a hard vertical seam in each layer, at nine different
places, sliding at nine different speeds. **That is the "weird issues with
boundaries".** The other half of it is that the world is 1920 cells wide while
travel is unbounded, so the cells run out while the backdrop keeps going.

The art is one 344-pixel-wide scene. It was never a tile and cannot be made one
by a flag.

### Reproduction

`assets/bg1/*.bmp` are nine 344x144 24-bit BMPs, magenta `FF00FF` is the key,
and `IMG_0195.PNG` is RGBA8 non-interlaced. Appendix A describes the decoder.

---

## 2. The structural decision: `Camera::SCALE` has to become per-scene

**Read this before writing any code — it is the only decision in the plan that
is not forced by a measurement.**

The tester's complaint "the size of player to the scene" is a ratio, and that
ratio is fixed by the art-pixel-to-world-cell mapping alone; the screen scale
cancels out of it. The reference gives the mapping as **one art pixel = one
world cell**: the player's 12 opaque columns against the scene's 344.

Check the alternatives, since art-pixels-per-cell is the only other knob:

| mapping | world size | player width / scene width | verdict |
|---|---|---|---|
| 1 art px = 1 cell | 344 x 144 | 12/344 = 3.5% | matches the reference |
| 1 art px = 2 cells | 688 x 288 | 12/688 = 1.7% | player half the reference size |
| 1 art px = 4 cells | 1376 x 576 | 0.87% | player a quarter |

So the world is 344 x 144 cells. The window then has to be a *sub-window* of it,
which the tester stated directly ("1920x1080 showing a window of the full
scene"):

    1920 / SCALE < 344  ->  SCALE > 5.58
    1080 / SCALE < 144  ->  SCALE > 7.50

**`SCALE >= 8`, and `SCALE = 10` is the value that makes 3440x1440 show the whole
scene exactly, which is what the reference is.** `Camera::SCALE` is currently 4.

### 2.1 Why not simply change the 4 to a 10

Because `src/game/display.h` spends a paragraph arguing that 4 px/cell is the
art direction, measured off `resources/video_screenshots/test_location.jpg`,
with `Player::WIDTH` sized to match; `tests/test_golden_frame.cpp` would change
checksum; `docs_test` asserts that checksum from prose; and both recorded
sessions in `session_*.rec` would replay at a different framing. That is a great
deal of recorded reasoning to overturn in order to fix one scene's backdrop.

### 2.2 What to do instead

**Make the scale a property of the scene, defaulting to 4.** `Camera` already
owns every cell-to-pixel conversion in the project — that is F3.2's whole point
— so it is the right owner, and a default-constructed `Camera` keeps 4. That is
what `tests/test_golden_frame.cpp:226` constructs, so **the golden checksum is
unaffected by construction rather than by luck.** Verify that claim rather than
assuming it; it is the plan's main safety property.

Scope of the change. All 37 sites are listed by
`grep -rn "Camera::SCALE" src tests`:

- `src/game/camera.h` — keep `static constexpr int DEFAULT_SCALE = 4;`, add an
  `int scale_ = DEFAULT_SCALE;` member, `scale()` and `set_scale()`. Replace
  `SCALE` inside the member functions with `scale_`.
- `src/game/display.h` — `viewport_w()` / `viewport_h()` / `padded_w()` /
  `padded_h()` take an `int scale` parameter. They stay `constexpr`; callers
  passing a runtime value is legal.
- `src/main.cpp` — 5 sites; thread the active scene's scale through.
- `src/render/frame.cpp` — 10 sites; all have `p.camera` in scope, so they
  become `camera.scale()`. **`frame.h`'s `Params` must not gain a second copy of
  the scale** — one source of truth, and it is the camera.
- `tests/plane_probe.cpp`, `tests/test_camera.cpp`, `tests/test_golden_frame.cpp`
  — `constexpr int VIEW_W = 1920 / Camera::SCALE` and friends become
  `Camera::DEFAULT_SCALE`, still constexpr. **No test value changes.**
- `src/physics/fixed.h:21`, `src/physics/player.h:42`, `src/game/boot.h:37`,
  `tests/bench_grid.cpp:8` — comments only. Update the prose; do not leave a
  comment asserting a constant that has become a default.

`src/physics/` must not learn about any of this. The scale is a rendering
property; the simulation is in cells and always was.

### 2.3 The scene column

Add a ninth field to `assets/scenes.txt` and to `SceneDef`: `scale`, omitted
meaning `Camera::DEFAULT_SCALE`. `scene_list.h` already states the rule this has
to follow — every field is read, an omitted one falls back to a default stated
at the field, and the optional fields so far are `mode` then `width height` as a
pair. Follow that shape, and extend `scene_list.cpp`'s parser and its test.

---

## 3. Scene geometry for `bg1`

With one art pixel to one world cell and `scale = 10`:

    world                344 x 144 cells
    layer draw size      3440 x 1440 screen px   (344*10, 144*10)
    viewport @1920x1080  192 x 108 cells   (padded 193 x 109)
    viewport @2560x1440  256 x 144         (padded 257 x 145)
    viewport @3440x1440  344 x 144         (the whole world; no scroll)
    mode                 fixed, NOT infinite

### 3.1 `fixed` is now correct, and `infinite` is now wrong

V27's `scenes.txt` comment argues at length that `infinite` is load-bearing
because `fixed` mode ignores the parallax factors. **That argument was right
about the code and wrong about the geometry, and it has to be deleted rather
than amended.** It was reached because under the old model — art stretched to
the window — the world was 1920 cells and the art was one window wide, so
nothing but tiling could fill the travel.

Under the new model the art is exactly world-sized, which changes the arithmetic
completely. For a bounded world of `W` cells, a viewport of `V`, and a layer `W`
cells wide drawn at factor `f`, at the rightmost camera position:

    right edge on screen = -(W - V) * scale * f + W * scale
    covers the window when   W - (W - V) * f >= V   i.e.   f <= 1

**Any factor at or below 1.0 covers the viewport at every camera position, with
no tiling and no gap.** That is not a coincidence — it is why the art is exactly
world-sized. The same holds vertically.

So `draw_backdrop_layer` needs a factor-reading branch on a *bounded* world,
which today only the `infinite` path provides. **Do not solve this by declaring
the scene infinite.** Split the condition instead:

- read `camera.parallax_origin_x/y(factor)` whenever the layer came from
  `backdrop.layers`, in both modes;
- tile with `wrap_axis` only when `p.is_infinite`;
- leave the existing generated-backdrop `fixed` path (the `max_cam_x`
  normalisation at `frame.cpp:130-149`) **exactly as it is** — it is what the
  golden frame checksums, and it is right for art that is *sized* per factor.

V27 answered two questions with one flag; this is the separation.

### 3.2 The floor has to be terrain, not the world border

`bg1` is currently `spawn floor`, which puts the body on the world's bottom
border. With the world 144 cells tall that is row 144, and the camera's vertical
clamp then pins the player to the very bottom edge of the window at every
resolution. The reference has the feet at row 132 (section 1.4).

Give `bg1` a material map and an albedo map, and `spawn terrain`:

- **material**: `assets/bg1_material.bmp`, 344 x 144, `0x000000` (Empty) for
  rows 0..131 and `0x888888` (Wall) for rows 132..143. Colours come from
  `src/scene/legend.h`; the legend is frozen, so use it, do not extend it.
- **albedo**: `assets/bg1_albedo.bmp`, 344 x 144, the nine layers composited
  back-to-front. Only rows 132..143 are ever read — an Empty cell places nothing
  — but `src/scene/bmp.cpp:105` requires the two maps to be the same size, so
  author the full frame.
- `scene_list.cpp:127` refuses one of material/albedo without the other, so it
  is both or neither.

`boot::stand_player_on_ground` then scans down the spawn column, finds the Wall
at row 132, and puts `pos_y` at 112 — **the reference's exact value, arrived at
by the engine's own scan rather than by a constant anyone typed.** That is the
check that this section is right, and it is worth printing once.

Framing that results, with `VERTICAL_ANCHOR = 0.80` unchanged:

    @3440x1440  world fits the viewport, view_y clamps to 0,
                feet at screen y 1320 / 1440 = 0.917   <- the reference
    @1920x1080  desired view_y = 122 - 109*0.80 = 34.8, max is 144-109 = 35,
                so view_y = 34.8, feet at (132-34.8)*10 = 972 / 1080 = 0.900

Within 2% of the reference at both. **`VERTICAL_ANCHOR` is not to be retuned for
this** — V23/V23a/V23b rejected a moving anchor twice by playing, and the
constant carries that history in its own comment.

One consequence, stated rather than left to be discovered: the bottom 12 rows
become **real, diggable matter**. That is correct for this engine, and it makes
`bg1` the first authored scene where the backdrop's foreground and the
simulation's floor are the same thing. It also means `01_fg_rocks` (rows
126..143, `is_foreground`) draws in front of those cells, which is exactly what
its README row asks for.

### 3.3 The layer table

Back to front, all grades 255, `parallax_y` equal to `parallax_x`:

| file | px | py | grade | foreground |
|---|---|---|---|---|
| bg1_09_sky.bmp           | 0.04 | 0.04 | 255 | no |
| bg1_08_ground.bmp        | 0.52 | 0.52 | 255 | no |
| bg1_07_mountains.bmp     | 0.12 | 0.12 | 255 | no |
| bg1_06_hills_far.bmp     | 0.20 | 0.20 | 255 | no |
| bg1_05_hills_midfar.bmp  | 0.30 | 0.30 | 255 | no |
| bg1_04_hills_mid.bmp     | 0.42 | 0.42 | 255 | no |
| bg1_03_hills_midnear.bmp | 0.55 | 0.55 | 255 | no |
| bg1_02_hills_near.bmp    | 0.70 | 0.70 | 255 | no |
| bg1_01_fg_rocks.bmp      | 1.00 | 1.00 | 255 | **yes** |

Three notes, each a decision rather than a transcription:

- **The grades are 255 because the reference measures them at 255** (1.1), not
  because grading is unwanted. V27 invented its nine values; they were never
  authored. If depth cueing is wanted later it is a tuning pass with the tester,
  recorded in TUNING.md — not a guess restored here.
- **`01_fg_rocks` is 1.00, not the README's 1.20.** Section 3.1's inequality
  caps a world-sized layer at 1.00; above that it gaps at the right edge. 1.00 is
  the low end of the README's own "1.00x - 1.20x", so nothing is being overruled.
  **If 1.20 is genuinely wanted, the fix is a wider foreground image, not a
  bigger number** — 344 * 1.20 = 413 px of art.
- **`08_ground` gets 0.52, the near end of its "0.28x - 0.52x (Ramp)".** One
  factor cannot express a ramp. This is the one item in the plan that is
  unresolved rather than decided, and it is unresolved in the tester's favour:
  0.52 keeps the near ground moving with the near hills. **Carry it into the
  ROADMAP entry as an open question, not into a code comment where it will read
  as settled.**

### 3.4 Vertical parallax is affordable now

V27 scaled every vertical factor by 0.020 because the layer had 72 px of
vertical headroom against 3236 px of camera travel. Under this plan the camera
travels 35 cells = 350 px and the layer has 360 px of headroom, so the factors
apply as authored. **Delete `py_scale` and the paragraph justifying it.** It was
a correct workaround for a geometry that no longer exists, and leaving it is
precisely the failure CLAUDE.md's second section names.

---

## 4. Phases

Each phase ends with `ctest --test-dir build -C Release --output-on-failure`,
all 19 suites. **`golden_frame_test` must stay green at every phase** — if it
goes red, the per-scene scale has leaked into the default path, and that is the
bug, not the checksum.

### Phase 1 — per-scene scale, no behaviour change

`Camera::DEFAULT_SCALE` plus a `scale_` member; `DisplayMode` methods take a
scale; thread it through `main.cpp` and `frame.cpp`; fix the four comment sites.
Nothing sets a scale other than 4 yet.

**Verify:** 19/19 including the golden checksum, and `.\build\Release\SlopPhysics.exe`
frames identically. This phase is a pure refactor and its whole value is that
the checksum proves it.

### Phase 2 — the `scale` column

`SceneDef::scale`, parser, defaulting, and a `test_scene_list` case for the
column present and absent. `activate_scene` calls `camera.set_scale(def.scale)`
before it computes the viewport.

**Verify:** 19/19. `empty` still opens at 480x270 cells — print it and read it.

### Phase 3 — bounded parallax in `draw_backdrop_layer`

Split "read the factors" from "tile horizontally" per 3.1. The generated
three-layer path must stay byte-identical.

**Verify:** 19/19, golden checksum unchanged. A `test_frame` case asserting that
a world-sized layer at `f <= 1` covers the viewport at both camera extremes is
cheap and pins the inequality in 3.1.

### Phase 4 — the material and albedo maps

Extend `tools/convert_background_layers.py` (or add a sibling — do not write a
third BMP writer; `tools/png_to_bmp.py` owns that) with a mode emitting
`assets/bg1_material.bmp` and `assets/bg1_albedo.bmp` per 3.2. **Row 132 is the
only magic number; derive it from `FRAME_H - OFFSET_Y` if that is readable from
the generated header, otherwise state it once with a comment pointing at section
1.4 of this plan.**

**Verify:** run the game and read the `Scene: 344x144, N cells placed` line.
`N` must be `344 * 12 = 4128`; a wrong order of magnitude means the legend did
not match, which is `scene_list.h`'s "declared empty versus stamped nothing"
distinction doing its job.

### Phase 5 — rewrite the `bg1` row and `load_bg1_layers`

`scenes.txt`: `bg1  bg1_material.bmp  bg1_albedo.bmp  -  terrain  fixed  344 144  10`.
Delete the `infinite` justification comment and replace it with why `fixed` is
right (3.1). `load_bg1_layers`: the 3.3 table, `l.w = 344 * scale`,
`l.h = 144 * scale`, no `py_scale`, no grades. Keep the `Backdrop: N of 9 ...`
launch line — it earned its place — and update it to print the scale and world
size actually used.

**Verify:** 19/19, then the tester plays it.

### Phase 6 — docs

- **ROADMAP.md** — a `V28` entry carrying: the ground-plane ramp as an open
  question (3.3), the 1.20 foreground factor needing wider art (3.3), and the
  `surface_plane.cpp` finding in section 5.
- **`assets/scenes.txt`** — the V27 comment block asserting `infinite` is
  load-bearing is now false. Delete it; do not amend it.
- **`src/game/display.h`** — the "At 4 px/cell: 1920x1080 shows 480x270 cells"
  paragraph now describes a default, not a constant. Rewrite it, and check
  whether `docs_test` reads those numbers.
- **MANUAL_TESTING.md** — a step for this, added to the owed list at the top the
  moment it is asked for. What it checks: the layer order (no grey plane over
  the hills), no vertical seams walking to either world edge, the player's feet
  near 90% of window height, and that the bottom band digs.
- **TUNING.md** — only if a grade or a factor moves off 3.3.
- **`notes/handoff_prompt.md`** — do not touch unless told to.

---

## 5. Out of scope, recorded so it is not rediscovered

**`src/render/surface_plane.cpp` reads the generated backdrop's geometry and does
not early-return on an authored stack.** Its terrain tint calls
`frame::ground_horizon_y(camera, backdrop.mountain_h)` and reads
`backdrop.ground_h` — the generated plane's numbers — while `bg1` no longer
draws that plane. Today it is inert only because `bg1` has no terrain.
**Phase 4 gives `bg1` terrain, so it stops being inert during this plan.**

It stays out of scope because guarding it means deciding what the tint should
follow when the plane it was derived from is not on screen, and that is a design
question. **Do not let Phase 4 ship without looking at what the bottom 12 rows
are tinted to.** If it is visibly wrong, the minimum correct move is an early
return disabling the tint for authored-stack scenes, with the design question
left in the ROADMAP entry.

---

## Appendix A — the decoder used for section 1

A standalone Python reader for both formats, no Pillow, since the numbers above
are worth only as much as their reproducibility.

- **BMP**: 24-bit, bottom-up when the header height is positive, row stride
  padded to 4 bytes, BGR channel order, pixel data at the offset in bytes 10-14.
- **PNG**: RGBA8 non-interlaced, zlib-inflate the concatenated IDAT chunks, then
  undo the five standard per-scanline filters (None/Sub/Up/Average/Paeth).

Composite by painting layers back-to-front, skipping `(255, 0, 255)`. Scale by
nearest neighbour at 10x. Diff against `IMG_0195.PNG` and take the bounding box
of the mismatching pixels; it is the player.
