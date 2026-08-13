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
fail the build. Read the **1920x1080** block; the 960x540 block is a historical
control and should reproduce the numbers on record. If it stops matching, the
refactor broke something rather than the engine.

Compare only within one sitting, back to back, with a **control scenario the
change cannot affect**. A confident 28% on this project once turned out to be the
compiler re-laying-out the hot loop; a later reading moved two scenarios that
contained no fire at all after a fire-only change. See `PERFORMANCE.md`.
