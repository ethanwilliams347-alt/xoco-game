# Game Roadmap

This document tracks the immediate next steps and the long-term vision. For deeper lore and feature brainstorming, see the `notes/` directory.

## 🎯 Project Goals
- **Core:** A production-level, barebones application.
- **Engine:** Custom pixel art destructible environment running on its own cellular automata physics engine.
- **Performance:** Strict no-bloat philosophy. Must run on low-end PCs (Windows/macOS/Linux).
- **Architecture:** Keep design choices minimal, but architected to easily allow massive expansion later.

## ⚖️ Scope Discipline (read before adding anything)

This is the hard constraint the rest of the document is written against. It is a promotion of `notes/reality_check.txt` into the plan itself, because that note was correct and was being ignored by the roadmap below it.

**Reference point:** Noita — the closest comparable — was built by a team of three over roughly eight years on a purpose-built engine. That is the cost of the physics pillar *alone*.

**The verdict:**
- **Achievable solo:** the pixel physics engine + one gameplay pillar + the extraction/agent loop. Ambitious, multi-year at part-time pace, but real. Solo games that ship succeed on *one* strong hook executed well.
- **Not achievable solo:** the "Ideal Systems" list as written. A simulated stock market, factories, business ownership, farming, a developing society, and reputation mechanics are each a full game's worth of design, balance, and UI. Shipping all of them alongside a physics roguelite is not a scope problem, it is an arithmetic problem.

**The rule going forward:** nothing from Long Term gets started until the v0.1 slice below is complete *and* playtested as fun. Ideas do not get implemented on arrival — they get **written down** in Long Term or `notes/brainstorm.txt` and wait there. Capturing an idea is free and worth doing generously; the discipline is only about what gets *built*, never about what gets *imagined*.

**Guard against the real failure mode.** The risk here is not running out of ideas, it is spending three years on an engine that never becomes a game. The physics sandbox is the seductive part — it is fun to build and gives constant visible progress. Treat engine work as a means to the slice, and stop extending it the moment the slice does not require more.

### 🚩 Definition of Done — v0.1 Vertical Slice
The single milestone that matters. Everything before Long Term serves this:

> The player enters **one** quantum world, uses physics-based movement and destruction to complete **one** objective type, extracts successfully or dies losing the run, and their pet ML agent visibly gains from the run and earns coins idly.

If that loop is not fun, no amount of factories or stock markets will save it. If it *is* fun, it is a demo worth showing and a foundation worth expanding.

## 🟢 Current Status
- [x] Initial GitHub Repo and CMake build system setup.
- [x] Barebones C++/SDL2 pixel physics engine prototype built (Sand, Water, Wall interactions).
- [x] **Data-driven material system.** Materials are rows in a table (`src/physics/material.h`), not branches in the update loop. Four generic behaviours — Static / Powder / Liquid / Gas — drive all movement, with density deciding what sinks through what.
- [x] **Six materials** proving the above: Sand, Water, Wall, Wood, Oil (floats on water), Steam (rises). Each cost one table row, no engine changes.
- [x] **Fixed timestep.** Simulation advances at a constant 60 Hz independent of render framerate, so the game behaves identically on a 60 Hz and a 144 Hz display.
- [x] **Headless test harness.** The simulation has no SDL dependency and is covered by physics tests (`tests/test_grid.cpp`) wired into CTest — conservation of matter, density stratification, static materials, border sealing.
- [x] **Chunked dirty-rect updates.** The world is split into 64x64 chunks, each tracking the bounds of the cells in it that can still move. Settled chunks are skipped entirely, so cost now scales with how much is *moving* rather than with world size. Every write wakes its 3x3 neighbourhood so nothing is left hanging in mid-air, and the awake-chunk count is on screen so culling bugs are visible rather than silent.
- [x] **Benchmark.** `tests/bench_grid.cpp` measures a 960x540 world across scenarios against the 60 Hz budget. Deliberately not a CTest test — timings inform, they do not gate.
- [x] **Reactions.** A data-driven `REACTIONS` table (`src/physics/reaction.h`, catalyst + target -> result, rolled per step) drives Fire: it ignites touching Wood and Oil, is doused into Steam by Water, and burns out on its own. Second and last major engine axis — transformation is now as data-driven as movement. The one non-obvious piece: Fire's self-decay is *spontaneous* (no neighbour required), which means it has no movement to piggyback a wake-up on, so a boxed-in Fire cell self-marks its own neighbourhood dirty every step purely to avoid freezing mid-burn. Covered by 5 new tests (25 total), including a dedicated regression test for that freeze case.

## 📊 Measured Performance
Numbers from `grid_bench` on the dev machine, 960x540 (518,400 cells), against a 16.67 ms frame:

| Scenario | Before chunking | After chunking | With reactions | % of a 60 Hz frame |
|---|---|---|---|---|
| **settled** — a world at rest | 1.64 ms | 0.0001 ms | **0.0001 ms** | ~0% |
| **sparse** — static terrain, one small falling blob | 0.27 ms | 0.069 ms | **0.056 ms** | 0.3% |
| **churning** — sand sinking through water, then settling | 2.59 ms | 2.83 ms | **2.41 ms** | 14.5% |
| **cascading** — nothing ever settles, sustained worst case | — | 13.6 ms | **10.1 ms** | **61%** |
| **burning** — a Wood slab permanently on fire | — | — | **0.42 ms** | 2.5% |

**Read this honestly.** `settled`/`sparse`/`churning`/`cascading` don't touch a reactive material (all Sand/Water/Wall), so the small movement in their numbers versus last session is run-to-run machine noise, not a code change — `try_react`'s target-type check is the first thing it does and exits immediately for every material with no reaction row, which is all of them here. Reactions cost real time only on cells that can actually react, which is exactly what `burning` measures: a Wood slab continuously on fire, ignition/spread/decay all happening at once, still comes in at 2.5% of the frame budget. The new axis did not move the ceiling found last session — `cascading` at ~60% of budget is still the number that matters, and it is still not yet a crisis for the reasons given below.

The prototype runs at 200x150, real gameplay is far closer to `sparse`/`burning` than to `cascading`, and the deferred optimisations in Engineering Notes below are untouched. Do not spend time on it now — spend it when a scenario the *game* actually produces goes over budget.

## 🔴 Short Term (Next Up)
*Both engine axes — movement and transformation — are done. What remains is the player.*

- [ ] **Player Character:** A controllable entity that is *not* a grid cell — a small rigid body sampling the grid for collision.
- [ ] **Player Physics:** Gravity, walking over uneven powder, and correct collision against solid pixels.
- [ ] **Basic Interaction:** Let the player dig or shoot the destructible terrain — the moment the sandbox becomes a game.

## 🟡 Medium Term (Core Gameplay Loop)
*Tying the physics engine into the lore. This section is the v0.1 slice.*
- [ ] **Quantum Worlds:** A portal/level generation system where the player enters a single "trial". One world, one generator — variety comes later.
- [ ] **Objective + Extraction:** One objective type, and a run that ends in either successful extraction or death with loss.
- [ ] **The Pet ML Agent:** The UI/system for the pet agent that "observes" the player and gains from completed runs.
- [ ] **Proof-of-Work Economy:** The idle earning loop — agent performs tasks between runs and earns coins.
- [ ] **Playtest gate:** Put the slice in front of people who did not build it. Do not proceed past this line on the strength of your own opinion.

## 🟣 Long Term (The "Ideal Systems" Vision)
*A deliberate wish list, kept for motivation. **Add to it freely** — that is what it is for.*

*The only rule attached to this section is that nothing in it gets **started** before the v0.1 slice is done and playtested. Most of these will never be built, and that is the expected and healthy outcome, not a failure. A list you enjoy adding to costs nothing; a half-finished feature branch costs everything.*

- [ ] Itemization and grid-based inventory.
- [ ] Base building and criminal/black market activities.
- [ ] Complex simulated stock/crypto market.
- [ ] Factories, Mining, and Business Ownership.
- [ ] Passive developing society and reputation mechanics.

## 🧭 Engineering Notes
Deferred deliberately, recorded so they are not rediscovered later:
- **Cell size.** `Element` is 8 bytes (type + colour + tag). Colour could be derived from the material table plus a small per-cell jitter seed, cutting the cell to ~2 bytes and roughly quartering the memory traffic of the hot loop. This is now the **first** thing to try if the worst case has to come down, because it targets active cells, which is where the remaining cost is.
- **RNG cost.** `std::mt19937` is called several times per active cell — including once per row purely to pick a sweep direction. A xorshift would be materially faster in the inner loop, and the per-row direction call could be derived from the frame tag instead. Same category as above: cheap, and aimed at active cells.
- **Threading.** Deliberately single-threaded. Chunked updates were the prerequisite, so this is now *possible* — but it is also the single largest source of subtle nondeterminism available, and the measured numbers do not justify it yet. Exhaust the two items above first.
- **Cross-platform.** The build targets Windows/macOS/Linux and uses no platform-specific code, but has only been *built and run on Windows*. Verify on the other two before claiming support.
- **Frame tag wraparound.** `Element::updated_tag` is one byte, so a cell asleep for an exact multiple of 256 steps is skipped for a single step and runs the next one. This is known, harmless, and cheaper than the alternatives — do not "fix" it with a wider counter without a measured reason.
- **All writes must go through `set_element` / `swap_elements`.** They are the only two functions that call `mark_dirty`. A new code path that writes cells directly will produce material frozen in mid-air, and the tests that catch it are the chunk tests in `test_grid.cpp`.
