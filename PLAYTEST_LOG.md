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
| A6 | Spawning material into water caps the water on top, then bursts outward on release | major | Two causes. The density sort is correct (sand 150 sinks through water 100). But `place()` overwrites unconditionally, so the brush *destroys* the water in its disc instead of displacing it, and volume is not conserved while dragging. The outward burst on release is `seek_level`'s pressure head resolving all at once once repainting stops. | open |
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

## Order of work

**Status: wave 2b playtested (session 2), retuned, awaiting re-confirmation.** The
model held — every one of the six notes was a look rather than a behaviour — and
the numbers and palette were corrected against them. One note, C1, is unresolved
and needs a reading before it can be acted on. Update this section when a wave
closes, not when a session ends: it is the only place the sequence exists.

Session 1's findings were sequenced into waves at triage, on two rules: a defect that
corrupts a *measurement* is fixed before the things it would mismeasure, and findings
that share one root cause are one wave rather than several fixes.

| Wave | Items | State |
|------|-------|-------|
| 1 | A2, A1 (+A1b), B1, A7, A8 | **done** |
| 2 | A3, A4 — fuel and burn duration | **superseded by 2b** |
| **2b** | **A3, A4 rebuilt on the reference model, then V7 pulled forward** | **retuned after session 2 — needs re-confirmation, and C1 answered** |
| 3 | A6 — the brush destroying water instead of displacing it | next |
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
