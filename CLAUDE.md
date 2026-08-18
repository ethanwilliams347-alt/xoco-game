# Toop / Xoco — working name

A from-scratch C++20 cellular-automata pixel-physics engine with an SDL2 shell,
built solo, aimed at a Steam release. `code/` is the repo root: **run every
command and every `tools/` script from here.**

<!-- Maintainer note: this file is context, loaded every session. Keep it under
     ~200 lines. Anything that is a multi-step procedure, or only matters in one
     part of the tree, belongs in .claude/rules/ instead. -->


## The one thing to know before changing anything

**In this project the reasoning is the deliverable, not a courtesy.** Almost
every constant, refusal and structural split here has a written argument behind
it, usually naming the bug that bought it. A change that is correct but arrives
without its reasoning recorded is half-done, and a change that quietly
contradicts a recorded decision is a regression even if every test passes.

So: **before changing something that looks arbitrary, find out whether it is.**
Grep the docs and the header comment at the constant first. The recurring failure
this project has actually suffered is not bad code — it is a stated rule that
stopped matching the code and kept being believed.

Corollary, and it applies to this file too: **a claim here that has gone false is
worse than no claim**, because it stops people checking.

## How to answer me

**End with a `**TL;DR**` whenever the reply contains a plan, an edit, or a
question for me** — three or four lines, plain, no jargon I would have to look
up. Answers to a question I asked, and status that changes nothing, do not need
one.

It exists because of the rule directly above: the reasoning *is* the deliverable
here, so replies are long by design, and the TL;DR is what makes them long
safely. So it summarises, it does not replace — **anything only in the TL;DR is
not recorded**, and a decision, a number or a caveat belongs in the body and in
the doc that owns it. Lead the TL;DR with what changed or what I have to decide,
and always keep the things I have to act on: what is owed to me (the Manual
Tester Checklist above, a playtest, a decision left open) and anything that
failed or went unmeasured.

## Commands

Multi-config Visual Studio generator, so `--config` / `-C` is required, not
optional.

```bash
cmake -B build -S .                          # first time; fetches SDL2 2.30.0
cmake --build build --config Release         # also copies assets/ next to the exe
ctest --test-dir build -C Release --output-on-failure
.\build\Release\SlopPhysics.exe
```

**The full suite is 17 suites and runs in about a second**, of which sixteen are
headless and link no SDL; **`golden_frame_test` is the one that links SDL2-static**
(V17, 2026-08-16). It still needs no display — it composes into a software
renderer over an `SDL_Surface` — but "the tests do not link SDL" stopped being
true that day, and this line is where it says so. There is no
reason to run a subset — always run all of it.

**The numbers in this paragraph are a test rather than a discipline** (`W3`,
2026-08-17). `docs_test` is the sixteenth suite and it asserts the docs'
checkable numeric claims — this suite count, `Element`'s size and its three free
bytes, the golden checksum quoted in prose, `FIXTURE_SCENE_CELLS`, the Manual
Tester Checklist's length and the two plan-file sizes — against the code and the
files they are drawn from. **A number here that goes stale now fails `ctest`.**
It can only ever check a claim with a machine-readable source; it cannot check
reasoning, and making it try would turn these documents into a format.

Not part of `ctest`, run by hand, each answering a question a pass/fail cannot:

```bash
.\build\Release\grid_bench.exe        # timings; read the 1920x1080 block, then the replayed row
.\build\Release\preview_light.exe     # dumps a frame; pipe through tools/rawpng.py
.\build\Release\burn_probe.exe        # burn timing and shape, in numbers
.\build\Release\water_probe.exe       # where poured water ends up
.\build\Release\rim_probe.exe         # surviving rim highlight after settling
.\build\Release\velocity_probe.exe    # what Element's spare bytes can hold
```

## Gotchas that have each cost a session

- **`assets/` is copied next to the exe at build time.** Editing a file in
  `assets/` by hand changes nothing until you rebuild or run
  `python tools/load_sprite.py --stage`. This is the first thing to check when a
  change "didn't show up".
- **Two headers are generated and editing either is overwritten work.**
  `src/render/player_sprite.h` by `python tools/player_sheet.py --header`, and
  **`src/render/backdrop_layers.h` by `python tools/generate_backdrop.py
  --header`** (V11, 2026-08-16 — the parallax factors used to exist in both C++
  and Python with a comment asking a human to keep them in step, and the failure
  that arrangement produces is a seam at the far edge of the world).
- **`main.cpp` is the project's one nondeterministic line.** It prints
  `World seed: N` and `Scene: WxH, N cells placed` at startup; **those printed
  counts are the launch check, not eyeballing the window.** A scene count of zero
  once meant a blank world that all six suites passed on.
- **A count taken before the last thing that can fail measures a different
  quantity than its label claims.** That is a general rule here, learned from
  `Props: 10 of 10 placed` printing before the planting scan could drop one.
- Windows-only so far. The build is portable and uses no platform-specific code,
  but macOS and Linux have never been built — do not claim they work.
- **`grid_bench`'s last row needs a file nobody at a keyboard-less session can
  produce.** It replays a recorded session (`session.rec`, written by pressing
  `F9` in the game) and it is the row the frame-budget rule is aimed at.
  **Absent is not zero** — when it prints "not run", say so rather than quoting
  the seven hand-built rows as if the played frame had been measured. P4 in
  ROADMAP.md; how to record one in README. **Two sessions exist as of 2026-08-13,
  both tracked in git**, so the row runs on a fresh clone — this bullet used to
  say recordings were "not in the repo", which was false.
  `session_2_digging_fluids_steam.rec` is the one to read (digging, fluids,
  steam); `session_1_painting.rec` is the quiet one. **`F9` overwrites
  `session.rec` on the first save of each launch** — that is how session 2 landed
  on session 1, which survived only in a commit. Copy one before playing again.
- **A played row is realistic by construction and representative only by
  evidence.** Those were treated as the same thing for the whole of P4's design,
  and the first recorded session broke the equation: it never dug once, never
  moved a grain of sand or a cell of water, and peaked at 16 of 510 chunks
  awake. **Read the `contents` census under the replay row before quoting the
  timing above it** — that census exists because the number alone was
  uninterpretable, and "the session was really played" would never have shown it.
  Session 2 settled it: **`churning` is not representative of played work.** Read
  that off **p99 and steps-over-budget**, never the census's sampled awake peak
  and **not off `worst` either** — four identical replays in one process spread
  `worst` by 72% while p99 held to 1.2%. A sampled figure cannot bound anything;
  a single largest sample is mostly the operating system.
- **The played row proves the budget is intact; it cannot price a change.** It
  costs 0.12 ms a step, so 10% of it is under the noise floor. A merge reading
  needs both halves: p99 and steps-over-budget on the replayed row, *and* the
  bracketed synthetic rows at 1920x1080. Stated in full in ROADMAP.md's P
  track; this bullet exists because the rule used to name only the first half.

## Invariants — breaking one of these is a defect, not a design choice

Each has a longer argument at the code or in [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md).

- **The simulation is deterministic and integer-only.** No `<random>`, no
  floats, no threading in `Grid`. Randomness is `sim_random`: a pure function of
  seed, step, cell index and a `Stream` tag. **`Stream` values are arbitrary and
  permanent** — changing one changes every world its seed ever produced.
  **`Player` was the known float exception and is not one any more** (F5,
  2026-08-12): its velocities and sub-cell remainder are `fx` 16.16 fixed point
  ([src/physics/fixed.h](src/physics/fixed.h)). The one float left in the body
  is `visual_x()`/`visual_y()`, which exist for the renderer and which nothing in
  `src/physics/` may read. **Since F6 (2026-08-13) no float under `src/physics/`
  reaches the grid** — `DigTool::march` was the last one that did. The two that
  remain are renderer boundaries of the same kind: `visual_x()`/`visual_y()`, and
  `DigTool::swing_progress()`, which only the animation reads. **State it that
  way, not as "no float in `src/physics/`"**, which a grep disproves in two
  seconds and which is how a true claim starts being ignored. Portable *in
  principle*; still only ever built on one machine, and those are different
  claims.
- **All cell writes go through `set_element`, `paint` or `swap_elements`**, and
  the first two delegate to a private `place()`. A fourth write path that
  reimplements `place()` rather than calling it produces material frozen in
  mid-air.
- **Every write wakes its 3x3 neighbourhood.** This is what chunked sleep
  correctness rests on.
- **`Grid::update()` never creates or destroys matter.** Digging deletes matter
  and is correct because it is an *external* write — do not relax the
  conservation test to accommodate it; they test different things.
- **Rendering never becomes a simulation input.** Light, animation and props are
  kept out of `ENGINE_SOURCES` in [CMakeLists.txt](CMakeLists.txt) specifically so
  that the day something in `src/physics/` reaches for one, the mistake has to be
  written into the build file to compile. Do not "tidy" those variables together.
- **Materials are data.** A new material is a row in `MATERIALS`
  ([src/physics/material.h](src/physics/material.h)), not an edit to the update loop.
- **`Element` is 12 bytes and has exactly three free ones, at offsets 1–3.** This
  bullet said "with nothing spare" and that was wrong — the alignment hole between
  `type` and `color` takes up to three `uint8_t`s **declared between them** without
  growing the struct, which the sitting on 2026-08-13 measured after two roadmap
  items had been sequenced around the shortage. E5a's velocity is spoken for
  there; after it the struct really is full. **A byte appended after `piece_tag`
  costs four**, since 13 rounds to 16 — 7.9 MB at 1920x1080. The `static_assert`s
  in [src/physics/element.h](src/physics/element.h) are the guard, and the reason
  the free bytes are safe to use on an ABI that packs differently — never widen one
  to make a build pass.
- **No ECS, no threading, no networking, no scripting layer.** Refused with
  reasons in ENGINEERING_NOTES.md, not merely unbuilt.
- **Zero new dependencies until a specific need cannot be met without one.**

## How work gets done here

1. **Read before writing.** [ROADMAP.md](ROADMAP.md) is the authority on both
   *what is next* and *why*, and is large — start at its **The plan** block,
   which is the only part that has to be re-read to know what to do, and search
   the rest rather than reading it front to back.
2. **A confirmed defect the headless suites can reach gets a failing test before
   its fix** — and the test must be verified against the *unfixed* code, so it
   fails for the right reason.
3. **`ctest` proves mechanics in isolation; it cannot prove they compose.** After
   any change to `src/physics/`, `src/game/` or `main.cpp` that the suites do not
   fully exercise, the Manual Tester Checklist in
   [MANUAL_TESTING.md](MANUAL_TESTING.md) is the other half — it moved out of
   README on 2026-08-16, where `## General Testing` is now the short public
   fundamentals pass and carries none of the reasoning. **MANUAL_TESTING.md opens
   with the list of what is owed to the tester; put an item on it the moment you
   ask for one, and take it off the moment it comes back.** Each of its
   **thirteen** steps names a regression that has actually
   happened — **except step 10, whose first result is a design decision rather
   than a pass or a fail.** Step 13 was the second such step and is not one any
   more: it covered V23's moving camera framing, and **V23b deleted that
   framing on 2026-08-17** when the tester asked for the centred camera back, so
   the step is now an ordinary check that the player stays at mid screen.
   Step 11 (V11's parallax check)
   carried a reopen trigger for a mid-ground band, and **that trigger fired on
   2026-08-16** (V19 4b) — so the step now checks a wrapping ground plane, not
   two static layers, and its new failure modes are a vertical tiling join and a
   stair-step between parallax strips. Step 12 watches the depth grades for
   over-correction; the mountains' 0.60 was looked at and passed the same day,
   and the ground's 0.53 joined it. **I cannot run any of it** — flag when it is
   owed and say which steps matter.
4. **Performance claims need bracketed measurement.** Same sitting, back to back,
   with a control scenario that the change cannot affect. Timings on one machine
   have differed by more than 2x on identical code. See
   [PERFORMANCE.md](PERFORMANCE.md) — it exists mostly to document how this has
   gone wrong before.
5. **Scope discipline is real and has never bent.** Nothing from the Long Term
   list starts before the v0.1 slice ships and playtests as fun. An idea does not
   get built on arrival — it gets written down. See [VISION.md](VISION.md).

## Where a decision gets written down

Putting it in the wrong file is the failure mode; the split is deliberate.

| What you have | Where it goes |
|---|---|
| Sequenced work, and why it is ordered that way | ROADMAP.md — **one file, both halves.** An item's order and its argument go in the same entry |
| A finished item's reasoning, once nothing open depends on it | ROADMAP_ARCHIVE.md — **nothing is ever required to read it** |
| A technical decision made and then deferred, or refused | ENGINEERING_NOTES.md — **and, since `W6`, how the engine works**: the architecture sections moved there out of README |
| A feel constant you retuned | TUNING.md — a row **and** a dated History line |
| A benchmark number or a measurement method, or how to run the bench | PERFORMANCE.md — `W6` moved the running procedure in from README |
| What a playtest asked and what came back | PLAYTEST_LOG.md — **symptoms only**, no fixes |
| Goals, scope, the wish list | VISION.md |
| How to build/run/test; the short public `## General Testing` pass | README.md — **a front door since `W6`**: it links out and carries no architecture and no benchmark procedure |
| How to get art in | ASSETS.md, and `../drawing_to_sprite.md` for the player |
| Raw lore, brainstorming, reference observations | `notes/` |

**`W4` shipped 2026-08-17 and the first two rows are what it left behind.** This
note used to be a warning that the row was wrong: the plan was split into
`ROADMAP.md` (the why) and `ROADMAP_ITEMS.md` (the order), so **every item was
written in two files and thereafter maintained in step** — all 48 IDs appeared
in both, across 582 KB. `ROADMAP_ITEMS.md` is now merged into `ROADMAP.md` and
deleted, and closed work moved to `ROADMAP_ARCHIVE.md`. **File one entry per
item, in `ROADMAP.md`, carrying its own order and its own argument.**

**The archive is not the old "Shipped" section under a new name.** The boundary
is *finished **and** nothing open depends on the reasoning* — so a closed item
whose finding still binds an open one has that finding written **into the open
item** before it moves. `V22` carrying `V23b`'s ~50% camera cap is the worked
example. **Move a finding before you move an entry**, or the archive quietly
becomes required reading and the whole item is undone. The reasoning is in
ROADMAP.md's **W — The workbench** section.

**`notes/handoff_prompt.md` is the exception: do not edit it unless told to.**
It is the last thing written before a session closes, and its job is to let the
next session pick the work up where this one meant to leave it. Read it freely —
it is usually the right file to start from. But it is a *handoff*, not a log:
editing it mid-session turns it into a running commentary on work still moving,
and the next session then starts from a description of a state that never
settled. Everything worth keeping already has a home in the table above; the
handoff points at those, it does not duplicate them. When asked to write it,
write what is needed to continue and nothing else — no recap of finished work
for its own sake. **Its shape is fixed** — four sections, no copied tables or
numbers — and that is written down in `.claude/rules/documentation.md`, which
also records the 2026-08-17 rewrite that the shape came out of.

## The detail lives in `.claude/rules/`

Three path-scoped rule files, loaded only when the matching part of the tree is
being worked on, so this file stays short enough to be obeyed. **Read the
relevant one before a substantial change** if it has not loaded on its own.

- `.claude/rules/simulation.md` — `src/physics/`, `src/game/`, `main.cpp`,
  `tests/`, `CMakeLists.txt`. The test harness, the source-set guard,
  determinism, and the things that look like bugs and are not.
- `.claude/rules/assets-and-formats.md` — `assets/`, `tools/`, `src/render/`,
  `src/scene/`, `src/ui/`. The locked palette, the frozen legend, the rules any
  authored format has to follow, and the renderer's limits.
- `.claude/rules/documentation.md` — the project docs and `notes/`. How to write
  in each one without breaking a convention that was learned the hard way.

## Keeping this file true

Add a line here when a mistake is made twice, or when a rule above turns out to
be stated wrongly. **Delete a line the moment it stops being true** — including
when the project changes direction, which it is expected to do. Anything that is
a multi-step procedure, or matters in only one part of the tree, belongs in a
rule file rather than here.
