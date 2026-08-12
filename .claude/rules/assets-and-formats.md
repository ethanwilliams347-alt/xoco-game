---
paths:
  - "**/assets/**"
  - "**/tools/**"
  - "**/src/render/**"
  - "**/src/scene/**"
  - "**/src/ui/**"
  - "**/src/main.cpp"
  - "**/drawing_to_sprite.md"
---

# Art pipeline and authored data formats

Loaded when touching assets, the Python tools, rendering, the scene/prop loaders
or the UI. Full walkthrough in `ASSETS.md`; this is what must not be broken.

## The palette is deferred — draw freely, and do not add a gate

**No palette is enforced on hand-drawn art, on purpose** (decided 2026-08-12;
full reasoning in [ENGINEERING_NOTES.md](../../ENGINEERING_NOTES.md), "The
palette is deferred, not lost"). The visual style is still being found and there
is one entity to find it from. **Draw in whatever colours you like.**

This file used to say "the palette is locked", and that is corrected rather than
deleted because of *how* it went false: `PALETTE`'s `char_*` group was frozen as
a **prediction** of a character before one was drawn, and never matched a single
shipped pixel. The validator was red on every run, so the checks bundled with it
got ignored too.

- **`tools/pixel_art.py`'s `PALETTE` still binds the *generated* layers** —
  backdrops, trees, terrain are built from those names, so they conform by
  construction. An off-palette pixel there means a generator has been edited to
  hardcode a colour instead of naming one. That is still worth catching.
- **`tools/validate_palette.py` is an audit, not a gate.** It runs only when you
  invoke it, and nothing in CMake or ctest invokes it. On a drawn sprite its
  output describes the file; it is not a defect.
- **`python tools/player_sheet.py --validate` is still worth running after every
  art change**, and its palette line is now a `NOTE`. The checks that matter are
  the structural ones — empty bottom row, gap through the silhouette, blank
  declared frame — each of which reads as a physics or animation bug when it
  fires. `--strict-palette` promotes the palette line back to an error.
- **Do not re-introduce palette enforcement** — not in ctest, not in a hook, not
  as a default flag — until the deferral is closed by a second and third entity
  drawn in the intended style. Then the set gets *derived from* the art that
  exists, the way the generated groups earned their lock.

The **format** rule is untouched, because it is about what the tools can read
rather than what colours are allowed: **BMP-with-colour-key is the default for
terrain, props and the player sheet**, and an asset in any other format is one
`read_bmp` cannot open, so it has to be **listed by name in
`assets/sprites.txt`**.

## The scene legend is not the render palette

[src/scene/legend.h](../../src/scene/legend.h) maps authored colours to
`ElementType`. It used to match against `MATERIALS[i].color` — the colour a
material is *drawn* in — which made every scene file a hostage of the art
direction. Retuning the palette then made all 27,192 authored pixels resolve to
`Empty`; the game booted blank and **every suite still passed**, because the
lookup lived in `main.cpp` where nothing links.

- **A legend value is arbitrary and permanent**, exactly like a `Stream` tag.
  Changing one invalidates every material map ever authored.
- **Never point the legend back at `MATERIALS` colours**, in either direction.
- An unrecognised colour is *reported*, never silently resolved to `Empty` — that
  conflation is what made a broken scene and an empty scene the same observation.

## Rules for any authored data format

Learned building the prop list; all four generalise to the next format.

1. **Per-cell data gets an image; a list gets a list.** The scene BMPs are
   grid-sized, so a parallel prop map would be 6.2 MB of black carrying nine
   meaningful pixels — and it still could not say *which* sprite each pixel meant
   without the frozen legend growing a row per species.
2. **A format must not carry a number its loader ignores.** The prop format has
   no `y`; props are planted by scanning the terrain under their own footprint.
   A `y` in the file would be read and discarded, and **a number the loader
   ignores is one an author eventually spends an afternoon tuning.** Writing one
   is a parse error with a message saying why, not a silent skip.
3. **A malformed list costs every record, not the bad line.** Tolerant parsing
   produces a scene that renders, renders wrong, and says nothing. Absent and
   broken are held separately: a missing file is not an error, because a scene
   with no props is a legitimate scene.
4. **Data that names a path can name any path.** A sprite name is letters,
   digits, `_` and `-` — no separators, no `..`. Apply this to every later format
   that lets authored data reach the filesystem.

## Generated files and duplicated constants

- **[src/render/player_sprite.h](../../src/render/player_sprite.h) is generated.**
  Edit the `ANIMATIONS` table in `tools/player_sheet.py`, then run
  `python tools/player_sheet.py --header`. Editing the header is overwritten work.
- Frame size, row order and frame counts are facts about the *game*, in that
  table — not facts about the image. `load_sprite.py` refuses a sheet whose grid
  disagrees, because it would otherwise load and draw the wrong rectangles
  silently.
- **Known unenforced duplication:** the parallax factors exist in both
  `main.cpp` (`PARALLAX_SKY_X/Y`, `PARALLAX_MOUNTAIN_X/Y`) and
  `tools/generate_backdrop.py`, with nothing checking they agree. The failure is a
  seam at the pan limit. Generating the header, as V3.1 did for the player sheet,
  is the precedent to copy.

## Swapping art

```bash
python tools/load_sprite.py <key> <file.bmp>   # validate, rebind, stage
python tools/load_sprite.py --list             # current bindings
python tools/load_sprite.py --stage            # re-copy assets/ into build dirs
python tools/load_sprite.py --stage --prune    # ...and drop staged files assets/ no longer has
```

No rebuild and no code change for anything in `assets/sprites.txt`. **But
`assets/` is copied next to the exe at build time**, so a file edited by hand
shows nothing until a rebuild or `--stage`.

**Staging never removes, so build dirs accumulate.** A renamed or deleted asset
keeps loading from next to the exe — twelve stale files had built up by
2026-08, three with no source in `assets/` at all. `--stage` reports them;
`--prune` deletes them. Run it after any rename or delete.

## Asset naming and layout — a standard, not a preference

Full version in `ASSETS.md`, "Where art lives" and "Trying new art". The parts
that matter when writing code or moving files:

- **`assets/` is flat and holds only what the game can load.** `load_sprite.py`
  refuses any name with a path separator, so this is enforced, not asked for.
- **`assets/wip/` holds drafts, candidates and superseded art**, and is
  unreachable by construction: the binder refuses paths into it, and `stage()`
  copies files while skipping directories. **Do not "fix" either of those** —
  they are what keeps the split real. Adding recursion to staging would silently
  ship every draft.
- **A filename says what a thing *is*, never which revision it is.** No `COPY_`,
  `_2`, `_final`, `_new`. This is not tidiness: the project accumulated five
  player sheets, two byte-identical, with the *bound* one called
  `COPY_player_sheet_fly.bmp` — so the generated `player_sprite.h` comment named
  a different file than the game actually loaded. Revisions are commits.
- **The bound player sheet is `assets/player_sheet.bmp` and keeps that name**
  whatever is drawn inside it. A new candidate gets a descriptive name
  (`player_sheet_owl.bmp`) and is rebound; when it wins it *takes* the plain
  name and the loser moves to `wip/`.
- **A small edit is made in place**, with git as the undo. Do not copy a file
  before editing it — that copy is what becomes the next `COPY_`.

Three things are *not* in the manifest and are still literals in `main.cpp`,
because they are not sprites: `assets/test_material.bmp` and `test_albedo.bmp`
(a location, read by `load_scene_from_bmp`) and `assets/test_props.txt`
(placements, read by `load_prop_list`).

## Rendering constraints

- The renderer stays `SDL_Renderer`. The shader path has been examined and
  refused twice, most recently because **every scheduled item has a named route
  through the renderer as it stands** (`SDL_ComposeCustomBlendMode`,
  `SDL_RenderGeometry`, `SDL_TEXTUREACCESS_TARGET` — all in the pinned SDL 2.30.0,
  none called in `src/` yet). The fork should be bought by the first item with no
  such route.
- `SDL_RenderCopyEx` offers translate, uniform scale, rotate and flip and
  **nothing else** — that is the real limit worth knowing.
- **Cell size is not how you get higher-resolution art.** Halving `Camera::SCALE`
  quadruples the cells in the viewport, re-authors every asset, and retunes every
  physics constant in the project — all of them are stated in cells against a
  scale of 4. Per-asset density is the cheap reading and costs a column in
  `sprites.txt`.
- Terrain is the **floor** of visual density, because its resolution is the
  simulation's. Mixed-resolution scenes have everything else denser than the
  ground and never the reverse.
- `src/ui/` is immediate-mode against `SDL_Renderer` with a hand-authored bitmap
  font covering only the glyphs actually needed. No Dear ImGui, no `SDL_ttf`.
  Hotbar icons take their colours from `MATERIALS` so an icon cannot drift from
  the palette it depicts — keep it that way.
