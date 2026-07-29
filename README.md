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

The simulation has no SDL dependency, so it is tested headlessly. There are
four suites, one per concern — `grid_test` for the cellular automata,
`player_test` for the character physics, `tool_test` for digging, and
`collapse_test` for structural support — and CTest runs all of them:

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

**Player**
- **`A` / `D`** or **arrow keys:** Walk left and right.
- **`Space`** (or **`W`** / **up arrow**): Jump.

- **Left-Click:** Dig. Cuts a hole in whatever solid terrain the cursor is
  aimed at, up to a limited range. The orange dot marks where the shot will
  actually land.

**World (development tools)**
- **Right-Click & Drag:** Spawn elements onto the screen.
- **Mouse Wheel:** Grow / shrink the brush.
- **`1`**: **Sand** — powder, piles into a slope.
- **`2`**: **Water** — liquid, spreads to find its level.
- **`3`**: **Wall** — solid, immovable terrain.
- **`4`**: **Eraser** — deletes pixels.
- **`5`**: **Wood** — solid, catches fire from a touching flame.
- **`6`**: **Oil** — liquid, lighter than water so it floats on top; ignites fast.
- **`7`**: **Steam** — gas, rises and pools against the ceiling.
- **`8`**: **Fire** — gas, ignites Wood and Oil on contact, extinguished by Water, burns out on its own.
- **`9`**: **Rubble** — what Wall and Wood break into when they collapse. A powder, heavier than sand.
- **`ESC`**: Quit the game.

The window title shows the current framerate, selected material, and brush size.
The player spawns in mid-air in the middle of the world; draw some terrain under
it with the brush and it will land on it.

## Engine Architecture

The simulation lives in `src/physics/` and knows nothing about SDL — `main.cpp` is the only file that opens a window or reads input.

Materials are **data, not code**. Each one is a row in the `MATERIALS` table in
[material.h](src/physics/material.h) describing its colour, density, and which of four
generic behaviours it follows:

| Behaviour | Movement |
|-----------|----------|
| `Static`  | never moves (Wall, Wood) |
| `Powder`  | falls, then slides diagonally into a pile (Sand, Rubble) |
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

### The player

The player is the one thing in the engine that is **not** a cell. It is a
4x8 axis-aligned box in [player.h](src/physics/player.h) with its own position
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

### Structural collapse

`Static` materials hold their shape, which means they will also hold it
somewhere they have no business holding it — dig the ground out from under a
stone slab and it stays in mid-air, while the sand beside it collapses
correctly. The inconsistency is visible side by side, which is what makes it
read as a bug rather than as a rule.

So Wall and Wood can now lose support and break into **Rubble**, a powder
denser than sand. Which materials collapse and what they break into is a
`debris` column in the same `MATERIALS` table — `ElementType::Count` means
"never collapses", which is the right answer for everything that already falls
on its own. Same discipline as solidity: one table, not two.

**Support is a flood fill.** From a disturbed structure cell, walk the connected
structure looking for one cell that is *grounded* — meaning the bottom of the
world, or something solid directly beneath it that is not part of the same
structure. One grounded cell holds the whole thing up. If the fill explores the
entire structure without finding one, all of it becomes debris at once, so a
slab comes down as a slab rather than dissolving grain by grain from the edges.
Powders bear load and liquids do not, so a slab resting on packed sand stands
and the same slab resting on water does not.

Two decisions are worth knowing about, because both are visible in play:

- **Support is checked on disturbance only, never as a global truth.** Sweeping
  the world every step would cost more than the simulation it is attached to,
  and a world as authored is assumed to be standing up on purpose. A structure
  nobody has touched is never questioned — so a floating platform drawn with the
  brush stays exactly where it was put, and only starts falling once something
  removes part of it or slides out from under it. Placing structure never
  triggers a check; only removing it does.
- **Structures over 4,096 cells are assumed supported rather than judged.** The
  asymmetry is deliberate. A missed collapse is invisible — a slab that should
  have fallen simply doesn't. A wrong collapse turns a level into rubble. When
  the answer is too expensive to compute, guess the harmless way.

This is the first feature in the engine with a **measurable performance cost**.
`swap_elements` now asks whether the cell above each end of a move is structure,
and `swap_elements` is the hottest path there is — it costs about 6% of the
worst-case benchmark scenario. That is documented in ROADMAP.md rather than
hidden, and it is being paid on purpose.
