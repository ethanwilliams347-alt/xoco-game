# Handoff — `V22` is built end to end and waiting on the tester's eye.

*Written 2026-08-22, at the close of the session that shipped `V22` part 3.*

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

Where it stands: **all three parts of `V22` are built, and the item is still
open**, because what it is for — does the world read as standing *in* the
receding plane — is a question only the tester can answer, and it has come back
"no" five times. **The right next action is almost certainly not to build
anything.** Read the `V22` entry's part 3 bullet, then wait for the answer, which
is on `MANUAL_TESTING.md`.

If the answer says the junction still reads as two objects, part 3's own entry
names the two things it knowingly did not fix and where each one lives — do not
re-derive them, and do not reach for the `cells` grade (see below). **`V19 4c` is
the largest thing left in the V block** and it is where the near-black foreground
belongs.

## What a fresh reader gets wrong

- **The `cells` grade is not the knob `V22` turns, and `TUNING.md` said it was
  for six days.** The row inferred the junction's direction from two grade
  values without reading the art underneath them; measured, the plane's near end
  was luminance 65.7 against the world's 23.3, the opposite of what the row
  assumed, so turning it as written would have deepened the inversion it was
  meant to fix. It is corrected in place rather than deleted, because the defect
  it named was real. **`TUNING.md` owns this; read the corrected row before
  touching any grade at that junction.**
- **The foreground band the reference spends a third of its frame on is a
  vignette, not a bottom band.** It runs up both edges and across the top, and
  in every reference frame it is *occluding matter at a nearer depth* — never a
  cutaway of the ground the character is standing on. Building it as a bottom
  strip would be the wrong mechanism at the right value. `ENTRY 12` part 2 in
  `notes/reference_observations.txt` owns this, and it is the constraint on
  `V19 4c`.
- **`Camera::VERTICAL_ANCHOR` cannot be set from the reference reading yet, and
  the trap is a units confusion that nearly landed.** The reference frames put
  the contact point tightly at **60–70% of the visible plane** and loosely at
  **67–85% of the frame** — those are different quantities, and `VERTICAL_ANCHOR`
  is a fraction of the *viewport*. Frame position is a consequence of how much
  foreground sits below the character, so **the anchor is blocked until that
  foreground exists.** `ENTRY 12` part 3 owns it.
- **`assets/test_albedo.bmp` stays.** Playtest session 12 proposed retiring it,
  and the answer went the other way: what read as a shelf was the file's
  *contents*, and the fix needed a per-cell colour to live in. `ASSETS.md` owns
  this.
- **Do not quote "47 luminance levels against the reference's 123" as if it were
  current.** It is `art_direction` section 9's headline number, it has not been
  re-measured since the morning of 2026-08-22, and **`plane_probe`'s band ladder
  cannot measure it** — the ladder excludes everything above the horizon, so it
  cannot see the sky. Re-measuring needs a capture of the running game.
- **The camera is not centred, and this is not `V23` coming back.** `V22` part 1
  put a *fixed* anchor into `Camera::follow` — one constant, no easing, no dig
  trigger, and `camera_bias.h` does not return. What sessions 8 and 9 rejected by
  playing was the anchor **moving**. **Do not re-derive the easing from the
  surviving constant.** The world-edge clamp still overrides the anchor near the
  floor, which `test_camera` pins.
- **`README.md` is a front door and nothing else, as of `W6`.** Architecture is
  in `ENGINEERING_NOTES.md`, bench procedure in `PERFORMANCE.md`. Those moved
  sections are linked **by anchor** from `MANUAL_TESTING.md` and the roadmaps and
  `docs_test` cannot see those links — renaming a heading means fixing its
  callers in the same commit. `.claude/rules/documentation.md` owns this.
- **`W5` has no part 4 and its ~150-line `main()` target is closed unmet**, by
  decision. Do not pick it up as unfinished work.
- **The UI is not in `frame.cpp` and must not go there.** The plan said it would
  join `render/frame.cpp` and **the plan was wrong**; a reticle that goes orange
  near a flame is the defect `frame.h`'s rule names.
- **`golden_frame_test` carries two checksums**, world and post-UI. A legitimate
  change puts the new values in the *same commit*. The pairing is itself
  information — an overlay checksum moving while the world's holds is the only
  combination meaning the **UI** changed.
- **Some of the docs' numbers are a test.** `docs_test` asserts the suite count,
  `Element`'s size, the golden checksums quoted in prose, the scene's cell count,
  the checklist's length and the plan-file sizes. **A failing `docs_test` means a
  document is wrong, not that the check is** — fix the sentence, never the check.
- **Check `git status` before assuming the tree is clean.** Commits here are
  asked for rather than taken, and **`V22` parts 2 and 3 are finished and
  uncommitted as this file is written**, along with a new `session_3_painting_fire.rec`.

## Owed to a human

**Three items, all added 2026-08-22.** `MANUAL_TESTING.md` opens with the
authoritative list and **that file is the authority, not this line**; read it
rather than trusting this paragraph. In outline: the gate question a fifth time,
a fresh recording that includes digging, and a one-line confirmation that the
mid-air spawn is gone.

You cannot run the Manual Tester Checklist yourself. Put an item on that list the
moment you ask for one, take it off the moment it comes back, and write it so a
beginner can act on it without reading the rest of the file. **Note what the
recording request cost to get right**: `input_log::fingerprint` hashes `e.color`,
so *any* change to cell colour invalidates every `.rec` and takes `P4`'s replayed
row dark. It was deliberately not asked for until the colour work had landed, so
the tester was spent once instead of twice. **Check that the same trap is not
open before asking again.**

## Standing constraints, carried verbatim from the user

- **Do not spawn subagents unless asked.**
- **Do not use workflows or deep-research unless asked.**
- **Everything outside the current block is held, not cancelled.**
- **Make it look almost exactly like the WnC reference frames**, and **the scene
  is to have the same layer system** as `resources/images/WnC_marsh.jpg` and
  `WnC_walking (1..4).jpg`. Measured, not eyeballed: `ENTRY 12`.
- **The game supports both day and night.** Neither is the default, and because
  the depth ordering inverts between them, **day/night and the band ladder are
  one feature.** Do not author a daylight palette ahead of the ladder.
- **Never grow a parallel compositor in the golden-frame test.**
- **Do not attempt band separation as a PALETTE edit** — the ladder **between**
  bands belongs in the grades. Raising the **ceiling all the bands hang from** is
  allowed, because a `Grade` is a multiply and can only darken. **A ramp *within*
  one band is always the art's job**, and `V22` part 3 is now the second worked
  example. The full test is in `.claude/rules/assets-and-formats.md`.
- **The shipped scene is the test scene.** There will never be a separate
  test-only world.
- **Do not author a near silhouette.** That row is a refusal, not an omission.
  Note that this and the foreground vignette are not the same thing and the
  distinction is load-bearing: the refusal is about paint occluding *the world
  the player digs*.

## Verification, every step

```bash
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure    # all 17, ~1s
python tools/validate_palette.py assets/test_albedo.bmp  # after regenerating art
.\build\Release\plane_probe.exe                          # the band ladder at the spawn
```

There is no reason to run a subset. **`golden_frame_test` is the only suite that
links SDL2-static**; it needs no display. **It hashes software rasterisation and
is blind to a GPU-only defect — do not quote it as covering the shipped frame.**

**`validate_palette.py` is an audit rather than a gate, and it earned that place
on 2026-08-22** by catching 25,480 off-palette pixels in a generated asset that
every suite was happy with. Run it after any generator change.

**A green `ctest` in the same command as a failed build is meaningless** — it ran
the previous exe. Check the build line before reading the test line.

**Text-encoding traps have now cost five sessions and one more this one.**
`MANUAL_TESTING.md` and this file are LF; almost everything else is CRLF. Read
with universal newlines, write CRLF files back as CRLF, pass `encoding="utf-8"`
at every `open`, and **verify the edit landed rather than trusting the exit
code**. Two specific shapes, both of which produce a silent no-op or a shell
error with nothing useful in it: a script piped in on stdin is decoded as UTF-8
while a file opened without an explicit `encoding=` uses the locale codepage, so
two byte-identical em dashes compare unequal; and a `bash` heredoc containing
Python triple-quotes or backticks can die with `unexpected EOF`. **Write the
patch script to a file and run it by path.**

**`assets/` is copied next to the exe at build time.** A generator's output shows
nothing until a rebuild or `python tools/load_sprite.py --stage`. First thing to
check when a change "didn't show up" — and it now applies to the world's colours,
not just to sprites.

**`main.cpp` prints the launch check** — `World seed: N` and
`Scene: 1920x1080, 334501 cells placed`. Read those, not the window; a scene
count of zero once meant a blank world that every suite passed on. Launch after
any change to the shell and confirm it still prints, with nothing on stderr.
