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
table](ROADMAP_ITEMS.md#-decisions-owed). Brush-while-paused is answered the
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
