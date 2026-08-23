#pragma once
#include <string>
#include <vector>

// Which scenes exist, and what each one is made of.
//
// **Why a list and not a rebuild.** Until now the location was three literals in
// `main.cpp` - `test_material.bmp`, `test_albedo.bmp`, `test_props.txt` - which
// is fine for one location and stops being fine the moment there are two. The
// first second scene is an *empty* one, asked for so the backdrop can be looked
// at with nothing standing in front of it, and it is the case that makes the
// shape of this file obvious: **an empty scene is a scene with no image, not an
// image of nothing.** Two grid-sized BMPs of pure black would be 12.4 MB
// carrying no information at all, which is exactly what `props.h` refuses one
// paragraph into its own header. So a scene names its files, and a scene that
// names none is empty by construction.
//
// Format, one record per line, `#` to end-of-line is a comment:
//
//     # name     material           albedo            props           spawn
//     fixture    test_material.bmp  test_albedo.bmp   test_props.txt  terrain
//     empty      -                  -                 -               floor
//
// Five fields, all required, `-` meaning "none" for the three filenames. Blank
// lines are fine. Anything else is an error with a line number, never a
// silently skipped row.
//
// **Every field is read, which is `props.h`'s rule 2 and the reason there is no
// `seed` column and no `x`/`y` spawn column.** A number the loader ignores is
// one an author eventually spends an afternoon tuning. `spawn` is two words
// rather than a coordinate for the same reason the prop format has no `y`: the
// ground is not one number, so the spawn row is *scanned* rather than authored,
// and all the file has to say is which scan to run.
namespace scene_list {

// Where the body starts, once the scene is stamped.
enum class Spawn {
    // Stand on the terrain under the spawn column - `boot::stand_player_on_ground`.
    // A scene with no terrain under that column leaves the body where it was and
    // the caller warns, which is that function's existing refusal.
    Terrain,
    // Put the body on the world's bottom border. **This is not a fallback for
    // Terrain and must not be made into one**: it exists for a scene that is
    // *meant* to be empty, and using it when a terrain scan merely failed would
    // turn a broken scene into a playable-looking one, which is the conflation
    // `scene/legend.h` exists to prevent.
    Floor,
};

struct SceneDef {
    std::string name;       // the scene's own name; also what the HUD shows
    std::string material;   // asset stem or empty; resolves to assets/<stem>
    std::string albedo;     // asset stem or empty
    std::string props;      // asset stem or empty
    Spawn spawn = Spawn::Terrain;
    int line = 0;           // 1-based source line, for diagnostics

    // A scene that names no material map places no cells. Stated as a question
    // about the *declaration* rather than about the result, because "declared
    // empty" and "stamped nothing" have to stay distinguishable - a legend that
    // matched no colour also places no cells, and that is a defect. `main.cpp`'s
    // startup warning keys on exactly this.
    bool declared_empty() const { return material.empty() && albedo.empty(); }
};

// Parses a scene list. Returns the records; on any malformed line the list comes
// back **empty** and `error` (if non-null) holds a message naming the line.
//
// All-or-nothing, for `load_prop_list`'s reason, which is not repeated here
// beyond its conclusion: a list that drops the row it could not read produces a
// world that loads, loads *wrong*, and says nothing.
//
// A missing file is **not** an error and yields an empty list. The caller is
// expected to fall back to its own built-in default, so that shipping this file
// is additive and deleting it cannot stop the game booting.
//
// Two conditions beyond a malformed line are also errors, because both are
// silent otherwise: a duplicate name (which makes "switch to X" ambiguous) and
// an empty list from a file that existed but held only comments.
std::vector<SceneDef> load_scene_list(const std::string& path, std::string* error = nullptr);

// True if `name` is something this loader will turn into a path or a scene name.
// Rejects empty names, path separators and `..`, so a scene list can never reach
// outside assets/ - the file is data, and data that names a path is data that
// can name any path. Exposed for the test, which is the only reason it is not
// static.
bool scene_name_ok(const std::string& name);

// The list `main.cpp` uses when `assets/scenes.txt` is absent: exactly the
// location the game shipped with before this file existed, and nothing else.
//
// **It lives here rather than in `main.cpp` so the fallback is testable**, and
// so that "the file is missing" and "the file lists one scene" produce provably
// the same world rather than two things that look the same.
std::vector<SceneDef> default_scene_list();

} // namespace scene_list
