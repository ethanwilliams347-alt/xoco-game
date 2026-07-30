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

// A value in [-range, range]. Used for colour jitter, where the caller wants a
// symmetric offset rather than a fraction.
inline constexpr int spread(int range, uint64_t seed, uint64_t step, uint64_t index, Stream stream) {
    if (range <= 0) return 0;
    const uint64_t span = static_cast<uint64_t>(2 * range + 1);
    return static_cast<int>(bits(seed, step, index, stream) % span) - range;
}

} // namespace sim_random
