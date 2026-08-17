# Manual Tester Checklist

The developer-facing half of testing. [README.md](README.md#general-testing) has
the short public checklist — the fundamentals, one line each, for someone who has
just cloned the repo and wants to see whether the thing works. **This file is the
long form**: the same ground, but with the regression each step exists to catch
named, the expectations that have been wrong before recorded, and the numbers a
tester needs in order to tell a known cost from a new defect.

---

## ⚠️ Owed to the tester — the current list

*Written so it can be done without reading the rest of this file. An item goes on
the moment a session asks for it and comes off the moment it is done — if this
list is empty, nothing is waiting on a human. Always clarify what my response should be*

**Build and launch:** 

cmake --build build --config Release

.\build\Release\SlopPhysics.exe

---

**One thing is owed: play V23 and tell me how the camera feels.** Added
2026-08-17. **This is the largest change to how the game looks that has been
made without a human seeing it first** — the player no longer sits at the centre
of the screen, and the framing now moves while you dig. Everything about whether
that is right is feel, and none of it is measurable from here.

**What changed, in one line:** standing still, you sit about four fifths of the
way down the screen so the ground plane gets the frame; hold the dig button
aimed downward and the camera moves until you are about a third of the way down,
so the ground you are digging into fills it instead. Let go and it returns.

**The three questions, in the order they matter.** *(This is checklist step 13,
written up in full at the bottom of this file.)*

1. **Does the movement read as the camera answering the dig, or as the camera
   wandering?** This is the one number that came from nothing — the two framings
   are read off the reference frames, but the *speed* between them
   (`EASE_PER_SEC`, currently a 0.6-second swing) is a guess. Too fast reads as a
   cut, too slow never arrives and drifts permanently. **Both failures get
   described as "nauseating", so please say which one it is**, not just that it
   is wrong.
2. **Standing on the surface — can you still see enough below you to play?**
   There are about 55 cells of world under your feet at the surface framing, and
   that number was the whole argument against doing this. If it feels blind,
   `SURFACE_ANCHOR` comes down and the plane gives some frame back.
3. **Does the ground plane finally read as a surface receding away from you?**
   That is the question three previous attempts failed at, and this is the first
   change aimed at the actual cause. **A "no" here is a real answer and I would
   rather have it than a polite yes** — it would mean the geometry was never the
   whole problem, which is worth knowing before V22 spends a scene rewrite on
   the same premise.

*Also worth reporting if you see it:* whether the camera does anything strange
at the very bottom or top of the world (it should just stop, the world edge
wins), and whether aiming the mouse without moving it makes the view creep on
its own — there is a feedback loop between the camera and the cursor that is
cut in the code and pinned by a test, but the test is arithmetic and your eye is
not.

*Closed and off this list:* the V21 brightness look came back **good** on
2026-08-17; the land-or-water decision closed on 2026-08-16 (land, with the
plane behaving the way the reference's water does — [V22](ROADMAP.md)).

*Two results worth keeping, because they close questions rather than sitting
open:* the backdrop ceiling is **settled at V21's value** — it took one step
down from V20 and the second look passed, so the "if it is still too bright the
problem is the colour, not the level" branch was never taken and the purple
question stays unasked. And **that ceiling is now a baseline**, not a
provisional setting: V22 tunes the world's grade *underneath* it, so a later
report of "too bright" means the new knob, not this one.

### A heads-up, not a task yet. **Added 2026-08-16 (V22).**

You settled the scene question by changing it: *the final product scene is the
test scene.* That is now a standing rule — **there is one scene, it ships, and
every system that needs testing earns a feature in it that a player would
actually walk past.** Nothing is owed from you on it.

> **⚠️ But it means a re-record is coming, and only you can do it.**
> When the scene is rebuilt, both saved sessions
> (`session_1_painting.rec`, `session_2_digging_fluids_steam.rec`) become stale
> — they replay into a world that no longer exists. That is caught
> automatically, so nothing breaks quietly. What it costs is that **the one
> benchmark proving the game still runs fast on a really-played frame stops
> working until you play for a few minutes and press `F9`** (how: README,
> "Recording a session"). Nothing to do today — this is here so it is not a
> surprise on the day I ask.

**Where answers go:** what you saw →
[PLAYTEST_LOG.md](PLAYTEST_LOG.md) (symptoms only, no theories). A decision →
[ROADMAP_ITEMS.md](ROADMAP_ITEMS.md#-decisions-owed). Or just tell me and I will
file them.

*Previous pass (five items on V20's palette) came back complete on 2026-08-16
and is filed at
[PLAYTEST_LOG.md, session 7](PLAYTEST_LOG.md#session-7--2026-08-16-v20s-raised-palette-and-what-the-plane-is-made-of).
Three of its five closed outright: the mountains are visible, the walking bands
are gone, and the frame rate is fine — which is the only reading `GROUND_STRIPS`
has ever had.*

---

## What this checklist is for

`ctest` proves each mechanic is correct in isolation, headlessly, one suite per concern. It cannot prove they still *compose* — that digging near falling sand near fire near water still feels and looks right together — because nothing about running in a window, taking real input, and rendering a frame is exercised by a suite that never opens one. The checklist below is the other half. It is not the playtest gate in `ROADMAP.md`'s Medium Term section, which is about whether the game is *fun* for someone who did not build it; this is about whether it is still *correct*, run by whoever just built the feature, in the two or three minutes before calling it done.

**Run this after any change that touches `src/physics/`, `src/game/` or `main.cpp` and is not fully exercised by the automated suites** — which in practice means anything touching rendering, input, or feel, since those are exactly what a headless test cannot see. Skip it for test-only or documentation-only commits; there is nothing here those could break. Each item names the regression it exists to catch, most of them things that have actually gone wrong once already in this project — a floating pile, a seam at a chunk border, a fire that never dies — so a "looks fine" pass is a real signal, not a formality.

## The steps

1. **Launch.** `cmake --build build --config Release`, then run the exe. Window opens, a `World seed: N` line prints to stdout (the seed check has no way to fail silently: if the number is missing, `main.cpp` stopped being the project's one nondeterministic line). The HUD in the window's top-left corner shows fps, current material, brush size, and chunks awake — the window title bar is now a plain, static label, not where this lives. The world is no longer empty at launch: `main.cpp` loads the authored F4 test scene (`assets/test_material.bmp` / `test_albedo.bmp`) over it first, so confirm terrain is visible immediately — a snowbank, fence posts, a bridge over a pit, a water channel — rather than a blank grid. **A `Scene: 1920x1080, 334901 cells placed` line prints alongside the seed, and that is the check rather than the eyeballing.** This step used to be eyeballed and it silently stopped being true for a whole commit: retuning the palette changed the colours the material map was matched against, every authored pixel resolved to `Empty`, and the game booted blank while all six suites passed. A count of zero, or a `WARNING` about unrecognised legend colours, means the scene file and [src/scene/legend.h](src/scene/legend.h) have come apart.

2. **Movement (`Player`).** Walk both directions, jump, land. Confirm the body rests flush on top of Wall and on top of settled Sand — no half-cell sinking, no hovering. **The sprite is 14x26 over an 8x20 collision box, so "flush" is a claim about the sprite's *feet*, not its bounding rect** — the mask overhangs above the box and the sleeves outside it, both by design. A figure that hovers one cell above every floor means some frame's bottom row went empty — `python tools/player_sheet.py --validate` checks exactly that, per frame, and is the first thing to run; a figure sunk into the floor means `src/render/player_sprite.h` is stale and needs `--header` re-run. Also confirm the figure turns to face the direction you walk, and keeps facing that way after you stop.

   **Animation, which is the half no suite can see.** `anim_test` pins which animation each state selects and that the clock advances correctly; what it cannot see is whether the result looks right. Walk both ways and confirm the cycle plays and the feet do not slide. Stop and confirm it settles to idle rather than freezing mid-stride. Jump and confirm the pose changes on the way up and again on the way down, and does not flicker at the apex. Dig and confirm the swing plays once and completes — then spam clicks and confirm it never sticks mid-swing. **Then press into a wall and hold the key**: the figure must stand, not walk on the spot, which is the whole reason the selector reads `velocity_x()` rather than the input. **Hold the dig button on solid terrain rather than clicking once**: the swing must *cycle* for as long as the button is down, at a visibly heavier pace than the walk — this is the D1 check and the failure it replaces was the figure pinned on the first frame of the swing until the button came up. Watch for the seam, too: the next swing starts on the step the last one ends, so there must be no hitch at the top of each one. **A swing is now a real duration and the dig rate is tied to it** — under two digs a second, deliberately. If it reads as feeble, the note in [ROADMAP.md](ROADMAP.md)'s wave 4 says the radius is the knob, not the rate. **Finally, if you can run the window at a different frame rate, confirm the walk cycle's speed does not change** — the clock is the fixed step for exactly this reason, and a cycle that speeds up with the display is the defect [src/render/player_anim.h](src/render/player_anim.h) exists to prevent. Walk it up a one-cell sand step without jumping ([The player](README.md#the-player) — `MAX_STEP_HEIGHT`). Confirm it cannot walk through Wall, Wood, or a settled sand pile.

   **Flight, which nothing above ever asked for and which is a shipped feature.** Hold the jump key in mid-air. The character is a bird: the key should beat wings on a rhythm rather than doing nothing or reading as a thrust — discrete downstrokes you can count, at 4 pixels per cell, are the whole point. Confirm a standing jump is still the strongest single upward move you have, that sustained climbing is a slow laboured grind rather than a helicopter, and that arresting a dive costs several beats and visibly loses height while you do it. All three are properties of the *margin* between `FLAP_IMPULSE` and the gravity a beat has to pay for, not of any one constant ([TUNING.md](TUNING.md), Flight weight), so "feels heavy" and "feels weightless" are the same row read from two sides.

   **This row and the two above are what F5 (2026-08-12) is owed a look for.** The player's velocities became fixed point instead of float that day, and the recorded traces say motion did not change — walk and jump are step-for-step identical, and fall and flight differ on 7 of 1381 steps, each by one cell and each re-converging immediately. **So the expected result of this whole step is "exactly as before".** Anything you can actually feel is a real finding, and flight is where to look first, since five of those seven steps were in sustained flight.

   **Then walk into a settled sand pile several cells tall and judge whether climbing it costs you anything.** A one-cell step is the *floor* of what `MAX_STEP_HEIGHT` permits and this row used to check only that, so the constant itself had never been tested by the thing that exists to test it. Playtest session 5 reported the result as "walking into a settled pile does nothing, the player walks over it", filed as an observation about the player/material relationship before it was traced to a number. **A body that climbs a quarter of its own height instantly, with no animation and no slowdown, reads as terrain not being there at all.**

   **`MAX_STEP_HEIGHT` was lowered from 5 to 3 on 2026-08-12, so this row now has two opposite failures to watch for, and the second one is the new risk.** Climbing a pile should cost you something — but a settled slope is a staircase of *one-cell* steps and must still be walkable at a steady pace. If sand you could previously stroll up now stops the body, or makes you jump repeatedly to get anywhere, the change went too far. `player_test` holds the headless half of this (it walks the body over a settled pile and asserts it gets across, at whatever the constant happens to be); what it cannot judge is whether the climb *reads* as effort or as an obstruction. That half is this row.

   **Finally, stand under a sand pour with a wall at your back and confirm you stay on your side of it.** The simulation does not know the body exists, so material lands in the cells it occupies and `Player::resolve_overlap` moves the body to the nearest position it fits in — **and that search checks only that the destination is clear, not that anything solid sits between**. Session 5 found it relocating the player through walls under a continuous pour (defect D2). `test_player` asserts the fixed behaviour headlessly; what it cannot see is the shove itself, which fires every step while you are buried and takes your control with it.

3. **Digging (`DigTool`).** Left-click cuts a circular hole at the crosshair. Aim at something past `RANGE` and confirm **the crosshair dims from solid white to dim white** — that is the range check now, and it reads at the cursor rather than needing a second marker elsewhere on screen. **Dim does not mean "nothing happens", and the checklist should not expect it to:** the ray still travels its full `RANGE` along that line and cuts the first solid cell it meets, so a dim crosshair aimed past a *near* wall still digs that wall. What dim means is that the cell under the crosshair is not the cell that gets hit. Confirm both halves — a dim shot into open sky beyond range does nothing, and a dim shot lined up through nearby terrain still bites it. Dig the bottom out of a standing Sand pile and confirm everything above it falls in — a gap left hanging is the classic dirty-rect bug ([Chunked updates](README.md#chunked-updates)). Fire at a wall from behind cover and confirm the shot stops at the near face rather than tunnelling through to whatever is behind it.

4. **Materials and brush.** Cycle all eight keys (`1`–`8`) and confirm each paints the right colour and the HUD's material name agrees. Sand piles into a slope. Water spreads flat. Pour Water into one side of a container whose two halves are joined only at the bottom and confirm both sides come level and then *stop* — a surface that keeps trading cells back and forth is level on average and never sleeps ([Liquids find their level](README.md#liquids-find-their-level)). **Then pour an untidy amount of Water onto flat ground and watch the HUD's chunk count go to zero.** Any puddle whose cell count did not happen to divide by its container's width used to shimmer forever, which is almost all of them; the leftover cells now come to rest as one patch, level to within a cell. The failure mode to watch for in the other direction is a puddle that settles into a visible *mound* instead of spreading — the sideways walk that jitters is also the one that flattens, and refusing too much of it trades a shimmer for a heap. Oil floats on Water rather than mixing into it. Steam rises and pools at the ceiling instead of the floor. Eraser (`8`) clears back to `Empty`. **Confirm the hotbar's highlight lands on the box you pressed**, and that each icon depicts the material its key actually places — the bindings and the icons come out of one table specifically so they cannot disagree, and this step is what checks that claim.

5. **Reactions and heat.** Ignite a Wood block with Fire (`7`) and watch it catch, spread, and eventually burn down to `Empty`. Same against Oil — should catch sooner. **Watch a long beam rather than a block**: what you want to see is a front that advances along it, not the whole beam lighting up at once and not a fire that stops dead after one cell ([Heat](README.md#heat)). Drop Fire next to Water and confirm the water boils off into Steam as well as dousing the flame. **Then leave the Steam alone under a solid ceiling and watch it for several seconds** — the expectation changed on 2026-08-12 with E9's steam half and the old one is recorded because a tester following it would file the fix as a defect. It used to be "condenses back to Water in about a second"; **it is now: the puff rises, gathers against the ceiling, sits there for a few seconds, and then drips — single cells of Water letting go of the ceiling and falling, the pocket shrinking from the top as they do, a wider pocket dripping faster than a narrow one.** What says it is working is the *waiting*: steam that turns to water almost as soon as it arrives is the defect this replaced, and it was reported three times in four sessions (A5, B3, D5) before anyone found that its life was being measured in degrees rather than in time. The failure in the other direction is a pocket that never goes at all, which would mean the contact rule is not seeing the ceiling. **Then do it under a wooden roof**: build a Wood ceiling, put Fire and Water beneath it, and confirm the Steam that results does *not* set the ceiling alight. Steam used to spawn a hundred degrees over Wood's ignition point, so putting a fire out was a way of starting a bigger one — and it only showed when the steam was confined, which is why open-air testing never found it and authored terrain would have. Place one Fire cell with nothing nearby and confirm it burns itself out on its own. **Then box a Fire cell in on all sides with Wall and confirm it still burns out** — this is the self-wake regression specifically: a fire with nowhere to move and nothing to check its own decay against will otherwise freeze forever with its chunk asleep ([Reactions](README.md#reactions)).

6. **Chunking / sleep-wake.** Let a mixed scene (sand, water, a fire) run to rest and confirm the awake-chunk count in the HUD returns to at or near zero — a nonzero idle count means something is being woken that should not be. **Burn something first, then wait**: heat keeps cells awake while it is still moving, by design, so the count should stay up for a while after the flames are gone and then come down as the scene cools. A count that never comes down means heat is not settling, which costs full price forever and is invisible to look at. Build a flat sand floor wide enough to cross a chunk boundary (chunks are 64 cells) and confirm it settles into one continuous surface with no step or seam at the boundary.

7. **Structures ([Structures and falling](README.md#structures-and-falling)).** Place a Wall or Wood shape with nothing under it and confirm it falls as one rigid piece, keeping its shape, rather than crumbling into loose grains or hanging in the air. Rest a shape on solid ground and confirm it stays put indefinitely — no spontaneous twitching. Dig one support cell out from under a large structure and confirm the whole thing drops promptly and lands clean, nothing left floating. **Then build a ledge with a drop beside it and cut a wide slab loose above the join**: it should come apart over the edge rather than perching level across it, and the two halves should stay apart afterwards. The negative half of that is the one to be fussy about — nothing that was standing still should ever break, so watch a settled structure through several of these and confirm it never so much as shifts.

8. **Performance sanity.** Paint a large, actively-falling scene and watch the HUD fps. **Know what to expect before you judge it, because this step used to ask for something the engine cannot do.** It said to expect fps "near the display's refresh"; P2 then measured the played world at 1920x1080 and a wide sand-over-water fill — which is close to `grid_bench`'s `churning` — costs **35 ms per simulated step there, about 2.1 frames of a 60 Hz budget**. A scene like that visibly bogging is the known cost of the played size, not a new defect, and a tester following the old wording would have filed it as one.

   What this step is actually for is a *change* in that behaviour. Judge it against `PERFORMANCE.md`'s table: `sparse` — a large static world with a small patch of action, i.e. ordinary play — is 0.4% of a frame and should feel like nothing at all, and `burning` is 44%. **If ordinary play bogs, that is a real lead**; if a deliberately pathological sand-over-water fill bogs, that is the table being obeyed. Either way it is a lead and not a verdict — follow it up with `grid_bench`, bracketed, per `PERFORMANCE.md`; a felt slowdown on its own is exactly the kind of unbracketed reading that document warns against trusting.

9. **Stability.** A few minutes of doing several of the above at once — digging near falling sand near fire near water, brush strokes back to back, movement keys held through a collapse — without a crash. There is one unexplained `0xC0000409` on record, seen twice under heavy machine load and never reproduced; if it recurs, note what else was running on the machine at the time and fold it into the crash-diagnosis item in `ROADMAP.md`'s Presentation & Tooling section rather than letting it evaporate again.

   **Covered on 2026-08-13 by the session 2 recording, on three of its four activities, and the fourth is named rather than assumed.** 5 minutes 40 seconds of play: 479 dig steps, 9,158 brush steps, `Fire`+`Water`+`Sand` all present in 198 of 341 samples with digging inside 11 of them, no crash, and the whole thing replayed byte-exact afterwards. **What is not covered is "movement keys held through a collapse"** — the player moved on 647 steps, but a collapse is not a material and the census has no column that could show one, and the tester did not watch for it. So this step is **passed with that clause outstanding**, which is a different thing from passed.

   **Two limits on reading a recording as this step, worth knowing before doing it again.** The census reports materials **co-present in the world**, not *near* each other — this step says "near", and world-wide co-presence is the weaker claim. And a recording can only ever show the step's *crash* half; the reason step 9 is a manual step is that a person also notices things that are wrong but not fatal, which no replay will report. **A recording strengthens this step and does not replace it.**

10. **The run can be lost (`S0`, new 2026-08-14).** The other nine steps ask whether the engine is still correct. **This one asks whether there is a game**, and it is the only step on this list whose result is a design decision rather than a pass or a fail — the combat question in [ROADMAP_ITEMS.md](ROADMAP_ITEMS.md#-decisions-owed) is due on it.

    **The mechanical half, which is what can actually regress.** At launch the HUD reads `HP:100` and a `GOAL:` bearing, and an `Objective: (1700, 932)` line prints beside the seed — a missing objective line means the column scanned no ground and the run cannot be won. Confirm the spawn drop costs nothing: the body falls several hundred cells at startup and must land at `HP:100`, because that fall is priced at 80 of 100 and is free by an explicit rule. Then **jump repeatedly and confirm jumping never costs health** — that is the rule most likely to break silently, since it is a relationship between two constants and only one of them is in this file. Walk off the diving ledge above the water channel and confirm a real drop *does* cost health. Paint Fire (`7`) onto yourself and confirm the bar comes down at about a fifth a second, then step out and confirm it stops; paint Steam (`6`) onto yourself and confirm it does **not** — steam spawns twelve degrees under the burn threshold, deliberately, and that gap is the one a MATERIALS retune could close without anybody noticing.

    **Then lose, and then win.** Stand in fire until the bar empties: the world freezes, `YOU DIED` reads over it, and the terrain behind stays visible — a panel that hid the world would hide the whole content of the ending. Press `R`, confirm the run starts over with the terrain restored, the body at full health and the objective still there. Then go and reach it: the objective is east across the water channel, which is walled on both sides and **cannot be walked across** — flight is the intended answer and this is the first thing in the built game that has ever required it. Confirm `OBJECTIVE REACHED` and that `R` works from there too.

    **What to be fussy about, in order.** *`R` must do nothing while the run is playing* — it is inert on purpose, and a mid-run restart would be a session thrown away by a mis-hit. **`Ctrl`+`R` is a different key and does reset mid-run**, added by `T1`; that is deliberate and not this step failing. *The `GOAL:` bearing must count down as you approach*, since a bearing that does not is worse than none. *And a body dug out of burning terrain must still be burning*: the burn rule sits deliberately above the unstuck path, so being buried in fire is not a way to stop taking damage.

    **The half that is not a check.** Play it as a run rather than as a test — spawn, cross, arrive — and then answer the question the item was built to ask: **does this need an enemy to be interesting?** Neither "yes" nor "no" is a failure and both close a decision that has been open for months. Record the answer in [ROADMAP_ITEMS.md](ROADMAP_ITEMS.md#-decisions-owed) and the symptoms in [PLAYTEST_LOG.md](PLAYTEST_LOG.md), the usual way round.

11. **Depth and parallax (`V11`, new 2026-08-16).** `golden_frame_test` hashes a composed frame, so the *order* of the layers and the *position* of each one are checked on every build — but it hashes a software rasterisation of a still frame at one camera position. **Parallax is a thing you can only see by moving**, and depth is a thing you can only judge by looking, so both halves of this step are outside what any suite can reach.

    **The mechanical half.** Walk a long way in one direction, on the surface, and watch the backdrop. The sky must drift slowest, the mountains faster than the sky, and the terrain fastest of all — that ordering is the entire feature and a layer moving *with* the world means its factor has been lost somewhere between `tools/generate_backdrop.py` and `Camera::parallax_origin_x`. Then jump and fall a long way and confirm the same holds vertically at a gentler rate. **Then walk to the far east and west edges of the world and look at the top and bottom of the screen**: this is the seam at the pan limit, where a backdrop layer runs out of image before the camera runs out of world and a band of flat clear colour appears. It should not happen, and if it does there will be a `WARNING: backdrop ... needs at least WxH` line on stderr from launch that says which layer and by how much — check stdout before hunting pixels.

    **The wrapping layer, which is a second and different way to make a seam (`V19`, added 2026-08-16).** The ground plane behind the world is not an image covering the pan range — it is a **tile repeated across the window**, so it cannot run out of image and the check above does not apply to it. What it can do instead is show a **vertical join** where one copy meets the next, at any camera position rather than only at the map's edge. Walk slowly across the surface and watch the plane below the horizon: the dashes on it should stream past continuously, with no repeating vertical line and no one-pixel column of the mountains showing through. **The plane is also drawn as 24 horizontal strips at 24 different parallax rates**, so there is a third thing to look for that no other layer can produce — a *horizontal* stair-step between strips, or a strip sliding the wrong way relative to the one above it. The near edge must always scroll faster than the horizon edge; if any part of the plane scrolls backwards relative to the part above it, the recession has inverted locally and the strip arithmetic is wrong.

    **What this step found on 2026-08-16, because it changes what you are looking at (`V20`).** The first run of the paragraph above came back "there seem to be some visual bugs with black bands appearing in between the plane pixels" and "mountains are not visible just the plane". Both were real and both are fixed, and **the second one was not a seam at all** — the plane's far edge was authored as a fraction of the *window* while the mountains were authored in their own image's coordinates, the two contradicted each other at every camera position, and since the plane is opaque and drawn after the mountains it simply covered them. So this step gains a check it did not have: **the plane's far edge must always sit below the mountains' skyline, at every height you can reach.** Walk to the surface, then fall as far as you can, and watch the join. If the ground ever climbs over the ridge, the horizon derivation has come loose from the art — it is `ground_horizon_y` in [src/render/frame.cpp](src/render/frame.cpp) and it should not be tunable.

    **The horizon, which is a standing check rather than a pass/fail.** V11 built a **mid-ground band** between the mountains and the world, at parallax 0.40, because five of eight reference frames spend most of their depth there — and removed it on 2026-08-16 when this step was run and the answer came back that **our terrain already fills that part of the frame** ([notes/reference_observations.txt](notes/reference_observations.txt) entry 4, which predicted its own disproof and got it). Nothing is owed here now. What this step keeps is the **reopen trigger**: if you are ever looking at a location whose terrain does *not* fill the space between the mountains and the ground — a flatter scene with a low horizon, or a zoomed-out camera once `Camera::SCALE` is runtime — say so, because the band becomes worth building again and the layer table makes it one row.

    **Do not judge depth by counting bands.** Our depth bands used to overlap almost completely in brightness, which is why a busy frame read flat. That was a renderer defect and **it was fixed on 2026-08-16** (step 3 of the V block) — the light pass could only add, and separating bands needs a multiply. **A frame that reads flat is that defect and not a missing layer**, which is exactly the mistake the mid-ground band nearly shipped as a fix for. That inference is still the right one; what has changed is where it points.

12. **The mountains read as a silhouette (`V11` step 3, new 2026-08-16).** This is the step that judges the one deliberate change to the composed frame, and it needs a **surface location with open sky and the mountain band visible** — anywhere you can see the horizon over distance.

    **What should be true.** The mountains should sit *darker* than the sky behind them and read as a cut-out shape against it. Before this change they measured luminance 28 against a sky of 26 — two levels of separation out of 255, and the far band was the brighter of the two, which is why the horizon was mush. They now carry a 0.60 multiply and should measure about 16 against 26.

    **⚠️ The luminance figures in this step are pre-V20 and none of them is current.** On 2026-08-16 the backdrop palette was raised wholesale — the whole frame had been occupying 9 levels out of 255 — so "16 against 26" is now roughly **44 against 62**. The grades themselves did not move; what they multiply did. The *arguments* below all survive the raise and the *numbers* do not, so read them for shape and re-measure before quoting a figure. Full entry at V20 in [ROADMAP.md](ROADMAP.md).

    **⚠️ And the question this step asks has flipped.** It was written to catch the grades over-correcting *downward*. V20 raised everything they multiply, so **the live risk is now the opposite: a frame that is too bright to be a night scene at all.** That is what to look for first, and it is item 1 of the owed list at the top of this file.

    **The failure to look for is over-correction, not under.** The number was chosen from a measurement rather than by eye, and those are not the same thing — **it was looked at for the first time on 2026-08-16 and came back good**, so this step is now a standing regression check rather than an open question. If the ridge reads as a black hole punched in the sky, or the mountains have stopped being visibly *mountains* and are just a dark edge, the value is too low; the knob is the `mountains` row in [src/render/frame.cpp](src/render/frame.cpp)'s layer table and the row in [TUNING.md](TUNING.md). **Retuning it moves `golden_frame_test`'s checksum, which is correct** — put the new value in the same commit.

    **The ground plane is the second graded band (`V19` step 4b, 2026-08-16) and it is judged the same way.** It sits behind the world, below the horizon, at a 0.53 multiply. What should be true: it is **darker than the sky at its far edge and brighter at its near edge**, and the darkest line anywhere in the frame is where it meets the horizon. Measured on the art it runs 0.44 to 0.81 of the sky's luminance; by eye that is "the ground fades into the horizon and comes up to meet you". **The failure this exists to catch is the plane and the sky reading as the same surface**, which is what the played frame showed before this step — 22.7 against 22.3, a flat fill the size of a third of the screen. If the plane looks flat rather than receding, the fault is the tile's ramp and not the grade, because a grade multiplies uniformly and cannot brighten one end of a band.

    **That last sentence was right and the ramp was the fault (`V20`, 2026-08-16).** The first run of this step came back "the effect isn't very convincing", and the tile measured a ramp of **9.8 levels against the reference plane's 61** — with its near third, where the recession is supposed to be strongest, completely flat. Two causes, both in the art: the palette had nowhere to ramp *to*, and the tile was an ordered dither between exactly two colours, which saturates. It is now ten distinct tones running **30 to 81** after the grade. The grade is still not the knob for this; the tile is.

    **Also confirm nothing else moved.** Only the mountains and the ground plane are graded. The sky, the terrain, the trees, the props and the player are all at 1.00 on purpose, so if the *whole frame* looks darker rather than just the ridge, the grade has landed in the wrong place — check whether `Params::world_grade` has acquired a caller.

    **And check a fire at the horizon if you can arrange one.** The grade pass is drawn *before* the additive light pass specifically so that a fire is not dimmed along with the scene. `golden_frame_test` measures this headlessly and a `static_assert` holds the ordering, so this is a confirmation rather than a hunt — but it is the one visible consequence of the ordering argument, and it is worth having seen once.

13. **The camera leaves screen centre, and digging brings it back (`V23`, new 2026-08-17).** **The newest step and the one with the least behind it** — every other step in this list names a regression that has actually happened, and this one names a change nobody has seen. Needs an open surface location with diggable ground under it, which the spawn is.

    **What you should see.** Standing still, you sit roughly four fifths of the way down the screen rather than at its centre, with the backdrop's ground plane taking most of the frame above you. Hold the dig button with the cursor below you and the view should move — over about six tenths of a second — until you are around a third of the way down and the ground you are digging into fills the frame. Release, and it returns at the same rate. Digging **sideways or upward should barely move it at all**: the camera is supposed to respond to the volume going *underneath* you, not to the dig button.

    **The failure modes, which are not symmetrical.** Too fast reads as a cut rather than a move; too slow means the camera is still arriving when you have stopped digging, so it never settles into either framing and just drifts. **Both get called "nauseating", so name which one.** The speed constant is the one number in this feature derived from nothing at all — the two framings come from the reference frames, the duration is a guess.

    **The cost this step is really watching.** At the surface framing there are about **55 cells of world visible below your feet**, down from ~135 at centre. That is the argument that nearly stopped this being built, and the dig framing is what pays it back. So: is normal play — walking, jumping, aiming at something below you before you start digging — noticeably blinder? A yes here is not a defect report, it is `SURFACE_ANCHOR` being too high.

    **Two edges worth a deliberate look.** Walk to the very bottom and the very top of the world: the view should simply stop at the world edge, with the anchor losing to the clamp, and never show anything outside the grid. And **hold the mouse perfectly still while digging downward** — the view must not creep on its own. There is a genuine feedback loop there (the camera moves the view, the view is what the mouse position is resolved through, the resolved aim is what moves the camera), it is cut in the code and pinned by a test with a negative control, but the test is arithmetic and this is the observation it stands in for.

    **Finally, the question the whole V-track has been failing at:** does the ground plane now read as a surface receding away from you, rather than as a band sitting behind you? Three previous attempts tried to buy that with colour and could not, because the plane was 100% occluded by the world in front of it — this is the first change aimed at the cause. **A "no" is a valuable answer here** and should not be softened: it would mean geometry was not the whole problem, and V22 is about to spend a scene rewrite on the assumption that it was.

---

## Where results go

Symptoms — what you saw, not what you think caused it — go to
[PLAYTEST_LOG.md](PLAYTEST_LOG.md). Root causes and fixes go to
[ROADMAP.md](ROADMAP.md), in the wave that spent them. A design answer (step 10)
goes to [ROADMAP_ITEMS.md](ROADMAP_ITEMS.md#-decisions-owed). A retuned feel
constant goes to [TUNING.md](TUNING.md) as a row **and** a dated History line.

**When a step's expectation turns out to be wrong, fix the wording here rather
than working around it.** One step once told testers to expect a frame rate the
engine cannot deliver, which would have had them file the known cost of the
played world as a new defect.
