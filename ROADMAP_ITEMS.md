# Roadmap Items — the working plan

This file is **the plan**: what is next, how big it is, and what is blocking. [ROADMAP.md](ROADMAP.md) is the archive of *reasoning* — every item's full argument, the things that were tried and failed, and the measurements. When the two disagree about **order**, this file wins; when they disagree about **why**, that one does.

**A few terms used throughout:**

- **Cell / grid** — the world is a giant spreadsheet of tiny squares; each square ("cell") holds one material like sand or water.
- **Chunk** — a 64x64 block of cells. Blocks where nothing is moving get skipped to save time.
- **Fixed timestep** — the simulation always advances in equal 60-per-second ticks, no matter how fast the screen refreshes, so the physics behaves the same on every computer.
- **Deterministic** — same starting number (the "seed") plus same button presses always produces the exact same world. Makes bugs reproducible.
- **Headless test** — an automated check that runs the simulation with no window open, so it can't see anything visual.
- **Field** — a second, much coarser grid laid over the world (one entry per 4x4 block of cells) holding a number per block. The lighting already works this way. Two more items below use the same shape.

**Sizes** are stated on every open item and mean roughly: *afternoon* (<1 day), *days* (2–4), *week*, *weeks* (2–4), *large* (a month or more). An item with no size has not been thought about enough to schedule.

---

## ▶️ Next up

*Reviewed 2026-08-09. This block is the only part of the file that has to be re-read to know what to do.*

| # | Item | Size | Blocked on |
|---|---|---|---|
| 1 | **Session 5 playtest — closes Wave 3** | a session | a person at the keyboard |
| 2 | **E4 — Does the player push material?** | afternoon (a decision) | session 5 |
| 3 | **S0 — The run can be lost** | week | — |
| 4 | **E10 — Powders come to rest** | days | — *(P2 shipped)* |

Everything after these four is in [Running order](#-running-order) below.

**P2 shipped 2026-08-10** and is out of this block. Every measurement from here on is quoted against 1920x1080, the size the game actually runs — the table is in [PERFORMANCE.md](PERFORMANCE.md). The result worth carrying forward into the items below: **the engine pays for awake cells, not for cells.** `sparse`, which stands in for an ordinary gameplay frame, costs the same at 1920x1080 as at 960x540 — 1.00x for four times the cells — so an item that adds per-cell work to *awake* cells is the kind to price carefully, and one that adds world size is not. It also left one decision open, in the table below.

**Why S0 is fourth and this is the biggest change to the plan.** Everything shipped so far is engine or visual foundation, and the whole of the game sits behind the whole of the engine track. `VISION.md` already names that risk in writing — "under-building looks exactly like discipline right up until the playtest gate" — and then the running order used to do it anyway. S0 is the thin version of two Medium Term items pulled forward so the game becomes **losable** before another two months of engine work. It is also the only thing in the plan that can answer the two open [Decisions](#-decisions-owed) below by playing rather than by argument.

---

## 🧭 Running order

**E4 → S0 → E10 → E5a → E6 → V11 → (E7 + E11 + V9) → rest of the slice → playtest gate → E5b, P1/P3, E8**, with the session 5 playtest ahead of all of it. P2 was ahead of both and shipped on 2026-08-10.

The two structural changes from the previous order (`E4 → E5 → E6 → E7 → E8`, then V, then P, then the whole game):

- **A gameplay spike (S0) moves ahead of most of the engine track.** Argued above.
- **E5 splits in two.** E5a is per-cell velocity — the enabler that E6, E4's shove and V9's debris all actually need. E5b is the air/pressure field, which is the larger and more speculative half, and it goes after the slice. They used to be one item, which made the enabler look as expensive as the research project attached to it.

**Item IDs are stable and are not renumbered when the order changes** — four other documents cite them by name. E10, E11, E5a/E5b, S0, P3 and V11 are new here; nothing existing was renumbered.

---

## ❓ Decisions owed

*Four open loops that are decisions rather than work. A decision with no deadline never closes, so each has one. Each may be answered with "no" — that is a closed loop, not a failure.*

| Decision | Due | How it gets settled |
|---|---|---|
| **Does the player displace material?** (E4) | after session 5 | Play it. If the artifact isn't obviously better, write "no" in `ENGINEERING_NOTES.md`. |
| **Is there combat in v0.1?** | end of S0 | S0 makes the run losable; whether it needs an enemy to be interesting is then a thing you can feel. Currently neither in nor out, which is the worst of the three states. |
| **What is the hook?** | end of E6 | Deliberately unnamed so the design isn't locked to whichever comparison got written down first. S0 and E6 between them make it answerable by playing. |
| **Does the brush paint while paused?** | with the pause hotkey | Surfaced by F2.4 and never answered. Costs a sentence. |
| **What does the frame-budget rule trigger on, now that the budget is measured at the played size?** *(new — P2)* | before E10 | The standing rule is "if one item alone breaks the frame budget, P1 gets pulled forward". P2 was not an item and added no work, and it found `churning` at 211% and `cascading` at 241% of a frame at 1920x1080 — already over, with nothing having got slower. So the rule as written does not fire, and the question is whether it should be re-aimed at the rows that represent a real frame (`sparse` 0.4%, `burning` 44%) rather than at any row. Settled by writing one sentence into `ROADMAP.md` next to the rule. **The recommendation on the record is: do not pull P1 forward on this reading** — the two breaching scenarios are the synthetic ones `PERFORMANCE.md` already says the game does not produce, and pulling a week-long layout change forward to serve them is optimising the benchmark. Watch `burning` instead; it is the realistic scenario nearest the line and the one the E track will push. |

---

## 🟤 Engine & Visual Depth — the selling point

Three tracks: **E** deepens the simulation, **V** gives it a visual identity, **P** pays for both in performance. An item only gets in here if it can name (1) something in the built game that's actually wrong, and (2) what it unlocks later.

### The finding that reorganised this track

Four rules in `grid.cpp` were each added to fix a real, visible artifact. Each is correct. Each has the same shape:

| Rule | Where | What it costs |
|---|---|---|
| `vent_fluid` | grid.cpp:1020 | a 7x7 box scan, per powder-touching-fluid, per tick |
| `make_room_above` | grid.cpp:202 | a walk up to 32 cells, per painted cell |
| `find_lower_surface` | grid.cpp:948 | a search of up to 512 cells, per awake surface cell, per tick |
| `fall_if_unsupported` | grid.cpp:448 | a flood fill of up to 4096 cells, up to 8 times per tick |

Each has a magic radius that was picked by sweeping values and measuring. **The engine answers "where should this go?" by looking around, instead of by carrying state that already knows.** That is the difference between this engine and the ones it is measured against: Noita, The Powder Toy and Sandspiel each carry two things this one does not — **a speed on every cell** and **a coarse air/pressure field** — and nearly every entry in that table is a symptom of one of them being missing. E10, E5a and E5b are those two things, and between them they retire three of the four rows.

### E — Simulation depth

Running order: **E4 → E10 → E5a → E6 → E7 + E11 → E5b → E8.**

- **E1 — Liquids find their level.** ✅ Done. Water in connected containers evens out instead of sitting at different heights.
- **E2 — Heat, the seventh axis.** ✅ Done. Every cell has a temperature, and things ignite, melt or boil by crossing a threshold rather than by a random dice roll.
- **E3 — Collapses break instead of dropping rigid.** ✅ Done. An unsupported structure cracks apart along the stress line instead of descending in one perfect block like an elevator.

- **E4 — The player displaces material, or deliberately does not.** *(afternoon — a decision, possibly no code)* The grid doesn't know the player exists, so sand falls straight through the body. This item's output is a *decision*: try it in play, and if it isn't obviously better, write down "no" and stop thinking about it. If the answer is yes, the implementation waits for E5a, which is what gives shoved material somewhere to go.

- **E10 — Powders come to rest.** *(days — new, and the biggest single improvement to how the game feels per hour spent)* Sand currently has no friction at all: a grain rolls off any edge it can and then takes a second fall in the same tick, so piles can't hold a slope, sand behaves like very thin water, and a tunnel dug through a dune flattens completely instead of partly caving in. This is also why the three failed rules recorded at A7/A7b/A7c fought each other — "a rule aimed at motion kept catching rest, and a rule that spared rest stopped catching the defect" is an exact description of a system with no *rest state* to aim at.
    - **The fix is one number per material, called inertial resistance in the game this is measured against.** A settled grain is asleep and stays asleep until something disturbs it — a neighbour moved, plus a per-material dice roll. Once it's free it keeps sliding until it comes to rest again. That gives cones that hold their shape, avalanches that trigger and then *stop*, and gravel / sand / snow / ash as four table rows instead of four code paths.
    - **It costs no memory.** `Element::ticks` is one byte that only structural cells and Fire ever touch — `element.h` says as much in writing ("Powders and fluids… have no use for a clock"). That is a whole free byte sitting on exactly the class of material that needs it. The `static_assert` at `element.h:109` doesn't move.
    - **It claims that byte for good, and E5a is why the meaning is decided now rather than twice.** For a non-structural, non-Fire cell the byte becomes a **packed speed**: 4 bits of sideways, 4 bits of up-down, both signed, so −8 to +7 cells per tick each way. E10 only ever writes zero or "falling" and only ever reads "is this zero" — but the representation is E5a's, so E5a is not a rewrite of it. The −8..+7 range is not arbitrary: 8 cells per tick is already `MAX_FALL_SPEED` for structures, so the two speed limits in the engine agree by construction.
    - *Verify:* a poured pile holds a measurable angle instead of flattening; a tunnel roof partly collapses instead of fully; the benchmark's `cascading` and `churning` numbers do not regress, because a resting grain now does *less* work than it did.

- **E5a — Velocity means something.** *(weeks — the first half of the old E5)* Nothing in the world has a speed. A grain dug out from under a pile falls at exactly the rate of a grain blasted out of it, because movement is a rule applied once per tick rather than a speed being integrated. Nothing can be thrown, splashed, sprayed or knocked. Three later items stand on this.
    - **It lives on the cell, in the grid — not in a separate list of in-flight particles**, and that is a reversal of how this item used to be written. A separate list means every rule in the engine needs a second implementation or an explicit "no": does a grain in flight conduct heat? react? get lit? cast a shadow? The games this is measured against all keep the moving cell *in* the grid and have it walk a straight line through the cells it crosses each tick, testing each one. One entity, one set of rules, no boundary to maintain. A genuinely separate particle list is still the right answer for things that must move smoothly between cells and interact with nothing at all — which is exactly V9's effects layer, already scoped that way.
    - **The note at grid.cpp:743 argues against the wrong thing, and it should be edited rather than deleted.** It records that powder acceleration was tried and removed for three measured reasons: motion got choppier, a continuous stream stratified into sheets, and `cascading` went 13.1 → 19.7 ms. All three are consequences of applying free-fall acceleration to *every falling grain*, and all three go away when speed is only non-zero because something *put* it there. Gravity accumulates on cells that are already moving, not on every grain in a stream — and since `place()` builds a fresh cell, a brush-stamped grain starts at zero like its neighbours, which is the whole of the stratification bug. A resting grain stays a one-cell-per-tick mover, so the common case is unchanged and the frame-time regression does not recur. Choppiness was always a *drawing* property of whole-cell movement and cannot be fixed in the simulation at all, which the note itself says.
    - **No new memory** — it is E10's byte, now carrying real values.
    - **Four traps, each a known failure of something this engine already does.** *Conservation:* a moving cell is still in the grid, so the existing conservation test keeps working — which is a real advantage of this design over the old one and should be stated in the test. *The wake rule:* a cell arriving in a sleeping chunk must wake it, same as every other write. *One definition of solid:* what stops a moving cell is `is_solid`, the same function the player and the dig ray use. *A ceiling:* the 4-bit range is the ceiling, and it makes tunnelling impossible by construction rather than by being fast enough.
    - *Verify:* same seed and same input gives byte-identical results (F1's invariant extended to the new axis, not assumed to survive it); a cell fired at a wall at full speed lands *against* it; a world that has been disturbed and then settles goes fully back to sleep.

- **E6 — Explosions.** *(week — nearly free once E5a lands)* Right now the only way to change the world is a dig that deletes a fixed sphere on a cooldown. There is no force in the engine at all. An explosion is a radius, a falloff, a heat deposit, a conversion pass and an impulse handed to E5a — five stages, four of which are axes that already exist. It is the single most impressive thing this engine can put on a screen, and it is one of the two things that make the hook question answerable by playing.
    - **It must not become a second destruction system.** The dig tool is a degenerate explosion — a radius with no impulse, heat or falloff. Two implementations will drift, and the drift will show up as a bug in fracture.
    - **First cost in the engine whose worst case a player chooses.** Cost goes as radius squared; it needs a ceiling, and the measurement has to be taken *at* the ceiling.

- **E7 — Breadth: more rows, not more code.** *(days per material, ongoing)* There are only eight materials and six interactions, so "what happens if I put X on Y" almost never has an answer. Ice, snow, acid, gunpowder, molten stone and smoke are mostly new table rows rather than new code, thanks to E2. Sequenced after E6 so rows aren't authored against half an engine. **The bound:** a row earns its place by making an interaction something a player can discover and be right about.

- **E11 — The cheap columns heat and fluids are missing.** *(days — new)* Four small gaps found reading the tables, grouped because they are all one column or one short rule and none of them justifies its own item.
    - **Specific heat.** `conductivity` is currently doing two jobs — how fast heat moves *through* a material and how much heat it takes to warm it. One extra column separates them, and then water is a genuine heat sink and metal is a fast conductor that stays hot. No engine work.
    - **Radiant heat.** Fire only heats what it physically touches, so standing next to a bonfire costs nothing. Fine until S0 makes fire able to hurt you, at which point it is the difference between a hazard and a trap.
    - **Viscosity.** `spread` (5 for water, 3 for oil) is the only fluid knob there is, so honey, tar and lava all have to be the same substance with a different number.
    - **Lateral flow is a jump, not a flow** (grid.cpp:933) — a liquid moving sideways swaps straight to the far end of the run without touching the cells in between. Standard for this kind of engine and **no action is proposed**; it is recorded because it is why streams read as snapping rather than pouring, so nobody spends a day looking for the bug.

- **E5b — The air field.** *(large — the second half of the old E5, and it absorbs the item that used to be called "gas pressure")* A second coarse grid over the world, one entry per 4x4 block, holding pressure and a velocity. **The pattern is already built and shipped:** the lighting is exactly this — a low-resolution, whole-number, reproducible grid stretched over the scene with one draw call. One system delivers six things that are currently separate gaps:
    - gas pressure (steam in a sealed room does nothing today);
    - explosions that *push* rather than delete-and-throw;
    - steam and smoke that drift and curl instead of rising in columns — the single thing that most makes a sandbox look alive;
    - wind, which V9's sparks and embers need in order not to look like a screensaver;
    - fire suffocating in a sealed space, which E9 currently cannot express;
    - and the pressure term that **retires `find_lower_surface`, `vent_fluid` and `make_room_above` together** — three of the four rows in the table above.
    - **Two honest costs, both of which are why this is after the slice.** The reference implementation uses decimals, and this engine forbids them in the simulation for reproducibility, so it needs a whole-number port — doable, and non-negotiable. And it is a *fixed* cost proportional to the awake area rather than one that scales with how much is moving, which is a different shape from everything else in this engine and needs its own PERFORMANCE.md entry rather than a bracketed measurement.
    - **It also buys something the current design has permanently ruled out.** `find_lower_surface` levels water by teleporting a cell across the body, which is instant and non-local, so waves, sloshing and surges are impossible *by construction* — not missing, impossible. Pressure propagates over several ticks, so they fall out for free.

- **E8 — Toppling, and rigid bodies properly.** *(large — deferred past v0.1, and the reason it is deferred has changed)* Structures currently drop or break but never tip over, which reads as lifeless. **This used to be written as "may close as not possible without wrecking the pixel art", and that sentence is withdrawn** — it closes a door the reference engines walk through, and someone reading this in a year would believe it. The objection was that rotating a piece resamples it and destroys the authored pixels. The reference answer: trace the outline of the connected piece, simplify it to a polygon, hand *that* to a rigid-body solver, take the piece's cells out of the grid entirely, and stamp them back in each frame from the body's own private copy of its pixels. The pixels are never resampled — they live with the body and are only *drawn* rotated. Rotation becomes a drawing problem, which is a solved one.
    - **The real objection, recorded as the real one:** it is bigger than everything else in the E track combined, it wants a physics library (a dependency, against the no-bloat rule) or a hand-rolled solver, and nothing in the slice needs it. It also *retires* the eight-flood-fills-per-tick cost in `resolve_support` and makes fracture and explosions act on bodies properly, so it gets better with age rather than worse. Not for v0.1.

- **E9 — Fuel, and a clock for steam.** *(days — half done)* The fire half is done. The remaining half: give steam its own condensation timer, so a puff can last a while without having to be dangerously hot, and so steam collects on ceilings and drips. E5b would give it suffocation as well.

### V — Visual identity

Running order: **V11 → V9 → V8 → V7-rest**, with **V10.1** alongside E6.

- **V1 / V2 / V5 / V6 / V3 / V3.1 / V7-emissive / V10** — ✅ Done. Transparent empty space and a backdrop; a palette tuned against it; the art direction written down; one locked palette shared by code and art with a validator; the player sprite decoupled from its hitbox and then animated; fire casting light; the reticle and material hotbar.

- **V11 — Make the visual system adaptable.** *(week — new, and it is the item that makes changing direction cheap instead of expensive)* The stated expectation is that the art direction will change several times. Right now a direction change is expensive in five specific, findable places, and none of them is hard to fix *today*.
    - **There is no renderer.** About 350 lines of frame composition sit inline in `main.cpp`, with the layer order hard-coded: clear → sky → mountains → props → world → player → light → reticle → HUD → hotbar → menu. `notes/reference_observations.txt` has *already* concluded that a mid-ground layer is needed that this stack has no slot for. Extract `render/frame.cpp` holding an explicit ordered list of layers. Afternoon; afterwards, adding a depth band is one list entry instead of surgery between two comments.
    - **Material colours are compile-time constants.** Changing direction means editing `MATERIALS`, recompiling, and re-checking the level file's colour codes. Give each row a *palette slot* and put slot → colour in a loadable theme file. The correctness pass already learned this exact lesson once, when it separated level colour codes from the render palette; this is one more step of the same move. Then a second biome is a file, and time-of-day is two files and a blend. **The cost is bounded and worth stating:** the pixel buffer holds baked colours, so switching theme needs one pass over the world — a one-off at swap time, not a per-frame cost, and the hot loop is untouched.
    - **`Camera::SCALE` is a compile-time constant** and it is baked into the reticle size, the sprite offsets and the prop rectangles. A zoomed-out biome, a different cell size, and the already-planned resolution options all collide with it. Making it a runtime value is far cheaper now than after three more systems read it.
    - **The light layer can only add light**, so every biome will be the same brightness. Generalise it from add-only to multiply-and-add — an exposure and tint term as well as a glow term — and night, underground, fog and per-biome colour grading all arrive without touching a single material colour. Given that the reference finding was that the read comes from *silhouette layering, not detail*, this is the highest-value visual knob available and the one that most makes direction changes cheap.
    - **The parallax numbers are duplicated** between `main.cpp` and `tools/generate_backdrop.py` with nothing checking they agree; the failure is a seam at the pan limit. Generate the header from the tool, exactly as V3.1 did for the player sheet.

- **V9 — A non-simulated effects layer, and impact feel.** *(week)* Sparks, embers, dust and smoke wisps drawn on top, interacting with nothing, from a fixed-size pool so a big event can't tank the frame rate. Sequenced after E6 because explosions are what it exists to dress.

- **V10.1 — Screen shake and hit-stop.** *(days — alongside E6)* Camera shake and a brief freeze on impact make explosions read as force. The trap: driven by the fixed simulation clock, not the display refresh, or their speed changes with frame rate.

- **V4 — Props at more than one depth, and `Snow`.** *(days)* The prop format shipped. `Snow` is now an E7 row rather than a V item, since heat makes it melt by table.

- **V8 — The backdrop: a second biome, time-of-day, a third depth layer.** *(weeks)* **Held deliberately.** Sky, mountains and parallax ship. The rest cannot currently name anything in the built game that reads badly, so it would be reference-driven breadth wearing engine clothes. V11 is the piece of this that *is* real, and it is scheduled above. Revisit when a second biome actually needs to exist.

- **V7-rest — Lighting that darkens.** *(week)* Today it only lights *hot* things and only ever adds. The darkening half is folded into V11's tint layer; what stays here is non-fire light sources.

### P — Performance

Running order: **~~P2~~ → P1 → P3.**

- ✅ **P2 — Re-baseline the benchmark at the size the game actually runs.** *(done 2026-08-10)* The benchmark measured 960x540 and the game runs 1920x1080, so every budget on record was quoted against a world a quarter the real size. Both sizes now run, with 960x540 kept as the historical series and as a control on the refactor. **Headline: `sparse` is 1.00x at four times the cells** — the engine pays for awake cells, not for cells. `churning` (211%) and `cascading` (241%) are over budget at the played size and always were; nothing got slower. Full entry in [ROADMAP.md](ROADMAP.md#p--performance), table in [PERFORMANCE.md](PERFORMANCE.md).
- **P1 — Split the cell array hot from cold.** *(week)* The simulation is limited by how fast data can be pulled from RAM. Most of each cell's data is read every tick; the colour is only read when the cell is drawn. Separating them means the hot loop reads less. Sequenced after E5a so the layout is settled against the final field set.
- **P3 — Run the chunks in parallel.** *(weeks — new)* The whole simulation is single-threaded. The reference engine updates chunks in four alternating passes arranged so that two chunks being updated at the same time are never neighbours, which makes edge writes safe and keeps the result reproducible because the pass order is fixed. **The 64x64 chunks and dirty rectangles here are already most of the prerequisite.** Worth naming next to P1 for scale: P1 buys tens of percent, this buys a multiple.

---

## 🟡 The slice — the actual game

**S0 is pulled forward out of this section; everything else stays behind Engine & Visual Depth.**

- **S0 — The run can be lost.** *(week — new, and it is item 4 in the plan)* Today fire is fully simulated and cannot hurt you, there is no way to fail, and there is nothing to do. This is the thin version of two items below, built now rather than in two months.
    - **What it is:** player health; damage from fire (the engine already supplies the temperature) and from landing too fast (it already supplies the speed); one hard-coded objective somewhere in the test scene; reaching it ends the run as a win, dying ends it as a loss. Death reuses `Run::reset(seed)` — it must not be a second code path.
    - **What it is not:** no generator, no save file, no pet agent, no economy, no UI beyond a health readout on the existing HUD. Those stay in the full items below.
    - **Why now:** it converts the sandbox into something that can be won and lost, which is the smallest possible thing that produces a *direction*. It also answers two of the four open [Decisions](#-decisions-owed) by playing rather than by argument, and both of those are currently blocking this whole section.
    - **Keep the direction of the dependency**, which is the rule `tool.cpp` established: the grid does not know about bodies, bodies read the grid. Damage is the player *asking* what it is standing in, the same way collision does. Not a health field on `Grid`, not a damage column on `Element`.

- **Quantum Worlds.** *(weeks)* A portal / level-generation system so the player can enter a single "trial". One world, one generator; variety comes later. Generation must draw only from the world-gen random streams F1.5 reserved, or adding one cave silently changes how sand falls elsewhere.
- **Player health and death — the full version.** *(days on top of S0)* Everything S0 stubbed: a proper damage model, more than two sources, and a real death presentation.
- **Objective + Extraction — the full version.** *(week)* A real objective type placed by the generator rather than hard-coded, and an extraction that is a place rather than a flag.
- **Save and persistence.** *(week)* Nothing writes a file yet, but "earns coins idly" requires progress to survive quitting. Needs a format, a location, and a deliberate answer for loading saves from older builds. **Check E5a before assuming F1's arithmetic still holds** — cells now carry speed, which is state a save has to include.
- **The Pet ML Agent.** *(weeks)* The companion that "watches" the player and improves from completed runs. Deterministic scripted progression, not actual machine learning.
- **Proof-of-Work Economy.** *(weeks)* The idle loop — the agent performs tasks between runs and earns coins.
- **Playtest gate.** Put the slice in front of people who didn't build it. Explicitly: do not proceed past this line on your own opinion. The two decisions above should have been *asked* before getting here, not answered by getting here.

---

## 🔵 Presentation & Tooling (after the slice, before polish)

Not blocking the slice. None of this makes the game fun; it makes it shippable and easier to work on.

### Sandbox / debug tooling

*The first three are each an afternoon and each removes a recurring cost from every item above. Worth taking as a single batch the next time something is blocked.*

- **World reset hotkey (`R`).** Wipe the world and respawn, so a messy test scene clears without restarting.
- **Pause and single-step (`P` / `.`).** Freeze the simulation and advance one tick at a time, so a collapse or reaction can be inspected instead of flying past. Carries the open brush-while-paused decision.
- **A free camera.** The camera only follows the player, so anything the player isn't standing next to can't be checked by eye. **This one is close to load-bearing for E10 and E5a** — both are judged on what a pile or a thrown cell does over a distance.
- **Continuous brush strokes.** The brush stamps once per tick at the cursor, so a fast drag leaves a dotted line. Stamp along the line between the last position and the current one.
- **Brush outline preview.** A hollow ring at the brush's size under the cursor, so the footprint is visible before committing paint.
- **Cell inspector.** Material name, temperature and piece ID under the cursor. **Grows a job with E10 and E5a** — speed is per-cell state with no other way to see it, and "why is that grain not moving" becomes a real question.

### Window and display

- **Display modes: fullscreen, borderless, windowed.** Exclusive vs. borderless as distinct options, alt-tab that doesn't break the window, the choice remembered between launches, and changing mode mid-game.
- **Resolution options (1920x1080, 2560x1440, 3440x1440).** The hard part is solved. What's left is the mode list, saving the choice, and one fairness question: an ultrawide sees more of the world, which is a gameplay decision rather than a display setting. **Overlaps V11's runtime scale** — take them together.

### Art

- **Custom pixel art and animation generator.** An in-project tool for authoring sprites. Flagged as a risk — a second application with its own UI and file format — and only earns its keep once hand-authoring is measurably the bottleneck.

### Shipping

- **Packaging and a release build.** There's no answer today to "how does someone who isn't you run this". Needs a named artifact, a version string, and a definition of what a release contains.
- **Build on macOS and Linux at least once.** Cross-platform has been claimed for several revisions on the strength of using no platform-specific code — a reasonable expectation, not a verified fact.
- **Crash diagnosis: assertions and a log.** Nothing logs anything, and there's one unexplained crash from a prior session never reproduced. Since a run is a seed plus an input log, a crash report could be a file that reproduces the crash rather than a description of it.
- **Audio.** No sound of any kind. Correctly deferred — but an explosion is the one thing that's actively worse silent than absent, so revisit at E6.
- **Settle on one project name.** The README, the window title and the repo folder all say different things. Trivial now, not trivial once something is published.

---

## 🌊 Waves — sub-plans that jump the queue

A **wave** is a batch of urgent fixes (usually from a playtest) that gets worked to completion before normal roadmap work resumes. Each one must state up front what counts as "finished".

| Wave | What it covers | State |
|---|---|---|
| **Wave 1** | Rendering, brush and powder bugs found in playtest session 1. | closed |
| **Wave 2** | First attempt at fire fuel and burn duration. | replaced by 2b |
| **Wave 2b** | Fire simulation rebuilt from scratch, plus glow lighting pulled forward. | closed |
| **Wave 2c** | Tuning the glow's shape/reach and the fire's timing and shape. | closed |
| **Wave 3** | Brush destroying water instead of pushing it aside, and the "water elevator" bug that hid behind it. | **code done — session 5 playtest is item 2 in the plan** |

### Wave 1 — rendering, brush and powder defects

- **A1 — The player rectangle jittered and ghosted.** The character stuttered while moving. Three separate causes: position was rounded to whole squares, the display refreshed faster than the physics updated, and the camera could only sit on whole squares so the whole *world* jerked instead.
- **A1b — Fixing the display revealed a hidden physics bug.** Landing on the ground reset the player's speed but not the leftover fraction of movement still queued, so the body slowly sank and snapped back. Invisible before because tests only checked whole squares.
- **A2 — The on-screen readout lagged material switches by up to a second.** The text was only rebuilt once per second, so it showed stale information. Fixed first because a wrong readout makes every other measurement untrustworthy.
- **A7 — Falling liquid threw out horizontal sticks.** Water was spending its "how far can I spread sideways" budget crossing open air instead of flowing along a surface.
- **A7b — The real complaint was sand, not water.** Sand cascades diagonally within a single update, so a whole row lands one step ahead of the row beneath it, leaving a shelf sticking out of the pile. Screenshots are what identified this.
- **A7c — Shelves and vertical columns are two failures of one rule.** Fixes that removed the shelves made piles grow straight up, and vice versa. Solved by letting a rolling grain finish its fall in the same step, so a shelf never exists even for a moment. **E10 is the proper fix and supersedes this** — all three of these fought the absence of a rest state.
- **Powder acceleration — added then removed.** Making grains speed up as they fall was meant to look smoother; because grains move in whole squares it just made each jump twice as big, which looked *more* choppy, and it cost a lot of performance. **See E5a** — the measurement was good and the conclusion drawn from it was too broad.
- **A8 — Moving material painted black trails.** Empty space had two conflicting colour definitions, so anything that moved left opaque black behind it, erasing the artwork underneath.
- **B1 — The dig marker was hard to see.** It was a filled orange square, the same colour family as fire — so it disappeared against the thing you most want to aim at. Shipped as part of V10.

### Waves 2 and 2b — fire rebuilt

- **C1 — Wood burned too fast.** Turned out to be about how quickly fire *spreads*, not how long it burns. Fixed by tuning wood's ignition temperature and how well it conducts heat.
- **C2 — Flames rose too fast.** Fire moved exactly one square per tick; the fix was to occasionally skip a tick, since a square is the smallest move possible.
- **C3 — Burnt wood was jet black.** Near-black over a dark blue background reads as a hole in the world rather than as charred material.
- **C4 — Burnt wood needed to last slightly longer.** The setting wasn't fine-grained enough to express the wanted value, so its precision was increased. Lesson: a tuning dial's precision is set by the smallest change anyone will want, not the largest value it holds.
- **C5 / C6 — Flames all died at exactly the same height.** Every flame lived the same number of ticks and rose at the same rate, producing a perfectly flat line across the top of a fire. Fixed by randomising each flame's lifespan.
- **C7 — Flame colours lacked intensity.** The colour blend from white-hot to dull red passed through greys, so the most visible part of a flame's life was its least colourful. Fixed by bending the blend through a saturated orange.
- **C8 — The lighting was blown out (too bright).** Several causes: light faded far too slowly over distance, one stray flame lit as brightly as a wall of fire, there was no brightness ceiling handling, and the glow spread in a diamond shape instead of a circle.
- **`preview_light` — a new visual test tool.** Builds a burning scene, combines all the layers exactly like the real game does, and saves the image plus brightness statistics. Existing tests all passed the broken frame because none of them ever combined layers.

### Wave 2c — the glow's shape, and the fire's timing

- **B9a — Hard light rays and shafts.** The glow bulged along the up/down/left/right axes because diagonal distance was calculated slightly wrong and the error compounded. Fixed with exact diagonal maths, extra "knight's move" light steps, and a smoothing pass that can't leak light through walls.
- **B9b — Light reached too far.** Small reduction to how much light survives each square of open air.
- **B9c — A single stray flame lit as brightly as a bonfire.** Halved the minimum brightness a mostly-empty area emits, so an ember and a blaze are clearly different.
- **B9d — Thin walls let light through.** Two stacked modelling mistakes: light blocking was being averaged when it should be multiplied, and wall opacity was measured per 4-cell block rather than per cell. A one-cell wall now blocks light properly.
- **A9a–A9d — Fire timing and shape, tuned as one change.** Fire spreads slower, wood takes longer to be consumed, char lasts longer, and the burn front is more ragged. Achieved by lowering wood's heat conduction and randomising each cell's ignition temperature slightly downward only (randomising both ways could make a fire refuse to cross a stick).

### Wave 3 — the brush destroyed water

- **A6 — Spawning material into water deleted the water.** The brush overwrote whatever was there, so dragging sand through a pool destroyed hundreds of cells of water, then "burst" when released. Fixed with a dedicated brush write path that pushes the occupant upward instead of erasing it.
- **A6b — The water elevator.** Falling sand kept swapping places with the water beneath it, and each swap lifts water one square — under a continuous stream this hands water all the way up the column. Fixed by sending displaced water sideways to its own surface instead of straight up.
- **`VENT_RADIUS`** — how far displaced water searches for somewhere to go. Measured at several values; 3 gave the best quality-for-cost. **E5b retires this rule entirely.**

---

## ✅ Shipped

Kept in full because the reasoning is the valuable part.

### The engine and its harness

- **Initial repo and build system.** ✅
- **Barebones C++/SDL2 pixel physics prototype.** ✅ Sand, water and wall interacting.
- **Data-driven material system.** ✅ Materials are rows in a table, not branches in code. Four generic behaviours (Static / Powder / Liquid / Gas) drive all movement, with density deciding what sinks through what.
- **Eight materials.** ✅ Sand, Water, Wall, Wood, Oil, Steam, Fire, plus transparent Empty — each cost one table row and no engine changes.
- **Fixed timestep.** ✅ The simulation runs at a constant 60 ticks per second regardless of display refresh rate.
- **Headless test harness.** ✅ The simulation runs with no window, covered by automated tests for conservation of matter, density layering, and border sealing.
- **Chunked dirty-rect updates.** ✅ The world is split into 64x64 blocks that track what can still move; settled blocks are skipped entirely, so cost scales with how much is *moving* rather than with world size.
- **Benchmark.** ✅ Times seven scenarios against the 60 Hz budget, at both 960x540 and the played 1920x1080 since P2. Deliberately not a pass/fail test — timings inform, they don't gate.
- **Reactions.** ✅ A data-driven table of catalyst + target → result. Fire ignites wood and oil, is doused into steam by water, and burns out on its own.
- **Player Character + Player Physics.** ✅ A rigid body that is *not* a grid cell — it has its own position and only reads the grid to ask "is this solid?". Uses whole squares plus a fraction rather than decimals, so edge-case collision bugs never arise.
- **Structures fall as rigid bodies.** ✅ An unsupported wall falls as one piece keeping its shape, rather than dissolving into loose gravel (which read as broken rather than physical).
- **Basic Interaction — digging.** ✅ A ray traced from the player to the first solid cell, which is then blown out to a small radius. Deliberately its own module rather than a method on the player, so the player's grid access can stay read-only.

### Foundations (F1–F4)

Four items that weren't features — each was a prerequisite for several slice items and got more expensive the longer it waited.

#### F1 — Determinism, first half: the simulation

- **F1.1 — Take a seed, keep the generator.** ✅ The world now takes a starting number and can report it back. A seed you can't read back is only half of reproducibility.
- **F1.2 — Add a wide step counter.** ✅ A counter of how many ticks have elapsed, wide enough not to repeat. The existing one-byte counter would have made randomness repeat every 256 ticks — visible patterning, not noise.
- **F1.3 — Introduce the hash, move one call site.** ✅ Replaced the traditional random number generator with a *stateless hash*: randomness is calculated fresh from (position, tick, seed, purpose), so there's no hidden generator state to save or get out of sync.
- **F1.4 — Move the remaining four call sites.** ✅ All randomness now goes through the hash, each purpose on its own separate "stream" so unrelated decisions can't correlate.
- **F1.5 — Reserve the stream separation, don't build it yet.** ✅ Wrote down the rule that world generation and simulation must draw from separate random streams, before there was a generator to break it — otherwise adding one extra cave would silently change how sand falls elsewhere.
- **F1.6 — Delete the old generator, then measure.** ✅ Measured honestly: the hash is about 1.7–1.9% *slower* in one scenario. Recorded as-is rather than as the win it was filed as.
- **F1.7 — Docs.** ✅ Wrote the determinism rules into the README, including an explicit note about what was *not* yet true.

#### F2 — Something owns the run

- **F2.1 — Create `Run` and move the three loose variables into it.** ✅ The world, player and dig tool were three unrelated local variables; four separate future features all needed to reset/save/swap them together.
- **F2.2 — `reset(seed)` on both the grid and the run.** ✅ Wipes every changeable field by hand rather than using a compiler shortcut, specifically so the test proving nothing is forgotten is possible to write.
- **F2.3 — An `Input` struct and `Run::step(input)`.** ✅ The second half of determinism. Input used to be sampled once per *drawn frame* and applied to every simulation tick in it, so a held key did different things at 30 fps and 144 fps. Now a run is a seed plus a replayable list of inputs.
- **F2.4 — Confirm the payoff rather than assuming it.** ✅ No code — checked that the reset hotkey and pause/single-step items are now genuinely small, and surfaced one unresolved policy question (should the brush paint while paused?).

#### F3 — Camera and world-space coordinates

- **F3.1 — Separate world size from window size.** ✅ The world used to be exactly the size of the window, so a level couldn't be bigger than one screen.
- **F3.2 — One `Camera`, and only it knows the pixel scale.** ✅ The conversion between world coordinates and screen coordinates was scattered across five places; now it lives in one file. **V11 revisits the "compile-time" half of this.**
- **F3.3 — Upload only what is visible.** ✅ Only the visible portion of the world is sent to the graphics card each frame, so the cost stops growing with world size.
- **F3.4 — Follow the player, clamped at the world edges.** ✅ The camera centres on the player and stops at the world's borders instead of showing empty space beyond them.
- **F3.5 — Answer off-screen simulation once, in writing.** ✅ Everything simulates always, regardless of where the camera is looking. If it didn't, an avalanche would freeze the moment you walked away — and two players with identical inputs could diverge just from looking at different things.

#### F4 — A way to get a level into the grid

- **F4.1 — `paint(x, y, type, colour)`.** ✅ Placing a cell with an explicit colour instead of a random shade, which is what lets hand-drawn artwork keep its own pixels.
- **F4.2 — Scene format and a headless loader.** ✅ A level is two same-size images: one saying which material each cell is, one saying what colour it should be.
- **F4.3 — BMP decoding, loaded at startup.** ✅ Shipped broken with three separate bugs (wrong row stride, scene bigger than the grid so every cell was silently dropped, and assets not copied next to the executable) — all fixed.
- **F4.4 — Make the first scene a test fixture wearing art.** ✅ A hand-generated scene where every feature exercises something specific: uneven stairs for step-up, fence posts for dig-the-base collapse, a pit with pillars for the collapse flood fill, a water channel, and jump ledges.

### E1–E3 — Simulation depth (details)

- **E1 — Liquids find their level.** ✅ A surface water cell looks through its own connected body for a lower surface and moves there. The obvious approach — letting the low side *rise* — doesn't work: it leaves a bubble that splits the body in two and the whole thing falls asleep two cells out of level. **E5b replaces the mechanism** — see the note there on what a search can never do that a field can.
- **E2 — Heat, the seventh axis.** ✅ Every cell carries a temperature; heat flows between touching cells; reactions have temperature windows. First attempt cost 18% performance and was rejected — a cell already at room temperature now does no thermal work at all, bringing it to ~2%.
- **E3 — Collapses break instead of dropping rigid.** ✅ A landing structure cracks along the boundary between the columns that hit something and the columns that hit nothing. Uses a per-cell "piece ID", so a crack is stored as two cells *disagreeing* rather than as a line, which lets it survive the piece moving.

### V1–V2 — Visual foundation (details)

- **V1 — Transparent empty and a backdrop layer.** ✅ Two changes that only work together: empty cells became see-through *and* the world texture was set to blend. Either alone is a no-op.
- **V2 — Palette and jitter pass.** ✅ Colours chosen against the new backdrop rather than against black; only fire and water keep full saturation, since they're the two things you must never miss. Shipped a critical bug: retuning the colours made the level file match nothing and the game booted to a blank world for a whole commit. **V11's theme file is the structural fix for the class of problem this was.**

### Correctness pass — a full read of the source

A line-by-line review of every source file, with each suspected bug reproduced by a test program before it was believed.

- **The startup scene loaded as an entirely empty world.** ✅ Fixed by separating the level file's colour codes (permanent markers) from the render palette (free to change), so retuning art can never invalidate a level again.
- **A settled pool of water never went to sleep.** ✅ A pool only settled if its cell count happened to divide evenly by its width — so almost every real puddle churned forever. Fixed by requiring a sideways move to land somewhere it can actually rest.
- **Steam was an undeclared ignition source.** ✅ Steam spawned hotter than wood's ignition point, so putting a fire out with water was a way of starting a bigger one. Now enforced at compile time: nothing may spawn hotter than the coldest ignition point.
- **Smaller things, each verified rather than assumed.** ✅ An overflow check that itself overflowed, a shared scratch buffer whose name hid a second user, and two stale comments — one describing a guarantee that measurement contradicted, one warning about a defect that had already been fixed.

### What the correctness pass changed about how to work here

- **A green test suite proved less than it looked like.** Both serious bugs lived exactly where tests couldn't see — one in an untested file, one in a case a test had deliberately excluded. **When a test comment explains why a case is skipped, that comment is a bug report.**
- **The manual checklist is load-bearing and wasn't run.** One launch would have caught the blank world immediately.
- **Two of the bugs were table edits with no code touched.** Data-driven design moves the danger into the *relationships between rows*, which have no compiler checking them unless one is written deliberately.
