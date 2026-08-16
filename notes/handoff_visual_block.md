# Handoff — the V-track renderer block, mid-block

You are picking up a **visual-rework block** on Toop/Xoco, a from-scratch C++20 /
SDL2 cellular-automata pixel-physics game. Repo root is `code/`; **run every
command from there.** Read `code/CLAUDE.md` first — it is short and it is the
working agreement, not a summary.

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
- **Do not attempt band separation as a PALETTE edit** (step 0's finding: it is a
  renderer defect, not a palette one).
- **The README Manual Tester Checklist is owed after each step and you cannot run
  it** — flag when it is owed and name which steps matter.

## Where the block stands

`ROADMAP_ITEMS.md` item 8 is the authority; it is five steps run strictly in
order, and each step's reason for coming before the next is the point.

| step | what | state |
|---|---|---|
| 0 | Rewrite the two dead `notes/` files against the CnC reference frames | ✅ done 2026-08-16 |
| 1 | **V17** — extract composition to `render/frame.cpp` changing nothing, then checksum it | ✅ done 2026-08-16 |
| 2 | **V11 core** — ordered layer table, parallax onto `Camera`, factors generated into a header | ✅ done 2026-08-16 |
| 3 | **V11's tint bullet + V7-rest's darkening half** — the light pass gains a multiply | ✅ done 2026-08-16 |
| 4 | **V18 — write the split view down, build none of it** | **next, not started** |

**Nothing in this block is committed.** It all sits in the working tree —
`git status` shows ~15 modified and 6 untracked files, including the three new
ones (`src/render/frame.{h,cpp}`, `tests/test_golden_frame.cpp`).

## What step 3 actually built

Two knobs that are one idea at two scopes, both expressed in the layer table in
[src/render/frame.cpp](../src/render/frame.cpp):

- **A per-layer `Grade`** — a plain RGB multiply on every row, applied with
  `SDL_SetTextureColorMod` (textures) or a `graded()` helper on
  `SDL_SetRenderDrawColor` (rects). No extra draw call, no extra texture. The
  mountains sit at **0.60**; every other row is identity.
- **A `Lighting::Grade` row** holding a full-screen `SDL_BLENDMODE_MOD` quad
  driven by `Params::world_grade` — for night, weather, biome. **It has no
  caller**, is identity today, and draws nothing at identity.

**The split is not tidiness.** A frame-wide multiply scales every band by the
same factor and leaves every ratio between them exactly where it was, so it
cannot separate anything. Separation is necessarily per-layer. (This is a
correction to `notes/reference_observations.txt` entry 2's *reasoning*, which
said the repair was pushing a band's range down "globally". The finding stands;
that clause does not.)

**Ordering is load-bearing:** `Lit → Grade → Light → Unlit`. The grade multiplies
and the light pass adds, so grade-then-light is the difference between a fire
that survives nightfall and one that gets dimmed by it. `Lighting` went from a
boundary index to a **rank function**, held by three `static_assert`s: exactly one
light pass; *at most* one grade (zero is coherent, two compose into a third grade
nothing declares); and the grade row's own grade must be identity or it
multiplies twice with neither number saying so. All three were verified against
the code they catch, then the file was restored byte-identical.

### The measurement that drove it

Entry 2 **understated itself**. Its channel ranges suggested overlap; in Rec. 709
luminance over the shipped art, **the sky averages 26 and the mountains were flat
28** — p05 and p95 both 28, no internal variation at all. Two levels of
separation out of 255 between the frame's two most distant bands, with the far
one the *brighter*. **A range is not a separation, and quoting one for the other
is how this understated itself.** At 0.60 the pair reads 26 against 16.

Direction is **darker with nearness** — the daylight instinct, that aerial
perspective washes distant things *toward* the sky, is backwards at night, when
the sky is the only bright thing.

### Golden checksum

`0x3d729ad7fbcaa839` → **`0x9d9e92a81c4df07b`**, updated in the same change. First
time this project's composed frame has changed on purpose. **The whole mechanism
was built at identity and run against the old number first — it held** — so the
move has exactly one cause. That is the house procedure, now used three times:
*build the no-op half, prove it, then build the half that changes something.*

Checksum history is four moves now, recorded in `.claude/rules/simulation.md`.
`git log -S` on the constant is how you read it, so **a legitimate frame change
puts the new checksum in the same commit as the change.**

### Three findings worth more than the code

1. **The estimate was a week; it took an afternoon, and that is evidence about a
   refusal.** This was priced as the item that would finally spend
   `SDL_ComposeCustomBlendMode` — the escape hatch the renderer-vs-shader refusal
   has held through two examinations. It needed no custom blend mode at all.
   `SDL_BLENDMODE_MOD` is stock and `SDL_SetTextureColorMod` does the per-layer
   half for free. **All three named escape hatches are still unspent, and the
   first item that looked like it would spend one didn't.** The software backend
   supporting both is what made it usable — a GPU-only path would have been
   invisible to the golden test.
2. **A check can be aimed at the right claim and still not see it.** The ordering
   test was first written as "the brightest pixel stays bright" — a true
   statement about the design and a bad instrument for it. The fixture's fire
   already saturates at 255, so a correct ordering and a reversed one both
   squeeze against the clip and landed 12% apart. Replaced with the light pass's
   *contribution* (compose with and without, subtract), which separates 1.0 from
   0.5 cleanly. **The failed attempt is written into the test comment rather than
   quietly deleted.**
3. **One thing ships without a caller, on stated terms.** `Params::world_grade`
   has no setter. It becomes a defect the day it is still unset *and* the
   ordering claim has stopped being checked by anything. Trigger for spending it:
   V8's time-of-day, or a second biome. If neither arrives, delete the row rather
   than leave it as decoration.

## Checklist results — step 3 is confirmed, and this needs writing up

The user ran the README Manual Tester Checklist after step 3 and reported:

```
12 looks good on both
11 looks good
5 looks good
```

Read that against what each step was asking:

- **Step 12** (new, written for step 3) had two halves and **both came back
  clean**. Its primary question was whether the mountains read as a silhouette,
  with the failure to watch for being **over**-correction — 0.60 was chosen from
  a luminance measurement, which is not the same as chosen by eye, and nobody had
  looked at it. They have now, and it holds. Its second half asked whether *only*
  the mountains moved; a whole-frame darkening would have meant `world_grade` had
  acquired a caller it should not have. It has not.
- **Step 11** — parallax ordering and the pan-limit seam, re-run because the
  layer table changed shape. Clean.
- **Step 5** — a fire is not dimmed by the grade. This is the ordering claim
  (grade multiplies, then light adds) observed rather than asserted, and it is
  the one result the golden test's contribution check could only argue for.

**This is not yet recorded anywhere.** It owes a `PLAYTEST_LOG.md` spot-check
entry — **symptoms only, no fixes**, per the file-split table in `CLAUDE.md` —
and step 3's note in `ROADMAP_ITEMS.md` should stop saying the number is
unlooked-at, because that is now false and a false claim there is the exact
failure mode this project keeps having.

## Blocking / owed

- **`SlopPhysics.exe` has not been relinked since step 3.** The user had the game
  running (PID 17812) and the link failed with `LNK1104`. Everything *compiles*,
  `main.cpp` included, and all twelve suites run — only the final link was
  blocked. Confirm the exe is current before trusting anything on screen.
- Step 3's docs are done: `TUNING.md` (new "Depth grading" section, rows, and a
  dated History line), `ROADMAP.md`, `ROADMAP_ITEMS.md`,
  `notes/reference_observations.txt` entry 2 (marked `*** ACTED ON`),
  `README.md` (step 12 added), `CLAUDE.md`, and the assets / simulation /
  documentation rule files.

## What is next

**Step 4 — V18: write the split view down, build none of it.** An afternoon. The
deliverable is a written design, not code.

After the block closes: **E10 (powders come to rest) resumes the head of the
queue.** It was unblocked and ready and got out-prioritised, deliberately —
nothing about it changed.

**V7-rest is still open**, one item smaller. The step table's title oversold it:
the multiply — the darkening half — is done, but **non-fire light sources are
untouched**, and that is all V7-rest ever was apart from the multiply. The ID
stays `V7-rest` despite the name now being wrong, because four documents cite it.

## Verification, every step

```bash
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure    # 12 suites, ~1s, all of them
```

There is no reason to run a subset. `golden_frame_test` is the twelfth and the
only one that links SDL2-static; it still needs no display. **It hashes software
rasterisation and is blind to a GPU-only defect — do not quote it as covering the
shipped frame.**
