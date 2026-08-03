// Run tests.
//
// The first suite in the project that drives player, tool and grid together
// through one entry point rather than one at a time - because Run::step() is
// itself the thing under test here, not any one subsystem's physics. Every
// scenario goes through run.step(input), the same call main.cpp makes once
// per fixed step, so what passes here is what a real session would actually
// see.

#include "game/run.h"
#include "test_util.h"
#include <algorithm>
#include <string>
#include <vector>

namespace {

const Input NOTHING;

Input held_right() {
    Input in;
    in.right = true;
    return in;
}

// A flat Wall floor spanning the grid, top surface at `floor_y`.
void build_floor(Run& run, int floor_y) {
    for (int x = 0; x < run.grid.get_width(); ++x)
        run.grid.set_element(x, floor_y, ElementType::Wall);
}

void step(Run& run, const Input& input, int n) {
    for (int i = 0; i < n; ++i) run.step(input);
}

// Same shape as test_grid.cpp's worlds_match, duplicated rather than shared:
// each test file owns its own small helpers, and this is three lines.
bool worlds_match(const Grid& a, const Grid& b) {
    if (a.get_pixels() != b.get_pixels()) return false;
    for (int y = 0; y < a.get_height(); ++y) {
        for (int x = 0; x < a.get_width(); ++x) {
            const Element ea = a.get_element(x, y);
            const Element eb = b.get_element(x, y);
            if (ea.type != eb.type || ea.color != eb.color ||
                ea.updated_tag != eb.updated_tag || ea.ticks != eb.ticks)
                return false;
        }
    }
    return true;
}

} // namespace

int main() {
    // --- a scripted sequence: land, walk, jump, dig ---
    // Nothing here freezes the grid or calls a subsystem directly - every
    // effect happens by handing run.step() an Input, the same as a real
    // session would.
    {
        // Wide enough that 60 steps of holding right (about 45 cells at
        // MOVE_SPEED) never reaches the far wall - the dig target below is
        // placed relative to wherever the player ends up, and needs real
        // headroom on both sides or it silently lands outside the grid,
        // where get_element() reads the border itself as Wall regardless of
        // what was actually dug.
        Run run(200, 50, 7);
        build_floor(run, 40);

        // The player spawns mid-air (Run's constructor); let it land before
        // asserting anything about walking.
        step(run, NOTHING, 60);
        check("the player lands on the floor", run.player.is_on_ground(),
              "cell_y=" + std::to_string(run.player.cell_y()));

        const int start_x = run.player.cell_x();
        step(run, held_right(), 60);
        check("holding right through run.step() walks the player right",
              run.player.cell_x() > start_x,
              "start=" + std::to_string(start_x) + " now=" + std::to_string(run.player.cell_x()));

        const int rest_y = run.player.cell_y();
        Input jump;
        jump.jump = true;
        run.step(jump);
        // Not asserting is_on_ground() here as well: leaving a floor takes
        // one whole cell of upward movement, and whether a single step's
        // rem_y carries that far is a detail of Player's own integration,
        // covered by its own tests. The velocity flip is the part that
        // belongs to this step's input handling, so that is what this checks.
        check("one jump step applies an upward impulse", run.player.velocity_y() < 0,
              "vel_y=" + std::to_string(run.player.velocity_y()));

        int highest = run.player.cell_y();
        for (int i = 0; i < 120; ++i) {
            run.step(NOTHING);
            if (run.player.cell_y() < highest) highest = run.player.cell_y();
        }
        check("the jump clears the ground and the player lands again",
              rest_y - highest >= 8 && run.player.is_on_ground(),
              "peak=" + std::to_string(rest_y - highest) + " cells");

        // A Wall block near wherever the player ended up, close enough to sit
        // well inside DigTool::RANGE without depending on an exact landing
        // spot - walking speed and jump arc are someone else's tests.
        const int wx = run.player.cell_x() + 6;
        for (int y = 30; y <= 35; ++y)
            for (int x = wx; x <= wx + 4; ++x)
                run.grid.set_element(x, y, ElementType::Wall);
        check("the dig target is actually solid",
              run.grid.get_element(wx + 2, 32).type == ElementType::Wall);

        Input dig;
        dig.dig = true;
        dig.cursor_x = wx + 2;
        dig.cursor_y = 32;
        run.step(dig);
        check("holding dig through run.step() removes solid terrain",
              run.grid.get_element(wx + 2, 32).type != ElementType::Wall);
    }

    // --- the brush is no longer a special case ---
    // Painted through run.step() rather than applied outside the loop, which
    // is the entire point of F2.3: nothing about placing a cell happens
    // anywhere but inside a fixed step now.
    {
        Run run(40, 40, 11);
        Input paint;
        paint.brush_active = true;
        paint.brush_type = ElementType::Sand;
        paint.brush_size = 2;
        paint.cursor_x = 20;
        paint.cursor_y = 20;
        run.step(paint);

        check("the brush reaches the grid through run.step()",
              run.grid.get_element(20, 20).type == ElementType::Sand);
        check("the brush paints a circle, not a single cell",
              run.grid.get_element(21, 20).type == ElementType::Sand &&
              run.grid.get_element(20, 21).type == ElementType::Sand);
    }

    // --- the same seed plus the same recorded Input sequence replays byte-
    //     identical, regardless of how the caller paces the calls ---
    // This is the assertion that actually closes F1. A generator or a
    // per-rendered-frame sample could not make this promise: the same
    // physical session played at 30 fps and at 144 fps would batch a held
    // key into a different number of steps per sample, and diverge. Here
    // there is no sampling left to diverge - the sequence below *is* the
    // input, independent of any frame it was ever rendered on.
    {
        std::vector<Input> sequence;
        for (int i = 0; i < 40; ++i) sequence.push_back(NOTHING); // let the player land
        for (int i = 0; i < 50; ++i) sequence.push_back(held_right());
        {
            Input jump; jump.jump = true;
            sequence.push_back(jump);
        }
        for (int i = 0; i < 30; ++i) sequence.push_back(NOTHING);
        for (int i = 0; i < 20; ++i) {
            Input paint;
            paint.brush_active = true;
            paint.brush_type = ElementType::Water;
            paint.brush_size = 3;
            paint.cursor_x = 60;
            paint.cursor_y = 10;
            sequence.push_back(paint);
        }
        for (int i = 0; i < 25; ++i) {
            Input dig;
            dig.dig = true;
            dig.cursor_x = 60;
            dig.cursor_y = 20;
            sequence.push_back(dig);
        }

        Run a(90, 50, 4040);
        build_floor(a, 45);
        for (const Input& in : sequence) a.step(in);

        // Same sequence, same seed, but consumed in batches of 7 with a
        // render-style read (get_pixels(), active_chunk_count()) between
        // batches - exactly what main.cpp does every rendered frame between
        // groups of fixed steps. If pacing or an intervening read could
        // perturb the result, this would be the run that shows it.
        Run b(90, 50, 4040);
        build_floor(b, 45);
        size_t i = 0;
        while (i < sequence.size()) {
            const size_t batch = std::min<size_t>(7, sequence.size() - i);
            for (size_t j = 0; j < batch; ++j) b.step(sequence[i + j]);
            i += batch;
            (void)b.grid.get_pixels();
            (void)b.grid.active_chunk_count();
        }

        check("a replayed Input sequence reproduces the same world regardless of batching",
              worlds_match(a.grid, b.grid));
        check("...and the same player position",
              a.player.cell_x() == b.player.cell_x() && a.player.cell_y() == b.player.cell_y());
    }

    return report();
}
