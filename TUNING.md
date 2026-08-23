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

## Depth grading

[src/render/frame.cpp](src/render/frame.cpp) — the layer table

**A layer's `Grade` is a multiply, and it is the only knob in this file that can
make the screen darker.** Everything above adds: V7's light pass is one additive
copy, so before 2026-08-16 no number anywhere could reduce a pixel below the
value its art was authored at. These are per-layer, applied by
`SDL_SetTextureColorMod`, and cost nothing — a graded layer is the same draw
call.

**Retuning one of these moves the golden frame checksum**, which is correct and
expected. The new value goes in `tests/test_golden_frame.cpp` in the *same
commit*, or `git log -S` on that constant stops listing every frame the renderer
has produced. That is the one way to misuse the golden test.

| Knob | Line | Now | What it does |
|---|---|---|---|
| `mountains` grade | [frame.cpp](src/render/frame.cpp) | 0.60 (153) | How dark the mountain band sits against the sky. **Measured, not eyeballed** — the only number in the backdrop that is. At 1.00 the sky averages luminance 26 and the mountains are flat 28, so the two most distant bands separate by two levels out of 255 and the far one is the *brighter*; at 0.60 the pair reads 26 against 16 and the mountains become a silhouette. Lower reads as a heavier, closer ridge; above about 0.85 the band stops separating at all. **Every luminance in this cell was measured before V20 raised the palette and none of them is current** — the same 0.60 now reads sky 95→62 against mountains 44. The *shape* of the argument survived the raise, which is the reason the number did not move; the figures did not. **V21 (2026-08-16) then took the ceiling back down by 0.80 after session 7 reported "too bright", and the figures moved again without the grade moving: sky 76→50 against mountains 35.** Two palette moves in one day and 0.60 outlived both — which is the case for keeping the ladder in the grades rather than in the art. |
| `sky` grade | [frame.cpp](src/render/frame.cpp) | 1.00 | Deliberately identity. The sky is the reference every other band's value is judged against, so grading it moves all of them at once and none of them relative to each other. |
| `cells` grade | [frame.cpp](src/render/frame.cpp) | 1.00 | Identity, and **it stays identity: V22 part 3 did not turn this row, and the reason is that the paragraph below it was wrong about which way the junction ran** (2026-08-22). Measured with `plane_probe`'s band ladder at the spawn: the plane's near end is luminance **65.7** and the world standing in front of it is **23.3** — the plane is *brighter* than the ground, not darker. The text below inferred the opposite from the two grades (world 1.00 against plane 0.53) **without looking at the art underneath them**, and the world's art is near-black, so 1.00 of near-black is still near-black. Turning this row as written would have darkened the world further and deepened the exact inversion it was trying to fix. **It also mapped the reference wrongly in a way worth naming, because the sentence still reads true:** the reference's plane *is* brighter than the silhouettes standing on it, but our terrain is not a silhouette — it is the walkable surface, which in every reference frame measured is the **brightest** band in the frame (72.9-79.9, reference_observations ENTRY 12). A silhouette and a walkable surface were being treated as one thing. What shipped instead is a ramp authored into the terrain albedo, because a grade is uniform and cannot brighten one end of a band — see the row below. **The original entry follows unchanged, because the defect it names is real and was fixed; only its direction was wrong.** *(Original:)* **as of 2026-08-16 this is an open question with an owner rather than a settled default.** The original reason stands on its own terms: the world's own spread (Oil 38 to Sand 171) is already the widest in the frame, and grading it compresses the one band that does not need help. **What that argument never considered is the junction.** The world sits at 1.00 against the ground plane's 0.53, so the plane's *near* end — the most forward thing in the backdrop — is darker than the ground standing in front of it, and the reference runs the other way round: its plane is brighter than what stands on it. That makes the two read as two objects rather than one surface receding, which is [V22](ROADMAP.md)'s entire subject, so **this row is the knob V22 turns.** Two things must not be lost when it moves: grade multiplies and light adds, so fire lights the world back up and the change is not a flat loss of legibility — **but it is still legibility while digging**, which is a thing you feel and not a thing measured, so it takes a playtest of its own and does not get settled inside a commit about something else. |
| `ground` grade | [frame.cpp](src/render/frame.cpp) | 0.53 (135) | **Where the ground plane sits on the value ladder, and it is the one grade derived from the reference rather than from our own art.** The reference's plane runs 0.45 to 0.80 of its sky; the tile is authored with that 1.8x internal ramp baked in (a uniform multiply cannot produce a ramp), so this number places the whole band. At 135 the plane measures **0.44 to 0.81 of our sky**, and its horizon edge lands at luminance 11.7 — below the graded mountains at 16.9 and below the darkest sky row at 18.1, which is what makes the horizon the darkest line in the frame. Raise it and the plane climbs toward the sky, which is exactly the defect the played frame showed (plane 22.7 against sky 22.3); lower it and the horizon pinch swallows the near edge too. **Pre-V20 figures, kept because the reasoning is still the reasoning and the numbers are not.** At the raised palette the same 0.53 puts the plane at **30 → 81**, its horizon edge 14 below the graded mountains at 44 and 32 below the darkest sky row at 62 — still the darkest line in the frame, at an amplitude that can be seen. **After V21's 0.80 ceiling the same 0.53 reads 24 → 62, and the horizon join went 14 → 11.** **That near end is 85 since V22 part 4 (2026-08-23) and the grade did not move to do it** — the tile's ramp did, in the row below; the horizon join is untouched at 11 because `ground_far` is untouched, which is the whole reason the amplitude was bought in the art rather than here. That join is the number to watch if the ceiling is asked to come down again: it is the reference's own signature at 14, and a ceiling move scales it, so the plane's readability is what a further darkening spends. Reasoning in full at the palette in [tools/pixel_art.py](tools/pixel_art.py). | **The 0.45→0.80 ramp is authored into the tile as ten distinct tones since V20, not as two dithered against each other**, which is what had left the near third of it flat. |
| ground tile ramp | [pixel_art.py](tools/pixel_art.py) `ground_near`, `ground_mark` | ×1.35 on V21's pair; `ground_far` unmoved | **The ground plane's own far-to-near amplitude** (V22 part 4, 2026-08-23). The plane is the one band that has to recede *within itself*, so a uniform grade cannot supply this and the row above only places the band. Post-grade the pair now runs **24 → 85, a span of 60.9 levels against the reference plane's 60.7** (entry 7's 77.5 → 138.2) — the palette comment had named that difference as the target since V20 and the pair had been authored to 38. **What made five sittings miss it is a geometry finding and it is the reason this row exists:** every previous attempt aimed at the tile's near edge, which at the spawn is *never on screen*. The plane runs 640 px from the horizon, the world's surface sits at 65% of that band, and `plane_src_at`'s inverse-depth mapping puts the contact at **tile row 198 of 256** — so the row the eye actually reads was carrying 0.778 of a ramp that was already 23 levels short, and the join measured **plane 51.3 against world 73.4**. It now measures **70.2 against 73.4**, and the composed frame runs 25.5 / 29.5 / 34.8 / 40.3 / 46.3 / 52.8 / 58.7 / 65.2 / 71.1 into the contact — a slope where there was a step. **`ground_far` did not move**, so the horizon stays the frame's dark pinch and the mountain join stays at 11. **`ground_mark` is scaled by the same factor on purpose**: its contrast against the ramp is then unchanged as a ratio and grows as a difference toward the near edge, which is the detail-energy ramp the reference carries at 12× and we measured here at 5× and running backwards — it is 7.8 mean now against 5.2, still peaking mid-tile, and that remainder is not fixed. Higher brings the plane forward until its near edge competes with the lit surface for the frame's peak; lower reopens the shelf. **The debt: 0.80 × 1.35 puts this one band 8% above V20, and V20 is what session 7 called "too bright"** — the visible part tops out at 71.5 rather than 85, and the world's lit surface at 73.4 stays the brightest thing beside the player, but if session 14 says it again this factor is the first thing to look at. |
| terrain depth ramp | [pixel_art.py](tools/pixel_art.py) `apply_depth_ramp` | lit 4, fade 24 | **How far the lit band reaches down into the world, in cells, and how far below that it takes to reach the deep fill** (V22 part 3, 2026-08-22). This is the terrain layer's own far-to-near ramp, the sibling of the ground tile's, and it is authored into `assets/test_albedo.bmp` rather than graded for the reason the plane's is: **a grade is uniform, so a ramp inside a band is always the art's job.** The four cells of lit band are what the rim light hands off into; the 24 of fade are set from the frame and not from taste — at 1920x1080 the viewport is 270 cells and `VERTICAL_ANCHOR` 0.80 leaves **54 cells of world visible below the feet**, and the reference spends the first ~40% of its below-contact region falling from the lit band to near-black. Measured at the spawn, the ladder went from *plane 65.7 / world 23.3* to a world peak of **73.4 at the feet**, falling 48.6 / 29.5 / 23.3 below it — which is the reference's shape (peak 72.9-79.9, foreground 19.4-24.2). **Deeper `fade` reads as a taller lit bank and eats the foreground; shallower reads as a bright crust on black.** Cell counts rather than a fraction of the window, the same argument `PLANE_TEXEL_SCALE` makes. The rungs between the two ends are named palette entries, not interpolated — `tools/validate_palette.py` caught 25,480 off-palette pixels the first time they were not. |
| `PLANE_TEXEL_SCALE` | [backdrop_wrap.h](src/render/backdrop_wrap.h) | 2.5 | **How many screen pixels one row of the ground tile occupies, and so how deep the whole plane is** (V24, 2026-08-18). It replaces no knob — it replaces `bottom_y = window_h`, a window constant standing in for a geometry nobody had written. **That is what playtest session 10 reported as the tile squishing as the camera climbs**: a parallaxed far edge against a pinned near edge is a layer whose nearer end is less parallaxed than its further end, and the band ran 651.4 px down to 457.0 across the camera's travel while the tile stayed wholly on screen. **2.5 is the shipped composition kept rather than re-chosen** — the spawn band was 651.4 over a 256-row tile, 2.544 px per texel — so this is a defect fix at the old look, not a retune. Larger reads as a deeper, more foreshortened plane and pushes the near edge further below the window; smaller lifts the near edge up the screen and spends more frames on the fill below it. **Stated per texel and not as a fraction of the window on purpose**, because a fraction would put the same tile on screen at two sizes at 1080 and 1440 — the reported defect again, with the window standing in for the camera. |
| `GROUND_STRIPS` | [frame.cpp](src/render/frame.cpp) | 24 | How many depths the plane is cut into. **The only knob in this file that is a cost as well as a look** — it multiplies the plane's draw calls. Below about 12 the parallax step between adjacent strips reads as banding while walking; above 24 buys nothing visible. **Not priced against the frame budget**, because `grid_bench` times the simulation and cannot see a draw call; the instrument is the frame rate in the running game. |
| ~~`GROUND_HORIZON_FRACTION`~~ | — | **retired 2026-08-16** | Was 0.55, "where the plane's far edge sits as a fraction of window height, where the played frame's terrain skyline already sits". **Not a feel knob and should never have been one** — it is where the ground meets the *mountains*, which is a fact about the mountains, and its justification named the terrain. Stated in window space it was free to contradict the art, and it did at every camera position the world reaches: the plane covered the whole mountain band. Replaced by a derivation from `backdrop_layers::MOUNTAINS_SKYLINE_MAX`, generated from the same seeded walk that draws the silhouette and so **deliberately not tunable**. See `ground_horizon_y` in [frame.cpp](src/render/frame.cpp) and V20 in ROADMAP.md. |
| `Params::world_grade` | [frame.h](src/render/frame.h) | identity | The world-*wide* multiply — night, underground, fog, per-biome. **Nothing sets it yet**; at identity the quad is not drawn at all. Set it and every world layer dims together while the fire does not, which is why the grade pass is ordered before the light pass and not after. |

## Camera framing — **one constant, and it is a composition rather than a feel**

[src/game/camera.h](src/game/camera.h)

| Constant | Value | What it does |
|---|---|---|
| `Camera::VERTICAL_ANCHOR` | **0.80** | How far down the viewport the followed point sits, as a fraction. 0.5 is centring |

**Set by V22 on 2026-08-18, and it is the reopening of a decision made by
playing, at the tester's direction.** V23b had deleted every camera knob one day
earlier; what brings one back is not a feel report but the arithmetic V22 cannot
get past — a centred player caps the receding plane's visible share at **~50% of
its band by construction**, and the reference reading V22 is authored against is
two thirds. 0.80 puts the contact at ~65% and is the same number V23 used for
`SURFACE_ANCHOR`, for the same reason: it is measured off the reference frames.

**What did *not* come back is the part the tester rejected.** V23's anchor
*moved* — 0.80 easing to 0.50 while digging — and sessions 8 and 9 rejected that
twice. There is one constant here, it never changes at runtime, and
`COLUMN_ANCHOR` and `EASE_PER_SEC` do not exist. **A moving anchor is a rejected
feel; a fixed anchor is a composition**, and that distinction is the whole
argument for this section existing again.

**The cost is known and is unpaid until a human looks at it.** 0.80 leaves ~55
cells of world below the player at the surface, and digging is the game's one
verb. V23 paid that back with the framing that was rejected; V22 does not, so
this constant is the most likely thing in this file to move on the next
playtest. It is one number in one place precisely so that it can.

The two things below were written after V23b as what the next attempt would
otherwise rediscover. Both were honoured by this one, which is the only evidence
that keeping them was worth anything.

Two things worth keeping, because they are what the next attempt would otherwise
rediscover:

- **A framing is a fraction of the viewport, never a count of cells.** What is
  being matched is a composition, and a cell offset expresses it only at the one
  viewport height it was tuned at; every other entry in `DISPLAY_MODES` gets a
  different picture from the same number.
- **A framing has to be asserted as *delivered*, not as requested.** The view
  clamps at `world_h - viewport_h`, so near the world's floor a request and its
  result are different numbers — which is exactly how V23's dig framing shipped
  past a green suite and reached a human as "upside down".

## Debug camera

[src/game/debug_view.h](src/game/debug_view.h)

**Here under protest, and the protest is the useful part.** This file is for how
the *game* feels and the free camera is a dev tool nobody ships. It is listed
anyway because the rule above is "weight, speed, timing, animation" and this is a
speed you would retune by feel — and because a tool speed that is wrong wastes
exactly the sittings T1 exists to make cheaper. **Nothing else in this table
should follow it in**: the test is whether you would change the number by looking
at the result, not whether the number is a speed.

| Knob | Line | Now | What it does |
|---|---|---|---|
| `PAN_CELLS_PER_SECOND` | [debug_view.h](src/game/debug_view.h) | 200 | Free-camera pan speed. Crosses the 480-cell viewport in ~2.4s, which is a speed you can stop *on* something — the failure at the fast end is overshooting the thing you detached the camera to look at. |
| `PAN_FAST_MULTIPLIER` | [debug_view.h](src/game/debug_view.h) | 4 | Held `Shift`. Crosses the whole 1920-cell world in ~2.4s, so the two speeds are "inspect" and "travel" rather than two versions of one. |

---

## History

Newest first. One line per retune: what moved, and what was being fixed.

- **2026-08-23** — **V22 part 4: the plane's ramp gains the 23 levels the
  target always named.** `ground_near` and `ground_mark` in
  [pixel_art.py](tools/pixel_art.py) scaled by 1.35, `ground_far` untouched, and
  `assets/backdrop_ground.bmp` regenerated. **Reported by playtest session 13**:
  *"the ground i am standing on is a separate shelf sitting in front of a
  parallaxed background, specifically the backdrop_ground.bmp"* — the sixth
  "no" to the same gate, and the first that named the layer. **Measured cause,
  and it is why part 3 did not close it:** part 3 matched the world to the
  plane's *authored* near end of 65.7, but the plane is only visible to 65% of
  its own depth — its near third sits behind the terrain — so the row that
  actually touches the world read **51.3 against 73.4**, a 22-level step with the
  world the brighter side. Now **70.2 against 73.4**. Post-grade span 24 → 85,
  which is the reference's 61 levels and the number the palette had named as the
  target since V20. **No `.rec` cost this time** — the tile is render-side and
  `input_log::fingerprint` hashes `e.color`, so
  `session_4_digging_fluids_fire.rec` survives. **Owed to a playtest**, and it is
  the gate question a seventh time.

- **2026-08-22** — **V22 part 3: the terrain gets the ramp the ground plane
  has had since V19.** `apply_depth_ramp` in [pixel_art.py](tools/pixel_art.py) at
  `lit_depth` 4 / `fade_depth` 24, nine new palette rungs, and
  `assets/test_albedo.bmp` regenerated. **Reported by playtest session 12**:
  *"looks like a separate shelf sitting in front of a painted backdrop"*.
  Measured cause, and it is the fifth attempt at this report and the first one
  aimed at a number: the band ladder ran **plane 65.7 / world 23.3** at the
  junction — the frame's brightest band directly behind its darkest, where
  every reference frame makes the walkable surface the brightest thing in the
  frame. Now **73.4 at the feet**, falling to 23.3 at the bottom of the window.
  **Not a grade, and the `cells` grade row above is corrected rather than
  turned** — it had the junction running the other way. **Both `.rec`
  recordings are invalidated** (`input_log::fingerprint` hashes `e.color`), so
  P4's replayed row is dark until the tester plays and presses `F9`. **Owed to a
  playtest**, and it is the gate question a fifth time.

- **2026-08-18** — **V24: the ground plane translates instead of squishing.**
  `PLANE_TEXEL_SCALE` 2.5 in [backdrop_wrap.h](src/render/backdrop_wrap.h), and
  the window height stops being an input to the plane's geometry at all.
  **Reported by playtest session 10**, against a question that had explicitly
  said it was not asking about the plane: *"the entire .bmp stays on screen and
  squishes as the sprite flies up."* The near edge was `window_h` — a constant —
  while the far edge was parallaxed, so the nearer end of one layer was less
  parallaxed than its further end. **Not a retune**: 2.5 reproduces the shipped
  spawn composition to within 11 px, and the visible change is entirely in what
  happens when the camera *moves*. The golden checksum moves to
  `0x77404ada6ab4d08f` and the overlay's to `0x57a56d7aa8cc6bf0` — **both, which
  is the combination that means the world changed rather than the UI.** The
  alternative fix, a real vertical factor on the near edge, is recorded at the
  constant and was not taken. **Owed to a playtest**, and it is a *motion* check,
  which the checklist has none of.

- **2026-08-18** — **V22: one camera knob comes back, as a constant.**
  `Camera::VERTICAL_ANCHOR` 0.80, inside `Camera::follow` where the game and the
  golden fixture cannot disagree about it. **This reopens V23b at the tester's
  direction**, on a stated concern that V22's two-thirds target is unreachable
  with a centred camera and an answer that V22 runs in full anyway. What two
  playtests rejected was the anchor *moving*, and none of that returns — no
  easing, no dig or airborne trigger, no `camera_bias.h`. The golden checksum
  moves to `0xf29c435ed9d923b1` and the overlay's to `0x6527211c3b5d3bb2`; the
  second moved only because it hashes the world with the UI over it, and **an
  overlay checksum moving beside an unmoved `GOLDEN` is the only combination
  that would have meant the UI itself changed.** **Owed to a playtest**, and
  that is the whole worth of this row: ~55 cells of world below the player is a
  cost nothing here has paid.

- **2026-08-17** — **V23b: all three camera knobs deleted; the camera centres
  again.** Playtest session 9 asked for the centred framing back, one day after
  V23 introduced it and hours after V23a's correction. `SURFACE_ANCHOR`,
  `COLUMN_ANCHOR` and `EASE_PER_SEC` are gone, along with `CameraBias` and
  `Camera::set_vertical_anchor` — **not set to neutral values, removed**, so
  that nothing here claims a knob the code does not have. `Camera::follow`'s
  vertical expression is character for character the pre-V23 one and the golden
  checksum went back to `0xcde4dc1a39927fca`, the number it held before V23,
  which is the evidence the revert is complete. **What is not settled by this is
  the receding plane**, whose visible share the centred camera caps at ~50% by
  construction — that measurement is unchanged and V22 still has to answer it
  some other way.
- **2026-08-17** — **V23a: the dig framing moved to centre and gained a second
  trigger, on the first human report.** `DIG_ANCHOR` 0.30 → `COLUMN_ANCHOR`
  0.50, renamed because it is no longer only about digging: being **airborne**
  now takes the same framing, which playtest session 8 asked for in the same
  breath ("digging or flying out of frame"). **The reason 0.30 moved is not that
  it felt wrong — it is that it was never delivered.** `Camera::follow` clamps
  the view at `world_h - viewport_h` = 810, so at the fixture scene's floor a
  request for 0.30 resolved to 0.51 on screen, and deeper to 0.70: the camera
  answered the dig least where there was most world below to see. The tester
  reported that as the illusion being "upside down". `EASE_PER_SEC` is untouched
  at 0.85 **and is now a 0.35-second swing rather than a 0.6-second one**, purely
  because the distance shrank — so the question about its speed is still open and
  the answer given cannot be reused.
- **2026-08-17** — **V23: the camera stopped centring, and three new knobs
  arrived at once.** Not a retune — a new section, listed here because the
  numbers in it are the least evidenced in the file and the next session should
  know that before trusting them. `SURFACE_ANCHOR` 0.80, `DIG_ANCHOR` 0.30,
  `EASE_PER_SEC` 0.85. *(The middle one moved the next day — see the entry
  above.)* **What was being fixed is not a feel complaint but a
  measurement**: `Camera::follow` centred strictly, which capped the receding
  plane's visible share at ~50% of its band by construction and put it at 20.2%
  at the spawn, so three rounds of value tuning had been aimed at pixels the
  plane never drew. **The two anchors come from the reference frames and the
  ease comes from nothing at all** — 0.85 is a guess at a duration, and it is
  the first of the three a playtest is likely to move.
- **2026-08-16** — **V21: the ceiling came back down by 0.80 and, again, neither
  grade moved.** Playtest session 7 answered checklist item 1 "too bright" — the
  one answer that item was written to invite. V20 had raised the backdrop group
  wholesale from a frame occupying 9 luminance levels; V21 multiplies that group
  by 0.80, preserving every ratio between bands, so post-grade the frame reads
  sky **76→50**, mountain rim **57**, body **35**, ground **24→62**. `mountains`
  0.60 and `ground` 0.53 are untouched for the second consecutive retune.
  **The factor is 0.80 rather than lower for a reason worth carrying:** a ceiling
  is a multiply, so it scales every *absolute* separation, and absolute
  separation is the exact quantity V20 existed to buy. The mountain/ground
  horizon join — the reference's signature at 14 levels — goes 14 → 11 at 0.80
  and would be 8 at 0.60, back under the reference's smallest band join. **The
  ceiling is nearly out of downward room; a further "too bright" is a hue and
  saturation item or a grade item, not a smaller factor.**
  Same commit fixed a stale hand-copy: `draw_clear` still held the *pre-V20*
  sky tone at luminance 18, under a comment naming a constant that had since
  both moved and stopped being the darkest of its pair. Invisible in the
  ordinary frame because the sky texture covers the window — and **outside the
  golden frame's coverage entirely**, since that fixture generates its own
  textures rather than loading `assets/`. The checksum is unmoved at
  `0xcde4dc1a39927fca` and that is correct, not a miss.
- **2026-08-16** — **V20: the two grades did not move and their meaning did, which
  is why this line exists.** Playtest session 6 reported the mountains as
  invisible and the plane as unconvincing; the cause was neither grade. The
  backdrop *palette* was raised wholesale — the whole frame occupied luminance
  15.5 to 24.5, against the reference's 51.6 to 173.6, because the value ladder
  had been built downward from a sky authored at L 18 and `Grade` can only
  darken. `mountains` stays **0.60** and `ground` stays **0.53**; what changed is
  what they are multiplying. **The numbers quoted in both rows above are now
  historical** — `ground` at 0.53 no longer lands the plane's horizon at
  luminance 11.7 but at **30**, and the plane's ramp is 50.6 levels where it was
  9.8. Re-measure before quoting either row's figures. `GROUND_HORIZON_FRACTION`
  was **retired rather than retuned**; the argument is in its row. Full entry at
  V20 in ROADMAP.md.

- **2026-08-16** — **three new knobs, none of them a retune: V19's ground plane**
  (step 4b of the visual rework). `ground` grade **0.53**, `GROUND_STRIPS` **24**,
  `GROUND_HORIZON_FRACTION` **0.55**. What was being fixed is a measurement
  nobody went looking for: on the played frame
  (`resources/game_screenshots/visual_rework_1.png`) the surface below the
  terrain's skyline has a luminance spread of **exactly 0.0** across 400 rows and
  37% of the frame, and sits at **22.7 against the upper sky's 22.3** — four
  tenths of a level out of 255 between the nearest surface in the frame and one
  of the most distant. Same shape as the sky-versus-mountains reading that bought
  the 0.60 grade above, on a much larger pair of surfaces, and step 3's
  measurement could not have caught it because it only compared the two backdrop
  bands to each other. **The grade is derived from the reference's ladder and
  then checked against our own art**, which is the first time both routes have
  been available for one number: authored and graded, the plane measures 0.44 to
  0.81 of our sky against the reference's 0.45 to 0.80.
- **2026-08-16** — **one knob moved and it is the first retune in this file that
  was measured rather than judged by eye** (V11's grade, step 3 of the visual
  rework). The mountain backdrop went from 1.00 to **0.60**. What was being
  fixed: `notes/reference_observations.txt` entry 2 said the depth bands do not
  separate by value, and the measurement behind this row is worse than the
  entry's channel ranges made it look — **the sky averages luminance 26 and the
  mountains are flat 28**, p05 and p95 both 28, so the two most distant bands in
  the frame are two levels apart out of 255 and the far one is the brighter of
  the pair. There was no knob that could fix that before this commit, which is
  the actual finding: the light pass only adds. **The direction is darker with
  nearness, not lighter** — daylight aerial perspective washes distant things
  toward the sky and that instinct is wrong at night, when the sky is the only
  bright thing and everything in front of it is a cut-out. Three more rows
  arrived unmoved (sky, cells, `world_grade`), and the reason each is identity is
  written at its row rather than left to be guessed at. **The golden checksum
  moved with it**, 0x3d729ad7fbcaa839 → 0x9d9e92a81c4df07b, in the same commit —
  the first deliberate change to the composed frame in the project's history.
  The whole grade mechanism was built and run at identity *first*, and held the
  old checksum, so that this number could only have one cause.
- **2026-08-14** — **two new knobs, both dev-facing, and no existing one moved**
  (T1). The free camera's pan speed and its fast multiplier. In History because
  the table gained a section, not because anything was retuned — **and the
  section is here under a stated protest**, since this file is about how the game
  feels and these are the feel of a tool. The line worth keeping is the admission
  test written at that section: a constant belongs here if you would change it by
  looking at the result, not because it happens to be a speed. Both are first
  values and neither has been used for a session's worth of work yet.
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
