# Playtest Log

Session records from running README's Manual Tester Checklist, and the supplementary checklists below, against a real build.

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
| A5 | Steam condenses back to water far too fast. | major | **open** — the steam half of E9 |
| A6 | Spawning material into water caps the water on top, then it bursts outward on release. | major | fixed — [wave 3](ROADMAP.md#wave-3--the-brush-destroyed-water-and-the-elevator-it-was-hiding), pending playtest |
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
- **B3 — steam should collect, wait, then drip increasingly fast and shrink as it goes.** *Tracked as the steam half of E9 — open. This and defect A5 are the same fix seen from two sides.*
- **B4 — rigid bodies should tip, topple and roll.** The observation underneath is that bodies falling flat and landing flat read as lifeless. *Tracked as E8 — open, and scoped to toppling only; the rolling half is deferred behind E5 for reasons in that item.*

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
| A6b | Water lifted to the top of a falling sand column and trickling back down its flanks in a staggered pattern. | major | fixed — [wave 3](ROADMAP.md#wave-3--the-brush-destroyed-water-and-the-elevator-it-was-hiding), pending playtest |

**Worth recording as a reporting pattern:** the first read was that A6's own fix had caused this. It had not — the behaviour predates wave 1 and was invisible because a second defect was eating the evidence. **A fix that makes a longstanding defect visible for the first time looks exactly like a fix that caused one**, which is the same lesson A1b taught from the rendering side, arriving from the physics side.

---

## Session 5 checklist — the pass that closes wave 3

**Not yet run.** [Wave 3](ROADMAP.md#wave-3--the-brush-destroyed-water-and-the-elevator-it-was-hiding)'s code is in and its exit condition is this session.

**Run README's Manual Tester Checklist first, in full** — wave 3 changed a grid write path and added a fluid-venting rule, so steps 1–9 all apply and step 4 in particular is about the brush.

**Record before starting:** build hash, Release/MSVC, world seed, the `Scene:` line, suite count.

### The one question this session exists to answer

Session 1's A6 reported two things — water capped on top of a spawned pile, and an outward burst on release. **The fix claims those are one defect and that the burst needed no separate fix**, because it was the deleted volume being repaid. That claim is a look, not a number, and no test asserts it. Row 3 is where it gets confirmed or contradicted.

| # | Check | What it catches |
|---|-------|-----------------|
| W-1 | Spawn sand into a pool with the brush and **hold the drag**. Water is pushed aside, not capped and not deleted. | A6. The measured version is 451 cells destroyed before, 0 after; this is whether that reads correctly in motion. |
| W-2 | **Release the brush** and watch the moment after. | The burst. It should now be absent or unremarkable rather than an outward surge. |
| W-3 | Pour sand from **well above** a pool — 25 cells of open air, so the brush never touches water — and watch the top of the column. | A6b, the elevator, in the exact configuration the screenshots showed. No water should appear above the cursor as the sand arrives. |
| W-4 | Keep pouring until the pile **stands out of the water** as a cone, then keep going. | The residue. A handful of cells (3 at step 350, 6 at step 500 on the probe) still reach the peak once the pile itself becomes the conveyor. **This is the first thing to look at if the screenshots repeat**, and the open question is whether it is visible at all. |
| W-5 | Watch water run **down the flanks** of a standing pile. Does it read as flow, or as the staggered clumps the original report described? | The trickle was the lifted water coming home. If the lift is gone and the staggering is not, that is a new finding about the flow model rather than about venting. |
| W-6 | Drag a sand brush through water and watch the **pool level**. Volume looks conserved — no shrinking pool. | The conservation half, by eye. The probe reports 2000 of 2000; this is whether anything visible contradicts it. |
| W-7 | HUD fps and awake-chunk count while churning sand and water together. | `VENT_RADIUS` is a cost knob set at the knee of a sweep (`churning` 3.13 → 4.93 ms/step). The sweep says this is affordable; the window is where that is confirmed. |

### What to bring back

Screenshots, in the same configuration as `water_issue_*.png` so the before and after are comparable — that is the whole reason those images are worth keeping.

Then, specifically: **W-2 and W-4.** Wave 3 closes on the burst being confirmed gone as a *consequence* of the conservation fix rather than as an unfixed second defect, and on the residue being judged either invisible or worth another pass.
