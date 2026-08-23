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
- **The audit earned its keep on 2026-08-22 and that is worth one line**, since
  the bullets around it are all about why it is *not* enforced. V22 part 3's
  first draft of the terrain ramp interpolated between two palette colours and
  dithered between the results, putting **25,480 pixels in nine unnamed
  colours** into `assets/test_albedo.bmp`. The audit reported it, the pass was
  rewritten to pick only between adjacent *named* rungs, and the measured result
  was the same to within 0.1 of a luminance level. **Run it after regenerating
  any asset**; the generated groups are the half of the palette rule still in
  force, and this is the failure mode they have.
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
- **[src/render/backdrop_layers.h](../../src/render/backdrop_layers.h) is
  generated**, by `python tools/generate_backdrop.py --header`. The factors and
  each layer's image size come out of the same table in that script, so they
  cannot disagree. **This bullet used to read "Known unenforced duplication" and
  described the arrangement V11 replaced on 2026-08-16** — `PARALLAX_SKY_X/Y`
  and `PARALLAX_MOUNTAIN_X/Y` in the C++, `PARALLAX_SKY`/`PARALLAX_MOUNTAIN` in
  the Python, a comment in each asking a reader to grep the other, and a seam at
  the pan limit as the failure. It is recorded rather than deleted because the
  *shape* is worth recognising: two copies of a constant with a comment between
  them is not enforcement, and the failure it permitted was invisible until
  somebody walked to the edge of the map.
- **The header carries each layer's generated size as well as its factors, and
  `main.cpp` warns at startup when the loaded BMP is smaller.** That is the seam
  becoming a printed line. A warning and not a fatal error — an undersized
  backdrop is cosmetic at one extreme of the world.
- **The nearer the band, the larger its file**, because a layer must cover the
  window plus the full pan range at its own factor: sky 3678x1512 (16 MB),
  mountains 4311x1642 (20 MB), and a mid-ground at 0.40 would be 5750x1965
  (32 MB) — more than both together. `python tools/generate_backdrop.py --sizes`
  prints the table; run it before committing to a layer.
- **That relationship now has an exception, and which layers it applies to is
  the thing to get right.** V19 pulled V16's wrapping layers forward on
  2026-08-16 because five new pan-sized bands would have roughly tripled
  `assets/`. **`backdrop_ground` is a tile: 256x256, 0.2 MB, against the 32 MB
  the same band would have cost priced flat.** A wrapping layer has no size
  relationship to the pan range at all — `backdrop_wrap::wrap_axis` says how
  many copies the window needs. So: **a wrapping row must never be sized through
  `layer_size()`**, which encodes the very relationship being retired; the
  generator's `LAYERS` table carries a `tile` field and `--sizes` labels the row
  so nobody reads the number as an image size. The sky and the mountains are
  still pan-sized and the bullet above is still live for them.
- **A mid-ground band was built and removed on 2026-08-16, and the reason is a
  rule about using reference at all.** `notes/reference_observations.txt` entry 4
  found that band doing most of the depth work in five of eight reference frames;
  it also wrote its own disproof condition, which fired on the first played
  frame: **our terrain already fills that space.** A painting has to author a
  mid-ground because nothing else will occupy it; we simulate 800 cells of ground
  into it. **Ask what in the reference is doing the work, not just what the work
  is** — the result transfers only if the mechanism does. Reopen trigger: a
  location whose terrain does not fill the band, or a zoomed-out camera once
  `Camera::SCALE` is runtime.
- **Depth is not bought by adding layers, it is bought by separating values** —
  and as of V11 step 3 (2026-08-16) there is a knob for that. Each row of the
  layer table in [src/render/frame.cpp](../../src/render/frame.cpp) carries a
  `Grade`, a multiply applied with `SDL_SetTextureColorMod`, and the mountains
  sit at 0.60. **This bullet used to end "that needs a multiply the light pass
  does not have yet"**; it has one. The refusal attached to it stands and has
  gained a boundary: **do not attempt band separation as a palette edit**, and do
  not attempt it as a new band either.
  - **V20 (2026-08-16) raised the backdrop palette wholesale, and that is not
    the refused move — the distinction is the whole point of writing this down
    rather than assuming the exemption.** The refusal is about *separating two
    bands from each other* by recolouring one of them, which puts the ladder in
    the art where a grade cannot reach it and makes every later direction change
    an asset re-author. What V20 did is raise the **ceiling all the bands hang
    from**, and it was forced by an asymmetry in the only tool available: `Grade`
    is a multiply, so it can only ever darken, and `sky_deep` was authored at
    luminance **18 of 255**. There was no headroom left to grade *into* — the
    whole frame occupied 9 levels, against the reference's 123 and its *night*
    frame's sky at L 163. The band-to-band separations stayed where they belong:
    `mountains` 0.60 and `ground` 0.53 did not move.
  - **The test for the next time this comes up:** if the change alters the
    *ratio* between two bands, it is the refused move and belongs in a grade. If
    it scales every band and leaves the ratios intact, it is a ceiling, and no
    grade can do it.
  - **V22 part 3 (2026-08-22) is the second worked example of "a ramp within a
    band is the art's job", and it is the one that shows the rule catching a
    mistake before it was made.** TUNING.md's `cells` grade row had been
    standing since 2026-08-16 saying V22 would turn it. It would not have
    worked: the world's art is near-black, so any multiply of it is still
    near-black, and the row had inferred the junction's direction from the two
    grades without reading the art under them. Measured, the plane's near end
    was 65.7 against the world's 23.3 — the opposite of what the row assumed —
    so grading the world *down* would have deepened the inversion. **The fix was
    a ramp authored into `assets/test_albedo.bmp`**, exactly as this bullet
    says, and the grade stayed at identity.
  - **V20 was not purely the second kind and it is worth being exact about
    that**, because a rule stated and then quietly exceeded is the failure this
    project keeps having. Two ratios *did* change in the same commit, both
    inside a single layer rather than between layers, and neither is reachable by
    a grade: the sky's ramp was **inverted** (it ran dark-at-top to bright-at-
    horizon, putting the frame's brightest row directly above the row that has to
    be its darkest), and the ground tile's internal far-to-near ramp was widened
    from 1.83 to 2.7. **A grade is uniform, so a ramp *within* a band is always
    the art's job** — that is already stated three bullets down. The refusal is
    about the ladder *between* bands, and that is what stayed in the grades.
  - The measurement, because it is the thing that makes the knob worth having:
    in luminance the sky averages 26 and the mountains were **flat 28** — two
    levels of separation out of 255, with the far band the brighter. TUNING.md's
    "Depth grading" section has the row and the retune history.
  - **The same reading was then taken on a much larger pair of surfaces and came
    back worse** (V19, 2026-08-16): on the played frame the ground below the
    terrain's skyline is a **flat fill** — luminance spread exactly 0.0 across
    400 rows and 37% of the frame — sitting at 22.7 against the upper sky's
    22.3. **Step 3's measurement could not have caught it**, because it compared
    the two backdrop bands to each other and nothing else. Worth generalising:
    when a value-separation measurement is taken, take it across *every* pair of
    large surfaces in the frame, not the pair the item is about.
  - **A grade cannot make a band recede within itself, and the ground plane is
    the first band that has to.** A multiply is uniform, so the plane's far-to-
    near ramp is authored into the tile and the grade only places the whole band
    on the ladder. Two knobs, two jobs — do not try to buy the ramp with the
    grade.
  - **V25 (2026-08-23) is the first change this rule could not serve, and the
    boundary it draws is the useful part.** The rule above is that band-to-band
    separation belongs in a grade and a ramp within a band belongs in the art.
    V25 is neither: the near terrain had to end up *brighter* than it is —
    luminance 89 against 24 at the bottom band — and **a grade is a multiply, so
    it can only darken**. The art half could not do it either, because the band
    in question is the simulated world, whose colours are per-material data and
    not an authored image. So it is the first thing in the project to **write
    pixels rather than scale them**: `src/render/surface_plane.cpp` blends the
    visible cell window toward the ground tile's value before the upload.
  - **The test that keeps this from becoming a loophole.** A pixel pass is
    allowed only where both of the other two answers are unavailable — the
    change is not a ratio between bands (a grade), and the band has no authored
    image to edit (the art). Terrain is the only band in the frame that meets
    the second condition today. **If a change to the sky, the mountains or the
    ground tile ever reaches for a pixel pass, that is the signal it was a grade
    or an art edit all along.**
  - **It is still rendering, and the guard is unchanged.** The pass reads
    `Grid::get_pixels()` and writes a scratch copy; it never writes back, so no
    cell colour changes and `input_log::fingerprint` — which hashes cell state,
    not pixels — is untouched, and every `.rec` survives. It lives in
    `RENDER_SOURCES` for the same reason `light.cpp` does.
  - **Retuning a grade moves the golden frame checksum**, and the new value goes
    in the same commit. That is expected, not a breakage.
  - **A grade is per-layer, and that is not incidental.** A frame-wide multiply
    scales every band by the same factor and leaves the ratios between them
    exactly where they were, so it cannot separate anything. The world-wide
    `Params::world_grade` exists for night and weather and is a different
    feature; it is identity today and draws nothing at identity.

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
  such route. **V11's grade pass is the first item to test that claim and it
  came back cheaper than the claim allowed for** (2026-08-16): "the light pass
  gains a multiply" looked like the item that would spend
  `SDL_ComposeCustomBlendMode`, and it needed no custom mode at all —
  `SDL_BLENDMODE_MOD` is stock, and `SDL_SetTextureColorMod` handles the
  per-layer half with no extra draw call. All three named escape hatches are
  still unspent. **The software backend supporting both is what made it usable**,
  since the golden frame test rasterises in software and would have been blind to
  a GPU-only path.
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
- **The UI's draw calls are `src/render/overlay.cpp`, and it is a separate
  translation unit from `frame.cpp` on purpose.** `frame.h`'s rule is that
  everything in the composition is in the world and gets lit; the overlay is
  drawn by its own call after `compose` returns and is never a row in the layer
  table. **Do not merge them to save a call** — a reticle that goes orange near a
  flame is defect B1, and that is the shape it comes back in. `golden_frame_test`
  carries a second checksum, `OVERLAY_GOLDEN`, taken after the overlay is drawn
  on top of the golden frame — so the *step* between the two numbers is the UI,
  and a UI change and a sky change cannot move the same one.
- **`overlay::Params` takes strings and flags, never a `Run` or a `Grid`.** What
  the readout *says* is the caller's decision and what it *looks like* is this
  file's; the split is the same one `frame::Params` draws, and it is what lets a
  test build a frame with no window and no simulation.
- `src/ui/` is immediate-mode against `SDL_Renderer` with a hand-authored bitmap
  font covering only the glyphs actually needed. No Dear ImGui, no `SDL_ttf`.
  Hotbar icons take their colours from `MATERIALS` so an icon cannot drift from
  the palette it depicts — keep it that way.
