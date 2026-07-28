#include "grid.h"
#include <algorithm>

namespace {
    int clamp_channel(int v) {
        return v < 0 ? 0 : (v > 255 ? 255 : v);
    }
}

Grid::Grid(int width, int height) : width(width), height(height) {
    cells.resize(width * height, Element{});
    pixels.resize(width * height, material_of(ElementType::Empty).color);

    chunks_x = (width + CHUNK_SIZE - 1) / CHUNK_SIZE;
    chunks_y = (height + CHUNK_SIZE - 1) / CHUNK_SIZE;
    chunk_current.resize(chunks_x * chunks_y);
    chunk_next.resize(chunks_x * chunks_y);
    // Both start empty: a world of nothing but Empty has nothing to simulate.

    std::random_device rd;
    rng.seed(rd());
}

// Waking only the cell that changed is the classic dirty-rect bug. Erase a grain
// from under a settled pile and the grains above it are still asleep, so the pile
// hangs in the air over the hole. Every write therefore wakes its 3x3
// neighbourhood, and because the neighbourhood is resolved per cell it crosses
// chunk borders correctly - otherwise the same bug reappears as seams along the
// invisible chunk lines.
void Grid::mark_dirty(int x, int y) {
    for (int ny = y - 1; ny <= y + 1; ++ny) {
        for (int nx = x - 1; nx <= x + 1; ++nx) {
            if (!is_within_bounds(nx, ny)) continue;
            const int ci = (ny / CHUNK_SIZE) * chunks_x + (nx / CHUNK_SIZE);
            chunk_next[ci].include(nx, ny);
        }
    }
}

int Grid::active_chunk_count() const {
    int n = 0;
    for (const DirtyRect& r : chunk_next) {
        if (!r.is_empty()) n++;
    }
    return n;
}

bool Grid::is_within_bounds(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

int Grid::get_index(int x, int y) const {
    return y * width + x;
}

Element Grid::get_element(int x, int y) const {
    // Out of bounds reads as solid so the world is sealed by its own border.
    if (!is_within_bounds(x, y)) return Element{ElementType::Wall, material_of(ElementType::Wall).color, 0};
    return cells[get_index(x, y)];
}

uint32_t Grid::jittered_color(const Material& mat) {
    if (mat.color_jitter == 0) return mat.color;

    const int j = static_cast<int>(mat.color_jitter);
    const int delta = static_cast<int>(rng() % (2 * j + 1)) - j;

    const int r = clamp_channel(static_cast<int>((mat.color >> 16) & 0xFF) + delta);
    const int g = clamp_channel(static_cast<int>((mat.color >> 8) & 0xFF) + delta);
    const int b = clamp_channel(static_cast<int>(mat.color & 0xFF) + delta);

    return 0xFF000000u | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
}

void Grid::set_element(int x, int y, ElementType type) {
    if (!is_within_bounds(x, y)) return;
    const int idx = get_index(x, y);

    Element el;
    el.type = type;
    el.color = jittered_color(material_of(type));
    el.updated_tag = frame_tag; // freshly placed cells wait until the next step to move

    cells[idx] = el;
    pixels[idx] = el.color;
    mark_dirty(x, y);
}

void Grid::swap_elements(int x1, int y1, int x2, int y2) {
    const int idx1 = get_index(x1, y1);
    const int idx2 = get_index(x2, y2);

    std::swap(cells[idx1], cells[idx2]);
    pixels[idx1] = cells[idx1].color;
    pixels[idx2] = cells[idx2].color;

    // Mark as updated so neither cell moves again this step
    cells[idx1].updated_tag = frame_tag;
    cells[idx2].updated_tag = frame_tag;

    // Both ends of the move changed, so both ends and their neighbours have to
    // be awake next step - this is what lets a falling grain keep falling and
    // what pulls the cells above it into motion behind it.
    mark_dirty(x1, y1);
    mark_dirty(x2, y2);
}

bool Grid::can_displace(const Material& mover, int tx, int ty, int dy) const {
    if (!is_within_bounds(tx, ty)) return false;

    const Element& target = cells[get_index(tx, ty)];
    if (target.type == ElementType::Empty) return true;

    const Material& t = material_of(target.type);
    if (t.move == MoveKind::Static) return false;

    // Only swap through another material when gravity would sort them that way,
    // otherwise the two cells would trade places forever.
    if (dy > 0) return mover.density > t.density; // sinking
    if (dy < 0) return mover.density < t.density; // rising
    return false;                                 // sideways: Empty only
}

bool Grid::step_powder(int x, int y, const Material& mat) {
    if (can_displace(mat, x, y + 1, 1)) {
        swap_elements(x, y, x, y + 1);
        return true;
    }

    const int dir = (rng() % 2 == 0) ? -1 : 1;
    if (can_displace(mat, x + dir, y + 1, 1)) {
        swap_elements(x, y, x + dir, y + 1);
        return true;
    }
    if (can_displace(mat, x - dir, y + 1, 1)) {
        swap_elements(x, y, x - dir, y + 1);
        return true;
    }
    return false;
}

// dy is +1 for liquids (settle downwards) and -1 for gases (rise).
bool Grid::step_fluid(int x, int y, const Material& mat, int dy) {
    if (can_displace(mat, x, y + dy, dy)) {
        swap_elements(x, y, x, y + dy);
        return true;
    }

    const int dir = (rng() % 2 == 0) ? -1 : 1;
    if (can_displace(mat, x + dir, y + dy, dy)) {
        swap_elements(x, y, x + dir, y + dy);
        return true;
    }
    if (can_displace(mat, x - dir, y + dy, dy)) {
        swap_elements(x, y, x - dir, y + dy);
        return true;
    }

    // Blocked vertically, so flow sideways to find a level. Travelling several
    // cells per step is what makes a pool settle quickly instead of oozing.
    for (const int d : {dir, -dir}) {
        int cx = x;
        for (int i = 0; i < mat.spread; ++i) {
            const int nx = cx + d;
            if (!is_within_bounds(nx, y)) break;
            if (cells[get_index(nx, y)].type != ElementType::Empty) break;
            cx = nx;
        }
        if (cx != x) {
            swap_elements(x, y, cx, y);
            return true;
        }
    }
    return false;
}

void Grid::step_cell(int x, int y) {
    Element& current = cells[get_index(x, y)];
    if (current.type == ElementType::Empty || current.updated_tag == frame_tag) return;

    // Claim the cell for this step whether or not it ends up moving, so it is
    // never visited twice in one sweep.
    current.updated_tag = frame_tag;

    const Material& mat = material_of(current.type);
    switch (mat.move) {
        case MoveKind::Static: break;
        case MoveKind::Powder: step_powder(x, y, mat); break;
        case MoveKind::Liquid: step_fluid(x, y, mat, 1); break;
        case MoveKind::Gas:    step_fluid(x, y, mat, -1); break;
    }
}

void Grid::update() {
    ++frame_tag;

    // Take the work that was accumulated during the previous step and start a
    // fresh set of bounds for the work this step generates.
    chunk_current.swap(chunk_next);
    for (DirtyRect& r : chunk_next) r.clear();

    // Chunk rows are walked bottom to top, and so are the cell rows inside them,
    // because a falling cell has to land in rows that have already settled. The
    // world is still swept row by row rather than chunk by chunk - processing a
    // whole chunk at a time would let material fall further on one side of a
    // chunk border than the other and produce visible seams.
    for (int cy = chunks_y - 1; cy >= 0; --cy) {
        // Union of the dirty bounds across this chunk row, so a row of sleeping
        // chunks costs one pass over chunks_x rects instead of CHUNK_SIZE passes
        // over the full world width.
        int row_min_y = 0, row_max_y = -1;
        for (int cx = 0; cx < chunks_x; ++cx) {
            const DirtyRect& r = chunk_current[cy * chunks_x + cx];
            if (r.is_empty()) continue;
            if (row_max_y < row_min_y) {
                row_min_y = r.min_y;
                row_max_y = r.max_y;
            } else {
                row_min_y = std::min(row_min_y, r.min_y);
                row_max_y = std::max(row_max_y, r.max_y);
            }
        }
        if (row_max_y < row_min_y) continue; // the whole chunk row is asleep

        for (int y = row_max_y; y >= row_min_y; --y) {
            // Alternate the sweep direction to stop piles leaning one way.
            const bool leftward = (rng() % 2 == 0);

            for (int i = 0; i < chunks_x; ++i) {
                const int cx = leftward ? (chunks_x - 1 - i) : i;
                const DirtyRect& r = chunk_current[cy * chunks_x + cx];
                if (r.is_empty() || y < r.min_y || y > r.max_y) continue;

                const int start_x = leftward ? r.max_x : r.min_x;
                const int end_x = leftward ? r.min_x - 1 : r.max_x + 1;
                const int step = leftward ? -1 : 1;

                for (int x = start_x; x != end_x; x += step) {
                    step_cell(x, y);
                }
            }
        }
    }
}
