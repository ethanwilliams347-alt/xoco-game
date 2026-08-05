#pragma once
#include <string>
#include <vector>

// The authorable half of V4's props layer: *which* non-simulated sprites a
// scene has and *where along the ground* each one stands.
//
// **Why this is a text file and not a second BMP, decided against the obvious
// precedent rather than without noticing it.** F4 authors terrain as a
// material map, and the reflex is to author props the same way - a prop map
// where one legend-coloured pixel marks one anchor. That is the right shape
// for per-cell data and the wrong shape for this. A material map is a value
// for every cell in the world; a prop list is nine records. The scene BMPs are
// grid-sized (1920x1080, ~6.2 MB each), so a parallel prop map would be
// another 6.2 MB of black to carry nine meaningful pixels - and it still could
// not say *which* sprite each pixel meant without a legend row per tree
// species, which is `scene/legend.h`'s frozen table growing a row every time an
// artist draws a new prop. The legend is frozen precisely so that cannot
// happen. So: per-cell data gets an image, a list gets a list.
//
// **There is no y coordinate here and that is the format's one real opinion.**
// Props are planted by scanning the terrain actually underneath them (see
// `snap_prop_to_terrain` in main.cpp) rather than by an authored ground line,
// because "the ground" is not one number - the fixture's floor slab is at
// FLOOR_TOP and the authored sand slope rises 150 cells above it, which is how
// three trees shipped 26%, 43% and 83% buried. A y in the file would be a
// number the loader ignores, and a number the loader ignores is one an author
// will eventually spend an afternoon tuning.
//
// Format, one record per line, `#` to end-of-line is a comment:
//
//     # sprite   x
//     tree_a     145
//     tree_b     187.5
//
// `sprite` names `assets/<sprite>.bmp`. `x` is a world cell column, the
// sprite's bottom-*centre*. Blank lines are fine. Anything else is an error
// with a line number, never a silently skipped row - see below.
struct PropDef {
    std::string sprite; // asset stem; resolves to assets/<sprite>.bmp
    float x = 0.0f;     // world cell, the sprite's bottom-centre column
    int line = 0;       // 1-based source line, for diagnostics
};

// Parses a prop list. Returns the records; on any malformed line the list comes
// back **empty** and `error` (if non-null) holds a message naming the line.
//
// **All-or-nothing rather than skip-the-bad-row, which is the opposite of what
// a tolerant parser would do and is deliberate.** This project has now shipped
// the same bug twice - V2's palette retune made every authored pixel resolve to
// Empty and the game booted blank with all suites green, and V4's first props
// pass placed three trees inside a hill because a missing ground answer fell
// back to a default. Both were silent. A prop list that drops the line it could
// not read is the same failure a third time: the scene renders, it renders
// *wrong*, and nothing says so. `scene/legend.h` states the rule this follows -
// an unmatched value must not be quietly conflated with "nothing here".
//
// A missing file is **not** an error and yields an empty list: a scene with no
// props is a legitimate scene, and it is distinguishable from a broken one by
// `error` being left untouched.
std::vector<PropDef> load_prop_list(const std::string& path, std::string* error = nullptr);

// True if `sprite` is a name this loader will turn into a path. Rejects empty
// names, path separators and `..`, so a prop list can never reach outside
// assets/ - the file is data, and data that names a path is data that can name
// any path. Exposed for the test, which is the only reason it is not static.
bool prop_sprite_name_ok(const std::string& sprite);
