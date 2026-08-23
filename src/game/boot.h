#pragma once
#include <vector>
#include "game/run.h"
#include "physics/grid.h"
#include "physics/material.h"
#include "physics/player.h"
#include "scene/props.h"

// W5, 2026-08-17: the decisions `main()` used to take before its first frame,
// moved out of the SDL shell so a test can reach them.
//
// **This exists for one reason and it is written down in ROADMAP.md's `W5`
// entry: `CLAUDE.md` requires the Manual Tester Checklist after a change to
// `main.cpp` *because the suites cannot reach it*, and the suites cannot reach
// it because it is one function.** So the shape of `main.cpp` is what converts
// machine-checkable work into human-checkable work, and the human is one
// person. Everything here was previously inline in `main()`, ran once at
// startup, printed a line to stdout, and could only be checked by a person
// reading that line.
//
// **A header rather than a sixth source-set variable, and that is deliberate.**
// `src/game/debug_view.h` set this precedent for exactly the same reason (T1):
// the guard in `CMakeLists.txt` is five variables kept apart so that a
// simulation source reaching for a renderer has to be *written into the build
// file* to compile, and every new variable is a chance to blur that. This
// header links `ENGINE_SOURCES` and nothing else - `PropDef` is a struct
// definition, not a link dependency, so nothing here pulls in the parser.
//
// **No SDL, by rule.** The one thing startup genuinely needs a window for is a
// prop's *size*, since that comes from its texture - so `plant_props` is handed
// widths as data rather than reaching for a texture. That is the whole of the
// seam: `main.cpp` queries SDL for the widths, this decides where each prop
// stands, and the decision has a test.
namespace boot {

// The simulated world's size, in cells - independent of the window since F3.1.
// Not equal to any window size divided by Camera::SCALE: that was only ever a
// coincidence of nothing having needed them to differ yet, and F4's scene is
// authored at 1920x1080 cells (the fixture's original 640x400 rescaled with the
// player body - see generate_test_scene.py), bigger than every viewport in the
// display table. That is deliberate, not a mismatch to fix - it exercises the
// panning half of the camera (F3.4) rather than just the decoupling half
// (F3.1-F3.3), which a world equal to the viewport never did.
//
// It also has to clear the *largest* viewport, not the one the game happens to
// launch at: 3440x1440 sees 861x361 padded cells, so a world sized to the
// smallest mode would leave the widest one with nothing to pan across.
inline constexpr int GRID_WIDTH = 1920;
inline constexpr int GRID_HEIGHT = 1080;

// S0's objective, and it is a column rather than a point because the row it
// sits at is scanned off the terrain below it (see `terrain_surface`). Placing
// a y here would be the mistake the prop format refuses by construction - a
// number an author tunes for an afternoon while the loader ignores it - and it
// is the same mistake that buried three trees in the snowbank.
//
// 1700 is chosen for what stands between it and the spawn, not for where it is.
// The player starts at GRID_WIDTH / 2, so this is ~740 cells east: past the
// jump ledges, across F4's water channel (cells 1100-1503, walled on both sides
// and full to within 175 cells of the top - the channel moved 100 cells east on
// 2026-08-22 for V22 part 2's spawn corridor, and 1700 still lands beyond it),
// and out onto the sleeper run. That
// is a traverse the character cannot walk, which makes flight the thing the run
// is actually about - and flight was a shipped feature nothing in the built
// game had ever asked for.
//
// **It is hard-coded, and that is S0's stated limit rather than an oversight.**
// A real objective is placed by a generator into a level format with a slot for
// it; both of those are the full "Objective + Extraction" item in ROADMAP.md
// and neither is started here.
inline constexpr int OBJECTIVE_X = 1700;

// The first solid row in a column, or -1 if the column is open all the way
// down. The same scan the prop planter does over a footprint, kept separate
// rather than shared with it: that one takes the *lowest* surface across a
// sprite's width so a tree leans into a hill, and this one is a single column,
// so a shared helper would have to be told which of the two it was being.
inline int terrain_surface(const Grid& grid, int x) {
    if (x < 0 || x >= grid.get_width()) return -1;
    for (int y = 0; y < grid.get_height(); ++y)
        if (is_solid(grid.get_element(x, y).type)) return y;
    return -1;
}

// The *lowest* first-solid row found anywhere across `[x0, x0 + width)`, or -1
// if no column in that span has ground under it.
//
// **Lowest, not the centre column's, and that is what makes a tree on a slope
// lean into the hill instead of floating off its uphill edge.** Columns outside
// the world are skipped rather than treated as open air: a prop half off the
// map is planted on the half that exists, which is the same answer the old
// inline loop gave.
inline int lowest_surface_under(const Grid& grid, int x0, int width) {
    int lowest = -1;
    for (int x = x0; x < x0 + width; ++x) {
        const int surface = terrain_surface(grid, x);
        if (surface > lowest) lowest = surface;
    }
    return lowest;
}

// The *highest* first-solid row across `[x0, x0 + width)`, or -1 if no column
// in that span has ground under it.
//
// **The mirror of `lowest_surface_under`, and the pair is worth reading
// together, because picking the wrong one of the two is a silent bug rather
// than a crash.** A prop wants the lowest so a tree leans into a hill; a
// *body* wants the highest, because a body falling onto uneven ground stops on
// the first thing its footprint meets, not on the deepest. Give a body the
// lowest and it stands with its feet inside the hill.
inline int highest_surface_under(const Grid& grid, int x0, int width) {
    int highest = -1;
    for (int x = x0; x < x0 + width; ++x) {
        const int surface = terrain_surface(grid, x);
        if (surface >= 0 && (highest < 0 || surface < highest)) highest = surface;
    }
    return highest;
}

// Whether the body found ground to stand on, and the row its feet ended on.
struct Standing {
    bool placed = false;
    int surface = 0;
};

// Stands the body on the terrain under its own spawn column, instead of leaving
// it where `Run`'s constructor put it.
//
// **Why this exists, and it is a playtest report rather than a tidy-up**
// (session 12, 2026-08-22): *"i am spawning in the air then falling"*. `Run`
// spawns the player at `height / 4` because it is built before any terrain is,
// and with the scene loaded that is 270 with the floor at 950 - a 660-cell free
// fall on every launch. It has always been that way and it was invisible while
// the body landed on a jump ledge partway down; V22 part 2 cleared the spawn
// corridor, and what that left is a long fall through an empty frame.
//
// **It is a `main.cpp` fix living here for the reason everything else in this
// file does**: it runs once at startup, it is a scan over the shipped scene,
// and a scan in `main()` is a scan no suite can reach. `boot_test` asserts it
// against the same scene the game loads.
//
// **The drop is not merely shortened, it is removed**, and that is a decision.
// A short fall would still spend `Player::has_landed` - the flag that makes the
// spawn drop free of fall damage - on the first frame of the game, which is a
// mechanic being consumed by the camera getting into position. Standing the
// body on the surface leaves it for the first jump, where it means something.
//
// **A column with no ground under it leaves the body where it was**, the same
// refusal `place_objective` makes: the only fallback available is a guess, and
// a body placed at a guessed row is worse than a body that falls, because
// falling at least ends up somewhere real. The caller warns.
inline Standing stand_player_on_ground(Run& run) {
    const int surface = highest_surface_under(run.grid, run.player.cell_x(), Player::WIDTH);
    if (surface < 0) return Standing{};
    run.player = Player(run.player.cell_x(), surface - Player::HEIGHT);
    return Standing{true, surface};
}

// Where S0's objective ended up, and whether it got placed at all.
struct Objective {
    bool placed = false;
    int x = 0;
    int y = 0;
};

// Plants the objective on whatever terrain is actually at `column`, the same
// way a prop is planted. Its row is the centre of a body standing on that
// surface, so "reach the objective" means "stand where the marker is" rather
// than something the player has to work out from a floating icon.
//
// **A column with no ground under it drops the objective rather than defaulting
// it**, which is the prop planter's rule and it is here for the same reason:
// the only fallback available is the top of the world, and an objective hanging
// in the sky is exactly as wrong as one buried. A run with no objective is
// still playable - you can still die - so the caller warns rather than refusing
// to start.
inline Objective place_objective(Run& run, int column = OBJECTIVE_X) {
    const int surface = terrain_surface(run.grid, column);
    if (surface < 0) return Objective{};
    const int y = surface - Player::HEIGHT / 2;
    run.set_objective(column, y);
    return Objective{true, column, y};
}

// One prop that found ground, as an index back into the caller's `PropDef`
// list plus the row its bottom edge sits on.
struct Planted {
    int def_index = 0;
    int anchor_y = 0;
};

// What the planting scan did with each record. **Three outcomes, held apart,
// because the count the launch check prints has to include both ways a prop can
// be dropped** - a sprite that would not load, and a prop with no ground under
// it. It once printed `10 of 10 placed` on a run that drew 9, because the count
// was taken before the second of those could happen.
struct PlantingReport {
    std::vector<Planted> planted;
    std::vector<int> no_texture;  // def indices whose sprite did not load
    std::vector<int> no_ground;   // def indices with no solid cell underneath
};

// Plants each prop on the terrain that is actually under it, rather than on a
// hardcoded ground line. **This is a fix for a class of bug, not for the three
// trees that had it:** props were authored at `FLOOR_TOP`, which is true of the
// floor slab and false of everything standing on it, so the three trees over
// the authored sand slope were 26%, 43% and 83% buried - invisibly, because
// they sit off-screen at spawn and the screenshot that "confirmed" the feature
// was of the other side of the world.
//
// `widths[i]` is `defs[i]`'s sprite width in world cells, or **0 for a sprite
// that did not load** - which is how the SDL half of this reaches a function
// that knows no SDL. A short `widths` is treated as all-zero from that point,
// so a caller that gets the two lists out of step drops props loudly rather
// than reading off the end.
//
// Runs once, after the scene is stamped and before the first frame: props are
// not simulated, so terrain that moves later does not drag them with it - which
// is correct for a tree and is the same "exercises no system" line that put
// them in this layer at all.
inline PlantingReport plant_props(const Grid& grid,
                                  const std::vector<PropDef>& defs,
                                  const std::vector<int>& widths) {
    PlantingReport report;
    report.planted.reserve(defs.size());
    for (int i = 0; i < static_cast<int>(defs.size()); ++i) {
        const int w = i < static_cast<int>(widths.size()) ? widths[i] : 0;
        if (w <= 0) {
            report.no_texture.push_back(i);
            continue;
        }
        const int x0 = static_cast<int>(defs[i].x - w / 2.0f);
        const int surface = lowest_surface_under(grid, x0, w);
        // No solid ground anywhere under it is a scene-authoring mistake, not
        // something to paper over with a default - a prop hanging in the air is
        // exactly as wrong as one buried, and silently placing it at the
        // fallback is how the first version of this hid its own bug.
        if (surface < 0) {
            report.no_ground.push_back(i);
            continue;
        }
        report.planted.push_back(Planted{i, surface});
    }
    return report;
}

}  // namespace boot
