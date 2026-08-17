# Roadmap Archive

*Closed work, moved out of [ROADMAP.md](ROADMAP.md) on 2026-08-17 by `W4`.
**Nothing is ever required to read this file.** Every constraint here that still
binds open work was absorbed into the live item that it binds; what remains is
the reasoning behind decisions that are finished, kept because several entries
record a wrong prediction next to the measurement that corrected it, and a plan
that deletes those is a plan that will make them again.*

*Sections keep the wording and the order they had in `ROADMAP.md`. Nothing was
rewritten on the way in.*

---

## 🌊 Waves — sub-plans that preempt the tracks

*A **wave** is a sub-plan admitted out of the running order, worked to
completion, and closed before the tracks resume. It is how this document absorbs
something that has to be dealt with immediately without either ignoring it or
rewriting the order to pretend it was always next.*

**The mechanism exists because a single playtest produced twelve findings in
three minutes.** Session 1 returned eight defects and four observations against
a suite that was 6/6 green, and the tracks had no way to take that: filing
twelve entries into E and V would have buried the tier's own argument under a
defect list, and filing none would have left the findings to be closed by being
forgotten. A wave is the third option — a named, bounded unit of work that jumps
the queue, states in advance what closes it, and names which tracked items it is
actually spending.

**Admission, and it is deliberately easier than the E/V test.** A wave needs (1)
something *already built* that is wrong — a defect, an observation from play, or
any finding that makes it wrong to continue in the current order — and (2) **an
exit condition written before the work starts.** It does not have to name what
it unlocks, because a wave is not depth being added; it is a debt being paid.
What it must never do is create tracked items by stealth: a wave that turns out
to want a real feature argues that feature into E, V or P on that section's own
admission test, and **B4 → E8 is the worked example** — a playtest asked for
tipping, toppling and rolling as one feature, and what it earned was an E-track
item with rolling deferred on cost.

**Two triage rules order the findings inside a wave, and both were learned from
session 1.** A defect that corrupts a *measurement* is fixed before the things
it would mismeasure — A2's stale HUD went first because every later reading was
taken through it. And findings that share one root cause are **one wave rather
than several fixes**: A3 and A4 are the same missing quantity, A9b/A9c/A9d are
one complaint said at three volumes, and splitting either would have produced
three tunings of a model that needed replacing.

**Every confirmed defect the headless suites can reach gets a failing test
before its fix.** The correctness pass already recorded that a green suite
proves less than it looks like; session 1 is the sharper data point, because
**six of its eight defects live in simulation code the existing suites could
already reach.** They were not missed because tests could not see them. They
were missed because nobody wrote the test. The corollary is the harder half:
**write the test so it fails for the right reason** — verify it against the
unfixed code, not against the fix, which is how A1b's regression tests were
established and how `preview_light` earned trust.

**Ground every probe scene, and this is the most reliable bug in the project.**
A beam hanging in mid-air is not a slow-burning beam, it is a falling one: four
separate times a floating or badly-anchored scene has answered a question nobody
asked — session 1's A4 measurement three times over, and `burn_probe`'s first
scene, where a 150-cell plank "burned through" in five steps because the
collapse system had dropped it. **The second trap is subtler and cost the probe
its second scene**: a plank resting *on* a Wall floor burns against the coldest
large body in the scene, because `material.h` says a wall is a heat sink big
enough to quench any fire touching it. Anchor to the sealed world edge or to
posts of the material under test.

**An exit condition is not a checkbox, and this is the rule most likely to be
quietly broken.** Waves 2b and 2c both closed on a playtest *answer* — "does
this read as burning?" — and not on their tables going green. Wave 2c's own
checklist says so outright: the sixteen rows can all pass on something that
still looks wrong. A wave whose exit condition is "the code is written" was
never a wave; it was an item.

**Fix records live here, with the wave, because a wave is one unit of work.**
The E and V items cite them rather than restating them — E9 owns the argument
for why fire needed rebuilding, wave 2b owns what the rebuild actually did and
what it cost. [PLAYTEST_LOG.md](PLAYTEST_LOG.md) holds only what was asked and
what came back; nothing in it is a plan.

| Wave | What it is | Spends | State |
|------|-----------|--------|-------|
| 1 | Session 1's rendering, brush and powder defects — A1 (+A1b), A2, A7 (+A7b, A7c), A8, B1 | V10's first half | **closed** |
| 2 | A3, A4 — fuel and burn duration, first attempt | E9 | **superseded by 2b** |
| 2b | The fire simulation rebuilt on fuel-holds-the-timer, then V7 pulled forward off its gate | E9, V7 | **closed** — session 3 answered "does this read as burning": yes |
| 2c | B9a–B9d the glow's shape and reach, A9a–A9d the fire's timing and shape | E9, V7 | **closed** — session 4 passed all sixteen rows, no findings |
| 3 | A6 — the brush destroying water instead of displacing it, and A6b the water elevator it was hiding | — | **closed** — session 5 answered W-2 and W-4: the burst is gone as a consequence of the conservation fix, and the residue is not what is visible |
| 4 | D1 the dig swing, D2 the unstuck search crossing solids, D3 water stranded above the pool, D6 the walk cycle, D7 the step height | — | **✅ closed 2026-08-12 — all five changed and playtested** |

**Nothing is queued behind wave 4, and three of session 5's findings were
deliberately kept out of it.** Two of those are the same two that were kept out
of wave 3's queue and for the same reason — steam's condensation clock (A5, B3,
now D5) is the second half of **E9** and is tracked there, and toppling (B4, now
D9) is **E8** — which is worth noting rather than repeating silently: **both
have now been reported twice, and being reported twice is an argument about
priority, not about ownership.** A finding does not migrate into the wave table
by being repeated. The third is D4, the fluid flow finding, and it is the
interesting exclusion: it is unquestionably *"something already built that is
wrong"*, which is the whole of the admission test, and it still must not be a
wave, because **a wave needs an exit condition written before the work starts
and D4 cannot be given one.** Nobody can currently say what "flows properly"
would look like as a pass, and the honest first move is an instrument — which is
a spike, in the E track, on the E track's terms. The hotbar (B2) was a fourth
exclusion two waves ago and is simply **done**.

### Wave 1 — the rendering, brush and powder defects

*Closed. Five defects and one observation, and the through-line is that **five
of the eight session-1 defects were invisible to a headless suite by
construction** — which is the argument that promoted a way to drive the window
into the V track's running order.*

- **A1 — the player rectangle jittered and ghosted. Three causes, and the first
  diagnosis found only the smallest.** (i) `Player::cell_x()` returns an int and
  discards `rem_x`/`rem_y`, so at 45 cells/s over a 60 Hz step the body advanced
  one cell on three steps of four. (ii) The sim steps at 60 Hz against a 165 Hz
  display and nothing interpolated, so each position was held ~2.75 frames.
  (iii) **The largest: the camera could only sit on whole cells.** The view is
  unclamped wherever the player usually is, which pins the player near screen
  centre and scrolls *the world* in four-pixel jerks instead — smoothing the
  player alone would have left the symptom almost intact. Collision was correct
  throughout and the integer model in `player.h` stays.
    - **A1b — fixing the rendering half exposed a pre-existing simulation bug**,
      and that is the durable lesson: **when a display change appears to cause a
      physics bug, the first hypothesis should be that it revealed one.**
      `rem_y` is *pending, collision-untested* motion, and `update()` zeroed
      `vel_y` on landing without zeroing the remainder it had already produced,
      so gravity re-added ~0.056 cells of pending fall per step until it crossed
      a whole cell and the floor test finally snapped the body back. The
      horizontal case is the same omission at 0.75 of a cell into a wall.
      **Every whole-cell assertion in `test_player.cpp` passed throughout**,
      because the body's *cell* never moved — the entire defect lived in the
      fraction the renderer used to discard. The regression tests are the only
      ones in the suite that assert on `visual_x()`/`visual_y()`, and they were
      verified by reverting the fix and watching them fail rather than by being
      written green.
- **A2 — the HUD lagged material switches by up to a second.** `hud_text` was
  rebuilt only inside a once-a-second timer, so the input was instant and the
  readout stale. **Fixed first, because it is a measurement bug** — and it is
  also half of observation B2, which asked for a hotbar because selecting
  materials *felt* slow.
- **A7 — falling liquid threw horizontal sticks, and the fix revealed the defect
  was mostly about powders.** `spread` is a distance a fluid may cover *along a
  surface* and it was being spent crossing open air, because `can_rest_at` is
  satisfied by "I could fall from there", which every point in mid-air
  satisfies. Fixed by stopping the lateral run one cell past the last supported
  cell — that one cell is what flowing off a lip *is*, so forbidding it outright
  strands water on a shelf.
    - **A7b — the reported defect was sand, and the liquid fix could not have
      touched it.** Sand is a `Powder` with `spread` 0 and never reaches the
      lateral scan. `swap_elements` tags only the two cells it touches, so a
      whole row cascades diagonally inside one sweep and lands one row down and
      one column over while the row beneath has not caught up — a shelf standing
      nine cells proud of the pile with nothing under it, resolving next step
      and immediately re-forming. **What made this findable was screenshots, of
      a state nobody would have thought to describe:** the written report said
      "horizontal protrusions in <10 pixel wide sticks", which was read as a
      liquid artefact because that was the mechanism already found. The images
      showed sand, showed the shelves were one cell thick, and showed a black
      wake that turned out to be an unrelated second defect (A8). **Ask for a
      screenshot whenever a defect is about what something looks like rather
      than what it does.**
    - **A7c — the first two fixes failed in opposite directions, and shelves and
      columns are one rule's two failures.** Refusing the diagonal roll unless
      its destination was supported removed the shelves and stopped *settled*
      grains relaxing down a face, so piles grew straight up; restricting that
      refusal to grains still falling brought the shelves back, because it is
      settled grains slumping that forms them. **A test for either alone can be
      passed by breaking the other**, which is why both are now asserted next to
      each other in `test_grid.cpp`. The third rule does not restrict the move,
      it *finishes* it: a grain that rolls off an edge keeps falling in the same
      step and arrives at its resting depth immediately, so there is no moment
      at which a shelf exists. Peak shelf 9 → 3 cells; steepest settled column
      drop 1 cell.
    - **Powder acceleration was added here and then removed entirely, and taking
      it out fixed three things at once.** It was adopted because grain motion
      read as "step based", and **it cannot fix that**: a grain moves in whole
      cells on a fixed tick, so doubling its speed does not animate a fall more
      finely, it makes each jump twice as far — 8 screen pixels instead of 4,
      which is *more* stepped. It also produced sheets with a one-cell gap,
      because `fall_ticks` is per cell and `place()` resets it, so a continuous
      brush stamped fresh speed-1 grains on top of grains already at speed 2.
      **Inherent to per-cell acceleration under a continuous source, not a
      tuning error — no cap value avoids it.** Removing it retired a 12.17 →
      19.7 ms/step regression on `cascading` (118% of a 60 Hz frame) and took
      awake chunks from 135/135 back to 105/135; the shelf and column fixes
      survive, because they come from the post-diagonal slide and not from the
      speed. **Stepping is a property of grid cells on a fixed tick and is not
      reachable from `step_powder` at all.** *(This used to end "— E5's
      free-particle layer is what fixes it, and that is a second independent
      argument for E5". **That claim is withdrawn.** Nothing in the simulation
      fixes stepping, because it is a property of drawing whole cells; the old
      E5 would only have hidden it for the small subset of matter that was in
      flight. The correct reading of this whole note is at E5a, which uses its
      three measurements for what they actually show.)*
- **A8 — material carved authored background out of whatever it passed through,
  and it was not what the first diagnosis said.** `Element`'s default colour was
  opaque black while Empty's `MATERIALS` row is transparent, and `swap_elements`
  moves whole Elements — so a grain leaving a cell swapped a
  *default-constructed* Empty into its place, painting every cell anything had
  ever moved through permanently black. Nothing to do with `place()` or authored
  albedo; the two values had simply disagreed since V1 made Empty transparent.
  **Screenshots settled it.**
- **B1 — the dig marker was hard to read**, and the stated fix and the real
  finding differed. Requested as an open crosshair; the finding underneath is
  that the marker was a filled one-cell rect in the same orange family as Fire,
  so it vanished against the one thing you most want to aim at. Shipped as V10's
  first half — see that item for what was added beyond the request and for the
  false checklist claim that was nearly written into README while doing it.

### Waves 2 and 2b — fire rebuilt on fuel-holds-the-timer

*Closed by session 3. **Wave 2 shipped, was measured against reference footage
of a scene burning, and was superseded rather than extended** — it was right
that duration is tunable per material and right that fire must cross a
horizontal beam, and wrong about which cell owns the timer. A 180-step
stationary orange cell is not a fire; it is an orange block, and no amount of
correct propagation makes it read as burning. E9 carries the model argument;
what follows is what the rebuild cost and what it taught.*

**Two items, deliberately, and the second is a reorder rather than a new one:**
the fire simulation rebuilt on fuel-holds-the-timer (E9), and V7's emissive half
pulled forward off its own gate. **They had to be judged in that order and not
together** — an emissive layer flatters a bad flame, which is why the session-3
checklist is two phases and says so.

**The eight notes that came back, and what each one turned out to be:**

- **C1 — "wood burns ~20% too fast" was ambiguous between burn *duration* and
  *spread rate*, and it was spread rate.** Wood's ignition point 120 → 150 and
  conductivity 90 → 72; burn-through of a 150-cell plank 1923 → 2632 steps, ~37%
  slower, with the plank still fully consumed. **The margin that matters is not
  the percentage but Charred's `heat_source` of 200**: an ignition point at or
  above it means fire can never light its neighbour, and 150 leaves 50 degrees.
- **C2 — flames moved ~10% too fast.** Fire rose exactly one cell per step, and
  a gas moves a whole cell or none, so 0.9 cells/step is a *skipped step*:
  `FLAME_RISE_SKIP_PERCENT`, measured at 90%.
- **C3 — burnt wood was jet black.** `0xFF2A211B` → `0xFF3A3431`, jitter 6 → 10.
  Near-black over V1's dark blue backdrop reads as a *hole*, not a material.
- **C4 — burnt wood should persist a second longer, and the request could not be
  expressed.** Charred's lifetime is a decay chance, and at per-mille resolution
  the two available values either side of the answer were 2.78 s and 3.33 s. The
  value was comfortably inside the column's range and still not reachable within
  it, so **the column went to per-myriad**. The rule worth keeping: **the
  resolution a tuning column needs is set by the smallest change anyone will
  want to make to it, not by the largest value it has to hold.** Second time
  this column has been widened, first time for this reason.
- **C5 and C6 — two notes, one cause, and neither is really about height.** A
  flame rose exactly one cell per step and lived exactly twelve steps, so every
  flame died exactly twelve cells above its fuel — a straight horizontal line
  across the top of a fire, which is the one shape nothing in nature makes.
  Nothing needed to change about how flames *move*; real flames do not all live
  equally long. One jittered constant (8–18 steps) closes both.
- **C7 — "flame colours lack intensity" was not a choice of colours, it was the
  path between them.** The ramp interpolated linearly from near-white to dull
  red, and the straight line between those two points in RGB runs through greys
  and dusty salmons, so the most-visible middle of every flame's life was spent
  in the least saturated colours available. Bending the ramp through a saturated
  orange at 55% of life keeps the whole of it on the outside of the colour
  space. **The lesson generalises past fire: "lacks intensity" is more often a
  statement about what a gradient passes through than about its endpoints.**
- **C8 — the lighting was blown out, and all seven suites passed on it. Three
  causes, and the tuning number was the least of them.** (i) `TRANSMIT_CLEAR`
  was 0.86 per four cells — 0.963 per *cell*, half brightness twenty cells out —
  so **a number that reads as gentle compounded into something enormous**; now
  0.72. (ii) Brightness ignored how much of a block was burning: taking the
  hottest cell is right for deciding *whether* a block emits, and using it for
  brightness too means one stray flame lights as hard as a wall of fire.
  Emission now scales with coverage above a floor. (iii) **There was no tone
  mapping and `MAX_EMISSION` was 1.7 on purpose** — the original reasoning,
  headroom so the brightest cells clip to white, was wrong twice: the layer
  composites additively over an already-bright scene so clipping happens in the
  blend regardless, and driving a signal past its ceiling destroys every
  gradient above the ceiling rather than only the peak. Now 1.0 through a
  Reinhard curve. A fourth cause was found while fixing those: **the glow was
  diamond-shaped**, because propagating to four neighbours only makes distance
  Manhattan — most likely what "crazy flare effects" named. Peak channel 255 →
  124, share of the view brightly lit 58% → 17%.

**The instrument that was missing, and now is not.** Every suite passed the
blown-out frame and was right to — they assert that light reaches, stops, falls
off symmetrically and is shaped correctly, all of which was true. **None of them
could see that the result was unusable, because none of them ever composites
anything.** `preview_light` builds a burning scene, composites backdrop, cells
and light exactly as `main.cpp` does, dumps the frame and prints peak channel,
share brightly lit and share clipped; `tools/rawpng.py` wraps the dump into a
PNG in fifteen lines of zlib and struct rather than a new dependency. **It
reproduced the defect before it was used to fix it**, which is the step that
makes it trustworthy — an instrument that only ever agrees with the change you
already made is not evidence. `test_light` has since gained a guard for this
specific blowout, mutation-checked against the old constants, **but the guard
was only writable after the preview showed what to assert.** The generalisable
version: **a test suite that never composites cannot catch a compositing defect,
and no amount of adding assertions to it will change that.**

**Three things stand between "correct fire" and "fire that looks like the
footage", and none of them is a fire problem.** They are named here so the next
person to compare this engine's fire against that reference finds the gap
already explained instead of re-opening the simulation. **Cell granularity** —
`Camera::SCALE` is 4, so a cell is a 4x4 screen block and the viewport shows
480x270 cells at 1080p, where the reference renders far more cells per screen; a
five-cell flame layer is a fine gradient there and a stack of large squares
here. That is a resolution decision with a P-tier cost, not something fire work
can reach. **No additive accumulation** — overlapping flames in the reference
brighten at the core, and this pixel buffer writes one opaque colour per cell,
with `element.h` already noting that translucency would need the buffer to
composite rather than overwrite. **Smoke**, which is a new material and a new
question about how it clears — a candidate row for E7, not for E9.

**Six values across three files are explicitly placeholders** — Wood's ignition
point and conductivity, and `TRANSMIT_CLEAR`, `TRANSMIT_SOLID`, `MAX_EMISSION`
and `COVERAGE_FLOOR` in the light field. The comments around them describe **the
shape of each knob and what breaks at its limits** rather than justifying one
measured number, because a comment that argues for a specific value is wrong the
first time someone changes it.

**A wrong claim made while C1 was open, kept for how it was wrong.** It was
argued that if C1 meant spread rate then no tuning pass could deliver it,
because "conduction reaches Wood's ignition point in about two steps". The
arithmetic divided the temperature gradient by 255 where `heat_flow` divides by
`CONDUCTION_DIVISOR`, which is **1024** — four times smaller flow per step, so
ignition takes tens of steps and there is plenty of room to tune. **Every other
number in that session was measured; this one was derived, and it was the only
one that was false.** The project already has the rule for performance work — a
number without a measurement is not evidence — and this is the same rule
arriving in simulation tuning, on a claim whose effect was to discourage trying
the simple thing first.

### Wave 2c — the glow's shape, and the fire's timing

*Closed by session 4, which passed all sixteen rows with no findings. Eight
findings, taken as two changes, and **B9a went first and alone** — it is the
only one that is a defect in a *method* rather than a value, and it contaminates
the rest: shafts change how far light appears to reach and how hard a stray
flame appears to light, so B9b and B9c could not be judged honestly until the
field was smooth. Fixing the cheap number first would have meant tuning against
an artefact.*

**B9a — hard rays and shafts. Three changes in `src/render/light.cpp`, and two
of the four suspects were innocent.**

1. **The diagonal exponent, k^1.5 → an exact k^√2.** The old value was defended
   in a comment as accurate "within a couple of percent", which it is — *per
   block*, and per block was the wrong unit, because the error compounds once
   per block crossed. Per unit of distance the diagonal cost k^1.06 against the
   axis's k^1.0, so the field bulged along the axes.
2. **Eight knight's-move neighbours at k^√5.** Orthogonal and diagonal steps
   alone measure distance to about 8%, worst at 22.5°. A (1,2) step is the only
   one that could jump a four-cell wall, so it draws from the lesser of the far
   block and the block it passes through — **light can only arrive by a route it
   has already lit**, so a wall in the way is carried as darkness rather than
   routed around.
3. **Two separable 1-2-1 smoothing passes** over the settled field, with taps
   weighted by how open each neighbour is, so the blur cannot pull light through
   a wall it just paid to be stopped by.

Glow-roundness ratio **1.273 → 1.197**, and the furthest direction moved from
270° (an axis) to 225° (a diagonal), which confirms the mechanism in (1) rather
than assuming it. **Two hypotheses died and both are worth keeping:** the
emitting block's square shape was measured correctly (a near-constant 0.7-block
residual at every radius, and 0.707 is exactly how much further a square's
corner is than its edge) and the inference from it was wrong — removing the
smoothing pass's emission floor cost peak brightness and moved not one contour,
because sixteen iterations of propagation had already copied the square outward,
and **a smoothing pass cannot unsmear a shape set upstream of it**. And "no
penumbra through gaps" was refuted outright: the edge was 4 blocks wide before
the change and 4 after.

**The instrument was wrong before the code was, for the third time in this
project.** The first roundness probe sampled the nearest block and reported 1.32
both before *and* after a change to the metric — nearest-block sampling
quantises each reach by half a block, ~10% at these distances, which is larger
than the 8% artefact and is itself angle-dependent. It now samples bilinearly,
which is also what the GPU does to this texture, so it measures the shape the
player sees rather than the one in the array. **Two new assertions in
`light_test`, both written because the existing shape test could not have caught
this**: it compared one axis against one diagonal, and the artefact's worst
error is at 22.5° — precisely between the two directions it sampled. **A
two-sample test of an eight-lobed artefact looks where the artefact is not.**

**B9b, reach.** `TRANSMIT_CLEAR` 0.55 → **0.52**, deliberately a small step
because B9c does most of the work: halving the coverage floor shortens the
*apparent* reach of the ragged, mostly-empty fires that are almost all of what a
player sets alight, without touching a wall of flame at all. Sweeping the
constant showed 0.48 taking a small source 28% in from where session 3 saw it,
which is not "slightly".

**B9c, stray flames.** `COVERAGE_FLOOR` 0.2 → **0.1**. A single flame cell in an
otherwise empty block was emitting a fifth of what a solid wall of fire does.
The floor exists only to keep a lone flame legible as a light source, and 0.1
still does that while widening the gap between an ember and a blaze.

**B9d, walls — and this one was a modelling error, not a number. Two of them,
stacked.** The block's transmission was interpolated *linearly* between the
clear figure and the solid one, and **those two numbers only mean anything as
exponents**; averaging them as though they were rates is what let a one-cell
wall — the commonest wall a player draws, and a quarter of a block — pass most
of its light. Fixing that exposed the deeper one: `TRANSMIT_SOLID` was defined
*per block*, four cells wide, so even at a punitive 0.06 that is **0.49 per
cell** — the model was quietly claiming a single cell of rock passes half the
light hitting it. **The two figures were being calibrated against each other in
different units.** Occlusion is now stated **per cell** and a block's
transmission is the product of what its cells pass, so a block holding one cell
of rock is exactly "three cells of air and one of rock". A one-cell wall went
from passing 44% of the open-air control to passing none of it. **The gap in
coverage this came from is the point: nothing in `light_test` had ever drawn a
*thin* wall** — the shadowing test uses a slab twelve cells thick, which every
version of this code has stopped easily.

**A9a–A9d — the A-side taken as one change, with nine seeds behind every number
and the shipped engine as the control.**

| | shipped | now |
|---|---|---|
| spread, 150-cell beam | 391 steps (6.5 s) | **476 steps (7.9 s)** |
| body of wood fully gone | 22 s | **33 s** |
| mean Charred lifetime | 3.15 s | **4.37 s** |
| burn-front raggedness | 2.59 cells | **3.19 cells** |
| beams that failed to burn through | 0 of 9 | **0 of 9** |

Four constants moved — Wood's conductivity 72 → 40, Charred's decay 46 → 34 per
myriad, a new `temp_jitter` column on the Wood row at 40, and a new
`IgnitionPoint` stream. **Wood's ignition point did not move, and that is a
finding rather than an omission.**

**Two of the stated causes were wrong.** *A9d was blamed on "conductivity 72 vs
ignition 150", and the ignition point turned out to be immovable* — 156 and 162
both stall a three-cell beam on two seeds in five, because Charred holds itself
at 200 and every degree added to 150 is taken out of a 50-degree margin that is
the reason fire propagates at all. Conductivity is the only knob that survived
and it has a hard cliff of its own: 40 burns through nine beams of nine, 34
burns through five. **Not a slowdown — below the cliff a cell sheds heat to
ambient faster than the front feeds it and the fire stops.** `material.h`
claimed conductivity "can never prevent" ignition because conduction has a floor
of one unit per step; the floor is real, the conclusion does not follow, and the
comment is corrected. *And A9a was blamed on "nothing jitters when a cell
crosses its ignition point", which is true and points at the wrong fix* — making
ignition a roll rather than a certainty is the obvious reading and was tried
first: raggedness went 0.75 → 1.06 cells while the front slowed 2.3x, and a
three-cell beam stopped burning entirely because the Charred ahead of the front
burned out before the next cell won its roll. **Jittering the threshold instead
buys far more shape per degree, for a reason about the temperature field rather
than about randomness** — ahead of a slow front the gradient is shallow, so a
cell needing ten degrees more than its neighbour crosses its line several
*cells* later, not a moment later.

**The jitter is one-sided, and that was bought with a measured regression.**
Varying the threshold both ways reads better — wood has knots as well as soft
spots — and across nine seeds the symmetric version stalled one beam where the
shipped engine burned through all nine. A cell that draws twenty degrees high,
in a beam three cells thick, is a plug the front cannot get past, and **a fire
that sometimes refuses to cross a stick is a worse defect than a straight burn
front.** The dice may now only ever make a cell *easier* to light: 150 is the
hardest any cell is, not the average. That also speeds fire up, which is why
conductivity had to fall to 40 rather than the 64 a jitter-free engine would
have wanted — **two of the four changes are paying for each other.** One knob
had no cliff and no coupling at all: Charred's decay reads the same to within
noise at every value between 46 and 20, so "how long wood resists" and "how fast
fire crosses it" are genuinely two dials, and it is where most of the visible
answer came from.

**Two checks added and one widened.** `random.h`'s stream-distinctness
`static_assert` did not cover `FlameLifetime` or `FlameRise` — they had been
left out of `SIM_STREAMS` since they were added, so **the check that exists to
catch a silent stream collision was silently not watching them.** A second
`static_assert` now holds the jitter's *bottom* above the coldest ignition point
in the table, because `lowest_ignition_point()` reads `min_temp` and knows
nothing about `temp_jitter`; widen the jitter past 60 without it and a puff of
steam could light a wall. And `grid_test`'s A3 lifetime bar had a ceiling of 320
against a mean of 217 — **an upper bound one retune above the current value,
which is an equality check wearing a range's clothes.** Now 900.

**What session 4 did not answer, and it is the wave's one gap.** The checklist
asked, outside the tables, which of the four A-side changes read as *no change
at all* — the note that would tell the next wave where to spend. No such note
came back, and a blanket pass is not the same answer as "all four were visible".
**A9a (23% more raggedness on a quantity that varies 2x between worlds) is
closed as *not contradicted* rather than as *confirmed visible*.** If burn-front
linearity is ever raised again, start from the jitter knob — there is headroom
to 60 before an assert stops it — and do not re-derive that it was already moved
once.

### Wave 3 — the brush destroyed water, and the elevator it was hiding

***Closed by session 5**, on the two rows it was written to close. **W-2: the
burst is gone.** That was the claim this wave could not test — the outward surge
was asserted to be the *same* defect as the deletion, repaid rather than fixed,
and no test says so — and a person at a keyboard confirmed it, which is exactly
what the exit condition was for. **W-4: the residue is not what is visible.**
The 3-to-6 cells caught in the mound's peak were the thing this wave expected to
be judged on and they are not what the tester saw. What the tester saw was the
flow itself, which is a finding about `step_fluid` and not about venting — **and
the wave's own checklist is what established the difference**, because W-5 was
written in advance to separate the two and was answered after W-3 had already
confirmed the lift was gone. See [session 5
results](PLAYTEST_LOG.md#session-5-results--wave-3-closes-and-the-water-underneath-it-does-not).
One residual is carried out of this wave rather than closed with it: **D3, water
still climbing a standing sand column**, which is `vent_fluid`'s straight-swap
fallback and is priced with D4 in the fluid spike.*

**A6 — spawning material into water capped it, then burst outward on release.
Two causes, one of them not a defect.** The density sort was correct (sand 150
sinks through water 100); `place()` overwrote unconditionally, so the brush
*destroyed* the water in its disc instead of displacing it and volume was not
conserved while dragging. **Fixed by giving the brush its own write path,
`Grid::displace`** — the occupant climbs to the first Empty above it and the
brush writes into the vacancy. `set_element` still overwrites for everyone else,
because for every other caller deletion is the point. A 4-radius sand brush
dragged 40 cells through a 1000-cell pool destroyed **451 cells** before and
**0** after. **The burst needed no separate fix — it was the deleted volume
being repaid.**

**A6b — the water elevator, found because A6's fix stopped hiding it.** Water
appeared at the top of a sand column tens of cells above the pool the moment
falling sand reached the water, then ran down the outside of the pile in ragged
clumps. **Establishing that this was not A6's fix misbehaving came first**, and
`water_probe` settled it — a size-3 brush pouring from 25 cells of open air,
never touching water at any point in the run:

| | water above the cursor, step 50 | pool at step 200 |
|---|---|---|
| A6 fix in | **32 cells** | 2000 of 2000 |
| A6 fix out (old code) | 5 cells | **1352 of 2000** |

**The lift is in both.** What the old code did was *eat* it: the brush deleted
the water as fast as the column delivered it, and 648 cells of the pool were
gone by step 200. **The elevator had been running since before wave 1 and was
invisible because a second defect was consuming the evidence — a conservation
fix did not cause this, it made it visible, which is the argument for having
made it.** Volume is conserved throughout (2000 → 2000), so this is transport
and not creation, which is why it needs a rule change and not a leak hunt. Both
halves of the report are one defect: `can_displace` lets a powder swap with any
lighter fluid below it and a swap moves the water *up one cell*, so under a
continuously fed column the exchange repeats every step and the water is handed
up the stream without bound. The trickle down the flanks is that same lifted
water coming home.

**Fixed by sending the displaced fluid sideways, to its own surface.**
`vent_fluid`: when a grain sinks into a fluid, the fluid goes to a free surface
of its own kind within `VENT_RADIUS` and the grain drops into the vacancy. **The
straight swap survives as the fallback**, which is right for a grain deep inside
a body — there is no conveyor above it, so its one-cell lift never adds up. Two
things the probe found that reasoning did not: **a pool surface is not the only
destination needed** (the first version fixed the pour and let the defect return
at step 175, once the sand had built a cone standing out of the water — what was
left was films of water clinging to the cone's flanks, which have sand
underneath and no surface of their own kind to reach; **the pile had become the
conveyor**), and **any-Empty is not a destination** (the nearest empty cell to a
grain entering the water is very often the air just above the sand pile, so the
water was put back on top of the pile). Water above the cursor: 32 cells at step
30 → 3 cells at step 350.

**`VENT_RADIUS` is a cost knob and was swept, not picked.** Against `churning`
at 3.13 ms/step with no venting: r=2 is 4.12 ms and clean to step 200, r=3 is
4.93 ms and clean to step 350, r=4 is 6.72 ms and clean to step 400. **3 is the
knee.** `cascading` is 12.7 ms/step at every radius including zero — this
touches the powder/fluid interface and nothing else.

> **Corrected 2026-08-13. The sweep was taken one build per data point, and
> re-run in one binary there is no knee.** Same scenario, same world size, four
> radii inside one process: **3.46 / 4.97 / 6.55 / 8.35 ms/step at r=0/2/3/4.**
> That is **0.063 / 0.064 / 0.061 ms per extra cell scanned** — flat, which is
> what a box scan should cost. The numbers above work out to **0.041 / 0.038 /
> 0.045**, and the low middle point *is* the knee this paragraph named. **The
> knee was the compiler, not the code.**
>
> **Two things survive and one does not.** The claim that this touches the
> powder/fluid interface and nothing else survives, and is now structural rather
> than observed: `churning` is the only scenario in `grid_bench` containing
> water, which is what made every other row a usable control. The absolute costs
> survive as a shape — cost does rise with the area of the box, as predicted.
> **"3 is the knee" does not survive, and 3 stays anyway** on the quality side
> of the same sweep, which was never the part in doubt. See `PERFORMANCE.md` for
> the tables, and note what this cost: the choice was defended for months on a
> comparison between three binaries, which is the failure `PERFORMANCE.md`'s E1
> entry was already on file to prevent.

**Two changes that sounded right and measured wrong, recorded so they are not
retried.** Restricting the vent to grains with another grain on top — the
conveyor condition itself, and apparently exact — made `churning` *slower* (6.7
→ 7.3 ms/step, because a churning mix is stacked grains nearly everywhere, so
the test bought no skips), stopped 15 of 135 chunks ever sleeping, and brought
the probe's residue back 200 steps earlier. Routing the brush's own displaced
fluid through `vent_fluid` could not be told from not doing it, to a single cell
over 500 steps.

**The residue, named rather than left to be rediscovered.** Once the pile has
grown up to the cursor, 3 cells at step 350 and 6 at step 500 still reach the
top — water caught inside the mound's peak with no surface and no downhill
within reach of the search. **Whether that is visible at all is what the
playtest is for, and it is the first thing to look at if the screenshots
repeat.** *(Answered: it is not. W-4 came back "looks correct but again ruined
by the visual bugs", so the quantity this wave chose to be judged on was
invisible next to one it had not measured. **That is the general risk in naming
your own residue** — a number you can produce becomes the thing you watch, and
this wave watched 3 cells while a whole material failed to read as liquid.)*

### Wave 4 — the defects session 5 found on the way past

*✅ **Closed 2026-08-12.** D2 and D1 fixed 2026-08-11, D3 on 2026-08-12, D6 and
D7 changed and playtested the same day — the walk reads right at 36 steps and
the step height reads right at 3, both accepted on a look rather than closed by
a test, which is what these two were always going to come down to.*

*Original entry — Four items, none of which is about water, all of which were
found by running README's nine steps rather than by the two phases the session
was scheduled for — **which is the argument for running the general checklist in
full even when a session exists to answer one question.** Session 5 was booked
to close wave 3 and discharge E4; four of its six defects came from the pass
everyone treats as a formality.*

> **The count in this heading was "four" until 2026-08-12 and the wave has held
> five since 2026-08-11**, when D3 was added — the defect whose whole lesson was
> that it had fallen out between the log and the plan. It then fell out of this
> heading for a day in the same way. The title is now countless on purpose; a
> number in a heading is a second copy of the list below it, and this file has
> just demonstrated what a second copy does.

**Admission:** all five are things already built that are wrong, and D2 is a
containment failure. **Exit condition, written before any code:** D2 cannot be
reproduced by a test that buries a body beside a wall; the dig swing repeats for
as long as the button is held; and a session at a keyboard reports the walk
cycle and the step height as *right, too far, or not far enough* — not as green
tables.

**Every one gets a failing test first, verified against the unfixed code**, per
the rule waves 1 and 2c both established. Three of the four are reachable
headlessly and the fourth is a look.

- ✅ **D2 — the unstuck search crosses solid terrain, and it is the most serious
  finding of the session.** *(Fixed 2026-08-11.)* `Player::resolve_overlap`
  rings outward for the nearest position where the body does not overlap solid
  and moves there. **It tests the destination and never the path.** A body with
  sand landing in its box beside a wall is relocated to the nearest free ring
  cell, which can be on the far side of that wall — so being poured on is a way
  through terrain. The glitch-like push is the same function firing every step
  under a continuous pour, with `update()` returning early each time, which also
  costs the player control while buried.
    - **The design is right and is not what changes.** The comment above it
      already argues the case correctly: the grid does not know the body exists,
      so "body overlaps terrain" is a state that occurs in ordinary play — being
      buried by a collapse, spawning into a wall — and without a way out every
      direction is blocked and the player is frozen for good. **What is missing
      is that the escape was never constrained to be reachable.** The ring
      search is the right shape; it needs to reject a destination it cannot get
      to without crossing something solid.
    - **This is E4's absence biting from the direction nobody was looking.**
      `ENGINEERING_NOTES.md` describes the unstuck search as "what stops that
      becoming a freeze" and calls the current behaviour "odd-looking but never
      broken". **It is broken**, and the note is corrected rather than softened.
      E4 being answered "no" makes this worse and not better: the search is now
      permanent load-bearing machinery rather than a stopgap until displacement
      lands.
    - *Verify:* a body buried beside a one-cell wall, stepped, never ends up on
      the far side of it — asserted in `test_player`, which has never contained
      a scene where the body is overlapped at all.
    - **What the fix could not be, and this is the whole of the difficulty.**
      The item above says "reject a destination it cannot get to without
      crossing something solid", and written literally that rule freezes the
      player: a buried body is *surrounded* by solid, so every escape crosses
      solid and every candidate is rejected — the failure mode `resolve_overlap`
      exists to prevent, reintroduced by its own fix. Path-solidity cannot
      separate the two cases at all: climbing twelve rows of sand and stepping
      through a one-cell wall both traverse solid material for their whole
      length.
    - **What separates them is the direction the burial goes, so that is what is
      measured.** `overlap_depth` counts how many of the body's 160 cells are
      inside solid, and an escape is admitted only if that count never *rises*
      along the straight line to it. Grinding up out of sand sheds a row of
      overlap per cell and slopes to zero; crossing a wall means entering a mass
      the body was not in, which is a spike on the first step — flush against
      the wall it is +20 cells before any of the sand is left behind. **The test
      had to be built so the bug is the cheaper answer**, or it passes without
      proving anything: the sand is `WIDTH + 4` deep so climbing out costs 12
      rings while stepping past the wall costs 9, and the unfixed search picks
      the wall every time. It did: `x=153` against a wall at 152, one cell
      through.
    - **The scan order made the failure less obvious than it should have been,
      which is worth remembering for the next scenario of this shape.** The
      escape it actually took was diagonally up-and-through, and by the time the
      loop finished the body had fallen to the floor on the far side — so the
      failing position read as an ordinary standing pose several cells from
      where the teleport happened. Assert on *ever crossed*, sampled every step,
      not on the final position.

- ✅ **D1 — the dig swing plays once while the button is held, and the cause is
  two constants that were never compared.** *(Fixed 2026-08-11.)*
  `DigTool::COOLDOWN_STEPS` is 6; `DIG` is 3 frames at `wait` 8, so a swing is
  24 steps. A held, connecting dig re-latches the one-shot every 6 steps against
  a frame that needs 8, so `elapsed` never reaches `wait` and **the animation is
  pinned on frame 0 for as long as the button is down**. Release, and the
  one-shot finally runs to completion — which is the single swing the tester
  saw.
    - **The identical collision was reasoned about three lines away and only for
      the other consumer.** The `FLY` branch's comment states that the wing
      animation is *longer* than the interval between beats and that sustained
      flight is therefore the branch re-firing rather than an animation left to
      loop. That is the same arithmetic, done deliberately, for the one of the
      two one-shots whose interval happens to be survivable. Nobody did it for
      the dig.
    - **`test_dig_retrigger` retriggers after `DIG.wait` — 8 steps — and the
      tool retriggers at 6.** The suite picks the one interval at which the bug
      cannot occur, and nothing in `tests/` mentions `COOLDOWN_STEPS`. **Third
      time in this project that an instrument looked where the artefact is
      not**, after wave 2c's two-sample roundness test and the nearest-block
      probe before it.
    - **The fix is a spec reversal and is recorded as one.** The request is a
      swing that cycles while the button is held, slower than the walk cycle.
      `player_anim.h` argues the opposite in writing — animate the tool, not the
      button, because a held button fires on a cooldown and animating the button
      would play a swing on steps where nothing happened. **That reasoning is
      sound and it is being overruled by a look**, which is allowed and has to
      be visible. The chosen shape keeps the principle that matters:
      **`ENGINEERING_NOTES.md` refuses frame-tied gameplay events outright** —
      rendering must never drive simulation, which F1 spent seven steps making
      impossible — so the swing clock goes in `DigTool`, on the fixed step, and
      fires at a fixed offset within its own cycle. The animation is *told* the
      phase. The forbidden version, and the one to watch for in review, is
      `src/render/` calling back into the tool.
    - **It sets the dig rate to the swing rate**, roughly 10 digs/second to
      under 2, which is a gameplay change accepted deliberately when the shape
      was chosen. The dig radius is the knob if it reads as feeble.
    - **Built as specified, and the shape is worth stating plainly because it is
      the general answer to "two clocks that must agree": delete one of them.**
      `COOLDOWN_STEPS = 6` is gone and `DigTool::SWING_STEPS = 36` replaces it —
      a swing with a duration rather than a rate limit. The animation keeps no
      clock at all for the dig: it reads `DigTool::swing_progress()` and divides
      by its own frame count. 36 is derived from the request, not picked —
      longer than the walk cycle's 30, which is what "slower than the walk"
      means in the only units either side shares.
    - **`swing_progress()` returns a fraction rather than a step index, and that
      is the part doing the preventive work.** A consumer handed "step 14 of 36"
      needs its own copy of 36 to interpret it, and a second copy of the swing's
      length is precisely the condition that produced this defect. A fraction is
      complete on its own, so there is no number on the render side that can be
      retuned into disagreement.
    - **The forbidden shape was avoided and the direction is worth re-checking
      in review:** the clock is in `DigTool`, on the fixed step, and `main.cpp`
      *reads* it. Nothing in `src/render/` calls into the tool, and
      `Run::step`'s return value is no longer wired to the animation at all.
    - **The impact sits on the swing's first step, and the wind-up version was
      built first and rejected.** Putting it two thirds in, where a real swing's
      arc bottoms out, is the more literal reading of the motion — and it puts
      half a second between the button and the world changing on a tool used
      constantly, which reads as input lag, not as weight. **The follow-through
      carries the weight instead**, and it is the half that was missing before:
      the frames after the first are the recovery from a blow that has already
      landed. It simplifies the tool too. The ray that decides whether a swing
      starts is now the same ray that digs, rather than a probe repeated later,
      and the aim used is the aim at the moment of the press — correct precisely
      because there is no time in between for it to have moved. A deferred
      impact needs a second march and has to answer what happens when the world
      shifts mid-swing; an immediate one has no such window.
    - A swing that would connect with nothing is never started, so the player is
      not committed to half a second of animation for a shot at thin air.
    - **The failing test is an integration test, and it had to be, which is the
      lesson from this one.** Every existing case in `test_anim` sets the dig
      condition by hand — and so every one of them passed throughout, because
      *hand-setting the condition replaces the exact number that was wrong*.
      `test_held_dig_cycles` drives the real `DigTool` into the real animation
      the way `main.cpp` does, and reported **1 of 3 frames reached** against
      the unfixed code: the frozen figure, reproduced headlessly.
    - **It also took two attempts to write, and the first one passed while the
      bug was present.** In a world of solid Wall the tunnel outruns `RANGE`,
      the tool stops connecting, and the one-shot then completes undisturbed —
      so the test saw all three frames and called it a cycle, when what it had
      actually observed was *the defect itself* (release the button, one swing
      plays). The terrain is now refilled every step. **A test for "does it
      repeat" must guarantee the repeating stimulus is still arriving at the end
      of the run**, and terrain that the subject destroys does not guarantee
      that.
    - **Three other suites asserted the old spec and were changed rather than
      worked around.** `test_tool`'s `dig_once` helper now holds the button
      through one swing; `test_run`'s wiring check does the same. The old
      `test_dig_oneshot` / `test_dig_retrigger` pair is replaced by cases that
      assert the phase-to-frame mapping and that the swing ends when the tool
      says it does — the one-shot machinery they used to cover is still
      exercised, by the wing beat, which is still a one-shot.

- **D6 — the walk cycle is ~10% too fast, and the column cannot express that.**
  `WALK` is 6 frames at `wait` 5, so 30 steps per cycle; the only adjacent
  integer is 6, which is 20% slower. **This is C4 arriving in a second tuning
  column** — the resolution a knob needs is set by the smallest change anyone
  will want to make to it, not by the largest value it holds. **6 is tried first
  anyway**, because "10%" is an estimate and not a measurement, and 20% reading
  fine is the cheapest possible outcome. If it reads sluggish, *that* is the
  evidence that earns sub-step resolution on the animation clock; widening the
  column on the estimate alone would be paying for precision nobody has shown is
  needed.
  > **2026-08-12 — changed to 6 in `tools/player_sheet.py`, and that is as far
  > as this can be taken without eyes.** The cycle is now 36 steps. There is no
  > assertion to write here: nothing is latched to `walk` the way `fly` is
  > latched to `FLAP_INTERVAL_STEPS`, so the only thing it has to agree with is
  > how fast the figure crosses ground, which no headless test can judge. All 10
  > suites pass, which says only that the change is inert to everything else.
  > **The outcome that matters is which of the two answers comes back** — "fine"
  > closes D6 for the price of one integer, and "sluggish" is the measurement
  > that earns the sub-step clock, at which point this entry stops being an
  > estimate and becomes C4's second data point.

- **D3 — water comes to rest above its own free surface.** *(Added to this wave
  2026-08-11; **given its entry here on 2026-08-12**, which is the second half
  of the same omission. D3 had no owner anywhere in the plan for a day, was
  filed into Wave 4 in `ROADMAP_ITEMS.md` — and then stayed only there, in the
  file that is explicitly **not** the authority on *why*. A defect carried in
  the order with its reasoning nowhere is the failure mode this project's file
  split exists to prevent, arrived at from the opposite direction.)*
    - **What it is.** Wave 3 stopped water riding to the top of a falling sand
      column by sending displaced fluid sideways to a free surface of its own
      kind; `vent_fluid`'s **straight swap survives as the fallback** for a
      grain deep inside a body, on the recorded grounds that *"there is no
      conveyor above it, so its one-cell lift never adds up"*. **In the played
      configuration it adds up.** That sentence is wrong in the same way wave
      3's residue bullet was wrong — a property argued harmless and then seen —
      and it is corrected here rather than softened.
    - **It is a residual to eliminate and not a rate to tune**, which is what
      takes it out of the realm of looks. Displacing sand into a pool must raise
      the pool's free *surface* — that is conservation, and it passes. **No
      configuration of sand and water makes it right for water to occupy a
      column standing above that surface.**
    - **It is the cheapest defect in this wave because both halves already
      exist.** The invariant is assertable exactly as stated — *no water cell
      may come to rest above the free surface, splash excepted* — and
      `water_probe` already measures the quantity. This is a headless assertion
      plus whatever it turns out to catch, not an investigation.
    - **The number this was signed off on is the thing to be suspicious of.**
      Wave 3 recorded 3 cells at step 350 and 6 at step 500 and judged them
      acceptable **on paper**; they are visible at 3440x1440. That is wave 3's
      own stated risk in naming your own residue, collected.
    - *Verify:* the assertion fails against the unfixed code for the stated
      reason — a resting water cell above the surface, not a splash frame — and
      `water_probe`'s count goes to zero at the steps it currently reports 3 and
      6.
    - > **Fixed at rest on 2026-08-12, and the cause named above is wrong.** The
      prediction is left standing because the *shape* of the error is worth more
      than a clean entry: it named `vent_fluid`'s straight swap, which is where
      the water is put onto the slope, and the defect was in what happens to it
      afterwards. The assertion was written first, failed against unfixed code
      with the world fully asleep and the water conserved, and reported two
      cells — and then the dump of their neighbourhood is what settled it. Each
      was a 4-connected body of **exactly one cell**, sitting on a sand
      shoulder, one row proud of a level pool it touched only diagonally.
      Nothing could reach it: it could not fall (solid beneath), `seek_level`
      searches the body and the body was itself, and `step_fluid`'s lateral rule
      refused to let it step onto the water beside it — that being precisely the
      move the wave-3 anti-jitter rule was written to refuse.
    - > **So the rule had a mirror nobody had looked at.** It forbade moving
      *onto* a destination perched on the same liquid and allowed moving *off*
      the liquid onto a solid shoulder at the same row — a one-way trip, because
      the return fails the same test. Sideways travel now runs along a floor and
      never up onto one, which makes perched-ness strictly decrease and is why
      it cannot reintroduce the jitter. One line; the argument is at
      `can_rest_at` in `step_fluid`.
    - > **`MAX_PRESSURE_CELLS` was the first suspect and was cleared by
      experiment, which is the only reason it is mentioned.** Raising it 512 →
      8192 levelled one tier of the mound and left both stranded cells exactly
      where they were, because a one-cell body has nothing to search. Had the
      budget been raised on the strength of the dump alone it would have looked
      like a fix, cost frame time in the hottest scenario, and left the defect
      in.
    - > **The half that survives, stated plainly rather than as a win.**
      `water_probe`'s *during-pour* line is unchanged — still 2 / 3 / 6 cells
      above the cursor at steps 400 / 450 / 500 — because that half really is
      the swap fallback under a live conveyor, and it is the limit `VENT_RADIUS`
      is already priced against (r=4 buys clean-to-400 for +36% on `churning`).
      What changed is that it no longer sets: after the brush is released the
      pool comes back perfectly level with **zero** cells above it, where it
      used to keep two forever. Whether a transient that clears in about a
      second is worth ~~36%~~ **27%** of the fluid budget is a judgement about
      looks, and it is owed a pair of eyes rather than another test. *(Corrected
      2026-08-13: the one-binary sweep prices r=3 → r=4 at +27.4% and +27.5% at
      the two world sizes, not the +36% the three-build sweep implied. **The
      judgement is unchanged and slightly easier** — it was already being
      declined at the larger figure.)*
    - > *Measured:* bracketed in one sitting, both directions, controls
      included. `churning` 35.8 → 36.7, `cascading` 40.7 → 41.1 ms/step at
      1920x1080 — and a **second run of the unchanged binary** gave 36.0 and
      41.3, so the deltas are inside run-to-run noise and there is nothing to
      report. `burning` first read as +7.7% and the repeat is what killed it; it
      is in `PERFORMANCE.md` for that reason and not for its value.

- **D7 — `MAX_STEP_HEIGHT` is 5 against a 20-cell body.** The player climbs a
  quarter of its own height instantly and with no animation, which is the
  likeliest whole explanation for "walking into a settled pile does nothing, the
  player walks over it". **It was reported as evidence about E4 and is almost
  certainly a number** — see the note in the session results about findings
  arriving wearing the mechanism the tester was asked to look for. **README's
  step 2 only ever asks for a one-cell sand step**, so the constant that governs
  this has never been checked by the checklist that exists to check it; that row
  is added with the fix.
  > **2026-08-12 — 5 → 3, and the prediction held: it was a number.** The entry
  > above is right about the cause but understates where 5 came from. It was not
  > chosen against a 20-cell body at all — it was the old 8-cell body's 2,
  > scaled to keep the same *ratio*, and that ratio had never been checked
  > either. **Scaling a constant with the thing it applies to preserves whatever
  > was wrong with it, silently and with a plausible-sounding justification
  > attached** — the comment in `player.h` argued the case convincingly for a
  > number nobody had validated. 3 is a curb rather than a table, and still
  > three times the one-cell steps a settled slope is made of.
  >
  > **The test that was missing was not the one D7 needed, it was the one the
  > fix risked.** Every lip case in `player_test` is built from Wall and is
  > written in terms of `MAX_STEP_HEIGHT`, so all four followed the constant
  > down without complaint — a cliff at the new height is still a cliff. What
  > none of them touched is *powder*, which is the terrain the player actually
  > meets and which is not a cliff but a staircase whose total rise dwarfs any
  > step height. A settled-pile case was added and passes at 5 and at 3, and it
  > deliberately asserts no number: the player crosses the pile, which must stay
  > true at any value a one-cell staircase can clear. **Lowering a constant that
  > four tests are parameterised on can only be caught by a test that is not.**
  >
  > README's step 2 row is added as promised, but pointed the other way from
  > what the entry expected — the risk that needs eyes now is sand that *used*
  > to be strollable stopping the body, not piles being skated over. Also fixed
  > there: the mechanics section had described the step-up as "up to 2 cells"
  > long after it stopped being 2, so it now names the constant instead of
  > restating it.

## ✅ Shipped

*Everything below is done. It is kept in full because the reasoning is the
valuable part — several entries record a wrong prediction next to the
measurement that corrected it, and a plan that deletes those is a plan that will
make them again. Nothing here is a task.*

### The engine and its harness

- [x] Initial GitHub Repo and CMake build system setup.
- [x] Barebones C++/SDL2 pixel physics engine prototype built (Sand, Water, Wall
  interactions).
- [x] **Data-driven material system.** Materials are rows in a table
  (`src/physics/material.h`), not branches in the update loop. Four generic
  behaviours — Static / Powder / Liquid / Gas — drive all movement, with density
  deciding what sinks through what.
- [x] **Eight materials** proving the above: Sand, Water, Wall, Wood, Oil
  (floats on water), Steam (rises), Fire, plus a transparent Empty. Each cost
  one table row, no engine changes.
- [x] **Fixed timestep.** Simulation advances at a constant 60 Hz independent of
  render framerate, so the game behaves identically on a 60 Hz and a 144 Hz
  display.
- [x] **Headless test harness.** The simulation has no SDL dependency and is
  covered by tests wired into CTest — conservation of matter, density
  stratification, static materials, border sealing. Six suites, one per concern:
  `tests/test_grid.cpp`, `tests/test_player.cpp`, `tests/test_tool.cpp`,
  `tests/test_collapse.cpp`, `tests/test_run.cpp`, `tests/test_scene.cpp`,
  sharing one harness in `tests/test_util.h`. **199 checks in total** (179
  before the correctness pass below added twenty).
- [x] **Chunked dirty-rect updates.** The world is split into 64x64 chunks, each
  tracking the bounds of the cells in it that can still move. Settled chunks are
  skipped entirely, so cost scales with how much is *moving* rather than with
  world size. Every write wakes its 3x3 neighbourhood so nothing is left hanging
  in mid-air, and the awake-chunk count is on screen so culling bugs are visible
  rather than silent.
- [x] **Benchmark.** `tests/bench_grid.cpp` measures seven scenarios against the
  60 Hz budget, at both 960x540 and the played 1920x1080 since P2. Deliberately
  not a CTest test — timings inform, they do not gate. Every scenario has to be
  built so its subject never falls asleep inside the measurement window, or the
  number reported is mostly the cost of an idle world: `cascading` scrapes the
  floor and pours in at the ceiling, `burning` drips fresh Fire, `collapsing`
  drains landed slabs away and feeds in replacements, and `shattering` exists
  because `collapsing` cannot see a landing. **P2 added the check that this rule
  was being kept**, by printing chunks awake at both ends of the window rather
  than only at the end — `churning` turns out to reach complete rest inside it
  at 960x540 (105 -> 0) and not at 1920x1080 (360 -> 90), which is the one row
  in the table whose size ratio cannot be read at face value.
- [x] **Reactions.** A data-driven `REACTIONS` table (`src/physics/reaction.h`,
  catalyst + target -> result, rolled per step) drives Fire: it ignites touching
  Wood and Oil, is doused into Steam by Water, and burns out on its own. Second
  major engine axis — transformation is as data-driven as movement. The one
  non-obvious piece: Fire's self-decay is *spontaneous* (no neighbour required),
  which means it has no movement to piggyback a wake-up on, so a boxed-in Fire
  cell self-marks its own neighbourhood dirty every step purely to avoid
  freezing mid-burn. Covered by tests in `tests/test_grid.cpp`, including a
  dedicated regression test for that freeze case.
- [x] **Player Character + Player Physics.** A rigid body
  (`src/physics/player.h`, 4x8 cells as shipped, 8x20 since the Noita rescale)
  that is *not* a cell: it has its own position and velocity and only ever reads
  the grid to ask whether a cell is solid. Position is an integer cell plus a
  sub-cell remainder rather than a float, so collision compares whole cells and
  the float-edge bugs never arise; movement resolves one cell at a time per
  axis, which makes tunnelling impossible by construction rather than by being
  fast enough. Solidity is derived from the existing `MoveKind` (Static and
  Powder are solid, Liquid and Gas are not), so it stays one table, not two.
  Gravity, jumping, a 2-cell step-up for walking over uneven powder, and an
  unstuck search for when falling sand buries the player. Covered by 18 tests in
  `tests/test_player.cpp`.
- [x] **Structures fall as rigid bodies.** Wall and Wood no longer hang in
  mid-air when the ground is dug out from under them. An unsupported piece falls
  **as one rigid body, keeping its shape the whole way down** — not as loose
  grains, and that distinction is the whole feature: the first implementation
  dissolved an unsupported slab into a Rubble powder, which read as a different
  bug, because masonry that turns to gravel the instant it comes free looks
  broken rather than physical. The piece stays in the cell grid while it falls,
  so it is a rigid body only in *how it moves*, not in where it lives — which is
  what keeps rendering, player collision, digging and fire working on it with no
  changes at all. Structure is a `structural` flag in the `MATERIALS` table, so
  it stays one table rather than a second list. Four decisions are load-bearing
  and each is a trap if reversed: support is checked **on disturbance only**,
  never as a global truth; pieces over 4,096 cells are **assumed supported**
  rather than judged, because a missed fall is invisible while a wrong one drops
  the level; **a fill records what it concluded, not just where it went**, or a
  wall on solid ground sheds chunks of itself; and a falling piece **accelerates
  by falling repeatedly rather than further**, which is what makes it
  structurally unable to step over a floor thinner than its speed. Full
  reasoning for all four is in [README](README.md#structures-and-falling).
  Covered by 31 tests in `tests/test_collapse.cpp` — including a disc whose
  exact shape is compared before and after landing (a rectangle still looks like
  a rectangle after a row of it slips, so rectangles alone were not catching
  this) and a piece at full speed that must land *on* a one-cell shelf rather
  than through it.
- [x] **Basic Interaction — digging.** The player can change the world, which is
  the line between a sandbox and a game. A dig is a ray marched one cell at a
  time from the body's centre to the first solid cell, which is then blown out
  to a small radius (`src/physics/tool.h`). Deliberately *not* a method on
  `Player`: keeping the body's grid reference `const` is what makes it
  structurally incapable of breaking the `set_element` write rule, so the verb
  lives in its own module that takes a mutable `Grid&`. What blocks a shot is
  the same `is_solid` the player collides against — one definition used twice,
  so terrain and powder stop a ray while water and fire do not. Range is real
  distance rather than a step count, and the cooldown is in fixed steps rather
  than seconds. Covered by 23 tests in `tests/test_tool.cpp` — including the
  anti-tunnelling case (a thin wall must not be dug *through* to the terrain
  behind it) and the collapse case (digging out the base of a settled pile makes
  it fall, which only works because removal goes through `set_element` and wakes
  the neighbourhood).

*Performance numbers, benchmark methodology, and the measurement mistakes that
methodology exists to prevent are in **[PERFORMANCE.md](PERFORMANCE.md)**. Read
it before trusting any timing claim in this document — several already had to be
corrected in place after a better measurement contradicted them.*

*Techniques this project deliberately does not need (entity/component system,
threading, networking, a scripting layer, a bespoke asset editor, shrinking cell
size) are recorded in [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md) so they
aren't mistaken for pillars when the itch to add one arrives.*

### Foundations (F1–F5)

*Four items that were not features. Each was a prerequisite for **several**
slice items rather than one, and each got steadily more expensive the longer it
waited — which is why they were pulled out rather than left implicit inside the
slice. Written as a budget rather than a starting point, and the budget held:
nothing in F1–F4 added a simulation axis. **That was a statement about F1–F4 and
was never a standing prohibition** — the section that followed spent a seventh
axis on purpose (E2, heat), bought by an observation from play, and said so in
full where it did it.*

*They ran F1 → F2 → F3 → F4, and **F5 joined them two months later** — a fifth
foundation, added by the 2026-08-11 plan review rather than planned with the
other four, because the thing it fixes was written down as a known limitation
and then spent as a guarantee three separate times. It is here rather than in a
track of its own for the reason the header gives: it was a prerequisite for
several slice items and got more expensive the longer it waited.*

* F2 wanted F1's seed and F2.3 finished what F1 started. **F3 came after F2
  because both rewrite `main.cpp`** — F2.1 moved the run state out and F2.3
  moved input handling out, so doing F2 first left F3.2 with strictly fewer
  coordinate sites to convert. F4 was genuinely independent and went last
  because it was the only one of the four that produced something visible, which
  made it the easiest to keep polishing past the point of being done.*

#### F1 — Determinism, first half: the simulation

*The one gap that was not written down anywhere in this document.*

`Grid` used to seed `std::mt19937` from `std::random_device` inside its own
constructor, with no way to set the seed and no way to read it back, so **no run
was reproducible** — not by a player, not by a bug report, and not by the test
suite. The test suite was paying for it: `test_grid.cpp` ran 150-200 steps in
places and asserted that failure was *astronomically unlikely* rather than
asserting an outcome.

Four Medium Term items depend on this and none of them said so. Quantum Worlds
cannot regenerate or share a trial. Save and persistence has to either serialise
2.5 KB of generator state or accept that loading diverges from the run that was
saved. The playtest gate collects bug reports that cannot be reproduced. And
health/death cannot be balanced against a fixed scenario if the scenario is
never the same twice.

**F1 alone did not deliver any of those four.** F1 made `Grid` a pure function
of its seed. The *game* was still not reproducible after F1.7, because input did
not enter the simulation cleanly: the brush was stamped once per **rendered**
frame, outside the fixed-step loop entirely, and the keyboard was sampled once
per rendered frame and then applied to *every* fixed step in that frame — so a
held dig issued two commands at 30 fps and one at 144 fps. Same seed, same
physical keypresses, different world. **F2.3 was the second half**, and only the
two together produced the thing the four items actually need, which is not a
seed but a seed plus a replayable input log.

**The decision: a stateless hash, not a seed argument.** The smaller change
would have been to pass a seed into the existing generator. The better one was
to delete the generator and derive randomness from a hash of `(cell position,
step, seed, salt)`. It settled three things at once — determinism, a world with
no generator state to serialise, and the **RNG cost** entry in
`ENGINEERING_NOTES.md`. Doing it as a seed first and a hash later would have
meant touching every call site twice. There were five call sites, all in
`grid.cpp` and all of the form `rng()`: colour jitter in `jittered_color`, one
direction pick each in `step_powder` and `step_fluid`, the chance roll in
`try_react`, and the per-row sweep direction in `update`. **Do not carry the
number five forward as a fact** — the durable version is the invariant, that no
call to the hash lives outside the one helper that owns it, which is what F1.6
was able to check mechanically once `<random>` was gone from the header.

- [x] **F1.1 — Take a seed, keep the generator.** `Grid` takes a `uint64_t seed`
  (defaulted to `Grid::DEFAULT_SEED`) and exposes `seed()`. `std::random_device`
  is gone from `grid.cpp` entirely and lives in `main.cpp`, which is now the
  only nondeterministic line in the project and prints the seed it chose — a
  seed that cannot be read back is only half of determinism. Three checks rather
  than the two planned, in `test_grid.cpp` (100 total): same seed matches,
  **different seed diverges**, and the whole 64-bit seed is used. The second is
  what makes the first mean anything, since two worlds also match when nothing
  random ever happened. The third came out of the implementation: `std::mt19937`
  seeds from 32 bits, so handing it the seed directly would silently discard
  everything above bit 31 — it goes through a `std::seed_seq` of both halves
  instead. **Side benefit worth knowing:** `grid_bench` used to draw a fresh
  random seed per run, so consecutive runs simulated slightly different worlds.
  They are now identical, which removes one small source of the run-to-run
  spread the measurement rules exist to work around.
- [x] **F1.2 — Add a wide step counter.** `Grid` carries a `uint64_t
  step_count`, read back through `steps()` — the same split as `world_seed` /
  `seed()`. `frame_tag` is one byte and cannot be the time input to a hash:
  randomness would repeat for a given cell every 256 steps, which is visible
  periodicity, not noise. **This step's own instructions were wrong about where
  to put the increment and are corrected here rather than quietly.** They said
  to increment beside `++frame_tag`, for the stated reason that everything in a
  step should agree on which step it is — but `++frame_tag` sits *below*
  `resolve_support()`, so doing that would have given the support resolve the
  previous step's number and the sweep the current one, which is precisely the
  inconsistency the instruction was trying to avoid. The increment therefore
  goes at the very top of `update()`. `frame_tag` cannot follow it up there: it
  is stamped after support resolves on purpose, so that cells a falling
  structure just moved are not marked as already updated and still get their
  turn in the sweep. Three checks (103 total), because a counter nothing
  observes can be quietly wrong until the thing depending on it is built: the
  clock starts at zero, counts every update, and **keeps counting through steps
  where the world is empty and no chunk wakes** — it has to measure time rather
  than activity.
- [x] **F1.3 — Introduce the hash, move one call site.** `src/physics/random.h`
  holds a splitmix64 finalizer over `(seed, step, index, stream)` with `coin`,
  `chance(pct)` and `spread(range)` on top; `Grid` gets thin inline wrappers
  that fold in the seed and step so a call site names only where it is asking
  from and which decision it is making. The **sweep direction** in
  `Grid::update` was moved; the other four followed in F1.4. **One correction to
  this step as written: it claimed the sweep "needs no salt" because its input
  is already a row and a step.** That is wrong — row `y` and cell index `y` are
  the same number, and cell index `y` is a real cell, so with no stream tag a
  row's direction would be drawn from the same value as that cell's own
  decisions. It has a tag. Seven checks (110 total), and they test the mixer
  **directly rather than through the world**: the failure mode that matters is a
  mixer that is merely *poor* — biased, correlated between streams, or repeating
  for a cell across steps — and such a mixer still produces a world that
  settles, stratifies and burns exactly as every other test expects. It only
  looks wrong in motion, which no assertion about the world can notice.
- [x] **F1.4 — Move the remaining four.** The direction picks in `step_powder`
  and `step_fluid` and the chance roll in `try_react` are on distinct streams,
  each keyed on the cell's own index. **Colour jitter in `jittered_color` is the
  one that is different**: it is a one-time authored value, not a per-step
  decision, so it hashes on `(seed, index)` with the step pinned at 0 — through
  its own `authored_spread` wrapper, so the exception is visible in the header
  rather than hidden at the call site. The deliberate visible change: repainting
  a cell in the same spot produces the same shade instead of a new one. Cells
  still carry their colour when they move, because `swap_elements` moves the
  whole `Element`. Two new checks (112 total): jitter is still live (20 distinct
  shades across 32 neighbouring cells), and a cell erased, stepped past and
  repainted comes back identical. `rng()` now appears nowhere in `src/`.
    - **This step turned up a real test bug that had nothing to do with it, and
      the sequence is worth keeping.** `fire ignites adjacent wood` went to
      0/30. The reaction odds were fine — the trials were not. `count_ignitions`
      builds thirty sealed pairs that are identical down to the cell, and since
      F1.1 made the default seed fixed they had all been the *same world*, so
      "thirty independent trials of a 2:1 race" could only ever return 0/30 or
      30/30. It happened to return 30/30, so it passed, so nobody looked. F1.4
      changed the numbers, that one world lost its race, and the same broken
      test failed loudly. **The test was equally wrong before and after; only
      its answer changed.** Each trial now takes its own seed. Two things
      follow: a determinism change can expose an unrelated latent defect, so a
      failure straight after one is not automatically caused by it; and a fixed
      seed makes a suite quieter without making it stronger, so anywhere a test
      repeats a trial for statistical weight, **the repetition has to vary
      something** or the seed has silently collapsed it to one sample.
- [x] **F1.5 — Reserve the stream separation, do not build it yet.** The rule is
  written down in `random.h` next to the thing it governs, before there is a
  generator to break it: world generation draws only from streams minted by
  `worldgen(n)`, the simulation only from the declared tags, neither borrows the
  other's. Without it, generating one extra cave shifts the values the
  simulation reads for those same cells, so a terrain tweak silently alters how
  sand falls somewhere it never touched — and that is indistinguishable from a
  bug in the sand. **Streams are minted from one base constant rather than
  hand-written.** Two notes left for whoever writes it: generation is authored,
  so it passes **step 0** like colour jitter does, and `index` means "which
  thing am I asking about", not necessarily a cell.
    - **The check is a `static_assert`, and the reason it is not a test is the
      point of the step.** Two streams sharing a value are one stream, silently.
      So `random.h` lists every declared stream in `SIM_STREAMS` — the duty
      `MATERIALS` already has towards `ElementType`, enforced the same way — and
      proves at compile time that they are pairwise distinct, along with the
      first sixteen minted generation streams. **Verified by temporarily
      duplicating a value and confirming the build fails**, because an assertion
      nobody has seen fire is an assertion that might be vacuously true. Two
      runtime checks alongside it (114 total), covering what the assert cannot:
      distinctness is the weak half, since two tags one bit apart are distinct
      and still correlated.
- [x] **F1.6 — Delete `std::mt19937` and `<random>` from `grid.h`, then
  measure.** Both are gone; `Grid` stores nothing but the two numbers that are
  its whole state, `world_seed` and `step_count`. Bracketed A/B, five runs each
  rather than three, alternating hash/generator throughout: `churning`'s ranges
  overlap (no effect — this is what "below the noise floor" looks like when it
  is actually true), but `cascading`'s do not, and the hash is the slower side
  of that gap by about 1.7–1.9%. **Recorded honestly rather than as the win it
  was filed as**, in `PERFORMANCE.md` and the RNG entry in
  `ENGINEERING_NOTES.md`, both corrected in place. F1 was never conditional on
  this paying for itself — determinism is the reason F1 exists, and it does not
  stop being the reason because the side benefit didn't land.
- [x] **F1.7 — Docs.** README's Engine Architecture gained a `### Determinism`
  section, placed after Reactions and before The player, alongside the write
  rule and the wake rule it already documents. States what is actually true —
  `Grid` is a pure function of its seed, every draw goes through
  `Grid::coin`/`Grid::chance` the same way every write goes through
  `set_element`/`swap_elements`, streams keep same-step decisions from
  correlating, colour jitter is the one exception and why — and is explicit
  about what was *not* yet true at the time: the game as a whole, because input
  still entered through the render loop rather than a per-step log, which is
  F2.3.

**F1 is complete.** `Grid` is a pure function of its seed; no generator, no
state, no `<random>` in the header. What F1 left behind: a random helper anyone
can extend without relearning its rules (add a `Stream`, key on the right
index), a step counter something can depend on, and a section of this document
that had never once been fully consistent with the source until the anchoring
convention forced it to be.

#### F2 — Something owns the run

`Grid`, `Player` and `DigTool` were three unrelated locals in `main()`, declared
next to `accumulator`, `current_brush` and `brush_size` with nothing
distinguishing run state from shell state. Four separate items were blocked on
the same missing type: the world reset hotkey resets *a run*, player death
restarts *a run*, save and persistence serialises *a run*, and Quantum Worlds
swaps *a run*. Each was independently "remember to also reset that other
variable", a bug class that grows with every system added. It was three members
at the time; the point was to fix it while it was still bookkeeping and not yet
a refactor.

- [x] **F2.1 — Create `src/game/run.h` / `run.cpp` and move the three locals
  into it.** `main.cpp` holds one `Run` — `grid`, `player` and `dig_tool` are
  its public members, constructed together from `(width, height, seed)`.
  SDL-free, added to `ENGINE_SOURCES` in `CMakeLists.txt` for the same reason
  `src/physics/` is: a run that needs a window cannot be driven by a test.
  *Verify:* clean build with no new warnings, all four suites still pass at 114
  checks — unchanged, because nothing about what runs changed, only where it
  lives.
- [x] **F2.2 — `Grid::reset(seed)` and `Run::reset(seed)`.** **One correction to
  this step as written: the signature is `Grid::reset(seed)`, not
  `Grid::reset()`.** The hint list never mentioned `world_seed`, which was an
  oversight — leaving it out of the wipe would mean a "reset" grid kept
  simulating its *old* seed's randomness. `world_seed` is cleared alongside
  everything else, and taking it as a parameter is what makes `Run::reset(seed)`
  able to hand the player an actual new world rather than a re-cleared old one.
  The wipe clears **every mutable member**, directly and by hand — deliberately
  not `*this = Grid(width, height, seed)`, because that shortcut is
  compiler-generated and cannot forget a member, which would make the point of
  the test below impossible to demonstrate. **Treat the member list as a hint
  and the test as the authority**, because the next member added to `Grid` will
  not appear in it.
    - *Verify, three parts.* A scene is built without ever calling `update()` —
      a floating Wood block with one cell knocked out of it, which queues a
      support check immediately on removal — so "reset clears the queued support
      check" is proven against a queue known to be non-empty going in, using a
      new accessor, `has_pending_support_checks()`, added for the same reason
      `active_chunk_count()` was. After `reset`, every cell is confirmed `Empty`
      and every chunk asleep. Then the assertion that earns its keep: a grid is
      dirtied, reset, and driven through the same scripted scene as a completely
      fresh grid on the same seed, and the two are compared. Comparing
      immediately after `reset()`, while both are still `Empty`, would not have
      caught a stale `step_count`. **Confirmed this actually catches
      something**: commented out the `step_count = 0` line, watched the test
      fail, put the line back. Six checks (120 total).
- [x] **F2.3 — A POD `Input` struct and `Run::step(const Input&)`. This is the
  second half of determinism.** `Input` lives in `src/game/run.h`:
  `left`/`right`/`jump`/`dig`, a shared `cursor_x`/`cursor_y` (the dig aim and
  the brush centre were always the same mouse position), and the three brush
  fields. `Run::step()` paints the brush, then steps the grid, then the player,
  then the dig tool — the same order `main.cpp` ran them in, with the brush
  moved from *before the whole fixed-step loop* to *the first thing inside one
  iteration of it*. `main.cpp` still samples SDL once per rendered frame — there
  is no such thing as input for a step that has not happened yet — but calls
  `run.step(input)` once per fixed step, so "paint once per frame, however many
  steps that frame buys" became "paint once per step". `Run::FIXED_DT` replaced
  the copy in `main.cpp`. Player's own `PlayerInput` is built from `Input`
  inside `step()` — the brush is a run-level concern the player has no business
  seeing.
    - *Verify.* A new suite, `tests/test_run.cpp` (10 checks, 130 total) — the
      first driving player, tool and grid together through one entry point.
      **Then the assertion that closes F1:** the same seed plus the same
      recorded `Input` sequence, replayed through two separate `Run`s — one
      driven straight through, one consumed in batches of 7 with a render-style
      read (`get_pixels()`, `active_chunk_count()`) between batches, mimicking
      what `main.cpp` does every frame — produces a byte-identical grid and an
      identical player position. That is the property save files, replays and
      reproducible bug reports all rest on.
    - **One bug in the test itself, worth recording because of what it looked
      like.** The scripted sequence walked the player toward the world's edge in
      a grid too narrow for that math — the dig target landed a few cells past
      the border. It still "passed" its own sanity check, because
      `get_element()` reads *anything* out of bounds as `Wall` by design, so the
      check was quietly reading the world's edge and would have read `Wall`
      after a real dig too. **An out-of-bounds accident here would not have
      looked broken, it would have looked like a passing test.**
    - **A single-step assertion that turned out not to be true.** Tried checking
      `!is_on_ground()` immediately after one jump step; it does not hold,
      because leaving a floor takes a whole cell of upward movement and whether
      one step's `rem_y` carries that far is `Player`'s own integration detail.
      Checked only the velocity flip, which *is* this step's own responsibility.
    - **Manual verification done against the real window.** Drove the exe with
      synthetic input rather than trusting the rewritten input path on the
      automated suites alone. Confirmed visually: holding right/left walks the
      player through `run.step()`; right-click-dragging paints a real streak of
      falling Sand that takes the awake-chunk count from 0 to 2 and back to 0;
      and left-click digging visibly erodes the pile exactly where aimed.
- [x] **F2.4 — Confirm the payoff rather than assuming it.** No code, on
  purpose. **World reset hotkey:** trivial now — `Run::reset(seed)` does
  everything but choose the seed and bind the key. **Pause and single-step:**
  also small, and for a specific reason — before F2.3, "the fixed step" was
  three separate calls inline in `main.cpp`'s loop body, with no single thing to
  hook a pause around. **One design question surfaced that neither item's
  wording addressed:** should the brush still paint while paused? A pause that
  also freezes editing forecloses using pause *to* set up a scene precisely.
  Left open on purpose — this step confirmed shape, not policy. **Conclusion:
  neither item's shape is wrong.**

*The Manual Tester Checklist — run after any change touching `src/physics/`,
`src/game/` or `main.cpp` that the automated suites can't fully exercise — is in
**[MANUAL_TESTING.md](MANUAL_TESTING.md)**.*

#### F3 — Camera and world-space coordinates

*Prerequisite for everything in the slice. The state described in the next
paragraph is what F3 was aimed at, not what is true today.*

The world was exactly the window: `GRID_WIDTH` was `WINDOW_WIDTH / PIXEL_SCALE`,
so a level could not be larger than one screen. A "trial" the size of a 200x150
screen is not a level, it is a room. **The cost was not the camera math.** It
was that the conversion between world and screen was *scattered* — `main.cpp`
divided the mouse by `PIXEL_SCALE` to get the brush and aim cell, multiplied by
it again to place the player's rect, and again to place the aim mark. Five
sites, and every future system that reads the mouse or draws a thing adds
another.

- [x] **F3.1 — Separate world size from window size.** `GRID_WIDTH` /
  `GRID_HEIGHT` are their own constants rather than `WINDOW_WIDTH / PIXEL_SCALE`
  expressions. *Verify:* doubled both, rebuilt, painted a streak of Sand. **The
  whole world squashed to fit, as predicted** — each cell rendered at roughly
  half its normal size, since `SDL_RenderCopy`'s null/null rects always stretch
  the entire grid-sized texture across the entire window. Reverted immediately
  after confirming it.
- [x] **F3.2 — One `Camera`, and only it knows `PIXEL_SCALE`.** New
  `src/game/camera.h` — `world_to_screen`, `screen_to_world`, and `cell_size()`
  (the last since removed: nothing ever called it, and `scale_length(1)` says
  the same thing). `PIXEL_SCALE` is now `Camera::SCALE`, private to the file,
  and every site in `main.cpp` that touched it directly goes through a `Camera`
  instance. *Verify:* `grep PIXEL_SCALE src/` matches only `camera.h`. All five
  suites pass at 130 checks unchanged (`main.cpp` isn't under test, so this is a
  pure refactor `ctest` can't see either way). Confirmed live: brush, movement
  and dig aim all unshifted, which is the expected result of a coordinate
  refactor that changed no values, only where they live.
- [x] **F3.3 — Upload only what is visible.** New
  `VIEWPORT_WIDTH`/`VIEWPORT_HEIGHT` size the streaming texture instead of
  `GRID_WIDTH`/`GRID_HEIGHT`, and the per-frame upload is one
  `SDL_UpdateTexture` against a rect clamped to `min(viewport, grid)` — the
  source pitch stays the grid's real row width even though the rect is narrower,
  so SDL reads the right columns with no intermediate copy.
    - *Verify.* **`grid_bench` genuinely cannot see this** — it links no SDL —
      so the number came from a temporary probe timing the `SDL_UpdateTexture`
      call itself, removed after measurement. Title-bar fps was not usable: the
      renderer is created with `SDL_RENDERER_PRESENTVSYNC`, so frame rate stayed
      flat regardless of upload cost. Bracketed at a 2x world: the old
      whole-grid upload averaged ~42-52 us, the new viewport-clamped upload
      ~18-19 us; at a 4x world the new path stayed at ~17-19 us, unchanged. That
      is the actual claim — not "faster today", but "no longer scales with world
      size, by construction."
- [x] **F3.4 — Follow the player, clamped at the world edges.**
  `Camera::follow(...)` centers the viewport on the player and clamps to `[0,
  world - viewport]` per axis. Because the offset can differ per axis,
  `world_to_screen`/`screen_to_world` split into `_x`/`_y` variants, and a new
  `scale_length()` covers the two call sites converting a *size* rather than a
  position — those must never have the offset subtracted, or the player's rect
  would shrink and grow as the camera panned. Feel work (deadzone, smoothing,
  look-ahead) stayed out of scope.
    - **Confirmed live at a 2x world**, the only way to see this step do
      something: a temporary build force-set `input.right = true` every frame,
      since synthetic keyboard input does not reach this window in this
      environment — confirmed independently on an unrelated key (`Escape`, which
      should quit and didn't). Near spawn the player sits centered; after
      several seconds of rightward movement it is pinned near the *window's*
      right edge — the visual signature of the camera having hit `max_view` and
      stopped scrolling while the player kept walking.
- [x] **F3.5 — Answer off-screen simulation once, in writing.** Everything
  simulates, always, regardless of the camera. The viewport is a *render*-only
  concept and it stays that way. The moment it also decided whether a chunk gets
  to step, it would quietly become a second scoping mechanism competing with
  chunking — unnecessary, since chunking already makes an idle region cost
  nothing on its own, and actively wrong for the region that *is* active: an
  avalanche the player triggers and walks away from would freeze the moment it
  left the viewport. **The sharper argument is determinism, not performance.**
  If simulation ever depended on camera position, two players on the same seed
  and the same input log could diverge purely because they were looking at
  different things — exactly the failure mode F1 exists to rule out, reimported
  through a feature that was only supposed to touch rendering.
    - **This also answers the open half of Resolution options without
      re-deciding it there.** World size and window size stay independent in
      *both* directions — a bigger window reveals more of a world that was
      always fully simulated.

#### F4 — A way to get a level into the grid

**This was a circular dependency in this document, not a missing idea.** Quantum
Worlds cannot exist without some way to populate a grid — nor can Objective +
Extraction. The loader that does it was written down only inside **First
authored scene + art pipeline**, in Presentation & Tooling, a section whose own
header says nothing in it starts while the slice is open. So the slice's first
item needed a thing that was forbidden to build until the slice was finished.

**Scope was phases 0-2 of the art-pipeline plan and nothing else** — about 160
lines. **Phases 3-6 became V1–V4 in Engine & Visual Depth**, by the same
mechanism and for a different reason: they stopped being "presentation, after
the loop" the moment the visual design became a stated selling point. The pixel
art **editor** stays deferred. F4 is deliberately agnostic about where levels
come from — the same loader serves a hand-painted scene and a generated one,
which is why it belongs in Foundations rather than inside either feature.

- [x] **F4.1 — `Grid::paint(x, y, type, colour)`.** Placement with an explicit
  colour instead of a jittered one, which is what lets authored art keep its own
  pixels. `set_element` and `paint` share one private `place()` that does the
  actual write — `mark_dirty` and the structure-removal support check live in
  exactly one place, so the two paths cannot drift apart the way a copy-pasted
  second implementation invited. *Verify:* **the wake check was rewritten rather
  than left as the `active_chunk_count() > 0` proxy it shipped with** — that
  proxy passes for any write at all and could not have caught a missing
  `mark_dirty`. The real check paints an unsupported Sand cell (a powder, so its
  fall is driven by the ordinary movement rule rather than the support queue —
  painting Wood would have asserted something false, since a freshly placed
  structural cell is deliberately left standing so a scene can paint a platform
  without it collapsing on load) and steps the world: it lands at the bottom,
  still carrying the colour it was painted with.
- [x] **F4.2 — Scene format and a headless loader in `src/scene/`.** Two
  same-size buffers, a material map and an albedo, in
  `src/scene/scene.h`/`scene.cpp` — SDL-free. `load_scene(grid, scene, offset_x,
  offset_y)` skips cells whose material is `Empty` rather than painting them:
  `Empty` has no legend colour, so its albedo pixel is an accident of whatever
  the art file holds there. *Verify:* `test_scene.cpp` builds a 2x2 buffer pair
  in memory, loads it at an offset, and asserts the grid matches cell for cell —
  no file I/O.
- [x] **F4.3 — BMP decode in `main.cpp`, loaded at startup.** *(the decode moved
  out of `main.cpp` to [src/scene/bmp.cpp](src/scene/bmp.cpp) with P4 on
  2026-08-13, and no longer goes through SDL at all; the account below is of the
  code as it shipped, and the pitch bug it describes is the reason the
  replacement is a single reader rather than two.)* `load_scene_from_bmp` reads
  two BMPs via `SDL_LoadBMP`, converts both to `ARGB8888`, and maps each
  material pixel to an `ElementType` by exact RGB match against `MATERIALS`.
  **Shipped broken and is the one step this pass had to fix rather than
  verify.** Three separate problems, each alone enough to sink it: the pixel
  loop indexed converted surfaces as `y * width + x`, ignoring
  `SDL_Surface::pitch`; the scene was authored at 640x400 while
  `GRID_WIDTH`/`GRID_HEIGHT` were still 200x150, so every cell landed outside
  the grid and `paint`'s bounds check dropped the entire scene without a
  warning; and the BMPs were loaded by a path relative to the working directory
  with no asset-copy step. Fixed all three: the loop walks each surface's own
  `pitch`; `GRID_WIDTH`/`GRID_HEIGHT` are `640`/`400` (deliberately bigger than
  the 200x150 viewport, which is what exercises the panning half of
  `Camera::follow`); and a `POST_BUILD` custom command copies `assets/` next to
  the built executable.
- [x] **F4.4 — Make the first scene a test fixture wearing art.**
  `generate_test_scene.py`, rewritten — the version it replaced was four flat
  rectangles that exercised nothing, at a resolution that didn't match the grid.
  The new layout at 640x400: an uneven Sand slope built from randomized 1-3 cell
  treads (blocky stairs, not a smooth ramp — a smooth ramp doesn't ask anything
  different of the player's step-up than flat ground); three standalone Wood
  fence posts for dig-the-base collapse; a real pit (floor genuinely absent)
  spanned by two grounded pillars and a beam, so digging one pillar exercises
  the support flood fill carrying the beam and the far corner down together —
  the L-piece case; a walled water channel with a diving ledge above it; a row
  of Wood sleepers close enough that breaching the wall lets spilled Water reach
  them; and four jump ledges at mixed heights. *Verify:* decoded the material
  BMP and confirmed every legend colour matches `MATERIALS` exactly (a
  near-match loads silently as `Empty` and would defeat the whole scene the same
  way the size mismatch in F4.3 did).

#### F5 — Fixed-point player kinematics *(shipped 2026-08-12)*

**The determinism guarantee was machine-local and three separate items were
spending it as portable.** `Grid` had been integer-only since F1.7 and was held
to it explicitly through the thermal pass; `Player` was not, and the gap was
correctly identified in ENGINEERING_NOTES.md, correctly filed as unscheduled,
and correctly annotated with "do not quote *the simulation is deterministic* as
covering the player". **It was then quoted as portable anyway**, by crash
diagnosis (whose whole idea is a crash report that reproduces the crash), by
save and persistence, and by the first non-Windows build — the item that would
have discovered the problem and had no reason to be looking for it.

That is the shape worth keeping from this item: **writing a limitation down does
not stop it being spent.** What stops it is the limitation not existing. F5 ran
*now* rather than later on a cost curve — `S0` reads `vel_y` for fall damage
next, and V14/V15 add solvers after that; converting four fields is days,
converting four fields plus a damage model plus a rig is not.

- [x] **F5.1 — `fx`, signed 16.16, in
  [src/physics/fixed.h](src/physics/fixed.h).** Not a class: a wrapper type
  would buy compile-time unit safety and cost the ability to read the arithmetic
  by eye, and the arithmetic is the half that has to be checkable. 16 fractional
  bits resolves ~1/16000 of a screen pixel; 16 integer bits holds a
  cells-per-second speed of ~500 against a range of 32767. Neither half is close
  to its limit, which is what makes the split boring rather than tuned. The one
  trap is pinned with a `static_assert`: **`fx::trunc` truncates toward zero and
  is not `>> 16`**, which floors — flooring would make a body drifting left take
  a whole-cell step every step and grow a remainder pointing *right*, walking
  left through terrain collision was never asked about.
- [x] **F5.2 — `rem_x`, `rem_y`, `vel_x`, `vel_y` converted; the constants
  written as exact rationals.** `fx::from_ratio(225, 2)`, never `fx::v(112.5f *
  65536)` — a compile-time float fold is exactly as machine-dependent as a
  runtime one, so a float literal on the path to a constant gives back what the
  item was for. TUNING.md's rows are unchanged in value and its line numbers
  were re-checked, which is the maintenance that file's deliberate use of line
  numbers costs.
- [x] **F5.3 — `dt` deleted from `Player::update()`.** ROADMAP_ITEMS predicted
  this as "the timestep is a compile-time rational rather than a parameter", and
  there turned out to be a second and better reason: **every caller already
  passed the same compile-time constant.** A parameter nobody varies is an
  invitation to vary it, and the first caller to pass a real frame time would
  have re-created the defect F1 spent a whole item closing. `Run::FIXED_DT` is
  now *derived* from `fx::STEPS_PER_SECOND` rather than written beside it — one
  rate, one spelling.
- [x] **F5.4 — Three relationships asserted at the constants**, the move
  `element.h` and `reaction.h` make about their data tables and for the same
  reason: TUNING.md invites these numbers to be changed by feel, and prose about
  how two of them relate goes stale the first time one is retuned alone. The
  fixed-point range against `MAX_FALL_SPEED`; that a wingbeat still outweighs
  the gravity it pays for; that `FLAP_MAX_CLIMB` stays under `JUMP_SPEED`. None
  of the three forbids a design change — each refuses to let one happen
  *silently*.

**The verify condition could not be met as written, and that is the item's most
useful finding.** ROADMAP_ITEMS asked for traces "identical to the float version
to the cell". Measured over 1381 recorded steps of fall, walk, jump and
sustained flight: **7 steps differ, each by one cell, each re-converging
immediately.** Walk and jump are byte-identical; every landing row, resting
position and jump peak matches. The cause is not a rounding mode that could have
been chosen differently — **1/60 is not representable in binary at any
precision**, so a step of `GRAVITY` is 8.333328 cells/s in fixed point against
8.333334 in float and neither is 8.3̇. Exactness was never on offer in either
scheme, and the condition should have said "no persistent divergence", which is
what was actually wanted and what was actually got.

**⚠️ It did not finish the job, and the residue is named rather than quietly
carried.** `DigTool::march` still chooses which cells a dig removes from a
`float` `sqrt` and two `lround`s, and **digging writes to the grid** — so across
toolchains that is a different world, not a different pixel. It is now the only
float left under `src/physics/`. Perhaps an afternoon (`len <= RANGE` becomes a
squared comparison, the `lround`s become rounded integer division), unscheduled
rather than refused, and **until it closes "determinism is portable" still
cannot be said** — which is the exact sentence the three items above were
spending in the first place.

> **Closed 2026-08-13 by `F6` below**, an afternoon as predicted. The estimate
> and the two named edits were both right; what the paragraph did not know is
> that the same expression was also holding a live 32-bit overflow.

#### F6 — `DigTool::march` is integer-only *(shipped 2026-08-13)*

**This is F5's residue, and it ran next because it was the cheapest item on the
board and three later items were spending the sentence it blocks.** The
paragraph above named it, sized it and left it unscheduled; the gate
prerequisite "build on macOS and Linux at least once" carried an instruction to
close it *first*, or the answer that item returns is "no" for a reason already
known.

- [x] **F6.1 — The range test is squared.** `len <= RANGE` is `dx*dx + dy*dy <=
  RANGE*RANGE`, so no length is ever taken. The exact-distance case is what a
  `sqrt` and a squared comparison can disagree about — `sqrt(3600)` coming back
  a bit under `60.0` shortens every dig on that toolchain and on no other — so a
  3-4-5 triangle putting a cell at *precisely* `RANGE` is now a test, along with
  the same ray one cell longer, which must fall outside.
- [x] **F6.2 — The out-of-range step count keeps its truncation without a
  division by a length.** The old `int(span * (RANGE / len))` is
  `isqrt(span*span * RANGE*RANGE / dist2)`, which is the same number because
  `floor(sqrt(x))` is `floor(sqrt(floor(x)))`. `isqrt` is Newton's method on
  integers, in the anonymous namespace in [tool.cpp](src/physics/tool.cpp) — not
  a `std::sqrt` cast to `int`, which would put the float straight back.
- [x] **F6.3 — The two `lround`s are `div_round`, rounding halves away from
  zero.** The obvious replacement, `(a + b/2) / b`, is **wrong and would have
  shipped silently**: C++ integer division truncates toward zero, so the
  negative side rounds the other way and digs land one cell off in two of the
  four quadrants. Doubling both terms compares the half without a fraction. A
  mirror-symmetry test asserts the ray up-left is the exact negation of the ray
  down-right, which is the assertion that catches it.

**It also silently closed a two-answers-to-one-question split.** The crosshair's
dim-past-range indicator in `main.cpp` already compared `dx*dx + dy*dy` against
`RANGE*RANGE` in integers; `march` compared a `float` length. One question, two
arithmetics, two files, free to disagree by a cell at the boundary — the same
shape as D1's two clocks, and nobody had noticed because nobody had aimed at the
boundary and counted. They are now the same comparison. This is why checklist
step 3 was worth running on an item whose tests all pass — **it did run, on
2026-08-13, and passed**: the boundary was not reported off by a cell in either
direction, and no diagonal was reported outreaching a straight shot.

**The item found a real bug, and it is not the one it was for.** `dx * dx + dy *
dy` was computed in `int`. At an aim ~46,000 cells out that product overflows a
32-bit signed integer, so the length came back garbage and the range limit
stopped applying — a dig either doing nothing or reaching across the level, and
both read as a *range* bug rather than as an overflow. **A test asserting the
range still holds at a distant aim fails against the unfixed code and passes
after**, which is the failing-test-first rule paying for itself on an item that
had no defect attached to it. Not reachable from the mouse today, since the
cursor is bounded by the window and `aim_x`/`aim_y` come from it — but the
arithmetic is now 64-bit throughout, which is the fix regardless of who can
currently reach it.

**What this does and does not license — and one wording correction while it is
being said.** No float under `src/physics/` reaches the grid any more, so a
replay that includes digging is reproducible in principle on any conforming
compiler. **The right sentence is "no float reaches the grid", not "no float in
`src/physics/`"**, which the paragraph above and three other documents were all
drifting toward and which a grep disproves: `DigTool::swing_progress()` is still
a float, exactly like `Player::visual_x()`/`visual_y()`, because the animation
is the only thing that reads it. That is the boundary working as designed rather
than an exception, and the distinction is worth the extra clause — this
project's recurring failure is a stated rule that stopped matching the code and
kept being believed. **It is still not a verified fact** — nothing has ever been
built anywhere but this machine, and that is exactly what the gate prerequisite
exists to check. F6 removes the known reason the answer would have been "no"; it
does not answer the question.

### <a id="t1-the-debug-tooling-batch"></a>T1 — The debug tooling batch *(shipped 2026-08-14)*

*Four items lifted out of [Presentation & Tooling](ROADMAP.md#-sandbox--debug-tooling) by
the 2026-08-11 plan review and put in front of `E10` and `E5a`: **world reset
hotkey, pause and single-step, a free camera, and the cell inspector.** `T` is
its own track letter and still holds exactly one item, on purpose — this is the
tooling that is a *prerequisite* for scheduled work, as distinct from the
tooling in that section that is a convenience.*

**Why it ran at all is the part worth keeping, because the argument had been
made three times and filed behind the slice three times.** This file's own
V-track note records that V2 could not be verified in the running window, says
every remaining V item has the same problem, and says the fix "should be pulled
forward the first time a V item cannot be checked" — and then V2 shipped a blank
world for a commit. E10's verify condition is *"a poured pile holds a measurable
angle"* and E5a's is *"a cell fired at a wall lands against it"*; neither is
checkable at 60 frames a second through a camera bolted to the player, with no
way to pause and no way to read a cell. **An argument that keeps getting made
but never gets filed against a position in the running order is
indistinguishable from one nobody made**, and that is what this item existed to
break.

- [x] **T1.1 — Pause (`P`) and single-step (`.`).** The pause freezes by not
  accumulating time, which is the **third** user of that one mechanism rather
  than a second one — the settings menu was first, a finished run second in
  `S0`. The alternative the entry warned about, skipping the step loop while the
  accumulator keeps filling, banks every paused second and spends it in one
  burst at `MAX_FRAME_TIME`'s quarter-second clamp. Single-step runs exactly one
  step and spends none of the accumulator, so `.` advances the world by a number
  you know.
- [x] **T1.2 — A free camera (`F`), panned with the movement keys.** `Camera` is
  unchanged: it still owns every conversion and the clamp, and detaching simply
  hands it a different centre. The body's movement input is suppressed while the
  camera is loose, so it stands still instead of walking off unwatched —
  **suppressed before the recorder sees it**, so what the log holds is what the
  simulation was given and a session with camera work in it still replays
  exactly.
- [x] **T1.3 — The cell inspector (`I`).** Material, temperature, whether the
  chunk is awake, and whichever of the fall clock, gas lifetime and piece tag
  apply. E2 and E3 both added per-cell state that has never been readable while
  the thing being debugged was on screen.
- [x] **T1.4 — A world reset hotkey (`Ctrl`+`R`).** `Run::reset(seed)` did
  everything but choose the seed and bind the key, exactly as F2.4 predicted.
  **It keeps the current seed** — a debugging session is worth nothing if the
  world changes underneath it, and the recorder's header seed is written once at
  startup, so a reseeding reset would silently make every later `F9` replay into
  the wrong world. That is a log that is *wrong* rather than absent, which is
  the failure P4 exists to prevent.

**The decisions this item was told to close are left open on purpose, by the
user's call on 2026-08-14, and what got built forecloses neither.** The `R`
question is answered by a modifier rather than by a merge: `R` keeps `S0`'s
meaning and is still inert mid-run, `Ctrl`+`R` is the unconditional one, and
whether they should ever be one key is still in the [decisions
table](ROADMAP.md#-decisions-owed). Brush-while-paused is answered the
same way — **no step runs while paused, so the brush does not paint, for the
same reason it does not on a finished run**: not because painting is refused,
but because painting is part of a step. Pressing `.` paints one stamp. The third
state that entry contemplated, a pause that steps the brush and not the physics,
is not built and is still the open question.

**It found a false claim in the engine, and the instrument found it on the first
thing it was pointed at.** `Grid::active_chunk_count()` said *"zero means the
world has come completely to rest"*. It does not. A falling structural piece is
carried by `pending_support`, and `resolve_support()` runs *before* `update()`
swaps the chunk rects — so its writes mark the set that is about to become this
step's work, the sweep adds none of its own because a Static cell does nothing
in the sweep, and **a slab can fall the height of the world with the counter
reading zero the whole way.** Nothing about the fall is wrong; the sentence was.
"At rest" is `active_chunk_count() == 0` **and** `has_pending_support_checks()
== false`, the corrected comment says so at the code, a test in `test_debug.cpp`
pins it, and the HUD's `CHUNKS:` now carries a `+FALLING` flag so the same wrong
reading is not available on screen either.

- **The transferable form, and it is the reason this project builds instruments
  before the items that need them:** the claim had been on that method since
  chunked sleep was written, it is read by the HUD, by the benchmark's census
  and by two suites, and **no test could have caught it because every test that
  cares about sleep uses powders**, which do all their moving inside the sweep.
  It took a person pointing something at a falling slab, and the thing to point
  had not existed until this item.
- **It also answers the sizing question honestly.** T1 was two days and took an
  afternoon, which is the opposite error from `S0`'s and has the same cause: the
  estimate priced the *building*, and here almost nothing had to be touched.
  `Run::reset` was already the one reset path, `Camera::follow` was already
  given a centre rather than reading the player, and `Run::step` was already the
  single place a step happens. **Three earlier items had each left the seam this
  one needed**, which is what a foundation track is for and is not something the
  estimate could have known.

### <a id="e1e3-simulation-depth"></a>E1–E3 — Simulation depth

- [x] **E1 — Liquids find their level.** A blocked liquid cell with `Empty`
  directly above it — a surface cell — searches its own connected body for
  another surface at least 2 rows lower, and moves there. Bounded at 64 cells of
  search (`MAX_PRESSURE_CELLS`) and gated at 2 rows of difference
  (`MIN_PRESSURE_HEAD`), both in `grid.h` with the reasoning next to them. A
  U-tube converges to dead level in under 50 steps and then sleeps.
    - **E5b replaces this mechanism, and the limitation that argues for
      replacing it was not visible when this shipped.** The move is a search
      followed by a teleport across the body, so it is instant and non-local:
      there is no propagation delay anywhere in it. That makes waves, sloshing,
      surges and water hammer not *missing* but **impossible**, as a consequence
      of the mechanism rather than of anything left undone. A pressure field has
      the delay built in and gets them for free. Nothing here was wrong — this
      is the correct cheap answer to the question that was actually asked, and
      it is what a later item is measured against.
    - **The direction of the move is the decision, and the obvious direction is
      the one that does not work.** The first implementation was the one the
      item's own wording implies: let the short arm *rise*. It does equalize —
      for two cells, and then it stops. Rising is a swap, so it leaves a bubble
      of `Empty` inside the body, and the transfer is not finished until the
      ordinary fall and spread rules have walked that bubble back down the arm,
      along the join and up the far side. That takes twenty-odd steps, and while
      the bubble is in the join it **cuts the body in two**, so the pressure
      search transiently answers "no head", the cells stop marking themselves
      dirty, the chunk sleeps, and the U-tube parks itself two cells out of
      level with nothing awake to notice. Every fix for that is some way of
      keeping an unlevel body awake, which is a standing cost charged to every
      pool in the world to serve the one that is out of level. Moving the *tall*
      cell down onto the low surface instead makes each transfer a single atomic
      swap: no journey to stay awake for, and the wake-up is free and local,
      because the 3x3 mark on the vacated cell is exactly the cell below it —
      the next surface cell and the next one to move.
    - *Verify, six checks (155 total).* The U-tube converges and **conserves
      water** — 25 cells placed, 25 after, which is the check that actually
      matters, since the obvious way to make water level is to invent some. A
      third check the roadmap did not ask for and should have: the U-tube is
      **asleep** once level. Level is only half of it — a rule that equalizes
      and then trades cells back and forth across the join is level on average
      and costs full price forever, and `MIN_PRESSURE_HEAD` being 2 rather than
      1 is the entire reason it does not. Then the negative case, three checks
      on a settled flat pool: it does not climb, it conserves, and it goes back
      to sleep.
    - **The fixture was wrong twice before the engine was right once, and both
      times it was the collapse rule behaving exactly as specified.** A U-tube
      drawn as walls in mid-air is a structure standing on nothing: the first
      swap of water inside it queues a support check and the whole container
      sinks through its own contents. Rebuilt on the world's bottom row — and it
      *still* dropped a cell, because the divider between the two arms sits over
      the join, so it is a slab hanging in mid-air even when the outer walls are
      not. It needs a lid. Worth recording because both failures presented as
      "the water did something strange", and neither was about water.
- [x] **E2 — Heat, the seventh axis.** The one deliberate pillar addition,
  bought by a named observation rather than by an itch. *Built as:* a `uint8_t
  temperature` on `Element` defaulting to `AMBIENT_TEMPERATURE` (20, on a scale
  read as degrees Celsius so the constants mean something to a person); three
  thermal columns on `MATERIALS` — `conductivity`, `spawn_temperature`,
  `heat_source`; and `Grid::step_thermal`, one pass per awake cell riding the
  existing sweep. `REACTIONS` rows gained a `min_temp`/`max_temp` window, and
  Wood and Oil moved from `{catalyst: Fire, chance: 12%/40%}` to `{spontaneous,
  100%, above 120/90}`. **The ignition point lives on the reaction rather than
  on the material**, which is a departure from this item's own original wording:
  a threshold is a property of a transformation, not of a substance, and Water
  needed exactly the same field to boil.
    - **The memory claim held, and it was checked the way this item demanded.**
      `sizeof(Element)` is still 12 — `static_assert` in `element.h`, plus a
      runtime check in `grid_test` so that an assertion which quietly stops
      being tight is visible. Counting fields would have given the same answer
      here; that is not the point, since counting fields is exactly what made
      `ENGINEERING_NOTES.md` wrong for several revisions.
    - **Four neighbours was the wrong number, and the burning-beam test is the
      only reason that is known.** Conduction was written orthogonal-only,
      borrowing E1's argument that a diagonal step lets two bodies touching at a
      corner exchange through a seam with no area. That argument is about moving
      *matter* and does not transfer. An ignited Wood cell becomes Fire, Fire is
      a gas, so it rises out of the beam on the next step — and the flame that
      should light the next cell along is then sitting diagonally above it and
      nowhere else. **The fire front stalled after exactly one cell**, and no
      conductivity would have fixed it, because heat cannot cross a gap the rule
      says does not exist. Eight neighbours also puts heat on the same
      neighbourhood as `has_neighbor` and `mark_dirty`, so contact means one
      thing throughout the engine. What caught it was the negative half of a
      test written to check that a beam burns *through* rather than lighting up
      all at once.
    - **The first measurement said 18% and that number was not accepted.** The
      full pass cost `cascading` +18% and `churning` +32%, putting the stated
      number to watch at **88% of a 60 Hz frame** against 71% before. This
      document's own rule says P1 gets pulled forward if one E item breaks the
      budget. It was not invoked, because the cost was almost entirely waste:
      `cascading` contains no fire and never gets warm, and it was paying eight
      neighbour probes per awake cell per step to confirm that nothing had
      changed. **A cell at exactly ambient now does no thermal work at all**,
      which is exact rather than approximate — conduction writes both ends by
      the same amount, so it does not matter which of a pair initiates it, and
      nothing can be off ambient and asleep. That took the overhead to **+2.2%**
      on `cascading` and **+2.7%** on `churning`, with `burning` at +29% of 3.3%
      of a frame. **The first honest measurement of a feature is a measurement
      of the first implementation of it, and reaching for the escape hatch in
      the plan before reading what the number was made of would have spent a
      whole roadmap item to avoid one comparison.**
    - *Verify, fourteen checks (169 total).* Ignition is no longer statistical,
      and that is the observation E2 answers rather than a tidier test: the old
      assertion was thirty independent seeds and a bar at 40%, because a
      12%/step roll raced Fire's own 6% burnout. It is now a threshold — Wood
      catches at step 11, Oil at step 8, same answer every run, and *the number
      of steps* is finally a quantity worth asserting on. Then: heat conducts
      out of a flame and **stops**; a beam burns away at the lit end while the
      far end stays Wood at ambient; water boils; steam condenses and conserves
      matter doing it; the byte is free; and **a burnt-out world cools back to
      ambient and goes to sleep** — the check the whole axis stands on, since
      heat that never settles keeps a chunk awake forever and does it silently.
    - **Three existing tests had to change, and each change is E2 being right
      rather than a test being loosened.** "Water extinguishes fire into steam"
      read Steam at 30 steps before heat existed and Water at 5 steps after — a
      puff pinned against cold stone dumps its heat into it and condenses — so
      it now asserts the cell *passed through* Steam, which is what the test was
      ever about. "Steam rises" came down from 300 steps to 100. And the
      steam-condensing fixture moved out of a sealed Wall box into open air:
      `Empty` has conductivity zero, so steam in air cools only by the slow
      bleed and lives a couple of hundred steps, where steam packed against
      stone is gone in ten.
- [x] **E3 — Collapses break instead of dropping rigid.** Answers the
  observation with **fracture rather than rotation, and that substitution is the
  decision, not an approximation of one.** A piece whose support is uneven
  splits along the stress into separate pieces that fall independently, reusing
  the support flood fill that already exists. True rigid-body rotation on a cell
  grid means resampling the piece every step it turns, which destroys the exact
  authored pixels [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md) calls the entire
  visual pillar — the feature would be bought by breaking the reason the engine
  is interesting. Fracture costs nothing in that currency, and masonry mostly
  *breaks* rather than tips anyway. Toppling is not dropped; it is **E8**.
    - *Built as:* a `uint8_t piece_tag` on `Element`, and
      `Grid::fracture_landing`. The support flood fill only crosses between two
      structural cells whose tags match, so **a crack is stored as a
      disagreement between two cells** rather than as a line between them —
      which is what lets it survive the piece moving, since `swap_elements`
      carries whole Elements. `is_grounded` was widened to match: "more of the
      same structure is not support" now means the same *piece*.
    - **A piece breaks when it lands, not while it falls, and that timing is the
      entire safety argument.** Every "nothing must move here" test in the suite
      is about a piece at rest, and a piece at rest has `fall_ticks` of zero and
      never reaches the code at all. Fracture cannot *start* a collapse; it can
      only let one that was already happening finish unevenly. That is why the
      pre-existing negative tests passed untouched on the first run.
    - **Splitting a falling piece would have been a no-op, which is worth
      recording because it was the obvious design.** Break a piece in mid-air
      and both halves are unsupported, so both fall by exactly one cell on
      exactly the same steps — identical to not having split it — and the next
      fill re-discovers them as one component because they are still touching.
      **Fracture without persistent state is impossible on this
      representation.** ~~It was the last free byte: `Element` is now 12 bytes
      with no padding left, so the next field added there is the first that
      actually costs memory.~~ *(Corrected 2026-08-13: `piece_tag` was the last
      free byte **of the tail hole**. Three more sit in the alignment hole
      between `type` and `color` and were never counted — measured at the
      instrumentation sitting, recorded at `element.h`'s `static_assert`.
      Nothing about E3 changes; this sentence was simply repeated into four
      documents and used to sequence two later items.)*
    - **Where the crack goes was wrong the first time, and the failure was
      silent.** The first rule put it near the middle of the piece with a random
      offset, on the theory that masonry breaks somewhere arbitrary. It does not
      work: a break only *does* anything if it separates a part that is held up
      from a part that is not, and anywhere else both fragments rest on the same
      ground, so nothing moves and the only trace is a tag nobody can see.
      Reading the ground instead makes this item's own words literal — the crack
      is the line between the columns that landed on something and the columns
      that landed on nothing — and it has a second benefit that was not the
      reason for it: **a piece landing flat on flat ground does not break at
      all**.
    - **Two more bugs, both of which presented as "fracture is not firing".**
      The fill that identifies the landed piece cannot follow structure, because
      the instant a slab touches the floor the two *are* one structural
      component — it walked out of the slab, across the whole floor, past
      `MAX_SUPPORT_CELLS`, and gave up. The piece is identified by being **in
      flight** (`fall_ticks != 0`) instead. And the landing has to be detected
      at the fill's *seed*, not at the grounded cell it ends on. Three wrong
      guesses in a row, all failing quietly and identically from the outside,
      which is the argument for instrumenting rather than reasoning about the
      fourth.
    - *Verify, ten checks (179 total).* The negative case first, as this item
      demanded: a large piece at rest is never broken, digging a hole through a
      grounded slab does not break the rest of it, and a piece that drops a
      single cell onto the floor lands intact — `FRACTURE_MIN_TICKS` is what
      separates "it tipped off a ledge" from "it came down". Then the positive
      case: a slab dropped across a step ends at two heights instead of one
      rigid level, which **doubles as the proof that a crack persists**, since
      the overhang can only descend if the *following* step's fill refuses to
      cross the crack.
    - **The benchmark could not price this and said so only under questioning**
      — `collapsing` drains its slabs *before* they land, by design and in its
      own comment, and fracture fires on landing and nothing else. The first
      bracketed A/B ran the feature zero times and returned a confidently flat
      number. A `shattering` scenario was added to `bench_grid.cpp` to close the
      gap.
    - **What this does not do, named rather than left to be discovered.** A
      piece breaks at *one* boundary per landing, so a slab coming down across
      four ledges breaks in two, not five.

### <a id="v1v2-visual-foundation"></a>V1–V2 — Visual foundation

- [x] **V1 — Transparent `Empty` and a backdrop layer.** `Empty` is `0x00000000`
  in `MATERIALS` and the streaming texture is created with
  `SDL_SetTextureBlendMode(SDL_BLENDMODE_BLEND)` — it was indeed absent, so the
  cell texture was painting opaque black over anything drawn behind it. Both
  changes affect no physics: `Empty`'s colour reaches the pixel buffer through
  the same `place()` path as every other material and is never read as a colour
  by anything in `src/physics/`.
    - **The two lines are only meaningful together, which is worth stating
      because either one alone looks like it did nothing.** A transparent
      `Empty` with no blend mode carries its alpha all the way to the screen and
      has it ignored — pixel-identical to the opaque black the table used to
      hold. A blend mode with an opaque `Empty` composites a fully opaque
      texture, which is also a no-op. There is no intermediate state in which
      half of this step is visible, so there is nothing to bisect if it goes
      wrong.
    - **The backdrop is a placeholder gradient, and that is the deliberate line
      between this item and the art pipeline.** What V1 delivers is the *layer*.
      A stack of `SDL_RenderFillRect` bands before the `SDL_RenderCopy`,
      replacing the `SDL_RenderClear` that used to fill the window with black.
      Authored backdrop art is **V8**.
    - *Verify.* Sixteen bands was the first number and **the steps between them
      were visibly banded on screen**, which reads as a rendering bug rather
      than as a placeholder — raised to 64, still rects rather than a per-pixel
      gradient because the renderer is vsync-bound and this has to stay free.
      Confirmed live against the F4 scene: the sky, the pit's interior and the
      gap under the bridge beam all show the gradient through, while Wall, Wood
      and the water channel render opaque over it — the pit is the one that
      actually proves it, since it is a region of `Empty` fully enclosed by
      terrain rather than open sky.
- [x] **V2 — Palette and jitter pass on `MATERIALS`.** Table only, exactly as
  scoped: eight `color` values and seven `color_jitter` values changed and not
  one line outside `material.h`. All six suites pass at 179 checks — the colour
  tests are written against `paint`'s pass-through and against "jitter produces
  more than one shade", never against a literal from the table, which is why a
  full repaint costs nothing to fix.
    - **The set is chosen against the backdrop, not against black.** V1 put a
      cool dark blue behind everything, so rows picked in isolation stop
      agreeing the moment that is true. The world went warm and desaturated so
      it separates from the backdrop by hue as well as by value, and **Fire and
      Water are the only rows that keep real saturation** — they are the two
      things that must never be missed on screen. Water moved from `#4444FF` to
      a teal `#2E7F96` for a compositing reason rather than a taste one: a blue
      liquid seen through a gap in the terrain read as a hole in the world
      rather than as water in it.
    - **Jitter was 8–24 and is now 3–6, with Fire at 16.** Random per-cell noise
      fights hand-placed dithering, and cutting it is the only way both can
      exist. Fire keeps the widest range because for fire the variation *is* the
      thing being drawn.
    - **The F4 scene is not affected by any of this, which was not obvious until
      it was checked.** `load_scene` calls `grid.paint(x, y, type, color)` with
      the colour out of the albedo BMP, and `paint` writes the caller's colour
      verbatim — so authored terrain has never taken its colour from `MATERIALS`
      at all. What the table governs is brush-placed cells, reaction products,
      and anything the engine creates. **That finding is what V6 exists to
      fix**, and the seam it names — authored wood burning into table-coloured
      fire — is the absence of a shared palette rather than a bug in either
      half.
    - *Verify.* **The intended check — paint a streak of each material in the
      running window — could not be made to happen, and the palette was signed
      off on a swatch sheet instead.** The startup camera sits below the F4
      scene, so the window shows only sky; synthetic input never reached the SDL
      window, with the HUD's brush indicator never leaving `SAND` as proof it
      was being dropped rather than mis-aimed. What was looked at is a sheet
      that parses the eight colours straight out of `material.h` and draws each
      one flat and at both jitter extremes over a reproduction of `main.cpp`'s
      64-band gradient. That is enough to judge harmony, saturation and the size
      of the jitter, and it is *not* enough to catch anything about how these
      read in motion or against the authored albedo. **This is now a scheduled
      problem rather than a one-off** — see the note at the top of Sandbox /
      debug tooling.
    - **This item shipped a critical regression and the note above is where it
      hid.** "Not one line outside `material.h`" was true and still wrong: the
      scene loader identified materials by matching the material map against
      `MATERIALS[i].color`, so retuning the palette made all 27,192 authored
      pixels of `assets/test_material.bmp` match nothing and load as `Empty`.
      **The game booted to a blank world for a whole commit** while every suite
      stayed green. The verify step above is the exact place it would have been
      caught — and it is the step that was substituted with a swatch sheet
      because the window could not be driven. Fixed in the correctness pass
      below.

### <a id="correctness-pass"></a>Correctness pass — a full read of the source

*Not a planned tier. A line-by-line review of every file in `src/`, with each
suspected defect reproduced by a compiled probe before it was believed and
re-run after it was fixed. Six months of "the tests pass" is not the same as
"the code is right", and the gap between those two is what this found.* Suite
went 179 → **199 checks**.

- [x] **The startup scene loaded as an entirely empty world.** See V2 above for
  the cause. The fix is a split: the **material-map legend is now its own frozen
  table** ([src/scene/legend.h](src/scene/legend.h)) and no longer the render
  palette. A legend value is an arbitrary, permanent marker — the same standing
  as a `sim_random::Stream` tag — so the art direction is free to change without
  invalidating scene files. Compile-time asserts cover distinctness and full
  coverage; a test asserts the legend has not drifted back into being the
  palette.
    - **Three defences, because three things had to fail at once.**
      `element_from_legend` returns *false* for an unknown colour rather than
      falling through to `Empty` — that conflation is what made a broken scene
      and an empty scene the same observation. `load_scene` returns a
      placed-cell count, so "parsed" and "put something in the world" stop being
      the same answer, and `main.cpp` prints it. And the lookup moved out of
      `main.cpp` into `src/scene/`, which links tests; it had been sitting in
      the one file no suite can reach.
    - **The frozen values are the ones already painted into the shipped BMP**,
      so the fix cost no repaint. Verified end-to-end against the real asset:
      27,192 cells placed, 0 unmatched, all keeping their authored colour.
- [x] **A settled pool of water never went to sleep.** A body of liquid only
  slept if its cell count happened to divide by its container's width. One cell
  more and it never did — two chunks awake and ~30 cells changing places every
  step, forever, on water that had visibly finished moving. Almost every real
  puddle lands in that case. The rule: **a lateral move now has to land
  somewhere it can rest or descend from**, so a cell perched on more of its own
  liquid with nowhere to go stays put.
    - **The obvious version of this fix is wrong, and the test asserts flatness
      for that reason.** The same sideways walk that jitters is also how cells
      get off the top of a mound, several cells a step for free. Refuse it
      outright and a poured column settles into a permanent six-cell heap and
      then sleeps holding it. Long-range levelling had been resting on the
      jitter the whole time.
    - **`MAX_PRESSURE_CELLS` 64 → 512**, because `seek_level` had to take over
      that job and its reach was far shorter than it read: breadth-first through
      the *body* rather than along its surface, spending its budget in both
      directions at once, so 64 bought about six columns in a pool five cells
      deep and the heap needed seven. Affordable now for a reason that was not
      true before — settled bodies sleep instead of asking the pressure question
      forever, which is the opposite of the trade the original 64 was chosen
      under.
    - *Verify.* Bracketed on/off/on per `PERFORMANCE.md`: `churning` at `3.7414`
      / `3.7373` / `3.7379` ms with `cascading` flat as the drift control. No
      measurable cost. **This was found by probe, not by the suite** — the one
      test in the area deliberately used a pool with a whole number of rows, and
      said so in a comment.
- [x] **Steam was an undeclared ignition source.** `Steam.spawn_temperature` was
  220, against Wood's ignition point of 120 and Oil's of 90 — so a pocket of
  steam lit wood with no flame anywhere in the world, and since *both* routes
  into steam are boiling and **water dousing a flame**, putting a fire out was a
  way of starting a bigger one. Spawn is now 88, below the coldest ignition
  point in `REACTIONS`.
    - **Conductivity cannot fix this and only temperature can**, which is the
      generalisable part: `heat_flow` has a floor of one unit per step, so any
      gap of two or more transfers in full sooner or later. A conductor hotter
      than an ignition point *is* an ignition source, however slowly it
      conducts.
    - **Now a `static_assert` rather than a remembered constraint.** Nothing may
      spawn hotter than `lowest_ignition_point()` unless it declares a non-zero
      `heat_source`. Fire is the sole exemption and now genuinely is the only
      one — the table comment claiming so had been false for as long as both
      numbers existed. Add a material that spawns hot, or lower an ignition
      point under an existing spawn temperature, and the build stops.
    - **Boiling now absorbs heat instead of creating 120 units per cell out of
      nothing**, which is both what a latent heat of vaporisation does and a bug
      fixed by the same number. Steam's life is exactly the span between its
      spawn and condensing points, so it is ~60 steps rather than ~140;
      condensing point moved 80 → 26 to keep the puff visible. **Two existing
      steam tests were tuned to the old lifetime and were retuned** — flagged
      explicitly, because changing a test to fit new behaviour deserves to be
      visible rather than quiet.
    - **Only reproducible when the steam is confined**, which is why open-air
      tests missed it and why the regression test seals a pocket under a wooden
      ceiling. In open air steam rises away and cools before it does damage.
      Authored terrain is full of the shape that triggers it.
- [x] **Smaller things, each verified rather than assumed.** `scene.cpp` widened
  its size check before multiplying rather than after (the overflow was inside
  the very check meant to reject a bad size). The streaming texture is cleared
  once at creation, since the upload only ever writes the rect the grid covers.
  `pressure_visit` → `scratch_visit`, because `fracture_landing` was quietly its
  second user and the name hid a deliberate sharing behind what looked like a
  single-purpose buffer — the ordering invariant that keeps it safe is now
  stated at the declaration, with a note that a third user is not free.
  `place()`'s silent reset of `piece_tag` and `fall_ticks` is documented, with
  **E7 named as the item that makes it live** (a structural-to-structural
  reaction row would re-weld pieces that had come apart).
    - **One comment was corrected because measurement contradicted it.**
      `step_thermal` claimed "nothing can be off ambient and asleep"; a
      burnt-out world sleeps completely holding 200,009 units against 200,000,
      with the difference stranded in cells one degree off ambient that the dead
      band will never move. Harmless — but the early-out is sound for the
      *symmetry* reason and not for that stronger claim, and future work would
      have leaned on the wrong guarantee.
    - **One comment was deleted because it warned about something that had
      stopped being true.** `main.cpp` said a grid not matching the window's
      proportions renders squashed; F3.3 sized the texture to the viewport and
      made the blit exact. It read as a known defect sitting in correct code.

### <a id="correctness-pass-lessons"></a>What the correctness pass changed about how to work here

*Three things, and they matter more than any individual fix above.*

- **A green suite proved less than it looked like it did.** Both serious bugs
  lived exactly where the tests could not see: one in `main.cpp`, which no suite
  links, and one in a case a test had *deliberately excluded* with a comment
  explaining why. Neither was carelessness — both were reasonable local
  decisions. **The rule that follows: when a test comment explains why a case is
  being skipped, that comment is a bug report.** The pool test said leftover
  cells "slide back and forth across an open surface forever", and that sentence
  was the entire defect, sitting in the repository, written down, for as long as
  the feature existed.
- **The manual checklist is load-bearing and was not run.** README's launch step
  says in as many words to confirm terrain is visible at startup. Running it
  once would have caught the blank world immediately. A checklist that is only
  executed when someone remembers is a checklist that fails silently at exactly
  the moment it is most needed — see **[a scriptable way to drive the
  window](ROADMAP.md#-sandbox--debug-tooling)**, which V2's verify step already wanted for
  a different reason.
- **Two of these bugs were table-only changes with no code touched.** V2 changed
  eight colour values; Steam's ignition problem was two numbers in two tables
  that nobody would think to read together. **Data-driven design moves the
  danger from the code into the relationships between rows**, and those
  relationships have no compiler behind them unless one is written — which is
  what the new `static_assert` in `reaction.h` is, and what the legend's
  distinctness check is. When the next table gains a column, ask what invariant
  now spans two tables and assert it.

---

*The Long Term "Ideal Systems" wish list is in **[VISION.md](VISION.md)** — add
to it freely, but nothing in it starts before the v0.1 slice is done and
playtested.*

*Deferred technical decisions and their reasoning (cell size, RNG cost,
threading, cross-platform status, frame tag wraparound, player/grid interaction,
player feel, the UI layer, material hardness, and the write-path invariant) are
in **[ENGINEERING_NOTES.md](ENGINEERING_NOTES.md)**.*

---

## 📦 `ROADMAP_ITEMS.md`, as it stood when it was merged away

*`W4` merged `ROADMAP_ITEMS.md` into `ROADMAP.md` on 2026-08-17 and deleted the
file. What moved into the live plan was that file's navigation — Next up,
Running order, Decisions owed, Prerequisites — its glossary, its per-track
running orders, its ablation table, and **the opening plain-language paragraph
of every open item**, which now sits inside that item's entry as an "In plain
terms" block.*

*What is below is the rest of it: the sub-bullets under those openers, which
restate arguments the live entries already make in full; the plain-language
restatements of items that are already closed; and that file's own summaries of
the waves and the shipped work, which the sections above this one carry. **The
duplication `W4` exists to remove is exactly this text**, and it is kept rather
than deleted because nothing in this project is deleted — not because anything
needs it.*

# Roadmap Items — the working plan

This file is **the plan**: what is next, how big it is, and what is blocking.
[ROADMAP.md](ROADMAP.md) is the archive of *reasoning* — every item's full
argument, the things that were tried and failed, and the measurements. When the
two disagree about **order**, this file wins; when they disagree about **why**,
that one does.


## 🟤 Engine & Visual Depth — the selling point

Three tracks: **E** deepens the simulation, **V** gives it a visual identity,
**P** pays for both in performance. An item only gets in here if it can name (1)
something in the built game that's actually wrong, and (2) what it unlocks
later.

### E — Simulation depth


- **E1 — Liquids find their level.** ✅ Done. Water in connected containers evens
  out instead of sitting at different heights.
- **E2 — Heat, the seventh axis.** ✅ Done. Every cell has a temperature, and
  things ignite, melt or boil by crossing a threshold rather than by a random
  dice roll.
- **E3 — Collapses break instead of dropping rigid.** ✅ Done. An unsupported
  structure cracks apart along the stress line instead of descending in one
  perfect block like an elevator.

- **E4 — The player displaces material, or deliberately does not.** *(afternoon
  — a decision, possibly no code)* The grid doesn't know the player exists, so
  sand falls straight through the body. This item's output is a *decision*: try
  it in play, and if it isn't obviously better, write down "no" and stop
  thinking about it. If the answer is yes, the implementation waits for E5a,
  which is what gives shoved material somewhere to go.

- **E10 — Powders come to rest.** *(days — new, and the biggest single
  improvement to how the game feels per hour spent)* Sand currently has no
  friction at all: a grain rolls off any edge it can and then takes a second
  fall in the same tick, so piles can't hold a slope, sand behaves like very
  thin water, and a tunnel dug through a dune flattens completely instead of
  partly caving in. This is also why the three failed rules recorded at
  A7/A7b/A7c fought each other — "a rule aimed at motion kept catching rest, and
  a rule that spared rest stopped catching the defect" is an exact description
  of a system with no *rest state* to aim at.
    - **The fix is one number per material, called inertial resistance in the
      game this is measured against.** A settled grain is asleep and stays
      asleep until something disturbs it — a neighbour moved, plus a
      per-material dice roll. Once it's free it keeps sliding until it comes to
      rest again. That gives cones that hold their shape, avalanches that
      trigger and then *stop*, and gravel / sand / snow / ash as four table rows
      instead of four code paths.
    - **It costs no memory** — *and the claim below is narrower than it was when
      it was written; corrected 2026-08-12.* It said the byte is one "that only
      structural cells and Fire ever touch". **`Steam` touches it now too**, as
      of E9's steam half, so the free byte is free on powders and liquids rather
      than on everything that is not structural. That does not change E10's case
      — powders are exactly what E10 is about, and `element.h` still says in
      writing that powders and fluids have no use for a clock — but it does
      change the *count* of things that would have to agree, which is the number
      this bullet was really making an argument about. The `static_assert` it
      cites has also been replaced by a role lookup asserted over every row; see
      `tick_role()`.
    - **It claims that byte for good, and E5a is why the meaning is decided now
      rather than twice.** For a non-structural, non-Fire cell the byte becomes
      a **packed speed**: 4 bits of sideways, 4 bits of up-down, both signed, so
      −8 to +7 cells per tick each way. E10 only ever writes zero or "falling"
      and only ever reads "is this zero" — but the representation is E5a's, so
      E5a is not a rewrite of it. The −8..+7 range is not arbitrary: 8 cells per
      tick is already `MAX_FALL_SPEED` for structures, so the two speed limits
      in the engine agree by construction.
    - **That paragraph is now conditional, and E10 is blocked on it.** The
      2026-08-11 review found the packed-speed representation does not carry
      what E5a needs — no sub-cell fraction means gravity cannot be integrated
      onto a moving cell — and that the byte's *third* role excludes structural
      and Fire cells with nothing saying so. Carried as a [decision
      owed](ROADMAP.md#-decisions-owed), answered at the instrumentation sitting. **E10
      does not write to the byte until it is answered**, which costs E10
      nothing: it is two days behind T1 anyway, and the entire argument for
      deciding the meaning now is that it should not be decided twice. If the
      answer turns out to be a second byte, E10's "it costs no memory" claim
      goes with it and should be struck rather than defended.
    - **The role split needs a compiler behind it, the way the other two do.**
      [element.h](src/physics/element.h) asserts that Fire is not structural
      because the byte means two things. A third meaning wants the same
      treatment — a `constexpr` role lookup per `ElementType` and a
      `static_assert` that every material claims exactly one — and E12's `Crust`
      (structural) turning into `Grit` (powder) is precisely the transition that
      makes it worth having rather than trusting. This is the hazard the
      correctness pass named: the danger lives in the relationships between rows
      and no compiler sees it unless one is written.
    - *Verify:* a poured pile holds a measurable angle instead of flattening; a
      tunnel roof partly collapses instead of fully; the benchmark's `cascading`
      and `churning` numbers do not regress, because a resting grain now does
      *less* work than it did.

- **E5a — Velocity means something.** *(weeks — the first half of the old E5)*
  Nothing in the world has a speed. A grain dug out from under a pile falls at
  exactly the rate of a grain blasted out of it, because movement is a rule
  applied once per tick rather than a speed being integrated. Nothing can be
  thrown, splashed, sprayed or knocked. Three later items stand on this.
    - **It lives on the cell, in the grid — not in a separate list of in-flight
      particles**, and that is a reversal of how this item used to be written. A
      separate list means every rule in the engine needs a second implementation
      or an explicit "no": does a grain in flight conduct heat? react? get lit?
      cast a shadow? The games this is measured against all keep the moving cell
      *in* the grid and have it walk a straight line through the cells it
      crosses each tick, testing each one. One entity, one set of rules, no
      boundary to maintain. A genuinely separate particle list is still the
      right answer for things that must move smoothly between cells and interact
      with nothing at all — which is exactly V9's effects layer, already scoped
      that way.
    - **The note at grid.cpp:743 argues against the wrong thing, and it should
      be edited rather than deleted.** It records that powder acceleration was
      tried and removed for three measured reasons: motion got choppier, a
      continuous stream stratified into sheets, and `cascading` went 13.1 → 19.7
      ms. All three are consequences of applying free-fall acceleration to
      *every falling grain*, and all three go away when speed is only non-zero
      because something *put* it there. Gravity accumulates on cells that are
      already moving, not on every grain in a stream — and since `place()`
      builds a fresh cell, a brush-stamped grain starts at zero like its
      neighbours, which is the whole of the stratification bug. A resting grain
      stays a one-cell-per-tick mover, so the common case is unchanged and the
      frame-time regression does not recur. Choppiness was always a *drawing*
      property of whole-cell movement and cannot be fixed in the simulation at
      all, which the note itself says.
    - ~~**"No new memory" is now a claim under test rather than a property**~~ ✅
      **Priced 2026-08-13 and it is a property again — but not by the route this
      bullet was watching.** The bullet read: *it is E10's byte, and whether
      that byte can hold a usable velocity is the open decision above … whether
      that fits in the byte E10 claims, or costs a thirteenth byte and pulls
      `P1` in front of this item, is unpriced.* **The byte cannot hold it, and
      no byte is spent anyway.** Velocity is three fields in `Element`'s
      alignment hole — signed 4.4 per axis, plus a nibble of sub-cell remainder
      per axis — `sizeof(Element)` stays 12, and `grid_bench` with them present
      is inside its noise band on every row. **The way this bullet was written
      is why it worked**: it named the two ways the claim could go wrong instead
      of asserting the cheap answer, which is exactly what made the question
      answerable in one sitting instead of being found mid-implementation. It
      was also wrong about the shape of the answer, and that is fine — a bullet
      that lists the failure modes does not have to have guessed the outcome.
    - **Four traps, each a known failure of something this engine already
      does.** *Conservation:* a moving cell is still in the grid, so the
      existing conservation test keeps working — which is a real advantage of
      this design over the old one and should be stated in the test. *The wake
      rule:* a cell arriving in a sleeping chunk must wake it, same as every
      other write. *One definition of solid:* what stops a moving cell is
      `is_solid`, the same function the player and the dig ray use. *A ceiling:*
      the 4-bit range is the ceiling, and it makes tunnelling impossible by
      construction rather than by being fast enough.
    - *Verify:* same seed and same input gives byte-identical results (F1's
      invariant extended to the new axis, not assumed to survive it); a cell
      fired at a wall at full speed lands *against* it; a world that has been
      disturbed and then settles goes fully back to sleep.

- **E6 — Explosions.** *(week — nearly free once E5a lands)* Right now the only
  way to change the world is a dig that deletes a fixed sphere on a cooldown.
  There is no force in the engine at all. An explosion is a radius, a falloff, a
  heat deposit, a conversion pass and an impulse handed to E5a — five stages,
  four of which are axes that already exist. It is the single most impressive
  thing this engine can put on a screen, and it is one of the two things that
  make the hook question answerable by playing.
    - **It must not become a second destruction system.** The dig tool is a
      degenerate explosion — a radius with no impulse, heat or falloff. Two
      implementations will drift, and the drift will show up as a bug in
      fracture.
    - **First cost in the engine whose worst case a player chooses.** Cost goes
      as radius squared; it needs a ceiling, and the measurement has to be taken
      *at* the ceiling.
    - **The impulse stage reaches powders, liquids and gases and nothing else,
      and `E12` is how structural material joins in.** E5a's velocity lives in a
      byte that structural and Fire cells already spend on other things, so **an
      explosion cannot hand an impulse to a wall cell or a flame** — wall debris
      goes through E3's fracture-and-fall path, which produces tumbling pieces
      rather than flying grains. That is a real limit on the most impressive
      thing this engine can put on a screen, and it was unstated until the
      2026-08-11 review. **`E12`'s `granulate()` is the mechanism that gets
      round it** — a solid that turns to powder at the moment it is disturbed is
      a solid that can then be thrown — which promotes E12 from *sequenced
      before E6* to a **dependency of E6**. Anyone building E6 against
      structural material without E12 will find this out by watching an
      explosion produce a shrug.

- **E7 — Breadth: more rows, not more code.** *(days per material, ongoing)*
  There are only eight materials and six interactions, so "what happens if I put
  X on Y" almost never has an answer. Ice, snow, acid, gunpowder, molten stone
  and smoke are mostly new table rows rather than new code, thanks to E2.
  Sequenced after E6 so rows aren't authored against half an engine. **The
  bound:** a row earns its place by making an interaction something a player can
  discover and be right about.

- **E11 — The cheap columns heat and fluids are missing.** *(days — new)* Four
  small gaps found reading the tables, grouped because they are all one column
  or one short rule and none of them justifies its own item.
    - **Specific heat.** `conductivity` is currently doing two jobs — how fast
      heat moves *through* a material and how much heat it takes to warm it. One
      extra column separates them, and then water is a genuine heat sink and
      metal is a fast conductor that stays hot. No engine work.
    - **Radiant heat.** Fire only heats what it physically touches, so standing
      next to a bonfire costs nothing. Fine until S0 makes fire able to hurt
      you, at which point it is the difference between a hazard and a trap.
    - **Viscosity.** `spread` (5 for water, 3 for oil) is the only fluid knob
      there is, so honey, tar and lava all have to be the same substance with a
      different number.
    - **Lateral flow is a jump, not a flow** — `step_fluid`'s lateral run swaps
      a cell straight to the far end without touching the cells in between.
      **"No action is proposed" is withdrawn, on session 5.** It was recorded as
      a harmless known property of the genre; the playtest returned it as the
      largest visual complaint on the record, twice in one sitting, the second
      time under a row written in advance to tell it apart from venting. The
      property is unchanged — the *classification* was wrong. Still not a
      column, and still not worth pulling E5b forward for: what replaces the
      entry is a bounded spike, in [ROADMAP.md](ROADMAP.md#e--simulation-depth).

- **E12 — `Crust` and `Grit`: a material that granulates when damaged.** *(days
  — new 2026-08-11)* A solid that holds its shape until it is disturbed, at
  which point the damaged part turns to sand and pours away. **Two table rows,
  not one row with a mode flag** — exactly the `Wood` → `Charred` precedent, so
  the solid gets player collision and rigid collapse and the powder gets piling,
  with no new branch in the update loop. The trigger is one small `granulate()`
  called from three places that already exist: the dig tool's impact, an
  overhang losing its support, and a piece landing hard. A **dice roll per
  disturbed cell** rather than a certainty, so some cells hold and some go and
  the crumble edge is ragged for free.
    - **It is admitted by a commitment, not by an observation, and that is
      stated rather than dressed up.** Nothing in play is wrong for want of it —
      [notes/granulating_enemies.md](notes/granulating_enemies.md) says so
      itself, and also says it *"rides along with whatever admits combat"*. What
      admits it is the commitment to `S1`.
    - **Sequenced after E10 for a specific reason:** a crumbling material that
      cannot hold a slope reads as a liquid, so built before powders have a rest
      state its entire output is a puddle.
    - **It is also a dependency of `E6`, which upgrades it from nice-to-have.**
      Explosions can only hand an impulse to cells that can hold a velocity, and
      structural cells cannot — so `granulate()` is the only route by which a
      solid participates in an explosion as flying matter rather than as a
      falling slab. Argued at E6. **This also weakens the "admitted by a
      commitment, not an observation" caveat below**: E12 now has a scheduled
      consumer that is not `S1` and is not behind the combat decision.
    - **Independently valuable before any enemy exists** — a crust ceiling that
      comes down is a hazard with zero actor code, and it is the cheapest way to
      find out whether the feel is worth what `S1` costs.
    - **The accumulated-damage version is a different item and is after P1.**
      Per-cell hit points buy visible wear and "three hits and it goes", and
      they cost the 500 KB byte `element.h` says is the next one spent. P1 is
      what makes that affordable; building it first buys the same feature at its
      worst price.

- **E5b — The air field.** *(large — the second half of the old E5, and it
  absorbs the item that used to be called "gas pressure")* A second coarse grid
  over the world, one entry per 4x4 block, holding pressure and a velocity.
  **The pattern is already built and shipped:** the lighting is exactly this — a
  low-resolution, whole-number, reproducible grid stretched over the scene with
  one draw call. One system delivers six things that are currently separate
  gaps:
    - gas pressure (steam in a sealed room does nothing today);
    - explosions that *push* rather than delete-and-throw;
    - steam and smoke that drift and curl instead of rising in columns — the
      single thing that most makes a sandbox look alive;
    - wind, which V9's sparks and embers need in order not to look like a
      screensaver;
    - fire suffocating in a sealed space, which E9 currently cannot express;
    - and the pressure term that **retires `find_lower_surface`, `vent_fluid`
      and `make_room_above` together** — three of the four rows in the table
      above.
    - ⚠️ **Priced 2026-08-13, and the performance argument for this item is
      gone.** Those three rules were ablated one at a time in one binary: **all
      three removed is worth 8.3% of the played mean and 4.6% of played p99**,
      on a row already at 0 of 20,415 steps over budget. **8% of 1.2% of a frame
      does not pay for a second grid over the world.** On `churning` it is 47%,
      but session 2 established `churning` is not representative — and the
      breakdown sharpened that: **`churning` and the played session do not even
      agree on which of the three rules is expensive** (venting 47% vs 0.1%;
      `seek_level` 0.3% vs 7.3%). **So this item stands on the six capabilities
      above and on D3/D4, and must not be argued for as an optimisation.** That
      is a real narrowing: "it also makes fluids cheaper" was doing work in this
      entry's case and it cannot any more. Tables in
      [PERFORMANCE.md](PERFORMANCE.md).
    - **Two honest costs, both of which are why this is after the slice.** The
      reference implementation uses decimals, and this engine forbids them in
      the simulation for reproducibility, so it needs a whole-number port —
      doable, and non-negotiable. And it is a *fixed* cost proportional to the
      awake area rather than one that scales with how much is moving, which is a
      different shape from everything else in this engine and needs its own
      PERFORMANCE.md entry rather than a bracketed measurement.
    - **It also buys something the current design has permanently ruled out.**
      `find_lower_surface` levels water by teleporting a cell across the body,
      which is instant and non-local, so waves, sloshing and surges are
      impossible *by construction* — not missing, impossible. Pressure
      propagates over several ticks, so they fall out for free.

- **E8 — Toppling, and rigid bodies properly.** *(large — deferred past v0.1,
  and the reason it is deferred has changed)* Structures currently drop or break
  but never tip over, which reads as lifeless. **This used to be written as "may
  close as not possible without wrecking the pixel art", and that sentence is
  withdrawn** — it closes a door the reference engines walk through, and someone
  reading this in a year would believe it. The objection was that rotating a
  piece resamples it and destroys the authored pixels. The reference answer:
  trace the outline of the connected piece, simplify it to a polygon, hand
  *that* to a rigid-body solver, take the piece's cells out of the grid
  entirely, and stamp them back in each frame from the body's own private copy
  of its pixels. The pixels are never resampled — they live with the body and
  are only *drawn* rotated. Rotation becomes a drawing problem, which is a
  solved one.
    - **The real objection, recorded as the real one:** it is bigger than
      everything else in the E track combined, it wants a physics library (a
      dependency, against the no-bloat rule) or a hand-rolled solver, and
      nothing in the slice needs it. It also *retires* the
      eight-flood-fills-per-tick cost in `resolve_support` and makes fracture
      and explosions act on bodies properly, so it gets better with age rather
      than worse. Not for v0.1.

- **E9 — Fuel, and a clock for steam.** ✅ **Done — the fire half in waves 2b/2c,
  the steam half on 2026-08-12.** Steam has its own condensation clock on
  `Element::ticks`, so a puff lasts without having to be dangerously hot; it
  collects against ceilings and drips. **The rule that works is narrower than
  the one this entry predicted**: only a steam cell *in contact with something
  solid* ages at all, so a pocket's interior waits its turn and drains from the
  top down. A plain countdown was built first and drained a four-deep pocket in
  about a second — it measured as drips and read as a puff. Sealed-pocket drain
  time went from **3 steps to 291**. Full record at [E9 in
  ROADMAP.md](ROADMAP.md#e--simulation-depth); the constants are in
  [TUNING.md](TUNING.md).
    - **Owed a playtest and it is the only thing outstanding.** Checklist step 5
      (reactions and heat) is the step all three original reports — A5, B3 and
      D5 — came out of.
    - **E5b would still give it suffocation**, which is unchanged and is the one
      part of this item's original scope that no longer belongs to it.
    - **It answered part of the `ticks` decision early, in the useful
      direction.** See the decision row below.

### V — Visual identity


**V12–V16 are new on 2026-08-11 and they are one plan.** The goal they serve,
stated once: **mixed pixel resolutions and sprite sizes in one scene, procedural
animation, backgrounds that are animated rather than static, and enemy bodies
that granulate where they are hit.** Four of the five name something in the
built game that is wrong on their own; **V15 is admitted by a condition
[notes/procedural_animation.md](notes/procedural_animation.md) wrote down in
advance** — "a second character type is committed to" — which is a better form
of admission than an observation, because it was specified before the thing that
satisfied it existed. The full arguments are in
[ROADMAP.md](ROADMAP.md#the-visual-system-this-track-is-now-building-toward).

- **V1 / V2 / V5 / V6 / V3 / V3.1 / V7-emissive / V10** — ✅ Done. Transparent
  empty space and a backdrop; a palette tuned against it; the art direction
  written down; one locked palette shared by code and art with a validator; the
  player sprite decoupled from its hitbox and then animated; fire casting light;
  the reticle and material hotbar.

- **V17 — A golden-frame check, before the render path is rewritten.**
  *(afternoon — new 2026-08-11, and it is a harness rather than a visual item)*
  V11 extracts ~350 lines of frame composition out of `main.cpp`, V12 replaces
  the colour-key path with a real alpha channel, and V13 changes every
  destination rectangle on screen. **Nothing today tests a composed frame.**
  That is not a guess: `preview_light` exists because *"existing tests all
  passed the broken frame because none of them ever combined layers"*, and V2
  shipped a blank world for an entire commit.
    - **Built as** a generalisation of `preview_light` — compose a fixed scene
      through the real layer stack, write a PNG, checksum it — covering all the
      layers rather than only lighting. The scene and the checksum go in the
      repo; a deliberate visual change updates the checksum in the same commit,
      which is what makes the diff reviewable.
    - **It is the only thing that will catch V13's silent traps.** V13's own
      entry says the player sheet's two compile-time size checks would, at 2x,
      **"pass while meaning nothing"**. A `static_assert` cannot catch a
      `static_assert` that has stopped meaning anything. A rendered frame can.
    - **The bound:** this checks that the frame did not change by accident. It
      is not an art-direction test and it cannot say a frame looks good.

- **V11 — Make the visual system adaptable.** *(week — new, and it is the item
  that makes changing direction cheap instead of expensive)* The stated
  expectation is that the art direction will change several times. Right now a
  direction change is expensive in five specific, findable places, and none of
  them is hard to fix *today*.
    - **There is no renderer.** About 350 lines of frame composition sit inline
      in `main.cpp`, with the layer order hard-coded: clear → sky → mountains →
      props → world → player → light → reticle → HUD → hotbar → menu.
      `notes/reference_observations.txt` has *already* concluded that a
      mid-ground layer is needed that this stack has no slot for. Extract
      `render/frame.cpp` holding an explicit ordered list of layers. Afternoon;
      afterwards, adding a depth band is one list entry instead of surgery
      between two comments.
    - **Material colours are compile-time constants.** Changing direction means
      editing `MATERIALS`, recompiling, and re-checking the level file's colour
      codes. Give each row a *palette slot* and put slot → colour in a loadable
      theme file. The correctness pass already learned this exact lesson once,
      when it separated level colour codes from the render palette; this is one
      more step of the same move. Then a second biome is a file, and time-of-day
      is two files and a blend. **The cost is bounded and worth stating:** the
      pixel buffer holds baked colours, so switching theme needs one pass over
      the world — a one-off at swap time, not a per-frame cost, and the hot loop
      is untouched.
    - **`Camera::SCALE` is a compile-time constant** and it is baked into the
      reticle size, the sprite offsets and the prop rectangles. A zoomed-out
      biome, a different cell size, and the already-planned resolution options
      all collide with it. Making it a runtime value is far cheaper now than
      after three more systems read it.
    - ✅ **The light layer can only add light**, so every biome will be the same
      brightness. Generalise it from add-only to multiply-and-add — an exposure
      and tint term as well as a glow term — and night, underground, fog and
      per-biome colour grading all arrive without touching a single material
      colour. Given that the reference finding was that the read comes from
      *silhouette layering, not detail*, this is the highest-value visual knob
      available and the one that most makes direction changes cheap. **Shipped
      2026-08-16 as step 3, in an afternoon against a week, and it came out as
      *two* knobs rather than the one this bullet describes.** A per-layer
      `Grade` on each row of the layer table, and a world-wide one as its own
      pass ordered before the additive light. The split is not tidiness: **a
      frame-wide multiply cannot separate two depth bands**, because it scales
      both and leaves the ratio between them untouched — so the "per-biome
      colour grading" this bullet asks for and the band separation the reference
      asked for are different operations and needed different knobs. The
      bullet's prediction that this is the highest-value knob available
      survived: the mountains at 0.60 are the whole of the depth fix, and they
      are one number.
    - **The parallax numbers are duplicated** between `main.cpp` and
      `tools/generate_backdrop.py` with nothing checking they agree; the failure
      is a seam at the pan limit. Generate the header from the tool, exactly as
      V3.1 did for the player sheet.

- **V12 — The asset layer: alpha, and more than one format.** *(days — new, take
  with V11)* Transparency today is one exact shade of magenta, swapped for
  "invisible" when the image loads. **So nothing in the game can have a soft
  edge, a semi-transparent pixel, or fade in and out** — and any drawing that
  happens to use that shade gets holes in it. There is also only one loader,
  called four times, with four matching cleanup lines at shutdown that are
  invisible to forget.
    - **Why it comes before the animation work rather than after:** a colour key
      is all-or-nothing, so a rotated part is a staircase of hard-edged blocks.
      Checking "do rotated limbs look acceptable at this size" against
      colour-keyed art answers a different question, in the wrong direction.
    - **It adds a dependency and that is a deliberate crossing.** `stb_image.h`
      is a single file dropped into the source tree with no build-system entry —
      the cheapest possible form. The standing rule is zero new dependencies
      until a specific need can't be met without one; **the need is alpha and
      BMP genuinely cannot carry it**, which is a different situation from the
      test fixture that made PNG lose to BMP the first time. The crossing gets
      written down where that decision lives.
    - **The trap that would quietly undo V6:** the locked-palette validator only
      understands BMP. Allow the new format everywhere and half the art silently
      stops being checked — which is exactly the state V6 exists to end, and it
      would surface a year later as "the palette drifted" with nothing able to
      say when. **The boundary is a rule:** BMP stays the format for everything
      inside the locked palette, the new format is only for assets deliberately
      outside it, and each of those is listed by name.
    - Also folds the four hand-managed textures into one small cache that owns
      loading and cleanup.

- **V13 — Sprites carry their own resolution.** *(days — new, take with V11)*
  "One pixel in the file is one square in the world" is assumed everywhere and
  enforced nowhere: both the props and the player compute their on-screen size
  straight from the image's size. **So the only way to draw a more detailed
  sprite is to make it a bigger object** — a character drawn at twice the detail
  is a character twice as tall. This is the item that buys mixed pixel
  resolutions and sprite sizes, which is the first of the four goals.
    - **Built as** one extra optional number on each line of
      `assets/sprites.txt` — how many image pixels make up one world square —
      plus which filtering that asset wants. The file already carries an
      optional frame size, so the format's shape doesn't change, and it belongs
      there rather than in code because it is a fact about the *file*, which is
      the thing that gets swapped.
    - **Four traps, three of them silent.** Prop planting scans the terrain
      under the sprite's width, which is a count of world squares and would be
      double at 2x. The player sheet's two compile-time size checks compare
      image pixels against world squares and at 2x would **pass while meaning
      nothing**, which is worse than failing. The sheet-building tool's "exactly
      14 wide" becomes a statement about squares that the tool measures in
      pixels. And the loud one: **terrain cannot participate** — its resolution
      *is* the simulation's, one colour per square — so mixed density has a
      floor and the floor is the world.
    - **Taken with V11 for arithmetic, not tidiness.** V11 makes the global zoom
      a runtime value and this makes the per-asset factor it multiplies; done
      apart, the same rectangles get rewritten twice and the middle version is
      wrong by exactly the factor nobody is tracking.

- **V14 — A part rig: rotation, and attachment from a table.** *(week — new)*
  The figure's only articulation today is *which* of nineteen drawn frames is
  showing. Pull 2–4 parts out of the sheet — an aiming arm, the head, later a
  cape — and drive them with rotations computed per tick. **This is the
  decomposition V3.1 already argued for and already built once**, and the reason
  it was pulled is the reason this version is different: the arm attached at a
  marker pixel in a second image, so every frame drawn afterwards had to carry
  one or the validator refused it. **Here the attachment point is a number in a
  table** in the file that already generates numbers. The tax that killed it is
  a tax the sheet imposes and a table does not.
    - **The one trap, already paid for once:** flipping a sprite mirrors the
      image and *then* rotates it, so the aim angle doesn't mirror with it. The
      obvious correction is right for a target level with the shoulder and
      inverts everywhere else — and both of the cases anyone checks by eye are
      the two it gets right.
    - **It is also the cheap experiment that prices V15**, because nobody has
      yet looked at whether a rotated 3-square limb reads acceptably at this
      scale. That answer is an input to the next item and should come out of
      this one in writing, with a screenshot.
    - The existing sheet checks all survive, because the body is still a sheet.
      **That is what makes this the cheap step.**

- **V15 — A skeletal rig, and feet that find the ground.** *(weeks — new, and
  **moved behind the playtest gate on 2026-08-11**)* A drawn walk cycle assumes
  a flat floor, and **nothing in this game is a flat floor for long** — the
  terrain is per-square and the one verb the game has is destroying it. This is
  feet that land on the terrain that is actually there, and a second character
  for the price of a re-pose instead of a second hand-drawn sheet.
    - **Its admission was circular and that is why it moved.** The trigger
      written down in advance is "a second character type is committed to", and
      the entry cashed that as "one has been (`S1`)". But `S1` says three lines
      into its own entry that it is **blocked on the combat decision, which it
      does not get to pre-empt** — so it is precisely *not* committed to — and
      `S1` in turn lists `V15` as a dependency. **V15 was admitted by S1, S1 was
      blocked on a decision, and S1 needed V15.** If combat comes back "no", the
      trigger evaporates and three weeks of rig work stays scheduled ahead of
      the gate with nothing asking for it.
    - **So V15 is now gated exactly as `S1` is: on the combat decision, due at
      the end of S0.** Nothing is lost by waiting, because the thing that makes
      this item cheap to judge is `V14` — whose stated output is a screenshot
      answering whether a rotated 3-square limb reads at this scale. **The
      decision and the evidence therefore arrive together**, which is a better
      position than the one this item was in.
    - The slope-aware-feet argument is still unproven by any playtest, so it
      shapes this item rather than admitting it — **and it is the one argument
      that could re-admit V15 independently of combat**, if a session says feet
      on broken terrain read badly. That is a legitimate second route in and it
      is named here so it can be used.
    - **It must replace the three sheet checks and nothing currently does.**
      Under a rig there are no frames, so "bottom row empty", "gap inside the
      collision box" and "declared frame is blank" become unexpressible — **and
      every one of them exists because it catches a bug that reads as a physics
      problem rather than an art one.** The rig versions are writable and are
      part of this item, not a follow-up.
    - **Anything with a solver in it inherits the fixed-tick clock and makes it
      stricter.** A spring's stiffness quietly means something different at each
      frame rate — worse than the walk cycle that caught this before, because a
      wrong-speed cycle is visible and a wrong-stiffness limb just looks
      slightly bad.
    - **Physics-driven limbs (the Rain World model) are still not scheduled.**
      Highest cost on the page, nothing observed asking for it, and no longer
      blocked by anything — held on price, which is a deferral rather than a
      refusal.

- **V19 — The seven-band scene, and a ground plane where the reference has
  water.** *(week — new 2026-08-16, admitted by request, and it is item 8's step
  4)* Build a scene composed the way `CnC_parallax_*` is composed, before the
  split-view path rather than after it. **The reference frame is seven layers
  and we ship two**; measured in
  [notes/reference_observations.txt](notes/reference_observations.txt) entry 7.
    - **Five new bands, not seven**, because two of the seven rows are already
      filled: sky and the mid range are `backdrop_sky` and `backdrop_mountains`,
      and the near silhouette is the simulated world. **A painted band in front
      of the world is refused** — the reference's foreground rock is a painting
      the boat passes behind and ours is diggable terrain.
    - **Art is one colour and one or two shades per band, generated by
      `tools/generate_backdrop.py`**, which is a scope decision rather than a
      placeholder: generated layers conform to `PALETTE` by construction, so
      nothing here needs a drawn asset and nothing here waits on V12.
    - **The land plane is the item's centre of gravity and it needs per-row
      parallax.** A receding plane has no single depth so it has no single
      factor; drawn flat it reads as a wall. Scroll factor is linear in distance
      below the horizon, built as N strips of one `SDL_RenderCopy` each, with
      the source row height shrinking so the texture gradient falls out of the
      same relation. **No custom blend mode and no render target** — the second
      item in a row to have looked like it would spend an escape hatch and not
      spend one.
    - **Two traps, both able to waste the item, both written up in ROADMAP.md**:
      the near ridge and treeline land in the band V11 deleted because our
      terrain fills it — fire that reopen trigger by screenshot first — and five
      pan-sized layers roughly triples `assets/`, which may pull V16 forward
      into this item.
    - **One decision it must not make quietly:** the reference's plane is
      brighter than what stands on it, and our world row is at grade 1.0.
      Grading the world down is coherent and is a change to how the play area
      reads while digging.

- **V16 — The backdrop moves.** *(week — new)* The two backdrop layers are
  static images whose only motion is the parallax shift, so **the backdrop's
  sole depth cue stops the moment the player stands still.** A wooded hillside
  at night is motionless: nothing drifts, nothing sways, no star varies. That is
  something in the built game that reads badly *today*, which is the question
  V8's remainder cannot answer — which is why this is a separate item from it
  rather than part of it.
    - **Built on V11's layer list**, which is why that comes first: a layer
      gains an optional motion driven by the tick count, and stays one draw call
      (two if it wraps). No new coordinate system.
    - **Driven by the simulation tick, not the display refresh** — the third
      item in this document to need that sentence, and it presents as an art
      problem every time.
    - **A wrapping layer retires the parallax-seam problem instead of
      documenting it.** The seam exists because each layer must be sized to the
      camera's pan range at its own factor, from constants duplicated in two
      files. A layer that tiles has no size relationship to the pan range at
      all. V11's generated header is the fix available today; this is the
      version where the failure can't occur.
    - **The bound:** backdrop motion is drawn, interacts with nothing, and is
      not V9. Weather that lands on terrain is a simulation feature and is not
      this item.

- **V9 — A non-simulated effects layer, and impact feel.** *(week)* Sparks,
  embers, dust and smoke wisps drawn on top, interacting with nothing, from a
  fixed-size pool so a big event can't tank the frame rate. Sequenced after E6
  because explosions are what it exists to dress.

- **V10.1 — Screen shake and hit-stop.** *(days — alongside E6)* Camera shake
  and a brief freeze on impact make explosions read as force. The trap: driven
  by the fixed simulation clock, not the display refresh, or their speed changes
  with frame rate.

- **V4 — Props at more than one depth, and `Snow`.** *(days)* The prop format
  shipped. `Snow` is now an E7 row rather than a V item, since heat makes it
  melt by table.

- **V8 — The backdrop: a second biome, time-of-day, a third depth layer.**
  *(weeks)* **Held deliberately.** Sky, mountains and parallax ship. The rest
  cannot currently name anything in the built game that reads badly, so it would
  be reference-driven breadth wearing engine clothes. V11 is the piece of this
  that *is* real, and it is scheduled above. Revisit when a second biome
  actually needs to exist.

- **V7-rest — Lighting that darkens.** *(days — smaller than it was)* Today it
  only lights *hot* things and only ever adds. The darkening half is folded into
  V11's tint layer; what stays here is non-fire light sources.
    - **The darkening half shipped 2026-08-16** with V11's grade (block step 3),
      so **this item's name is now wrong and is left as the item ID rather than
      renamed** — four documents cite `V7-rest` and IDs here are stable. What is
      left is exactly the second sentence: light sources that are not fire. The
      estimate drops from a week to days because the multiply was the expensive
      half and it is done.
    - **The step-3 note in the block above records a trap this item inherits:**
      the grade pass is ordered *before* the additive light on purpose, so
      anything added here adds on top of the grade and is not dimmed by it. That
      is the correct behaviour for a lamp in a dark cave, and it is the thing to
      check first if a new source ever looks like it is "ignoring the lighting".

### P — Performance


**The frame-budget rule now has teeth, and this is the change the review pressed
hardest on.** The rule was "if one item alone breaks the frame budget, P1 gets
pulled forward", and P2 broke it in a way the rule could not see: two rows were
already over at the played size with nothing having got slower. Meanwhile **six
scheduled items — E10, E12, E5a, E6, E7, E11 — each add per-awake-cell work or
increase how many cells are awake**, in an engine P2 proved pays for awake cells
and nothing else, and both performance items sat behind the playtest gate. That
is a plan to add every cost first and measure last, and its failure mode is
arriving at the gate at twenty frames a second with six candidate causes.

**The replacement rule, as restated 2026-08-13 once `P4`'s row had a number in
it. Every E-track item takes a benchmark reading at merge, and the reading has
two halves that answer two different questions:**

- **Is the budget broken?** Read `P4`'s replayed row, and read **p99 and
  steps-over-budget** — never the mean. A session that sleeps through 95% of its
  steps and stutters through the rest has an excellent mean. **A single step
  over 16.67 ms that was not over before does not merge**, whatever the
  percentages say.
- **Does the change cost anything at all?** Read the synthetic rows at
  1920x1080, bracketed on/off/on in one binary as always. A regression does not
  merge until it is under 10% or the cost is written into `PERFORMANCE.md` with
  an argument for why it is worth paying.

**Both halves are required, and the second one is here because the first cannot
see small costs.** The rule originally read "under 10%, against `P4`'s row", on
the reasoning that a budget is only as honest as the world it is measured in —
which is right, and is why the first half is stated against that row. What it
did not anticipate is that a played session costs **0.12 ms a step**, so 10% of
it is twelve microseconds: under the noise floor, on a row with three times the
budget spare. The played row proves the game is inside its budget; it is far too
quiet to price a per-awake-cell change. That correction is recorded in the
[decisions table](ROADMAP.md#-decisions-owed) and in `PERFORMANCE.md` rather than
replacing the original wording.

- ✅ **P2 — Re-baseline the benchmark at the size the game actually runs.**
  *(done 2026-08-10)* The benchmark measured 960x540 and the game runs
  1920x1080, so every budget on record was quoted against a world a quarter the
  real size. Both sizes now run, with 960x540 kept as the historical series and
  as a control on the refactor. **Headline: `sparse` is 1.00x at four times the
  cells** — the engine pays for awake cells, not for cells. `churning` (211%)
  and `cascading` (241%) are over budget at the played size and always were;
  nothing got slower. Full entry in [ROADMAP.md](ROADMAP.md#p--performance),
  table in [PERFORMANCE.md](PERFORMANCE.md).
- ✅ **P4 — A benchmark scenario that is a real frame.** *(a day — new
  2026-08-11, **done 2026-08-13**)* **Instrument shipped and first session
  recorded and measured the same day: 24,437 steps, 407 s of play, mean 0.1212
  ms/step, p99 1.4745 ms, worst step 4.8193 ms, 0 steps over budget, replayed to
  the recorded end state exactly.** It closed the frame-budget decision — and
  corrected half of what this section expected that decision to be, since a row
  this quiet cannot price a per-cell change; see the [decisions
  table](ROADMAP.md#-decisions-owed) and [PERFORMANCE.md](PERFORMANCE.md). **The row now
  also prints a `contents` census** — inputs counted exactly, world materials
  sampled once a second in a second untimed pass — built immediately afterwards
  because the timing alone could not be interpreted. It reported that the first
  session contained **no digging, no moving sand, no moving water and no
  steam**, at a peak of 16 of 510 chunks awake. **So the item's instrument is
  complete and its evidence is one unrepresentative session**; a second one is
  owed. Every one of the seven benchmark scenarios is hand-built, and the plan
  has now twice had to argue about which of them counts as realistic — most
  recently over whether `churning` at 211% of a frame is a problem or an
  artifact. **That argument is unwinnable by construction and it does not have
  to be had**, because F2.3 already made a run *a seed plus a replayable list of
  inputs*, and [test_run.cpp](tests/test_run.cpp) already proves a recorded
  sequence replays byte-identically.
    - **Built as** a recorded input log from a real playtest session, replayed
      by the benchmark as an eighth scenario. It is a played frame by
      construction, not by assertion, and it is the row the frame-budget rule
      above is aimed at. **Recording is always on and `F9` writes what has been
      played so far** — see
      [README](README.md#the-replayed-row-and-recording-one-p4). A recording
      that *started* on a keypress was the obvious design and does not work: a
      log has to begin at a world the replay can rebuild, and the only such
      world is the fixture scene before the first step.
    - **What it owed and who owed it: a played session — delivered 2026-08-13.**
      ~~It settles two open questions on its own~~ — it settled nothing until
      someone played and pressed `F9`, and **the instrument could not be run by
      whoever built it**, the same way the manual checklist cannot. That held
      for exactly as long as it took to hand it to someone at the keyboard, and
      it is worth keeping as the shape of the thing: the item read "shipped" for
      a day while the only output it existed to produce did not exist. **Both
      questions are now answered** — see the [decisions table](ROADMAP.md#-decisions-owed)
      — and the answer to the second one ("is `churning` representative?") came
      back as *representative of what*, which is not one of the two answers this
      entry expected.
    - **The trap was real and is closed by construction rather than by a note.**
      The log has to be re-recorded whenever the fixture scene or the format
      changes, or it replays into a world that no longer matches and silently
      measures nothing. The log carries the scene's cell count and a fingerprint
      of the whole world before the first step, and the bench **refuses to run**
      on a mismatch instead of producing a plausible number. A changed
      *simulation* also moves the end state and is not a failure — that one is
      reported and left to the reader, because the bench genuinely cannot tell
      it from a stale log. `scene_test` pins the fixture's cell count so the
      staleness shows up in `ctest`, on the commit that caused it, rather than
      in a benchmark nobody runs that day.
    - **It cost one thing that was not in the estimate, and the cost is an
      improvement:** the scene loader was 90 lines of `SDL_LoadBMP` inside
      `main.cpp`, so nothing headless could stamp the world the game plays in.
      It is now `src/scene/bmp.cpp`, and `main.cpp` calls it — **one reader, not
      a headless one beside the SDL one**, which is the D1/F6 lesson (one
      question, two implementations, drift) applied before the drift rather than
      after. Verified by the count `README`'s launch check already pinned:
      334,901 cells, unchanged, and independently reproduced by
      `tools/pixel_art.py`'s reader.

- **P1 — Split the cell array hot from cold.** *(week)* The simulation is
  limited by how fast data can be pulled from RAM. Most of each cell's data is
  read every tick; the colour is only read when the cell is drawn. Separating
  them means the hot loop reads less. **Sequenced directly after E5a** so the
  layout is settled against the final field set. ~~*and possibly before it, if
  the `ticks` decision comes back needing a thirteenth byte*~~ — **that trigger
  is retired: the decision closed on 2026-08-13 needing no new byte at all, so
  P1 stays where it is.**
    - **One finding from that sitting lands on this item and it is not a
      comfortable one.** Pricing the rejected alternative meant benchmarking a
      deliberately widened `Element`, and the 16-byte struct came out **10.5%
      faster** on `cascading` than the 12-byte one, against a 0.7% noise band —
      bracketed, with a return to baseline 0.03% off the first reading. A struct
      carrying 33% more memory made the memory-bound scenario faster. **This
      item's premise is that the hot loop is bandwidth-bound, and the one
      measurement on record that speaks to stride width points the other way.**
      It does not refute P1 — reading *less* per cell and reading it at a
      *worse* stride are different things, and the likeliest explanation is that
      a 12-byte stride straddles cache lines where 16 divides them evenly, which
      is a fact about alignment rather than about volume. But P1 is now an item
      with an unexplained measurement standing next to its central claim, and
      **the first hour of it should be spent reproducing that number rather than
      writing a split.** Numbers and method in [PERFORMANCE.md](PERFORMANCE.md).
      *(Also worth carrying: "a thirteenth byte" was never an available option —
      alignment rounds 13 to 16, so appending to this struct costs four bytes
      per cell, not one.)*
- **P3 — Run the chunks in parallel.** *(weeks — new)* The whole simulation is
  single-threaded. The reference engine updates chunks in four alternating
  passes arranged so that two chunks being updated at the same time are never
  neighbours, which makes edge writes safe and keeps the result reproducible
  because the pass order is fixed. **The 64x64 chunks and dirty rectangles here
  are already most of the prerequisite.** Worth naming next to P1 for scale: P1
  buys tens of percent, this buys a multiple.

---

## 🟡 The slice — the actual game

**S0 is pulled forward out of this section; everything else stays behind Engine
& Visual Depth.**

- **S0 — The run can be lost.** *(week — new, and it is item 4 in the plan)*
  Today fire is fully simulated and cannot hurt you, there is no way to fail,
  and there is nothing to do. This is the thin version of two items below, built
  now rather than in two months.
    - **What it is:** player health; damage from fire (the engine already
      supplies the temperature) and from landing too fast (it already supplies
      the speed); one hard-coded objective somewhere in the test scene; reaching
      it ends the run as a win, dying ends it as a loss. Death reuses
      `Run::reset(seed)` — it must not be a second code path.
    - **What it is not:** no generator, no save file, no pet agent, no economy,
      no UI beyond a health readout on the existing HUD. Those stay in the full
      items below.
    - **Why now:** it converts the sandbox into something that can be won and
      lost, which is the smallest possible thing that produces a *direction*. It
      also answers two of the open [Decisions](ROADMAP.md#-decisions-owed) — combat, and
      the hook — by playing rather than by argument, and both are currently
      blocking this whole section. Since 2026-08-11 the combat row also carries
      **what to do if the answer is yes**, which it did not, and that was the
      difference between a decision and a deferral with a date on it.
    - **~~Blocked on `F5`, added 2026-08-11.~~ Unblocked 2026-08-12.** Damage
      from landing too fast reads `Player::velocity_y()`, which was a `float`
      and is now `fx` cells per second. Write the threshold against
      `fx::from_int(n)` or against `MAX_FALL_SPEED`, never against a bare number
      — a raw `400` is 400/65536 of a cell per second and would fire on the
      first step of every fall.
    - **Keep the direction of the dependency**, which is the rule `tool.cpp`
      established: the grid does not know about bodies, bodies read the grid.
      Damage is the player *asking* what it is standing in, the same way
      collision does. Not a health field on `Grid`, not a damage column on
      `Element`.

- **S1 — The enemy that granulates.** *(weeks — new 2026-08-11)* **Blocked on
  the combat decision, which is due at the end of S0**, and it does not get to
  pre-empt it. An enemy whose body is a solid material that holds its shape
  until it is damaged, at which point the damaged part turns to sand and pours
  away — shoot its leg and its leg runs out onto the floor.
    - **Built as** a body like the player's, plus a small **per-enemy** grid of
      bytes saying which parts of it are still there — about 364 bytes for a
      player-sized enemy, so it costs nothing per world square. Alive, the
      sprite is drawn through that grid and damage both clears bits and **spawns
      real simulated sand** at those world positions, which piles, gets wet and
      burns like any other sand. Dead, the body writes whatever is left of
      itself into the world as real solid cells — **and from that instant it is
      terrain, so the existing collapse system drops and shatters the corpse for
      free.**
    - **This item reopens E4, and that is the most important line here.** E4 —
      "does a body push material out of the way" — was closed **no** on
      2026-08-10, on evidence from a session where **nothing in the game
      depended on the answer**. An enemy that sheds sand out of itself is
      exactly the case the design note predicted would fail: *the sand falls
      straight through the enemy producing it*, so the effect breaks precisely
      at the moment it exists to be looked at. Carried as a [decision
      owed](ROADMAP.md#-decisions-owed) with a due date rather than buried here.
    - **Depends on:** E12 (the material), V12 (a real alpha channel to punch
      holes in), V15 (a rig, which is what makes a second character cheap —
      build it against a second hand-drawn sheet and V15's whole argument is
      forfeited), and E5a (so shed sand is *thrown* rather than merely
      appearing).
    - **The V15 dependency was circular and is now broken in one direction,
      2026-08-11.** V15's admission trigger was "a second character type is
      committed to", cashed as `S1` — while `S1` says here that it is blocked
      and does not get to pre-empt the combat decision. **A blocked item cannot
      be the commitment that admits its own dependency.** Both are now behind
      the same gate: the combat decision at the end of S0, and if that is yes,
      the first enemy is a two-day ugly one rather than this. `S1` is the payoff
      and stays where it is.
    - **Its one new rendering requirement is a render-to-texture pass**, which
      SDL already supports and this project has never used. No shader needed.
    - **What it is not: a general actor framework.** The standing refusal of an
      entity/component system is on the grounds that "there is one body, and
      there will be perhaps four things" — **a second concrete body type is
      inside that budget; a system for arbitrary body types is the thing being
      refused.**

- **Quantum Worlds.** *(weeks)* A portal / level-generation system so the player
  can enter a single "trial". One world, one generator; variety comes later.
  Generation must draw only from the world-gen random streams F1.5 reserved, or
  adding one cave silently changes how sand falls elsewhere.
- **Player health and death — the full version.** *(days on top of S0)*
  Everything S0 stubbed: a proper damage model, more than two sources, and a
  real death presentation.
- **Objective + Extraction — the full version.** *(week)* A real objective type
  placed by the generator rather than hard-coded, and an extraction that is a
  place rather than a flag.
- **Save and persistence.** *(week)* Nothing writes a file yet, but "earns coins
  idly" requires progress to survive quitting. Needs a format, a location, and a
  deliberate answer for loading saves from older builds. **Check E5a before
  assuming F1's arithmetic still holds** — cells now carry speed, which is state
  a save has to include.
- **The Pet ML Agent.** *(weeks)* The companion that "watches" the player and
  improves from completed runs. Deterministic scripted progression, not actual
  machine learning.
- **Proof-of-Work Economy.** *(weeks)* The idle loop — the agent performs tasks
  between runs and earns coins.
- **Playtest gate.** Put the slice in front of people who didn't build it.
  Explicitly: do not proceed past this line on your own opinion. The two
  decisions above should have been *asked* before getting here, not answered by
  getting here.
    - **As written before 2026-08-11 this gate could not be run at all, and the
      deadlock was in the rules rather than in anyone's judgement.** The gate
      needs a build someone else can execute. **Packaging and a release build**
      lives in Presentation & Tooling, and [VISION.md](VISION.md) says nothing
      in that section starts until the slice is complete *and playtested as
      fun*. So the gate required packaging, packaging required the gate, and the
      loop closed silently because neither document mentions the other.
    - **Three items are therefore promoted out of Presentation & Tooling and
      named as gate prerequisites**, with the reason written where they land —
      the same move this project has now made three times, for F4, for Engine &
      Visual Depth, and for `T1`:
        - **Packaging, a named artifact and a version string.** There is
          currently no answer to "how does someone who isn't you run this".
        - **One verified build on a machine that is not the dev machine.**
          Cross-platform has been *claimed* for several revisions on the
          strength of using no platform-specific code. It is an expectation, not
          a fact, and `F5` gives it a second job: it is also the only thing that
          can verify determinism is portable.
        - **A minimal log and an assertion handler.** Nothing logs anything and
          there is one unexplained crash from a prior session never reproduced.
          An external session that crashes and produces nothing is a wasted
          session, and there are only so many first impressions.
    - **About a week for all three, and it can sit late** — it has to precede
      the gate and nothing else depends on it. What it must not do is remain
      invisible until the week the gate is supposed to happen.

---

## 🔵 Presentation & Tooling (after the slice, before polish)

Not blocking the slice. None of this makes the game fun; it makes it shippable
and easier to work on.

### Sandbox / debug tooling

**Four of these moved out on 2026-08-11 and are now
[`T1`](ROADMAP.md#-prerequisites--things-other-items-are-already-standing-on), scheduled
in front of E10.** The world reset hotkey, pause and single-step, the free
camera and the cell inspector are prerequisites for items already on the plan,
not conveniences — this section said so about two of them and then kept all four
behind the slice. Two remain here, and they remain because they genuinely are
conveniences:

- **Continuous brush strokes.** The brush stamps once per tick at the cursor, so
  a fast drag leaves a dotted line. Stamp along the line between the last
  position and the current one.
- **Brush outline preview.** A hollow ring at the brush's size under the cursor,
  so the footprint is visible before committing paint.

### Window and display

- **Display modes: fullscreen, borderless, windowed.** Exclusive vs. borderless
  as distinct options, alt-tab that doesn't break the window, the choice
  remembered between launches, and changing mode mid-game.
- **Resolution options (1920x1080, 2560x1440, 3440x1440).** The hard part is
  solved. What's left is the mode list, saving the choice, and one fairness
  question: an ultrawide sees more of the world, which is a gameplay decision
  rather than a display setting. **Overlaps V11's runtime scale** — take them
  together.

### Art

- **Custom pixel art and animation generator.** An in-project tool for authoring
  sprites. Flagged as a risk — a second application with its own UI and file
  format — and only earns its keep once hand-authoring is measurably the
  bottleneck.

### Shipping

- **Packaging and a release build.** ⬆️ **Promoted 2026-08-11 to a [playtest
  gate](#-the-slice--the-actual-game) prerequisite.** There's no answer today to
  "how does someone who isn't you run this". Needs a named artifact, a version
  string, and a definition of what a release contains.
- **Build on macOS and Linux at least once.** ⬆️ **Promoted 2026-08-11 to a gate
  prerequisite.** Cross-platform has been claimed for several revisions on the
  strength of using no platform-specific code — a reasonable expectation, not a
  verified fact. **After `F5` it has a second job:** it is the only thing that
  can verify determinism survives a different machine, which is the whole reason
  F5 exists. *(F5 shipped 2026-08-12; `F6` closed `DigTool::march`'s floats on
  2026-08-13, so **the precondition this entry carried is met and the item is
  ready to run**. `src/physics/` contains no float, so a "no" from this item
  would now be new information rather than a known one. That is the whole of
  what F6 bought: this item is still the only thing that can turn a reasonable
  expectation into a verified fact.)*
- **Crash diagnosis: assertions and a log.** ⬆️ **Promoted 2026-08-11 to a gate
  prerequisite** — an external session that crashes and produces nothing is a
  wasted session. Nothing logs anything, and there's one unexplained crash from
  a prior session never reproduced. Since a run is a seed plus an input log, a
  crash report could be a file that reproduces the crash rather than a
  description of it — **and that is only true after `F5`**, because a float
  replay does not reproduce on someone else's machine. *(F5 shipped 2026-08-12
  and `F6` followed on 2026-08-13, so a replay that includes digging is now
  integer arithmetic end to end — reproducible in principle on another machine,
  and unverified on one until the entry above runs.)*
- **Audio.** No sound of any kind. Correctly deferred — but an explosion is the
  one thing that's actively worse silent than absent, so revisit at E6.
- **Settle on one project name.** The README, the window title and the repo
  folder all say different things. Trivial now, not trivial once something is
  published.

---

## 🌊 Waves — sub-plans that jump the queue

A **wave** is a batch of urgent fixes (usually from a playtest) that gets worked
to completion before normal roadmap work resumes. Each one must state up front
what counts as "finished".

| Wave | What it covers | State |
|---|---|---|
| **Wave 1** | Rendering, brush and powder bugs found in playtest session 1. | closed |
| **Wave 2** | First attempt at fire fuel and burn duration. | replaced by 2b |
| **Wave 2b** | Fire simulation rebuilt from scratch, plus glow lighting pulled forward. | closed |
| **Wave 2c** | Tuning the glow's shape/reach and the fire's timing and shape. | closed |
| **Wave 3** | Brush destroying water instead of pushing it aside, and the "water elevator" bug that hid behind it. | **closed** — session 5 confirmed the burst gone and the residue invisible |
| **Wave 4** | Session 5's four non-water defects — the dig swing, the unstuck search pushing the player through walls, the walk cycle, the step height — **plus D3, added 2026-08-11**. | ✅ **closed 2026-08-12.** D2 and D1 shipped 2026-08-11, D3, D6 and D7 on 2026-08-12; the last two were accepted on a look |

### Wave 1 — rendering, brush and powder defects

- **A1 — The player rectangle jittered and ghosted.** The character stuttered
  while moving. Three separate causes: position was rounded to whole squares,
  the display refreshed faster than the physics updated, and the camera could
  only sit on whole squares so the whole *world* jerked instead.
- **A1b — Fixing the display revealed a hidden physics bug.** Landing on the
  ground reset the player's speed but not the leftover fraction of movement
  still queued, so the body slowly sank and snapped back. Invisible before
  because tests only checked whole squares.
- **A2 — The on-screen readout lagged material switches by up to a second.** The
  text was only rebuilt once per second, so it showed stale information. Fixed
  first because a wrong readout makes every other measurement untrustworthy.
- **A7 — Falling liquid threw out horizontal sticks.** Water was spending its
  "how far can I spread sideways" budget crossing open air instead of flowing
  along a surface.
- **A7b — The real complaint was sand, not water.** Sand cascades diagonally
  within a single update, so a whole row lands one step ahead of the row beneath
  it, leaving a shelf sticking out of the pile. Screenshots are what identified
  this.
- **A7c — Shelves and vertical columns are two failures of one rule.** Fixes
  that removed the shelves made piles grow straight up, and vice versa. Solved
  by letting a rolling grain finish its fall in the same step, so a shelf never
  exists even for a moment. **E10 is the proper fix and supersedes this** — all
  three of these fought the absence of a rest state.
- **Powder acceleration — added then removed.** Making grains speed up as they
  fall was meant to look smoother; because grains move in whole squares it just
  made each jump twice as big, which looked *more* choppy, and it cost a lot of
  performance. **See E5a** — the measurement was good and the conclusion drawn
  from it was too broad.
- **A8 — Moving material painted black trails.** Empty space had two conflicting
  colour definitions, so anything that moved left opaque black behind it,
  erasing the artwork underneath.
- **B1 — The dig marker was hard to see.** It was a filled orange square, the
  same colour family as fire — so it disappeared against the thing you most want
  to aim at. Shipped as part of V10.

### Waves 2 and 2b — fire rebuilt

- **C1 — Wood burned too fast.** Turned out to be about how quickly fire
  *spreads*, not how long it burns. Fixed by tuning wood's ignition temperature
  and how well it conducts heat.
- **C2 — Flames rose too fast.** Fire moved exactly one square per tick; the fix
  was to occasionally skip a tick, since a square is the smallest move possible.
- **C3 — Burnt wood was jet black.** Near-black over a dark blue background
  reads as a hole in the world rather than as charred material.
- **C4 — Burnt wood needed to last slightly longer.** The setting wasn't
  fine-grained enough to express the wanted value, so its precision was
  increased. Lesson: a tuning dial's precision is set by the smallest change
  anyone will want, not the largest value it holds.
- **C5 / C6 — Flames all died at exactly the same height.** Every flame lived
  the same number of ticks and rose at the same rate, producing a perfectly flat
  line across the top of a fire. Fixed by randomising each flame's lifespan.
- **C7 — Flame colours lacked intensity.** The colour blend from white-hot to
  dull red passed through greys, so the most visible part of a flame's life was
  its least colourful. Fixed by bending the blend through a saturated orange.
- **C8 — The lighting was blown out (too bright).** Several causes: light faded
  far too slowly over distance, one stray flame lit as brightly as a wall of
  fire, there was no brightness ceiling handling, and the glow spread in a
  diamond shape instead of a circle.
- **`preview_light` — a new visual test tool.** Builds a burning scene, combines
  all the layers exactly like the real game does, and saves the image plus
  brightness statistics. Existing tests all passed the broken frame because none
  of them ever combined layers.

### Wave 2c — the glow's shape, and the fire's timing

- **B9a — Hard light rays and shafts.** The glow bulged along the
  up/down/left/right axes because diagonal distance was calculated slightly
  wrong and the error compounded. Fixed with exact diagonal maths, extra
  "knight's move" light steps, and a smoothing pass that can't leak light
  through walls.
- **B9b — Light reached too far.** Small reduction to how much light survives
  each square of open air.
- **B9c — A single stray flame lit as brightly as a bonfire.** Halved the
  minimum brightness a mostly-empty area emits, so an ember and a blaze are
  clearly different.
- **B9d — Thin walls let light through.** Two stacked modelling mistakes: light
  blocking was being averaged when it should be multiplied, and wall opacity was
  measured per 4-cell block rather than per cell. A one-cell wall now blocks
  light properly.
- **A9a–A9d — Fire timing and shape, tuned as one change.** Fire spreads slower,
  wood takes longer to be consumed, char lasts longer, and the burn front is
  more ragged. Achieved by lowering wood's heat conduction and randomising each
  cell's ignition temperature slightly downward only (randomising both ways
  could make a fire refuse to cross a stick).

### Wave 3 — the brush destroyed water

- **A6 — Spawning material into water deleted the water.** The brush overwrote
  whatever was there, so dragging sand through a pool destroyed hundreds of
  cells of water, then "burst" when released. Fixed with a dedicated brush write
  path that pushes the occupant upward instead of erasing it.
- **A6b — The water elevator.** Falling sand kept swapping places with the water
  beneath it, and each swap lifts water one square — under a continuous stream
  this hands water all the way up the column. Fixed by sending displaced water
  sideways to its own surface instead of straight up.
- **`VENT_RADIUS`** — how far displaced water searches for somewhere to go.
  Measured at several values; ~~3 gave the best quality-for-cost~~ **3 gives the
  best quality; the cost curve it was picked on turned out to be flat.** **E5b
  retires this rule entirely.**
    - ✅ **Re-swept in one binary 2026-08-13 and the sub-item below is answered,
      though not in its own terms.** It asked what venting's share of the played
      size is: **at 1920x1080 `churning` costs 26.06 ms/step with venting off
      and 49.41 with it at r=3**, so venting is **23.35 ms of that row**, not
      the ~1.8 ms the 960x540 sweep implied. The same span re-measured at
      960x540 is **+3.09 ms**, so even the small world's own figure was
      understated by 72% — the old number was wrong before world size entered
      into it. *(Do not read 3.09 against 23.35 as a scaling factor: `churning`
      settles inside its window at 960x540 and does not at 1920x1080, so part of
      that ratio is how much of each window was spent asleep. The bench says so
      at `run`.)* **On the recorded session the whole r=0-to-r=4 span is worth
      about 1% of the mean.** The two readings are not in conflict; they are the
      two halves of the frame-budget rule, and which one governs depends on how
      much powder-into-fluid work a real world does. Full table in
      [PERFORMANCE.md](PERFORMANCE.md).
    - **The sweep behind that choice is quoted at 960x540 and P2 has made it
      unverified rather than wrong.** `churning` was 3.13 ms/step without
      venting and 4.93 with, so venting cost +1.80 ms against a 16.67 ms budget
      and "affordable" followed. At the played 1920x1080 `churning` is 35.25
      ms/step, and whether venting's share is still ~1.8 ms or has scaled with
      the rest is not known. **It cannot be closed by a playtest** — session 5's
      W-7 says so in as many words — **and it must not be closed by rebuilding
      with the constant changed**, which is the cross-build method
      `PERFORMANCE.md`'s E1 entry records as having produced a confident 28%
      that was entirely code layout. It needs a runtime toggle, the way E1 and
      E2 had one. Small, and worth doing before E5b is scoped, since E5b's case
      rests partly on retiring a rule whose cost at the played size nobody has
      measured.

### Wave 4 — the defects session 5 found on the way past

**Finished when:** the player cannot be pushed through a wall, the dig swing
repeats while the button is held, `water_probe` asserts D3's invariant, and
someone at a keyboard says the walk speed and step height are right. Not when
the tests are green.

**Four of the five came from README's nine-step checklist** — the pass the
session was *not* booked to run. It was booked to close wave 3 and answer E4,
and the routine checklist produced four of the session's six defects. That is
the argument for never skipping it.

**D3 was added on 2026-08-11 and the way it was missed is worth more than the
defect.** Every other defect this session produced is cited somewhere in this
file — D1, D2, D4, D5, D6, D7, D8 and D9 all appear, and D8 appears specifically
to record that it has *no* owner and why. **D3 appeared nowhere at all.** It is
not that it was judged and deferred; it fell out between the log and the plan,
and it fell out despite being the most actionable finding of the six —
[PLAYTEST_LOG.md](PLAYTEST_LOG.md) had already stated its invariant in
assertable form and already named the tool that measures it. **The process fix,
which is the actual output here:** every defect ID in the log must resolve to an
item, a wave, or an explicit "no owner, and why" line, and that is checkable
mechanically by grep.

- ✅ **D2 — sand landing on the player can push the player through solid walls.**
  *(major — **fixed 2026-08-11**; the reasoning and the fix's shape are in
  [ROADMAP.md](ROADMAP.md#wave-4--the-defects-session-5-found-on-the-way-past).
  It was carried here as "do this first" until 2026-08-12, three commits after
  it shipped.)* The simulation doesn't know the player is there, so material
  falls into the cells the body occupies. That's a normal state — being buried
  by a collapse — and the game escapes it by searching outward for the nearest
  spot the body fits and moving there. **It checks that the destination is clear
  and never checks whether anything solid is in the way**, so a body being
  poured on next to a wall can be relocated to the far side of it. The escape
  mechanism is the right idea; it just needs to refuse a destination it can't
  actually reach. It also fires every step under a continuous pour, which is why
  it reads as a glitchy shove and why you lose control while it's happening.
- ✅ **D1 — the dig animation freezes instead of repeating while the button is
  held.** *(**Fixed 2026-08-11.**)* The tool fires every 6 ticks; the swing
  takes 24. Every shot restarts the swing from its first frame, so it never gets
  to the second one. Fixed as a swing that loops while the button is held, with
  the tool firing at one point inside each swing — **built in the simulation,
  not the renderer**, because animation frames are never allowed to drive
  gameplay (that would break the guarantee that the same seed and inputs always
  produce the same run).
    - **The fire rate is split out of this defect, 2026-08-11, and that is a
      correction rather than a refinement.** The entry used to end "**this slows
      digging from about 10 per second to under 2**, which is a deliberate trade
      for the look" — **a 5x change to the only verb in the game, shipped inside
      a bug fix.** The defect is that the swing never advances past its first
      frame; the fire rate is a *design* number that happens to be adjacent to
      it, and nothing about fixing the former requires picking the latter.
    - **So the fix is the loop, and the rate is a number in
      [TUNING.md](TUNING.md).** Start at a 12-tick swing (~5 per second) rather
      than 24, which keeps the animation readable and does not gut the
      interaction. **Revisit it at S0**, which is the first moment there is a
      fail state to balance a dig rate *against* — deciding it now means
      deciding it with no information, and deciding it permanently means
      deciding it with no information twice.
        - > **What actually shipped, recorded 2026-08-12 as a correction rather
          than a rewrite.** The paragraph above is what was *planned*;
          `DigTool::SWING_STEPS = 36` is what was *built*, three times the 12
          recommended here, and it was chosen for a reason this file did not
          have — "slower than the walk cycle" was the request, and 30 steps is
          the walk cycle, which is the only unit both sides share. The
          recommendation was not overruled silently; it was made against an
          estimate and the build had a constraint. **The half of this bullet
          that survived is the important half**: the rate is a number and not
          part of the defect, and **the revisit at S0 still stands** at 36
          exactly as it would have at 12. Left in place because a wrong
          prediction next to its correction is worth more than a clean paragraph
          — the shape of the miss was reasoning about a rate in isolation from
          the clock it has to beat.
- ✅ **D6 — the walk cycle is about 10% too fast.** The animation timer only
  holds whole ticks, and the next value along is 20% slower. Trying that first;
  if it reads sluggish, *that* is what earns a finer timer. *(Changed to wait 6
  on 2026-08-12 and **accepted the same day** — 36 steps does not read sluggish,
  so the 20% overshoot cost nothing and the animation clock keeps its whole-step
  resolution. See [ROADMAP.md](ROADMAP.md) D6.)*
- ✅ **D3 — water comes to rest above its own free surface.** *(added to this
  wave 2026-08-11 — it had no owner anywhere in this file; **fixed 2026-08-12**.
  It was not `vent_fluid`, which is what this entry and its ROADMAP.md
  counterpart both predicted — the correction, and why the wrong guess is worth
  keeping, are at the [D3 entry in
  ROADMAP.md](ROADMAP.md#wave-4--the-defects-session-5-found-on-the-way-past).
  The half that survives is a during-pour transient that now clears on release,
  and it is a looks judgement rather than an open defect.)* Wave 3 stopped water
  riding to the top of a falling sand column, and a residual climb survived it.
  **This is a residual to eliminate and not a rate to tune**, which
  [PLAYTEST_LOG.md](PLAYTEST_LOG.md) already settled: displacing sand into a
  pool must raise the pool's free *surface* — that is conservation, and it
  passes — but **no configuration of sand and water makes it right for water to
  occupy a column standing above that surface.**
    - **It is the cheapest defect on this list because both halves already
      exist.** The invariant is assertable as written — *no water cell may come
      to rest above the free surface, splash excepted* — and `water_probe`
      already measures the quantity. This is a headless assertion plus whatever
      it turns out to catch, not an investigation.
    - **Doing it inside this wave rather than filing it as an item** is
      deliberate: it is a session-5 water defect and wave 3 was the session-5
      water wave. Leaving it loose is how it got lost the first time.

- ✅ **D7 — the player steps over a settled sand pile as if it weren't there.**
  The auto-step-up height is 5 cells against a body 20 cells tall, so the player
  climbs a quarter of its own height instantly and with no animation. Reported
  as evidence about E4 and it's almost certainly just this number. README's
  checklist only ever asks you to walk up a **one-cell** step, so nothing has
  been testing it. *(Set to 3 on 2026-08-12, with a settled-pile test and a
  README row, and **accepted the same day** — piles read as objects and nothing
  that used to be strollable stopped the body. See [ROADMAP.md](ROADMAP.md)
  D7.)*

---

## ✅ Shipped

Kept in full because the reasoning is the valuable part.

### The engine and its harness

- **Initial repo and build system.** ✅
- **Barebones C++/SDL2 pixel physics prototype.** ✅ Sand, water and wall
  interacting.
- **Data-driven material system.** ✅ Materials are rows in a table, not branches
  in code. Four generic behaviours (Static / Powder / Liquid / Gas) drive all
  movement, with density deciding what sinks through what.
- **Eight materials.** ✅ Sand, Water, Wall, Wood, Oil, Steam, Fire, plus
  transparent Empty — each cost one table row and no engine changes.
- **Fixed timestep.** ✅ The simulation runs at a constant 60 ticks per second
  regardless of display refresh rate.
- **Headless test harness.** ✅ The simulation runs with no window, covered by
  automated tests for conservation of matter, density layering, and border
  sealing.
- **Chunked dirty-rect updates.** ✅ The world is split into 64x64 blocks that
  track what can still move; settled blocks are skipped entirely, so cost scales
  with how much is *moving* rather than with world size.
- **Benchmark.** ✅ Times seven scenarios against the 60 Hz budget, at both
  960x540 and the played 1920x1080 since P2. Deliberately not a pass/fail test —
  timings inform, they don't gate.
- **Reactions.** ✅ A data-driven table of catalyst + target → result. Fire
  ignites wood and oil, is doused into steam by water, and burns out on its own.
- **Player Character + Player Physics.** ✅ A rigid body that is *not* a grid
  cell — it has its own position and only reads the grid to ask "is this
  solid?". Uses whole squares plus a fraction rather than decimals, so edge-case
  collision bugs never arise.
- **Structures fall as rigid bodies.** ✅ An unsupported wall falls as one piece
  keeping its shape, rather than dissolving into loose gravel (which read as
  broken rather than physical).
- **Basic Interaction — digging.** ✅ A ray traced from the player to the first
  solid cell, which is then blown out to a small radius. Deliberately its own
  module rather than a method on the player, so the player's grid access can
  stay read-only.

### Foundations (F1–F4)

Four items that weren't features — each was a prerequisite for several slice
items and got more expensive the longer it waited.

#### F1 — Determinism, first half: the simulation

- **F1.1 — Take a seed, keep the generator.** ✅ The world now takes a starting
  number and can report it back. A seed you can't read back is only half of
  reproducibility.
- **F1.2 — Add a wide step counter.** ✅ A counter of how many ticks have
  elapsed, wide enough not to repeat. The existing one-byte counter would have
  made randomness repeat every 256 ticks — visible patterning, not noise.
- **F1.3 — Introduce the hash, move one call site.** ✅ Replaced the traditional
  random number generator with a *stateless hash*: randomness is calculated
  fresh from (position, tick, seed, purpose), so there's no hidden generator
  state to save or get out of sync.
- **F1.4 — Move the remaining four call sites.** ✅ All randomness now goes
  through the hash, each purpose on its own separate "stream" so unrelated
  decisions can't correlate.
- **F1.5 — Reserve the stream separation, don't build it yet.** ✅ Wrote down the
  rule that world generation and simulation must draw from separate random
  streams, before there was a generator to break it — otherwise adding one extra
  cave would silently change how sand falls elsewhere.
- **F1.6 — Delete the old generator, then measure.** ✅ Measured honestly: the
  hash is about 1.7–1.9% *slower* in one scenario. Recorded as-is rather than as
  the win it was filed as.
- **F1.7 — Docs.** ✅ Wrote the determinism rules into the README, including an
  explicit note about what was *not* yet true.

#### F2 — Something owns the run

- **F2.1 — Create `Run` and move the three loose variables into it.** ✅ The
  world, player and dig tool were three unrelated local variables; four separate
  future features all needed to reset/save/swap them together.
- **F2.2 — `reset(seed)` on both the grid and the run.** ✅ Wipes every
  changeable field by hand rather than using a compiler shortcut, specifically
  so the test proving nothing is forgotten is possible to write.
- **F2.3 — An `Input` struct and `Run::step(input)`.** ✅ The second half of
  determinism. Input used to be sampled once per *drawn frame* and applied to
  every simulation tick in it, so a held key did different things at 30 fps and
  144 fps. Now a run is a seed plus a replayable list of inputs.
- **F2.4 — Confirm the payoff rather than assuming it.** ✅ No code — checked
  that the reset hotkey and pause/single-step items are now genuinely small, and
  surfaced one unresolved policy question (should the brush paint while
  paused?).

#### F3 — Camera and world-space coordinates

- **F3.1 — Separate world size from window size.** ✅ The world used to be
  exactly the size of the window, so a level couldn't be bigger than one screen.
- **F3.2 — One `Camera`, and only it knows the pixel scale.** ✅ The conversion
  between world coordinates and screen coordinates was scattered across five
  places; now it lives in one file. **V11 revisits the "compile-time" half of
  this.**
- **F3.3 — Upload only what is visible.** ✅ Only the visible portion of the
  world is sent to the graphics card each frame, so the cost stops growing with
  world size.
- **F3.4 — Follow the player, clamped at the world edges.** ✅ The camera centres
  on the player and stops at the world's borders instead of showing empty space
  beyond them.
- **F3.5 — Answer off-screen simulation once, in writing.** ✅ Everything
  simulates always, regardless of where the camera is looking. If it didn't, an
  avalanche would freeze the moment you walked away — and two players with
  identical inputs could diverge just from looking at different things.

#### F4 — A way to get a level into the grid

- **F4.1 — `paint(x, y, type, colour)`.** ✅ Placing a cell with an explicit
  colour instead of a random shade, which is what lets hand-drawn artwork keep
  its own pixels.
- **F4.2 — Scene format and a headless loader.** ✅ A level is two same-size
  images: one saying which material each cell is, one saying what colour it
  should be.
- **F4.3 — BMP decoding, loaded at startup.** ✅ Shipped broken with three
  separate bugs (wrong row stride, scene bigger than the grid so every cell was
  silently dropped, and assets not copied next to the executable) — all fixed.
- **F4.4 — Make the first scene a test fixture wearing art.** ✅ A hand-generated
  scene where every feature exercises something specific: uneven stairs for
  step-up, fence posts for dig-the-base collapse, a pit with pillars for the
  collapse flood fill, a water channel, and jump ledges.

### E1–E3 — Simulation depth (details)

- **E1 — Liquids find their level.** ✅ A surface water cell looks through its
  own connected body for a lower surface and moves there. The obvious approach —
  letting the low side *rise* — doesn't work: it leaves a bubble that splits the
  body in two and the whole thing falls asleep two cells out of level. **E5b
  replaces the mechanism** — see the note there on what a search can never do
  that a field can.
- **E2 — Heat, the seventh axis.** ✅ Every cell carries a temperature; heat
  flows between touching cells; reactions have temperature windows. First
  attempt cost 18% performance and was rejected — a cell already at room
  temperature now does no thermal work at all, bringing it to ~2%.
- **E3 — Collapses break instead of dropping rigid.** ✅ A landing structure
  cracks along the boundary between the columns that hit something and the
  columns that hit nothing. Uses a per-cell "piece ID", so a crack is stored as
  two cells *disagreeing* rather than as a line, which lets it survive the piece
  moving.

### V1–V2 — Visual foundation (details)

- **V1 — Transparent empty and a backdrop layer.** ✅ Two changes that only work
  together: empty cells became see-through *and* the world texture was set to
  blend. Either alone is a no-op.
- **V2 — Palette and jitter pass.** ✅ Colours chosen against the new backdrop
  rather than against black; only fire and water keep full saturation, since
  they're the two things you must never miss. Shipped a critical bug: retuning
  the colours made the level file match nothing and the game booted to a blank
  world for a whole commit. **V11's theme file is the structural fix for the
  class of problem this was.**

### Correctness pass — a full read of the source

A line-by-line review of every source file, with each suspected bug reproduced
by a test program before it was believed.

- **The startup scene loaded as an entirely empty world.** ✅ Fixed by separating
  the level file's colour codes (permanent markers) from the render palette
  (free to change), so retuning art can never invalidate a level again.
- **A settled pool of water never went to sleep.** ✅ A pool only settled if its
  cell count happened to divide evenly by its width — so almost every real
  puddle churned forever. Fixed by requiring a sideways move to land somewhere
  it can actually rest.
- **Steam was an undeclared ignition source.** ✅ Steam spawned hotter than
  wood's ignition point, so putting a fire out with water was a way of starting
  a bigger one. Now enforced at compile time: nothing may spawn hotter than the
  coldest ignition point.
- **Smaller things, each verified rather than assumed.** ✅ An overflow check
  that itself overflowed, a shared scratch buffer whose name hid a second user,
  and two stale comments — one describing a guarantee that measurement
  contradicted, one warning about a defect that had already been fixed.

### What the correctness pass changed about how to work here

- **A green test suite proved less than it looked like.** Both serious bugs
  lived exactly where tests couldn't see — one in an untested file, one in a
  case a test had deliberately excluded. **When a test comment explains why a
  case is skipped, that comment is a bug report.**
- **The manual checklist is load-bearing and wasn't run.** One launch would have
  caught the blank world immediately.
- **Two of the bugs were table edits with no code touched.** Data-driven design
  moves the danger into the *relationships between rows*, which have no compiler
  checking them unless one is written deliberately.

---

## ✅ Items closed inside the live tracks

*Moved out of `ROADMAP.md` by `W4` on 2026-08-17, in the pass that took the
boundary past the `## ✅ Shipped` heading. Each of these was finished work
sitting in an open track, with the next item's argument underneath it. Where
one of them still constrained an open item, that constraint was written into
the open item before the move — `V23b`'s ~50% camera cap into `V22` is the
worked example. Each entry is left in `ROADMAP.md` as a one-line stub
pointing here.*

### V5 — Write the art direction down

- [x] **V5 — Write the art direction down.** *(done — see
  [notes/art_direction.txt](notes/art_direction.txt))* *Observed:* there is no
  art direction in this repository. The only thing resembling one is the opening
  paragraph of the art-pipeline note — "detailed snowy Japanese railway-crossing
  scenes, roughly 640x400, cold desaturated blue-grey palette with warm sodium
  and signal accents, heavy ordered dithering in the sky and snow" — and it is a
  *reference target for one test scene*, written to justify a test fixture. It
  has since become the de facto direction for the whole game by nobody deciding
  anything.
    - **The finding that makes this urgent: the reference and the fiction are
      two different games' art, and nothing in this plan reconciles them.**
      `notes/story.txt` is a dystopian United States where a chatbot runs the
      government, humans are barred from the economy and live on rations, and
      "quantum magic" is how they reach other worlds. A snowy Japanese level
      crossing is not that. V2's palette has *already* been authored against the
      crossing reference — the world went cold-backdrop and warm-desaturated
      because of it — and V3, V4, V7 and V8 will each be authored against
      *something*. The only question is whether it is the same something, and
      right now the answer is being decided per-asset by whoever is looking at
      which file.
    - **The reconciliation is available and cheap, which is why this is a
      writing task and not a crisis.** By the fiction, quantum worlds are
      *other* worlds — so a snowy crossing is a perfectly legitimate trial
      location, and the dystopian US is the home base the player returns to.
      That is **two directions**, and the plan currently budgets for zero.
      Retiring the crossing reference is equally acceptable. What is not
      acceptable is drifting, because every asset authored under an unstated
      direction is an asset that has to be re-authored when it is stated.
    - **Deliverable:** `notes/art_direction.txt`, and it is the document every
      later V item is checked against. It has to answer, in writing and in this
      order: **which locations exist and what each one reads as**; **the
      palette's intent** (V2's set is already a decision — cool backdrop, warm
      desaturated world, saturation reserved for Fire and Water — and this is
      where it stops being an implementation detail of one table and becomes a
      rule); **the dithering rule**, since V2 cut `color_jitter` specifically to
      make room for hand-placed ordered dithering and nothing has yet placed
      any; **what the player reads as**, which V3 cannot start without; and
      **what is drawn versus what is simulated**, which the original three-layer
      model already sketched and which V4, V8 and V9 all depend on.
    - **Why it runs before V3 rather than after.** V3 is the player sprite: the
      single most direction-dependent asset in the game. You cannot draw the
      protagonist without knowing whether they are a figure in a snow suit at a
      level crossing or a ration-line human in an AI-run America, and a sprite
      authored against the wrong answer is not a tuning problem, it is a redraw.
    - **Reference footage is an input to this item specifically, and it is worth
      being concrete about what it is for.** Two reference points now exist
      rather than one — the level-crossing stills the art-pipeline note opened
      with, and gameplay capture of comparable games — and the second is useful
      *because* it is not the first: two directions that disagree force the
      choice to be made rather than absorbed. What to actually take from it is
      bounded, and it is not "a look": contrast and readability against a busy
      simulated background, how much of a frame is not simulated at all, and
      what a player's eye is drawn to when everything is moving. Those are the
      questions this document cannot answer from its own screenshots, because
      its own screenshots are of a debug palette. Extraction and the entry
      format are in
      [notes/reference_observations.txt](notes/reference_observations.txt);
      frames are gitignored and the written observation is the artifact.
    - *Verify.* No code, and the check is the same one F3.5 used: the answer is
      written down before anything that needs it can re-derive it,
      inconsistently, on its own. The concrete test is that V3, V4, V7, V8 and
      V9 can each name the line in it they are authored against.
    - **Resolved as: the crossing stills are retired, the Noita-forest
      screenshot in
      [notes/reference_observations.txt](notes/reference_observations.txt) is
      adopted as the first quantum world's direction, and the dystopian US stays
      the frame story per the reconciliation this item itself proposed.**
      Locations are per-world from here on; a second biome gets its own section
      when it exists rather than inheriting this one's palette. V3 (the player)
      is explicitly not answered by this pass — see the note at the bottom of
      art_direction.txt — and stays open.
    - **Correction, 2026-08-16 — half of that resolution had gone false and
      nothing noticed for three months.** Both deliverables,
      `notes/art_direction.txt` and `notes/reference_observations.txt`, were
      **deleted in commit `e05609d`** while roughly fifteen places — ASSETS.md,
      ENGINEERING_NOTES.md, this file in six places, `generate_test_scene.py`,
      `tests/rim_probe.cpp`, `src/main.cpp` twice, and five of the `tools/`
      scripts — went on citing them by name for a specific rule each. That is
      the exact failure this project's first rule names: a stated rule that
      stopped matching the tree and kept being believed. **Both files are
      rewritten from scratch as of 2026-08-16**, against the Cast n Chill frames
      in `resources/images/`, and each section a caller names is marked with who
      names it so the next deletion is visible. What changed in substance: the
      **Noita-forest screenshot no longer exists in `resources/` and no
      observation of it survived**, so that adoption is unverifiable and the CnC
      frames are adopted in its place — *the crossing stills stay retired, which
      was the half that was right*. V3 has since shipped, so the player section
      now records what was built rather than leaving the question open. Two new
      findings came out of the rewrite and are not V5's: the **band-value
      defect** (our four depth bands overlap almost completely, so nothing but
      the rim separates them — a renderer problem, not a palette one, because
      the light pass can only add) and the **failed parallax measurement** (the
      three "parallax" reference frames are three generated lakes, not one pan,
      so our `0.04`/`0.15` remain unmeasured and a "measured" factor came within
      one step of being fabricated). Both are written up at the notes.

### V6 — One locked palette, shared by the table and the art

- [x] **V6 — One locked palette, shared by the table and the art.** *(done — see
  [tools/pixel_art.py](tools/pixel_art.py),
  [tools/validate_palette.py](tools/validate_palette.py))* *Observed:* V2 found
  that authored terrain never reads `MATERIALS` at all — `load_scene` calls
  `grid.paint(x, y, type, color)` with the colour out of the albedo BMP and
  `paint` writes it verbatim. So the engine's eight colours and the art's
  colours are two independent sets that happen to have been chosen by the same
  person on the same day, and nothing anywhere checks that they agree.
  *Unlocks:* every authored asset after it, and it is what stops V2's palette
  from being a one-off.
    - **The seam V2 named is not a bug, it is the absence of a shared palette.**
      Authored wood burning into table-coloured fire is the visible symptom; the
      cause is that there is no set both sides draw from. Fixing it at the seam
      — carrying an authored colour across a reaction — is the wrong repair and
      would put art data into `src/physics/`, which nothing there currently
      reads and nothing there should.
    - **Built as** a named palette both sides index: a header the `MATERIALS`
      rows are written in terms of, and the same values exported in whatever
      form the external editor wants. Plus a **validator**, which is the half
      that actually holds the line: a check that every colour in an albedo BMP
      is in the palette, run the same way `generate_test_scene.py`'s legend
      match is already checked. An off-palette pixel that loads silently is the
      same failure mode as F4.3's size mismatch — the whole scene dropped
      without a warning — and it gets the same fix, which is to make it loud.
    - **What this deliberately does not do:** it does not derive per-cell colour
      from the material table. That is the optimisation `ENGINEERING_NOTES.md`
      prices at ~6x the hot loop's memory traffic and rejects outright, because
      it is mutually exclusive with authored per-cell colour, which is the
      visual pillar. A shared palette constrains what colours are *chosen*; it
      changes nothing about where they are *stored*.
    - **Built as** `tools/pixel_art.py` — the locked `PALETTE` dict, an
      ordered-dithering helper (`dither_mix`, over the `bayer_threshold` matrix)
      and the rim-light pre-process every generator in `tools/` now shares —
      plus `tools/validate_palette.py`, which fails loudly on an off-palette
      pixel the same way an unmatched legend colour already does (`main.cpp`'s
      `load_scene_from_bmp`), and `assets/palette.gpl` for loading the set into
      an external editor directly, regenerated by `tools/export_palette_gpl.py`.
      `MATERIALS` in `src/physics/material.h` and `src/scene/legend.h` are
      untouched, as specified. Not yet done: nothing currently *runs* the
      validator automatically (no pre-commit hook, no build step) — it is a
      script to run by hand until an off-palette asset actually ships once.

### V3 — Player sprite decoupled from its hitbox

- [x] **V3 — Player sprite decoupled from its hitbox.** *(done — superseded by
  V3.1 below; its generator has been replaced by
  [tools/player_sheet.py](tools/player_sheet.py))* *Observed:* the body is drawn
  as a plain white rectangle, and a person cannot be drawn as one. *Unlocks:*
  the player reading as a character at all, and V9's animation follow-on.
  Collision stays 8x20 — it is tuned to the physics and 18 tests stand on it —
  and a larger sprite is drawn anchored to the box's bottom-centre with an
  offset. Touches no physics and breaks no tests.
    - **This item said "4x8" in three places and the code had said 8x20 since
      the Noita rescale**, which is exactly the failure mode the "point at code
      by name" convention at the top of this file exists to prevent — the
      convention covers line numbers and did not cover *quoted constants*, which
      go stale the same way and are harder to spot because they read as
      deliberate. `notes/art_direction.txt` carried the same stale pair. Both
      corrected against `player.h`.
    - **Built as** a 14x26 colour-keyed BMP from a hand-authored ASCII grid
      (`tools/generate_player.py`, since replaced), in the locked palette plus
      one new `char_*` group in `tools/pixel_art.py`. ASCII rather than a binary
      for the reason `hotbar.cpp`'s icons are hand-set bits: at this size the
      source form should be the one a diff can show, and a one-pixel change
      should review as a one-character change.
    - **The offsets are duplicated between the generator and `main.cpp` and
      nothing enforces agreement** — the same trap V8's parallax factors have,
      written down in both places for the same reason. Each side asserts the
      half it can see: the generator checks canvas size, that the bottom row is
      painted (a sprite whose lowest row is empty hovers above every floor
      forever, and reads as a physics bug), and that no box row is empty;
      `main.cpp` has a `static_assert` that the sprite is at least the box's
      size.
    - **Facing is tracked in `main.cpp`, not on `Player`,** and stickily — it
      holds the last direction actually pressed. Presentation state on the
      simulation body is state F1's determinism tests would then have to account
      for, for a flag that only ever picks an `SDL_FLIP`.
    - **`char_accent` (`0x945128`) is now the brightest value in the locked
      palette**, above `rim_grass`, which the palette had called "the one bright
      accent in the whole terrain layer". It is six pixels on the mask and is
      defensible at that area, but it sits in the warm-orange family reserved
      for Fire — the tension is recorded next to the entry in `pixel_art.py`
      rather than left to be rediscovered. **Check it against a burning scene
      before any more copper is added**, and specifically once V7's emissive
      pass covers more than temperature.
    - **Still open, and deliberately:** one static pose only, per the animation
      note below. The fallback rectangle is kept for a missing asset rather than
      deleted — a game you can still move around in is a better diagnostic than
      an invisible player.
    - **Authored against V5's answer, which is why it moved behind it.**
      Everything technical about this item is standard practice and was never
      the hard part; the hard part is what the figure *is*.
    - **What this does not do, named rather than left to be discovered:
      animation.** A sprite that slides while walking and holds one pose while
      falling reads worse in motion than a rectangle does, because a rectangle
      makes no claim. Walk, fall, land and dig are the four states the existing
      code can already distinguish without any new state being tracked —
      `Player` knows its velocity and whether it is on the ground, and `DigTool`
      knows its cooldown. That is the follow-on and it is deliberately not this
      item. *(Done — **V3.1** below.)*

### V3.1 — Animation, and one decomposed limb

- [x] **V3.1 — Animation, and one decomposed limb.** *(done — see
  [tools/player_sheet.py](tools/player_sheet.py),
  [src/render/player_anim.h](src/render/player_anim.h))* *Observed:* V3's single
  pose responded to nothing but facing, so the figure slid while walking — the
  exact defect V3's own text predicted would read worse than the rectangle.
  *Unlocks:* the player reading as *doing* something rather than as being
  somewhere.
    - **The finding that shaped this, from reading how Noita actually does it:**
      its expressiveness is not frame count. Its player sheet carries ~50
      animations, but the reason the wizard reads as alive at fourteen pixels
      wide is **decomposition** — the wand arm is a separate sprite that rotates
      toward the cursor over a body playing a short loop, and the cape is
      separate simulated cloth. Neither is baked into the sheet, so a handful of
      body loops covers every direction of aim. **The lesson was "decompose",
      not "draw fifty animations"**, and it is what decided this item's scope.
    - **Built as** Noita's model with the parts this game cannot yet use left
      out: a sheet whose rows are animations, and a table of `(row, col, frames,
      wait, loop)`. `col` exists so two animations can share a row — `rise` and
      `fall` are one row holding two poses, which is Noita's own "same row,
      several names".
    - **The decomposed limb was built and then pulled, and the sheet is what
      remains.** A rotating aiming arm drawn over the body loop, attached at a
      marker pixel in a per-frame hotspot image, is the thing the finding above
      actually argues for — and it went back out rather than shipping
      half-right. **What it cost while it was in was not the draw call; it was
      the second image.** Every frame drawn from then on had to carry a marker
      pixel or `--validate` failed, which is a standing tax on authoring in
      exchange for a limb the game was not yet using well.
      `tools/player_sheet.py` no longer emits a `SHOULDER` table, generates no
      hotspot image, and asks nothing of the artist beyond the sheet;
      `assets/player_hotspots.bmp` and `assets/player_arm.bmp` are deleted, and
      neither held hand-authored work — both were `--starter` output with one
      uniform shoulder position. **Bringing it back is: the hotspot image, the
      marker-pixel validation, a `SHOULDER` table beside `ANIMATIONS`, and a
      rotate-about-the-shoulder `SDL_RenderCopyExF`.** Nothing about the sheet
      format has to change to allow it.
        - **Its one non-obvious trap, recorded because it cost a bug the first
          time:** `SDL_RenderCopyEx` mirrors the *texture* and then rotates the
          quad, so flipping the sprite does not mirror the aim angle — it turns
          the arm's rest direction from `+x` into `-x`, which the rotation has
          to undo with a 180° offset. The reflection (`180 - angle`) is right
          for a cursor level with the shoulder and inverts the vertical
          everywhere else, so aiming up-left pointed the arm down-left, and both
          of the cases anyone checks by eye are the two it gets right.
    - **Five animations from state the code already had**, which was V3's stated
      bound and held: `idle`, `walk`, `rise`/`fall` (chosen by the sign of
      `velocity_y()`, so a long drop holds the falling pose instead of cycling),
      and `dig` as a one-shot. `Player` gained one read-only `velocity_x()` and
      nothing else.
    - **The clock is the fixed step, and this is the trap V10 names for screen
      shake arriving somewhere nobody expected it.** A walk cycle driven off the
      rendered frame runs at nearly 3x speed at 165 Hz — and presents as an
      *art* problem, which is what makes it dangerous. **The fix is not to move
      animation into the simulation:** F3.5 forbids rendering feeding
      simulation, and animation state on `Player` is state the determinism tests
      would then have to carry. It is advanced inside `main.cpp`'s existing
      fixed-step loop and lives entirely in `src/render/`.
    - **The selector is SDL-free and tested** (`anim_test`, the eighth suite),
      for the same reason V7's `LightField` is: what it produces is a row and a
      column. Its failure modes are all silent and all *look like art problems*
      — a cycle that restarts every step reads as "the walk animation is bad", a
      one-shot that never clears reads as "the dig sprite is stuck" — so none of
      them would send anyone to the file that caused them.
        - **One of those tests caught a bug that compiles, links and runs.** A
          `constexpr` variable at namespace scope in a header has internal
          linkage, so every translation unit gets its own object at its own
          address — and the selector identifies the current animation by
          pointer. `main.cpp`'s `IDLE` and `player_anim.cpp`'s `IDLE` were
          different addresses, so every comparison was false and the animation
          restarted every step. `inline constexpr` fixes it and the generator
          now emits it with the reason attached. **This is the argument for
          testing the selector rather than checking it by eye**: the symptom is
          a figure standing still twitching, which is indistinguishable from bad
          art.
    - **Authoring moved from ASCII to a hand-drawn sheet, reversing V3's own
      reasoning on purpose.** One pose as an ASCII grid meant a one-pixel change
      reviewed as a one-character change; twenty-four such blocks is where that
      argument inverts. `--starter` bootstraps the sheet with V3's pose stamped
      into every frame slot, so the game runs end to end from the first minute
      and each redrawn frame improves on a working baseline rather than being a
      prerequisite for one.
    - **`tools/generate_player.py` is gone and its layout duplication with it.**
      Everything about the sheet is emitted into a generated
      `src/render/player_sprite.h`, so the trap V3 shipped with — four numbers
      in two files, nothing enforcing agreement — is closed rather than
      documented, which is what a sheet's several dozen numbers made necessary.
    - **Deliberately not done, with reasons:** the **verlet cape** (off-grid
      float cloth; it needs an off-grid substrate that E5a deliberately does not
      provide, so it waits on E8's body extraction rather than on the E track),
      **sprite stains** (Noita maps the sprite's UVs so world materials splash
      and stain it — the most on-brand idea it has for an engine whose whole
      subject is materials; filed in `ENGINEERING_NOTES.md`), and **frame-tied
      gameplay events**, which Noita has and which this project cannot adopt:
      firing gameplay off an animation frame is rendering driving simulation,
      which F3.5 forbids outright.

### V7 — Per-cell emissive lighting

- [x] **V7 — Per-cell emissive lighting.** *Observed:* fire is the brightest
  thing in the world and casts nothing. Terrain does not shadow itself, a flame
  in a pit lights nothing, and the only reason the scene reads at all is that
  every cell is drawn at full brightness regardless of what is around it.
  *Unlocks:* the part of this tier that would genuinely read as cutting-edge,
  and it is the one item here that a screenshot sells on its own.
    - **E2 is what makes this affordable rather than speculative, and that is
      the whole reason it is sequenced here.** "Which cells are emitting" is a
      question the engine already answers — a cell above ambient is a light
      source, for free, and nothing new has to be tracked to know it.
    - **The architectural decision this item forces, named now because the
      alternative is much larger.** The renderer is `SDL_Renderer` with no
      shader path, chosen by the same zero-dependency logic that picked BMP over
      PNG and immediate-mode over Dear ImGui. Per-pixel lighting at the played
      size is 256,000 cells on the CPU every frame, which is not affordable. The
      shape that is: a **downsampled light grid** — one value per small block of
      cells, propagated over a handful of iterations, uploaded as a small
      texture and stretched with linear filtering over the cell texture. That is
      a second `SDL_RenderCopy`, no new dependency, and a resolution knob to
      trade quality against cost. **The alternative is moving to a shader
      path**, which is a genuinely large decision about what this project's
      renderer is, and it should be made deliberately with its own entry in
      `ENGINEERING_NOTES.md` if it is made at all — not discovered halfway
      through a CPU implementation that turned out too slow.
    - **The trap: light must never become a simulation input.** F3.5 settled
      that rendering does not feed the simulation, and gave the sharper reason —
      two players on the same seed and the same input log must not diverge
      because of what was on screen. A light value read by anything in
      `src/physics/` reimports exactly the failure mode F1 spent seven steps
      ruling out, through a feature that was only ever supposed to touch
      rendering.
    - **The gate this item was given as a footnote is now broken on purpose, and
      here is the argument.** The gate said: lighting an unlit debug palette
      flatters nothing, so wait for V5, V6, V3, V4 and V8. That reasoning is
      about **albedo** — it is true of terrain, whose look is what colour it is,
      and it is false of fire, whose look is what it *emits*. Reference footage
      of a burning scene ([PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 1
      follow-up) makes the size of the difference plain: the flame cells occupy
      a modest band and the whole cavern is lit, walls tens of cells out picking
      up orange. Strip the glow and what is left is a ribbon of orange noise on
      black — which is precisely what E9's rebuilt fire will produce on its own.
      **Fire is the one subject in this tier where the light is the art**, so it
      is the one subject the gate was never really about.
        - **What this does not license is doing V7 early for everything else.**
          The gate stands for terrain, for the player and for the backdrop, and
          V7 stays after V8 in the running order for those. What is pulled
          forward is emissive light from cells that are hot, which E2 already
          tells the engine for free. Anything needing authored albedo to respond
          correctly waits where it was.
        - **The scope trap, named because this wave is the one most likely to
          grow.** V7's architecture note below is the affordable shape and it
          does not change: a downsampled light grid, a handful of propagation
          iterations, one extra `SDL_RenderCopy`. Pulling the item forward is a
          reorder, not a licence to reach for the shader path — that decision
          keeps its own entry in `ENGINEERING_NOTES.md` and is not made in
          passing while chasing a look.
    - **Built as specified, and the scope trap held.**
      `src/render/light.{h,cpp}` — a light grid at one texel per 4x4 cells,
      sixteen max-propagation iterations with a convergence early-out, packed to
      ARGB and drawn with one additive `SDL_RenderCopy` under linear filtering.
      **No shader path was reached for, so `ENGINEERING_NOTES.md` gains no
      entry**, which is the outcome that note was written to make checkable.
      Measured in `PERFORMANCE.md`: 0.20 ms/frame lit, 0.05 ms unlit, ~1.2% of a
      60 Hz frame — and the measurement needed a second instrument, because
      `grid_bench` times the simulation step and this is not in it.
        - **The "light must never be a simulation input" trap is enforced by the
          dependency direction and by the build file.** `light.h` includes
          `physics/grid.h`; nothing under `src/physics/` includes `light.h`, and
          it is kept out of `ENGINE_SOURCES` in its own `RENDER_SOURCES`
          variable so that the day something in the simulation needs it, the
          mistake has to be written down in `CMakeLists.txt` to compile at all.
        - **What is still open, and deliberately not done here:** emission is
          read from temperature alone, so this lights hot things and nothing
          else — the gate on terrain, player and backdrop albedo stands exactly
          where V8 left it. Additive only, so nothing in the scene is *darkened*
          by the absence of light; a world with no fire in it looks precisely as
          it did before. That is the reorder's boundary, not an oversight.

### V20 — The value ceiling, and the two defects V19 4b shipped

- [x] **V20 — The value ceiling, and the two defects V19 4b shipped.** *(new and
  done 2026-08-16, admitted by playtest)* *Observed:*
  [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 6, the first human eyes on V19 4b,
  returned three visual defects against the ground plane. *Unlocks:* every
  remaining V19 band, because all three causes are properties of the stack those
  bands would be authored into rather than of the plane.

    **The direction question was asked with the report — "are we going down the
    wrong route to achieve the Cast n Chill graphics" — and the answer is no.**
    Every mechanism entry 7 identifies is built and none is misconceived. What
    was wrong is amplitude, and one structural mistake produced it.

    - **The structural mistake: the ladder was built downward from a floor
      instead of downward from a ceiling.** `sky_deep` was authored at luminance
      **18 of 255**, and the only separation tool the project owns is `Grade`,
      which is a multiply and can therefore only darken — so every band added
      since V11 was pushed further toward zero from a sky that was already at
      the bottom. Entry 7's ladder is 0.78 per band over six bands, a factor of
      3.4; from L 18, band six lands at **L 5**.
        - **The numbers, measured on `resources/game_screenshots/plane_test
          (1-4).png` and on the shipped BMPs.** The whole composition occupied
          **L 15.5 to 24.5** against the reference's 51.6 to 173.6. Nine levels.
          **The reference's *smallest single band join* is 14** — larger than
          our entire frame's range — and its **night** frame has its sky at L
          163, nine times ours.
        - **The palette group was raised wholesale**, stated as post-grade
          targets since that is what reaches the screen: sky 95 → 62 top to
          horizon, mountain rim 71, mountain body 44, ground far **30** (the
          frame's darkest value, entry 7's mechanism 2) ramping to 78 at the
          near edge. The two grades were **not** touched — 0.60 and 0.53 keep
          their `TUNING.md` rows and their history, and the horizon join comes
          out at 14 levels, which is the reference's own.
        - **The sky ran the wrong way and nobody had looked.** `sky_deep` (the
          top) was darker than `sky_horizon` (the bottom), so the frame's
          brightest row sat immediately above the row mechanism 2 requires to be
          its darkest. Corrected by the palette alone; the generator loop is
          unchanged.
        - **The one-line version of the error is preserved in `pixel_art.py`
          rather than deleted**, because the shape recurs: the ground pair was
          justified as "ratio near/far 1.83 against the reference plane's 1.78".
          That is arithmetically true and it is the wrong quantity — entry 7's
          plane is a ratio of 1.78 **and a difference of 61 levels**, and
          matched as a ratio down at L 18 it bought a ramp of 18 levels before
          the grade and **9.8 after**. **When a mechanism is absolute contrast,
          matching its ratio is not matching it.**
    - **Defect F-2, "mountains are not visible just the plane", was not the
      grade and the checklist asked the right question about the wrong layer.**
      Measured, the mountains against the sky were the **largest contrast
      anywhere in the frame**. The plane was covering them. `draw_ground` placed
      the horizon at 0.55 of the *window*, justified as "where the played
      frame's terrain skyline sits" — which names the terrain, while the layer
      it collides with is the mountains, authored independently in their own
      image's coordinates by a different script. The plane's horizon ran between
      screen rows 594 and 238; the mountain silhouette began at 604. The plane
      is opaque RGB with no colour key and is drawn after the mountains in
      `LAYERS`. **The two constants were contradictory at every camera position
      the world reaches, and nothing could have said so** — step 12 was watching
      the grades.
        - **Fixed by making the contradiction unrepresentable, not by retuning
          either number.** The horizon is now derived from
          `backdrop_layers::MOUNTAINS_SKYLINE_MAX`, generated from the same
          seeded walk that draws the silhouette, so the plane's far edge *is*
          the mountains' deepest skyline row. The silhouette was also raised in
          the art (`MOUNTAIN_BASE_FRACTION` 0.58 → 0.31) so there is room below
          it for a plane at all.
        - **A second, independent half of the same defect: the horizon had the
          wrong parallax factor.** It moved at the plane's own 0.11 while the
          mountains move at 0.06 — **a receding plane's far edge is at infinity
          by construction, so its factor has to be the smallest in the scene,
          not the plane's near-edge one.** Given 0.11 it climbed past the
          mountains within a few hundred cells of descent even from a correct
          starting position. Now on the mountains' factor, which locks the two
          together.
        - **Stated as a fraction of the loaded mountains texture and not as a
          row index, and that distinction cost a diagnosis.** Stated as a row it
          was a number only the shipped 1642-row BMP could satisfy; against the
          golden fixture's 300-row synthetic mountain it put the whole plane
          below the window. The frame composed cleanly, every check in
          `test_golden_frame.cpp` passed, and **the checksum silently reverted
          to a value from before the plane existed.** It was caught only because
          that value was recognised. `.claude/rules/simulation.md` already warns
          that a checksum over a null-textured layer covers the layer's
          *absence*; this is the same hazard reached by geometry, and the
          fixture now asserts the plane's presence by composing once without its
          texture and requiring a different frame.
    - **Defect F-1, "black bands appearing in between the plane pixels", was two
      causes that produce one symptom, and each needed its own half of the
      pipeline.**
        - **The arithmetic half.** `backdrop_wrap.h`'s own comment states the
          requirement — "strip i's bottom source row is strip i+1's top source
          row, or the texture repeats or skips a row at every boundary" — and
          the draw call broke it: `SDL_Rect src{0, (int)s.src_y, w, (int)s.src_h
          + 1}` rounds each strip's start and its height *independently*. Run
          against the shipped geometry that draws **268 rows of a 256-row tile
          and misses at 12 of its 23 boundaries**, which is why the symptom was
          a set of bands rather than one line. Fixed by rounding the shared
          *boundaries* instead, in `plane_src_row`, so adjacent strips meet by
          construction.
        - **`test_backdrop.cpp` property 4 passed the whole time, and
          generalising why is worth more than the fix.** It checks the float
          rows meet, and they do. Nothing asked whether their *roundings* did.
          **When a continuous quantity is checked for a property and then
          quantised before use, the property has to be re-checked on the
          quantised value** — otherwise the passing test is measuring something
          the renderer never sees. The new property 7 asserts conservation on
          the integers, and was verified against the unfixed arithmetic.
        - **The art half, which no arithmetic could have fixed.** The tile was a
          **49/49 ordered dither between exactly two tones**, point-sampled at
          up to ten source rows per screen row near the horizon, so which rows a
          strip landed on decided whether it came out dark — a band that moves
          as the camera moves. Cause: `dither_mix` picks between two colours, so
          a "ten band" ramp built from it contains **two colours and ten
          proportions of them**. `banded_ramp` now produces ten distinct flat
          tones with dithered hand-offs, which is what V5's rule ("flat tones,
          not a per-pixel smooth blend") actually asked for and what it had
          quietly stopped being.
    - **Defect F-3, "dashes move smoothly but the effect isn't very convincing",
      was amplitude plus one specific loss.** The plane's ramp was **9.8 graded
      levels against the reference's 61**, and tile rows 160-255 — the near
      third, where entry 7's mechanism 3 says contrast should be *growing* —
      were flat at L 40.4, because the two-colour dither had already saturated.
      The ramp is now 30 → 81 post-grade, **50.6 levels**, gradient to the near
      edge. The marks were also weighted toward the viewer and cleared out of
      the top 30% of the tile: a mark near the horizon is being point-sampled at
      ten to one, which is a speckle that flickers as the camera moves rather
      than texture, and it is the wrong end of the frame for it under both
      mechanism 2 and mechanism 3.
    - **The golden checksum moved to `0xcde4dc1a39927fca` and the house
      procedure was set aside rather than claimed.** "Ship the no-op half first"
      needs a no-op half; this is three causes that all move pixels and none of
      which can be staged at identity. That is written into the constant's
      comment plainly, because **a procedure claimed and not followed is worse
      than one openly set aside.** The separation it does not have is carried
      instead by `backdrop_test`'s two new properties, which pin the rounding
      independently of any frame.
    - *Verify.* Full suite green (13/13). **Not verified and owed: every part of
      this that a human has to look at** — the raised palette is a large
      aesthetic swing that no test can judge, the mountains' new position is a
      composition, and F-1's art half is specifically a defect that only appears
      while the camera moves. Checklist steps 11 and 12 are both owed, and step
      12's question has changed: it now asks whether the frame is *too bright*,
      which is the opposite of what it was written to watch for.

### V23 — The camera leaves centre, and digging brings it back

- [x] **V23 — The camera leaves centre, and digging brings it back.** *(new and
  built 2026-08-17)* *Observed:* not a playtest report — a **measurement taken
  while starting V22's scene work**, which is the unusual thing about this item
  and the reason it exists at all. V22's plan was to author a flat open spawn so
  the ground plane's near portion would show; measuring the geometry first
  established that no scene could do it. *Unlocks:* V22, which was blocked on a
  trade this resolves, and with it V19 4c and 4d behind it.

    **The measurement, because the item is a consequence of it rather than of a
    preference.** `draw_ground` runs the plane from `ground_horizon_y` to the
    bottom of the window unconditionally, so the plane is never partly drawn and
    "not visible at spawn" meant the world was occluding **100%** of it.
    `Camera::follow` then centred strictly, with no vertical term, so wherever
    the view is unclamped the player is pinned at screen centre and the plane's
    band below them **cannot exceed ~50% by construction**; at the spawn, where
    the camera sits at its bottom clamp, it measured **20.2%**. The reference
    wants two thirds. **Three rounds of value tuning had been aimed at pixels
    the plane never drew**, which is the whole explanation for why they went
    nowhere, and no fourth round could have worked either.

    - **The trade this had to resolve, and the reading that dissolved it.**
      Anchoring the player low enough to match the reference leaves ~55 cells of
      world below them, which starves digging — the same collision as the
      near-foreground-silhouette refusal, in a new form: *the reference composes
      a scene you look across, and this is a world you dig down into.* It was
      filed as an open decision with a recommendation to split the difference.
      **The answer came back "match the reference, and let digging move the
      camera"**, and
      [notes/reference_observations.txt](notes/reference_observations.txt) entry
      10 is the reading that supports it: across the three frames the subject
      sits at 0.60, 0.36 and 0.27 down — but **the near volume holds the lower
      55-65% in all three.** What is constant is the composition, not the
      subject; the subject rides on the edge of whichever volume the frame is
      about, and rises when that volume goes underneath it. **The two framings
      were never in conflict — they are one rule in two states**, and the state
      is chosen by what the player is doing.
    - **So the anchor is a fraction of viewport height, not a bias in cells**,
      and that is the one design decision here worth defending on its own. What
      is being matched is a composition; a cell count expresses it only at the
      viewport height it was tuned at, and every other entry in `DISPLAY_MODES`
      would get a different picture from the same constant. `Camera::follow`
      already computed `center_y - viewport_h / 2`, so the anchor is that `/ 2`
      becoming a variable, and **0.5 is exactly the old expression** — which is
      what made a genuine no-op half available.
    - **The one real trap, and it is a feedback loop rather than a tuning
      problem.** The anchor moves the view; the view is what `screen_to_world_y`
      resolves the mouse through; the resolved cursor is what `Input` carries
      into the step; the aim is what picks the anchor. That closes, and it
      closes **positively**: digging downward raises the view, which slides the
      world under a stationary hand further down, which reads as a steeper dig.
      It saturates rather than oscillates, so it would not have looked like a
      bug — it would have looked like the camera crawling away under a still
      hand. **Cut by measuring the aim in the unbiased frame**, where the
      anchor's own term cancels exactly rather than being damped and hoped
      about. The suite simulates a held mouse at two anchors to check it, **and
      carries a negative control proving the uncorrected form fails that check**
      — without which the test passes for a reason nobody has verified.
    - **The golden checksum moved for the seventh time, to `0xf29c435ed9d923b1`,
      and this one had its no-op half and took it.** The mechanism shipped in
      its own commit at the 0.5 default and ran against `0xcde4dc1a39927fca`,
      which held; the step between the two numbers is the fixture adopting the
      shipped anchor and nothing else. **The fixture adopts it deliberately**: a
      hash over a configuration the game never runs covers nothing that ships,
      which is the null-texture lesson in a second form.
    - **Nothing was invalidated, and that was checked rather than assumed.**
      `Input::cursor_x/y` are stored in **world cells**, converted in `main.cpp`
      before the step, so a camera change cannot change what a recorded log
      replays — both `.rec` sessions survive, and P4's row stays lit. The scene
      is untouched: launch prints `334901`, matching `FIXTURE_SCENE_CELLS`.
      **This is the cost V22 was going to pay and this item does not**, which is
      worth noting before V22 resumes and pays it anyway.
    - **What is not settled is the feel, and it is the majority of the item's
      risk.** The two anchors are read off still frames; `EASE_PER_SEC` is read
      off nothing at all. **A still image cannot say how fast a camera should
      move**, and the failure at both ends of that constant has the same name
      for opposite reasons — too fast is a cut, too slow never composes either
      framing. Owed to the tester as its own checklist step.

### V10 — The in-window UI layer: a dig reticle and a material hotbar

- [x] **V10 — The in-window UI layer: a dig reticle and a material hotbar.**
  *(done — see [src/ui/hotbar.cpp](src/ui/hotbar.cpp); screen shake and hit-stop
  are split out to **V10.1** below and are the only part still open)*
  *Observed:* [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 1, observations B1 and
  B2. *Unlocks:* the first player-facing UI in the project, and the reason it is
  here rather than in Presentation & Tooling is that everything in that section
  is dev-facing by its own preamble. A reticle the player aims with is not a
  debug convenience.
    - **Both halves came out of a playtest as feature requests and neither is
      taken at face value, because the stated fix and the real finding differ in
      both cases.** The reticle was asked for as four non-intersecting ticks;
      the finding underneath is that the current marker is a filled one-cell
      rect in the same orange family as Fire, so it vanishes against the one
      thing you most want to aim at. Contrast and shape are the requirement, and
      the tick design satisfies it. The hotbar was asked for because selecting
      materials felt slow — **and a measurable part of that was defect A2, a HUD
      that rebuilt once a second and showed a stale material for up to a full
      second after the key was pressed.** That is fixed, and it is a caution
      worth keeping: a chunk of this request was a rendering lag being read as
      an interaction problem. What remains is real, since eight materials behind
      eight number keys with no visible affordance still has to be memorised.
    - **Both halves of the UI are now built; what is left in this item is the
      two game-feel effects at the bottom of it.** The reticle shipped first, on
      its own. The hotbar is `src/ui/hotbar.h` / `.cpp`: a row of eight 8x8
      hand-authored icons, bottom centre, the selected slot framed in its
      material's own colour. **Ordered by `MoveKind` rather than by the order
      materials were added** — structural solids, powder, liquids, gases, eraser
      last — so neighbouring keys behave alike and a mis-hit lands near what was
      meant. That moved the eraser from `4` to `8`, which is the one binding in
      this project anyone had already learnt, and it is worth watching for in
      the next playtest rather than leaving to be discovered.
        - **The table moved into `src/ui/` and the keydown switch in `main.cpp`
          became a loop over it.** That was not tidying. Two lists — one saying
          key 5 places Oil, one drawing an oil icon over the fifth box — can
          disagree, and the failure is silent and worse than no hotbar at all,
          because the affordance is then actively lying. One table is read by
          both.
        - **Sized off `DisplayMode::ui_scale()`, which this entry predates.**
          Authored in fixed pixels it is a postage stamp at 3440x1440, which is
          the mode the art is measured against.
        - **One departure from "tinted from `MATERIALS`", stated rather than
          hidden:** Oil is `0xFF2C2620` and Charred is barely lighter, and both
          sit on a dark backing panel, so painted faithfully they are black
          squares — technically correct and functionally absent. A base below a
          brightness floor is lifted to it proportionally, keeping the hue.
          Every other row passes through untouched.
        - **Three shapes were changed after looking at them at real size rather
          than at the ASCII art**, which is the argument for authoring them as
          ASCII in the first place: the eraser's inner ring was in the shadow
          tone and vanished against the panel, Oil's sheen was two detached
          pixels that read as noise, and Oil's first silhouette was a mound that
          collided with Sand's.
    - **Built as** an icon atlas in `src/ui/`, laid out exactly like the bitmap
      font already there — hardcoded small bitmaps, tinted per material from
      `MATERIALS` so an icon can never drift from the palette it depicts. Not
      drawn with renderer primitives: an afternoon of fighting `SDL_Renderer`
      yields a worse pyramid than forty bytes of hand-set bits, and the tinting
      is what keeps this honest when V6 locks the palette.
    - **Sequenced after the simulation work rather than before it, for one
      concrete reason:** an icon depicts behaviour, and E9 changes what steam
      *does*. Authoring a steam plume against the current puff means authoring
      it twice.
    - **This does not do a menu, a settings screen, or a font beyond the one
      that exists.** Naming that here because a "UI layer" is the single easiest
      item in this document to let grow, and the bound is that everything in it
      is visible during play and costs no interaction to reach.
    - **Screen shake and hit-stop were scoped here and are split out as V10.1
      rather than carried as an unchecked bullet under a done item.** The two UI
      halves shipped and the two feel effects did not, and a `[ ]` box on a
      mostly-built item is exactly the state that made this document need a
      bookkeeping pass in the first place.

### P2 — Re-baseline the benchmark at the size the game actually runs

- [x] **P2 — Re-baseline the benchmark at the size the game actually runs.**
  *(done — 2026-08-10; table in [PERFORMANCE.md](PERFORMANCE.md))* `grid_bench`
  measured 960x540 and the game runs **1920x1080** — four times the cells, and
  nothing measured it. **The item inverted while nobody was looking:** 960x540
  was written down as the target the played size had yet to reach, and the Noita
  rescale took the world past it, so the benchmark became the *smaller* number
  and every budget in `PERFORMANCE.md` was quoted against a world a quarter the
  size of the real one. Both sizes now run; 960x540 stays as the historical
  series and as a control, with every scenario constant written to reproduce its
  old literal value exactly at that size.
    - **The stale comment was the cleanest evidence of how it inverted.** The
      bench justified its size as "1920x1080 at a 2px scale = 960x540 cells".
      `Camera::SCALE` is 4, not 2 — and since F3.1 decoupled world size from
      window size, `GRID_WIDTH`/`GRID_HEIGHT` are a cell count that no scale
      factor applies to at all. The arithmetic was true of a world that was the
      window, and it went on reading as a justification for two architectures
      after that world stopped existing.
    - **The finding is better than the item expected, and it is `sparse` at
      1.00x.** An ordinary gameplay frame — large static world, small patch of
      action — costs the *same* at 1920x1080 as at 960x540. `settled` is at the
      noise floor in both. **The engine pays for awake cells, not for cells**,
      so the size of the sleeping majority is free; there is no single "4x" to
      apply to a row, and the ratios run from 1.00x to 7.54x.
    - **Two scenarios are over budget at the played size and neither is a
      regression:** `churning` 211% and `cascading` 241%. That is what the
      played world has cost since the rescale; it had simply never been
      measured. Nothing got slower.
    - **Two measurement defects were found and fixed on the way**, both
      invisible while the bench ran a single size. The stepped scenarios kept
      their cadence in a function-scope `static int tick`, so a second run of
      the same scenario inherited the first's counter and was not the same
      scenario. And `shattering` needed a witness: fracture fires only on
      landing, a slab at 1920x1080 falls twice as far while the clear cycle
      stays at 60 steps, and "the slabs are wiped mid-flight and nothing lands"
      would have produced a confident number about a feature that ran zero times
      — which is exactly how E3's first measurement failed. `piece_tag` is
      counted untimed after the clock and reads 2,384 / 2,348, so it does land
      and does break.
    - **It also put a question mark over P1's grading criterion**, which is
      recorded in `PERFORMANCE.md` next to the prediction rather than here: that
      criterion rests on `churning` fitting in cache, which was observed at
      960x540 and cannot be assumed at four times the size.

### P4 — A benchmark scenario that is a real frame

- [x] **P4 — A benchmark scenario that is a real frame.** *(built and first
  session recorded 2026-08-13 — numbers and method in
  [PERFORMANCE.md](PERFORMANCE.md), how to record one in
  [README](README.md#the-replayed-row-and-recording-one-p4))* **This item
  retires an argument rather than a defect.** All seven scenarios in
  `grid_bench` are hand-built, and the plan has twice had to decide which of
  them counts as a realistic frame — most recently over `churning` at 211%,
  where "call it synthetic" was the convenient answer and sand sinking through
  water is the most ordinary thing a player does in a falling-sand game. **That
  argument is unwinnable by classification and does not have to be had:** F2.3
  made a run a seed plus a replayable list of inputs, so a row that *is* a
  played frame can simply be recorded.
    - **The design decision worth carrying: recording is always on, and `F9`
      saves.** The obvious build — `F9` starts recording — cannot work, and
      finding out why took one paragraph rather than an afternoon only because
      the fingerprint check was designed first. A log must begin at a world the
      replay can rebuild, and the only such world is the fixture scene before
      the first step. A recording started two minutes in would replay from the
      fixture into inputs that assume two minutes of dug tunnels, which is
      precisely the "silently measures nothing" failure the item exists to
      remove. Cost of always-on: one 24-byte `Input` per fixed step, ~1.4 MB an
      hour, capped at half an hour.
    - **The staleness trap is closed by construction rather than by a note in a
      file.** The log carries the seed, the fixture's placed-cell count and a
      fingerprint of every cell before the first step; the bench rebuilds the
      world and **refuses** on a mismatch. `scene_test` pins the fixture count
      too, so a changed scene fails in `ctest` on the commit that changed it
      rather than in a benchmark nobody runs that day. **A changed simulation is
      the third case and is deliberately not refused** — it moves the end state
      legitimately, the bench cannot distinguish it from a stale log, so it
      reports what it saw. An instrument that guesses which of two
      indistinguishable things happened is worse than one that says it cannot
      tell.
    - **It reports four statistics because a mean is the wrong one for a
      budget.** Mean, p99, worst step, and how many steps exceeded 16.67 ms. A
      session that sleeps through 95% of its steps and spends the rest at 40 ms
      stutters and has an excellent mean.
    - **It cost a refactor that was not in the estimate, and the refactor is the
      better half of the item.** The scene loader was 90 lines of `SDL_LoadBMP`
      inside `main.cpp` — so the world the game actually boots into could not be
      built by anything headless, which is why no test had ever loaded the
      shipped fixture and why V2's blank world went unnoticed for a commit. It
      is now `src/scene/bmp.cpp`; `main.cpp` calls it and its SDL path is
      deleted. **One reader, not a headless one beside the SDL one** — two
      implementations of one question is the shape of D1's two clocks and F6's
      two range tests, both of which drifted, and this is the first time that
      lesson has been applied *before* the drift. Checked by the count
      `README`'s launch step already pinned — 334,901 cells, unchanged,
      independently reproduced by `tools/pixel_art.py`'s reader.
    - ~~**What it owes: a played session, which the builder cannot supply.**~~
      **Supplied 2026-08-13.** 24,437 steps, 407 s of play: **mean 0.1212
      ms/step, p99 1.4745 ms, worst step 4.8193 ms, 0 steps over budget**,
      replayed to the recorded end state exactly. Both questions closed — see
      `ROADMAP_ITEMS.md`'s decisions table and `PERFORMANCE.md`.
    - **The number corrected the rule this item was built to serve, which is the
      outcome worth carrying.** The frame-budget rule was to be stated against
      this row *instead of* the synthetic ones. But a played session costs 0.12
      ms a step, so "under 10% on the replayed row" is twelve microseconds —
      under the noise floor, on a row with three times the budget spare. **A row
      that is realistic enough to settle a budget is, for that same reason, too
      quiet to price a per-cell change.** Both kinds of row are kept and given
      separate jobs. The item still succeeded at what it was for: the argument
      about which hand-built scenario counts as realistic is over, and
      `churning` turned out to be neither a defect nor an artifact but a real
      activity type at an intensity nobody sustained — 360 of 510 chunks awake
      against the session's 129.
    - **Then its own census overturned the reading of its first session, within
      the hour, and this is the part to carry.** The row was justified — here
      and in `ROADMAP_ITEMS.md` and `PERFORMANCE.md` — on the argument that a
      *played* frame is realistic **by construction**, which is true and which
      was silently treated as meaning representative. A census was added to the
      row (inputs counted exactly, world sampled once a second in a second
      untimed pass) and reported that the session **never dug once, never moved
      a grain of sand or a cell of water, never contained steam, and peaked at
      16 of 510 chunks awake**. It is a painting session. **Realistic by
      construction; representative only by evidence** — two properties this
      item's entire argument ran together, and no amount of "but it was really
      played" would ever have separated them. Only counting what was in it did.
      The budget rule stands (16 of 510 awake makes the case that a played row
      cannot price a per-cell change *stronger*); the `churning` question
      reopens; a second session is owed.
    - **And a result nobody asked it for: 24,437 steps of real play replay
      byte-exact.** `test_run.cpp` proved that for short synthetic sequences;
      this is seven minutes of a person digging, pouring and walking, rebuilt
      from a seed and an input list onto the same world fingerprint — the first
      end-to-end evidence that F5's and F6's fixed-point conversions hold over a
      long run rather than over a test.

### S0 — The run can be lost

- [x] **S0 — The run can be lost.** *(new 2026-08-09, and it is the item that
  changed the running order; **built 2026-08-14**)* *Observed:* fire is fully
  simulated and cannot hurt the player, there is no failure state, and there is
  nothing in the built game to do. *Unlocks:* the two questions immediately
  above, by playing rather than by argument — which is the only way this project
  has ever settled a design question, and which is currently blocking every item
  in this section.

    **What shipped, against what the entry below promised: all of it, and
    nothing else.** Health on `Player` (`MAX_HEALTH`, `health()`, `is_alive()`);
    damage from contact heat and from landing too fast; one objective placed in
    the fixture scene by `main.cpp`; reaching it wins and dying loses, both
    through `Run::reset(seed)`; an `HP` readout on the HUD that already existed.
    The refusal list held — no generator, no save file, no second damage source,
    no death presentation, no UI beyond the readout. Tests are in
    `test_player.cpp` (the two damage rules, both sides of each) and
    `test_run.cpp` (outcomes, the objective, reset). **Both recorded sessions
    still replay byte for byte**, which is the claim that matters most: S0 reads
    the grid and the body and writes neither, so the simulation is untouched.

    **Five things the entry did not predict, and the first is the one worth
    carrying.**

    - **The spawn drop is a fall, and it was the hardest fall in the game.**
      `Run` puts the body a quarter of the world's height up in open air on
      purpose, and priced by the rule as written that is terminal velocity — 80
      of 100 health — before the player has touched a key. Nothing in this item,
      in the full "Player health and death" item, or in `player.h`'s spawn
      comment saw it coming, and it is obvious the moment it is stated. The fix
      is one bool on `Player` (`has_landed`): the first landing of a run is
      free, spent there rather than by moving the spawn, which `Run` cannot
      place on terrain it does not know about. **The generalisable form: a
      damage rule prices everything the world already does to the body,
      including the things it does before the run starts.** Every later hazard
      inherits this — E6's explosions, S1's contact damage — and each will have
      its own version of "something that was free is now charged".
    - **The obvious landing test does not detect landings, and this was measured
      rather than reasoned.** The natural way to write fall damage is to read
      `move_y` reporting a block. A body falling six cells a step onto a floor
      exactly six cells below walks the whole distance unblocked, ends the step
      flush on the ground with its velocity intact, and has `vel_y` quietly
      zeroed by the resting rule on the *next* step — so the landing never
      happens and the damage is zero. The correct edge is `on_ground` going
      false→true. **Written the obvious way, terminal-velocity falls do no
      damage whenever the arithmetic comes out even**, and the test that catches
      it was run against both forms: 100 health against the naive version, 20
      against the correct one. That is the same shape as D1's two clocks and
      F6's two range tests — a quantity with two plausible spellings that agree
      almost always.
    - **A restart would have silently invalidated P4's session log, and the fix
      is that a restart starts a new one.** A log replays by rebuilding the
      world from the seed and the scene and feeding the inputs back; a log
      spanning a `reset()` would replay into a world two minutes of play deep,
      and `grid_bench` cannot tell that from a stale log — which is precisely
      the "silently measures nothing" failure P4 exists to remove. **Why the new
      log is valid is asserted rather than argued**: reset on the same seed plus
      a re-stamp of the same scene reproduces the starting world exactly,
      checked on the fingerprint in `run_test`. The alternative — giving `Run`
      the scene so `reset()` could restore it — is the better answer when saves
      arrive and is recorded in `ENGINEERING_NOTES.md` rather than built here.
    - **The objective is a point in the world, not a cell**, and that is a limit
      rather than a shortcut. A cell means a row in `MATERIALS`, which means
      answering what happens when it is dug, burnt, displaced or buried — four
      questions the full item below exists to answer and which a spike has no
      business deciding by accident.
    - **It is ~740 cells from the spawn and the viewport is 480 cells wide, so
      it starts off-screen.** Without something pointing at it the run is "walk
      east until you find it", which is not a difficulty, it is a missing
      instrument — the same failure the free camera keeps being scheduled
      behind. Answered inside the item's own limit: a bearing (`GOAL:740E`)
      appended to the readout that already exists, not an arrow at the screen
      edge.

    **Two things it did on purpose that are worth reading as design and not as
    detail.** The objective sits east of F4's water channel, which is walled on
    both sides and full to within 175 cells of the top — **a traverse the
    character cannot walk**. Flight has been shipped since before this item and
    nothing in the built game had ever asked for it; this is what asks. And
    **death is checked before the objective**, so a body that reaches the marker
    on the step its last health goes has lost. That is an arbitrary call between
    two things that cannot both be true, written down at `Run::step` so it is a
    decision rather than an accident of ordering.

    **Two exceptions to "reset gives you a fresh run" now exist, and the second
    is this item's.** `Grid::vent_radius` survives because it is configuration;
    `Run`'s objective survives because it is a property of the *level*, and the
    caller re-stamps the same scene on reset — a cleared objective is one the
    caller has to remember to place again, and forgetting produces a run that is
    unwinnable and says nothing. Both are asserted in tests rather than left as
    prose.

    **What it could not answer, and this is the item's whole point rather than a
    shortfall.** The combat decision is due at the end of S0 and the hook at the
    end of E6; both are answered by playing and neither can be settled from the
    desk. **The playtest is owed** — the checklist gained a step 10 for it.
    Performance: this sitting could not price the change and says so, because
    every synthetic row in `grid_bench` — including the ones that never
    construct a `Player` — ran 2.2–2.9x above `PERFORMANCE.md`'s table, and the
    replayed row's 2.5x sits inside that band. What the sitting *does* establish
    is that the budget is intact at 0 of 20,415 steps over it even on a machine
    running two and a half times slow. **No number from it goes in
    `PERFORMANCE.md`.**
    - **It is the thin half of "Player health and death" and "Objective +
      Extraction", pulled forward and built now.** Health; damage from fire,
      whose temperature the engine already supplies, and from landing too fast,
      whose speed it already supplies; one hard-coded objective placed in the
      test scene; reaching it ends the run as a win and dying ends it as a loss,
      through `Run::reset(seed)` and not a second code path. A health readout on
      the HUD that already exists. **That is the whole of it.**
    - **What it explicitly is not:** no generator, no save file, no pet agent,
      no economy, no second damage source, no death presentation, no UI beyond
      the readout. Those stay in the full items below and are not started here.
      **The discipline that makes a spike safe is that it names what it is not
      building**, and this bullet is that.
    - **Why it is admitted ahead of most of the engine tier, stated against
      `VISION.md` rather than around it.** That document's Scope Discipline
      guards one direction — building the wrong things — and says outright that
      it does *not* defend against the slice being too thin to be fun, that
      under-building looks exactly like discipline until the playtest gate, and
      that this is the one place the cutting reflex is worth checking.
      Everything shipped to date is engine or visual foundation. This is that
      check, taken at a cost of about a week, and it is the cheapest available
      answer to "the game feels underdeveloped and without direction" — which is
      an observation about the built game and therefore the same class of
      evidence the E track admits items on.
    - **Keep the direction of the dependency.** This is the first gameplay
      system spanning both sides of the rule `tool.cpp` established: the grid
      does not know about bodies, bodies read the grid. Damage is the player
      *asking* what it is standing in, the same way collision does. It is not a
      health field on `Grid` and not a damage column on `Element`, and it will
      be tempting to make it one.

### Pick the UI layer, in writing, before anything needs one

- [x] **Pick the UI layer, in writing, before anything needs one.** Decided and
  recorded in `ENGINEERING_NOTES.md`: immediate-mode, drawn directly against
  `SDL_Renderer`, no new dependency — a library (Dear ImGui, Nuklear) loses for
  the same reason PNG lost to BMP in F4, and hand-rolled retained-mode loses for
  the same reason an ECS is off the table, both recorded in full there rather
  than repeated here. The one gap immediate-mode against `SDL_Renderer` opens on
  its own is text, since the renderer draws no glyphs — closed with a small
  hand-authored 3x5 bitmap font (`src/ui/text.h`/`text.cpp`, SDL-side and
  outside `ENGINE_SOURCES` like `main.cpp`), covering A-Z, 0-9 and a handful of
  punctuation rather than the whole ASCII table. *Verify, done exactly as
  written:* the frame-rate/brush/awake-chunk readout that the window title bar
  carried alone now draws inside the game window itself with the new font, over
  a translucent backing rect for legibility against whatever the simulation is
  doing underneath — the window title returns to being a plain, static label.
  This is the drawing primitive the health readout, pet agent panel and
  resolution options will each build their own screen on; none of those three
  screens exist yet.

### World reset hotkey

- [x] **World reset hotkey.** *(shipped 2026-08-14 as `T1.4`, on `Ctrl`+`R`
  rather than `R` — see [T1](ROADMAP_ARCHIVE.md#t1-the-debug-tooling-batch))* Wipes the grid back
  to Empty and respawns the player, so a messy test scene can be cleared without
  relaunching the exe. `Grid::reset(seed)` and `Run::reset(seed)` are built and
  tested in **F2.2**, so by the time this item comes up it is the key binding
  and a seed to pass — the current world's own seed keeps a debugging session
  reproducible; a freshly drawn one is the other reasonable choice, and this is
  where that gets decided.

### Pause and single-step (`P` to toggle, `.` to step once while paused)

- [x] **Pause and single-step (`P` to toggle, `.` to step once while paused).**
  *(shipped 2026-08-14 as `T1.1`; the brush-while-paused question is answered by
  construction and the third state is still open — see
  [T1](ROADMAP_ARCHIVE.md#t1-the-debug-tooling-batch))* Freezes the fixed-step loop so a collapse
  or a reaction can be inspected one frame at a time instead of flying past. The
  one thing to get right: `accumulator` must not keep growing while paused, or
  unpausing dumps a burst of queued steps at once. F2.4 confirmed the shape and
  left one policy question open on purpose: whether the brush still paints while
  paused, since a pause that also freezes editing forecloses using pause *to*
  set up a scene precisely.

### Cell inspector

- [x] **Cell inspector.** *(shipped 2026-08-14 as `T1.3`, on `I`; it found the
  false claim on `Grid::active_chunk_count()` the first time it was used — see
  [T1](ROADMAP_ARCHIVE.md#t1-the-debug-tooling-batch))* Extend the in-window HUD with the material
  name — and now the temperature and piece tag — under the cursor via
  `grid.get_element(gridX, gridY)`. Cheap, and E2 and E3 both added per-cell
  state that is currently invisible and was debugged without it.

