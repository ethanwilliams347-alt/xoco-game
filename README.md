# Toop / Xoco (working title)

A barebones, performant pixel-art destructible physics engine (cellular automata) built from scratch in C++ and SDL2.

## Lore & Premise
Set in a dystopian United States where a Clippy-like AI chat bot took over the government and outlawed humans from participating in the economy. Humans are treated as beasts of burden. Because humans are deemed inefficient and untrustworthy, all intellectual work must be done by AI agents.

To survive, you must use mysterious ancient science to access different worlds. By completing gameplay objectives in these worlds (games like poker, comabt, puzzles), you generate training data for your illegal pet ML agents, allowing them to perform "proof-of-work" tasks on the dark AI economy to earn coins.

## Tech Stack
- **Engine:** Custom C++ Cellular Automata Engine
- **Rendering:** SDL2 (Hardware accelerated textures)
- **Build System:** CMake

## How to Build

1. Ensure you have [CMake](https://cmake.org/download/) and a C++ compiler installed (e.g., MSVC via Visual Studio Build Tools).
2. Open a terminal in the root directory.
3. Generate the build files (this will automatically fetch SDL2 via FetchContent):
   ```bash
   cmake -B build -S .
   ```
4. Compile the executable:
   ```bash
   cmake --build build --config Release
   ```
5. Run the game:
   ```bash
   .\build\Release\SlopPhysics.exe
   ```
   (`.\build\Debug\SlopPhysics.exe` if you built with `--config Debug` instead — whichever config you built is the one whose folder has the exe.) The build step above also copies `assets/` next to the executable automatically, which is what the F4 test scene loads at startup; nothing extra to run first.

## Tuning

[TUNING.md](TUNING.md) is the running log of the feel knobs — player weight,
flight, animation speed, dig reach, light reach — with the file and line for
each and what turning it costs. It is where to look before hunting a constant,
and where to add an entry after retuning one.

## Assets

[ASSETS.md](ASSETS.md) is the reference for getting art into the game — a new
sprite, a player sheet, a location, a backdrop — and for swapping one out to look
at a design. The short version: drop a `.bmp` into `assets/`, then

```bash
python tools/load_sprite.py player_sheet my_new_sheet.bmp
```

binds it in [assets/sprites.txt](assets/sprites.txt), checks it against the
frame grid the code is compiled against, and stages it next to the exe — so it
shows up on the next launch with no rebuild and no code change. `--list` shows
the current bindings.

One thing is still worth knowing: `assets/` is **copied next to the exe at build
time**, so editing a file by hand shows nothing until you rebuild (or run
`load_sprite.py --stage`). That accounts for most "my change didn't show up".
The location BMPs and the prop list are not sprites and are still literals in
`main.cpp`; ASSETS.md has the table.

For the player character specifically, [drawing_to_sprite.md](../drawing_to_sprite.md)
covers the pipeline from a drawing to a validated sheet.

## Running the Tests

The simulation has no SDL dependency, so it is tested headlessly. There are
fourteen suites, one per concern — `grid_test` for the cellular automata,
`player_test` for the character physics, `tool_test` for digging,
`collapse_test` for structural support, `run_test` for the three of them driven
together through one `Run::step()`, `scene_test` for the level loader,
`light_test` for the emissive light field, `anim_test` for the player's
animation selector, `props_test` for the prop list format, `sprites_test`
for the sprite manifest that decides which BMP each key loads, and `debug_test`
for T1's debug tooling — the pause, the free camera's clamp and the cell
inspector's text, none of which would be reachable by any test had they been
written where the keys are bound. Three more cover the renderer's arithmetic:
`backdrop_test` for the wrapping-layer maths, `camera_test` for the view's
framing and its world-edge clamps, and `golden_frame_test`,
which composes a fixed scene through the real renderer and checksums it — **the
one suite that links SDL**, though it still needs no display. CTest runs all of
them.

The last three are not simulation, and they are headless for the same reason it
is: `LightField` produces a plain ARGB buffer, `player_anim` produces a sheet
row and column, and `load_prop_list` produces a list of records, so in all three
cases the part with the logic in it can be tested without opening a window. None
is in `ENGINE_SOURCES` — the first two are in `RENDER_SOURCES` and the third in
`SCENE_PROP_SOURCES` — which is what keeps the day something in `src/physics/`
wants one of them a visible edit to `CMakeLists.txt`.

`props_test` is the odd one in what it spends its checks on: most of them assert
a malformed prop list is **rejected wholesale** rather than parsed with the bad
line skipped. That is deliberate and the reasoning is in `src/scene/props.h` —
this project has twice shipped a scene that rendered, rendered wrong, and said
nothing about it.

```bash
ctest --test-dir build -C Release --output-on-failure
```

## Running the Benchmark

Timings are not part of the test suite — a slow machine should not fail the
build. The benchmark is a separate executable, run it by hand:

```bash
.\build\Release\grid_bench.exe
```

It runs seven scenarios at **two** world sizes — 960x540 and the 1920x1080 the
game actually simulates — and reports milliseconds per step against the 16.67 ms
budget of a 60 Hz frame, then measures the light field separately at the
**widest** display mode's viewport (861x361 cells, 3440x1440) — the mode that
costs the most, so the number is a ceiling rather than a sample. Run it before
and after any change that claims to make the simulation faster.

**Read the 1920x1080 block; the 960x540 block is there to be checked against.**
Every number in `PERFORMANCE.md` before P2 was measured at 960x540, which is a
quarter of the played world, so the small block is kept as the historical series
and as a control: the scenario constants are written to reproduce their old
values exactly at 960x540, and if that block stops matching the numbers on
record, the refactor is what broke rather than the engine.

**Corrected 2026-08-13 — that last sentence asks for the wrong comparison, and
it is the one this page's own next paragraph forbids.** Absolute times are not
portable across sittings, so "matches the numbers on record" cannot be the
control. **What the small block controls is its awake-chunk and peak-fracture
counts**, which are deterministic on a fixed seed: `churning` 105 → 0,
`cascading` 75 → 105, 2,384 peak fractured cells, and at 1920x1080 360 → 90,
270 → 300, 2,348. Those have now reproduced across four sittings and two
different builds of the engine — including one pair whose `churning` times
differ by 32% — while no timing in these documents has ever reproduced across
sittings at all. If a count moves, the engine's behaviour changed; if a time
moves, that may be nothing but the afternoon.

The light section is run once rather than once per size, and deliberately: the
field is sized to the viewport, not to the world, so a second size would produce
a second identical number that invited being read as evidence about world size.

**The last block is the `VENT_RADIUS` sweep**, which runs `churning` and the
recorded session at r=0/2/3/4 — four radii inside one process, so the four rows
are comparable to each other in a way that four builds would not be. Two things
to know before reading it: **every row but r=3 reports a diverged end state and
that is the measurement, not a failure** (a different radius is a different
simulation, so the same inputs are supposed to end somewhere else), and **the
r=3 row reading `exact` is the actual check** — it is what says the shipped
simulation is unchanged. It costs about a minute of extra runtime.

**After it comes the fluid breakdown**, which removes one displacement rule at a
time — `vent_fluid`, `seek_level` and `make_room_above`, the three E5b would
retire — and prices each on `churning` and on the recorded session. Read it with
two things in mind: **the rows are separate simulations rather than a partition
of a step** (removing a rule changes what the world does, so a row's gap from
`all` is that rule's share of *that scenario*), and **`no lift` on `churning` is
a null control** — nothing paints there, so that row prices the instrument and
tells you the table's noise floor. Another minute or so of runtime; the whole
benchmark is now around three minutes.

Read `PERFORMANCE.md` before trusting a number out of it. Timings from different
sittings on the same machine have been seen to differ by more than 2x on
identical code, so a comparison is only worth anything if both sides were
measured back to back — and that document explains how a claim in these docs got
that wrong once already.

### The replayed row, and recording one (P4)

The seven scenarios above are all hand-built, and the plan has twice had to
argue about which of them counts as a realistic frame. The last row of the
benchmark ends that argument by not being hand-built: it replays a **recorded
session** — what someone actually did, one input per fixed step — so it is a
played frame by construction. **It is the row the frame-budget rule in
`ROADMAP.md` is aimed at.**

```bash
.\build\Release\SlopPhysics.exe      # play for a minute or two, then press F9
.\build\Release\grid_bench.exe       # reads session.rec from the current directory
.\build\Release\grid_bench.exe other_session.rec   # or name one
```

**Recording is always on and F9 writes what you have played so far**, from the
first step of the session — not from when you pressed the key. A log has to
begin at a world the replay can rebuild, and that world is the fixture scene
before the first step; there is no way to start one in the middle. Press F9
again later in the same session and you get `session_2.rec`, a longer take,
not an overwrite.

**That protection ends when the game does, and it has already cost a recording.**
The counter behind it lives in the running process, so the *first* F9 of the next
launch writes `session.rec` again and overwrites whatever is there. Session 1
survived on 2026-08-13 only because it had been committed to git; session 2
landed on top of it a day later. **Copy a recording you care about to a name that
says what is in it** — `session_1_painting.rec`,
`session_2_digging_fluids_steam.rec` — before launching the game again. A session
is several minutes of a person's time and is the only benchmark input in this
project that cannot be regenerated on demand. `grid_bench` takes the path as an
argument (`grid_bench.exe session_2_digging_fluids_steam.rec`), so a renamed
recording is not a less convenient one.

**Play the session you want measured.** A minute of standing still measures a
sleeping world and is a worse row than no row. Dig, pour water into the channel,
set something alight, walk somewhere — the kinds of thing step 9 of the manual
checklist asks for.

**The row prints a `contents` census under the timing, and you should read it
first.** It counts what you did (exact, from the log) and samples what was in the
world once a second, in a second untimed pass. It exists because the first
recorded session read **0 of 24,437 steps over budget** and nobody could tell
whether that meant the engine was fast or the session was quiet. The census
answered it: **that session never dug once, never moved a single grain of sand or
cell of water, and peaked at 16 of 510 chunks awake.** A real session, and not a
representative one — see [PERFORMANCE.md](PERFORMANCE.md).

**So: play the expensive cases on purpose, and check the census afterwards.** The
row cannot be quoted as a budget for ordinary play until a session exists that
contains ordinary play. Worth covering, because the first one covered none of
them: **digging** (0 steps last time), **sand falling into water** (untouched),
**steam under a ceiling** (never seen — this is the one E9 has been waiting on),
and **moving around** (the player stood still for 98.4% of it). The census line
`Fire+Water+Sand all present in N of M samples, digging in N of those` is checklist
step 9 asked as one question.

**Session 2 covered them, on 2026-08-13** — 479 dig steps, sand into the channel,
fire under a ceiling, 20,415 steps replayed byte-exact and **0 of them over
budget**. The paragraph above is kept because it is still the instruction for
recording a *new* session, and because the gap it describes is the reason the
project has two recordings instead of one. **A third session is not owed.** If you
record one anyway, the case none of them has covered yet is **movement held
through a collapse** — the one clause of step 9 the census cannot see, since a
collapse is not a material and does not appear in any column.

**Sampled means presence, not absence.** A fire that lit and burned out between
two samples reads as never seen. Do not read a `never seen` row as proof the
session lacked something — read it as the instrument not having caught it.

**Three things invalidate a log**, and the first two are refused rather than
measured:

- **the fixture scene changed** — the replay would start in a different world.
  `scene_test` fails first, on the pinned cell count, which is the cheap warning;
- **the log format changed** — a version mismatch, refused by name;
- **the simulation changed** — which is *not* a failure. The replay reports that
  the end state differs and keeps the timing, because an E-track item that
  changes physics legitimately changes where the session ends up. Only you can
  tell that apart from a stale log, so the bench says what it saw rather than
  guessing.

The row reports a mean, a p99 and a worst step, and how many steps went over
budget. **The mean is the least useful of the four**: a session that sleeps
through 95% of its steps and spends the rest at 40 ms stutters badly and has an
excellent mean. Note also that this row times a whole `Run::step` — grid, player,
dig tool and brush — where every row above it times `Grid::update` alone, so the
two are not directly comparable.

## General Testing

The automated suites prove each mechanic is correct on its own; they never open a
window, so they cannot tell you whether the pieces still work *together*. This is
the short version of that other half — a fundamentals pass anyone can run in a
few minutes with a built copy of the game, no knowledge of the internals needed.

Build, run the exe, and work down the list. Anything that does not behave as
described is worth reporting.

| # | Check | What you should see |
|---|---|---|
| 1 | **Launch** | The window opens and terrain is visible immediately — a snowbank, fence posts, a bridge, a water channel. The console prints a `World seed:` line and a `Scene: …, N cells placed` line; **`N` should be large, not zero.** |
| 2 | **HUD** | Top-left shows frame rate, the selected material, brush size, health, and the number of awake chunks. |
| 3 | **Movement** | Walk left and right, jump, land. The character rests flush on the ground, faces the way it walks, and cannot pass through walls. Small one-cell steps are walkable without jumping. |
| 4 | **Flight** | Hold jump in mid-air. The character beats its wings in countable downstrokes — climbing should feel like slow, laboured work, never like a hovering helicopter. |
| 5 | **Digging** | Left-click carves a round hole in terrain. The crosshair is solid white in range and dim white out of it. Cut the bottom out of a sand pile and everything above it falls in. |
| 6 | **Materials** | Keys `1`–`8` select materials; `8` erases. Sand piles into slopes, water spreads flat and settles still, oil floats on water, steam rises to the ceiling. |
| 7 | **Fire and steam** | Fire spreads along wood as a moving front and eventually burns out on its own. Fire beside water makes steam; steam gathers under a ceiling, waits, then drips back as water. |
| 8 | **Settling** | Leave a busy scene alone. The awake-chunk count in the HUD should fall back to zero — a world at rest costs nothing. |
| 9 | **Structures** | A wall or wood shape with nothing under it falls as one rigid piece and lands intact. A shape resting on solid ground never twitches or drifts. |
| 10 | **Depth** | Walk a long way and watch the background. Sky drifts slowest, mountains faster, terrain fastest. No visible seams or repeating vertical lines anywhere in the backdrop. |
| 11 | **The run** | Take falling damage from a real drop, take burn damage standing in fire, die and see `YOU DIED`, press `R` to restart, then fly east across the water channel to reach the objective. |
| 12 | **Stability** | A few minutes of doing all of the above at once — digging near falling sand near fire near water — with no crash and no obvious slowdown during ordinary play. |

Developers: the long form of this list, with the specific regression each step
exists to catch and the numbers needed to tell a known cost from a new defect, is
in **[MANUAL_TESTING.md](MANUAL_TESTING.md)**. Run that one after any change to
`src/physics/`, `src/game/` or `main.cpp`.

## Controls

**Player**
- **`A` / `D`** or **arrow keys:** Walk left and right.
- **`Space`** (or **`W`** / **up arrow**): Jump.

- **Left-Click:** Dig. Cuts a hole in whatever solid terrain the cursor is
  aimed at, up to a limited range. The crosshair replaces the mouse pointer
  inside the window and shows whether the target is reachable: **solid white
  is in range, dim white is out of it.**

**World (development tools)**
- **Right-Click & Drag:** Spawn elements onto the screen.
- **Mouse Wheel:** Grow / shrink the brush.
- The eight material keys below are drawn as a **hotbar** along the bottom of
  the window, so which key places what no longer has to be memorised. The
  selected slot is framed in that material's own colour. The icons are
  hand-authored 8x8 bitmaps in `src/ui/hotbar.cpp` that take their colours from
  `MATERIALS`, so an icon cannot drift from the palette it depicts, and the key
  bindings come out of the same table the icons do, so a slot cannot lie about
  what its key does.
  The row is ordered by *behaviour* rather than by the order the materials were
  added — the two structural solids, then the powder, then the liquids, then the
  gases, with the eraser last — so neighbouring keys do roughly alike things and
  a mis-hit is survivable. **Note that this moved the eraser from `4` to `8`.**
- **`1`**: **Wood** — solid; catches fire when heated, then smoulders for seconds.
- **`2`**: **Wall** — solid, immovable terrain.
- **`3`**: **Sand** — powder, piles into a slope.
- **`4`**: **Water** — liquid, spreads to find its level.
- **`5`**: **Oil** — liquid, lighter than water so it floats on top; ignites fast and flashes rather than smouldering.
- **`6`**: **Steam** — gas, rises and pools against the ceiling.
- **`7`**: **Fire** — gas; rises, fades from white-hot to red, and dies within a fifth of a second. A flame is what burning *throws off* — the thing actually on fire is the charred wood underneath it, which is what heats its neighbours and spreads the burn.
- **`8`**: **Eraser** — deletes pixels.
- **`ESC`**: Open the settings menu. **This used to quit outright**, which is the wrong thing for a key sitting next to a menu — quitting is now an item inside it, so it takes two deliberate presses.
- **`R`**: Start a new run — **only once the current one is over**, and inert while you are playing. A key that throws a session away is a bad one to mis-hit, which is the same reason quitting moved off `ESC`.

**Debug tooling (`T1`, dev-facing)**
*None of this is player-facing and none of it changes the simulation. It exists because the two items after it — powders coming to rest (`E10`) and per-cell velocity (`E5a`) — are both verified by looking closely at a place, and until now the camera was bolted to the player, the world could not be stopped, and a cell's state could not be read at all.*
- **`P`**: Pause and resume. The world freezes by the same mechanism the settings menu and a finished run use — time stops accumulating — so nothing is banked and there is no burst of catch-up steps on resume. A `PAUSED` line appears under the HUD, because a stopped world that says nothing is indistinguishable from a hung one.
- **`.`**: Advance exactly one fixed step. Only while paused; ignored otherwise, since a step on top of the steps a frame already runs is a stutter rather than a single-step. Holding it steps at the key-repeat rate, which is the way to scrub through a collapse. **The brush and the dig tool act on the step you take** — they are part of a step, not of a frame — so pausing to line up a stroke and pressing `.` is how you place material precisely.
- **`F`**: Detach the camera from the player and pan it with the movement keys (**hold `Shift`** for fast). The body stands still while the camera is loose rather than walking off unwatched. **The brush still paints wherever the camera is looking**, which is the point — it is how a test scene gets built somewhere the player is not. Digging still works too, but it is aimed and range-limited from the body, so it will not reach. `F` again returns the camera.
- **`I`**: The cell inspector — a line under the HUD reading what is in the cell under the cursor: material, temperature, whether its chunk is awake, and whichever of the fall clock, gas lifetime and piece tag apply. `E2` and `E3` both added per-cell state that has never been readable while the thing being debugged was on screen.
- **`Ctrl`+`R`**: Reset the world unconditionally, mid-run included — the debug version of `R`. It keeps the current seed, so the world you were debugging comes back exactly; a fresh seed would also invalidate the session recorder's header, which is written once at startup. The modifier is what keeps it away from a key you might mis-hit.
- **`CHUNKS:` on the HUD gains `+FALLING`** when a structural piece is in the air. `CHUNKS:0` on its own does *not* mean the world has stopped — a falling slab is carried by the support queue rather than by the chunk rects, so the counter can read zero the whole way down.

- **`F9`**: Write everything played so far to `session.rec` — the benchmark's replayed row (P4). A line appears under the HUD saying what was written and how many steps it holds. Every session is recorded from its first step whether or not you ever press this; see [The replayed row](#the-replayed-row-and-recording-one-p4). It is on `F9` rather than a letter because every letter within reach of the movement keys is a hotbar slot, and a key that writes a file is a bad one to hit while reaching for sand.

**Settings menu** (`ESC`)
- **Arrow keys / `W` / `S`:** Move the cursor. **`Enter`:** Choose. **`ESC`:** Resume.
- Offers 1920x1080, 2560x1440 and 3440x1440. The pixel scale is fixed at 4 screen pixels per cell in every mode, so a wider window shows *more world* rather than a bigger one — 480x270 cells at 1080p up to 860x360 at 3440x1440, which is the framing of the reference footage this game's art is measured against. A mode larger than your desktop is listed but greyed out rather than hidden.
- The chosen mode is written to `settings.txt` beside the executable and reloaded next launch. If that file names a resolution the current display cannot fit — a monitor changed between runs — it is ignored with a note on stderr and the largest mode that does fit is used. The mode actually chosen prints at startup next to the seed.

**The run (`S0`)**
- You have **100 health** and two ways to lose it: **standing in something hot**
  — a flame, or wood that has caught — and **landing too fast**. Contact only,
  so a fire across the room does nothing; air carries no heat in this engine and
  that is a simulation property rather than a gameplay simplification. A drop of
  about three body heights is free and a terminal-velocity landing costs 80, so
  the worst fall in the game is survivable exactly once. **The drop you take at
  spawn is free** — it is several hundred cells and would otherwise be the
  hardest fall you ever take.
- **Beating your wings arrests a fall**, which is what makes a bad landing a
  mistake rather than an accident.
- The **objective** is the gold-and-white marker east of the water channel,
  ~740 cells from where you spawn. The `GOAL:` figure in the HUD is the distance
  to it in cells and which way it lies. The channel is walled on both sides and
  full nearly to the top: **you cannot walk there**, and flight is the intended
  answer.
- Reaching it ends the run as a win, running out of health ends it as a loss.
  Either way the world **freezes where it was** rather than clearing, so the
  thing that killed you is still there to look at, and `R` starts a new one.
- **Everything about this is deliberately thin** — one objective hard-coded into
  the test scene, two damage sources, no save, no death animation. See `S0` in
  [ROADMAP.md](ROADMAP.md) for what it is not, and why it was built before the
  rest of the engine track.

The HUD in the top-left corner of the window shows health and the objective
bearing, then the current framerate, selected material, brush size, and
awake-chunk count (see [Chunked updates](#chunked-updates)); the window title bar
itself is just a static label. The player spawns in mid-air in the middle of the
world; the F4 test scene loaded at startup gives it plenty to land on, and the
brush still works for drawing more terrain anywhere else.

## Engine Architecture

The simulation lives in `src/physics/` and knows nothing about SDL — `main.cpp` is the only file that opens a window or reads input.

Materials are **data, not code**. Each one is a row in the `MATERIALS` table in
[material.h](src/physics/material.h) describing its colour, density, thermal
properties (see [Heat](#heat)), and which of four generic behaviours it
follows:

| Behaviour | Movement |
|-----------|----------|
| `Static`  | holds its shape; falls as a rigid piece if unsupported (Wall, Wood) |
| `Powder`  | falls, then slides diagonally into a pile (Sand) |
| `Liquid`  | falls, then spreads sideways to find its level (Water, Oil) |
| `Gas`     | rises, then spreads (Steam) |

Density decides what sinks through what, so sand sinking in water and oil floating on
water both fall out of the same rule rather than being special-cased. Adding a material
means adding a table row, not editing the update loop.

### Liquids find their level

Falling and spreading sideways is not enough to make something read as a fluid.
The density rule refuses every upward move unless the mover is lighter than its
target, and `Empty` has density 0, so a liquid can never rise under any
circumstances — which means a U-bend can never equalize. The short arm has no way
to gain a cell. Water ends up behaving like a powder that happens to flow.

So a liquid cell that has run out of ordinary moves, and has `Empty` directly
above it, searches its own connected body for **another surface at least two rows
lower**, and moves there. It is not a pressure field: a real solve is a second
simulation with its own convergence behaviour and its own state to save, and this
buys the same visible result for a bounded search — 64 cells, orthogonally
connected, same material only.

**The tall side moves down; the short side does not rise.** That direction is the
whole design, and the other one was tried first. Rising is a swap, so it leaves a
bubble of `Empty` *inside* the body, and the transfer is not finished until the
ordinary fall and spread rules have walked that bubble back down the arm, across
the join and up the far side — twenty-odd steps, during which the bubble cuts the
body in two, the search transiently answers "no", the cells stop marking
themselves dirty, and the chunk sleeps with the world still out of level. Every
fix for that amounts to keeping unlevel bodies awake, which charges every pool in
the world for the one that needs it. Moving the tall cell instead makes each
transfer a single atomic swap, so there is no journey to stay awake for, and the
wake-up is automatic: the vacated cell's 3x3 mark is exactly the cell below it,
which is the next surface cell and the next one to move. A body equalizes at one
cell per step and then sleeps, with no self-wake rule of its own.

Two consequences worth knowing:

- **Level means level to within one cell.** The two-row threshold is hysteresis,
  not a tuning preference. Each transfer moves the two surfaces one cell towards
  each other, so a one-row threshold would flip which side was high, forever —
  level on average, awake and costing full price the whole time.
- **A cell can travel further than one cell in a step.** Bounded by the search,
  and only ever between two points of one connected body of the same liquid. What
  it looks like is one side dropping while the other rises, which is what a U-tube
  does.

Conservation is what keeps the whole thing honest, because the obvious way to make
water level is to invent some: the move is a swap with the `Empty` above the
receiving surface, so nothing is created and nothing is deleted, and there is a
test that says so alongside the one that says it levels.

### Chunked updates

The world is divided into 64x64 chunks. Each chunk stores the bounding box of the
cells inside it that might still move; a chunk with nothing moving is skipped
entirely, so a settled world costs almost nothing to simulate.

The rule that keeps this correct: **every write wakes its 3x3 neighbourhood**, not
just the cell that changed. Digging a grain out from under a pile has to wake the
grains above it or they hang in mid-air over the hole. Because the neighbourhood is
resolved per cell it crosses chunk borders naturally, which is what stops the same
bug reappearing as seams along the invisible chunk lines.

All writes go through `set_element` and `swap_elements`, and both call
`mark_dirty`. Any new code that mutates cells must go through them too.

The HUD (top-left corner of the window) shows how many chunks are awake. In an
idle world it should sit at or near zero.

### Reactions

Movement is data-driven; transformation is the second axis. Each row in the
`REACTIONS` table in [reaction.h](src/physics/reaction.h) is a rule of the
shape `catalyst + target -> result`, gated on the target's temperature and
rolled once per eligible cell per step:

| Catalyst | Target  | Temperature | Chance | Result  |
|----------|---------|-------------|--------|---------|
| Water    | Fire    | any         | 90%    | Steam   |
| *(none)* | Wood    | ≥ 120       | 100%   | Charred |
| *(none)* | Oil     | ≥ 90        | 100%   | Fire    |
| *(none)* | Water   | ≥ 100       | 100%   | Steam   |
| *(none)* | Steam   | ≤ 26        | 100%   | Water   |
| *(none)* | Charred | any         | 0.6%   | Empty   |

A catalyst of `Count` means "no neighbour required". Rows are checked in order;
the first row whose target, catalyst and temperature conditions all match is
the only one considered that cell that step, which is what makes dousing (row
1) take priority over anything below it without any special-casing.

Chances are stored per *mille*, not per cent, and row 6 is why: it is a burn
duration in disguise. A spontaneous decay row is a lifetime — mean steps is
1000/chance — so 6 per mille is about 167 steps, near three seconds at 60 Hz.
Whole percents bottom out at a hundred-step mean, which is not long enough for
wood, and that is the whole reason the column is wider than it looks like it
needs to be.

**Fire is not in this table as something that burns out, and that is the
important change.** Wood does not become Fire; it becomes **Charred** — still
solid, still structural, still holding up whatever it was holding up, and hot
enough (a declared 200° heat source) to bring its neighbours to their own
ignition point. That is what spreads a fire. The flame you see is thrown off it
by the `emits` column in `MATERIALS`, into a randomly chosen *empty* neighbour,
and lives twelve steps on a countdown in `Element::ticks` before it disappears.

Three things fall out of that shape rather than being written as rules:

- **Fire hugs surfaces and never fills a volume**, because a buried cell has no
  empty neighbour to emit into and so does not visibly burn at all.
- **Fire spreads sideways as readily as upwards**, because what spreads it is
  conduction from a cell that cannot move, not a rising gas that has to stay in
  contact with its fuel.
- **Flames ramp from white-hot to dim red**, because the countdown gives every
  flame cell an age, and age is the only thing that differs between two cells
  sharing one `MATERIALS` row.

Oil is the exception and it is deliberate: it flashes straight to Fire with no
smouldering state, because it is a Liquid, and a burning state on something that
moves would have to survive `swap_elements` and every fluid rule. Only static
materials smoulder.

**Only one row still rolls dice, and that is the point of the table above.**
Ignition used to be a 12%-per-step chance for Wood touching Fire, which is why
fire spread by luck rather than by heat and never looked like it was burning
*through* anything — there was no state between "wood" and "on fire" for the
eye to follow. Wood now ignites because it got hot, and how long that takes is
set by its conductivity. Dousing keeps its chance and is deliberately *not*
temperature-gated: water puts a flame out because it is water, and a cold
splash should not be less effective than a warm one. Burnout keeps a chance rather than a threshold, because a lifetime is not a
threshold and has nothing to gate on — and after E9 that row is Charred's, not
Fire's, since the thing with a lifetime is the fuel.

**A second wake rule, alongside chunking's.** A cell that stops moving stops
generating `mark_dirty` calls and its chunk goes back to sleep — that's the
whole point of chunking. But a spontaneous decay doesn't need movement to happen;
a burning cell boxed in with nowhere to go would take its one shot at the roll
on the frame it was created and then freeze forever, un-woken, never given
another chance to decay or to ignite what it's touching. So a cell that
is a *spontaneous* reaction target **and is currently inside that row's
temperature window** marks its own 3x3 neighbourhood dirty every step,
movement or not. Both halves of that are load-bearing: without the first, Fire
freezes; without the second, every wooden beam and every pool in the world
would self-mark forever, since they are spontaneous targets too, and chunking
would be handed back its entire saving. Cold Wood, cold Water and cold Steam
stay fully sleep-eligible.

### Heat

Every cell carries a `uint8_t temperature`, ambient (20) unless something has
heated it. It rides in padding `Element` already had, so it cost no memory —
`sizeof(Element)` is still 12, asserted at compile time rather than counted.
The scale is read as degrees Celsius so the constants mean something: water
boils at 100, wood catches at 120, a flame holds 250.

Three columns in `MATERIALS` drive it. `conductivity` sets both how fast a
material takes heat on and how fast it sheds it; a pair of neighbours exchanges
at the *lower* of the two, so an insulator between two conductors stops the
heat rather than averaging with it. `spawn_temperature` is what a freshly
placed cell gets, and defaults to "whatever the spot was already at" — heat
belongs to the place, so material dug out of a hot wall arrives hot, and an
ignited Wood cell becomes Fire that is already burning rather than a flame
starting from room temperature. `heat_source` is the temperature a material
holds itself at regardless of its surroundings, and Fire is the only row in
the table that has one.

`Empty` has conductivity zero: air is not simulated, so heat travels through
matter in contact and nowhere else. That is a deliberate simplification and it
is most of why the pass is affordable — a settled pool is hundreds of cells,
the air above it is tens of thousands.

**Integer arithmetic only**, because floating-point diffusion would put
cross-platform nondeterminism straight back into `Grid`. Three properties fall
out of the integer form and each one is doing a job:

- **A dead band.** Two cells within one degree exchange nothing. Without it a
  pair would trade a unit back and forth forever and no chunk containing
  anything warm could ever sleep. The cost is that "ambient" means ambient to
  within a degree — the same trade [Liquids find their
  level](#liquids-find-their-level) makes for "level".
- **A floor of one unit**, so a slow conductor across a small difference does
  not truncate to zero and stall partway.
- **A ceiling of half the difference**, so an exchange never overshoots and
  turns into an oscillation.

Heat conducts across all **eight** neighbours, unlike the pressure search,
which is orthogonal only. The difference is not inconsistency: a diagonal step
there would move *matter* through a seam with no contact area, whereas heat
through a corner is harmless — and refusing it breaks the feature outright.
An ignited Wood cell becomes Fire, Fire is a gas, so it rises out of the beam
on the next step, leaving the flame that should light the next cell along
sitting diagonally above it and nowhere else. With four neighbours the fire
front stalls after exactly one cell, and no conductivity fixes it.

That argument was written when Wood ignited straight into Fire, and E9 has since
replaced its subject: the cell that stays put and conducts is `Charred`, which is
Static and never had anywhere to rise to. **The eight neighbours matter more now,
not less.** The whole propagation story is "the burning cell holds its place and
heats what it touches", so the set of cells it can reach is the only thing
deciding where fire goes — and a four-neighbour version would refuse to carry a
fire diagonally across a gap that is plainly touching on screen.

**A cell sitting at exactly ambient does no thermal work at all.** This is the
difference between heat costing 18% of the worst-case frame and costing 2%,
and it is exact rather than an approximation: conduction writes both ends of an
exchange by the same amount, so it does not matter which of a pair initiates
it, and a cell cannot be off ambient and asleep. Heat is also the only thing in
the engine that *leaves* — every cell bleeds slowly back towards ambient, which
is what stops a single candle eventually cooking the map, and what lets a
burnt-out scene go back to sleep. See [PERFORMANCE.md](PERFORMANCE.md) for the
bracketed numbers.

### Determinism

`Grid` is a pure function of its seed. There is no random generator anywhere
in the simulation — `src/physics/random.h` holds a stateless hash instead,
and every random value is a pure function of `(seed, step, index, stream)`.
Nothing carries state between draws, so two grids built with the same seed
and stepped the same number of times are byte-identical, and a save file
only ever needs to record the seed and the step count to say where a run
had got to.

**The write rule has a counterpart for randomness: every random draw goes
through `Grid::coin` / `Grid::chance`, and no other code calls the hash
directly.** That is what keeps the invariant checkable rather than assumed
— a stray call reaching for its own randomness would not look wrong at the
call site, only in a diverged replay much later.

Each call site is tagged with its own `Stream` (colour jitter, sweep
direction, powder/fluid direction, reactions), so two decisions about the
same cell on the same step never draw the same number — without that, a
cell that rolled to move left would always roll the same side of its
reaction check too, a permanent correlation rather than a one-off
coincidence. World generation, when it exists, gets its own separate range
of streams reserved for exactly this reason: generating one extra cave
must never change how sand falls somewhere that cave doesn't touch.

One deliberate exception: colour jitter hashes on position only, with no
step in the input, because it's a one-time authored value rather than a
per-step decision — a cell erased and repainted in the same spot comes
back the same shade instead of a new one.

**This used to say "covers the simulation, not yet the game"**, on the
grounds that the brush painted once per rendered frame and a held key was
sampled once per frame and replayed into every fixed step inside it. **F2.3
closed that** — one `Input` now drives exactly one fixed step, brush
included (`src/game/run.h`), so a recorded sequence replays the same
however the original session was paced. The old wording is kept here rather
than deleted because it was true for several revisions and the same
sentence would otherwise be written again from memory.

**What it covers now, stated exactly, because two later items spend this
sentence as a guarantee.** The grid, the input path and — since F5,
2026-08-12 — the player's motion are integer arithmetic end to end and
reproduce on any conforming compiler. **`DigTool::march` was the one
remaining exception and is not one since F6, 2026-08-13** — it picked which
cells a dig removes using a `float` `sqrt` and two `lround`s, and digging
writes to the grid, so a replay containing a dig reproduced within one
binary and not across toolchains. It is now a squared range comparison and
an integer rounded division, and **no float under `src/physics/` reaches
the grid.** (Two floats remain there and are renderer boundaries rather
than exceptions: the player's `visual_x()`/`visual_y()` and the dig tool's
`swing_progress()`, which only the animation reads.)

**The claim that follows from that is narrower than it sounds, and the
distinction is deliberate.** The simulation is integer arithmetic end to
end, so it reproduces on any conforming compiler *in principle* — and the
project has still only ever been built on one machine, so that is a
reasonable expectation rather than a verified fact. "Build on macOS and
Linux at least once" is a release-gate prerequisite for exactly this
reason, and it is the only thing that can turn the one claim into the
other.

Replacing the generator with the hash cost a small amount rather than
saving one — see the RNG entry in `ENGINEERING_NOTES.md` for
the measured number and why it was recorded rather than assumed.

### The player

The player is the one thing in the engine that is **not** a cell. It is a
8x20 axis-aligned box in [player.h](src/physics/player.h) with its own position
and velocity, and it only ever *reads* the grid — it never writes a cell, so it
cannot break the "all writes go through `set_element`" rule.

That split is deliberate. A cell moves at most one step per frame in one of
eight directions, which is right for sand and useless for a character that needs
sub-cell speed, a jump arc, and a body several cells tall that has to stay in one
piece.

Position is an **integer cell plus a sub-cell remainder**, not a float. Collision
then only ever compares whole cells, so a resting player sits at an exact cell
rather than a hair inside the floor, and the whole class of "the box is 0.0001
into the wall" bugs never comes up. The remainder carries the fractional part of
a move into the next step, which is what keeps motion smooth below one cell per
step.

**The remainder and the velocities are fixed point** — `fx`, signed 16.16, in
`src/physics/fixed.h` — and were `float` until 2026-08-12. Nothing about the
character's motion changed; what changed is that it is now the same motion on
every machine, since float arithmetic is not reproducible across compilers,
optimisation levels or architectures and the sub-cell remainder is precisely
where a last-bit difference grows into a whole cell. Speeds are stated in cells
per *second* and converted to a per-step amount by `fx::per_step()`; the
timestep is not a parameter anywhere, because it is fixed and a parameter nobody
varies is an invitation to vary it. `visual_x()`/`visual_y()` are still float and
are the only ones: they exist so the renderer can interpolate between steps, and
nothing in `src/physics/` may read them.

Movement resolves one cell at a time, each axis separately. Sub-stepping makes
tunnelling impossible by construction rather than by being fast enough — a
player falling at terminal velocity still tests every cell it passes through.
Resolving the axes separately is what lets the player slide along a surface
instead of sticking to it.

**What counts as solid** is derived from the material table, not listed
separately: `Static` and `Powder` are solid, `Liquid` and `Gas` are not. So the
player stands on sand and falls through water, and a new material gets correct
collision the moment its row is added.

Two rules do the rest:

- **Step-up.** A blocked horizontal move retries with the body lifted up to
  `MAX_STEP_HEIGHT` cells — the number lives in [TUNING.md](TUNING.md), and this
  line said "2" long after it stopped being 2, which is why it now says the
  name. That is the whole of "walking over uneven powder" — a settled sand
  slope is a staircase of one-cell steps, and without this the player would have
  to jump over every grain. Grounded only, so you cannot climb a shaft by
  nudging into the wall mid-air.
- **Unstuck.** The grid does not know the player exists and will drop sand into
  the cells the body occupies, so "body overlaps terrain" is a state that occurs
  in normal play, not just through a bug — and every direction being blocked
  would freeze the player permanently. When it happens, the body searches
  outward for the nearest position it fits in and takes it, preferring straight
  up. Buried deeper than the search radius, it grinds upward a cell per step
  until it reaches open air.

### Interaction

Digging lives in [tool.h](src/physics/tool.h), **not** on `Player`. The body and
the verb are separate concerns, and the split has a concrete payoff: `Player`
holds only a `const Grid&`, so it cannot break the "all writes go through
`set_element`" rule even by accident. Tools take a mutable `Grid&` and are the
only player-side code that does.

A dig is a **ray marched one cell at a time** from the player's centre toward
the cursor, stopping at the first solid cell, which is then blown out to a small
radius. One cell per step for the same reason the player's movement sub-steps: a
ray that samples every Nth cell can pass straight through a thin wall and dig
the terrain behind it — and that wall is exactly the one the player was
sheltering behind. What stops the ray is the same `is_solid` the player collides
against, so terrain and powder block a shot while water and fire do not. One
definition, used twice.

Range is measured as real distance rather than as a step count, so a diagonal
dig does not reach 1.4x as far as a straight one.

**Digging is a swing, not a rate limit**, and the difference is what session 5's
D1 bought. The tool holds a 36-step swing clock, counted in fixed steps rather
than seconds so it runs at the same speed on every machine.

**The hole comes out on the swing's first step and the rest is follow-through.**
Putting the impact partway in is the more literal reading of a swing's arc, and
it was tried; it costs half a second between the press and the world changing,
on a tool used constantly, and a dig that lands late reads as input lag rather
than as weight. Landing it immediately also means the aim used is the aim at the
moment of the press, with no window for the world to move underneath it. Holding
the button starts the next swing on the step the last one ends, so it cycles
seamlessly. A shot that connects with nothing starts no swing at all, which makes
the limit read as tool speed instead of as a random input lockout.

This is also **the only clock the dig animation has** — `player_anim` is handed
`DigTool::swing_progress()` and divides it by its own frame count. The swing used
to be timed twice, once in the tool and once in the sprite table, and the two
disagreed.

**Digging destroys matter, deliberately.** The conservation-of-matter test
covers `Grid::update()` — the simulation itself still never creates or deletes a
cell, and that invariant is intact. Digging is an *external* write that removes
matter outright, which is correct for a tool and would be a bug anywhere inside
the step loop. The two are testing different things; don't reconcile them.

Removal goes through `set_element` like everything else, and that is the entire
reason digging out the base of a sand pile makes the pile collapse instead of
leaving it hanging over the hole — each removed cell wakes its 3x3
neighbourhood. There is a test for exactly that.

The world border cannot be dug through. `set_element` bounds-checks, so the part
of a hole that falls outside the world is silently dropped.

### Structures and falling

`Static` materials hold their shape, which means they will also hold it
somewhere they have no business holding it — dig the ground out from under a
stone slab and it stays in mid-air, while the sand beside it falls correctly.
The inconsistency is visible side by side, which is what makes it read as a bug
rather than as a rule.

So Wall and Wood can now lose support, and when they do the whole connected
piece **falls as one rigid body, keeping its shape the entire way down**. It is
not converted into loose grains: a slab that dissolves into gravel the instant
it comes free just reads as a different bug. The shape is what makes it look
like masonry.

The piece **stays in the cell grid while it falls**. It is a rigid body in how
it *moves*, not in where it *lives* — so rendering, player collision, digging,
fire and every other system keep working on it with no special case anywhere.
Which materials count as structure is a `structural` flag in the same
`MATERIALS` table. Same discipline as solidity: one table, not two.

**A piece that lands with speed on it breaks.** Dropping rigidly with the shape
perfectly intact is what made masonry descend like an elevator, so a piece that
comes down across uneven ground splits into two that are separate from then on:
the half over the drop carries on down, the half that landed stays. Fracture,
not rotation — true rigid-body rotation on a cell grid means resampling the
piece every step it turns, which destroys the exact authored pixels that are
the whole visual pillar, and masonry mostly breaks rather than tips anyway.

**The crack goes where the support ends.** It is not a random line through the
piece; it is the boundary between the columns that landed on something and the
columns that landed on nothing, which is the only place a break changes
anything. A piece landing flat on flat ground does not break at all.

**A crack is a disagreement between two cells, not a line between them.** Each
cell carries a `piece_tag`, and the support fill only crosses between cells
whose tags match. That is what makes a crack survive the piece moving — cells
carry their tag when they move, the same way they carry their colour — and
persistence is the entire feature. Breaking a piece in mid-air instead would do
nothing at all: both halves are unsupported, so both fall on exactly the same
steps, and the next fill finds them touching and treats them as one piece
again.

**Fracture can never start a collapse, only finish one unevenly.** It is
reachable only from a landing that arrived with speed, and a piece at rest has
`ticks` of zero, so nothing that was standing still can be broken by it.
That matters more than it sounds: a missed collapse is invisible, while a wrong
one turns a level into rubble, and this is the change in the engine most able
to get that wrong. The guarantee is structural rather than a matter of care.

**Support is a flood fill.** From a disturbed structure cell, walk the connected
piece looking for one cell that is *grounded* — meaning the bottom of the world,
or something solid directly beneath it that is not part of the same piece. One
grounded cell holds the whole thing up. Powders bear load and liquids do not, so
a slab resting on packed sand stands, and the same slab over water sinks through
it.

**A fill records what it concluded, not merely where it went** — and that is
load-bearing rather than an optimisation. One disturbance queues many cells to
re-check, and a fill stops the instant it finds one grounded cell, so it leaves
a partial trail of marks across a piece it has just decided is held up. If the
marks only said "seen", the next queued cell would start its own fill, find that
trail in the way, never reach the grounded cell behind it, conclude the piece is
unsupported, and drop whichever fragment it could reach. A wall standing on
solid ground would shed chunks of itself for no visible reason, one chunk per
step, which looks exactly like the material particlising instead of holding
together. So each mark carries its verdict, and a fill that runs into a settled
cell adopts that verdict instead of re-deriving it — two connected cells are the
same piece, so its answer is already this piece's answer. It also makes the
common case cheap, since the hundreds of cells queued off one piece now cost one
fill between them rather than one fill each.

**Falling is a per-column, bottom-up shift of one cell.** Processing
each column from its lowest cell upward means a cell only ever moves into space
its lower neighbour has already left, and it handles a column containing two
separate parts of the same piece — an arch — without having to find those parts
explicitly. An L-shaped piece keeps its corner, which is the test that proves
the piece moves as one rather than as independent columns.

**A falling piece accelerates by falling repeatedly, not by falling further.**
Everything above happens once per *cell of travel*, and a piece gets more than
one cell of travel in a step by having the entire question — flood fill,
grounded check, shift — run over again from scratch. It comes loose at one cell
per step, gains a cell for every four steps in the air, and tops out at eight.

Doing it this way is not a shortcut, it is the point. Because a piece never
moves a cell without first re-deriving what is holding it up, it is
*structurally* incapable of stepping over a floor thinner than its speed. Write
it the obvious way instead — look once, then move eight cells — and a slab at
full pelt sails straight through a one-cell shelf, which then needs its own
special-case guard, which then has to be got right. The cost is that speed 8
means eight times the work in that step; the ceiling exists to bound exactly
that, and it is the constant to reach for if falling rubble ever shows up in a
profile.

Speed lives on the cells, as a count of steps spent falling. That is not where
you would put it if you had a choice — it belongs to the piece — but a piece
has **no identity between steps**. It is re-discovered by flood fill every
time, so the cells are the only thing that persists. Moving a cell carries the
count along for free, since a move is a swap of whole elements. The count is
zeroed by the same code path that concludes a piece is supported, which is the
only way a piece ever stops falling — without that, a slab that fell a hundred
cells and landed would still be "at speed" whenever it was next dug free, and
would leave the ledge like it had been fired rather than tipping off it.

The move is always legal when the piece is unsupported, and that falls out of
the definitions rather than needing a check: "unsupported" means nothing solid
is under any part of it, so every cell is moving into empty space, into a fluid,
or into space the piece itself just vacated. Because the move is a swap and
structural materials are denser than every fluid, the water displaced from
underneath surfaces on top of the slab instead of being deleted.

Two decisions are worth knowing about, because both are visible in play:

- **Support is checked on disturbance only, never as a global truth.** Sweeping
  the world every step would cost more than the simulation it is attached to,
  and a world as authored is assumed to be standing up on purpose. A piece
  nobody has touched is never questioned — so a floating platform drawn with the
  brush stays exactly where it was put, and only starts falling once something
  removes part of it or slides out from under it. Placing structure never
  triggers a check; only removing it does.
- **Pieces over 4,096 cells are assumed supported rather than judged.** The
  asymmetry is deliberate. A missed fall is invisible — a slab that should have
  come down simply doesn't. A wrong fall drops the level. When the answer is too
  expensive to compute, guess the harmless way.

`swap_elements` now asks whether the cell above each end of a move is structure,
and `swap_elements` is the hottest path there is, so this was expected to cost
something. It does not measurably: a bracketed A/B against the same binary with
the check compiled out cannot separate the two. An earlier revision of the docs
claimed about 5%, which turned out to be the benchmark measuring the machine
rather than the code. PERFORMANCE.md has the numbers, the method, and why the old
figure was wrong — it is worth reading before trusting any timing in this
project.
