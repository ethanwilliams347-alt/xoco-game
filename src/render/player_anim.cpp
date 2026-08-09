#include "render/player_anim.h"

namespace player_anim {

using player_sprite::Anim;

const Anim& select(const Conditions& c) {
    if (!c.on_ground) {
        // Two poses on one sheet row, chosen by the sign of the fall rather
        // than advanced by a clock. A long drop holds the falling pose instead
        // of cycling through a rise it is plainly not doing - which is what a
        // two-frame looping "air" animation would have done, and it is the
        // reason these are two named animations sharing a row.
        //
        // Zero counts as rising, so the apex of a jump does not flicker
        // between the two poses on the one step the velocity passes through
        // it.
        return c.vel_y <= 0.0f ? player_sprite::RISE : player_sprite::FALL;
    }
    return c.moving ? player_sprite::WALK : player_sprite::IDLE;
}

namespace {

void begin(State& s, const Anim& a, bool oneshot) {
    // Re-selecting the animation already playing must not restart it, or a
    // walk cycle resets to frame zero every single step and the figure stands
    // still with one leg twitching. Identity is the pointer, which works
    // because every Anim is a distinct constexpr object in the generated
    // header.
    if (s.anim == &a && s.oneshot == oneshot) return;
    s.anim = &a;
    s.frame = 0;
    s.elapsed = 0;
    s.oneshot = oneshot;
}

}  // namespace

void update(State& s, const Conditions& c, int steps) {
    if (c.dig_fired) {
        // Latch, and restart even if a dig is already playing - a second dig
        // landing mid-swing is a new swing, not a continuation of the old one.
        s.anim = &player_sprite::DIG;
        s.frame = 0;
        s.elapsed = 0;
        s.oneshot = true;
    } else if (c.flapped && !c.on_ground) {
        // Same latch-and-restart as the dig, and for the same reason: each
        // beat is its own downstroke, not a continuation of the last. Because
        // FLY's own length exceeds the interval between beats (see the note on
        // the fly row in tools/player_sheet.py), sustained flight is this
        // branch firing over and over rather than an animation left to loop -
        // which is what keeps the wings in phase with the impulses the physics
        // is applying instead of merely near them.
        //
        // **`!on_ground` is doing real work.** The launch off the ground is a
        // beat too and sets `flapped`, but on the step it fires the feet are
        // still down; without the guard, tapping the key while standing plays
        // a wing beat in place, which reads as the bird flapping at the floor.
        // The airborne steps that follow pick it up on the next beat.
        s.anim = &player_sprite::FLY;
        s.frame = 0;
        s.elapsed = 0;
        s.oneshot = true;
    } else if (s.oneshot && s.anim == &player_sprite::FLY && c.on_ground) {
        // Landing cancels a beat still in flight. A one-shot normally owns the
        // clock until it finishes, which is right for a dig - a swing is not
        // interrupted by touching the floor - and wrong for a wing beat, whose
        // entire premise is that the body is in the air. Without this the bird
        // spends up to fifteen steps flapping while stood on solid ground.
        s.oneshot = false;
        begin(s, select(c), false);
    } else if (!s.oneshot) {
        begin(s, select(c), false);
    }

    for (int i = 0; i < steps; ++i) {
        // A wait of zero means "this animation does not advance on a clock" -
        // the single-frame poses. Guarding it here rather than forbidding it in
        // the table keeps a zero from becoming an infinite loop below.
        if (s.anim->wait <= 0 || s.anim->frames <= 1) {
            if (!s.oneshot) continue;
            // A one-shot that cannot advance would never finish. Treat it as
            // complete rather than hanging on frame zero forever.
            s.oneshot = false;
            begin(s, select(c), false);
            continue;
        }

        if (++s.elapsed < s.anim->wait) continue;
        s.elapsed = 0;

        if (++s.frame < s.anim->frames) continue;

        if (s.anim->loop) {
            s.frame = 0;
        } else {
            // Done. Hand control back to the conditions, which is what makes
            // this a one-shot rather than an animation that has to be cleared
            // by whoever set it.
            s.oneshot = false;
            s.frame = 0;
            begin(s, select(c), false);
        }
    }
}

}  // namespace player_anim
