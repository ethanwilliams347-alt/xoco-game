# A procedural animation overhaul — options and whether it is earned

Options analysis. **Status changed 2026-08-11: P1 and P2 are now admitted, as
`V14` and `V15` in [ROADMAP.md](../ROADMAP.md#v--visual-identity). P3 is not.**
This document stays the design work; the roadmap carries the scheduling and the
sizes. **The mechanism that admitted them is the last section of this file**,
which named three things that would change the answer — and the third of them
happened, so the section is now a record of a prediction that came true rather
than a list of conditions. It has been updated in place and the original
conditions are kept verbatim, because the value of a written trigger is that it
can be checked afterwards against what actually fired it.

**The question:** replace or supplement the sprite-sheet animation V3.1 shipped
with animation that is *computed* — limbs as separate parts with rotations,
poses derived rather than drawn, feet placed against the terrain actually under
them, motion that responds to velocity instead of selecting from a fixed set of
loops.

**The reason it is worth asking here rather than in general:** this project's
terrain is per-cell, arbitrary and constantly destroyed. A drawn walk cycle
assumes a flat floor. Nothing in this game is a flat floor for long.

---

## What already exists, and what the project already decided

Reading this first is not optional, because two of the four paths below were
already argued and one was already built and removed.

**V3.1 shipped a sheet, and it shipped a finding that points past a sheet.**
Its own text: *"The lesson was 'decompose', not 'draw fifty animations'."* The
observation was that Noita's wizard reads as alive at fourteen pixels wide
because the wand arm is a separate rotating sprite and the cape is separate
simulated cloth — not because the sheet is large. **The project has already
concluded that decomposition is the direction.** A procedural overhaul is not a
new idea here; it is the existing one taken further.

**The decomposed arm was built and pulled, and the reason is the important
part.** From V3.1: *"What it cost while it was in was not the draw call; it was
the second image."* Every frame drawn thereafter had to carry a marker pixel or
`--validate` refused it — a standing tax on authoring, paid for a limb the game
was not yet using. `tools/player_sheet.py` no longer emits a `SHOULDER` table.

**That failure mode is the single best argument for going procedural**, and it
is worth stating plainly: the hotspot image was a way to attach a part to a
*drawn* frame. A rig does not need one, because the attachment point is a
number in the rig rather than a pixel in a parallel image. **The tax that killed
the arm is a tax the sheet imposes and a rig does not.**

**Three things carry over unchanged and should not be re-derived:**

- **The clock.** [player_anim.h](../src/render/player_anim.h) advances on fixed
  simulation steps, never on rendered frames. At 165 Hz a frame-driven cycle
  runs at nearly 3x speed *and presents as an art problem*, which is what makes
  it dangerous. Any solver with state — springs, verlet, IK smoothing — inherits
  this exactly and is worse for it, because a spring's stiffness is a physical
  constant that silently means something different at each frame rate.
- **The rotation trap, already paid for once.** V3.1 records that
  `SDL_RenderCopyEx` mirrors the texture *then* rotates the quad, so flipping the
  sprite does not mirror the angle — it needs a 180° offset, and the naive
  `180 - angle` reflection is correct for a cursor level with the shoulder and
  inverts the vertical everywhere else. Both cases anyone checks by eye are the
  two it gets right. Every path below that rotates anything walks into this.
- **The testability property.** The selector is SDL-free and tested
  (`anim_test`) because what it produces is a row and a column. V3.1's own note:
  its failure modes *"all look like art problems"* — a cycle restarting every
  step reads as "the walk animation is bad". **A procedural rig has strictly more
  silent failure modes than a selector does**, so keeping the solver SDL-free and
  testable is not a nice-to-have carried over from the old design; it is the
  thing that makes the new one debuggable at all.

---

## The four paths

### P0 — Do nothing procedural; bring back the aiming arm instead

Not a null option. It is the decomposition the project already argued for,
already scoped, and already priced: *"the hotspot image, the marker-pixel
validation, a `SHOULDER` table beside `ANIMATIONS`, and a rotate-about-the-
shoulder `SDL_RenderCopyExF`. Nothing about the sheet format has to change."*

- **Cost:** small, and known to the line.
- **Buys:** aim that reads, at whatever moment aiming exists.
- **Against it:** it re-imposes the authoring tax that pulled it the first time.
  Nothing has changed to make that tax cheaper — which is the actual case for
  looking past it.

**This is the baseline every other path has to beat.**

### P1 — Hybrid: sheet body, a few rotating parts

Keep the sheet for the torso/legs loops. Pull 2–4 parts out of it — aiming arm,
head, later a cape — and drive them with rotations and offsets computed per
step.

- **Cost:** moderate. One part table, one solver, per-part sprites. The sheet
  pipeline, the generated header and every `--validate` check survive intact.
- **Buys:** continuous aim without frame-count multiplication, head tracking,
  and the first real test of whether rotated parts read acceptably at this scale.
- **Key difference from P0:** attachment points come from **the rig, not a
  hotspot image**. A part attaches at a body-space offset per animation *frame
  index*, held in a small table beside `ANIMATIONS` and emitted into the
  generated header — a dozen numbers in the file that already generates numbers,
  rather than a parallel BMP the artist must not forget to paint.
- **Against it:** two systems to hold in your head. The body still slides over
  a slope, because the body is still a drawn frame.

### P2 — Full skeletal rig: parts, transforms, IK

Delete the sheet. The character is ~6–10 part sprites; poses are keyframed
transforms; feet are placed by IK against the actual terrain.

- **Cost:** high. New authoring format, new tooling, new validation, a solver,
  and re-drawing the character as parts. `player_sheet.py`, `player_sprite.h`,
  `build_player_sheet.py`, `set_player_frame.py` and the whole of
  [drawing_to_sprite.md](../../drawing_to_sprite.md) are replaced or retired.
- **Buys:** the one thing frames categorically cannot do — **feet that land on
  the terrain that is actually there.** In a game where the floor is per-cell,
  destructible and frequently rubble, a flat-footed walk cycle is a permanent
  small lie. This is the only argument in this document that is specific to
  *this* game rather than to animation in general, and it is the strongest one.
- **Also buys:** amortisation across characters. A rig is written once and
  re-posed; a sheet is drawn per character. **If enemies arrive, this is where
  the economics flip** — see [granulating_enemies.md](granulating_enemies.md).
- **Against it:** it discards a shipped, tested, documented pipeline to solve a
  problem no playtest has yet reported.

### P3 — Physics-driven rig: verlet/spring limbs

Limbs as point masses with distance constraints; motion emerges from the body's
velocity rather than being authored. The Rain World model.

- **Cost:** highest, and it is the only path with a *scheduling* blocker rather
  than merely a price.
- **Buys:** motion that responds to everything without anyone authoring the
  response — the character reacting to a fall, a shove, a slope, an explosion,
  for free.
- **Against it — and the argument has changed shape as of 2026-08-09, so read
  this before citing it.** It used to run: V3.1 deferred the verlet cape *"behind
  E5's free-particle layer — the same 'build it twice' argument that deferred
  rolling behind E5 in E8"*, a limb rig is the same class of thing as a cape, and
  E5 was going to be an off-grid layer of point masses with velocity and
  constraints, so building a bespoke one first is building it twice.
  **The premise is now false.** E5 split, and E5a keeps moving matter *in the
  grid* precisely so that it does not need a second set of rules — it is not an
  off-grid substrate and never will be. Nothing in the E track will hand this a
  point-mass layer for free.
  **The conclusion survives on a different and weaker argument**, which is worth
  stating honestly rather than quietly keeping the old one: a verlet limb rig is
  still the highest-cost option on this page, still has no observation from play
  behind it, and now has *no* scheduled item that makes it cheaper later. It is
  held on cost, not on sequencing. The one thing that would genuinely amortise it
  is E8's body extraction — which is deferred past v0.1 on its own cost — so if
  this is ever wanted sooner it should be priced as its own item and admitted on
  its own observation.

---

## Comparison

*(**P0-P3 here are local labels for the four options on this page, not the
P-track item IDs in ROADMAP.md.** That track gained a `P3` of its own -- chunk
threading -- on 2026-08-09. Nothing here refers to it.)*

| | Cost | Slope-aware feet | Continuous aim | Scales to enemies | Blocked on |
|---|---|---|---|---|---|
| **P0** sheet + arm | Small | No | Yes | No | — |
| **P1** hybrid | Moderate | No | Yes | Partly | — |
| **P2** skeletal rig | High | **Yes** | Yes | **Yes** | — |
| **P3** physics rig | Highest | Yes | Yes | Yes | nothing — see below |

---

## Two objections worth taking seriously

**"Rotation destroys pixel art at 14x26."** Half true, and the half that is
false is the one that matters. A 3-px-wide limb rotated in *art* space is mush.
But rotation happens in **screen** space through `SDL_RenderCopyExF`, after
`Camera::SCALE` has already taken the sprite to 4 screen pixels per cell — a
3-cell limb is 12 screen pixels at the point of rotation, which is enough to
survive it. Noita rotates its wand arm exactly this way at a comparable scale,
and V3.1's pulled arm was already specified as a `SDL_RenderCopyExF`. What it
does cost is **pixel-grid alignment**: rotated parts stop sitting on the same
lattice as the terrain, which reads as "not pixel art" to some eyes and as
"animated" to others. That is a taste call, it is checkable in an afternoon with
P1, and it should be checked before P2 is priced seriously.

**"Procedural means less work."** It means *different* work. The sheet costs
drawing frames; a rig costs tuning constants — and this project has already
written down that tuning without a reference is guesswork, and that reference
*"sets the target for an item that has already earned its place; it does not
create items."* Budget the tuning honestly, and note that
`player_sheet.py --validate`'s three checks (empty bottom row, gap inside the
collision box, blank declared frame) all become meaningless under a rig, because
there are no frames. **Nothing currently replaces them**, and each one exists
because it catches a bug that reads as a physics problem rather than an art one.
Whatever replaces the sheet must also replace those, or the class of bug they
catch comes back invisible.

---

## Recommendation

**Do P0 or P1 when aiming exists, and not before.** P1 is the better of the two
for the same money once you count the hotspot tax, and it is the cheapest way to
answer the rotation-legibility question that P2 and P3 both depend on.

**Hold P2 until a second character exists.** Its two real arguments —
slope-aware feet, and amortisation across characters — are one unproven and one
not yet applicable. The moment enemies are real, the second becomes the whole
argument, and P2 should be reconsidered *then* rather than pre-emptively.

**Hold P3 on cost, not behind an item.** This used to read "hold P3 behind E5,
unconditionally... on the build-it-twice rule". E5 split on 2026-08-09 and E5a
keeps matter in the grid, so there is no off-grid point-mass layer coming and
the build-it-twice rule does not apply here any more. What holds it is that it
is the most expensive option on this page with nothing observed in play asking
for it. See the P3 bullet above for the full correction.

**Do not do a "full overhaul" as one item.** The name is the danger: it bundles
four separately-priced things behind one decision, and three of the four have no
observation behind them.

---

## What would admit this

The V track's test is two questions: **what is wrong in the built game**, and
**what does it unlock**. Reference footage is a primary input here (unlike the E
track) but answers only the second.

Searched [PLAYTEST_LOG.md](../PLAYTEST_LOG.md): **there is no animation finding
in it.** Five sessions, no complaint about the walk cycle, the fall pose or the
dig. On the first question, today, this document has nothing — and V3.1's own
"observed" line is the standard to match: *"V3's single pose responded to
nothing but facing, so the figure slid while walking."* That is what an
admissible observation looks like.

Three things would change that, and it is worth writing them down now so they
are recognised when they arrive:

1. **A playtest says the figure reads as stiff, floaty, or pasted onto the
   terrain.** Most likely once terrain is routinely rubble rather than authored
   slabs. → admits P2's foot IK on its own.
2. **Aiming ships**, and the sheet has to cover aim directions. → admits P0/P1
   immediately; the frame-count argument is already written in V3.1.
3. **A second character type is committed to.** → admits P2 on amortisation, and
   that is the case where "overhaul" stops being the wrong frame.

Until one of those, the honest status is: **the direction is right, the
sequencing says wait, and P1 is the cheap experiment that would tell us most.**

---

## What actually admitted it — 2026-08-11

**Trigger 3 fired.** A second character type was committed to: an enemy whose
body granulates where it is damaged, now `S1` in ROADMAP.md's slice section,
with its design in [granulating_enemies.md](granulating_enemies.md). That is
verbatim the third condition above, and the consequence written next to it —
*"admits P2 on amortisation, and that is the case where 'overhaul' stops being
the wrong frame"* — is the argument `V15` is admitted on.

**Triggers 1 and 2 have not fired, and that is why V14 still comes first.**
No playtest has called the figure stiff or pasted onto the terrain, and aiming
does not exist. So the slope-aware-feet case is still unproven and still
*shapes* V15 rather than admitting it, and the recommendation above — **P1 is
the cheapest way to answer the rotation-legibility question that P2 depends
on** — is unchanged and is the reason the running order is `V14 → V15` rather
than straight to the rig.

**Two things from this document became roadmap text rather than staying here**,
because they are traps a reader of the item must not have to find in a note:
the flip-then-rotate offset, which cost a bug the first time, and the fact that
`player_sheet.py --validate`'s three checks all become unexpressible under a
rig with **nothing currently replacing them**. The second is written into V15
as work inside the item rather than as a caveat.

**P3 is unchanged: held on cost.** Nothing about the commitment above makes a
verlet limb rig cheaper, and the correction recorded in its bullet — that the
"build it twice" deferral behind E5 is void, since E5a keeps matter in the grid
— makes it *unblocked* rather than *due*. A deferral that states a price is not
a refusal.
