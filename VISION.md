# Vision & Scope

This is the project's "why" and "how big is this allowed to get" document — goals, the discipline that keeps the plan from over-growing, and the wish list it protects against. None of it is a task list; for the actual sequenced engineering work, see [ROADMAP.md](ROADMAP.md).

## 🎯 Project Goals
- **Core:** A production-level, visually impressive, and fun game.
- **Engine:** Custom pixel art destructible environment running on its own cellular automata physics engine.
- **The selling point — the engine and how it looks.** This is what the project is bought on, and it is a goal rather than a quality bar applied at the end. A pixel-physics game is judged on how its world *behaves* and how it *looks behaving* — depth in the simulation and a distinctive, deliberately-authored visual identity are the product, not the finish on it. Both are therefore allowed real investment ahead of the slice, and [ROADMAP.md](ROADMAP.md)'s Engine & Visual Depth section is where that investment is scheduled and bounded.
- **Performance:** Strict no-bloat philosophy. Must run on low-end PCs (Windows/macOS/Linux).
- **Architecture:** Keep design choices minimal, but architected to easily allow massive expansion later.

## ⚖️ Scope Discipline (read before adding anything)

This is the hard constraint the roadmap is written against. It is a promotion of `notes/reality_check.txt` into the plan itself, because that note was correct and was being ignored by the roadmap below it.

**Reference point:** Noita — the closest comparable — was built by a team of three over roughly eight years on a purpose-built engine. That is the cost of the physics pillar *alone*.

**The verdict:**
- **Achievable solo:** the pixel physics engine + one gameplay pillar + the extraction/agent loop. Ambitious, multi-year at part-time pace, but real. Solo games that ship succeed on *one* strong hook executed well.
- **Not achievable solo:** the "Ideal Systems" list as written. A simulated stock market, factories, business ownership, farming, a developing society, and reputation mechanics are each a full game's worth of design, balance, and UI. Shipping all of them alongside a physics roguelite is not a scope problem, it is an arithmetic problem.

**The rule going forward:** nothing from the Long Term wish list below or Presentation & Tooling (in [ROADMAP.md](ROADMAP.md)) gets started until the v0.1 slice (Definition of Done, in ROADMAP.md) is complete *and* playtested as fun. Ideas do not get implemented on arrival — they get **written down** in Long Term below, Presentation & Tooling in ROADMAP.md, or `notes/brainstorm.txt` and wait there. Capturing an idea is free and worth doing generously; the discipline is only about what gets *built*, never about what gets *imagined*.

**That rule has been tested three times now and has not bent once, which is worth recording because it easily could have.** The scene loader (art pipeline phases 0-2) and the visual work that makes an authored scene cohere (phases 3-6) were both sitting inside Presentation & Tooling and both got built early — but in each case the *items moved out* of that section, to F4 and to Engine & Visual Depth respectively, each with the reason written down where they landed. The third test was easier and is worth counting anyway: when the visual track was deepened (V5–V9 in ROADMAP.md), the new items were written straight into Engine & Visual Depth rather than added to Presentation & Tooling and then quietly worked on. Nothing was ever built while still filed behind "do not start this yet." That is the only version of this rule that survives contact with a schedule: a rule with one exception has no force left, so the answer to "this needs to happen sooner" is to argue the item into a different section, in writing, or to leave it alone.

Those three are the only deferral buckets. If an idea does not fit one of them, it belongs in Medium Term or Engine & Visual Depth (ROADMAP.md) with a real justification — and those two sections answer to different tests, each stated in its own preamble — or it does not belong there.

**Guard against the real failure mode, and be precise about which one it is.** The risk is not running out of ideas. It is spending years on *breadth* that never becomes a game — a stock market, a farming layer and a reputation system bolted onto a loop nobody has played. That is what this section exists to stop, and it is what `notes/reality_check.txt` was pointing at.

**It is not aimed at depth in the pillar, and an earlier revision of this document had that wrong.** It used to say to treat engine work as a means to the slice and stop extending it the moment the slice did not require more. That sentence is retired deliberately, not softened: the engine and its visual design are the **selling point** (see Project Goals above), so depth in them is the product rather than a detour from it. The two kinds of spending look similar from a distance and are not the same thing, and conflating them would cost this project the only thing that distinguishes it. The test that replaces the old sentence is: **does this deepen the pillar the project is sold on, and can it name what prompted it?** Depth bought by an observation from play passes. Depth bought by an itch does not, and belongs in `ENGINEERING_NOTES.md` with the reasoning written down.

**What still gets cut is everything in the Long Term list below**, and the arithmetic there has not changed at all. Factories, a simulated stock market, business ownership, farming and a developing society are each a full game, and none of them starts before the v0.1 slice is done and playtested as fun.

**This section guards one direction only, and that is worth saying out loud.** Everything above defends against building the wrong things. Nothing in it defends against the slice being **too thin to be fun**, which is the other way a solo game dies and much harder to see coming, because under-building looks exactly like discipline right up until the playtest gate. Be concrete about where that currently stands: the Definition of Done (ROADMAP.md) cashes out "physics-based movement and destruction" as walking, jumping, and one dig tool on a cooldown. **What the hook finally is has not been decided, and is deliberately left open here rather than guessed at** — naming it early would quietly commit the design to whichever comparison got written down. The point is that it is a **question**, and the cheapest place to ask it is before the playtest gate rather than at it. Cutting scope is the reflex this document is built around; this is the one place it is worth checking the reflex.

**One clarification, because Foundations in ROADMAP.md looks like exactly what these paragraphs warn against.** It is not. Nothing in F1–F4 adds a material, a behaviour, or a simulation axis; all four are things several slice features already assume and none of them currently has. The test there is not "is this engine-shaped", it is "does a slice feature need this, and does it get more expensive the longer it waits". **Engine & Visual Depth, which follows Foundations, passes a different test on purpose** — it is the pillar investment this document now explicitly permits, it does spend a seventh simulation axis (heat), and every item in its physics track names the thing observed in play that bought it. Both sections say which test they are answering; an item that answers neither belongs in `ENGINEERING_NOTES.md`.

**The pet ML agent is not machine learning, and that is a decision, not an omission.** This was the largest unpriced item in the plan and it is settled here so it stops being ambiguous. Real ML — training a model on gameplay telemetry — means a training pipeline, a model format, an inference runtime shipped with the game, nondeterministic output that has to be balanced anyway, and a payoff that cannot be predicted before it is built. That is a research project attached to a solo game, and it fails the same arithmetic as the Ideal Systems list. For v0.1 the agent is a **deterministic progression system**: run telemetry (objectives completed, depth reached, materials used) feeds hand-authored stats that gate which proof-of-work tasks the agent can take and how fast it completes them. It should *read* as a thing that learns. It does not have to *be* one. If the slice is fun and the fiction is carrying weight, revisiting this with a real model is a legitimate Long Term entry — but the slice does not get to depend on it.

## 🟣 Long Term (The "Ideal Systems" Vision)
*A deliberate wish list, kept for motivation. **Add to it freely** — that is what it is for.*

*The only rule attached to this section is that nothing in it gets **started** before the v0.1 slice is done and playtested. Most of these will never be built, and that is the expected and healthy outcome, not a failure. A list you enjoy adding to costs nothing; a half-finished feature branch costs everything.*

- [ ] Itemization and grid-based inventory.
- [ ] Base building and criminal/black market activities.
- [ ] Complex simulated stock/crypto market.
- [ ] Factories, Mining, and Business Ownership.
- [ ] Passive developing society and reputation mechanics.
