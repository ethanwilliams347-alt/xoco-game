# Vision & Scope

This is the project's "why" and "how big is this allowed to get" document — goals, the discipline that keeps the plan from over-growing, and the wish list it protects against. None of it is a task list; for the actual sequenced engineering work, see [ROADMAP.md](ROADMAP.md).

## 🎯 Project Goals
- **Core:** A production-level, barebones application.
- **Engine:** Custom pixel art destructible environment running on its own cellular automata physics engine.
- **Performance:** Strict no-bloat philosophy. Must run on low-end PCs (Windows/macOS/Linux).
- **Architecture:** Keep design choices minimal, but architected to easily allow massive expansion later.

## ⚖️ Scope Discipline (read before adding anything)

This is the hard constraint the roadmap is written against. It is a promotion of `notes/reality_check.txt` into the plan itself, because that note was correct and was being ignored by the roadmap below it.

**Reference point:** Noita — the closest comparable — was built by a team of three over roughly eight years on a purpose-built engine. That is the cost of the physics pillar *alone*.

**The verdict:**
- **Achievable solo:** the pixel physics engine + one gameplay pillar + the extraction/agent loop. Ambitious, multi-year at part-time pace, but real. Solo games that ship succeed on *one* strong hook executed well.
- **Not achievable solo:** the "Ideal Systems" list as written. A simulated stock market, factories, business ownership, farming, a developing society, and reputation mechanics are each a full game's worth of design, balance, and UI. Shipping all of them alongside a physics roguelite is not a scope problem, it is an arithmetic problem.

**The rule going forward:** nothing from the Long Term wish list below or Presentation & Tooling (in [ROADMAP.md](ROADMAP.md)) gets started until the v0.1 slice (Definition of Done, in ROADMAP.md) is complete *and* playtested as fun. Ideas do not get implemented on arrival — they get **written down** in Long Term below, Presentation & Tooling in ROADMAP.md, or `notes/brainstorm.txt` and wait there. Capturing an idea is free and worth doing generously; the discipline is only about what gets *built*, never about what gets *imagined*.

Those three are the only deferral buckets. If an idea does not fit one of them, it either belongs in Short/Medium Term (ROADMAP.md) with a real justification, or it does not belong there.

**Guard against the real failure mode.** The risk here is not running out of ideas, it is spending three years on an engine that never becomes a game. The physics sandbox is the seductive part — it is fun to build and gives constant visible progress. Treat engine work as a means to the slice, and stop extending it the moment the slice does not require more.

**This section guards one direction only, and that is worth saying out loud.** Everything above defends against building too much. Nothing in it defends against the slice being **too thin to be fun**, which is the other way a solo game dies and much harder to see coming, because under-building looks exactly like discipline right up until the playtest gate. Be concrete about where that currently stands: the Definition of Done (ROADMAP.md) cashes out "physics-based movement and destruction" as walking, jumping, and one dig tool on a cooldown. Noita's identity is wands; this project's is a shovel. That may be enough to test whether the loop is fun and it may not — the point is that it is a **question**, and the cheapest place to ask it is before the playtest gate rather than at it. Cutting scope is the reflex this document is built around; this is the one place it is worth checking the reflex.

**One clarification, because the Foundations section in ROADMAP.md looks like exactly what this paragraph warns against.** It is not. Nothing in it adds a material, a behaviour, or a simulation axis; all three items are things several slice features already assume and none of them currently has. The test is not "is this engine-shaped", it is "does a slice feature need this, and does it get more expensive the longer it waits". Those three pass. Anything that only passes the first half belongs in `ENGINEERING_NOTES.md`.

**The pet ML agent is not machine learning, and that is a decision, not an omission.** This was the largest unpriced item in the plan and it is settled here so it stops being ambiguous. Real ML — training a model on gameplay telemetry — means a training pipeline, a model format, an inference runtime shipped with the game, nondeterministic output that has to be balanced anyway, and a payoff that cannot be predicted before it is built. That is a research project attached to a solo game, and it fails the same arithmetic as the Ideal Systems list. For v0.1 the agent is a **deterministic progression system**: run telemetry (objectives completed, depth reached, materials used) feeds hand-authored stats that gate which proof-of-work tasks the agent can take and how fast it completes them. It should *read* as a thing that learns. It does not have to *be* one. If the slice is fun and the fiction is carrying weight, revisiting this with a real model is a legitimate Long Term entry — but the slice does not get to depend on it.

## 🟣 Long Term (The "Ideal Systems" Vision)
*A deliberate wish list, kept for motivation. **Add to it freely** — that is what it is for.*

*The only rule attached to this section is that nothing in it gets **started** before the v0.1 slice is done and playtested. Most of these will never be built, and that is the expected and healthy outcome, not a failure. A list you enjoy adding to costs nothing; a half-finished feature branch costs everything.*

- [ ] Itemization and grid-based inventory.
- [ ] Base building and criminal/black market activities.
- [ ] Complex simulated stock/crypto market.
- [ ] Factories, Mining, and Business Ownership.
- [ ] Passive developing society and reputation mechanics.
