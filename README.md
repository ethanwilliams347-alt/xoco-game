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

## Running the Tests

The simulation has no SDL dependency, so it is tested headlessly:

```bash
ctest --test-dir build -C Release --output-on-failure
```

## Running the Benchmark

Timings are not part of the test suite — a slow machine should not fail the
build. The benchmark is a separate executable, run it by hand:

```bash
.\build\Release\grid_bench.exe
```

It simulates a 960x540 grid (1920x1080 at a 2px scale, the resolution the project
is aiming at) across five scenarios and reports milliseconds per step against the
16.67 ms budget of a 60 Hz frame. Run it before and after any change that claims
to make the simulation faster.

## Controls
- **Left-Click & Drag:** Spawn elements onto the screen.
- **Mouse Wheel:** Grow / shrink the brush.
- **`1`**: **Sand** — powder, piles into a slope.
- **`2`**: **Water** — liquid, spreads to find its level.
- **`3`**: **Wall** — solid, immovable terrain.
- **`4`**: **Eraser** — deletes pixels.
- **`5`**: **Wood** — solid, catches fire from a touching flame.
- **`6`**: **Oil** — liquid, lighter than water so it floats on top; ignites fast.
- **`7`**: **Steam** — gas, rises and pools against the ceiling.
- **`8`**: **Fire** — gas, ignites Wood and Oil on contact, extinguished by Water, burns out on its own.
- **`ESC`**: Quit the game.

The window title shows the current framerate, selected material, and brush size.

## Engine Architecture

The simulation lives in `src/physics/` and knows nothing about SDL — `main.cpp` is the only file that opens a window or reads input.

Materials are **data, not code**. Each one is a row in the `MATERIALS` table in
[material.h](src/physics/material.h) describing its colour, density, and which of four
generic behaviours it follows:

| Behaviour | Movement |
|-----------|----------|
| `Static`  | never moves (Wall, Wood) |
| `Powder`  | falls, then slides diagonally into a pile (Sand) |
| `Liquid`  | falls, then spreads sideways to find its level (Water, Oil) |
| `Gas`     | rises, then spreads (Steam) |

Density decides what sinks through what, so sand sinking in water and oil floating on
water both fall out of the same rule rather than being special-cased. Adding a material
means adding a table row, not editing the update loop.

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

The window title shows how many chunks are awake. In an idle world it should sit
at or near zero.

### Reactions

Movement is data-driven; transformation is the second axis. Each row in the
`REACTIONS` table in [reaction.h](src/physics/reaction.h) is a rule of the
shape `catalyst + target -> result`, rolled once per eligible cell per step:

| Catalyst | Target | Chance | Result |
|----------|--------|--------|--------|
| Fire     | Wood   | 12%    | Fire   |
| Fire     | Oil    | 40%    | Fire   |
| Water    | Fire   | 90%    | Steam  |
| *(none)* | Fire   | 6%     | Empty  |

A catalyst of `Count` means "no neighbour required" — Fire's own burnout is
spontaneous, not triggered by contact. Rows are checked in order; the first
row whose target and catalyst condition both match is the only one considered
that cell that step, which is what makes dousing (row 3) take priority over
natural burnout (row 4) without any special-casing — as long as Water is
adjacent, row 4 is never reached.

**A second wake rule, alongside chunking's.** A cell that stops moving stops
generating `mark_dirty` calls and its chunk goes back to sleep — that's the
whole point of chunking. But Fire's burnout doesn't need movement to happen;
if Fire is boxed in with nowhere to go, it would take its one shot at the 6%
roll on the frame it was created and then freeze forever, un-woken, never
given another chance to decay or to ignite what it's touching. So any cell
that is a *spontaneous* reaction target (right now, only Fire) marks its own
3x3 neighbourhood dirty every single step it exists, movement or not. This is
deliberately narrow: Wood and Oil are only ever reaction *targets* of a
catalyst-based row, never spontaneous, so idle Wood and Oil far from any fire
stay fully sleep-eligible and chunking's performance for the common case is
untouched.
