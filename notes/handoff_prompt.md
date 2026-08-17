# Handoff — start the W track (the workbench). V22 is blocked on a human.

You are picking up Toop/Xoco, a from-scratch C++20 / SDL2 cellular-automata
pixel-physics game. Repo root is `code/`; **run every command from there.** Read
`code/CLAUDE.md` first — it is short, and it is the working agreement rather than
a summary.

## The one rule that governs everything here

**In this project the reasoning is the deliverable, not a courtesy.** A change
that is correct but arrives without its argument recorded is half-done. A stated
rule that has gone false is worse than no rule, because it stops people checking.
Almost every constant and refusal here has a written argument behind it, usually
naming the bug that bought it — so **before changing something that looks
arbitrary, find out whether it is.**

## Start here: the W track

**`ROADMAP_ITEMS.md` item 9 is the authority on the order; ROADMAP.md's
`🛠️ W — The workbench` section is the authority on why.** Opened 2026-08-17 out
of an external review of the repo. Six items, run in this order:

| | what | size |
|---|---|---|
| **W1** | Reflow `ROADMAP.md`, `ROADMAP_ITEMS.md`, `ENGINEERING_NOTES.md`, `PERFORMANCE.md` to **80 columns**. Pure formatting — no wording, no number, no decision changes | afternoon |
| **W2** | `.claude/settings.json` with a permission allowlist for `cmake`, `ctest`, `git`, `python tools/*`. The repo has no settings file at all today | afternoon |
| **W3** | A **fifteenth `ctest` suite** asserting the docs' checkable numbers against their sources — suite count, `Element` size, the golden checksum, `FIXTURE_SCENE_CELLS` | afternoon |
| **W4** | **One live plan file**, shipped rationale to a dated `ROADMAP_ARCHIVE.md`. Includes rewriting the `CLAUDE.md` routing-table row that causes the split | days |
| **W5** | **Extract `main()`** — a `boot` unit, the per-frame composition into `render/frame.cpp`, a `main()` of ~150 lines | days |
| **W6** | Trim `README.md` to a front door — architecture to `ENGINEERING_NOTES.md`, benchmark procedure to `PERFORMANCE.md` | afternoon |

**W1 is genuinely first and it is genuinely mechanical.** It is not the most
valuable item; it is the one with no design risk, and it makes every grep in
every later item cheaper. **W3 is the most valuable and is deliberately third**,
because W1 and W4 both move the lines it would pin, and pinning them twice is the
mistake V20 and V21 already made one level down.

**The three numbers that admitted the track**, so you do not have to re-derive
them: docs are **1.09 MB** against 1.20 MB of `src/` + `tests/` + `tools/`; a
measured `grep -C2` into `ROADMAP.md` returned **9,930 bytes for 20 lines**
because it averages 394 characters to the line; and **all 48 roadmap item IDs
appear in both roadmap files**, which the routing table mandates rather than
carelessness producing.

**What this track is not.** The volume of writing is *not* the defect. The review
checked `CLAUDE.md`'s most falsifiable claim — "14 suites" — against
`ctest -N` and found it **true**, and concluded the documentation discipline here
is the project's real strength. Every W item changes **where reasoning is stored
or how it is retrieved**. **None of them is licence to record less.**

**Do not re-open these; they were examined and are healthy.** `grid.cpp` is 1,664
lines of 35 named methods averaging 47 — that is subject matter, not sprawl. The
14 separate test executables, the `.claude/rules/` three-way split, `TUNING.md`
and `PLAYTEST_LOG.md` are all load-bearing separations. `main.cpp`'s
`#include <random>` is the world seed and is exactly where the determinism
invariant draws its line.

## ⚠️ One thing is owed, and it is what unblocks the V track

**Play V23 and report how the camera feels** — `MANUAL_TESTING.md`, top of file;
the long form is now **checklist step 13**, written 2026-08-17. **V22 must not
start until it comes back**, and that is the whole reason the W track goes first:
V22 is blocked on a human and none of W1–W6 are blocked on anything.

V23 is **the largest change to how the game looks that has been made without a
human seeing it first** — the player no longer sits at screen centre, and the
framing moves while digging. Three questions, in the order they matter:

1. Does the movement read as the camera **answering the dig** or as the camera
   **wandering** — and *which* failure, since both get called "nauseating".
   `EASE_PER_SEC` **came from nothing**; say so if asked, do not defend it.
2. Standing on the surface, are ~55 cells of world below you **enough to play
   in**.
3. Does the ground plane **finally read as receding**.

**Question 3 is the one that gates V22.** Three attempts have failed at it and
V23 is the first aimed at the cause the geometry actually had. A **"no" is a real
answer and worth more than a polite yes** — it would mean geometry was never the
whole problem, which V22 needs to know *before* spending a scene rewrite on the
same premise.

## When the report comes back: V22, the plane the player is in

The decision behind it is closed and **must not be re-asked**: the plane stays
**land**, and what transfers from the reference is the **relationship**, not the
material — the boat is *in* the receding plane, not in front of it, and **a band
you stand before cannot recede around you at any value.** Full closure in
`ROADMAP_ITEMS.md`'s Decisions-owed table. Two halves, neither built:

- **The world row's grade.** Ours runs the opposite way to the reference, with
  `cells` at 1.00. It is a `TUNING.md` row **and** a playtest, and it must not be
  settled inside a commit about something else. The junction between the world's
  surface and the plane's near end is the one join in the frame nobody has tuned.
- **The fixture-scene rewrite.** Dressing, not deletion — stairs become terrain,
  the pit a cave mouth, the channel a river, spread along the world rather than
  piled at spawn. **This is the step that costs both `.rec` recordings.**
  `tests/test_scene.cpp` pins `FIXTURE_SCENE_CELLS = 334901` so it fails in
  `ctest` rather than in a benchmark nobody runs; `bench_grid` (two call sites)
  and `rim_probe` load the fixture too. **P4's replayed row goes dark until the
  tester plays and presses `F9`** — flag it before spending it.

Then **4c** (far range, near ridge, treeline — the treeline is the one band
authored warm, and a warm world spends the player's cool-against-warm isolation
twice), then **4d**, the value ladder tuned once all seven bands can be judged
together. **Do not author a near silhouette** — that row is a refusal, not an
omission: paint in front of the world would occlude the one verb the game has.

## Standing constraints, carried verbatim from the user

- **Do not spawn subagents unless asked.**
- **Do not use workflows or deep-research unless asked.**
- **Everything outside the current block is held, not cancelled** — **E10 keeps
  the head of the queue** when the V and W blocks close.
- **Never grow a parallel compositor in the golden-frame test.** A legitimate
  frame change puts its new checksum in the *same commit*.
- **Do not attempt band separation as a PALETTE edit** — the refusal is about the
  ladder **between** bands, which belongs in the grades. Raising the **ceiling all
  the bands hang from** is something no grade can do, because `Grade` is a
  multiply and can only darken, and that half is allowed (V20 did it). The full
  test is in `.claude/rules/assets-and-formats.md` — read it before touching a
  backdrop colour.
- **The shipped scene is the test scene.** There will never be a separate
  test-only world. Adopted 2026-08-16, full argument in `ENGINEERING_NOTES.md`.
- **The Manual Tester Checklist is owed and you cannot run it** — flag when it is
  owed and name which steps matter. It lives in `MANUAL_TESTING.md`, not README.
- **`MANUAL_TESTING.md` opens with "Owed to the tester" and that list is
  maintained, not appended to.** Put an item on the moment you ask for one, take
  it off the moment it comes back, and **write it so a beginner can act on it
  without reading the rest of the file.**

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
exact-match Python replace** that silently no-op'd on a CRLF file while working on
an LF one. Check the build line before reading the test line. **This one matters
more than usual for W1**, which is a bulk rewrite of four files by script.

**`assets/` is copied next to the exe at build time.** A generator's output shows
nothing until a rebuild or `python tools/load_sprite.py --stage`. First thing to
check when a change "didn't show up".

**`main.cpp` prints the launch check** — `World seed: N` and
`Scene: WxH, N cells placed`. Read those, not the window; a scene count of zero
once meant a blank world that every suite passed on. **`N` is 334901.**
