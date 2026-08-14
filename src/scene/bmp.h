#pragma once
#include "scene/scene.h"
#include <cstdint>
#include <string>
#include <vector>

// Reading an authored BMP without SDL, and turning a material/albedo pair into
// a `Scene`.
//
// **This exists because of P4, and the reason is worth stating: the scene
// loader used to live inside `main.cpp`, where nothing headless could reach
// it.** P4 replays a recorded session through the benchmark, and a replayed
// session that starts in a *different world* than the one it was recorded in
// measures nothing - so the bench has to stamp the same fixture scene the game
// stamps, and it has no window, no renderer and no SDL.
//
// The reader is deliberately not a second implementation: `main.cpp` calls this
// too, and its own `SDL_LoadBMP` path is gone. Two readers agreeing today is
// two readers that can disagree later, which is the shape of D1's two clocks
// and F6's two range tests - both of which were one answer computed two ways,
// and both of which drifted. The unmatched-colour reporting moved across
// unchanged, because that reporting is the whole of why a broken legend is
// visible at all (see scene/legend.h).
namespace bmp {

// One decoded image, **top row first** regardless of how the file stores it,
// as 0xAARRGGBB with alpha forced opaque. Matches `tools/pixel_art.py`'s
// `read_bmp`, which is the other reader of these same files.
struct Image {
    int width = 0;
    int height = 0;
    std::vector<uint32_t> pixels;
};

// 24-bit and 32-bit uncompressed BMPs, which is what every tool in `tools/`
// writes. Anything else is refused with a message rather than half-read:
// a palette-indexed or RLE file decoded as raw rows produces a *plausible*
// image, and a scene that loads as noise is harder to diagnose than one that
// does not load.
bool read(const char* path, Image& out, std::string* error);

// Builds a `Scene` from a material map and an albedo map, exactly as the game
// does. Returns an empty scene on failure with `error` set.
//
// `warning` is set (rather than `error`) when the file loaded but named a
// colour in no legend entry - which is a fault in the *scene*, not in the load,
// and which the caller is expected to print. That distinction is not a nicety:
// a palette change once made all 27,192 authored pixels unmatched, the world
// booted blank, and every suite passed.
Scene load(const char* material_path, const char* albedo_path,
           std::string* error, std::string* warning);

} // namespace bmp
