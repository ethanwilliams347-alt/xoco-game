#pragma once
#include <string>
#include <vector>

// Which BMP a piece of art actually loads from, as data rather than as a string
// literal in main.cpp.
//
// The problem this solves is the one README.md warns about at the top of its
// assets section: every asset path used to be a hardcoded literal, so swapping a
// sheet meant editing C++, and `assets/` is copied next to the exe at build
// time, so it also meant a rebuild. Between them, "look at this drawing in the
// game" was a code change. Now a BMP dropped into assets/ and named here is
// picked up on the next launch, and tools/load_sprite.py is the command that
// writes the entry and stages the file.
//
// **This is a rebinding table, not a loader.** It maps a stable key that code
// refers to onto a filename that art owns. Nothing here opens a BMP, knows what
// SDL is, or decides what a frame means - a sheet's animation table still lives
// in tools/player_sheet.py and its generated header, because a row count is a
// fact about the *game* and only incidentally a fact about the image.
//
// The one thing the frame size below is for is catching the mismatch early. A
// sheet bound to a key whose frames are a different size still loads and still
// draws; it just draws the wrong rectangles, and it does it silently. Carrying
// the expected frame size in the record lets the caller say so at load instead.
//
// Absent or unlisted is never an error. Every lookup takes the default the code
// would have used anyway, so deleting this file gets you the shipped art rather
// than a black screen - the same reasoning as props.h's "absent is not
// malformed".
struct SpriteBinding {
    std::string key;    // what code asks for, e.g. "player_sheet"
    std::string file;   // a bare filename inside assets/, e.g. "my_sheet.bmp"

    // Frame size in cells for a sheet, or 0 for a plain single image. Advisory:
    // the caller checks it, the loader only carries it.
    int frame_w = 0;
    int frame_h = 0;
};

class SpriteManifest {
public:
    // `assets/<file>` for `key`, or `assets/<fallback_file>` if the key is not
    // listed. The fallback is what makes an unlisted key harmless.
    std::string path_for(const std::string& key, const std::string& fallback_file) const;

    // Null when the key is not listed. For callers that want to check a frame
    // size before drawing with it.
    const SpriteBinding* find(const std::string& key) const;

    const std::vector<SpriteBinding>& bindings() const { return bindings_; }

    std::vector<SpriteBinding> bindings_;
};

// Parses the manifest. One record per line, `#` starts a comment:
//
//     <key> <file.bmp> [<frame_w> <frame_h>]
//
// A missing file yields an empty manifest and leaves `error` untouched, which is
// how a caller tells "no manifest" apart from "a manifest that is wrong".
SpriteManifest load_sprite_manifest(const std::string& path, std::string* error);

// Same rules as prop sprite names: no path separators, no `..`. A manifest is
// data, and data that names a path is data that can name any path.
bool sprite_file_name_ok(const std::string& file);
