#pragma once
#include "material.h"
#include <cstdint>

// A + optional-neighbor -> result, rolled once per eligible cell per step. Rows
// are checked in order; the first row whose target and catalyst condition are
// both satisfied is the only one considered that frame (win or lose the roll,
// no falling through to a later row) - this is what makes water-dousing take
// priority over Fire's natural burnout below without any special-casing.
struct Reaction {
    ElementType catalyst; // required 8-neighbor; ElementType::Count = spontaneous, no neighbor needed
    ElementType target;   // the cell type this row may transform

    // 0-10000, rolled once per eligible cell per step.
    //
    // **Per myriad rather than per cent, and that is a burn duration in
    // disguise.** A spontaneous decay row is a lifetime: mean steps is
    // 10000/chance. This is where E9 keeps burn durations, rather than as a
    // `burn_duration` column on MATERIALS, and it is the same argument as the
    // temperature window below: how long a transformation takes belongs to the
    // transformation.
    //
    // **The column has now been widened twice, and the second time is the
    // instructive one.** Per cent bottomed out at a hundred-step mean, too short
    // for wood, so it went to per mille - a limit on the longest lifetime
    // expressible. Per mille then failed on a different axis: a playtest asked
    // for wood to burn one second longer, and 6 and 5 per mille are 2.78 s and
    // 3.33 s with nothing in between. The value was well inside the range and
    // still not adjustable. See `chance_per_myriad` in random.h.
    uint16_t chance_per_myriad;

    ElementType result;

    // Temperature window the target cell must be inside for this row to be
    // eligible at all, inclusive on both ends. 0-255 is "any temperature", and
    // it is what the rows that are genuinely about contact rather than heat use.
    //
    // **This is where the ignition point lives, rather than as a column on
    // MATERIALS.** A threshold is a property of a transformation, not of a
    // substance: the same Water row that boils at 100 would need a second
    // number the moment anything else about water were temperature-gated, and
    // Fire is already both a target and a catalyst with no threshold of its own.
    // Keeping it here also means the whole rule stays readable on one line.
    uint8_t min_temp;
    uint8_t max_temp;

    // How far *below* `min_temp` this cell's threshold sits, drawn once per
    // coordinate and never redrawn. Zero for every row but Wood's, and one-sided
    // downwards: `min_temp` is the hardest any cell is, not the average.
    //
    // **The one-sidedness is the whole safety property, and it was bought with a
    // regression.** The first version varied the threshold both ways, which
    // reads better - wood has knots as well as soft spots. Measured across nine
    // seeds on a beam three cells thick, the shipped engine burned through all
    // nine and the symmetric version stalled one: a cell that drew a threshold
    // twenty degrees high, in a beam with only three cells of cross-section, is
    // a plug the front cannot get past before the Charred behind it burns out.
    // A fire that sometimes refuses to cross a stick is a worse defect than a
    // straight burn front, so the dice may now only ever make a cell easier to
    // light. Nine seeds, nine burn-throughs, at every jitter tried.
    //
    // **This is A9a, and it is here rather than as a jitter on the reaction's
    // chance because the two were measured against each other.** Session 3 asked
    // for a less linear burn front; the obvious fix was to make ignition a roll
    // instead of a certainty, so each cell waits a random few steps after
    // crossing its threshold. That was tried and it does not work: the front's
    // raggedness went from 0.75 cells to 1.06 while its speed fell by 2.3x, and
    // on a plank three cells thick the fire stopped propagating at all - the
    // Charred ahead of it burned out before the next cell won its roll.
    //
    // Jittering the *threshold* instead works because of the shape of the
    // temperature field rather than because of anything about randomness. Ahead
    // of a burn front the gradient is shallow - that is what "spreads slowly"
    // means - so a cell needing ten degrees more than its neighbour crosses its
    // line not a moment later but several cells later. The same size of dice
    // buys far more shape per degree, and once it is one-sided it also *speeds
    // the fire up* rather than costing nothing - which is why Wood's
    // conductivity had to come down to pay for it. See the row below.
    //
    // It also has a physical reading, which the timing version did not: wood is
    // not uniform, and a soft or dry patch is a spot with a lower ignition
    // point, not a spot that hesitates.
    //
    // Storage: none. It is drawn from the cell's coordinates with
    // `authored_pick`, the step-0 draw colour jitter uses, so it is stable for
    // the life of the world without occupying a byte of Element - which has none
    // spare.
    uint8_t temp_jitter;
};

// Wood's ignition point runs from 150 down to this much below it, cell by cell.
// See `temp_jitter` above for why the variation is on the threshold rather than
// the timing, why it only ever goes downwards, and why it is only wood that has
// any.
//
// **Bounded below rather than above, which is the opposite of what it looks
// like it should be.** Because the jitter is one-sided the hard bound is not
// Charred's 200 - no cell is ever harder to light than the stated 150, so
// nothing can become a permanent hole - it is Oil's ignition point at 90. That
// is what `lowest_ignition_point()` promises nothing may spawn above, and the
// softest cell in the world sits at 110, comfortably clear of it. Widen this
// past 60 and a puff of steam could light a wall.
//
// Measured at 24 and 40 across nine seeds. Forty gives a front 23% more ragged
// than the shipped engine's and stalls in none of them.
inline constexpr uint8_t WOOD_IGNITION_JITTER = 40;

// Heat, not luck, is what spreads a fire now. Wood and Oil ignite when they get
// hot enough, full stop - conduction from a neighbouring flame is what gets
// them there, and how long that takes is set by their conductivity in
// MATERIALS. The dice are left on exactly one row, Fire's own burnout, which is
// a lifetime rather than a threshold and has nothing to be gated on.
inline constexpr Reaction REACTIONS[] = {
    // Dousing keeps its place at the top and its chance, and is deliberately
    // *not* temperature-gated: water hitting a flame puts it out because it is
    // water, and gating it on heat would mean a cold splash did nothing.
    { ElementType::Water, ElementType::Fire,   9000, ElementType::Steam,   0, 255, 0 },
    // **Wood catches into Charred, not into Fire.** The cell that was fuel stays
    // where it was, keeps holding up whatever it was holding up, and burns; the
    // flame is thrown off it by the `emits` column and is a separate, much
    // shorter-lived thing. Before E9's rebuild this row read `Fire`, which made
    // the fuel and the flame the same cell - see PLAYTEST_LOG.md session 1.
    //
    // **150 is Wood's ignition point, and together with its conductivity it is
    // what sets how fast fire *travels* - session 2's C1.** Raised from 120, with
    // conductivity dropped 90 -> 72 alongside it; measured on a 150-cell plank,
    // burn-through went from 1923 to 2632 steps, about 37% slower. A placeholder,
    // and safe to move - but **the hard bound is Charred's `heat_source`, 200.**
    // A burning cell holds itself at that temperature, so an ignition point at or
    // above it means a fire can never light its neighbour and will not propagate
    // at all. The margin is now 50 degrees rather than 80; at 190 expect
    // propagation to be extremely slow, and at 200 expect none.
    { ElementType::Count, ElementType::Wood, 10000, ElementType::Charred, 150, 255, WOOD_IGNITION_JITTER },
    // Oil has no smouldering state; it is a Liquid, so it flashes. See its row
    // in material.h for why that boundary is where it is.
    { ElementType::Count, ElementType::Oil,  10000, ElementType::Fire,     90, 255, 0 },
    { ElementType::Count, ElementType::Water,10000, ElementType::Steam,   100, 255, 0 },
    // Condensing point, lowered from 80 alongside Steam's spawn temperature -
    // see the note on that row in material.h. Steam's whole life is the span
    // between the two numbers, so shortening one end meant moving the other.
    { ElementType::Count, ElementType::Steam,10000, ElementType::Water,     0,  26, 0 },
    // **Wood's burn duration, expressed as a lifetime.** Untemperatured on
    // purpose: a cell that is already burning is not waiting on a threshold.
    //
    // Session 2 asked for this to last one second longer, and **that request is
    // the reason the column is per myriad at all** - the two values per mille
    // could express either side of the answer were 2.78 s and 3.33 s.
    //
    // **46 was measured, not derived, and the two differ by more than they look.**
    // The arithmetic mean is 10000/46 = 217 steps = 3.62 s, but what a slab
    // actually burns for is 3.85 s: decay is only rolled for *awake* cells, and
    // the interior of a uniform body of Charred reaches thermal equilibrium with
    // itself, stops marking itself dirty, and sleeps through rolls it would
    // otherwise have lost. The closed-form number is therefore a lower bound on
    // the lifetime, and the gap widens the longer the lifetime is. Measured
    // against the same probe before and after: 2.86 s -> 3.85 s, which is the
    // second that was asked for. A real burn front is thinner than that slab and
    // sleeps less, so it will sit nearer the 3.62.
    //
    // **34 is session 3's answer to the same question asked again**, and it is
    // the one change of that session's four that nothing constrained: measured
    // across nine seeds, a Charred cell now lives 4.27 s against 46's 3.15, and
    // the 24x24 body it is part of takes 29 s to disappear rather than 22.
    // Unlike wood's conductivity and its ignition point, this knob has no cliff
    // anywhere near it and does not touch how fast fire travels at all - the
    // spread probe reads the same to within noise at every value tried between
    // 46 and 20. "How long wood resists" and "how fast fire crosses it" are
    // genuinely two dials, which is not true of the other three.
    { ElementType::Count, ElementType::Charred, 34, ElementType::Empty,     0, 255, 0 },
    // Flame's own burnout is gone from this table. It is a countdown on
    // `Element::ticks` now, in `step_fire`, because the colour ramp has to read
    // the flame's age and a dice roll has no age to read.
};

// The coldest temperature at which anything in the table catches fire.
//
// Exists to be asserted against rather than read: a material that *spawns*
// hotter than this is an ignition source whether or not anything calls it one,
// because heat_flow has a floor of one unit per step, so any temperature
// difference of two or more eventually transfers in full. Conductivity can only
// change how long that takes. See the static_assert at the bottom of material.h
// for the rule this feeds, and Steam's row for the bug that motivated it.
inline constexpr uint8_t lowest_ignition_point() {
    uint8_t lowest = 255;
    for (const Reaction& r : REACTIONS)
        if (r.result == ElementType::Fire && r.min_temp < lowest) lowest = r.min_temp;
    return lowest;
}

// **Nothing may spawn hot enough to light something, unless it is declared as a
// heat source.** Fire is the one row that is allowed to, and it says so with a
// non-zero `heat_source`; everything else has to arrive cool enough that it can
// only ever warm its surroundings, never ignite them.
//
// This is checked here rather than trusted because the bug it catches is
// entirely invisible at the call site: `spawn_temperature` is a column about
// how a material is *created*, ignition thresholds are rows in a different
// table about how a material is *destroyed*, and nothing about editing either
// one suggests reading the other. Steam sat 100 degrees over the line for as
// long as the two numbers existed. Add a material that spawns hot, or lower an
// ignition point under an existing spawn temperature, and this stops the build
// instead of shipping a world where putting out a fire spreads it.
//
// Conduction cannot overshoot - each exchange is capped at half the difference
// and stops inside the dead band - so a neighbour converges towards a spawn
// temperature and never past it. Strictly below the threshold is therefore
// genuinely safe, not merely likely to be.
namespace detail {
constexpr bool spawn_temperatures_cannot_ignite() {
    for (int i = 0; i < static_cast<int>(ElementType::Count); ++i) {
        const Material& m = material_of(static_cast<ElementType>(i));
        if (m.heat_source != 0) continue; // a declared heat source is meant to
        if (m.spawn_temperature >= lowest_ignition_point()) return false;
    }
    return true;
}
} // namespace detail

static_assert(detail::spawn_temperatures_cannot_ignite(),
              "a material that is not a declared heat source spawns hot enough to ignite something");

// The same rule applied to the bottom of the jitter's range, which is where the
// softest cell in the world actually sits. Checked separately because the
// function above reads `min_temp` and knows nothing about `temp_jitter`, and a
// jitter widened past the margin would slip under the ignition floor without
// disturbing a single assertion. That is the identical shape of bug the check
// above exists to catch - two columns in different tables, neither of which
// suggests reading the other.
namespace detail {
constexpr bool jittered_thresholds_stay_above_spawn_temperatures() {
    for (const Reaction& r : REACTIONS) {
        if (r.temp_jitter == 0) continue;
        if (static_cast<int>(r.min_temp) - static_cast<int>(r.temp_jitter) <
            static_cast<int>(lowest_ignition_point()))
            return false;
    }
    return true;
}
} // namespace detail

static_assert(detail::jittered_thresholds_stay_above_spawn_temperatures(),
              "a jittered ignition threshold reaches below the coldest ignition point in the table");
