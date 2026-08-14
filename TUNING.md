# Tuning Log

The knobs worth turning by feel, where they live, and what each one costs you.
This is a running log: **add a row when you retune something, and date the
entry in the History section at the bottom.**

Scope is deliberately narrow. This lists constants you change to make the game
*feel* different — weight, speed, timing, animation. It is not an index of every
constant in the codebase. Simulation-correctness numbers (`MAX_SUPPORT_CELLS`,
`MIN_PRESSURE_HEAD`, the reaction thresholds) are not feel knobs and are
documented at the point they are used.

**A pass over the feel of all the controls is expected, and is not scheduled**
*(noted 2026-08-12, when D6 and D7 were accepted)*. Both were accepted as *fine
for now* rather than as right, by someone who said in the same breath that the
controls as a whole will want revisiting. So the values in the movement table
are a floor, not a settled answer — and the honest reading of a row that says
"accepted on a look" is that nobody has yet compared it against an alternative.
The prerequisite was `F5`, on the grounds that retuning a float kinematics model
and then converting it means tuning it twice. **`F5` shipped 2026-08-12, so that
prerequisite is met and the feel pass is now unblocked** — still unscheduled,
which is a different thing.

Two rules that apply to everything below:

- **The speeds are `fx` fixed point, not floats — write them as exact rationals.**
  *(F5, 2026-08-12.)* `fx::from_int(500)` is 500 cells/s; `fx::from_ratio(225, 2)`
  is 112.5. **Never `fx::v(112.5f * 65536)`**: a compile-time float fold is as
  machine-dependent as a runtime one, and machine-independence is the entire
  reason these stopped being floats. The value in the `Now` column below is the
  number you would say out loud; the code writes it as the rational that produces
  it exactly. `MAX_STEP_HEIGHT` and `FLAP_INTERVAL_STEPS` are plain `int` and
  always were — they count cells and steps, not distance per time.
- **Velocities are in cells per second, and one cell is 4 screen pixels**
  (`Camera::SCALE`, [src/game/camera.h:10](src/game/camera.h#L10)). The player
  body is 8x20 cells, so "a body height" is 20.
- **Anything measured in `steps` is a fixed simulation step, not a rendered
  frame.** The sim runs at 60 steps/s regardless of fps. 60 steps = 1 second.

---

## Player weight and movement

[src/physics/player.h](src/physics/player.h)

| Knob | Line | Now | What it does |
|---|---|---|---|
| `MOVE_SPEED` | [75](src/physics/player.h#L75) | 112.5 | Horizontal speed. Applied directly with no acceleration or friction, so this is the whole of how fast walking feels. |
| `JUMP_SPEED` | [76](src/physics/player.h#L76) | 175.0 | Standing jump. Set outright rather than added, and uncapped by `FLAP_MAX_CLIMB`, so it stays the strongest single upward move. ~1.5 body heights. |
| `GRAVITY` | [77](src/physics/player.h#L77) | 500.0 | Downward acceleration. **Affects flight too** — there is no separate flight gravity — so raising it makes the character heavier in the air as well as falling faster. |
| `MAX_FALL_SPEED` | [78](src/physics/player.h#L78) | 400.0 | Terminal velocity. Higher reads as heavier, and costs frame time in the `collapsing` scenario (see [PERFORMANCE.md](PERFORMANCE.md)). |
| `MAX_STEP_HEIGHT` | [154](src/physics/player.h#L154) | 3 | Tallest lip walked over without jumping. Lower it and settled sand becomes a staircase you have to jump up; raise it and terrain stops reading as terrain — the body climbs a fifth of its own height instantly and with no animation, which at 5 played as piles not being there at all. **Both failures now have tests in `player_test`** (cliff cases derived from the constant, plus a settled-pile case that must pass at any value), but neither can judge whether the climb *feels* like effort. |

**Scale warning.** These were all multiplied by 2.5 when the body grew. Tune
from the current values by feel; the pre-scale numbers now describe a much
slower character.

## Flight weight

[src/physics/player.h:80-131](src/physics/player.h#L80)

The felt weight of flight is **one relationship**, not one constant: a beat has
to pay `GRAVITY * (FLAP_INTERVAL_STEPS / 60)` ≈ 117 cells/s of gravity, and how
much of `FLAP_IMPULSE` is left over is the climb.

| Knob | Line | Now | What it does |
|---|---|---|---|
| `FLAP_IMPULSE` | [129](src/physics/player.h#L129) | 177.0 | Subtracted from `vel_y` per beat. Sets how many beats it costs to arrest a dive — most of what "heavy" means moment to moment. Margin over the gravity toll is ~60. |
| `FLAP_MAX_CLIMB` | [130](src/physics/player.h#L130) | 98.0 | Ceiling on upward speed. **This, not the impulse, sets sustained climb rate.** |
| `FLAP_INTERVAL_STEPS` | [131](src/physics/player.h#L131) | 14 | Steps between beats. Sets the rhythm, and scales the gravity toll — a longer interval is heavier for free. **Tied to the fly animation's `wait`, below.** |

**Raise the impulse without raising the cap and you will still be told it feels
heavy** — the extra only goes into recovering from falls, while climbing stays
at exactly its old speed. The two move together.

Too much margin makes a helicopter; too little makes a rock that flaps.

## Damage and losing the run

*(S0, 2026-08-14.)* [src/physics/player.h:176-224](src/physics/player.h#L176)

**Every one of these is only correct relative to a constant somewhere else, and
four of those relationships are held by `static_assert`s at the bottom of
[player.h](src/physics/player.h) rather than by this table.** That is deliberate
and it is the reason this section can be short: if you retune one side and not
the other, the build fails with the sentence explaining which pair you broke.
Read those asserts before changing anything here.

| Knob | Line | Now | What it does |
|---|---|---|---|
| `MAX_HEALTH` | [181](src/physics/player.h#L181) | 100 | The size of the bar, and the unit the two rules below are priced in. Everything else here is a fraction of it, so raising this alone makes the game easier by exactly that ratio rather than changing anything's character. |
| `BURN_TEMPERATURE` | [198](src/physics/player.h#L198) | 100 | How hot a cell the body is standing **in** has to be to hurt. Sits in a gap: above Steam's spawn of 88 so a doused fire is not a weapon, well below Fire's 250 and Charred's 200. **Not a difficulty knob** — the gap is narrow at the bottom and both sides are asserted. Contact only; air carries no heat, so a fire you are not touching does nothing. |
| `BURN_DAMAGE` / `BURN_INTERVAL_STEPS` | [205-206](src/physics/player.h#L205) | 2 / 6 | 20 health a second, so five seconds in flame kills. Tune as the **pair** — the rate is what matters and either one alone moves it. A per-step version is 120/s, which reads as an instant death with nothing to react to, and reacting is the whole content of a hazard. |
| `SAFE_FALL_SPEED` | [223](src/physics/player.h#L223) | 240.0 | Landing speed below which a fall is free. Has to clear `JUMP_SPEED` plus a step of gravity or the character hurts itself by jumping — asserted. At the current `GRAVITY` this is a drop of about 58 cells, ~3 body heights. |
| `FALL_DAMAGE_DIVISOR` | [224](src/physics/player.h#L224) | 2 | Damage is `(landing speed − SAFE_FALL_SPEED) / this`, in whole cells per second. At terminal velocity that is 80 of 100: **one free terminal-velocity mistake, and the second one kills.** Lowering it past the point where a terminal landing exceeds `MAX_HEALTH` makes the worst fall an instant death, which is a design change and is asserted against. |
| `Run::OBJECTIVE_REACH` | [133](src/game/run.h#L133) | 6 | How close the collision box has to get, in cells, for the objective to count as reached. Measured box-to-point, so it reads the same from beside or under. Under half a body height — large enough not to need pixel-accuracy at 4 screen pixels per cell. |

**The one number here that is not a feel knob and looks like one is
`BURN_TEMPERATURE`.** It is a threshold into `MATERIALS`, and MATERIALS has
already moved one of the two numbers it sits between, for an unrelated reason
(Steam's spawn went 220 → 88 because at 220 it was a fire-starter). If it ever
moves back up, this stops meaning what the table says and starts meaning "steam
hurts you".

## Animation timing

[tools/player_sheet.py:89](tools/player_sheet.py#L89) — the `ANIMATIONS` table.
`wait` is steps per frame, so **lower is faster**. After editing, run
`python tools/player_sheet.py --header` to regenerate
[src/render/player_sprite.h](src/render/player_sprite.h); the header is
generated and editing it directly is overwritten work.

| Animation | Now | Notes |
|---|---|---|
| `idle` | 2 frames, wait 30 | One second per frame. A breathing pause. |
| `walk` | 6 frames, wait 6 | 36 steps — 0.6s per cycle. Not linked to `MOVE_SPEED` — change the speed a lot and the feet will visibly skate until this is retuned to match. **The column cannot go finer than a whole step, so the next value either way is 20% off; if 6 reads sluggish that is the case for a sub-step animation clock, not for going back to 5.** |
| `rise` / `fall` | 1 frame, wait 0 | Poses, not animations. `wait 0` means "does not advance on a clock". |
| `dig` | 3 frames, wait 8 | 24 steps (0.4s), against `DigTool::COOLDOWN_STEPS` of 6 — a held dig restarts the swing long before it finishes, so frames 1-2 only show on an isolated click. |
| `fly` | 6 frames, wait 3 | **Constrained, see below.** |

**The fly row's `wait` is not a free choice.** `frames * wait` must exceed
`FLAP_INTERVAL_STEPS` (14), or the beat finishes before the next one starts and
the sprite drops back to the rise/fall pose for the gap — a visible stutter
mid-climb. At 6 x 3 = 18 it does not, and the cost is that frames 4-5 (the
recovery back to the top) are only seen when the player *stops* flapping.
Dropping to `wait 2` gives 12 and re-opens the gap.

## Dig tool

[src/physics/tool.h](src/physics/tool.h)

| Knob | Line | Now | What it does |
|---|---|---|---|
| `RANGE` | [23](src/physics/tool.h#L23) | 3 body heights | How far you can reach. Expressed in `Player::HEIGHT` so it survives a rescale. |
| `RADIUS` | [29](src/physics/tool.h#L29) | 3/4 body width | Size of the hole. |
| `COOLDOWN_STEPS` | [34](src/physics/tool.h#L34) | 6 | Steps between digs while held. **Read with the `dig` animation's 24-step length** — they disagree on purpose, but a large change to either is worth checking against the other. |

## Fire and steam timing

[src/physics/grid.h](src/physics/grid.h)

**This section is new on 2026-08-12 and two of its four rows are older than it
is.** The flame constants were retuned by playtest twice — session 2 asked for
slower flames and a ragged top — and were never given rows here, because their
argument was written at the point of use and that felt like enough. It was not:
`CLAUDE.md`'s routing table says a feel constant retuned gets a row *and* a
dated History line, and a knob a playtest has moved twice is a feel knob by any
reading. They are listed now so the next person asking "what do I change to make
fire slower" finds it in the one file that is meant to answer that. **Only the
steam rows changed on 2026-08-12; the flame rows are recorded, not retuned.**

| Knob | Line | Now | What it does |
|---|---|---|---|
| `STEAM_LIFETIME_MEAN` | [grid.h](src/physics/grid.h) | 200 | Steps of **contact with something solid** before a steam cell becomes water. Not elapsed time — steam in the middle of its own pocket does not age at all, so this is how long the *contact layer* lasts and a pocket's total life is this times its depth. **Hard-capped at 255 by `Element::ticks` being one byte**; if a playtest wants materially longer, the answer is a coarser tick, not a bigger number. |
| `STEAM_LIFETIME_SPREAD` | [grid.h](src/physics/grid.h) | 40 | Half-width of the jitter, so cells live 160–240 steps of contact. Stops a pocket placed in one stroke condensing in lockstep, which is the behaviour E9 was fixing rather than a smaller version of it. |
| `FLAME_LIFETIME_MEAN` | [grid.h](src/physics/grid.h) | 13 | How long one flame lives. Sets how *tall* a fire reads, not how fast it spreads — spread is Wood's conductivity and ignition point, and those are correctness constants documented at their rows in `material.h`/`reaction.h`. |
| `FLAME_RISE_SKIP_PERCENT` | [grid.h](src/physics/grid.h) | 10 | Steps out of a hundred a flame stays put, which is how a whole-cell mover expresses 0.9 cells per step. Session 2: "the flames move about 10 percent too fast". |

**The one relationship worth knowing before turning these.** Steam's lifetime
used to be the span between its `spawn_temperature` and a condensing point in
`REACTIONS`, so it could not be tuned without moving a temperature — and the
spawn end is pinned low by a `static_assert`, because steam hotter than the
coldest ignition point in the table is a fire-starter, which has shipped once.
E9's steam half cut that coupling: **the two numbers above are now the only
things that set how long steam lasts, and neither of them touches heat.**

## Light

[src/render/light.cpp:71](src/render/light.cpp#L71),
[src/render/light.h](src/render/light.h)

Expensive knobs. Measure, don't guess — [PERFORMANCE.md](PERFORMANCE.md) has
the numbers.

| Knob | Line | Now | What it does |
|---|---|---|---|
| `TRANSMIT_CLEAR` | [light.cpp:71](src/render/light.cpp#L71) | 0.77 | Per-block falloff through air; sets how far light reaches. Costs more than it looks — a longer reach delays the convergence early-out. |
| `ITERATIONS` | [light.h:58](src/render/light.h#L58) | 24 | Propagation passes. Must be raised with the reach or light stops short of where the falloff says it should get. |
| `BLOCK` | [light.h:43](src/render/light.h#L43) | 4 | Cells per light sample. Doubling it is ~4x cheaper and **re-opens a defect a playtest found** — light bleeding through thin walls. Don't turn this one quietly. |

---

## History

Newest first. One line per retune: what moved, and what was being fixed.

- **2026-08-14** — **six new knobs and no existing one moved** (S0). Health, the
  burn threshold and rate, the safe landing speed and the fall-damage divisor,
  and the objective's reach. In History because the table gained a section, not
  because anything was retuned: **every number in it is a first value chosen to
  sit in a stated relationship with a constant that already existed**, and four
  of those relationships are `static_assert`s rather than rows here. **None of
  them has been playtested.** The two worth watching first are the burn rate —
  five seconds in flame to die, chosen so there is time to react and not
  otherwise argued — and `SAFE_FALL_SPEED`, whose whole job is to be high enough
  that jumping never hurts and low enough that a real drop does. The line
  numbers of every constant above this section are **unchanged**, which was
  checked rather than assumed.

- **2026-08-12** — **every movement and flight constant changed type and none
  changed value** (F5). `float` → `fx` signed 16.16 fixed point, written as exact
  rationals. This is in the History because the *line numbers above all moved* and
  because "112.5" is now spelled `fx::from_ratio(225, 2)`, not because anything
  feels different: the recorded traces are cell-for-cell identical on walk and
  jump, and differ on 7 of 1381 steps of fall and flight, each by one cell and
  each re-converging within a step. **That is the size of a retune's smallest
  possible effect, and it was not a retune — do not treat those 7 steps as a feel
  change to chase.**
- **2026-08-12** — **steam's lifetime stopped being a temperature and became a
  number** (E9, the steam half). `STEAM_LIFETIME_MEAN` 200 and
  `STEAM_LIFETIME_SPREAD` 40 are new; the `{ Steam → Water, 0..26 }` row in
  `REACTIONS` is deleted. What was being fixed is A5 / B3 / D5 — the same
  complaint three times in four sessions — and the measurement is the one worth
  keeping: a pocket sealed under a stone ceiling used to drain **in 3 steps**
  and now takes **291**, with its first drop at step 181 instead of 27. The old
  numbers were not chosen badly; steam's life was the span between its spawn
  temperature and its condensing point, so a pocket pressed against the coldest
  thing in the scene had the shortest life in the game, which is the exact
  reverse of what all three reports asked for.
- **2026-08-12** — `FLAME_LIFETIME_MEAN` (13) and `FLAME_RISE_SKIP_PERCENT` (10)
  **given rows in this file without being changed.** Both were retuned by
  playtest in session 2 and neither was ever logged here, so for four sessions
  the file that is meant to answer "what makes fire feel different" did not
  mention fire. Recorded as a gap being closed rather than as a retune; the
  values are exactly what session 2 left them at.

- **2026-08-12** — `MAX_STEP_HEIGHT` 5 → 3 (D7). **Accepted on a look the same day.** 5 was never chosen, it was
  scaled from the old 8-cell body's 2 and kept that body's ratio; a quarter of
  a 20-cell body climbed instantly is what playtesting read as "the player
  walks over settled piles as if they weren't there". Not 2 — the leg really
  is 2.5x longer, so that would undo the scaling rather than correct it.
- **2026-08-12** — `walk` animation: wait 5 → 6, cycle 30 → 36 steps (D6). The
  walk read about 10% too fast; 6 is the only adjacent value a whole-step
  column holds and it is 20% slower, so this overshoots the estimate on
  purpose. **Accepted on a look the same day — 36 steps does not read
  sluggish, so the whole-step column is not yet the thing holding this back and
  C4's second data point did not arrive.**
- **2026-08-09** — Flight weight: `FLAP_IMPULSE` 130 → 177, `FLAP_MAX_CLIMB`
  70 → 98. Flight playtested ~40% too heavy; both moved together so the climb
  ceiling rose with the impulse.
- **2026-08-09** — `fly` animation: 3 frames at wait 5 → 6 frames at wait 3,
  for the new 6-pose wing cycle. Beat length 15 → 18 steps, still over the
  14-step flap interval.
