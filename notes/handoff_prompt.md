# Handoff — `V22` is in progress. A playtest is owed before part 2.

*Written 2026-08-18, at the close of the session that started `V22` and shipped
its part 1.*

You are picking up Toop/Xoco, a from-scratch C++20 / SDL2 cellular-automata
pixel-physics game. Repo root is `code/`; **run every command from there.** Read
`code/CLAUDE.md` first — it is short, and it is the working agreement rather than
a summary.

**This file is a pointer, not a summary.** Every fact below is owned by another
document, and where the two disagree the other document wins. Its only job is to
tell you *where to start* and *what a fresh reader would get wrong*; the reason
it says so little about the work itself is `W4`, which spent a day removing a
duplicate plan and would be undone by growing one here. The rule that governs all
of it: **in this project the reasoning is the deliverable, not a courtesy.** A
change that is correct but arrives without its argument recorded is half-done,
and a stated rule that has gone false is worse than no rule, because it stops
people checking — so **before changing something that looks arbitrary, find out
whether it is.**

## Start here

**`ROADMAP.md`'s `▶️ The plan` block, at the top of the file.** It is the only
part of that file that has to be re-read to know what to do, and it is the
authority on both the order and the argument. The rest of `ROADMAP.md` is a
search target, not a read.

Where it stands: **`V22` is open and part 1 of three is done.** The previous
session did what the last handoff asked — read the session 8 note, formed a view,
and found the item's stated two-thirds target arithmetically unreachable under
the centred camera `V23b` restored. That was put to the user, who **decided on
2026-08-18 to run `V22` in full, over the stated concern**, and that decision is
what reopened the framing. Part 1 is the camera; **parts 2 and 3 are not
started.**

**Do not open part 2 by starting work.** Read the `▶️ Started 2026-08-18` bullet
at the top of the `V22` entry and the session 8 note below it first: part 2 is
the fixture-scene rewrite, it is the load-bearing half, and it is the step that
invalidates both `.rec` recordings. **A playtest of part 1 is owed, and its
answer can move what part 2 would be authored against** — which is why part 2 is
not already underway. **`E10` keeps the head of the queue** once the V block
closes.

## What a fresh reader gets wrong

- **The camera is not centred any more, and this is not `V23` coming back.**
  `V22` part 1 put a *fixed* anchor into `Camera::follow` — one constant, no
  easing, no dig trigger, and `camera_bias.h` does not return. What sessions 8
  and 9 rejected by playing was the anchor **moving**; a moving anchor is a
  rejected feel, a fixed anchor is a composition. **Do not re-derive the easing
  from the surviving constant.** The argument is at the constant in
  `src/game/camera.h`, the number is owned by `TUNING.md`, and the history is in
  `ROADMAP.md` under `V22` and under `V23`/`V23a`/`V23b`. The anchor also states
  an intent rather than a guarantee: the world-edge clamp still overrides it near
  the floor, which is what `V23a` shipped past a green suite and what
  `test_camera` now pins.
- **`README.md` is a front door and nothing else, as of `W6`.** The engine
  architecture is in `ENGINEERING_NOTES.md` and how to run the bench is in
  `PERFORMANCE.md`; README links out and describes neither. The test for a new
  paragraph there is whether another document already owns the topic.
  **Those moved sections are linked by anchor** from `MANUAL_TESTING.md` and the
  roadmaps, and `docs_test` cannot see those links — renaming one of those
  headings means fixing its callers in the same commit.
  `.claude/rules/documentation.md` owns this.
- **`W5` has no part 4 and its ~150-line `main()` target is closed unmet**, by
  decision on 2026-08-18. Do not pick it up as unfinished work; the argument for
  why length was the wrong target is at the end of the `W5` entry, and reopening
  it needs a concrete change that the wiring in `main()` makes hard.
- **The UI is not in `frame.cpp` and must not go there.** `W5` part 3 put the
  HUD, reticle, hotbar, run-over wash and settings screen in their own
  translation unit, drawn after the composition returns. The plan said they would
  join `render/frame.cpp` and **the plan was wrong** — `frame.h`'s rule is about
  the light pass, and a reticle that goes orange near a flame is the defect it
  names. `.claude/rules/assets-and-formats.md` owns the rule.
- **`golden_frame_test` carries two checksums**, one for the composed world and
  one taken after the UI is drawn over it. Both follow the same rule: a
  legitimate change puts the new value in the *same commit*. **Both moved on
  2026-08-18** with the framing. The pairing is itself information — an overlay
  checksum that moves while the world checksum holds is the only combination that
  means the **UI** changed.
- **A checksum covers drawing and is blind to wiring.** The UI look that closed
  on 2026-08-18 was asked for precisely because the second checksum hashes a
  fixture the test builds, not the values `main.cpp` puts into the struct — a
  field wired to the wrong variable draws perfectly and says something false.
  **Carry the shape of that, because it will recur: when an extraction moves a
  decision behind a seam, ask what the number on the far side cannot see.**
- **Some of the docs' numbers are a test.** `docs_test` asserts the suite count,
  `Element`'s size and free bytes, the golden checksum quoted in prose, the
  scene's cell count, the checklist's length and the plan-file sizes against the
  code and files they come from. **A failing `docs_test` means a document is
  wrong, not that the check is** — fix the sentence, never the check. Part 2 will
  fail it on the scene's cell count by design; that is the check working.
- **A line count written from a guess got shipped as a measurement** and
  survived a session before being caught. It is corrected in place in the `W5`
  entry rather than deleted. Measure before writing a number into a document.
- **`ROADMAP_ITEMS.md` no longer exists.** Older documents cite it by name in
  their own historical entries; those are correct as written. A *live* link to
  it is a defect and `docs_test` fails on one.
- **Check `git status` before assuming the tree is clean.** Commits here are
  asked for rather than taken, and **`V22` part 1 is finished and uncommitted as
  this file is written.**

## Owed to a human

**One item, added 2026-08-18: does the new framing earn what it costs?**
`MANUAL_TESTING.md` opens with the authoritative list and **that file is the
authority, not this line**; read it rather than trusting this paragraph. The
short version is that the player now sits low on screen rather than centred,
which buys the composition room above and takes most of the world that used to be
visible below the feet — so the question is a trade, and **"put it back" is a
fair answer.** It is one number in `TUNING.md`.

You cannot run the Manual Tester Checklist yourself. Put an item on that list the
moment you ask for one, take it off the moment it comes back, and write it so a
beginner can act on it without reading the rest of the file. Note that **part 2
will owe a second one** — the fixture-scene rewrite invalidates both recordings,
so P4's replayed row goes dark until the tester plays and presses `F9`. Flag that
cost before spending it, not after.

## Standing constraints, carried verbatim from the user

- **Do not spawn subagents unless asked.**
- **Do not use workflows or deep-research unless asked.**
- **Everything outside the current block is held, not cancelled.**
- **Never grow a parallel compositor in the golden-frame test.** A legitimate
  frame change puts its new checksum in the *same commit*.
- **Do not attempt band separation as a PALETTE edit** — the refusal is about the
  ladder **between** bands, which belongs in the grades. Raising the **ceiling
  all the bands hang from** is something no grade can do, because `Grade` is a
  multiply and can only darken, and that half is allowed (V20 did it). The full
  test is in `.claude/rules/assets-and-formats.md` — read it before touching a
  backdrop colour.
- **The shipped scene is the test scene.** There will never be a separate
  test-only world. Adopted 2026-08-16, full argument in `ENGINEERING_NOTES.md`.
- **Do not author a near silhouette.** That row is a refusal, not an omission:
  paint in front of the world would occlude the one verb the game has.

## Verification, every step

```bash
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure    # all of them, ~1s
```

There is no reason to run a subset. **`golden_frame_test` is the only suite that
links SDL2-static**; it needs no display. **It hashes software rasterisation and
is blind to a GPU-only defect — do not quote it as covering the shipped frame.**
The suite count is checked by `docs_test`, which deliberately does **not** check
this file, since the handoff is rewritten whole at every session close.

**A green `ctest` in the same command as a failed build is meaningless** — it ran
the previous exe. That happened here once, from a **CRLF/LF mismatch in an
exact-match Python replace** that silently no-op'd on a CRLF file while working
on an LF one. Check the build line before reading the test line. **The same
mismatch has now cost four sessions.** `MANUAL_TESTING.md` and this file are LF;
almost everything else is CRLF. Read with universal newlines, write CRLF files
with `newline="\r\n"`, and **verify the edit landed rather than trusting the exit
code**.

**A second decoding trap in the same family cost an edit this session, and it is
worth knowing because the symptom is identical.** A script piped in on stdin is
decoded as UTF-8 while a file opened without an explicit `encoding=` is decoded
with the locale codepage, so two byte-identical em dashes compare unequal and an
exact-match replace fails with nothing to see. Write the script to a file, pass
`encoding="utf-8"` at every `open`, and prefer index-based slicing to
exact-match replace. The same shape sank a `bash` heredoc whose terminator
arrived with a stray carriage return.

**`assets/` is copied next to the exe at build time.** A generator's output shows
nothing until a rebuild or `python tools/load_sprite.py --stage`. First thing to
check when a change "didn't show up".

**`main.cpp` prints the launch check** — `World seed: N` and
`Scene: WxH, N cells placed`. Read those, not the window; a scene count of zero
once meant a blank world that every suite passed on. **`N` is 334901, and part 2
is expected to change it** — when it does, the new value goes into the same
change, in `tests/test_scene.cpp` and in every document `docs_test` checks.
Launch after any change to the shell and confirm it still prints, with nothing on
stderr.
