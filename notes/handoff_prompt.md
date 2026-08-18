# Handoff — `W6` is next. One look is owed to the tester.

*Written 2026-08-18, at the close of the session that shipped `W5`.*

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

Where it stands: **`W6` is next** — trim `README.md` to a front door. Read its
entry for the boundary it draws; the one thing to know going in is that `##
General Testing` stays public and stays short, and that is a written rule rather
than a preference.

`W5` closed its three parts but **left one thing open on purpose, and it is a
decision rather than a task**: its `main()` line-count target is not met, and
whether a fourth part is worth a sitting is argued at the end of the `W5` entry.
The reach argument that justified parts 1 to 3 is spent. Do not start a part 4
without reading that paragraph and forming a view.

After the W block, the V track resumes at **V22**, and **`E10` keeps the head of
the queue** when the V and W blocks both close.

## What a fresh reader gets wrong

- **The UI is not in `frame.cpp` and must not go there.** `W5` part 3 moved the
  HUD, reticle, hotbar, run-over wash and settings screen out of `main.cpp` into
  their own translation unit, drawn by its own call after the composition
  returns. The plan said they would join `render/frame.cpp` and **the plan was
  wrong** — `frame.h`'s rule is about the light pass, and a reticle that goes
  orange near a flame is the defect it names. `.claude/rules/assets-and-formats.md`
  owns the rule; `frame.h` carries the argument.
- **`golden_frame_test` now carries two checksums.** One for the composed world,
  one taken after the UI is drawn on top of it, so a UI change and a sky change
  move different numbers. Both follow the same rule as before: a legitimate
  change puts the new value in the *same commit*. The second has no earlier value
  to be compared against yet, and the comment at the constant says so.
- **Some of the docs' numbers are a test.** `docs_test` asserts the suite count,
  `Element`'s size and free bytes, the golden checksum quoted in prose, the
  scene's cell count, the checklist's length and the plan-file sizes against the
  code and files they come from. **A failing `docs_test` means a document is
  wrong, not that the check is** — fix the sentence, never the check.
  `.claude/rules/documentation.md` owns the rules for writing under it.
- **A line count written from a guess got shipped as a measurement**, and it
  survived a session before being caught. It is corrected in place in the `W5`
  entry rather than deleted. Measure before writing a number into a document;
  `docs_test` cannot check a claim about a file it does not know to look at.
- **The camera work is gone.** `V23`/`V23a` shipped and `V23b` deleted them the
  same day at the tester's direction — the player is centred again. Do not plan
  around a moving camera. **The one thing that survived is a constraint on
  `V22`**, written into the `V22` entry.
- **The ground plane has come back "no" four times.** `V22` is next in the V
  track and its premise is weaker than when it was written. Read the session 8
  note in its entry before spending a week on it.
- **`V22`'s fixture-scene rewrite costs both `.rec` recordings**, which darkens
  P4's replayed row until the tester plays and presses `F9`. Flag it before
  spending it.
- **`ROADMAP_ITEMS.md` no longer exists.** Older documents cite it by name in
  their own historical entries; those are correct as written. A *live* link to it
  is a defect and `docs_test` fails on one.
- **Check `git status` before assuming the tree is clean.** Commits here are
  asked for rather than taken, and work has sat finished-but-uncommitted across a
  session close more than once.

## Owed to a human

**One look, and it is about the UI's wiring rather than its drawing.**
`MANUAL_TESTING.md` opens with the authoritative list and describes what to
check. **That file is the authority, not this line** — read it rather than
trusting this paragraph.

The shape of it is worth carrying, because it will recur: the new checksum covers
a fixture the test builds, not the values the game puts into the struct, so a
field wired to the wrong variable draws perfectly and says something false. **A
checksum passes that.** When an extraction moves a decision behind a seam, ask
what the number on the far side of it cannot see.

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
mismatch cost a session an hour**, in `W4`'s merge script, and it recurred a
third time this session in a script that aborted between its edit and its write,
leaving a document unchanged while reporting success. Read with universal
newlines, write with `newline="\r\n"`, and **verify the edit landed rather than
trusting the exit code**.

**`assets/` is copied next to the exe at build time.** A generator's output shows
nothing until a rebuild or `python tools/load_sprite.py --stage`. First thing to
check when a change "didn't show up".

**`main.cpp` prints the launch check** — `World seed: N` and
`Scene: WxH, N cells placed`. Read those, not the window; a scene count of zero
once meant a blank world that every suite passed on. **`N` is 334901.** Launch
after any change to the shell and confirm it still prints, with nothing on
stderr.
