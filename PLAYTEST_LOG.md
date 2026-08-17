# Playtest Log

Session records from running the Manual Tester Checklist ([MANUAL_TESTING.md](MANUAL_TESTING.md) — it lived in README.md until 2026-08-16, which is what the older entries below mean when they say so), and the supplementary checklists below, against a real build.

**This file holds questions and answers. Nothing else.** What was asked at a session, what came back, and in whose words — that is the whole of it. Everything downstream of a finding lives in [ROADMAP.md](ROADMAP.md): why it happened, what was done about it, what that cost, and what the fix got wrong on the way. A finding here is a symptom and a status, and the status is a link.

**That split replaced an earlier one that did not hold.** This file used to carry the order of work at the bottom, on the reasoning that a roadmap absorbing every playtest finding stops being readable as a statement of intent within two sessions. The reasoning was right about roadmaps and wrong about the remedy: two documents each holding part of the sequence meant neither was authoritative, and the wave table here went a full session describing the material hotbar as queued after it had already shipped. **The sequence now lives in one place**, as [Waves](ROADMAP.md#-waves--sub-plans-that-preempt-the-tracks) — which is the mechanism that lets a roadmap absorb twelve findings without interleaving them into its tracks.

**What a session entry records:** the build, **the world seed**, the `Scene:` line, the suite count at the time, the per-step or per-row result, and what was found. The seed is not optional bookkeeping — F1 made the simulation a pure function of its seed and step count precisely so that "it looked wrong" converts into a case someone else can reproduce. **A finding without its seed is an anecdote.**

**Findings are of two kinds and they convert into work at wildly different rates.** A **defect** is a fact: something did the wrong thing, and it goes in a table with a severity. An **observation** is a feeling about how something reads — real data about an experience, but the fix suggested alongside it is a hypothesis rather than a specification, and it only becomes a tracked item after being argued for on its own merits. Session 1 produced eight defects in three minutes and four observations that between them are most of a milestone. **The two are never mixed in one table.**

**Bring screenshots, especially of states nobody would think to describe.** The A7b defect was only findable because images showed sand rather than the liquid the written report was read as describing, showed the shelves were one cell thick, and showed a black wake that turned out to be a second, unrelated defect no amount of re-reading the text would have produced. **A screenshot is worth asking for whenever a defect is about what something looks like rather than what it does** — it carries the things the reporter did not know were relevant.

**A check whose answer a human cannot produce is not a check.** Session 3 asked the tester to watch one spot and judge whether the lit cells were a *different* set second to second or the same set shifted; the honest answer was "I'm not sure", and at 165 fps nobody can hold two frames of a flame band in their head and diff them. That is the checklist's fault, not the tester's, and the failure is silent because **an "I'm not sure" is indistinguishable from a pass**. Questions like that need an instrument, and instruments belong in `tests/`.

---

## Session 1 — 2026-08-02

- **Build:** `313aa94`, Release, MSVC
- **World seed:** `18164811273671827879`
- **Scene:** `Scene: 640x400, 27192 cells placed`
- **Suites at time of test:** 6/6 green, 199 checks
- **Result:** 9/9 steps executed. No test blockers — the pass completed without a fix being applied mid-session, which is why the later steps are trustworthy.

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

Severity: **major** = wrong behaviour a player will hit in normal play; **minor** = wrong but cosmetic or narrow.

| ID | What was seen | Severity | Status |
|----|---------------|----------|--------|
| A1 | Player rectangle jitters and ghosts while moving. Collision itself feels correct in every direction. | major | fixed — [wave 1](ROADMAP.md#wave-1--the-rendering-brush-and-powder-defects) |
| A2 | HUD lags material switches by up to a second — the key press takes effect immediately but the readout does not. | minor | fixed — [wave 1](ROADMAP.md#wave-1--the-rendering-brush-and-powder-defects) |
| A3 | Wood burns away far too fast. | major | fixed — [waves 2b/2c](ROADMAP.md#waves-2-and-2b--fire-rebuilt-on-fuel-holds-the-timer) |
| A4 | Fire will not propagate along a horizontal wood beam. Alongside a vertical beam it works. | major | fixed — [waves 2b/2c](ROADMAP.md#waves-2-and-2b--fire-rebuilt-on-fuel-holds-the-timer) |
| A5 | Steam condenses back to water far too fast. | major | ✅ **closed 2026-08-13** — fixed 2026-08-12 as the steam half of E9, **confirmed in play** by the [two-step spot check](#spot-check--2026-08-13--the-two-owed-steps-run-together). See B3 and D5, the same report again |
| A6 | Spawning material into water caps the water on top, then it bursts outward on release. | major | **fixed and confirmed** — [wave 3](ROADMAP.md#wave-3--the-brush-destroyed-water-and-the-elevator-it-was-hiding), session 5 W-1/W-2 |
| A7 | Falling liquid throws horizontal sticks under 10 cells wide. | minor | fixed — [wave 1](ROADMAP.md#wave-1--the-rendering-brush-and-powder-defects) |
| A8 | Material carves authored background out of whatever it passes through, leaving a permanent black wake. | major | fixed — [wave 1](ROADMAP.md#wave-1--the-rendering-brush-and-powder-defects) |

### Follow-up reports, same day

Three reports came back after fixes shipped, and each is logged as its own finding because each was a distinct observation rather than a restatement.

| ID | What was seen | Severity | Status |
|----|---------------|----------|--------|
| A1b | After A1's fix: the player bobs into and out of the floor on a ~0.3 s cycle, and can phase about three quarters of a cell into walls, sand and wood. | major | fixed — [wave 1](ROADMAP.md#wave-1--the-rendering-brush-and-powder-defects) |
| A7b | After the liquid fix: a pouring **powder** fringes itself with one-cell-thick horizontal shelves. Reported with screenshots (`resources/video_screenshots/`) of a sand pile spiked with horizontal protrusions "in <10 pixel wide sticks". | major | fixed — [wave 1](ROADMAP.md#wave-1--the-rendering-brush-and-powder-defects) |
| A7c | After the shelf fix: sand now stacks into perfect vertical columns, and grain motion reads as stepping rather than flowing. Then, after a speed change: sand falls in sheets with a one-cell gap between them. | major | fixed — [wave 1](ROADMAP.md#wave-1--the-rendering-brush-and-powder-defects) |

**A7b is the entry to read if you are deciding whether to ask for an image.** The written report was accurate and was read as describing the liquid defect that had just been found, because that was the mechanism already in hand. The screenshots showed it was sand — a different code path entirely, which the liquid fix could not have touched — and showed a second defect (A8) nobody had reported.

### Observations — feelings, not defects

Each names a real problem. None is a specification, and the fix suggested alongside is the reporter's hypothesis, recorded as stated.

- **B1 — the dig marker is hard to read.** Requested as an open crosshair (four non-intersecting ticks) that follows the cursor everywhere and dims outside dig range. *Tracked as V10, first half — built.*
- **B2 — selecting materials is slow.** Requested as a hotbar with per-material icons and key numbers. *Tracked as V10, second half — built. Part of this observation was defect A2 rather than an interaction problem, which is worth knowing before taking the next feature request at face value.*
- **B3 — steam should collect, wait, then drip increasingly fast and shrink as it goes.** *Tracked as the steam half of E9 — built 2026-08-12, ✅ **confirmed in play 2026-08-13** ([spot check](#spot-check--2026-08-13--the-two-owed-steps-run-together)): it collects, waits, and drips. This and defect A5 are the same fix seen from two sides, and D5 is the third time it was reported.*
- **B4 — rigid bodies should tip, topple and roll.** The observation underneath is that bodies falling flat and landing flat read as lifeless. *Tracked as E8 — open, and scoped to toppling only; the rolling half is deferred on cost for reasons in that item. (It used to be deferred "behind E5"; E5 split on 2026-08-09 and E5a is not an off-grid layer, so the dependency was withdrawn — the deferral is now on the price of a solver.)*

### Follow-up — fire measured against reference footage

Not a play session: seven frames of a Noita scene burning, spanning 1–2 seconds of video, so roughly 10–20 simulation steps between frames. That ratio is what makes the sequence readable at all.

**Caveat on measurement.** The frames are different pixel widths (714–812), so they are crops or rescales from a video player rather than a fixed viewport. **Nothing positional compares across frames.** Every observation below is measured *against a scene feature* — flame height against plank thickness, burn front against the plank it sits on — and the ones that would have needed absolute pixels were dropped rather than estimated.

| # | Observation | Consequence for this engine |
|---|-------------|------------------------------|
| 1 | The plank is still there under the fire. Flame occupies the **air around** the fuel; a gap only appears after sustained burning. | We convert `Wood → Fire`. The fuel *is* the flame. Wrong at the root. |
| 2 | Flame contents change **completely** between frames — not shifted, a different set of lit pixels. | Flame cells live ~5–15 steps. Ours lived 180. |
| 3 | The burn front advances about its own width in 1–2 s. | Fuel lasts seconds; flame lasts a moment. **Two numbers on two cells.** |
| 4 | Flames rise, detach as embers, fade out. | Directionally what we do, at the wrong scale. |
| 5 | Colour ramps white-hot at the fuel → orange → dim red at the tips. | One row plus jitter cannot produce this. Needs age. |
| 6 | Fire is a layer hugging the fuel surface; interiors never burn. | Falls out for free if burning cells emit into *empty* neighbours. |
| 7 | Wood darkens to char before disappearing. Three states, not two. | We have two. |
| 8 | A large soft glow lights the cavern tens of cells out. | **Not simulation.** V7. |
| 9 | Smoke haze above the fire. | No such material. |

**The finding is 2 and 3 together, not either alone:** fuel duration is long, flame duration is short, and they are quantities on different cells. What the engine did with that is [wave 2b](ROADMAP.md#waves-2-and-2b--fire-rebuilt-on-fuel-holds-the-timer), including the three gaps this comparison opens that are not fire problems and are deliberately not scheduled against it.

---

## Session 2 — 2026-08-02 — fire, seen for the first time

The playtest [wave 2b](ROADMAP.md#waves-2-and-2b--fire-rebuilt-on-fuel-holds-the-timer) was waiting on. Seven notes across two rounds, then an eighth on a second look at the lighting alone.

**Every note is a look rather than a behaviour**, which is itself the result: nothing in this session says the simulation did the wrong thing. The fuel/flame rebuild's *model* survived contact and only its numbers, its palette and its lighting did not. The second round came back on the lighting with the burn and flame rates pronounced good.

| # | Note, as reported |
|---|-------------------|
| C1 | Wood burns ~20% too fast. *(Ambiguous as asked — burn duration or spread rate? Answered by the fix: spread rate.)* |
| C2 | Flames move ~10% too fast. |
| C3 | Burnt wood is jet black; it should read as charcoal. |
| C4 | Burnt wood should persist about one second longer. |
| C5 | Flames have a hard height cutoff. |
| C6 | Uniform flame height looks unnatural. |
| C7 | Flame colours lack intensity. |
| C8 | **"The lighting effects look completely blown out and over exposed... the crazy flare effects are bad."** Reported from a screenshot (`resources/video_screenshots/bugged_fire.png`) — a flat yellow-white wash with the terrain invisible inside it — with the rest of the fire pronounced good. |

**C5 and C6 were reported as two notes and are one observation**, which is worth noting as a pattern rather than as a fact about flames: a single cause routinely surfaces as several complaints, and treating them as several fixes is how a model gets tuned three times instead of replaced once.

All eight are fixed. What each turned out to be, and the two that were not what they sounded like, are in [wave 2b](ROADMAP.md#waves-2-and-2b--fire-rebuilt-on-fuel-holds-the-timer).

---

## Session 3 checklist — the pass that closes wave 2b

**A supplement to README's Manual Tester Checklist, not a replacement.** Run that one first, in full: wave 2b touched `src/physics/` and the render path, so steps 1–9 all apply, and step 5 in particular was rewritten under this fire and has never been run against it. What follows is the *additional* pass, and it exists because the general checklist asks whether fire is correct and this wave's open question is whether it is any good.

**Record before starting:** build hash, Release/MSVC, world seed, the `Scene:` line, and the suite count.

### Order is load-bearing

Do phase A before phase B, and **do not skip ahead when the lit scene looks good** — an emissive layer flatters a bad flame, which is the whole reason this wave names these as two questions rather than one. If you cannot separate them by looking, the falsification is available: `preview_light` composites the same scene with and without the light pass, and `tools/rawpng.py` turns either into a PNG.

### Phase A — does the fuel/flame model read as fire?

Judge these with the glow present but ignore it. The question is the *shape and motion of the flame band*, not how bright the room is.

| # | Check | What it catches |
|---|-------|-----------------|
| A-1 | Light one end of a long **horizontal** wood beam, grounded on the sealed floor. A burn front advances along it. | The A4 defect the fuel model was built for. Session 1 measured this wrong three times by hanging the beam in mid-air — ground it, or you are testing E3. |
| A-2 | The same beam **vertical**. Front advances at a comparable rate. | Vertical used to be the only case that worked, for the wrong reason. |
| A-3 | Watch a burning beam for several seconds. **The wood is still there under the flame**, in a charred state, and a gap only opens after sustained burning. | Reference finding 1 — the defect wave 2 shipped. If fuel is vanishing into flame, the rebuild did not land. |
| A-4 | Watch one spot on the fire across a second. **The lit cells are a different set, not a shifted one.** | Reference finding 2. *(This row was a checklist defect — see the session 4 checklist for why it was deleted rather than carried.)* |
| A-5 | Look along the **top edge** of a broad fire. It is ragged, not a straight horizontal line. | C5/C6 — the jittered lifetime. A flat top means the jitter is not reaching the flame. |
| A-6 | Watch a single flame from fuel to tip. Colour runs white-hot → saturated orange → dim red, and **never passes through grey or salmon**. | C7 — the bent ramp. Desaturation mid-life means the three-stop ramp regressed to a straight line in RGB. |
| A-7 | Burn a wide wood slab and time how long the **charred** remains before it decays. | C4's open prediction: a real burn front is thinner and sleeps less than the test slab, so this should read nearer **3.62 s** than the 3.85 s measured on a probe. Trusting 3.85 is the thing this row exists to prevent. |
| A-8 | Burn a plank end to end and check it is **fully consumed**, and that fire still lights its neighbour. | The C1 margin: Charred's `heat_source` is 200 and Wood's ignition point is 150. Fire that stalls mid-plank means that gap closed. |
| A-9 | Judge the **spread rate** against C1's retune (~37% slower). Still too fast? Still too slow? | C1 was the note that reopened this wave. Answer it in the same terms it was asked. |
| A-10 | Rate the whole thing on one question: **does this read as burning?** | The actual exit condition. A-1 through A-9 can all pass on something that still looks wrong, and if they do, say so here rather than declaring the wave closed on the table. |

### Phase B — is the glow carrying it or hiding it?

Only after A-10 has an answer.

| # | Check | What it catches |
|---|-------|-----------------|
| B-1 | Look at a fire against dark terrain. **No flat white plateau**, and terrain inside the lit area is still legible as terrain. | C8's headline symptom. A wash means tone mapping is not doing its job. |
| B-2 | Look for **vertical and horizontal shafts** out of a fire — the glow is round, not diamond-shaped. | Four-neighbour propagation makes distance Manhattan. This is most likely what "crazy flare effects" named. |
| B-3 | Judge the **reach**. At `TRANSMIT_CLEAR` 0.72 the glow is visible ~35 cells out and bright within ~12; at 0.55 it is ~15 and ~8. | **A look decision and explicitly unresolved.** V7's own argument cited a cavern lit tens of cells out; a tight rim light is defensible and different. Pick one deliberately — this row is a question, not a pass/fail. |
| B-4 | Move a **single stray flame** away from a solid fire front. It should light weakly, not like a wall of fire. | The coverage-scaled emission fix. |
| B-5 | Put fire **in a pit** and behind a wall. Light stops at solids and does not leak through them. | The suites assert this headlessly, so a failure here is a compositing problem rather than a light-field one. |
| B-6 | Run the scene with **no fire in it at all** and confirm it looks exactly as it did before V7. | The reorder's stated boundary: additive only, nothing is darkened. |
| B-7 | HUD fps under a large fire, then `preview_light`'s three numbers on the same scene: **peak channel, share brightly lit, share clipped**. | The instrument exists so "looks overexposed" has a before and an after. Post-fix numbers are peak **124** and **17%**. |

### What to bring back

Screenshots, and specifically of states nobody would think to describe. **Every note in session 2 was visual**, so the default for this session is that a note without an image is incomplete.

Then: which of A-10 and B-3 got a clear answer. **Wave 2b closes on A-10 being yes and B-3 being decided — not on the tables being green.**

---

## Session 3 results — fire reads as fire, and the glow is the open work

Seed `9418527765904368373`. Scene: 640x400, 27192 cells placed.

**A-10 answered: yes.** The rebuilt fuel/flame model reads as burning. The tester's words are worth keeping verbatim because they are a tuning brief and not a compliment: *"it looks like a very flammable piece of material going up in flames and disintegrating, like a piece of paper, wool, or 'fatwood'."* Every structural check passed — fuel survives under the flame as Charred, a consumed plank still lights its neighbour, the colour ramp walks white → orange → red without passing through grey, the top edge is ragged. **What is wrong is that every timescale is a notch fast, and all four A-side findings are that one observation seen from different angles.** Wave 2b's exit condition is met.

**B-3 answered: reduce further.** `TRANSMIT_CLEAR` was already at 0.55 for this session — the tightest of the three worked examples written into `light.cpp`, the one described there as "a tight rim light". The reach is still long. **That is a real result and not a small one:** it says the reach question was never a choice between the three documented values, and the next value is off the bottom of that table.

**A-4 was not answered** — "I'm not sure". See the preamble: the row was a checklist defect and is deleted rather than carried forward.

### Findings

| # | Where | Finding, as reported |
|---|-------|----------------------|
| **A9a** | A-1, A-2 | Burn front is too linear — the boundary between lit and unlit fuel is close to a straight edge in both orientations. |
| **A9b** | A-3 | Wood is fully consumed and gone too quickly; the material reads as tinder rather than as timber. No sense of resilience. |
| **A9c** | A-7 | Charred lifetime lands near the predicted duration but should be longer. *Filed separately from A9b because it is a **measured** result where A9b is a felt one; if one knob fixes both, that is a confirmation rather than a coincidence.* |
| **A9d** | A-9 | Spread rate is still a little too fast, after C1 already slowed it 37%. |
| **B9a** | B-2 | The glow emits hard rays and shafts. Too geometric. **The headline finding of the session.** |
| **B9b** | B-3 | Reach should come down again, from an already-tight 0.55. |
| **B9c** | B-4 | A lone stray flame still lights too hard. |
| **B9d** | B-5 | Light penetrates walls, and should penetrate both Wall and Wood less. |

**Confirmed working and not to be touched while fixing the above:** no white plateau (B-1), unlit scenes identical to pre-V7 (B-6), 165 fps under a large fire (B-7), neighbour ignition across a fully-consumed plank (A-8), colour ramp (A-6), ragged edge (A-5).

All eight became [wave 2c](ROADMAP.md#wave-2c--the-glows-shape-and-the-fires-timing), where the fixes are recorded — including which of the suspected causes named at triage turned out to be wrong.

---

## Session 4 checklist — the pass that closes wave 2c

**Run README's Manual Tester Checklist first, in full.** Wave 2c changed four constants in `src/physics/` and rewrote the occlusion model in `src/render/`, so steps 1–9 all apply.

**Record before starting:** build hash, Release/MSVC, world seed, the `Scene:` line, suite count.

### A-4 has been deleted, and the reason is the useful part

Session 3's A-4 asked the tester to judge whether the lit cells were a *different* set second to second or the *same* set shifted. The honest answer was "I'm not sure", and **that is the checklist's fault, not the tester's**: at 165 fps nobody can hold two frames of a flame band in their head and diff them. A check whose answer a human cannot produce is not a check, and its "I'm not sure" is indistinguishable from a pass. **The row is not carried forward as a to-do.** If flame turnover is ever in doubt it needs an instrument — diff the flame mask between steps and report the share of cells that survived — and instruments belong in `tests/`, not in a table somebody reads at a keyboard.

### What changed since session 3, in the terms you judged it in

Session 3's notes are the control. Where you said "slightly", here is what slightly turned out to be, so you can answer *too far / not far enough* rather than starting from scratch:

| You said | Now |
|---|---|
| A-9, A-10: spread still a little too fast | **22% slower** end to end (6.5 s → 7.9 s across a 150-cell beam) |
| A-3: wood is fully gone very quickly | **50% longer** to disappear (22 s → 33 s for a body of wood) |
| A-7: charred could persist slightly longer | **39% longer** (3.2 s → 4.4 s per charred cell) |
| A-1, A-2: burn front could be less linear | **23% more ragged** — the weakest of the four |
| B-2: hard rays and shafts, too geometric | Field is round at every angle, not only on the axes and diagonals |
| B-3: reach could be reduced slightly | `TRANSMIT_CLEAR` 0.55 → 0.52 |
| B-4: stray flame should light even weaker | `COVERAGE_FLOOR` halved, 0.2 → 0.1 |
| B-5: light penetrates walls | A one-cell wall passed 44% of the light through it; it now passes none |

### Ground every test scene. This is the fourth session it has bitten

A beam hanging in mid-air is not a slow-burning beam, it is a falling one, and the probe, the A3 lifetime test and two earlier sessions have all reported gravity as a result about fire. **Build beams on posts or on the sealed floor.** A plank lying *along* a wall is the other trap and it is less obvious: a wall is a heat sink big enough to quench any fire touching it, so a beam resting on one burns against the coldest large body in the scene and will read far too slow.

### Phase A — the fire's own timing and shape

Judge with the glow present but ignore it.

| # | Check | What it catches |
|---|-------|-----------------|
| A-1 | Light one end of a long **horizontal** beam, grounded. Judge the pace against session 3's "still a little too fast" — is 22% slower now right, still fast, or has it gone dull? | A9d. **The one to be suspicious of.** Wood's conductivity fell 72 → 40 and the cliff where fire stops propagating entirely is at 34. There is very little room left below this, so "still too fast" needs a different mechanism, not a smaller number. |
| A-2 | The same beam **vertical**. Comparable rate. | Vertical was once the only case that worked, for the wrong reason. |
| A-3 | Watch the burn front along a broad beam. **Is it still a straight line sweeping along, or does it gnaw unevenly?** | A9a, and the change least likely to have landed. 23% more raggedness on a quantity that already varies 2x between worlds may simply not be visible. If it reads unchanged, say so plainly. |
| A-4 | Burn a plank end to end. **Fully consumed, and fire still lights its neighbour.** Try a beam only two or three cells thick. | The margin, and the specific regression this wave nearly shipped. A thin beam is where a fire that sometimes refuses to cross shows up; nine seeds say it never does, and nine seeds is not the world. **A stall here is serious — report the seed.** |
| A-5 | Watch a burning body for several seconds. **The wood is still there under the flame**, charred, and a gap opens only after sustained burning. | Reference finding 1. If fuel vanishes into flame the E9 model has regressed. |
| A-6 | Time how long **charred** persists, and judge the body as a whole — does wood now read as timber rather than tinder? | A9b/A9c, and the change most likely to be visible. Session 3's phrase was "a sense of resilience"; that is the word to judge against. |
| A-7 | Look along the **top edge** of a broad fire. Ragged, not a straight horizontal line. | C5/C6. Unchanged this wave — regression check. |
| A-8 | Watch a single flame from fuel to tip. White-hot → saturated orange → dim red, **never through grey or salmon**. | C7's bent ramp. Unchanged this wave; regression check. |
| A-9 | Rate the whole thing on one question: **does this read as burning, at a pace that feels right?** | The exit condition. Session 3's answer was "yes, but like paper or fatwood" — the question now is whether it has moved toward timber without becoming dull. |

### Phase B — the glow, re-judged on a field that has changed shape

Only after A-9 has an answer.

| # | Check | What it catches |
|---|-------|-----------------|
| B-1 | Fire against dark terrain. **No flat white plateau**; terrain inside the lit area still legible. | Regression check. |
| B-2 | Look for **shafts and rays**. The halo should be round at every angle — including the diagonals *between* the axes and the corners, which is where the octagon used to bulge. | B9a. The old artefact was worst at 22.5°, precisely between the two directions the old test sampled, so **look off-axis rather than along it**. |
| B-3 | Judge the **reach** again. | **A look decision, still explicitly open.** Answer in the same terms: right, still too far, or now too tight. |
| B-4 | Move a **single stray flame** away from the fire front. It should light weakly. | `COVERAGE_FLOOR` at 0.1. Session 3 said "even weaker" against 0.2. |
| B-5 | Fire behind a **one-cell wall**, and fire in a pit. Light stops dead at the solid. | B9d. This one has a hard before/after — 44% through a thin wall, now 0% — so a leak here means the compositing, not the field. |
| B-6 | A scene with **no fire at all** looks exactly as it did before V7. | The additive-only boundary. |
| B-7 | HUD fps under a large fire, then `preview_light` on the same scene. | **Re-check, not a regression check.** Propagation gained eight knight's-move taps per block per iteration and four smoothing sweeps since the last fps reading. Current instrument numbers: peak **118**, **35%** of blocks lit, **0.0%** clipped. |

### What to bring back

Screenshots, including of states nobody would think to describe. A note without an image is incomplete.

Then, specifically: **A-9 and B-3.** Wave 2c closes on A-9 reading as a fire that burns at a believable pace, and B-3 being *decided* rather than deferred a third time. The tables being green does not close it.

And one thing that is not a row: **if any of the four A-side changes reads as *no change at all*, that is the most valuable note in the session.** Three of them are worth 22–50% and one is worth 23% on a noisy quantity; knowing which of those a person can actually see is what tells the next wave where to spend.

---

## Session 4 results — wave 2c closes, and nothing is carried

All sixteen rows pass: **A-1 through A-9 and B-1 through B-7. No findings.**

**A-9 answered: yes.** The fire reads as burning at a pace that feels right. Session 3's answer was "yes, but like paper or fatwood"; the four A-side changes moved it toward timber and it has not gone dull.

**B-3 decided, not deferred.** `TRANSMIT_CLEAR` at 0.52 with `COVERAGE_FLOOR` at 0.1 is the right reach. **This is the third session the row was asked in and the first it was answered** — it is settled and not to be reopened without a new observation.

**B-7 re-checked, not assumed.** Performance is stable under a large fire after the eight knight's-move taps and four smoothing sweeps added since the 165 fps reading.

**What was not recorded, and is the session's one gap:** the checklist asked, outside the tables, which of the four A-side changes read as *no change at all*. **No such note came back, and a blanket pass is not the same answer as "all four were visible"** — so the raggedness change is closed as *not contradicted* rather than as *confirmed visible*. The consequence for anyone raising burn-front linearity again is recorded in [wave 2c](ROADMAP.md#wave-2c--the-glows-shape-and-the-fires-timing).

---

## Between sessions — A6b, reported from screenshots

Not a checklist pass. Three screenshots, `resources/video_screenshots/water_issue_*.png`, taken against a build that already had A6's fix in it.

**What was seen:** water appears at the very top of a sand column tens of cells above the pool, **the moment the falling sand reaches the water**, and then runs down the outside of the pile in ragged clumps rather than as a sheet.

| ID | What was seen | Severity | Status |
|----|---------------|----------|--------|
| A6b | Water lifted to the top of a falling sand column and trickling back down its flanks in a staggered pattern. | major | **partly fixed** — [wave 3](ROADMAP.md#wave-3--the-brush-destroyed-water-and-the-elevator-it-was-hiding). Session 5 W-3 confirms the lift to the top of the column is gone; a residual climb remains (**D3**) and the staggered flanks turned out to be a separate finding about the flow model, not about the lift (**D4**). |

**Worth recording as a reporting pattern:** the first read was that A6's own fix had caused this. It had not — the behaviour predates wave 1 and was invisible because a second defect was eating the evidence. **A fix that makes a longstanding defect visible for the first time looks exactly like a fix that caused one**, which is the same lesson A1b taught from the rendering side, arriving from the physics side.

---

## Session 5 checklist — the pass that closes wave 3

**Run — see [the results](#session-5-results--wave-3-closes-and-the-water-underneath-it-does-not) below.** [Wave 3](ROADMAP.md#wave-3--the-brush-destroyed-water-and-the-elevator-it-was-hiding)'s code is in and its exit condition is this session.

**Run README's Manual Tester Checklist first, in full** — wave 3 changed a grid write path and added a fluid-venting rule, so steps 1–9 all apply and step 4 in particular is about the brush. **Step 8 was rewritten on 2026-08-10 and the rewrite matters to this session:** it used to ask for fps "near the display's refresh" on a wide sand-over-water fill, which P2 has since measured at ~35 ms/step at the played size. Reading the old wording, W-7 below would have been filed as a wave 3 performance defect. It is not one.

**Record before starting:** build hash, Release/MSVC, world seed, the `Scene:` line, suite count, **and the display mode** — the `Display: WxH (WxH cells at 4x)` line the game prints at startup.

**The display mode is new to this list and is not bookkeeping.** Every fps observation in this session is uninterpretable without it. `PERFORMANCE.md` measures the light field at 6.43 ms/frame at 1920x1080 against **15.48 ms/frame at 3440x1440** — 38.6% of a 60 Hz frame against 92.9% — for the same scene, because the field scans what is on screen and the widest mode has 2.4x as much of it. (Both halves of that pair are quoted from the same sitting on purpose; P2's own run re-read the wide figure at 15.25 ms, which is consistent, but pairing one sitting's number with another's is the comparison this project's method forbids.) A session run at the widest mode and one run at 1080p will disagree about how the game performs by more than any wave 3 change could, and until now nothing in this file recorded which had happened. Sessions 1–4 did not record it and their fps notes should be read with that in mind.

### Phase A — the question wave 3 exists to answer

*Phase A closes wave 3 and is the reason this session is scheduled; Phase B below discharges the E4 decision that is due with it. Order is load-bearing in the same way it was in session 3: do A first, while the water is the only thing you have been looking at.*

Session 1's A6 reported two things — water capped on top of a spawned pile, and an outward burst on release. **The fix claims those are one defect and that the burst needed no separate fix**, because it was the deleted volume being repaid. That claim is a look, not a number, and no test asserts it. Row 3 is where it gets confirmed or contradicted.

| # | Check | What it catches |
|---|-------|-----------------|
| W-1 | Spawn sand into a pool with the brush and **hold the drag**. Water is pushed aside, not capped and not deleted. | A6. The measured version is 451 cells destroyed before, 0 after; this is whether that reads correctly in motion. |
| W-2 | **Release the brush** and watch the moment after. | The burst. It should now be absent or unremarkable rather than an outward surge. |
| W-3 | Pour sand from **well above** a pool — 25 cells of open air, so the brush never touches water — and watch the top of the column. | A6b, the elevator, in the exact configuration the screenshots showed. No water should appear above the cursor as the sand arrives. |
| W-4 | Keep pouring until the pile **stands out of the water** as a cone, then keep going. | The residue. A handful of cells (3 at step 350, 6 at step 500 on the probe) still reach the peak once the pile itself becomes the conveyor. **This is the first thing to look at if the screenshots repeat**, and the open question is whether it is visible at all. |
| W-5 | Watch water run **down the flanks** of a standing pile. Does it read as flow, or as the staggered clumps the original report described? | The trickle was the lifted water coming home. If the lift is gone and the staggering is not, that is a new finding about the flow model rather than about venting. |
| W-6 | Drag a sand brush through water and watch the **pool level**. Volume looks conserved — no shrinking pool. | The conservation half, by eye. The probe reports 2000 of 2000; this is whether anything visible contradicts it. |
| W-7 | HUD fps and awake-chunk count while churning sand and water together. **Read the note under this table first — the expected answer changed.** | `VENT_RADIUS` is a cost knob set at the knee of a sweep (`churning` 3.13 → 4.93 ms/step). What the window can confirm is that nothing behaves *worse than that sweep predicts in kind*; what it can no longer confirm is that the sweep's verdict of "affordable" still holds, because the sweep was run at a quarter of the played world. |

**W-7 cannot be answered by eye any more, and it is more useful to say so than to leave the row looking answerable.** Two things changed under it:

- **The sweep behind it is quoted at 960x540.** `churning` with no venting was 3.13 ms/step and `VENT_RADIUS = 3` took it to 4.93 — an affordable +1.80 ms against a 16.67 ms budget. P2 re-measured `churning` at the played 1920x1080 and it is **35.25 ms/step, 211% of a frame**, venting included. Whether venting's share of that is still ~1.8 ms or has scaled with everything else is **not known**, and this session cannot find out: at 35 ms/step the scene is bogging for reasons that have nothing to do with wave 3, so any felt slowdown is swamped.
- **Answering it properly needs a runtime toggle, which does not exist.** `VENT_RADIUS` is a compile-time `constexpr` (`grid.h:207`), so an on/off comparison means two builds — and `PERFORMANCE.md`'s E1 entry records a confident 28% from exactly that method that turned out to be the compiler re-laying-out the hot loop. **Do not re-run the sweep across a rebuild to close this.** It is an engine item, not a playtest row.

> **Answered 2026-08-13, and the row was right about both halves.** The toggle was built (`Grid::set_vent_radius`) and the question W-7 could not answer by eye has a number: at 1920x1080, venting is **23.35 ms of `churning`'s row** — 26.06 ms/step with it off against 49.41 with it at r=3 — so it did *not* stay at ~1.8 ms. The second bullet's warning was also right and then some: re-swept in one binary, the cost curve is **linear in the scanned area with no knee**, and the knee was the entire reason 3 was chosen. **What the row got wrong is only that the rebuild was forbidden outright** — it is admissible with a control the change cannot affect, and `churning` being the only bench scenario containing water supplies six of them. Kept as written because "this cannot be closed by eye, and here is the instrument it needs instead" is exactly what a checklist row is for. Numbers in `PERFORMANCE.md`; the engine item is closed in `ROADMAP_ITEMS.md`.

**So what W-7 is actually for in this session:** confirm that mixing sand and water does not produce something *qualitatively* wrong — a stall that does not recover, an awake-chunk count that never comes back down after the scene settles, a hang. Those are wave 3's business. The frame cost is not, and a wide sand-over-water fill running slowly is the played size behaving as `PERFORMANCE.md` says it does.

### Phase B — the E4 decision, which this session is the gate for

**This phase was missing and the session could not have discharged what the plan says it discharges.** [Decisions owed](ROADMAP_ITEMS.md#-decisions-owed) has *"Does the player displace material? (E4) — due after session 5 — Play it. If the artifact isn't obviously better, write 'no'."* E4 is item 2 in the plan and blocked on this session. Every row above is about water and venting; none of them looks at the player at all, so as written this session would have ended with E4 exactly as blocked as it started.

**What is being judged is the absence, not a feature.** E4 is not built — the grid does not know the player exists, so material falls straight through the body. There is nothing to toggle and nothing to compare against. The question is only whether that reads as *broken* when you go looking for it, and the bar the decision sets is deliberately high: "obviously better" or the answer is no. **A "yes" is not a small answer** — the implementation waits on E5a, which is what gives shoved material somewhere to go, so a yes queues work behind another item rather than starting any.

Go looking for it in the four places it should be most visible:

| # | Check | What it is evidence for |
|---|-------|-------------------------|
| E-1 | Stand still under a **sand pour** from the brush and watch the grains cross the body. | The most direct case. If sand streaming through the figure is not noticeable while you are staring at it, it will never be noticeable in play, and that is a "no" with evidence. |
| E-2 | **Walk into the side of a settled pile** and keep walking through it. | Whether the body reads as passing through terrain rather than moving it — the case a player will hit constantly without looking for it. |
| E-3 | **Dig out the support under a pile so it collapses onto you**, and stand in it. | The highest-drama version. Wave 3's venting and E3's fracture both fire here, so it is also the busiest scene the body will be standing in. |
| E-4 | **Stand in water** and walk along the bottom of a pool. | Fluid rather than powder. Worth separating: liquid closing over a body reads very differently from grains passing through one, and E4 could reasonably be answered "yes for powder, no for fluid". |

**Record a plain yes/no plus one sentence of why.** The decision's own instruction is to write "no" and stop thinking about it if the artifact is not obviously better, so an inconclusive answer should be recorded as a "no" — that is what the due date is for.

### What to bring back

Screenshots, in the same configuration as `water_issue_*.png` so the before and after are comparable — that is the whole reason those images are worth keeping.

Then, specifically: **W-2 and W-4.** Wave 3 closes on the burst being confirmed gone as a *consequence* of the conservation fix rather than as an unfixed second defect, and on the residue being judged either invisible or worth another pass.

And **the E4 answer**, in one sentence, because item 2 in the plan is waiting on it and this is the session that owes it.

---

## Session 5 results — wave 3 closes, and the water underneath it does not

- **Build:** Release, MSVC
- **World seed:** `16445138370698876092`
- **Display:** `3440x1440 (860x360 cells at 4x)` — the widest mode, and **the first session on record to state one**
- **Scene:** `Scene: 1920x1080, 334901 cells placed`; props 9 of 9 placed
- **Result:** all three passes run — README's nine steps, then Phase A, then Phase B.

**Wave 3's exit condition is met, on the two rows it was written to close.** **W-2:** no burst on release, so the outward surge is confirmed gone *as a consequence* of the conservation fix rather than as a second defect nobody had fixed — that claim was a look with no test behind it and it holds. **W-4:** the residue is not independently visible; what is visible is the flow itself, which is a different finding and is below.

**W-5 is the session's real result, and it counts for more than the others because a row written in advance discriminated it.** The checklist said: *"If the lift is gone and the staggering is not, that is a new finding about the flow model rather than about venting."* The lift is gone and the staggering is not. That converts the general checklist's "water/oil ... does not flow properly and is bugged" from an unlocalised complaint into a finding about `step_fluid`'s lateral run, and it is what withdrew E11's *"no action is proposed"* — see [E11](ROADMAP_ITEMS.md#e--simulation-depth). **This is the strongest form a checklist has taken in this project: a question whose two possible answers pointed at two different subsystems, asked before either was suspected.**

**E4 answered: no.** Two of the four rows turned out not to be testing E4 at all — E-1 is a defect in the unstuck search and E-2 is almost certainly `MAX_STEP_HEIGHT`, so neither is evidence about displacement. E-3 works. That leaves E-4, water, as the only surviving argument, which is the reverse of the split the checklist guessed at ("yes for powder, no for fluid"). Nothing returned "obviously better", and the decision's own rule is that an inconclusive answer is a "no". Recorded in [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md).

### Per-step result — README's Manual Tester Checklist

| # | Step | Result |
|---|------|--------|
| 1 | Launch | **Pass.** Display, seed and scene lines all printed; 334901 cells; props 9 of 9. |
| 2 | Movement | **Pass with defects.** Sprite movement good. Collision accepted — the body stops at walls with a one-pixel sprite overhang, judged stylistic and **not filed**. Animation: D6, and D1. |
| 3 | Digging | **Pass.** |
| 4 | Materials and brush | **Pass with defects.** Sand good — D8. Water and oil — D4. |
| 5 | Reactions and heat | **Pass with defect.** Fire burns wood well; waves 2b/2c hold as a regression check. Steam — D5. |
| 6 | Chunking / sleep-wake | **Pass.** |
| 7 | Structures | **Pass with observation.** Correct as listed — D9. |
| 8 | Performance sanity | **Pass**, and worth more than usual: this is the widest mode, where `PERFORMANCE.md` puts the light field alone at 92.9% of a 60 Hz frame. |
| 9 | Stability | **Pass, no crash.** |

### Phase A — the question wave 3 exists to answer

| # | Result |
|---|--------|
| W-1 | **Pass with observation.** Water is displaced, not deleted or capped — "the attempt is clear" — but it goes into a hump on top of the pool and "the jagged shapes the water flows in ruins the illusion of properly simulated water physics". The mechanism is right and the look is not. Feeds D4. |
| W-2 | **Pass.** No burst on release. **Wave 3 exit row.** |
| W-3 | **Pass with defect.** No water spawning on top of the column — A6b's headline symptom is gone. Water still climbs the column, "about 50 percent too quickly" — D3. |
| W-4 | **Pass.** "Looks correct but again ruined by the visual bugs." The residue is not what is visible. **Wave 3 exit row.** |
| W-5 | **Fail.** "Still staggered clumps." The discriminating row — D4. |
| W-6 | **Pass.** Pool level looks conserved; nothing visible contradicts the probe's 2000 of 2000. |
| W-7 | **Pass.** Nothing qualitatively wrong — no stall, no chunk count that fails to come down. The frame cost was never this row's business. |

### Phase B — the E4 decision

| # | Result |
|---|--------|
| E-1 | Pouring sand "pushes the player in a glitch-like way, clipping through objects if encountered" — **D2, and not an E4 answer.** |
| E-2 | "Walking into a settled pile does nothing, the player walks over it, no interaction" — **D7, and probably not an E4 answer either.** |
| E-3 | Collapse onto the body "works, but overall rudimentary". |
| E-4 | "No interaction with the body in water." The one row that is E4 evidence. |

### Defects

Severity: **major** = wrong behaviour a player will hit in normal play; **minor** = wrong but cosmetic or narrow.

| ID | What was seen | Severity | Status |
|----|---------------|----------|--------|
| D1 | The dig animation plays once when the dig button is held down, instead of repeating. | major | wave 4 |
| D2 | Pouring sand onto the player pushes it in a glitch-like way, and **can push it through solid objects**. | major | wave 4 |
| D3 | Water still climbs a standing sand column, "about 50 percent too quickly", after A6b's headline symptom is gone. | major | **open** — the fluid spike |
| D4 | Water and oil do not flow — jagged shapes on displacement, staggered clumps down a pile. Reported twice in one session, at step 4 and again at W-5. | major | **open** — the fluid spike |
| D5 | Steam condenses back to water far too quickly; it should collect and then drip slowly. | major | **fixed 2026-08-12** — the steam half of E9. **Third report** — see A5 and B3, and it is the third report that scheduled it. ✅ **Confirmed in play 2026-08-13** ([spot check](#spot-check--2026-08-13--the-two-owed-steps-run-together)) |

**D3 is a residual to eliminate and not a rate to tune**, and that was settled rather than assumed: displacing sand into a pool must raise the pool's free *surface*, which is conservation and is W-6's pass, but no configuration of sand and water makes it right for water to occupy a column standing *above* that surface. The invariant is therefore assertable — **no water cell may come to rest above the free surface**, splash excepted — which takes this out of the realm of looks. `water_probe` already measures the quantity.

### Observations — feelings, not defects

| ID | What was seen | Status |
|----|---------------|--------|
| D6 | The walk cycle is about 10% too quick. | wave 4 |
| D7 | Walking into a settled sand pile does nothing — the player walks over it with no interaction. | wave 4, as a tuning value rather than a feature |
| D8 | Sand "could have more movement". **Disambiguated after the session: falling sand reads as stepped and jerky rather than flowing.** | **open, and deliberately unowned** — see below |
| D9 | Structures are "all very stiff and not very interactable"; off-balance shapes should topple. | E8 — **second report**, see B4. No order change. |
| D10 | Standing in water produces no interaction with the body. | E4's only surviving row; answered "no" and re-asked at E5a. |

**D8 is the second report of A7c and it is filed with no item on purpose.** Session 1 said "grain motion reads as stepping rather than flowing"; powder acceleration was tried against it, made it measurably worse, and was removed, and [wave 1](ROADMAP.md#wave-1--the-rendering-brush-and-powder-defects) concluded that stepping is *"a property of drawing whole cells on a fixed tick and is not reachable from `step_powder` at all"*. **That conclusion is still right, and the consequence has never been drawn: the finding was correctly refused by the simulation track and then handed to nobody.** It is specifically *not* E10 — E10 makes sand move less, holds a slope, and comes to rest, so filing D8 against it would put a rendering complaint on a simulation item and produce a third round of the A7/A7b/A7c rule fight. The nearest real owner is V11's runtime `Camera::SCALE`, since a 4x4 screen block is the unit doing the stepping. **It stays an observation until something argues it into the V track on that section's own admission test.**

**D7 is filed as an observation and will almost certainly be closed as a number.** `MAX_STEP_HEIGHT` is 5 against a 20-cell collision box, so the body steps a quarter of its own height instantly and without animation, which is a very plausible whole explanation for "walks over it". README's step 2 only ever asks for a **one-cell** sand step, so nothing has been checking the constant that actually governs this.

**Two of this session's findings were misattributed at the point of reporting, and both were caught by reading code rather than by re-testing.** E-1 was reported inside a phase about the *absence* of player/material interaction and is the opposite — something is interacting, via `resolve_overlap`. E-2 was reported the same way and is a step-height value. **The pattern is the one A7b already taught from the other direction:** a finding arrives wearing the mechanism the tester was asked to look for. Phase B asked four questions about displacement and got two answers about something else, which is not a failure of the phase — it is the reason a "no" here is trustworthy.

---

## Spot check — 2026-08-13 — the playtest F5 owed

Not a session: one step of README's Manual Tester Checklist, run because F5
rewrote the player's kinematics into fixed point and the suites cannot judge
feel. **The expected answer was "exactly as before", which is the hardest kind
of result to report honestly** — a tester looking for a change tends to find
one — so the row below is recorded in the terms it was asked in.

| # | Question | Result |
|---|----------|--------|
| 2 | Movement, animation, and the new flight row: does walking, jumping, landing, the one-cell sand step and sustained flight feel as they did before? | **Pass.** No difference reported. |

**What this does and does not close.** It closes the playtest F5 owed. It does
**not** confirm D5 — the steam half of E9 is still owed checklist step 5, which
this pass did not cover, and D5's row above still reads "wants confirming in
play".

> **Both of those were run later the same day — see the spot check below.** Step
> 5 came back a pass, so D5's row above is now confirmed rather than owed.

---

## Spot check — 2026-08-13 — the two owed steps, run together

Not a session: two steps of README's Manual Tester Checklist, run back to back
in one launch. **Step 5** had been owed since E9's steam half shipped on
2026-08-12; **step 3** was owed by `F6`, which rewrote `DigTool::march` into
integer arithmetic the same day. They are recorded together because they were
run together, not because they are related.

- **Suites at time of test:** 10/10 green
- **World seed / `Scene:` line:** **not captured.** Recorded as missing rather
  than left blank — this file's own rule is that a finding without its seed is
  an anecdote, and the only reason that costs nothing here is that **there were
  no findings.** A defect out of either step below would have been unreproducible.

### Step 3 — Digging

| # | Question | Result |
|---|----------|--------|
| 3a | Does a left-click cut a circular hole at the crosshair? | **Pass.** |
| 3b | Does the crosshair dim from solid to dim white past `RANGE`? | **Pass.** |
| 3c | Does a dim shot into open sky beyond range do nothing? | **Pass.** |
| 3d | **Does a dim shot lined up through *nearby* terrain still cut it?** | **Pass.** |
| 3e | Does digging the base out of a settled sand pile bring the column down? | **Pass.** No gap left hanging. |
| 3f | Does a shot stop at the near face of a wall rather than tunnelling to the one behind? | **Pass.** |

**3d is the row this pass existed for.** The range boundary is the one place a
`sqrt` and a squared comparison can disagree, and it is also where the crosshair
indicator and the dig ray could disagree with *each other* — they were computed
two different ways in two different files until F6. Nothing was reported off by
a cell in either direction, and no diagonal was reported reaching further than a
straight shot.

### Step 5 — Reactions and heat

| # | Question | Result |
|---|----------|--------|
| 5a | Does fire advance along a long Wood beam as a front, rather than lighting it all at once or stopping after one cell? | **Pass.** |
| 5b | Does Oil catch sooner than Wood? | **Pass.** |
| 5c | Does Fire beside Water boil off Steam *and* get doused? | **Pass.** |
| 5d | **Does Steam under a solid ceiling rise, gather, wait several seconds, and then drip single Water cells — the pocket shrinking from the top?** | **Pass.** |
| 5e | Does Steam under a *Wood* roof leave the roof unlit? | **Pass.** |
| 5f | Does a lone Fire cell burn itself out? | **Pass.** |
| 5g | Does a Fire cell boxed in on all sides with Wall still burn out? | **Pass.** |

**5d confirms A5, B3 and D5 — the same symptom reported three times across four
sessions.** The row was written so that its two failure directions point at
different mechanisms: condensing immediately would have meant the lifetime was
still being measured in degrees, and a pocket that never went at all would have
meant the ceiling-contact rule was not seeing the ceiling. Neither was reported.

**What this closes.** A5, B3 and D5 stop reading "wants confirming in play". E9's
steam half is no longer owed a playtest, and neither is F6. **What it does not
close:** nothing here says anything about determinism across machines — step 3
was run on the same machine as every other test this project has ever run, which
is the gate prerequisite's job and not this pass's.

---

## Spot check — 2026-08-13 — the P4 recording run

Not a session, and **not run as a checklist pass either** — someone played seven
minutes to produce the recorded session `P4`'s benchmark row needs, and two
checklist steps fell out of it. They are recorded here because the evidence
exists, in the terms the evidence actually supports rather than the terms the
checklist asks in.

- **World seed:** `10949426797942825974`
- **`Scene:` line:** `Scene: 1920x1080, 334901 cells placed` — and `Props: 9 of 9 placed`
- **Session:** 24,437 fixed steps, 407.3 s, written to `session.rec`

| # | Question | Result |
|---|----------|--------|
| 1 | Launch: does the window open, does the seed print, and does the scene load to its pinned count with terrain visible? | **Pass.** 334,901 cells, exactly the pinned figure; terrain reported visible. |
| 9 | Stability: a few minutes of mixed activity without a crash. | **Not covered — see below.** No crash in 407 s, but the census shows none of the mixed activity happened. |

**Step 1 is the row that mattered**, because `P4` moved the scene loader out of
`main.cpp` into `src/scene/bmp.cpp` and the game had not been launched since.
The count is the check rather than the eyeballing, and it is now agreed on by
three independent readers — the C++ loader in the game, `tools/pixel_art.py`, and
the figure this checklist has carried since the SDL-side loader existed.

**Step 9 is recorded as a fail-to-cover, not a pass — and this is no longer a
judgement call.** It was first written up as "partial" on the reasoning that
nothing crashed in 407 seconds but the session's contents were unknown. The
benchmark's session census, added the same afternoon, made them known:

```
dig 0 steps, brush 8085, moving 384, jumping 97 (of 24437)
Sand   at start 16001, peak 16001     Water  at start 70350, peak 70350
Steam  never seen
peak 16 of 510 chunks awake once the scene had settled
Fire+Water+Sand all present in 201 of 408 samples, digging in 0 of those
```

**The dig tool never fired. Sand and Water peak at exactly their starting counts,
so nothing in the terrain moved for seven minutes. The player stood still for
98.4% of the steps.** Step 9 asks for digging near falling sand near fire near
water, with movement keys held through a collapse, and **none of those four
happened.** No crash in a world that was 3% awake is a much weaker statement than
"a few minutes of doing several of the above at once", and recording it as
partial credit would be the "nothing crashed" → "the hard case was tried" slide
this file exists to prevent.

**What this spot check therefore contributes is step 1 and a measurement**, and
its more useful output is a warning about itself: **a session being genuinely
played does not make it a representative one**, and until the census existed
there was no way to notice. **Step 9 is owed in full**, and the next recording run
is where it gets done.

> **One correction to the paragraph above, filed 2026-08-13 and kept beside it
> because the conclusion survives and the reasoning does not.** "Sand and Water
> peak at exactly their starting counts, **so** nothing in the terrain moved" is
> not a valid step: material moving conserves its count, so equal counts prove
> only that no sand was painted or dug. What actually supports the conclusion is
> **16 of 510 chunks awake** — every write wakes its 3x3 neighbourhood, so a
> world with matter in motion cannot be 3% awake. **The census's material rows
> cannot see movement at all; its awake-chunk figure is the column that can.**
> Worth knowing before the next session is read, because the same wrong step is
> available every time.

## Spot check — 2026-08-13 — the second P4 recording, played to cover step 9

Same shape as the run above: not a full session, someone played to produce the
recording `P4` was blocked on, **and this time the cases were chosen in advance**
from the list the first census exposed as missing.

- **World seed:** `505286022235307382`
- **Session:** 20,415 fixed steps, 340.2 s, saved with `F9`
- **Kept as:** `session_2_digging_fluids_steam.rec`

| # | Question | Result |
|---|----------|--------|
| 9 | Stability: a few minutes of mixed activity without a crash. | **Pass, with one clause outstanding.** No crash in 340 s; three of the step's four activities are evidenced, the fourth is not. |

**What the census evidences directly:** 479 dig steps, 9,158 brush steps,
`Fire`+`Water`+`Sand` all present in **198 of 341 samples with digging inside 11
of them**, `Steam` present at up to 10,731 cells, and 44 of 510 chunks awake at
peak against the first session's 16. The recording replayed byte-exact
afterwards, so nothing in those 340 seconds put the simulation into a state it
could not reproduce.

**What is not evidenced is "movement keys held through a collapse."** The player
moved on 647 steps and jumped on 351, but **a collapse is not a material and no
census column can show one**, and the tester did not watch for it specifically.
Recording that as covered would be the same "nothing crashed" → "the hard case
was tried" slide the entry above refuses, so it is named instead: **step 9 passes
on digging-near-sand-near-fire-near-water, on back-to-back brush strokes and on
not crashing, and the collapse clause is still owed.**

**Two limits on using a recording as this step at all**, both worth carrying
forward. The census reports materials **co-present in the world**, not *near* one
another — step 9 says "near", and co-presence is the weaker claim, so the 11
samples are an upper bound on the real overlap rather than a measurement of it.
And a recording can only ever answer the *crash* half of a stability step: the
reason step 9 is manual is that a person also notices what is wrong without being
fatal, and no replay reports that. **A recording strengthens step 9; it does not
retire it.**

**The unexplained `0xC0000409` did not recur**, in this session or the previous
one — roughly 12.5 minutes of play across the two. Recorded as an observation and
not as progress: it was only ever seen twice under heavy machine load, and two
clean sessions on an unloaded machine is not evidence about a load-dependent
crash.

## Spot check — 2026-08-14 — step 10 runs, and the question is held open

Not a session: one step of README's Manual Tester Checklist, **step 10**, owed
since `S0` was built the same day. It is the only step on that list whose result
is a design decision rather than a pass or a fail, and it is the thing the combat
question has been waiting on for months.

- **Suites at time of test:** 10/10 green (11 including `debug_test`, added after)
- **World seed / `Scene:` line:** **not captured.** Recorded as missing rather
  than left blank, the same way the 2026-08-13 pair was, and it costs nothing for
  the same reason: there were no findings. A defect out of this step would have
  been unreproducible.

### Result

**Passed. Nothing was reported.**

**What that does and does not evidence, stated because the difference matters
here more than usual.** What came back was a global "looks good" on the step as a
whole. The step names three things to be fussy about — that `R` is inert while
the run is playing, that the `GOAL:` bearing counts down as you approach, and
that a body dug out of burning terrain is still burning — and **none of the three
was reported on individually.** Recording them as separately verified would be
inventing evidence out of an absence of complaints, which is the same slide from
"nothing was wrong" to "the hard case was tried" that the step-9 entry above
refuses. So: **the step passed, and the three fussy clauses are covered by the
tester not hitting them rather than by anyone having gone looking.** They are
cheap to re-run and worth naming the next time the run is played.

### The half that is not a check

**The combat question was deliberately not answered, and that is a decision about
the decision rather than the step failing.** The item was built to make the
question answerable by playing, and the sitting that made it answerable chose to
hold it open. The entry in [ROADMAP_ITEMS.md](ROADMAP_ITEMS.md#-decisions-owed)
is updated to say that, in place of a date it would have missed.

**This is exactly the state that entry warns about, so it is written down as
such:** the decision has been open for months precisely because it had a gate
instead of a deadline, and "we will decide after the next thing" is how it gets
another one. The difference this time is that the thing it was waiting for now
exists and can be played again in five minutes, which is a much cheaper way to
close it than any of the previous ones.

---

## Spot check — 2026-08-16 — the V11 checklist, and the horizon answers a question

Not a session: four steps of README's Manual Tester Checklist, owed by step 2 of
the V-track block. **Step 11 is new with V11** and was written with two halves —
a mechanical parallax check and a question about whether a mid-ground band is
needed. Both were answered.

- **Suites at time of test:** 12/12 green
- **World seed / `Scene:` line:** not captured. Recorded as missing rather than
  left blank; there were no defects, so nothing is unreproducible by it.

### Results

- **Step 11 — parallax and depth.** Reported **good**. Backdrop drift ordering
  reads correctly while walking, and no seam was reported at the pan limits.
- **Step 1 — launch.** Reported good, and specifically **no new backdrop-size
  warning**. That line is new with V11 and prints only when a loaded BMP is
  smaller than the generated header says it should be, so its absence is the
  check, not its content.
- **Step 2 — movement and animation.** *"Not sure but no obvious problems."*
- **Step 5 — reactions and heat.** *"Not sure but no obvious problems."*

### The answer step 11 was written to get

**Asked:** standing on the surface and looking at the horizon, is there a visible
gap between the mountains and the ground you are on?

**Answered: no — the terrain already fills it.**

That is the whole finding. What it cost and what was done about it are in
ROADMAP.md at V11 and beside entry 4 of `notes/reference_observations.txt`, which
is where the reasoning lives; this file records only that the question was put to
a played frame and what came back.

### A note on how to read "not sure but no obvious problems"

Steps 2 and 5 came back that way, and **it is worth being precise about what that
is worth rather than filing it as a weak pass.** V11 changed *where* the
composition lives and not *what* it draws, and the golden checksum says so
arithmetically — the composed frame is byte-identical to the one the inline code
produced. **So "exactly as before" was the predicted result of those two steps,
and a tester who cannot tell anything changed is the prediction being confirmed,
not a check that failed to happen.**

The clause worth keeping outstanding: neither step was reported as *examined
closely*, and the checksum covers a software rasterisation of one still frame.
**A GPU-only or motion-only difference in those two steps would not have been
caught by either instrument**, and nothing here claims otherwise. Same shape as
step 9's "passed with a clause outstanding" on 2026-08-13.

## Spot check — 2026-08-16 — the grade pass is looked at

Run after step 3 of the V block, on the three steps the change could reach.
Reported verbatim:

```
12 looks good on both
11 looks good
5 looks good
```

**Step 12 is the one that mattered, and "on both" is the whole of it.** The step
asks two questions and the tester answered them separately. The mountains read as
a silhouette, and — the half that was actually at risk — **they are not
over-corrected**. 0.60 was picked from a luminance measurement, which is not the
same as picked by looking at it, and until this run nobody had looked. The second
half, that *only* the mountains moved, is the check that `Params::world_grade` has
not quietly acquired a caller; a frame-wide darkening would have said it had.

**Step 5 is worth more than its one line suggests.** A fire is not dimmed by the
grade — which is the layer ordering (grade multiplies, then light adds) observed
rather than asserted. `golden_frame_test` can only argue that ordering
*indirectly*, by composing with and without the light pass and comparing the
contribution, and it argues it that way because the direct instrument failed: the
fixture's fire saturates at 255, so a correct ordering and a reversed one both
squeeze against the clip. **This is the first time anyone has seen the thing the
test stands in for.**

Step 11 re-run because the layer table changed shape; no seam at the pan limit.

Symptoms only, as ever. What the numbers are and why they are those numbers is in
TUNING.md's "Depth grading" section and at step 3 in ROADMAP_ITEMS.md.

## Gate screenshot — 2026-08-16 — does terrain fill the mid-ground band?

V19's own precondition, asked before any band was authored. The question is the
reopen trigger V11 wrote down when it deleted its mid-ground layer on the same
day: **a location whose terrain does not fill the band.** One frame supplied,
`resources/game_screenshots/visual_rework_1.png`.

**Answer: it does not fill it. The trigger fires.** Measured over the frame in
Rec. 709 luminance, taking each row's median as its background and counting
pixels more than 4 levels off it:

```
region            rows      non-background    background L
upper sky        60- 250          0.7%          18 - 35
mid band        250- 470         30.3%          18 - 35
low band        470- 588         51.0%          18 - 112
ground plane    600-1000          0.5%          22.7 flat
```

The horizon sits at y≈590 of 1080. The mid band is **70% bare sky**, and what
occupies the other 30% is the terrain hill at the far left and the trees at the
far right — not the band's own depth. V11's deletion was made against F4's
snowbank, whose terrain rose into that space; this location's does not.

Two things nobody went looking for, and they are the reason this entry is longer
than the question deserved.

**The ground plane has no internal variation at all.** Luminance 22.70 on every
sampled pixel across 400 rows — a spread of 0.0 — over 37% of the frame. Not a
flat *band*: a flat *fill*.

**The ground plane and the upper sky are the same value.** Sky at rows 100–240
reads 22.3–22.4 against the plane's 22.70. Four tenths of a level out of 255
between the nearest thing in the frame and one of the most distant.

That second one is the same shape as the sky-versus-mountains reading that
bought the 0.60 grade in step 3, on a larger pair of surfaces, and it was not
caught then because the measurement that found it only compared the two backdrop
bands. Symptoms only — what to do about it is V19 in ROADMAP.md.

---

## Session 6 — 2026-08-16: V19 4b's ground plane, first human eyes

- **Build:** V19 4b, Release, MSVC. **Suites at time of test:** 13/13.
- **Steps run:** the five items on MANUAL_TESTING.md's owed list — checklist
  steps 9, 10, 11 and 12.
- **Evidence:** `resources/game_screenshots/plane_test (1).png` through `(4).png`.
  Reported without a seed, which is a gap the file's own rule names; three of the
  four findings turned out to be measurable off the screenshots and the shipped
  BMPs alone, so it did not cost anything this time. **It would have on the
  fourth.**

### Per-item result, in the tester's words

| # | Asked | Came back |
|---|-------|-----------|
| 1 | Vertical tiling join in the plane; do the dashes stream smoothly? | *"dashes seem to move smoothly but the effect isn't very convincing"* — **no join reported.** The wrapping arithmetic passes; the read does not. |
| 2 | Stair-step between the 24 parallax strips | *"there seem to be some visual bugs with black bands appearing in between the plane pixels"* — **fail.** |
| 3 | Do mountains read as a dark cut-out; does the plane recede? | *"mountains are not visible just the plane"* — **fail.** |
| 4 | Does the run need an enemy to be interesting? | *"needs an enemy but i will do that later"* — **answered yes.** Filed at [ROADMAP_ITEMS.md](ROADMAP_ITEMS.md#-decisions-owed). |
| 5 | Movement keys held through a structure collapse | *"looks fine"* — **pass.** |

### Defects

| ID | Severity | Symptom |
|----|----------|---------|
| **F-1** | High | **Horizontal black bands across the ground plane**, between the strips rather than at their edges. Item 2. |
| **F-2** | High | **The mountain band is not visible in any of the four frames.** The plane is the only backdrop that reads. Item 3. |
| **F-3** | Medium | **The plane does not read as a receding surface** — the mechanism is there and the effect is not. Item 1. |

### Observations, measured off the four frames rather than felt

Kept separate from the table above, per this file's rule: these are numbers about
how the frame reads, and every fix suggested by one is a hypothesis.

**The whole frame occupies 9 levels of luminance.** Row-mean Rec. 709 down
`plane_test (1)` and `(4)`, HUD rows excluded: **L 15.5 to 24.5** in frame 1,
**15.9 to 23.3** in frame 4. For scale, the reference frames measured in
`notes/reference_observations.txt` entry 7 run **L 51.6 to 173.6** — a 123-level
spread — and **the smallest single band join in that reference, the horizon at 14
levels, is larger than our entire frame's range.**

**The plane's authored far-to-near ramp is 18.5 levels before grading and 9.8
after.** Measured down `assets/backdrop_ground.bmp`, row means: L 22.1 at the far
edge to 40.7 at the near, times the 0.53 grade. The same ramp in the reference is
**77.5 to 138.2, 61 levels.** Entry 7 calls this the mechanism that needs no
light source and no colour, and it is built — at a sixth of the amplitude.

**The near third of that ramp is flat.** Tile rows 160 through 255 all read
40.4 ± 0.4. Entry 7's mechanism 3 says contrast *grows* with nearness and the
reference spends most of its budget in the last two layers; ours spends none
there.

**The plane's texture is three colours.** `(24,20,38)`, `(44,37,64)` and a sparse
`(60,52,82)`, in a 49/49/2 split — so the "ramp" above is a dither *density*
gradient between two tones 18 levels apart, not a value gradient.

**The mountains' one silhouette tone is L 28.1, graded to 16.9.** The sky rows
they stand against read 26.7 to 35.2. That is a 10-to-18-level separation, which
is the largest contrast anywhere in the frame — so **F-2 is not the 0.60 grade
over-correcting**, which is what checklist step 12 was written to catch. Step 12
asked the right question about the wrong layer.

Symptoms and measurements only. What causes each and what to do about it is
ROADMAP.md's.

---

## Session 7 — 2026-08-16: V20's raised palette, and what the plane is *made of*

Five items were owed (V20's). All five came back in one pass. **Two of the
answers are new information and one of them is a direction question that no
previous session had the frame to ask.**

### Per-item result, in the tester's words

| # | asked | answer |
|---|---|---|
| 1 | Is it still a night scene? | **"too bright"** |
| 2 | Can you see the mountains now? | "mountains visible" |
| 3 | Do the black bands still appear while walking? | "black bands gone" |
| 4 | Does that ground look like it goes away from you? | **"no"** — see below |
| 5 | Is the frame rate still fine? | "frame rate fine" |

Items 2, 3 and 5 close cleanly. Item 2 closes the defect session 6 filed as the
mountains being invisible; item 3 closes the walking-only banding, which is the
one no test could have reached; item 5 is the only reading `GROUND_STRIPS` has
ever had, and the answer is that 24 strips times their tiling copies is not
felt. **That is a reading, not a measurement, and it is the only instrument that
exists** — `grid_bench` times `Grid::update` and `Run::step` and cannot see a
draw call.

### Item 1 — "too bright"

Recorded as the plain answer it was. The item offered four answers — too bright,
too washed out, the colours look wrong, fine — and **"washed out" was available
and not chosen**, which is the discrimination worth keeping: the report is level,
not separation, and not hue.

For scale against the reference rather than against feel: the frame the tester
saw ran post-grade sky 95→62, mountain rim 71, body 44, ground 30→81. The
reference's own **night** frame has its sky at L 163. So the report is not that
our sky outran the reference's night sky — it did not, by a wide margin.

### Item 4 — the plane does not recede, and the reason is not its values

The answer was "no", and it arrived with a reading of the reference that no
earlier session had made:

> look at the CnC_parallax .jpg files. the player is on the water plane. this may
> require a new starting scene to be completely redrawn/spawned

Checked against `resources/images/CnC_parallax_1..3.jpg`. **The reference's
receding plane is a lake surface, and the boat — the player — floats on it.**
Three observations off those frames, symptoms only:

- The plane's depth cues are **reflections of the bands standing above it**,
  vertically mirrored and value-compressed, not marks authored onto a surface.
  Every band in the scene appears twice.
- The horizontal ripple dashes **grow in size and frequency toward the viewer**,
  which our tile does reproduce — it is the one mechanism of the three that we
  built.
- **The player occupies the plane rather than standing in front of it.** The
  boat sits at roughly the plane's vertical midpoint in all three frames, with
  plane both above and below it.

Ours is an opaque band drawn *behind* the world, with the player standing on
terrain in front of it. **That is a difference in what the plane is, not in how
it is shaded**, and it is why item 4 could not have been answered by a retune.

The tester's own framing — that this may need the starting scene redrawn or
spawned differently — is recorded as theirs and is not resolved here. It
contradicts a recorded V19 decision ("the seven-band scene, with land where the
reference has water") and reopening that is a direction call, not a defect fix.

Symptoms and measurements only. What causes each and what to do about it is
ROADMAP.md's.

## Session 7b — 2026-08-16: the plane the player is not touching

A follow-up on the same build, unprompted, after the V22 direction was settled
as "land, with the player in the plane". It is filed as its own session rather
than appended to 7 because it reports on a **different surface** than any item
in 7 did, and because it arrived after a decision had already been taken on
7's evidence — which is exactly the sequence where a late symptom gets read as
a restatement of an earlier one and dropped.

### What was reported

> *"in the current build, the player is not touching the plane, they are only
> touching the original test scene ground. this may require a completely new
> scene from the ground up without the previous test scene. the player in my
> game is supposed to be touching the land layer like the player boat is
> touching the water in CnC_lighthouse.jpg specifically."*

### What is in that, separated from what it implies

Three separable observations, and the value of the report is that they are not
the same claim:

1. **The player's feet are on the fixture's terrain, not on the ground plane.**
   This is a fact about the build and is directly checkable.
2. **The two do not read as one surface.** The plane's near portion is not
   merely mis-valued against the terrain — it is *not visible at spawn*.
3. **A named reference frame for the target**, `CnC_lighthouse.jpg`, rather
   than a description of the desired look. Measured the same day at
   [notes/reference_observations.txt](notes/reference_observations.txt) entry 9.

The third is what makes this session worth its own heading. Every previous
report about the plane described an *appearance* ("it doesn't recede", "too
bright"); this one names a frame and a relationship, and the measurement taken
off it found a number no appearance-level report could have produced.

**The tester's own proposed cause — a completely new scene — is recorded as
theirs and is not resolved here.** It is a strong candidate and it collides
with F4.4's stated purpose for that scene, which is a scheduling and
ownership question rather than a symptom.

Symptoms and measurements only. What causes each and what to do about it is
ROADMAP.md's.

## Session 8 — 2026-08-17: V23's camera, and the plane's answer is still no

The V23 camera report, which was the one thing owed to the build and the one
thing V22 was waiting on. Step 13's three questions were asked in order; two
came back with an answer, and the third came back with the same answer it has
given three times.

### What was reported

> *"the camera illusion might be upside down, the character should be bottom
> part of screen when on ground but then smooth center to player when digging
> or flying out of frame. the ground plane is still not correct, the character
> should be standing within the ground plane. currently the player is standing
> on the test albedo.bmp setup with the backdrop"*

### What is in that, separated from what it implies

**Question 1 — does the movement read as answering the dig, or as wandering.**
Answered as neither cleanly: **"upside down"**, which is a third failure the
step did not offer and which is worth more than either of the two it did. The
tester then stated the framing they expected in full, and it is not a rejection
of the two-state idea — **on the ground, low; digging or airborne, centre.**
Two things in that differ from what shipped: the dig framing is named as
**centre**, where V23 built 0.30, and **being airborne is named as a trigger**,
where V23 triggers on the dig aim only.

**Question 2 — are ~55 cells below enough.** Not answered directly, and the
phrase **"or flying out of frame"** is the nearest thing to an answer. It
reports the player leaving the frame rather than the world below being too
shallow, which is a different complaint than the one the question was written
to collect.

**Question 3 — does the ground plane finally read as receding.** **No.** This
is the fourth no, and it is the one that gates V22. The tester repeats session
7b's observation in the same terms — the player stands on the fixture scene,
not in the plane — after a change that was aimed at the cause the geometry
actually had. **That the answer did not move is the finding**, and it is a
finding about the premise rather than about V23: camera framing was necessary
and is not sufficient.

**One thing the report names that no previous one did:** the fixture scene by
its file, `albedo.bmp`. Sessions 6, 7 and 7b described the surface; this one
points at the artefact that draws it.

Symptoms and measurements only. What causes each and what to do about it is
ROADMAP.md's.

## Session 9 — 2026-08-17: the camera, put back

Not a checklist pass. A direction, given after V23a shipped and before the
re-test it asked for was run.

### What was asked for

> *"lets go back to the camera always being centered."*

### What is in that

**The whole two-state framing is withdrawn, not retuned.** No number was named
and none was asked about; the request is for the behaviour the game had before
V23 — the player at mid screen, always. Read against session 8, which asked for
*low on the ground, centre when digging or airborne*, this reverses the first
half a day later and after one look at the corrected version.

**The three questions this list was carrying have no subject any more** — is
centred right, does jumping twitch, is the 0.35 s swing the right speed. They
were withdrawn rather than answered, and that is worth recording as a distinct
outcome from "answered no": nothing was measured about them.

**Nothing was said about the ground plane**, so session 8's fourth **no** stands
as the live answer on it. The camera is no longer a candidate route to it.

Symptoms and directions only. What was done about this is V23b in ROADMAP.md.
