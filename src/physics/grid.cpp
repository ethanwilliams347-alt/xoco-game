#include "grid.h"
#include "reaction.h"
#include <algorithm>

namespace {
    int clamp_channel(int v) {
        return v < 0 ? 0 : (v > 255 ? 255 : v);
    }
}

Grid::Grid(int width, int height, uint64_t seed) : width(width), height(height), world_seed(seed) {
    cells.resize(width * height, Element{});
    pixels.resize(width * height, material_of(ElementType::Empty).color);

    chunks_x = (width + CHUNK_SIZE - 1) / CHUNK_SIZE;
    chunks_y = (height + CHUNK_SIZE - 1) / CHUNK_SIZE;
    chunk_current.resize(chunks_x * chunks_y);
    chunk_next.resize(chunks_x * chunks_y);
    // Both start empty: a world of nothing but Empty has nothing to simulate.

    support_visit.resize(width * height, 0);
    support_state.resize(width * height, 0);

    // Nothing to seed. The seed is stored and read straight out of world_seed by
    // the hash in random.h, so the whole 64 bits reach the work by construction -
    // where std::mt19937 needed a seed_seq to stop its 32-bit seeding from
    // silently discarding the top half. The test for that (F1.1) still stands and
    // still passes; it now checks a property the code cannot lose.
}

void Grid::reset(uint64_t seed) {
    world_seed = seed;

    std::fill(cells.begin(), cells.end(), Element{});
    std::fill(pixels.begin(), pixels.end(), material_of(ElementType::Empty).color);

    // Both go back to empty, same as a fresh grid's - see the comment on these
    // two members for why there are two rather than one.
    std::fill(chunk_current.begin(), chunk_current.end(), DirtyRect{});
    std::fill(chunk_next.begin(), chunk_next.end(), DirtyRect{});

    pending_support.clear();
    support_stack.clear();
    support_component.clear();
    std::fill(support_visit.begin(), support_visit.end(), 0);
    std::fill(support_state.begin(), support_state.end(), 0);
    support_epoch = 0;
    resolving_support = false;

    frame_tag = 0;
    step_count = 0;

    // width, height, chunks_x and chunks_y are not here on purpose: the vectors
    // above are cleared in place rather than resized, which is only correct as
    // long as the grid's dimensions never change out from under them. Nothing
    // in this class exposes a way to change them after construction, and reset()
    // must not become the first.
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

// The shade is a function of the cell's position and nothing else, so a cell
// erased and repainted in the same spot comes back the same colour instead of a
// new one. That is a deliberate change of behaviour, not a side effect: a
// one-time authored value has no business moving every time the world is
// touched, and pinning it to the spot means the look of a scene is reproducible
// from the seed alone. If it ever reads as too regular under a slow brush, mix
// a placement counter into the index rather than reaching back for a generator.
//
// Cells carry their colour when they move, because swap_elements moves the whole
// Element - so a falling grain keeps the shade it was painted with rather than
// flickering to whatever its new index would hash to.
uint32_t Grid::jittered_color(const Material& mat, uint64_t index) const {
    if (mat.color_jitter == 0) return mat.color;

    const int j = static_cast<int>(mat.color_jitter);
    const int delta = authored_spread(j, index, sim_random::Stream::ColorJitter);

    const int r = clamp_channel(static_cast<int>((mat.color >> 16) & 0xFF) + delta);
    const int g = clamp_channel(static_cast<int>((mat.color >> 8) & 0xFF) + delta);
    const int b = clamp_channel(static_cast<int>(mat.color & 0xFF) + delta);

    return 0xFF000000u | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
}

void Grid::set_element(int x, int y, ElementType type) {
    if (!is_within_bounds(x, y)) return;
    const int idx = get_index(x, y);
    const ElementType old_type = cells[idx].type;

    Element el;
    el.type = type;
    el.color = jittered_color(material_of(type), static_cast<uint64_t>(idx));
    el.updated_tag = frame_tag; // freshly placed cells wait until the next step to move

    cells[idx] = el;
    pixels[idx] = el.color;
    mark_dirty(x, y);

    // Structure was removed here, so anything that was leaning on it has to be
    // re-examined. Removal only: placing a new structure cell does not trigger
    // a check, which is what lets the brush draw a floating platform on purpose
    // without it immediately falling apart.
    if (!resolving_support && is_structural(old_type) && !is_structural(type)) {
        for (int ny = y - 1; ny <= y + 1; ++ny)
            for (int nx = x - 1; nx <= x + 1; ++nx)
                queue_support_check(nx, ny);
    }
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

    // The other way structure loses its footing: nothing was removed, but the
    // sand that was holding a slab up just slid out from under it. Only the
    // cell directly above each end can have been standing on what moved.
    //
    // This is the hottest path in the engine, so the check was expected to be
    // expensive and was written to be cheap. A bracketed A/B (hook in, out, in)
    // cannot measure it at all -- see the benchmark table in ROADMAP.md, which
    // also records the earlier 5% figure this replaced and why that was wrong.
    // An early-out that skipped ends still holding something solid was tried and
    // measured as noise too, so it was removed rather than kept on the theory
    // that it ought to help.
    if (!resolving_support) {
        queue_support_check(x1, y1 - 1);
        queue_support_check(x2, y2 - 1);
    }
}

bool Grid::is_grounded(int x, int y) const {
    const int below_y = y + 1;

    // The bottom of the world holds everything up. Note that the side borders
    // deliberately do not: a shelf bolted to the left edge with nothing beneath
    // it is unsupported, same as anywhere else.
    if (below_y >= height) return true;

    const ElementType below = cells[get_index(x, below_y)].type;

    // More of the same structure is not support -- whether *it* is held up is
    // the question the flood fill is already answering.
    if (is_structural(below)) return false;

    return is_solid(below);
}

void Grid::queue_support_check(int x, int y) {
    if (!is_within_bounds(x, y)) return;
    const int idx = get_index(x, y);
    if (!is_structural(cells[idx].type)) return;
    pending_support.push_back(idx);
}

int Grid::fall_speed(uint8_t ticks) {
    const int s = 1 + static_cast<int>(ticks) / TICKS_PER_SPEEDUP;
    return s < MAX_FALL_SPEED ? s : MAX_FALL_SPEED;
}

void Grid::resolve_support() {
    if (pending_support.empty()) return;

    resolving_support = true;

    std::vector<int> seeds;
    std::vector<int> deferred; // still falling, but not fast enough for this pass

    // One pass per cell of travel. Everything queued gets the first pass; each
    // pass after that is only for the pieces that have been in the air long
    // enough to have earned it. Re-running the whole question rather than
    // taking a longer stride is what keeps a piece at speed 8 from stepping
    // over a floor one cell thick.
    for (int pass = 0; pass < MAX_FALL_SPEED && !pending_support.empty(); ++pass) {
        // Taken by value: a piece that falls re-queues itself into
        // pending_support, and that must not extend the loop running now, or
        // one piece would fall the whole way down inside a single pass.
        seeds.clear();
        seeds.swap(pending_support);

        // A fresh epoch per pass. Every cell of travel is a new question about
        // a world that has just changed, so last pass's verdicts must not be
        // read as answers to this one.
        if (++support_epoch == 0) { // wrapped, so old marks can no longer be told apart
            std::fill(support_visit.begin(), support_visit.end(), uint8_t{0});
            support_epoch = 1;
        }

        for (const int seed : seeds) {
            if (!is_structural(cells[seed].type)) continue;
            if (support_visit[seed] == support_epoch) continue; // its piece already moved this pass

            // Too slow for this pass. It keeps its place in the queue so the
            // next step picks it up again; it just does not travel any further
            // this one.
            if (fall_speed(cells[seed].fall_ticks) <= pass) {
                deferred.push_back(seed);
                continue;
            }

            const int y = seed / width;
            fall_if_unsupported(seed - y * width, y);
        }
    }

    for (const int idx : deferred) pending_support.push_back(idx);

    // Whatever is still queued moved at some point during this step, so it is
    // in the air and has now been for one step longer. A tick is a step, not a
    // cell: speed has to follow time in the air, or a piece would speed up
    // because it was already fast. Pieces that came to rest are not here -
    // settle_marks() put them back to zero and stopped re-queueing them.
    for (const int idx : pending_support) {
        if (cells[idx].fall_ticks < 255) cells[idx].fall_ticks++;
    }

    resolving_support = false;
}

void Grid::settle_marks(SupportState state, int extra) {
    const uint8_t s = static_cast<uint8_t>(state);

    // Settling on Supported is the only way a piece ever stops falling, so it
    // is also where the clock goes back to zero. Without that, a slab that fell
    // a long way and landed would keep the speed it landed at, and digging it
    // free a minute later would have it leave at full pelt instead of tipping
    // off the ledge.
    //
    // Adopting a neighbour's Moved verdict lands here too, and zeroes a piece
    // that is genuinely still falling. That costs it its run-up, which is a
    // visible stutter but a rare one - it needs two separate pieces to touch
    // mid-fall - and erring towards slower is the same direction every other
    // guess in this file errs in.
    const bool at_rest = (state == SupportState::Supported);

    // Every cell this fill marked is either already filed in the component or
    // still waiting on the stack; between them they are the whole marked set.
    const auto settle = [&](int idx) {
        support_state[idx] = s;
        if (at_rest) cells[idx].fall_ticks = 0;
    };

    if (extra >= 0) settle(extra);
    for (const int idx : support_component) settle(idx);
    for (const int idx : support_stack) settle(idx);
}

void Grid::fall_if_unsupported(int x, int y) {
    support_stack.clear();
    support_component.clear();

    const int seed = get_index(x, y);
    support_stack.push_back(seed);
    support_visit[seed] = support_epoch;
    support_state[seed] = static_cast<uint8_t>(SupportState::Pending);

    while (!support_stack.empty()) {
        const int idx = support_stack.back();
        support_stack.pop_back();

        const int cy = idx / width;
        const int cx = idx - cy * width;

        // One grounded cell anywhere is enough to hold the whole structure up.
        // Everything reached on the way here is part of that same piece, so it
        // is held up too -- recording that is what stops a later seed from
        // re-deciding the question with half the piece walled off from it.
        if (is_grounded(cx, cy)) {
            settle_marks(SupportState::Supported, idx);
            return;
        }

        support_component.push_back(idx);
        if (static_cast<int>(support_component.size()) > MAX_SUPPORT_CELLS) {
            settle_marks(SupportState::Supported, -1); // too big to judge: assume held up
            return;
        }

        // Neighbours are pushed in reading order, so the last ones pushed - and
        // therefore the first ones popped off the stack - are the row below.
        // The search runs downhill, which is where the ground is.
        for (int ny = cy - 1; ny <= cy + 1; ++ny) {
            for (int nx = cx - 1; nx <= cx + 1; ++nx) {
                if (nx == cx && ny == cy) continue;
                if (!is_within_bounds(nx, ny)) continue;

                const int nidx = get_index(nx, ny);
                if (!is_structural(cells[nidx].type)) continue;

                if (support_visit[nidx] == support_epoch) {
                    if (static_cast<SupportState>(support_state[nidx]) == SupportState::Pending) {
                        continue; // already in this fill
                    }
                    // Touching a cell an earlier fill already settled. The two
                    // are adjacent, so they are one piece, and its answer is
                    // this piece's answer: Supported means held up, and Moved
                    // means it has already had its turn this pass, so this half
                    // waits for the next one rather than falling on its own.
                    // Either way nothing here moves now.
                    settle_marks(SupportState::Supported, idx);
                    return;
                }

                support_visit[nidx] = support_epoch;
                support_state[nidx] = static_cast<uint8_t>(SupportState::Pending);
                support_stack.push_back(nidx);
            }
        }
    }

    // The whole piece was explored and none of it was standing on anything.
    drop_component();
}

void Grid::drop_component() {
    // Bottom-up within each column, so a cell is only ever moved into space its
    // lower neighbour has already left. Doing this per column also handles a
    // column that contains two separate parts of the same piece -- an arch, say
    // -- without needing to find the runs explicitly.
    std::sort(support_component.begin(), support_component.end(),
              [w = width](int a, int b) {
                  const int ax = a % w, ay = a / w;
                  const int bx = b % w, by = b / w;
                  return ax != bx ? ax < bx : ay > by;
              });

    for (const int idx : support_component) {
        const int cy = idx / width;
        const int cx = idx - cy * width;

        // Always legal. "Unsupported" means no cell of the piece has anything
        // solid under it, so every cell about to move is moving into Empty, into
        // a fluid, or into space another cell of the piece just left. Structural
        // materials are denser than every fluid, so the swap sends whatever was
        // below up to the top of the piece rather than deleting it -- which is
        // also why a slab sinks through water instead of resting on it.
        swap_elements(cx, cy, cx, cy + 1);

        // Mark and re-queue the cell's new home, so the piece is recognised as
        // already-moved for the rest of this pass and gets another look on the
        // next one. This is the only thing that makes it keep falling, and it
        // is what a pass costs: the queue it leaves behind is the input to
        // whichever pass comes next, in this step or the following one.
        //
        // The vacated cell is marked too. Anything of this piece still above it
        // is about to move into it, and until this pass is over no other fill
        // should treat that space as a fresh question.
        const int moved = get_index(cx, cy + 1);
        support_visit[moved] = support_epoch;
        support_state[moved] = static_cast<uint8_t>(SupportState::Moved);
        support_visit[idx] = support_epoch;
        support_state[idx] = static_cast<uint8_t>(SupportState::Moved);
        pending_support.push_back(moved);
    }
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

    const int dir = coin(static_cast<uint64_t>(get_index(x, y)), sim_random::Stream::PowderDirection) ? -1 : 1;
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

    // Its own stream, separate from the powder pick. A cell is only ever one or
    // the other, so the two never collide in practice - but the tag costs one xor
    // and means neither function's behaviour depends on the other's existing.
    const int dir = coin(static_cast<uint64_t>(get_index(x, y)), sim_random::Stream::FluidDirection) ? -1 : 1;
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

bool Grid::has_neighbor(int x, int y, ElementType t) const {
    for (int ny = y - 1; ny <= y + 1; ++ny) {
        for (int nx = x - 1; nx <= x + 1; ++nx) {
            if (nx == x && ny == y) continue;
            if (!is_within_bounds(nx, ny)) continue;
            if (cells[get_index(nx, ny)].type == t) return true;
        }
    }
    return false;
}

bool Grid::try_react(int x, int y) {
    Element& cell = cells[get_index(x, y)];

    // Spontaneous-reaction targets must keep re-checking every frame even if
    // they never move. Fire's own movement only calls mark_dirty when
    // step_fluid actually relocates it; if Fire is fully boxed in (e.g.
    // surrounded by Wood with no Empty cell to rise into), it would generate
    // zero dirty marks after the frame it was created and freeze forever -
    // never decaying, never given another chance to ignite the Wood it is
    // touching. This unconditional self-mark is the fix, and it only applies
    // to Fire (a spontaneous target), not to Wood/Oil, which stay fully
    // sleep-eligible when no fire is nearby.
    for (const Reaction& r : REACTIONS) {
        if (r.catalyst == ElementType::Count && r.target == cell.type) {
            mark_dirty(x, y);
            break;
        }
    }

    for (const Reaction& r : REACTIONS) {
        if (r.target != cell.type) continue;
        if (r.catalyst != ElementType::Count && !has_neighbor(x, y, r.catalyst)) continue;
        // One roll per cell per step: the loop commits to the first eligible row
        // and returns either way, so this never draws twice from the same
        // coordinates - which it would otherwise do, getting the same answer.
        if (chance(static_cast<int>(r.chance_pct), static_cast<uint64_t>(get_index(x, y)),
                   sim_random::Stream::Reaction)) {
            set_element(x, y, r.result);
            return true;
        }
        return false; // first eligible row commits the cell for this frame, win or lose
    }
    return false;
}

void Grid::step_cell(int x, int y) {
    Element& current = cells[get_index(x, y)];
    if (current.type == ElementType::Empty || current.updated_tag == frame_tag) return;

    // Claim the cell for this step whether or not it ends up moving, so it is
    // never visited twice in one sweep.
    current.updated_tag = frame_tag;

    if (try_react(x, y)) return; // converted; let the new material move starting next frame

    const Material& mat = material_of(current.type);
    switch (mat.move) {
        case MoveKind::Static: break;
        case MoveKind::Powder: step_powder(x, y, mat); break;
        case MoveKind::Liquid: step_fluid(x, y, mat, 1); break;
        case MoveKind::Gas:    step_fluid(x, y, mat, -1); break;
    }
}

void Grid::update() {
    // First of all, so that every part of this step agrees on which step it is:
    // the support resolve below, the sweep after it, and the hash that will read
    // this once the generator is gone. It deliberately does not sit next to
    // ++frame_tag further down - that one has to stay below resolve_support(),
    // and the two counters answer different questions.
    ++step_count;

    // Before the sweep and before the chunk swap, so the cells a falling piece
    // vacates land in the bounds that are about to be simulated -- whatever was
    // displaced out from under it starts flowing on this step rather than the
    // next one.
    resolve_support();

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
            //
            // Keyed on the row, and it takes a stream tag anyway. F1.3 in the
            // roadmap says this site "needs no salt" because its input is
            // already a row and a step - but row y and cell index y are the same
            // number, and cell index y is a real cell, so without a tag a row's
            // direction would be drawn from the same value as that cell's own
            // decisions. The tag is free; the collision would not have been.
            const bool leftward = coin(static_cast<uint64_t>(y), sim_random::Stream::SweepDirection);

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
