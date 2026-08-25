# Handoff — the scene is being rebuilt from hand-made layers, and V12 is next.

*Written 2026-08-24, at the close of the session that retired the `fixture`
scene and reorganised the V track around it.*

You are picking up Toop/Xoco, a from-scratch C++20 / SDL2 cellular-automata
pixel-physics game. Repo root is `code/`; **run every command from there.** Read
`code/CLAUDE.md` first — it is short, and it is the working agreement rather than
a summary.

**This file is a pointer, not a summary.** Every fact below is owned by another
document, and where the two disagree the other document wins. Its only job is to
tell you *where to start* and *what a fresh reader would get wrong*. The rule
that governs all of it: **in this project the reasoning is the deliverable, not a
courtesy.** A change that is correct but arrives without its argument recorded is
half-done, and a stated rule that has gone false is worse than no rule, because
it stops people checking — so **before changing something that looks arbitrary,
find out whether it is.**

## Start here

**`ROADMAP.md`'s `▶️ The plan` block, at the top of the file**, and then the
block at the head of its `V — Visual identity` section, which is where this
session's reorganisation is argued. The plan block is the only part of that file
that has to be re-read to know what to do; the rest is a search target, not a
read.

**Next is `V12` — the asset layer: alpha, and more than one format.** It is
`▶️ Next up` item 10's first step and the reason it leads is mechanical rather
than aesthetic: both drawing tools export PNG with alpha, and today an
alpha-carrying export becomes a BMP whose transparent pixels are black rather
than magenta, so it loads without error and renders as a black box. `V13`
follows it and the argument is in the same block. **This is a build item — the
previous three sessions were not**, and that change is the main thing to know.

**The direction changed on 2026-08-24 and it is now started, not merely
recorded.** The owner retired the `fixture` scene and every scene made before
`empty`, and is rebuilding the scene logic and the fundamental visuals from that
baseline out of hand-made layers drawn in Pixquare and LibreSprite. `VISION.md`
owns the direction, `ASSETS.md` owns the pipeline, and the V-track block owns
the plan it displaced.

**A chain of items is suspended rather than closed, and the distinction is
load-bearing.** `V20`, `V21`, `V22`, `V25` and `V19 4c`/`4d` were all tuning the
*generated* `backdrop_ground.bmp`. Their measurements stay true about that tile,
and **seven of the eight gate askings came back "no" about generated art** —
which is a finding about the mechanism and is the strongest argument the
hand-made direction has. Do not delete them and do not resume them.

## What a fresh reader gets wrong

- **The gate question is withdrawn, and eight askings of it are not a backlog to
  work through.** `MANUAL_TESTING.md` says why in full: those five questions are
  all about terrain, they needed the retired `fixture` scene, and they judge art
  that is being replaced. **Nothing is owed to the tester right now.** The trap
  is reading the reasoning section below them, which still describes the
  generated plane, as instruction rather than history — it is labelled as
  history where it starts.
- **`default_scene_list()` returns `empty` as of 2026-08-24, not the fixture.**
  It is the fallback `main.cpp` uses when `assets/scenes.txt` is missing or
  malformed, so it was quietly the guarantee the game boots at all. The argument
  is at the function. Note what it is *not*: `Spawn::Floor` here is not a
  recovery path, which the refusal beside that enumerator forbids — `empty` is
  *meant* to be empty rather than having failed a terrain scan.
- **Retiring the fixture is mostly unpaid.** The one item settled is the boot
  fallback above. Still standing on it: `FIXTURE_SCENE_CELLS` and `docs_test`,
  `test_boot.cpp`'s fixture half, `plane_probe`, `bench_grid`'s two loaders and
  both `.rec` recordings — so **`P4`'s replayed frame-budget row dies with the
  fixture** — and `rim_probe`. The full bill is in the V-track block. **Read it
  before removing anything**; the ordering rule it records is that a dependency
  gets answered before the removal, not after.
- **A closed decision constrains whatever the new scene becomes.** *"Does the
  spawn serve the fixture, or the plane?"* (2026-08-16, in `Decisions owed`)
  permanently killed a test-only scene separate from the shipped one, and killed
  replacing the fixture *unless the replacement carries every exercise forward*.
  It names five. It is not overturned by the rebuild; its condition is inherited.
- **`plane_probe` measures nothing useful against `empty`.** It is the
  instrument `V22` and `V25` were read with and it needs terrain. Do not quote a
  flat reading from it as a result.
- **`golden_frame_test` no longer covers the whole shipped frame.** The
  near-ground pass runs at the texture upload in `main.cpp` and the golden frame
  composes from a texture already uploaded, so the checksum will not move if
  that pass breaks. Written down at the top of `tests/test_surface_plane.cpp` and
  in `.claude/rules/simulation.md`. **Do not close the gap by composing inside
  the golden test** — that is the parallel compositor the same rule refuses.
- **`OVERLAY_GOLDEN` moves whenever the world moves**, so only a moved
  `OVERLAY_GOLDEN` beside an *unmoved* `GOLDEN` says the UI changed.
- **The near ground's brightness is a render-side tint, not the art.**
  Regenerating `assets/backdrop_ground.bmp` changes the plane and *also* what
  the terrain is tinted toward, because the pass samples that tile. `TUNING.md`
  has the row; it is not reachable by a `Grade`, and that is the point of it.
- **The location is a row in `assets/scenes.txt`, not literals in `main.cpp`**,
  with `F7` cycling rows in the running game; the format is
  `src/scene/scene_list.h`. **`empty` is the only live row** and the `fixture`
  row is commented out at the bottom. Older `ROADMAP.md` entries describe the
  old arrangement as current; they are dated and correct as written.
- **A *new* file in `assets/` is not staged until `SlopPhysics` relinks.** The
  existing rule covers *edited* files; a newly added one fails more quietly,
  because the loader falls back and the game boots looking normal.
  `python tools/load_sprite.py --stage` is the fix. **This one is about to
  matter a lot more**, since hand-made layers arrive as new files.
- **Check `git status` before assuming anything about the tree.** Commits here
  are asked for rather than taken. Everything through this session is in as
  `bbc8bf4`.

## Owed to a human

**Nothing, and that is a deliberate state rather than an oversight.**
`MANUAL_TESTING.md` is the authority and it currently opens by saying so, with
the reasoning for the withdrawal.

You cannot run the Manual Tester Checklist yourself. Put an item on that list the
moment you ask for one, take it off the moment it comes back, and write it so a
beginner can act on it without reading the rest of the file. **Two askings were
lost to avoidable causes** — one because a change to the thing being asked about
landed the same day, one because the question needed a non-default setup that
was not in the question. The next question will be owed by the hand-made layers
and it will not be the old one, because it will be asked of a different kind of
art.

There is an unrun visual check nobody has been asked for: **the hotbar moved to
the top-right corner** and no human has looked at it in a window. It is not on
the owed list because it is not worth a tester by itself — fold it into the next
launch.

## Standing constraints, carried verbatim from the user

- **Do not spawn subagents unless asked.**
- **Do not use workflows or deep-research unless asked.**
- **Everything outside the current block is held, not cancelled.**
- **Make it look almost exactly like the WnC reference frames**, and **the scene
  is to have the same layer system** as `resources/images/WnC_marsh.jpg` and
  `WnC_walking (1..4).jpg`. Measured, not eyeballed: `ENTRY 12`.
- **The ground the player is on is to act like the water in
  `resources/CnC_parallax_*`** — one contiguous plane receding into the
  background and partially coming toward the foreground, with the player sat
  *into* the page on it. Measured in `ENTRY 14`; the reference's own mechanism
  there is a mirror, which a ground cannot copy.
- **The scene is to be built up from hand-made layers**, drawn in Pixquare on
  iPad and LibreSprite on PC. **Started 2026-08-24** — this bullet said "not
  started" until that day and no longer does.
- **The game supports both day and night.** Neither is the default, and because
  the depth ordering inverts between them, **day/night and the band ladder are
  one feature.** Do not author a daylight palette ahead of the ladder.
- **Never grow a parallel compositor in the golden-frame test.**
- **Do not attempt band separation as a PALETTE edit** — the ladder **between**
  bands belongs in the grades. Raising the **ceiling all the bands hang from** is
  allowed, because a `Grade` is a multiply and can only darken. **A ramp *within*
  one band is always the art's job**, and that clause is what the hand-made
  direction is now doing at scale. The test that keeps `V25`'s pixel-writing from
  becoming a loophole is in `.claude/rules/assets-and-formats.md`.
- **The shipped scene is the test scene.** There will never be a separate
  test-only world. *(The `empty` scene is an instrument for looking at the
  backdrop, not a second world to tune against — nothing is authored in it.)*
- **Do not author a near silhouette.** That row is a refusal, not an omission.
  This and the foreground vignette are not the same thing and the distinction is
  load-bearing: the refusal is about paint occluding *the world the player digs*.

## Verification, every step

```bash
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure    # all 19, ~1s
python tools/validate_palette.py assets/<file>.bmp       # after any art lands
```

There is no reason to run a subset. **`golden_frame_test` is the only suite that
links SDL2-static**; it needs no display. **It hashes software rasterisation and
is blind to a GPU-only defect — do not quote it as covering the shipped frame**,
and since `V25` it does not reach the near-ground pass at all.

**`docs_test` will fail if a number in the docs goes stale, including the
checklist's step count.** It counts lines matching `^[0-9]+\. \*\*` in
`MANUAL_TESTING.md` and cross-checks the word against three files, so **a
numbered list added to that file in that exact format silently becomes a
"checklist step"**. Write `**1.**` instead, or update all three prose claims.

**`validate_palette.py` is an audit rather than a gate**, and it earned that
place by catching off-palette pixels in a generated asset that every suite was
happy with. **It is about to be pointed at hand-drawn files rather than
generated ones**, which is a different question — an artist's file failing it is
a decision to make, not automatically a defect.

**A green `ctest` in the same command as a failed build is meaningless** — it ran
the previous exe. Check the build line before reading the test line.

**Text-encoding traps have now cost eight sessions**, the latest on 2026-08-24.
`MANUAL_TESTING.md`, `CLAUDE.md`, `.claude/rules/assets-and-formats.md`, `notes/`
and this file are LF; almost everything else is CRLF, and `PLAYTEST_LOG.md` is
mixed. Read with universal newlines, write each file back in the endings it
already had, pass `encoding="utf-8"` at every `open`, and **verify the edit
landed rather than trusting the exit code**. Three shapes, each producing a
silent no-op or a shell error with nothing useful in it:

- A file opened without an explicit `encoding=` uses the locale codepage, so two
  byte-identical em dashes compare unequal.
- **A `bash` heredoc mangles backslashes and dies on long content.** A `\n`
  inside a C++ string literal in the patch arrives as a real newline, and a
  script over roughly a hundred lines fails with `unexpected EOF`. **Write the
  patch script to a file and run it by path** — and note that writing the script
  *via a heredoc* does not escape this: on 2026-08-24 that turned a `\n` in a
  patched test into a real line break, which compiled and passed. **No suite
  catches it. Read the patched line back.**
- `open(path, 'w')` truncates before the write that fails. **Encode to bytes
  first, write to a temporary path, then `os.replace`.** A script that did not
  do this destroyed `ROADMAP.md` in a previous session.

**`assets/` is copied next to the exe at build time.** A generator's output shows
nothing until a rebuild or `python tools/load_sprite.py --stage` — and see the
newly-added-file trap above, which is worse because it fails quietly.

**`main.cpp` prints the launch check** — `World seed: N`, then a `Scene:` line
naming the scene and its cell count. Read those, not the window. **While `empty`
is the only shipped row the check is the *name* on that line, not the count**: a
declared-empty scene legitimately prints `0 cells placed`, which is the same
reading the blank-world bug produced.
