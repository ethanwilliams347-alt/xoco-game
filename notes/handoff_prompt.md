# Handoff — `V25` is built and waiting on the tester's eye; `T2` is uncommitted.

*Written 2026-08-23, at the close of the session that shipped `V25` and `T2`.*

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

Where it stands: **`V25` is built and the question it exists to answer is open.**
It is the first attempt at the tester's recurring "a separate shelf in front of a
backdrop" complaint that is not a value change — the defect turned out to be
composition, and `notes/reference_observations.txt` `ENTRY 9` had said so a week
before four value items were built against it anyway. Read the `V25` entry, then
**wait for the answer**, which is on `MANUAL_TESTING.md`.

**The right next action is almost certainly not to build anything.** The gate
question has now come back "no" seven times and the eighth asking is the first
one aimed at a different mechanism; spending another item before it returns is
how the previous four were spent.

If the answer is still "no", the `V25` entry's closing bullets name what it
knowingly did not do and where each belongs — do not re-derive them. `V19 4c`
still owns the tile's detail-energy ramp and the mist band at the plane's far
edge.

## What a fresh reader gets wrong

- **`golden_frame_test` no longer covers the whole shipped frame, and its
  checksum will not move if `V25` breaks.** The near-ground pass runs at the
  texture upload in `main.cpp`, and the golden frame is composed from a texture
  that is already uploaded. This is written down at the top of
  `tests/test_surface_plane.cpp` and in `.claude/rules/simulation.md`; the trap
  is quoting a green checksum as evidence the frame is intact. **Do not close
  the gap by composing inside the golden test** — that is the parallel
  compositor the same rule refuses.
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
- **The location is no longer three literals in `main.cpp`.** It is a row in
  `assets/scenes.txt`, with `F7` cycling between rows in the running game. The
  format is `src/scene/scene_list.h`. Two documents asserted the literals until
  this session; both are corrected, but older entries in `ROADMAP.md` still
  describe the old arrangement as current — they are dated and correct as
  written.
- **A *new* file in `assets/` is not staged until `SlopPhysics` relinks.** The
  existing rule covers *edited* files; a newly added one behaves the same way and
  the failure is quieter, because the loader falls back and the game boots
  looking normal. `python tools/load_sprite.py --stage` is the fix. This cost
  time this session.
- **`.rec` files survived `V25`.** `input_log::fingerprint` hashes cell state and
  the pass writes only pixels, so `P4`'s replayed row is live. The trap is the
  other direction and it is unchanged: *any* change to a cell's colour
  invalidates every recording. Check before asking the tester to play.
- **Check `git status` before assuming the tree is clean.** Commits here are
  asked for rather than taken, and **`T2` is finished and uncommitted as this
  file is written** — the scene list, the empty scene, and the two defects it
  surfaced in the reset path.

## Owed to a human

**One item.** `MANUAL_TESTING.md` opens with the authoritative list and **that
file is the authority, not this line**; read it rather than trusting this
paragraph. In outline: the gate question an eighth time, against `V25`.

You cannot run the Manual Tester Checklist yourself. Put an item on that list the
moment you ask for one, take it off the moment it comes back, and write it so a
beginner can act on it without reading the rest of the file. **The seventh asking
was never answered** — `V25` landed on top of it the same day and changed the
frame again, so an answer would have been about a frame that no longer exists.
That is a cost worth not repeating: a gate question and a change to the thing it
asks about should not be in flight together.

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
  test-only world. *(The `empty` scene added by `T2` is an instrument for looking
  at the backdrop, not a second world to tune against — nothing is authored in
  it.)*
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

**`plane_probe` prints two instruments and you need both.** The band ladder says
what the frame came out at; the census under it says how much of the near band
the pass reached. A hole in the second reads as a slightly-low flat number in the
first, which is exactly the shape that gets explained away as a tuning problem —
that happened this session and the census exists because of it.

**`validate_palette.py` is an audit rather than a gate**, and it earned that
place by catching off-palette pixels in a generated asset that every suite was
happy with. Run it after any generator change.

**A green `ctest` in the same command as a failed build is meaningless** — it ran
the previous exe. Check the build line before reading the test line.

**Text-encoding traps have now cost six sessions and one more this one.**
`MANUAL_TESTING.md`, `CLAUDE.md`, `.claude/rules/assets-and-formats.md`,
`notes/` and this file are LF; almost everything else is CRLF, and
`PLAYTEST_LOG.md` is mixed. Read with universal newlines, write each file back in
the endings it already had, pass `encoding="utf-8"` at every `open`, and **verify
the edit landed rather than trusting the exit code**. Three shapes, each of which
produces a silent no-op or a shell error with nothing useful in it:

- A file opened without an explicit `encoding=` uses the locale codepage, so two
  byte-identical em dashes compare unequal.
- **A `bash` heredoc mangles backslashes and dies on long content.** A `\n`
  inside a C++ string literal in the patch arrives as a real newline, and a
  script over roughly a hundred lines fails with `unexpected EOF`. **Write the
  patch script to a file and run it by path** — this is the reliable form, and it
  was relearned the hard way twice this session.
- `open(path, 'w')` truncates before the write that fails. **Encode to bytes
  first, write to a temporary path, then `os.replace`.** A script that did not
  do this destroyed `ROADMAP.md` in a previous session.

**`assets/` is copied next to the exe at build time.** A generator's output shows
nothing until a rebuild or `python tools/load_sprite.py --stage` — and see the
newly-added-file trap above, which is worse because it fails quietly.

**`main.cpp` prints the launch check** — `World seed: N`, then a `Scene:` line
naming the scene and its cell count, then the objective and the prop count. Read
those, not the window; a scene count of zero once meant a blank world that every
suite passed on. Launch after any change to the shell and confirm it still
prints, with nothing on stderr.
