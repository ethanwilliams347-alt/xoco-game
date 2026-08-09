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
ten suites, one per concern — `grid_test` for the cellular automata,
`player_test` for the character physics, `tool_test` for digging,
`collapse_test` for structural support, `run_test` for the three of them driven
together through one `Run::step()`, `scene_test` for the level loader,
`light_test` for the emissive light field, `anim_test` for the player's
animation selector, `props_test` for the prop list format, and `sprites_test`
for the sprite manifest that decides which BMP each key loads — and CTest runs
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

It simulates a 960x540 grid across seven scenarios and reports milliseconds per
step against the 16.67 ms budget of a 60 Hz frame, then measures the light field
separately at the **widest** display mode's viewport (861x361 cells, 3440x1440) —
the mode that costs the most, so the number is a ceiling rather than a sample.
Run it before and after any change that claims to make the simulation faster.

The bench world stays at 960x540 while the game's world is 1920x1080 cells. That
is a known gap, not a claim that world size is free: stepping is chunk-driven so
most of it does not scale with total cells, but nobody has measured the part that
does, and rewriting the table wholesale is what `PERFORMANCE.md` asks for rather
than editing one row.

Read `PERFORMANCE.md` before trusting a number out of it. Timings from different
sittings on the same machine have been seen to differ by more than 2x on
identical code, so a comparison is only worth anything if both sides were
measured back to back — and that document explains how a claim in these docs got
that wrong once already.

## Manual Testing

`ctest` proves each mechanic is correct in isolation, headlessly, one suite per concern. It cannot prove they still *compose* — that digging near falling sand near fire near water still feels and looks right together — because nothing about running in a window, taking real input, and rendering a frame is exercised by a suite that never opens one. The checklist below is the other half. It is not the playtest gate in `ROADMAP.md`'s Medium Term section, which is about whether the game is *fun* for someone who did not build it; this is about whether it is still *correct*, run by whoever just built the feature, in the two or three minutes before calling it done.

**Run this after any change that touches `src/physics/`, `src/game/` or `main.cpp` and is not fully exercised by the automated suites** — which in practice means anything touching rendering, input, or feel, since those are exactly what a headless test cannot see. Skip it for test-only or documentation-only commits; there is nothing here those could break. Each item names the regression it exists to catch, most of them things that have actually gone wrong once already in this project — a floating pile, a seam at a chunk border, a fire that never dies — so a "looks fine" pass is a real signal, not a formality.

1. **Launch.** `cmake --build build --config Release`, then run the exe. Window opens, a `World seed: N` line prints to stdout (the seed check has no way to fail silently: if the number is missing, `main.cpp` stopped being the project's one nondeterministic line). The HUD in the window's top-left corner shows fps, current material, brush size, and chunks awake — the window title bar is now a plain, static label, not where this lives. The world is no longer empty at launch: `main.cpp` loads the authored F4 test scene (`assets/test_material.bmp` / `test_albedo.bmp`) over it first, so confirm terrain is visible immediately — a snowbank, fence posts, a bridge over a pit, a water channel — rather than a blank grid. **A `Scene: 1920x1080, 334901 cells placed` line prints alongside the seed, and that is the check rather than the eyeballing.** This step used to be eyeballed and it silently stopped being true for a whole commit: retuning the palette changed the colours the material map was matched against, every authored pixel resolved to `Empty`, and the game booted blank while all six suites passed. A count of zero, or a `WARNING` about unrecognised legend colours, means the scene file and [src/scene/legend.h](src/scene/legend.h) have come apart.

2. **Movement (`Player`).** Walk both directions, jump, land. Confirm the body rests flush on top of Wall and on top of settled Sand — no half-cell sinking, no hovering. **The sprite is 14x26 over an 8x20 collision box, so "flush" is a claim about the sprite's *feet*, not its bounding rect** — the mask overhangs above the box and the sleeves outside it, both by design. A figure that hovers one cell above every floor means some frame's bottom row went empty — `python tools/player_sheet.py --validate` checks exactly that, per frame, and is the first thing to run; a figure sunk into the floor means `src/render/player_sprite.h` is stale and needs `--header` re-run. Also confirm the figure turns to face the direction you walk, and keeps facing that way after you stop.

   **Animation, which is the half no suite can see.** `anim_test` pins which animation each state selects and that the clock advances correctly; what it cannot see is whether the result looks right. Walk both ways and confirm the cycle plays and the feet do not slide. Stop and confirm it settles to idle rather than freezing mid-stride. Jump and confirm the pose changes on the way up and again on the way down, and does not flicker at the apex. Dig and confirm the swing plays once and completes — then spam clicks and confirm it never sticks mid-swing. **Then press into a wall and hold the key**: the figure must stand, not walk on the spot, which is the whole reason the selector reads `velocity_x()` rather than the input. **Hold the dig button on solid terrain rather than clicking once**: the swing must replay on every dig, not only the first — the tool fires on a cooldown and the animation is driven by what `Run::step` returns, because inferring it from the cooldown either side missed every dig but the first. **Finally, if you can run the window at a different frame rate, confirm the walk cycle's speed does not change** — the clock is the fixed step for exactly this reason, and a cycle that speeds up with the display is the defect [src/render/player_anim.h](src/render/player_anim.h) exists to prevent. Walk it up a one-cell sand step without jumping ([The player](#the-player) — `MAX_STEP_HEIGHT`). Confirm it cannot walk through Wall, Wood, or a settled sand pile.

3. **Digging (`DigTool`).** Left-click cuts a circular hole at the crosshair. Aim at something past `RANGE` and confirm **the crosshair dims from solid white to dim white** — that is the range check now, and it reads at the cursor rather than needing a second marker elsewhere on screen. **Dim does not mean "nothing happens", and the checklist should not expect it to:** the ray still travels its full `RANGE` along that line and cuts the first solid cell it meets, so a dim crosshair aimed past a *near* wall still digs that wall. What dim means is that the cell under the crosshair is not the cell that gets hit. Confirm both halves — a dim shot into open sky beyond range does nothing, and a dim shot lined up through nearby terrain still bites it. Dig the bottom out of a standing Sand pile and confirm everything above it falls in — a gap left hanging is the classic dirty-rect bug ([Chunked updates](#chunked-updates)). Fire at a wall from behind cover and confirm the shot stops at the near face rather than tunnelling through to whatever is behind it.

4. **Materials and brush.** Cycle all eight keys (`1`–`8`) and confirm each paints the right colour and the HUD's material name agrees. Sand piles into a slope. Water spreads flat. Pour Water into one side of a container whose two halves are joined only at the bottom and confirm both sides come level and then *stop* — a surface that keeps trading cells back and forth is level on average and never sleeps ([Liquids find their level](#liquids-find-their-level)). **Then pour an untidy amount of Water onto flat ground and watch the HUD's chunk count go to zero.** Any puddle whose cell count did not happen to divide by its container's width used to shimmer forever, which is almost all of them; the leftover cells now come to rest as one patch, level to within a cell. The failure mode to watch for in the other direction is a puddle that settles into a visible *mound* instead of spreading — the sideways walk that jitters is also the one that flattens, and refusing too much of it trades a shimmer for a heap. Oil floats on Water rather than mixing into it. Steam rises and pools at the ceiling instead of the floor. Eraser (`8`) clears back to `Empty`. **Confirm the hotbar's highlight lands on the box you pressed**, and that each icon depicts the material its key actually places — the bindings and the icons come out of one table specifically so they cannot disagree, and this step is what checks that claim.

5. **Reactions and heat.** Ignite a Wood block with Fire (`7`) and watch it catch, spread, and eventually burn down to `Empty`. Same against Oil — should catch sooner. **Watch a long beam rather than a block**: what you want to see is a front that advances along it, not the whole beam lighting up at once and not a fire that stops dead after one cell ([Heat](#heat)). Drop Fire next to Water and confirm the water boils off into Steam as well as dousing the flame; leave the Steam alone and confirm it eventually condenses back to Water rather than drifting at the ceiling forever (about a second now, not two — Steam's temperature had to come down below the coldest ignition point in `REACTIONS`, and its life is exactly that span). **Then do it under a wooden roof**: build a Wood ceiling, put Fire and Water beneath it, and confirm the Steam that results does *not* set the ceiling alight. Steam used to spawn a hundred degrees over Wood's ignition point, so putting a fire out was a way of starting a bigger one — and it only showed when the steam was confined, which is why open-air testing never found it and authored terrain would have. Place one Fire cell with nothing nearby and confirm it burns itself out on its own. **Then box a Fire cell in on all sides with Wall and confirm it still burns out** — this is the self-wake regression specifically: a fire with nowhere to move and nothing to check its own decay against will otherwise freeze forever with its chunk asleep ([Reactions](#reactions)).

6. **Chunking / sleep-wake.** Let a mixed scene (sand, water, a fire) run to rest and confirm the awake-chunk count in the HUD returns to at or near zero — a nonzero idle count means something is being woken that should not be. **Burn something first, then wait**: heat keeps cells awake while it is still moving, by design, so the count should stay up for a while after the flames are gone and then come down as the scene cools. A count that never comes down means heat is not settling, which costs full price forever and is invisible to look at. Build a flat sand floor wide enough to cross a chunk boundary (chunks are 64 cells) and confirm it settles into one continuous surface with no step or seam at the boundary.

7. **Structures ([Structures and falling](#structures-and-falling)).** Place a Wall or Wood shape with nothing under it and confirm it falls as one rigid piece, keeping its shape, rather than crumbling into loose grains or hanging in the air. Rest a shape on solid ground and confirm it stays put indefinitely — no spontaneous twitching. Dig one support cell out from under a large structure and confirm the whole thing drops promptly and lands clean, nothing left floating. **Then build a ledge with a drop beside it and cut a wide slab loose above the join**: it should come apart over the edge rather than perching level across it, and the two halves should stay apart afterwards. The negative half of that is the one to be fussy about — nothing that was standing still should ever break, so watch a settled structure through several of these and confirm it never so much as shifts.

8. **Performance sanity.** Paint a large, actively-falling scene (a wide sand-over-water fill is close to `grid_bench`'s `churning`) and watch the HUD fps stay near the display's refresh rather than cratering. If something feels newly slow, that is a lead, not a verdict — follow it up with `grid_bench`, bracketed, per `PERFORMANCE.md`; a felt slowdown on its own is exactly the kind of unbracketed reading that document warns against trusting.

9. **Stability.** A few minutes of doing several of the above at once — digging near falling sand near fire near water, brush strokes back to back, movement keys held through a collapse — without a crash. There is one unexplained `0xC0000409` on record, seen twice under heavy machine load and never reproduced; if it recurs, note what else was running on the machine at the time and fold it into the crash-diagnosis item in `ROADMAP.md`'s Presentation & Tooling section rather than letting it evaporate again.

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

**Settings menu** (`ESC`)
- **Arrow keys / `W` / `S`:** Move the cursor. **`Enter`:** Choose. **`ESC`:** Resume.
- Offers 1920x1080, 2560x1440 and 3440x1440. The pixel scale is fixed at 4 screen pixels per cell in every mode, so a wider window shows *more world* rather than a bigger one — 480x270 cells at 1080p up to 860x360 at 3440x1440, which is the framing of the reference footage this game's art is measured against. A mode larger than your desktop is listed but greyed out rather than hidden.
- The chosen mode is written to `settings.txt` beside the executable and reloaded next launch. If that file names a resolution the current display cannot fit — a monitor changed between runs — it is ignored with a note on stderr and the largest mode that does fit is used. The mode actually chosen prints at startup next to the seed.

The HUD in the top-left corner of the window shows the current framerate,
selected material, brush size, and awake-chunk count (see [Chunked
updates](#chunked-updates)); the window title bar itself is just a static
label. The player spawns in mid-air in the middle of the world; the F4 test
scene loaded at startup gives it plenty to land on, and the brush still works
for drawing more terrain anywhere else.

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

**This covers the simulation, not yet the game.** The brush is painted
once per rendered frame outside the fixed-step loop, and a held key is
sampled once per rendered frame and applied to every fixed step inside
it — so the same seed and the same physical input can still produce a
different world at a different framerate. Closing that gap means moving
input onto a per-step log rather than sampling it in the render loop, which
is separate, not-yet-built work.

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

- **Step-up.** A blocked horizontal move retries with the body lifted up to 2
  cells. That is the whole of "walking over uneven powder" — a settled sand
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
dig does not reach 1.4x as far as a straight one. The cooldown is counted in
fixed steps rather than seconds, so the tool fires at the same rate on every
machine. A shot that connects with nothing costs no cooldown, which makes the
limit read as tool speed instead of as a random input lockout.

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
