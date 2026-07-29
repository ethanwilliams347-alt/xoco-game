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

**The rule going forward:** nothing from Long Term or Presentation & Tooling gets started until the v0.1 slice below is complete *and* playtested as fun. Ideas do not get implemented on arrival — they get **written down** in Long Term, Presentation & Tooling, or `notes/brainstorm.txt` and wait there. Capturing an idea is free and worth doing generously; the discipline is only about what gets *built*, never about what gets *imagined*.

Those three are the only deferral buckets. If an idea does not fit one of them, it either belongs in Short/Medium Term with a real justification, or it does not belong in this document.

**Guard against the real failure mode.** The risk here is not running out of ideas, it is spending three years on an engine that never becomes a game. The physics sandbox is the seductive part — it is fun to build and gives constant visible progress. Treat engine work as a means to the slice, and stop extending it the moment the slice does not require more.

**The pet ML agent is not machine learning, and that is a decision, not an omission.** This was the largest unpriced item in the plan and it is settled here so it stops being ambiguous. Real ML — training a model on gameplay telemetry — means a training pipeline, a model format, an inference runtime shipped with the game, nondeterministic output that has to be balanced anyway, and a payoff that cannot be predicted before it is built. That is a research project attached to a solo game, and it fails the same arithmetic as the Ideal Systems list. For v0.1 the agent is a **deterministic progression system**: run telemetry (objectives completed, depth reached, materials used) feeds hand-authored stats that gate which proof-of-work tasks the agent can take and how fast it completes them. It should *read* as a thing that learns. It does not have to *be* one. If the slice is fun and the fiction is carrying weight, revisiting this with a real model is a legitimate Long Term entry — but the slice does not get to depend on it.

### 🚩 Definition of Done — v0.1 Vertical Slice
The single milestone that matters. Everything before Long Term serves this:

> The player enters **one** quantum world, uses physics-based movement and destruction to complete **one** objective type, extracts successfully or dies losing the run, and their pet ML agent visibly gains from the run and earns coins idly.

If that loop is not fun, no amount of factories or stock markets will save it. If it *is* fun, it is a demo worth showing and a foundation worth expanding.

## 🟢 Current Status
- [x] Initial GitHub Repo and CMake build system setup.
- [x] Barebones C++/SDL2 pixel physics engine prototype built (Sand, Water, Wall interactions).
- [x] **Data-driven material system.** Materials are rows in a table (`src/physics/material.h`), not branches in the update loop. Four generic behaviours — Static / Powder / Liquid / Gas — drive all movement, with density deciding what sinks through what.
- [x] **Seven materials** proving the above: Sand, Water, Wall, Wood, Oil (floats on water), Steam (rises), Fire. Each cost one table row, no engine changes.
- [x] **Fixed timestep.** Simulation advances at a constant 60 Hz independent of render framerate, so the game behaves identically on a 60 Hz and a 144 Hz display.
- [x] **Headless test harness.** The simulation has no SDL dependency and is covered by tests wired into CTest — conservation of matter, density stratification, static materials, border sealing. Now four suites, one per concern: `tests/test_grid.cpp`, `tests/test_player.cpp`, `tests/test_tool.cpp`, `tests/test_collapse.cpp`, sharing one harness in `tests/test_util.h`. 81 checks in total.
- [x] **Chunked dirty-rect updates.** The world is split into 64x64 chunks, each tracking the bounds of the cells in it that can still move. Settled chunks are skipped entirely, so cost now scales with how much is *moving* rather than with world size. Every write wakes its 3x3 neighbourhood so nothing is left hanging in mid-air, and the awake-chunk count is on screen so culling bugs are visible rather than silent.
- [x] **Benchmark.** `tests/bench_grid.cpp` measures a 960x540 world across scenarios against the 60 Hz budget. Deliberately not a CTest test — timings inform, they do not gate.
- [x] **Reactions.** A data-driven `REACTIONS` table (`src/physics/reaction.h`, catalyst + target -> result, rolled per step) drives Fire: it ignites touching Wood and Oil, is doused into Steam by Water, and burns out on its own. Second and last major engine axis — transformation is now as data-driven as movement. The one non-obvious piece: Fire's self-decay is *spontaneous* (no neighbour required), which means it has no movement to piggyback a wake-up on, so a boxed-in Fire cell self-marks its own neighbourhood dirty every step purely to avoid freezing mid-burn. Covered by 5 new tests (25 total), including a dedicated regression test for that freeze case.
- [x] **Player Character + Player Physics.** A 4x8 rigid body (`src/physics/player.h`) that is *not* a cell: it has its own position and velocity and only ever reads the grid to ask whether a cell is solid. Position is an integer cell plus a sub-cell remainder rather than a float, so collision compares whole cells and the float-edge bugs never arise; movement resolves one cell at a time per axis, which makes tunnelling impossible by construction rather than by being fast enough. Solidity is derived from the existing `MoveKind` (Static and Powder are solid, Liquid and Gas are not), so it stays one table, not two. Gravity, jumping, a 2-cell step-up for walking over uneven powder, and an unstuck search for when falling sand buries the player. Covered by 18 tests in a second suite, `tests/test_player.cpp`.
- [x] **Structures fall as rigid bodies.** Wall and Wood no longer hang in mid-air when the ground is dug out from under them. An unsupported piece falls **as one rigid body, keeping its shape the whole way down** — it is not converted into loose grains. That distinction is the whole feature: the first implementation dissolved an unsupported slab into a Rubble powder, which was cheap and read as a different bug, because masonry that turns to gravel the instant it comes free looks broken rather than physical. Rubble was removed along with it. The piece stays in the cell grid while it falls, so it is a rigid body only in *how it moves*, not in where it lives — which is what keeps rendering, player collision and digging working on it with no changes at all. Which materials are structure is a `structural` flag in the `MATERIALS` table, so it stays one table rather than a second list. Support is a flood fill through connected structure looking for one cell that is grounded, where grounded means the world floor or something solid that is not part of the same piece; powders bear load, liquids and gases do not, so a slab stands on packed sand and sinks through water. Falling is a per-column bottom-up shift by one cell per step, so an arch or an L keeps its corner, and the fluid displaced from underneath surfaces on top rather than being deleted. **Two decisions worth knowing.** First, support is checked *on disturbance only*, never as a global truth — a sweep of the world every step would cost more than the simulation it is attached to, and a world as authored is assumed to be standing up on purpose, so a floating platform placed with the brush stays where it was put until something touches it. Second, pieces over 4,096 cells are assumed supported rather than judged, because a missed fall is invisible while a wrong one drops the level. Covered by 15 tests in a fourth suite, `tests/test_collapse.cpp`, including an L-shaped piece that has to keep its corner and a slab that has to land as a slab. This is the first feature in the project with a measurable performance cost — see the table above.
- [x] **Basic Interaction — digging.** The player can now change the world, which is the line between a sandbox and a game. A dig is a ray marched one cell at a time from the body's centre to the first solid cell, which is then blown out to a small radius (`src/physics/tool.h`). Deliberately *not* a method on `Player`: keeping the body's grid reference `const` is what makes it structurally incapable of breaking the `set_element` write rule, so the verb lives in its own module that takes a mutable `Grid&`. What blocks a shot is the same `is_solid` the player collides against — one definition used twice, so terrain and powder stop a ray while water and fire do not. Range is real distance rather than a step count, and the cooldown is in fixed steps rather than seconds. Covered by 23 tests in a third suite, `tests/test_tool.cpp` — including the anti-tunnelling case (a thin wall must not be dug *through* to the terrain behind it) and the collapse case (digging out the base of a settled pile makes it fall, which only works because removal goes through `set_element` and wakes the neighbourhood).

## 📊 Measured Performance
Numbers from `grid_bench` on the dev machine, 960x540 (518,400 cells), against a 16.67 ms frame:

| Scenario | Support check off | Support check on | % of a 60 Hz frame |
|---|---|---|---|
| **settled** — a world at rest | 0.0001 ms | **0.0001 ms** | ~0% |
| **sparse** — static terrain, one small falling blob | 0.063 ms | **0.066 ms** | 0.4% |
| **churning** — sand sinking through water, then settling | 2.76 ms | **2.90 ms** | 17.4% |
| **cascading** — nothing ever settles, sustained worst case | 11.73 ms | **12.23 ms** | **73.4%** |
| **burning** — a Wood slab permanently on fire | 0.47 ms | **0.48 ms** | 2.9% |

**How to read this table, because the columns changed.** Earlier versions compared against numbers recorded in previous sessions, and that stopped being trustworthy: a run of this benchmark on the same binary drifted from 10.6 ms to 11.7 ms on `cascading` between one session and the next, purely from machine state. Comparing a new feature against a stale absolute number would have charged it for a 10% regression it did not cause.

So both columns above are measured **back to back on the same machine state**, with the support hook compiled out for the left column and in for the right. That is the only comparison that means anything, and it is the one to reproduce when this changes again. **Absolute numbers are only comparable within a row.**

**The support check costs about 5%, and that is real.** Reactions were free in any scenario with nothing to react (`try_react` exits on its first check) and digging was free because it runs a few dozen cells on a cooldown. This one is not: `swap_elements` now asks whether the cell above each end of a move is structure, and `swap_elements` is the hottest path in the engine. The cost is one extra memory touch a row above the cell being moved — a cache miss more often than arithmetic. An early-out that skipped ends still holding something solid was written, measured as noise, and deleted again rather than kept on the theory that it ought to help.

Note that `churning` contains no Wall or Wood at all, so nothing in it can ever fall — its 5% is pure toll paid by the check on a world where the feature does nothing. That is the price of the design, and it is worth knowing which half of the cost is the toll and which is the work.

Affordable, and paid deliberately. `cascading` is a synthetic worst case the game does not produce, real gameplay sits near `sparse`/`burning`, and the two cheap wins in Engineering Notes (cell size, RNG) are both untouched and both aimed squarely at this loop. Spend them when a scenario the *game* produces goes over budget — not now. At 73% of frame, though, `cascading` no longer has much headroom left, and it is the number to watch.

The prototype runs at 200x150, real gameplay is far closer to `sparse`/`burning` than to `cascading`, and the deferred optimisations in Engineering Notes below are untouched. Do not spend time on it now — spend it when a scenario the *game* actually produces goes over budget.

## 🔴 Short Term (Next Up)
*One item left before Medium Term, and it is the one that was always conditional. It should be settled by playing rather than by argument.*

- [ ] **Player displaces material.** The grid does not know the player exists, so sand falls straight through the body and the unstuck search cleans up afterwards. Fixing it properly means the player pushing material out of the way, which is a real design problem — it has to happen without destroying matter. Now that digging exists, the question is answerable by playing: if the artifact is obvious in practice, do it; if it isn't, leave it. See Engineering Notes.

## 🟡 Medium Term (Core Gameplay Loop)
*Tying the physics engine into the lore. This section is the v0.1 slice.*

*This list is longer than it used to be. Three items below were previously absent even though the Definition of Done depends on all three — a camera, a save file, and a way to die. They were not deferred decisions, they were gaps, and a plan that hides its prerequisites is not a plan. Assume the slice is roughly twice the work the old five-line version implied.*

- [ ] **Camera and world-space coordinates.** *Prerequisite for everything under it.* The world is currently exactly the window: `GRID_WIDTH` is derived from `WINDOW_WIDTH`, so a level cannot be larger than one screen. A "trial" the size of a 200x150 screen is not a level, it is a room. This needs a world/screen coordinate split, a camera that follows the player, and a decision about whether off-screen chunks still simulate — which chunking makes cheap to answer, since a sleeping chunk already costs nothing.
- [ ] **Quantum Worlds:** A portal/level generation system where the player enters a single "trial". One world, one generator — variety comes later.
- [ ] **Player health and death.** There is no damage model at all today. Fire is fully simulated and cannot hurt the player, because the grid cannot see the body. "Dies losing the run" is half the Definition of Done and it currently has no implementation behind it. Needs: what damages the player (fire and fall speed are the two the engine can already supply for free), how it is shown, and what death does to the run.
- [ ] **Objective + Extraction:** One objective type, and a run that ends in either successful extraction or death with loss.
- [ ] **Save and persistence.** The Definition of Done requires the agent to earn coins *idly*, which means progress survives quitting. Nothing in the project writes a file today. Needs a format decision, a save location, and an explicit answer to what happens when a save from an older build is loaded — the cheap answer, refusing to load mismatched versions, is fine and should be chosen deliberately rather than by accident.
- [ ] **The Pet ML Agent:** The UI/system for the pet agent that "observes" the player and gains from completed runs. Deterministic progression, not real ML — see Scope Discipline.
- [ ] **Proof-of-Work Economy:** The idle earning loop — agent performs tasks between runs and earns coins.
- [ ] **Playtest gate:** Put the slice in front of people who did not build it. Do not proceed past this line on the strength of your own opinion.

## 🔵 Presentation & Tooling (after the slice, before polish)
*Not slice-blocking — none of this makes the loop fun, and none of it should be started while Short Term or Medium Term is open. Recorded here rather than in Long Term because these are concrete, bounded, and genuinely expected to get built, unlike the wish list below.*

- [ ] **Display modes: fullscreen, borderless, windowed.** Industry-standard behaviour, which is more than three `SDL_SetWindowFullscreen` calls: exclusive fullscreen vs. borderless-windowed as separate options, alt-tab that doesn't corrupt the window state, the choice persisted between launches, and correct behaviour when the mode is changed mid-run rather than only at startup.
- [ ] **Resolution options, including 1920x1080, 2560x1440 and 3440x1440.** This one is not just a window setting — the simulation grid is currently derived from the window size (`GRID_WIDTH = WINDOW_WIDTH / PIXEL_SCALE`), so today a bigger window means a bigger, more expensive world rather than a bigger view of the same world. Decide the model first: a fixed simulation size that scales to fit the display (letterboxed or with a camera), or a world that genuinely grows with resolution. **The camera item in Medium Term settles this**, and it lands first — do not answer the question twice. The second option means a 3440x1440 player simulates ~2.5x the cells of a 1920x1080 player and the benchmark budget stops meaning anything. Ultrawide also changes what is on screen, which is a gameplay-fairness question, not only a rendering one.
- [ ] **Custom pixel art and animation generator for game assets.** A tool for authoring sprites and animations that fit the engine's palette and cell scale, so art is produced in-project instead of hand-drawn in an external editor and re-exported. Worth being blunt about the risk: this is a second application, with its own UI, file format and edit loop, and it is exactly the kind of seductive side-build the Scope Discipline section warns about. It only earns its keep once there is enough art volume that authoring by hand is measurably the bottleneck. Until then, an external editor plus a small import step is the cheaper answer.

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
- **The player is invisible to the grid.** The simulation has no idea a body is standing there, so material falls through the player instead of piling on top of it, and the unstuck search is what stops that becoming a freeze. The real fix is for the player to displace material, which is harder than it looks: shoving cells aside must not create or destroy matter, and the obvious cheat — stamping the body into the grid as a temporary solid each step — either deletes whatever was already in those cells or needs a full displacement pass of its own. Deliberately deferred; the current behaviour is odd-looking but never broken.
- **Player feel is deliberately raw.** Horizontal speed is a direct function of input — no acceleration, friction, air control, coyote time, or jump buffering. All of that is feel work, and feel work is worth doing once there is something to feel. Do not start tuning it before Basic Interaction exists.
- **Player and tool cost are not benchmarked.** A collision test reads at most 32 cells and a step runs a handful of them, against 518,400 cells of grid. A dig marches at most 24 cells and writes at most 29, on a cooldown. Both are far below measurement noise, and `grid_bench` numbers are unchanged after adding each. Revisit only if there is ever more than one body, or a tool that fires every step without a cooldown.
- **There is no UI layer, and nothing has picked one.** No menus, no settings screen, no HUD, no text rendering, no audio of any kind. The window title bar is doing the entire job of the user interface today. Three separate items now imply a UI — display/resolution options, the pet agent panel, and a health readout — and the first of them to be built will silently decide the answer for the other two. Make that choice on purpose (immediate-mode drawn against SDL, a library, or hand-rolled), and make it once.
- **Materials have no hardness, and everything is diggable.** Adding a hardness or indestructible column to `MATERIALS` would be a second axis with no consumer: the world border is already sealed by `set_element`'s bounds check, so nothing can be dug through that matters yet. Revisit when there is a reason for one material to resist a tool more than another — bedrock around an objective, or a tool upgrade curve.
- **Digging is a deliberate matter sink.** The conservation-of-matter test covers `Grid::update()` — the simulation never creates or destroys cells on its own, and that invariant still holds. Digging is an *external* write that deletes matter outright, which is correct for a tool and would be a bug anywhere in the step loop. Do not "fix" the conservation test to accommodate it; they are testing different things.
- **All writes must go through `set_element` / `swap_elements`.** They are the only two functions that call `mark_dirty`. A new code path that writes cells directly will produce material frozen in mid-air, and the tests that catch it are the chunk tests in `test_grid.cpp`.
