#pragma once
#include <string>
#include "physics/element.h"
#include "physics/grid.h"

// T1's debug tooling - pause and single-step, a free camera, and the cell
// inspector's text - as one object rather than as four more locals in main()'s
// frame loop.
//
// **SDL-free and header-only, like camera.h and display.h beside it**, and that
// is the whole reason the interesting parts of T1 are in here rather than in the
// event handler. Everything below is a decision with a wrong answer - what a
// pause does to the accumulator, what a free camera does to the body, what a
// burst of key-repeat does to a single-step - and a decision that only exists
// inside an SDL event switch is one no suite can reach. What stays in main.cpp
// is the key bindings and the drawing, which is the half a test could not check
// anyway.
//
// **It is not in ENGINE_SOURCES, RENDER_SOURCES or SCENE_PROP_SOURCES**, and it
// deliberately does not become a fourth variable: it is a header, so it links
// into whatever includes it, and the guard those three variables exist to
// enforce is untouched. Nothing under src/physics/ may include this, for the
// same reason nothing there may read the light field - it is an instrument for
// looking at the simulation, and an instrument the simulation can read is a
// simulation input.
struct DebugView {
    // --- pause and single-step ---------------------------------------------
    //
    // **The pause freezes the world by not accumulating time, and this is the
    // third user of that one mechanism rather than a second mechanism.** The
    // settings menu froze this way first, a finished run followed it in S0, and
    // the alternative - skipping the step loop while the accumulator keeps
    // filling - banks every second spent paused and spends it in one burst of
    // catch-up steps on resume, which at MAX_FRAME_TIME's quarter-second clamp
    // is a visible lurch. The `Pause and single-step` entry in ROADMAP.md names
    // that as the one thing to get right, and it is got right by copying what
    // is already here instead of writing something new.
    bool paused = false;

    // Single-steps asked for and not yet run. An `int` rather than a `bool`
    // because holding the key down produces OS key-repeat events and stepping
    // once per repeat is a genuinely useful way to scrub through a collapse.
    //
    // **Capped, because a burst of repeats inside one long frame would run as
    // many steps as arrived and that is unpausing by accident** - the whole
    // value of a single-step is that the number of steps that ran is a number
    // you know.
    static constexpr int MAX_QUEUED_STEPS = 8;
    int queued_steps = 0;

    void toggle_pause() {
        paused = !paused;
        // Anything queued belongs to the pause it was asked for during. Leaving
        // it would spend queued steps immediately on resume, on top of the
        // frame's own, which is the lurch this whole design is avoiding.
        queued_steps = 0;
    }

    // Ignored while running, on purpose: `.` during play would advance the world
    // by one step *on top of* the steps the frame already runs, which is not a
    // single-step, it is a stutter with no way to see that it happened.
    void request_single_step() {
        if (!paused) return;
        if (queued_steps < MAX_QUEUED_STEPS) queued_steps++;
    }

    // True once per request. The caller runs exactly one fixed step per true and
    // must not touch the accumulator while doing it.
    bool consume_single_step() {
        if (queued_steps <= 0) return false;
        queued_steps--;
        return true;
    }

    // --- the free camera ---------------------------------------------------
    //
    // **The camera follows the player and nothing else, so anything the player
    // is not standing next to cannot be looked at.** That is why this exists and
    // why T1 sits in front of E10 and E5a: "a poured pile holds a measurable
    // angle" and "a cell fired at a wall lands against it" are both verified by
    // looking at a place, and a camera bolted to a body cannot be pointed at
    // one.
    bool free_camera = false;

    // The centre the camera is asked to follow while detached, in world cells.
    // A float because Camera::follow takes one - a camera that can only sit on
    // whole cells is the larger half of defect A1, and a debug camera that
    // reintroduces it would make the free camera useless for judging exactly the
    // rendering defects it was promoted to judge.
    float cam_x = 0.0f;
    float cam_y = 0.0f;

    // Cells per second of pan, and the multiplier while a fast modifier is held.
    // 200 crosses the 480-cell viewport in about two and a half seconds, which
    // is a speed you can stop on something; fast crosses the 1920-cell world in
    // about the same time, which is a speed you get somewhere with.
    static constexpr float PAN_CELLS_PER_SECOND = 200.0f;
    static constexpr float PAN_FAST_MULTIPLIER = 4.0f;

    // Detaching starts where the attached camera was, so the view does not jump
    // on the frame the key is pressed. Anything else means the first thing a
    // free camera does is lose the thing you were looking at.
    //
    // **Clamped on the way in, for the reason `pan` is.** The body spends a lot
    // of its time within half a viewport of a world edge, where `Camera::follow`
    // is clamping - and a centre taken raw from there is outside the pannable
    // range, so the first press of a pan key would do nothing for up to a
    // second. Clamping here cannot move the view, because `follow` was clamping
    // that same centre to that same view a moment ago; all it changes is that
    // the control answers immediately.
    void detach_camera(float center_x, float center_y, int viewport_w, int viewport_h,
                       int world_w, int world_h) {
        free_camera = true;
        cam_x = clamp_centre(center_x, viewport_w, world_w);
        cam_y = clamp_centre(center_y, viewport_h, world_h);
    }

    void attach_camera() { free_camera = false; }

    // **Clamped to the centres that produce distinct views, not merely to the
    // world, and not left to Camera::follow's own clamp.** follow() clamps the
    // *view*, so a centre outside this range looks like a key doing nothing and
    // then banks an offset that has to be un-panned before the view moves again
    // - a control that stops responding and later responds late, which is
    // indistinguishable from a stuck key. Clamping to the world instead of to
    // this range leaves the same defect in a band half a viewport wide along
    // every edge, which is where you go to look at the world's corners and
    // therefore exactly where a free camera gets used.
    //
    // The range is `Camera::clamp_view`'s, transposed from a view to a centre,
    // and it collapses to a single point on an axis where the world is no bigger
    // than the viewport - there the view cannot move at all, so neither should
    // this.
    void pan(float dx_cells, float dy_cells, int viewport_w, int viewport_h,
             int world_w, int world_h) {
        cam_x = clamp_centre(cam_x + dx_cells, viewport_w, world_w);
        cam_y = clamp_centre(cam_y + dy_cells, viewport_h, world_h);
    }

    // --- the cell inspector ------------------------------------------------
    //
    // Off by default and toggled, rather than a permanent extra line. The same
    // argument the recorder's notice is drawn on: a readout that is always there
    // is furniture within a minute and invisible by the time it matters, and
    // this one would also push the line README's launch check reads.
    bool inspector = false;

private:
    static float clamp(float v, float lo, float hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    static float clamp_centre(float v, int viewport_len, int world_len) {
        const float half = static_cast<float>(viewport_len) / 2.0f;
        if (world_len <= viewport_len) return static_cast<float>(world_len) / 2.0f;
        return clamp(v, half, static_cast<float>(world_len) - half);
    }
};

// What the cell under the cursor actually holds, as one line.
//
// **E2 and E3 both added per-cell state that is invisible from the window and
// both were debugged without this.** Temperature decides every reaction and
// `piece_tag` decides what a fracture cut; neither has ever been readable while
// the thing being debugged was on screen.
//
// Uppercase-free and comma-free by construction: ui::draw_text upper-cases what
// it is given and draws an unknown character as a blank, so a comma would be an
// invisible gap rather than a missing glyph. `X:1234 Y:567` costs two characters
// over `1234,567` and needs no new row in the font.
inline std::string describe_cell(const Grid& grid, int x, int y) {
    std::string s = "X:" + std::to_string(x) + " Y:" + std::to_string(y) + " ";

    if (x < 0 || x >= grid.get_width() || y < 0 || y >= grid.get_height()) {
        // Named rather than blanked. Out of the world and empty are two very
        // different answers to "what is under the cursor", and the free camera
        // makes the first one reachable by ordinary mouse movement for the first
        // time - the view can now sit at a world edge with half the window off
        // the end of the grid.
        return s + "OUTSIDE THE WORLD";
    }

    const Element e = grid.get_element(x, y);
    s += material_of(e.type).name;
    s += " T:" + std::to_string(static_cast<int>(e.temperature));

    // Whether the chunk under the cursor will be simulated on the next step.
    // This is the answer to "has it settled yet?" at a *place*, which the HUD's
    // world-wide awake-chunk count cannot give: E10's whole subject is powders
    // coming to rest, and "the world still has 30 chunks awake" does not say
    // whether this pile is one of them.
    //
    // **Labelled `CHUNK:` rather than printed as a bare AWAKE/ASLEEP, because a
    // bare one would be a claim about the cell and this is not one.** The first
    // thing this inspector was pointed at was a slab in mid-fall, which reads
    // asleep: a falling structural piece is carried by the support queue, not by
    // the chunk rects, so its chunks are genuinely idle while it moves. The
    // engine is right and the label was wrong - see the corrected comment on
    // `Grid::active_chunk_count()`, and note that `FALL:` below is the field
    // that actually says this cell is moving.
    s += grid.chunk_awake_at(x, y) ? " CHUNK:AWAKE" : " CHUNK:ASLEEP";

    switch (tick_role(e.type)) {
        case TickRole::FallClock:
            if (e.ticks) s += " FALL:" + std::to_string(static_cast<int>(e.ticks));
            break;
        case TickRole::Lifetime:
            s += " LIFE:" + std::to_string(static_cast<int>(e.ticks));
            break;
        case TickRole::None:
            // **Only ever printed when something is wrong.** A material with no
            // role for `ticks` should carry zero in it forever - `place()`
            // builds a fresh Element on every transition, which is what
            // guarantees it - so a non-zero here is a fourth claimant on the
            // byte that element.h's assertions cannot see, because they check
            // the table rather than the world. Cheap to surface and it can only
            // ever appear on a real defect.
            if (e.ticks) s += " STRAY TICKS:" + std::to_string(static_cast<int>(e.ticks));
            break;
    }

    if (e.piece_tag) s += " PIECE:" + std::to_string(static_cast<int>(e.piece_tag));
    return s;
}

// The debug state, as a line, or empty when none of it is on.
//
// **A paused game that does not say it is paused is indistinguishable from a
// hung one**, and this project has one unexplained hard crash on the record
// already - a session where the world stopped and nobody could tell which of the
// two had happened would be a wasted afternoon. The free camera has the milder
// version of the same problem: the body stops answering the movement keys, and
// without a word on screen that reads as broken input rather than as a mode.
inline std::string debug_status(const DebugView& view) {
    std::string s;
    if (view.paused) s += "PAUSED (. STEPS)";
    if (view.free_camera) {
        if (!s.empty()) s += "  ";
        s += "FREE CAMERA (F RETURNS)";
    }
    return s;
}
