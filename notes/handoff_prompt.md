# Handoff — the gate question is still unanswered, and the tree is uncommitted.

*Written 2026-08-24, at the close of the session that took the tester's answers
on `V25` and moved the hotbar.*

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

**`ROADMAP.md`'s `▶️ The plan` block, at the top of the file.** It is the only
part of that file that has to be re-read to know what to do, and it is the
authority on both the order and the argument. The rest of `ROADMAP.md` is a
search target, not a read.

Where it stands: **`V25` is built and the question it exists to answer is still
open after an eighth asking.** The answers came back on 2026-08-24 but were
given against the `empty` scene, which has no terrain — and `V25` tints terrain.
`MANUAL_TESTING.md` now opens with the five questions and the setup step that
makes them answerable; [PLAYTEST_LOG.md](../PLAYTEST_LOG.md) session 14 has what
came back and why two of the four answers cannot be about `V25` at all.

**The right next action is still almost certainly not to build anything.** That
was true when the previous handoff said it and the intervening session did not
change it — the gate question has now been asked eight times and answered
"no" seven, and the eighth is the first aimed at a different mechanism. Spending
another item before it returns is how the previous four were spent.

**One thing did get settled and it narrows the search.** Asked to look at the
plane with nothing in front of it, the tester reported it reads as a receding
plane, with no join and no tiling seam in motion. Session 13 had already moved
the fault off the plane and onto the join; session 14 removed the other side of
that join and the remaining side came back clean. **Both sessions now bracket
the junction from opposite sides** — so whatever is left is at the join, not on
either surface. If the eighth answer is still "no", the `V25` entry's closing
bullets name what it knowingly did not do and where each belongs; `V19 4c` still
owns the tile's detail-energy ramp and the mist band at the plane's far edge.

**A direction arrived that is recorded and not started.** The scene is to be
built up from hand-made layers drawn in Pixquare and LibreSprite. It is in
`VISION.md` under Project Goals, with the answer that prompted it, and
`ASSETS.md` carries the one pipeline trap. **Do not start it** — the deferral
rule in `VISION.md` is the one this project has never bent.

## What a fresh reader gets wrong

- **`MANUAL_TESTING.md` opens with bare questions now, and that shape is
  deliberate and permanent (2026-08-24).** The questions and their setup step
  are the first thing in the file; the reasoning behind them moved below, under
  *Behind the questions*, and none of it was deleted. It changed because the
  setup step used to sit under several paragraphs and the tester answered from
  the frame that ships instead of the one being asked about. **Do not restore
  the old order**, and when you add a question, put anything it depends on
  *inside* the question block.
- **A tester answer is not automatically an answer to the question it is
  numbered against.** Check what was on screen. Four answers came back numbered
  against the gate question that describe a scene with no terrain in it, and two
  of them are trivially true there.
- **`golden_frame_test` no longer covers the whole shipped frame, and its
  checksum will not move if `V25` breaks.** The near-ground pass runs at the
  texture upload in `main.cpp`, and the golden frame is composed from a texture
  that is already uploaded. This is written down at the top of
  `tests/test_surface_plane.cpp` and in `.claude/rules/simulation.md`; the trap
  is quoting a green checksum as evidence the frame is intact. **Do not close
  the gap by composing inside the golden test** — that is the parallel
  compositor the same rule refuses.
- **`OVERLAY_GOLDEN` moves whenever the world moves, so only a moved
  `OVERLAY_GOLDEN` beside an *unmoved* `GOLDEN` says the UI changed.** That
  combination is what the hotbar move produced this session and it is the
  expected reading, not a defect.
- **`plane_probe`'s `plane` column changed meaning on 2026-08-23 and readings
  taken across that line are not comparable.** It mapped screen row to tile row
  linearly, which is the degenerate case of the relation the renderer actually
  uses, so it reported the plane at rows the draw never puts there. Any number
  quoted from that column in an entry dated earlier is describing a frame that
  was never on screen. The correction is argued at the call site.
- **The near ground's brightness is a render-side tint, not the art.**
  Regenerating `assets/backdrop_ground.bmp` changes the plane and *also* changes
  what the terrain is tinted toward, because the pass samples that tile. If you
  are chasing a value in the lower fifth of the frame, `TUNING.md` has the row —
  it is not reachable by a `Grade`, and that is the point of it.
- **The location is a row in `assets/scenes.txt`, not literals in `main.cpp`**,
  with `F7` cycling between rows in the running game. The format is
  `src/scene/scene_list.h`. **`empty` is currently the only live row and the
  `fixture` row is commented out at the bottom of that file** — uncommenting it
  is the whole restore. Older entries in `ROADMAP.md` still describe the old
  arrangement as current; they are dated and correct as written.
- **A *new* file in `assets/` is not staged until `SlopPhysics` relinks.** The
  existing rule covers *edited* files; a newly added one behaves the same way and
  the failure is quieter, because the loader falls back and the game boots
  looking normal. `python tools/load_sprite.py --stage` is the fix.
- **`.rec` files survived `V25`.** `input_log::fingerprint` hashes cell state and
  the pass writes only pixels, so `P4`'s replayed row is live. The trap is the
  other direction and it is unchanged: *any* change to a cell's colour
  invalidates every recording. Check before asking the tester to play.
- **Check `git status` before assuming the tree is clean.** Commits here are
  asked for rather than taken, and **the documentation half of
  2026-08-24 is uncommitted as this is written** — the `MANUAL_TESTING.md`
  restructure, the session 14 record, and the `VISION.md` / `ASSETS.md` entries
  for the hand-made-layers direction. The code half of that day (the hotbar move,
  its checksum, and the scene archive) went in as `cd4b5a1`.

## Owed to a human

**Five questions, and they are the first thing in `MANUAL_TESTING.md` — that
file is the authority, not this line.** In outline: the gate question a ninth
time, against `V25`, **with the `fixture` row uncommented first**. That last
clause is the whole reason the eighth asking produced nothing usable.

You cannot run the Manual Tester Checklist yourself. Put an item on that list the
moment you ask for one, take it off the moment it comes back, and write it so a
beginner can act on it without reading the rest of the file. **Two askings have
now been lost to avoidable causes** — the seventh because `V25` landed on top of
it the same day and changed the frame it asked about, the eighth because the
scene it needed was not the scene that ships. A gate question and a change to
the thing it asks about should not be in flight together, and a question that
needs a non-default setup has to carry that setup in the question.

There is also an unrun visual check the tester has not been asked for: **the
hotbar moved to the top-right corner this session** and no human has looked at
it in a window. It is not on the owed list because it is not worth spending a
tester on by itself — fold it into the next launch.

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
  *into* the page on it. Measured in `ENTRY 14`; it is what `V25` is built
  against, and the reference's own mechanism there is a mirror, which a ground
  cannot copy.
- **The scene is ultimately to be built up from hand-made layers**, drawn in
  Pixquare on iPad and LibreSprite on PC. Recorded in `VISION.md`, **not
  started**.
- **The game supports both day and night.** Neither is the default, and because
  the depth ordering inverts between them, **day/night and the band ladder are
  one feature.** Do not author a daylight palette ahead of the ladder.
- **Never grow a parallel compositor in the golden-frame test.**
- **Do not attempt band separation as a PALETTE edit** — the ladder **between**
  bands belongs in the grades. Raising the **ceiling all the bands hang from** is
  allowed, because a `Grade` is a multiply and can only darken. **A ramp *within*
  one band is always the art's job.** `V25` is the first change none of the three
  could serve and the first to write pixels; the test that keeps that from
  becoming a loophole is in `.claude/rules/assets-and-formats.md`.
- **The shipped scene is the test scene.** There will never be a separate
  test-only world. *(The `empty` scene is an instrument for looking at the
  backdrop, not a second world to tune against — nothing is authored in it.)*
- **Do not author a near silhouette.** That row is a refusal, not an omission.
  Note that this and the foreground vignette are not the same thing and the
  distinction is load-bearing: the refusal is about paint occluding *the world
  the player digs*.

## Verification, every step

```bash
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure    # all 19, ~1s
python tools/validate_palette.py assets/test_albedo.bmp  # after regenerating art
.\build\Release\plane_probe.exe                          # the band ladder, before and after V25
```

There is no reason to run a subset. **`golden_frame_test` is the only suite that
links SDL2-static**; it needs no display. **It hashes software rasterisation and
is blind to a GPU-only defect — do not quote it as covering the shipped frame**,
and since `V25` it does not reach the near-ground pass at all.

**`docs_test` will fail if a number in the docs goes stale, including the
checklist's step count.** It counts lines matching `^[0-9]+\. \*\*` in
`MANUAL_TESTING.md` and cross-checks the word against three files, so **a
numbered list added to that file in that exact format silently becomes a
"checklist step"**. The questions block avoids it by writing `**1.**` instead;
keep it that way or update all three prose claims.

**`plane_probe` prints two instruments and you need both.** The band ladder says
what the frame came out at; the census under it says how much of the near band
the pass reached. A hole in the second reads as a slightly-low flat number in the
first, which is exactly the shape that gets explained away as a tuning problem.

**`validate_palette.py` is an audit rather than a gate**, and it earned that
place by catching off-palette pixels in a generated asset that every suite was
happy with. Run it after any generator change.

**A green `ctest` in the same command as a failed build is meaningless** — it ran
the previous exe. Check the build line before reading the test line.

**Text-encoding traps have now cost seven sessions.** `MANUAL_TESTING.md`,
`CLAUDE.md`, `.claude/rules/assets-and-formats.md`, `notes/` and this file are
LF; almost everything else is CRLF, and `PLAYTEST_LOG.md` is mixed. Read with
universal newlines, write each file back in the endings it already had, pass
`encoding="utf-8"` at every `open`, and **verify the edit landed rather than
trusting the exit code**. Three shapes, each of which produces a silent no-op or
a shell error with nothing useful in it:

- A file opened without an explicit `encoding=` uses the locale codepage, so two
  byte-identical em dashes compare unequal.
- **A `bash` heredoc mangles backslashes and dies on long content.** A `\n`
  inside a C++ string literal in the patch arrives as a real newline, and a
  script over roughly a hundred lines fails with `unexpected EOF`. **Write the
  patch script to a file and run it by path** — this is the reliable form.
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
