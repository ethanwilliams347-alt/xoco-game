# Game Roadmap

This document tracks the concrete, sequenced engineering work — what's next,
what's done, and why each step is ordered the way it is. The project's goals and
scope discipline, performance methodology, and long-lived technical decisions
live in their own documents:

- **[VISION.md](VISION.md)** — project goals, scope discipline (why the plan is
  sized the way it is), and the Long Term wish list.
- **[PERFORMANCE.md](PERFORMANCE.md)** — `grid_bench` numbers, measurement
  methodology, and the mistakes that methodology exists to prevent. **Also how
  to run the bench**, since `W6` moved that procedure in from README.
- **[ENGINEERING_NOTES.md](ENGINEERING_NOTES.md)** — deferred technical
  decisions and the reasoning behind them, and — since `W6` —
  **[Engine Architecture](ENGINEERING_NOTES.md#engine-architecture)**, how the
  simulation works.
- **[README.md](README.md)** — how to build, run, and test the game, the
  controls, and the short `## General Testing` fundamentals pass. **A front door
  since `W6`**: everything longer links out.
- **[MANUAL_TESTING.md](MANUAL_TESTING.md)** — the Manual Tester Checklist in
  full (**thirteen steps as of 2026-08-17**), and the list of what is currently
  owed to the tester.
- **[PLAYTEST_LOG.md](PLAYTEST_LOG.md)** — what was asked at each playtest and
  what came back. **Questions and answers only.** No plan, no root causes, no
  fixes: a defect's symptom is recorded there and everything downstream of it —
  why it happened, what was done, what that cost — lives here, in the wave that
  spent it.
- **[ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md)** — closed work, moved out of this
  file. **Nothing is ever required to read it**; anything in it that still binds
  open work was absorbed into the item it binds before it went.
- **`notes/`** — informal lore and feature brainstorming, upstream of anything
  here.

**Three conventions govern this file, all learned the hard way.**

**This is the only document that carries development steps.** The order of work
used to live at the bottom of `PLAYTEST_LOG.md`, on the reasoning that a roadmap
absorbing every playtest finding stops being readable as a statement of intent.
The reasoning was sound and the remedy was not: two documents each holding part
of the sequence meant neither was authoritative, and the wave table went a full
session describing the material hotbar as queued after it had shipped. **A plan
split across two files is a plan that disagrees with itself.** The fix is
[Waves](ROADMAP_ARCHIVE.md#-waves--sub-plans-that-preempt-the-tracks) — findings
are absorbed here without being interleaved into the tracks, which is what the
split was reaching for.

**That sentence then went false for months, in the way it warned about, and
`W4` made it true again on 2026-08-17.** `ROADMAP_ITEMS.md` was split out to hold
the *order* while this file held the *why* — so every item was written twice and
maintained in step, all 48 IDs in both files across 582 KB, and this paragraph
sat above the arrangement it forbids. The items file is now merged into this one
and deleted. **An item's order and its argument go in the same entry**; if the
plan ever wants a second file again, read the `W4` entry first.

**Live work comes first, finished work leaves the file.** Everything here is
open; closed work is in
[ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md), kept for its reasoning rather than its
status and **required reading by nothing**. An early revision interleaved the two
and read as a task list while being sixty percent retrospective — a reader could
not find the next item without scrolling past four completed sections written in
the imperative. Moving the finished sections to the bottom fixed the reading
order and not the size: on 2026-08-17 the `## ✅ Shipped` section was 86 KB of a
448 KB file, and the other 362 KB was **also** mostly closed work, sitting inside
the live tracks. `W4` is the item that took the rest of it out.

**Steps point at code by name — a function, or a line quoted out of it — never
by line number.** F1.1 on its own shifted `grid.cpp` by five lines, `grid.h` by
seventeen and `main.cpp` by fourteen, which silently falsified every numeric
reference in this document including the one describing the bug F1.1 had just
fixed. A plan read across many sessions cannot use an anchor that moves every
time the plan is executed.

**A few terms, and what the sizes mean.** *(Moved here from `ROADMAP_ITEMS.md`
by `W4` when that file was merged into this one.)*

**A few terms used throughout:**

- **Cell / grid** — the world is a giant spreadsheet of tiny squares; each
  square ("cell") holds one material like sand or water.
- **Chunk** — a 64x64 block of cells. Blocks where nothing is moving get skipped
  to save time.
- **Fixed timestep** — the simulation always advances in equal 60-per-second
  ticks, no matter how fast the screen refreshes, so the physics behaves the
  same on every computer.
- **Deterministic** — same starting number (the "seed") plus same button presses
  always produces the exact same world. Makes bugs reproducible.
- **Headless test** — an automated check that runs the simulation with no window
  open, so it can't see anything visual.
- **Field** — a second, much coarser grid laid over the world (one entry per 4x4
  block of cells) holding a number per block. The lighting already works this
  way. Two more items below use the same shape.

**Sizes** are stated on every open item and mean roughly: *afternoon* (<1 day),
*days* (2–4), *week*, *weeks* (2–4), *large* (a month or more). An item with no
size has not been thought about enough to schedule.

---

## ▶️ The plan

*Moved here from `ROADMAP_ITEMS.md` by `W4` on 2026-08-17, when that file was
merged into this one and deleted. **This block is the only part of the file that
has to be re-read to know what to do.***

### ▶️ Next up

*Reviewed 2026-08-11 after an external review of the plan.*

| # | Item | Size | Blocked on |
|---|---|---|---|
| ~~1~~ | ~~**Wave 4 — session 5's five defects**~~ | ~~days~~ | ✅ **closed 2026-08-12** — D2 and D1 shipped 2026-08-11, D3, D6 and D7 on 2026-08-12, the last two accepted on a look |
| ~~2~~ | ~~**E9 (steam half) — steam collects, then drips**~~ | ~~days~~ | ✅ **fully closed 2026-08-13** — built, tested and benchmarked 2026-08-12; **checklist step 5 passed 2026-08-13**, which confirms A5, B3 and D5 — the same report three times in four sessions ([spot check](PLAYTEST_LOG.md#spot-check--2026-08-13--the-two-owed-steps-run-together)) |
| ~~3~~ | ~~**F5 — Fixed-point player kinematics**~~ | ~~days~~ | ✅ **closed 2026-08-12** — built and trace-verified; **playtested 2026-08-13, checklist step 2 passed with no difference reported** ([spot check](PLAYTEST_LOG.md#spot-check--2026-08-13--the-playtest-f5-owed)) |
| ~~4~~ | ~~**The instrumentation sitting**~~ | ~~2 days~~ | ✅ **closed 2026-08-13, four of four** — `ticks`, `P4`, `VENT_RADIUS` and the fluid spike, plus `churning` demoted. **S0 is now next.** Two of the four answers contradicted something already on the record; the price it produced for E5b (8% of a played step) is the one that changes another item — see [P4](#p--performance) |
| ~~5~~ | ~~**S0 — The run can be lost**~~ | ~~week~~ | ✅ **built 2026-08-14, and it took two days rather than a week.** Health, contact-heat and fall damage, one objective, win and loss both through `Run::reset(seed)`, `HP` on the HUD. **Playtested 2026-08-14 and checklist step 10 passed** — the mechanical half returned nothing ([spot check](PLAYTEST_LOG.md#spot-check--2026-08-14--step-10-runs-and-the-question-is-held-open)). **The design half is deliberately unanswered:** the combat decision was held open rather than closed on the first sitting with it, which is a decision about the decision and is recorded as one below. Findings in [ROADMAP.md](ROADMAP.md#-medium-term-core-gameplay-loop) |
| ~~6~~ | ~~**T1 — The debug tooling batch**~~ | ~~2 days~~ | ✅ **shipped 2026-08-14, and it took an afternoon rather than two days.** Pause and single-step, a free camera, the cell inspector, an unconditional world reset. **It found a false claim in the engine on the first thing it was pointed at** — `active_chunk_count() == 0` does not mean the world is at rest, because a falling slab is carried by the support queue and not by the chunk rects. Entry in [ROADMAP.md](ROADMAP.md#t1-the-debug-tooling-batch) |
| ~~7~~ | ~~**E10 — Powders come to rest**~~ | ~~days~~ | ⏸️ **held 2026-08-16, deliberately, in favour of the V-track block below.** Nothing about E10 changed and neither blocker came back; it was unblocked and ready and got out-prioritised. See the note under the table |
| 8 | **V-track: the renderer block** | ~2 weeks | ⏸️ **steps 0–3, 4a and 4b done. V23/V23a shipped and V23b deleted them again** on 2026-08-17 at the tester's direction, so the camera contributes nothing to the block's outcome and step 7 (V22) is unblocked but back to its original constraint — see the V23b row and the note below |
| 9 | **W-track: the workbench** | ~3 days | ✅ **closed 2026-08-18** — `W1`–`W4` shipped 2026-08-17, `W5`'s three parts 2026-08-17/18, **`W6` 2026-08-18**. **The one thing left open in it was closed as a decision on 2026-08-18: `W5` gets no part 4** — see the end of that entry. **`V22` is next**, and the tester's UI look came back good the same day, so it is unblocked. **`W4` chose the strict archive boundary** — *finished and nothing open depends on the reasoning* — so `ROADMAP_ITEMS.md` is gone and this file is the whole plan |

**Item 8 in detail, because it is a block rather than an item.** Started
2026-08-16 on a request for a split-view and parallax backdrop system. Run
strictly in this order; each step's reason for coming before the next is the
point.

| step | what | size | state |
|---|---|---|---|
| 0 | **Rewrite the two dead `notes/` files** — `art_direction.txt` and `reference_observations.txt`, fresh against the CnC frames | afternoon | ✅ **done 2026-08-16** |
| 1 | **V17 — the golden-frame check.** Split in two: extract the inline composition to `render/frame.cpp` *changing nothing*, then checksum that frame | afternoon | ✅ **done 2026-08-16** |
| 2 | **V11 core** — the ordered layer list, a mid-ground band, parallax onto `Camera`, the factors generated into a header | days | ✅ **done 2026-08-16** |
| 3 | **V11's tint bullet + V7-rest** — the light pass gains a multiply so it can darken | week | ✅ **done 2026-08-16** *(an afternoon; V7-rest's non-fire sources are **not** in it — see below)*. **Checklist steps 12, 11 and 5 all passed the same day** ([spot check](PLAYTEST_LOG.md#spot-check--2026-08-16--the-grade-pass-is-looked-at)) — 12 on both halves, so the 0.60 grade has now been looked at and is not over-corrected |
| 4 | **V19 — the seven-band scene, with a ground plane where the reference has water** | week | **in progress** — 4a and 4b done 2026-08-16, **4c next**. Sub-steps below |
| 5 | **V18 — write the split view down, build none of it** | afternoon | queued behind V19 |
| 6 | **V23 — the camera's vertical anchor, and the dig framing that moves it** | afternoon | ✅ **done 2026-08-17**, and it was **not on this list yesterday** — see the note below |
| 6a | **V23a — the dig framing was never delivered** | afternoon | ✅ **done 2026-08-17** on playtest session 8. `DIG_ANCHOR` 0.30 → `COLUMN_ANCHOR` 0.50 and airborne became a second trigger. **The clamp was swallowing the framing** — 0.30 resolved to 0.51 on screen at the fixture floor and 0.70 deeper, so the camera answered the dig least where there was most to see. **The feel re-test it owed was never run** — session 9 withdrew the whole framing hours later (row 6b) |
| 6b | **V23b — the camera goes back to centre** | an hour | ✅ **done 2026-08-17** on playtest session 9: *"lets go back to the camera always being centered."* **A deletion, not a retune** — `camera_bias.h`, its suite, `Camera::set_vertical_anchor` and the three TUNING rows are gone, and the golden checksum returned to its pre-V23 value `0xcde4dc1a39927fca`, which is the evidence the revert is complete. Suite count unchanged at 14 (`camera_test` keeps the Camera half). **Nothing is owed to the tester by this.** ⚠️ **It hands V22 back the ~50% cap** the centred camera puts on the plane's visible share |
| 7 | **V22 — the plane the player is in** | week | **in progress, started 2026-08-18. Part 2 is now blocked on step 8.** **Part 1 (the framing) is done**; part 2 is the fixture-scene rewrite and part 3 the world/plane value junction. ⚠️ **It runs in full by decision on 2026-08-18, over a stated concern** — the gate answered "no" a fourth time and the two-thirds target needs the camera V23b deleted, so part 1 reopens that deletion. **Read the session 8 note and the reopening bullet below before touching part 2**, which is the step that costs both `.rec` recordings |
| 8 | **V24 — the plane's near edge is nailed to the window** | afternoon | ✅ **built 2026-08-18**, from playtest session 10, ahead of V22 part 2. The plane's near edge was pinned to the bottom of the window while its far edge was parallaxed, so the tile never left the frame and was squeezed 30% across the camera's travel — an inverted depth ordering inside one layer. `PLANE_TEXEL_SCALE` 2.5 replaces the pin and the window is no longer an input to the plane's geometry. **A motion playtest is owed**; no `.rec` cost |

> **Step 6 was not planned and that is the thing to notice about it.** It exists
> because V22's scene work started with a measurement instead of a redraw, and
> the measurement said the scene was the wrong instrument: `Camera::follow`
> centred strictly, so the receding plane's visible band capped at ~50% **by
> construction** and measured 20.2% at the spawn, against a reference reading of
> two thirds. **An afternoon of scene authoring would have moved that to about
> 22% and invalidated both recorded sessions to do it.** The general form is
> worth keeping — *when the plan is to author against a target, compute the
> target's reachability first* — because the cost of not doing it here would
> have been paid entirely in a human's afternoon, not in a build step.

**V19 in detail, because it is four commits and not one.** The gate it had to
fire first is answered — [gate
screenshot](PLAYTEST_LOG.md#gate-screenshot--2026-08-16--does-terrain-fill-the-mid-ground-band),
the mid band is 70% bare sky, so the near ridge and the treeline have somewhere
to go. **Do not re-ask it.**

| step | what | state |
|---|---|---|
| 4a | **The wrapping arithmetic** — `render/backdrop_wrap.h`, a thirteenth suite linking no source set at all, nothing calling it and the checksum unmoved | ✅ done 2026-08-16 (`76a3d30`) |
| 4b | **The ground plane** — the strip loop, the tile, the layer row and its grade. The item's centre of gravity, and the first thing in V19 to move the checksum | ✅ done 2026-08-16 |
| 4c | **The far range, the near ridge and the treeline** — three rows reusing 4b's draw path, one colour and a shade each. **The treeline is the one band authored warm**; everything else stays on the cool ramp. **No longer a *shore* treeline** — the decision came back land, so it stands on ground rather than across water | **unblocked 2026-08-16; queued behind V22** |
| 4d | **The value ladder tuned against entry 7**, once all seven bands exist and can be judged together | queued |

> **4c was blocked for part of one day and the block paid for itself** *(blocked
> and released 2026-08-16, playtest session 7)*. The decision was **"is the
> receding plane land, or water the player is on?"**, and it came back **land,
> with the player in the plane** — closed in [Decisions owed](#-decisions-owed).
> Two of 4c's three bands survived either answer; the third was written here as
> the *shore* treeline, and **there is no shore, so it is a treeline standing on
> land** — one band re-aimed before a pixel of it was authored, which is the
> whole return on holding the step. *(The original reasoning is kept because it
> is the reusable part.)* Authoring first would have been authoring against a
> scene that was about to change. V20 and V21 spent two rounds retuning the
> ladder these bands hang from, which is the same mistake one level down —
> **settle what the plane is before adding bands beside it.**
>
> **4c now queues behind V22 rather than behind nothing, and the order is not
> arbitrary.** V22 changes what the plane's *near* end reads as; 4c authors
> three bands at its *far* end. Doing 4c first would tune three new bands
> against a junction that is about to move, and 4d exists to tune the whole
> ladder once anyway — so the cheap order is V22, then 4c, then 4d. **This is
> the third time in this block that the fix was ordering rather than work.**

> **What 4b left open on purpose, and it is not an oversight to close inside
> 4c.** In the reference the plane is *brighter* than what stands on it; ours
> runs the other way, with the world row at grade 1.00. Grading the world down
> is coherent — grade multiplies and light adds, so fire lights it back up — but
> it changes how the play area reads while digging. **That is a `TUNING.md` row
> and a playtest, not an implementation detail**, and it must not be settled
> inside a commit that is about something else.
>
> **Checklist steps 11 and 12 are owed on 4b and have not been run.** 11 matters
> most: the layer table changed shape *and* a wrapping layer is a new way to
> produce a seam, plus 24 strips is a new way to produce a stair-step. 12 is the
> grade, watching for over-correction. `golden_frame_test` does not cover either
> — it hashes a software rasterisation and is blind to a GPU-only defect.


> **Why this displaced E10, stated so the re-order is a decision and not a
> drift.** E10 is a good item and was correctly next. What moved it is that
> **the V track's first four items all turned out to be the same item**: V11
> says there is no renderer, V17 says nothing checks the composed frame, V7-rest
> says the light pass cannot darken, and V8's remainder wants a third depth band
> — and every one of those is blocked on the ~350 lines of frame composition
> sitting inline in `main.cpp`. Doing them one at a time means extracting that
> code three times. This is the sitting where they get done together, and after
> it the V track is one item deep instead of four.
>
> **Step 0 was scheduled as documentation and returned two engine findings**,
> which is the argument for having run it first rather than last. The
> **band-value defect** — our four depth bands overlap so completely that
> nothing but the rim highlight separates them — is measured, is why a busy
> frame reads flat, and **is exactly what step 3 exists to make fixable**, since
> correcting a band's value range is a multiply and the light pass can only add.
> And the **parallax factors were never measured**: an attempt to derive them
> from the reference failed because the three "parallax" frames are three
> generated lakes rather than one pan, so step 2 must ship the *existing*
> eyeballed factors and retune separately, with the golden frame proving the
> refactor was a no-op. Both are written up in
> [notes/reference_observations.txt](notes/reference_observations.txt); the
> correction to V5 is filed beside its entry in ROADMAP.md.
>
> **Step 1 landed 2026-08-16, and one number in this table was wrong.** The
> composition was **~175 lines, not ~350** — the larger figure counted the
> reticle, the HUD, the hotbar, the run-over wash and the settings menu along
> with it, and those are UI. They stayed in `main.cpp`, and the boundary is not
> filing: V7's light pass is the last thing drawn in the world, and everything
> after it is deliberately not lit. Recorded rather than corrected in place
> because the ~350 estimate is quoted in V11's and V17's entries in ROADMAP.md
> too, and it is the *composition* half that mattered to both.
>
> Two things the step produced that were not in the plan. `FRAME_SOURCES` is a
> **fifth** source-set variable, kept out of `RENDER_SOURCES` because
> `frame.cpp` is the only rendering source that calls SDL and folding it in
> would make nine headless suites link SDL2. And `golden_frame_test` is the
> **first test target in the project that links SDL** — `SlopPhysics` was the
> only one before it. It still needs no display.
>
> **What the checksum does and does not prove.** It was verified sensitive
> before being trusted: swapping the sky and mountain draws moves it, and so
> does one cell of camera travel. It cannot retroactively prove the extraction
> was a no-op — there was no checksum before the move, so that claim rests on
> the diff being verbatim — but from here it is what makes step 2's restructure
> checkable. It hashes the *software* rasterisation and is blind to a GPU-only
> defect; do not quote it as covering the shipped frame.
>
> **Step 2 landed 2026-08-16, an afternoon rather than days, and the sequence it
> was verified in is worth more than the code.** Four bullets: the composition
> is an ordered `Layer` table in `frame.cpp`, the parallax offset is
> `Camera::parallax_origin_x/y(factor)`, the factors generate into
> `src/render/backdrop_layers.h`, and a mid-ground band sits between the
> mountains and the world. **Three of those four must change no pixel, and they
> were built and run first, against V17's checksum, which held unchanged at
> `0x3d729ad7fbcaa839`.** Only then was the band added and the number moved to
> `0x06f6412da7af6607`. So the restructure is a *measured* no-op rather than an
> asserted one — which is exactly what V17's entry said it could not do for the
> extraction itself, there being no checksum before that move. **The golden
> frame paid for itself one commit after it was built**, and the transferable
> form is that the order to do it in is: build the no-op half, prove it, then
> build the half that changes something.
>
> **The mid-ground band was built and then removed the same day, and that is the
> step's most valuable result.** It went in as a slot at 0.40/0.16 and went out
> on checklist step 11, which was written to ask the one question
> [reference_observations.txt](notes/reference_observations.txt) entry 4 had
> raised against itself: our world is 1080 cells tall and the camera sees 270,
> so the band the reference fills by hand may be one our *terrain* already
> fills. **Played and looked at 2026-08-16: no gap, the terrain fills it.** The
> entry is marked disproved beside its original text rather than rewritten.
> **The half worth carrying is why it did not transfer** — the observation is
> correct about the reference and its *mechanism* is hand-composition: a
> painting must author that band because nothing else will occupy it, and we
> simulate 800 cells of ground into the same space.
> `reference_observations.txt`'s header names that constraint in advance and
> entry 4 is the first observation to fail it. Two secondary findings survive
> the deletion: **depth here is a renderer problem before it is a layer-count
> problem** (entry 2's overlap is what makes a busy frame read flat, and no
> number of bands fixes it — step 3's multiply does), and **a near band is the
> most expensive layer in the stack**, 32 MB at 0.40, which is a much better
> argument for V16 than the duplication ever was.
>
> **The checksum returned to V17's exact value, `0x3d729ad7fbcaa839`, and that
> is a second independent proof.** The first proof was procedural — restructure
> first, run it against the old number, then add the band. The second is
> arithmetic: with the band removed, the whole of V11 composes the
> byte-identical frame the inline code composed at V8. **`git log -S` on that
> constant will show three commits where the file's history reads as one**,
> which is a thing to know before quoting it.
>
> **And it is the strongest evidence V11 has for its own admission argument.**
> The item was admitted on "adding a band is one entry rather than surgery
> between two comments". What actually got measured was the *removal* — one
> table row, one draw function, one factor, one fixture texture, in and out
> inside a day. Changing your mind was always the expensive direction, and that
> is the direction that got priced.
>
> Checklist step 11 stays, minus the band question: it is the parallax-ordering
> and pan-limit-seam check, and it keeps the reopen trigger — a location whose
> terrain does not fill the band, or a zoomed-out camera once `Camera::SCALE` is
> runtime.
>
> **Two smaller things the step produced that were not in the plan.** The
> `Lighting` field V11's sixth bullet asked for is **three values and not four,
> deliberately**: V11's own text wants V16's animated bands behind the world
> *and* untinted by the light pass, and that is not representable while the
> light pass is one additive full-screen copy — position is the whole of what
> "lit" can mean today. It is enforced by `static_assert` over the table rather
> than described in a comment, and the assert was verified by reordering the
> table and watching the build fail with the right message. And **the seam at
> the pan limit is now a printed line**: the generated header carries each
> layer's expected size, so `main.cpp` warns at startup when the loaded BMP is
> smaller, instead of the defect waiting at the far edge of the map for somebody
> to walk to it.
>
> **Step 3 landed 2026-08-16 and it is the first commit in this project's
> history where the composed frame changed on purpose.** The mechanism is two
> things that are one idea at two scopes: a `Grade` — a plain RGB multiply — on
> every row of the layer table, applied with `SDL_SetTextureColorMod`; and a
> `Lighting::Grade` row holding a full-screen `SDL_BLENDMODE_MOD` quad driven by
> `Params::world_grade`. Step 2's procedure was reused exactly and it is now the
> house style: **the entire mechanism was built with every grade at identity and
> run against `0x3d729ad7fbcaa839` first, which held.** Only then did the one
> real value go in. So the checksum's move to `0x9d9e92a81c4df07b` has exactly
> one cause and the diff cannot be hiding a second.
>
> **The item was estimated at a week and took an afternoon, and the estimate was
> wrong for a specific reason worth keeping.** "The light pass gains a multiply"
> was priced as the item that would finally spend `SDL_ComposeCustomBlendMode` —
> the escape hatch the renderer-vs-shader refusal has been holding in reserve
> through two examinations. It needed no custom blend mode at all:
> `SDL_BLENDMODE_MOD` is stock SDL and `SDL_SetTextureColorMod` does the
> per-layer half for free, with no extra draw call. **All three named escape
> hatches are still unspent, and the first item that looked like it would spend
> one did not.** That is evidence for the refusal, not merely an absence of
> evidence against it.
>
> **The one measured change, and it is the first retune in TUNING.md that was
> measured rather than judged by eye.** Entry 2 of
> [notes/reference_observations.txt](notes/reference_observations.txt) said the
> depth bands do not separate by value. Re-measured in luminance against the
> shipped BMPs, **the entry understated its own finding**: the sky averages 26
> and the mountains are *flat 28* — p05 and p95 both 28, no internal variation
> at all — so the two most distant bands in the frame sit two levels apart out
> of 255, and the far one is the **brighter** of the pair. The mountains now
> carry a 0.60 grade; the pair reads 26 against 16 and the band is a silhouette.
> **The direction is darker-with-nearness, and the instinct that says otherwise
> is a daylight instinct** — aerial perspective washes distant things *toward*
> the sky, which is right at noon and backwards at night, when the sky is the
> only bright thing in the frame.
>
> **A correction to entry 2's reasoning, which is not a correction to its
> finding.** The entry says the repair is "pushing a band's value range down
> *globally*", and global is precisely what it cannot be: a frame-wide multiply
> scales every band by the same factor and leaves every ratio between them
> exactly where it was. Separation is necessarily per-layer. Both knobs got
> built, and keeping them distinct is the point — reading entry 2 as an argument
> for the world-wide grade would be reading it as an argument for the one knob
> that could not have answered it.
>
> **`Lighting` went from three values to four and the invariant went from a
> boundary to a rank**, which is the shape V11's layer table was supposed to
> have. The order is Lit → Grade → Light → Unlit and it is load-bearing, not a
> convention: **the grade multiplies and the light pass adds, so
> grade-then-light is the difference between a fire that survives nightfall and
> one that gets dimmed by it.** Three `static_assert`s hold it — exactly one
> light pass, *at most* one grade (zero is a coherent build, two compose into a
> third grade nothing declares), and the grade row's own per-layer grade must be
> identity or it multiplies twice with neither number saying so. All three were
> verified against the code they catch, then the file confirmed byte-identical
> to its backup.
>
> **The golden test grew a second instrument, and the first attempt at it was a
> bad one that is recorded rather than deleted.** A checksum says "not the frame
> I expected" and is deliberately blind to *how* — useless for a claim like "the
> multiply darkens" — so the suite now measures mean and peak luminance. The
> ordering check was first written as "the brightest pixel stays bright", which
> is a true statement about the design and a bad instrument for it: the
> fixture's fire already saturates at 255, so a correct ordering and a reversed
> one both squeeze against the clip and land 12% apart. Measuring the light
> pass's *contribution* — compose with and without it, subtract — separates them
> cleanly at 1.0 versus 0.5. **The lesson is that a check can be aimed at the
> right claim and still not be able to see it**, and saturation is where that
> happens.
>
> **One thing shipped without a caller, on stated terms.** Nothing sets
> `Params::world_grade`; at identity the quad is not drawn at all, so the grade
> pass currently returns before its first draw call on every frame the game
> composes. It ships because the per-layer half of the same mechanism is live
> and proves the multiply works, and because the ordering argument is the
> expensive part to add later. **It becomes a defect the day it is still unset
> and the ordering claim has stopped being checked by anything.** Trigger for
> spending it: V8's time-of-day, or a second biome. If neither arrives, the row
> gets deleted rather than left as decoration.
>
> **V7-rest is not closed by this and the step table's title oversold it.** The
> multiply — the darkening half — is done and is what the block needed.
> **Non-fire light sources are untouched**, and that is all V7-rest has ever
> been apart from the multiply. It stays open at its own entry, one item
> smaller.
>
> **V19 was inserted ahead of V18 on 2026-08-16, by request, and the ordering is
> the request's rather than an inference.** The ask was to build a scene
> composed the way `CnC_parallax_*` is composed *before* going down the
> underground-cave split-view path, which is what V18 writes up. Nothing about
> V18 changed and it is not blocked; it is one item further down. **V19 is
> admitted by the request and not by the reference** —
> `notes/reference_observations.txt` says at its head that reference answers
> "what is possible" and never "what is wrong here", so entries 7 and 8 are what
> the item is built *against*, not what admitted it. Full entry in
> [ROADMAP.md](ROADMAP.md#v--visual-identity).
>
> **The measurement that shaped it, and the one number in it that is
> corroboration rather than a finding.** Entry 7 walked a single column of
> `CnC_parallax_1` and segmented it into luminance plateaus: **seven layers, not
> the three we ship**, descending at about 0.78 per band and compounding to a
> factor of 3.4 from sky to nearest. The reference's mid-mountain band compounds
> to 0.63 and **ours has sat at 0.60 since step 3**, chosen from a luminance
> measurement against our own sky with nobody having seen this ladder. Two
> independent routes to within 5% is the reason the ladder is worth transferring
> at all.
>
> **The water layer is the interesting half and three of its four mechanisms
> survive being turned into land.** The value ladder, the horizon being the
> darkest line in the frame (row-mean luminance bottoms out at the waterline: 69
> against 156 at the top and 140 at the bottom), and contrast growing with
> nearness — 87 of the frame's levels are spent between the plane's near edge
> and the silhouette standing on it, against 14 to 45 at every join further
> back. The fourth, the reflection, is the one that does not transfer, and it
> was measured anyway so a later reader can see what we chose to drop. **What
> replaces it is a mechanism and not an appearance:** the reference's water is
> bright near the camera because it mirrors the sky, and land cannot — but a
> horizontal surface faces the sky where a vertical silhouette does not, which
> gets the same brightening at night for a different reason. That distinction is
> the rule entry 4's deleted mid-ground band was bought with, applied before the
> fact this time.
>
> **Two traps are already known and one of them can waste the item.** The near
> ridge and the treeline land in exactly the mid-ground band V11 built and
> deleted the same day, because **our terrain already fills that space**; the
> reopen trigger recorded then is a location whose terrain does not, and **this
> item has to fire that trigger by screenshot before authoring anything into the
> band** — if it comes back the same way, the honest outcome is a five-layer
> scene and the deletion stands twice. And the cost is real: a layer covers the
> window plus the pan range at its own factor, so `--sizes` prices the new bands
> at 17-32 MB each and **five of them roughly triples `assets/`**. A ground
> plane is the nearest band, the worst case, and exactly the kind of texture
> that tiles — so **V16 may have to be pulled into V19 rather than following
> it**, which is a sequencing question to settle before authoring rather than
> after.
>
> **Everything else is held, not cancelled.** E10 keeps its place at the head of
> the queue when this block closes.

**Item 9 in detail, because it is also a block rather than an item.** Opened
2026-08-17 out of an external review of the repo, and it is **the thing to start
on next**. The full reasoning, the measurements and the two things the review
checked and found *healthy* are in
[ROADMAP.md](ROADMAP.md#️-w--the-workbench-how-this-project-is-worked-on) — this
table is the order and the sizes only. **`W1`, `W2` and `W4` all shipped
2026-08-17, and `W3` shipped the same day.** `W3` was held behind `W4` on
purpose — it pins doc numbers to their sources and `W4` moved the very lines it
would pin — and with that dependency discharged it pinned numbers that will
hold, including the two `W4` produced: the live-plan and archive sizes. **`W5`
was the one item in the track that needed the tester afterwards, and `W6` closed
the track on 2026-08-18.** The table is kept as the record of what the track
bought; nothing in it is queued.

| # | what | size | state |
|---|---|---|---|
| W1 | **Reflow `ROADMAP.md`, `ROADMAP_ITEMS.md` (since deleted by W4), `ENGINEERING_NOTES.md`, `PERFORMANCE.md` to 80 columns.** Pure formatting; no wording, no decision, no number changes | afternoon | **shipped 2026-08-17** |
| W2 | **`.claude/settings.json`** — a permission allowlist for `cmake`, `ctest`, `git`, `python tools/*`. The repo has no settings file at all today | afternoon | **shipped 2026-08-17** — 40 rules, because every entry is written once per shell and `git` had to be split verb by verb; no `deny` list, on purpose |
| W3 | **The doc-truth suite** — a fifteenth `ctest` asserting the docs' checkable numbers against their sources (suite count, `Element` size, golden checksum, `FIXTURE_SCENE_CELLS`) | afternoon | **shipped 2026-08-17** as `docs_test`, seven checks, plus the checklist length and the two `W4` sizes; every check was verified against a mutated doc before being kept |
| W4 | **One live plan file; shipped rationale to a dated `ROADMAP_ARCHIVE.md`.** Includes the `CLAUDE.md` routing-table row that causes the split — that is part of the item, not a follow-up | days | **shipped 2026-08-17**, in two commits — a verified-lossless relocation, then the merge and the per-entry sweep. Boundary chosen: *finished **and** nothing open depends on the reasoning* |
| W5 | **Extract `main()`** — a `boot` unit, the shell's decisions, the UI composition, and a `main()` of ~150 lines | days | **parts 1–3 shipped 2026-08-17/18**: `game/boot.h` + `boot_test`; `game/pacer.h` and `game/settings_menu.h` + `shell_test`; `render/overlay.cpp` + a second golden checksum. **The acceptance check is met** — two of checklist step 1's launch lines are assertions now. `main.cpp` 1,625 → **1,395**. **The ~150-line target is not met and, on 2026-08-18, will not be**: there is no part 4. What remains in `main()` is the frame's wiring and the input handling, and the case for moving them was length alone |
| W6 | **Trim `README.md` to a front door** — architecture to `ENGINEERING_NOTES.md`, benchmark procedure to `PERFORMANCE.md`, `## General Testing` untouched | afternoon | **shipped 2026-08-18** — 917 lines → **259**. 536 lines of architecture and 153 of benchmark procedure relocated verbatim; the anchors moved with them and every inbound link was repointed. **`W6` closes the W track** |

> **Why this goes ahead of V22, since V22 was "next" as of yesterday.** **V22 is
> blocked on a human and none of W1–W6 are blocked on anything.** The V23 feel
> report is owed, V22 must not start until it comes back, and question 3 of that
> report can still change what V22 *is*. Running this track inside that window
> costs the V track nothing. Two reasons that would hold regardless: **W5 is
> aimed at the queue V22 is sitting in** — every checklist step it converts into
> a headless assertion is one the tester no longer runs before a visual change
> can ship — and **W1 and W4 are proportional to the size of the corpus, so they
> are cheapest now and never cheaper again.**
>
> **The three numbers that admitted the track.** Docs are 1.09 MB against 1.20
> MB of `src/` + `tests/` + `tools/`. A measured `grep -C2` into `ROADMAP.md`
> returned **9,930 bytes for 20 lines**, because the file averages 394
> characters to the line and a grep hit is atomic at the line. And **all 48
> roadmap item IDs appear in both roadmap files** — a perfect overlap across 582
> KB, mandated by the routing table rather than produced by carelessness.
>
> **What this track is not.** The volume of writing is not the defect and
> reducing it is not the goal; the review's own finding was that the
> documentation discipline here is real and that the docs it checked are
> accurate. **Every item changes where reasoning is stored or how it is found,
> and none of them is licence to record less.**
>
> **`Grid` was examined and is not in this track.** `grid.cpp` is 1,664 lines of
> 35 named methods averaging 47 lines; the 14 separate test executables, the
> `.claude/rules/` split, `TUNING.md` and `PLAYTEST_LOG.md` are all load-bearing
> and stay as they are. Recorded so the question is not re-opened by the next
> reader who notices the file size.

Everything after these nine is in [Running order](#-running-order) below.

> **S0 was estimated at a week and took two days, and the reason is worth
> recording against the estimate rather than celebrated.** The week was priced
> on the item's *scope* — health, two damage sources, an objective, win and
> loss, a readout — and every one of those was as small as it looked. What the
> estimate did not contain, and what actually cost the time, was **two
> interactions with things already built**: `Run` spawns the body in mid-air, so
> the first thing every run contained was a terminal-velocity fall priced at 80
> of 100 health; and `Run::reset` mid-session would have silently invalidated
> P4's session log. Neither is in this item's description and both are in its
> build. **The transferable form: a spike's estimate covers what it builds, and
> what it costs is what it touches.** Both findings are written up at the item.
>
> **It is also the first item in this plan whose output is a question rather
> than a feature.** The combat decision is due now and cannot be answered from
> the desk — see [Decisions owed](#-decisions-owed).

**~~F5 left one thing behind…~~ ✅ Closed 2026-08-13 as `F6`.** `DigTool::march`
is integer-only: the range test is squared, the step count is an `isqrt` of an
integer quotient, and the two `lround`s are a rounded integer division that
keeps `lround`'s halves-away-from-zero so the ray stays symmetric about the
origin. **No float under `src/physics/` reaches the grid any more.** *(Stated
that way on purpose. The older wording here — "the only float left under
`src/physics/`" — was never quite true: `DigTool::swing_progress()` is a float
and stays one, because only the animation reads it, the same boundary
`Player::visual_x()`/`visual_y()` sit on.)* Two things worth carrying:

- **It was an afternoon, as estimated, and it found a live bug that had nothing
  to do with determinism.** `dx*dx + dy*dy` was computed in `int` and overflows
  at a distant aim, which silently removed the range limit. The test that
  catches it fails against the unfixed code. Not reachable from the mouse — the
  cursor is bounded by the window — but the arithmetic is 64-bit now regardless.
- **It closed a disagreement nobody had noticed, and that is the argument for
  the wording of checklist step 3.** The crosshair's dim-past-range indicator in
  `main.cpp` **already** compared `dx*dx + dy*dy` against `RANGE*RANGE` in
  integers, while `march` compared a `float` length — two answers to one
  question, computed two ways, in two files, and able to differ by a cell at the
  boundary. Exactly the shape D1's two clocks had. They are now the same
  arithmetic. ✅ **Checklist step 3 passed 2026-08-13** ([spot
  check](PLAYTEST_LOG.md#spot-check--2026-08-13--the-two-owed-steps-run-together))
  — the range boundary was not reported off by a cell in either direction, and
  no diagonal was reported reaching further than a straight shot. **F6 owes
  nothing further.**
- **"Determinism is portable" is now *unblocked*, not *established*.** The known
  reason a second machine would answer "no" is gone; nothing has still ever been
  built on one. That is the gate prerequisite's job and it is unchanged.
  Reasoning in
  [ROADMAP.md](ROADMAP.md#f6--digtoolmarch-is-integer-only-shipped-2026-08-13)
  and [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md) under the F5 entry.

**This block was rewritten on 2026-08-11 by a review of the plan rather than by
a playtest, and that is a first.** The review's twelve findings are answered
across this file; nine were corrections to what was written here and three added
work. What actually changed about *what to do next*: the fluid spike is no
longer a lone item but the largest of four questions one set of instruments
answers together (item 4); **F5 and T1 are new and both are prerequisites of
things already in this block** — S0 reads a velocity that is currently a float,
and E10 and E5a are both judged by eye with no free camera and no way to read a
cell's state; and Wave 4 gained **D3**, which was the only session-5 defect with
no owner anywhere in this file.

**Two of the review's findings were unresolved by design and carried as
[decisions owed](#-decisions-owed)**: what `Element::ticks` can actually
represent (E10 and E5a both spend it, and E5a's spend does not fit), and whether
the frame-budget rule is aimed at a real frame. Item 4 is what answers both.

> **The first of those closed on 2026-08-13**, ahead of the rest of the sitting,
> because E10 could not start until it did. **The answer was not on the list of
> candidate answers, and the reason is worth carrying: the entry was a
> well-argued question about how to divide one byte, and the byte was never the
> constraint.** `Element` has three unused bytes in the alignment hole between
> `type` and `color` — it always has — and `element.h`'s claim that E3 spent
> "the last free byte" was counting the *tail* hole and treating the front one
> as arithmetic rather than as space. Two items had been sequenced around a
> scarcity that was not there. **The generalisable form: when a plan is arguing
> about how to divide a scarce resource, measure the resource before dividing
> it.** `velocity_probe` prints `sizeof` and `offsetof`; the whole question took
> one run to answer, and this struct's size had already been got wrong once by
> counting fields (`ENGINEERING_NOTES.md` records that).

**Seven items were added on 2026-08-11 and none of them is in this block,
deliberately.** `V12`–`V16`, `E12` and `S1` are the plan for a stated visual
goal — mixed pixel resolutions and sprite sizes, procedural animation, animated
backgrounds, and enemy bodies that granulate where they are hit. **They change
nothing about what to do next.** The block above stands; the new work sits where
the V track already sat, `E12` after `E10`, and `S1` behind the combat decision.
See [Running order](#-running-order).

> **Correction, same day.** The sentence above was written twice in this file as
> "none of them moved anything", and that is not true — **`V16` was inserted
> ahead of `V9`, which puts `E7`, `E11` and `V9` all later than they were.** The
> accurate claim, which is still the strong one, is that **nothing in the Next
> up block moved**; the V track's internal order and the position of the late E
> items did. This is corrected rather than quietly reworded because this file's
> usefulness rests on it being exactly right about its own history, and a reader
> checking the previous order against this one would have found the discrepancy
> and trusted the rest of the paragraph less for it.

**Session 5 ran on 2026-08-10 and is what reordered this block.** It closed wave
3, answered E4 "no", and returned six defects and five observations —
[results](PLAYTEST_LOG.md#session-5-results--wave-3-closes-and-the-water-underneath-it-does-not).
Both of the items it retired were items 1 and 2 here.

**E10 is now 7 — it was 4, then 5 on session 5, and the 2026-08-11 review put
`F5`, the instrumentation sitting and `T1` in front of it. It also lost its
playtest evidence, which matters more than the position.** The session's sand
note read "could have more movement", which looked like a case for E10 until it
was disambiguated: it means *falling sand looks stepped and jerky*, which is the
**second report of A7c** and which [wave
1](ROADMAP.md#wave-1--the-rendering-brush-and-powder-defects) already
established is a property of drawing whole 4x4 cells and is **not reachable from
`step_powder` at all**. E10 does the opposite — it makes sand come to *rest*.
**The item stands on its own merits and none of them are this note**: piles that
hold a slope, tunnels that partly cave, and gravel/sand/snow/ash as four table
rows instead of four code paths. Filing D8 against it would put a rendering
complaint on a simulation item and buy a third round of the A7/A7b/A7c rule
fight.

**Three things the session found are deliberately *not* in this block.** D4
(fluids do not flow) is the loudest finding on the record and its real fix is
plausibly E5b, which is *large* and stays after the slice — item 3 is a bounded
spike that prices it, not a licence to start it. D5 is here because it has now
been asked for three times in four sessions in near-identical words, which is
the strongest signal this log produces. D9 (toppling, the second report of B4)
does **not** move E8: a second report is an argument about priority and E8's
deferral is about price, and its price has not changed.

**P2 shipped 2026-08-10** and is out of this block. Every measurement from here
on is quoted against 1920x1080, the size the game actually runs — the table is
in [PERFORMANCE.md](PERFORMANCE.md). The result worth carrying forward into the
items below: **the engine pays for awake cells, not for cells.** `sparse`, which
stands in for an ordinary gameplay frame, costs the same at 1920x1080 as at
960x540 — 1.00x for four times the cells — so an item that adds per-cell work to
*awake* cells is the kind to price carefully, and one that adds world size is
not. It also left one decision open, in the table below.

**Why S0 is near the front and this is the biggest change to the plan** *(it was
fourth; it is fifth since `F5` went in front of it, because S0 reads a velocity
that is currently a float)*. Everything shipped so far is engine or visual
foundation, and the whole of the game sits behind the whole of the engine track.
`VISION.md` already names that risk in writing — "under-building looks exactly
like discipline right up until the playtest gate" — and then the running order
used to do it anyway. S0 is the thin version of two Medium Term items pulled
forward so the game becomes **losable** before another two months of engine
work. It is also the only thing in the plan that can answer the two open
[Decisions](#-decisions-owed) below by playing rather than by argument.

---

### 🧭 Running order

**~~Wave 4~~ → ~~E9-steam~~ → ~~F5~~ → ~~instrumentation sitting~~ → ~~S0~~ →
~~T1~~ → E10 → E12 → E5a → P1 → E6 → V17 → (V11 + V12 + V13) → V16 → (E7 + E11 +
V9) → V14 → rest of the slice → gate prerequisites → playtest gate → V15 + S1
(if combat) → E5b, P3, E8.** P2 shipped 2026-08-10; the session 5 playtest ran
the same day and E4 closed with it, answered "no".

**Two blocks sit in front of that line and neither is in it, which is why it has
not been rewritten.** The V-track block (item 8) preempted `E10`, and the
W-track block (item 9) is **the immediate next work as of 2026-08-17**, running
ahead of the V block's last step because that step is gated on a human. Both are
sub-plans with their own internal order, both are in [Next up](#️-next-up), and
**`E10` still keeps the head of this queue when they close** — it was
out-prioritised, not changed.

The two structural changes from the order before 2026-08-11 (`E4 → E5 → E6 → E7
→ E8`, then V, then P, then the whole game):

- **A gameplay spike (S0) moves ahead of most of the engine track.** Argued
  above.
- **E5 splits in two.** E5a is per-cell velocity — the enabler that E6, E4's
  shove and V9's debris all actually need. E5b is the air/pressure field, which
  is the larger and more speculative half, and it goes after the slice. They
  used to be one item, which made the enabler look as expensive as the research
  project attached to it.

**Five changes came out of the 2026-08-11 plan review, and each is a case of
something already written in this file not being acted on.**

- **`T1` — the debug tooling batch — moves in front of `E10`.** This file
  already said the free camera is *"close to load-bearing for E10 and E5a"* and
  that the cell inspector *"grows a job with E10 and E5a"*, and then scheduled
  both behind the slice. E10's verify condition is "a poured pile holds a
  measurable angle" and E5a's is "a cell fired at a wall lands against it" —
  neither is checkable at 60 fps with a camera bolted to the player and no way
  to read a cell. Two days, and it is the same argument that moved F1–F4 out of
  Presentation & Tooling.
- **`P1` moves to directly after `E5a`, which is where the P track always said
  it went.** The master order used to put it after the playtest gate while the P
  section said "sequenced after E5a so the layout is settled against the final
  field set". Those are not the same instruction and the P section is the one
  with the reason attached. ~~It matters more now: E5a may not fit in
  `Element`'s existing 12 bytes at all (see the [decision](#-decisions-owed)),
  and P1 is the item that makes a thirteenth byte a different question.~~ *(That
  second reason is retired — the [decision](#-decisions-owed) closed on
  2026-08-13 and E5a fits in three bytes the struct already had. **The first
  reason is the one that always had the argument attached and it still stands**,
  and it got sharper: E5a's three bytes are hot, so it moves the hot/cold ratio
  P1 is trying to improve rather than leaving it alone.)
- **`V17` — a golden-frame check — goes in front of the render rewrite.** V11,
  V12 and V13 rewrite frame composition, the asset path and every destination
  rectangle, and nothing today tests a composed frame. `preview_light` exists
  precisely because *"existing tests all passed the broken frame because none of
  them ever combined layers"*, and V2 shipped a blank world for a whole commit.
  An afternoon.
- **`V15` and `S1` move behind the gate, together.** Argued at V15 — the two
  admit each other in a circle, and the circle only closes if combat is answered
  yes.
- **The gate acquires named prerequisites.** Argued at the [playtest
  gate](#-medium-term-core-gameplay-loop). It cannot presently be run at all.

**Seven items were added on 2026-08-11 and none of them moved the Next up
block** — the original wording was "none of them moved anything", which was
wrong and is corrected above: `V16` went in ahead of `V9`, so `E7`, `E11` and
`V9` are all later than they were. `V12`–`V16`, `E12` and `S1` are the plan for
a stated visual goal — mixed pixel resolutions and sprite sizes, procedural
animation, animated backgrounds, and enemy bodies that granulate where they are
damaged. The block `V11 + V12 + V13` is taken together because all three edit
the same two destination rectangles in `main.cpp` and V11's runtime scale and
V13's per-asset scale are the two factors of one multiplication. `E12` sits
after `E10` because a crumbling material that cannot hold a slope reads as a
liquid. `S1` is in the slice section and **blocked on the combat decision**,
which it does not get to pre-empt. Full arguments in
[ROADMAP.md](ROADMAP.md#the-visual-system-this-track-is-now-building-toward).

**Item IDs are stable and are not renumbered when the order changes** — four
other documents cite them by name. E10, E11, E5a/E5b, S0, P3 and V11 are new
here, as are E12, S1 and V12–V16; **F5, T1, P4 and V17 are new on 2026-08-11
from the plan review**. Nothing existing was renumbered. `T` is a new track
letter and it holds exactly one item on purpose: the debug tooling that is a
prerequisite for other work, as distinct from the tooling in Presentation &
Tooling that is a convenience.

---

### ❓ Decisions owed

*Open loops that are decisions rather than work. A decision with no deadline
never closes, so each has one. Each may be answered with "no" — that is a closed
loop, not a failure.*

**A distinction this table was not making, added 2026-08-11.** Three of the rows
carried a recommended answer, a due date, and nobody who could overrule the
recommendation, which is a **decided thing wearing a decision's clothes** and it
costs a re-read every cycle. Those are now written as **decided, with a named
reopen trigger** — which is strictly more useful, because "no, unless X" tells a
reader what to watch for and "recommendation: no" only tells them what someone
thought. The rows that remain genuinely open are the ones where the answer is
not known and the evidence that would settle it does not exist yet.

| Decision | Due | How it gets settled |
|---|---|---|
| ~~**Does the player displace material?** (E4)~~ | ~~after session 5~~ | ✅ **Closed 2026-08-10: no.** Session 5's Phase B returned nothing "obviously better", and **two of its four rows turned out not to be about displacement at all** — E-1 is a defect in the unstuck search (D2) and E-2 is almost certainly `MAX_STEP_HEIGHT` (D7). That leaves E-4, water, as the only surviving argument, which is the *reverse* of the split the checklist guessed at. Written into [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md); re-ask at E5a, and note that D2 makes the unstuck search permanent load-bearing machinery rather than a stopgap. |
| ~~**Is there combat in v0.1?**~~ | ~~now due — S0 built 2026-08-14~~ | ✅ **Closed 2026-08-16: yes.** Answered by playtest, which is what this row spent months insisting on: *"needs an enemy but i will do that later"* ([PLAYTEST_LOG.md session 6](PLAYTEST_LOG.md#session-6--2026-08-16-v19-4bs-ground-plane-first-human-eyes)). **The deferral is part of the answer and must not be read as the decision still being open** — the entry below already wrote what a "yes" means in work, and it means the ugly enemy (sprite, hitbox, contact damage, dies and despawns, about two days), not `S1`. So the loop this row was guarding is shut: combat is in, the first step is known and small, and *when* it gets built is now an ordering question for the running order rather than a decision anybody is waiting on. **What did not get answered, and is worth naming rather than assuming:** the tester said the run needs an enemy, not that the run was uninteresting — nothing here says the objective, the water crossing or the fall damage failed, and the "what is the hook?" row below is untouched by this. *(Original entry, unchanged below, because its reasoning is what makes the answer actionable.)* **The deadline has arrived and the thing it was waiting for exists.** The run is losable: fire burns the body, a bad landing costs health, an objective east of the water channel ends the run when reached. **What is owed is somebody playing it** — checklist step 10 — because this decision was always "whether it needs an enemy to be interesting is a thing you can *feel*", and nothing about having built S0 makes it feelable from the desk. **Do not let this close by default.** It has been open for months precisely because it had a gate instead of a deadline, and "S0 shipped, so we will decide after the next thing" is how it gets one again. *(Original entry, unchanged below, because its reasoning is what makes the answer actionable.)* S0 makes the run losable; whether it needs an enemy to be interesting is then a thing you can feel. Currently neither in nor out, which is the worst of the three states. **The "yes" branch now has a first step attached, because it did not have one and that made the decision unactionable for months.** As written, the first enemy was `S1`, which depends on E12, V12, V14, V15 and E5a — very nearly the whole remaining plan. So "yes, combat" was a thing decided in weeks and untestable for half a year, which is not a decision, it is a deferral with a date on it. **If the answer is yes, the immediate next step is a deliberately ugly enemy — a sprite, a hitbox, contact damage, dies and despawns, about two days — and `S1` stays where it is.** S1's granulating body is the *payoff*, not the mechanic, and the mechanic is the half that has to be felt before the payoff is worth weeks. |
| ~~**Does the player leave screen centre?**~~ *(V22, 2026-08-17)* | ~~before V22 authors anything~~ | ✅ **Closed 2026-08-17, same day, and the recommendation in this row was not taken.** *"match the reference. and the digging will move the camera along the angle to the same pov as CnC_fishing, CnC_underwater_1, and CnC_underwater_2."* Answer: **(3), the full move — and the second half of the sentence is what made (3) affordable.** The row framed this as a trade between two framings and went looking for the least-bad fixed point between them, recommending 20–30 cells. **That was the wrong shape of question**, and it is left here rather than deleted because the error is reusable: a trade between two states is only a trade if you have to pick one. The reference’s own three frames put their subject at 0.60, 0.36 and 0.27 down the frame while holding the near volume at the lower 55–65% in all three ([entry 10](notes/reference_observations.txt)) — so the framing is a **function of what the player is doing**, not a constant to compromise on. Built the same day as **V23**: `SURFACE_ANCHOR` 0.80, `DIG_ANCHOR` 0.30, easing between them. **What this row was right about is the cost** — ~55 cells of world below the player at the surface framing — and that is precisely what the dig framing pays back. **Closed again, by withdrawal, 2026-08-17.** Session 8 called the delivered framing "upside down" (V23a found the clamp behind it) and session 9 asked for plain centring, so the mechanism was deleted — **the answer to this row is now (1), no move at all**, and it was reached by playing rather than by the argument above. The cost this row was right about is back: ~50% is all the plane can have below a centred player.|
| ~~**Does the spawn serve the fixture, or the plane?**~~ *(V22, 2026-08-16)* | ~~before V22 builds a scene~~ | ✅ **Closed 2026-08-16 by dissolving it rather than picking a side.** *"what if the plan is to eventually only have the final product scene be the test scene."* **Both — and the question was malformed because it assumed the fixture is a *place*.** It is a set of exercises, and an exercise can be dressed. So the plan is: **there is one scene, it ships, and every system that needs exercising earns a feature in it that a player would plausibly encounter.** The stairs become terrain, the pit with pillars becomes a cave mouth, the water channel becomes a river. **This kills option (3) permanently** — the tested thing and the looked-at thing never diverge — and it kills option (2) unless a replacement carries every exercise forward. What is left is option (1) upgraded from *move it out of shot* to *dress it and spread it out*, which is a stronger constraint and a better one: **a test feature that cannot be dressed into something a player would meet is a feature that exists only to be tested, and this plan makes that visible instead of letting it hide at the spawn.** The full principle, its one real cost, and the mechanism that pays for it are in [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md). *(Original entry below, unchanged, because the collision it describes is real and is what the plan resolves.)* **Open, and it is a collision between two things that are each correct.** [PLAYTEST_LOG.md session 7b](PLAYTEST_LOG.md#session-7b--2026-08-16-the-plane-the-player-is-not-touching): *"the player is not touching the plane, they are only touching the original test scene ground... this may require a completely new scene from the ground up."* **The report is right about the build.** `CnC_lighthouse.jpg`, measured at [reference entry 9](notes/reference_observations.txt), puts the figure two thirds of the way down the plane with a third of it in front, and **nothing between the figure and the horizon.** F4.4 deliberately made the first scene *a test fixture wearing art*: uneven stairs, fence posts, a pit with pillars, a water channel, jump ledges — every one a vertical feature on a jagged surface, standing between the player and the horizon, at the spawn. **Neither purpose is wrong and they cannot both own the same location.** Three answers: **(1) move the fixture** — keep every region and every system it exercises, put an open, flat run at the spawn and the fixture further along the world where the camera does not open on it; the world is 1920 cells and the fixture is 1600, so this needs the fixture shrunk or the world widened, and that is the only real work in it. **(2) Replace it** — a new generated scene authored for the composition, the fixture deleted; cheapest to author, and it silently retires five named regression exercises, which is the kind of loss that is invisible until something regresses. **(3) Two scenes** — the fixture stays as a test-only scene the suites and probes load, and the game ships a different one; honest, and it is the option that permanently splits "what the game looks like" from "what the tests measure", which is a thing this project has so far refused to do on purpose. **Recommendation: (1).** It is the only one that changes no test's meaning, and the fixture's value is its regions, not its address. **What every option costs, and it is owed to the tester rather than to a build:** changing `assets/test_material.bmp` invalidates both recorded sessions — `src/game/input_log.h` names it as the first invalidator, and `tests/test_scene.cpp` pins `FIXTURE_SCENE_CELLS = 334901` so it fails in `ctest` and not in a benchmark nobody runs. **P4's replayed row goes dark until somebody plays and re-records, and only the tester can do that.** `bench_grid` (two call sites) and `rim_probe` load the fixture too. The scene is generated by `generate_test_scene.py`, so the authoring itself is an afternoon. Asked of the tester in `MANUAL_TESTING.md`. |
| ~~**Is the receding plane land, or water the player is on?**~~ *(V22, 2026-08-16)* | ~~before V19 4c authors its three bands~~ | ✅ **Closed 2026-08-16, same day, and the answer is a fourth option none of the three offered.** *"you are correct when i said i want the first scene to be land not water, but i still want the land layer to act like the water layer with the player sitting on/in the plane."* **The material stays land and the *relationship* becomes the water layer's.** That splits a question this row had welded together: "what is the plane made of" and "where is the player relative to it" are independent, and every one of the three options answered both at once. Land keeps the scene, the world and the fluid sim untouched — option 3's whole cost was in that half. **What transfers is the half that was actually doing the work in the reference:** the boat is *in* the receding plane, not in front of it, which is why the plane recedes around it; ours is a band the player stands before, and a band you stand before cannot recede around you, at any value. **Reflections are dropped and that is not a loss** — the replacement was recorded at entry 7 before the fact, and it is a mechanism rather than an appearance: a horizontal surface faces the sky where a vertical silhouette does not, so the plane brightens toward the viewer for a different reason than the lake does. **The refusal against a near foreground silhouette is untouched and this must not be read as bending it.** That refusal is about paint *in front of* the world occluding the one verb the game has, and nothing here changes a draw order: the plane stays behind the world and stops where it always stopped. **The player is put "in" it by value, not by geometry** — the world's surface has to read as the plane's near end, which is a continuity across the one junction in the frame nobody has ever tuned. **So this closure promotes the deliberately-open TUNING row from optional polish to the mechanism itself**: "does the world row take a grade below the plane's?" was filed as a look-and-see, and it is now the question V22 is made of. It still gets its own row and its own playtest — see the note at 4b — but it is no longer optional and no longer postponable. **4c is unblocked by the same answer:** there is no shore, so its third band is a treeline standing on land. *(Original entry below, unchanged, because the reading that produced the answer is in it.)* **Open, and it reopens a decision made deliberately the other way.** V19 is titled "the seven-band scene, with land where the reference has water"; [PLAYTEST_LOG.md session 7](PLAYTEST_LOG.md#session-7--2026-08-16-v20s-raised-palette-and-what-the-plane-is-made-of) answered "does the plane recede" with **no**, and supplied the reading that explains three failed attempts to fix it by shading: in `CnC_parallax_1..3.jpg` **the plane is a lake and the boat floats on it.** Its depth comes from *reflecting* the bands above it — a mechanism we never built — and from the player *occupying* it rather than standing in front of it. **Neither is reachable by a retune, which is why this is a decision and not a defect.** Three answers, and the cheapest is respectable: **(1) leave it** — a digging game on solid ground, the plane stays a backdrop band; **(2) reflections only** — mirror the mountains into the existing plane, buying the missing cue without touching the world or the scene, and reachable from the current draw path; **(3) the water plane** — a new starting scene with the player on or beside water, reaching `src/scene/`, the world and possibly the fluid sim. **Due before 4c because 4c's third band is the *shore* treeline**, which is a different band depending on whether there is a shore. **This is the same lesson as the deleted mid-ground band — ask what in the reference is doing the work** — and the refusal recorded against a near foreground silhouette applies to option 3 with full force: a painted plane in front of the world occludes the one verb the game has. Asked of the tester in `MANUAL_TESTING.md`. |
| **What is the hook?** | end of E6 | Deliberately unnamed so the design isn't locked to whichever comparison got written down first. S0 and E6 between them make it answerable by playing. |
| **Does the brush paint while paused?** *(the third state only — the first two are settled)* | **held open 2026-08-14**, at T1 | Surfaced by F2.4 and never answered. Costs a sentence. **S0 answered the adjacent case without being asked and the precedent is worth naming:** a finished run freezes by not accumulating time, exactly as the settings menu does, so the brush does *not* paint — because no step runs at all rather than because painting is refused. If the pause hotkey uses the same mechanism the question answers itself; if it wants a pause that steps the brush but not the physics, that is a third state and needs the sentence. **T1 shipped on 2026-08-14 and the precedent held, so two thirds of this is now settled by construction rather than by a sentence:** no step runs while paused, so the brush does not paint - not because painting is refused but because painting is *part of a step*, and pressing `.` paints exactly one stamp. **What is still genuinely open is only the third state** - a pause that steps the brush and not the physics - and it is now a feature request with a known cost rather than a policy gap, because the two mechanisms it would have to separate are currently one call. Left open by the user's call rather than closed by the build. |
| **Does `R` stay the restart key, and does anything else share it?** *(new — S0)* | **held open 2026-08-14**; T1 built one answer without closing the question | S0 binds `R` to "start a new run" and makes it **inert while the run is playing**, on the same argument that moved quitting off `ESC`: a key that throws a session away is a bad one to mis-hit. T1 adds a world-reset hotkey, which is the unconditional version of the same action, and the two will want either one key with two meanings or two keys — **whichever it is, T1 decides it rather than discovering it**, because a debug reset that also works mid-run is exactly the mis-hit S0 refused. **T1 shipped 2026-08-14 and took the third option, which this entry did not list: one key, two meanings, separated by a modifier.** `R` alone keeps S0's meaning and stays inert mid-run; `Ctrl`+`R` is the unconditional debug reset and works while playing. That is not a discovery - it is chosen for the reason the entry gives, that a modifier is not a thing you mis-hit while reaching for sand - but it is **not the merge-or-split answer the question asked for**, so the question stands: whether the two should eventually be one key, and what a second consumer of "throw this run away" does to that. Held open by the user's call rather than closed on the build. **Reopen trigger, if it is ever closed as "leave it": a third thing wanting the same verb** - save-and-load and the level editor are both plausible, and three meanings on one key with two modifiers is where this stops being cheap. |
| ~~**Does the world's cell size change?**~~ *(V13)* | ~~before V13 ships~~ | ✅ **Decided 2026-08-11: no.** "Higher-resolution pixel art" has two readings and only one is expensive. Halving `Camera::SCALE` to 2 puts **four times the cells in the viewport**, re-authors every asset, and retunes every physics constant — all of them are stated in cells against a scale of 4 (`Player::WIDTH`/`HEIGHT` at 8x20, `DigTool::RANGE`, `LightField`'s reach). V13 buys denser *art* per asset with none of that. **Reopen trigger: V13 ships, denser art is judged worth having, and the thing still reading as too coarse is the *terrain*** — which is the one surface V13 provably cannot reach, since its resolution is the simulation's. Nothing else reopens it. Write this sentence into V13. |
| ~~**Does the renderer stay `SDL_Renderer`?**~~ *(V7 required this be decided deliberately if ever)* | ~~before V15 is scoped~~ | ✅ **Decided 2026-08-11: yes, it stays.** V7's entry says the shader path is *"a genuinely large decision about what this project's renderer is"* and must not be discovered halfway through a CPU implementation. **Every item now scheduled has a route without one**, and the routes are named so this can be checked rather than believed: `SDL_ComposeCustomBlendMode` for V11's multiply term, `SDL_RenderGeometry` for textured triangles, `SDL_TEXTUREACCESS_TARGET` for S1's masked body — all present in the pinned SDL 2.30.0, none used anywhere today. **Reopen trigger: the first scheduled item with no such route**, and the one candidate on the horizon is sub-cell terrain detail, because the terrain's resolution *is* the simulation's and no asset work reaches it. Whoever hits that writes the fork here rather than starting it. |
| **Does a body displace material — re-asked, for a body that sheds matter?** *(new — S1)* | at S1 scoping | E4 closed **"no"** on 2026-08-10, on evidence from a session where **nothing in the game depended on the answer**. `S1` is a body that spawns `Grit` out of itself, and [notes/granulating_enemies.md](notes/granulating_enemies.md) predicted the failure that "no" now permits: the grit falls through the enemy producing it, so the effect fails exactly at the moment it exists to be looked at. `ENGINEERING_NOTES.md` already says re-ask at E5a; this is a second and stronger reason, with a named consumer rather than a schedule position. **A narrow yes — bodies that shed matter displace it, the player still does not — is a legitimate answer** and is the one to consider first, since it is the only case with evidence behind it. |
| ~~**What does the frame-budget rule trigger on, now that the budget is measured at the played size?**~~ *(P2 — reframed 2026-08-11)* | ~~at the instrumentation sitting (item 4)~~ | ✅ **The rule is decided; the `churning` question bundled with it is not — see (3).** Closed 2026-08-13 on the first recorded session, and the answer is not the one this entry argued for. The session: 24,437 steps, 407 s of play, **mean 0.1212 ms/step (0.7% of a frame), p99 1.4745 ms (8.8%), worst step 4.8193 ms (28.9%), 0 of 24,437 steps over budget** — and that is a whole `Run::step`, so it already includes the player, dig tool and brush the synthetic rows leave out. **(1) The rule triggers on p99 and on steps-over-budget, never on the mean**, which this entry got right in advance. **(2) But "decide it against that row, not against a taxonomy of the synthetic ones" is half backwards, and the number is what showed it.** A merge test of "under 10% on the replayed row" is 10% of 0.12 ms — twelve microseconds, well under this benchmark's noise — on a row with three times the budget in headroom. A per-awake-cell cost that only bites under load would pass it while being invisible to it. **So both kinds of row are kept and given different jobs: the replayed row is the authority on whether the budget is broken, the synthetic rows on whether a change costs anything at all.** The restated rule is in the [P track](#p--performance). **(3) Whether `churning` is representative is still open, and the same-day census is why.** The row now reports what a session contained, and the first session contained **no digging, no moving sand and no moving water at all** — peak 16 of 510 chunks awake. It cannot be compared against a liquid scenario because it has no liquid in it. **The half of the question worth keeping is that "representative" needed a completing phrase**: `churning` measures the engine under sustained liquid churn, which is a real thing a player does and not a synthetic one, and the argument was never going to be settled by classifying scenarios. **It is settled by a session that actually pours sand into water, and that session is item 4's remaining work.** Numbers, and the correction in full, in [PERFORMANCE.md](PERFORMANCE.md). |
| *The original entry, kept because its reasoning was right where its conclusion was not:* | ~~at the instrumentation sitting (item 4)~~ | The standing rule is "if one item alone breaks the frame budget, P1 gets pulled forward". P2 found `churning` at 211% and `cascading` at 241% at 1920x1080 with nothing having got slower, so the rule as written does not fire. **The previous recommendation was to re-aim the rule away from those rows as synthetic, and the review pushed back on half of it, correctly.** `cascading` is genuinely synthetic — a row is scraped off the floor and poured back at the ceiling every step, which nothing produces. **`churning` is sand sinking through water, and that is the most ordinary thing a player does in a falling-sand game** — F4.4 put a water channel in the fixture scene precisely so it would happen. Calling it synthetic to keep the rule quiet is the failure `PERFORMANCE.md` is otherwise entirely about. **This is not settled by arguing about which rows are realistic, because there is an instrument that ends the argument: `P4` replays a recorded session and produces a row that *is* a played frame, by construction.** Decide the rule against that row, not against a taxonomy of the synthetic ones. **`P4` shipped 2026-08-13 and this decision now has exactly one thing standing in front of it: a session someone has actually played** (`F9` writes it; [README](PERFORMANCE.md#the-replayed-row-and-recording-one-p4)). The row reports mean, p99, worst step and steps-over-budget, so the rule can be written against a statistic rather than against an average — **and the average is the one to avoid**, since a session that stutters for 5% of its steps has an excellent one. |
| ~~**What can `Element::ticks` actually represent, and does E5a fit in it?**~~ *(E10 / E5a)* | ~~at the instrumentation sitting (item 4)~~ | ✅ **Closed 2026-08-13: it cannot represent a velocity, and E5a does not go in it. The answer is none of the three candidates — the bytes were already there.** Measured with `velocity_probe` (`tests/velocity_probe.cpp`), built for this and kept. **(1) The packed 4+4 integer cannot carry gravity, confirmed rather than argued.** One step of `Player::GRAVITY` is 5/36 of a cell per step; added to a whole-cell integer it truncates to zero every step forever, so the probe's thrown grain **never comes back down in 600 steps** — a raycast, exactly as the entry predicted. Its slowest non-zero speed is 1 cell/step = 60 cells/s, 15% of the player's own terminal velocity, so nothing can be nudged, only launched: E4's shove is 0 cells/step under this representation. **(2) `Element` has three free bytes and always did**, at offsets 1–3, in the alignment hole between `type` and `color` — measured with `sizeof`/`offsetof`, not counted. `element.h`'s "that was the last free byte" counted the *tail* hole only, and the correction is recorded beside it there. **A signed 4.4 velocity byte per axis plus a nibble per axis of sub-cell remainder is exactly three bytes, and the struct stays at 12.** `grid_bench` with those three fields present is inside the noise band on every row (PERFORMANCE.md). **(3) The sub-cell remainder is the quantity this entry never named**, and it is half the requirement: a cell's position is a cell index, so a velocity with a fraction needs somewhere to spend the fraction, exactly as `Player` carries `rem_x` alongside `vel_x`. Any answer that budgeted for velocity alone was budgeting for half the problem. **(4) Gravity is applied by differencing a running total off the global step counter** — Bresenham on an acceleration — because 5/36 of a cell is 2.222 sixteenths and a truncated 2 makes gravity 10% light on every thrown thing forever. Flown at all nine phases of the pattern: range 224–228 cells against the `fx` 16.16 reference's 228, worst deviation **2 cells**, versus 25 for the truncated version and a 156%-of-the-mean spread for the stochastic one. **`P1` does not move** — no byte is spent, so the trigger written into its entry never fires. **E10 is unblocked and its "costs no memory" claim survives**, but for a different reason than it gave: it does not need the byte either, since "is this cell moving" is `vel != 0`. |
| *The original entry, kept because its two problems were both real and one of them was found by taking it seriously:* | at the instrumentation sitting (item 4), and before E10 writes to the byte | E10 claims the byte and E5a redefines it as 4 bits of signed dx plus 4 of signed dy. **Two problems, and the second is the one that costs weeks if it is found during E5a.** *(a)* Integer cells-per-tick has no sub-cell fraction, so the only representable speeds are 0 through 7 and **gravity cannot be integrated onto a moving cell at all** — a thrown grain travels a straight line at constant speed and stops. That is a raycast, not the throw/splash/spray E5a exists to buy, and it is the one thing every engine this is measured against uses a fraction for. *(b)* [element.h](src/physics/element.h) already gives the byte two mutually exclusive roles guarded by a `static_assert`, and E5a's is a third — so **structural and Fire cells are excluded, which means E6 cannot throw wall debris or fire, and that is written nowhere.** Settled by prototyping the representation against `cascading` at the sitting, not from the desk. **The candidate answers are: accept the coarse integer and write the restriction down; spend a second byte and pull `P1` in front of `E5a` rather than after it; or split speed across `ticks` plus reclaimed bits elsewhere.** Whichever wins, E10 must not write to the byte before it is chosen, because E10's whole claim is that it is not a rewrite of E5a's representation. **What changed on 2026-08-12, and it moves the ground without answering the question.** E9's steam half made `Steam` the second material to spend the byte as a *lifetime*, so problem *(b)* above is now measurably worse and measurably better at the same time. Worse: **two Gases are excluded from carrying a velocity, not one**, so E6 cannot hand an impulse to steam either — though that widens an exclusion E6's entry already states rather than creating a new one. Better: the roles are no longer a belief. [element.h](src/physics/element.h) has `tick_role()` and an assertion over every row that nothing carrying a lifetime is structural — **the `constexpr` role lookup E10's entry asks for, built early and by a different item** — so whatever representation wins, adding it is an edit to one function that fails the build if the roles collide, instead of a byte three systems each believe they own. **Problem *(a)*, the sub-cell fraction, is untouched and is still the one that costs weeks if it is found during E5a.** |

---

### 🔧 Prerequisites — things other items are already standing on

*New section, 2026-08-11. Three entries, all found by the plan review, all with
the same shape: **something already scheduled depends on this, and the
dependency was written down in this file and then not acted on.** That is a
different admission test from either of the two sections below, and it is the
same one F1–F4 answered — "does a scheduled item need this, and does it get more
expensive the longer it waits". `P4` and `V17` pass the same test and are filed
in their own tracks because they belong to those tracks' running orders.*

- **~~F5 — Fixed-point player kinematics.~~** ✅ **Shipped 2026-08-12.** `rem_x`,
  `rem_y`, `vel_x` and `vel_y` are `fx` signed 16.16
  ([src/physics/fixed.h](src/physics/fixed.h)), the constants are exact
  rationals, and `Player::update()` takes no `dt`. Three things the entry below
  did not predict:
    - **The verify condition "traces identical to the float version to the cell"
      was not achievable in principle**, and finding out why is the useful part:
      **1/60 is not representable in binary**, so a step's worth of gravity is
      8.333328 cells/s in fixed point and 8.333334 in float and neither is
      right. Measured: **7 of 1381 recorded steps differ, each by one cell, each
      re-converging immediately**; walk and jump traces are byte-identical, and
      every landing, rest position and peak matches. The condition should have
      been "no persistent divergence", which is what was actually wanted.
    - **The `dt` parameter was worth deleting for a second reason.** Every
      caller already passed the same compile-time constant, so a parameter that
      looked like it varied did not — and the first caller to pass a real frame
      time would have re-created the defect F1 spent an item closing.
      `Run::FIXED_DT` is now *derived* from `fx::STEPS_PER_SECOND` rather than
      written beside it.
    - **It found the remaining float**, `DigTool::march` — F5 removed the larger
      half of the exposure and did not remove all of it. *(Closed 2026-08-13 as
      `F6`; see the note under [Next up](#️-next-up).)*
- **~~F6 — `DigTool::march` is integer-only.~~** ✅ **Shipped 2026-08-13**, an
  afternoon, unscheduled when it started. Squared range test, `isqrt` for the
  truncated step count, `div_round` for the two `lround`s. **The lesson is about
  the replacement, not the removal:** the obvious integer form of `lround`, `(a
  + b/2) / b`, rounds the wrong way for negatives because C++ truncates toward
  zero, so digs would have landed one cell off in two quadrants of four — a
  mirror-symmetry test is what stands between that and shipping. It also turned
  up a 32-bit overflow in the old expression that had removed the range limit at
  a distant aim.
- *The original entry, kept for the cost curve it argues:*
  [player.h](src/physics/player.h) held `rem_x`, `rem_y`, `vel_x` and `vel_y` as
  `float`, integrated against a `float dt` with float constants. **The
  determinism guarantee is therefore machine-local, and three things in this
  plan spend it as portable.**
    - **What is actually true today:** [test_run.cpp](tests/test_run.cpp) proves
      a recorded input sequence replays byte-identically — for **one binary on
      one machine**. Float results are not reproducible across compilers,
      optimisation levels, x87 versus SSE, or FMA contraction. Nothing has been
      wrong yet because nothing has run anywhere else.
    - **What spends it anyway:** *crash diagnosis*, whose whole idea is that "a
      crash report could be a file that reproduces the crash" — a repro that
      does not reproduce on the developer's machine is worse than none, because
      it costs a session to find out; *save and persistence*, which stores a
      run; and *build on macOS and Linux at least once*, which is the item that
      would discover this and has no reason to be looking for it.
    - **The fix, and it is small because the hard half is already done:**
      `pos_x` is already an integer and the fraction is already separate — the
      design [player.h](src/physics/player.h) chose specifically to avoid "the
      class of float-edge bugs where a box is 0.0001 into a wall". Make
      `rem_x`/`rem_y` `int32_t` with 16 fractional bits, make the constants
      integers, and **drop `dt` from `Player::update` entirely**: the timestep
      is fixed, so it is a compile-time rational rather than a parameter.
    - **Why now rather than later, stated as a cost curve.** Every step from
      here widens the float surface: **S0 reads `vel_y` for fall damage**, V14
      and V15 add solvers, and V15's entry already warns that "a spring's
      stiffness quietly means something different at each frame rate".
      Converting four fields is days; converting four fields plus a damage model
      plus a rig is not.
    - **After Wave 4, not before** — Wave 4's D2, D6 and D7 all edit the same
      file, and doing this first means resolving the same lines twice.
    - *Verify:* the existing replay test still passes; the walk, jump and fall
      traces are identical to the float version to the cell; and the constants
      in [TUNING.md](TUNING.md) still mean what they say.

- **T1 — The debug tooling batch.** *(2 days — new, and it is item 6)* Four
  items lifted out of Presentation & Tooling: **world reset hotkey, pause and
  single-step, a free camera, and the cell inspector.** They are moved rather
  than promoted on enthusiasm, and the argument is that this file already made
  it twice and then filed them behind the slice anyway: the free camera is
  *"close to load-bearing for E10 and E5a"* and the cell inspector *"grows a job
  with E10 and E5a — speed is per-cell state with no other way to see it"*.
    - **The concrete problem:** E10's verify condition is "a poured pile holds a
      measurable angle instead of flattening" and E5a's is "a cell fired at a
      wall lands *against* it". **Neither is checkable at 60 frames a second
      through a camera bolted to the player, with no way to pause and no way to
      read a cell's state.** Building both against eyeballs is how you get a
      third round of the A7/A7b/A7c rule fight — that entire episode is three
      attempts to fix something nobody could look at closely.
    - **It also closes an open decision for free:** pause and single-step
      carries the brush-while-paused question, which has been open since F2.4
      and costs a sentence.
    - **F2.4 already checked the price** and concluded these are now genuinely
      small, which is why this is two days rather than a week.
    - **This is the third time the argument has been written down, and
      [ROADMAP.md](ROADMAP.md) makes it hardest of all.** Its V-track note
      records that **V2 could not be verified in the running window at all** —
      the startup camera sits below the F4 scene, synthetic input never reached
      the SDL window, and the palette was signed off on a swatch sheet instead —
      and then says **"every remaining V item has the same problem"**, that the
      fix "is a prerequisite for verifying the V track rather than a nicety",
      and that it "should be pulled forward the first time a V item cannot be
      checked". **That has already happened, at V2, and V2 shipped a blank world
      for a commit.** Three separate entries reached this conclusion
      independently and none of them moved the item. That is the pattern this
      batch exists to break, and it is worth noting the failure mode: an
      argument that keeps getting *made* but never gets *filed against a
      position in the order* is indistinguishable from one nobody made.
    - **What stays in Presentation & Tooling:** continuous brush strokes and the
      brush outline preview. Both are conveniences, neither is load-bearing for
      a scheduled item, and moving them too would make this a section rather
      than a batch.

- **The instrumentation sitting.** *(~2 days — item 4, and it is a batch rather
  than an item)* Four open questions in this file are all "build an instrument,
  then decide", and three of them were already scheduled separately. Taken
  together because they share the instruments and because their answers
  interact.
    1. ~~**The fluid spike**~~ ✅ **Closed 2026-08-13, and the price kills the
       performance argument for E5b.** The three rules E5b retires together —
       `vent_fluid`, `find_lower_surface` and `make_room_above` — are switchable
       at runtime, and `grid_bench` ablates them one at a time in one process.
       **All three removed is worth 8.3% of the played mean and 4.6% of played
       p99**, on a row already at 0 of 20,415 steps over budget. **8% of 1.2% of
       a frame is not a reason to build a second grid over the world.** E5b
       stands on the six capabilities its own entry lists, and on D3/D4, which
       are quality defects no timing can close. *(The original item text, kept:
       D4 is the loudest finding on the record and its plausible fix is E5b,
       which is large; **the output is a price, not a fix** — which is exactly
       what it produced, and the price came back small.)*
        - **`churning` and the played session disagree about which rule is
          expensive, and that is the transferable finding.** On `churning` at
          1920x1080, venting is **47.1%** of the row and `seek_level` is
          **0.3%**; in play, venting is **0.1%** and `seek_level` is **7.3%**.
          Both make sense — `churning` is powder sinking into fluid everywhere,
          which is all venting and leaves no settled surface to seek from, while
          a played world has large quiet pools with long surfaces. **A synthetic
          worst case is not merely unrepresentative in degree; here it points at
          a different rule.** Session 2 taught the degree half of this lesson;
          this is the kind half.
        - **The table has a null control built into it and it worked.**
          `make_room_above` cannot fire on `churning`, which never paints, so
          that row prices the instrument rather than the rule: it reads
          **+0.2%**, which is what makes `seek_level`'s 0.3% readable as "at the
          floor" instead of "small". **A `find_lower_surface` that the E-track
          table describes as "up to 512 cells per awake surface cell per tick"
          costs `churning` nothing measurable.**
        - *(The bullet below was the state of this item before it closed, and it
          called the shape right: the envelope came first, the breakdown second,
          and the breakdown is what turned the envelope into a decision.)*
          **Session 2 does not answer it, but it does supply the budget the
          price gets judged against**, which was missing before. D3 and D4 are
          quality defects and no timing can close them. What was unknown is what
          a fix is *allowed* to cost: a session containing real fluid work now
          reads **worst step 4.83 ms of a 16.67 ms frame**, so there is roughly
          **11.8 ms of headroom at the busiest single moment** of played fluid
          activity. That is the envelope E5b's price gets held against, and it
          is large enough that this item is now a question about correctness and
          looks rather than about affordability. **It is still owed a
          measurement of where fluid time actually goes** — the replayed row is
          a whole `Run::step` and does not break down by subsystem.
    2. ~~**`P4`** — the replayed-session benchmark row.~~ ✅ **Closed
       2026-08-13.** Instrument, census, budget rule and **both sessions** done.
       Session 2 — 20,415 steps with 479 dig steps, sand into the water channel,
       fire under a ceiling and steam — read **0 of 20,415 steps over budget,
       p99 7.1%, worst single step 29.0%**, and replayed byte-exact. *(The line
       below is kept as written: it was the blocker, and what unblocked it is a
       person playing for six minutes, not any code.)* **Still owed: a second
       session.** The census added the same afternoon showed the first one never
       dug, never moved sand or water, and peaked at 16 of 510 chunks awake — so
       it cannot answer the `churning` question, and the two items below cannot
       be judged against it either. **This is the batch's remaining blocker and
       it is two minutes of play**, listing the cases to cover in
       [README](PERFORMANCE.md#the-replayed-row-and-recording-one-p4).
        - **`churning` is settled and the answer is no** — it is not
          representative of played work. Session 2 deliberately drove sand into
          water and its **worst single step of 20,415 was 4.83 ms**, against
          `churning`'s 37.25 ms/step *sustained* at 360 of 510 chunks awake.
          **The load-bearing statistic is `worst`, not the sampled awake peak**,
          because timing is measured every step and the census samples every
          sixtieth — reasoning in [PERFORMANCE.md](PERFORMANCE.md). `churning`
          keeps its row and its job; what it loses is any claim to be a
          realistic frame.
    3. ~~**The `VENT_RADIUS` runtime toggle**~~ ✅ **Closed 2026-08-13.**
       `Grid::set_vent_radius` exists, `grid_bench` sweeps r=0/2/3/4 in one
       process, and the shipped radius still replays both recorded sessions byte
       for byte. **Three answers came out of it and two of them were not the
       question asked.**
        - **The knee is not there.** In one binary `churning` costs **3.46 /
          4.97 / 6.55 / 8.35 ms/step at r=0/2/3/4** — linear in the area of the
          scanned box to within 5%, at both world sizes. The recorded sweep's
          r=3 point sat 9-16% below its own neighbours per scanned cell, and
          **that dip is the whole of the argument for 3**. It does not survive
          one compile. **3 stays**, on quality alone: r=4 buys 50 steps of a
          transient D3 showed clears itself, for 27% more scan. Numbers in
          [PERFORMANCE.md](PERFORMANCE.md).
        - **The toggle costs `churning` 32% and the played session nothing** —
          +31.9% at 960x540 and +31.6% at 1920x1080, with every control row
          inside 2% except `burning`'s 7.7%, against +0.3% mean and +1.2% p99 on
          the replay, which is inside the noise of repeating the identical run.
          **`churning` is the only bench scenario containing water**, so the
          other six rows are controls the change cannot reach, which is what
          makes a cross-build reading admissible here. Shipped on that split,
          with the 32% written down against E10 and E5a, both of which make the
          powder/fluid interface busier.
        - **The two halves of the frame-budget rule disagreed for the first
          time, and the disagreement was the finding.** Synthetic: this costs a
          third. Played: this costs nothing. Both true, and neither alone would
          have said *where* the cost is. That is the clearest vindication the
          two-part rule has had since it was written.
        - **It also corrected a claim from earlier the same day**, which is
          filed beside the original rather than tidied away: `worst` was named
          as the statistic that settled `churning`, and four identical replays
          in one process spread it by **72%** while mean and p99 held to 0.3%
          and 1.2%. The conclusion is unchanged; the statistic should have been
          p99 plus steps-over-budget.
        - *(Kept as written, because it called the shape right and the size
          wrong — the build was an afternoon, and what came out of it was three
          findings rather than the one number it promised.)* **Session 2 makes
          this buildable, and it is now the batch's most tractable remaining
          item.** The blocker was never the toggle itself — it was that a toggle
          needs a *load containing sand sinking into water* to be measured
          against, and until 2026-08-13 the only such load was `churning`, which
          this batch has just established is not representative. Session 2 is:
          479 dig steps, `Sand` peaking 1,827 cells above its start, `Water`
          1,755 above its own. **The shape is: make `VENT_RADIUS` a runtime
          value, then replay `session_2_digging_fluids_steam.rec` twice in one
          process at two radii.** One binary, one sitting, same input stream —
          which is exactly what dodges the E1 trap of the compiler re-laying-out
          the hot loop between builds.
        - **Expect the end-state check to report a mismatch on the second run,
          and do not read that as a broken replay.** Changing the venting radius
          changes the simulation, so the worlds legitimately diverge; the bench
          already *reports* rather than enforces end state for precisely this
          reason. **What is being compared is the cost of two engine
          configurations under one input stream, not determinism** — and that
          distinction has to be written into whatever records the result,
          because "replayed to a different end state" reads as a failure to
          anyone who has not been told.
    4. ~~**The `Element::ticks` representation prototype**~~ ✅ **Answered
       2026-08-13, and run first rather than last.** It was the one question
       with a consumer waiting — E10 must not write to the byte until it is
       settled, and E10 is item 7. Instrument:
       [tests/velocity_probe.cpp](tests/velocity_probe.cpp), kept. **The answer
       is that the packed speed cannot carry a gravity term** — the probe's
       thrown grain never comes back down — **and that it does not need to,
       because `Element` has three unused bytes nobody had counted.** Full
       answer in the [decisions table](#-decisions-owed); the numbers behind the
       memory half are in [PERFORMANCE.md](PERFORMANCE.md).
    - **The output is four written answers and no shipped feature**, which is
      the point and is the same shape as E4. An instrument that gets built and
      then not decided against is worse than no instrument, so none of these
      closes by being measured — each closes by a sentence written into this
      file.
    - **One and a half are answered, and the half is the instructive part.** The
      `ticks` question closed outright. `P4` built its instrument, took a
      session, decided the budget rule — and then its own census showed the
      session was a painting session with the terrain untouched, which reopens
      `churning` and leaves the fluid spike and `VENT_RADIUS` without a row to
      be judged against. **The instrument is finished and the batch is not**,
      which is a distinction this entry's own rule already made: *"an instrument
      that gets built and then not decided against is worse than no instrument"*
      — and the corollary it did not state is that an instrument fed one
      unrepresentative input produces a decision that looks made. **Remaining
      size is about a day plus two minutes of someone's play.**
    - ✅ **Closed 2026-08-13: four of four.** `ticks`, `P4`, `VENT_RADIUS` and
      the fluid spike are all answered, and `churning` was demoted along the
      way. **The batch's own rule held throughout — none of these closed by
      being measured, each closed by a sentence written into this file** — and
      the estimate is worth recording against the outcome: it said "about two
      days", the work took one, and the critical path was neither an instrument
      nor a decision but **six minutes of someone playing in a way that touched
      the terrain.** The four answers, shortest form: the packed speed cannot
      carry gravity and does not need to; a played row is realistic by
      construction and representative only by evidence; the `VENT_RADIUS` knee
      was an artifact of measuring one build per point; and E5b is worth 8% of a
      played step, so it is not an optimisation. **Two of the four contradicted
      something already written down**, which is the argument for the batch
      having existed.
    - **Updated 2026-08-13, after `VENT_RADIUS`: three and a half of four.**
      Only the fluid spike's subsystem breakdown is left, and it is the one item
      in the batch that still needs an instrument built rather than run — the
      replayed row times a whole `Run::step` and cannot say where inside it the
      time goes. Everything else is answered. **The batch's own rule held all
      the way through: none of these closed by being measured, each closed by a
      sentence written into this file.**
    - **Updated 2026-08-13, after session 2: two and a half of four.** `ticks`
      and `P4` are closed outright, and `P4` took `churning` with it. The fluid
      spike has its **budget envelope** but not its subsystem breakdown;
      `VENT_RADIUS` is untouched but is no longer blocked on anything except
      being built. **The blocker that actually held this batch for a day was an
      input, not an instrument** — every tool needed had been finished since the
      morning, and what was missing was six minutes of someone playing in a way
      that touched the terrain. That is worth keeping in view the next time a
      batch is sized: the estimate said "about a day" and was right about the
      work and wrong about the critical path. **Remaining size is under a day,
      and none of it is blocked on the user.**

---


---

### 🚩 Definition of Done — v0.1 Vertical Slice
The single milestone that matters. Everything before Long Term (VISION.md)
serves this:

> The player enters **one** quantum world, uses physics-based movement and
> destruction to complete **one** objective type, extracts successfully or dies
> losing the run, and their pet ML agent visibly gains from the run and earns
> coins idly.

If that loop is not fun, no amount of factories or stock markets will save it.
If it *is* fun, it is a demo worth showing and a foundation worth expanding.

### 🧭 Where this stands

**Shipped:** the engine and its harness — data-driven materials and reactions, a
fixed timestep, chunked dirty-rect updates, a player body, digging, rigid
structural collapse, and a headless test rig that is now **eleven suites at 421
checks** *(the count read "nine suites at 296" until 2026-08-12 and "ten at 333"
until 2026-08-14; the tenth suite had been there for some time and nothing
re-counted, and `debug_test` is the eleventh)*. Then **F1–F4** (determinism,
`Run`, camera and world-space coordinates, and a way to get a level into the
grid), **E1–E3** (liquids level, heat, fracture), and **V1–V2** (a backdrop
layer, and a palette chosen against it). All of that is at the bottom of this
file with its reasoning intact, along with **[the correctness
pass](ROADMAP_ARCHIVE.md#correctness-pass)** that followed a full read of the
source. Then **V5**
(the art direction — see [notes/art_direction.txt](notes/art_direction.txt)) and
**V6** (the locked palette and its validator — see
[tools/pixel_art.py](tools/pixel_art.py)). Most recently **V7**'s emissive half
pulled forward off its own gate, **V10** (the reticle and the material hotbar —
the reticle is the cursor now), and **V3**/**V3.1** (the player sprite, then
animation from a sheet). **Those five are checked in place in the V track rather
than moved down here**, which is a deliberate change of convention at V5 and not
an oversight: their entries are long, they are cited by `notes/`,
`PERFORMANCE.md` and each other, and moving them would break the one thing this
file is strict about. The Shipped section below is where E1–E3 and V1–V2 went
and it is not growing.

**Open, in order:** the **instrumentation sitting** (which absorbed the fluid
spike), **S0**, then the rest of **Engine & Visual Depth** — the tier this
project is actually sold on — then the **Medium Term** slice, then
**Presentation & Tooling**. *(P2 shipped, session 5 ran, and E4 came back "no",
which is a closed loop rather than a skipped item. **Wave 4, E9's steam half and
F5 all closed 2026-08-12** — E9 is now done in full, and it is the first thing
on this line to be retired by a defect being reported three times rather than by
a plan.)* [ROADMAP_ITEMS.md](ROADMAP.md) carries that order as a table
with sizes against it and is the file to open first; **this one remains the
authority on why, and it is no longer the authority on what is next.** That
split is a deliberate change made on 2026-08-09 and the reason is in the review
that prompted it: this document is 196 KB, the order lives in six preambles
scattered through it, and a plan that is expensive to reorder is a force against
changing direction.

**V4's second half is done — a scene-authorable prop format**
(`src/scene/props.{h,cpp}`, `assets/test_props.txt`, `props_test`). That was the
item after V10 and V3/V3.1, which between them spent the running order this line
used to state.

**The order changed on 2026-08-09, and the two changes are structural rather
than a reshuffle.** They came out of a review that read the physics source
against the engines this project is measured on, and both are recorded in full
at the items they affect.

- **A gameplay spike (`S0`) is pulled ahead of most of the E track.** Everything
  shipped to date is engine or visual foundation, and the whole of the game sat
  behind the whole of the engine tier. `VISION.md` names that exact risk in
  writing — under-building looks like discipline right up until the playtest
  gate — and the running order was doing it anyway. S0 is the thin half of two
  Medium Term items, built now, so the run becomes losable. It is also the only
  thing in the plan that can settle the hook and combat questions by playing,
  and both of those currently block the whole slice.
- **E5 splits into E5a and E5b**, because it was two items wearing one number:
  the per-cell velocity that E6, E4's shove and V9's debris all actually need,
  and the air/pressure field, which is far larger and goes after the slice.
  Bundled, the enabler looked as expensive as the research project attached to
  it, which is most of why it kept not being started.

**Seven more items are new as of 2026-08-11, and they are one plan rather than
seven ideas: `V12`, `V13`, `V14`, `V15`, `V16`, `E12` and `S1`.** They come from
a stated commitment to what the visual system is *for* — varying pixel
resolutions and sprite sizes in one scene, procedural animation, animated rather
than static backgrounds, and entity bodies that granulate locally when damaged —
and the place they are argued is [the V track's own section on
it](#the-visual-system-this-track-is-now-building-toward). **Two things about
how they were admitted are worth knowing before reading them.** Five of the
seven pass the tier's normal test on their own: a colour key that forbids a soft
edge anywhere, a draw path that turns a denser drawing into a bigger object, a
backdrop whose only depth cue stops when the player does, an articulation limit
already written down twice, and a walk cycle that assumes a floor this game does
not have. **The other two — `V15` and `E12` — are admitted by conditions that
`notes/procedural_animation.md` and `notes/granulating_enemies.md` wrote down in
advance**, each naming what would stop it being a note, and the commitment
supplied it. That is a better form of admission than an observation, not a
weaker one, because it was specified before the thing that satisfied it existed.
**Nothing in the running order moved for them:** wave 4, E9's steam half, the
fluid spike and `S0` are unchanged, the visual block sits where `V11` already
sat, and `S1` is blocked on the combat decision it does not get to pre-empt.

**Three items are new: `E10`, `E11` and `V11`,** plus `P3` in the performance
track. E10 (powders come to rest) is the cheapest large improvement in feel
available and it was found by reading `step_powder` against the reference
behaviour. V11 makes the visual system cheap to redirect, which is a stated
requirement that nothing in the plan was serving. E11 is four table columns that
were each too small to be an item alone. **Nothing was renumbered** — the IDs
stay stable, per the rule below.

**Session 5 ran, wave 3 closed, and the session is worth reading before anything
is picked up.** [Wave
3](ROADMAP_ARCHIVE.md#wave-3--the-brush-destroyed-water-and-the-elevator-it-was-hiding) closed on
W-2 and W-4 exactly as written, and then the session produced six defects and
five observations against parts of the game nobody had scheduled — **four of the
six defects came from README's nine-step checklist, which the session was not
booked to run and which everyone treats as a formality.** The previous version
of this paragraph warned that four items had been sequenced without anyone at a
keyboard; the session bore that out at a rate of roughly one finding per
checklist step, and the ordering above is a direct consequence rather than a
reshuffle. Full record: [session 5
results](PLAYTEST_LOG.md#session-5-results--wave-3-closes-and-the-water-underneath-it-does-not).

**The session's most valuable output was not a fix, it was a discrimination, and
it changed a plan entry.** W-5 was written in advance to separate two possible
causes of one symptom — *"if the lift is gone and the staggering is not, that is
a new finding about the flow model rather than about venting"* — and it came
back on the second branch. That converted an unlocalised "water is bugged" into
a finding against `step_fluid`'s lateral run, and **withdrew E11's "no action is
proposed"**, which had classified the jump as a harmless known property of the
genre. The property was described correctly and filed wrongly. **The
generalisable form: a checklist row whose two answers point at two different
subsystems is worth more than a row that can only pass or fail**, and it costs
the same to write.

**Two findings arrived attached to the wrong mechanism, and both were caught by
reading code rather than by re-testing.** Phase B asked four questions about the
*absence* of player/material interaction; E-1 came back describing something
that very much interacts (`resolve_overlap`, now D2 — and a containment failure)
and E-2 came back describing a step-height constant. **That is not a failure of
the phase, it is why its "no" is trustworthy** — but it is the second time this
project has had a finding wearing the mechanism the tester was primed to look
for, after A7b, and the remedy both times was to go to the source before filing
the work.

**Read this before picking V8 out of the V track's running order, because the
order is now misleading about it.** V8's first three layers are *built* — sky
and mountains ship with parallax from `main.cpp`'s `PARALLAX_SKY_X/Y` and
`PARALLAX_MOUNTAIN_X/Y`, and trees at factor ~1 shipped as V4's prop slice,
drawn before the cell texture exactly as `notes/art_direction.txt`'s layer model
specifies. What remains in V8 is a second biome, time-of-day variation, and a
third depth band, and **all three are gated on a reference that does not exist
yet**: none can answer this tier's first admission question, because nothing in
the built game is wrong or reads badly there. Picking V8 now would be
reference-driven breadth wearing engine clothes, which the preamble below names
as the exact failure mode to watch for. **V4's prop half was taken instead and
is the worked example of the difference** — it could name what was wrong (no
format, so no second scene could have props) and what it unlocked (Quantum
Worlds). V8's remainder can name neither yet, and the honest move is to leave it
until a second biome actually needs one.

**One small piece of V8 is real and is not the rest of it:** its parallax
factors are duplicated between `main.cpp` and `tools/generate_backdrop.py` with
nothing enforcing agreement, and the failure is a seam at the pan limit. V3.1
closed the identical duplication for the player sheet by generating the header,
which is the precedent to copy. That is an afternoon and it is not a tier item.

**One thing to read first if you have been away:** the correctness pass linked
above found that V2's palette retune had silently emptied the startup scene, and
that a settled pool of water never went to sleep. Both are fixed and both now
have tests, but the *reason* they went unnoticed is the part that changes how
the next item should be built — see [What the correctness pass changed about how
to work here](ROADMAP_ARCHIVE.md#correctness-pass-lessons).

---

## 🛠️ W — The workbench (how this project is worked on)

*New track, opened 2026-08-17 out of an external review of the repo. **It is the
next thing to be worked on**, ahead of V22, and the reason it can go first is in
the ordering note at the bottom of this section rather than in enthusiasm.*

**Every entry in this section is wrapped at 80 columns on purpose.** That is
`W1`'s convention, applied to the first text written after it was decided, so
the convention has at least one compliant example before anybody is asked to
follow it.

### The finding this track exists to answer

The review's one structural claim, and everything below is downstream of it:
**the reasoning in this project is archived rather than indexed.** The rule at
the top of `CLAUDE.md` — that the reasoning is the deliverable — is correct, is
being followed, and is producing documents that are accurate. The review checked
the most falsifiable claim it could find, `CLAUDE.md`'s "the full suite is 14
suites", against `ctest --test-dir build -N`, and it is true. *(Its first
measurement said otherwise: `grep -c add_test CMakeLists.txt` returns **20**.
Six of those twenty are comments explaining why `grid_bench` and the five probes
are deliberately **not** registered. **The doc was right and the grep was
wrong**, and that is recorded here rather than dropped because it is the exact
shape of false alarm this project's own rules invite — a reader who trusted the
cheaper measurement would have "corrected" a true claim into a false one.)*

What is failing is not the policy. It is the **storage format**, and it fails in
four measurable ways.

**1. The doc corpus is the size of the codebase.** 1.09 MB of Markdown against
1.20 MB of `src/` + `tests/` + `tools/`. `ROADMAP.md` alone is 399 KB — 69% the
size of all of `src/` — and 67,371 words. `CLAUDE.md` already concedes the
consequence, instructing readers to "search it, don't read it front to back". A
document that can only be grepped is a database with no index.

**2. The four heaviest documents are unwrapped, and that multiplies the cost of
every search into them.** A grep hit is atomic at the line, so a 3,000-character
line returns 3,000 characters whether or not the match needed them.

| File | Lines | Avg line | Max line | Lines >500 chars |
|---|---|---|---|---|
| `ENGINEERING_NOTES.md` | 91 | **601** | 2,386 | 53 |
| `ROADMAP.md` | 1,008 | **394** | 3,083 | 367 |
| `ROADMAP_ITEMS.md` | 641 | 285 | 4,102 | 128 |
| `PERFORMANCE.md` | 428 | 205 | 1,688 | 69 |
| `README.md` | 901 | 63 | 508 | 1 |
| `PLAYTEST_LOG.md` | 975 | 83 | 1,042 | 17 |

Measured: `grep -C2` for `determinis` in `ROADMAP.md` returned **9,930 bytes for
20 lines**. The same shape of query against wrapped text returns about 1.3 KB.
**The project already wraps** — `README.md` and `PLAYTEST_LOG.md` are clean — so
this is an inconsistency to remove, not a new convention to import.

**3. The plan is written twice, by instruction.** Extracting every item ID
matching `(E|V|P|F)[0-9]+[a-z]?` from both roadmap files gives **48 IDs in
`ROADMAP.md`, 48 in `ROADMAP_ITEMS.md`, and 48 in both** — a perfect overlap
across 582 KB, with near-identical section headings on top of it. This is not a
discipline failure. `CLAUDE.md`'s routing table says *"ROADMAP.md (the why) /
ROADMAP_ITEMS.md (the order)"*, and **every item has both a why and a
position**, so the table mandates that every item be written in two places and
thereafter maintained in step. **The clearest evidence is in this file's own
preamble**, which states "This is the only document that carries development
steps" — a sentence that was true when written and that `ROADMAP_ITEMS.md` has
falsified. By the project's own standard that is worse than no claim, and it is
`W4`'s job.

**4. Nothing is mechanised.** There is no `.claude/settings.json` in the repo at
all — no permission allowlist, no hooks. The consequence that matters is not the
prompting; it is that **the project's self-declared first risk, "a stated rule
that stopped matching the code and kept being believed", is defended only by
human vigilance.** Four commits exist purely to repair such a claim, the most
recent being `814ac71`, "Correct the suite count in CLAUDE.md and README, which
V23 made false". Vigilance caught those four. It is the wrong instrument for the
fifth.

### What the review checked and found healthy, so nobody spends a session on it

Recorded because "not a problem" is a finding, and an unrecorded one gets
re-litigated.

- **`Grid` is well factored and is not to be split.** `grid.cpp` is the largest
  translation unit at 1,664 lines, and it is 35 named methods — `step_powder`,
  `step_fluid`, `step_thermal`, `vent_fluid`, `fracture_landing`, `seek_level`
  and the rest — averaging 47 lines. Size here is subject matter, not sprawl.
- **The 14 separate test executables stay separate.** The isolation is what the
  `ENGINE_SOURCES` / `RENDER_SOURCES` split buys, and the whole suite runs in
  about a second. There is nothing to win.
- **The `.claude/rules/` three-way split, `TUNING.md` and `PLAYTEST_LOG.md` are
  all load-bearing separations** with their arguments already written down. None
  of them is a consolidation candidate.
- **`main.cpp`'s `#include <random>` is not an invariant violation.** It is used
  once, at `std::random_device rd;`, to pick the world seed — which is precisely
  the boundary the determinism rule draws.

### The items

**`W1` — Reflow the four unwrapped documents to 80 columns. Shipped
2026-08-17.** *(afternoon)*

*What it cost and what it bought, recorded because the item's own verification
clause asked for exactly these numbers.* Line counts went 1,220 → 6,158
(`ROADMAP.md`), 695 → 2,466 (`ROADMAP_ITEMS.md`), 92 → 785
(`ENGINEERING_NOTES.md`), 429 → 1,377 (`PERFORMANCE.md`); average line length
394/285/601/205 → 68/77/71/62, the two above 70 being table rows, which are
exempt. **Word sequence and link count are identical per file, verified
mechanically against `HEAD` rather than by reading** — that check is the whole
safety argument for a scripted bulk rewrite of the corpus, and it is the check to
repeat if this is ever done again. The retrieval gain, three queries run against
the old and new `ROADMAP.md` back to back: `EASE_PER_SEC` 2,014 → 372 bytes,
`determinism` 29,474 → 5,062, `parallax` 54,607 → 9,726. **About a fifth to a
sixth, which is what the item predicted, and the prediction is left standing
above rather than rewritten to match.**

*Two things the pass learned that the item did not anticipate.* **A per-line
wrapper is not enough — paragraphs have to be regrouped and rewrapped**, or a
long line's tail becomes a two-word orphan line and the file reads worse than it
did unwrapped. And **four-space-indented prose under a nested list is not an
indented code block**; treating it as one, which is the conventional Markdown
reading, left eleven of `ROADMAP.md`'s longest lines untouched. There are no
indented code blocks in these four files, which is what made the simpler rule
safe. **The wrapping convention now lives in `.claude/rules/documentation.md` as
a rule the tree obeys**, with the before/after numbers; it used to be stated
there as a rule four named files violated.


`ROADMAP.md`, `ROADMAP_ITEMS.md`, `ENGINEERING_NOTES.md`, `PERFORMANCE.md`. Pure
formatting, **zero information loss, no decision touched, no wording changed** —
which is what makes it the first item rather than the most valuable one. It also
repairs `git diff` and `git blame`, which currently render a one-word change as
a 3,000-character line rewrite. *Verify:* word count and link count unchanged
per file before and after; `ctest` untouched by construction; a spot grep costs
roughly a sixth of what it did.

**`W2` — `.claude/settings.json` with a permission allowlist. Shipped
2026-08-17.** *(afternoon)*
`cmake`, `ctest`, `git`, `python tools/*`. The mechanical half of the friction.
Note that this adds no dependency — it is a config file the harness already
looks for.

*Shipped as 40 rules, and the count is the interesting part.* The item named
four things to allow and the file lists ten times that, for two reasons the item
did not anticipate. **This project is worked through two shells, not one** —
`PowerShell` is the primary here and `Bash` is also available, they take
different syntax, and a permission rule is scoped to the tool that runs the
command. So every entry is written twice, once per tool, and a rule that exists
for only one of them is a rule that silently does nothing half the time. And
**`git` could not be allowed as `git`**: the four-word summary hides that `git
log` and `git push` are not the same kind of act. The file allows the reading
verbs (`status`, `diff`, `log`, `show`, `blame`, `grep`, `ls-files`, `branch`)
and the three staging/committing ones the working agreement already governs
(`add`, `rm`, `commit`), and stops there. The exe entries cover the six by-hand
probes in `CLAUDE.md`'s command list; `SlopPhysics.exe` is deliberately **not**
among them, because it opens a window and blocks, and nothing headless should be
launching it.

*There is no `deny` list, and that is a decision rather than an omission.*
Anything unmatched already falls through to asking, so a deny list would only
defend against an allow rule written too broadly — which is a reason to keep the
allow rules narrow, not a reason to maintain a second list that has to stay in
step with the first. The concrete cost of the alternative: denying `git push`
outright would mean the user cannot ask for a push without first editing this
file, which is friction pointed the wrong way. **If a broad allow is ever added,
that is the moment to revisit this**, not before.

*The file is versioned on purpose and `.gitignore` already said so.* It ignores
`/.claude/settings.local.json` and `/CLAUDE.local.md` by name — the machine-local
halves — with a comment explaining that `CLAUDE.md` and `.claude/rules/` are
versioned because they are project instructions. `settings.json` is the third
member of that set, and needed no `.gitignore` change to become one.

**`W3` — Make doc-truth a test rather than a discipline.** *(afternoon)* A
fifteenth `ctest` suite asserting the docs' **checkable numeric claims** against
their sources: the suite count against the registered tests, `Element`'s size
and free-byte offsets against the `static_assert`s in
[element.h](src/physics/element.h), the golden checksum quoted in prose against
the one in [tests/test_golden_frame.cpp](tests/test_golden_frame.cpp),
`FIXTURE_SCENE_CELLS` against [tests/test_scene.cpp](tests/test_scene.cpp).
**This is the highest-leverage item in the track and it is deliberately not
first**, because `W1` and `W4` will move the very lines it pins and pinning them
twice is the mistake V20 and V21 already made one level down. **Scope limit,
stated so it does not creep:** it can only ever check claims that have a
machine-readable source of truth. It cannot check reasoning, and an attempt to
make it do so turns the docs into a format rather than an argument.

**Shipped 2026-08-17** as `docs_test` ([tests/test_docs.cpp](tests/test_docs.cpp)),
registered last in `CMakeLists.txt` so the count it asserts is the one at the
bottom of `ctest`'s output. Seven checks, the four specified plus three the item
asked for or the work turned up:

- **The suite count**, from `add_test(NAME ` registrations — *not* `grep -c
  add_test`, which returns 20 against a true 15 because six of the mentions are
  comments explaining why the probes are deliberately unregistered. That false
  alarm is on the record in this track's opening finding, and the suite is
  written to not reproduce it. The headless count is **derived** (`suites` minus
  the one target linking `SDL2-static`) rather than written down twice.
- **`Element`'s size and its three free bytes**, from `sizeof` and `offsetof`,
  not from the comment in `element.h` — this struct's byte count has been got
  wrong twice by counting fields, so the check counts the hole the compiler
  actually leaves.
- **The golden checksum**, parsed out of `test_golden_frame.cpp` and required in
  `TUNING.md` and `ROADMAP.md`. Three copies of `0xcde4dc1a39927fca` is three
  chances to drift, and `V23b` made the number's *provenance* the evidence a
  revert was complete.
- **`FIXTURE_SCENE_CELLS`**, as the whole `Scene: 1920x1080, N cells placed`
  launch line, in `MANUAL_TESTING.md` and `ASSETS.md`. A stale copy there has
  the tester passing a world that failed.
- **The Manual Tester Checklist's length** (thirteen), counted from the numbered
  markers and matched against the word spelled in `CLAUDE.md`, `ROADMAP.md` and
  `.claude/rules/documentation.md`. It changed twice in two days.
- **The two sizes `W4` created**, live plan and archive, against the files on
  disk. **The 10% tolerance is the point, not a weakness:** the docs quote KB
  rounded and not consistently binary or decimal — 350,147 bytes is 342 KiB or
  350 kB against a stated 346 — so an exact assertion would mean editing prose on
  every roadmap commit, which is how a check gets deleted. Ten percent catches
  the file quietly doubling, which is the failure that matters.
- **No live Markdown link to the deleted `ROADMAP_ITEMS.md`.** Prose citing it
  inside a dated entry is correct as written and is not what this looks for.

**Two decisions worth carrying forward.**

**Claims are matched on whitespace-normalised text.** The first version compared
raw bytes and failed on a true sentence, because `W1` wraps prose at 80 columns
and the break moves whenever a word earlier in the paragraph is edited — the
first instinct was to reword `CLAUDE.md` so the needle fit on one line, which is
the document bending to the checker. Normalising makes a claim about a
*sentence* checkable and incidentally kills the CRLF/LF mismatch that has cost
this project two sessions.

**Two things are deliberately not pinned, and this is the boundary to defend.**
`notes/handoff_prompt.md`, because it is rewritten whole at every session close,
so a pin would fail on the day it stops mentioning a number rather than the day
a number goes wrong. And historical records — `PLAYTEST_LOG.md`,
`ROADMAP_ARCHIVE.md`, dated result lines inside live entries — because a number
there is a record of what was measured *then*, correct as written, and pinning
one would force the archive to be edited, which is precisely what the archive
promises never to need.

**Every check was verified against a mutated document before being kept**, one
at a time, restoring after each: a wrong cell count in `ASSETS.md`, a wrong size
in the rules file, a wrong checksum in `TUNING.md` and again in `ROADMAP.md`, a
wrong free-byte count in `CLAUDE.md`, a wrong step word in `ROADMAP.md`, and a
deleted checklist marker. **Two mutations initially failed to fail and both were
the mutation's fault, not the check's** — replacing only the *first* of
`TUNING.md`'s two checksum copies left the claim satisfied, and renumbering a
checklist marker from `13.` to `14.` does not change how many markers there are.
That is worth recording because "the test did not go red" was the wrong
conclusion twice in ten minutes. The suite's first genuine failure was real:
`CLAUDE.md` and `README.md` still said fourteen suites, and it caught them.

**`W4` — One live plan; the shipped reasoning moves to an archive.**
**Shipped 2026-08-17**, in two commits, and the split into two is the part worth
copying. *(Spec, as written: `ROADMAP_ITEMS.md` keeps* Next up, Running order,
Decisions owed *and* Prerequisites, *each open item absorbs its own rationale
inline, the rationale for shipped items moves to a dated `ROADMAP_ARCHIVE.md`
that **nothing is ever required to read**, nothing is deleted, and the
`CLAUDE.md` routing-table row is part of the item. The archive boundary was left
as the user's call.)*

**The boundary the user chose was the strict one — *finished **and** nothing
open depends on the reasoning*** — with the sequencing done as two commits
rather than one. The alternatives were on the table and both were worse: a
section-level cut is safe and moves only a third of the file, and a status-only
cut ("anything marked done goes") is fast and **quietly breaks the file's one
promise**, because a closed item whose finding still binds an open one becomes
required reading the moment it is archived. The strict boundary costs a
judgement per entry and buys an archive nobody has to open.

- **Part 1 was a pure relocation and was verified as one.** The `## ✅ Shipped`
  section and the Waves section — the two that are wholly closed — moved out in
  their original wording. The check was a **token multiset comparison** against
  the previous commit: nothing added, and exactly one token lost, the `---`
  separator at the cut. That is the check to reuse; a diff is useless here
  because a move looks like a total rewrite to `git diff`.
- **The measurement that made the boundary a decision rather than a formality:**
  `## ✅ Shipped` was **86 KB of a 448 KB file**. Cutting at the heading — the
  obvious move — would have moved 19% and left the problem. The other 362 KB was
  *also* mostly closed work, interleaved with open items inside the live tracks,
  because the tracks are ordered lists where each step's reason for preceding
  the next is the content.
- **Part 2 folded the two files into one.** `ROADMAP_ITEMS.md`'s navigation went
  to a **The plan** block at the top of this file; its glossary went to the
  preamble; its per-track running orders went to their track headings; **its
  ablation table went into the live E track**, where it turned out to be the one
  thing in that file with no counterpart here and load-bearing on E5a, E5b and
  E10. **The opening plain-language paragraph of each of the 23 open items was
  folded into that item's entry** as an *In plain terms* block. Everything else
  was the duplication, and it went to the archive rather than being deleted.
- **Then 15 closed entries were swept out of the live tracks**, each leaving a
  one-line stub pointing at its full entry in the archive. This is the pass the
  strict boundary buys and the one a status-only cut would have got wrong.
- **One finding was absorbed before its entry moved, and it is the worked
  example the rule is now stated on:** `V23b` is closed, but the ~50% cap that a
  centred camera puts on the receding plane's visible share is a live constraint
  on `V22`. It is now written into `V22`, saying where it came from and when.
  **Move the finding before you move the entry** — otherwise the archive becomes
  required reading and the item is undone.
- **Result:** 448 KB + 199 KB in two files → **346 KB live, 319 KB archived**,
  with 18 cross-file links retargeted and 17 anchors repointed across the two
  commits. `ROADMAP_ITEMS.md` is deleted. The preamble sentence *"this is the
  only document that carries development steps"* is true again, and the
  paragraph above it now records that it was false in between rather than
  quietly resuming.
- **What this does not do:** it does not shrink the live file to something a
  session reads front to back, and it was never going to. 346 KB is still a
  search target. What changed is that **everything in it is open**, so a search
  hit is about work that has not happened yet.

**`W5` — Extract `main()`.** *(days)* `main()` runs from its opening brace to
the end of [src/main.cpp](src/main.cpp) — **1,377 lines**, about 92
blank-delimited blocks, covering display-mode negotiation, texture creation,
backdrop and prop binding, sheet loading, seeding, scene load, objective
planting, the `F9` recorder, the V23 camera wiring and the settings menu, and
then the frame loop. The line count is not the argument. **The argument is the
causal chain:** `CLAUDE.md` requires the Manual Tester Checklist after changes
to `main.cpp` *because the suites cannot reach it*, and the suites cannot reach
it *because it is one function*. So `main.cpp`'s shape is the thing converting
machine-checkable work into human-checkable work — and the human is one person,
who is currently holding the V23 feel report that V22 is gated on. **The pattern
is already proven in this repo and stopped halfway.**
[src/game/run.h](src/game/run.h) is SDL-free for exactly this reason, in its own
words — *"a run that needs a window cannot be driven by a test"* — and
`tests/test_run.cpp` is 562 lines of driving it. The shape: a `boot` unit
returning a populated struct, the per-frame composition joining the existing
`render/frame.cpp`, and a `main()` of roughly 150 lines of SDL lifecycle and
pump. *Verify:* the launch line still prints `Scene: WxH, N cells placed` with
`N` = 334901; `golden_frame_test` unmoved; **and at least one checklist step
demoted to a headless assertion, because if none can be, the item did not buy
what it was admitted on.** **This is the one item in the track that needs the
tester afterwards.**

**Part 1 shipped 2026-08-17: the `boot` unit, and the acceptance check is
already met.** `src/game/boot.h` now holds the world size, `OBJECTIVE_X`, the
two terrain scans, `place_objective` and `plant_props`; `choose_display_mode`
joined `game/display.h`; `boot_test` is the sixteenth suite, links
`ENGINE_SOURCES` + `SCENE_PROP_SOURCES`, and runs from the repo root.

- **What was actually demoted, which is the number the item is judged on.**
  Checklist step 1 carried three printed lines. `scene_test` already pinned the
  cell count; the other two — `Objective: (1700, 932)` and `Props: 9 of 9
  placed` — were a person noticing a number was absent. Both are now asserted
  **against the shipped fixture**, which is what makes them the same check the
  tester was running rather than a weaker one beside it. The regression class
  behind the second is the buried trees, which hid through a whole feature
  because the props sit off-screen at spawn.
- **The seam is that SDL gets *told*, not asked.** A prop's width comes from its
  texture, which is the only thing in startup that genuinely needs a window — so
  `plant_props` takes a `widths` vector and `main.cpp` fills it from
  `SDL_QueryTexture`. The test fills it from `bmp::read` on the same BMPs, and
  one BMP pixel is one world cell, so it is the same number. **That question —
  *what does this need a window for, and can it be told instead?* — is the
  reusable part**, and it is what parts 2 and 3 should be pointed at first.
- **A header, not a sixth source-set variable**, following `debug_view.h`. The
  build graph is unchanged, which is the property that keeps `CMakeLists.txt`'s
  guard meaning what it says.
- *Verified:* all 16 suites pass; `golden_frame_test`'s checksum did not move;
  the exe was launched and printed `Display: 3440x1440`, the seed, `Scene:
  1920x1080, 334901 cells placed`, `Objective: (1700, 932)` and `Props: 9 of 9
  placed`, with nothing on stderr. `main.cpp` went 1,625 → 1,554 lines, and
  **that is reported rather than claimed as progress** — the line count is not
  the argument and part 1 was never going to move it much, since what left was
  ~70 lines of scan and the reasoning went with it.

**Part 2 shipped 2026-08-17: the shell's decisions, same treatment.** All three
candidates went, into two headers with one suite. `src/game/pacer.h` holds the
freeze rule, the step count, the interpolation alpha and the teleport clamp;
`src/game/settings_menu.h` holds the menu's navigation and selection;
`shell_test` is the seventeenth suite and links `ENGINE_SOURCES` only.

- **The freeze rule is one function now, which is the thing part 2 was told to
  get right.** `pacer::world_advances(menu_open, run_over, paused)` — three
  reasons, one mechanism, and the argument for the mechanism (freezing means
  *not accumulating*, never skipping the step loop with the accumulator still
  filling) is written at the function, where the fourth caller will read it,
  instead of in a comment at the one call site. It is stated as three named
  bools rather than one `frozen` so that a caller has to say which of the three
  it means and cannot freeze for a fourth reason nobody wrote down.
- **The menu adds a shape `boot.h` did not have: the SDL half in the middle.**
  Applying a display mode needs a window, and it happens *between* the decision
  and its consequence. So `menu::key` returns `Act::ApplyMode` and the caller
  reports back through `mode_applied` / `mode_refused` — a round trip, rather
  than a callback that would drag the window back into the state machine. That
  is the only shape in which "try it, and if it fails say so and change nothing"
  is expressible in here. **The asymmetry worth keeping**: a switch that applied
  but could not be *saved* leaves the menu open, because the mode did change and
  the unkeepable promise is about next launch — the only place to say so is the
  screen still open.
- **What `shell_test` catches that a look at the screen cannot.** Sixty frozen
  frames bank nothing and the frame after runs one step, not sixty; one second
  buys sixty steps however it is cut up; a ten-second stall is clamped; a
  negative or NaN frame time never reaches the accumulator (a NaN there is
  permanent — every later comparison against `FIXED_DT` is false, so the world
  stops stepping and nothing says why); alpha is pinned to 1 while paused; and
  the teleport clamp snaps **both** axes when either exceeds the limit, since
  easing one while snapping the other draws a diagonal that never happened.
- *Verified:* all 17 suites pass; `golden_frame_test`'s checksum did not move,
  which is the evidence that a rewritten frame loop draws the same frame; the exe
  was launched, printed the five launch lines with nothing on stderr, and closed
  cleanly through `SDL_QUIT`, so the loop ran. `main.cpp` is 1,544 lines, down
  from 1,554 — reported, not claimed, for the reason part 1's line is. **This
  entry said 1,436 until 2026-08-18, which was a guess written as a
  measurement**; part 2 moved code sideways into two headers and replaced it
  with glue, so ten net lines is what that actually costs, and the misprint made
  the extraction look four times more productive than it was.
- **`main.cpp` is not near 150 lines and part 3 is what closes that**, so the
  target in the item head above is still open, not quietly dropped.

**Part 3 — the composition. Shipped 2026-08-18.** The HUD, the reticle, the
run-over wash and the settings screen were ~250 lines of draw calls after
`frame::compose`, and they are now `render/overlay.cpp` behind one call taking
one struct — which puts them under `golden_frame_test` for the first time.

- **They did *not* join `render/frame.cpp`, and this entry said they would.**
  `frame.h` carries a rule in bold — *UI is not here and does not become here* —
  whose argument is the light pass: everything in that file is in the world and
  gets lit, everything after it is not, and defect B1 is a reticle that goes
  orange near a flame. The goal part 3 was written for is **reach**, not one
  file, so the UI got its own translation unit called after `compose` returns,
  and the composition never learns it exists. Correcting the plan rather than
  the rule is the point: the rule had its reason written down and the plan did
  not.
- **Two checksums, not one.** `GOLDEN` still hashes `frame::compose` alone and
  is taken before the overlay runs; `OVERLAY_GOLDEN` (`0x0db76672f5ec3189`)
  hashes the surface after `overlay::draw` has run on top of it. One number over
  both would make a change to the HUD indistinguishable from a change to the
  sky, which is the whole reason the boundary exists.
- **The no-op half was shipped first, as the house style requires, and it
  read clean.** The move was built and the full suite run with
  `golden_frame_test` still calling only `compose`: 17/17, `GOLDEN` unmoved at
  `0xcde4dc1a39927fca`. So the ~250 lines are known not to have touched a pixel
  of the world *before* any of them were hashed, and `OVERLAY_GOLDEN` is a
  first statement about the UI's pixels rather than a mixture of two causes.
  **It has no earlier value to be compared against**, and the comment at the
  constant says so plainly: this check cannot yet prove the UI is unchanged,
  only that from here on it notices.
- **A checksum over four blocks does not say four blocks are in it**, which is
  the null-texture lesson `simulation.md` names, so each is switched off on its
  own and required to move the frame: the reticle, the HUD stack, the hotbar,
  the wash, the settings screen, the notice, and the greyed *(TOO LARGE)* row.
  All eight passed. The fixture also runs at `ui_scale` **2 rather than this
  surface's own `window_h / 270`, which is 1** — no shipped mode produces 1, and
  every backing rect and gap in the overlay is a multiple of that number.
- **`src/ui/` and `overlay.cpp` joined `FRAME_SOURCES`**, which is what actually
  buys the reach: `text.cpp` and `hotbar.cpp` were built only into the game
  executable, so nothing linked them. No sixth source-set variable — the guard
  in `CMakeLists.txt` still means what it says, and the comment there now names
  the test the contents pass rather than leaning on the word "frame".
- *Verified:* 17/17; `GOLDEN` unmoved across the move and again after; the exe
  launched, printed all five launch lines with nothing on stderr and exited
  through `SDL_QUIT`. `main.cpp` is **1,395 lines**, down from 1,544.
- **The ~150-line target is closed unmet, 2026-08-18: there is no part 4.**
  What remains in `main()` is the frame's *wiring* — building `frame::Params`
  and `overlay::Params` from a `Run`, a `Camera` and a `DisplayMode` — plus the
  input handling. Those are the two blocks a fourth part would have taken, and
  the decision went against it: the reach argument that justified parts 1 to 3
  is spent. Each of those parts moved a *decision* behind a seam a test could
  then hold — boot order, the pacer, the settings menu, the overlay's drawing.
  The wiring and the input handling are neither: they are `main()` reading its
  own locals, and a unit that took them would need every one of those locals
  passed in, which is the shape of a move that buys nothing. **What was left
  was length, and length is not a defect.** 1,395 lines is the number; the
  target was written before parts 1 to 3 showed what was actually separable,
  and it is the target that was wrong, not the file. Reopen this only if a
  concrete change is made hard by the wiring living in `main()` — that is a
  symptom the target never had.

**`W6` — Trim `README.md` to a front door.** *(afternoon)* **Shipped
2026-08-18.** It was 917 lines doing
four jobs: build/run/test, 153 lines of benchmark procedure, 534 lines of engine
architecture, and the public fundamentals pass. The benchmark section is a
**lossy restatement** of a file that owns the topic — it shares
`PERFORMANCE.md`'s entire distinctive vocabulary while carrying a fraction of it
(`churning` 5 uses against 69, `p99` 1 against 19). Architecture goes to
`ENGINEERING_NOTES.md`, benchmark procedure to `PERFORMANCE.md`, and README
keeps build, run, test, controls, `## General Testing` and links out. **`##
General Testing` stays public and stays short** — that is already a written rule
and this item must not be read as licence to move it.

- *Done:* **917 lines → 259.** `## Engine Architecture` and its eight
  subsections (536 lines) are at the end of `ENGINEERING_NOTES.md`; `## Running
  the Benchmark` and `### The replayed row, and recording one (P4)` (153 lines)
  are at the top of `PERFORMANCE.md`, above a new `## The numbers` heading that
  the existing series needed anyway. **Both moved verbatim**, so the section
  anchors are unchanged and only the file in front of the `#` moved.
- **The links were the actual work, and they were the risk.** Nine anchors were
  linked from `MANUAL_TESTING.md` (six checklist steps), `PERFORMANCE.md`,
  `ROADMAP.md` and `ROADMAP_ARCHIVE.md`; all were repointed, and README's own
  four internal links now cross files. `docs_test` does not check these — it
  checks for a live link to the deleted `ROADMAP_ITEMS.md`, which is a different
  question — so a moved heading is checked by nobody. **That is now a written
  rule in `.claude/rules/documentation.md`: rename one of those headings and
  fix the callers in the same commit.**
- *Not done, deliberately:* README keeps its `## Running the Benchmark` heading
  as a seventeen-line pointer with the command in it. Deleting it outright would
  have made "how do I run the bench" unanswerable from the front door, which is
  the opposite of the item. **A pointer that names the owning document is not
  the restatement this item was spent on** — the restatement was 153 lines
  sharing `PERFORMANCE.md`'s whole vocabulary at a fraction of its content.
- *Also updated, because the boundary moved:* `CLAUDE.md`'s routing table (three
  rows), `.claude/rules/documentation.md` (the three per-file sections), and
  `ENGINEERING_NOTES.md`'s opening paragraph, which described a file that no
  longer held only deferred decisions.
- *Verified:* 17/17, and the build line read before the test line. No prose
  change to any moved paragraph beyond three self-references that the move had
  turned circular (`README` telling the reader to go and read `PERFORMANCE.md`,
  from inside `PERFORMANCE.md`; the same for the RNG entry).
- **The W track closes here, and as of 2026-08-18 it closes entirely** — the
  one thing left open in it, `W5`'s ~150-line `main()` target, was closed unmet
  by decision the same day; the argument is at the end of the `W5` entry. Next
  is the V track at `V22`, which the tester's UI look unblocked that day.

### Refused, or deferred with the reason

- **Untracking the generated BMPs is deferred, not scheduled.** `assets/` is 49
  MB, of which about 44 MB is script-generated — `backdrop_mountains.bmp` 21 MB,
  `backdrop_sky.bmp` 16 MB, `test_albedo.bmp` and `test_material.bmp` 6.2 MB
  each — all reproducible from `tools/generate_backdrop.py` and
  `generate_test_scene.py`, with `.git` at 20 MB and growing by tens of MB per
  backdrop regeneration. **The reason it is not scheduled is that generating at
  build time makes Python a *build* dependency where it is presently only a
  *tools* dependency**, which is the "zero new dependencies" invariant, and
  because `golden_frame_test`, `test_scene.cpp` and `rim_probe` all need those
  fixtures byte-identical. **Reopen trigger: clone or fetch time actually
  hurting.** Whoever spends it proves generator determinism first — a generator
  that is one byte non-reproducible turns a checksum suite into a flaky one.
- **Splitting `.claude/rules/simulation.md` is noted and not scheduled.** At 25
  KB it loads for `src/physics/`, `src/game/`, `main.cpp`, `tests/` and
  `CMakeLists.txt` — effectively every code path — and its
  harness-and-build-graph half could scope more narrowly. It is left alone
  because the measured baseline it contributes to is acceptable and because `W5`
  changes which files exist.

### Why this track goes ahead of V22, which is the only ordering claim here

**V22 is blocked on a human and `W1`–`W6` are blocked on nothing.** The V23 feel
report is owed, V22 must not start until it comes back, and question 3 of that
report can still return an answer that changes what V22 is. Running the
workbench track in that window costs the V track nothing at all.

Two second-order reasons, both of which would hold anyway. **`W5` is aimed
directly at the queue V22 is sitting in** — every checklist step it converts to
an assertion is a step the tester does not have to run before the next visual
change ships. And **`W1` and `W4` are cheapest now and never cheaper again**:
both are proportional to the size of the corpus, and the corpus only grows.

**One thing must not be read into this.** The volume of writing is *not* the
defect and reducing it is not the goal — the review's own conclusion was that
the documentation discipline is this project's genuine strength. Every item
above changes where reasoning is stored or how it is retrieved. **None of them
is licence to record less.**

---

## 🟤 Engine & Visual Depth — the selling point

*Three tracks. **E** deepens the simulation, **V** gives it an identity, **P**
pays for both. This is the tier the project is bought on, and it is scheduled
ahead of the gameplay slice on purpose — the argument is in `VISION.md`'s
Project Goals and Scope Discipline and is not repeated here.*

**This section used to carry a budget — "nine items, and nine is a ceiling" —
and that framing is retired deliberately rather than softened.** It was written
when this tier was a concession wedged in front of the slice and had to justify
existing at all. It is not that any more: `VISION.md` names the engine and its
visual design as **the product**, which makes depth here the thing being built
rather than a detour from it. A ceiling on the pillar is a ceiling on the game.

**What replaces the ceiling is a stronger admission test, not the absence of
one.** Scope discipline is unchanged and still points exactly where
`notes/reality_check.txt` aimed it — at the Ideal Systems wish list in
`VISION.md`, which is still arithmetic this project cannot afford. An item earns
a place in this section by answering **both** of these in its own text, and an
item that cannot is filed in `ENGINEERING_NOTES.md` instead:

1. **What was observed.** Something in the built game that is wrong, missing, or
   reads badly — not an idea someone had, and not a feature another game has.
2. **What it makes possible.** Which later item, or which moment a player would
   actually see, is unavailable until it exists.

That is a higher bar than "this would be cool" and a lower one than "never".
Every item below states both.

**Reference footage cannot admit an item here, and it can specify one.**
Gameplay capture of comparable games is a real input to this project —
[notes/reference_observations.txt](notes/reference_observations.txt) is where
what is seen in it gets written down — but it answers the *second* question and
never the first. It shows what is possible; it says nothing about what is wrong
here, and the failure mode is cheap to walk into: an hour of capture yields
eleven engine items, none of which can name anything wrong with this game. That
is breadth spending wearing engine clothes. **The legitimate use is the opposite
direction.** E5a, E6 and E8 are already admitted on observations from this
project, and what they currently lack is numbers — how far a blast throws
debris, how fast it travels, how long a fire front takes to cross a beam. Those
are guesses today. Reference sets the target for an item that has already earned
its place; it does not create items. The V track answers to this differently and
says so in its own preamble.

The E track's observations, restated because they are the reason the track
exists:

- **Water never levels.** `Grid::can_displace` refuses every upward move unless
  the mover is lighter than its target, and `Empty` has density 0 — so a liquid
  can *never* rise. → **E1**, done.
- **Fire is random, not hot.** Ignition was a per-cell dice roll; nothing in the
  world had a temperature. → **E2**, done.
- **Collapses look stiff.** `drop_component` translated an unsupported piece
  straight down with its shape perfectly intact. Masonry descended like an
  elevator. → **E3**, done.
- **Nothing in the world has a speed.** A grain dug out from under a pile falls
  at exactly the rate of a grain blasted out of it, because movement is a rule
  applied once per step rather than an integration of anything. Nothing can be
  thrown, splashed, sprayed or knocked. → **E10** (rest) and **E5a** (motion)
- **Nothing pushes.** The one verb that changes the world deletes a sphere on a
  cooldown. There is no force in the engine at all. → **E6**
- **The interaction space is nearly empty.** Eight materials besides `Empty` —
  and `Charred` is E9's burning state rather than a thing anyone places — over
  six `REACTIONS` rows, four of which are fire. A sandbox is judged on how many
  of the questions "what happens if I put X on Y" have an answer, and today
  almost none do. → **E7**
- **The player is invisible to the grid.** Sand falls straight through the body
  and the unstuck search cleans up afterwards. → **E4**

**Order is E → V → P, and both arrows are load-bearing.** *E before V* because E
items change what materials mean and what is on screen, and tuning a palette or
authoring a sprite for a world that is about to gain thrown debris and
explosions means authoring it twice — the same argument that put E2 ahead of V2,
which held. *P after both* because P1 is a memory-layout change and the layout
cannot be settled before the field set is. **The sharp version of that argument
is retired and the ordering survives it.** It used to read "`Element` has no
padding left, so E5 is the first item whose data genuinely costs memory" — which
was true of the old E5 and is not true of what replaced it: E10 and E5a claim
the already-present `Element::ticks`, so the struct stays at 12 bytes and
nothing in the E track now grows the cell array at all. P still runs after E and
V, for the weaker but sufficient reason that P1 wants to know what the hot loop
actually reads before deciding what to split out of it, and E5a changes that.
**P2 is the exception and now runs first of everything** — see the P track's own
preamble.

**Every E and V item carries its own bracketed measurement** per
[PERFORMANCE.md](PERFORMANCE.md), and the standing rule stands: if one item
alone breaks the frame budget, P1 gets pulled forward ahead of the rest. That
rule was invoked once, at E2, and correctly *not* triggered — the escape hatch
turned out to be a one-line early-out. Read that entry before reaching for it
again.

**P2 put this rule in a state it was not written for, and it is an open decision
rather than a thing to reflex on** — see [Decisions
owed](ROADMAP.md#-decisions-owed). The rule triggers on *an item* pushing
the budget over. P2 is not an item; it is a correction to the instrument, and it
found two scenarios already over budget at the played size (`churning` 211%,
`cascading` 241%) with nothing having got slower. Read literally the rule does
not fire, and that reading is defensible — the two breaching scenarios are the
synthetic ones this file already says the game does not produce, while `sparse`,
which stands in for a real frame, is unchanged at 1.00x. But "the rule
technically does not fire" is the same move as the stale comment P2 just
deleted, so it is written down as a decision with a due date instead of being
settled by whoever reads it next.

**Item IDs are stable and are not renumbered when the order changes.**
`PERFORMANCE.md`, `ENGINEERING_NOTES.md`, `VISION.md` and
`notes/art_direction.txt` all cite items in this section by name; renumbering to
make the list read top-to-bottom would silently falsify four other documents to
save one line of explanation. Where the running order differs from the numbering
it is stated in the track's own preamble.

### The finding that reorganised this track

Four rules in `grid.cpp` were each added to fix a real, visible artifact. Each
is correct. Each has the same shape:

| Rule | What it costs *in principle* | `churning` | played session |
|---|---|---|---|
| `vent_fluid` | a 7x7 box scan, per powder-touching-fluid, per tick | **47.1%** | 0.1% |
| `find_lower_surface` | a search of up to `MAX_PRESSURE_CELLS`, per awake surface cell, per tick | 0.3% | **7.3%** |
| `make_room_above` | a walk up to `MAX_DISPLACE_RISE` cells, per painted cell | — *(cannot fire)* | 0.5% |
| `fall_if_unsupported` | a flood fill of up to `MAX_SUPPORT_CELLS`, up to 8 times per tick | not ablatable | not ablatable |
| **all three ablatable together** | **the whole of what E5b retires** | **47.5%** | **8.3%** |

**The last two columns were added 2026-08-13 and they change what this table
says.** Written from the code, every row here looks alarming — and measured by
ablation, **the scariest-looking row costs `churning` nothing, and the two
scenarios do not agree on which rule is expensive.** `find_lower_surface`'s "up
to 512 cells per awake surface cell per tick" is 0.3% of `churning`, which is
the noise floor of that table; venting is 47% there and 0.1% in play. **A cost
written from reading the code is a worst case per invocation, and says nothing
about how often the invocation happens** — which is the whole distance between
this table's old form and its new one. The line numbers this table used to carry
(`grid.cpp:1020` and three others) were removed at the same time: they had gone
stale, which is the failure `.claude/rules/documentation.md` forbids line-number
references to prevent.

Each has a magic radius that was picked by sweeping values and measuring. **One
of those sweeps has since been shown to be an artifact** — `VENT_RADIUS`'s knee
was a property of measuring one build per data point, and the real cost curve is
flat (PERFORMANCE.md). Treat the others as unverified by the same standard until
they are re-run in one binary. **The engine answers "where should this go?" by
looking around, instead of by carrying state that already knows.** That is the
difference between this engine and the ones it is measured against: Noita, The
Powder Toy and Sandspiel each carry two things this one does not — **a speed on
every cell** and **a coarse air/pressure field** — and nearly every entry in
that table is a symptom of one of them being missing. E10, E5a and E5b are those
two things, and between them they retire three of the four rows.

### E — Simulation depth

Running order: **~~E4~~ → ~~E9-steam~~ → E10 → E12 → E5a → P1 → E6 → E7 + E11 →
E5b → E8.** E4 closed "no" on 2026-08-10. `P1` is inside this list rather than
after it as of 2026-08-11 — see the P track. **E12 is a dependency of E6, not
merely earlier than it**, for the reason written in both entries: an explosion
cannot hand an impulse to a structural cell, so granulating is the only route by
which a solid becomes flying matter.

*Running order: **E4 → E10 → E12 → E5a → E6 → E7 + E11 → E5b → E8**, with `S0`
between E4 and E10 out of [Medium Term](#-medium-term-core-gameplay-loop) and
`P2` ahead of all of it. **E12 is new on 2026-08-11 and sits after E10 for a
reason that is specific rather than positional** — a crumbling material that
cannot hold a slope reads as a liquid, so built before powders have a rest state
its entire output is a puddle. Changed 2026-08-09 from `E4 → E5 → E6 → E7 → E8`;
the argument is in ["Where this stands"](#-where-this-stands) and the sizes are
in [ROADMAP_ITEMS.md](ROADMAP.md). E4 is still first because it is the
oldest open question in the project and may close as "no" without any code at
all. E10 is next because it is days of work for the largest single improvement
in how the simulation feels, and because it settles the representation E5a then
fills in. E5a is the axis the three after it are built on.*

***The finding that reorganised this track, because four of the items below are
answers to it.*** *A review of `grid.cpp` on 2026-08-09 found four rules —
`vent_fluid`, `make_room_above`, `find_lower_surface` and `fall_if_unsupported`
— each added to fix a real observed artifact, each correct, and each the same
shape: a bounded search of the neighbourhood, with a magic radius picked by
sweeping values and measuring. That is the diagnosis, and it is architectural
rather than a code-quality problem: **the engine answers "where should this go?"
by looking around, rather than by carrying state that already knows.** The
engines this project is measured against — Noita, The Powder Toy, Sandspiel —
each carry two things this one does not: a velocity on every cell, and a coarse
air/pressure field. Nearly every one of those four rules is a symptom of one of
the two being absent. E10 and E5a are the first; E5b is the second, and it
retires three of the four rules outright. The fourth, `fall_if_unsupported`, is
retired by E8. **This is the strongest form the admission test takes** — every
one of those four rules is a thing observed in the built game, and each names
what it makes possible.*

- [x] **E1 — Liquids find their level.** *(done — see
  [Shipped](ROADMAP_ARCHIVE.md#e1e3-simulation-depth))*
- [x] **E2 — Heat, the seventh axis.** *(done — see
  [Shipped](ROADMAP_ARCHIVE.md#e1e3-simulation-depth))*
- [x] **E3 — Collapses break instead of dropping rigid.** *(done — see
  [Shipped](ROADMAP_ARCHIVE.md#e1e3-simulation-depth))*

- [ ] **E4 — The player displaces material, or deliberately does not.**
  *Observed:* the grid does not know the player exists, so material falls
  straight through the body and the unstuck search is what stops that becoming a
  freeze. *Unlocks:* nothing else in this document — which is precisely why it
  is cheap and why it goes first. **This item's deliverable is an answer, not
  necessarily a feature, and it should be settled by playing rather than by
  argument.** If the artifact is obvious in practice, do it; if it is not, close
  it as "no" in `ENGINEERING_NOTES.md` and stop paying attention to it.
    - **If the answer is yes, the implementation waits for E5a, and that is the
      one thing this item has learned since it was written.** The hard part was
      never detecting the overlap, it is that shoving cells aside must not
      create or destroy matter, and the obvious cheat — stamping the body into
      the grid as a temporary solid each step — either deletes what was already
      there or needs a full displacement pass of its own. E5a gives displaced
      matter somewhere to *go*: a cell the body walks into is **handed the
      body's velocity and stays in the grid**, which is a shove rather than a
      deletion and conserves for free, since nothing left the cell array to be
      conserved separately. *(This bullet used to say the cell "becomes a free
      particle", which was the old E5 design; the shove is simpler under the
      current one.)* Deciding this before E5a and building it after is the
      correct split, not a stall.
    - **Keep the direction of the dependency**, which is the rule `tool.cpp`
      established: the grid does not know about bodies, bodies read the grid.
      Displacement is the player *asking* what it is standing in and then
      writing through the ordinary write path, not a body pointer on `Grid`.

    > **In plain terms.** *(afternoon — a decision, possibly no code)* The
    > grid doesn't know the player exists, so sand falls straight through
    > the body. This item's output is a *decision*: try it in play, and if
    > it isn't obviously better, write down "no" and stop thinking about it.
    > If the answer is yes, the implementation waits for E5a, which is what
    > gives shoved material somewhere to go.

- [ ] **E10 — Powders come to rest.** *Observed:* `step_powder` rolls a grain
  into any free diagonal and then takes a second fall in the same step, so there
  is no static friction anywhere in the engine. Sand behaves as a very thin
  liquid: piles cannot hold a slope, and a tunnel dug through a dune flattens
  completely rather than partly caving in. **A7, A7b and A7c are the same
  observation arrived at three times** — the comment at `grid.cpp:806` records
  two rules tried and abandoned and reads, in full, "a rule aimed at motion kept
  catching rest, and a rule that spared rest stopped catching the defect". That
  is an exact description of a system with no rest *state* for a rule to aim at,
  and the third rule that shipped works by removing the intermediate moment
  rather than by modelling the missing thing. *Unlocks:* the whole powder half
  of how this engine feels, four E7 rows that are currently indistinguishable
  from sand (gravel, snow, ash, gunpowder), and the representation E5a needs.
    - **The mechanism is one number per material — inertial resistance in the
      engine this is measured against.** A settled grain is at rest and stays at
      rest until disturbed: a neighbour moving is what proposes the change, and
      a per-material roll is what decides it. Once free, a grain keeps sliding
      until it comes to rest again. Cones hold their angle, avalanches trigger
      and then *stop*, and the difference between sand and gravel becomes a
      column rather than a code path. **It is deliberately not a friction
      coefficient or an angle in degrees** — a probability per material is the
      cheapest thing that produces the behaviour and it composes with the
      deterministic hash for free, the same way the powder direction pick
      already does.
    - **It costs no memory — still true after the sitting, and now for a checked
      reason rather than the one written here.** *(2026-08-13: the claim below
      rests on `ticks` being free for powders, and E10 no longer uses `ticks` at
      all. It costs no memory because the per-cell state it needs is a rest bit
      it reads out of E5a's velocity, and because that velocity itself fits in
      three bytes `Element` already had. The measurement is in PERFORMANCE.md;
      the original argument is kept because it is still a correct description of
      what `ticks` is doing.)* `Element::ticks` is read or written in exactly
      six places, all of them either support resolution (structural cells) or
      `step_fire`. `element.h` states the position outright — "Zero for
      everything else. Powders and fluids move one cell per step by their own
      rules and have no use for a clock." That is a whole byte already in the
      struct, already carried by `swap_elements`, already zeroed by `place()`,
      sitting unused on precisely the class of material that needs it. The
      `static_assert` at `element.h:109` does not move and the struct stays at
      12 bytes.
    - ~~**It claims that byte permanently, so the meaning is decided here rather
      than twice.**~~ **Withdrawn 2026-08-13 by the instrumentation sitting. E10
      does not claim `Element::ticks` and neither does E5a.** The bullet read:
      *for a non-structural, non-`Fire` cell the byte becomes a packed velocity
      — four bits of `vx` and four of `vy`, both signed, giving −8..+7 cells per
      step on each axis … the range is not arbitrary: `MAX_FALL_SPEED` for
      structural pieces is already 8, so the two speed limits in the engine
      agree by construction.* Two things were wrong with it and the second is
      the useful one.
        - **Four bits of whole cells per step cannot hold an acceleration.** One
          step of `Player::GRAVITY` is 5/36 of a cell per step, which truncates
          to zero in an integer, every step, forever. `velocity_probe` flies the
          representation and the grain never comes back down inside 600 steps.
          The plan review predicted this from the desk in 2026-08-11 and was
          right; the sitting's contribution is that it is now measured rather
          than argued, and that the same probe rules out the two cheap rescues —
          a stochastic gravity increment has the correct mean and a
          **156%-of-the-mean spread** across grains given an identical impulse,
          and a truncated fixed-point increment makes gravity permanently 10%
          light.
        - **The byte was never the constraint.** `Element` has three unused
          bytes at offsets 1–3, in the alignment hole between `type` and
          `color`, and it always has. Velocity goes there — signed 4.4 per axis
          plus a nibble per axis of sub-cell remainder — and `ticks` keeps
          exactly the two roles it already has. **The instructive part is that
          this bullet's reasoning was good and its premise was never checked**:
          it is a careful argument about how to subdivide one byte, written by
          people who had `sizeof` available and used arithmetic instead.
          `element.h` carries the correction next to the claim.
    - **What E10 actually needs from any of this is one bit, and it survives
      unchanged.** "Is this grain moving" is `vel != 0` under the new
      representation exactly as it was "is `ticks` non-zero" under the old one,
      so E10's design is untouched by the reversal — it reads a rest state, and
      where the rest state is stored was never E10's question.
      **`MAX_FALL_SPEED` still sets the ceiling** and the two speed limits still
      agree by construction; 4.4 signed tops out at 7.9375 cells per step
      against structural material's 8, which is the same agreement with a
      fraction under it.
    - **`element.h`'s third role for this byte needs the same treatment the
      first two got.** The struct already carries a `static_assert` forbidding
      `Fire` from being structural, because `ticks` means two things. This makes
      it three, and the invariant to assert is the same shape: a material that
      is `structural` or is `Fire` may not have an inertial-resistance value,
      and a material that has one may not be either. Write it next to the
      existing one. The correctness pass's rule is that a data-driven design's
      danger lives in the relationships between rows, and this is the third time
      that has been true in this file.
    - *Verify.* A poured pile holds a measurable angle rather than flattening,
      asserted as a number so it can regress. A tunnel roof partially collapses.
      A disturbed pile settles and the chunk goes back to sleep. And the
      benchmark does not regress on `cascading` or `churning` — a resting grain
      now does strictly *less* work than it did, so a regression means the
      disturbance propagation is waking cells it should not.

    > **In plain terms.** *(days — new, and the biggest single improvement
    > to how the game feels per hour spent)* Sand currently has no friction
    > at all: a grain rolls off any edge it can and then takes a second fall
    > in the same tick, so piles can't hold a slope, sand behaves like very
    > thin water, and a tunnel dug through a dune flattens completely
    > instead of partly caving in. This is also why the three failed rules
    > recorded at A7/A7b/A7c fought each other — "a rule aimed at motion
    > kept catching rest, and a rule that spared rest stopped catching the
    > defect" is an exact description of a system with no *rest state* to
    > aim at.

- [ ] **E5a — Velocity means something.** *(the first half of what was E5)*
  *Observed:* nothing in the world has a speed. Powders and liquids move one
  cell per step by rule; only rigid pieces accelerate, and they do it by falling
  repeatedly rather than faster. A grain dug out from under a pile travels at
  exactly the rate of a grain blasted out of it. *Unlocks:* E6 (an explosion
  with nothing to throw is a hole), E4's shove, E7's gunpowder, and V9's debris.
    - **It lives on the cell, in the grid, and that reverses what this item used
      to say.** The previous design was a sparse list of free particles that
      *leave* the grid and re-enter it on landing, chosen because `element.h`
      says there is no free byte. The byte turned out to exist (see E10), and
      the dual representation was the more expensive half of that decision
      anyway: a particle outside the grid needs an explicit answer to every rule
      in the engine — does it conduct heat, does it react, does `LightField` see
      it, does it occlude — and each answer is either a second implementation or
      a documented "no" that will be wrong later. **The engines this is measured
      against keep the moving cell in the grid** and walk it along a straight
      line through the cells it crosses each step, testing each one. One entity,
      one set of rules, no boundary to maintain. *A genuinely separate particle
      list is still right for matter that must move between cells and interact
      with nothing* — which is exactly V9's effects layer, already scoped as
      non-simulated, and that is where the idea belongs.
    - **The removal note at `grid.cpp:743` argues against the wrong thing, and
      the fix is to edit it rather than delete it.** It records that powder
      acceleration was tried and removed for three measured reasons: motion got
      choppier, a continuously fed stream stratified into sheets one cell apart,
      and `cascading` went 13.1 → 19.7 ms/step with awake chunks going 105/135 →
      135/135. The measurements are good and the conclusion drawn from them is
      too broad. All three are consequences of applying free-fall acceleration
      to *every falling grain*, and all three go away when velocity is non-zero
      only because something **put** it there:
        - *Stratification* was caused by `place()` resetting `ticks`, so the
          brush stamped speed-1 grains on top of speed-2 ones every step. Under
          this design a brush-stamped grain starts at zero **and so do its
          neighbours**, because gravity only accumulates on a cell that is
          already moving. The mechanism that produced the sheets is absent
          rather than tuned.
        - *The frame-time regression* was the whole world gaining a per-cell
          clock. Here a resting grain is still a one-cell-per-step mover doing
          exactly what it does today, so the common case is unchanged — and E10
          makes the resting case *cheaper* than it currently is.
        - *Choppiness* was always a property of drawing whole cells on a fixed
          tick and cannot be fixed in the simulation at all, which the note
          itself says correctly. It is not an argument about velocity.
    - **No new memory, and as of 2026-08-13 that is measured rather than claimed
      — but it is not E10's byte.** *(The bullet read: "It is E10's byte, now
      carrying real values. `Element` stays at 12." The second sentence holds;
      the first does not.)* The representation is settled and it is **three
      bytes in the alignment hole between `type` and `color`**: `int8_t vel_x`,
      `int8_t vel_y` as signed 4.4, and one `uint8_t` holding a nibble of
      sub-cell remainder per axis. `sizeof(Element)` stays 12 with those fields
      present, and `grid_bench` with them present is inside its noise band on
      every row.
        - **The sub-cell remainder is the half of the requirement the plan never
          wrote down**, and it is why one byte was never going to be enough
          regardless of how it was divided. A cell's position is a cell index. A
          velocity finer than one cell per step therefore needs somewhere to
          keep the part of a cell that has been crossed and not completed —
          which is precisely what `Player` carries `rem_x` alongside `vel_x`
          for, and has since before F5. Every version of this decision,
          including the two that argued for spending a second byte, budgeted for
          velocity alone.
        - **Gravity is applied by differencing a running total taken off the
          global step counter**, which is Bresenham's line algorithm pointed at
          an acceleration. 5/36 of a cell is 2.222 sixteenths; a truncated
          increment of 2 makes gravity 10% light on everything thrown,
          permanently and in one direction, which the probe measures as a range
          of 252 cells against the reference's 228. Differencing `floor(n * 20 /
          9)` gives an increment alternating 2,2,2,2,3 whose mean is exactly
          right, and it needs no per-cell accumulator because the step number is
          state the engine already has. **Flown at all nine phases** — since a
          cell launched on an arbitrary step starts at an arbitrary phase — the
          spread is 4 cells in 228 and the worst deviation from the `fx` 16.16
          reference is **2 cells**.
        - **What was rejected, with the number that rejected it.** A whole-cell
          integer velocity with gravity applied stochastically at 5/36 per step
          is deterministic, costs no storage, and is the project's own idiom —
          and 64 grains handed an identical impulse land across a **364-cell
          spread on a 233-cell mean**. An explosion under it would not throw
          debris, it would scatter it. Recorded because the idea is a good one
          that fails on a measurement rather than on an argument, and it will be
          proposed again.
    - **Integer arithmetic, one cell at a time.** Both are already in the
      project and both are load-bearing. Movement resolves one cell per axis per
      iteration, exactly as `Player` does it, so tunnelling is impossible by
      construction rather than by being fast enough. No floating point anywhere,
      because F1 spent seven steps making `Grid` a pure function of its seed and
      F1.7 wrote that down as an invariant.
    - **Four traps, each a known failure mode of something this engine already
      does.** *Conservation* — a moving cell is still **in** the grid, so the
      existing conservation test keeps working unchanged, and that is a real
      advantage of this design over the previous one; say so in the test rather
      than leaving it as luck. *The wake rule* — a cell arriving in a sleeping
      chunk wakes it, the same as every other write, and a cell that still has
      velocity must keep its own chunk awake or it freezes mid-flight, which is
      the same bug as the boxed-in `Fire` cell. *One definition of solid* — what
      stops a moving cell is `is_solid`, the same function the player collides
      against and the dig ray stops on. *A ceiling* — the four-bit range is the
      ceiling, and unlike a list bound it needs no overflow policy.
    - *Verify.* Same seed and same input produces byte-identical results, which
      is F1's invariant extended to the new axis rather than assumed to survive
      it. A cell fired at a wall at full speed lands *against* it (the
      anti-tunnelling case, the same shape as the dig ray's). A world that is
      disturbed and then settles goes fully back to sleep. And conservation,
      which for once needs no new machinery.

    > **In plain terms.** *(weeks — the first half of the old E5)* Nothing
    > in the world has a speed. A grain dug out from under a pile falls at
    > exactly the rate of a grain blasted out of it, because movement is a
    > rule applied once per tick rather than a speed being integrated.
    > Nothing can be thrown, splashed, sprayed or knocked. Three later items
    > stand on this.

- [ ] **E6 — Explosions.** *Observed:* the only verb that changes the world is a
  dig that deletes a fixed sphere on a cooldown. Nothing in the game applies
  force. *Unlocks:* the moment that shows every axis at once — heat (E2),
  fracture (E3), thrown debris (E5a) — and it is the single most legible thing
  this engine can put on a screen.
    - **This is also a candidate answer to the question `VISION.md` leaves open
      on purpose**, which is what the hook finally is. That document is explicit
      that the slice as specified — walking, jumping, and one dig tool — may be
      too thin to be fun, that under-building looks exactly like discipline
      until the playtest gate, and that naming the hook early would quietly
      commit the design to whichever comparison got written down. This item does
      not name it either. What it does is make the question *answerable by
      playing*, which is the same move E4 makes and the only honest way this
      document has ever settled a design question.
    - **Built as** a radius, a falloff, a heat deposit, a conversion pass and an
      impulse handed to E5a — in that order, because each stage is one of the
      existing axes and none of them is new code. Everything it writes goes
      through `set_element` / `paint` / `swap_elements`. A radial write that
      touches `cells` directly will produce material frozen in mid-air, and the
      tests that catch it are the chunk tests in `test_grid.cpp`.
    - **It must not become a second destruction system, and this is the trap
      most likely to be walked into.** The dig tool is a degenerate explosion —
      a radius with no impulse, no heat and no falloff. If explosions ship as a
      parallel implementation there will be two answers to "what does
      destruction do to a structure", they will drift, and the drift will
      present as a bug in fracture. The same argument that made `set_element`
      and `paint` share one private `place()` applies here and is stronger,
      because the two paths are further apart.
    - **The cost is the first in this engine whose worst case a player sets, and
      that is a real change in kind.** Every existing worst case is a property
      of the world; this one is a property of what someone chooses to do in it.
      Cost goes as radius squared, and an unbounded radius is an unbounded
      frame. It needs a ceiling, and the measurement has to be taken at the
      ceiling rather than at a typical value, per
      [PERFORMANCE.md](PERFORMANCE.md) — and the benchmark has to be shown to
      *reach* it, which is the rule E3 wrote into that file after a perfectly
      executed measurement of nothing.

    > **In plain terms.** *(week — nearly free once E5a lands)* Right now
    > the only way to change the world is a dig that deletes a fixed sphere
    > on a cooldown. There is no force in the engine at all. An explosion is
    > a radius, a falloff, a heat deposit, a conversion pass and an impulse
    > handed to E5a — five stages, four of which are axes that already
    > exist. It is the single most impressive thing this engine can put on a
    > screen, and it is one of the two things that make the hook question
    > answerable by playing.

- [ ] **E7 — Breadth: more rows, not more code.** *Observed:* eight materials
  besides `Empty`, only seven of them placeable (`Charred` is E9's burning
  state), over six `REACTIONS` rows, four of which are about fire. The whole
  interaction space a player can probe is fire/wood, fire/oil, fire/water and
  water/steam. *Unlocks:* nothing structural, and that is the point — it is the
  cheapest depth available in this engine precisely because E2 already made
  transformation a temperature-gated table rather than a branch.
    - **Melting and freezing need no new mechanism at all**, which is the
      strongest evidence E2 was the right axis to spend. Boiling is already a
      row with a `min_temp`; melting is the same row shape pointed the other
      way, and freezing is the `max_temp` half that Steam→Water already uses.
      Stone → molten → cooled stone is three rows and no engine change.
    - **Candidates, each one row or a small handful:** `Snow` (V4's row, and
      with heat in the engine it melts by table rather than by special case),
      `Ice`, molten stone, `Acid`, `Gunpowder` (which is E6 wearing a material),
      and smoke as a thing distinct from steam. None of them is scheduled here —
      this item is the budget and the test, not the list.
    - **This is the item most likely to become an endless table, and the bound
      is stated rather than hoped for.** A row earns its place by making an
      interaction *legible* to a player — someone has to be able to discover it
      and be right about what they discovered. Breadth is also the cheapest
      possible way to *look* deep, which is exactly why it is sequenced after
      E5a and E6 rather than before: rows authored before matter can be thrown
      and before anything explodes would be authored against half an engine and
      revisited.
    - **Watch for the hardness signal.** `ENGINEERING_NOTES.md` records that
      `MATERIALS` has no hardness column and that adding one would be an axis
      with no consumer. If this item's rows start reaching for a per-material
      strength number, that is the signal that entry has been waiting for.

    > **In plain terms.** *(days per material, ongoing)* There are only
    > eight materials and six interactions, so "what happens if I put X on
    > Y" almost never has an answer. Ice, snow, acid, gunpowder, molten
    > stone and smoke are mostly new table rows rather than new code, thanks
    > to E2. Sequenced after E6 so rows aren't authored against half an
    > engine. **The bound:** a row earns its place by making an interaction
    > something a player can discover and be right about.

- [ ] **E11 — The columns heat and fluids are missing.** *Observed:* four gaps
  found reading `material.h` against what the tables are being asked to express.
  *Unlocks:* nothing structural, and none of the four justifies an item alone —
  they are grouped because each is one column or one short rule, and because
  three of the four become load-bearing the moment E7 starts adding rows.
  Sequenced with E7 for that reason.
    - **`conductivity` is doing two jobs and one of them is heat capacity.** Its
      comment says so plainly — "one number sets both how fast a material heats
      and how fast it forgets" — which is a modelling shortcut, correctly taken
      when heat was new and there was one consumer. It means water cannot be a
      heat sink and metal cannot be a fast conductor that *stays* hot, because
      those two behaviours differ only in the number this column is standing in
      for. One extra column separates them, and E7's molten stone and ice both
      want it.
    - **Heat has no reach.** Conduction is between touching cells only, so
      standing next to a bonfire is thermally free. That is a deliberate and
      correct simplification — `material.h` explains that not simulating air is
      what makes the thermal pass affordable — and it stops being harmless at
      `S0`, where fire becomes a hazard and a hazard you can stand beside is a
      decoration. The cheap version is a short-range term from `heat_source`
      cells only, which is a handful of cells in any real scene, rather than a
      general radiative pass over the world.
    - **Fluids have one number.** `spread` (5 for water, 3 for oil) is the whole
      of what distinguishes one liquid from another in motion, so honey, tar and
      lava are the same substance at different settings. A viscosity term — how
      often a cell is willing to spend its lateral move at all — is one column
      and one early-out.
    - **Lateral flow is a jump, and "no action is proposed" is withdrawn.**
      `step_fluid`'s lateral run walks outward as far as `spread` allows and
      then `swap_elements` moves the cell to the furthest usable landing in one
      step, so a liquid crosses several cells while occupying none of them. This
      is standard for the genre and is *why* streams read as snapping rather
      than pouring. The entry used to end there, on the reasoning that recording
      it as a known property stops someone hunting it as a bug, and that if it
      were ever addressed it would be E5b's business rather than a column's.
      **Session 5 refuted the "no action" half twice in one sitting, and the
      second time under a check written in advance to discriminate.** The
      general checklist returned *"water/oil should be reworked completely, it
      does not flow properly and is bugged"*; then W-5 returned *"still
      staggered clumps"* after W-3 had confirmed the elevator was gone — which
      is verbatim the case the [session 5
      checklist](PLAYTEST_LOG.md#session-5-checklist--the-pass-that-closes-wave-3)
      named ahead of time as *"a new finding about the flow model rather than
      about venting."* **The property is unchanged; the classification was
      wrong.** A known artefact of the genre and the largest visual complaint on
      the record against a material the player sees constantly are not the same
      entry, and only the first of those can be closed by writing it down.
        - **What is not withdrawn is where the fix lives.** It is still not a
          column, and E5b is still the candidate mechanism — pressure
          propagating over several ticks is what makes a *continuous* lateral
          move natural rather than bolted on. But E5b is *large* and sits after
          the slice, and one playtest note is not a reason to pull a month of
          work in front of `S0`; that is the exact pattern the 2026-08-09
          reorder exists to stop. **What replaces "no action" is a bounded
          spike, and its output is a decision rather than a fix:** build an
          instrument that puts a number on the staggering — the `preview_light`
          move, because "still staggered clumps" has no before or after and no
          fluid change can currently be judged — then price the cheap
          alternative against it, which is a lateral move that walks one cell
          per step instead of teleporting. If that reads as flow, most of the
          look is bought for a day. If it does not, or if it costs too much on
          `churning` (already 211% of a frame at the played size), E5b is priced
          with evidence instead of with argument. **Either result closes the
          loop; neither is a rework, and the spike is not a licence to start
          one.**
        - **The residual lift belongs to the same spike and is a separate
          finding.** Session 5's W-3 confirmed A6b's headline symptom gone and
          then reported water still climbing a standing sand column. **That is a
          residual to eliminate rather than a rate to tune** — displacing sand
          must raise a pool's free *surface*, and no configuration makes it
          right for water to occupy a column above that surface. Wave 3 kept
          `vent_fluid`'s straight-swap fallback for a grain deep inside a body
          on the grounds that *"there is no conveyor above it, so its one-cell
          lift never adds up"*; in the played configuration it adds up, so that
          sentence is wrong in the same way this bullet's was — a property
          argued harmless and then seen. The invariant is assertable, which is
          what takes it out of the realm of looks: **no water cell may come to
          rest above the pool's free surface**, splash excepted. `water_probe`
          already measures the quantity, and 3 cells at step 350 was judged
          acceptable on paper and is visible at 3440x1440.

    > **In plain terms.** *(days — new)* Four small gaps found reading the
    > tables, grouped because they are all one column or one short rule and
    > none of them justifies its own item.

- [ ] **E12 — `Crust` and `Grit`: a material that granulates when damaged.**
  *(new 2026-08-11; the design is
  [notes/granulating_enemies.md](notes/granulating_enemies.md) Part A, path M2,
  and is not repeated here)* *Observed:* **nothing in play is wrong for want of
  this, and the note says so in its own closing section.** What changed is that
  an enemy whose body granulates locally when damaged has been committed to, and
  this is the half of that idea that can be built, played and judged **with no
  actor code at all**. *Unlocks:* `S1`; and before `S1`, crumbling terrain as a
  hazard — a crust ceiling that comes down — which is the cheapest possible way
  to find out whether the feel is worth what `S1` costs.
    - **Admitted the same way V15 is: on a condition the note wrote down before
      the thing that satisfies it existed.** That note is explicit that the
      material is *"weak on the first question"* and that it *"rides along with
      whatever admits combat."* This is that. **The honest framing is that E12
      is admitted by a commitment and V15 by a trigger, and neither is admitted
      by an observation** — both are recorded that way rather than dressed up,
      because the tier's admission test is worth more when the exceptions are
      visible.
    - **Two `MATERIALS` rows, not one row with a mode flag**, which the note
      settles and which is the `Wood` → `Charred` precedent exactly: `Crust` is
      `MoveKind::Static` and structural, `Grit` is a `MoveKind::Powder` at
      sand-like density. `is_solid()` and `is_structural()` are derived from
      `MoveKind`, so `Crust` gets player collision and rigid collapse and `Grit`
      gets piling **with no new branch in the update loop**. A mode flag would
      need a per-cell bit, which `element.h` says outright is the byte that
      costs 500 KB.
    - **The trigger is a `Grid::granulate(x, y)` called from three sites that
      already exist**, each reading as something different:
      `Tool::update`/`march` (struck it and it crumbled where you hit it),
      `fall_if_unsupported` (an overhang comes loose and *becomes* a sand-fall),
      and `fracture_landing` (holds shape through the fall, shatters on impact).
      The third is the cheapest and the best of the three, because it already
      means "this piece just hit something hard" and already has the component
      flood-filled.
    - **A roll per disturbed cell rather than a certainty**, with its own
      `Stream` tag registered in `SIM_STREAMS` — some cells hold and some go,
      which buys a ragged crumble edge for free. **Read `reaction.h`'s jitter
      entry before tuning it**, because that entry records the version that was
      tried, measured, and did not work: jittering the *timing* bought far less
      shape per unit than jittering the *threshold*, and this is the same shape
      of knob.
    - **`REACTIONS` cannot express this and ruling that out in writing is worth
      a line**, because it is the first thing the engine's shape suggests. A
      reaction row is gated on neighbour type and a temperature window; "was
      hit" is neither. `Crust + Fire → Grit` is expressible and may even be
      good, but it is a different behaviour and not this one. **Reactions are
      the wrong axis, not an insufficient one.**
    - **Sequenced after E10 for a reason that is specific rather than tidy:** a
      crumbling crust that cannot hold a slope reads as a liquid, so built
      before powders have a rest state this item's whole output is a puddle. The
      note says this and it is the sharpest sequencing claim either document
      makes.
    - **The accumulated-damage version (M3) is not this item and is after P1.**
      Per-cell integrity buys visible wear and "three hits and it goes", and it
      costs the 500 KB byte at today's prices. **P1 is the item that makes it
      affordable**, and building M3 first buys the same feature at its worst
      price. It is also the first thing that genuinely fires the hardness signal
      `ENGINEERING_NOTES.md` has been holding — **M2 does not**, and conflating
      the two is how that entry gets opened a version early.
    - *Verify.* Headless, entirely: a struck `Crust` cell becomes `Grit` at the
      impact point and its neighbours mostly do not; an unsupported crust
      overhang granulates into a pile rather than dropping as a slab; a crust
      slab dropped from height shatters on landing; conservation is unchanged,
      because granulation is a type change and not a write of new matter.

    > **In plain terms.** *(days — new 2026-08-11)* A solid that holds its
    > shape until it is disturbed, at which point the damaged part turns to
    > sand and pours away. **Two table rows, not one row with a mode flag**
    > — exactly the `Wood` → `Charred` precedent, so the solid gets player
    > collision and rigid collapse and the powder gets piling, with no new
    > branch in the update loop. The trigger is one small `granulate()`
    > called from three places that already exist: the dig tool's impact, an
    > overhang losing its support, and a piece landing hard. A **dice roll
    > per disturbed cell** rather than a certainty, so some cells hold and
    > some go and the crumble edge is ragged for free.

- [ ] **E8 — Toppling.** *Observed:* E3 named this as the follow-on it existed
  to make affordable, and left it undone deliberately — a fragment that fracture
  has separated is the cheap candidate for it. **Playtest session 1 observed it
  independently and from the other end:** bodies fall flat and land flat, and
  the result reads as lifeless (observation B4). That is the same gap arrived at
  by watching rather than by reasoning, which is the strongest form the
  admission test takes — and it is the worked example of a playtest request
  earning a *tracked item* rather than a wave, per the admission rule in
  [Waves](ROADMAP_ARCHIVE.md#-waves--sub-plans-that-preempt-the-tracks). It was for a while
  carried in both places at once, which is the duplication that consolidation
  removed. *Unlocks:* the last of the three ways masonry can fail; today it
  drops or it breaks, and it never tips.
    - **Toppling and rolling are two items, and this one is only the first.**
      Session 1 asked for "tip, topple, and roll" as a single feature. They are
      separated by an engine boundary rather than by degree: a whole-cell pivot
      is a permutation of a piece's own cells, and a body that *rolls* has left
      the grid — it carries a real transform, is re-rasterised every frame,
      changes cell count under rotation, and needs a resolution pass for the
      overlaps. **Rolling is the body-extraction route described in the bullet
      below**, and it is deferred on cost. *(This used to say rolling was
      deferred behind E5 on the grounds that free particles are the natural
      substrate for off-grid matter. That was reasoning about the old E5 design;
      E5a keeps matter in the grid, so it is not a substrate for anything and
      rolling no longer depends on it. The dependency is the solver, not the
      particle layer.)*
    - **E3's argument was believed to be a hard constraint and it is not, and
      that correction is the main thing that changed here on 2026-08-09.** The
      argument ran: true rigid-body rotation on a cell grid means resampling the
      piece every step it turns, which destroys the exact authored pixels
      [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md) calls the entire visual
      pillar — so the feature would be bought by breaking the reason the engine
      is interesting. **The premise is that the piece has to live in the grid
      while it rotates, and the reference implementation simply does not do
      that.** Noita's route: trace the connected region's outline with marching
      squares, simplify the contour, triangulate it, hand *that* to a rigid-body
      solver, **remove the piece's cells from the grid entirely**, and stamp
      them back each frame from the body's own private pixel buffer. The
      authored pixels are never resampled — they live with the body and are only
      *drawn* rotated, which makes rotation a rendering problem, and rendering
      rotated pixel art is solved. **Explosions and fracture then act on bodies
      properly** rather than on a flood fill, and the eight-flood-fills-per-step
      cost in `resolve_support` goes away with them.
    - **So the objection is recorded as the real one, which is cost and
      dependency rather than possibility.** It is larger than the rest of the E
      track combined; it wants either a physics library (a dependency, against
      the no-bloat rule in `VISION.md`) or a hand-rolled solver, and that choice
      is itself an item; and nothing in the slice needs it. **It is deferred
      past v0.1 on those grounds and not on impossibility.** The previous text
      said it "may close as 'not possible without wrecking the pixel art'", and
      **that sentence is withdrawn rather than softened**: it is the kind of
      line that a reader a year from now takes as settled, and it would have
      closed a door that the engines this project is measured against walk
      through routinely. A deferral states its price; it does not claim the
      thing cannot be done.
    - **The cheap route is still worth trying first and is the one that fits
      before E5b.** A small piece pivots in whole-cell steps, so the move is a
      *permutation* of its own cells — every colour relocated, none invented or
      blended. The trap is that a rotation mapping two source cells onto one
      target cell is exactly where pixels get destroyed. If that cannot be
      avoided for pieces small enough to matter, the whole-cell route closes and
      the item becomes the full body extraction above, at its stated price.

    > **In plain terms.** *(large — deferred past v0.1, and the reason it is
    > deferred has changed)* Structures currently drop or break but never
    > tip over, which reads as lifeless. **This used to be written as "may
    > close as not possible without wrecking the pixel art", and that
    > sentence is withdrawn** — it closes a door the reference engines walk
    > through, and someone reading this in a year would believe it. The
    > objection was that rotating a piece resamples it and destroys the
    > authored pixels. The reference answer: trace the outline of the
    > connected piece, simplify it to a polygon, hand *that* to a rigid-body
    > solver, take the piece's cells out of the grid entirely, and stamp
    > them back in each frame from the body's own private copy of its
    > pixels. The pixels are never resampled — they live with the body and
    > are only *drawn* rotated. Rotation becomes a drawing problem, which is
    > a solved one.

- [x] **E9 — Fuel, and a clock for steam. The two thermal quantities that
  temperature was standing in for.** *(both halves done; the fuel half in waves
  2b/2c, the steam half on 2026-08-12)* *Observed:*
  [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 1, defects A3, A4 and A5.
  *Unlocks:* fire that reads as burning rather than as flickering, and it closes
  the last loose end the correctness pass left behind.
    - **Confirmed in play 2026-08-13, checklist step 5** ([spot
      check](PLAYTEST_LOG.md#spot-check--2026-08-13--the-two-owed-steps-run-together)).
      The pocket rises, gathers, waits, and drips from the top. **That retires
      A5, B3 and D5 — one symptom, reported three times across four sessions,
      and the only one on the record that was scheduled by repetition rather
      than by a plan.** The step was written so its two failure directions point
      at different mechanisms — condensing immediately would mean the lifetime
      is still a temperature, never dripping would mean the ceiling-contact rule
      is blind — and neither came back.
    - **Both halves are now done. The steam half shipped 2026-08-12 and this is
      what it turned out to be.** The item's own prediction was "give steam its
      own condensation counter"; that is what was built, and the two things it
      got wrong are worth more than the part it got right.
        - **The counter is on `Element::ticks`, and steam is the second material
          to spend it as a lifetime rather than a third role on the byte.** Fire
          already used it that way and Fire is a Gas; steam is a Gas; so the
          rule generalises rather than being extended. The old guard was a
          `static_assert` naming Fire, which was right and did not generalise —
          it is now `tick_role()` in [element.h](src/physics/element.h) plus an
          assertion over every row that nothing carrying a lifetime is
          structural. **This is the compiler that E10's entry asks for, arriving
          early and from a different direction**, and it is the reason the open
          `ticks` decision is not blocked by this item: the roles are now
          counted in one place, so whatever E5a's representation turns out to
          be, adding it is a change to a function that asserts rather than a
          belief about a byte.
        - **The condensing *reaction row* is deleted, and that deletion is the
          actual fix.** `{ Steam → Water, 0..26 }` is gone. The prediction above
          framed the problem as steam's life being too short; the sharper
          statement is that **it was measured in the wrong units**. Life was the
          span between spawn temperature and condensing point, so it depended on
          what steam was *touching* — and because `Empty` conducts nothing while
          stone conducts well, a pocket had its **shortest** life pressed
          against the ceiling it is supposed to collect on. Measured: a sealed
          pocket used to drain in **3 steps**. That is why the same complaint
          came back three times (A5, B3, D5) reading each time like a request to
          change a number, when no number in reach would have fixed it.
        - **"Condense at contact points rather than uniformly" was right, and
          one step short.** The bullet below predicted contact-point
          condensation and it was built that way first — a plain countdown,
          running faster where a cell touched a ceiling. It measured as drips
          and read as a puff: **the whole four-deep pocket drained in about a
          second**, because every cell was ageing on its own schedule whether or
          not it was touching anything. The rule that works is stricter and
          simpler: **only a cell in contact ages at all.** The interior of a
          pocket is not on a clock, it is waiting its turn. A pocket then drains
          from the top down and takes as long as it is deep, which is what
          "collects, waits, then drips" actually means. Same fixture after:
          **291 steps, first drop at 181.**
        - **Nothing in it writes a drop, and that is the design rather than an
          economy.** A contact cell that runs out becomes `Water` in place;
          `Water` is denser than the steam beneath it, so `can_displace` —
          already there — carries it down through the pocket and onto the floor.
          Writing the drop downward by hand was the obvious version and would
          have been a fourth write path reimplementing a decision the density
          rule already makes. **Drip rate scaling with pocket size is likewise
          not implemented**: a wider pocket has more cells against the ceiling,
          each on its own clock, so it drips faster and slows as it drains. Both
          of the behaviours B3 asked for are emergent.
        - **Two things it cost, both recorded where they will be found.** Steam
          no longer condenses because it is *cold* — cold-quenching is
          re-expressible as a catalyst row whenever something asks, and is filed
          in [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md) so it is not refiled
          as a bug. And the branch added to `step_cell` reads as **+3–4% on
          `churning` and `cascading`, neither of which contains a steam cell**;
          it merges under the frame-budget rule and the reading is in
          [PERFORMANCE.md](PERFORMANCE.md) with the reason it is probably layout
          rather than the branch. **The more useful finding there: no benchmark
          scenario contains steam, so this feature's cost where it is actually
          used is unmeasured, which is a named requirement for `P4`.**
        - *Verified:* four new checks in `grid_test`, and **the two that carry
          the feature were confirmed against the unfixed engine first** — the
          pocket-drain span reads 3 steps before and 291 after, which is the
          defect and the fix in one number. Ten suites green. **The manual
          checklist is owed**: step 5 (reactions and heat) is the one that
          matters, and it is the step all three original reports came out of.
    - **The fuel half is done, and it was the whole of what was left in this
      item until 2026-08-12.** Fire was rebuilt, tuned and confirmed good in
      play across [waves 2b and 2c](ROADMAP_ARCHIVE.md#-waves--sub-plans-that-preempt-the-tracks)
      — session 4 answered "does this read as burning, at a pace that feels
      right" with yes, and fire goes back to being a regression check rather
      than an open question. Everything about what those waves changed, cost and
      got wrong is recorded with them; this entry keeps the argument for *why*
      the model had to change. **The steam clock below has never been built**,
      and it was briefly carried in the wave table as a queued wave of its own,
      which meant two documents each believed they owned it. It is tracked here
      and only here.
    - **The fuel half is built, and the first build of it put the timer on the
      wrong cell.** A3 and A4 were both closed by a `burn_duration` column, with
      the flame seeded from the material it consumed and held in place while it
      still owed burning time. That is correct about duration and correct about
      propagation, and it models fire as *fuel that has become flame*. Reference
      footage of a scene burning says fire is the other thing: **the fuel stays,
      in a burning state, and emits short-lived flame into the air around it** —
      see [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 1 follow-up. Flame lives
      ~5–15 steps and is completely replaced between frames; the fuel lasts
      seconds. Two quantities, two cells. One 180-step stationary flame cell is
      an orange block regardless of how correctly it propagates.
        - **What survives the rebuild:** duration is a per-material number, fire
          crosses a horizontal beam, and the regression tests that pin both.
          What changes is ownership of the timer.
        - **`Wood` gains a burning state as its own row (`Charred`) and an
          `emits` column, so this stays a table edit.** Burning things throwing
          flame has to be a column and not a Wood special case, or the next
          flammable material needs engine work instead of a row. The one genuine
          engine addition is emission itself — a burning cell writing into a
          randomly chosen adjacent *Empty* — and it pays for itself immediately:
          "fire is a layer on the surface and interiors never burn" is not a
          rule anyone writes, it is what emission-into-empty does when a cell is
          buried.
        - **The byte budget decides char's lifetime, and the forced answer is
          the better one.** Burning wood must stay structural — a burning
          ceiling that drops instantly is worse than one that burns — and
          structural cells already spend `ticks` on the free-fall clock.
          `Element` has no spare byte, so char *cannot* hold a countdown, and
          its lifetime becomes a per-step decay chance instead. That needs
          `chance_pct` widened to per-mille, since a 180-step mean is 0.56%. The
          gain is variance: a plank's cells stop winking out in lockstep.
          **Flame keeps a real countdown**, because Fire is a Gas and never
          structural, and that countdown is what the colour ramp reads.
        - **The `static_assert` written for the first build is what made this
          analysable rather than a bug.** It forbids Fire from being structural,
          on the grounds that `ticks` means two things. Extending fire to a
          material that *is* structural walked straight into it, at compile
          time, with the reason attached — which is the whole argument for the
          correctness pass's rule, arriving from a direction it did not predict:
          the invariant was between a struct field and a table row, not between
          two tables.
    - *(done, kept for the reasoning)* **Fire has no fuel,~~ and that is why
      three separate defects look like tuning problems and are not.** Wood
      ignites at 100% on reaching 120° and the resulting Fire dies on a flat
      6%/step roll — about 17 steps, roughly a quarter second. Fire from wood,
      fire from oil and fire from nothing are the same cell with the same
      lifetime, so "wood burns too fast" has no number to change. It needs a
      `burn_duration` column on `MATERIALS` and a per-cell fuel counter that
      Fire inherits from whatever it consumed.
    - *(done, kept for the reasoning)* **The horizontal-beam defect is the same
      missing quantity, which is why these are one item and not two.** Fire is a
      `Gas` at density −10, so it rises: a flame on top of a horizontal beam
      floats off after a single step and the wood below never reaches its
      threshold, while alongside a *vertical* beam the flame rises parallel to
      its fuel and stays in contact the whole way up. Fire that is consuming
      fuel should stay where the fuel is; fire with none rises and expires. One
      rule, and it fixes propagation and duration together.
    - *(done 2026-08-12, kept for the reasoning)* **Steam's lifetime is the span
      between its spawn and condensing points, so temperature is doing double
      duty as a clock — and that coupling is where the ignition bug came from.**
      *(This and the two bullets under it were the open half of E9, and they
      diagnosed it correctly. Session 1 raised it twice, as defect A5 and as
      observation B3, and they are the same fix seen from two sides.)* Steam
      used to spawn at 220° purely to make a puff last, which put it 100° over
      Wood's ignition point and made dousing a fire a way of starting one. The
      fix dropped spawn to 88° and paid for it in lifetime; session 1 confirms
      the bill came due. **Give steam its own condensation counter and both ends
      are satisfied at once** — a long-lived puff, and a spawn temperature that
      stays under the ignition floor the `static_assert` at the bottom of
      `reaction.h` already guards.
    - *(done 2026-08-12, kept for the reasoning — and see the correction above:
      "at contact points" had to become "only at contact points" before it read
      as collecting rather than as a puff)* **What that unlocks is the behaviour
      session 1 actually asked for:** condense at contact points rather than
      uniformly, so steam collects against a ceiling, waits, and emits water
      downward while the pocket shrinks — drip rate scaling on pocket size,
      which is both what was asked for and the physically right shape.
    - **The trap, and this area has been burned by it once already — it was
      avoided, and by the route this bullet named.** No thermal column was added
      in the end, so the specific hazard did not arise; what did arise is its
      shape one level up, a *byte* gaining a third claimant across two files,
      and it was answered the way this bullet says to answer it: with an
      assertion next to the one already in `reaction.h`. Original text follows.
      `spawn_temperature` and the ignition thresholds live in two different
      tables, and nothing about editing either one suggests reading the other.
      Any new thermal column inherits that hazard. The correctness pass's rule
      applies directly — when a table gains a column, ask what invariant now
      spans two tables and assert it, next to the one already in `reaction.h`.

- [ ] **E5b — The air field.** *(the second half of what was E5, and it absorbs
  the item that used to sit below this list as "gas pressure — named and
  deliberately not scheduled")* *Observed:* the four searches named in this
  track's preamble, plus the gap that used to be its own note — steam in a
  sealed room builds no pressure and smoke does not fill a space it should,
  because gases rise and spread by the same per-cell rule as everything else.
  *Unlocks:* six things that are currently six separate gaps, which is the
  argument for it. **Scheduled after the playtest gate**, and the two costs
  below are why.
    - **A second coarse grid over the world — pressure and a velocity, one entry
      per 4×4 block. The pattern is already built, shipped and measured:**
      `LightField` is exactly this shape — a low-resolution, integer,
      deterministic field stretched over the scene and composited with a single
      `RenderCopy`. Reusing its block size, its padding rules and its "does this
      frame have any at all" early-out is most of the design already done.
    - **What one system delivers:** gas pressure; explosions that *push* rather
      than delete-and-throw; steam and smoke that advect and curl instead of
      rising in columns, which is the single thing that most makes a sandbox
      read as alive; wind, which V9's sparks need in order not to look like a
      screensaver; fire suffocating in a sealed space, which E9 cannot currently
      express at all; and the pressure term that **retires `find_lower_surface`,
      `vent_fluid` and `make_room_above` together**.
    - **It also buys back something the current design has ruled out
      permanently, and that is the strongest argument for it.** E1 levels water
      by teleporting a surface cell across the connected body — instant, and
      non-local by construction. Waves, sloshing, surges and water hammer are
      therefore not missing features, they are **impossible**, because the
      mechanism has no propagation delay for them to live in. Pressure diffuses
      over several steps, so they fall out for free rather than being added.
    - **Two honest costs, and both are why it is after the slice rather than
      before.** The reference implementation (The Powder Toy's) is
      floating-point, and `Grid` forbids floating point for the reproducibility
      F1 spent seven steps establishing and F1.7 wrote down — so this needs a
      fixed-point port, which is doable, which is non-negotiable, and which is
      real work that the reference does not do for you. And it is a **fixed**
      cost proportional to the awake area rather than one that scales with how
      much is moving. Every other cost in this engine has the second shape;
      chunking, dirty rects and the thermal early-out are all built on it. That
      difference deserves its own [PERFORMANCE.md](PERFORMANCE.md) entry with a
      measurement at the played size, not a bracketed number attached to an
      item.
    - **The four searches do not get deleted on the strength of the field
      existing.** Each one is currently the only thing preventing a specific,
      reproduced defect — A6, A6b and the U-tube case all have probes and tests.
      The order is: field lands, each search is removed one at a time, and the
      probe that motivated it is the thing that has to still pass. A search
      removed on the argument that the field "should" cover it is how A6b comes
      back.

    > **In plain terms.** *(large — the second half of the old E5, and it
    > absorbs the item that used to be called "gas pressure")* A second
    > coarse grid over the world, one entry per 4x4 block, holding pressure
    > and a velocity. **The pattern is already built and shipped:** the
    > lighting is exactly this — a low-resolution, whole-number,
    > reproducible grid stretched over the scene with one draw call. One
    > system delivers six things that are currently separate gaps:

### V — Visual identity

Running order: **V17 → (V11 + V12 + V13) → V19 → V16 → V9 → V14 → V8 →
V7-rest**, with **V10.1** alongside E6. **V15 moved behind the playtest gate on
2026-08-11** and is gated on the combat decision, for the reason written in its
entry. **V19 is new on 2026-08-16 and sits where it does by request**, not by
inference; **V16 may end up inside it rather than after it**, because five new
pan-sized layers is the cost that makes a wrapping layer worth having, and that
is the first thing V19 has to decide.

*Running order: **~~V7-emissive → V5 → V6 → V10 → V3 → V4-props~~ → V11 + V12 +
V13 → V19 → V16 → V9 → V14 → V15 → V8 → V7-rest**, with V10.1 alongside E6 and
V15 pulled by `S1`. **V19 is new on 2026-08-16 and is placed by request rather
than by inference** — the ask was for a `CnC_parallax`-shaped scene before the
split-view path — and **V16 may end up inside it**, since five new pan-sized
layers is the cost that finally makes a wrapping layer worth having. **V12–V16
are new as of 2026-08-11 and are the visual system this track was not planning
for** — see [The visual system this track is now building
toward](#the-visual-system-this-track-is-now-building-toward) immediately below,
which is the one place the four goals behind them are stated together. **V11 is
new as of 2026-08-09 and goes to the head of the open half**, and the reason is
a requirement this track was not serving: the art direction is expected to
change several times, and five specific things in the codebase make each change
expensive — the frame composition is 350 inline lines in `main.cpp` with a
hard-coded layer order, material colours are compile-time constants,
`Camera::SCALE` is a compile-time constant, the light layer can only add and
never subtract, and the parallax factors are duplicated across two files. None
of those is hard to fix today and every one of them gets harder. **V9 moves
ahead of V8** because E6 is what it exists to dress and E6 now lands before it.
The struck-through half is done and is left visible rather than deleted, because
the order is the argument and a reader needs to see which arrows were actually
followed. **V8 has moved from second in this line to second-last, and it moved
by everything else earning a place rather than by being demoted**; see the note
in "Where this stands" for why its remainder still cannot answer this tier's
admission test. **The five items now ahead of it are the standard it failed**:
each names something in the built game that is wrong, and V16 in particular
takes the half of "the backdrop needs work" that *can* — the backdrop does not
move — leaving V8 holding only the parts that still cannot. The order below is
not the numbering — see the note on stable IDs above. **A way to drive the
window is a prerequisite for everything from V3 onward and is promoted into this
track from [Sandbox / debug tooling](#sandbox--debug-tooling)**, where the
argument for moving it is written out; V10 is placed ahead of that line because
a HUD is the one visual item that can still be judged on a screenshot. V5 and V6
are what everything after them is authored **against**, so they run first even
though they were written last. **V7 is split by its gate rather than placed
once**: its emissive half runs immediately after E9's fire rebuild, because fire
is the one subject here whose look is what it emits rather than what colour it
is, and the argument for that is written into V7 itself. Everything in V7 that
needs authored albedo to respond to keeps the original gate and stays where it
was.*

***Reference footage is a primary input to this track, which is the opposite of
its standing in E.*** A look cannot be derived from your own bugs — nothing
about the fact that the player is a white rectangle tells you what should be
there instead, and that asymmetry is the whole reason the two tracks answer to
reference differently. V5 is the item that consumes it; V7, V8 and V9 each cite
it for a specific question they would otherwise guess at. Observations go in
[notes/reference_observations.txt](notes/reference_observations.txt), the frames
themselves are gitignored, and the caveat is craft rather than process: what
comes out is our direction informed by theirs, not their palette lifted. **V6 is
where that stops being a good intention and becomes checkable**, since a locked
palette is a constraint a validator can enforce and a resemblance is not.*

**This track is in progress as of 2026-08-16, and it displaced E10 to get
there.** The running order above is unchanged; what changed is that the head of
it is now being worked, as one block rather than as five items. The trigger was
a request for a split-view and parallax backdrop system, and the finding that
made it a block is that **V11, V17, V7-rest and V8's remainder are all the same
item wearing four hats** — every one of them is blocked on the 350 inline lines
of frame composition, and taking them separately means extracting that code
three times. The step order, sizes and current state live in
[ROADMAP_ITEMS.md](ROADMAP.md#-next-up); the reasons are here and at each
item.

Two things came out of the first step — rewriting the two `notes/` files, which
is documentation and was expected to return nothing:

- **The four depth bands do not separate by value, and the reference says they
  must.** Measured against the CnC frames: sky spans roughly `0x14`–`0x28`,
  mountains `0x20`–`0x30`, trees `0x18`–`0x59`, terrain `0x1B`–`0x78`. They
  overlap almost completely, so **nothing distinguishes one band from the next
  except the rim highlight** — which is why a busy frame reads flat. **This is a
  renderer defect and not a palette one**, and the distinction matters because
  the obvious repair is wrong: pushing a band's range down is a *multiply*, the
  light pass can only *add*, so no edit to `PALETTE` can express it. It is the
  concrete thing V7-rest's darkening now exists to fix, and the first time that
  item has had a measured symptom rather than an intention. **Fixed 2026-08-16
  at block step 3, and the re-measurement was worse than the channel ranges
  above suggested** — in luminance the sky averages 26 and the mountains are
  *flat 28*, p05 and p95 both 28, so the two most distant bands separate by two
  levels out of 255 and the far one is the brighter of the pair. A range is not
  a separation, and quoting one for the other is how this understated itself.
  The mountains now carry a 0.60 per-layer multiply. **One clause above is wrong
  and is corrected rather than deleted:** "pushing a band's range down" is
  right, but it cannot be done *globally* — a frame-wide multiply scales every
  band alike and leaves every ratio between them where it was, so separation is
  necessarily per-layer. The world-wide grade got built alongside it and is a
  different feature (night, fog, biome); reading this bullet as the argument for
  it would be reading it as the argument for the one knob that could not have
  answered it.
- **The parallax factors were never measured, and nearly got a false
  provenance.** The three "parallax" reference frames were taken to be one scene
  panning, which would have priced every band. They are not: region shifts come
  back non-monotonic in depth and *opposite in sign* at the two edges, with `dy`
  zero everywhere, which no camera translation produces. They are three
  separately generated lakes sharing a sprite vocabulary. So `PARALLAX_SKY_X`
  and friends remain what they have always been — chosen by eye — and **V11 must
  ship the existing factors unchanged**, with the golden frame proving the
  extraction was a no-op, and retune as a separate commit that can be pointed
  at. Recording an eyeballed number as a measured one is the failure this
  project's first rule is about, and it was one step away. **V11 shipped on
  2026-08-16 and did exactly this** — `0.04/0.02` and `0.15/0.06` are
  byte-for-byte what they were, and the golden checksum held unchanged across
  the whole restructure before the one deliberate change moved it. **The
  mid-ground band's new `0.40/0.16` is the case this rule had not covered**,
  since it is not a retune of an existing factor but a factor where there was
  none: it ships with a *stated derivation* (parallax is inverse depth, so
  sqrt(0.15 × 1.00) ≈ 0.40) and is labelled a derivation rather than a
  measurement everywhere it appears.

Both are written up in
[notes/reference_observations.txt](notes/reference_observations.txt); the
correction to V5's resolution is filed beside V5's own entry.

#### The visual system this track is now building toward

***Added 2026-08-11. Four things were named as the intended end state of the
visual system — varying pixel resolutions and sprite sizes in one scene,
procedural animation, backgrounds that are animated rather than static, and
entity bodies that granulate locally when damaged. This section is where that
commitment is recorded, because three of the five new items below cannot be read
without it and one of them is admitted by it rather than by an observation.***

**A commitment is not a licence to skip the admission test, and four of the five
still pass it on their own.** V12, V13 and V16 each name something in the built
game that is wrong or reads badly — a colour key that forbids a soft edge
anywhere, a draw path that makes a denser drawing a bigger object, a backdrop
whose only motion is parallax and therefore stops when the player does. V14
names an articulation limit the track has already written down twice. **V15 is
the one item admitted on the commitment itself, and it is admitted honestly**:
[notes/procedural_animation.md](notes/procedural_animation.md) wrote down, in
advance, the three things that would admit a skeletal rig, and the third is *"a
second character type is committed to."* That is exactly what happened. **The
note predicted the trigger and the trigger fired**, which is the strongest form
this document's admission test has ever taken — better than an observation,
because it was specified before the thing that satisfied it existed. The same
move admits `E12`'s material half through
[notes/granulating_enemies.md](notes/granulating_enemies.md), whose own closing
section names the condition under which it stops being notes.

**What the commitment does *not* do is move the running order.** None of this
displaces wave 4, E9's steam half, the fluid spike or `S0`; the visual block
sits where V11 already sat. The reason to write it all down now rather than when
each item comes up is the same reason V11 exists at all — every one of these
gets more expensive the more code reads the assumption it removes, and three of
them share draw sites, so taken apart the same two rectangles in `main.cpp` are
edited three times.

**Two things the commitment asks for are deliberately not items, and both are
recorded as decisions rather than left to be discovered mid-implementation** —
see [Decisions owed](ROADMAP.md#-decisions-owed). *Higher-resolution pixel
art* is not bought by making the world's cells smaller: at `Camera::SCALE` 2 the
viewport holds four times the cells, every asset is re-authored, and every
physics constant is retuned, because all of them are stated in cells against a
scale of 4 (`Player::WIDTH`/`HEIGHT`, `DigTool::RANGE`, `LightField`'s reach in
blocks). V13 buys denser *art* with none of that, and it is also the cheap
experiment that says whether denser art is wanted at all. *And the shader path
stays refused*, which V7 already required be decided deliberately if it is ever
decided: every item below has a route through `SDL_Renderer` as it stands —
`SDL_ComposeCustomBlendMode` for a multiply term, `SDL_RenderGeometry` for
textured triangles, `SDL_TEXTUREACCESS_TARGET` for a masked body, all present in
the pinned SDL 2.30.0 and none of them used anywhere yet. **The fork should be
bought by the first thing that has no such route**, and sub-cell terrain detail
is the one candidate on the horizon, because the terrain's resolution is the
simulation's resolution and no amount of asset work changes that.

- [x] **V1 — Transparent `Empty` and a backdrop layer.** *(done — see
  [Shipped](ROADMAP_ARCHIVE.md#v1v2-visual-foundation))*
- [x] **V2 — Palette and jitter pass on `MATERIALS`.** *(done — see
  [Shipped](ROADMAP_ARCHIVE.md#v1v2-visual-foundation))*

- [x] **V5 — Write the art direction down.** *(done — full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#v5-write-the-art-direction-down))*
- [x] **V6 — One locked palette, shared by the table and the art.** *(done —
  full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#v6-one-locked-palette-shared-by-the-table-and-the-art))*
- [x] **V3 — Player sprite decoupled from its hitbox.** *(done — full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#v3-player-sprite-decoupled-from-its-hitbox))*
- [x] **V3.1 — Animation, and one decomposed limb.** *(done — full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#v31-animation-and-one-decomposed-limb))*
- [ ] **V4 — Props overlay and the `Snow` material.** The non-simulated props
  layer (signage, fencing, anything that exercises no system and therefore
  belongs on top rather than in the grid), plus `Snow` as one `MATERIALS` row: a
  powder lighter than Sand, and the single most on-theme material for the
  reference art. **Sequenced after E2 on purpose** — with heat in the engine,
  snow melting into water is a thermal row rather than a special case someone
  has to hand-write, which is the whole argument for E2 arriving first. **The
  `Snow` row is also E7's first row**, and if E7 has run by the time this item
  comes up it belongs there rather than here; the props layer is what is
  genuinely V4's.
    - **The props half got a first slice alongside V5/V6: trees.**
      `tools/generate_props.py` generates colour-keyed tree sprites;
      `main.cpp`'s `Prop` struct anchors one to a world position and draws it
      before the cell texture, which is what lets authored terrain occlude a
      trunk's base for free (see notes/art_direction.txt's layer model for why
      that ordering, not the more obvious "props on top," is correct here).
    - **The prop format is now built and the hardcoded list is gone** —
      `src/scene/props.{h,cpp}`, `assets/test_props.txt`, and `props_test` as
      the ninth suite. The nine trees are unchanged on screen; what changed is
      that they are data. A second scene can have its own props now, which is
      what Quantum Worlds was going to need and did not have.
        - **Four decisions came out of building it and all four outlived the
          item, so they are recorded as standing rules in
          [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md) rather than here:**
          per-cell data gets an image and a list gets a list (the reflex is to
          copy F4's material map, and the frozen legend is what makes that not
          merely expensive but forbidden); a format must not carry a number its
          loader ignores, which is why there is no `y`; a malformed list costs
          every record rather than the bad line, and the test suite that follows
          from that asserts *empty* rather than *short*; and data that names a
          path gets its name validated. **What belongs here is the sequencing
          claim, not the rules:** this was the item taken instead of V8's
          remainder, and it is the worked example of the tier's admission test,
          because it could name what was wrong (no format, so no second scene
          could have props) and what it unlocked (Quantum Worlds).
        - **One consequence worth naming: an unplantable prop is now dropped
          rather than parked at a fallback.** The authored-y version had a y to
          fall back to and falling back to it is how the burial hid; with no
          authored y the only fallback is 0, the top of the world, so it would
          hang in the sky. Removing it and warning is the honest version of the
          same message.
            - **The startup count contradicted that warning for a whole
              revision, and moving it is the fix.** `Props: N of M placed`
              printed immediately after the textures loaded — 59 lines above the
              terrain scan, and therefore before either way a prop can be
              dropped — so it counted texture loads while saying "placed".
              Confirmed against the running binary rather than argued: a fixture
              with one unplantable prop printed `Props: 10 of 10 placed` on
              stdout while stderr warned that one of them was not drawn. **The
              warning was right and the number contradicted it, which is worse
              than no number** — [README](README.md)'s launch check makes this
              line the check *rather than* the eyeballing. The `printf` now runs
              after planting; the same fixture reads `9 of 10` and the clean one
              still reads `9 of 9`. The general form is in
              [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md): a count taken before
              the last thing that can fail is measuring a different quantity
              than its label claims.
    - **Still open in V4:** `Snow` is untouched, and per this item's own text it
      belongs to **E7** rather than here if E7 has run by then. Props themselves
      are still one flat layer at parallax ~1 with no depth sorting among them,
      which is correct for the forest and is the thing to revisit if a scene
      ever wants props at two depths.

    > **In plain terms.** *(days)* The prop format shipped. `Snow` is now an
    > E7 row rather than a V item, since heat makes it melt by table.

- [ ] **V8 — The backdrop: authored, and parallax.** *Observed:* V1 shipped the
  *layer* and a 64-band gradient placeholder, and said in as many words that
  authored backdrop art was not its job. F3.4 then landed a camera that
  genuinely pans, which is what turns the placeholder from plain into wrong.
  *Unlocks:* the depth the reference art gets most of its beauty from, at a cost
  the layer model already priced.
    - **A panning camera is what makes a single static backdrop impossible to
      get right.** It either slides 1:1 with the world, in which case it reads
      as painted onto the terrain, or it does not move at all, in which case it
      reads as a wall behind a moving world. Parallax layers are what a backdrop
      is *for* once the camera moves — and the original three-layer model was
      written before F3.4 existed, so it assumed a static image and did not say
      this.
    - **Cost is one `SDL_RenderCopy` per layer**, which is what that note
      already budgeted, and the offsets come from `Camera::view_x()`/`view_y()`
      scaled per layer — no new coordinate system, and specifically not a second
      one, which is the thing F3.2 spent a step consolidating.
    - **A first slice shipped alongside V5/V6, and it is a slice rather than the
      whole item.** Two static layers — sky+stars and a mountain silhouette,
      `tools/generate_backdrop.py` — replace V1's gradient in `main.cpp`, each
      offset by `camera.view_x()`/`view_y()` at its own parallax factor exactly
      as specced above. **Left open:** more than one biome's worth of backdrop
      (this is the forest world only), any time-of-day variation, and a third
      depth band beyond sky/mountains if a future reference asks for one. The
      factor pair for each layer lives in two places that must be kept in sync
      by hand — `main.cpp`'s `PARALLAX_SKY_X/Y`/`PARALLAX_MOUNTAIN_X/Y` and the
      matching constants at the top of `tools/generate_backdrop.py` — because
      the image has to be sized to the exact pan range its own factor implies;
      nothing enforces the two files agreeing, which is written down in both
      places as the trap it is.

    > **In plain terms.** *(weeks)* **Held deliberately.** Sky, mountains
    > and parallax ship. The rest cannot currently name anything in the
    > built game that reads badly, so it would be reference-driven breadth
    > wearing engine clothes. V11 is the piece of this that *is* real, and
    > it is scheduled above. Revisit when a second biome actually needs to
    > exist.

- [x] **V7 — Per-cell emissive lighting.** *(done — full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#v7-per-cell-emissive-lighting))*
- [ ] **V9 — A non-simulated effects layer, and impact feel.** *Observed:* E3's
  fractures already happen in silence — a slab cracks in half and drops with no
  dust — and E6's explosions will be worse, because the bigger the event the
  more conspicuous the absence. *Unlocks:* the feedback that makes destruction
  read as destruction rather than as the world rearranging itself.
    - **Sparks, embers, dust and smoke wisps interact with nothing, so
      simulating them buys nothing and costs the hot loop.** They are drawn on
      top, on the same layer as V4's props. This is the distinction
      [notes/art_direction.txt](notes/art_direction.txt) already draws for
      scenery — if it exercises no system it belongs in a drawn layer rather
      than in the grid — applied to effects instead of to decoration. **Do not
      confuse this with E5a.** E5a is matter with a velocity that stays in the
      grid; this is a visual that expires and never enters it. A spark that has
      to conserve mass is a cell and belongs there.
    - **Bounded by construction:** a fixed-size pool with the oldest recycled,
      and no allocation per spark. The failure mode otherwise is a frame-rate
      cliff that appears exactly when the most is happening on screen, which is
      the worst possible time for it.
    - **This is where the free-particle idea E5a rejected actually belongs**,
      and the two items should be read together. E5a keeps moving matter in the
      grid precisely so that it does not need a second answer to every rule in
      the engine. A spark needs *no* answer to any of them — it conducts no
      heat, reacts with nothing, occludes nothing — so a sparse off-grid list
      with a sub-cell position is exactly right here and exactly wrong there.
      The dividing line is conservation: a thing that has to conserve mass is a
      cell, and a thing that expires is one of these.

    > **In plain terms.** *(week)* Sparks, embers, dust and smoke wisps
    > drawn on top, interacting with nothing, from a fixed-size pool so a
    > big event can't tank the frame rate. Sequenced after E6 because
    > explosions are what it exists to dress.

- [ ] **V11 — The visual system is adaptable.** *Observed:* five places where
  changing the art direction is expensive out of proportion to the change, all
  found by reading the render path on 2026-08-09, and all cheaper to fix now
  than after another visual item lands on top of them. *Unlocks:* the ability to
  change direction repeatedly, which is a stated expectation of this project and
  is currently served by nothing. **Admitted on the first question rather than
  the second, which is unusual for this track and is the point:** none of the
  five is a look, all five are the cost of changing one.

    > **In plain terms.** *(week — new, and it is the item that makes
    > changing direction cheap instead of expensive)* The stated expectation
    > is that the art direction will change several times. Right now a
    > direction change is expensive in five specific, findable places, and
    > none of them is hard to fix *today*.

    > **Status 2026-08-16: three of the six bullets are shipped and one is
    > partly.** The layer list, the parallax duplication and the sixth bullet's
    > lighting field are done; the mid-ground band has a slot and no art, for
    > reasons below. **The tint bullet — the light pass gaining a multiply — is
    > step 3 of the V block and is what remains of this item**, along with
    > runtime `Camera::SCALE` and the theme-loadable material colours, which
    > stay with V12/V13. Each bullet below carries its own outcome.

    - **~~There is no renderer.~~ ✅ Shipped 2026-08-16, in two commits by
      design.** *(Original text, kept because the estimate in it is what got
      corrected:)* Roughly 350 lines of frame composition sit inline in
      `main.cpp` with the layer order hard-coded — clear, sky, mountains, props,
      cells, player, light, reticle, HUD, hotbar, menu.
      [notes/reference_observations.txt](notes/reference_observations.txt) has
      **already** concluded that a mid-ground band is needed that this stack has
      no slot for, and V8's remainder is a third depth layer. Extract
      `render/frame.cpp` holding an explicit ordered list of layers. An
      afternoon, after which adding a band is one entry rather than surgery
      between two comments — and `main.cpp` stops being the file every visual
      change has to touch.
        - **It was ~175 lines, not ~350**, and the difference is that the larger
          figure counted the reticle, the HUD, the hotbar, the run-over wash and
          the settings menu. Those are UI, they stayed in `main.cpp`, and **the
          boundary is load-bearing rather than filing**: V7's light pass is the
          last thing drawn in the world and everything after it is deliberately
          unlit, which is what defect B1 was about.
        - **Split into V17 (move it, change nothing, checksum it) and V11
          (restructure it), and that order is the finding.** V17's own entry had
          to admit it could not retroactively prove the extraction was a no-op,
          there being no checksum before the move — the claim rested on the diff
          being verbatim. V11 is where that stopped being a limitation: the
          layer table, the camera method and the generated header were built and
          run **first**, against V17's number, which held unchanged at
          `0x3d729ad7fbcaa839`; the band was added after, and the number moved
          to `0x06f6412da7af6607`. **The general form: when a change has a no-op
          half and a visible half, build the no-op half first and prove it,
          because afterwards you cannot.**
        - **`FRAME_SOURCES` is a fifth source-set variable**, kept out of
          `RENDER_SOURCES` because `frame.cpp` is the only rendering source that
          calls SDL and folding it in would make nine headless suites link SDL2.
          `golden_frame_test` is the first test target in the project that links
          SDL, and still needs no display.
    - **Material colours are `constexpr`, so a retune is a recompile and a
      re-validation.** Give each `MATERIALS` row a palette *slot* and put slot →
      colour in a loadable theme. **The correctness pass already learned exactly
      this lesson once** — it separated the level file's colour codes from the
      render palette after V2's retune booted the game to a blank world for a
      whole commit — and this is one further step of the same move, from "the
      level does not depend on the palette" to "the palette is not in the
      binary". A second biome is then a file and time-of-day is two files and a
      blend. **The cost, stated rather than discovered:** `pixels` holds baked
      jittered colour, so a theme swap needs one pass over the world. That is a
      one-off at swap time, not a per-frame cost, and the hot loop is untouched
      — which is the only version of this that is affordable and should be built
      as that version deliberately.
    - **`Camera::SCALE` is a compile-time constant**, and `display.h` argues for
      keeping it one. That argument is about the window, and it is right about
      the window; it is now also load-bearing for the reticle's size, the sprite
      offsets and the prop rectangles. A zoomed-out biome, a different cell size
      and the already-scheduled resolution options all collide with it, and
      every system that reads it makes the change more expensive. Make it
      runtime while three things read it rather than six.
    - **The light layer can only add, so every biome will be the same
      brightness.** V7 says this in its own text as an open-by-design limitation
      — it lights only *hot* things and it only ever brightens. Generalising it
      to multiply-and-add (an exposure and tint term as well as an emissive one)
      gives night, underground, fog and per-biome grading without touching a
      single material colour. **Given that the reference finding was that the
      read comes from silhouette layering rather than detail, this is the
      highest-value visual control available**, and it is a strictly smaller
      change than V7-rest, which it partly absorbs. ✅ **Shipped 2026-08-16
      (block step 3), an afternoon against a week.** It came out as *two* knobs
      and not the one this bullet describes — a per-layer `Grade` on every row
      of the layer table, and a world-wide one as its own pass — for the reason
      recorded at the depth-band finding above: a global multiply cannot
      separate bands. **The estimate was wrong for a reason worth keeping**:
      this was priced as the item that would finally spend
      `SDL_ComposeCustomBlendMode`, the escape hatch the renderer-versus-shader
      refusal has held in reserve through two examinations. It needed no custom
      blend mode — `SDL_BLENDMODE_MOD` is stock and `SDL_SetTextureColorMod`
      does the per-layer half with no extra draw call. **All three named escape
      hatches are still unspent, and the first item that looked like it would
      spend one did not**, which is evidence for that refusal rather than merely
      an absence of evidence against it. The bullet's own prediction held: the
      mountains at 0.60 are the whole of the depth fix and they are one number.
    - **~~The parallax factors are duplicated~~ ✅ Shipped 2026-08-16.**
      *(Original:)* between `main.cpp` and `tools/generate_backdrop.py` with
      nothing enforcing agreement; the failure is a seam at the pan limit.
      Generate the header from the tool, exactly as V3.1 did for the player
      sheet. This one is already named in "Where this stands" as an afternoon
      that is not a tier item — it is folded in here because it is the same
      class of problem as the other four and it is silly to do separately. **V16
      retires the problem rather than documenting it** — a layer that wraps
      needs no size relationship to the pan range at all — so this bullet is the
      cheap version bought now and not the last word.
        - **Built as** `python tools/generate_backdrop.py --header` writing
          `src/render/backdrop_layers.h`, from the same table that sizes the
          images. The Python is the one copy.
        - **It bought a second thing the bullet did not ask for, and it is the
          better half.** The header carries each layer's *generated size* as
          well as its factors, so `main.cpp` can compare what it actually loaded
          against what the generator would have produced and warn at startup.
          **The seam stops being a pixel at the far edge of the map and becomes
          a line on stderr.** Retiring the duplication removes the way the two
          sides come apart; the size check catches the case where they came
          apart before the header existed, or where somebody hand-edits
          generated work. The first is prevention and the second is detection,
          and it turned out to be cheap to have both.
        - **A cost this bullet never priced, found by adding a band: image size
          grows with parallax factor, so the *nearest* layer is the most
          expensive one.** Sky is 16 MB, mountains 20 MB, and a mid-ground at
          0.40 would be 32 MB — more than both together, in a repo that tracks
          its assets. `--sizes` prints the table. **This is a considerably
          stronger argument for V16 than "the duplication is annoying" was**: a
          wrapping layer has no size relationship to the pan range at all, so it
          retires a growing asset bill and not just a stale-constant hazard.
    - **A sixth thing, added 2026-08-11, and it is the layer list earning its
      keep on the first day it exists.** The light pass is drawn over
      *everything* between the clear and the reticle, so the boundary between
      "in the world and therefore lit" and "UI and therefore not" is currently a
      comment and a position in 350 inline lines. **Every item after this one
      adds a layer that has to declare which side it is on** — V16's animated
      bands are behind the world and must not pick up firelight from in front of
      it, and a flat-shaded or vector element under V13 has no business being
      tinted by the same crude additive wash as a 4x4 cell. Make it a field on
      the layer record. It costs a bool now and it is a rewrite of the composite
      order later.
        - **✅ Shipped 2026-08-16 as `frame::Lighting`, and it is three values
          rather than the bool this bullet asked for — but it is *narrower* than
          what the bullet wanted, not wider, and the reason matters.** `Lit`,
          `Light` (the boundary, and there is exactly one) and `Unlit`.
          **Writing it exposed that this bullet's own example is not currently
          representable.** V16's animated bands are asked to be *behind* the
          world and *not* tinted by the light pass — but the light pass is one
          additive full-screen copy, so anything drawn before it is tinted, full
          stop. There is no bool that delivers that; it needs the light pass to
          stop being a single wash, which is this item's tint bullet (step 3) or
          a render target.
        - **So the field says only what the composition can enforce, which is an
          ordering**, and two `static_assert`s over the table hold it: exactly
          one boundary entry, and no layer whose declared side disagrees with
          its position. **Verified by reordering the table and watching the
          build fail with the right message**, per the rule about testing a
          guard against the code it is meant to catch. A fourth enum value
          meaning "behind the world but unlit" would have been a field that
          lies, which is the exact failure `CLAUDE.md`'s first rule names — **a
          stated rule that has gone false is worse than no rule, and one that
          was never true is worse still.**
        - The honest consequence, written here because it is a thing step 3 now
          owes: **the sky and mountains are currently tinted by firelight**, and
          by V16's standard they should not be. That is a real defect of the
          single-pass architecture, it predates this item, and it is now
          *declared* rather than merely true.

    - **A seventh, and it is the bullet the reference actually asked for: a
      mid-ground band. ❌ Built 2026-08-16 and removed the same day, on the
      played-frame check its own source note demanded. This is the most useful
      thing in the whole item and it is a deletion.**
      [notes/reference_observations.txt](notes/reference_observations.txt) entry
      4 finds a distinct band between "distant mountains" and "the ground the
      character is on" in five of eight reference frames, carrying the most
      silhouette detail and doing most of the depth work — while our stack goes
      from mountains at 0.15 straight to the world at 1.00 with nothing in
      between. A slot was built at 0.40/0.16.
        - **Entry 4 wrote its own disproof condition, it was checked, and it
          fired.** "A simulated world might fill that band with terrain by
          itself — our world is 1080 cells tall and the camera sees 270 of them.
          Worth checking against a played frame before building a band that
          terrain was already occupying." It went out as checklist step 11 and
          came back **no gap: the terrain already fills it.** The row came out;
          the entry is marked disproved rather than deleted, per the
          documentation rule about keeping wrong predictions beside their
          corrections.
        - **The reason it did not transfer is the transferable part.** Five of
          eight reference frames really do carry that band and it really is
          doing their depth work — the observation is right about *them*. What
          does not cross over is the **mechanism**: a painting has to author
          that band because nothing else will put anything there, and we
          simulate 800 cells of real ground into the same space.
          **`reference_observations.txt`'s own header names this constraint in
          advance** — "anything taken from these frames has to survive being
          composed by a simulation instead of by an artist" — and entry 4 is the
          first observation to fail exactly that test. **The lesson filed at the
          note: ask what in the reference is doing the work, not just what the
          work is.** "A mid-ground band carries the depth" is a result; "a human
          paints one because nothing else would" is the mechanism, and the
          mechanism is what tells you whether it transfers.
        - **It also mis-aimed the depth problem, and that is worth separating
          from the above.** Entry 2 measured our bands at sky 0x14–0x28,
          mountains 0x20–0x30, trees 0x18–0x59, terrain 0x1B–0x78 — near-total
          overlap, which is *why* a busy frame reads flat. A mid-ground drawn
          from today's palette sits in the same value range as the mountains
          behind it, so it would have added a band without adding depth. **Depth
          here is a renderer problem before it is a layer-count problem**, and
          the fix is the tint bullet's multiply (step 3). A band was very nearly
          shipped as the answer to a question it could not have answered.
        - **What the episode proves about this item, and it is the strongest
          evidence V11 has.** The claim admitting V11 was "adding a band is one
          entry rather than surgery between two comments". **The first thing
          that actually happened was a band being *removed* in one row** — the
          table, one draw function, one factor, one fixture texture, in and out
          inside a day. The expensive direction was always going to be changing
          your mind, and that is the direction that got measured. **The golden
          checksum returned to V17's exact value**, `0x3d729ad7fbcaa839`, which
          is a second and independent proof that everything else in V11 composes
          the identical frame the inline code did at V8. **Step 3 then moved it
          for the first time on purpose**, to `0x9d9e92a81c4df07b`, using the
          same procedure a third time: the whole grade mechanism was built at
          identity and run against the old number, which held, before the one
          real value went in. That procedure is now the house style for this
          repo, and it is stated as a rule at V17's entry.
        - **The cost that was never priced, and it survives the deletion as an
          argument for V16.** Layer image size grows with parallax factor, so a
          *near* band is the most expensive layer in the stack: the mid-ground
          would have been 32 MB, more than the sky and mountains together, in a
          repo that tracks its assets.
        - *Reopen trigger:* a location whose terrain does **not** fill the band
          — a flatter scene with a lower horizon than F4's snowbank, or a
          zoomed-out camera once `Camera::SCALE` is runtime, which is this
          item's own remaining bullet. Recorded at the layer table in
          `frame.cpp`, at `generate_backdrop.py`, and at entry 4.
    - **V12 and V13 are the same item type and should be taken as one block with
      this one.** All three are the cost of changing the visual direction rather
      than a direction; all three edit the same two destination rectangles in
      `main.cpp`; and V11's runtime `Camera::SCALE` and V13's per-asset scale
      are the two factors of one multiplication. Done separately, those
      rectangles are rewritten three times and the middle version is wrong in a
      way that only shows on one asset.

- [ ] **V12 — The asset layer: alpha, and more than one format.** *Observed:*
  transparency in this project is one exact colour, keyed at load
  (`load_art_texture` in [main.cpp](src/main.cpp), `COLOR_KEY` in
  [tools/pixel_art.py](tools/pixel_art.py)), so **nothing in the game can have a
  soft edge, a partial-opacity pixel or a fade**, and every drawing has to avoid
  `#FF00FF` or it acquires holes — a failure mode [ASSETS.md](ASSETS.md)
  documents because it has happened. `SDL_LoadBMP` is the only decoder and
  `load_art_texture` is the only loader, called four times with four
  hand-managed `SDL_DestroyTexture` calls at shutdown. *Unlocks:* V13's
  smooth-filtered art, V14's rotated parts, `S1`'s masked body, and per-sprite
  opacity through `SDL_SetTextureAlphaMod`, which SDL has always offered and
  nothing here has ever called.
    - **Rotation is the reason this comes before V14 rather than after it.** A
      colour-keyed edge is binary by construction: the key becomes alpha 0 or
      alpha 255 and there is nothing between. Rotate that through
      `SDL_RenderCopyExF` and every diagonal is a staircase of hard-edged 4x4
      blocks, which is precisely the "rotated parts stop sitting on the lattice"
      objection [notes/procedural_animation.md](notes/procedural_animation.md)
      says is a taste call checkable in an afternoon. **Checking it against
      colour-keyed art answers a different question than the one being asked**,
      and answers it in the wrong direction.
    - **This crosses the zero-dependency line and that is a decision, not an
      implementation detail.** `ENGINEERING_NOTES.md` records PNG losing to BMP
      at F4 and the UI library losing at V10, both under "zero new dependencies
      until a specific need can't be met without one". **The F4 verdict does not
      bind here and the reason is worth stating rather than assumed:** it was
      about a *test fixture*, where the need was "read an image" and BMP met it.
      The need now is alpha, which BMP cannot carry at all — the format's 32-bit
      variant is not what `read_bmp` accepts and not what SDL hands back.
      `stb_image.h` is a single vendored header with no build-system entry,
      which is the cheapest possible form of crossing the line, and the crossing
      goes in `ENGINEERING_NOTES.md` next to the entry it is departing from.
    - **The trap, and it is the one that would quietly undo V6.** The locked
      palette and `tools/validate_palette.py` only understand BMP. Allow PNG
      everywhere and half the art silently stops being checked, which is exactly
      the state V6 exists to end — and it would present a year later as "the
      palette drifted", with nothing able to say when. **The boundary is a rule,
      not a convention:** BMP-with-colour-key stays the format for everything
      inside the locked palette — terrain, props, the player sheet — and
      PNG-with-alpha is only for assets deliberately outside it, each one
      listed. `assets/sprites.txt` is where that list already wants to live,
      since it is the file that says what each key loads from.
    - **Built as** one small `TextureCache` in `src/render/` that owns load and
      destroy, keyed by path. Today the prop cache is a
      `std::vector<std::pair<std::string, SDL_Texture*>>` in `main.cpp` that
      doubles as the destroy list, which works and does not generalise: every
      new kind of asset after it adds a member, a load site and a matching line
      in the shutdown sequence, and the failure mode of forgetting the third is
      invisible.
    - *Verify.* An asset with a genuine alpha gradient loads and composites; the
      existing colour-keyed assets are byte-identical on screen (this item must
      change nothing that ships today); `validate_palette.py` refuses to be
      pointed at a file the boundary rule says it does not cover, rather than
      passing it vacuously.

    > **In plain terms.** *(days — new, take with V11)* Transparency today
    > is one exact shade of magenta, swapped for "invisible" when the image
    > loads. **So nothing in the game can have a soft edge, a semi-
    > transparent pixel, or fade in and out** — and any drawing that happens
    > to use that shade gets holes in it. There is also only one loader,
    > called four times, with four matching cleanup lines at shutdown that
    > are invisible to forget.

- [ ] **V13 — Sprites carry their own resolution.** *Observed:* "one BMP pixel
  is one world cell" is not enforced anywhere and is assumed everywhere —
  `Prop`'s destination rect and the player's are both native size times
  `Camera::SCALE`, taken straight from `SDL_QueryTexture`. **So the only way to
  draw a more detailed sprite is to make it a bigger object**: a character drawn
  at twice the density is a character twice as tall. *Unlocks:* varying pixel
  resolution inside one scene, which is the first of the four goals above; a
  detailed protagonist over chunky terrain; and flat or vector art at any
  density, rasterised to a texture at load, which needs nothing new in the
  renderer.
    - **Built as** a `pixels_per_cell` field on the manifest record in
      [src/scene/sprites.h](src/scene/sprites.h), plus the filter mode
      (`nearest` or `linear`, already a per-texture setting via
      `SDL_SetTextureScaleMode` and already used for the light texture) on the
      same row. The format already carries an optional `[frame_w frame_h]`, so
      this is a third optional field and the parser's shape does not change.
      **It belongs in the manifest and not in code for the reason `sprites.h`
      already gives about the frame table pointing the other way:** how many
      cells tall a character is, is a fact about the game; how many pixels the
      drawing spends on each of those cells is a fact about the file, and the
      file is what gets swapped.
    - **Four traps, three of which are silent.** The prop planting scan in
      `main.cpp` walks `prop.w` columns of terrain looking for ground — that is
      a count of *world cells* and would be twice the footprint it draws.
      `player_sprite.h`'s two `static_assert`s compare `FRAME_W`/`FRAME_H`
      against `Player::WIDTH`/`HEIGHT`, which are cells; at 2x they compare
      pixels against cells and **pass while meaning nothing**, which is worse
      than failing. `build_player_sheet.py`'s "exactly 14 px wide, 26 or fewer
      tall" becomes a statement about cells that the tool measures in pixels.
      And the loud one: the terrain cannot participate at all, because its
      resolution *is* the simulation's — one `uint32_t` per cell in
      `Grid::pixels`. **Mixed density therefore has a floor and the floor is the
      world**, which is a design consequence to accept deliberately rather than
      meet on the first asset.
    - **Sequenced with V11 for the arithmetic rather than for tidiness.** V11
      makes `Camera::SCALE` a runtime value; this makes the per-asset factor it
      multiplies. Every draw site touches both, and the intermediate state where
      one is data and the other is a constant is the one where a zoom and a
      dense sprite disagree by exactly the factor nobody is tracking.
    - *Verify.* A 2x-density prop occupies the same world footprint as its 1x
      version and plants at the same y; the player sheet at 1x is
      pixel-identical to what ships today; and a deliberately mismatched sheet
      is *refused* rather than drawn sliced, which is the failure `main.cpp`'s
      existing sheet-size warning already exists to catch and which this item
      gives a second way to produce.

    > **In plain terms.** *(days — new, take with V11)* "One pixel in the
    > file is one square in the world" is assumed everywhere and enforced
    > nowhere: both the props and the player compute their on-screen size
    > straight from the image's size. **So the only way to draw a more
    > detailed sprite is to make it a bigger object** — a character drawn at
    > twice the detail is a character twice as tall. This is the item that
    > buys mixed pixel resolutions and sprite sizes, which is the first of
    > the four goals.

- [ ] **V14 — A part rig: rotation, and attachment from a table.** *(this is P1
  in [notes/procedural_animation.md](notes/procedural_animation.md), which is
  the design work and is not repeated here)* *Observed:* `player_anim::State`
  produces a row and a column, so the whole articulation available to the figure
  is which of nineteen drawn frames is showing. The aiming arm V3.1 built to fix
  exactly this was pulled, and its own entry records that **what killed it was
  the hotspot image, not the draw** — every frame drawn afterwards had to carry
  a marker pixel or `--validate` refused it. *Unlocks:* continuous aim without
  multiplying frame counts, head and limb motion the sheet cannot express, and
  the answer to whether rotated parts read at 4 screen pixels per cell — which
  V15 depends on and which nobody has looked at.
    - **The whole difference from the pulled version is where the attachment
      point lives.** A part attaches at a body-space offset per animation frame
      index, held in a table beside `ANIMATIONS` in `tools/player_sheet.py` and
      emitted into the generated `player_sprite.h`. That is a dozen numbers in
      the file that already generates numbers, against a parallel BMP the artist
      must not forget to paint. **The tax that killed the arm is a tax the sheet
      imposes and a table does not**, and that sentence is the entire case for
      this item over simply rebuilding V3.1's arm.
    - **The rotation trap has been paid for once and must not be paid again.**
      `SDL_RenderCopyEx` mirrors the texture and *then* rotates the quad, so
      flipping the sprite does not mirror the angle — it needs a 180° offset,
      and the naive `180 - angle` reflection is correct for a cursor level with
      the shoulder and inverts the vertical everywhere else. **Both of the cases
      anyone checks by eye are the two it gets right.** It is written down at
      V3.1 and it is written down here because this is the item that walks into
      it.
    - **The solver stays SDL-free and produces transforms, exactly as
      `player_anim` produces a row and a column and `LightField` produces a
      buffer.** `main.cpp` turns a transform into an `SDL_RenderCopyExF`. That
      is what makes it testable in `anim_test`, and V3.1's entry is the argument
      for why that matters here more than anywhere: **a rig's failure modes all
      look like art problems**, and a part attached one cell off reads as "the
      arm is drawn badly" rather than as a table being wrong.
    - **This does not retire `--validate`.** The sheet body survives, so the
      three checks that catch bugs presenting as physics — empty bottom row, gap
      inside the collision box, blank declared frame — all still apply to it.
      **That is the whole reason this is the cheap step and V15 is not.**
    - *Verify.* A part tracks a target through all four quadrants with the
      figure facing both ways (the trap above, tested rather than eyeballed);
      the body loop is unchanged with no parts declared; and the
      rotation-legibility question is answered in writing, with a screenshot,
      because it is the input V15 is priced against.

    > **In plain terms.** *(week — new)* The figure's only articulation
    > today is *which* of nineteen drawn frames is showing. Pull 2–4 parts
    > out of the sheet — an aiming arm, the head, later a cape — and drive
    > them with rotations computed per tick. **This is the decomposition
    > V3.1 already argued for and already built once**, and the reason it
    > was pulled is the reason this version is different: the arm attached
    > at a marker pixel in a second image, so every frame drawn afterwards
    > had to carry one or the validator refused it. **Here the attachment
    > point is a number in a table** in the file that already generates
    > numbers. The tax that killed it is a tax the sheet imposes and a table
    > does not.

- [ ] **V15 — A skeletal rig, and feet that find the ground.** *(P2 in
  [notes/procedural_animation.md](notes/procedural_animation.md))* *Observed:* a
  drawn walk cycle assumes a flat floor, and **nothing in this game is a flat
  floor for long** — the terrain is per-cell, arbitrary and constantly destroyed
  by the one verb the game has. Wave 4's D6 and D7 are both the figure's
  relationship to ground it is not actually reading. *Unlocks:* feet that land
  on the terrain that is there, and a second character for the price of a
  re-pose instead of a second sheet — which is the thing `S1` needs and the
  reason this item exists now rather than later.
    - **Admitted on a trigger written in advance, and that is the unusual
      part.** `notes/procedural_animation.md`'s closing section names three
      things that would admit a rig; the third is *"a second character type is
      committed to → admits P2 on amortisation, and that is the case where
      'overhaul' stops being the wrong frame."* One has been. **The note also
      says what to do about the other two arguments and it still holds:** the
      slope-aware-feet case remains unproven by any playtest, so it is a reason
      this item is *shaped* the way it is, not the reason it is admitted.
    - **It must replace `--validate`'s three checks, and nothing currently
      does.** Under a rig there are no frames, so an empty bottom row, a gap
      inside the collision box and a blank declared frame all become
      unexpressible — and each of those exists because it catches a bug that
      **reads as a physics problem rather than an art one**. The rig equivalents
      are checkable and have to be written as part of this item, not after it:
      the lowest part touches the baseline in every reachable pose, no pose
      leaves a gap through the silhouette inside the collision box, and no part
      names a sprite that does not load. **A rig that ships without these buys
      expressiveness by making a class of bug invisible again.**
    - **The clock is the fixed step and any solver with state makes that
      stricter rather than looser.** A spring's stiffness and an IK smoothing
      rate are constants that silently mean something different at every frame
      rate — worse than the walk cycle V3.1 caught, because a wrong-speed cycle
      is visible and a wrong-stiffness limb just looks slightly bad.
      `player_anim.h`'s clock note covers this and is the thing to read first.
    - **Deliberately not P3.** Verlet or spring limbs remain the most expensive
      option on that page with nothing observed in play asking for them, and the
      note's own correction — that the "build it twice" deferral behind E5 is
      void since E5a keeps matter in the grid — makes them *unblocked* rather
      than *due*. **Held on cost, and the deferral states a price rather than
      claiming impossibility**, which is the form E8's entry settled on.
    - *Verify.* A figure standing on a two-cell step has both feet on terrain
      rather than one in the air; a figure walking across ground the player has
      just dug does not slide; the determinism suites are untouched, because
      none of this may reach `src/physics/`; and the three replacement checks
      fail on art that would have failed the originals.

    > **In plain terms.** *(weeks — new, and **moved behind the playtest
    > gate on 2026-08-11**)* A drawn walk cycle assumes a flat floor, and
    > **nothing in this game is a flat floor for long** — the terrain is
    > per-square and the one verb the game has is destroying it. This is
    > feet that land on the terrain that is actually there, and a second
    > character for the price of a re-pose instead of a second hand-drawn
    > sheet.

- [ ] **V19 — The seven-band scene, and a ground plane where the reference has
  water.** *(new 2026-08-16, admitted by request)* *Observed:* the request was
  for a scene composed the way `CnC_parallax_*` is composed, built before the
  split-view path rather than after it. **This item is admitted by that request
  and not by the reference**, which matters because
  [notes/reference_observations.txt](notes/reference_observations.txt)'s own
  bound says reference answers "what is possible" and never "what is wrong here"
  — it cannot admit an item, and entry 7 says so at the top rather than being
  read as if it had. *Unlocks:* the first scene in this project composed as a
  depth stack rather than as
  sky-plus-mountains-plus-whatever-the-simulation-put-there, and the composition
  V18's split view is a modification of.
    - **The reference frame is seven layers and we ship two.** Entry 7 walked
      one column and segmented it: sky, far range, mid range, near ridge, shore
      treeline, the ground plane, and a near silhouette. We have `backdrop_sky`
      and `backdrop_mountains`, then the simulated world. **The mapping is not
      one new layer per missing row**, and working that out is most of this
      item:
        - sky → `backdrop_sky`, exists, regrade only
        - far range → **new**
        - mid range → `backdrop_mountains`, exists, already at 0.60 against a
          reference-implied 0.63
        - near ridge → **new**, and it lands in the band V11 built and deleted
          (see the trap below)
        - shore treeline → **new**, same band, same trap
        - ground plane → **new, and it is the item's centre of gravity** —
          *built 2026-08-16 as step 4b; the rest of this entry's bullets on it
          are marked with what came back*
        - near silhouette → **already filled by the simulated world and its
          props.** The reference's foreground rock is a painting the boat passes
          behind; ours is diggable terrain. **Do not author a painted band in
          front of the world** — it would occlude the one verb the game has.
    - **Art is cheap here on purpose and that is a scope decision, not a
      placeholder excuse.** One colour and one or two shades per layer, rough
      silhouettes, generated by `tools/generate_backdrop.py` exactly as sky and
      mountains already are. **Generated layers conform to `PALETTE` by
      construction**, which is the property the palette deferral (2026-08-12)
      explicitly kept for this half of the pipeline. Nothing here needs a drawn
      asset, so nothing here is blocked on V12's alpha channel.
    - **What the water layer actually does, since it is the thing being ported
      to land.** Entry 7 separates four mechanisms, and the split is the useful
      part because only three survive the water being removed: the value ladder
      above the horizon (0.78 per band, compounding); **the horizon being the
      darkest line in the frame** (row-mean luminance bottoms out at the
      waterline, 69 against 156 at the top and 140 at the bottom); contrast
      growing with nearness, with 87 of the frame's levels spent between the
      plane's near edge and the silhouette on it against 14-45 at every join
      further back; and a texture gradient, the plane's marks widening 1.3x-3.0x
      toward the viewer. **The reflection is the one that does not transfer**,
      and it was measured anyway (compressed to ~0.6 vertically, contrast cut to
      44%, lifted +61, desaturated) so that a later reader can tell which of the
      plane's properties we chose to drop.
    - **The land plane brightens toward the viewer for a reason that is not the
      reflection**, and this is the substitution the item turns on. The
      reference's water is bright near the camera because it mirrors the sky; a
      land plane cannot do that. But **a horizontal surface faces the sky and a
      vertical silhouette does not**, which is why moonlit ground reads brighter
      than the trees standing on it — same result, different mechanism, and it
      holds at night when the sky is the only source. That keeps mechanism 2
      intact: the plane still meets the horizon at low contrast and still
      diverges toward the viewer, so the dark pinch survives the water going
      away. **Stating the mechanism rather than copying the appearance is the
      rule entry 4's deleted mid-ground band was bought with** — the result
      transfers only if the mechanism does.
    - **The plane needs per-row parallax and that is the one piece of new
      rendering.** A receding plane has no single depth, so it has no single
      parallax factor; drawn flat at one factor it reads as a wall behind the
      world rather than as ground going away. For a ground plane, screen row
      distance below the horizon goes as inverse world distance, so **the scroll
      factor is linear in the distance below the horizon** — `f(y) = f_far +
      (f_near - f_far) * (y - y_horizon) / (H - y_horizon)`, which is arithmetic
      a strip loop can do in integers. Built as N horizontal strips, one
      `SDL_RenderCopy` each, source row height shrinking with distance so
      mechanism 4's texture gradient falls out of the same relation instead of
      being authored twice. **No custom blend mode, no render target, no
      shader** — so this does not spend any of the three named escape hatches
      either, which is now the second item in a row to have looked like it
      would.
        - **Built 2026-08-16 as step 4b, and the prediction above was right
          about the mechanism and backwards about the arithmetic.** The
          relation, the strip loop, the integer arithmetic and the escape-hatch
          claim all held; `backdrop_wrap::plane_strip` is the built form of the
          formula in the line above it. What is wrong is "source row height
          shrinking with distance" — it *grows* with distance, because a far
          strip spans a huge range of world distance in a handful of screen
          rows. The mechanism named in the same sentence (the reference's marks
          widening 1.3x-3.0x toward the viewer) is what the geometry produces
          and what shipped: measured on the built strips the near strip is
          magnified 3.46x against the far strip's 1.06x. **The sentence was
          wrong, not the mechanism**, and it is corrected rather than deleted
          because the shape is the one this project keeps meeting — a prediction
          that names the right effect and inverts the relation that causes it.
          `tests/test_backdrop.cpp`'s property 6 is what a linear mapping fails,
          and it was verified against one.
        - **One cheapness taken deliberately: only the vertical is scaled.** A
          true plane shrinks a mark in both axes; the tile's width is constant
          across every strip. Scaling it per strip is free arithmetically and
          looks worse — adjacent strips would tile at different widths, their
          phases would diverge, and the mark pattern would stair-step at all
          twenty-four strip boundaries. Twenty-four visible seams bought to fix
          a foreshortening nobody can see on a night-dark texture.
        - **`GROUND_STRIPS` is the item's one unpriced cost and it is unpriced
          for a reason worth stating.** The plane issues 24 strips times their
          tiling copies every frame, against one draw call for every other band.
          *Verify* below asks for both halves of the frame-budget rule — and
          **`grid_bench` times `Grid::update` and `Run::step` and cannot see a
          draw call at all**, so neither half can reach this. The synthetic rows
          and the replayed row are both blind to it. The honest instrument is
          the frame rate in the running game, which is checklist step 11, and
          this bullet exists so the next reader does not go looking for a bench
          row that cannot exist.
    - **The grades come out of the ladder and are a derivation from measurement,
      not a measurement.** Entry 7's compounded column gives 1.00 / 0.74 / 0.63
      / 0.46 / 0.37 for the five bands and 0.45 → 0.80 across the plane. Those
      are ratios against the reference's sky and our sky is a night sky at L 26,
      so they are a *shape* to author against and every one of them is a
      `TUNING.md` row with a dated History line the first time it is touched.
      **The corroboration worth keeping:** our mountains were set to 0.60 from a
      luminance measurement against our own art, and the reference's same band
      compounds to 0.63 — two independent routes to within 5%, which is the
      strongest evidence the ladder is a real thing and not a property of one
      painting.
        - **The plane's own two numbers came back inside 2% of the ladder**
          (2026-08-16). Authored with the 0.45 → 0.80 ramp baked into the tile
          and placed by a 0.53 grade, `assets/backdrop_ground.bmp` measures
          **0.44 → 0.81 of our sky**, at an internal ratio of 1.84 against the
          reference plane's 1.78. Its horizon edge lands at luminance 11.7,
          under the graded mountains at 16.9 and under the darkest sky row at
          18.1 — so **the horizon is the darkest line in the frame**, which is
          mechanism 2 surviving the water being removed. That is a derivation
          checked against our own art rather than a second measurement, and it
          is stated that way in the `TUNING.md` row.
        - **The ramp is in the art and the level is in the grade, and the split
          is forced rather than chosen.** A `Grade` multiplies uniformly, so it
          cannot make a surface brighten toward the viewer; only the tile can.
          What the grade does is place the band. Anyone reaching for the grade
          to fix a plane that reads flat is reaching for the wrong knob.
    - **One band gets the warm accent and the rest stay cool.** Entry 8 found
      the sky at 0.0% warm pixels and the shore treeline at 47%, with the warm
      population held at a near-constant luminance (96-104) while the cool
      population descends 174 → 73. So the accent **crosses** the ladder instead
      of obeying it. Applied here that is one band — the treeline — authored
      warm at a fixed value, everything else on the cool ramp. It is entry 6's
      hue-isolation trick at band scale and it is why our player is already cool
      against a warm world; **this inverts that locally and the two must be
      checked against each other**, because a warm treeline and a warm world is
      the player's isolation spent twice.
    - **Trap 1, and it is the one that can waste the whole item: two of the five
      bands land where V11 built a band and deleted it the same day.** The near
      ridge and the treeline sit in the mid-ground, and on 2026-08-16 a
      0.40/0.16 band went in there and came out on checklist step 11 because
      **our terrain already fills that space** — 1080 cells of world into a
      270-cell viewport. The reopen trigger recorded at the time is "a location
      whose terrain does not fill the band, or a zoomed-out camera once
      `Camera::SCALE` is runtime." **This item must fire that trigger before
      authoring anything into the band, and the check is one screenshot**: where
      does the terrain's skyline sit in the target scene? If it fills the band
      again, the honest outcome is a five-layer scene rather than a seven-layer
      one, and the deletion stands twice instead of being quietly reversed.
    - **Trap 2: the reference cannot supply a parallax factor and it nearly
      supplied one anyway.** Entry 1 established the three frames are three
      generated lakes rather than one pan — region shifts non-monotonic in depth
      and opposite in sign at the two edges. So every new factor here is a
      **stated derivation**, labelled as one everywhere it appears, exactly as
      the deleted band's 0.40 was. Parallax is inverse depth, so a geometric
      ladder between the two factors that already exist is the defensible
      construction: 0.04 → 0.08 → 0.15 → 0.28 → 0.52 → 1.00 is that ladder at a
      ratio of about 1.9, and **it lands the existing mountains at 0.15 without
      moving them**, which is a check on the construction rather than a
      coincidence to lean on.
    - **Trap 3: the composition being copied is a horizon composition and this
      game is a side-on cross-section.** The reference looks *across* a plane to
      a horizon; our camera looks at a wall of cells. The plane therefore cannot
      be our terrain — **it is a backdrop band drawn behind the world**, ground
      receding behind the play plane, and the terrain stays the near silhouette
      in front of it. This is the whole reason the mapping above has seven rows
      and only five new ones.
    - **Trap 4, and it is a decision rather than a hazard: the reference's plane
      is brighter than what stands on it, and our world layer is at grade 1.0.**
      Plane near edge 138 against foreground rock 52 is the frame's largest
      contrast and it runs in that direction. Ours would run the other way
      unless the world row takes a grade below the plane's — which is coherent
      (the table's order is Lit → Grade → Light → Unlit, so a darkened world is
      lit back up by fire and lamps rather than through them) and is also a
      change to how the play area reads. **Do not decide it inside this item's
      implementation.** Either the plane sits low enough that they do not
      compete, or the world gets a grade and that is its own row in `TUNING.md`
      with a playtest against legibility while digging.
    - **The cost is real and it is the best argument V16 has ever had.** A layer
      must cover the window plus the pan range at its own factor, so the nearer
      the band the larger the file: `--sizes` prices 0.08 at 17.3 MB, 0.25 at
      24.8 MB and 0.40 at 32.3 MB, against sky's 15.9 and mountains' 20.3 today.
      **Five new pan-sized layers roughly triples the asset directory.** The
      plane in particular is the nearest band and the worst case, and a plane is
      exactly the kind of texture that tiles — so **V16's wrapping layer may
      have to be pulled into this item rather than following it**, and that is a
      sequencing question to answer before authoring, not after. `python
      tools/generate_backdrop.py --sizes` before committing to any factor.
        - **Answered 2026-08-16: pulled in, and the price it paid off is larger
          than the estimate above.** `backdrop_ground.bmp` is a 256x256 tile at
          **0.2 MB**, against the 32 MB the same band costs priced flat — the
          plane alone is the whole of V16's argument recovered. `--sizes` now
          labels a wrapping row rather than printing a number that would be
          wrong, and `layer_size()` carries a refusal saying a wrapping row must
          not be run through it. The arithmetic landed at step 4a
          (`backdrop_wrap.h`, thirteenth suite, no source set linked at all) and
          the draw path at 4b.
    - **Two mechanical consequences that are expected and not breakages.** The
      golden checksum moves, and the new value goes in the same commit as the
      change — the house procedure applies in full: build every new layer at
      identity, run it against the current number, and only then put the real
      grades in, so the move has exactly one cause. And the layer table's three
      `static_assert`s hold the rank order; adding five `Lighting::Lit` rows
      must not disturb "exactly one light pass, at most one grade, and the grade
      row's own grade is identity."
        - **The house procedure had to be adapted at 4b, and the adaptation is
          the part to copy for the remaining three bands.** "Run it against the
          current number" is what the four previous uses did, and **a new band
          that draws pixels has no such half** — a layer that composes to the
          old checksum is a layer that is not in the frame, and leaving its
          texture null to arrange that is precisely the anti-pattern
          `.claude/rules/simulation.md` names. So the separation was bought with
          **two numbers instead of one**: the plane at identity gave
          `0xfd8e2f04b7037278`, and the grade then took it to
          `0x24eb769681836a0e`. The first number is the geometry, the step
          between them is the grade, and both are written into the constant's
          comment because the intermediate is not recoverable from the file
          afterwards. The `static_assert`s held untouched, as predicted.
    - *Verify.* The terrain-fills-the-band question is answered by screenshot
      **before** any band is authored, and written down whichever way it comes
      back; the frame composes byte-identically with every new grade at
      identity, before the grades go in; the per-row plane scroll is priced on
      both halves of the frame-budget rule — P4's replayed row for p99 and
      steps-over-budget, and the bracketed synthetic rows at 1920x1080, because
      N strips a frame is a per-frame cost the played row is too quiet to see;
      the parallax factors are labelled derivations in the header comment and in
      `TUNING.md`; and README checklist steps 11 and 12 are both owed — 11
      because the layer table changes shape again, 12 because the whole item is
      a retune of the thing step 12 exists to watch.

    > **In plain terms.** *(week — new 2026-08-16, admitted by request, and
    > it is item 8's step 4)* Build a scene composed the way
    > `CnC_parallax_*` is composed, before the split-view path rather than
    > after it. **The reference frame is seven layers and we ship two**;
    > measured in
    > [notes/reference_observations.txt](notes/reference_observations.txt)
    > entry 7.

- [x] **V20 — The value ceiling, and the two defects V19 4b shipped.** *(done —
  full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#v20-the-value-ceiling-and-the-two-defects-v19-4b-shipped))*
- [x] **V21 — The ceiling comes back down a fifth, and a stale hand-copy goes
  with it.** *(new and done 2026-08-16, admitted by playtest)* *Observed:*
  [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 7 answered V20's checklist item 1
  **"too bright"** — the one answer that item was explicitly written to invite.
  Three of its five items closed outright (mountains visible, walking bands
  gone, frame rate fine), so V20's two defect fixes are confirmed and only its
  aesthetic swing overshot. *Unlocks:* nothing on its own; it settles the ladder
  that V19 4c's three bands would be authored against, which is why it goes
  first.

    **This is a ceiling move of the second kind and nothing else.**
    `.claude/rules/assets-and-formats.md` states the test: a change that alters
    the ratio between two bands is the refused move and belongs in a grade; a
    change that scales every band and leaves the ratios intact is a ceiling, and
    no grade can perform it. V21 multiplies the backdrop palette group by
    **0.80** and touches nothing else. `mountains` 0.60 and `ground` 0.53 are
    untouched for the second consecutive retune — which is the argument for
    keeping the ladder in the grades, made twice in one day.

    - **Post-grade the frame now reads sky 76 → 50 top to horizon, mountain rim
      57, mountain body 35, ground far 24 ramping to 62.** Every one of those is
      exactly 0.80 of V20's. `star` is deliberately not scaled: it is a point
      accent carrying no ratio, and the reference's night frame holds its stars
      and moon at full value while everything around them descends.
    - **The factor is 0.80 rather than lower, and the reason is the one number
      V20 existed to buy.** A ceiling is a multiply, so it scales every
      *absolute* separation — and absolute separation is precisely what V20
      learned to hold when matching the reference plane by ratio bought 9.8
      levels. The mountain/ground horizon join is the tightest and is the
      reference's own signature at **14 levels**; at 0.80 it goes to 11, at 0.72
      to 10, at 0.60 to 8 — under the reference's smallest single band join and
      most of the way back to the flat frame session 6 reported. **The ceiling
      is nearly out of downward room.** Recorded so the next session does not
      reach for a smaller factor by reflex: a second "too bright" is a
      hue-and-saturation item (this group is a strongly saturated violet, and
      saturation reads as brightness) or a grade item, and either is a different
      argument.
    - **The report was "too bright" and not "washed out", and the item offered
      both.** Kept because the discrimination is the whole justification for a
      uniform scale: the complaint is level, not separation and not hue, and a
      uniform scale is the only move that answers level without disturbing the
      other two.
    - **A stale hand-copied constant, found while measuring and fixed in the
      same commit.** `draw_clear` held `0x14, 0x10, 0x22` — luminance 18 — under
      a comment reading `sky_deep, tools/pixel_art.py`. **Both halves had gone
      false.** V20 raised the group and did not carry the change here, and V20
      had *also* inverted the sky ramp, which made `sky_deep` the brighter of
      the pair while the comment's stated intent is "the palette's darkest sky
      tone" — now `sky_horizon`. Corrected to `sky_horizon` at the V21 level.
        - **It was invisible, and why it was invisible is the point.** The sky
          texture covers the window, so the clear only shows through a missing
          or unreadable BMP — the exact failure the clear exists to make
          survivable. **A constant that only matters in a failure path will not
          be caught by anything watching the success path.**
        - It is still hand-copied and that is now the **second** duplicated
          backdrop constant to go stale silently, after V11's parallax factors.
          Generating it into `backdrop_layers.h` is the fix; deferred to
          ENGINEERING_NOTES.md rather than done here, because a retune that
          quietly grows a code path is how a measurement stops being bracketed.
    - **The golden checksum did not move, and that is the correct result rather
      than a miss.** V21 is outside its coverage in both halves: the fixture
      **generates its own textures instead of loading `assets/`**, so no palette
      edit can reach it, and the clear is fully overdrawn by the sky.
      `0xcde4dc1a39927fca` stands. **Stated positively because the inverse of
      the V20 trap is just as dangerous** — there, an unchanged checksum was a
      silent defect; here it is a proof of scope. The distinction is whether the
      unchanged value was *predicted before the run*, and this one was.
    - *Verify.* Full suite green (13/13). **Not verified and owed: the entire
      visible effect of the item.** No test in the project can see either change
      — one is asset colour the fixture does not load, one is a failure-path
      constant. The brightness re-check is item 1 of the owed list in
      MANUAL_TESTING.md and it is the only instrument there is.

- [x] **V23b — The camera goes back to centre, and the mechanism goes with
  it.** *(2026-08-17, hours after V23a)* *Observed:* playtest session 9, a
  direction rather than a result: *"lets go back to the camera always being
  centered."* *Unlocks:* nothing. It removes an item's worth of code and hands
  V22 back a constraint it had been relieved of.

    **What shipped is a deletion.** `src/game/camera_bias.h` and
    `tests/test_camera_bias.cpp` are gone; `Camera::set_vertical_anchor`,
    `vertical_anchor()` and the `anchor_y_` member are gone; `main.cpp` no longer
    updates a bias per frame. `Camera::follow`'s vertical expression is character
    for character the pre-V23 one, and the golden checksum went back to
    **`0xcde4dc1a39927fca`** — the value it held before V23, not a new number.
    **That the checksum returned to an earlier value is the evidence the revert
    is complete**, and it is worth expecting the next time `git log -S` on that
    constant lists the same value twice.

    - **Removed rather than neutralised, and that was the decision in this
      item.** The alternative was to leave the mechanism with both anchors at
      0.50 — a smaller diff that keeps the option warm. It was refused because
      this project's recurring failure is a stated rule that stopped matching the
      code: TUNING.md would have gone on listing three knobs, the checklist would
      have gone on describing a framing, and every one of them would have been
      describing behaviour nothing produces. **A knob nobody turns is worse than
      no knob, because it reads as a supported way to change the game.**
    - **The suite count is unchanged at 14.** The Camera half of the retired
      suite is now `tests/test_camera.cpp` — the centred framing, all four
      world-edge clamps, the A1 fractional split and the parallax endpoints. The
      framing checks went with the framing. **The centring assertion matters
      more than it did**: it is now the whole of the shipped composition rule and
      nothing else states it.
    - **What this costs is V22's route, and the cost is exactly the measurement
      V23 was built on.** A centred camera caps the receding ground plane's
      visible share below the player at **~50% by construction**, and it measured
      20.2% at the spawn against a reference reading of "clearly past half". That
      measurement is untouched by the revert. **So V22 cannot reach its target
      framing by moving the player down the frame any more**, and its remaining
      route is the fixture scene — which session 8 had already argued is the
      load-bearing half.
    - **Three sessions, three directions, one day, and that is a result about
      this project's process rather than about the camera.** V23 was built from
      a measurement and three reference stills, V23a from the first human look,
      V23b from the second. **The reference reading was never wrong about the
      references** — what it lacked was any evidence about play, and the item
      shipped with that stated and shipped anyway. The reusable form: when a feel
      item's whole evidence base is still images, its first playtest is not a
      verification step, it is the experiment, and the item should be sized as
      one that might be deleted.
    - **What is not settled and must not be quietly re-litigated.** Whether a
      non-centred framing is right *at all* was never separately answered — it
      was asked for, delivered wrong, corrected, and withdrawn. The two things
      the next attempt would otherwise rediscover are kept at `Camera::follow`:
      an anchor belongs on the camera and not at the caller (it is called twice
      per rendered frame and applying it at one site tears the backdrop), and a
      framing is a fraction of the viewport, never a count of cells.

- [x] **V23a — The dig framing was never delivered, and the report that caught
  it.** *(2026-08-17, the day after V23)* *Observed:* playtest session 8, the
  V23 feel report V22 was gated on. *Unlocks:* nothing new — it repairs V23 and
  re-owes the same report.

    **The finding is not a feel result and that is what makes it worth reading.**
    The tester's words were that the illusion "might be upside down", which is a
    third failure the checklist did not offer — it asked whether the camera read
    as *answering the dig* or as *wandering*. Checked against the code, the sign
    is right: `SURFACE_ANCHOR` 0.80 does draw the player low. **What was wrong is
    that the dig framing was requested and not delivered.** `Camera::follow`
    clamps the view at `world_h - viewport_h` = 810, so a `DIG_ANCHOR` of 0.30
    resolved to **0.51 on screen at the fixture floor** (row ~948) and to **0.70
    at row 1000**. The camera answered the dig *least* where there was most world
    below to see and most where there was least — **an inversion of the item's
    intent produced entirely by the clamp**, and a move that visibly starts and
    does not arrive.

    **The lesson generalises past this item and is the reason for the new test.**
    V23 asserted the constants and the arithmetic that chooses between them; it
    never asserted what a player is shown, because the clamp lives one call away
    in `Camera`. **A composition constant has to be tested as delivered, through
    `follow`, not as requested** — `camera_bias_test` now pins the on-screen
    fraction at the fixture floor, and that check fails against V23's code, which
    is how it was verified.

    **What shipped.** `DIG_ANCHOR` 0.30 → `COLUMN_ANCHOR` 0.50 —
    **renamed, because it gained a second trigger**: being airborne now takes the
    same framing whatever the cursor is doing, which session 8 asked for in the
    same breath ("digging or flying out of frame") and which V23 had no notion
    of. A falling player previously kept the surface framing and spent its ~55
    cells in under a second, which is the likeliest thing behind "flying out of
    frame". Four checks were written first and failed for the right reasons.

    - **The tester's number and the reference reading converged, and the
      convergence is not a coincidence.** 0.50 is what the report asked for, and
      0.50 is also what the clamp was already delivering at the surface (0.511).
      **The tester was describing the framing the game actually produced in the
      one place they could see it**, and V23's 0.30 was a number that existed
      only in the constant. Recorded because it is a reusable shape: when a
      report and a measurement disagree with a constant, check whether the
      constant is reaching the screen at all.
    - **`EASE_PER_SEC` did not move and its meaning did.** The swing is 0.30
      rather than 0.50, so the same 0.85 now crosses it in **0.35 s where the
      checklist asked the tester about 0.6 s**. The speed question is therefore
      still open and session 8's answer to it cannot be reused — flagged in
      MANUAL_TESTING.md rather than quietly re-asked.
    - **One consequence reasoned about and not measured, stated so the re-test
      can find it.** A jump is airborne, so the camera now leans on every hop.
      A hop is shorter than the swing, so it should read as a lean; **if it
      twitches, the fix is to trigger on descending rather than on airborne**,
      which is a different feel and not a retune of this one.
    - **What this does not touch.** The ground plane. Session 8 answered
      question 3 **no** for the fourth time, which is a finding about V22's
      premise and is written up there, not here.

- [x] **V23 — The camera leaves centre, and digging brings it back.** *(done —
  full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#v23-the-camera-leaves-centre-and-digging-brings-it-back))*
- [ ] **V24 — The plane's near edge is nailed to the window.** *(new
  2026-08-18, from playtest session 10; **ahead of V22 part 2**)* *Observed:*
  [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 10, reported against a question
  that was not asked: *"the entire .bmp stays on screen and squishes as the
  sprite flies up. this does not make sense."* *Blocks:* V22 parts 2 and 3, and
  V19 4c behind them.

    **The report is exactly right and the arithmetic is four lines.**
    `draw_ground` in [src/render/frame.cpp](src/render/frame.cpp) builds its
    `backdrop_wrap::Plane` with `horizon_y` from `ground_horizon_y` — the
    mountains' vertical factor, 0.06 — and `bottom_y` from `window_h`, **a
    window constant**. So the plane's far edge is parallaxed and its near edge
    is not, and `plane_strip` distributes the tile's whole 256 rows across
    whatever band is left between them. Measured over the camera's full
    vertical travel at 1920x1080, `mountain_h` 1642:

    | `view_fy` | horizon | band | px per texel |
    |---|---|---|---|
    | 810 (spawn, bottom clamp) | 428.6 | 651.4 | 2.54 |
    | 400 | 527.0 | 553.0 | 2.16 |
    | 0 (top of world) | 623.0 | 457.0 | 1.79 |

    **The tile never leaves the frame at any camera height and is compressed 30%
    vertically across the climb.** That is the report, in numbers, and it is a
    rescale rather than a scroll — which is why no seam test and no checksum
    ever caught it.

    - **The defect is an inverted depth ordering, and stating it that way is
      what makes the fix obvious.** The near edge's *effective* vertical
      parallax factor is **0.0**, because it is pinned to the window; the far
      edge's is 0.06. **The nearer end of the plane is less parallaxed than the
      further end**, which is the one thing a parallax stack may never do. Every
      other layer in the frame is ordered correctly and this one is inverted
      inside itself.
    - **`backdrop_layers::GROUND.parallax_y` is 0.11, is generated, and is read
      by nothing.** The only consumers of any `parallax_y` are
      `draw_backdrop_layer` (sky and mountains) and `ground_horizon_y`, which
      deliberately takes the **mountains'** 0.06 instead — V20's correction, and
      it is right: the plane's far edge is at infinity, so its factor has to be
      the smallest in the scene. What was never noticed is that this leaves the
      plane's *near* edge with no vertical factor at all, and the code supplied
      a window constant in its place. **A generated constant with no consumer is
      how a stack gets built with a rung missing**, and `main.cpp` prints this
      number in its mismatch warning, which makes it look live.
    - **Why it goes ahead of V22 part 2, and this is an ordering claim rather
      than a severity one.** V22 part 3 tunes the value junction where the
      world's surface meets the plane's near end. **That junction currently
      moves and rescales with camera height**, so a grade tuned to make the two
      read as one surface is tuned at one camera position and wrong at every
      other. Part 2 authors the terrain that meets it. **Both are authoring
      against a target that moves** — the same rule step 6 and 4c were each
      re-ordered for, arriving a third time. Fix the geometry, then author
      against it.
    - **It is also the first ground-plane report that is not about the plane at
      rest.** Sessions 6, 7, 7b and 8 all asked what the plane *is*; this asks
      what it *does*. Worth keeping as a shape: four rounds of value work went
      into a layer whose motion nobody had checked, and the thing that surfaced
      it was V22 part 1 moving the camera, not any of the work aimed at the
      plane.
    - **✅ The geometry decision came back the same day: (b), constant scale.**
      Two candidates, both of which fix the inversion and which look different
      in motion. **(a) Give the near edge its own vertical factor** — the ladder
      in `tools/generate_backdrop.py` derives y as ~0.4x, so 0.52 gives ~0.21 —
      and let the band grow as the camera climbs. That is honest perspective for
      a horizontal surface seen from higher up, and it magnifies the tile's
      marks about 75% over the world's height. **(b) Hold the depth mapping
      fixed** — constant screen pixels per texel — and let the whole band
      translate and clip, which is what every other layer in the frame does.
      **(b) was chosen, and the reason is worth keeping**: the report's
      complaint is that the layer *resizes*, and (a) resizes it too — just for a
      better reason. A fix that answers a rescale with a different rescale is
      betting the tester objected to the arithmetic rather than to what they
      saw. **Do not re-derive (a) from the shipped constant**; it is recorded at
      `PLANE_TEXEL_SCALE` so that a later reader can find it without
      rediscovering it.
    - **What sits below the near edge, which both candidates had to answer.**
      Freeing the near edge from the window is what creates the case: the plane
      is a fixed depth of art now, so a low camera or a tall window can leave
      rows underneath it. At 1920x1080 that is at most 11 px at the world's
      floor; at 3440x1440 it is a few hundred, because the horizon derives from
      the mountains' art and does not grow with the window. **It is filled with
      the tile's nearest row** — what is below the near end of a receding plane
      is ground nearer still, and the nearest thing the art knows is its last
      row. **Not left to the clear colour on the grounds that the terrain covers
      it**, which it does at the spawn and does not promise anywhere else.
    - **What the fix costs.** Both golden checksums move, in the same commit as
      the change. It does **not** touch the scene, so **no `.rec` recording is
      invalidated** — this is not part 2's cost arriving early. A playtest is
      owed once it is built, and it is a *motion* check, which no step of the
      Manual Tester Checklist currently is: step 11 covers seams and stair-steps
      on the parallax strips at rest.
    - **▶️ Built 2026-08-18.** `PLANE_TEXEL_SCALE` 2.5 and `plane_geometry` in
      [src/render/backdrop_wrap.h](src/render/backdrop_wrap.h); `draw_ground`
      calls it instead of building the `Plane` inline. Golden checksum
      `0x77404ada6ab4d08f`, overlay `0x57a56d7aa8cc6bf0`, both in the same
      commit. **The signature is where the fix is stated**: `plane_geometry`
      does not take the window height, so the defect is not merely fixed but
      unrepresentable — a layer's geometry is a fact about the layer, and the
      window only decides how much of it you can see.
    - **The failing test came first and it is a property, not a case**
      (`backdrop_test`, property 9). Three things that must not change as the
      horizon moves: the band's height, the scale in pixels per texel, and that
      the near edge translates exactly as far as the far edge. **Stated that way
      rather than as a formula**, because a test that restates its
      implementation checks nothing — all three hold for any value of the
      constant. It was run against the unfixed code first and failed on all
      three; the intermediate step that made that possible was a
      behaviour-preserving `plane_geometry` that still took `window_h` and
      handed it back.
    - **`PLANE_TEXEL_SCALE` is 2.5 because that is the composition already
      shipped**, not because it was chosen: the spawn band was 651.4 px over a
      256-row tile, 2.544 px per texel. **So this is a defect fix at the old
      look**, and the only visible change is what the plane does while the
      camera moves — which is what makes the playtest it owes answerable.


- [ ] **V22 — The plane the player is in.** *(new 2026-08-16, **unblocked the
  same day**; retitled from "What the receding plane is made of" when the
  decision came back)* *Observed:* [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 7
  answered "does the plane recede" with **"no"**, and supplied the reading that
  explains why every previous attempt to fix it by shading failed: in
  `resources/images/CnC_parallax_1..3.jpg` **the reference's receding plane is a
  lake, and the boat floats on it.** *Unlocks:* V19 4c and 4d, which tune bands
  against a junction this item moves. **Nothing is built yet.**

    **The decision came back land, and the title change is the answer.** *"i
    want the first scene to be land not water, but i still want the land layer
    to act like the water layer with the player sitting on/in the plane."* The
    three options this entry offered each answered two questions at once — *what
    is the plane made of* and *where is the player relative to it* — and the
    answer separates them, taking land for the first and the lake's relationship
    for the second. **So the item is no longer about the plane's material at
    all**, which is why it is retitled: leaving the name pointed at the material
    is how an item quietly gets built to its own obsolete premise.

    - **▶️ Started 2026-08-18, in full, and the first thing done was the thing
      this entry says cannot be reached without.** The item was put up for
      re-scoping first, because the bullets below add up to a target the
      shipped constraint set forbids: two thirds needs the player off screen
      centre, and V23b deleted the mechanism for that by playing. **The answer
      was to run V22 in full**, which is a decision to reopen V23b, and it is
      recorded here rather than in the camera's own history because it is V22's
      premise that spends it.

      **Three parts, and the order is not the one session 8 recommended.**
      Session 8 said to sequence the fixture-scene rewrite first, and it said so
      while the camera was gone. With the camera back on the table the cheap
      thing decides the expensive thing's target, so:

      1. **The framing.** `Camera::VERTICAL_ANCHOR` 0.80 in `Camera::follow`.
         **Done 2026-08-18.** One constant, no easing, no trigger — the moving
         anchor is what two playtests rejected and none of it returns. Golden
         checksum `0xf29c435ed9d923b1`, overlay `0x6527211c3b5d3bb2`, both in
         the same commit as the change. `camera_test` asserts the framing in
         *screen* terms, which is what V23a proved a `view_y()` check cannot do.
      2. **The fixture-scene rewrite.** The load-bearing half, and the one that
         costs both `.rec` recordings.
      3. **The world/plane value junction**, which is the deliberately-open
         TUNING question this item consumed. `Params::world_grade` already
         exists at identity, so the knob is built and unset.

      **Why part 1 first, stated because it contradicts a written
      recommendation.** This entry's own arithmetic says no scene edit can move
      the contact point, because the scene does not decide where on the screen
      the player is drawn — the camera does. Authoring a scene against a
      junction position before fixing the junction position is authoring against
      a target that is about to move, which is the mistake 4c was held back to
      avoid and the mistake step 6 exists because of. **The same rule, third
      time: compute the target's reachability before authoring against it.**

      **What part 1 does not do, and it must not be read as having done it.** It
      moves the contact point from 20.2% to ~65% of the band *arithmetically*.
      The plane's visible share in front of the player is still **exactly
      zero**, because the world occludes it — that is part 2's job and no camera
      constant touches it. **A framing that frames nothing is what V23 already
      shipped once**, and the fourth "no" is the evidence.

      **What it costs is unpaid.** ~55 cells of world below the player at the
      surface, against digging as the game's one verb. V23 paid that back with a
      dig framing and that framing was rejected; this does not pay it back at
      all. **It is owed to a playtest and it is on `MANUAL_TESTING.md`.**

    - **⚠️ The gate answered "no" for the fourth time, on 2026-08-17, and this
      is the first thing to read before spending a week here.**
      [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 8 was the V23 feel report this
      item was gated on, and its third question — does the plane finally read as
      receding — came back **no**, in session 7b's exact terms: *"the character
      should be standing within the ground plane. currently the player is
      standing on the test albedo.bmp setup with the backdrop."* **V23 was the
      first attempt aimed at the cause the geometry actually had, it landed, and
      the answer did not move.** That is the finding, and it is a finding about
      this item's premise rather than about V23: **camera framing was necessary
      and is not sufficient either.**

      What it does *not* license is re-opening the closed decision — the plane
      stays land, and what transfers from the reference is the relationship.
      What it does say is that **two mechanisms have now each been proven
      necessary and insufficient in turn** (value continuity, then framing), and
      the one thing all three reports have named and nothing has yet changed is
      **the fixture scene itself**. Session 8 named it by its file for the first
      time, `albedo.bmp`. The fixture-scene rewrite was already this item's
      second half and was already the expensive one; **this is the evidence that
      it is the load-bearing half, not the dressing.** Sequence it first.

      **The cost of that has not changed and must be flagged before it is
      spent:** the rewrite costs both `.rec` recordings and takes P4's replayed
      row dark until the tester plays and presses `F9`.
    - **Value continuity is necessary and it is not sufficient, and that
      correction arrived hours after the bullet below was written.**
      [PLAYTEST_LOG.md session
      7b](PLAYTEST_LOG.md#session-7b--2026-08-16-the-plane-the-player-is-not-touching)
      reported that the player is not touching the plane at all — they are on
      the fixture scene's terrain, and the plane's near portion is not *visible*
      at spawn. **There is therefore no junction to tune.** The bullet below
      assumed the two surfaces meet at a line whose values could be brought
      together; they meet at the ragged silhouette of a test fixture, and no
      grade reconciles a surface with an object standing in front of it. The
      bullet is kept because everything in it stays true once the surfaces do
      meet — it was the right mechanism attached to the wrong stage of the
      problem.
    - **`CnC_lighthouse.jpg` was named specifically and measuring it produced
      the number this item was missing** —
      [notes/reference_observations.txt](notes/reference_observations.txt) entry
      9. The horizon sits 59.7% down, the boat's waterline 85.2% down, and the
      plane runs to the bottom of the frame: so **64% of the plane is behind the
      figure and 36% is in front of it.** The figure is not at the plane's near
      edge looking across it, it is two thirds of the way down it. That is what
      makes the plane recede *around* the figure, it is geometry rather than
      shading, and it is the reason three rounds of value work went nowhere. The
      second measured fact costs more: **between the boat and the horizon there
      is nothing but plane** — no clutter, no vertical feature except a
      lighthouse island deliberately pushed to one side.
    - **The third-in-front lands on the *world*, not on the backdrop, and that
      is what makes this expensive.** The plane in front of our player is not
      backdrop at all — it is the simulated, diggable world the player already
      stands on. So the requirement is that the terrain at spawn presents a flat
      open surface running to where the backdrop plane takes over, with the
      contact point around two thirds down the combined band. **This is the
      item's real cost and it was not visible when the direction was settled**:
      nothing about it is a rendering change.
    - **The two-thirds is not reachable by editing the scene, and the arithmetic
      says so before anybody redraws. (2026-08-17.)** The plane runs
      `ground_horizon_y` to the *bottom of the window*, unconditionally — it is
      never partly drawn, so "the plane is not visible at spawn" is the world
      **occluding** it, not the plane failing to reach.
      `generate_test_scene.py`'s `FLOOR_TOP = s(380)` is world row 950 of 1080
      and the slab is full width, so everything below the terrain skyline is
      world and the plane's share in front of the player is **exactly zero**,
      not a small number. Now the part that kills the cheap fix: at spawn the
      camera is at its **bottom clamp** (world 1080, viewport 270, so `view_fy`
      pins at 810), the horizon lands at screen 428.6 and the player's feet at
      560, putting the contact at **20.2% of the band**. `Camera::follow`
      centres strictly, with no vertical bias — so **anywhere the camera is
      unclamped the player is pinned at screen centre, and the contact can never
      exceed ~50% of the band by construction.** A surface two thirds down the
      band requires the player ~80 cells *below* screen centre. **No scene edit
      can produce that, because the scene does not control where on the screen
      the player is drawn — the camera does.**
    - **Which turns the item into a camera question, and the camera question has
      a cost that points the other way.** A vertical bias of ~80 cells puts the
      contact at 65.3% of the band (`view_fy` 735, horizon 446.6, feet at screen
      860) and — unlike a scene edit — it holds as the player traverses instead
      of only at the spawn. But 860 of 1080 is the player at **80% down the
      screen**, leaving ~55 cells of world visible below them. **That starves
      digging, which is the game's one verb**, and it is the *same* collision
      the near-foreground-silhouette refusal is about, arriving in a new form:
      the reference composes a scene you look **across**, and this is a world
      you dig **down** into. The two want opposite thirds of the frame. **A
      decision is owed on that trade and it is not one to settle inside this
      item** — filed in ROADMAP_ITEMS.md.
    - **That decision came back the same day: match the reference, and let
      digging move the camera. Built as V23 above.** The recommendation in the
      row was to split the difference at 20-30 cells, and it was **not** taken —
      worth recording as a wrong call rather than quietly dropping, because the
      reason it was wrong is instructive. It treated the two framings as
      endpoints of one axis to compromise along, and looked for the least-bad
      fixed point on it. The answer was that the framing does not have to be
      fixed at all: the reference's own three frames put their subject at 0.60,
      0.36 and 0.27 down, and hold their composition constant underneath. **A
      trade between two states is only a trade if you have to pick one**, and
      that is the question this entry never asked. The bullet above stands
      otherwise — the ~80-cell number and the ~55-cell cost were both right, and
      the second is exactly what the dig framing exists to pay back.
    - **It collides head-on with F4.4, and the collision is the finding rather
      than an obstacle to route around.** F4.4 deliberately made the first scene
      *a test fixture wearing art* — uneven stairs for step-up, fence posts for
      dig-the-base collapse, a pit with pillars for the flood fill, a water
      channel, jump ledges. Every one of those is a vertical feature standing
      between the player and the horizon, on a jagged surface, at the spawn.
      **The first scene now has two jobs and they are incompatible at the same
      location:** exercise every system where the player starts, and present an
      unbroken plane where the player starts. Nothing is wrong with either job.
      **A decision is owed on which one the spawn serves**, filed in
      ROADMAP_ITEMS.md, and the cheap answer is that the fixture keeps its job
      somewhere the camera does not open on.
    - **The mechanism is value continuity across one junction, and not a change
      of draw order.** The plane stays behind the world and still ends at the
      bottom of the window. What makes the player *in* it is that the world's
      surface has to read as the plane's near end rather than as a different
      object standing in front of it — the frame's one large junction that has
      never been tuned, because the world row sits at grade 1.00 and the plane's
      at 0.53, so today the near end of the plane is *darker* than the ground in
      front of it and the reference runs the other way. **This is why the fix is
      reachable at all**: the composition-order problem the entry below names is
      real, but it is soluble by value because the plane is already behind the
      world at the place it matters.
    - **It therefore consumes the deliberately-open TUNING question rather than
      sitting beside it**, and that has to be stated because the two were filed
      as independent. "Does the world row take a grade below the plane's?" was a
      look-and-see left open at 4b; it is now the load-bearing knob of a
      scheduled item. **It keeps its own row and its own playtest** — a grade on
      the world changes how the play area reads while digging, which is exactly
      the reason it was not settled inside 4b — but it stops being optional and
      stops being postponable.
    - **Reflection stays dropped and the replacement was written before the
      fact**, at entry 7: the lake is bright near the camera because it mirrors
      the sky, and land cannot, but a horizontal surface faces the sky where a
      vertical silhouette does not — the same brightening for a different
      reason. Land reflecting mountains would be the appearance without the
      mechanism, which is the deleted mid-ground band's mistake exactly.
    - **Changing the fixture scene invalidates both recorded sessions, and that
      is the consequence to state out loud before anyone redraws anything.**
      `src/game/input_log.h` names "the fixture scene changes" as the *first* of
      the three things that invalidate a log, and `tests/test_scene.cpp` pins
      `FIXTURE_SCENE_CELLS = 334901` specifically so this fails in `ctest`
      rather than in a benchmark nobody runs on the breaking commit. **So P4's
      replayed row — the only instrument in the project that proves the frame
      budget is intact on a real frame — goes dark until somebody plays and
      records again, and only the tester can do that.** The guard working as
      designed is the good news; the cost is real and it is a human's afternoon,
      not a build step. `tests/bench_grid.cpp` loads the fixture in two places
      and `rim_probe` in one, so those go quiet too until the new scene exists.
      **The scene is generated** (`generate_test_scene.py`), which is the one
      piece of cheap news: a new scene is a new generator, not pixel work.
    - **The refusal against a near foreground silhouette is untouched, and this
      item must not be cited as having bent it.** That refusal is about paint in
      front of the world occluding digging. Nothing here draws anything new in
      front of the world; if a later step reaches for that, it is a fresh
      argument and this entry does not supply it.

    *(Everything below is the entry as written while the decision was open, kept
    because the reading in it is what produced the answer.)* **This reopens a
    decision that was made deliberately the other way** — V19 is titled "the
    seven-band scene, with land where the reference has water" — so it is a
    direction change and not a defect, and it is not settled inside an item
    about something else. That rule already exists here for the world-grade
    question and applies identically.

    - **The mechanism we did not build is reflection.** The reference's plane
      shows every band standing above it a second time, mirrored and
      value-compressed. That is where its depth comes from. Of its three cues we
      built exactly one: the ripple dashes growing toward the viewer. **This is
      the same lesson as the deleted mid-ground band — ask what in the reference
      is doing the work.** Three sessions have now tried to buy that plane's
      recession with values, and values were never what it was made of.
    - **The second difference is occupancy, and it is the one the tester
      named.** The boat sits at roughly the plane's vertical midpoint with plane
      both above and below it. Ours is drawn *behind* the world with the player
      standing in front of it on terrain. **A band the player is in front of
      cannot recede around them**, which is a property of the composition order,
      not of the art.
    - **Three options are on the table and the cheapest is respectable.** (1)
      Leave it — ours is a digging game on solid ground and the plane stays a
      backdrop band. (2) Reflections only — keep the plane where it is and
      mirror the mountains into it; buys the missing cue without touching the
      world or the scene, and is reachable from the existing draw path. (3) The
      water plane — a new starting scene, the player on or beside water; reaches
      `src/scene/`, the world and possibly the fluid sim, and is much the
      largest.
    - **Option 3 is the one to price honestly before anyone agrees to it**,
      because the reference is a *painting of a boat on a lake* and ours would
      be a simulation with a diggable world in it. The refusal already recorded
      against a near foreground silhouette applies with the same force: a
      painted plane in front of the world occludes the one verb the game has.

- [ ] **V16 — The backdrop moves.** *Observed:* `backdrop_sky` and
  `backdrop_mountains` are two static textures whose only motion is the parallax
  offset, so **the backdrop's sole depth cue stops the moment the player does.**
  A wooded hillside at night is motionless: nothing drifts, nothing sways, no
  star varies. That is a thing in the built game that reads badly, which is
  precisely the question V8's remainder cannot answer and the reason this is a
  separate item from it rather than part of it. *Unlocks:* depth that survives
  the camera standing still, and the third of the four goals above.
    - **Built on V11's layer list and it is why that list comes first.** A layer
      gains an optional function of the step count — a scroll rate, a short
      frame sequence, a tint from V11's grading term — and stays one
      `SDL_RenderCopy`, or two for a layer that wraps. **No new coordinate
      system**, which is the constraint V8 already priced this whole layer
      against.
    - **The clock is the fixed simulation step and not the rendered frame**, for
      the third time in this document: V3.1's walk cycle ran at nearly 3x speed
      at 165 Hz, V10.1 is warned about the same thing, and a drifting cloud is
      the same shape. **It presents as an art problem in every one of the three
      cases**, which is what makes it worth writing down again rather than
      assuming it is known.
    - **A wrapping layer retires the parallax duplication rather than
      documenting it**, and that is a real second payoff. The seam at the pan
      limit exists because each layer's size has to be derived from the camera's
      pan range at that layer's factor — two constants in two languages with
      nothing enforcing agreement. A layer that tiles has no size relationship
      to the pan range at all. V11 generates the header because that is the
      afternoon available today; **this is the version where the failure mode
      cannot occur**, and the two are not redundant because the header is what
      keeps the existing hand-sized art correct in the meantime.
    - **The bound, stated because this is the easiest item here to let grow.**
      Motion in the backdrop is drawn, non-simulated, interacts with nothing,
      and is not V9 — [notes/art_direction.txt](notes/art_direction.txt)'s
      drawn/simulated split is the test, and a moving thing that a player could
      expect to interact with has failed it. Weather that lands on terrain is a
      simulation feature and is not this item.
    - *Verify.* The backdrop reads as alive with the player standing still —
      which is the observation, so it is the check; motion is identical at 60
      and 165 Hz, measured rather than assumed; and a missing backdrop BMP still
      degrades to the flat clear colour, since the `if` around each layer is
      what keeps a failed load from showing two-frames-ago garbage.

    > **In plain terms.** *(week — new)* The two backdrop layers are static
    > images whose only motion is the parallax shift, so **the backdrop's
    > sole depth cue stops the moment the player stands still.** A wooded
    > hillside at night is motionless: nothing drifts, nothing sways, no
    > star varies. That is something in the built game that reads badly
    > *today*, which is the question V8's remainder cannot answer — which is
    > why this is a separate item from it rather than part of it.

- [x] **V10 — The in-window UI layer: a dig reticle and a material hotbar.**
  *(done — full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#v10-the-in-window-ui-layer-a-dig-reticle-and-a-material-hotbar))*
- [ ] **V10.1 — Screen shake and hit-stop.** *Observed:* split out of V10, which
  shipped its two UI halves without them. *Unlocks:* the impact half of E6 — an
  explosion that moves the camera reads as force, and the same effect applied to
  E3's fractures is what V9's dust is compensating for the absence of.
    - **They have a real trap.** Both are game feel rather than art, and the
      fixed timestep makes them easy to get wrong: a shake driven off the
      rendered frame and a simulation driven off the fixed step must not share a
      clock, or the effect changes with frame rate. That is the same class of
      bug F1 and F2.3 spent two sections retiring, and it would be reintroduced
      by a feature nobody thinks of as simulation. **V3.1 hit this exact trap
      and its entry is the worked example** — a walk cycle off the rendered
      frame ran at nearly 3x speed at 165 Hz and presented as an art problem.
    - **Sequenced with E6 rather than before it**, because a shake with nothing
      worth shaking for is tuned against the dig tool and then re-tuned against
      the first explosion.

    > **In plain terms.** *(days — alongside E6)* Camera shake and a brief
    > freeze on impact make explosions read as force. The trap: driven by
    > the fixed simulation clock, not the display refresh, or their speed
    > changes with frame rate.

### P — Performance

Running order: **~~P2~~ → P4 → P1 → P3.** P4 comes first because it is an
instrument and the other two are judged with it. P1 sits **directly after E5a**,
which is where this section always said it went — the master running order used
to contradict this and was corrected on 2026-08-11.

*Running order: **~~P2~~ → ~~P4~~ → P1 → P3** — `P4` was added on 2026-08-11 and
goes first of the remaining three because it is an instrument and the other two
are judged with it; `ROADMAP_ITEMS.md` has carried that order since, and this
line said `P2 → P1 → P3` until P4 shipped on 2026-08-13. It is not the numbering
— see the note on stable IDs above. **P2 is now the first item in the whole
plan, ahead of the E track and ahead of the session 5 playtest**, which is a
change from it merely being first within this section: it is half a day, and
every item in the new order carries a bracketed measurement that would otherwise
be quoted against a world a quarter the size of the one actually being played.
Taking it after E10 means measuring E10 twice. P1 is the layout change and wants
the field set settled, which means after E5a. P3 is new and is last because it
is the largest of the three and the only one that changes how the sweep is
structured.*

- [x] **P2 — Re-baseline the benchmark at the size the game actually runs.**
  *(done — full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#p2-re-baseline-the-benchmark-at-the-size-the-game-actually-runs))*
- [x] **P4 — A benchmark scenario that is a real frame.** *(done — full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#p4-a-benchmark-scenario-that-is-a-real-frame))*
- [ ] **P1 — Split the cell array hot from cold.**
  [PERFORMANCE.md](PERFORMANCE.md) establishes that `cascading` — the stated
  number to watch — is **bandwidth-bound**, and `ENGINEERING_NOTES.md` prices a
  12→2 byte cell at roughly 6x the hot loop's memory traffic while rejecting it
  outright as mutually exclusive with authored per-cell colour. **Splitting the
  array captures most of that win without giving up authored colour, which is
  the reason to reach for this one first:** `type` / `updated_tag` /
  `temperature` / `piece_tag` are read for nearly every awake cell every step,
  `color` is touched only when a cell is written and when the frame is uploaded.
  Two documents call that trade impossible; this is the thing that makes it not.
  Measured bracketed (on, off, on) with `churning` as the control, per the rules
  in that file — and **recorded honestly if it does not pay**, because both of
  the previous entries filed as cheap wins failed and were left on the record
  rather than deleted.
    - **This item has gained consumers since it was written, and they are the
      reason it is sequenced after E5a rather than before.** `element.h` records
      that E3 spent the last free byte: the struct is 12 with nothing spare, so
      the next field costs 500 KB at the target resolution and a wider stride.
      **The field set this has to be settled against changed on 2026-08-09 and
      it changed in P1's favour.** E10 and E5a add no field at all — ~~they
      claim `Element::ticks`, which is already in the struct and unused for
      non-structural, non-`Fire` cells~~ **they add three fields into the
      alignment hole between `type` and `color`, which the sitting on 2026-08-13
      found had been unused since the struct existed** *(the `ticks` plan is
      withdrawn; see E5a and `element.h`)* — so `Element` stays at 12 bytes
      either way and the hot/cold split is against the same twelve it was always
      going to be. **The split does change, though: the three new bytes are
      hot** — a velocity is read on every awake cell — so E5a moves the hot side
      from eight bytes to eleven against `color`'s four, which makes the ratio
      P1 is trying to improve *worse* rather than leaving it alone. That is an
      argument for P1, not against E5a, and it is the sharpest version of "the
      layout wants to be settled against the final field set" this entry has
      had. What the layout must still accommodate is `temperature`, V7's
      downsampled light buffer, and eventually E5b's air field, both of which
      are separate low-resolution grids rather than anything in the cell array.
      **The "sparse particle list" this bullet used to name no longer exists** —
      that was the old E5 design and P1 was waiting on a structure that is not
      being built.
    - **The prediction is already written down in
      [PERFORMANCE.md](PERFORMANCE.md) so it can be graded rather than
      reinterpreted afterwards**, including the part that matters more than the
      win: if `churning` improves as much as `cascading`, the explanation is
      wrong even if the numbers are good.

    > **In plain terms.** *(week)* The simulation is limited by how fast
    > data can be pulled from RAM. Most of each cell's data is read every
    > tick; the colour is only read when the cell is drawn. Separating them
    > means the hot loop reads less. **Sequenced directly after E5a** so the
    > layout is settled against the final field set. ~~*and possibly before
    > it, if the `ticks` decision comes back needing a thirteenth byte*~~ —
    > **that trigger is retired: the decision closed on 2026-08-13 needing
    > no new byte at all, so P1 stays where it is.**

- [ ] **P3 — Run the chunks in parallel.** *Observed:* the simulation is
  single-threaded — there is no `<thread>`, no task system and no parallel loop
  anywhere in `src/`, and every measurement in [PERFORMANCE.md](PERFORMANCE.md)
  is one core's. *Unlocks:* the headroom that E5b in particular will want, and
  it is worth naming next to P1 for scale: **P1 buys tens of percent and this
  buys a multiple.**
    - **The reference approach fits what is already built, which is the whole
      reason this is worth scheduling rather than dismissing.** Noita updates
      chunks in four alternating passes, arranged so that two chunks being
      updated simultaneously are never neighbours. That is what makes an edge
      write safe without a lock — a cell can only ever write into a chunk that
      is not currently being touched — and it is why the pattern preserves
      reproducibility: the pass order is fixed, so the same seed and input still
      produce the same world. **The 64×64 chunks, the per-chunk dirty rects and
      the "writes go through `swap_elements`" discipline are already most of the
      prerequisite**, and none of them was built for this.
    - **Determinism is the thing to prove rather than assume, and F1 is what
      makes proving it cheap.** Randomness is a stateless hash of (position,
      tick, seed, purpose) with no generator state, so nothing about thread
      scheduling can perturb a draw — which is a property F1.3 bought for a
      different reason and which turns out to be the precondition for this. The
      test is the one that already exists: same seed, same inputs,
      byte-identical world, now run at every thread count the build supports.
    - **Two things that will not survive the change unexamined:** the
      row-alternating sweep direction keyed on `y`, and `frame_tag`, which is a
      single grid-wide counter written by every visited cell. Both are correct
      single-threaded and both need to be re-derived per pass rather than
      ported.
    - **Sequenced last of the three.** It restructures the sweep, so doing it
      before P1 settles the layout means restructuring twice, and doing it
      before the played-size baseline exists means optimising against the wrong
      number — which is the mistake P2 exists to stop.

    > **In plain terms.** *(weeks — new)* The whole simulation is single-
    > threaded. The reference engine updates chunks in four alternating
    > passes arranged so that two chunks being updated at the same time are
    > never neighbours, which makes edge writes safe and keeps the result
    > reproducible because the pass order is fixed. **The 64x64 chunks and
    > dirty rectangles here are already most of the prerequisite.** Worth
    > naming next to P1 for scale: P1 buys tens of percent, this buys a
    > multiple. ---

---

## 🟡 Medium Term (Core Gameplay Loop)
*Tying the physics engine into the lore. This section is the v0.1 slice.*

*This list is longer than it used to be, and every addition came from an audit
rather than from an idea. Three items were previously absent even though the
Definition of Done depends on all three — a camera, a save file, and a way to
die. A fourth, the UI decision, was sitting in `ENGINEERING_NOTES.md` as a
warning with nothing scheduled to act on it. None of those were deferred
decisions; they were gaps, and a plan that hides its prerequisites is not a
plan. Assume the slice is at least twice the work the old five-line version
implied. Two of the additions have since moved out again, upward rather than
away: the camera is F3 and the level loader is F4, because neither was ever
slice content — they are what slice content stands on.*

***Engine & Visual Depth sits ahead of this section with one exception, and the
exception is new.** `S0` below is pulled out of this section and scheduled
between E4 and E10, for the reasons written at that item. Everything else here
still sits behind the engine tier, and the paragraph below is the argument for
that and is unchanged — it is worth reading with S0 in mind rather than replaced
by it, because the case for engine depth first was always about *depth*, never
about the slice having to be last.*

***The original argument, which still holds for the other seven items:** this is
a deliberate delay to the slice rather than an accident of ordering.* The engine
and how it looks are the selling point, so they are worth depth before the loop
is built on top of them — the argument in full is in that section's preamble and
in `VISION.md`'s Scope Discipline. Three consequences for the items below.
**Quantum Worlds and Objective + Extraction get a better engine to be built
against** — heat, levelling liquids, fracturing structures, thrown debris and
explosions are all things a level would otherwise have to be designed around the
absence of. **Player health has one of its two damage sources waiting for it**:
"what damages the player" is answered for free by E2, since fire that has a
temperature is fire the body can be asked to read, exactly the way collision
already asks what it is standing in. And **E6 makes the hook question answerable
by playing** rather than by argument — see that item, and see the two open
questions immediately below, which are the ones this section still owes an
answer to.*

**Two open questions this section has not answered, recorded here rather than
left to be discovered at the playtest gate.**

- **What the hook is.** `VISION.md` leaves this open on purpose and is explicit
  about why: naming it early would commit the design to whichever comparison got
  written down, and the Definition of Done currently cashes out "physics-based
  movement and destruction" as walking, jumping and one dig tool on a cooldown.
  That document is equally explicit that under-building looks exactly like
  discipline right up until the playtest gate. **E6 is the first thing in the
  plan that makes this a question you can answer by playing**, which is the only
  way this project has ever settled a design question. It still has to be
  *asked*, before the gate rather than at it.
- **Whether there is combat, and it is currently neither in nor out.**
  `notes/core_features.txt` names "physics-based combat and exploration" as a
  key mechanic. Nothing in this document schedules any combat, the Definition of
  Done does not mention it, and no item names its absence. That is the worst of
  the three available states: either combat is in the slice, in which case it is
  an item here and the slice grows by a real amount, or it is deliberately out
  of v0.1, in which case that belongs in writing next to the note's line.
  Deciding it costs a paragraph; discovering it costs a milestone.

**Both of those questions now have a due date rather than a gate, and `S0` is
why.** They were each written as "answerable by E6" and then left with nothing
scheduled to force the answer, which is how an open loop becomes a permanent
one. [ROADMAP_ITEMS.md](ROADMAP.md) carries them in a Decisions table with
a deadline against each: combat at the end of S0, the hook at the end of E6. A
decision with no deadline never closes, and this section has been carrying two
of them plus E4's for long enough to be evidence of that.

- [x] **S0 — The run can be lost.** *(done — full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#s0-the-run-can-be-lost))*
- [ ] **S1 — The enemy that granulates.** *(new 2026-08-11; the design is
  [notes/granulating_enemies.md](notes/granulating_enemies.md) Part B, path
  E-C)* *Observed:* **nothing, and that is stated rather than worked around.**
  There is no combat, so nothing in the built game is wrong for want of an enemy
  in the sense the E track means. **This item is blocked on the combat decision,
  which is due at the end of `S0`** — and the note it comes from says exactly
  that: the enemy *"becomes admissible when the Definition of Done's objective
  loop needs something to oppose the player."* *Unlocks:* the simulation being
  the combat system rather than a backdrop to one, which is the same argument
  `VISION.md` makes for the engine being the product.
    - **Built as E-C: a body, plus a `W×H` byte mask in body space.** Roughly
      364 bytes for a player-sized enemy, **per enemy and not per cell**, so
      `element.h`'s no-spare-byte constraint is untouched. Alive, the sprite
      draws *through* the mask and damage clears bits while spawning real `Grit`
      cells into the grid at those world positions — the sand pouring off a
      wounded enemy is genuine simulated powder that piles, gets wet and burns.
      Dead, the body deletes itself and writes its remaining mask into the grid
      as real `Crust`. **From that instant it is terrain, and E3 collapses the
      corpse for free.**
    - **This item is the consumer that reopens E4, and that is the most
      important thing on this entry.** E4 — does a body displace material — was
      closed **"no"** on 2026-08-10 by session 5, and the note this design comes
      from predicted the exact failure that decision now allows: *"the grit
      falls through the enemy that is producing it. The effect fails precisely
      at the moment it exists to be looked at."* **E4's "no" was decided on
      evidence gathered when nothing in the game depended on the answer**, and
      `ENGINEERING_NOTES.md` already says to re-ask at E5a. This is a second and
      stronger reason to re-ask, with a named consumer instead of a schedule
      position, and it is carried in [Decisions
      owed](ROADMAP.md#-decisions-owed) rather than left inside this
      bullet.
    - **Three things the note names that will be met on the first day**, kept
      here because each is cheap to design for and expensive to retrofit. *The
      mask is in body space, not sprite space* — a hole punched in the idle
      frame lands somewhere meaningless on walk frame 4, so it aligns to the
      collision box and every frame draws through it. *Damage location has to be
      real* — a hit that clears bits at the sprite's centre regardless of where
      you struck collapses the whole effect, and `Tool::aim_point`/`march`
      already resolve a world-space impact point to convert. *The corpse is
      gameplay, not decoration* — it can bury the player, block a tunnel or
      smother a fire, which is probably excellent and is a design consequence to
      accept deliberately.
    - **Its one new renderer requirement is a render target, and it needs no
      shader.** Drawing a sprite with holes in it is a per-enemy
      `SDL_TEXTUREACCESS_TARGET` texture — 14x26 at 1x — rebuilt only when the
      mask changes, which is on damage rather than per frame. **Nothing in this
      project has ever used a render target**, so it is worth naming as the one
      capability this item adds rather than discovering it mid-build. It depends
      on **V12** for a real alpha channel to punch, since a colour key cannot
      express a hole that is not exactly one colour.
    - **It depends on V15 for the reason V15 is admitted at all**, and the two
      should be read together: a rig is written once and re-posed, a sheet is
      drawn per character. An enemy built against a second hand-drawn sheet buys
      the animation twice and forfeits the amortisation that is V15's whole
      argument.
    - **What it is not:** not a generic actor framework. `ENGINEERING_NOTES.md`
      refuses an entity/component system on the grounds that *"there is one
      body, and there will be perhaps four things"* — **a second concrete body
      type is inside that budget and a system for arbitrary body types is the
      thing being refused**, and the difference is the whole of what keeps this
      item from becoming an engine rewrite.

    > **In plain terms.** *(weeks — new 2026-08-11)* **Blocked on the combat
    > decision, which is due at the end of S0**, and it does not get to pre-
    > empt it. An enemy whose body is a solid material that holds its shape
    > until it is damaged, at which point the damaged part turns to sand and
    > pours away — shoot its leg and its leg runs out onto the floor.

- [x] **Pick the UI layer, in writing, before anything needs one.** *(done —
  full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#pick-the-ui-layer-in-writing-before-anything-needs-one))*
- [ ] **Player health and death — the full version.** *(the thin half is `S0`
  above and **shipped 2026-08-14**; what is left here is everything S0
  explicitly does not build)* A real damage model rather than two hard-coded
  sources, more sources as the engine supplies them (E6 adds a third), and a
  death that is presented rather than merely applied. The dependency rule and
  the "not a field on `Grid`" constraint are stated at S0, held through the
  build, and hold for both halves.
    - **What S0 leaves this item, now that the thin half is real rather than
      planned.** The two sources are an `if` each in `Player::update` and that
      is correct at two; a third wants a table, and the moment to build one is
      the moment E6 supplies it rather than now. **The spawn-drop finding at S0
      is the thing to re-read before adding any source** — every new hazard has
      its own version of "something the world already did to the body for free
      is now charged", and the current one is spent on a single bool.
    - **`damage_this_step()` exists and nothing reads it.** It is the event half
      of the health number, built for the same reason `DigTool` reports the step
      a blow landed, and it is what a hit reaction, a sound or a screen shake
      each want. Kept because it costs a forwarded int; "presented rather than
      merely applied" is the item that spends it.
- [ ] **Playtest gate:** Put the slice in front of people who did not build it.
  Do not proceed past this line on the strength of your own opinion. Two things
  make this cheaper than it sounds and both are earned above: F1 plus F2.3 mean
  a bug report is a seed and an input log rather than a description, and the two
  open questions at the top of this section should have been asked *before*
  getting here rather than answered *by* getting here.

## 🔵 Presentation & Tooling (after the slice, before polish)
*Not slice-blocking — none of this makes the loop fun, and none of it should be
started while Engine & Visual Depth or Medium Term is open. Recorded here rather
than in the Long Term wish list (`VISION.md`) because these are concrete,
bounded, and genuinely expected to get built, unlike that list.*

**This section's rule has now been tested three times and has not bent.** The
scene loader left for F4, the art-pipeline visual work left for V1–V4, and V5–V9
were written directly into Engine & Visual Depth rather than here. In every case
the *item moved or was filed elsewhere* — nothing was ever built while still
sitting behind "do not start this yet". That is the only version of this rule
that survives contact with a schedule: a rule with one exception has no force
left, so the answer to "this needs to happen sooner" is to argue the item into a
different section, in writing, or to leave it alone.

### Sandbox / debug tooling
*Dev-facing, not player-facing — these speed up iterating on the physics engine
itself rather than polishing what ships. Each is small and self-contained in
`main.cpp`/`grid.h`, so the risk of scope creep is low, but they still wait
behind the sections above like everything else here.*

**One of these has stopped being purely a convenience and is worth promoting
when the time comes.** V2 could not be verified in the running window at all —
the startup camera sits below the F4 scene so only sky is visible, and synthetic
input never reached the SDL window, with the HUD's brush indicator never leaving
`SAND` as proof it was being dropped rather than mis-aimed. The palette was
signed off on a swatch sheet instead. **Every remaining V item has the same
problem**, and it is worse for them: a sprite, a backdrop and a lighting pass
cannot be judged on a static sheet at all. Whatever the answer is — a free
camera, a debug scene spawner, or a way to drive the window that actually works
— it is a prerequisite for verifying the V track rather than a nicety, and it
should be pulled forward the first time a V item cannot be checked.

**That trigger has now fired, and this note is the written argument the rule
above demands.** [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 1 was the first time
the manual checklist was actually run, and it returned eight defects against a
suite that was 6/6 green on 199 checks. Five of the eight are rendering or feel.
**A1 is the one that settles this**: the player rectangle jitters because the
renderer reads a truncated integer position and discards the sub-cell remainder,
and *no headless test can see that* — the simulation was correct the entire
time, collision included. It is the first defect in this project that was
invisible to the suites by construction rather than by omission, which is
exactly the category this item exists to cover, and the other six are only
reachable by tests nobody had written.

**The item therefore moves rather than being started here** — a free camera and
a way to drive the window are now listed as a prerequisite in the V track's own
running order, which is where the work that needs them lives. Left in this
section it would have to be built while this section says not to build it, and a
rule with one exception has no force left. **What has not changed:** it is still
dev-facing, so it is not V10, and it does not become slice-blocking by being
promoted. It becomes the thing V5's successors are checked *with*.

- [x] **World reset hotkey.** *(done — full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#world-reset-hotkey))*
- [x] **Pause and single-step (`P` to toggle, `.` to step once while paused).**
  *(done — full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#pause-and-single-step-p-to-toggle-to-step-once-while-paused))*
- [x] **A free camera, or some other way to look at the world.** *(shipped
  2026-08-14 as `T1.2`, on `F` — see
  [T1](ROADMAP_ARCHIVE.md#t1-the-debug-tooling-batch))* Not in this list before,
  and it is the item the note above is about. The camera
  follows the player and nothing else, so anything the player is not standing
  next to is unverifiable by eye.
- [ ] **Continuous brush strokes.** The brush is stamped once per fixed step at
  the raw mouse position, so a fast drag leaves gaps in the line instead of a
  solid stroke. Fix is to track the last painted grid position and stamp the
  brush along the segment to the current one (Bresenham), not just at the
  endpoint — which also gives straight-line drawing for free on a deliberate
  fast drag.
- [ ] **Brush outline preview.** Draw a hollow ring at `brush_size` under the
  cursor at all times, not just while right-click is held, so the brush
  footprint is visible before paint is committed. Reuses the existing circle
  math from the paint loop, just as an outline test instead of a fill.
- [x] **Cell inspector.** *(done — full entry in
  [ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md#cell-inspector))*
### Window and display

- [ ] **Display modes: fullscreen, borderless, windowed.** Industry-standard
  behaviour, which is more than three `SDL_SetWindowFullscreen` calls: exclusive
  fullscreen vs. borderless-windowed as separate options, alt-tab that doesn't
  corrupt the window state, the choice persisted between launches, and correct
  behaviour when the mode is changed mid-run rather than only at startup.
- [ ] **Resolution options, including 1920x1080, 2560x1440 and 3440x1440.**
  **The hard part of this item is already answered and should not be
  re-litigated here.** It used to read as an open modelling question, because
  the simulation grid was derived from the window size (`GRID_WIDTH =
  WINDOW_WIDTH / PIXEL_SCALE`) and a bigger window therefore meant a bigger,
  more expensive world rather than a bigger view of the same one. F3.1 separated
  the two constants, F3.3 sized the upload to the viewport rather than the
  world, and **F3.5 settled the model in writing**: world size and window size
  are independent in both directions, so growing the viewport reveals more of a
  world that was always fully simulated and never changes how much of it runs.
  F4.3 then made that concrete and the rescale went further — the world is
  1920x1080 against viewports of 480x270 to 860x360, so the camera genuinely
  pans today. What is left here is the window plumbing itself: the mode list,
  persisting the choice, and applying it mid-run rather than only at startup.
  Ultrawide keeps one open question that is *not* rendering — a wider viewport
  shows more of the world, which is a gameplay-fairness call rather than a
  display setting, and it is the one thing in this item still worth deciding
  rather than implementing.

### Art

- [ ] **Custom pixel art and animation generator for game assets.** A tool for
  authoring sprites and animations that fit the engine's palette and cell scale,
  so art is produced in-project instead of hand-drawn in an external editor and
  re-exported. Worth being blunt about the risk: this is a second application,
  with its own UI, file format and edit loop, and it is exactly the kind of
  seductive side-build `VISION.md`'s Scope Discipline section warns about. It
  only earns its keep once there is enough art volume that authoring by hand is
  measurably the bottleneck. Until then, an external editor plus a small import
  step is the cheaper answer. **V6 changes one thing about it and does not
  change the verdict:** a locked palette is exactly the constraint such a tool
  would exist to enforce, so if this is ever built, V6's validator is the half
  of it that already exists.
- **Making an authored scene cohere — moved out, nothing left here.** Both
  halves of the old art-pipeline plan have left this section: phases 0-2 became
  **F4** in Foundations, because the loader was a hard prerequisite for Quantum
  Worlds sitting behind this section's "do not start yet" rule; phases 3-6
  became **V1–V4** in Engine & Visual Depth, because the visual design became a
  stated selling point rather than a finish applied at the end. **V5–V9 were
  then written directly into that section and never passed through here at
  all**, which is the same rule holding in the easier direction. What remains
  genuinely deferred from that note is the editor above.

### Shipping
*`VISION.md`'s first Project Goal says "production-level" and none of this
existed anywhere in the document. Not slice-blocking — but "production-grade" is
a claim these five items are the evidence for, and a plan that omits them is
quietly redefining the goal.*

- [ ] **Packaging and a release build.** There is no answer today to "how does
  someone who is not you run this". SDL is already statically linked, which
  removes the DLL problem, so this is mostly a named artifact, a version string,
  and a decision about what a release actually consists of.
- [ ] **Build on macOS and Linux at least once.** `ENGINEERING_NOTES.md` has
  claimed cross-platform support for several revisions on the strength of using
  no platform-specific code. That is a reasonable expectation, not a verified
  fact, and the gap between the two is exactly the kind of thing this document
  is otherwise good about naming. CI would settle it permanently and is worth
  considering at the same time, since the whole build is CMake and FetchContent.
- [ ] **Crash diagnosis: assertions and a log.** Nothing in the project logs
  anything and nothing owns "what do we do with a crash we cannot reproduce" —
  there is already one unexplained `0xC0000409` (stack buffer overrun) from a
  prior session, seen twice under heavy load and never reproduced. **The payoff
  here is bought by F1 and F2.3 and should be spent on purpose:** once a run is
  a seed plus an input log, a crash report is a file that reproduces the crash
  rather than a description of it. That is also precisely what the playtest gate
  needs.
- [ ] **Audio.** No sound of any kind, mentioned in this document only as a
  clause inside the UI note. Correctly deferred — it makes nothing about the
  loop more or less fun to *test* — but a pixel physics game with no audio is
  not a production-grade game, and it should be a line item rather than a
  parenthetical. **E6 raises the stakes on this rather than changing the
  order:** an explosion is the one thing in the plan that is actively worse
  silent than absent.
- [ ] **Settle on one project name.** The README says "Toop / Xoco (working
  title)", the window title says "SLOP Pixel Physics", and the repo is
  `xoco-game`. Three names for one thing. Trivial to fix, and it stops being
  trivial the moment anything is published under one of them.

---

