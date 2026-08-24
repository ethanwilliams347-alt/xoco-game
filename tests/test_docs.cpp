// W3 - the doc-truth suite. Asserts the documents' *checkable numeric claims*
// against the sources those numbers come from, so that a number going stale is
// a red `ctest` rather than a thing somebody notices six sessions later.
//
// **Why this exists.** The rule at the top of `CLAUDE.md` is that the reasoning
// is the deliverable, and its stated failure mode is a rule that stopped
// matching the code and kept being believed. Every check below is one of those
// waiting to happen: a number written in prose, whose source of truth is a line
// of code or a file on disk, maintained by discipline alone.
//
// **The scope limit, stated here so it does not creep.** This suite can only
// ever check a claim with a *machine-readable source of truth*. It cannot check
// reasoning, and an attempt to make it do so turns the docs into a format
// rather than an argument. If a claim needs a human to decide whether it is
// still true, it does not belong here.
//
// **Two things deliberately not pinned.**
//  - `notes/handoff_prompt.md`. It is rewritten whole at every session close, so
//    pinning it would fail on the day it stops mentioning a number rather than
//    on the day a number goes wrong. Its stale figures are fixed by hand when it
//    is rewritten, which is the one time anybody is reading it.
//  - Historical records - `PLAYTEST_LOG.md`, `ROADMAP_ARCHIVE.md`, and dated
//    result lines inside `ROADMAP.md` entries. A number in those is a record of
//    what was measured then; it is *correct as written* even after the world
//    moves, and pinning it would force the archive to be edited, which is
//    exactly what the archive promises never to need.
//
// Run from the repo root: the CMake entry sets WORKING_DIRECTORY, the same way
// `scene_test` does.

#include <cctype>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "physics/element.h"
#include "test_util.h"

namespace {

// The first record of a scene list: its first line that is neither blank nor a
// comment, whitespace-split. Deliberately not a second copy of
// `scene_list::load_scene_list` - this suite links nothing, on purpose, so that
// a doc claim and the code it describes cannot both be wrong in the same way.
//
// **The whole row rather than just the name (2026-08-23).** **The whole row rather
// than just the name (2026-08-23)**, because the launch-check line it feeds
// depends on more than the name: a scene that names no maps prints `0x0, 0
// cells placed` and is not a failure, and a version of this that read only the
// name asserted the fixture's count against whatever row happened to be first.
std::vector<std::string> first_scene_fields(const std::string& path) {
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        const size_t hash = line.find('#');
        if (hash != std::string::npos) line.erase(hash);
        std::istringstream fields(line);
        std::vector<std::string> row;
        std::string field;
        while (fields >> field) row.push_back(field);
        if (!row.empty()) return row;
    }
    return {};
}

std::string read_file(const std::string& path) {
    // Binary, on purpose. All three roadmap files are CRLF on disk and two of
    // the sources are LF; a text-mode read on one platform and not another is
    // how an exact-match compare silently stops matching. Needles below are
    // therefore written without embedded newlines.
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// Prose is wrapped at 80 columns (`W1`), so any needle long enough to be
// unambiguous is long enough to straddle a line break - and the break moves
// whenever a word earlier in the paragraph is edited. Comparing on
// whitespace-normalised text is what makes a claim about a *sentence* checkable
// without the document having to be written around the checker. It also makes
// the CRLF/LF split a non-issue, which has cost this project two sessions.
std::string normalise_ws(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    bool in_space = false;
    for (const char c : text) {
        const bool space = (c == ' ' || c == '\t' || c == '\r' || c == '\n');
        if (space) {
            in_space = true;
        } else {
            if (in_space && !out.empty()) out += ' ';
            in_space = false;
            out += c;
        }
    }
    return out;
}

// Read a document as one whitespace-normalised line, for claim matching.
std::string read_prose(const std::string& path) {
    return normalise_ws(read_file(path));
}

bool contains(const std::string& hay, const std::string& needle) {
    return hay.find(needle) != std::string::npos;
}

int count_occurrences(const std::string& hay, const std::string& needle) {
    if (needle.empty()) return 0;
    int n = 0;
    for (size_t at = hay.find(needle); at != std::string::npos;
         at = hay.find(needle, at + needle.size()))
        n++;
    return n;
}

// The number words the docs actually use. Deliberately a short table rather
// than a general spell-out: if a count grows past this, the doc sentence is
// being rewritten anyway and this table is the reminder to look at it.
std::string number_word(int n) {
    static const char* words[] = {"zero",     "one",     "two",      "three",
                                  "four",     "five",    "six",      "seven",
                                  "eight",    "nine",    "ten",      "eleven",
                                  "twelve",   "thirteen", "fourteen", "fifteen",
                                  "sixteen",  "seventeen", "eighteen", "nineteen",
                                  "twenty"};
    if (n < 0 || n > 20) return "<out of table>";
    return words[n];
}

// Every doc claim is a file plus a needle built out of a measured value, so the
// needle stops matching the moment the measurement moves.
struct Claim {
    const char* file;
    std::string needle;
};

void check_claims(const char* name, const std::vector<Claim>& claims) {
    std::string missing;
    for (const Claim& c : claims) {
        const std::string text = read_prose(c.file);
        if (text.empty()) {
            missing += std::string("\n    ") + c.file + " could not be read";
        } else if (!contains(text, c.needle)) {
            missing += std::string("\n    ") + c.file + " no longer says: " + c.needle;
        }
    }
    check(name, missing.empty(), missing);
}

// ---------------------------------------------------------------------------
// Sources of truth
// ---------------------------------------------------------------------------

// The suite count, from the only place that decides it. `grep -c add_test` is
// the wrong instrument and this project has already been bitten by it: six of
// the twenty `add_test` mentions in `CMakeLists.txt` are comments explaining
// why `grid_bench` and the probes are deliberately *not* registered, so the
// grep returns 20 against a true 15. Count the registrations, not the word.
int registered_suites(const std::string& cmake) {
    return count_occurrences(cmake, "add_test(NAME ");
}

// Exactly one suite links SDL, and `CLAUDE.md` states the headless count as a
// separate number. Derive it rather than writing it down twice.
int sdl_linked_suites(const std::string& cmake) {
    return count_occurrences(cmake, "_test PRIVATE SDL2-static");
}

// Pull an integer out of a source line of the form `... = <digits>...`.
// Returns -1 when the anchor is gone, which fails the check rather than
// silently comparing against zero.
long long int_after(const std::string& text, const std::string& anchor) {
    const size_t at = text.find(anchor);
    if (at == std::string::npos) return -1;
    size_t i = at + anchor.size();
    while (i < text.size() && (text[i] == ' ' || text[i] == '=')) i++;
    std::string digits;
    while (i < text.size() && text[i] >= '0' && text[i] <= '9') digits += text[i++];
    return digits.empty() ? -1 : std::stoll(digits);
}

// The golden checksum as written in the test that owns it, hex digits only.
std::string golden_hex(const std::string& src) {
    const std::string anchor = "constexpr uint64_t GOLDEN = 0x";
    const size_t at = src.find(anchor);
    if (at == std::string::npos) return {};
    size_t i = at + anchor.size();
    std::string hex;
    while (i < src.size() && std::isxdigit(static_cast<unsigned char>(src[i])))
        hex += src[i++];
    return hex;
}

// Checklist steps are the `1. **` … `13. **` list items at the left margin of
// MANUAL_TESTING.md. Counting the markers is the source; the docs spell the
// total in words in three places.
int checklist_steps(const std::string& text) {
    int n = 0;
    size_t line_start = 0;
    while (line_start <= text.size()) {
        const size_t nl = text.find('\n', line_start);
        const std::string line = text.substr(
            line_start, (nl == std::string::npos ? text.size() : nl) - line_start);
        size_t i = 0;
        while (i < line.size() && line[i] >= '0' && line[i] <= '9') i++;
        if (i > 0 && line.compare(i, 4, ". **") == 0) n++;
        if (nl == std::string::npos) break;
        line_start = nl + 1;
    }
    return n;
}

// A claimed size in KB, read out of the doc that states it: the first
// `<digits> KB` at or after `anchor`.
long long claimed_kb(const std::string& text, const std::string& anchor) {
    size_t at = text.find(anchor);
    if (at == std::string::npos) return -1;
    while (true) {
        at = text.find(" KB", at);
        if (at == std::string::npos) return -1;
        size_t end = at;
        size_t begin = end;
        while (begin > 0 && text[begin - 1] >= '0' && text[begin - 1] <= '9') begin--;
        if (begin < end) return std::stoll(text.substr(begin, end - begin));
        at += 3;
    }
}

long long file_size(const std::string& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    return in ? static_cast<long long>(in.tellg()) : -1;
}

} // namespace

int main() {
    const std::string cmake       = read_file("CMakeLists.txt");
    const std::string golden_src  = read_file("tests/test_golden_frame.cpp");
    const std::string scene_src   = read_file("tests/test_scene.cpp");
    const std::string manual      = read_file("MANUAL_TESTING.md");
    const std::string rules_docs  = read_prose(".claude/rules/documentation.md");

    check("the sources this suite reads are all present",
          !cmake.empty() && !golden_src.empty() && !scene_src.empty() &&
              !manual.empty() && !rules_docs.empty(),
          "run from the repo root - the CMake entry sets WORKING_DIRECTORY");

    // 1. The suite count. The claim the external review picked as the most
    //    falsifiable one in the corpus, which is a good reason for it to be the
    //    first thing here.
    const int suites  = registered_suites(cmake);
    const int with_sdl = sdl_linked_suites(cmake);
    const int headless = suites - with_sdl;
    check("this suite is registered, so the count includes itself", suites >= 15,
          std::to_string(suites) + " registered");
    check("exactly one suite links SDL", with_sdl == 1,
          std::to_string(with_sdl) + " link SDL2-static");
    check_claims(
        "the suite count in the docs matches CMakeLists.txt",
        {
            {"CLAUDE.md", "The full suite is " + std::to_string(suites) + " suites"},
            {"CLAUDE.md", number_word(headless) + " are headless"},
            {"README.md", number_word(suites) + " suites"},
        });

    // 2. `Element`'s size and its three free bytes. Measured, not counted -
    //    this struct's byte count has been got wrong twice by counting fields,
    //    which is why `element.h` asserts it and why the doc claim is pinned to
    //    `sizeof` rather than to that comment.
    const size_t element_size = sizeof(Element);
    const size_t free_bytes   = offsetof(Element, color) - offsetof(Element, type) -
                              sizeof(Element{}.type);
    check("Element is the size the invariant says", element_size == 12,
          std::to_string(element_size) + " bytes");
    check("the front hole is three bytes at offsets 1-3", free_bytes == 3,
          std::to_string(free_bytes) + " free, color at " +
              std::to_string(offsetof(Element, color)));
    check_claims("the Element claims in the docs match the struct",
                 {
                     {"CLAUDE.md", "`Element` is " + std::to_string(element_size) +
                                       " bytes and has exactly " +
                                       number_word(static_cast<int>(free_bytes)) +
                                       // Split before the '3' so the hex escape
                                       // for the en dash cannot swallow it.
                                       " free ones, at offsets 1\xE2\x80\x93" "3."},
                     {"ENGINEERING_NOTES.md", "`Element` is **" +
                                                  std::to_string(element_size) +
                                                  " bytes**"},
                 });

    // 3. The golden checksum quoted in prose. `V23b` is the reason this is
    //    worth a check: the frame went back to its pre-V23 value and the
    //    evidence that the revert was complete is that number appearing in two
    //    documents and one test. Three copies is three chances to drift.
    const std::string golden = golden_hex(golden_src);
    check("the golden checksum can be read out of its test", golden.size() == 16,
          "0x" + golden);
    check_claims("the golden checksum quoted in prose is the current one",
                 {
                     {"TUNING.md", "0x" + golden},
                     {"ROADMAP.md", "0x" + golden},
                 });

    // 4. `FIXTURE_SCENE_CELLS`. It is the launch check - the number a tester
    //    reads off stdout to know the world is not blank - so a stale copy in
    //    MANUAL_TESTING.md would have the tester passing a scene that failed.
    const long long cells = int_after(scene_src, "constexpr int FIXTURE_SCENE_CELLS");
    check("FIXTURE_SCENE_CELLS can be read out of scene_test", cells > 0,
          std::to_string(cells));
    // **The line gained the scene's name on 2026-08-23**, when the location
    // became a row in `assets/scenes.txt` instead of two literals in
    // `main.cpp`. The name is read out of that file rather than written here,
    // for this suite's own reason: a claim is only checkable against a
    // machine-readable source, and "the first scene is called fixture" has one.
    const std::vector<std::string> first_scene = first_scene_fields("assets/scenes.txt");
    check("the first scene's row can be read out of assets/scenes.txt",
          first_scene.size() >= 5, std::to_string(first_scene.size()) + " fields");
    // **The expected line is built from what that row actually declares**, not
    // from an assumption that it is the fixture. The `fixture` row was archived
    // on 2026-08-23 - commented out in `assets/scenes.txt`, restorable in one
    // line - and this check went on asserting the fixture's 1920x1080 and its
    // cell count against a row that names no maps at all. A scene declaring `-`
    // for material and albedo stamps nothing, so `main.cpp` prints the default
    // `Scene{}` extent and a count of zero, and that is the line the tester has
    // to be told to expect.
    const bool first_is_empty =
        first_scene.size() >= 3 && first_scene[1] == "-" && first_scene[2] == "-";
    const std::string scene_line =
        first_scene.empty()
            ? std::string()
            : "Scene: " + first_scene[0] +
                  (first_is_empty ? ", 0x0, 0 cells placed"
                                  : ", 1920x1080, " + std::to_string(cells) + " cells placed");
    check_claims("the launch-check line matches the first shipped scene",
                 {
                     {"MANUAL_TESTING.md", scene_line},
                     {"ASSETS.md", scene_line},
                 });

    // 5. The Manual Tester Checklist's length. Three documents spell it in
    //    words, and it has changed twice in two days (step 13 arrived with V23
    //    and changed meaning with V23b).
    const int steps = checklist_steps(manual);
    check("the checklist has steps to count", steps > 0, std::to_string(steps));
    check_claims("the checklist length in the docs matches MANUAL_TESTING.md",
                 {
                     {"CLAUDE.md", "**" + number_word(steps) + "** steps"},
                     {"ROADMAP.md", "**" + number_word(steps) + " steps as of"},
                     {".claude/rules/documentation.md",
                      "**" + number_word(steps) + " steps"},
                 });

    // 6. The two sizes `W4` created. New numbers are the ones most likely to
    //    drift, and these two drift on every commit that touches the plan.
    //
    //    **The tolerance is the point, not a weakness.** These are quoted
    //    rounded, and "KB" in the docs is not consistently binary or decimal -
    //    350,147 bytes is 342 KiB or 350 kB and the doc says 346. Asserting the
    //    exact figure would mean editing prose on every roadmap edit, which is
    //    the kind of check people delete. Ten percent catches the failure that
    //    matters: the file quietly doubling, or the archive being cited at a
    //    size it stopped having.
    struct SizeClaim { const char* path; const char* anchor; const char* what; };
    const SizeClaim size_claims[] = {
        {"ROADMAP.md", "**ROADMAP.md** \xE2\x80\x94 the one live plan",
         "the live plan's stated size"},
        {"ROADMAP_ARCHIVE.md", "against a ", "the archive's stated size"},
    };
    for (const SizeClaim& sc : size_claims) {
        const long long actual_kib = file_size(sc.path) / 1024;
        const long long stated     = claimed_kb(rules_docs, sc.anchor);
        const bool ok = stated > 0 && actual_kib > 0 &&
                        std::llabs(stated - actual_kib) * 10 <= stated;
        check(sc.what, ok,
              std::to_string(stated) + " KB stated in .claude/rules/documentation.md, " +
                  std::to_string(actual_kib) + " KiB on disk");
    }

    // 7. `ROADMAP_ITEMS.md` is deleted, so a *live link* to it is a broken
    //    link. Prose citing it inside a dated historical entry is correct as
    //    written and is not what this looks for - only the Markdown link form.
    {
        const char* docs[] = {"CLAUDE.md",          "README.md",
                              "ROADMAP.md",         "ENGINEERING_NOTES.md",
                              "MANUAL_TESTING.md",  "TUNING.md",
                              "PERFORMANCE.md",     "VISION.md",
                              "ASSETS.md",          ".claude/rules/documentation.md"};
        std::string offenders;
        for (const char* d : docs) {
            const std::string text = read_file(d);
            if (contains(text, "](ROADMAP_ITEMS.md"))
                offenders += std::string("\n    ") + d;
        }
        check("no live document links to the deleted ROADMAP_ITEMS.md",
              offenders.empty(), offenders);
    }

    return report();
}
