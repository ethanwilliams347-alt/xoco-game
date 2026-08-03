#pragma once
#include <cstdint>

// Deterministic randomness for the simulation.
//
// There is no generator here and no state at all. A random value is a pure
// function of *where* and *when* it is asked for, which is what lets a run be
// reproduced from nothing but a seed and a step count. A stateful generator
// would mean a save file has to carry its internal state - 2.5 KB of it for
// std::mt19937 - and restore it byte-exactly, or the loaded world silently
// diverges from the one that was saved. Here there is nothing to carry.
//
// It also means randomness does not depend on *how many times it has been
// asked*. With a generator, adding one extra roll anywhere shifts every
// subsequent value in the world, so an unrelated change to one material's
// behaviour reshuffles all of them. Here each decision draws from its own
// coordinates and nothing else, so behaviour changes stay local.
namespace sim_random {

// Every distinct decision the engine makes draws from its own stream.
//
// Without this, two decisions taken about the same cell on the same step are
// literally the same number, so they would correlate permanently: a cell that
// rolled "move left" would always roll the same side of its reaction check, and
// no amount of stirring elsewhere would separate them. One tag per decision
// costs one xor and removes the whole class of problem.
//
// The values are arbitrary but must never change - changing one changes every
// world its seed ever produced. They are large odd constants sharing no
// structure, so the tag does mixing work rather than merely offsetting.
enum class Stream : uint64_t {
    ColorJitter     = 0x9E3779B97F4A7C15ull,
    SweepDirection  = 0xD1B54A32D192ED03ull,
    PowderDirection = 0xA0761D6478BD642Full,
    FluidDirection  = 0xE7037ED1A0B428DBull,
    Reaction        = 0x8EBC6AF09C88C6E3ull,
    Fracture        = 0xC2B2AE3D27D4EB4Full,
    // Whether a burning cell throws a flame this step, and which empty
    // neighbour it goes to. Two decisions, one stream: they are asked about the
    // same cell on the same step, so they are separated by index rather than by
    // tag - see emit_flame().
    Emission        = 0xF1BBCDCBB3D935A7ull,
    // How long one flame lives. Its own stream rather than a third use of
    // Emission: it is drawn about the *new* cell at the moment of emission, so
    // sharing would correlate a flame's lifetime with whether its parent chose
    // to emit at all - and a fire whose longest flames only ever appear in the
    // same spots is exactly the uniformity this was added to break.
    FlameLifetime   = 0xB5026F5AA96619E9ull,
    // Whether a flame rises this step. Per cell per step, so it must not share
    // with anything else asked about the same cell on the same step.
    FlameRise       = 0xCA9E6D9C1B5D4C77ull,
    // How much hotter or cooler than its material's stated figure *this spot*
    // has to get before it catches. Drawn with authored_spread - pinned at step
    // 0 - because it is a property of the wood at that coordinate, like its
    // colour jitter, and not a decision the cell retakes every step. A cell that
    // re-rolled its own ignition point each frame would simply catch on the
    // first frame the lowest roll came up, which is the timing jitter this
    // replaced and measurably does nothing.
    IgnitionPoint   = 0xD6E8FEB86659FD93ull,
};

// splitmix64's finalizer. Two multiplies and three xor-shifts, entirely in
// registers, and not cryptographic - it does not need to be. The requirement is
// only that inputs one apart give unrelated outputs, which matters here more
// than it sounds: the inputs are consecutive cell indices and consecutive step
// numbers, so a weak mix shows up immediately as diagonal banding in falling
// powder and as a drift in one direction along each row.
inline constexpr uint64_t mix(uint64_t x) {
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
    return x ^ (x >> 31);
}

// The one place the four inputs are combined. Each varying input is multiplied
// by a large odd constant before being folded in, which spreads its low bits -
// the ones that actually change from cell to cell and step to step - across the
// whole word before the mix sees them.
inline constexpr uint64_t bits(uint64_t seed, uint64_t step, uint64_t index, Stream stream) {
    uint64_t x = seed ^ static_cast<uint64_t>(stream);
    x = mix(x ^ (step * 0x9E3779B97F4A7C15ull));
    x = mix(x ^ (index * 0xD1B54A32D192ED03ull));
    return x;
}

// A 50/50 decision. Reads the low bit, which is safe here because the mixer's
// last step folds the high half down into it.
inline constexpr bool coin(uint64_t seed, uint64_t step, uint64_t index, Stream stream) {
    return (bits(seed, step, index, stream) & 1ull) != 0;
}

// True with probability pct/100. The modulo bias is on the order of 1 in 2^57
// and is not worth a rejection loop in the inner simulation loop.
inline constexpr bool chance(int pct, uint64_t seed, uint64_t step, uint64_t index, Stream stream) {
    if (pct <= 0) return false;
    if (pct >= 100) return true;
    return static_cast<int>(bits(seed, step, index, stream) % 100ull) < pct;
}

// True with probability per_myriad/10000, for decisions too rare to express in
// whole percents. A spontaneous decay row is the case that needs it: mean
// lifetime is 10000/per_myriad steps.
//
// **This was per *mille*, and it was widened for a reason worth recording,
// because it is the second time the same wall has been hit.** Whole percents
// bottomed out at a hundred-step mean, which could not express wood's three
// seconds, so the column went to per-mille. Per-mille then bottomed out on the
// other axis - not on how long a thing can last, but on how *finely* a long
// lifetime can be adjusted. A playtest asked for wood's burn to last one second
// longer, and the available steps either side of it were 6 per mille (2.78 s)
// and 5 per mille (3.33 s): the request fell between two adjacent values of the
// column and could not be expressed at all.
//
// The generalisable form: **the resolution a tuning column needs is set by the
// smallest change anyone will want to make to it, not by the largest value it
// has to hold.** A lifetime of 167 steps is comfortably inside per-mille's
// range and still not adjustable within it.
inline constexpr bool chance_per_myriad(int per_myriad, uint64_t seed, uint64_t step, uint64_t index, Stream stream) {
    if (per_myriad <= 0) return false;
    if (per_myriad >= 10000) return true;
    return static_cast<int>(bits(seed, step, index, stream) % 10000ull) < per_myriad;
}

// A value in [0, n). Used where a cell has to choose one of several neighbours
// rather than answer yes or no.
inline constexpr int pick(int n, uint64_t seed, uint64_t step, uint64_t index, Stream stream) {
    if (n <= 1) return 0;
    return static_cast<int>(bits(seed, step, index, stream) % static_cast<uint64_t>(n));
}

// A value in [-range, range]. Used for colour jitter, where the caller wants a
// symmetric offset rather than a fraction.
inline constexpr int spread(int range, uint64_t seed, uint64_t step, uint64_t index, Stream stream) {
    if (range <= 0) return 0;
    const uint64_t span = static_cast<uint64_t>(2 * range + 1);
    return static_cast<int>(bits(seed, step, index, stream) % span) - range;
}

// ---------------------------------------------------------------------------
// Reserved for world generation. Nothing draws from this yet.
//
// The rule, written down before there is a generator to break it: **world
// generation draws only from streams minted by worldgen(), and the simulation
// draws only from the tags declared above. Neither borrows the other's.**
//
// The point is not tidiness. A generator sharing the simulation's streams would
// mean that generating one extra cave changes the values the simulation reads
// for those same cells, so a terrain tweak silently alters how sand falls in
// parts of the world it never touched - and the two would be impossible to tell
// apart from a bug in the sand.
//
// Two notes for whoever writes the generator:
//
//   - Generation is authored, not per-step. Pass **step 0**, the same as colour
//     jitter does, so a feature's shape does not depend on which step happened
//     to generate it.
//   - `index` means "which thing am I asking about", not necessarily a cell. A
//     draw about the seventh cave passes 7.
//
// Streams are minted rather than hand-written so the generator can take as many
// as it needs without anyone picking further magic constants by eye. All of this
// is one function and one constant, which is a large part of why the hash is the
// right shape: with a stateful generator, a separate salt space would be a
// second generator object to own, seed, save and keep in lockstep with the first.
inline constexpr Stream worldgen(uint64_t n) {
    return static_cast<Stream>(mix(0x5851F42D4C957F2Dull ^ (n * 0x2545F4914F6CDD1Dull)));
}

// Every simulation stream, listed once so the check below can see them all. A
// new tag has to be added here as well as to the enum - the same duty MATERIALS
// has towards ElementType, and enforced the same way.
inline constexpr Stream SIM_STREAMS[] = {
    Stream::ColorJitter,
    Stream::SweepDirection,
    Stream::PowderDirection,
    Stream::FluidDirection,
    Stream::Reaction,
    Stream::Fracture,
    Stream::Emission,
    // **These three were missing, and FlameLifetime and FlameRise had been
    // missing since they were added.** The list is what the distinctness check
    // below can see, so a tag left out of it is a tag nothing checks - exactly
    // the silent collision the check exists to catch, and the check itself
    // passing the whole time. Adding a tag to the enum without adding it here
    // is easy to do and invisible afterwards, which is why the enum's own
    // comment asks for both.
    Stream::FlameLifetime,
    Stream::FlameRise,
    Stream::IgnitionPoint,
};

// Two streams sharing a value are one stream, silently. Nothing about that looks
// wrong from the outside - the world still runs, it just has a correlation in it
// that no assertion is placed to catch - so it is checked at compile time, where
// it cannot be skipped. Covers the declared tags against each other and against
// the first sixteen minted generation streams.
namespace detail {
constexpr bool streams_distinct() {
    constexpr int sim_n = sizeof(SIM_STREAMS) / sizeof(SIM_STREAMS[0]);
    constexpr int gen_n = 16;
    uint64_t all[sim_n + gen_n] = {};
    for (int i = 0; i < sim_n; ++i) all[i] = static_cast<uint64_t>(SIM_STREAMS[i]);
    for (int i = 0; i < gen_n; ++i)
        all[sim_n + i] = static_cast<uint64_t>(worldgen(static_cast<uint64_t>(i)));

    for (int i = 0; i < sim_n + gen_n; ++i)
        for (int j = i + 1; j < sim_n + gen_n; ++j)
            if (all[i] == all[j]) return false;
    return true;
}
} // namespace detail

static_assert(detail::streams_distinct(), "two streams share a value and are therefore one stream");

} // namespace sim_random
