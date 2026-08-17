# Handoff — W3 is next. Nothing is owed to the tester.

*Written 2026-08-17, at the close of the session that shipped `W1`, `W2` and
`W4`.*

You are picking up Toop/Xoco, a from-scratch C++20 / SDL2 cellular-automata
pixel-physics game. Repo root is `code/`; **run every command from there.** Read
`code/CLAUDE.md` first — it is short, and it is the working agreement rather than
a summary.

**This file is a pointer, not a summary.** Every fact below is owned by another
document, and where the two disagree the other document wins. Its only job is to
tell you *where to start* and *what a fresh reader would get wrong*; the reason
it says so little about the work itself is `W4`, which spent a day removing a
duplicate plan and would be undone by growing one here.

## The one rule that governs everything here

**In this project the reasoning is the deliverable, not a courtesy.** A change
that is correct but arrives without its argument recorded is half-done. A stated
rule that has gone false is worse than no rule, because it stops people checking.
Almost every constant and refusal here has a written argument behind it, usually
naming the bug that bought it — so **before changing something that looks
arbitrary, find out whether it is.**

## Start here

**`ROADMAP.md`'s `▶️ The plan` block, at the top of the file.** It is the only
part of that file that has to be re-read to know what to do, and it is the
authority on both the order and the argument — as of `W4` there is no second
plan file. The rest of `ROADMAP.md` is a search target, not a read.

Where it stands: **`W3` is next** and unblocked. It is the fifteenth `ctest`
suite, asserting the docs' checkable numbers against their sources, and it is
third in the W track on purpose — `W1` and `W4` both moved the lines it pins, and
pinning them twice is the mistake V20 and V21 already made one level down. Both
of those have now shipped, so the dependency is discharged. **Include the two
numbers `W4` created** — the live-plan and archive sizes — since new numbers are
the ones most likely to drift.

Then `W5` (extract `main()`; the one W item that needs the tester afterwards) and
`W6`. After the W block, the V track resumes at **V22**, and **`E10` keeps the
head of the queue** when the V and W blocks both close.

## What a fresh reader gets wrong

- **The camera work is gone.** `V23` and `V23a` shipped on 2026-08-17 and `V23b`
  deleted them the same day at the tester's direction — the player is centred
  again, `camera_bias.h` and its suite no longer exist, and the golden checksum
  is back to its pre-V23 value. Do not plan around a moving camera. **The one
  thing that survived is a constraint on `V22`**, and it is written into the
  `V22` entry: centring caps the receding plane's visible band at ~50% by
  construction, and it measured 20.2% at the spawn.
- **The ground plane has come back "no" four times.** `V22` is next in the V
  track and its premise is weaker than when it was written. Read the session 8
  note in its entry before spending a week on it.
- **`V22`'s fixture-scene rewrite costs both `.rec` recordings**, which darkens
  P4's replayed row until the tester plays and presses `F9`. Flag it before
  spending it.
- **`ROADMAP_ITEMS.md` no longer exists** (`W4`, 2026-08-17). Older documents
  cite it by name in their own historical entries; those are records of what was
  true then and are correct as written. A *live* link to it is a defect.

## Owed to a human

**Nothing.** `MANUAL_TESTING.md` opens with the authoritative list and it is
currently empty; the camera questions that sat there for two days were withdrawn
rather than answered when `V23b` removed their subject. **That file is the
authority, not this line** — check it rather than trusting this paragraph.

You cannot run the Manual Tester Checklist yourself. Put an item on that list the
moment you ask for one, take it off the moment it comes back, and write it so a
beginner can act on it without reading the rest of the file.

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
ctest --test-dir build -C Release --output-on-failure    # 14 suites, ~0.8s, all of them
```

There is no reason to run a subset. **The suite is 14** — `golden_frame_test` is
the last of them and the only one that links SDL2-static; it needs no display.
**It hashes software rasterisation and is blind to a GPU-only defect — do not
quote it as covering the shipped frame.** *(W3 raises this to 15. If you have
done W3 and this line still says 14, that is exactly the failure W3 exists to
catch, and it should have caught itself.)*

**A green `ctest` in the same command as a failed build is meaningless** — it ran
the previous exe. That happened here once, from a **CRLF/LF mismatch in an
exact-match Python replace** that silently no-op'd on a CRLF file while working
on an LF one. Check the build line before reading the test line. **The same
mismatch cost this session an hour**, in `W4`'s merge script, and the second
occurrence is why it is still here: all three roadmap files are CRLF on disk, so
a script that reads them must read with universal newlines and write with
`newline="\r\n"`.

**`assets/` is copied next to the exe at build time.** A generator's output shows
nothing until a rebuild or `python tools/load_sprite.py --stage`. First thing to
check when a change "didn't show up".

**`main.cpp` prints the launch check** — `World seed: N` and
`Scene: WxH, N cells placed`. Read those, not the window; a scene count of zero
once meant a blank world that every suite passed on. **`N` is 334901.**
