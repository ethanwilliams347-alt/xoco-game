# Enemies made of a material that granulates when damaged

Options analysis, written before any code. **Status changed 2026-08-11: M2 is
admitted as `E12` and E-C as `S1` in [ROADMAP.md](../ROADMAP.md), the second one
blocked on the combat decision exactly as the last section of this file said it
would have to be.** M1 stays ruled out, M3 stays behind P1, and E-A remains a
free consequence of E12 rather than an item. This document stays the design
work; the roadmap carries the scheduling, the sizes and the dependencies. **The
last section is updated in place** and its original conditions are kept
verbatim, because a written admission test is only worth anything if it can be
checked afterwards against what actually satisfied it.

**The idea, stated once so the paths can be compared against it:** an enemy
whose body is a solid material that holds its shape until it is disturbed or
damaged, at which point the damaged part granulates and pours away like sand.
Shoot its leg, its leg runs out onto the floor.

The appeal is that it makes the simulation the combat system rather than a
backdrop to one — which is the same argument `VISION.md` makes for the engine
being the product. The difficulty is that it needs one thing to be both a
**material** (per-cell, no identity, physics acts on it) and an **actor**
(identity, intent, acted upon by nothing but its own AI), and this engine
deliberately keeps those two things apart.

---

## The constraints that eliminate options before design starts

All five are already written down elsewhere. They are collected here because
each one kills a path that otherwise looks reasonable.

**1. `Element` has no spare bytes.** ~~12 bytes, `static_assert`ed, and
[element.h](../src/physics/element.h) says outright that the last free byte is
spent: "the next field added here is the first one that actually costs
something — 500 KB at the target resolution and a wider stride through the hot
loop."~~ **Corrected 2026-08-13:** it had three, in the alignment hole between
`type` and `color`, and `element.h`'s claim was counting the tail hole only.
**They are spoken for** — E5a's velocity takes all three — so the *conclusion*
here survives and the reasoning behind it does not: any design wanting a per-cell
number is still proposing a cost, and the cost is **four bytes, not one**, because
alignment rounds 13 up to 16. That is 7.9 MB at 1920x1080. Any design that wants a
per-cell number — enemy ID, hit points, structural integrity — is proposing that.

**2. `Element::ticks` is already double-booked and the assert knows it.** It is
free-fall time for a structural cell and fuel for a Fire cell, safe only
because Fire is a Gas and can never be structural. A third meaning cannot be
added; there is no property left to assert on.

**3. `piece_tag` is owned by fracture and is not a spare identity field.** It is
never written back to zero, it wraps past 255 to 1, and unrelated pieces
sharing a tag re-weld on contact. That asymmetry is deliberate and harmless
*for fracture*. It is not harmless for "which enemy is this cell".

**4. Determinism is a tested invariant.** F1 means any new roll draws from
`sim_random` with its own `Stream` tag registered in `SIM_STREAMS`
([random.h](../src/physics/random.h)) — never the clock, never a member
generator. This costs nothing if planned and is a rewrite if not.

**5. An ECS is refused, but "perhaps four things" is allowed.**
[ENGINEERING_NOTES.md](../ENGINEERING_NOTES.md) refuses an entity/component
system on the grounds that "there is one body, and there will be perhaps four
things." Several enemies is inside that budget. A generic actor framework is
not, and the difference is worth holding onto: a second concrete body type is
in scope, a system for arbitrary body types is the thing being refused.

There is a sixth that is not a constraint but a standing invitation. The
hardness entry in `ENGINEERING_NOTES.md` says materials have no strength
column, and names the signal for revisiting: *"If E3's implementation starts
reaching for a per-material strength number, that is the signal this entry was
waiting for."* A material defined by how much damage it takes before crumbling
is reaching for exactly that. Path **M3** below is that entry coming due.

---

## Part A — the material

Two decisions, and they are independent: **how it is represented**, and **what
triggers the change**.

### Representation — settled, one option is clearly right

Two `MATERIALS` rows, not one row with a mode flag:

```
{ "Crust", …, MoveKind::Static, 32000, 0, true,  … }   // solid, holds neighbours up
{ "Grit",  …, MoveKind::Powder,   150, 0, false, … }   // sand-like once broken
```

This is the **Wood → Charred** precedent exactly — "a state of the fuel, not a
kind of fire" — and it earns the same things Charred earns for free:
`is_solid()` and `is_structural()` are derived from `MoveKind`, so `Crust` gets
player collision and rigid-piece collapse and `Grit` gets piling, with no new
branch in the update loop. A mode flag on one row would need a per-cell bit
(constraint 1) and a branch everywhere `MoveKind` is read.

Cost: two table rows. There is no serious alternative and no reason to defer
this decision behind the others.

### Trigger — three paths, genuinely different

| | What it is | Cost | Gives you |
|---|---|---|---|
| **M1** | A `REACTIONS` row | Free | Nothing — see below |
| **M2** | `granulate(x, y)` called from existing disturbance sites, with a roll | Small | Crumbles on hit, on collapse, on impact |
| **M3** | A per-cell integrity byte, damage accumulates, granulates at zero | 500 KB + hot-loop stride | Wear, cracking, "three hits and it goes" |

**M1 does not work and is worth ruling out in writing**, because it is the
first thing the engine's shape suggests. [reaction.h](../src/physics/reaction.h)
rows are gated on *neighbour type* and a *temperature window* only. "Was hit"
is neither. A row could express `Crust + Fire → Grit`, which is a real and
possibly good behaviour, but it cannot express damage at all. Reactions are the
wrong axis, not an insufficient one.

**M2 is the recommended v1.** Granulation becomes a small `Grid` method called
from three sites that already exist, each producing a distinctly different
feel:

| Hook | Where | Reads as |
|---|---|---|
| Tool impact | `Tool::update` / `march`, [tool.h](../src/physics/tool.h) | Struck it and it crumbled at the point of impact |
| Loss of support | `fall_if_unsupported`, [grid.h](../src/physics/grid.h) | An overhang comes loose and *becomes* a sand-fall |
| Landing | `fracture_landing(x, y)` | Holds shape through the fall, shatters on impact |

The third is the cheapest and the most satisfying — `fracture_landing` is
already "this piece just hit something hard" and already has the component
flood-filled.

Make it a **roll per disturbed cell** (`chance_per_myriad`, new `Stream` tag)
rather than a certainty. Some cells hold and some go, which gives a ragged
crumble edge for free — the same argument [reaction.h](../src/physics/reaction.h)
records for jittering wood's ignition *threshold* rather than its timing, and
worth reading before tuning this, because that entry also records the version
that was tried and measurably did not work.

Estimated: two rows, one method, three call sites, one headless test per site.
Fully testable without the window.

**M3 is not "never", it is "after P1".** Accumulated damage genuinely buys
things M2 cannot — visible wear, a material that resists a weak tool, a
difficulty knob per enemy. What makes it expensive today is only constraint 1,
and **P1 (the hot/cold `Element` array split) is already scheduled and exists
partly to make a decision like this affordable.** Ordering M3 before P1 buys the
feature at its worst price. Ordering it after buys the same feature for close to
nothing. There is no third consideration here — build M2, and if play says the
enemies need to *wear down* rather than *crumble*, that observation is what
admits M3, on the far side of P1.

---

## Part B — the enemy

Three paths. The trade is always the same one: **cells get physics and have no
identity; bodies have identity and get no physics.**

### E-A — the enemy is grid cells

A blob of `Crust` cells in the world.

Everything works and nothing needs writing: damage, fire, water, collapse,
granulation, occlusion, the player standing on the corpse. It is the purest
expression of the idea.

It cannot act. Cells have no identity, so there is nothing to attach intent to,
and the only per-cell field available is `piece_tag` (constraint 3) — borrowing
it means two systems writing one byte with no assert able to separate them,
which is the failure `element.h` documents for `ticks` and which would surface
as enemies silently welding into each other's corpses.

**Verdict: not an enemy.** Worth keeping in mind as a *hazard* — a crumbling
pillar, a collapsing crust ceiling — which is a real thing this material buys
with zero actor work, and which is the cheapest way to find out whether the
material is fun before building anything that chases you.

### E-B — the enemy is a body, granulation is an effect

A `Player`-shaped actor: integer cell position plus remainder, collision via
`is_solid`, animation through [player_anim.h](../src/render/player_anim.h),
sprite drawn over the world.

Full AI, determinism, and animation reuse — `player_anim` is deliberately
SDL-free and takes a plain `Conditions` struct, so it is genuinely reusable
against a second sheet.

But the enemy is not *made* of anything. Granulation would be particles drawn
next to a physics engine doing the real thing three feet away, and it will read
as fake for exactly that reason: the terrain beside it sets the standard.

**Verdict: this is the safe path and it forfeits the idea.**

### E-C — a body with a per-enemy damage mask, that becomes cells when it dies

A body as in E-B, plus a `W×H` byte mask **in body space** — roughly 364 bytes
for a player-sized enemy. Per-enemy, not per-cell, so constraint 1 is untouched.

- **Alive:** the sprite is drawn *through* the mask. Damage clears bits and
  spawns real `Grit` cells into the grid at those world positions — the sand
  pouring off a wounded enemy is genuine simulated powder that piles, gets wet,
  and burns.
- **Dead:** the body deletes itself and writes its remaining mask into the grid
  as real `Crust` cells. From that instant it is terrain, and the existing
  support/fracture system finds it unsupported, drops it, and granulates it on
  impact. **The collapsing corpse is free — E3 already built it.**

**Verdict: recommended.** It is the only path that is grid-native at the moment
that matters without needing per-cell identity at any moment.

Three things it will run into:

- **The mask must be in body space, not sprite space.** A hole punched in the
  idle frame lands somewhere meaningless on walk frame 4. Align the mask to the
  collision box and draw every frame through it. (Fallback if that looks wrong:
  freeze animation once significantly damaged — a crumbling thing that stops
  walking smoothly is arguably correct.)
- **Damage location has to be real.** If a hit clears bits at the sprite centre
  regardless of where you struck, the whole effect collapses. `Tool::aim_point`
  and `march` already resolve a world-space impact point; convert it to body
  space and use it.
- **The corpse is gameplay, not decoration.** It can bury the player, block a
  tunnel, or smother a fire. Probably excellent, but it is a design consequence
  to accept deliberately rather than discover.

---

## The problem all of Part B shares, and it is upstream

**A body is invisible to the grid** — `ENGINEERING_NOTES.md` records this for
the player, and it is scheduled as **E4**, the current head of the E track.
Material falls straight through the body and the unstuck search cleans up after
it.

For an enemy pouring grit out of its own body, that lands in the worst possible
place: **the grit falls through the enemy that is producing it.** The effect
fails precisely at the moment it exists to be looked at.

This is the strongest sequencing argument in the document. E4 asks whether
bodies displace material; E5a (velocity on the cell) is what gives displaced
matter somewhere to go, and E5a is also what would let granulated cells be
*thrown* rather than merely appear. Both are already ahead of this idea in the
running order, both are admitted on their own observations, and this idea wants
both.

(Updated 2026-08-09. This said "E5 (the free-particle layer)". E5 split: E5a is
velocity carried on the cell, in the grid, and E5b is the air/pressure field.
The argument is unchanged and the dependency is now E5a specifically. E10 --
powders coming to rest -- is also worth reading against this idea, since a
crumbling crust that cannot hold a slope will read as a liquid.)

It does not block Part A. Terrain-side granulation — E-A hazards, crumbling
crust — needs neither E4 nor E5a and can be built and judged now.

---

## Recommended sequence

1. **The material as terrain (M2).** Two rows, `granulate()`, three hooks.
   Testable headless, independently valuable, and the only way to find out
   whether the *feel* is worth building an enemy around. If crumbling terrain
   is not fun, nothing downstream saves it.
2. **E-A hazards.** Free once step 1 lands — a crust ceiling that comes down.
   Still no actor code.
3. **E4, then E10, then E5a**, which are ahead of this in the E track anyway and which this
   idea depends on for its best moment.
4. **E-C**, the body plus mask, on top of a material that has already been
   played with.
5. **M3** only if play asks for wear rather than crumble, and only after P1.

The temptation is to jump to 4, because that is the idea. Steps 1 and 2 cost
little and answer whether step 4 is worth its price.

---

## What this would have to answer to be admitted

The E track's test is two questions: **what in the built game is wrong without
this**, and **what does it unlock**. Reference footage and "this would be cool"
do not admit an item — that is written into the track's preamble as the exact
failure mode to watch for.

- **The material (M2)** can answer the second and is weak on the first. Nothing
  observed in play is wrong for want of it. Honest framing: it is a **V-track
  or E7-adjacent** item, or it rides along with whatever admits combat.
- **The enemy (E-C)** cannot currently answer either. There is no combat, so
  nothing is wrong for want of an enemy in the sense the track means. It becomes
  admissible when the Definition of Done's objective loop needs something to
  oppose the player — at which point *this* document is the design work already
  done, and that is what it is for.

Filed as notes rather than as a plan, deliberately.

---

## What actually admitted it — 2026-08-11

**The condition in the second bullet above was met.** The enemy was committed to
as a stated goal for the visual system, which is the sense in which "the
objective loop needs something to oppose the player" was going to arrive — and
the sentence that follows it holds: *"at which point this document is the design
work already done, and that is what it is for."* It was.

**M2 → `E12`, in the E track, after E10.** Admitted on the commitment rather
than on an observation, and the roadmap says so rather than manufacturing one.
The sequencing claim in this document — a crumbling crust that cannot hold a
slope reads as a liquid — is what puts it after E10 specifically, and it is the
sharpest ordering argument either document makes.

**E-C → `S1`, in the slice, blocked on the combat decision** due at the end of
S0. Its dependencies are E12, V12 (a real alpha channel, since a colour key
cannot express a hole that is not exactly one colour), V15 (the rig, which is
what makes a second character cheap — the amortisation this document names) and
E5a.

**One thing in this document has gone stale in a way that matters, and it is the
strongest argument on the page.** The section "The problem all of Part B shares"
was written when **E4 was the open head of the E track**. E4 has since closed —
**answered "no" on 2026-08-10** by playtest session 5, on evidence gathered when
nothing in the game depended on the answer. So the failure predicted there is
not merely a dependency any more, it is the **current behaviour**: grit shed by
an enemy will fall straight through the enemy shedding it. The prediction was
right and the schedule moved out from under it. That is carried as a decision
owed in [ROADMAP_ITEMS.md](../ROADMAP_ITEMS.md#-decisions-owed) with `S1` as the
named consumer, and **a narrow answer — bodies that shed matter displace it, the
player still does not — is the one to consider first**, since it is the only
case with a consumer behind it.

**Unchanged:** M1 is still ruled out on the axis argument, not on cost. M3 is
still after P1 and is still the thing that genuinely fires the hardness entry in
[ENGINEERING_NOTES.md](../ENGINEERING_NOTES.md) — **M2 does not**, and treating
them as one is how that entry gets opened a version early. E-A hazards are still
free once E12 lands and are still the cheapest way to find out whether any of
this is fun before `S1` is paid for.
