#pragma once
#include <cmath>

// V23: where the player sits on the screen, and why it moves when they dig.
//
// **SDL-free and header-only, like camera.h, display.h and debug_view.h beside
// it.** The rule this follows is T1's: when a shell feature contains a decision
// with a wrong answer, the decision goes in a header under `src/game/` and only
// the key binding stays in `main.cpp`. Written into the SDL frame loop instead,
// every constant and every edge below would be unreachable by any test.
//
// ---------------------------------------------------------------------------
// What this is for
//
// The receding ground plane (V19/V22) was invisible in play, and three rounds
// of value tuning failed to fix it because they were aimed at the wrong thing:
// `draw_ground` runs the plane from the horizon to the **bottom of the window**
// unconditionally, and the fixture scene's floor is a full-width slab at row
// 950 of 1080, so the plane's share of the screen in front of the player was
// not small - it was **exactly zero**. No grade reaches pixels the plane never
// drew.
//
// The measurement that turned that into this item: `Camera::follow` centred
// strictly, so wherever the view is unclamped the player is pinned at screen
// centre and the plane's visible band caps at ~50% **by construction**. At the
// spawn it measured 20.2%. Both reference frames put it at 64-70%.
//
// **So this is a camera item and could never have been a scene item.** The
// scene does not decide where on screen the player is drawn.
//
// ---------------------------------------------------------------------------
// The trade, and the answer that dissolved it
//
// Anchoring the player low enough to match the reference (~0.80 down the frame)
// leaves only ~55 cells of world visible *below* them, which starves digging -
// the game's one verb. That was filed as an open decision, recommendation
// "split the difference", precisely because it looked like a straight conflict:
// **the reference composes a scene you look across, and this is a world you dig
// down into, and they want opposite thirds of the frame.**
//
// The resolution came from reading all three reference frames together instead
// of one at a time (notes/reference_observations.txt entry 5 and entry 9):
//
//     CnC_fishing.jpg      subject ~0.60 down    look *across* the plane
//     CnC_underwater_1.jpg subject ~0.36 down    look *down* the water column
//     CnC_underwater_2.jpg subject ~0.27 down    look *down* the water column
//
// **What is constant across all three is the composition, not the subject.**
// The near volume - the lit plane above the water, the column below it - holds
// the lower ~55-65% of the frame in every one. What moves is the subject, and
// it moves *up* exactly when the interesting volume is underneath it.
//
// So the two framings are not in conflict; they are the same rule in two
// states, and the state is chosen by what the player is doing. Standing on the
// surface, they sit low and the plane gets its two thirds. Digging downward,
// they rise and the excavated column gets them instead. The camera travels
// between the two, which is what makes it read as one continuous world rather
// than as two cameras - the same conclusion entry 5 already reached about the
// reference's own "split screen", which turned out to be one camera plus a UI
// gauge.
//
// ---------------------------------------------------------------------------
// The feedback loop, which is the one real trap in here
//
// The anchor moves the view; the view is what `screen_to_world_y` resolves the
// mouse through; the resolved cursor cell is what `Input` carries into the
// step; and the aim is what this class reads to decide the anchor. That is a
// closed loop, and it is **positive**: digging downward raises the view, which
// slides the world position under a stationary mouse further down, which reads
// as a steeper downward aim, which raises the view again. It does not
// oscillate - it saturates - but it would run the camera to the dig framing
// from an aim that was never steep enough to ask for it, and the player would
// feel the view crawling away under a still hand.
//
// **Cut by measuring the aim in the unbiased frame.** `update` is handed the
// cursor's offset from the player and adds the anchor's own contribution back
// out of it, so the angle this reads is the angle that would have been aimed at
// centre. The loop is broken at the point where it closes, rather than damped
// by a slow ease and hoped about.
class CameraBias {
public:
    // Both read straight off the reference frames above, and both are feel
    // constants with rows in TUNING.md.
    //
    // SURFACE is 0.80 rather than the fishing frame's 0.60 because that frame's
    // plane is cut by its split-view waterline rather than by the frame edge -
    // its fraction is not the same measurement as ours and the two are not
    // averageable (entry 9's disproof run). What survives from the references is
    // the ordinal claim, "clearly past half, around two thirds", and 0.80 is
    // where our own arithmetic puts the plane's contact at the ~65% that claim
    // asks for.
    static constexpr float SURFACE_ANCHOR = 0.80f;

    // The framing for when the interesting volume is *below* the player -
    // digging into it, or falling through it. **Named for the situation and not
    // for the dig**, because session 8 gave it a second trigger; it was called
    // `DIG_ANCHOR` until 2026-08-17.
    //
    // **0.50 is the tester's number and it replaced 0.30, which was the mean of
    // the two underwater reference frames.** The reference reading was not
    // wrong about the references; it was unreachable in this world, and that is
    // what session 8 reported as the camera being "upside down".
    //
    // The arithmetic, because it is the whole argument. The view clamps at
    // `world_h - viewport_h` = 1080 - 270 = 810, so at the fixture scene's
    // floor (row ~948) a target of 0.30 resolves to **0.51 on screen**, and at
    // row 1000 to 0.70 - a swing of a tenth. **The camera answered the dig
    // least where there was most world below to see, and most where there was
    // least**, which is an inversion of the item's whole intent and reads
    // exactly as a move that starts and does not arrive. 0.50 is delivered
    // wherever the player actually plays: at the fixture floor it resolves to
    // 0.511, inside the tolerance the test asserts.
    //
    // **So this is not the reference being overruled by feel.** It is the
    // reference reading meeting a constraint the item never checked it against,
    // and `camera_bias_test` now pins the delivered framing rather than the
    // requested one - the check that would have caught this before a human had
    // to.
    static constexpr float COLUMN_ANCHOR = 0.50f;

    // Anchor units per second. **The full swing was 0.50 and is 0.30 since
    // session 8 moved the column framing to 0.50, so this crosses it in 0.35 s
    // rather than the 0.6 s this comment used to quote** - the constant did not
    // change, the distance it covers did, and the speed the tester was asked
    // about is therefore not the speed they saw. Fast enough to read as the
    // camera answering the dig, slow enough to be a move rather than a cut;
    // still the one number here that came from nothing.
    static constexpr float EASE_PER_SEC = 0.85f;

    // `aim_dx`/`aim_dy` are the cursor's offset from the player in world cells,
    // y down, exactly as the dig tool sees it. `viewport_h` is needed to undo
    // the anchor's own contribution to that offset - see the feedback note
    // above; it is the same viewport height handed to `Camera::follow`.
    void update(float dt_seconds, bool digging, bool on_ground, int aim_dx, int aim_dy,
                int viewport_h) {
        current_ = ease(current_, target(current_, digging, on_ground, aim_dx, aim_dy, viewport_h),
                        dt_seconds);
    }

    float anchor() const { return current_; }

    // Exposed for the test, and for a reason worth stating: `update` composes
    // two decisions - which framing the aim asks for, and how fast the camera
    // is allowed to get there - and a suite that could only see the composed
    // result would have to reverse one out of the other to check either.
    //
    // `anchor_now` is the anchor the handed-in aim was resolved through, which
    // has to be the live one and not `SURFACE_ANCHOR`: the cursor's world cell
    // is `mouse_y / SCALE + view_fy` and `view_fy` carries `-viewport_h *
    // anchor_now`, so adding that term back **cancels it exactly** and leaves
    // the aim that a centred camera would have reported for the same hand
    // position. A constant here would leave a residue that grows with the
    // camera's distance from where the constant assumed it was, which is the
    // feedback loop back in a quieter form.
    static float target(float anchor_now, bool digging, bool on_ground, int aim_dx, int aim_dy,
                        int viewport_h) {
        // **Airborne takes the column framing outright, ahead of every aim
        // test below.** Session 8 asked for it in the same breath as the dig
        // ("digging or flying out of frame"), and the reason it belongs first
        // is that a falling player has no aim worth reading: the volume that
        // matters has gone underneath them whatever the cursor is doing, and
        // the surface framing leaves them ~55 cells from the bottom edge,
        // which a terminal-velocity fall crosses in well under a second.
        //
        // Consequence worth stating rather than discovering: **an ordinary jump
        // moves the camera now**, since a jump is airborne. The ease takes 0.35
        // s to cross this gap and a hop is shorter than that, so a hop reads as
        // a lean rather than a swing - but if the re-test comes back "the
        // camera twitches when I jump", this is the line that did it, and the
        // fix is a condition on descending rather than on airborne.
        if (!on_ground) return COLUMN_ANCHOR;

        if (!digging) return SURFACE_ANCHOR;

        const float unbiased_dy =
            static_cast<float>(aim_dy) + static_cast<float>(viewport_h) * (anchor_now - 0.5f);

        // How downward the aim is, 0 to 1. Digging level or upward keeps the
        // surface framing outright: the reason to move the camera is that the
        // interesting volume has gone *underneath* the player, and neither of
        // those cases has.
        const float len = std::sqrt(static_cast<float>(aim_dx) * static_cast<float>(aim_dx) +
                                    unbiased_dy * unbiased_dy);
        if (len <= 0.0f) return SURFACE_ANCHOR;
        const float downward = clamp01(unbiased_dy / len);

        return SURFACE_ANCHOR + (COLUMN_ANCHOR - SURFACE_ANCHOR) * downward;
    }

    // Frame-rate independent by construction rather than by tuning: a fixed
    // step per call would move the camera at the render rate, which is the
    // exact class of defect F2.3 removed from the simulation and there is no
    // reason to reintroduce it in the view.
    static float ease(float from, float to, float dt_seconds) {
        const float step = EASE_PER_SEC * dt_seconds;
        const float gap = to - from;
        if (gap > step) return from + step;
        if (gap < -step) return from - step;
        return to;
    }

private:
    static float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

    // Starts where a standing player belongs, so the first frame of a run is
    // already composed rather than easing into position from centre.
    float current_ = SURFACE_ANCHOR;
};
