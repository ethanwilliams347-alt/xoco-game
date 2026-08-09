# Adding and swapping assets

How to get new art into the game — a sprite, a player sheet, a location, a
backdrop — and how to swap one out quickly to look at a design.

All commands run from `code/` (the repo root), like every other `tools/` script.

For the player character specifically there is a longer, drawing-first walkthrough
in [`../drawing_to_sprite.md`](../drawing_to_sprite.md) — that one starts at "I
have a drawing", this one starts at "I have a file and I want the game to load it".

---

## The two things that catch everyone

**1. Every asset path is a hardcoded string literal in `main.cpp`.** There is no
asset manifest and no command-line override. Renaming a file, or pointing a
*tool* at a different file, does not change what the game loads:

| What | Path the game loads | Where that literal lives |
|---|---|---|
| Player sheet | `assets/player_sheet.bmp` | [main.cpp:446](src/main.cpp#L446) |
| Location — materials | `assets/test_material.bmp` | [main.cpp:517](src/main.cpp#L517) |
| Location — albedo | `assets/test_albedo.bmp` | [main.cpp:517](src/main.cpp#L517) |
| Prop placements | `assets/test_props.txt` | [main.cpp:472](src/main.cpp#L472) |
| Prop sprites | `assets/<name>.bmp`, named by the prop list | [main.cpp:415](src/main.cpp#L415) |
| Backdrop — sky | `assets/backdrop_sky.bmp` | [main.cpp:392](src/main.cpp#L392) |
| Backdrop — mountains | `assets/backdrop_mountains.bmp` | [main.cpp:393](src/main.cpp#L393) |

`SHEET_PATH` in [tools/player_sheet.py](tools/player_sheet.py) is the *tools'*
path, not the game's — it is not emitted into
[player_sprite.h](src/render/player_sprite.h). Changing it points the validator
at a different file while the exe keeps loading the old one, and nothing
complains, because both files exist and both are valid.

**2. `assets/` is copied next to the executable at build time.** The game runs
from `build/Release/` and loads `build/Release/assets/...`, populated by the
post-build `copy_directory` at [CMakeLists.txt:69-73](CMakeLists.txt#L69-L73).
Editing a file in `assets/` changes nothing you can see until you rebuild:

```bash
cmake --build build --config Release
```

That step is not optional and it is not just for code. If a change "doesn't show
up", check this first — then check that you edited the file in the table above.

---

## The fast loop for trying a design

Because the paths are literals, the quickest way to look at new art is to make
your file **take the existing name** rather than teaching the game a new one:

```bash
cp assets/player_sheet.bmp assets/player_sheet_backup.bmp   # only if it isn't reproducible
cp assets/my_new_idea.bmp  assets/player_sheet.bmp
cmake --build build --config Release
.\build\Release\SlopPhysics.exe
```

Same shape for any row in the table. Keep your variants under their own names
and copy the one you want onto the loaded name — that way the game always reads
a file you know it reads, and you never have two paths that could disagree.

Editing the literal in `main.cpp` instead works too and costs the same rebuild.
It is the better move once a variant is the real one rather than an experiment.

---

## Rules every image asset follows

- **24-bit uncompressed BMP.** Not 32-bit, not RLE, not 8-bit indexed.
  [pixel_art.py:210-216](tools/pixel_art.py#L210) is the codec every tool here
  reads and writes. LibreSprite's and GIMP's own BMP exports are usually *not*
  this, which is why the workflow exports PNG and converts:
  ```bash
  python tools/png_to_bmp.py edited.png assets/whatever.bmp
  ```
- **Transparency is magenta `#FF00FF`, not alpha.** BMP has no usable alpha
  here, so `load_art_texture` colour-keys that exact value into the texture's
  alpha ([main.cpp:162-165](src/main.cpp#L162)). Any magenta in your art becomes
  a hole. `COLOR_KEY` is defined once at
  [pixel_art.py:111](tools/pixel_art.py#L111).
- **Colour-keying is per-asset and opt-in.** Sprites, props and the mountain
  layer pass `colorkey=true`; the sky layer passes `false` because it is an
  opaque full-rect background. A new sprite that renders as a magenta box was
  loaded with the flag off.
- **One BMP pixel is one world cell** — for sprites, props, player frames and
  scenes. The camera's `SCALE` (4 screen pixels per cell) does the rest, so
  there is no separate art scale to track. **Backdrops are the exception**: they
  are authored in *screen pixels*, because they are drawn in screen space with a
  parallax factor rather than placed in the world.
- **Palette conformance is a final step, not a gate.** Draw in whatever colours
  you like while iterating; `tools/snap_to_palette.py` conforms art to the
  locked set when the shape is settled. Only `tools/validate_palette.py` and a
  bare `player_sheet.py --validate` are strict.

---

## Player sheet

One BMP, `assets/player_sheet.bmp`, 84x104 — a 6x4 grid of 14x26 frames. Which
slot means what is the `ANIMATIONS` table at
[player_sheet.py:89-99](tools/player_sheet.py#L89):

| Row | Columns | Animation |
|---|---|---|
| 0 | 0-1 | `idle` (2 frames, 30 steps each) |
| 1 | 0-5 | `walk` (6 frames, 5 steps each) |
| 2 | 0 | `rise` — single pose, chosen by velocity, not by a clock |
| 2 | 1 | `fall` — single pose, same row as `rise` |
| 3 | 0-2 | `dig` (3 frames, one-shot) |

**The sheet is a derived file.** Build it from frame BMPs rather than editing it
in place — that is what stops a frame landing in a column its animation doesn't
own, and stops an animation half-updating so the loop flashes between two
different characters:

```bash
python tools/build_player_sheet.py assets/my_frame.bmp        # stamps into all 13 slots
python tools/player_sheet.py --validate --allow-off-palette
```

To override individual slots as you draw real animation frames:

```bash
python tools/build_player_sheet.py assets/base.bmp \
    --frame walk:0=assets/walk_0.bmp --frame walk:1=assets/walk_1.bmp
```

To paste one frame into a slot of an existing sheet, `tools/set_player_frame.py`.

Frame art must be **exactly 14 px wide** and **26 or fewer tall** — shorter gets
bottom-aligned for you, wider or taller is an error rather than a silent crop
([build_player_sheet.py:72-87](tools/build_player_sheet.py#L72)).

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
`prop_sprite_name_ok` enforces so a data file can never name a path outside
`assets/` ([props.h:63-67](src/scene/props.h#L63)).

**2. Add a line to `assets/test_props.txt`:**

```
# sprite   x
my_rock    420.0
```

`x` is a world cell — the sprite's bottom-**centre** column. **There is no y and
that is deliberate**: each prop is planted on whatever terrain is actually under
its own footprint at load time, because "the ground" is not one number. Three
trees once shipped 26%, 43% and 83% buried from an authored ground line. The
reasoning is in full at [props.h:21-28](src/scene/props.h#L21).

Rebuild and run. No code change — the prop list is read at startup and the
texture cache is keyed by name, so nine trees are three images.

Two failure modes worth knowing:

- **A malformed line costs every prop, not the bad line.** The parser is
  all-or-nothing on purpose and prints the line number. A tolerant parser that
  skipped the bad row would be the third instance of this project shipping a
  scene that rendered, rendered wrong, and said nothing
  ([props.h:48-57](src/scene/props.h#L48)).
- **A missing image is a warning, not an error** — printed once per name, and
  every prop naming it is skipped ([main.cpp:416-419](src/main.cpp#L416)).

`tools/generate_props.py` is how the existing trees were made, if you want a
procedural starting point rather than a drawing.

---

## Locations (scenes)

A location is **two BMPs of the same dimensions**, loaded together:

- **`test_material.bmp` — the material map.** Each pixel's colour names an
  `ElementType` via the frozen table in
  [legend.h:32-47](src/scene/legend.h#L32). This is authoring metadata, *not*
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
   ([main.cpp:47-48](src/main.cpp#L47)) and the scene loads at origin `(0,0)`, so
   a smaller image covers the top-left corner and leaves the rest empty — fine
   for a test, and the camera will pan off it.
2. Name them `test_material.bmp` / `test_albedo.bmp`, or edit the literals at
   [main.cpp:517](src/main.cpp#L517).
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

- `PARALLAX_SKY_X/Y` and `PARALLAX_MOUNTAIN_X/Y` at
  [main.cpp:394-395](src/main.cpp#L394)
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
   ([main.cpp:156](src/main.cpp#L156)) — returns `nullptr` and prints on failure,
   so a missing asset degrades rather than crashes.
2. `SDL_QueryTexture` for its native size if you need it.
3. Draw it in the right layer. The four-layer order (backdrop, props, player,
   terrain — trees draw *before* terrain) is `notes/art_direction.txt`.
4. `SDL_DestroyTexture` at shutdown.

Before adding a second parallel BMP to carry per-item data, read
[props.h:8-19](src/scene/props.h#L8) — the argument for why placements are a
text list and not an image is short and it applies to more than props. Per-cell
data gets an image; a list gets a list.

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
