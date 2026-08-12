# Adding and swapping assets

How to get new art into the game — a sprite, a player sheet, a location, a
backdrop — and how to swap one out quickly to look at a design.

All commands run from `code/` (the repo root), like every other `tools/` script.

For the player character specifically there is a longer, drawing-first walkthrough
in [`../drawing_to_sprite.md`](../drawing_to_sprite.md) — that one starts at "I
have a drawing", this one starts at "I have a file and I want the game to load it".

---

## Loading a sprite: the one command

Drop the `.bmp` into `assets/` in File Explorer, then:

```bash
python tools/load_sprite.py player_sheet my_new_sheet.bmp
```

That validates the image, rebinds the key in
[assets/sprites.txt](assets/sprites.txt), and copies both into the build
directories. **Launch the game — no rebuild, no code change.**

```bash
python tools/load_sprite.py --list          # what is bound to what right now
python tools/load_sprite.py --stage         # re-copy assets/ into the build dirs
```

The keys are whatever [assets/sprites.txt](assets/sprites.txt) lists — currently
`player_sheet`, `backdrop_sky`, `backdrop_mountains`, and one per prop sprite
(`tree_a`…). A prop's key is the name `test_props.txt` already uses, so
rebinding one swaps the drawing without touching any scene's placements.

What the tool refuses, and why each refusal is worth having:

| Refusal | Because |
|---|---|
| A file outside `assets/` | The game only ever looks there. |
| Not a readable 24-bit BMP | SDL opens some files the rest of the pipeline can't; `tools/png_to_bmp.py` converts. |
| A sheet whose grid ≠ the animation table | It would load, draw, and simply draw the *wrong rectangles* — silently. |

It also warns when a sprite has no magenta pixels, since that sprite will draw
as a solid rectangle — correct for a backdrop, wrong for anything with a
silhouette.

**Frame layout is not something this tool can change.** Frame size, row order
and frame counts are the `ANIMATIONS` table in
[tools/player_sheet.py](tools/player_sheet.py) and the header it generates —
facts about the game, not about the image. A sheet of a genuinely different
shape means editing that table and rerunning `--header`; the tool prints exactly
that when it rejects one.

## The thing that still catches everyone

**`assets/` is copied next to the executable at build time.** The game runs from
`build/Release/` and loads `build/Release/assets/...`, populated by the
post-build `copy_directory` in [CMakeLists.txt](CMakeLists.txt). `load_sprite.py`
stages files for you, but *editing* a file in `assets/` by hand still changes
nothing you can see until you rebuild (or run `--stage`):

```bash
cmake --build build --config Release
```

That step is not optional and it is not just for code. If a change "doesn't show
up", check this first — then check that you edited a file the game actually
loads.

Not everything goes through the manifest. These are still literals in
`main.cpp`, because they are not sprites — a location is a pair of BMPs read by
a different loader, and the prop list is a placement file:

| What | Path the game loads | Where that literal lives |
|---|---|---|
| Location — materials | `assets/test_material.bmp` | `load_scene_from_bmp(...)` call in [main.cpp](src/main.cpp) |
| Location — albedo | `assets/test_albedo.bmp` | same call |
| Prop placements | `assets/test_props.txt` | `load_prop_list(...)` call in [main.cpp](src/main.cpp) |

The manifest is safe to break. Every key falls back to the file the code shipped
with, so a deleted, truncated or malformed `sprites.txt` gets you the old art
and a warning on stderr — never a black screen. A malformed one is discarded
*wholesale* rather than line by line, because half-applied rebinding (some art
moved, some didn't) is harder to diagnose than the art you already had.

The player sheet's path used to be written in **two** places — `SHEET_PATH` in
`tools/player_sheet.py` and a literal in `main.cpp` — with nothing enforcing
that they agreed, so you could validate one file while the exe loaded another.
Both now resolve the `player_sheet` key through the manifest, so they cannot
drift.

Several BMPs in `assets/` are no longer read by anything — `player_sheet.bmp`
and `player_sheet_2.bmp` (superseded sheets), plus the working files
`COPY_player_sheet_fly.bmp`, `copy_candidate_a.bmp` and `v2_owl_pixel_art.bmp`.
They are safe to delete and worth deleting — a stale sheet that still loads and
still validates is exactly what makes the failure above hard to see. Check
`--list` before deleting anything: a file is unused only if no key names it.

---

## The fast loop for trying a design

Keep every variant under its own name and rebind between them. Nothing is
overwritten, so going back is another one-line command rather than a restore:

```bash
python tools/load_sprite.py player_sheet candidate_a.bmp
.\build\Release\SlopPhysics.exe
python tools/load_sprite.py player_sheet candidate_b.bmp
.\build\Release\SlopPhysics.exe
```

Same shape for any key. For the location and prop-list rows in the table above —
the ones that are still literals — the old approach is still the approach: copy
your file **onto the loaded name** and rebuild.

---

## Rules every image asset follows

- **24-bit uncompressed BMP.** Not 32-bit, not RLE, not 8-bit indexed.
  `read_bmp` / `write_bmp` in [pixel_art.py](tools/pixel_art.py) are the codec
  every tool here reads and writes, and `read_bmp` rejects anything else with a
  named error. LibreSprite's and GIMP's own BMP exports are usually *not*
  this, which is why the workflow exports PNG and converts:
  ```bash
  python tools/png_to_bmp.py edited.png assets/whatever.bmp
  ```
- **Transparency is magenta `#FF00FF`, not alpha.** BMP has no usable alpha
  here, so `load_art_texture` in [main.cpp](src/main.cpp) colour-keys that exact
  value into the texture's alpha. Any magenta in your art becomes a hole.
  `COLOR_KEY` is defined once in [pixel_art.py](tools/pixel_art.py).
  **The one place magenta is *not* transparency is a material map** — the scene
  legend uses `#FF00FF` to mean Oil. Material maps are never colour-keyed, so
  the two uses never meet, but do not carry the habit from one to the other.
- **Colour-keying is per-asset and opt-in.** Sprites, props and the mountain
  layer pass `colorkey=true`; the sky layer passes `false` because it is an
  opaque full-rect background. A new sprite that renders as a magenta box was
  loaded with the flag off.
- **One BMP pixel is one world cell** — for sprites, props, player frames and
  scenes. The camera's `SCALE` (4 screen pixels per cell) does the rest, so
  there is no separate art scale to track. **Backdrops are the exception**: they
  are authored in *screen pixels*, because they are drawn in screen space with a
  parallax factor rather than placed in the world.
- **There is no palette to conform to right now, and that is deliberate.** Draw
  in whatever colours you like — nothing checks and nothing will fail. The
  `char_*` group in `tools/pixel_art.py` is a placeholder from before any
  character was drawn and no shipped pixel has ever used it; a real set gets
  derived from the art once there are two or three entities to derive it from.
  Reasoning in [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md), "The palette is
  deferred, not lost". `assets/palette.gpl` is still a reasonable starting point
  to load in an editor, and `tools/snap_to_palette.py` still exists for the day
  there is a set worth snapping to. **The generated layers — backdrops, trees,
  terrain — are a different case**: they are built from `PALETTE` by name and do
  still conform, so `tools/validate_palette.py` is meaningful on those.

---

## Player sheet

One BMP, `assets/player_sheet_fly.bmp`, 84x130 — a 6x5 grid of 14x26 frames.
Which slot means what is the `ANIMATIONS` table in
[tools/player_sheet.py](tools/player_sheet.py):

| Row | Columns | Animation |
|---|---|---|
| 0 | 0-1 | `idle` (2 frames, 30 steps each) |
| 1 | 0-5 | `walk` (6 frames, 5 steps each) |
| 2 | 0 | `rise` — single pose, chosen by velocity, not by a clock |
| 2 | 1 | `fall` — single pose, same row as `rise` |
| 3 | 0-2 | `dig` (3 frames, one-shot, 8 steps each) |
| 4 | 0-5 | `fly` (6 frames, one wing beat, re-latched by each flap) |

`fly`'s 3-step wait is tied to `Player::FLAP_INTERVAL_STEPS`; the two have to be
retuned together, and the fly row's comment in `player_sheet.py` says why. The
fly row is also the one row `--validate` holds to a different baseline rule: it
is marked `airborne=True`, so the "bottom row empty" and "gap inside the
collision box" checks are replaced by "not blank, no hole through the figure".
Flight lifts the torso off the bottom of the frame on purpose and there is no
floor for it to look wrong against.

**The sheet is a derived file.** Build it from frame BMPs rather than editing it
in place — that is what stops a frame landing in a column its animation doesn't
own, and stops an animation half-updating so the loop flashes between two
different characters:

```bash
python tools/build_player_sheet.py assets/my_frame.bmp        # stamps into all 19 slots
python tools/player_sheet.py --validate
```

To override individual slots as you draw real animation frames:

```bash
python tools/build_player_sheet.py assets/base.bmp \
    --frame walk:0=assets/walk_0.bmp --frame walk:1=assets/walk_1.bmp
```

To paste one frame into a slot of an existing sheet, `tools/set_player_frame.py`.

Frame art must be **exactly 14 px wide** and **26 or fewer tall** — shorter gets
bottom-aligned for you, wider or taller is an error rather than a silent crop
(`load_frame` in [build_player_sheet.py](tools/build_player_sheet.py)).

`--validate` checks the three things only art can get wrong, each of which reads
as a different bug than it is: an empty bottom row (figure hovers above every
floor, looks like physics), an empty row inside the collision box (gap through
the silhouette), and a declared-but-blank frame (player vanishes mid-cycle,
looks like flicker).

**If you change the layout** — frame size, frame counts, a new animation — edit
`ANIMATIONS` and the constants in `player_sheet.py`, then regenerate the header
so `main.cpp` agrees:

```bash
python tools/player_sheet.py --header      # rewrites src/render/player_sprite.h
```

Never hand-edit `player_sprite.h`. A stale header renders the figure sunk into
the floor or reading the wrong frames.

---

## Props (new sprites in the world)

Two pieces: the image, and a line in a text file saying where it stands.

**1. Drop a 24-bit BMP in `assets/`,** magenta-keyed, sized in world cells. Name
it whatever you like — a bare stem, no path separators and no `..`, which
`prop_sprite_name_ok` in [src/scene/props.h](src/scene/props.h) enforces so a
data file can never name a path outside `assets/`.

**2. Add a line to `assets/test_props.txt`:**

```
# sprite   x
my_rock    420.0
```

`x` is a world cell — the sprite's bottom-**centre** column. **There is no y and
that is deliberate**: each prop is planted on whatever terrain is actually under
its own footprint at load time, because "the ground" is not one number. Three
trees once shipped 26%, 43% and 83% buried from an authored ground line. The
reasoning is in full in the `PropDef` header comment in
[src/scene/props.h](src/scene/props.h).

Rebuild and run. No code change — the prop list is read at startup and the
texture cache is keyed by name, so nine trees are three images.

Two failure modes worth knowing:

- **A malformed line costs every prop, not the bad line.** The parser is
  all-or-nothing on purpose and prints the line number. A tolerant parser that
  skipped the bad row would be the third instance of this project shipping a
  scene that rendered, rendered wrong, and said nothing — the argument is at
  `load_prop_list` in [src/scene/props.h](src/scene/props.h).
- **A missing image is a warning, not an error** — printed once per name by the
  `prop_texture` lambda in [main.cpp](src/main.cpp), and every prop naming it is
  skipped.
- **A prop with no terrain under it is dropped, with a warning**, rather than
  parked at a fallback height where it would hang in the sky. The
  `Props: N of M placed` line on stdout is the check; `N < M` means one of the
  two warnings above fired.

`tools/generate_props.py` is how the existing trees were made, if you want a
procedural starting point rather than a drawing.

---

## Locations (scenes)

A location is **two BMPs of the same dimensions**, loaded together:

- **`test_material.bmp` — the material map.** Each pixel's colour names an
  `ElementType` via the frozen `SCENE_LEGEND` table in
  [src/scene/legend.h](src/scene/legend.h). This is authoring metadata, *not*
  the colours anything is drawn in.
- **`test_albedo.bmp` — the albedo map.** The colour each cell actually renders
  as. This is where the art lives.

The legend, which is frozen — changing a value invalidates every scene file ever
authored:

| Colour | Material |
|---|---|
| `#000000` | Empty (the artist's "nothing here") |
| `#EEDD82` | Sand |
| `#4444FF` | Water |
| `#888888` | Wall |
| `#6B4423` | Wood |
| `#FF00FF` | Oil |
| `#00FFFF` | Steam |
| `#FF0000` | Fire |
| `#FF8800` | Charred |

**Keeping the legend separate from the render palette is the entire reason that
table exists.** The loader used to match scene pixels against `MATERIALS[i].color`
— the colour a material is *drawn* in — which made every scene file a hostage of
the art direction. V2 retuned the palette, every authored pixel stopped matching,
the whole scene loaded as Empty, and the game booted to a blank world with all
suites green. Paint material maps in legend colours only.

To author one:

1. Produce both BMPs at matching dimensions. The world is 1920x1080 cells
   (`GRID_WIDTH` / `GRID_HEIGHT` in [main.cpp](src/main.cpp)) and the scene loads
   at origin `(0,0)`, so a smaller image covers the top-left corner and leaves
   the rest empty — fine for a test, and the camera will pan off it.
2. Name them `test_material.bmp` / `test_albedo.bmp`, or edit the literals in the
   `load_scene_from_bmp(...)` call in [main.cpp](src/main.cpp).
3. Rebuild, run, and **read stdout rather than eyeballing it**:
   ```
   Scene: 1920x1080, 334901 cells placed
   ```
   That count is the check. `0 cells placed` gets its own warning, and any pixel
   in no legend entry prints a `WARNING` naming the unrecognised colours.
4. Props are per-scene too — `assets/test_props.txt` is written against a
   specific scene's geometry, so a new location generally wants its own list.

`generate_test_scene.py` at the repo root generates the existing fixture and is
worth reading as a worked example — note that it is a *fixture wearing art*,
where each region exists to exercise one named system.

---

## Backdrops

Two static parallax layers, `backdrop_sky.bmp` (opaque) and
`backdrop_mountains.bmp` (colour-keyed), drawn behind everything.

These are the one asset type you should not hand-size. They are authored in
**screen pixels**, and their dimensions are derived from `GRID_WIDTH/HEIGHT`,
the display-mode table, `Camera::SCALE` and the parallax factors together — a
layer that is too small runs out of image before the camera runs out of world,
and a seam appears at the pan limit. The factors live in *two* languages with no
build step able to enforce agreement:

- `PARALLAX_SKY_X/Y` and `PARALLAX_MOUNTAIN_X/Y` in [main.cpp](src/main.cpp)
- `PARALLAX_SKY` / `PARALLAX_MOUNTAIN` in
  [tools/generate_backdrop.py](tools/generate_backdrop.py)

Change one side and you must change the other. To retheme a backdrop, edit the
generator's colours and regenerate rather than repainting the BMP by hand:

```bash
python tools/generate_backdrop.py
```

If you do want to hand-paint one, generate first and match the output's exact
dimensions.

---

## A brand-new kind of asset

There is no plugin point; adding a new category means editing `main.cpp`. The
existing pattern, in order:

1. `load_art_texture(renderer, "assets/thing.bmp", /*colorkey=*/true)` at startup
   in [main.cpp](src/main.cpp) — returns `nullptr` and prints on failure, so a
   missing asset degrades rather than crashes.
2. `SDL_QueryTexture` for its native size if you need it.
3. Draw it in the right layer. The order in the render loop is **sky →
   mountains → props → terrain (the cell texture) → player → light → reticle and
   HUD**. The first four are the four-layer scenery model in
   `notes/art_direction.txt`; the player and the light pass are not layers in
   that model and sit on top of it. **Props draw *before* terrain on purpose** —
   a trunk overlapping authored ground gets buried by it for free, with no depth
   test.
4. `SDL_DestroyTexture` at shutdown.

Before adding a second parallel BMP to carry per-item data, read the header
comment in [src/scene/props.h](src/scene/props.h) — the argument for why
placements are a text list and not an image is short and it applies to more than
props. Per-cell data gets an image; a list gets a list.

---

## When something doesn't show up

| Symptom | Cause |
|---|---|
| No change at all after editing art | Didn't rebuild — `assets/` is copied post-build |
| No change, and you did rebuild | Edited a file the game doesn't load; check the path table above |
| Sprite is a magenta rectangle | Loaded with `colorkey=false` |
| Sprite has holes in it | Real art pixels landed on exactly `#FF00FF` |
| Asset fails to load at all | Not 24-bit uncompressed BMP — round-trip via `tools/png_to_bmp.py` |
| World boots blank | Material map isn't in legend colours; watch stdout for the `cells placed` count and the unmatched-colour warning |
| Figure hovers above the floor | A frame's bottom row is empty — `player_sheet.py --validate` |
| Figure sunk into the floor | `src/render/player_sprite.h` is stale — rerun `--header` |
| Figure flashes between two designs | Only some slots of the sheet were redrawn |
| Props all vanished | One malformed line in the prop list; the error names it |
| One prop type missing | Its BMP didn't load; warned once by name on stderr |
| Seam at the edge of the world | Backdrop size and the `PARALLAX_*` constants disagree |
