# Handoff — V23 shipped, V22 next, one thing owed to the tester

You are picking up a **visual-rework block** on Toop/Xoco, a from-scratch C++20 /
SDL2 cellular-automata pixel-physics game. Repo root is `code/`; **run every
command from there.** Read `code/CLAUDE.md` first — it is short, and it is the
working agreement rather than a summary.

## The one rule that governs this block

**In this project the reasoning is the deliverable, not a courtesy.** A change
that is correct but arrives without its argument recorded is half-done. A stated
rule that has gone false is worse than no rule, because it stops people checking.
Almost every constant and refusal here has a written argument behind it, usually
naming the bug that bought it — so **before changing something that looks
arbitrary, find out whether it is.**

## Standing constraints, carried verbatim from the user

- **Do not spawn subagents unless asked.**
- **Do not use workflows or deep-research unless asked.**
- **Everything outside this block is held, not cancelled** — E10 keeps the head
  of the queue when the block closes.
- **Never grow a parallel compositor in the golden-frame test.** A legitimate
  frame change puts its new checksum in the *same commit*.
- **Do not attempt band separation as a PALETTE edit** — the refusal is about the
  ladder **between** bands, which belongs in the grades. Raising the **ceiling
  all the bands hang from** is something no grade can do, because `Grade` is a
  multiply and can only darken, and that half is allowed (V20 did it). The full
  test, with V20's honest exceptions, is in
  `.claude/rules/assets-and-formats.md` — read it before touching a backdrop
  colour.
- **The shipped scene is the test scene.** There will never be a separate
  test-only world; a fixture exercise that cannot be dressed into something a
  player would plausibly meet is a fixture exercise that should be visible as
  such. Adopted 2026-08-16, full argument in `ENGINEERING_NOTES.md`.
- **The Manual Tester Checklist is owed and you cannot run it** — flag when it is
  owed and name which steps matter. It lives in `MANUAL_TESTING.md`, not README.
- **`MANUAL_TESTING.md` opens with "Owed to the tester" and that list is
  maintained, not appended to.** Put an item on the moment you ask for one, take
  it off the moment it comes back, and **write it so a beginner can act on it
  without reading the rest of the file.**

## ⚠️ Read this before starting anything

**One thing is owed and V22 should not start until it comes back**
(`MANUAL_TESTING.md`, top of file; the long form is checklist step 13).

V23 is **the largest change to how the game looks that has been made without a
human seeing it first** — the player no longer sits at screen centre, and the
framing moves while digging. The three questions, in the order they matter:

1. Does the movement read as the camera **answering the dig** or as the camera
   **wandering** — and *which* failure, since both get called "nauseating".
2. Standing on the surface, are ~55 cells of world below you **enough to play
   in**.
3. Does the ground plane **finally read as receding**.

**Question 3 is the one that gates V22.** Three attempts have failed at it and
V23 is the first aimed at the cause the geometry actually had. A **"no" is a real
answer and worth more than a polite yes** — it would mean geometry was never the
whole problem, which V22 needs to know *before* it spends a scene rewrite on the
same premise.

Question 1 is the one with no evidence behind it at all: the two anchors are read
off the reference frames, but `EASE_PER_SEC` **came from nothing**. Say so if
asked; do not defend it.

## Where the block stands

`ROADMAP_ITEMS.md` item 8 is the authority; **what `git status` says is the truth
about state, not this table.**

| step | what | state |
|---|---|---|
| 0 | Rewrite the two dead `notes/` files against the CnC reference frames | ✅ done |
| 1 | **V17** — extract composition to `render/frame.cpp`, then checksum it | ✅ done |
| 2 | **V11 core** — ordered layer table, parallax onto `Camera`, generated header | ✅ done |
| 3 | **V11's tint bullet + V7-rest's darkening half** — the light pass gains a multiply | ✅ done |
| 4 | **V19 — the seven-band scene with a ground plane** | 4a, 4b done; V20 and V21 fixed 4b; **4c and 4d queued behind V22** |
| 5 | **V18 — write the split view down, build none of it** | queued |
| 6 | **V23 — the camera's vertical anchor and the dig framing** | ✅ **done 2026-08-17** |
| 7 | **V22 — the plane the player is in** | **next**, gated on the owed feel report |

## V23 — what just shipped, and the one general lesson in it

**V23 was not on the plan yesterday, and how it got there is the reusable part.**
V22's scene work started with a **measurement instead of a redraw**, and the
measurement said the scene was the wrong instrument: `Camera::follow` centred
strictly, so the receding plane's visible band capped at **~50% by construction**
and measured **20.2%** at spawn, against a reference reading of two thirds. An
afternoon of scene authoring would have moved that to about 22% **and invalidated
both `.rec` recordings to do it.** General form: **when the plan is to author
against a target, compute the target's reachability first.**

The mechanism, all of it SDL-free and headless-tested:

- **`Camera` gained a vertical anchor** — `follow` computes
  `center_y - viewport_h * anchor_y_`, and `0.5` is exactly the old expression.
  **It is a fraction, not a bias in cells**, because a cell count expresses a
  composition only at the one viewport height it was tuned at, and
  `DISPLAY_MODES` has several.
- **`src/game/camera_bias.h`** holds the state machine: `SURFACE_ANCHOR` 0.80,
  `DIG_ANCHOR` 0.30, `EASE_PER_SEC` 0.85. Digging sideways or upward
  deliberately does not move it.
- Wired in `main.cpp` **after** the mouse-to-world conversion, and **frozen while
  `debug.free_camera` is detached** (T1 left the same argument on the free
  camera).

### The trap V23 found, which anything reading the cursor can fall into

There is a **positive feedback loop** in this design: the anchor moves the view,
the view resolves the mouse through `screen_to_world_y`, the resolved aim picks
the anchor. **It saturates rather than oscillating**, so it would never have
looked like a bug — it would have looked like the camera slowly crawling away
under a perfectly still hand.

It is cut by measuring the aim in the **unbiased** frame:
`aim_dy + viewport_h * (anchor_now - 0.5f)`. **The live anchor, not the
constant** — `view_fy` carries `-viewport_h * anchor_now`, so passing the current
value cancels exactly, while `SURFACE_ANCHOR` leaves a residue that grows with
camera distance, which is the same loop in a quieter form. I wrote the constant
version first. `tests/test_camera_bias.cpp` pins this with a **negative control**
(`"the still-hand check can actually fail"`) proving the uncorrected form
disagrees — a test that only passes on the fixed code proves less than a pair.

### The checksum

Now **`0xf29c435ed9d923b1`** (seventh move). **The house "no-op half first"
procedure was available and was taken**: the whole mechanism shipped at identity
in `8b77e33` against the standing `0xcde4dc1a39927fca`, which **held as
predicted**, and only then did the wiring move it in `bf295dd`. The golden
fixture calls `set_vertical_anchor(CameraBias::SURFACE_ANCHOR)` so **the checksum
covers the composition that actually ships** — that is the null-texture lesson in
a second form.

### What V23 did not cost

**`Input::cursor_x/y` are stored in world cells**, so no camera change can
invalidate a `.rec`. Both recordings survive and P4's replayed row is still lit.
The scene is untouched; launch prints `334901`, matching `FIXTURE_SCENE_CELLS`.

## Next step — V22, the plane the player is in

The decision behind it is closed and **must not be re-asked**: the plane stays
**land**, and what transfers from the reference is the **relationship**, not the
material — the boat is *in* the receding plane, not in front of it, and **a band
you stand before cannot recede around you at any value.** Full closure in
`ROADMAP_ITEMS.md`'s Decisions-owed table.

Two halves, and V23 built neither of them:

- **The world row's grade.** In the reference the plane is *brighter* than what
  stands on it; ours runs the other way with `cells` at 1.00. This was filed as
  optional polish and the land decision **promoted it to the mechanism itself**.
  It is a `TUNING.md` row **and a playtest**, and it must not be settled inside a
  commit that is about something else. The junction between the world's surface
  and the plane's near end is the one join in the frame nobody has ever tuned.
- **The fixture-scene rewrite.** Dressing, not deletion — stairs become terrain,
  the pit becomes a cave mouth, the channel becomes a river, spread along the
  world rather than piled at the spawn. **This is the step that costs both
  recordings**: `assets/test_material.bmp` is the first of three invalidators in
  `src/game/input_log.h`, `tests/test_scene.cpp` pins `FIXTURE_SCENE_CELLS =
  334901` so it fails in `ctest` rather than in a benchmark nobody runs, and
  `bench_grid` (two call sites) and `rim_probe` load the fixture too. **P4's
  replayed row goes dark until the tester plays and presses `F9`** — flag it
  before spending it, and note `F9` overwrites `session.rec` on the first save of
  a launch.

Then **4c** (far range, near ridge, treeline — the treeline is the one band
authored warm, and a warm world spends the player's cool-against-warm isolation
twice, so check the two against each other), then **4d**, the value ladder tuned
once all seven bands can be judged together. **Do not author a near
silhouette** — that row is a refusal, not an omission: paint in front of the
world would occlude the one verb the game has.

**The ordering lesson has now paid three times in this block:** 4c behind V22
because V22 moves the junction 4c would be authored against; V23 before V22
because the target was unreachable; 4d last because tuning a ladder twice is the
mistake V20 and V21 already made once.

## Also still open

- **`GROUND_STRIPS` is unpriced and cannot be priced by the bench.** `grid_bench`
  times `Grid::update` and `Run::step` and **cannot see a draw call at all**, so
  neither half of the frame-budget rule reaches the plane's 24 strips times their
  tiling copies. The instrument is the frame rate in the running game.
- **V18** (step 5): write the split view down, build none of it. The deliverable
  is a written design.
- **E10 (powders come to rest)** resumes the head of the queue when the block
  closes. Unblocked, deliberately out-prioritised, nothing changed.
- **V7-rest** — the darkening multiply is done; **non-fire light sources are
  untouched**, and that is all it ever was besides the multiply. The ID stays
  because four documents cite it.
- **Combat is decided: yes, deferred**, and **the deferral is part of the answer,
  not the decision still being open.** It means the ugly enemy (sprite, hitbox,
  contact damage, dies and despawns, about two days), not `S1`.
- **Deferred, small:** generate the backdrop clear colour into
  `backdrop_layers.h`; the dated frozen-snapshot mechanism.

## Verification, every step

```bash
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure    # 14 suites, ~0.8s, all of them
```

There is no reason to run a subset. **The suite is 14 as of V23** —
`camera_bias_test` is new and links no sources. `golden_frame_test` is the last
of them and the only one that links SDL2-static; it needs no display. **It hashes
software rasterisation and is blind to a GPU-only defect — do not quote it as
covering the shipped frame.**

**A green `ctest` in the same command as a failed build is meaningless** — it ran
the previous exe. That happened here once, from a **CRLF/LF mismatch in an
exact-match Python replace** that silently no-op'd on a CRLF file while working on
an LF one. Check the build line before reading the test line.

**`assets/` is copied next to the exe at build time.** A generator's output shows
nothing until a rebuild or `python tools/load_sprite.py --stage`. First thing to
check when a change "didn't show up".

**`main.cpp` prints the launch check** — `World seed: N` and
`Scene: WxH, N cells placed`. Read those, not the window; a scene count of zero
once meant a blank world that every suite passed on.
