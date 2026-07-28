#include "tool.h"
#include <algorithm>
#include <cmath>

void DigTool::march(const Grid& grid, int from_x, int from_y, int aim_x, int aim_y,
                    int& out_x, int& out_y, bool& out_hit) const {
    out_hit = false;
    out_x = from_x;
    out_y = from_y;

    const int dx = aim_x - from_x;
    const int dy = aim_y - from_y;

    const int span = std::max(std::abs(dx), std::abs(dy));
    if (span == 0) return; // aiming at your own feet

    // Range is a real distance, not a cell count along the dominant axis --
    // otherwise a diagonal dig would reach ~1.4x as far as a straight one.
    const float len = std::sqrt(static_cast<float>(dx * dx + dy * dy));
    const int steps = (len <= static_cast<float>(RANGE))
                          ? span
                          : static_cast<int>(static_cast<float>(span) * (static_cast<float>(RANGE) / len));

    for (int i = 1; i <= steps; ++i) {
        // One cell per iteration along the dominant axis, so the path is
        // connected and nothing is skipped over.
        const int cx = from_x + static_cast<int>(std::lround(static_cast<float>(dx) * i / span));
        const int cy = from_y + static_cast<int>(std::lround(static_cast<float>(dy) * i / span));

        out_x = cx;
        out_y = cy;

        // The same is_solid() the player collides against, so what blocks a dig
        // is exactly what blocks a body -- terrain and powders stop the ray,
        // water and fire do not. One definition, used twice.
        //
        // get_element() reads out-of-bounds as Wall, so the ray stops at the
        // world border rather than running off forever. The border still cannot
        // be dug through: set_element() bounds-checks, so the part of the hole
        // that lies outside the world is silently dropped.
        if (is_solid(grid.get_element(cx, cy).type)) {
            out_hit = true;
            return;
        }
    }
}

void DigTool::aim_point(const Grid& grid, int from_x, int from_y, int aim_x, int aim_y,
                        int& out_x, int& out_y) const {
    bool hit = false;
    march(grid, from_x, from_y, aim_x, aim_y, out_x, out_y, hit);
}

bool DigTool::update(Grid& grid, bool held, int from_x, int from_y, int aim_x, int aim_y) {
    if (cooldown > 0) --cooldown;
    if (!held || cooldown > 0) return false;

    int hit_x = 0;
    int hit_y = 0;
    bool hit = false;
    march(grid, from_x, from_y, aim_x, aim_y, hit_x, hit_y, hit);

    // Digging thin air costs nothing and starts no cooldown. Only a connecting
    // shot has a rate limit, which is what makes the limit feel like a tool
    // speed rather than a random input lockout.
    if (!hit) return false;

    cooldown = COOLDOWN_STEPS;

    // This deletes matter outright, which is correct for a tool and would be a
    // bug anywhere inside the simulation step -- see the conservation-of-matter
    // test, which covers Grid::update() and is testing a different thing.
    //
    // Going through set_element() rather than touching cells[] is what wakes
    // the 3x3 neighbourhood of every removed cell, and that is the whole reason
    // digging under a settled pile makes the pile fall instead of leaving it
    // hanging over the hole.
    for (int oy = -RADIUS; oy <= RADIUS; ++oy) {
        for (int ox = -RADIUS; ox <= RADIUS; ++ox) {
            if (ox * ox + oy * oy > RADIUS * RADIUS) continue;
            grid.set_element(hit_x + ox, hit_y + oy, ElementType::Empty);
        }
    }

    return true;
}
