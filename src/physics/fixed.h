#pragma once
#include <cstdint>

// Signed 16.16 fixed point, and nothing else.
//
// This exists for one reason: **`float` results are not reproducible across
// machines.** Not across compilers, optimisation levels, x87 versus SSE, or
// whether the compiler contracted a multiply-add. The engine's determinism
// guarantee is the thing three separate plan items spend as portable - a crash
// report that reproduces the crash, a save file that replays, and the first
// build on a machine that is not this one - and every one of them is worthless
// if the number that decided where the body landed differs in its last bit.
//
// Integer arithmetic has no such freedom. `a / b` on int32_t is the same value
// on every conforming compiler on every architecture, and so is the whole chain
// built out of it.
//
// **16 fractional bits, and the split is not arbitrary.** The fractional half
// has to resolve a sub-cell remainder finely enough that motion below one cell
// per step is smooth - 1/65536 of a cell is ~1/16000 of a screen pixel at
// Camera::SCALE, which is far past visible. The integer half has to hold a
// velocity in cells *per second*, which is how every constant in TUNING.md is
// written and is ~500 at its largest, against a range of +/-32767. Neither half
// is close to its limit, which is what makes 16/16 the boring choice rather
// than a tuned one.
//
// Nothing here is a class. A wrapper type would buy compile-time unit safety
// and cost the ability to read the arithmetic at a glance, and the arithmetic
// is the part that has to be checkable by eye.
namespace fx {

// The representation. int32_t rather than int, because "how many bits" is the
// whole contract - a 16-bit int would silently overflow at four cells.
using v = int32_t;

inline constexpr int FRAC_BITS = 16;
inline constexpr v ONE = 1 << FRAC_BITS;

// The fixed simulation rate. Here rather than on `Run` because the conversion
// from a per-second constant to a per-step amount happens inside the physics,
// and `Run::FIXED_DT` is the same number written as a duration for the frame
// pacer's benefit. **They must agree**; the static_assert in run.h is what says
// so.
inline constexpr int STEPS_PER_SECOND = 60;

// Build a value from an exact rational, at compile time. Constants like
// MOVE_SPEED = 112.5 are written `from_ratio(225, 2)` rather than as a float
// literal cast, so there is no float anywhere in the chain that produces them -
// including at compile time, where a compiler is just as free to fold
// differently as it is at runtime.
constexpr v from_ratio(int64_t num, int64_t den) {
    return static_cast<v>((num * ONE) / den);
}

constexpr v from_int(int32_t n) { return n * ONE; }

// Whole part, **truncated toward zero** - which is what integer division
// already does in C++, and is deliberately not a floor. The sub-cell remainder
// scheme needs the same rounding in both directions: a body moving left by 0.6
// of a cell must take zero whole steps, exactly as a body moving right by 0.6
// does. Flooring would turn that into a step of -1 and a remainder of +0.4, and
// the body would drift left while standing still.
constexpr int32_t trunc(v a) { return a / ONE; }

// The fractional part left over after `trunc`, carrying the sign of `a`.
constexpr v frac(v a) { return a - trunc(a) * ONE; }

// For rendering and for test output only. Anything in src/physics/ that reaches
// for this has just put a float back into the simulation.
constexpr float to_float(v a) { return static_cast<float>(a) / static_cast<float>(ONE); }

// A per-second rate as a per-step amount. One integer division, so it is exact
// on every machine, and it truncates toward zero - symmetric about zero, so
// gravity down and a wingbeat up lose the same amount.
//
// The error is under 1/65536 of a unit per step and does **not** accumulate
// into a drift the way a float sum does: the truncated value is a constant, so
// adding it a thousand times is a thousand times one exact number rather than a
// thousand roundings.
constexpr v per_step(v per_second) { return per_second / STEPS_PER_SECOND; }

// The one property above that a plausible "optimisation" would break, pinned so
// that it breaks the build instead. `a >> 16` is faster than `a / 65536` and is
// *not the same function*: it floors, so a body drifting left at half a cell a
// step would take a whole-cell step every step and grow a positive remainder,
// walking left through terrain the collision code was never asked about.
//
// Written as an assertion on the pair rather than on either one, because what
// matters is that the two directions agree.
static_assert(trunc(from_ratio(3, 2)) == 1 && trunc(from_ratio(-3, 2)) == -1,
              "fx::trunc must round toward zero in both directions, not floor");
static_assert(frac(from_ratio(3, 2)) == ONE / 2 && frac(from_ratio(-3, 2)) == -(ONE / 2),
              "fx::frac must carry the sign of its argument, or a body moving left "
              "accumulates a remainder pointing right");
static_assert(from_ratio(225, 2) == 112 * ONE + ONE / 2, "from_ratio is not exact");

} // namespace fx
