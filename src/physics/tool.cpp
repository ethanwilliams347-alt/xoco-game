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
    // A swing that has run its length is over before this step is considered,
    // so a held button starts the next one on the same step the last finished
    // and the cycle has no seam in it. That seamlessness is the requirement -
    // a one-step gap at the top of every swing is a stutter at 60 Hz.
    if (swing >= 0 && ++swing >= SWING_STEPS) swing = -1;

    // Mid-swing is follow-through: the blow already landed on the step the
    // swing opened, and these steps exist so the animation has somewhere to
    // play out and so the next blow cannot arrive immediately.
    if (swing >= 0) return false;
    if (!held) return false;

    int hit_x = 0;
    int hit_y = 0;
    bool hit = false;
    march(grid, from_x, from_y, aim_x, aim_y, hit_x, hit_y, hit);

    // Swinging at thin air costs nothing and starts nothing. Only a connecting
    // shot commits the body to a swing, which is what makes the rate feel like
    // a tool speed rather than an input lockout - and it also means the player
    // is not locked into 36 steps of animation for a swing that was never going
    // to hit anything.
    if (!hit) return false;

    swing = 0;

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
