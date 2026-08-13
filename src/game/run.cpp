#include "run.h"

Run::Run(int width, int height, uint64_t seed)
    : grid(width, height, seed)
    , player(width / 2, height / 4)
{
}

void Run::reset(uint64_t seed) {
    grid.reset(seed);
    player = Player(grid.get_width() / 2, grid.get_height() / 4);
    dig_tool = DigTool();
}

bool Run::step(const Input& input) {
    // A filled circle of radius brush_size, painted before physics runs so a
    // freshly placed cell does not move on the same step it was placed.
    //
    // `displace` rather than `set_element`, which is A6: the brush is the one
    // writer that has to move what it lands on instead of deleting it.
    //
    // **Bottom row first, and the order is load-bearing for exactly one brush.**
    // Displacement lifts the occupant to the first Empty above it and gives up
    // at anything static. Painting top-down with the Wall brush would therefore
    // build a lid over the rest of the disc a row at a time, and every row under
    // it would find no room and be deleted - the same defect A6 is about, moved
    // one level up. Bottom-up, each row escapes before the row above it exists.
    // No other brush shows it, because everything else the brush paints is
    // movable and a climb walks straight through it.
    if (input.brush_active) {
        for (int dy = input.brush_size; dy >= -input.brush_size; --dy) {
            for (int dx = -input.brush_size; dx <= input.brush_size; ++dx) {
                if (dx * dx + dy * dy <= input.brush_size * input.brush_size) {
                    grid.displace(input.cursor_x + dx, input.cursor_y + dy, input.brush_type);
                }
            }
        }
    }

    grid.update();

    // After the grid, so the player collides against the world as it now is
    // rather than as it was a step ago. Player::update() takes its own
    // PlayerInput rather than an Input - built here rather than widening
    // PlayerInput's job, since the brush is a run-level concern the player
    // itself has no business knowing about.
    PlayerInput player_input;
    player_input.left = input.left;
    player_input.right = input.right;
    player_input.jump = input.jump;
    player_input.aim_x = input.cursor_x;
    player_input.aim_y = input.cursor_y;
    player_input.dig = input.dig;
    player.update(grid, player_input);

    // Last, so the dig is aimed from where the body actually ended up this
    // step. Called every step whether or not the button is held, because
    // that is what advances the tool's cooldown.
    return dig_tool.update(grid, input.dig, player.center_x(), player.center_y(),
                           input.cursor_x, input.cursor_y);
}
