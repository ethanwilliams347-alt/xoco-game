#pragma once

#include "render/backdrop_wrap.h"

// The band table for `bg1_08_ground`, the one layer of the `art_src/Background_1`
// set that is a *surface* rather than an object (`V28c`, 2026-08-26).
//
// **Why this is a header and not three literals in main.cpp.** It knows no SDL,
// so `boot_test` can include it and check every number in it against the BMP it
// describes - which is the only enforcement available here, since no headless
// suite composes an authored frame and the defect these numbers fix is one a
// person has to see.
//
// --- what the tester saw -----------------------------------------------------
//
// After V28b locked the stack vertically: *"the midground layers look like they
// move horizontally on the ground plane."*
//
// V28 gave the plane a single 0.52 - the near end of the art README's
// `0.28x - 0.52x` ramp - while the hills standing on it run 0.20 to 0.70. Over
// the world's 152 cells of horizontal travel (1520 px at 10x) that is up to
// 334 px of slip between a hill's foot and the ground under it: a tenth of the
// scene's width. **A single factor on a receding surface is the defect, not the
// value of the factor.**
//
// --- why it is fixable, and the measurement that says so ---------------------
//
// `bg1_08_ground` is uniform across all 344 columns on 61 of its 81 opaque rows.
// The only horizontal detail is three wavy shore contours, and only two of them
// have anything standing on them:
//
//     rows  70..77   shore 1   hills_midfar's foot lands at art row 73 (0.30)
//     rows  87..97   shore 2   hills_near's   foot lands at art row 89 (0.70)
//     rows 102..106  shore 3   nothing stands on it
//
// So the plane is cut into three bands, each carrying one contour at the factor
// of whatever stands on it, **and every cut is placed inside a run of rows that
// is uniform in colour** - 78..86 and 98..101 both are. A band boundary is a
// discontinuity in scroll offset; put it where both sides are the same flat
// colour and it is invisible at every camera position. That is why the
// boundaries are 82 and 100 rather than round numbers, and it is the one thing
// here that `boot_test` can and does check.
//
// --- why not a smooth ramp ---------------------------------------------------
//
// The obvious alternative, and it is worse by a number. The contacts demand
// about 0.025 of factor per art row between rows 73 and 89, so a contour ten
// rows thick would have its top and bottom scrolling 0.25 apart - 380 px of
// shear across its own thickness over the world's travel. The shorelines would
// tear sideways. V19's generated plane ramps smoothly because its texture is
// noise with no feature wide enough to shear; this one is paint.
//
// --- the third band ----------------------------------------------------------
//
// 1.00 because the near field is where the foreground rocks and the diggable
// terrain are, and both are locked to the world. Same argument V28b makes
// vertically, and the same cap: `draw_backdrop_layer`'s coverage inequality
// needs every factor <= 1.
namespace bg1 {

inline constexpr backdrop_wrap::Band GROUND_BANDS[] = {
    {  0,  82, 0.30f},   // far water + shore 1; hills_midfar stands here
    { 82, 100, 0.70f},   // shore 2; hills_near stands here
    {100, 144, 1.00f},   // near field; the rocks and the terrain live here
};

// The art's native size, which the table above is stated in.
inline constexpr int NATIVE_W = 344;
inline constexpr int NATIVE_H = 144;

} // namespace bg1
