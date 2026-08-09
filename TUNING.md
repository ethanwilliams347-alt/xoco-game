# Tuning Log

The knobs worth turning by feel, where they live, and what each one costs you.
This is a running log: **add a row when you retune something, and date the
entry in the History section at the bottom.**

Scope is deliberately narrow. This lists constants you change to make the game
*feel* different — weight, speed, timing, animation. It is not an index of every
constant in the codebase. Simulation-correctness numbers (`MAX_SUPPORT_CELLS`,
`MIN_PRESSURE_HEAD`, the reaction thresholds) are not feel knobs and are
documented at the point they are used.

Two rules that apply to everything below:

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
| `MOVE_SPEED` | [66](src/physics/player.h#L66) | 112.5 | Horizontal speed. Applied directly with no acceleration or friction, so this is the whole of how fast walking feels. |
| `JUMP_SPEED` | [67](src/physics/player.h#L67) | 175.0 | Standing jump. Set outright rather than added, and uncapped by `FLAP_MAX_CLIMB`, so it stays the strongest single upward move. ~1.5 body heights. |
| `GRAVITY` | [68](src/physics/player.h#L68) | 500.0 | Downward acceleration. **Affects flight too** — there is no separate flight gravity — so raising it makes the character heavier in the air as well as falling faster. |
| `MAX_FALL_SPEED` | [69](src/physics/player.h#L69) | 400.0 | Terminal velocity. Higher reads as heavier, and costs frame time in the `collapsing` scenario (see [PERFORMANCE.md](PERFORMANCE.md)). |
| `MAX_STEP_HEIGHT` | [133](src/physics/player.h#L133) | 5 | Tallest lip walked over without jumping. Lower it and settled sand becomes a staircase you have to jump up. |

**Scale warning.** These were all multiplied by 2.5 when the body grew. Tune
from the current values by feel; the pre-scale numbers now describe a much
slower character.

## Flight weight

[src/physics/player.h:71-122](src/physics/player.h#L71)

The felt weight of flight is **one relationship**, not one constant: a beat has
to pay `GRAVITY * (FLAP_INTERVAL_STEPS / 60)` ≈ 117 cells/s of gravity, and how
much of `FLAP_IMPULSE` is left over is the climb.

| Knob | Line | Now | What it does |
|---|---|---|---|
| `FLAP_IMPULSE` | [120](src/physics/player.h#L120) | 177.0 | Subtracted from `vel_y` per beat. Sets how many beats it costs to arrest a dive — most of what "heavy" means moment to moment. Margin over the gravity toll is ~60. |
| `FLAP_MAX_CLIMB` | [121](src/physics/player.h#L121) | 98.0 | Ceiling on upward speed. **This, not the impulse, sets sustained climb rate.** |
| `FLAP_INTERVAL_STEPS` | [122](src/physics/player.h#L122) | 14 | Steps between beats. Sets the rhythm, and scales the gravity toll — a longer interval is heavier for free. **Tied to the fly animation's `wait`, below.** |

**Raise the impulse without raising the cap and you will still be told it feels
heavy** — the extra only goes into recovering from falls, while climbing stays
at exactly its old speed. The two move together.

Too much margin makes a helicopter; too little makes a rock that flaps.

## Animation timing

[tools/player_sheet.py:89](tools/player_sheet.py#L89) — the `ANIMATIONS` table.
`wait` is steps per frame, so **lower is faster**. After editing, run
`python tools/player_sheet.py --header` to regenerate
[src/render/player_sprite.h](src/render/player_sprite.h); the header is
generated and editing it directly is overwritten work.

| Animation | Now | Notes |
|---|---|---|
| `idle` | 2 frames, wait 30 | One second per frame. A breathing pause. |
| `walk` | 6 frames, wait 5 | Half a second per cycle. Not linked to `MOVE_SPEED` — change the speed a lot and the feet will visibly skate until this is retuned to match. |
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

- **2026-08-09** — Flight weight: `FLAP_IMPULSE` 130 → 177, `FLAP_MAX_CLIMB`
  70 → 98. Flight playtested ~40% too heavy; both moved together so the climb
  ceiling rose with the impulse.
- **2026-08-09** — `fly` animation: 3 frames at wait 5 → 6 frames at wait 3,
  for the new 6-pose wing cycle. Beat length 15 → 18 steps, still over the
  14-step flap interval.
