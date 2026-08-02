#include "grid.h"
#include "reaction.h"
#include <algorithm>

namespace {
    int clamp_channel(int v) {
        return v < 0 ? 0 : (v > 255 ? 255 : v);
    }

    // How much heat moves from `a` to `b` in one step, given the rate the pair
    // conducts at. Negative flows the other way. Three properties are wanted
    // here and each one is a line:
    //
    //  - **A dead band.** Two things within a degree of each other exchange
    //    nothing. Without it a pair would trade a unit back and forth forever,
    //    every step, and no chunk containing anything warm could ever sleep -
    //    which is the same standing cost E1 rejected its first design over.
    //  - **A floor of one unit.** Integer division truncates toward zero, so a
    //    slow conductor across a small difference would compute a flow of zero
    //    and heat would simply stop partway. One unit is the smallest step this
    //    representation has, and it is what makes the whole thing eventually
    //    reach the temperature it is heading for rather than stalling short.
    //  - **A ceiling of half the difference.** The exchange must never
    //    overshoot, or a hot cell and a cold one swap places and oscillate. Half
    //    is the exact point where the two ends meet.
    int heat_flow(int a, int b, int rate, int divisor) {
        const int delta = a - b;
        if (delta > -2 && delta < 2) return 0; // dead band
        if (rate <= 0) return 0;

        int flow = delta * rate / divisor;
        const int half = delta / 2; // toward zero, and |half| >= 1 given the band
        if (delta > 0) {
            if (flow < 1) flow = 1;
            if (flow > half) flow = half;
        } else {
            if (flow > -1) flow = -1;
            if (flow < half) flow = half;
        }
        return flow;
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
    scratch_visit.resize(width * height, 0);

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

    pressure_queue.clear();
    std::fill(scratch_visit.begin(), scratch_visit.end(), 0);
    scratch_epoch = 0;

    fracture_component.clear();
    next_piece_tag = 1;

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
    place(x, y, type, jittered_color(material_of(type), static_cast<uint64_t>(idx)));
}

void Grid::paint(int x, int y, ElementType type, uint32_t color) {
    if (!is_within_bounds(x, y)) return;
    place(x, y, type, color);
}

// Shared by set_element and paint, which differ only in where the colour comes
// from - a jittered draw from the material table versus one the caller names
// outright. Everything downstream of "here is the colour" - the write rule,
// the wake, the support re-check - has to stay identical between the two, or
// authored terrain and brush-placed terrain would silently behave differently
// the moment someone touched one write path without the other.
void Grid::place(int x, int y, ElementType type, uint32_t color) {
    const int idx = get_index(x, y);
    const ElementType old_type = cells[idx].type;

    Element el;
    el.type = type;
    el.color = color;
    el.updated_tag = frame_tag; // freshly placed cells wait until the next step to move

    // Heat belongs to the spot rather than to what is standing in it, so a cell
    // written into a hot region arrives hot and a reaction product keeps the
    // temperature that caused it - ignited Wood becomes Fire that is already
    // burning, not a flame starting from room temperature. Materials that are
    // hot by definition say so in the table and override it; `Empty` names
    // ambient there, which is what makes erasing a cell also clear its heat.
    const uint8_t spawn = material_of(type).spawn_temperature;
    el.temperature = spawn != 0 ? spawn : cells[idx].temperature;

    // **`fall_ticks` and `piece_tag` are reset here, and that is a decision
    // rather than an oversight** - `el` is a fresh Element, so every field not
    // named above goes back to its default, and only `temperature` argues for
    // itself. Worth saying out loud because the other two are invisible: a cell
    // written into a broken piece does not join it, it becomes a piece of its
    // own with tag 0, so patching a crack with the brush leaves a seam the
    // support fill can see and nobody else can.
    //
    // Harmless today, because nothing can reach the case that would hurt.
    // Reactions go through here, but no row in REACTIONS turns one structural
    // material into another - Wood burns to Fire, which is not structural, so
    // the tag it loses meant nothing. **E7 is what makes this live**: a row like
    // molten stone freezing back to stone is structural-to-structural, and it
    // would silently reset the tag of every cell it touched, re-welding pieces
    // that had come apart. If that row is ever written, decide here whether the
    // tag survives a transformation before writing it.

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

    const Element& under = cells[get_index(x, below_y)];

    // More of the same structure is not support -- whether *it* is held up is
    // the question the flood fill is already answering.
    //
    // "The same structure" now means the same *piece*, not merely the same kind
    // of material. A fragment that has broken off and come to rest on the slab
    // it broke away from is standing on something, exactly as it would be if it
    // had landed on unrelated masonry, and the fill below will never reach
    // across the crack to discover otherwise. Without this, a heap of broken
    // pieces would be one mutually-unsupported tower that sinks through itself.
    if (is_structural(under.type) && under.piece_tag == cells[get_index(x, y)].piece_tag) return false;

    return is_solid(under.type);
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

        // Fracture's own visited marks, advanced with the pass rather than per
        // call, so that one landing costs one fill instead of one per seed. A
        // slab lands with every cell of it queued, and each of those seeds would
        // otherwise re-walk the whole piece to ask a question the first one has
        // already answered -- 28 redundant fills for a 60-wide slab, on the one
        // step where the most work is already happening. Sharing the buffer with
        // the E1 pressure search is safe because that search only ever runs
        // inside the cell sweep, and support resolves before the sweep starts.
        if (++scratch_epoch == 0) {
            std::fill(scratch_visit.begin(), scratch_visit.end(), uint8_t{0});
            scratch_epoch = 1;
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
    // The piece being judged is the run of connected structure sharing this
    // tag. Everything below tests against it rather than against "is this
    // structural", which is what makes a crack a real boundary.
    const uint8_t tag = cells[seed].piece_tag;

    // Read now, because settle_marks zeroes fall_ticks the moment this concludes.
    const bool was_falling = cells[seed].fall_ticks >= FRACTURE_MIN_TICKS;

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
            // It has landed. If it arrived with speed on it, this is the moment
            // it breaks -- before the marks are settled, so the fresh tags are
            // in place for the next step's fills to see, and before settle_marks
            // zeroes the fall_ticks fracture reads.
            //
            // Seeded from where the fill *started*, not from the grounded cell
            // it ended at, and the difference is not cosmetic. is_grounded
            // refuses to count more of the same structure as support, so a slab
            // coming to rest on a stone floor is not grounded where it touches:
            // the fill walks on down through the floor and answers "grounded"
            // at the bottom of the world, a hundred cells away and stationary.
            // Handing that cell to fracture asks about the wrong piece, and it
            // fails quietly - the seed is at rest, so nothing ever breaks.
            if (was_falling) fracture_landing(x, y);

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
                if (cells[nidx].piece_tag != tag) continue; // across a crack: a different piece

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

void Grid::fracture_landing(int x, int y) {
    const int seed = get_index(x, y);
    const uint8_t tag = cells[seed].piece_tag;

    // Its own fill, and deliberately not support_visit's: the fill that called
    // this is mid-flight and its marks are load-bearing for the rest of the
    // pass. The epoch is advanced once per pass by resolve_support, not here,
    // which is also what makes this early return work -- every other cell of a
    // piece that has already been through here is stamped, so a landing costs
    // one fill however many of its cells were queued.
    if (scratch_visit[seed] == scratch_epoch) return;

    fracture_component.clear();
    fracture_component.push_back(seed);
    scratch_visit[seed] = scratch_epoch;

    int min_x = x, max_x = x;

    for (size_t head = 0; head < fracture_component.size(); ++head) {
        const int idx = fracture_component[head];
        const int cy = idx / width;
        const int cx = idx - cy * width;

        if (cx < min_x) min_x = cx;
        if (cx > max_x) max_x = cx;

        // Too big to break is the same answer as too big to judge, and for the
        // same reason: when the question costs more than it is worth, guess the
        // way that leaves the level standing.
        if (static_cast<int>(fracture_component.size()) > MAX_SUPPORT_CELLS) return;

        for (int ny = cy - 1; ny <= cy + 1; ++ny) {
            for (int nx = cx - 1; nx <= cx + 1; ++nx) {
                if (nx == cx && ny == cy) continue;
                if (!is_within_bounds(nx, ny)) continue;
                const int nidx = get_index(nx, ny);
                if (!is_structural(cells[nidx].type)) continue;
                if (cells[nidx].piece_tag != tag) continue;
                // **Still in flight.** This is what identifies "the piece that
                // just landed", and connectivity cannot: the instant a slab
                // touches the floor the two are one structural component, so a
                // fill that followed structure alone would walk out of the slab,
                // across the entire floor, past MAX_SUPPORT_CELLS, and give up -
                // which is exactly what it did, silently, and looked from the
                // outside like fracture simply not firing. fall_ticks is zero
                // for everything at rest and non-zero for everything that has
                // been moving, and it is read here before settle_marks clears
                // it, which is why fracture_landing is called where it is.
                if (cells[nidx].fall_ticks == 0) continue;
                if (scratch_visit[nidx] == scratch_epoch) continue;
                scratch_visit[nidx] = scratch_epoch;
                fracture_component.push_back(nidx);
            }
        }
    }

    if (static_cast<int>(fracture_component.size()) < MIN_FRACTURE_CELLS) return;

    // A seam needs a piece on both sides of it, so a piece under three cells
    // wide has nowhere to crack. The seam is drawn between columns rather than
    // along one, which is what makes fracture cost no matter: no cell is
    // removed, relabelled out of existence, or duplicated - every cell ends up
    // in exactly one of the two pieces.
    const int span = max_x - min_x + 1;
    if (span < 3) return;

    // **The crack goes where the support ends, and that is the whole rule.**
    //
    // The first version put it near the middle of the piece with a random
    // offset, on the theory that masonry breaks somewhere arbitrary. It does
    // not work, and the reason is worth keeping: a break only *does* anything
    // if it separates a part that is held up from a part that is not. Put it
    // anywhere else and both fragments still rest on the same ground, so
    // nothing moves and the only trace of the break is a tag nobody can see. A
    // 60-wide slab dropped across a 30-wide step broke at column 55 and looked
    // exactly like no fracture at all.
    //
    // Reading the ground instead makes ROADMAP.md's "splits along the stress"
    // literal: this is the piece's own footprint, column by column, and the
    // crack is the line between the columns that landed on something and the
    // columns that landed on nothing. It also means **a piece that lands flat
    // on flat ground does not break at all** - there is no boundary to break at
    // - which is both correct and what stops every routine landing burning a
    // tag out of the 255 there are.
    std::vector<int> lowest(span, -1);
    for (const int idx : fracture_component) {
        const int c = (idx - (idx / width) * width) - min_x;
        if (lowest[c] < 0 || idx > lowest[c]) lowest[c] = idx; // larger index = lower row
    }

    // Scanning from the left is arbitrary but deterministic. A piece with
    // several boundaries breaks at one of them now and is asked again the next
    // time a fragment lands, which keeps one landing's work bounded.
    //
    // Note this asks a plainer question than is_grounded does. There, "more of
    // the same structure is not support", because whether *that* is held up is
    // the thing the fill is working out. Here the piece has stopped and the
    // only question is whether this column arrived on top of anything - the
    // floor it just landed on counts, and it is not part of the piece, because
    // the piece is the set of cells that were in flight.
    const auto landed_on_something = [&](int idx) {
        const int cy = idx / width;
        const int cx = idx - cy * width;
        if (cy + 1 >= height) return true; // the bottom of the world
        return is_solid(cells[get_index(cx, cy + 1)].type);
    };

    int crack = -1;
    bool prev_grounded = false, have_prev = false;
    for (int c = 0; c < span; ++c) {
        if (lowest[c] < 0) continue; // a gap in the footprint is not a boundary
        const bool g = landed_on_something(lowest[c]);
        if (have_prev && g != prev_grounded) { crack = min_x + c; break; }
        prev_grounded = g;
        have_prev = true;
    }
    if (crack < 0) return; // it landed evenly: there is nothing to break

    // A cell of jitter either way, so a collapse does not read as a machine cut
    // exactly along the lip of the ledge every single time.
    crack += sim_random::spread(1, world_seed, step_count,
                                static_cast<uint64_t>(seed), sim_random::Stream::Fracture);
    if (crack < min_x + 1) crack = min_x + 1;
    if (crack > max_x) crack = max_x;

    const uint8_t fresh = next_piece_tag;
    next_piece_tag = static_cast<uint8_t>(next_piece_tag + 1);
    if (next_piece_tag == 0) next_piece_tag = 1; // 0 means "never broken"

    for (const int idx : fracture_component) {
        const int cy = idx / width;
        const int cx = idx - cy * width;
        if (cx >= crack) cells[idx].piece_tag = fresh;

        // **Both** halves are re-queued, not just the one that was relabelled.
        // The half that keeps its tag is usually the one now hanging over
        // nothing -- it is the far side of the ledge that has to fall, and it is
        // the side the crack did *not* rename. Queuing only the renamed cells
        // left the overhang settled, unwoken and hanging in the air, looking
        // for all the world like fracture had not fired at all. A crack nobody
        // re-examines does nothing until something else disturbs the area.
        queue_support_check(cx, cy);
        mark_dirty(cx, cy);
    }
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
    //
    // **A lateral move has to land somewhere it can rest or descend from**, and
    // that condition is the whole of why a pool can sleep. Without it, the last
    // partial row of any body of liquid slides back and forth across its own
    // flat surface forever: it cannot sink (equal density fails can_displace),
    // and seek_level will not take it either, because a one-cell head is inside
    // MIN_PRESSURE_HEAD's hysteresis. But bare Empty to the side was always
    // reason enough to move, so it moved, every step, at up to `spread` cells a
    // time and in a direction drawn fresh each step. A tank filled to an exact
    // multiple of its width slept; one cell more and it never did - two chunks
    // awake and ~34 cells changing places per step, forever, on a body of water
    // that had visibly finished settling.
    //
    // Resting on something solid still counts, which is what keeps a puddle
    // spreading across a floor: that move is refused only when the destination
    // would be perched on more of the same liquid with nowhere to go, which is
    // precisely the move that achieves nothing. The cost is that a body is
    // level to within one cell rather than exactly - the same trade
    // MIN_PRESSURE_HEAD already makes, and it now buys a surface that is
    // *still* rather than merely level on average.
    const auto can_rest_at = [&](int nx) {
        if (can_displace(mat, nx, y + 1, 1)) return true;      // it can carry on down
        return is_solid(get_element(nx, y + 1).type);          // or it has a floor (OOB reads as Wall)
    };

    for (const int d : {dir, -dir}) {
        int cx = x;
        int best = x;
        for (int i = 0; i < mat.spread; ++i) {
            const int nx = cx + d;
            if (!is_within_bounds(nx, y)) break;
            if (cells[get_index(nx, y)].type != ElementType::Empty) break;
            cx = nx;
            // Furthest *usable* landing, not merely furthest reachable: a cell
            // may pass over a stretch it could not stop on to get to one it can.
            if (can_rest_at(cx)) best = cx;
        }
        if (best != x) {
            swap_elements(x, y, best, y);
            return true;
        }
    }

    // Out of ordinary moves. A liquid gets one last question - see the comment
    // on MAX_PRESSURE_CELLS in the header. Gases are excluded: `dy < 0` has
    // already spent its vertical move on going up, and a gas finding its level
    // is not a thing anyone has asked to see.
    if (dy > 0 && seek_level(x, y)) return true;

    return false;
}

int Grid::find_lower_surface(int x, int y, ElementType type) {
    // Fresh epoch per search. Unlike the support fill's, these marks must not
    // outlive the one search that made them: two adjacent surface cells of the
    // same body ask genuinely different questions, because the threshold is
    // measured from the asking cell's own row.
    if (++scratch_epoch == 0) { // wrapped, so old marks can no longer be told apart
        std::fill(scratch_visit.begin(), scratch_visit.end(), uint8_t{0});
        scratch_epoch = 1;
    }

    const int target_row = y + MIN_PRESSURE_HEAD;

    pressure_queue.clear();
    const int seed = get_index(x, y);
    pressure_queue.push_back(seed);
    scratch_visit[seed] = scratch_epoch;

    for (size_t head = 0; head < pressure_queue.size(); ++head) {
        const int idx = pressure_queue[head];
        const int cy = idx / width;
        const int cx = idx - cy * width;

        // A surface of this body, low enough to be worth moving to. Note this
        // can never be the asking cell itself, which is at row y.
        if (cy >= target_row && cells[idx - width].type == ElementType::Empty) return idx;

        // Checked after the test above, so a body that reaches the cap still
        // gets to answer with what it found rather than being cut off one cell
        // short of it.
        if (static_cast<int>(pressure_queue.size()) >= MAX_PRESSURE_CELLS) return -1;

        static constexpr int DX[4] = { 0,  0, -1, 1 };
        static constexpr int DY[4] = {-1,  1,  0, 0 };
        for (int k = 0; k < 4; ++k) {
            const int nx = cx + DX[k];
            const int ny = cy + DY[k];
            if (!is_within_bounds(nx, ny)) continue;

            const int nidx = get_index(nx, ny);
            if (cells[nidx].type != type) continue;
            if (scratch_visit[nidx] == scratch_epoch) continue;

            scratch_visit[nidx] = scratch_epoch;
            pressure_queue.push_back(nidx);
        }
    }

    return -1;
}

bool Grid::seek_level(int x, int y) {
    // Only a surface cell moves. Anything with liquid or solid on top of it is
    // pinned by what is above it, and letting a buried cell go would tunnel a
    // hole through the middle of a body rather than lower its surface.
    //
    // This is also what bounds the cost: in a settled pool it is one array read
    // per cell, and only the thin surface line reaches the search at all. The
    // row-0 case falls out of the same test, since get_element reads
    // out-of-bounds as Wall.
    if (get_element(x, y - 1).type != ElementType::Empty) return false;

    const int target = find_lower_surface(x, y, cells[get_index(x, y)].type);
    if (target < 0) return false;

    // Onto the receiving surface, not into it. The cell above the target is the
    // Empty that find_lower_surface required, so this is a swap with Empty and
    // conserves matter for the same reason every other move in this file does.
    const int ty = target / width;
    swap_elements(x, y, target - ty * width, ty - 1);
    return true;
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

bool Grid::step_thermal(int x, int y, const Material& mat) {
    const int idx = get_index(x, y);

    // **A cell that is exactly at ambient does no thermal work at all**, and
    // this one line is the difference between heat costing 18% of the frame in
    // the worst case and costing nothing measurable. It was not an optimisation
    // added on suspicion: the bracketed A/B below-the-line put `cascading` at
    // 88% of a 60 Hz frame with the full pass running on every awake cell, and
    // `cascading` contains no fire and never gets warm - it was paying eight
    // neighbour probes a cell a step to confirm, every step, that nothing had
    // changed.
    //
    // It is exact rather than an approximation, and the reason is that
    // **conduction is symmetric**. Every exchange writes both ends by the same
    // amount, so it does not matter which of the pair initiates it. If a
    // neighbour is more than a degree from ambient then that neighbour is off
    // ambient, is therefore not skipped here, and will do the exchange itself
    // when the sweep reaches it. If it is within a degree, the dead band means
    // there was nothing to do either way.
    //
    // **This used to claim that nothing can be off ambient and asleep, and that
    // is not true** - it is the dead band that makes it false, and the dead band
    // is also what makes a warm world settle at all. A cell one degree off
    // ambient exchanges nothing with ambient and nothing with a neighbour at the
    // same temperature, so it stops marking itself dirty and sleeps sitting at
    // 19 or 21. Measured rather than argued: a burnt-out 100x100 world sleeps
    // completely with 200,009 total heat against the 200,000 a uniformly ambient
    // one would hold - nine units stranded in cells that will never give them up.
    //
    // Harmless, and worth stating correctly anyway, because the skip above is
    // sound for the *symmetry* reason and not for the stronger claim. What
    // actually has to hold is only this: a cell with somewhere for its heat to
    // go is awake. A cell whose every gradient is inside the dead band has
    // nowhere for it to go, which is why it is allowed to sleep holding it.
    //
    // Heat sources are excluded: Fire is at ambient for exactly one moment, the
    // step it is placed, and skipping it then would leave it cold forever.
    if (cells[idx].temperature == AMBIENT_TEMPERATURE && mat.heat_source == 0) return false;

    bool changed = false;

    // A source holds itself up rather than settling with its surroundings. Only
    // upwards: a flame in a furnace is not cooled by being in a furnace.
    if (mat.heat_source > cells[idx].temperature) {
        cells[idx].temperature = mat.heat_source;
        changed = true;
    }

    // All eight neighbours, not the four orthogonal ones, and that is not the
    // same call E1's pressure search made. There, a diagonal step would let two
    // pools that merely touch at a corner equalize into each other, which is
    // wrong because it moves *matter* through a seam with no area. Heat through
    // a corner is harmless, and refusing it is what actually breaks:
    //
    // an ignited Wood cell becomes Fire, which is a gas, so it rises out of the
    // beam on the next step. The flame that should light the next cell along is
    // then sitting diagonally above it and nowhere else, and with a
    // four-neighbour rule the fire front stalls after exactly one cell. That is
    // not a tuning problem - no conductivity makes heat cross a gap the rule
    // says does not exist. The first version of this function was written the
    // other way and the burning-beam test is what caught it.
    //
    // It also puts heat on the same 8-neighbourhood as `has_neighbor` and
    // `mark_dirty`, so contact means one thing throughout the engine.
    if (mat.conductivity > 0) {
        static constexpr int DX[8] = { 0,  0, -1, 1, -1,  1, -1, 1 };
        static constexpr int DY[8] = {-1,  1,  0, 0, -1, -1,  1, 1 };
        for (int k = 0; k < 8; ++k) {
            const int nx = x + DX[k];
            const int ny = y + DY[k];
            // The world's border is an insulator. get_element() would read it as
            // Wall, which conducts, and every edge of the world would then act
            // as an infinite heat sink - the same trap has_neighbor() documents.
            if (!is_within_bounds(nx, ny)) continue;

            const int nidx = get_index(nx, ny);
            const int nrate = material_of(cells[nidx].type).conductivity;
            if (nrate == 0) continue; // Empty, or anything else outside the system

            // The pair conducts at the lower of the two, so an insulator between
            // two conductors stops the heat rather than averaging with it.
            const int rate = mat.conductivity < nrate ? mat.conductivity : nrate;
            const int flow = heat_flow(cells[idx].temperature, cells[nidx].temperature,
                                       rate, CONDUCTION_DIVISOR);
            if (flow == 0) continue;

            // Applied to both ends by the same amount, so conduction moves heat
            // and never creates or destroys it - the conservation-of-matter
            // argument, in the one place the new axis could break it. The
            // neighbour is written whether or not it has been visited this step;
            // temperature is not gated on updated_tag, because it is not a move.
            cells[idx].temperature = static_cast<uint8_t>(cells[idx].temperature - flow);
            cells[nidx].temperature = static_cast<uint8_t>(cells[nidx].temperature + flow);
            changed = true;
        }
    }

    // And the world forgets. Without this the total heat in a world could only
    // ever go up, since Fire adds and nothing removes, and a scene would slowly
    // cook itself. This is the only place heat leaves the simulation, and it is
    // also what eventually puts a burnt-out scene back to sleep.
    const int bleed = heat_flow(cells[idx].temperature, AMBIENT_TEMPERATURE,
                                mat.conductivity, AMBIENT_DIVISOR);
    if (bleed != 0) {
        cells[idx].temperature = static_cast<uint8_t>(cells[idx].temperature - bleed);
        changed = true;
    }

    if (changed) mark_dirty(x, y);
    return changed;
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
    //
    // The temperature window is part of that test and not an afterthought.
    // Since E2 made ignition spontaneous-and-gated rather than catalyst-driven,
    // Wood and Water are `Count` rows too, and marking on the row alone would
    // wake every wooden beam and every pool in the world, every step, forever -
    // it would hand the entire cost of the sleep system back. A cell only
    // self-marks while it is actually inside the window that could transform it,
    // and getting into that window means its temperature is moving, which
    // step_thermal is already marking it for.
    for (const Reaction& r : REACTIONS) {
        if (r.catalyst == ElementType::Count && r.target == cell.type &&
            cell.temperature >= r.min_temp && cell.temperature <= r.max_temp) {
            mark_dirty(x, y);
            break;
        }
    }

    for (const Reaction& r : REACTIONS) {
        if (r.target != cell.type) continue;
        if (cell.temperature < r.min_temp || cell.temperature > r.max_temp) continue;
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

    const Material& mat = material_of(current.type);

    // Before the reaction, not after: the row that transforms this cell is
    // gated on the temperature it has *now*, so a cell that reaches its
    // ignition point this step ignites on this step rather than one later.
    step_thermal(x, y, mat);

    if (try_react(x, y)) return; // converted; let the new material move starting next frame

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





