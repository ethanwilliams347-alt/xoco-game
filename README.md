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
seventeen suites, one per concern — `grid_test` for the cellular automata,
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
one suite that links SDL**, though it still needs no display. It carries two
checksums as of 2026-08-18: one for the world the composition draws, and a
second taken after the screen-space layer — the HUD, the reticle, the hotbar,
the run-over wash and the settings screen — is drawn on top of it. They are
separate numbers so that a change to the UI and a change to the sky cannot be
confused for each other. `boot_test` covers
what the game decides before its first frame — where the objective and each prop
are planted on the terrain actually under them, and which display mode to open
at — and it runs the *shipped* scene, so the two lines a launch used to be
checked by are assertions instead. `shell_test` covers the decisions the shell
takes every frame and every keypress — how much simulated time a frame buys,
what freezing the world means, where between two steps the picture falls, and
the settings menu's navigation and selection. The seventeenth is
`docs_test`, which is not about the engine at all: it asserts this document's
and the others' checkable numbers — the suite count in this very paragraph
included — against the code and files they are drawn from, so a number that goes
stale fails the build rather than surviving until somebody notices. CTest runs
all of them.

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

It runs seven scenarios at two world sizes, then a light-field measurement, a
`VENT_RADIUS` sweep, a fluid-rule breakdown, and a replayed recording of a real
session — about three minutes in all. **What each block means, how to record a
session with `F9`, and what makes a number worth quoting are in
[PERFORMANCE.md](PERFORMANCE.md#running-the-benchmark)**, which owns the topic.
Read it before trusting a number out of the bench: timings from different
sittings on the same machine have differed by more than 2x on identical code.

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

- **`F9`**: Write everything played so far to `session.rec` — the benchmark's replayed row (P4). A line appears under the HUD saying what was written and how many steps it holds. Every session is recorded from its first step whether or not you ever press this; see [The replayed row](PERFORMANCE.md#the-replayed-row-and-recording-one-p4). It is on `F9` rather than a letter because every letter within reach of the movement keys is a hotbar slot, and a key that writes a file is a bad one to hit while reaching for sand.

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
awake-chunk count (see [Chunked updates](ENGINEERING_NOTES.md#chunked-updates)); the window title bar
itself is just a static label. The player spawns in mid-air in the middle of the
world; the F4 test scene loaded at startup gives it plenty to land on, and the
brush still works for drawing more terrain anywhere else.

## Where the rest of it is

This file is the front door. The reasoning lives in the documents that own it:

- **[ENGINEERING_NOTES.md](ENGINEERING_NOTES.md)** — how the engine works, and
  the technical decisions that were deferred or refused.
- **[PERFORMANCE.md](PERFORMANCE.md)** — benchmark numbers, how to run the
  bench, and the methodology a number is only meaningful under.
- **[MANUAL_TESTING.md](MANUAL_TESTING.md)** — the full Manual Tester Checklist,
  and what is currently owed to the tester.
- **[ROADMAP.md](ROADMAP.md)** — what is next and why it is ordered that way.
- **[TUNING.md](TUNING.md)**, **[ASSETS.md](ASSETS.md)**,
  **[VISION.md](VISION.md)**, **[PLAYTEST_LOG.md](PLAYTEST_LOG.md)**.
