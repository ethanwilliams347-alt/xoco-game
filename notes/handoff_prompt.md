# Handoff — V19 mid-item (4c next), with V20 just shipped on top of 4b

You are picking up a **visual-rework block** on Toop/Xoco, a from-scratch C++20 /
SDL2 cellular-automata pixel-physics game. Repo root is `code/`; **run every
command from there.** Read `code/CLAUDE.md` first — it is short, and it is the
working agreement rather than a summary.

## The one rule that governs this block

**In this project the reasoning is the deliverable, not a courtesy.** A change
that is correct but arrives without its argument recorded is half-done. A stated
rule that has gone false is worse than no rule, because it stops people checking.
Almost every constant and refusal here has a written argument behind it, usually
naming the bug that bought it — so **before changing something that looks
arbitrary, find out whether it is.**

## Standing constraints, carried verbatim from the user

- **Do not spawn subagents unless asked.**
- **Do not use workflows or deep-research unless asked.**
- **Everything outside this block is held, not cancelled** — E10 keeps the head
  of the queue when the block closes.
- **Never grow a parallel compositor in the golden-frame test.**
- **Do not attempt band separation as a PALETTE edit.** ⚠️ **This constraint
  gained a boundary on 2026-08-16 and the boundary is now the load-bearing
  half** — V20 raised the backdrop palette wholesale and that was *not* the
  refused move. The refusal is about the ladder **between** bands, which belongs
  in the grades. Raising the **ceiling all the bands hang from** is something no
  grade can do, because `Grade` is a multiply and can only darken. The full test
  for next time, with V20's own honest exceptions listed, is in
  `.claude/rules/assets-and-formats.md` — read that bullet before touching a
  backdrop colour.
- **The Manual Tester Checklist is owed and you cannot run it** — flag when it is
  owed and name which steps matter. **It lives in `MANUAL_TESTING.md` now, not in
  README** (moved 2026-08-16); README's `## General Testing` is the short public
  pass and carries none of the reasoning.
- **`MANUAL_TESTING.md` opens with "Owed to the tester" and that list is
  maintained, not appended to.** Put an item on it the moment you ask for one,
  take it off the moment it comes back, keep it tidy, and **write it so a
  beginner can act on it without reading the rest of the file** — the user asked
  for that specifically after a version that only cited step numbers.

## ⚠️ Read this before starting anything

**Five items are currently owed to the tester** (`MANUAL_TESTING.md`, top of
file), all of them V20's, and **nothing in this block should move until they come
back.** V20 is a large deliberate aesthetic swing that no test can judge, and one
of the five explicitly invites "you went too far" as an answer. If the brightness
has overshot, 4c would be authored against a ladder that is about to change.

The five, in short: is it still a night scene; are the mountains visible now; do
the black bands still appear while *walking*; does the plane read as receding;
is the frame rate still fine.

## Where the block stands

`ROADMAP_ITEMS.md` item 8 is the authority; five steps, strictly in order.

| step | what | state |
|---|---|---|
| 0 | Rewrite the two dead `notes/` files against the CnC reference frames | ✅ done |
| 1 | **V17** — extract composition to `render/frame.cpp`, then checksum it | ✅ done |
| 2 | **V11 core** — ordered layer table, parallax onto `Camera`, generated header | ✅ done |
| 3 | **V11's tint bullet + V7-rest's darkening half** — the light pass gains a multiply | ✅ done |
| 4 | **V19 — the seven-band scene with a ground plane** | **in progress — 4a, 4b done; V20 fixed 4b; 4c next** |
| 5 | **V18 — write the split view down, build none of it** | queued behind V19 |

**What `git status` says is the truth about state, not this table** — V20 is
built and green and its commit boundary at the time of writing is whatever the
working tree shows. Run it.

## V20 — what just happened, and why 4c inherits a different frame

Playtest session 6 was the first human eyes on 4b and returned three visual
defects. **The direction question came with it — "are we going down the wrong
route for the Cast n Chill graphics" — and the answer is no.** Every mechanism
`reference_observations.txt` entry 7 identifies is built and none is
misconceived. The frame was running them at roughly a sixth of the amplitude and
then multiplying them down again.

Full entry at **V20 in `ROADMAP.md`**; symptoms only in `PLAYTEST_LOG.md` session
6. The three things 4c has to know:

- **The palette moved and the grades did not.** The whole frame had been
  occupying **9 luminance levels out of 255** — smaller than the reference's
  *smallest single band join* — because the ladder was built downward from a sky
  authored at L 18 and `Grade` can only darken. Post-grade now: sky 95 → 62,
  mountain rim 71, mountain body 44, ground far **30** (the frame's darkest
  value), ground near 81. `mountains` 0.60 and `ground` 0.53 are untouched.
  **⚠️ Every luminance quoted in `TUNING.md`'s two grade rows, in checklist step
  12, and in V19's own ROADMAP entry is pre-V20 and none of them is current.**
  The rows carry the warning; re-measure before quoting a figure.
- **`GROUND_HORIZON_FRACTION` is retired, not retuned.** The plane's far edge is
  now derived from `backdrop_layers::MOUNTAINS_SKYLINE_MAX`, generated from the
  same seeded walk that draws the silhouette, and it moved onto the *mountains'*
  vertical parallax factor. **A receding plane's far edge is at infinity, so its
  factor has to be the smallest in the scene, not the plane's near-edge one** —
  it had the largest, and climbed over the band it recedes toward. `ground_horizon_y`
  in `frame.cpp` is deliberately not tunable.
- **`dither_mix` picks between exactly two colours**, so every "ten band" ramp in
  `generate_backdrop.py` contained two colours and ten *proportions* of them, and
  saturated. `banded_ramp` replaces it with ten distinct flat tones. **4c's three
  bands must use `banded_ramp`** if they ramp at all.

### The trap V20 fell into, which 4c can fall into identically

The horizon was briefly derived from an **absolute row** of the shipped 1642-row
mountains BMP. The golden fixture builds a *300-row synthetic* mountain texture,
so that put the entire plane below the fixture's window: the frame composed
cleanly, **every check in `test_golden_frame.cpp` passed, and the checksum
silently reverted to a value from before the plane existed.** It was caught only
because that value was recognised.

`test_golden_frame.cpp` now asserts the plane's presence by composing once
without its texture and requiring a different frame. **Every band 4c adds needs
the same guard**, and the general form is: `.claude/rules/simulation.md` warns
that a checksum over a null-textured layer covers the layer's *absence*, and this
is the same hazard reached by geometry instead. A layer stated in one image's
coordinates will not survive a fixture built at another size — **state it as a
fraction of the loaded texture.**

### The checksum

Now **`0xcde4dc1a39927fca`** (sixth move). V20 **set the house "no-op half
first" procedure aside rather than claiming it** — three causes that all move
pixels and none stageable at identity — and said so plainly in the constant's
comment, because a procedure claimed and not followed is worse than one openly
set aside. The separation it lacks is carried by `backdrop_test`'s two new
properties instead.

**4c has a no-op half available and should use it** — see the two-number
adaptation recorded at V19's entry: a new band that draws pixels composes at
identity first, then takes its grade.

## Next step — 4c, the remaining three bands

The far range, the near ridge and the shore treeline: three rows reusing 4b's
draw path, one colour and a shade each, generated the same way. **The gate that
admits the last two is passed and written down — do not re-ask it.** (V11 deleted
a mid-ground band, wrote the reopen trigger "a location whose terrain does not
fill the band", and V19 fired it by screenshot on 2026-08-16: the mid band
measured 70% bare sky. Entry in `PLAYTEST_LOG.md`.)

Entry 7's value ladder is what the grades are authored against:

```
band              L      x band behind    x sky
sky            173.6         -            1.00
far range      129.2        0.74          0.74
mid range      110.2        0.85          0.63
near ridge      80.7        0.73          0.46
shore treeline  63.5        0.79          0.37
ground plane   77.5 -> 138.2
foreground rock 51.6        0.81          0.30
```

**The plane is the row that breaks the ladder and that is the mechanism worth
copying** — every other band is a single value, the plane recedes *within
itself*, and that is where the into-the-page effect comes from rather than from
the count of layers.

**The treeline is the one band authored warm** (entry 8: sky 0.0% warm pixels,
treeline 47%, the warm population held at a near-constant luminance while the
cool one descends). That inverts the player's own cool-against-warm isolation
locally, so **the two have to be checked against each other** — a warm treeline
and a warm world spends the player's isolation twice.

**Do not author a near silhouette.** The reference's foreground rock is a
painting the boat passes behind; ours is diggable terrain, and a painted band in
front of the world would occlude the one verb the game has. That row is a
refusal, not an omission.

### The one lesson from V20 that 4c should carry into its own numbers

The ground pair had been justified as "ratio near/far 1.83 against the
reference's 1.78". Arithmetically true, wrong quantity: entry 7's plane is a
ratio of 1.78 **and a difference of 61 levels**, and matched as a ratio down at
L 18 it bought 9.8 levels after the grade. **When a mechanism is absolute
contrast, matching its ratio is not matching it.** The ladder above is ratios —
check what each one comes to in levels before shipping it.

## The one decision deliberately left open

**Whether the world row takes a grade below the plane's.** In the reference the
plane is *brighter* than what stands on it; ours runs the other way with the
world at 1.0. Grading the world down is coherent — grade multiplies and light
adds, so fire lights it back up — but it changes how the play area reads while
digging. **That is a `TUNING.md` row and a playtest, not an implementation
detail.** Do not settle it inside a commit that is about something else.

V20 makes this more live, not less: the backdrop is now considerably brighter
than it was, so the world is closer to being the darkest thing in the frame by
default. If the tester comes back saying the play area reads badly against the
new backdrop, **this is the row that answers it** and it should be opened
deliberately rather than absorbed into 4c.

## Also still open

- **`GROUND_STRIPS` is unpriced and cannot be priced by the bench.** The plane
  issues 24 strips times their tiling copies every frame; `grid_bench` times
  `Grid::update` and `Run::step` and **cannot see a draw call at all**, so
  neither half of the frame-budget rule reaches it. The instrument is the frame
  rate in the running game — it is item 5 of the owed list. 4c adds three more
  bands to the same frame.
- **V18** (step 5): write the split view down, build none of it. An afternoon.
  The deliverable is a written design.
- **E10 (powders come to rest)** resumes the head of the queue when the block
  closes. Unblocked and ready; deliberately out-prioritised, nothing changed.
- **V7-rest** is still open, one item smaller than its name suggests: the
  darkening multiply is done, **non-fire light sources are untouched**, and that
  is all it ever was besides the multiply. The ID stays `V7-rest` because four
  documents cite it.
- **Combat is decided: yes, deferred.** Playtest session 6 answered "needs an
  enemy but i will do that later". **The deferral is part of the answer and is
  not the decision still being open** — it means the ugly enemy (sprite, hitbox,
  contact damage, dies and despawns, about two days), not `S1`. Closed in
  `ROADMAP_ITEMS.md`.

## Verification, every step

```bash
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure    # 13 suites, ~1s, all of them
```

There is no reason to run a subset. `golden_frame_test` is the last of them and
the only one that links SDL2-static; it still needs no display. **It hashes
software rasterisation and is blind to a GPU-only defect — do not quote it as
covering the shipped frame.**

**`assets/` is copied next to the exe at build time.** A generator's output shows
nothing until a rebuild or `python tools/load_sprite.py --stage`. This is the
first thing to check when a new band "didn't show up".

**`main.cpp` prints the launch check** — `World seed: N` and
`Scene: WxH, N cells placed`. Read those, not the window; a scene count of zero
once meant a blank world that every suite passed on.
