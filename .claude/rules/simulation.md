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

**T1 added an eleventh suite and the reason generalises.** `debug_test` covers
`src/game/debug_view.h` — the pause, the single-step queue, the free camera's
clamp and the cell inspector's text. Every one of those is a decision with a
wrong answer, and every one of them would have been **unreachable by any test**
had it been written in the SDL event switch where the keys are bound. So: when a
shell feature has a decision in it, the decision goes in an SDL-free header under
`src/game/` and the key binding stays in `main.cpp`. `debug_view.h` is a header
on purpose and deliberately did **not** become a fourth source-set variable,
which is what leaves the guard below intact.

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

- **`CHUNKS:0` while a slab is falling.** `active_chunk_count()` counts what the
  *sweep* will visit, and a falling structural piece is not swept — it is carried
  by `pending_support`, and `resolve_support()` runs **before** `update()` swaps
  the chunk rects, so its writes mark the set that is about to become this step's
  work and the sweep after it adds none of its own (a Static cell does nothing in
  the sweep). **A slab can fall the height of the world with the counter reading
  zero the whole way.** Nothing is broken; the sentence that used to sit on that
  method — "zero means the world has come completely to rest" — was. **"At rest"
  is `active_chunk_count() == 0` *and* `!has_pending_support_checks()`**, and
  anything asking whether the world has settled has to ask both. Found 2026-08-14
  by pointing T1's cell inspector at a falling slab; pinned in `test_debug.cpp`.
  Worth knowing *why* no test caught it for months: **every suite that cares about
  sleep uses powders**, which do all their moving inside the sweep.
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
- **The burn rule sits *above* `resolve_overlap`'s early return in
  `Player::update`, and the fall-damage block sits at the very bottom.** Neither
  is where it landed by accident. Moving the burn check below the return makes
  being dug out of a fire free — a hazard that switches itself off in the
  situation it exists for. The fall check has to be after `on_ground` is
  recomputed, because that is the edge it reads.
- **The first landing of a run does no fall damage** (`Player::has_landed`).
  `Run` spawns the body a quarter of the world up, so the spawn drop reaches
  terminal velocity and is priced at 80 of 100 health. This is not mercy and not
  a magic number; it is the world setup being charged to the player.
- **`Run`'s objective survives `Run::reset`**, the second such exception after
  `Grid::vent_radius` and for a parallel reason — it is a property of the level,
  and the caller re-stamps the same scene. Pinned by a test in `test_run.cpp`.
- **`Run::reset` does not restore the terrain.** `Run` does not own the scene;
  `main.cpp` does. A caller that resets and does not re-stamp gets an empty
  world, which is a sharp edge that is named at `Run::reset` rather than removed.

## S0's damage rules, and the one trap in them

**Fall damage keys on `on_ground` going false→true, never on `move_y` reporting
a block**, and this is the mistake to not re-introduce. A body falling six cells
a step onto a floor exactly six cells below walks the whole distance unblocked,
lands flush with its velocity intact, and has `vel_y` zeroed by the resting rule
on the *next* step — so the block never happens. Written the obvious way,
terminal-velocity falls do no damage whenever the arithmetic comes out even.
Both forms were built and measured: 100 health against the naive one, 20 against
the correct one, in `player_test`. Same shape as D1's two clocks.

**Four relationships between the damage constants and constants elsewhere are
`static_assert`s at the bottom of `player.h`**, not comments — a standing jump
must never hurt (against `JUMP_SPEED` and `GRAVITY`), the worst landing must be
survivable and must cost something (against `MAX_FALL_SPEED` and `MAX_HEALTH`),
and the burn threshold must sit between Steam's spawn temperature and Fire's
(against two rows of `MATERIALS`). The last is the live one: MATERIALS has
already moved Steam's spawn once, for an unrelated reason, and a move back up
would silently make a doused fire a weapon.

**Health is on the body and there is no health on `Grid` or `Element`.** S0 is
the first gameplay system spanning both sides of the rule `tool.cpp` set, and it
keeps the direction: damage is the player *asking* what it is standing in, the
same way collision does. The grid still does not know a player exists.

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

**Two sessions exist and both are tracked in git** — `session_1_painting.rec` and
`session_2_digging_fluids_steam.rec`. Pass the path as an argument; with none,
the bench reads `session.rec`. **`F9` overwrites `session.rec` on the first save
of each launch** (the "second save does not overwrite" counter is per-process), so
copy a recording to a descriptive name before playing again — session 1 was
overwritten this way and recovered from a commit.

**`churning` is settled as of 2026-08-13: not representative of played work.**
Session 2 drove sand into water deliberately and its worst step of 20,415 was
4.83 ms, against `churning`'s 37.25 ms/step sustained at 360 of 510 chunks awake.
**Note which statistic did that.** The census's awake-chunk peak (44 of 510) is
sampled every 60 steps and **cannot bound anything** — a one-second spike hides
between two samples. The timing is not sampled: every step is measured, so a step
at `churning`'s load would have had to hide from the clock. **When a sampled
figure and a per-step figure support the same sentence, write it against the
per-step one.** `churning` keeps its row — it is still the authority on sustained
liquid churn, and nothing about this demotes it to noise.

**`worst` is the least stable number the replay prints, and the paragraph above
originally leaned on it.** Replaying one session four times in a single process,
on identical code and identical input, gives means within 0.3% and p99s within
1.2% — and worsts of 4.82, 4.84, 4.86 and 8.29 ms. **One sample out of 20,415
catches whatever the OS was doing at that instant.** The `churning` conclusion
survives it (a factor of eight is far outside a 72% band) but the general rule
gains a clause: prefer the per-step statistic over the sampled one, **and prefer
the distribution over its single largest sample.** p99 and steps-over-budget are
what the frame-budget rule names, and they were the right pair all along.

**`VENT_RADIUS` is `Grid::set_vent_radius` since 2026-08-13**, and the bench
sweeps r=0/2/3/4 over both `churning` and the recorded session in one process.
Three things to know before touching it:

- **Every sweep row but r=3 reports a diverged end state, by design.** A
  different radius is a different simulation, so identical inputs are *supposed*
  to produce a different world. **The r=3 row reading `exact` is the real check**
  — if the shipped radius ever stops replaying byte for byte, the toggle changed
  the simulation, which is the one outcome it must not do.
- **`reset()` deliberately does not clear it.** It is configuration, not world
  state, and a world-reset hotkey silently reverting a caller's setting would be
  the defect. Pinned by a test in `test_grid.cpp`; argument at `Grid::reset`.
- **The conversion off `constexpr` costs `churning` 32% and the played session
  nothing measurable.** That was established with a cross-build A/B, which is
  admissible *because it had a control*: **`churning` is the only bench scenario
  containing water**, so the other six rows are untouchable by the change and
  all moved under 2% (`burning` 7.7% at one size, the widest excursion). The rule
  is "back to back with a control the change cannot affect", not "never rebuild"
  — this project has read it as the latter at least once, in the very header
  comment being corrected.

**Three displacement rules are ablatable at runtime** — `vent_fluid` (radius 0),
`seek_level` and `make_room_above` — and `grid_bench` removes them one at a time
to price what E5b retires. Two rules for reading that table:

- **The rows are separate simulations, not a partition of a step.** Removing a
  rule changes what the world does, so every later step in that run is doing
  different work. A row's gap from `all` is that rule's share *of that scenario*.
  Do not turn it into a pie chart.
- **Look for the null control before believing a small number.**
  `make_room_above` cannot fire on `churning`, which never paints, so that row
  measures the instrument: it reads +0.2%, and that is what makes `seek_level`'s
  0.3% on the same scenario readable as "at the floor" rather than "small". **An
  ablation table without a rule that cannot fire in it has no noise floor.**

**The synthetic scenario and the played session disagree about *which* fluid rule
is expensive, not just by how much.** Venting is 47% of `churning` and 0.1% of
the played session; `seek_level` is 0.3% of `churning` and 7.3% of the played
session. `churning` is powder sinking into fluid everywhere - all venting, no
settled surface to seek from - and a played world is large quiet pools with long
surfaces. **When quoting a synthetic row about a subsystem, that is the failure
mode to check for**: unrepresentative in kind, not merely in degree.

**The census table skips `Empty`** (the print loop starts at `t = 1`), so brush
strokes that *erase* are counted in `brush` and appear in no material row. Session
2's painted column sums to 8,493 against `brush 9158` — 665 erasing steps,
invisible. If the columns do not add up, that gap is the eraser, not a bug.
Worth fixing if the census is touched again: erasing destroys matter as an
external write, the same category as digging.

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
