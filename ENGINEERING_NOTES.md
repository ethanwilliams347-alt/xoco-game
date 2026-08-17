# Engineering Notes

Technical decisions that were deliberately made and then deferred, recorded so
they are not rediscovered — or reversed by accident — later. This is reference
material, not a task list; for sequenced work, see [ROADMAP.md](ROADMAP.md).

### Not foundations — resist these
*Recorded so they are not mistaken for pillars when the next architectural itch
arrives. Each one is a real technique that this project does not need.*

An **entity/component system** — there is one body, and there will be perhaps
four things; a `Run` with named members is the correct amount of structure.
**Threading** — already deferred below, and F1 (ROADMAP.md) makes determinism an
explicit invariant that threading is the single largest known threat to.
**Networking** and a **scripting layer** — neither appears anywhere in the
Definition of Done (ROADMAP.md). The **asset editor** — already priced as a
second application in Presentation & Tooling (ROADMAP.md). And **cell size**,
for the reason in the entry below, which has since changed and is worth reading
rather than remembering.

**One thing this list used to forbid and no longer does: a new simulation
axis.** ROADMAP.md's Engine & Visual Depth section spends one — heat, as E2 —
and that is a decision rather than the rule collapsing. The distinction that
survives, and the one to hold future proposals to: **an axis has to be bought by
something observed in play, named where it is spent.** E2 is bought by fire that
ignites on a dice roll and therefore never looks like it is burning *through*
anything; E1 and E3 are bought the same way, by water that cannot rise and
structures that descend like an elevator. That is a much higher bar than "this
would be cool" and a much lower one than "never" — and the reason to move it at
all is that `VISION.md` now names the engine and its visual design as the
selling point, which makes depth in them the product rather than a detour from
it. **The items on the list above are untouched by that change**, and all five
are still refused: none of them is a simulation axis, and none is bought by an
observation. If a sixth candidate arrives, it goes here with its reasoning
unless it can name the thing in play that demands it.

### The shipped scene is the test scene, and it stays that way

*Adopted 2026-08-16, by request, while V22 was choosing between three ways to
split them. It is recorded here rather than in the roadmap because it is a
standing constraint on every later scene decision, not a step.*

**There is one authored scene. It ships, and it is what the suites and probes
load.** Every system that needs exercising earns a feature in it that a player
would plausibly encounter — the stairs become terrain, the pit with pillars a
cave mouth, the water channel a river. The F4.4 fixture is not deleted by this
and its regions are not lost; what changes is that **the fixture is a set of
exercises rather than a place**, and an exercise can be dressed and moved.

- **What it refuses, and this is the load-bearing half.** A test-only scene
  alongside a shipped one is off the table permanently. The argument is not that
  two scenes are hard to maintain — it is that **they diverge by neglect and
  nothing reports it**: the shipped scene gets every art pass, the test scene
  gets none, and the suites go on passing against a world that stopped
  resembling the game months earlier. This project has that failure in its
  history at a smaller scale (the scene legend pointed at the render palette,
  the game booted blank, and every suite passed) and the lesson recorded then
  was that a check must not be able to succeed against something nobody looks
  at.
- **It is also a design filter, which is the unexpected return.** **A test
  feature that cannot be dressed into something a player would meet is a feature
  that exists only to be tested.** Today those hide at the spawn where nobody
  has to justify them; under this rule they have to be placed in a world someone
  walks through, and the ones that cannot be are visible immediately. That is
  worth having independently of the composition problem that produced it.
- **The one real cost, stated plainly because the rule does not remove it.** A
  test fixture wants to be *stable*; a product scene wants to *change*. Welding
  them means every art pass on the scene invalidates both recorded sessions and
  re-pins `FIXTURE_SCENE_CELLS`. That is rare today and it would not be rare
  during an art run — and **P4's replayed row is the only instrument in the
  project that prices a real played frame.** An instrument that is perpetually
  stale is one nobody trusts, which is the failure mode `PERFORMANCE.md` exists
  to document.
- **The mechanism that pays for it, owed at the first re-record and not
  before.** Pin the replayed benchmark to a **dated frozen snapshot** of the
  scene rather than to the live one, with each recorded session travelling with
  the snapshot it was played in. **This is not the two-scene split wearing a
  hat, and the distinction is exactly the one above:** two scenes are two
  *authored* artifacts that drift apart because nobody is editing one of them; a
  snapshot is a *copy* of the one authored scene taken on a known date, which
  cannot drift by neglect — it can only age, and its age is in its filename
  where a reader trips over it. Refreshing it is then a deliberate act with a
  visible cost, which is the correct shape for something that invalidates a
  benchmark.
- **The full list of things that carry the scene's identity, so none of them is
  found late.** `tests/test_scene.cpp` (`FIXTURE_SCENE_CELLS`), both `.rec`
  files via `src/game/input_log.h`'s fingerprints, `tests/bench_grid.cpp` (two
  call sites), `tests/rim_probe.cpp`, `assets/test_props.txt` (props are planted
  by scanning the terrain under their own footprint, so new terrain re-plants or
  drops them), and **`MANUAL_TESTING.md` step 1, which quotes the printed cell
  count as its check** — that last one is a document rather than a build and is
  therefore the one that goes stale without failing.
- **A second cost that has to be priced before the scene is authored, not
  after.** The world is 1920x1080 cells and the fixture already occupies
  1600x1000 of it. An open run at the spawn *plus* distributed dressed features
  does not fit, so either the features get smaller or the world gets wider.
  Widening is not free in either direction: `Element` is 12 bytes, so 1920x1080
  is 24.9 MB and a doubled width is ~50 MB — affordable — **but `grid_bench`'s
  1920x1080 block is the frame-budget baseline, and changing the world size
  moves the baseline rather than measuring against it.** If the world grows, the
  baseline is re-established deliberately and said so out loud, in its own
  sitting.

### Deferred decisions

- **Generate the backdrop clear colour into `backdrop_layers.h`, rather than
  hand-copying it into `frame.cpp`.** *(Deferred 2026-08-16, V21, with a defect
  already spent against it.)* `draw_clear` names a palette tone in a comment and
  repeats its RGB as a literal. **That copy went stale and shipped:** V20 raised
  the backdrop group wholesale and did not carry the change, so the clear sat at
  luminance 18 under a comment naming `sky_deep` — a constant that had by then
  both moved *and* stopped being the darkest of its pair, because V20 inverted
  the sky ramp in the same commit. V21 corrected the value to `sky_horizon`; it
  did **not** remove the duplication.
    - **This is the second duplicated backdrop constant to go stale silently**,
      after the parallax factors V11 generated away, and
      `.claude/rules/assets-and-formats.md` already draws the general conclusion
      from that one: two copies of a constant with a comment between them is not
      enforcement. The mechanism to fix it exists and is proven —
      `tools/generate_backdrop.py --header` already emits the layer table into
      `backdrop_layers.h` from the same Python source of truth, so this is one
      more emitted constant, not a new pipeline.
    - **Deferred rather than done because V21 is a retune**, and a retune that
      grows a code path in the same commit is how a measurement stops being
      bracketed. The whole item is one palette multiply plus this one-line
      correction, and it is owed a human's eyes with nothing else moving in the
      frame.
    - **The reason it survived two sessions is worth more than the fix.** The
      clear only shows through a *missing or unreadable BMP* — the exact failure
      it exists to make survivable. **A constant that only matters in a failure
      path cannot be caught by anything watching the success path**, and neither
      the golden frame nor a playtest is watching that path. Generalise before
      writing the next such fallback: it needs either generation or a test that
      deliberately removes the asset.
    - **Reopen trigger: the next backdrop constant that has to be named in both
      languages** — or a third staleness, whichever comes first. If V22 goes the
      reflection or water route, the plane's tones land in exactly this position
      and that is the moment to do it.

- **Cell size.** `Element` is **12 bytes**, not the 8 this note claimed for
  several revisions. `ElementType` is a byte, but `uint32_t color` forces 4-byte
  alignment, so there were already 3 bytes of padding after the type before
  `updated_tag` and `fall_ticks` were added — both of which now sit in padding
  the struct already had, which is what `element.h`'s `static_assert` is there
  to keep true. The real figure was confirmed by compiling
  `static_assert(sizeof(Element) == 12)`, not by counting fields, and the assert
  is left in place at `<= 12` so this cannot drift again unnoticed. Colour could
  be derived from the material table plus a small per-cell jitter seed, cutting
  the cell to ~2 bytes — so the win is roughly **6x** the memory traffic of the
  hot loop rather than the 4x recorded here, which makes it a *larger* prize
  than it looked. It changes nothing about *that* trade: **deriving colour is
  mutually exclusive with authored per-cell colour.** Per-cell colour is what
  makes hand-painted terrain keep its own pixels while it falls —
  `swap_elements` moves whole `Element`s and `drop_component` falls through that
  same swap, so a hand-painted slab collapses carrying its own shading, and the
  engine gets that for free from storing colour per cell rather than by being
  designed for it. That is the entire visual pillar, so authoring wins and
  **deriving colour** stays off the table however good the number gets. The
  accepted consequence is that shading is baked per cell, so a lit top edge
  travels with the cell and can end up on the side of a fallen slab; at this
  scale that reads as material texture, and it only breaks down under
  large-scale gradients, which is why none get painted into the simulated layer
  (measured in [notes/art_direction.txt](notes/art_direction.txt)'s rim
  section).
    - **The padding after the type is real space, and this note said so before
      anyone believed it.** *(2026-08-13, the instrumentation sitting.)* The
      bullet above records "3 bytes of padding after the type" as part of
      explaining why the struct is 12 and not 8 — and every later document
      treated the struct as full anyway, because `element.h` wrote down "that
      was the last free byte" while counting only the *tail* hole. Two roadmap
      items were then sequenced around a scarcity that did not exist: E10 and
      E5a were both going to subdivide `Element::ticks`, and the plan carried an
      open decision about how. **The three bytes at offsets 1–3 take fields for
      free** — `+1`, `+2` or `+3` bytes declared between `type` and `color` all
      leave `sizeof(Element) == 12`, measured by `velocity_probe` rather than
      counted, and `grid_bench` cannot tell the difference. E5a's velocity goes
      there.
    - **A byte appended to this struct costs four.** 13 rounds up to 16 under
      the 4-byte alignment `color` forces, so "spend a thirteenth byte" — which
      is how the option was written in the plan for two months — was never on
      offer: the real price is +4 bytes per cell, 7.9 MB at 1920x1080, +33%
      memory traffic. Worth knowing before the *next* field is proposed, because
      after E5a the struct genuinely is full and this is the price of the one
      after that.
    - **And the widened struct measured 10.5% faster on `cascading`**,
      bracketed, against a 0.7% noise band. Recorded in
      [PERFORMANCE.md](PERFORMANCE.md) rather than acted on. It does not license
      spending the memory; it does mean the "wider stride through the hot loop"
      half of this struct's standing cost argument is unverified and currently
      contradicted, and `P1` is the item that owes an explanation.

- **`Element::ticks` does not become a velocity, and E5a does not use it.**
  *(Decided 2026-08-13 at the instrumentation sitting, after being planned the
  other way since 2026-08-09.)* The plan was to pack four bits of signed `vx`
  and four of `vy` into the byte for any cell that is not structural and not
  `Fire`. **It cannot work, for a reason that has nothing to do with how the
  bits are divided: one step of `Player::GRAVITY` is 5/36 of a cell per step,
  and there is no representation of that in whole cells.** The increment
  truncates to zero every step forever, so a thrown grain flies in a straight
  line until something stops it — `velocity_probe` flies exactly this and it
  never comes back down. That is a raycast, and the throw, splash and spray E5a
  exists to buy are all arcs.
    - **The two cheap rescues were measured and both fail.** Applying the
      increment stochastically at 5/36 per step is deterministic, costs nothing,
      and is this project's own idiom — and 64 grains given an identical impulse
      land across a 364-cell spread on a 233-cell mean, so an explosion scatters
      its debris instead of throwing it. Moving to fixed point but truncating
      the increment makes gravity permanently 10% light in one direction. **Both
      of these will be proposed again**, which is why the numbers are here
      rather than the conclusion alone.
    - **What is used instead:** signed 4.4 per axis in two of the free bytes, a
      nibble of sub-cell remainder per axis in the third, and the gravity
      increment obtained by differencing a running total off the *global* step
      counter — Bresenham applied to an acceleration, exact in the mean, no
      per-cell accumulator. Flown at every phase of the pattern it stays within
      2 cells of an `fx` 16.16 reference trajectory.
    - **The half the plan never named: position is a cell index.** A velocity
      with a fraction in it needs a sub-cell remainder to spend the fraction
      into, or half a cell per step is indistinguishable from zero. `Player` has
      carried `rem_x` next to `vel_x` since before F5 for exactly this reason.
      Every version of the `ticks` decision — including the two that were
      willing to spend a whole extra byte — budgeted for the velocity and not
      for the remainder, and would have discovered the second quantity during
      implementation.
    - **`ticks` keeps its two existing roles** (fall clock for structural cells,
      lifetime for gases) and `tick_role()` in `element.h` is unchanged. The
      third claimant is simply gone.

**What this entry got wrong was treating that as the end of the matter, and it
is not.** The win was priced as *memory traffic in the hot loop*, and then the
only route to it considered was shrinking the cell — which happens to be the one
route authored colour forbids. There is another: **stop reading colour in the
hot loop at all.** `type` and `updated_tag` are consulted for nearly every awake
cell every step; `color` is touched only when a cell is written and when the
frame is uploaded. Splitting the single `Element` array into hot and cold arrays
captures most of the bandwidth win **and keeps every authored pixel**, because
it changes where colour lives rather than whether it exists. That is scheduled
as **P1** in ROADMAP.md's Engine & Visual Depth section, and
[PERFORMANCE.md](PERFORMANCE.md) — which independently records that `cascading`
is bandwidth-bound — carries the prediction and will carry the measurement.
Filed here as a correction rather than a rewrite, because "mutually exclusive"
was a true statement about the wrong pair of things, and that is a failure mode
worth being able to recognise again.
- **RNG cost — closed by F1.6 (ROADMAP.md), and it was not a win.** This sat
  here for two revisions as a pure performance item — `std::mt19937` called
  several times per active cell, a stateless hash assumed to be materially
  faster in the inner loop. **The assumption was wrong.** Bracketed measurement
  (see [PERFORMANCE.md](PERFORMANCE.md)) shows the hash costing `cascading`
  about 1.7–1.9%, not saving anything; `churning` is unaffected either way.
  `std::mt19937` and `<random>` are gone from `grid.h` regardless, because
  determinism — not speed — was always F1's actual reason to exist: a stateful
  generator meant no run in the project was reproducible, and that is fixed now
  whatever the cost turned out to be. Left here rather than deleted so the wrong
  prediction stays on the record next to the number that corrected it.
- **Threading.** Deliberately single-threaded. Chunked updates were the
  prerequisite, so this is now *possible* — but it is also the single largest
  source of subtle nondeterminism available, which F1 (ROADMAP.md) makes an
  explicit, tested invariant rather than an unexamined property. That raises the
  bar rather than lowering it: a threaded sweep would have to keep
  byte-identical results for a given seed, or the save format, the playtest bug
  reports and Quantum Worlds all lose their footing at once. The measured
  numbers do not come close to justifying that.
- **Cross-platform.** The build targets Windows/macOS/Linux and uses no
  platform-specific code, but has only been *built and run on Windows*. Verify
  on the other two before claiming support.
- **The scene legend is not the render palette, and must never be pointed back
  at it.** A material map names an `ElementType` per pixel by colour. That
  lookup used to match against `MATERIALS[i].color` — the colour the material is
  *drawn* in — which made every scene file a hostage of the art direction. V2
  retuned the palette and, on the same commit, every one of the 27,192 authored
  pixels in `assets/test_material.bmp` stopped matching anything: the whole
  scene loaded as `Empty`, the game booted to a blank world, and **all six
  suites still passed**, because the lookup lived in `main.cpp` where no test
  links. The legend is now its own frozen table in
  [src/scene/legend.h](src/scene/legend.h), the lookup is SDL-free and tested,
  and an unrecognised colour is reported rather than silently resolving to
  `Empty` — that conflation is what made a broken scene and an empty scene the
  same observation. **The standing rule: a legend value is arbitrary and
  permanent, the same way a `sim_random::Stream` tag is.** Changing one
  invalidates every material map ever authored. This matters now rather than in
  the abstract, because V6 (ROADMAP.md) is a second deliberate palette pass and
  would have done the same thing again. **Closed the rest of the way on
  2026-08-13 by P4, which needed the same thing for an unrelated reason:** the
  *loader* has left `main.cpp` too ([src/scene/bmp.cpp](src/scene/bmp.cpp), no
  SDL), so `scene_test` now loads the shipped `assets/test_material.bmp` and
  asserts it stamps 334,901 cells and that every authored pixel matched a legend
  entry. The failure described above — a real scene file loading as an empty
  world with the whole suite green — now fails in `ctest`. **Worth noting the
  shape: the lookup was made testable in V2's aftermath and the file it reads
  stayed unreachable until an unrelated item happened to need it headlessly.
  Half a fix looks like a whole one for as long as nobody asks the other half a
  question.**
- **Per-cell data gets an image; a list gets a list — and the frozen legend is
  why.** Scenes are authored as a material map, one legend colour per pixel, so
  the reflex when a *second* kind of scene data arrives is to author it the same
  way. That reflex is right for anything defined per cell and wrong for a list
  of records, and V4's prop format is the worked example: the scene BMPs are
  grid-sized at 1920x1080, so a parallel prop map is 6.2 MB of black carrying
  nine meaningful pixels — and it still cannot say *which* sprite each pixel
  means without [src/scene/legend.h](src/scene/legend.h) growing a row per
  species. **The legend is frozen precisely so it cannot grow that way**, per
  the entry above, so the image form does not merely cost more, it forces a
  change to the one table that must never change. Props are
  `assets/test_props.txt`, parsed by [src/scene/props.cpp](src/scene/props.cpp).
  Three rules came out of building it and all three generalise past props:
    - **A format must not carry a number its loader ignores.** The prop format
      has no `y`: props are planted by scanning the terrain under their own
      footprint, because "the ground" is not one number — authoring against a
      single floor constant is what put three trees 26%, 43% and 83% inside a
      snowbank. A `y` in the file would be read and discarded, and **a number
      the loader ignores is one an author eventually spends an afternoon
      tuning.** Writing one is a parse error rather than a silent skip, and the
      message says why.
    - **A malformed list costs every record, not the bad line.** A tolerant
      parser that drops the row it could not read produces a scene that renders,
      renders wrong, and says nothing — the same failure as the blank world
      above and the buried trees. The absent/broken distinction is held
      separately: a missing file is not an error, because a scene with no props
      is a legitimate scene. **The consequence for tests is the non-obvious
      half** — most of `props_test`'s checks assert the loader returned *empty*,
      not *short*. A suite written the obvious way, asserting the good rows
      survived, passes on the bug it exists to catch.
    - **Data that names a path can name any path.** A sprite name is letters,
      digits, `_` and `-`: no separators, no `..`. Cheap here, and the rule to
      apply to every later format that lets authored data reach the filesystem.
    - **The reporting rule these three imply, learned by getting it wrong.** A
      record dropped for any reason has to be visible in the *count*, not only
      in a warning. `main.cpp` printed `Props: N of M placed` before the
      planting scan ran, so a run that dropped a prop printed `10 of 10 placed`
      on stdout while warning on stderr that one was not drawn. **A count taken
      before the last thing that can fail is measuring a different quantity than
      its label claims**, and README's launch check makes that line the check
      rather than the eyeballing.
- **✅ Closed 2026-08-12 by `F5` — the player's motion is fixed point, and `Grid`
  is no longer the only deterministic half.** Velocities and the sub-cell
  remainder are `fx` signed 16.16 ([src/physics/fixed.h](src/physics/fixed.h));
  the constants are exact rationals; `Player::update()` no longer takes a `dt`
  at all, because the rate is `fx::STEPS_PER_SECOND` and every per-step amount
  folds at compile time. `visual_x()`/`visual_y()` remain float and are the
  renderer's boundary, not an exception — the renderer rounds to a pixel, so a
  last-bit difference there is a difference in a number that was going to be
  discarded.
    - **The note below is left standing because of what it got right and what it
      did not stop.** It correctly identified the exposure, correctly said no
      fix was scheduled, and correctly asked that "the simulation is
      deterministic" not be quoted as covering the player. It was then quoted as
      portable anyway by three separate later items — crash diagnosis, save and
      persistence, and the first non-Windows build — none of which had any
      reason to be looking here. **Writing a limitation down does not stop
      people spending it**; what stops it is the limitation not existing.
    - **The verify condition could not be met as written, and the reason is
      worth keeping.** ROADMAP_ITEMS.md asked for traces "identical to the float
      version to the cell". They are not quite: over 1381 recorded steps of
      fall, walk, jump and sustained flight, **7 steps differ, each by one cell
      and each re-converging immediately**; walk and jump are byte-identical,
      and every landing, resting position and peak matches. The cause is not a
      rounding-mode choice that could have been made differently — **1/60 is not
      representable in binary**, so `GRAVITY` per step is 8.333328 cells/s in
      fixed point against 8.333334 in float, and neither is 8.3̇. Exactness was
      never available in either scheme. What *is* available, and is the whole
      point, is that the fixed-point value is the same on every machine.
    - **The float note, kept:** `Grid` is integer-only on purpose — F1.7 wrote
      that down as an invariant and the thermal pass was held to it explicitly.
      `Player` is not: position is an integer cell plus a `float` remainder, and
      velocity, gravity and `dt` are all floats. Within one binary this is
      perfectly reproducible, and `test_run.cpp`'s replay check passes because
      of it. Across compilers or architectures it is not guaranteed — x87 excess
      precision, contraction into FMA, and `/fp:fast`-style flags can each
      change the last bit, and the sub-cell remainder is exactly where a
      last-bit difference turns into a whole cell of divergence at some later
      step. **Nothing is broken today and no fix is scheduled**, because the
      project builds one way on one platform; it is filed here so that "the
      simulation is deterministic" is not quoted as covering the player when
      someone brings up a second toolchain, or a replay file, or a networked
      run. The fix, if it is ever needed, is the same one `Player` already
      half-uses: integer sub-cell units instead of a float remainder.
      `DigTool::march`'s `sqrt`/`lround` are the same class of exposure and much
      smaller, since they only pick a target cell.
    - **⚠️ That last sentence is now the whole of the remaining exposure, and it
      is larger than "much smaller" makes it sound.** *(2026-08-12, found while
      closing F5.)* `march` picks which cells a dig removes from a `float`
      `sqrt` and two `lround`s of a float product, and **digging writes to the
      grid** — so a last-bit difference there is a different cell deleted, which
      is a different world, not a different pixel. Nothing else under
      `src/physics/` contains a float. **So: do not yet claim determinism is
      portable.** F5 removed the larger half; this is the other half, it is
      perhaps an afternoon (`len <= RANGE` becomes `dx*dx + dy*dy <=
      RANGE*RANGE`, and the two `lround`s become rounded integer division), and
      it is unscheduled rather than refused. The gate prerequisite "one verified
      build on a machine that is not the dev machine" is what would otherwise
      discover it, and it is the item with the least reason to be looking.
    - **✅ Closed 2026-08-13 by `F6`, and the estimate above was right — an
      afternoon, both named edits, no surprises in the shape of it.** `march`
      computes a squared range comparison, takes its truncated step count as
      `isqrt(span*span * RANGE*RANGE / dist2)` (equal to the old `int(span *
      RANGE / len)` because `floor(sqrt(x))` is `floor(sqrt(floor(x)))`), and
      places cells with a `div_round` that keeps `std::lround`'s
      halves-away-from-zero. **No float under `src/physics/` reaches the grid.**
      *(Not "no float in `src/physics/`" — `DigTool::swing_progress()` is one
      and stays one, on the same renderer boundary as `visual_x()`/`visual_y()`.
      The note above says "the only float left under `src/physics/`" and was
      already slightly wrong when written; corrected here rather than edited
      there.)* Three things to keep:
        - **The obvious integer replacement for `lround` is wrong.** `(a + b/2)
          / b` truncates toward zero on the negative side, so the ray up-left
          stops being the mirror of the ray down-right and digs land one cell
          off in two quadrants of four. This is the general trap when a float
          rounding function is replaced: **`lround` rounds away from zero and
          integer division rounds toward it**, and the two agree on every test
          case that only uses positive numbers. The mirror-symmetry assertion in
          `test_tool.cpp` exists for exactly this and nothing else.
        - **It found a live 32-bit overflow that had nothing to do with
          determinism.** `dx * dx + dy * dy` was `int`; past ~46,000 cells of
          aim it wraps, the length is garbage and the range limit stops
          applying. Not reachable from the mouse, which is bounded by the window
          — but the test that asserts range still holds at a distant aim **fails
          against the unfixed code**, which is the failing-test-first rule
          finding a real defect on an item that had none attached. **The general
          form: an expression being replaced for one reason is the cheapest
          moment to ask what else it was getting wrong**, because it is the one
          time anybody is reading it closely.
        - **What may now be said, exactly.** The simulation is integer
          arithmetic end to end and reproduces on any conforming compiler *in
          principle*. It has still never been built anywhere but this machine.
          F6 removed the known reason a second toolchain would disagree; it did
          not verify that none remains, and the gate prerequisite is unchanged.
          **"Portable in principle" and "verified portable" are different claims
          and this document should never let them merge** — the entry above is
          on the record precisely because a written limitation got spent as a
          guarantee three times.
- **`Element::ticks` now has three roles and one of them is asserted rather than
  remembered — and steam's condensation is deliberately no longer a
  temperature.** *(2026-08-12, E9's steam half.)* The byte is a free-fall clock
  on a structural cell and a countdown on a Gas; `Fire` was the only Gas that
  used it until `Steam` became the second. The old guard was a `static_assert`
  naming Fire, which was correct and did not generalise, so it is replaced by
  `tick_role()` in [element.h](src/physics/element.h) plus an assertion over
  **every** row that nothing with a lifetime is structural. That is the same
  move `reaction.h`'s spawn-temperature check makes and for the same stated
  reason: data-driven design puts the danger in the relationships between rows,
  and no compiler sees it unless one is written.
    - **What was given up, recorded so it is not refiled as a bug.** The `{
      Steam → Water, 0..26 }` row in `REACTIONS` is deleted, so **steam no
      longer condenses because it is cold.** It condensed because it was cold
      *and* because that was the only clock it had, and those two jobs are what
      E9 exists to separate: spawn temperature is pinned low by the
      ignition-floor `static_assert` — steam hotter than the coldest ignition
      point in the table is a fire-starter, which has shipped once — so the
      lifetime end could never be tuned without moving a temperature.
      Cold-quenching is expressible again whenever anything asks for it, as a
      *catalyst* row (`Water` + `Steam` → `Water`), which is a different rule
      from a lifetime rather than this one coming back. Nothing observed in play
      asks for it today.
    - **The known collision with `E5a`, and it is a finding for the
      instrumentation sitting rather than a problem with this item.**
      ROADMAP_ITEMS.md carries an open decision about what the byte can
      represent, because `E5a` wants it as a packed per-cell velocity for
      non-structural cells. Steam joining Fire means **two** Gases now spend it
      on a lifetime, so neither can carry a velocity — which *widens an
      exclusion that was already written down* (E6's entry already states that
      an explosion cannot hand an impulse to a flame) by one row, rather than
      creating a new one. It does not pre-empt the decision: whichever
      representation wins, `tick_role()` is where the roles are counted and the
      assertion is what will fail if a fourth one is added carelessly.
- **Frame tag wraparound.** `Element::updated_tag` is one byte, so a cell asleep
  for an exact multiple of 256 steps is skipped for a single step and runs the
  next one. This is known, harmless, and cheaper than the alternatives — do not
  "fix" it with a wider counter without a measured reason.
- **The player is invisible to the grid.** The simulation has no idea a body is
  standing there, so material falls through the player instead of piling on top
  of it, and the unstuck search is what stops that becoming a freeze. The real
  fix is for the player to displace material, which is harder than it looks:
  shoving cells aside must not create or destroy matter, and the obvious cheat —
  stamping the body into the grid as a temporary solid each step — either
  deletes whatever was already in those cells or needs a full displacement pass
  of its own. Deliberately deferred — scheduled as **E4** in ROADMAP.md's Engine
  & Visual Depth section, where the deliverable is explicitly an *answer* rather
  than necessarily a feature, and where the implementation (if the answer is
  yes) waits on **E5a** because that is what gives displaced matter somewhere to
  go. *(Updated 2026-08-09: this said "E5's free-particle layer". E5 split into
  E5a — velocity on the cell, in the grid — and E5b, the air field. The shove is
  simpler under the new design: the cell the body walks into is handed the
  body's velocity and stays in the cell array, so nothing has to leave the grid
  and be conserved separately.)*
    - **E4 is answered, and the answer is no. Decided 2026-08-10 by [playtest
      session
      5](PLAYTEST_LOG.md#session-5-results--wave-3-closes-and-the-water-underneath-it-does-not),
      Phase B.** The decision's own rule was "play it, and if the artifact isn't
      obviously better, write no and stop thinking about it". Nothing in the
      four rows came back obviously better. **The instructive part is that two
      of the four rows were not testing displacement at all**: E-1 reported sand
      *pushing* the player — the opposite of the premise, and it is
      `resolve_overlap` firing, now defect D2 — and E-2 reported walking over a
      settled pile, which is almost certainly `MAX_STEP_HEIGHT` at 5 against a
      20-cell body, now D7. E-3 works. **That leaves standing in water (E-4) as
      the only surviving argument, which is the reverse of the split the
      checklist had guessed at** — it predicted "yes for powder, no for fluid".
      Re-ask at E5a; a yes would have waited on E5a regardless, so the "no"
      costs nothing that was going to happen sooner.
    - **The sentence below said the current behaviour is "odd-looking but never
      broken", and it is withdrawn rather than softened.** D2 is a containment
      failure: the unstuck search tests only that its *destination* is clear,
      never that the path to it is, so a body being poured on beside a wall can
      be relocated through the wall. **Fixed 2026-08-11 in [wave
      4](ROADMAP.md#wave-4--the-four-defects-session-5-found-on-the-way-past);
      the search now rejects any destination it cannot travel to.** The shape of
      that test is the part worth carrying: *"no solid on the path"* is the
      obvious reading and it is wrong — a buried body is surrounded by solid, so
      that rule rejects every escape and freezes the exact case this machinery
      exists for. What the fix tests instead is that the burial never gets
      *worse* along the way out, which distinguishes grinding up out of sand
      (less buried every cell) from burrowing into a wall the body was not in
      before. **E4 closing "no" raises the stakes on that fix rather than
      lowering them:** the unstuck search was described here as the thing that
      stops the player freezing *until* displacement lands, and displacement is
      now not landing, so it is permanent load-bearing machinery and has to be
      correct on its own terms.
- **The renderer stays `SDL_Renderer`, and the shader path is refused again
  rather than by default.** V7's entry required this decision be made
  deliberately with an entry here *if it is ever made*, precisely so it could
  not be discovered halfway through a CPU implementation that turned out too
  slow. **Re-examined 2026-08-11 against V12–V16 and S1, and the answer is still
  no** — but the reason has changed from "nothing needs it" to something
  checkable: **every scheduled item has a named route through the renderer as it
  stands.** `SDL_ComposeCustomBlendMode` gives V11's multiply term, so grading
  and darkening need no shader. `SDL_RenderGeometry` gives textured triangles,
  which is what any future mesh deformation or non-uniform transform would want
  — `SDL_RenderCopyEx` offers translate, uniform scale, rotate and flip and
  **nothing else**, which is the real limit worth knowing.
  `SDL_TEXTUREACCESS_TARGET` gives S1 its masked body. All three are in the
  pinned SDL 2.30.0 and **none of them is called anywhere in `src/` today**,
  which is the fact that makes "we have not run out of renderer" a measurement
  rather than an opinion. **The fork should be bought by the first item that has
  no such route, and there is one candidate:** sub-cell terrain detail. The
  terrain's colour resolution is one `uint32_t` per simulation cell in
  `Grid::pixels` and no amount of asset work reaches it, so a shaded or textured
  *interior* to a cell is the one thing on the horizon that a blit cannot do.
  Note that this is also the item the cell-size entry above forbids solving the
  other way, by making cells smaller.
- **The world's cell size is not how you get higher-resolution art, and the two
  readings of that request are worth separating before anyone acts on either.**
  Halving `Camera::SCALE` from 4 to 2 puts **four times the cells in the
  viewport**, re-authors every asset in `assets/`, and retunes every physics
  constant in the project — all of them are stated in cells against a scale of
  4: `Player::WIDTH`/`HEIGHT` at 8x20, `DigTool::RANGE`, `LightField`'s reach in
  blocks, `MAX_STEP_HEIGHT`. `display.h` argues the same point from the window's
  side and is right about the window; this is the same conclusion reached from
  the art's side. **The cheap reading is per-asset density — V13 — and it costs
  a column in `assets/sprites.txt`.** The expensive reading is a denser world,
  and it should only ever be bought against something a denser *world* is needed
  for, not against art that wanted more pixels. **The permanent consequence
  either way:** terrain is the floor of visual density, because its resolution
  is the simulation's, so mixed-resolution scenes have everything else denser
  than the ground and never the reverse.
- **Alpha is worth a dependency and the colour key is not worth keeping
  everywhere — but the palette validator only understands BMP, and that is the
  part that will bite.** V12 crosses the zero-dependency line with a single
  vendored `stb_image.h`. **The F4 verdict that made PNG lose to BMP does not
  bind it**, and the distinction is worth holding: that decision was about a
  *test fixture*, where the need was "read an image" and BMP met it; the need
  now is a real alpha channel, which BMP does not carry in any form `read_bmp`
  or `SDL_LoadBMP` produces, so the rule ("zero new dependencies until a
  specific need can't be met without one") fires rather than bends. **The hazard
  is not the dependency, it is V6 quietly becoming advisory.**
  `tools/validate_palette.py`, `snap_to_palette.py` and the whole `pixel_art.py`
  codec are BMP-only, so an asset in the new format is an asset nothing checks.
  **The standing rule: BMP-with-colour-key is the format for everything inside
  the locked palette — terrain, props, the player sheet — and the new format is
  only for assets deliberately outside it, each listed by name in
  `assets/sprites.txt`.** A palette that drifts is discovered a year later with
  nothing able to say when, which is the same shape as the blank-world failure
  two entries above. *(Partly superseded 2026-08-12 — see "The palette is
  deferred, not lost" below. The format half of this entry stands unchanged:
  BMP-with-colour-key is still the default and anything outside it is still
  listed by name in `assets/sprites.txt`, because that is about what the tools
  can **read**. The "inside the locked palette" half no longer binds hand-drawn
  art, because there is no locked palette for it to be inside — which is,
  exactly as predicted here, drift discovered later with nothing able to say
  when. It could say when: the answer is "from the first sheet".)*
- **The palette is deferred, not lost. Decided 2026-08-12.** V6 chose a palette
  and called it locked, the entry above restates that as a standing rule, and
  **the claim has gone false for hand-drawn art** — recorded here as a
  correction rather than a rewrite, because the way it went false is the useful
  part. The set was frozen when the only art in the project was *generated*:
  backdrops, trees and terrain are built by `tools/` **from** `PALETTE` by name,
  so they conform by construction and cannot drift. The `char_*` group was
  different in kind — it was frozen as a *prediction* of what a character would
  look like, before one was drawn. It never described any shipped pixel: **not
  one pixel of any of the four player sheets in `assets/` has ever been
  on-palette**, and the current fly poses are warm olive against a `char_*`
  group that is deliberately cool blue-grey, shaded across ~154 tones against a
  group of six. The validator was therefore red on every run from the day real
  art landed, which is the specific failure this project keeps re-learning — **a
  rule that stops matching the thing it names is worse than no rule, because it
  stops people checking.** Here it cost the `player_sheet.py --validate`
  *baseline* checks, which do catch real bugs (a figure hovering a cell above
  every floor, a hole through a silhouette, a declared frame nobody drew) and
  which were being ignored along with the palette noise they were buried in.
    - **What changed:** off-palette is a **note, not a failure**.
      `player_sheet.py --validate` warns and passes; `--strict-palette` restores
      the error. `tools/validate_palette.py` is documented as an audit you run
      on purpose, and is still meaningful on the *generated* layers, where an
      off-palette pixel means a generator has been edited to hardcode a colour
      instead of naming one. Nothing was deleted: `snap_to_palette.py`,
      `export_palette_gpl.py` and the `PALETTE` values all remain, and
      `assets/palette.gpl` is still worth loading in an editor as a starting
      point rather than a constraint.
    - **Why defer rather than re-derive from the current poses.** A palette
      derived from one entity is a palette fitted to one entity. There is a
      single character and no enemies yet; `E12`'s granulating material and
      whatever else the roster grows are the things a real set has to hold
      *against*, and the actual question a palette answers — does the figure
      separate from what it stands on — cannot be asked with nothing else in
      frame. Choosing now would freeze the answer to the easy version of the
      question.
    - **What ends the deferral**, so this does not sit open by drift: **a second
      and third entity drawn in the intended style, at which point the set is
      derived from the art that exists rather than predicted.** Two things to
      carry into that: `pixel_art.py`'s separation argument survives the
      placeholder that carried it — a character in the terrain's warm-dark
      family disappears against terrain, and the current poses are warm olive,
      so that is worth *looking at* in the running game whatever palette gets
      chosen. And the failure mode to avoid repeating is freezing a group before
      art exists to freeze it against; the generated groups earned their lock by
      being built from, not predicted at.

- **Sprite stains — Noita's idea, recorded rather than adopted.** Noita ships a
  third image beside its player sheet (`player_uv_src.png`) that maps the
  sprite's surface, so world materials splash onto the character and stain it:
  blood, oil and soot accumulate on the actual pixels rather than being a tint.
  Of everything in that engine's animation model, this is the one most obviously
  *for* a project whose entire subject is materials — a player who walks out of
  a fire visibly sooty is the destruction system reporting on itself, and no
  other feature makes the simulation legible on the character. **It is not
  scheduled and does not currently pass the admission test in ROADMAP.md's
  Engine & Visual Depth preamble:** it can name what it makes possible and
  cannot name anything observed in the built game that is wrong without it.
  Filed here so that when a playtest does observe it — "I set myself on fire and
  nothing about me changed" — the idea is already written down with its shape.
  **Its cost is not the tint; it is the extra buffer and the per-cell write path
  into it**, and it should be priced against V9's effects layer, which may cover
  the same ground more cheaply.
- **Frame-tied gameplay events are refused, not deferred.** Noita attaches
  events to animation frames — a kick lands on the frame the leg extends. That
  is rendering driving simulation, which F3.5 settled against and which F1 spent
  seven steps making impossible: two players on the same seed and the same input
  log must not diverge because of which animation frame was showing. If a future
  feature wants "the dig happens partway through the swing", the direction is
  the other one — the simulation decides when the dig fires and the animation is
  *told*, which is exactly how `dig_fired` already flows in `main.cpp`. Recorded
  because the temptation arrives disguised as polish.
- **Player feel is deliberately raw.** Horizontal speed is a direct function of
  input — no acceleration, friction, air control, coyote time, or jump
  buffering. All of that is feel work, and feel work is worth doing once there
  is something to feel. Do not start tuning it before Basic Interaction exists.
- **Player and tool cost are not benchmarked.** A collision test reads at most
  32 cells and a step runs a handful of them, against 518,400 cells of grid. A
  dig marches at most 24 cells and writes at most 29, once per 36-step swing.
  Both are far below measurement noise, and `grid_bench` numbers are unchanged
  after adding each. Revisit only if there is ever more than one body, or a tool
  that fires every step without a rate limit of some kind. *(D1 lengthened the
  gap between digs from 6 steps to 36 and added one extra ray march per swing,
  at the moment of impact — both move this further below the noise floor, not
  towards it.)*
- **UI layer — closed by the first Medium Term item (ROADMAP.md).
  Immediate-mode, drawn directly against `SDL_Renderer`, no new dependency.**
  Three real candidates: (a) immediate-mode primitives against `SDL_Renderer`,
  the same call style already used for the player body and the dig-aim marker;
  (b) a library — Dear ImGui or Nuklear, both of which slot onto an
  `SDL_Renderer` backend with little glue; (c) hand-rolled retained-mode — a
  small widget/window system of our own. (b) is out for the same reason PNG lost
  to BMP in F4 — BMP is in core SDL2, already fetched, and PNG would have meant
  pulling in SDL_image or zlib for a test fixture: the UI this project actually
  needs — a health bar, a handful of pet-agent numbers, a resolution picker — is
  not big enough to earn a new dependency, and "zero new dependencies until a
  specific need can't be met without one" is now a standing pattern, not a
  one-off call. (c) is out for the reason an ECS and a bespoke asset editor are
  out (see *Not foundations* above): a retained widget tree is structure sized
  for a UI this project does not have and does not have a second UI-heavy screen
  to justify keeping in sync. That leaves (a), and it is a genuine fit, not just
  the last one standing: every UI element on the list so far is small,
  per-frame, and already expressed naturally as rects (a health bar is a rect
  that shrinks) — exactly the shape immediate-mode against a 2D renderer is for.
  The one gap it opens is text: `SDL_Renderer` draws no glyphs on its own, and
  `SDL_ttf` is a dependency in exactly the shape being avoided for a handful of
  digits and short labels. Closed the same way the BMP legend was: a small
  hand-authored bitmap font (`src/ui/text.h`/`text.cpp`, SDL-side like
  `main.cpp` and unlike `src/physics`), covering only the glyph set actually
  needed rather than the whole ASCII table, extended on demand the way
  `MATERIALS` gains a row rather than a second table. **Smallest thing that
  proves the choice:** the frame-rate/brush/awake-chunk readout that the window
  title bar was carrying alone moved into the game window itself, drawn with the
  new bitmap font — the first pixel actually drawn by this UI layer rather than
  described in a note. Health, the pet panel and resolution options each still
  need their own screen or readout built later; this only proves the drawing
  primitive they will all sit on.
- **Materials have no hardness, and everything is diggable.** Adding a hardness
  or indestructible column to `MATERIALS` would be a second axis with no
  consumer: the world border is already sealed by `set_element`'s bounds check,
  so nothing can be dug through that matters yet. Revisit when there is a reason
  for one material to resist a tool more than another — bedrock around an
  objective, or a tool upgrade curve. **Two scheduled items will pass close to
  this and neither is a reason to build it early:** E2 adds thermal columns to
  the same table, and E3 (fracture) needs to decide where a structure breaks —
  which is a *stress* question answered by how the piece is supported, not a
  *hardness* one answered by the material. If E3's implementation starts
  reaching for a per-material strength number, that is the signal this entry was
  waiting for; until it actually does, it has not arrived. **A third item now
  passes close to it and it is also not the signal: `E12`'s granulating
  material.** Its shipped form (M2 in
  [notes/granulating_enemies.md](notes/granulating_enemies.md)) is a roll per
  disturbed cell, which needs no strength number at all. **The
  accumulated-damage form (M3) is this entry coming due** — a per-cell integrity
  byte, damage accumulating, granulation at zero — and it is scheduled after
  **P1** for exactly the reason recorded at the top of this file: the byte costs
  500 KB and a wider stride today, and P1 is the item that makes a decision like
  it affordable. Opening this entry at M2 buys the axis a version before
  anything consumes it.
- **Digging is a deliberate matter sink.** The conservation-of-matter test
  covers `Grid::update()` — the simulation never creates or destroys cells on
  its own, and that invariant still holds. Digging is an *external* write that
  deletes matter outright, which is correct for a tool and would be a bug
  anywhere in the step loop. Do not "fix" the conservation test to accommodate
  it; they are testing different things.
- **All writes must go through `set_element`, `paint` or `swap_elements`.**
  Three public write paths now, not two — `paint` arrived with F4.1 so authored
  art can keep its own pixels instead of a jittered colour. **They are not three
  implementations of the wake rule, and that is the point:** `set_element` and
  `paint` differ only in where the colour comes from and both delegate to a
  private `place()`, so `place()` and `swap_elements` are the only two functions
  in the class that call `mark_dirty`. Written that way after `paint` shipped as
  a copy of `set_element` with one line changed, which left two bodies free to
  drift apart on every rule that matters — the wake, the frame tag, and the
  structure-removal support check. A new code path that writes cells directly,
  or a fourth write path that reimplements `place()` rather than calling it,
  will produce material frozen in mid-air; the tests that catch it are the chunk
  tests in `test_grid.cpp`.

- **`Run` does not own the level, and `reset()` therefore does not restore it**
  *(S0, 2026-08-14)*. `Run::reset(seed)` wipes the grid and rebuilds the body;
  putting the world's terrain back is the caller's job, because the scene is a
  `Scene` loaded from two BMPs by `main.cpp` and `Run` has never seen one. **The
  alternative was examined and deferred rather than missed:** `load_scene` is in
  `ENGINE_SOURCES`, so `Run` *could* hold the starting scene and re-stamp it
  itself, which would make `reset()` genuinely total and would give every caller
  — the game, the benchmark, a future save file — one behaviour instead of a
  convention. It was not built because S0 needs exactly one caller to do it and
  paying for the general version at a spike's expense is how a spike stops being
  one. **The signal that it has come due is a second caller resetting a run**,
  and the two on the horizon are the save/persistence item (which has to rebuild
  a world from a file anyway) and T1's world-reset hotkey. Whoever hits it
  should know the reason it is safe today is asserted rather than assumed:
  `run_test` checks that reset-on-the-same-seed plus a re-stamp reproduces the
  starting world's fingerprint exactly.

- **A restart starts a new session recording rather than continuing the old
  one** *(S0, 2026-08-14)*. P4's log replays by rebuilding the world from the
  seed and the scene and feeding the recorded inputs back into it. A log
  spanning a `Run::reset` would replay into a world several minutes of play
  deep, and `grid_bench` **cannot tell that from a stale log** — which is
  precisely the "silently measures nothing" failure P4 was built to remove,
  arriving through a feature rather than through neglect. Three options were on
  the table: stop recording at the first restart, make the restart part of the
  input stream so a replay reproduces it, or start a fresh log. **The third wins
  because it costs nothing and loses nothing structural:** reset on the same
  seed plus a re-stamp of the same scene reproduces the starting world exactly,
  so the new log is as valid as the first, and what is lost is only the part of
  the session before the restart. The second option is the *right* answer and is
  blocked on the entry above — an `Input` carrying a restart flag needs
  `Run::step` to be able to rebuild the world, which needs `Run` to own the
  scene. **Revisit both together, not separately.**

- **S0's objective is a point in the world, not a cell, and not an entity**
  *(2026-08-14)*. `Run` holds an `(x, y)` and a bool; reaching it is a
  box-to-point distance test. A cell would mean a row in `MATERIALS` and four
  questions with it — what happens when the objective is dug, burnt, displaced,
  or buried by a collapse — two of which are design decisions rather than
  implementations. An *entity* would mean the first thing that is neither a cell
  nor the player, which is the entity/component itch this file already refuses
  at the top. **A point defers all of it at the cost of one limitation worth
  stating: the objective cannot be interacted with, only arrived at.** The full
  "Objective + Extraction" item in ROADMAP.md is where that gets chosen
  deliberately, and it should read this entry rather than inherit the point by
  default — the point is a spike's answer, not a recommendation.
