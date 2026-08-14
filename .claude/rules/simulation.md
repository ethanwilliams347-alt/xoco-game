---
paths:
  - "**/src/physics/**"
  - "**/src/game/**"
  - "**/src/main.cpp"
  - "**/tests/**"
  - "**/CMakeLists.txt"
---

# Working in the simulation

Loaded when touching `src/physics/`, `src/game/`, `main.cpp`, the tests or the
build file. The short invariant list is in `CLAUDE.md`; this is the detail behind
it and the traps that have caught people.

## The test harness

**There is no test framework, by design.** A failing test is a non-zero exit
code, which is all CTest needs. [tests/test_util.h](../../tests/test_util.h)
provides `check(name, ok, detail)` and `report()`; a suite's `main()` ends with
`return report();`. Do not introduce Catch2, GoogleTest or a mocking library —
that is the "zero new dependencies" rule, and this harness has never been the
thing standing in the way of a test.

A new suite is a new `add_executable` + `add_test` pair in `CMakeLists.txt`
linking the **narrowest** source set that compiles it (see the next section).

**Write the failing test before the fix, and verify it against the unfixed
code.** A test written after the fix passes for reasons nobody has checked. Six
of session 1's eight defects lived in code the existing suites could already
reach — they were missed because nobody wrote the test, not because tests could
not see them.

**Assert the right shape of failure.** `props_test` spends most of its checks
asserting a malformed list is rejected *wholesale* rather than parsed with the
bad line skipped. A suite written the obvious way — asserting the good rows
survived — passes on the exact bug it exists to catch. Whenever a loader's
contract is "reject everything", the tests have to say so.

## The source-set split is a guard, not bookkeeping

`CMakeLists.txt` deliberately keeps four variables apart:

- `ENGINE_SOURCES` — what the simulation *is*. SDL-free, headless, integer-only.
- `RENDER_SOURCES` — `light.cpp`, `player_anim.cpp`. Headless and testable, but
  emphatically not simulation: **light must never become a simulation input.**
- `SCENE_PROP_SOURCES` — `props.cpp`, `sprites.cpp`. Text in, records out. Nothing
  in `src/physics/` may read these.
- Everything else (`main.cpp`, `src/ui/`) is SDL-side and builds only into the game.

The point is that the day something in the simulation reaches for light or a
prop, **the mistake has to be written into the build file to compile.** Merging
these variables removes the only thing enforcing the boundary.

## Determinism

- Randomness is [src/physics/random.h](../../src/physics/random.h): a pure
  function of seed, step, cell index and a `Stream` tag. No generator, no state.
- **`Stream` enum values are arbitrary and permanent.** Changing one changes
  every world its seed ever produced. Adding a new decision means adding a new
  tag, never reusing an existing one — two decisions taken about the same cell on
  the same step with the same tag are literally the same number and correlate
  forever.
- Some draws are pinned at **step 0** on purpose (`IgnitionPoint`, colour jitter):
  they are properties of the material at that coordinate, not decisions retaken
  each step. A per-step re-roll of an ignition point just fires on the first frame
  the lowest roll comes up.
- `Grid` is integer-only, and **so is `Player` since F5 (2026-08-12)**. This bullet
  used to read "`Player` uses a float sub-cell remainder and is the known,
  documented exception — reproducible within one binary, not across toolchains",
  and that is left here rather than deleted because the shape of it is worth
  recognising: the exception was correctly identified, correctly written down, and
  then quoted as portable by three separate later items anyway. Writing a
  limitation down does not stop people spending it.
- **Fixed point is `fx` ([src/physics/fixed.h](../../src/physics/fixed.h)), signed
  16.16.** Speeds are cells *per second*, matching TUNING.md; `fx::per_step()` is
  the only way a per-second constant becomes a per-step amount. Two traps:
  `fx::trunc` truncates toward zero and **must not** be "optimised" into `>> 16`,
  which floors (a static_assert holds this); and a constant is built with
  `fx::from_int`/`fx::from_ratio`, never by casting a float literal, since a
  compile-time float fold is as machine-dependent as a runtime one.
- **`DigTool::march` is integer-only since F6 (2026-08-13)** and was the last float
  on the simulation path. Two traps if that code is ever touched: the range test is
  squared (`dx*dx + dy*dy <= RANGE*RANGE`) and must stay squared, since a library
  `sqrt` is a float result; and `div_round` reproduces `std::lround`'s
  **halves-away-from-zero**, which `(a + b/2) / b` does not — C++ integer division
  truncates toward zero, so the naive form rounds the wrong way in two of the four
  quadrants and every positive-only test still passes. The arithmetic is 64-bit
  because `span*span * RANGE*RANGE` leaves 32-bit range at an ordinary distant aim.
- **The one float left on `Player` is `visual_x()`/`visual_y()`**, and it is the
  boundary to the renderer rather than an exception to this rule. Nothing under
  `src/physics/` may read them, and no test asserts on them except the two A1
  regression cases that exist to watch the remainder itself.

## Things that look like bugs and are not

Do not "fix" these without a measured reason; each is recorded with its argument.

- **`Element::updated_tag` wraparound.** One byte, so a cell asleep for an exact
  multiple of 256 steps is skipped for one step. Known, harmless, cheaper than a
  wider counter.
- **`piece_tag` wraparound welding two unrelated pieces.** That is a *missed*
  fracture, which is the harmless direction — the same asymmetry
  `MAX_SUPPORT_CELLS` is chosen on.
- **Digging destroys matter.** External write, correct for a tool, deliberately
  outside the conservation invariant.
- **The player is invisible to the grid**, so material falls through it and
  `resolve_overlap` relocates the body. Displacement was asked and answered *no*
  (E4, playtest session 5), which makes the unstuck search permanent load-bearing
  machinery rather than a stopgap.
- **Player feel is raw on purpose** — no acceleration, friction, air control,
  coyote time or jump buffering. Do not add them as polish.

## `Element`'s free bytes

**There are three, at offsets 1–3, and they only work if the field is declared
between `type` and `color`.** That is the alignment hole `uint32_t color` forces;
a `uint8_t` appended after `piece_tag` instead costs **four** bytes per cell,
because 13 rounds up to 16. Both facts were measured on 2026-08-13 with
`velocity_probe`, not counted — this struct's size has been got wrong twice by
counting fields, and `element.h` carries a claim that it was already full with the
correction printed next to it.

E5a's per-cell velocity is spoken for in all three (signed 4.4 per axis, plus a
nibble of sub-cell remainder per axis). **After that the struct is genuinely
full**, and the `static_assert` at `<= 12` is what says so — it is also what makes
the front hole safe on an ABI that packs differently, since a layout without one
fails the build rather than silently growing.

**`Element::ticks` is not a velocity and never becomes one.** It has two roles,
`tick_role()` asserts them, and a third was planned for two months and rejected on
a measurement: whole cells per step cannot hold `Player::GRAVITY`'s 5/36 of a cell
per step, so it truncates to zero and a thrown grain never falls. See
ENGINEERING_NOTES.md before proposing it again — the two obvious rescues are
recorded there with the numbers that killed them.

## Adding to the data tables

`MATERIALS` and `REACTIONS` are data; adding a material is a row, not a change to
the update loop. Density decides what sinks through what, so oil floating on
water is not special-cased.

The hazard of data-driven design is that the danger moves into the *relationships
between rows*, which have no compiler behind them unless one is written. That is
what the `static_assert`s at the bottom of `element.h` and `reaction.h` are — a
default-constructed `Element` must carry `Empty`'s colour from `MATERIALS`, and
`Fire` must not be structural because it shares `ticks` with the fall clock.
**When adding a column or a row, ask what relationship it creates and assert it.**

There is deliberately **no hardness or indestructible column**. It would be a
second axis with no consumer. E3's fracture asks a *stress* question, not a
hardness one; if its implementation genuinely reaches for a per-material strength
number, that is the signal — until then it has not arrived.

## Measuring

`grid_bench` is not a test and must not become one — a slow machine should not
fail the build.

**The replayed row (P4) is the one the frame-budget rule is aimed at**, and it is
the only row whose input is a file: `session.rec`, one `Input` per fixed step,
written by `F9` in the game. It times a whole `Run::step` rather than
`Grid::update`, so **do not compare it with the rows above by division**. If it
prints "not run", the row is *absent*, not zero. The bench refuses a log recorded
against a different fixture scene; a differing *end state* is reported instead,
because a changed simulation and a stale log look identical from there and only a
person knows which happened.

**Read the `contents` census before quoting the timing above it.** The row prints
what the session contained — inputs counted exactly from the log, world materials
sampled once a second in a **second, untimed replay pass** (a census evicts the
cache, so sampling inside the timed loop would perturb p99 and worst, the two
statistics the rule reads; determinism is what makes a second pass free and
exact). **Sampled means presence, not absence** — `never seen` is the instrument
not catching it, not proof it was absent. The first recorded session read 0 of
24,437 steps over budget and the census showed why: no digging, no moving sand or
water, 16 of 510 chunks awake. **A played row is realistic by construction and
representative only by evidence**, and P4's whole design ran those together.

**The two kinds of row do different jobs and a merge reading needs both.** The
first recorded session (2026-08-13) reads 0.12 ms mean with 0 of 24,437 steps
over budget, so "under 10% on the replayed row" is 10% of twelve microseconds —
under this benchmark's noise, and blind to a per-awake-cell cost that only bites
under load. **The replayed row is the authority on whether the budget is broken**
— read p99 and steps-over-budget, never the mean — **and the synthetic rows are
the authority on whether a change costs anything at all.** Numbers and the
correction in `PERFORMANCE.md`.

Recording is always on from step 0 and `F9` writes what has been played — a log
that starts mid-session cannot be replayed, since the replay can only rebuild the
world as it was before the first step. Read the **1920x1080** block; the 960x540 block is a historical
control. **What it controls is the awake-chunk and peak-fracture counts, not the
times** — the bench runs on a fixed seed, so those counts are a property of the
simulation and a refactor that changes what the engine does moves them, while the
machine cannot. This bullet used to say the block "should reproduce the numbers
on record", which asked for exactly the cross-sitting timing comparison the rule
below forbids; corrected 2026-08-13 in `PERFORMANCE.md`, where the counts that did
reproduce are listed.

Compare only within one sitting, back to back, with a **control scenario the
change cannot affect**. A confident 28% on this project once turned out to be the
compiler re-laying-out the hot loop; a later reading moved two scenarios that
contained no fire at all after a fire-only change. See `PERFORMANCE.md`.
