// W5's startup decisions, headless.
//
// **This suite is the item's acceptance rather than a side effect of it.**
// `W5` was admitted on converting human-checkable work into machine-checkable
// work: `CLAUDE.md` asks for the Manual Tester Checklist after a change to
// `main.cpp` *because the suites cannot reach it*, and they cannot reach it
// because it is one function. Everything asserted below used to run inside that
// function, print a line to stdout, and be checkable only by a person reading
// the line at launch.
//
// Two halves, and the second is the one that pays for the item.
//
// **The unit half** covers the decisions in `game/boot.h` and
// `choose_display_mode` in `game/display.h` against worlds this file builds.
//
// **The fixture half runs the shipped scene**, which is the whole of Manual
// Tester Checklist step 1 that was not already covered: `scene_test` pins the
// cell count, and what was left was the two launch lines after it -
// `Objective: (x, y)` and `Props: N of N placed`. Both were eyeballed on
// stdout. Both are assertions now.
//
// It reads real BMPs for the prop widths rather than assuming a size, because
// the width is what decides which columns a prop's footprint scans, and a
// planting test against a made-up width would pass over art that had changed
// shape. One BMP pixel is one world cell, so `bmp::read` gives exactly the
// number `SDL_QueryTexture` gives the game.
#include <string>
#include <vector>
#include "game/boot.h"
#include "game/display.h"
#include "game/run.h"
#include "physics/grid.h"
#include "scene/bmp.h"
#include "scene/props.h"
#include "scene/scene.h"
#include "test_util.h"

namespace {

// A floor at `top` across `[x0, x1)`, in a world otherwise empty.
void build_floor(Grid& grid, int x0, int x1, int top) {
    for (int x = x0; x < x1; ++x)
        for (int y = top; y < top + 4; ++y)
            grid.set_element(x, y, ElementType::Wall);
}

void test_terrain_surface() {
    Grid grid(64, 64);
    build_floor(grid, 10, 20, 30);

    check("terrain_surface: the first solid row in a built column",
          boot::terrain_surface(grid, 12) == 30);
    check("terrain_surface: -1 in a column that is open all the way down",
          boot::terrain_surface(grid, 5) == -1);
    // Out of bounds is -1 and not a read off the end. The prop planter walks a
    // footprint that can hang over the edge of the world, so this is the case
    // it hits rather than a defensive one.
    check("terrain_surface: -1 left of the world", boot::terrain_surface(grid, -1) == -1);
    check("terrain_surface: -1 right of the world", boot::terrain_surface(grid, 64) == -1);
}

void test_lowest_surface_under() {
    Grid grid(64, 64);
    // A step: the left half of the span is higher ground than the right.
    build_floor(grid, 0, 10, 20);
    build_floor(grid, 10, 30, 34);

    // **The lowest, not the nearest and not the centre column's.** This is the
    // whole of why a tree on a slope leans into the hill rather than floating
    // off its uphill edge, and taking the highest instead is the version that
    // buried three trees.
    check("lowest_surface_under: takes the lowest surface across the footprint",
          boot::lowest_surface_under(grid, 5, 10) == 34,
          std::to_string(boot::lowest_surface_under(grid, 5, 10)));
    check("lowest_surface_under: -1 when no column in the span has ground",
          boot::lowest_surface_under(grid, 40, 10) == -1);
    // Half off the map plants on the half that exists. The columns outside the
    // world are skipped, not counted as open air - counting them would be the
    // same answer here and a different one if the rule were "highest".
    check("lowest_surface_under: a footprint hanging off the left edge still plants",
          boot::lowest_surface_under(grid, -5, 10) == 20);
}

void test_place_objective() {
    {
        Run run(64, 64);
        build_floor(run.grid, 30, 40, 25);
        const boot::Objective obj = boot::place_objective(run, 35);
        check("place_objective: places on the terrain actually at that column",
              obj.placed && obj.x == 35 && obj.y == 25 - Player::HEIGHT / 2,
              std::to_string(obj.y));
        check("place_objective: ...and the run agrees",
              run.has_objective() && run.objective_x() == obj.x &&
                  run.objective_y() == obj.y);
    }
    {
        // **Dropped rather than defaulted.** The only fallback available is the
        // top of the world, and an objective hanging in the sky is exactly as
        // wrong as one buried - so a column with no ground leaves the run
        // without an objective, and the caller says so.
        Run run(64, 64);
        const boot::Objective obj = boot::place_objective(run, 35);
        check("place_objective: a column with no ground places nothing", !obj.placed);
        check("place_objective: ...and does not leave the run holding one",
              !run.has_objective());
    }
}

void test_plant_props() {
    Grid grid(64, 64);
    build_floor(grid, 0, 10, 20);
    build_floor(grid, 10, 30, 34);

    std::vector<PropDef> defs = {
        {"on_the_step", 10.0f, 1},  // footprint spans both floor heights
        {"over_air", 50.0f, 2},     // nothing under it
        {"no_sprite", 5.0f, 3},     // texture did not load
    };
    const std::vector<int> widths = {8, 8, 0};

    const boot::PlantingReport r = boot::plant_props(grid, defs, widths);

    check("plant_props: a prop over ground is planted", r.planted.size() == 1 &&
          r.planted[0].def_index == 0);
    check("plant_props: ...at the lowest surface under its own footprint",
          r.planted.size() == 1 && r.planted[0].anchor_y == 34,
          r.planted.empty() ? "nothing planted" : std::to_string(r.planted[0].anchor_y));
    check("plant_props: a prop over open air is dropped, not defaulted",
          r.no_ground.size() == 1 && r.no_ground[0] == 1);
    check("plant_props: a prop whose sprite did not load is dropped separately",
          r.no_texture.size() == 1 && r.no_texture[0] == 2);
    // **Both drops have to be reachable from the report**, because the launch
    // line counts placed against defined and it once read `10 of 10` on a run
    // that drew 9. A report that merged the two would still produce the right
    // total, and would stop the caller being able to say which happened.
    check("plant_props: every record is accounted for exactly once",
          r.planted.size() + r.no_ground.size() + r.no_texture.size() == defs.size());

    // A caller that gets the two lists out of step drops props loudly rather
    // than reading off the end of the shorter one.
    const boot::PlantingReport shortened = boot::plant_props(grid, defs, {8});
    check("plant_props: a short widths list drops the records it does not cover",
          shortened.planted.size() == 1 && shortened.no_texture.size() == 2);
}

void test_choose_display_mode() {
    const bool all_fit[3] = {true, true, true};
    const bool small_only[3] = {true, false, false};
    const bool none_fit[3] = {false, false, false};

    {
        const ModeChoice c = choose_display_mode(all_fit, 3, -1);
        check("choose_display_mode: nothing stored opens at the largest that fits",
              c.index == 2 && c.why == ModeChoice::Why::Largest);
    }
    {
        const ModeChoice c = choose_display_mode(all_fit, 3, 0);
        check("choose_display_mode: a stored mode that still fits wins",
              c.index == 0 && c.why == ModeChoice::Why::Stored);
    }
    {
        // The monitor changed between runs. Opening at the stored 3440x1440 on
        // a laptop screen puts the settings menu - the one way back - off the
        // edge of the display.
        const ModeChoice c = choose_display_mode(small_only, 3, 2);
        check("choose_display_mode: a stored mode that no longer fits is ignored",
              c.index == 0 && c.why == ModeChoice::Why::StoredTooBig);
    }
    {
        // An oversized window is a bad session; no window at all is no session.
        const ModeChoice c = choose_display_mode(none_fit, 3, 2);
        check("choose_display_mode: nothing fitting still opens, at the smallest",
              c.index == 0 && c.why == ModeChoice::Why::NothingFits);
    }
}

void test_stand_player_on_ground() {
    // The mirror pair. Giving a body the *lowest* surface under its footprint
    // puts its feet inside the hill, which is a silent bug rather than a crash,
    // so the two are asserted against the same terrain in one place.
    Grid g(40, 40);
    for (int x = 0; x < 20; ++x) g.set_element(x, 30, ElementType::Wall);   // low step
    for (int x = 20; x < 40; ++x) g.set_element(x, 24, ElementType::Wall);  // high step
    check("highest_surface_under: a footprint straddling a step takes the high side",
          boot::highest_surface_under(g, 16, 8) == 24);
    check("lowest_surface_under: ...and the prop planter still takes the low one",
          boot::lowest_surface_under(g, 16, 8) == 30);

    {
        // Session 12's report, as an assertion: the body ends up standing on
        // the terrain rather than falling to it.
        Run run(40, 40);
        // Row 34 rather than 30 on purpose: a 40-cell world spawns the body at
        // height / 4 = 10, so a surface at 30 would put a 20-cell body exactly
        // where it already was and the assertion below would pass on a
        // stand_player_on_ground that does nothing at all.
        for (int x = 0; x < 40; ++x) run.grid.set_element(x, 34, ElementType::Wall);
        const int before = run.player.cell_y();
        const boot::Standing s = boot::stand_player_on_ground(run);
        check("stand_player_on_ground: it finds the ground", s.placed && s.surface == 34,
              std::to_string(s.surface));
        check("stand_player_on_ground: the feet land exactly on the surface",
              run.player.cell_y() + Player::HEIGHT == 34,
              std::to_string(run.player.cell_y()));
        check("stand_player_on_ground: ...which is somewhere it was not", before != run.player.cell_y());
    }
    {
        // A world with nothing under the spawn column leaves the body where it
        // was, rather than guessing a row. Same refusal `place_objective` makes.
        Run run(40, 40);
        const int before = run.player.cell_y();
        const boot::Standing s = boot::stand_player_on_ground(run);
        check("stand_player_on_ground: no ground means no placement", !s.placed);
        check("stand_player_on_ground: ...and the body is not moved to a guess",
              run.player.cell_y() == before);
    }
}

// --- the fixture half: the launch check, as assertions ---

void test_shipped_fixture() {
    std::string error, warning;
    Scene scene = bmp::load("assets/test_material.bmp", "assets/test_albedo.bmp",
                            &error, &warning);
    if (!error.empty()) {
        check("fixture: the shipped scene loads", false, error);
        return;
    }

    Run run(boot::GRID_WIDTH, boot::GRID_HEIGHT);
    const int cells = load_scene(run.grid, scene, 0, 0);
    check("fixture: the shipped scene stamps cells into the world", cells > 0,
          std::to_string(cells));

    // **The spawn, on the scene the game actually loads.** The unit case above
    // proves the scan; this proves the shipped fixture still has floor under
    // the spawn corridor V22 part 2 cleared for it. A regression here is the
    // 660-cell free fall coming back, which is what session 12 reported.
    const boot::Standing stand = boot::stand_player_on_ground(run);
    check("fixture: the body stands on the shipped scene rather than falling to it",
          stand.placed, "no ground under the spawn column");
    check("fixture: ...with its feet on the surface",
          stand.placed && run.player.cell_y() + Player::HEIGHT == stand.surface,
          std::to_string(run.player.cell_y()) + " + " + std::to_string(Player::HEIGHT) +
              " vs surface " + std::to_string(stand.surface));

    // **`Objective: (x, y)` stops being a line to read.** A run with no
    // objective cannot be won, and the only thing that used to say so was a
    // stderr warning nobody sees unless they are looking.
    const boot::Objective obj = boot::place_objective(run);
    check("fixture: the objective plants on the shipped scene",
          obj.placed && obj.x == boot::OBJECTIVE_X,
          "no ground under x=" + std::to_string(boot::OBJECTIVE_X));
    check("fixture: ...on ground rather than at the top of the world",
          obj.placed && obj.y > 0, std::to_string(obj.y));

    // **`Props: N of N placed` stops being a line to read.** The regression
    // this catches is the one that has actually happened: props authored
    // against a ground line that is true of the floor slab and false of
    // everything standing on it.
    std::string prop_error;
    const std::vector<PropDef> defs =
        load_prop_list("assets/test_props.txt", &prop_error);
    check("fixture: the shipped prop list parses", prop_error.empty(), prop_error);
    check("fixture: ...and is not empty", !defs.empty());

    std::vector<int> widths(defs.size(), 0);
    bool every_sprite_read = true;
    for (size_t i = 0; i < defs.size(); ++i) {
        bmp::Image img;
        std::string img_error;
        if (bmp::read(("assets/" + defs[i].sprite + ".bmp").c_str(), img, &img_error)) {
            widths[i] = img.width;
        } else {
            every_sprite_read = false;
        }
    }
    check("fixture: every prop sprite the list names exists and reads",
          every_sprite_read);

    const boot::PlantingReport r = boot::plant_props(run.grid, defs, widths);
    check("fixture: every prop in the shipped list finds ground",
          r.planted.size() == defs.size(),
          std::to_string(r.planted.size()) + " of " + std::to_string(defs.size()) +
              " placed; " + std::to_string(r.no_ground.size()) + " over air, " +
              std::to_string(r.no_texture.size()) + " without a sprite");
}

}  // namespace

int main() {
    test_terrain_surface();
    test_lowest_surface_under();
    test_place_objective();
    test_plant_props();
    test_stand_player_on_ground();
    test_choose_display_mode();
    test_shipped_fixture();
    return report();
}
