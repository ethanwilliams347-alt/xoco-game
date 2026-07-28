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
is aiming at) across four scenarios and reports milliseconds per step against the
16.67 ms budget of a 60 Hz frame. Run it before and after any change that claims
to make the simulation faster.

## Controls
- **Left-Click & Drag:** Spawn elements onto the screen.
- **Mouse Wheel:** Grow / shrink the brush.
- **`1`**: **Sand** — powder, piles into a slope.
- **`2`**: **Water** — liquid, spreads to find its level.
- **`3`**: **Wall** — solid, immovable terrain.
- **`4`**: **Eraser** — deletes pixels.
- **`5`**: **Wood** — solid, will be flammable once reactions land.
- **`6`**: **Oil** — liquid, lighter than water so it floats on top.
- **`7`**: **Steam** — gas, rises and pools against the ceiling.
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
