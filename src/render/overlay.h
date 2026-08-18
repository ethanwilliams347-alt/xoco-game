#pragma once
#include <SDL.h>
#include <cstdint>
#include <string>
#include <vector>
#include "game/display.h"

// The screen-space layer of one frame: the reticle, the HUD stack, the hotbar,
// the run-over wash and the settings screen, in the order they are drawn. W5
// part 3's half of the extraction, and at this commit it is *only* a move -
// every line in overlay.cpp came out of main.cpp unchanged, comments included,
// and golden_frame_test says so in a number rather than this sentence doing it.
//
// **This is not frame.cpp and must not become part of it.** frame.h's rule -
// "UI is not here and does not become here" - is about the light pass, not
// about which file the code sits in: everything in frame.cpp is in the world
// and gets lit, and a reticle that goes orange near a flame is defect B1. That
// rule is why this is a second translation unit with its own entry point,
// called by main.cpp *after* `frame::compose` returns, rather than a run of
// rows appended to the layer table. The composition never learns these exist.
//
// **What it buys is reach, not tidiness.** These ~250 lines were the largest
// block in the project that no suite linked, so every question about them -
// does the HUD backing still cover the text, does the wash still sit under the
// menu, is the selected row still marked - could only be answered by looking at
// a window. Behind one function taking one struct they are inside
// golden_frame_test, where the answer is a checksum.
namespace overlay {

// One line of the HUD stack under the main readout, with its colour. A vector
// rather than a fixed set of optional fields because T1 makes three of them
// conditional and the whole point of the cursor this draws with is that the
// lines do not know their own y - see the note at `draw` in overlay.cpp for the
// two-lines-on-top-of-each-other bug that shape exists to prevent.
struct Line {
    std::string text;
    uint32_t colour = 0xFFE0E0E0;
};

// Everything the screen-space layer reads, and nothing else.
//
// The same seam frame::Params draws: every field is something the caller
// already had, there is no state here, and nothing in this struct knows about a
// Run, a Grid or a window. **The HUD's strings arrive already built** - what the
// readout says is a decision about simulation state and belongs to the caller;
// where it lands on the screen is what this file owns.
struct Params {
    // The window, in screen pixels, and the UI scale that goes with it.
    int window_w = 0;
    int window_h = 0;
    int ui_scale = 1;

    // V10/B1's reticle. `show` is false when the pointer is not over this
    // window - SDL_GetMouseState keeps reporting the last position inside it
    // after the mouse leaves, and the caller is the only one that can tell.
    bool show_reticle = false;
    int mouse_x = 0;
    int mouse_y = 0;
    bool in_range = false;

    // The main readout, then the stack under it in draw order.
    std::string hud_text;
    std::vector<Line> hud_lines;

    // Index into ui::HOTBAR, or -1 for no highlight.
    int hotbar_selected = -1;

    // S0's ending. `won` picks the headline and its colour.
    bool run_over = false;
    bool won = false;

    // The settings screen. `modes`/`available` are arrays of `mode_count`, and
    // `cursor` may be `mode_count` itself, which is the Quit row - the same
    // index menu::quit_index returns, and it is passed rather than recomputed
    // so the drawing and the state machine cannot disagree about which row is
    // which.
    bool settings_open = false;
    int cursor = 0;
    int mode_count = 0;
    int current_mode = -1;
    const DisplayMode* modes = nullptr;
    const bool* available = nullptr;
    std::string notice;   // empty draws nothing
};

// Draws the screen-space layer over whatever is already in the framebuffer.
// Presents nothing, and leaves the draw colour and blend mode as it found
// them - main.cpp's present is the next thing that runs, and the golden frame
// test reads the surface instead.
void draw(SDL_Renderer* renderer, const Params& p);

} // namespace overlay
