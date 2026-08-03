# Playtest Log

Session records from running README's Manual Tester Checklist against a real build.

**This is the defect log, not the plan.** ROADMAP.md says what the project intends to
build; this file says what was found wrong with what is already built. They are kept
apart deliberately: a roadmap that absorbs every playtest finding stops being readable
as a statement of intent within about two sessions, and a defect that lives in a
roadmap gets closed by being forgotten rather than by being fixed.

**What goes in a session entry:** the build it was run against, **the world seed**, the
per-step result, and a defect table. The seed is not optional bookkeeping — F1 made the
simulation a pure function of its seed and step count precisely so that "it looked wrong"
converts into a case someone else can reproduce. A finding without its seed has thrown
that away.

**Findings are triaged, not queued.** A defect is a fact and goes in the table. A
*feeling* about how something reads is data about an experience, and the fix it suggests
is a hypothesis rather than a specification — those are recorded separately, as
observations, and only become roadmap items after being argued for on their own merits.
The distinction matters because the two convert into work at wildly different exchange
rates: session 1 produced eight defects in three minutes, and four "additions" that
between them are most of a milestone.

**The order of work lives in this file, at the bottom, and is updated as it moves.**
Session 1's twelve findings were sequenced into waves during triage, and for the first
three of them that sequence existed only in the conversation that produced it. That is
the actual failure mode this log is meant to prevent: the defects were all written down
and the *plan for them* was not, so every session had to re-derive what came next and
was one interesting tangent away from losing it. See [Order of work](#order-of-work).

**Every confirmed defect that the headless suites can reach gets a failing test before
its fix.** ROADMAP's correctness-pass lessons already record that a green suite proved
less than it looked like. This is the second data point for the same lesson, and it is
sharper: six of session 1's eight defects live in simulation code the existing suites
can already reach. They were not missed because tests could not see them. They were
missed because nobody wrote the test.

---

## Session 1 — 2026-08-02

- **Build:** `313aa94`, Release, MSVC
- **World seed:** `18164811273671827879`
- **Scene:** `Scene: 640x400, 27192 cells placed`
- **Suites at time of test:** 6/6 green, 199 checks
- **Result:** 9/9 steps executed. No test blockers — the pass completed without a fix
  being applied mid-session, which is why the later steps are trustworthy.

### Per-step result

| # | Step | Result |
|---|------|--------|
| 1 | Launch | **Pass.** Seed and scene line both printed, count correct, no legend warning. |
| 2 | Movement | **Pass with defect.** Collision correct in every direction; rendering is not — see A1. |
| 3 | Digging | **Pass.** Range limit, marker, dirty-rect and cover-fire cases all correct. |
| 4 | Materials and brush | **Pass with defects.** Chunk count settles to 0 after stress — the fix this step was written to check holds. HUD readout lags — see A2. |
| 5 | Reactions and heat | **Fail.** Steam no longer ignites the ceiling (the regression this step exists for is fixed), but fire behaviour is wrong in three ways — A3, A4, A5. |
| 6 | Chunking / sleep-wake | **Pass.** Count rests correctly. |
| 7 | Structures | **Pass.** Rigid fall, shape retention, no twitching, clean drops, nothing floating. |
| 8 | Performance sanity | **Pass.** 164–165 fps held throughout, no drops under load. |
| 9 | Stability | **Pass, no crash.** Surfaced three rendering/physics defects — A6, A7, A8. |

### Defects

Severity: **major** = wrong behaviour a player will hit in normal play; **minor** =
wrong but cosmetic or narrow.

| ID | Defect | Severity | Root cause | Status |
|----|--------|----------|-----------|--------|
| A1 | Player rectangle jitters and ghosts while moving | major | **Three causes, and the first diagnosis found only the smallest of them.** (i) `Player::cell_x()` returns an int and discards `rem_x`/`rem_y`; at 45 cells/s over a 60 Hz step the player advances 1 cell on 3 steps of 4. (ii) The sim steps at 60 Hz against a 165 Hz display, so each position was held ~2.75 frames with no interpolation. (iii) **The largest: the camera could only sit on whole cells.** The world is 640x400 against a 200x150 viewport, so the view is unclamped wherever the player usually is — which pins the player near screen centre and scrolls *the world* in 4-pixel jerks instead. Smoothing the player alone would have left the symptom almost intact. Collision is unaffected throughout; the integer model at `player.h:89` is correct and stays. | fixed, needs visual confirmation |
| A2 | HUD lags material switches by up to a second | minor | `hud_text` is rebuilt only inside the once-a-second timer in `main.cpp`. The input is instant; the readout is stale. A measurement bug, so it was fixed first. | fixed |
| A3 | Wood burns away far too fast | major | There is no burn duration anywhere. Wood ignites at 100% on reaching 120°, and the resulting Fire dies on a flat 6%/step roll — ~17 steps, ~0.28 s. Fire from wood, from oil and from nothing are the same cell. Not tunable: there is no fuel quantity to tune. | fixed, needs visual confirmation |
| A4 | Fire will not propagate along horizontal wood | major | Fire is `MoveKind::Gas` at density −10, so it rises. A flame on top of a horizontal beam floats off after one step and the wood never reaches its 120° threshold. Alongside a *vertical* beam the flame rises parallel to the fuel and stays in contact. Same missing fuel concept as A3. **Fixed by the fuel counter rather than by a propagation rule**: a flame that still owes burning time does not move, so it stays on the cell it is consuming and conducts sideways as readily as upwards. Rising becomes what a flame does when it is *finished*. | fixed, needs visual confirmation |
| A5 | Steam condenses back to water far too fast | major | Steam's lifetime *is* the span between spawn 88° and condense 26°, so temperature is doing double duty as its clock. This is the residue of the ignition fix, which had to drop spawn temperature and paid lifetime for it. | open |
| A6 | Spawning material into water caps the water on top, then bursts outward on release | major | Two causes. The density sort is correct (sand 150 sinks through water 100). But `place()` overwrites unconditionally, so the brush *destroys* the water in its disc instead of displacing it, and volume is not conserved while dragging. The outward burst on release is `seek_level`'s pressure head resolving all at once once repainting stops. **Fixed by giving the brush its own write path, `Grid::displace`** — the occupant climbs to the first Empty above it and the brush writes into the vacancy, so a stroke conserves volume. `set_element` still overwrites for everyone else, because for every other caller deletion is the point. Measured: a 4-radius sand brush dragged 40 cells through a 1000-cell pool destroyed **451 cells** before, and **0** after. The burst was not a second defect and needed no separate fix — it was the deleted volume being repaid. | fixed, needs visual confirmation |
| A7 | Falling liquid throws horizontal sticks under 10 cells wide | minor | `spread` is a distance a fluid may cover *along a surface*, and it was being spent crossing open air as well: `can_rest_at` is satisfied by "I could fall from there", which every point in mid-air satisfies. A cell inside a falling stream cannot descend — its own kind is below it — so it reached the lateral scan, found five cells of nothing beside it, and relocated to the far end. Liquids only; Sand is a Powder with spread 0. **Fixed by stopping the lateral run one cell past the last supported cell** — that one cell is what flowing off a lip *is*, so forbidding it outright would strand water on a shelf. | fixed, needs visual confirmation |
| A8 | Material carves authored background out of whatever it passes through | major | **Not what the first diagnosis said, and screenshots are what settled it.** `Element`'s default colour was `0xFF000000` — opaque black — while Empty's `MATERIALS` row is transparent. `swap_elements` moves whole Elements, so a grain leaving a cell swapped a *default-constructed* Empty into its place, and that Empty carried opaque black into `pixels`. Every cell anything had ever moved through was painted black permanently. Nothing to do with `place()` or authored albedo; the two values had simply disagreed since V1 made Empty transparent. | fixed |

### A note on measuring A4 — the scene lied twice before the code was wrong once

The A4 regression test went through three scenes, and the first two both reported
confidently on something other than fire.

**Scene 1 hung the beams in mid-air.** Horizontal read 20 of 20 cells consumed and
vertical read 11, which looks exactly like "spreads sideways, struggles downwards" —
a plausible, interesting result. It was neither. The metric was `!= Wood`, and burning
the end off an unsupported beam makes the rest of it *unsupported*, so the collapse
system dropped the remainder out of frame and every sampled cell read Empty. The test
was measuring E3, not E9. The tell was that vertical sat at exactly 11 from 600 steps
through 4800: propagation that is merely slow keeps moving, and a number that does not
change with four doublings of the budget is not a rate at all.

**Scene 2 grounded the beams and lit them with a single placed flame.** Both dropped to
1 cell consumed, which reads as a total failure of the fix. Also wrong: the *igniting*
flame is unfuelled, so it is a free gas and rises off the beam within one step — A4's
own defect, arriving from the source's side. The fix was working; the match was
blowing itself out.

Scene 3 grounds the beams and holds the match for 30 steps. Both beams then burn end to
end, and removing `if (fuelled) return` puts them back at 2 and 1.

The lesson generalises past this test, and it is the same shape as the A7 one above:
**a physics scene tests everything in the engine at once, so a test of one subsystem has
to be built to exclude the others.** Support, buoyancy and lifetime were all live in a
twenty-cell wooden beam, and two of them answered before fire got the chance.

### Follow-up, same day — A1's fix exposed A1b

Fixing the rendering half of A1 made a **pre-existing simulation bug** visible for the
first time: the player bobbed into and out of the floor continuously, and could phase
about three quarters of a cell into walls, sand and wood.

| ID | Defect | Severity | Root cause | Status |
|----|--------|----------|-----------|--------|
| A1b | Player bobs into the floor on a ~0.3 s cycle; phases ~0.75 cells into walls | major | `rem_y` is *pending, collision-untested* motion, and `update()` zeroed `vel_y` on landing without zeroing the remainder it had already produced. Standing still, gravity re-added ~0.056 cells of pending fall per step and `move_y` was never called until it crossed a whole cell — at which point the floor test finally ran and snapped the body back. The horizontal case is the same omission: `rem_x` reached 0.75 of a cell into a wall before any step was attempted. | fixed |

**This was always happening.** Every whole-cell assertion in `test_player.cpp` passed
throughout, because the body's *cell* never moved — the entire defect lived in the
fraction the renderer used to discard. It is the sharpest possible illustration of the
correctness pass's lesson that a green suite proves less than it looks like, and it adds
a corollary worth keeping: **when a display change appears to cause a physics bug, the
first hypothesis should be that it revealed one.**

Regression tests added in `tests/test_player.cpp`, and **verified by reverting the fix
and watching them fail** rather than by being written green — 0.111 cells of sink
observed, and a body edge at 40.75 against a wall face at 40.0. They are the only tests
in the suite that assert on `visual_x()`/`visual_y()`, and that is deliberate: nothing
that rounds to whole cells could have caught this.

### Follow-up — A7 was diagnosed as a liquid bug and is mostly a powder bug

Reported again after the liquid fix shipped, with screenshots
(`resources/video_screenshots/`) of a sand pile fringed with horizontal spikes.
**Sand is a `Powder` with `spread` 0 and never reaches the lateral scan the first fix
changed**, so that fix could not have addressed what was actually being seen. The liquid
defect was real and is still fixed; it was not the defect being reported.

| ID | Defect | Severity | Root cause | Status |
|----|--------|----------|-----------|--------|
| A7b | A pouring powder fringes itself with one-cell-thick horizontal shelves | major | `swap_elements` tags only the two cells it touches, so an entire row cascades diagonally inside one sweep — the grain at x moves to (x+1, y+1), the grain at x+1 is still untagged and moves to (x+2, y+1), on to the end of the row. Every grain lands one row down and one column over in the same step, while the row beneath was swept earlier and has not caught up, leaving a shelf standing nine cells proud of the pile with nothing under it. Resolves next step and immediately re-forms, so it reads as constant flickering spikes. **Fixed by requiring a diagonal roll to land on something** — the grain that would start the cascade has nothing to roll onto and waits for the slope instead of running ahead of it. | fixed, needs visual confirmation |

Peak mid-air shelf **9 cells → 4**. `grid_bench` got slightly *faster* — cascading 12.14
→ 11.39 ms/step, churning 3.13 → 3.00 — because the rule prevents swaps rather than
adding work.

**What made this findable was the screenshots, and specifically that they were of a
state nobody would have thought to describe.** The written report said "horizontal
protrusions… in <10 pixel wide sticks", which is accurate and which I read as a liquid
artifact because that was the mechanism I had already found. The images showed sand,
showed the shelves were one cell thick and attached to a pile, and showed a black wake
that turned out to be a *second, unrelated* defect (A8) that no amount of re-reading the
text would have produced. **A screenshot is worth asking for whenever a defect is about
what something looks like rather than what it does** — it carries the things the reporter
did not know were relevant.

### Follow-up 2 — A7b's first two fixes were both wrong, and the third costs frame time

Reported after the shelf fix: sand now stacked into perfect vertical columns, and grain
motion read as stepping rather than flowing.

**Three rules were tried and the first two failed in opposite directions.** Refusing the
diagonal roll unless its destination was already supported removed the shelves — and
stopped *settled* grains relaxing down a face, so piles grew straight up. Restricting
that refusal to grains still falling brought the shelves straight back, because it is the
settled grains slumping that forms them. A rule aimed at motion kept catching rest; a
rule that spared rest stopped catching the defect. **Shelves and columns are opposite
failures of one rule, so a test for either alone can be passed by breaking the other** —
which is why both are now asserted, in `test_grid.cpp`, next to each other.

The third rule does not restrict the move, it *finishes* it: a grain that rolls off an
edge keeps falling in the same step and arrives at its resting depth immediately, so
there is no moment at which a shelf exists. Peak shelf **9 → 3 cells**, steepest settled
column drop **1 cell** — a smooth face with no vertical walls.

Separately, powders now accelerate. They moved exactly one cell per step whatever they
were doing, so every grain in the world travelled at a constant 60 cells/second and a
two-cell drop looked identical to a fifty-cell one. Constant speed is what "step based"
*is* — there is no motion for the eye to follow, only a sequence of positions.
**`fall_ticks` paid for this at zero memory cost**: the field already existed, is already
carried by `swap_elements`, and its own comment recorded that only structural pieces used
it. `element.h` says the struct has no room for another field, so a field that was being
paid for and not spent is the only kind available.

| ID | Defect | Severity | Root cause | Status |
|----|--------|----------|-----------|--------|
| A7c | Sand stacks into vertical columns; grain motion reads as stepping | major | Columns: the shelf fix forbade settled grains from relaxing down a slope. Stepping: powders moved at a constant one cell per step with no acceleration. | fixed |

**Powder acceleration was then removed entirely, and taking it out fixed three things
at once.** Reported after the speed was halved: sand fell in sheets with a one-cell gap
between them. `fall_ticks` is per cell and `place()` resets it, so the brush stamped a
fresh layer of speed-1 grains every step on top of grains that had already reached speed
2; the fast ones pulled away from the slow ones by exactly one cell per step. **Inherent
to per-cell acceleration under a continuous source**, not a tuning error — no cap value
avoids it.

Removing it also retired the "too fast" complaint and **the entire frame-time
regression** below: cascading went back to 11.85 ms/step, 71% of a 60 Hz frame, awake
chunks 135/135 → 105/135. The shelf and column fixes survive, because those come from the
post-diagonal slide and not from the speed.

**The idea was wrong on its own terms and the code now says so where someone would try it
again.** It was adopted to fix motion reading as "step based", and it cannot: a grain
moves in whole cells on a fixed tick, so doubling its speed does not animate a fall more
finely, it makes each jump twice as far — 8 screen pixels instead of 4, which is *more*
stepped. Stepping is a property of grid cells on a fixed tick and is not reachable from
`step_powder` at all. **E5's free-particle layer is what fixes it**, since matter in
flight there has a real position and can be drawn between cells. That is now a second,
independent argument for E5.

**~~⚠️ Performance regression~~ — resolved by the above.** For the record, since the
measurement is the useful part: `grid_bench`
`cascading` goes **12.17 → 19.7 ms/step, 118% of a 60 Hz frame** — over budget, and the
benchmark flags it. Measured cleanly: the acceleration is free (caps of 1, 2, 3 and 4 all
land within noise of each other), and the entire cost is the post-diagonal slide, which
roughly doubles the swaps in a world where everything is cascading and takes the awake
chunk count from 105/135 to 135/135.

The mitigating facts, stated rather than assumed: `cascading` is a synthetic worst case at
960x540 with the whole world in motion, the shipping world is 640x400 with only part of it
moving, and session 1 measured a steady 164–165 fps in the real game. That is a reason to
think the budget is not being blown in practice; **it is not a reason to leave a 118%
number unexamined**, and PERFORMANCE.md's bracketed method is what should settle it.

### A note on measuring A7

Worth recording because the first two measurements were both wrong and only the third
was informative.

- A count of "laterally isolated cells" went 8 → 5 with the fix. Real, but it could not
  say whether the survivors were harmless one-cell overhangs or genuine spikes.
- Switching to "longest horizontal run with nothing underneath" read **4 cells both with
  and without the fix** — which looked like the fix doing nothing. It was the probe
  measuring **its own 4-cell-wide source row**, which is an overhang by construction.
- Excluding the source rows: **4 cells before, 2 after.**

**A metric that gives the same answer before and after is evidence about the metric at
least as often as it is evidence about the fix.** The instinct to trust it and conclude
the fix was worthless would have been wrong here, and it is the same shape as A2 — an
instrument reporting confidently and incorrectly. Both times the tell was that the
number disagreed with a mechanism that had already been read in the source.

`grid_bench` after the fix: cascading 12.14 ms/step, churning 3.13 ms/step — the two
scenarios that exercise fluids most, both unchanged within noise. The extra check is one
array read on a path that already reads its neighbours.

### Observations (feelings, not defects)

These are experiences reported during play. Each names a real problem; none of them is
a specification, and the fix suggested alongside is a hypothesis.

- **B1 — The dig marker is hard to read.** *(built — V10, first half)* Requested as an
  open crosshair (four non-intersecting ticks) that follows the cursor everywhere and
  dims outside dig range. The underlying problem was real and narrower than the request:
  the marker was a filled rect exactly one cell wide in the same orange family as Fire,
  so it disappeared against the one thing you most want to aim at.
  - **Two things were added beyond the request, both because the stated fix would have
    left the finding half-addressed.** The reticle is drawn in *screen pixels* rather
    than cells — a cell is four pixels at the current scale, and a shape drawn inside
    four pixels is not a shape. And each arm sits on a dark outline, because a white
    reticle vanishes against snow and steam in precisely the way the orange one vanished
    against fire; outlined, one of the two always contrasts.
  - **The impact marker was removed outright on review**, after first being kept. The
    argument for keeping it was that the reticle answers "am I in reach" while the mark
    answers "where does the shot stop", and those differ. That is still true, and it is
    the cost being paid: **out of range, or through cover, nothing on screen now shows
    where the shot actually lands.** Judged worth it — two markers for one action read as
    clutter, and the information was only ever needed in the two cases where the dig is
    about to disappoint you anyway. Worth revisiting if aiming through cover starts
    feeling blind.
  - **The OS pointer is hidden inside the window**, so the reticle is the only cursor.
    Drawn only while the window has mouse focus: `SDL_GetMouseState` keeps reporting the
    last in-window position after the pointer leaves, so without that guard the reticle
    sticks to the edge and reads as frozen UI rather than as a cursor that went
    elsewhere.
  - **A false check was nearly written into README doing this.** The obvious claim — that
    an out-of-range crosshair refuses to dig — is wrong: `march` truncates the ray at
    `RANGE` and still cuts the first solid cell along it, so a dim crosshair aimed past a
    near wall digs that wall. Dim means *the cell under the crosshair is not the cell that
    gets hit*, which is a narrower statement. Caught by reading `tool.cpp` rather than by
    assuming, and it is the same hazard the correctness pass named: a checklist item that
    asserts the wrong thing fails silently, because it passes.
- **B2 — Selecting materials is slow.** Requested as a hotbar with per-material icons
  and key numbers. **Part of this was A2 and is already fixed** — the readout was stale,
  not the input. What remains is genuine: eight materials behind eight number keys with
  no visible affordance means the mapping has to be memorised.
- **B3 — Steam should collect, wait, then drip increasingly fast and shrink as it goes.**
  The strongest of the four, because it and A5 are the same fix seen from two sides.
  Giving steam its own condensation clock instead of borrowing temperature restores a
  long-lived puff *and* keeps spawn temperature below the ignition floor, which is the
  constraint the ignition fix left behind.
- **B4 — Rigid bodies should tip, topple and roll.** The observation behind it is that
  bodies falling flat and landing flat read as lifeless, which is true. **The requested
  fix crosses an engine boundary that E3 already ruled on** — see E8 in ROADMAP.md.
  Grid-aligned toppling is affordable; free rotation resamples authored pixels and buys
  the feature by breaking the reason the engine is interesting.

### Follow-up — fire measured against reference footage

Seven frames of a Noita scene burning, spanning 1–2 seconds of video, so roughly 10–20
simulation steps between frames. That ratio is what makes the sequence readable at all.

**Caveat on measurement, and it is the same lesson as A7 and A4 above.** The frames are
different pixel widths (714–812), so they are crops or rescales from a video player
rather than a fixed viewport. Nothing positional compares across frames. Every finding
below is measured *against a scene feature* — flame height against plank thickness, burn
front against the plank it sits on — and the ones that would have needed absolute pixels
were dropped rather than estimated.

| # | Observation | Consequence for us |
|---|-------------|--------------------|
| 1 | The plank is still there under the fire. Flame occupies the **air around** the fuel; a gap only appears after sustained burning. | We convert `Wood → Fire`. The fuel *is* the flame. Wrong at the root. |
| 2 | Flame contents change **completely** between frames — not shifted, a different set of lit pixels. | Flame cells live ~5–15 steps. Ours lived 180. |
| 3 | The burn front advances about its own width in 1–2 s. | Fuel lasts seconds; flame lasts a moment. **Two numbers on two cells.** |
| 4 | Flames rise, detach as embers, fade out. | Directionally what we do, at the wrong scale. |
| 5 | Colour ramps white-hot at the fuel → orange → dim red at the tips. | One row plus jitter cannot produce this. Needs age. |
| 6 | Fire is a layer hugging the fuel surface; interiors never burn. | Falls out for free if burning cells emit into *empty* neighbours. |
| 7 | Wood darkens to char before disappearing. Three states, not two. | We have two. |
| 8 | A large soft glow lights the cavern tens of cells out. | **Not simulation.** V7. |
| 9 | Smoke haze above the fire. | No such material. Not scheduled. |

**The finding is 2 and 3 together, not either alone.** Fuel duration is long, flame
duration is short, and they are quantities on different cells. Wave 2 collapsed them
into one — it put the long duration on the flame and then pinned the flame in place to
make propagation work. A 180-step stationary orange cell is not a fire; it is an orange
block, and no amount of correct propagation would have made it read as burning.

Worth being precise about what wave 2 got *right*, because it is not being reverted:
duration is tunable per material, fire spreads along a horizontal beam, and the tests
that pin both stay. What changes is which cell owns the timer.

#### Named, observed, and deliberately not scheduled

Three things stand between "correct fire" and "fire that looks like the footage", and
none of them is a fire problem:

- **Cell granularity.** `Camera::SCALE` is 4, so a cell is a 4×4 screen block and the
  viewport shows 200×150 cells; the reference renders far more cells per screen. A
  five-cell flame layer is a fine gradient there and a stack of large squares here. This
  is a resolution decision with a P-tier cost, not something fire work can reach.
- **No additive accumulation.** Overlapping flames in the reference brighten at the
  core. Our pixel buffer writes one opaque colour per cell — `element.h` already notes
  that translucency would need the buffer to composite rather than overwrite.
- **Smoke**, which is a new material and a new question about how it clears.

They are written down here so that the next person to compare our fire against this
footage finds the gap already explained instead of re-opening the simulation.

### What this session changed about the plan

Four ROADMAP edits, and no defect list added there:

1. **E8 rescoped** — toppling and rolling separated explicitly, with rolling deferred
   behind E5 rather than left implicit in one item.
2. **A5's open question closed** — steam lifetime is a restructure, not a tune, and the
   answer is recorded rather than left as a judgement call to be re-made.
3. **A UI item created** — B1 and B2 are player-facing and had no home on any track.
4. **The window-driving tooling item argued forward** — five of eight defects are
   visual, and A1 is the first defect in this project that no headless test could have
   caught. That is the trigger condition its own note names.

---

## Session 2 — 2026-08-02 — fire, seen for the first time

The playtest wave 2b was waiting on. Seven notes across two rounds, all on fire, all visual, and
**every one of them a look rather than a behaviour** — nothing in this session
says the simulation did the wrong thing, which is itself the result: the fuel/flame
rebuild's *model* survived contact and only its numbers, its palette and its
lighting did not. The second round (C8) came back on the lighting alone, with the
burn and flame rates confirmed good.

| # | Note | Cause | Fix |
|---|------|-------|-----|
| C1 | Wood burns ~20% too fast | Ambiguous: burn *duration* or *spread rate* | **Answered: spread rate.** Wood ignition 120 → 150, conductivity 90 → 72. Burn-through of a 150-cell plank 1923 → 2632 steps, ~37% slower. |
| C2 | Flames move ~10% too fast | Fire rose exactly one cell per step | `FLAME_RISE_SKIP_PERCENT` — a gas moves a whole cell or none, so 0.9 cells/step is a *skipped step*. Measured at 90% of steps. |
| C3 | Burnt wood is jet black, should be charcoal | `0xFF2A211B` | `0xFF3A3431`, jitter 6 → 10. Near-black over V1's dark blue backdrop reads as a *hole*, not a material. |
| C4 | Burnt wood should persist 1 second longer | Charred decay at 60 per myriad | 46 per myriad. Measured 2.86 s → 3.85 s on one probe: **+0.99 s.** |
| C5 | Flames have a hard height cutoff | Fixed 12-step lifetime | Lifetime jittered 8–18 steps |
| C6 | Uniform flame height looks unnatural | *Same cause as C5* | *Same fix* |
| C7 | Flame colours lack intensity | Two-stop linear ramp | Three stops, bent through saturated orange |
| C8 | Lighting blown out and overexposed | Shallow falloff, coverage ignored, no tone mapping — three causes | See below. Peak 255 → 124, view brightly lit 58% → 17% |

### The two notes that were one defect, and the one that was a colour-space problem

**C5 and C6 have a single cause and neither is really about height.** A flame rose
exactly one cell per step and lived exactly twelve steps, so every flame in the
world died exactly twelve cells above the fuel that threw it — a straight
horizontal line across the top of a fire, which is the one shape nothing in nature
makes. Nothing needed to change about how flames *move*; what was missing is that
real flames do not all live equally long. One jittered constant closes both notes.

**C7 was not a choice of colours, it was the path between them.** The ramp
interpolated linearly from near-white `(255,242,200)` to dull red `(196,46,16)`,
and the straight line between those two points in RGB runs through greys and dusty
salmons — the shortest path between a near-white and a dark red goes nowhere near
saturated orange. The most-visible middle of every flame's life was being spent in
the least saturated colours available. Bending the ramp through a saturated orange
at 55% of life keeps the whole of it on the outside of the colour space. **The
lesson generalises past fire:** "lacks intensity" is more often a statement about
what a gradient passes through than about its endpoints.

### C4 forced a change to the tuning column itself

The request was "one second longer" and **it could not be expressed.** Charred's
lifetime is a decay chance, and at per-mille resolution the two available values
either side of the answer were 6 (2.78 s) and 5 (3.33 s). The value was
comfortably inside the column's range and still not adjustable within it, so the
column went to per-myriad. The rule worth keeping: **the resolution a tuning
column needs is set by the smallest change anyone will want to make to it, not by
the largest value it has to hold.** That is the second time this column has been
widened and the first time for this reason.

Both the before and after were measured on the same throwaway probe rather than
computed, and the two differ by more than expected: the closed-form mean at 46 per
myriad is 3.62 s, and a slab actually burns for 3.85 s, because decay is only
rolled for *awake* cells and the interior of a uniform body of Charred reaches
equilibrium with itself and sleeps through rolls it would have lost. **A real burn
front is thinner and sleeps less**, so it should sit nearer 3.62 — which is worth
watching for in the next playtest rather than trusting.

### Where the measurement was wrong before the code was

Two probes lied before either was believed, and both lied *plausibly*:

- **The first scenes floated.** Charred is structural, so a slab placed in mid-air
  fell, and a scan at fixed coordinates reported the wood as having burned away in
  three steps. This is the identical mistake session 1 made three times; every
  scene is now anchored to the sealed bottom edge.
- **A single-cell observer could not count flames.** Watching one cell for "when
  does the fire go out" reported lifetimes of 6–30 steps for a quantity bounded at
  8–18, because Charred re-emits into the only empty cell it has and a flame can
  die and be replaced *within one sweep* — so the observer saw unbroken Fire across
  two flames. It looked like a bug in the engine and was a bug in the measurement.
  The test now walls the source off the instant it lights.

### C8 — the lighting was blown out, and all seven suites passed on it

Reported from a screenshot (`resources/video_screenshots/bugged_fire.png`) with
the rest of the fire pronounced good: **"the lighting effects look completely
blown out and over exposed... the crazy flare effects are bad."** The frame is a
flat yellow-white wash with the terrain invisible inside it.

**Three separate causes, and the tuning number was the least of them.**

1. **The falloff was far too shallow.** `TRANSMIT_CLEAR` was 0.86 per four cells,
   which is 0.963 per *cell* — half brightness still twenty cells out, a tenth of
   it at sixty. A number that reads as gentle compounds into something enormous.
   Now 0.72.
2. **Brightness ignored how much of a block was burning.** Taking the hottest cell
   is right for deciding *whether* a block emits — one flame in a block of air
   must not be averaged away — but using it for brightness too means a single
   stray flame lights as hard as a solid wall of fire. With max-propagation on top,
   a ragged mostly-empty flame front lit like a solid slab. Emission now scales
   with coverage above a floor.
3. **There was no tone mapping, and `MAX_EMISSION` was 1.7 *on purpose*.** The
   original reasoning — headroom so the brightest cells clip to white — was wrong
   twice: this layer is composited additively over a scene that is already bright,
   so clipping happens in the blend regardless, and driving a signal past its
   ceiling destroys every gradient above the ceiling rather than only the peak.
   That is precisely the flat white plateau in the screenshot. Now 1.0 through a
   Reinhard curve, which compresses instead of clipping.

A fourth thing was found while fixing those: **the glow was diamond-shaped.**
Propagating to four neighbours only makes distance Manhattan, so light reached
furthest along the axes and appeared as vertical and horizontal shafts out of
every fire — most likely what "crazy flare effects" names. Diagonal neighbours
are now gathered too, at `k^1.5` to pay for the longer step. That is the whole of
the 2.4x cost increase recorded in [PERFORMANCE.md](PERFORMANCE.md).

Measured before and after on the same scene: peak channel **255 → 124**, share of
the view brightly lit **58% → 17%**.

### The instrument that was missing, and now is not

**Every one of the seven suites passed on the blown-out frame, and they were right
to.** They assert that light reaches, stops, falls off symmetrically and is
shaped correctly — all of which was true. None of them could see that the result
was unusable, because none of them ever composites anything.

`preview_light` is the answer: it builds a burning scene, composites backdrop,
cells and light exactly as `main.cpp` does, dumps the frame, and prints the three
numbers that turn "looks overexposed" into something with a before and an after —
peak channel, share of the view brightly lit, share of pixels clipped to white.
`tools/rawpng.py` wraps the dump into a PNG, in fifteen lines of zlib and struct
rather than a new dependency.

**It reproduced the defect before it was used to fix it**, which is the step that
makes it trustworthy: run against the old constants it produced the same yellow
wash at the same 75%-of-blocks-lit. An instrument that only ever agrees with the
change you already made is not evidence.

The generalisable version, and the reason this is written up rather than filed:
**a test suite that never composites cannot catch a compositing defect, and no
amount of adding assertions to it will change that.** `test_light` has gained a
guard for this specific blowout — peak below 200, brightly-lit area under 45%,
mutation-checked against the old constants, which fails it on both — but the guard
was only writable *after* the preview showed what to assert.

### C1 answered, and the tuning values are now the player's

**C1 was the spread rate, not the burn duration** — resolved by the change itself
rather than by discussion: Wood's ignition point went 120 → 150 and its
conductivity 90 → 72. Measured on a 150-cell plank lit at one end, burn-through
went from 1923 to 2632 steps, **~37% slower**, and the plank is still fully
consumed, which is the thing that had to be checked. The margin that matters is
not the percentage but Charred's `heat_source` of 200: an ignition point at or
above it means fire can never light its neighbour. 150 leaves 50 degrees.

**Six values across three files are now explicitly placeholders** — Wood's
ignition point and conductivity, and `TRANSMIT_CLEAR`, `TRANSMIT_SOLID`,
`MAX_EMISSION` and `COVERAGE_FLOOR` in the light field. They are expected to
move, so the comments around them were rewritten to describe **the shape of each
knob and what breaks at its limits**, rather than to justify one measured
number - a comment that argues for a specific value is wrong the first time
someone changes it, which is the failure mode this whole file exists to catch.

The lighting values in particular are a *look* decision and are recorded as one:
at `TRANSMIT_CLEAR` 0.55 the glow is visible ~15 cells out and bright within ~8,
against ~35 and ~12 at 0.72. **That is a deliberate departure from V7's own
argument for being pulled forward**, which cited reference footage of a cavern
lit tens of cells from the flame. A tight rim light is a defensible and arguably
better look — a contained fire reads as hotter — but it is a different one, and
noting it here means the next person to read that roadmap entry is not confused
by the mismatch.

### A wrong claim made while C1 was open, corrected

While asking which reading C1 meant, this log argued that if it meant *spread
rate* then no tuning pass could deliver it: "conduction reaches Wood's ignition
point in about two steps, and at that granularity a 20% change is not expressible
at all — the fix is a structural change to how ignition is paced."

**That was wrong, and the arithmetic behind it was wrong in a specific, checkable
way.** It divided the temperature gradient by 255 where `heat_flow` actually
divides by `CONDUCTION_DIVISOR`, which is **1024** — four times smaller flow per
step, so ignition takes tens of steps rather than two. The measured spread rate
is about 0.085 cells per step, roughly twelve steps per cell, which is plenty of
room to tune. The change that answered C1 was two numbers and moved the rate 37%.

Worth keeping because of *how* it was wrong. The claim was reasoned from source
that was read but not run, and it was the kind of claim that discourages someone
from trying the simple thing first. Every other number in this session's entry
was measured; this one was derived, and it was the only one that was false. The
project already has a rule for performance work — a number without a measurement
is not evidence — and this is the same rule arriving in simulation tuning.

---

## Session 3 checklist — the pass that closes wave 2b

**This is a supplement to README's Manual Tester Checklist, not a replacement.** Run
that one first, in full: wave 2b touched `src/physics/` and the render path, so steps
1–9 all apply, and step 5 in particular has been rewritten under this fire and has
never been run against it. What follows is the *additional* pass, and it exists
because the general checklist asks whether fire is correct and this wave's open
question is whether it is any good.

**Record before starting:** build hash, Release/MSVC, world seed, the `Scene:` line,
and the suite count. A finding without its seed is not reproducible and F1 exists
precisely so that it can be.

### Order is load-bearing and the file has already argued for it

Do phase A before phase B, and **do not skip ahead when the lit scene looks good** —
an emissive layer flatters a bad flame, which is the whole reason wave 2b names these
as two questions rather than one. If you cannot separate them by looking, the
falsification is available: `preview_light` composites the same scene with and
without the light pass, and `tools/rawpng.py` turns either into a PNG.

### Phase A — does the fuel/flame model read as fire?

Judge these with the glow present but ignore it. The question is the *shape and
motion of the flame band*, not how bright the room is.

| # | Check | What it catches |
|---|-------|-----------------|
| A-1 | Light one end of a long **horizontal** wood beam, grounded on the sealed floor. A burn front advances along it. | The A4 defect the fuel model was built for. Session 1 measured this wrong three times by hanging the beam in mid-air — ground it, or you are testing E3. |
| A-2 | The same beam **vertical**. Front advances at a comparable rate. | Vertical used to be the only case that worked, for the wrong reason. |
| A-3 | Watch a burning beam for several seconds. **The wood is still there under the flame**, in a charred state, and a gap only opens after sustained burning. | Reference finding 1 — the defect wave 2 shipped. If fuel is vanishing into flame, the rebuild did not land. |
| A-4 | Watch one spot on the fire across a second. **The lit cells are a different set, not a shifted one.** | Reference finding 2. Flames should live 8–18 steps. A flame that persists is the 180-step orange block again. |
| A-5 | Look along the **top edge** of a broad fire. It is ragged, not a straight horizontal line. | C5/C6 — the jittered lifetime. A flat top means the jitter is not reaching the flame. |
| A-6 | Watch a single flame from fuel to tip. Colour runs white-hot → saturated orange → dim red, and **never passes through grey or salmon**. | C7 — the bent ramp. Desaturation mid-life means the three-stop ramp regressed to a straight line in RGB. |
| A-7 | Burn a wide wood slab and time how long the **charred** remains before it decays. | C4's open prediction: a real burn front is thinner and sleeps less than the test slab, so this should read nearer **3.62 s** than the 3.85 s measured on a probe. Trusting 3.85 is the thing this row exists to prevent. |
| A-8 | Burn a plank end to end and check it is **fully consumed**, and that fire still lights its neighbour. | The C1 margin: Charred's `heat_source` is 200 and Wood's ignition point is 150. Fire that stalls mid-plank means that gap closed. |
| A-9 | Judge the **spread rate** against C1's retune (ignition 120 → 150, conductivity 90 → 72, ~37% slower). Still too fast? Still too slow? | C1 was the note that reopened this wave. Answer it in the same terms it was asked. |
| A-10 | Rate the whole thing on one question: **does this read as burning?** | This is the actual exit condition. A-1 through A-9 can all pass on something that still looks wrong, and if they do, say so here rather than declaring the wave closed on the table. |

### Phase B — is the glow carrying it or hiding it?

Only after A-10 has an answer.

| # | Check | What it catches |
|---|-------|-----------------|
| B-1 | Look at a fire against dark terrain. **No flat white plateau**, and terrain inside the lit area is still legible as terrain. | C8's headline symptom. `MAX_EMISSION` went to 1.0 through a Reinhard curve for this; a wash means tone mapping is not doing its job. |
| B-2 | Look for **vertical and horizontal shafts** out of a fire — the glow is round, not diamond-shaped. | The fourth C8 cause: four-neighbour propagation makes distance Manhattan. This is most likely what "crazy flare effects" named. |
| B-3 | Judge the **reach**. At `TRANSMIT_CLEAR` 0.72 the glow is visible ~35 cells out and bright within ~12; at 0.55 it is ~15 and ~8. | **This is a look decision and is explicitly unresolved.** V7's own argument cited a cavern lit tens of cells out; a tight rim light is defensible and different. Pick one deliberately — this row is a question, not a pass/fail. |
| B-4 | Move a **single stray flame** away from a solid fire front. It should light weakly, not like a wall of fire. | The coverage-scaled emission fix. A lone flame lighting hard means `COVERAGE_FLOOR` is too high or coverage is not being read. |
| B-5 | Put fire **in a pit** and behind a wall. Light stops at solids and does not leak through them. | `TRANSMIT_SOLID`. The suites assert this headlessly, so a failure here is a compositing problem rather than a light-field one. |
| B-6 | Run the scene with **no fire in it at all** and confirm it looks exactly as it did before V7. | The reorder's stated boundary: additive only, nothing is darkened. A changed unlit scene means the light pass is subtracting somewhere. |
| B-7 | HUD fps under a large fire, then `preview_light`'s three numbers on the same scene: **peak channel, share brightly lit, share clipped**. | The instrument exists so "looks overexposed" has a before and an after. Session 2's post-fix numbers are peak **124** and **17%** — anything far off those is a regression with a number attached. |

### What to bring back

Screenshots, and specifically of states nobody would think to describe. A7b was only
findable because the images showed a second, unrelated defect (A8) that no amount of
re-reading the written report would have produced. **Every note in session 2 was
visual**, so the default for this session is that a note without an image is
incomplete.

Then: which of A-10 and B-3 got a clear answer. Wave 2b closes on A-10 being **yes**
and B-3 being **decided** — not on the tables being green.

---

## Session 3 results — fire reads as fire, and the glow is the open work

Seed `9418527765904368373`. Scene: 640x400, 27192 cells placed.

**A-10 answered: yes.** The rebuilt fuel/flame model reads as burning. The tester's
words are worth keeping verbatim because they are a tuning brief and not a
compliment: *"it looks like a very flammable piece of material going up in flames
and disintegrating, like a piece of paper, wool, or 'fatwood'."* Nothing in the
model is wrong — every structural check passed (fuel survives under the flame as
Charred, a consumed plank still lights its neighbour, the colour ramp walks
white → orange → red without passing through grey, the top edge is ragged). What is
wrong is that every timescale is a notch fast, and all four A-side findings are that
one observation seen from different angles. **Wave 2b's exit condition is met.**

**B-3 answered: reduce further.** `TRANSMIT_CLEAR` was already at 0.55 for this
session — the tightest of the three worked examples written into `light.cpp`, the
one described there as "a tight rim light". The reach is still long. That is a real
result and not a small one: it says the reach question was never a choice between
the three documented values, and the next value is off the bottom of that table.

**A-4 was not answered** — "I'm not sure". Whether the lit set of cells is a
*different* set second to second or the same set shifted is not a question eyes can
answer at 165 fps, and asking a tester to answer it was a checklist defect. It needs
an instrument (count flame cells born per step, or diff the flame mask between
steps), or it needs dropping.

### Findings

| # | Where | Finding | Suspected cause |
|---|-------|---------|-----------------|
| **A9a** | A-1, A-2 | Burn front is too linear — the boundary between lit and unlit fuel is close to a straight edge in both orientations. | Ignition is driven purely by conduction, which is deterministic and symmetric; nothing jitters *when* a cell crosses its ignition point, only what happens after. |
| **A9b** | A-3 | Wood is fully consumed and gone too quickly; the material reads as tinder rather than as timber. No sense of resilience. | Charred's per-step decay chance (46 per myriad) — the only knob that sets how long fuel persists. |
| **A9c** | A-7 | Charred lifetime lands near the predicted duration but should be longer. | Same knob as A9b. Filed separately because it is a *measured* result and A9b is a felt one; if one knob fixes both, that is a confirmation, not a coincidence. |
| **A9d** | A-9 | Spread rate is still a little too fast, after C1 already slowed it 37%. | Wood conductivity 72 against ignition point 150. C1 moved this once and undershot. |
| **B9a** | B-2 | The glow emits hard rays and shafts. Too geometric. **The headline defect of the session.** | Max-propagation over 4 orthogonal + 4 diagonal neighbours produces an octagonal distance metric, and max (unlike a sum) never averages the axis artefacts away. The shafts are the octagon's edges. |
| **B9b** | B-3 | Reach should come down again, from an already-tight 0.55. | `TRANSMIT_CLEAR`. See above — this is now below every documented worked example. |
| **B9c** | B-4 | A lone stray flame still lights too hard. | `COVERAGE_FLOOR` 0.2 — the floor that deliberately keeps a single flame visible is set too generously. |
| **B9d** | B-5 | Light penetrates walls, and should penetrate both Wall and Wood less. | `TRANSMIT_SOLID` 0.15 is a *transmission*, not a block, and it compounds only once per 4-cell block — a 1-block-thick wall passes more than it looks like it should. Also check `occlusion_of` for Wood. |

Confirmed working and not to be touched while fixing the above: no white plateau
(B-1), unlit scenes identical to pre-V7 (B-6), 165 fps under a large fire (B-7),
neighbour ignition across a fully-consumed plank (A-8), colour ramp (A-6), ragged
edge (A-5).

### Ordering the fixes

**B9a first and alone.** It is the only finding that is a defect in a *method*
rather than a value — the other seven are numbers that are wrong by a notch. And it
contaminates the rest: shafts change how far light appears to reach and how hard a
stray flame appears to light, so B9b and B9c cannot be judged honestly until the
field is smooth. Fixing the cheap number first would mean tuning against an artefact,
which is the session-1 mistake wearing different clothes.

**Then the A-side numbers as one change**, since A9b/A9c/A9d are one complaint. The
risk to watch is that slowing everything uniformly turns a fire that is *exciting and
too fast* into one that is *correct and dull* — the tester liked what they saw, and
"slightly" appears in all four notes. Small steps, re-measured, and A9a is the one
that buys character rather than just time.

### B9a — done, and two of the four things it was blamed on were innocent

**What changed in `src/render/light.cpp`:**

1. **The diagonal exponent, from k^1.5 to an exact k^√2.** The old value was
   defended in a comment as accurate "within a couple of percent", which it is —
   *per block*, and per block was the wrong unit, because the error compounds once
   per block crossed. At the live `TRANSMIT_CLEAR` the diagonal arrived about 4%
   dim per block and roughly half as bright ten blocks out. Per unit of distance
   the diagonal cost k^1.06 against the axis's k^1.0, so the field bulged along the
   axes.
2. **Eight knight's-move neighbours at k^√5.** Orthogonal and diagonal steps alone
   measure distance to about 8%, worst at 22.5° — halfway between the two
   directions they point in. A (1,2) step is the only one here that could jump a
   4-cell wall, so it draws from the lesser of the far block and the block it
   passes through: light can only arrive by a route it has already lit, so a wall
   in the way is carried as darkness rather than routed around.
3. **Two separable 1-2-1 smoothing passes** over the settled field, with taps
   weighted by how open each neighbour is so the blur cannot pull light through a
   wall it just paid to be stopped by.

**Measured, before → after:** glow-roundness ratio **1.273 → 1.197**, and the
furthest direction moved from **270°** (an axis) to **225°** (a diagonal), which is
the mechanism in (1) confirmed rather than assumed. Peak channel unchanged at 118.
All seven suites pass.

**Two hypotheses died, and both are worth keeping:**

- **The emitting block's square shape.** Reach-by-angle showed the residual bulge
  was a near-constant 0.7 blocks at *every* radius rather than a percentage growing
  with distance, and 0.707 is exactly how much further a square's corner is than its
  edge. That measurement was right. The inference from it — that the smoothing
  pass's emission floor was keeping the square sharp — was wrong: removing the floor
  cost peak brightness and moved not one contour, because sixteen iterations of
  propagation had already copied the square outward. A smoothing pass cannot unsmear
  a shape set upstream of it.
- **No penumbra through gaps.** Max-propagation has no soft shadow by construction,
  so a beam through a hole in a wall ought to have a cut edge, which is what "hard
  rays" sounds like. Measured: the edge was **4 blocks wide before the change and 4
  after**. Attenuation through the near-clear blocks around the opening was already
  softening it. Refuted outright.

**The instrument was wrong before the code was.** The first roundness probe sampled
the nearest block and reported a ratio of 1.32 both before *and* after a change to
the metric — the tell this file already knows: a number that does not move when its
subject does is measuring the ruler. Nearest-block sampling quantises each reach by
half a block, ~10% at these distances, which is larger than the 8% artefact and is
itself angle-dependent. It now samples bilinearly, which is also what the GPU does
to this texture, so it measures the shape the player sees rather than the one in the
array.

**Two new assertions in `light_test`**, both written because the existing shape test
could not have caught this: it compares one axis against one diagonal, and the
chamfer artefact's worst error is at 22.5° — precisely between the two directions it
samples. A two-sample test of an eight-lobed artefact looks where the artefact is
not. The replacements sweep the full circle, and check that a beam through a gap has
a gradient rather than a cut.

**Still open, and to be judged by eye and not by this:** the ratio is 1.197 and not
1.0, and no measurement here proves that what is left is what the tester meant by
"hard rays and shafts". The before/after preview frames are visibly smoother — the
halo's faceted lobes are gone — but the scene `preview_light` builds is not the scene
they were playing. **B9b–B9d are now unblocked**, and the next session re-judges the
glow with the field round.

### B9b, B9c, B9d — done, and one of them was not a value at all

**B9b, reach.** `TRANSMIT_CLEAR` 0.55 → **0.52**. Deliberately a small step, and
the reason is in B9c: halving the coverage floor shortens the *apparent* reach of
the ragged, mostly-empty fires that are almost all of what a player sets alight,
without touching a wall of flame at all. Sweeping the constant and measuring open-air
reach at each value showed 0.48 taking a small source about 28% in from where session
3 saw it, which is not "slightly". At 0.52 a big fire loses about 5% and a stray
flame about 12% — the reduction lands mostly where B9c asked for it.

**B9c, stray flames.** `COVERAGE_FLOOR` 0.2 → **0.1**. A single flame cell in an
otherwise empty block was emitting a fifth of what a solid wall of fire does. The
floor exists only to keep a lone flame legible as a light source, and 0.1 still does
that while widening the gap between an ember and a blaze, which is the distinction
the coverage term was added for.

**B9d, walls — and this one was a modelling error, not a number.** Two of them,
stacked:

1. The block's transmission was interpolated *linearly* between the clear figure and
   the solid one. Those two numbers only mean anything as exponents, and averaging
   them as though they were rates is what let a one-cell-thick wall - the commonest
   wall a player draws, and a quarter of a block - pass most of its light.
2. Fixing that exposed the deeper one: `TRANSMIT_SOLID` was defined *per block*, four
   cells wide. Even at a punitive 0.06 that is 0.06^(1/4) = **0.49 per cell** - the
   model was quietly claiming a single cell of rock passes half the light hitting it.
   The two figures were being calibrated against each other in different units.

Occlusion is now stated **per cell** and a block's transmission is the product of
what its cells pass, so a block holding one cell of rock is exactly "three cells of
air and one of rock" and nothing else. `TRANSMIT_SOLID` survives as a derived
quantity for the tests and the smoothing pass to reason about; four cells of rock at
0.12 each is 0.0002, which is to say a solid block is opaque, as it always should
have been. `CELL_LIQUID` and `CELL_GAS` were restated in the same units at the same
time - under the old blend a half-flooded block would have gone nearly black, which
is not what "fluids dim what passes through them" was ever meant to say.

**Measured:** a one-cell wall went from passing **44%** of the open-air control to
passing **none of it**. Peak channel unchanged at 118; blocks lit 39% → 35%; nothing
clipped. All seven suites pass.

**One new test, and the gap in coverage it came from is the point.** Nothing in
`light_test` had ever drawn a *thin* wall - the shadowing test uses a slab twelve
cells thick, which every version of this code has stopped easily. B9d could not have
been caught by this file, and would not be caught by it again. There is now a
one-cell wall with an open-air control at the same distance.

**Two probe distances moved**, from five blocks to three, and it is worth being
explicit that this is not a test bent to fit its subject: B9c deliberately made a
single flame dimmer, the control probe went dark along with the shadowed one, and a
comparison against a probe that is dark because nothing was ever going to reach it
asserts nothing. The pairing - two probes, equal distance, one thing different
between them - is what the test is, and it is intact.

**Still for the eye:** whether 0.52 and 0.1 are the right *look* is not something any
of this measures. The next session judges B-3 and B-4 again with a round field, an
opaque wall, and the A-side timings still untouched.

### A9a–A9d — done, and the log's own diagnosis was wrong on two counts

Wave 2c's A-side, taken as one change because A9b, A9c and A9d are one complaint said
at three volumes. Nine seeds behind every number below; the shipped engine is the
control, measured the same way on the same worlds.

| | shipped | now |
|---|---|---|
| spread, 150-cell beam | 391 steps (6.5 s) | **476 steps (7.9 s)** |
| body of wood fully gone | 22 s | **33 s** |
| mean Charred lifetime | 3.15 s | **4.37 s** |
| burn-front raggedness | 2.59 cells | **3.19 cells** |
| beams that failed to burn through | 0 of 9 | **0 of 9** |

Four constants moved: Wood's conductivity 72 → 40, Charred's decay 46 → 34 per myriad,
a new `temp_jitter` column on the Wood row at 40, and a new `IgnitionPoint` stream.
Wood's ignition point did **not** move, and that is a finding rather than an omission.

**The instrument came first, and it was wrong three times before it was right.** There
was no burn-timing probe in the repo at all — session 2's "1923 → 2632 steps" was
measured by hand and left no tool behind — so `tests/burn_probe.cpp` is new. Its first
scene floated, and the collapse system dropped the plank out from under it: a 150-cell
beam "burned through" in five steps, which was gravity being reported as fire. Its
second scene sat the plank on a Wall floor, which held it up and ruined the reading a
different way — `material.h` says outright that a wall is a heat sink big enough to
quench any fire touching it, so that plank spent its whole length fighting the coldest
large body in the scene. That version could not propagate at all once thresholds
varied, and the failure looked exactly like a defect in the jitter. **Fourth time a
floating or badly-anchored scene has answered a question nobody asked** — the count is
kept in the A3 lifetime test's comment and it is now the most reliable bug in this
project.

**Two of the log's stated causes for these findings were wrong.**

*A9d's cause was named as "Wood conductivity 72 vs ignition 150", and the ignition
point turned out to be immovable.* 156 and 162 were both measured; both stall a
three-cell beam on two seeds in five. Charred holds itself at 200, so every degree
added to 150 is taken out of a 50-degree margin that is already the reason fire
propagates at all. The knob that survived is conductivity alone, and it has a hard
cliff of its own — 40 burns through nine beams of nine, 34 burns through five. Not a
slowdown: below the cliff a cell sheds heat to ambient faster than the front feeds it
and the fire stops. `material.h` claimed conductivity "can never prevent" ignition
because conduction has a floor of one unit per step. The floor is real; the conclusion
does not follow, and the comment has been corrected.

*A9a's cause was named as "nothing jitters when a cell crosses its ignition point",
which is true and points at the wrong fix.* Making ignition a roll rather than a
certainty is the obvious reading of that sentence, and it was tried first: raggedness
went 0.75 → 1.06 cells while the front slowed 2.3x, and a three-cell beam stopped
burning entirely because the Charred ahead of the front burned out before the next cell
won its roll. Jittering the **threshold** instead buys far more shape per degree, for a
reason that is about the temperature field and not about randomness — ahead of a slow
front the gradient is shallow, so a cell needing ten degrees more than its neighbour
crosses its line several *cells* later, not a moment later.

**The jitter is one-sided, and that was bought with a measured regression.** Varying the
threshold both ways reads better — wood has knots as well as soft spots — and across
nine seeds the shipped engine burned through all nine while the symmetric version
stalled one. A cell that draws twenty degrees high, in a beam three cells thick, is a
plug the front cannot get past. A fire that sometimes refuses to cross a stick is a
worse defect than a straight burn front, so the dice may now only ever make a cell
*easier* to light: 150 is the hardest any cell is, not the average. That also means the
jitter speeds fire up, which is why conductivity had to fall to 40 rather than the 64 a
jitter-free engine would have wanted. Two of the four changes are paying for each other.

**One knob had no cliff and no coupling at all:** Charred's decay. Spread reads the same
to within noise at every value tried between 46 and 20, so "how long wood resists" and
"how fast fire crosses it" are genuinely two dials. It is the only one of the four that
was free, and it is also where most of the visible answer to *"the wood is fully gone
very quickly"* came from — 22 s to 33 s for a body to disappear.

**Two checks were added and one was widened.** `random.h`'s stream-distinctness
`static_assert` did not cover `FlameLifetime` or `FlameRise` — they had been left out of
`SIM_STREAMS` since they were added, so the check that exists to catch a silent stream
collision was silently not watching them. A second `static_assert` now holds the
jitter's *bottom* above the coldest ignition point in the table, because
`lowest_ignition_point()` reads `min_temp` and knows nothing about `temp_jitter`; widen
the jitter past 60 without it and a puff of steam could light a wall. And `grid_test`'s
A3 lifetime bar had a ceiling of 320 against a mean of 217 — an upper bound one retune
above the current value, which is an equality check wearing a range's clothes. It broke
for the ordinary reason that somebody tuned the thing it watches. Now 900.

**Still for the eye, and this is the whole of what is left in wave 2c:** every number
above is a ratio against a control, and not one of them says the fire *looks* right.
Session 4 judges A-1 through A-10 and B-2 through B-5 together, which is why the A-side
was done before that session rather than after it.

---

## Session 4 checklist — the pass that closes wave 2c

**Run README's Manual Tester Checklist first, in full.** Wave 2c changed four constants
in `src/physics/` and rewrote the occlusion model in `src/render/`, so steps 1–9 all
apply. What follows is the additional pass.

**Record before starting:** build hash, Release/MSVC, world seed, the `Scene:` line,
suite count. F1 exists so a finding can be reproduced from the seed; a note without one
is an anecdote.

### A-4 has been deleted, and the reason is the useful part

Session 3's A-4 asked the tester to watch one spot and judge whether the lit cells were
a *different* set second to second or the *same* set shifted. The honest answer that
came back was "I'm not sure", and that is the checklist's fault, not the tester's: at
165 fps nobody can hold two frames of a flame band in their head and diff them. **A
check whose answer a human cannot produce is not a check, and its "I'm not sure" is
indistinguishable from a pass.** It is gone from this session. If flame turnover is ever
in doubt it needs an instrument — diff the flame mask between steps and report the
share of cells that survived — and instruments belong in `tests/`, not in a table
somebody reads at a keyboard. The row is not carried forward as a to-do; write the tool
if the question comes back.

### What changed since session 3, in the terms you judged it in

Session 3's notes are the control. Where you said "slightly", here is what slightly
turned out to be, so you can answer *too far / not far enough* rather than starting from
scratch:

| You said | Now |
|---|---|
| A-9, A-10: spread still a little too fast | **22% slower** end to end (6.5 s → 7.9 s across a 150-cell beam) |
| A-3: wood is fully gone very quickly | **50% longer** to disappear (22 s → 33 s for a body of wood) |
| A-7: charred could persist slightly longer | **39% longer** (3.2 s → 4.4 s per charred cell) |
| A-1, A-2: burn front could be less linear | **23% more ragged** — the weakest of the four, see A-3 below |
| B-2: hard rays and shafts, too geometric | Field is round at every angle, not only on the axes and diagonals |
| B-3: reach could be reduced slightly | `TRANSMIT_CLEAR` 0.55 → 0.52 |
| B-4: stray flame should light even weaker | `COVERAGE_FLOOR` halved, 0.2 → 0.1 |
| B-5: light penetrates walls | A one-cell wall passed 44% of the light through it; it now passes none |

### Ground every test scene. This is the fourth session it has bitten

A beam hanging in mid-air is not a slow-burning beam, it is a falling one, and the
probe, the A3 lifetime test and two earlier sessions have all reported gravity as a
result about fire. **Build beams on posts or on the sealed floor.** A plank lying
*along* a wall is the other trap and it is less obvious: `material.h` says a wall is a
heat sink big enough to quench any fire touching it, so a beam resting on one burns
against the coldest large body in the scene and will read far too slow.

### Phase A — the fire's own timing and shape

Judge with the glow present but ignore it; an emissive layer flatters a bad flame, which
is why these are two phases and not one.

| # | Check | What it catches |
|---|-------|-----------------|
| A-1 | Light one end of a long **horizontal** beam, grounded. Judge the pace against session 3's "still a little too fast" — is 22% slower now right, still fast, or has it gone dull? | A9d. **The one to be suspicious of.** Wood's conductivity fell 72 → 40, and the cliff where fire stops propagating entirely is at 34. There is very little room left below this, so "still too fast" needs a different mechanism, not a smaller number. |
| A-2 | The same beam **vertical**. Comparable rate. | Vertical was once the only case that worked, for the wrong reason. |
| A-3 | Watch the burn front along a broad beam. **Is it still a straight line sweeping along, or does it gnaw unevenly?** | A9a, and the change least likely to have landed. 23% more raggedness on a quantity that already varies 2x between worlds may simply not be visible. If it reads unchanged, say so plainly — there is headroom to 60 on the jitter before an assert stops it, and past that it is a different mechanism. |
| A-4 | Burn a plank end to end. **Fully consumed, and fire still lights its neighbour.** Try a beam only two or three cells thick. | The margin, and the specific regression this wave nearly shipped. A thin beam is where a fire that sometimes refuses to cross shows up; nine seeds say it never does, and nine seeds is not the world. A stall here is serious — report the seed. |
| A-5 | Watch a burning body for several seconds. **The wood is still there under the flame**, charred, and a gap opens only after sustained burning. | Reference finding 1. If fuel vanishes into flame the E9 model has regressed. |
| A-6 | Time how long **charred** persists before it decays, and judge the body as a whole — does wood now read as timber rather than tinder? | A9b/A9c, and the change most likely to be visible. Session 3's phrase was "a sense of resilience"; that is the word to judge against. |
| A-7 | Look along the **top edge** of a broad fire. Ragged, not a straight horizontal line. | C5/C6, the jittered flame lifetime. Unchanged this wave — this row is a regression check. |
| A-8 | Watch a single flame from fuel to tip. White-hot → saturated orange → dim red, **never through grey or salmon**. | C7's bent ramp. Unchanged this wave; regression check. |
| A-9 | Rate the whole thing on one question: **does this read as burning, at a pace that feels right?** | The exit condition. A-1 through A-8 can all pass on something that still looks wrong. Session 3's answer was "yes, but like paper or fatwood" — the question now is whether it has moved toward timber without becoming dull. |

### Phase B — the glow, re-judged on a field that has changed shape

Only after A-9 has an answer.

| # | Check | What it catches |
|---|-------|-----------------|
| B-1 | Fire against dark terrain. **No flat white plateau**; terrain inside the lit area still legible. | C8's headline symptom. Regression check. |
| B-2 | Look for **shafts and rays** out of a fire. The halo should be round at every angle — including the diagonals *between* the axes and the corners, which is where the octagon used to bulge. | B9a. The old artefact was worst at 22.5°, precisely between the two directions the old test sampled, so look off-axis rather than along it. |
| B-3 | Judge the **reach** again. Session 3 asked for slightly less and got 0.55 → 0.52, plus a halved coverage floor that shortens ragged fires more than solid ones. | **A look decision, still explicitly open.** Answer in the same terms: right, still too far, or now too tight. |
| B-4 | Move a **single stray flame** away from the fire front. It should light weakly. | `COVERAGE_FLOOR` at 0.1. Session 3 said "even weaker" against 0.2. |
| B-5 | Fire behind a **one-cell wall**, and fire in a pit. Light stops dead at the solid. | B9d. This is the one with a hard before/after — 44% through a thin wall, now 0% — so a leak here means the compositing, not the field. |
| B-6 | A scene with **no fire at all** looks exactly as it did before V7. | The additive-only boundary. |
| B-7 | HUD fps under a large fire, then `preview_light` on the same scene. | **Re-check, not a regression check.** Propagation gained eight knight's-move taps per block per iteration and four smoothing sweeps since the last fps reading, and session 3's 165 fps was measured before any of it. Current instrument numbers on the reference scene: peak channel **118**, **35%** of blocks lit, **0.0%** clipped. |

### What to bring back

Screenshots, including of states nobody would think to describe — A7b was only findable
because an image showed an unrelated defect that no written report would have produced.
A note without an image is incomplete.

Then, specifically: **A-9 and B-3.** Wave 2c closes on A-9 reading as a fire that burns
at a believable pace, and B-3 being *decided* rather than deferred a third time. The
tables being green does not close it.

And one thing that is not a row: if any of the four A-side changes reads as *no change
at all*, that is the most valuable note in the session. Three of them are worth 22–50%
and one is worth 23% on a noisy quantity; knowing which of those a person can actually
see is what tells the next wave where to spend.

---

## Session 4 results — wave 2c closes, and nothing is carried

All sixteen rows pass: **A-1 through A-9 and B-1 through B-7**. No findings.

**A-9 answered: yes.** The fire reads as burning at a pace that feels right. Session 3's
answer was "yes, but like paper or fatwood"; the four A-side changes moved it toward
timber and it has not gone dull. The exit condition is met.

**B-3 decided, not deferred.** `TRANSMIT_CLEAR` at 0.52 with `COVERAGE_FLOOR` at 0.1 is
the right reach. This is the third session the row was asked in and the first it was
answered — it is settled and not to be reopened without a new observation.

**B-7 re-checked, not assumed.** Performance is stable under a large fire after the eight
knight's-move taps and four smoothing sweeps added since the 165 fps reading. The
propagation cost the wave added is affordable.

**What was not recorded, and is the session's one gap:** the checklist asked, outside the
tables, which of the four A-side changes read as *no change at all* — the note that would
tell the next wave where to spend. No such note came back. A blanket pass is not the same
answer as "all four were visible", so the raggedness change (A9a, the 23% one on a
quantity that varies 2x between worlds) is closed as *not contradicted* rather than as
*confirmed visible*. If burn-front linearity is ever raised again, start from the jitter
knob — there is headroom to 60 before an assert stops it — and do not re-derive that it
was already moved once.

**Wave 2c is closed. Wave 3 (A6) starts.**

---

## A6b — the water elevator, found because A6's fix stopped hiding it

Three screenshots, `resources/video_screenshots/water_issue_*.png`, taken against a
build that already had A6's fix in it. Water appears at the very top of a sand column
tens of cells above the pool, the moment the falling sand reaches the water, and then
runs down the outside of the pile in ragged clumps rather than as a sheet.

**This is not A6's fix misbehaving, and that was worth establishing before touching
anything.** `water_probe` pours a size-3 sand brush from 25 cells of open air above a
2000-cell pool — the brush never touches water at any point in the run — and reports
water above the *cursor*, which nothing but a lift can explain:

| | water above the cursor, step 50 | pool at step 200 |
|---|---|---|
| A6 fix in (today) | **32 cells** | 2000 of 2000 |
| A6 fix out (old code) | 5 cells | **1352 of 2000** |

The lift is in both. What the old code did was *eat* it: the brush deleted the water the
moment it arrived at the disc, at roughly the rate the column delivered it, and 648 cells
of the pool were gone by step 200. The elevator has been running since before wave 1 and
was invisible because a second defect was consuming the evidence. **A conservation fix
did not cause this; it made it visible, which is the argument for having made it.**

Both halves of the report are one defect. `can_displace` lets a powder swap with any
lighter fluid below it, and a swap moves the water *up one cell into the grain's old
place*. Under a continuously fed column there is always another grain above, so the
exchange repeats every step and the water is handed up the stream without bound —
nothing in the rule refers to where the water's surface is. It spills out at the top of
the column and trickles back down the flanks, and that trickle is the "staggered tree"
pattern: it is not a flow model problem, it is the same lifted water coming home.

Two numbers worth keeping. It is fast — nothing above the cursor at step 20, 25 cells at
step 30, arriving with the first grains, which is exactly the report's "the second the
sand touches the water". And it is not permanent: 400 steps after releasing the brush,
water above the cursor is back to 0. **Volume is conserved throughout (2000 → 2000), so
this is transport and not creation** — which is why it needs a rule change and not a
leak hunt.

### Fixed by sending the displaced fluid sideways, to its own surface

`vent_fluid`. When a grain sinks into a fluid, the fluid goes to a free surface of its
own kind within `VENT_RADIUS`, and the grain drops into the vacancy rather than trading
places with it. The straight swap is still there as the fallback, which is the right
answer for a grain deep inside a body: there is no conveyor above it, so its one-cell
lift never adds up to anything.

Two things the probe found that reasoning did not:

- **A pool surface is not the only destination needed.** The first version sent the
  fluid only to open water, which fixed the pour and then let the defect return at step
  175, once the sand had built a cone standing out of the water. What was left was the
  films of water clinging to the cone's *flanks*, which have sand underneath and so have
  no surface of their own kind to reach; grains rolling down the slope lifted them
  instead. The pile had become the conveyor. Films may now also drain to somewhere they
  can rest, level or downhill and never up.
- **Any-Empty is not a destination.** Also tried, also reproduced the defect: the nearest
  empty cell to a grain entering the water is very often the air just above the sand
  pile, so the water was put back on top of the pile.

| | water above the cursor | first appears |
|---|---|---|
| before | 32 cells, rising to 1160 in the air | **step 30** |
| after | 3 cells | step 350 |

**`VENT_RADIUS` is a cost knob and was swept, not picked.** Against `churning` at
3.13 ms/step with no venting: r=2 is 4.12 ms and clean to step 200, r=3 is 4.93 ms and
clean to step 350, r=4 is 6.72 ms and clean to step 400. **3 is the knee.** `cascading`
is 12.7 ms/step at every radius including zero — this touches the powder/fluid interface
and nothing else.

**Two changes that sounded right and measured wrong, recorded so they are not retried.**
Restricting the vent to grains with another grain on top — the conveyor condition
itself, and apparently exact — made `churning` *slower* (6.7 → 7.3 ms/step, because a
churning mix is stacked grains nearly everywhere, so the test bought no skips), stopped
15 of 135 chunks ever sleeping, and brought the probe's residue back 200 steps earlier.
Routing the *brush's* displaced fluid through `vent_fluid` as well could not be told
from not doing it, to a single cell over 500 steps, so it is not in.

**The residue, named rather than left to be rediscovered.** Once the pile has grown all
the way up to the cursor, a handful of cells — 3 at step 350, 6 at step 500, against 32
at step 30 before — still reach the top. It is water caught inside the mound's peak with
no surface and no downhill within reach of the search. Whether that is visible at all is
a question for a playtest, and it is the first thing to look at if the screenshots repeat.

---

## Order of work

**Status: wave 2c is closed. Session 4 passed all sixteen rows with no findings — A-9
reads as fire at a believable pace, and B-3's reach is decided at `TRANSMIT_CLEAR` 0.52
rather than deferred a third time. Fire is done being the open question; it goes back to
being a regression check. Wave 3's code is now in — A6 has a brush write path that
displaces rather than deletes, and `run_test` fails by 451 cells against the old one, so
the defect is pinned rather than argued. What is left in wave 3 is a playtest: the
finding's second half, the burst on release, is claimed here to be the *same* defect and
not a separate one, and that claim is a look and not a number.**
Update this section when a wave closes, not when a session ends: it is the only place
the sequence exists.

Session 1's findings were sequenced into waves at triage, on two rules: a defect that
corrupts a *measurement* is fixed before the things it would mismeasure, and findings
that share one root cause are one wave rather than several fixes.

| Wave | Items | State |
|------|-------|-------|
| 1 | A2, A1 (+A1b), B1, A7, A8 | **done** |
| 2 | A3, A4 — fuel and burn duration | **superseded by 2b** |
| **2b** | **A3, A4 rebuilt on the reference model, then V7 pulled forward** | **fire confirmed (session 3) — closed** |
| **2c** | **B9a–B9d the glow, A9a–A9d the fire's timing and shape** | **fire and glow confirmed (session 4) — closed** |
| **3** | **A6 — the brush destroying water instead of displacing it, and A6b the water elevator it was hiding** | **code done — open on a session 5 checklist** |
| 4 | A5, B3 — steam's own condensation clock (rest of E9) | queued |
| 5 | B2 — the material hotbar (rest of V10) | queued |
| 6 | B4 — grid-aligned toppling (E8) | queued |

### Why there is a wave 2b, and what closes it

Wave 2 shipped and was then measured against reference footage of a scene burning
([session 1 follow-up](#follow-up--fire-measured-against-reference-footage)). The fuel
model it introduced is right about duration and wrong about *which cell holds the
duration*, so it is being rebuilt rather than extended.

Wave 2b is deliberately two items and not more:

1. **The fire simulation rebuilt** on fuel-holds-the-timer — ROADMAP E9.
2. **V7 (per-cell emissive lighting) pulled forward**, which is a reorder of an existing
   item, not a new one. The argument for breaking its gate is written into V7 itself.

Both items are now built and the wave is still open, because its exit condition is
a playtest and not a checkbox. **What to look at, in the order the two changes
stack:** whether wood burning into a dark Charred cell that throws short-lived
flame reads as fire at all, and then — only once that is answered — whether the
glow is carrying it or hiding it. The second question cannot be asked first: an
emissive layer flatters a bad flame, which is exactly why the two are being judged
in that order rather than together. **The specific thing to disbelieve is my own
tuning.** Reach, falloff and the colour ramp were picked against the reference
footage as read from a description of it, not against the footage on screen next
to the game, and no test in `light_test` asserts anything about how it *looks* —
they assert that light reaches, stops, and is shaped correctly, which is a
different claim entirely.

**Wave 2b closes when fire is confirmed good in a playtest, and then wave 3 starts.**
Naming the exit condition here because this wave is the one most likely to grow: fire
touches simulation, colour and lighting at once, and each of those has a further thing
worth doing. Smoke, additive compositing and the cell:pixel ratio are all named in the
follow-up as *observed and not scheduled* for exactly that reason.
