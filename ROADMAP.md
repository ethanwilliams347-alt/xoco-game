# Game Roadmap

This document tracks the concrete, sequenced engineering work — what's next,
what's done, and why each step is ordered the way it is. The project's goals and
scope discipline, performance methodology, and long-lived technical decisions
live in their own documents:

- **[VISION.md](VISION.md)** — project goals, scope discipline (why the plan is
  sized the way it is), and the Long Term wish list.
- **[PERFORMANCE.md](PERFORMANCE.md)** — `grid_bench` numbers, measurement
  methodology, and the mistakes that methodology exists to prevent.
- **[ENGINEERING_NOTES.md](ENGINEERING_NOTES.md)** — deferred technical
  decisions and the reasoning behind them.
- **[README.md](README.md)** — how to build, run, and test the game, including
  the short `## General Testing` fundamentals pass.
- **[MANUAL_TESTING.md](MANUAL_TESTING.md)** — the Manual Tester Checklist in
  full (**thirteen steps as of 2026-08-17**), and the list of what is currently
  owed to the tester.
- **[PLAYTEST_LOG.md](PLAYTEST_LOG.md)** — what was asked at each playtest and
  what came back. **Questions and answers only.** No plan, no root causes, no
  fixes: a defect's symptom is recorded there and everything downstream of it —
  why it happened, what was done, what that cost — lives here, in the wave that
  spent it.
- **[ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md)** — closed work, moved out of this
  file. **Nothing is ever required to read it**; anything in it that still binds
  open work was absorbed into the item it binds before it went.
- **`notes/`** — informal lore and feature brainstorming, upstream of anything
  here.

**Three conventions govern this file, all learned the hard way.**

**This is the only document that carries development steps.** The order of work
used to live at the bottom of `PLAYTEST_LOG.md`, on the reasoning that a roadmap
absorbing every playtest finding stops being readable as a statement of intent.
The reasoning was sound and the remedy was not: two documents each holding part
of the sequence meant neither was authoritative, and the wave table went a full
session describing the material hotbar as queued after it had shipped. **A plan
split across two files is a plan that disagrees with itself.** The fix is
[Waves](ROADMAP_ARCHIVE.md#-waves--sub-plans-that-preempt-the-tracks) — findings are absorbed here
without being interleaved into the tracks, which is what the split was reaching
for.

**Live work comes first, finished work leaves the file.** Everything here is
open; closed work is in
[ROADMAP_ARCHIVE.md](ROADMAP_ARCHIVE.md), kept for its reasoning rather than its
status and **required reading by nothing**. An early revision interleaved the two
and read as a task list while being sixty percent retrospective — a reader could
not find the next item without scrolling past four completed sections written in
the imperative. Moving the finished sections to the bottom fixed the reading
order and not the size: on 2026-08-17 the `## ✅ Shipped` section was 86 KB of a
448 KB file, and the other 362 KB was **also** mostly closed work, sitting inside
the live tracks. `W4` is the item that took the rest of it out.

**Steps point at code by name — a function, or a line quoted out of it — never
by line number.** F1.1 on its own shifted `grid.cpp` by five lines, `grid.h` by
seventeen and `main.cpp` by fourteen, which silently falsified every numeric
reference in this document including the one describing the bug F1.1 had just
fixed. A plan read across many sessions cannot use an anchor that moves every
time the plan is executed.

---

### 🚩 Definition of Done — v0.1 Vertical Slice
The single milestone that matters. Everything before Long Term (VISION.md)
serves this:


If that loop is not fun, no amount of factories or stock markets will save it.
If it *is* fun, it is a demo worth showing and a foundation worth expanding.

### 🧭 Where this stands

**Shipped:** the engine and its harness — data-driven materials and reactions, a
fixed timestep, chunked dirty-rect updates, a player body, digging, rigid
structural collapse, and a headless test rig that is now **eleven suites at 421
checks** *(the count read "nine suites at 296" until 2026-08-12 and "ten at 333"
until 2026-08-14; the tenth suite had been there for some time and nothing
re-counted, and `debug_test` is the eleventh)*. Then **F1–F4** (determinism,
`Run`, camera and world-space coordinates, and a way to get a level into the
grid), **E1–E3** (liquids level, heat, fracture), and **V1–V2** (a backdrop
layer, and a palette chosen against it). All of that is at the bottom of this
file with its reasoning intact, along with **[the correctness
pass](ROADMAP_ARCHIVE.md#correctness-pass)** that followed a full read of the source. Then **V5**
(the art direction — see [notes/art_direction.txt](notes/art_direction.txt)) and
**V6** (the locked palette and its validator — see
[tools/pixel_art.py](tools/pixel_art.py)). Most recently **V7**'s emissive half
pulled forward off its own gate, **V10** (the reticle and the material hotbar —
the reticle is the cursor now), and **V3**/**V3.1** (the player sprite, then
animation from a sheet). **Those five are checked in place in the V track rather
than moved down here**, which is a deliberate change of convention at V5 and not
an oversight: their entries are long, they are cited by `notes/`,
`PERFORMANCE.md` and each other, and moving them would break the one thing this
file is strict about. The Shipped section below is where E1–E3 and V1–V2 went
and it is not growing.

**Open, in order:** the **instrumentation sitting** (which absorbed the fluid
spike), **S0**, then the rest of **Engine & Visual Depth** — the tier this
project is actually sold on — then the **Medium Term** slice, then
**Presentation & Tooling**. *(P2 shipped, session 5 ran, and E4 came back "no",
which is a closed loop rather than a skipped item. **Wave 4, E9's steam half and
F5 all closed 2026-08-12** — E9 is now done in full, and it is the first thing
on this line to be retired by a defect being reported three times rather than by
a plan.)* [ROADMAP_ITEMS.md](ROADMAP_ITEMS.md) carries that order as a table
with sizes against it and is the file to open first; **this one remains the
authority on why, and it is no longer the authority on what is next.** That
split is a deliberate change made on 2026-08-09 and the reason is in the review
that prompted it: this document is 196 KB, the order lives in six preambles
scattered through it, and a plan that is expensive to reorder is a force against
changing direction.

**V4's second half is done — a scene-authorable prop format**
(`src/scene/props.{h,cpp}`, `assets/test_props.txt`, `props_test`). That was the
item after V10 and V3/V3.1, which between them spent the running order this line
used to state.

**The order changed on 2026-08-09, and the two changes are structural rather
than a reshuffle.** They came out of a review that read the physics source
against the engines this project is measured on, and both are recorded in full
at the items they affect.

- **A gameplay spike (`S0`) is pulled ahead of most of the E track.** Everything
  shipped to date is engine or visual foundation, and the whole of the game sat
  behind the whole of the engine tier. `VISION.md` names that exact risk in
  writing — under-building looks like discipline right up until the playtest
  gate — and the running order was doing it anyway. S0 is the thin half of two
  Medium Term items, built now, so the run becomes losable. It is also the only
  thing in the plan that can settle the hook and combat questions by playing,
  and both of those currently block the whole slice.
- **E5 splits into E5a and E5b**, because it was two items wearing one number:
  the per-cell velocity that E6, E4's shove and V9's debris all actually need,
  and the air/pressure field, which is far larger and goes after the slice.
  Bundled, the enabler looked as expensive as the research project attached to
  it, which is most of why it kept not being started.

**Seven more items are new as of 2026-08-11, and they are one plan rather than
seven ideas: `V12`, `V13`, `V14`, `V15`, `V16`, `E12` and `S1`.** They come from
a stated commitment to what the visual system is *for* — varying pixel
resolutions and sprite sizes in one scene, procedural animation, animated rather
than static backgrounds, and entity bodies that granulate locally when damaged —
and the place they are argued is [the V track's own section on
it](#the-visual-system-this-track-is-now-building-toward). **Two things about
how they were admitted are worth knowing before reading them.** Five of the
seven pass the tier's normal test on their own: a colour key that forbids a soft
edge anywhere, a draw path that turns a denser drawing into a bigger object, a
backdrop whose only depth cue stops when the player does, an articulation limit
already written down twice, and a walk cycle that assumes a floor this game does
not have. **The other two — `V15` and `E12` — are admitted by conditions that
`notes/procedural_animation.md` and `notes/granulating_enemies.md` wrote down in
advance**, each naming what would stop it being a note, and the commitment
supplied it. That is a better form of admission than an observation, not a
weaker one, because it was specified before the thing that satisfied it existed.
**Nothing in the running order moved for them:** wave 4, E9's steam half, the
fluid spike and `S0` are unchanged, the visual block sits where `V11` already
sat, and `S1` is blocked on the combat decision it does not get to pre-empt.

**Three items are new: `E10`, `E11` and `V11`,** plus `P3` in the performance
track. E10 (powders come to rest) is the cheapest large improvement in feel
available and it was found by reading `step_powder` against the reference
behaviour. V11 makes the visual system cheap to redirect, which is a stated
requirement that nothing in the plan was serving. E11 is four table columns that
were each too small to be an item alone. **Nothing was renumbered** — the IDs
stay stable, per the rule below.

**Session 5 ran, wave 3 closed, and the session is worth reading before anything
is picked up.** [Wave
3](ROADMAP_ARCHIVE.md#wave-3--the-brush-destroyed-water-and-the-elevator-it-was-hiding) closed on
W-2 and W-4 exactly as written, and then the session produced six defects and
five observations against parts of the game nobody had scheduled — **four of the
six defects came from README's nine-step checklist, which the session was not
booked to run and which everyone treats as a formality.** The previous version
of this paragraph warned that four items had been sequenced without anyone at a
keyboard; the session bore that out at a rate of roughly one finding per
checklist step, and the ordering above is a direct consequence rather than a
reshuffle. Full record: [session 5
results](PLAYTEST_LOG.md#session-5-results--wave-3-closes-and-the-water-underneath-it-does-not).

**The session's most valuable output was not a fix, it was a discrimination, and
it changed a plan entry.** W-5 was written in advance to separate two possible
causes of one symptom — *"if the lift is gone and the staggering is not, that is
a new finding about the flow model rather than about venting"* — and it came
back on the second branch. That converted an unlocalised "water is bugged" into
a finding against `step_fluid`'s lateral run, and **withdrew E11's "no action is
proposed"**, which had classified the jump as a harmless known property of the
genre. The property was described correctly and filed wrongly. **The
generalisable form: a checklist row whose two answers point at two different
subsystems is worth more than a row that can only pass or fail**, and it costs
the same to write.

**Two findings arrived attached to the wrong mechanism, and both were caught by
reading code rather than by re-testing.** Phase B asked four questions about the
*absence* of player/material interaction; E-1 came back describing something
that very much interacts (`resolve_overlap`, now D2 — and a containment failure)
and E-2 came back describing a step-height constant. **That is not a failure of
the phase, it is why its "no" is trustworthy** — but it is the second time this
project has had a finding wearing the mechanism the tester was primed to look
for, after A7b, and the remedy both times was to go to the source before filing
the work.

**Read this before picking V8 out of the V track's running order, because the
order is now misleading about it.** V8's first three layers are *built* — sky
and mountains ship with parallax from `main.cpp`'s `PARALLAX_SKY_X/Y` and
`PARALLAX_MOUNTAIN_X/Y`, and trees at factor ~1 shipped as V4's prop slice,
drawn before the cell texture exactly as `notes/art_direction.txt`'s layer model
specifies. What remains in V8 is a second biome, time-of-day variation, and a
third depth band, and **all three are gated on a reference that does not exist
yet**: none can answer this tier's first admission question, because nothing in
the built game is wrong or reads badly there. Picking V8 now would be
reference-driven breadth wearing engine clothes, which the preamble below names
as the exact failure mode to watch for. **V4's prop half was taken instead and
is the worked example of the difference** — it could name what was wrong (no
format, so no second scene could have props) and what it unlocked (Quantum
Worlds). V8's remainder can name neither yet, and the honest move is to leave it
until a second biome actually needs one.

**One small piece of V8 is real and is not the rest of it:** its parallax
factors are duplicated between `main.cpp` and `tools/generate_backdrop.py` with
nothing enforcing agreement, and the failure is a seam at the pan limit. V3.1
closed the identical duplication for the player sheet by generating the header,
which is the precedent to copy. That is an afternoon and it is not a tier item.

**One thing to read first if you have been away:** the correctness pass linked
above found that V2's palette retune had silently emptied the startup scene, and
that a settled pool of water never went to sleep. Both are fixed and both now
have tests, but the *reason* they went unnoticed is the part that changes how
the next item should be built — see [What the correctness pass changed about how
to work here](ROADMAP_ARCHIVE.md#correctness-pass-lessons).

---

## 🛠️ W — The workbench (how this project is worked on)

*New track, opened 2026-08-17 out of an external review of the repo. **It is the
next thing to be worked on**, ahead of V22, and the reason it can go first is in
the ordering note at the bottom of this section rather than in enthusiasm.*

**Every entry in this section is wrapped at 80 columns on purpose.** That is
`W1`'s convention, applied to the first text written after it was decided, so
the convention has at least one compliant example before anybody is asked to
follow it.

### The finding this track exists to answer

The review's one structural claim, and everything below is downstream of it:
**the reasoning in this project is archived rather than indexed.** The rule at
the top of `CLAUDE.md` — that the reasoning is the deliverable — is correct, is
being followed, and is producing documents that are accurate. The review checked
the most falsifiable claim it could find, `CLAUDE.md`'s "the full suite is 14
suites", against `ctest --test-dir build -N`, and it is true. *(Its first
measurement said otherwise: `grep -c add_test CMakeLists.txt` returns **20**.
Six of those twenty are comments explaining why `grid_bench` and the five probes
are deliberately **not** registered. **The doc was right and the grep was
wrong**, and that is recorded here rather than dropped because it is the exact
shape of false alarm this project's own rules invite — a reader who trusted the
cheaper measurement would have "corrected" a true claim into a false one.)*

What is failing is not the policy. It is the **storage format**, and it fails in
four measurable ways.

**1. The doc corpus is the size of the codebase.** 1.09 MB of Markdown against
1.20 MB of `src/` + `tests/` + `tools/`. `ROADMAP.md` alone is 399 KB — 69% the
size of all of `src/` — and 67,371 words. `CLAUDE.md` already concedes the
consequence, instructing readers to "search it, don't read it front to back". A
document that can only be grepped is a database with no index.

**2. The four heaviest documents are unwrapped, and that multiplies the cost of
every search into them.** A grep hit is atomic at the line, so a 3,000-character
line returns 3,000 characters whether or not the match needed them.

| File | Lines | Avg line | Max line | Lines >500 chars |
|---|---|---|---|---|
| `ENGINEERING_NOTES.md` | 91 | **601** | 2,386 | 53 |
| `ROADMAP.md` | 1,008 | **394** | 3,083 | 367 |
| `ROADMAP_ITEMS.md` | 641 | 285 | 4,102 | 128 |
| `PERFORMANCE.md` | 428 | 205 | 1,688 | 69 |
| `README.md` | 901 | 63 | 508 | 1 |
| `PLAYTEST_LOG.md` | 975 | 83 | 1,042 | 17 |

Measured: `grep -C2` for `determinis` in `ROADMAP.md` returned **9,930 bytes for
20 lines**. The same shape of query against wrapped text returns about 1.3 KB.
**The project already wraps** — `README.md` and `PLAYTEST_LOG.md` are clean — so
this is an inconsistency to remove, not a new convention to import.

**3. The plan is written twice, by instruction.** Extracting every item ID
matching `(E|V|P|F)[0-9]+[a-z]?` from both roadmap files gives **48 IDs in
`ROADMAP.md`, 48 in `ROADMAP_ITEMS.md`, and 48 in both** — a perfect overlap
across 582 KB, with near-identical section headings on top of it. This is not a
discipline failure. `CLAUDE.md`'s routing table says *"ROADMAP.md (the why) /
ROADMAP_ITEMS.md (the order)"*, and **every item has both a why and a
position**, so the table mandates that every item be written in two places and
thereafter maintained in step. **The clearest evidence is in this file's own
preamble**, which states "This is the only document that carries development
steps" — a sentence that was true when written and that `ROADMAP_ITEMS.md` has
falsified. By the project's own standard that is worse than no claim, and it is
`W4`'s job.

**4. Nothing is mechanised.** There is no `.claude/settings.json` in the repo at
all — no permission allowlist, no hooks. The consequence that matters is not the
prompting; it is that **the project's self-declared first risk, "a stated rule
that stopped matching the code and kept being believed", is defended only by
human vigilance.** Four commits exist purely to repair such a claim, the most
recent being `814ac71`, "Correct the suite count in CLAUDE.md and README, which
V23 made false". Vigilance caught those four. It is the wrong instrument for the
fifth.

### What the review checked and found healthy, so nobody spends a session on it

Recorded because "not a problem" is a finding, and an unrecorded one gets
re-litigated.

- **`Grid` is well factored and is not to be split.** `grid.cpp` is the largest
  translation unit at 1,664 lines, and it is 35 named methods — `step_powder`,
  `step_fluid`, `step_thermal`, `vent_fluid`, `fracture_landing`, `seek_level`
  and the rest — averaging 47 lines. Size here is subject matter, not sprawl.
- **The 14 separate test executables stay separate.** The isolation is what the
  `ENGINE_SOURCES` / `RENDER_SOURCES` split buys, and the whole suite runs in
  about a second. There is nothing to win.
- **The `.claude/rules/` three-way split, `TUNING.md` and `PLAYTEST_LOG.md` are
  all load-bearing separations** with their arguments already written down. None
  of them is a consolidation candidate.
- **`main.cpp`'s `#include <random>` is not an invariant violation.** It is used
  once, at `std::random_device rd;`, to pick the world seed — which is precisely
  the boundary the determinism rule draws.

### The items

**`W1` — Reflow the four unwrapped documents to 80 columns. Shipped
2026-08-17.** *(afternoon)*

*What it cost and what it bought, recorded because the item's own verification
clause asked for exactly these numbers.* Line counts went 1,220 → 6,158
(`ROADMAP.md`), 695 → 2,466 (`ROADMAP_ITEMS.md`), 92 → 785
(`ENGINEERING_NOTES.md`), 429 → 1,377 (`PERFORMANCE.md`); average line length
394/285/601/205 → 68/77/71/62, the two above 70 being table rows, which are
exempt. **Word sequence and link count are identical per file, verified
mechanically against `HEAD` rather than by reading** — that check is the whole
safety argument for a scripted bulk rewrite of the corpus, and it is the check to
repeat if this is ever done again. The retrieval gain, three queries run against
the old and new `ROADMAP.md` back to back: `EASE_PER_SEC` 2,014 → 372 bytes,
`determinism` 29,474 → 5,062, `parallax` 54,607 → 9,726. **About a fifth to a
sixth, which is what the item predicted, and the prediction is left standing
above rather than rewritten to match.**

*Two things the pass learned that the item did not anticipate.* **A per-line
wrapper is not enough — paragraphs have to be regrouped and rewrapped**, or a
long line's tail becomes a two-word orphan line and the file reads worse than it
did unwrapped. And **four-space-indented prose under a nested list is not an
indented code block**; treating it as one, which is the conventional Markdown
reading, left eleven of `ROADMAP.md`'s longest lines untouched. There are no
indented code blocks in these four files, which is what made the simpler rule
safe. **The wrapping convention now lives in `.claude/rules/documentation.md` as
a rule the tree obeys**, with the before/after numbers; it used to be stated
there as a rule four named files violated.


`ROADMAP.md`, `ROADMAP_ITEMS.md`, `ENGINEERING_NOTES.md`, `PERFORMANCE.md`. Pure
formatting, **zero information loss, no decision touched, no wording changed** —
which is what makes it the first item rather than the most valuable one. It also
repairs `git diff` and `git blame`, which currently render a one-word change as
a 3,000-character line rewrite. *Verify:* word count and link count unchanged
per file before and after; `ctest` untouched by construction; a spot grep costs
roughly a sixth of what it did.

**`W2` — `.claude/settings.json` with a permission allowlist. Shipped
2026-08-17.** *(afternoon)*
`cmake`, `ctest`, `git`, `python tools/*`. The mechanical half of the friction.
Note that this adds no dependency — it is a config file the harness already
looks for.

*Shipped as 40 rules, and the count is the interesting part.* The item named
four things to allow and the file lists ten times that, for two reasons the item
did not anticipate. **This project is worked through two shells, not one** —
`PowerShell` is the primary here and `Bash` is also available, they take
different syntax, and a permission rule is scoped to the tool that runs the
command. So every entry is written twice, once per tool, and a rule that exists
for only one of them is a rule that silently does nothing half the time. And
**`git` could not be allowed as `git`**: the four-word summary hides that `git
log` and `git push` are not the same kind of act. The file allows the reading
verbs (`status`, `diff`, `log`, `show`, `blame`, `grep`, `ls-files`, `branch`)
and the three staging/committing ones the working agreement already governs
(`add`, `rm`, `commit`), and stops there. The exe entries cover the six by-hand
probes in `CLAUDE.md`'s command list; `SlopPhysics.exe` is deliberately **not**
among them, because it opens a window and blocks, and nothing headless should be
launching it.

*There is no `deny` list, and that is a decision rather than an omission.*
Anything unmatched already falls through to asking, so a deny list would only
defend against an allow rule written too broadly — which is a reason to keep the
allow rules narrow, not a reason to maintain a second list that has to stay in
step with the first. The concrete cost of the alternative: denying `git push`
outright would mean the user cannot ask for a push without first editing this
file, which is friction pointed the wrong way. **If a broad allow is ever added,
that is the moment to revisit this**, not before.

*The file is versioned on purpose and `.gitignore` already said so.* It ignores
`/.claude/settings.local.json` and `/CLAUDE.local.md` by name — the machine-local
halves — with a comment explaining that `CLAUDE.md` and `.claude/rules/` are
versioned because they are project instructions. `settings.json` is the third
member of that set, and needed no `.gitignore` change to become one.

**`W3` — Make doc-truth a test rather than a discipline.** *(afternoon)* A
fifteenth `ctest` suite asserting the docs' **checkable numeric claims** against
their sources: the suite count against the registered tests, `Element`'s size
and free-byte offsets against the `static_assert`s in
[element.h](src/physics/element.h), the golden checksum quoted in prose against
the one in [tests/test_golden_frame.cpp](tests/test_golden_frame.cpp),
`FIXTURE_SCENE_CELLS` against [tests/test_scene.cpp](tests/test_scene.cpp).
**This is the highest-leverage item in the track and it is deliberately not
first**, because `W1` and `W4` will move the very lines it pins and pinning them
twice is the mistake V20 and V21 already made one level down. **Scope limit,
stated so it does not creep:** it can only ever check claims that have a
machine-readable source of truth. It cannot check reasoning, and an attempt to
make it do so turns the docs into a format rather than an argument.

**`W4` — One live plan; the shipped reasoning moves to an archive.** *(days)*
`ROADMAP_ITEMS.md` keeps *Next up*, *Running order*, *Decisions owed* and
*Prerequisites*, and each open item **absorbs its own rationale inline** instead
of citing a second file for it. The rationale for **shipped** items — the bulk
of the 399 KB — moves to a dated `ROADMAP_ARCHIVE.md` that **nothing is ever
required to read**. Nothing is deleted; this project does not delete a wrong
prediction and will not start here. **The routing-table row in `CLAUDE.md` is
part of the item, not a follow-up** — leaving it in place would re-create the
duplication on the next item filed. **The judgement call is the archive
boundary** and it is the user's, not a session's: the default is *shipped and
closed*, and the argument against a looser line is that every item left live is
one that has to be re-read forever.

**`W5` — Extract `main()`.** *(days)* `main()` runs from its opening brace to
the end of [src/main.cpp](src/main.cpp) — **1,377 lines**, about 92
blank-delimited blocks, covering display-mode negotiation, texture creation,
backdrop and prop binding, sheet loading, seeding, scene load, objective
planting, the `F9` recorder, the V23 camera wiring and the settings menu, and
then the frame loop. The line count is not the argument. **The argument is the
causal chain:** `CLAUDE.md` requires the Manual Tester Checklist after changes
to `main.cpp` *because the suites cannot reach it*, and the suites cannot reach
it *because it is one function*. So `main.cpp`'s shape is the thing converting
machine-checkable work into human-checkable work — and the human is one person,
who is currently holding the V23 feel report that V22 is gated on. **The pattern
is already proven in this repo and stopped halfway.**
[src/game/run.h](src/game/run.h) is SDL-free for exactly this reason, in its own
words — *"a run that needs a window cannot be driven by a test"* — and
`tests/test_run.cpp` is 562 lines of driving it. The shape: a `boot` unit
returning a populated struct, the per-frame composition joining the existing
`render/frame.cpp`, and a `main()` of roughly 150 lines of SDL lifecycle and
pump. *Verify:* the launch line still prints `Scene: WxH, N cells placed` with
`N` = 334901; `golden_frame_test` unmoved; **and at least one checklist step
demoted to a headless assertion, because if none can be, the item did not buy
what it was admitted on.** **This is the one item in the track that needs the
tester afterwards.**

**`W6` — Trim `README.md` to a front door.** *(afternoon)* It is 901 lines doing
four jobs: build/run/test, 153 lines of benchmark procedure, 534 lines of engine
architecture, and the public fundamentals pass. The benchmark section is a
**lossy restatement** of a file that owns the topic — it shares
`PERFORMANCE.md`'s entire distinctive vocabulary while carrying a fraction of it
(`churning` 5 uses against 69, `p99` 1 against 19). Architecture goes to
`ENGINEERING_NOTES.md`, benchmark procedure to `PERFORMANCE.md`, and README
keeps build, run, test, controls, `## General Testing` and links out. **`##
General Testing` stays public and stays short** — that is already a written rule
and this item must not be read as licence to move it.

### Refused, or deferred with the reason

- **Untracking the generated BMPs is deferred, not scheduled.** `assets/` is 49
  MB, of which about 44 MB is script-generated — `backdrop_mountains.bmp` 21 MB,
  `backdrop_sky.bmp` 16 MB, `test_albedo.bmp` and `test_material.bmp` 6.2 MB
  each — all reproducible from `tools/generate_backdrop.py` and
  `generate_test_scene.py`, with `.git` at 20 MB and growing by tens of MB per
  backdrop regeneration. **The reason it is not scheduled is that generating at
  build time makes Python a *build* dependency where it is presently only a
  *tools* dependency**, which is the "zero new dependencies" invariant, and
  because `golden_frame_test`, `test_scene.cpp` and `rim_probe` all need those
  fixtures byte-identical. **Reopen trigger: clone or fetch time actually
  hurting.** Whoever spends it proves generator determinism first — a generator
  that is one byte non-reproducible turns a checksum suite into a flaky one.
- **Splitting `.claude/rules/simulation.md` is noted and not scheduled.** At 25
  KB it loads for `src/physics/`, `src/game/`, `main.cpp`, `tests/` and
  `CMakeLists.txt` — effectively every code path — and its
  harness-and-build-graph half could scope more narrowly. It is left alone
  because the measured baseline it contributes to is acceptable and because `W5`
  changes which files exist.

### Why this track goes ahead of V22, which is the only ordering claim here

**V22 is blocked on a human and `W1`–`W6` are blocked on nothing.** The V23 feel
report is owed, V22 must not start until it comes back, and question 3 of that
report can still return an answer that changes what V22 is. Running the
workbench track in that window costs the V track nothing at all.

Two second-order reasons, both of which would hold anyway. **`W5` is aimed
directly at the queue V22 is sitting in** — every checklist step it converts to
an assertion is a step the tester does not have to run before the next visual
change ships. And **`W1` and `W4` are cheapest now and never cheaper again**:
both are proportional to the size of the corpus, and the corpus only grows.

**One thing must not be read into this.** The volume of writing is *not* the
defect and reducing it is not the goal — the review's own conclusion was that
the documentation discipline is this project's genuine strength. Every item
above changes where reasoning is stored or how it is retrieved. **None of them
is licence to record less.**

---

## 🟤 Engine & Visual Depth — the selling point

*Three tracks. **E** deepens the simulation, **V** gives it an identity, **P**
pays for both. This is the tier the project is bought on, and it is scheduled
ahead of the gameplay slice on purpose — the argument is in `VISION.md`'s
Project Goals and Scope Discipline and is not repeated here.*

**This section used to carry a budget — "nine items, and nine is a ceiling" —
and that framing is retired deliberately rather than softened.** It was written
when this tier was a concession wedged in front of the slice and had to justify
existing at all. It is not that any more: `VISION.md` names the engine and its
visual design as **the product**, which makes depth here the thing being built
rather than a detour from it. A ceiling on the pillar is a ceiling on the game.

**What replaces the ceiling is a stronger admission test, not the absence of
one.** Scope discipline is unchanged and still points exactly where
`notes/reality_check.txt` aimed it — at the Ideal Systems wish list in
`VISION.md`, which is still arithmetic this project cannot afford. An item earns
a place in this section by answering **both** of these in its own text, and an
item that cannot is filed in `ENGINEERING_NOTES.md` instead:

1. **What was observed.** Something in the built game that is wrong, missing, or
   reads badly — not an idea someone had, and not a feature another game has.
2. **What it makes possible.** Which later item, or which moment a player would
   actually see, is unavailable until it exists.

That is a higher bar than "this would be cool" and a lower one than "never".
Every item below states both.

**Reference footage cannot admit an item here, and it can specify one.**
Gameplay capture of comparable games is a real input to this project —
[notes/reference_observations.txt](notes/reference_observations.txt) is where
what is seen in it gets written down — but it answers the *second* question and
never the first. It shows what is possible; it says nothing about what is wrong
here, and the failure mode is cheap to walk into: an hour of capture yields
eleven engine items, none of which can name anything wrong with this game. That
is breadth spending wearing engine clothes. **The legitimate use is the opposite
direction.** E5a, E6 and E8 are already admitted on observations from this
project, and what they currently lack is numbers — how far a blast throws
debris, how fast it travels, how long a fire front takes to cross a beam. Those
are guesses today. Reference sets the target for an item that has already earned
its place; it does not create items. The V track answers to this differently and
says so in its own preamble.

The E track's observations, restated because they are the reason the track
exists:

- **Water never levels.** `Grid::can_displace` refuses every upward move unless
  the mover is lighter than its target, and `Empty` has density 0 — so a liquid
  can *never* rise. → **E1**, done.
- **Fire is random, not hot.** Ignition was a per-cell dice roll; nothing in the
  world had a temperature. → **E2**, done.
- **Collapses look stiff.** `drop_component` translated an unsupported piece
  straight down with its shape perfectly intact. Masonry descended like an
  elevator. → **E3**, done.
- **Nothing in the world has a speed.** A grain dug out from under a pile falls
  at exactly the rate of a grain blasted out of it, because movement is a rule
  applied once per step rather than an integration of anything. Nothing can be
  thrown, splashed, sprayed or knocked. → **E10** (rest) and **E5a** (motion)
- **Nothing pushes.** The one verb that changes the world deletes a sphere on a
  cooldown. There is no force in the engine at all. → **E6**
- **The interaction space is nearly empty.** Eight materials besides `Empty` —
  and `Charred` is E9's burning state rather than a thing anyone places — over
  six `REACTIONS` rows, four of which are fire. A sandbox is judged on how many
  of the questions "what happens if I put X on Y" have an answer, and today
  almost none do. → **E7**
- **The player is invisible to the grid.** Sand falls straight through the body
  and the unstuck search cleans up afterwards. → **E4**

**Order is E → V → P, and both arrows are load-bearing.** *E before V* because E
items change what materials mean and what is on screen, and tuning a palette or
authoring a sprite for a world that is about to gain thrown debris and
explosions means authoring it twice — the same argument that put E2 ahead of V2,
which held. *P after both* because P1 is a memory-layout change and the layout
cannot be settled before the field set is. **The sharp version of that argument
is retired and the ordering survives it.** It used to read "`Element` has no
padding left, so E5 is the first item whose data genuinely costs memory" — which
was true of the old E5 and is not true of what replaced it: E10 and E5a claim
the already-present `Element::ticks`, so the struct stays at 12 bytes and
nothing in the E track now grows the cell array at all. P still runs after E and
V, for the weaker but sufficient reason that P1 wants to know what the hot loop
actually reads before deciding what to split out of it, and E5a changes that.
**P2 is the exception and now runs first of everything** — see the P track's own
preamble.

**Every E and V item carries its own bracketed measurement** per
[PERFORMANCE.md](PERFORMANCE.md), and the standing rule stands: if one item
alone breaks the frame budget, P1 gets pulled forward ahead of the rest. That
rule was invoked once, at E2, and correctly *not* triggered — the escape hatch
turned out to be a one-line early-out. Read that entry before reaching for it
again.

**P2 put this rule in a state it was not written for, and it is an open decision
rather than a thing to reflex on** — see [Decisions
owed](ROADMAP_ITEMS.md#-decisions-owed). The rule triggers on *an item* pushing
the budget over. P2 is not an item; it is a correction to the instrument, and it
found two scenarios already over budget at the played size (`churning` 211%,
`cascading` 241%) with nothing having got slower. Read literally the rule does
not fire, and that reading is defensible — the two breaching scenarios are the
synthetic ones this file already says the game does not produce, while `sparse`,
which stands in for a real frame, is unchanged at 1.00x. But "the rule
technically does not fire" is the same move as the stale comment P2 just
deleted, so it is written down as a decision with a due date instead of being
settled by whoever reads it next.

**Item IDs are stable and are not renumbered when the order changes.**
`PERFORMANCE.md`, `ENGINEERING_NOTES.md`, `VISION.md` and
`notes/art_direction.txt` all cite items in this section by name; renumbering to
make the list read top-to-bottom would silently falsify four other documents to
save one line of explanation. Where the running order differs from the numbering
it is stated in the track's own preamble.

### E — Simulation depth

*Running order: **E4 → E10 → E12 → E5a → E6 → E7 + E11 → E5b → E8**, with `S0`
between E4 and E10 out of [Medium Term](#-medium-term-core-gameplay-loop) and
`P2` ahead of all of it. **E12 is new on 2026-08-11 and sits after E10 for a
reason that is specific rather than positional** — a crumbling material that
cannot hold a slope reads as a liquid, so built before powders have a rest state
its entire output is a puddle. Changed 2026-08-09 from `E4 → E5 → E6 → E7 → E8`;
the argument is in ["Where this stands"](#-where-this-stands) and the sizes are
in [ROADMAP_ITEMS.md](ROADMAP_ITEMS.md). E4 is still first because it is the
oldest open question in the project and may close as "no" without any code at
all. E10 is next because it is days of work for the largest single improvement
in how the simulation feels, and because it settles the representation E5a then
fills in. E5a is the axis the three after it are built on.*

***The finding that reorganised this track, because four of the items below are
answers to it.*** *A review of `grid.cpp` on 2026-08-09 found four rules —
`vent_fluid`, `make_room_above`, `find_lower_surface` and `fall_if_unsupported`
— each added to fix a real observed artifact, each correct, and each the same
shape: a bounded search of the neighbourhood, with a magic radius picked by
sweeping values and measuring. That is the diagnosis, and it is architectural
rather than a code-quality problem: **the engine answers "where should this go?"
by looking around, rather than by carrying state that already knows.** The
engines this project is measured against — Noita, The Powder Toy, Sandspiel —
each carry two things this one does not: a velocity on every cell, and a coarse
air/pressure field. Nearly every one of those four rules is a symptom of one of
the two being absent. E10 and E5a are the first; E5b is the second, and it
retires three of the four rules outright. The fourth, `fall_if_unsupported`, is
retired by E8. **This is the strongest form the admission test takes** — every
one of those four rules is a thing observed in the built game, and each names
what it makes possible.*

- [x] **E1 — Liquids find their level.** *(done — see
  [Shipped](ROADMAP_ARCHIVE.md#e1e3-simulation-depth))*
- [x] **E2 — Heat, the seventh axis.** *(done — see
  [Shipped](ROADMAP_ARCHIVE.md#e1e3-simulation-depth))*
- [x] **E3 — Collapses break instead of dropping rigid.** *(done — see
  [Shipped](ROADMAP_ARCHIVE.md#e1e3-simulation-depth))*

- [ ] **E4 — The player displaces material, or deliberately does not.**
  *Observed:* the grid does not know the player exists, so material falls
  straight through the body and the unstuck search is what stops that becoming a
  freeze. *Unlocks:* nothing else in this document — which is precisely why it
  is cheap and why it goes first. **This item's deliverable is an answer, not
  necessarily a feature, and it should be settled by playing rather than by
  argument.** If the artifact is obvious in practice, do it; if it is not, close
  it as "no" in `ENGINEERING_NOTES.md` and stop paying attention to it.
    - **If the answer is yes, the implementation waits for E5a, and that is the
      one thing this item has learned since it was written.** The hard part was
      never detecting the overlap, it is that shoving cells aside must not
      create or destroy matter, and the obvious cheat — stamping the body into
      the grid as a temporary solid each step — either deletes what was already
      there or needs a full displacement pass of its own. E5a gives displaced
      matter somewhere to *go*: a cell the body walks into is **handed the
      body's velocity and stays in the grid**, which is a shove rather than a
      deletion and conserves for free, since nothing left the cell array to be
      conserved separately. *(This bullet used to say the cell "becomes a free
      particle", which was the old E5 design; the shove is simpler under the
      current one.)* Deciding this before E5a and building it after is the
      correct split, not a stall.
    - **Keep the direction of the dependency**, which is the rule `tool.cpp`
      established: the grid does not know about bodies, bodies read the grid.
      Displacement is the player *asking* what it is standing in and then
      writing through the ordinary write path, not a body pointer on `Grid`.

- [ ] **E10 — Powders come to rest.** *Observed:* `step_powder` rolls a grain
  into any free diagonal and then takes a second fall in the same step, so there
  is no static friction anywhere in the engine. Sand behaves as a very thin
  liquid: piles cannot hold a slope, and a tunnel dug through a dune flattens
  completely rather than partly caving in. **A7, A7b and A7c are the same
  observation arrived at three times** — the comment at `grid.cpp:806` records
  two rules tried and abandoned and reads, in full, "a rule aimed at motion kept
  catching rest, and a rule that spared rest stopped catching the defect". That
  is an exact description of a system with no rest *state* for a rule to aim at,
  and the third rule that shipped works by removing the intermediate moment
  rather than by modelling the missing thing. *Unlocks:* the whole powder half
  of how this engine feels, four E7 rows that are currently indistinguishable
  from sand (gravel, snow, ash, gunpowder), and the representation E5a needs.
    - **The mechanism is one number per material — inertial resistance in the
      engine this is measured against.** A settled grain is at rest and stays at
      rest until disturbed: a neighbour moving is what proposes the change, and
      a per-material roll is what decides it. Once free, a grain keeps sliding
      until it comes to rest again. Cones hold their angle, avalanches trigger
      and then *stop*, and the difference between sand and gravel becomes a
      column rather than a code path. **It is deliberately not a friction
      coefficient or an angle in degrees** — a probability per material is the
      cheapest thing that produces the behaviour and it composes with the
      deterministic hash for free, the same way the powder direction pick
      already does.
    - **It costs no memory — still true after the sitting, and now for a checked
      reason rather than the one written here.** *(2026-08-13: the claim below
      rests on `ticks` being free for powders, and E10 no longer uses `ticks` at
      all. It costs no memory because the per-cell state it needs is a rest bit
      it reads out of E5a's velocity, and because that velocity itself fits in
      three bytes `Element` already had. The measurement is in PERFORMANCE.md;
      the original argument is kept because it is still a correct description of
      what `ticks` is doing.)* `Element::ticks` is read or written in exactly
      six places, all of them either support resolution (structural cells) or
      `step_fire`. `element.h` states the position outright — "Zero for
      everything else. Powders and fluids move one cell per step by their own
      rules and have no use for a clock." That is a whole byte already in the
      struct, already carried by `swap_elements`, already zeroed by `place()`,
      sitting unused on precisely the class of material that needs it. The
      `static_assert` at `element.h:109` does not move and the struct stays at
      12 bytes.
    - ~~**It claims that byte permanently, so the meaning is decided here rather
      than twice.**~~ **Withdrawn 2026-08-13 by the instrumentation sitting. E10
      does not claim `Element::ticks` and neither does E5a.** The bullet read:
      *for a non-structural, non-`Fire` cell the byte becomes a packed velocity
      — four bits of `vx` and four of `vy`, both signed, giving −8..+7 cells per
      step on each axis … the range is not arbitrary: `MAX_FALL_SPEED` for
      structural pieces is already 8, so the two speed limits in the engine
      agree by construction.* Two things were wrong with it and the second is
      the useful one.
        - **Four bits of whole cells per step cannot hold an acceleration.** One
          step of `Player::GRAVITY` is 5/36 of a cell per step, which truncates
          to zero in an integer, every step, forever. `velocity_probe` flies the
          representation and the grain never comes back down inside 600 steps.
          The plan review predicted this from the desk in 2026-08-11 and was
          right; the sitting's contribution is that it is now measured rather
          than argued, and that the same probe rules out the two cheap rescues —
          a stochastic gravity increment has the correct mean and a
          **156%-of-the-mean spread** across grains given an identical impulse,
          and a truncated fixed-point increment makes gravity permanently 10%
          light.
        - **The byte was never the constraint.** `Element` has three unused
          bytes at offsets 1–3, in the alignment hole between `type` and
          `color`, and it always has. Velocity goes there — signed 4.4 per axis
          plus a nibble per axis of sub-cell remainder — and `ticks` keeps
          exactly the two roles it already has. **The instructive part is that
          this bullet's reasoning was good and its premise was never checked**:
          it is a careful argument about how to subdivide one byte, written by
          people who had `sizeof` available and used arithmetic instead.
          `element.h` carries the correction next to the claim.
    - **What E10 actually needs from any of this is one bit, and it survives
      unchanged.** "Is this grain moving" is `vel != 0` under the new
      representation exactly as it was "is `ticks` non-zero" under the old one,
      so E10's design is untouched by the reversal — it reads a rest state, and
      where the rest state is stored was never E10's question.
      **`MAX_FALL_SPEED` still sets the ceiling** and the two speed limits still
      agree by construction; 4.4 signed tops out at 7.9375 cells per step
      against structural material's 8, which is the same agreement with a
      fraction under it.
    - **`element.h`'s third role for this byte needs the same treatment the
      first two got.** The struct already carries a `static_assert` forbidding
      `Fire` from being structural, because `ticks` means two things. This makes
      it three, and the invariant to assert is the same shape: a material that
      is `structural` or is `Fire` may not have an inertial-resistance value,
      and a material that has one may not be either. Write it next to the
      existing one. The correctness pass's rule is that a data-driven design's
      danger lives in the relationships between rows, and this is the third time
      that has been true in this file.
    - *Verify.* A poured pile holds a measurable angle rather than flattening,
      asserted as a number so it can regress. A tunnel roof partially collapses.
      A disturbed pile settles and the chunk goes back to sleep. And the
      benchmark does not regress on `cascading` or `churning` — a resting grain
      now does strictly *less* work than it did, so a regression means the
      disturbance propagation is waking cells it should not.

- [ ] **E5a — Velocity means something.** *(the first half of what was E5)*
  *Observed:* nothing in the world has a speed. Powders and liquids move one
  cell per step by rule; only rigid pieces accelerate, and they do it by falling
  repeatedly rather than faster. A grain dug out from under a pile travels at
  exactly the rate of a grain blasted out of it. *Unlocks:* E6 (an explosion
  with nothing to throw is a hole), E4's shove, E7's gunpowder, and V9's debris.
    - **It lives on the cell, in the grid, and that reverses what this item used
      to say.** The previous design was a sparse list of free particles that
      *leave* the grid and re-enter it on landing, chosen because `element.h`
      says there is no free byte. The byte turned out to exist (see E10), and
      the dual representation was the more expensive half of that decision
      anyway: a particle outside the grid needs an explicit answer to every rule
      in the engine — does it conduct heat, does it react, does `LightField` see
      it, does it occlude — and each answer is either a second implementation or
      a documented "no" that will be wrong later. **The engines this is measured
      against keep the moving cell in the grid** and walk it along a straight
      line through the cells it crosses each step, testing each one. One entity,
      one set of rules, no boundary to maintain. *A genuinely separate particle
      list is still right for matter that must move between cells and interact
      with nothing* — which is exactly V9's effects layer, already scoped as
      non-simulated, and that is where the idea belongs.
    - **The removal note at `grid.cpp:743` argues against the wrong thing, and
      the fix is to edit it rather than delete it.** It records that powder
      acceleration was tried and removed for three measured reasons: motion got
      choppier, a continuously fed stream stratified into sheets one cell apart,
      and `cascading` went 13.1 → 19.7 ms/step with awake chunks going 105/135 →
      135/135. The measurements are good and the conclusion drawn from them is
      too broad. All three are consequences of applying free-fall acceleration
      to *every falling grain*, and all three go away when velocity is non-zero
      only because something **put** it there:
        - *Stratification* was caused by `place()` resetting `ticks`, so the
          brush stamped speed-1 grains on top of speed-2 ones every step. Under
          this design a brush-stamped grain starts at zero **and so do its
          neighbours**, because gravity only accumulates on a cell that is
          already moving. The mechanism that produced the sheets is absent
          rather than tuned.
        - *The frame-time regression* was the whole world gaining a per-cell
          clock. Here a resting grain is still a one-cell-per-step mover doing
          exactly what it does today, so the common case is unchanged — and E10
          makes the resting case *cheaper* than it currently is.
        - *Choppiness* was always a property of drawing whole cells on a fixed
          tick and cannot be fixed in the simulation at all, which the note
          itself says correctly. It is not an argument about velocity.
    - **No new memory, and as of 2026-08-13 that is measured rather than claimed
      — but it is not E10's byte.** *(The bullet read: "It is E10's byte, now
      carrying real values. `Element` stays at 12." The second sentence holds;
      the first does not.)* The representation is settled and it is **three
      bytes in the alignment hole between `type` and `color`**: `int8_t vel_x`,
      `int8_t vel_y` as signed 4.4, and one `uint8_t` holding a nibble of
      sub-cell remainder per axis. `sizeof(Element)` stays 12 with those fields
      present, and `grid_bench` with them present is inside its noise band on
      every row.
        - **The sub-cell remainder is the half of the requirement the plan never
          wrote down**, and it is why one byte was never going to be enough
          regardless of how it was divided. A cell's position is a cell index. A
          velocity finer than one cell per step therefore needs somewhere to
          keep the part of a cell that has been crossed and not completed —
          which is precisely what `Player` carries `rem_x` alongside `vel_x`
          for, and has since before F5. Every version of this decision,
          including the two that argued for spending a second byte, budgeted for
          velocity alone.
        - **Gravity is applied by differencing a running total taken off the
          global step counter**, which is Bresenham's line algorithm pointed at
          an acceleration. 5/36 of a cell is 2.222 sixteenths; a truncated
          increment of 2 makes gravity 10% light on everything thrown,
          permanently and in one direction, which the probe measures as a range
          of 252 cells against the reference's 228. Differencing `floor(n * 20 /
          9)` gives an increment alternating 2,2,2,2,3 whose mean is exactly
          right, and it needs no per-cell accumulator because the step number is
          state the engine already has. **Flown at all nine phases** — since a
          cell launched on an arbitrary step starts at an arbitrary phase — the
          spread is 4 cells in 228 and the worst deviation from the `fx` 16.16
          reference is **2 cells**.
        - **What was rejected, with the number that rejected it.** A whole-cell
          integer velocity with gravity applied stochastically at 5/36 per step
          is deterministic, costs no storage, and is the project's own idiom —
          and 64 grains handed an identical impulse land across a **364-cell
          spread on a 233-cell mean**. An explosion under it would not throw
          debris, it would scatter it. Recorded because the idea is a good one
          that fails on a measurement rather than on an argument, and it will be
          proposed again.
    - **Integer arithmetic, one cell at a time.** Both are already in the
      project and both are load-bearing. Movement resolves one cell per axis per
      iteration, exactly as `Player` does it, so tunnelling is impossible by
      construction rather than by being fast enough. No floating point anywhere,
      because F1 spent seven steps making `Grid` a pure function of its seed and
      F1.7 wrote that down as an invariant.
    - **Four traps, each a known failure mode of something this engine already
      does.** *Conservation* — a moving cell is still **in** the grid, so the
      existing conservation test keeps working unchanged, and that is a real
      advantage of this design over the previous one; say so in the test rather
      than leaving it as luck. *The wake rule* — a cell arriving in a sleeping
      chunk wakes it, the same as every other write, and a cell that still has
      velocity must keep its own chunk awake or it freezes mid-flight, which is
      the same bug as the boxed-in `Fire` cell. *One definition of solid* — what
      stops a moving cell is `is_solid`, the same function the player collides
      against and the dig ray stops on. *A ceiling* — the four-bit range is the
      ceiling, and unlike a list bound it needs no overflow policy.
    - *Verify.* Same seed and same input produces byte-identical results, which
      is F1's invariant extended to the new axis rather than assumed to survive
      it. A cell fired at a wall at full speed lands *against* it (the
      anti-tunnelling case, the same shape as the dig ray's). A world that is
      disturbed and then settles goes fully back to sleep. And conservation,
      which for once needs no new machinery.

- [ ] **E6 — Explosions.** *Observed:* the only verb that changes the world is a
  dig that deletes a fixed sphere on a cooldown. Nothing in the game applies
  force. *Unlocks:* the moment that shows every axis at once — heat (E2),
  fracture (E3), thrown debris (E5a) — and it is the single most legible thing
  this engine can put on a screen.
    - **This is also a candidate answer to the question `VISION.md` leaves open
      on purpose**, which is what the hook finally is. That document is explicit
      that the slice as specified — walking, jumping, and one dig tool — may be
      too thin to be fun, that under-building looks exactly like discipline
      until the playtest gate, and that naming the hook early would quietly
      commit the design to whichever comparison got written down. This item does
      not name it either. What it does is make the question *answerable by
      playing*, which is the same move E4 makes and the only honest way this
      document has ever settled a design question.
    - **Built as** a radius, a falloff, a heat deposit, a conversion pass and an
      impulse handed to E5a — in that order, because each stage is one of the
      existing axes and none of them is new code. Everything it writes goes
      through `set_element` / `paint` / `swap_elements`. A radial write that
      touches `cells` directly will produce material frozen in mid-air, and the
      tests that catch it are the chunk tests in `test_grid.cpp`.
    - **It must not become a second destruction system, and this is the trap
      most likely to be walked into.** The dig tool is a degenerate explosion —
      a radius with no impulse, no heat and no falloff. If explosions ship as a
      parallel implementation there will be two answers to "what does
      destruction do to a structure", they will drift, and the drift will
      present as a bug in fracture. The same argument that made `set_element`
      and `paint` share one private `place()` applies here and is stronger,
      because the two paths are further apart.
    - **The cost is the first in this engine whose worst case a player sets, and
      that is a real change in kind.** Every existing worst case is a property
      of the world; this one is a property of what someone chooses to do in it.
      Cost goes as radius squared, and an unbounded radius is an unbounded
      frame. It needs a ceiling, and the measurement has to be taken at the
      ceiling rather than at a typical value, per
      [PERFORMANCE.md](PERFORMANCE.md) — and the benchmark has to be shown to
      *reach* it, which is the rule E3 wrote into that file after a perfectly
      executed measurement of nothing.

- [ ] **E7 — Breadth: more rows, not more code.** *Observed:* eight materials
  besides `Empty`, only seven of them placeable (`Charred` is E9's burning
  state), over six `REACTIONS` rows, four of which are about fire. The whole
  interaction space a player can probe is fire/wood, fire/oil, fire/water and
  water/steam. *Unlocks:* nothing structural, and that is the point — it is the
  cheapest depth available in this engine precisely because E2 already made
  transformation a temperature-gated table rather than a branch.
    - **Melting and freezing need no new mechanism at all**, which is the
      strongest evidence E2 was the right axis to spend. Boiling is already a
      row with a `min_temp`; melting is the same row shape pointed the other
      way, and freezing is the `max_temp` half that Steam→Water already uses.
      Stone → molten → cooled stone is three rows and no engine change.
    - **Candidates, each one row or a small handful:** `Snow` (V4's row, and
      with heat in the engine it melts by table rather than by special case),
      `Ice`, molten stone, `Acid`, `Gunpowder` (which is E6 wearing a material),
      and smoke as a thing distinct from steam. None of them is scheduled here —
      this item is the budget and the test, not the list.
    - **This is the item most likely to become an endless table, and the bound
      is stated rather than hoped for.** A row earns its place by making an
      interaction *legible* to a player — someone has to be able to discover it
      and be right about what they discovered. Breadth is also the cheapest
      possible way to *look* deep, which is exactly why it is sequenced after
      E5a and E6 rather than before: rows authored before matter can be thrown
      and before anything explodes would be authored against half an engine and
      revisited.
    - **Watch for the hardness signal.** `ENGINEERING_NOTES.md` records that
      `MATERIALS` has no hardness column and that adding one would be an axis
      with no consumer. If this item's rows start reaching for a per-material
      strength number, that is the signal that entry has been waiting for.

- [ ] **E11 — The columns heat and fluids are missing.** *Observed:* four gaps
  found reading `material.h` against what the tables are being asked to express.
  *Unlocks:* nothing structural, and none of the four justifies an item alone —
  they are grouped because each is one column or one short rule, and because
  three of the four become load-bearing the moment E7 starts adding rows.
  Sequenced with E7 for that reason.
    - **`conductivity` is doing two jobs and one of them is heat capacity.** Its
      comment says so plainly — "one number sets both how fast a material heats
      and how fast it forgets" — which is a modelling shortcut, correctly taken
      when heat was new and there was one consumer. It means water cannot be a
      heat sink and metal cannot be a fast conductor that *stays* hot, because
      those two behaviours differ only in the number this column is standing in
      for. One extra column separates them, and E7's molten stone and ice both
      want it.
    - **Heat has no reach.** Conduction is between touching cells only, so
      standing next to a bonfire is thermally free. That is a deliberate and
      correct simplification — `material.h` explains that not simulating air is
      what makes the thermal pass affordable — and it stops being harmless at
      `S0`, where fire becomes a hazard and a hazard you can stand beside is a
      decoration. The cheap version is a short-range term from `heat_source`
      cells only, which is a handful of cells in any real scene, rather than a
      general radiative pass over the world.
    - **Fluids have one number.** `spread` (5 for water, 3 for oil) is the whole
      of what distinguishes one liquid from another in motion, so honey, tar and
      lava are the same substance at different settings. A viscosity term — how
      often a cell is willing to spend its lateral move at all — is one column
      and one early-out.
    - **Lateral flow is a jump, and "no action is proposed" is withdrawn.**
      `step_fluid`'s lateral run walks outward as far as `spread` allows and
      then `swap_elements` moves the cell to the furthest usable landing in one
      step, so a liquid crosses several cells while occupying none of them. This
      is standard for the genre and is *why* streams read as snapping rather
      than pouring. The entry used to end there, on the reasoning that recording
      it as a known property stops someone hunting it as a bug, and that if it
      were ever addressed it would be E5b's business rather than a column's.
      **Session 5 refuted the "no action" half twice in one sitting, and the
      second time under a check written in advance to discriminate.** The
      general checklist returned *"water/oil should be reworked completely, it
      does not flow properly and is bugged"*; then W-5 returned *"still
      staggered clumps"* after W-3 had confirmed the elevator was gone — which
      is verbatim the case the [session 5
      checklist](PLAYTEST_LOG.md#session-5-checklist--the-pass-that-closes-wave-3)
      named ahead of time as *"a new finding about the flow model rather than
      about venting."* **The property is unchanged; the classification was
      wrong.** A known artefact of the genre and the largest visual complaint on
      the record against a material the player sees constantly are not the same
      entry, and only the first of those can be closed by writing it down.
        - **What is not withdrawn is where the fix lives.** It is still not a
          column, and E5b is still the candidate mechanism — pressure
          propagating over several ticks is what makes a *continuous* lateral
          move natural rather than bolted on. But E5b is *large* and sits after
          the slice, and one playtest note is not a reason to pull a month of
          work in front of `S0`; that is the exact pattern the 2026-08-09
          reorder exists to stop. **What replaces "no action" is a bounded
          spike, and its output is a decision rather than a fix:** build an
          instrument that puts a number on the staggering — the `preview_light`
          move, because "still staggered clumps" has no before or after and no
          fluid change can currently be judged — then price the cheap
          alternative against it, which is a lateral move that walks one cell
          per step instead of teleporting. If that reads as flow, most of the
          look is bought for a day. If it does not, or if it costs too much on
          `churning` (already 211% of a frame at the played size), E5b is priced
          with evidence instead of with argument. **Either result closes the
          loop; neither is a rework, and the spike is not a licence to start
          one.**
        - **The residual lift belongs to the same spike and is a separate
          finding.** Session 5's W-3 confirmed A6b's headline symptom gone and
          then reported water still climbing a standing sand column. **That is a
          residual to eliminate rather than a rate to tune** — displacing sand
          must raise a pool's free *surface*, and no configuration makes it
          right for water to occupy a column above that surface. Wave 3 kept
          `vent_fluid`'s straight-swap fallback for a grain deep inside a body
          on the grounds that *"there is no conveyor above it, so its one-cell
          lift never adds up"*; in the played configuration it adds up, so that
          sentence is wrong in the same way this bullet's was — a property
          argued harmless and then seen. The invariant is assertable, which is
          what takes it out of the realm of looks: **no water cell may come to
          rest above the pool's free surface**, splash excepted. `water_probe`
          already measures the quantity, and 3 cells at step 350 was judged
          acceptable on paper and is visible at 3440x1440.

- [ ] **E12 — `Crust` and `Grit`: a material that granulates when damaged.**
  *(new 2026-08-11; the design is
  [notes/granulating_enemies.md](notes/granulating_enemies.md) Part A, path M2,
  and is not repeated here)* *Observed:* **nothing in play is wrong for want of
  this, and the note says so in its own closing section.** What changed is that
  an enemy whose body granulates locally when damaged has been committed to, and
  this is the half of that idea that can be built, played and judged **with no
  actor code at all**. *Unlocks:* `S1`; and before `S1`, crumbling terrain as a
  hazard — a crust ceiling that comes down — which is the cheapest possible way
  to find out whether the feel is worth what `S1` costs.
    - **Admitted the same way V15 is: on a condition the note wrote down before
      the thing that satisfies it existed.** That note is explicit that the
      material is *"weak on the first question"* and that it *"rides along with
      whatever admits combat."* This is that. **The honest framing is that E12
      is admitted by a commitment and V15 by a trigger, and neither is admitted
      by an observation** — both are recorded that way rather than dressed up,
      because the tier's admission test is worth more when the exceptions are
      visible.
    - **Two `MATERIALS` rows, not one row with a mode flag**, which the note
      settles and which is the `Wood` → `Charred` precedent exactly: `Crust` is
      `MoveKind::Static` and structural, `Grit` is a `MoveKind::Powder` at
      sand-like density. `is_solid()` and `is_structural()` are derived from
      `MoveKind`, so `Crust` gets player collision and rigid collapse and `Grit`
      gets piling **with no new branch in the update loop**. A mode flag would
      need a per-cell bit, which `element.h` says outright is the byte that
      costs 500 KB.
    - **The trigger is a `Grid::granulate(x, y)` called from three sites that
      already exist**, each reading as something different:
      `Tool::update`/`march` (struck it and it crumbled where you hit it),
      `fall_if_unsupported` (an overhang comes loose and *becomes* a sand-fall),
      and `fracture_landing` (holds shape through the fall, shatters on impact).
      The third is the cheapest and the best of the three, because it already
      means "this piece just hit something hard" and already has the component
      flood-filled.
    - **A roll per disturbed cell rather than a certainty**, with its own
      `Stream` tag registered in `SIM_STREAMS` — some cells hold and some go,
      which buys a ragged crumble edge for free. **Read `reaction.h`'s jitter
      entry before tuning it**, because that entry records the version that was
      tried, measured, and did not work: jittering the *timing* bought far less
      shape per unit than jittering the *threshold*, and this is the same shape
      of knob.
    - **`REACTIONS` cannot express this and ruling that out in writing is worth
      a line**, because it is the first thing the engine's shape suggests. A
      reaction row is gated on neighbour type and a temperature window; "was
      hit" is neither. `Crust + Fire → Grit` is expressible and may even be
      good, but it is a different behaviour and not this one. **Reactions are
      the wrong axis, not an insufficient one.**
    - **Sequenced after E10 for a reason that is specific rather than tidy:** a
      crumbling crust that cannot hold a slope reads as a liquid, so built
      before powders have a rest state this item's whole output is a puddle. The
      note says this and it is the sharpest sequencing claim either document
      makes.
    - **The accumulated-damage version (M3) is not this item and is after P1.**
      Per-cell integrity buys visible wear and "three hits and it goes", and it
      costs the 500 KB byte at today's prices. **P1 is the item that makes it
      affordable**, and building M3 first buys the same feature at its worst
      price. It is also the first thing that genuinely fires the hardness signal
      `ENGINEERING_NOTES.md` has been holding — **M2 does not**, and conflating
      the two is how that entry gets opened a version early.
    - *Verify.* Headless, entirely: a struck `Crust` cell becomes `Grit` at the
      impact point and its neighbours mostly do not; an unsupported crust
      overhang granulates into a pile rather than dropping as a slab; a crust
      slab dropped from height shatters on landing; conservation is unchanged,
      because granulation is a type change and not a write of new matter.

- [ ] **E8 — Toppling.** *Observed:* E3 named this as the follow-on it existed
  to make affordable, and left it undone deliberately — a fragment that fracture
  has separated is the cheap candidate for it. **Playtest session 1 observed it
  independently and from the other end:** bodies fall flat and land flat, and
  the result reads as lifeless (observation B4). That is the same gap arrived at
  by watching rather than by reasoning, which is the strongest form the
  admission test takes — and it is the worked example of a playtest request
  earning a *tracked item* rather than a wave, per the admission rule in
  [Waves](ROADMAP_ARCHIVE.md#-waves--sub-plans-that-preempt-the-tracks). It was for a while
  carried in both places at once, which is the duplication that consolidation
  removed. *Unlocks:* the last of the three ways masonry can fail; today it
  drops or it breaks, and it never tips.
    - **Toppling and rolling are two items, and this one is only the first.**
      Session 1 asked for "tip, topple, and roll" as a single feature. They are
      separated by an engine boundary rather than by degree: a whole-cell pivot
      is a permutation of a piece's own cells, and a body that *rolls* has left
      the grid — it carries a real transform, is re-rasterised every frame,
      changes cell count under rotation, and needs a resolution pass for the
      overlaps. **Rolling is the body-extraction route described in the bullet
      below**, and it is deferred on cost. *(This used to say rolling was
      deferred behind E5 on the grounds that free particles are the natural
      substrate for off-grid matter. That was reasoning about the old E5 design;
      E5a keeps matter in the grid, so it is not a substrate for anything and
      rolling no longer depends on it. The dependency is the solver, not the
      particle layer.)*
    - **E3's argument was believed to be a hard constraint and it is not, and
      that correction is the main thing that changed here on 2026-08-09.** The
      argument ran: true rigid-body rotation on a cell grid means resampling the
      piece every step it turns, which destroys the exact authored pixels
      [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md) calls the entire visual
      pillar — so the feature would be bought by breaking the reason the engine
      is interesting. **The premise is that the piece has to live in the grid
      while it rotates, and the reference implementation simply does not do
      that.** Noita's route: trace the connected region's outline with marching
      squares, simplify the contour, triangulate it, hand *that* to a rigid-body
      solver, **remove the piece's cells from the grid entirely**, and stamp
      them back each frame from the body's own private pixel buffer. The
      authored pixels are never resampled — they live with the body and are only
      *drawn* rotated, which makes rotation a rendering problem, and rendering
      rotated pixel art is solved. **Explosions and fracture then act on bodies
      properly** rather than on a flood fill, and the eight-flood-fills-per-step
      cost in `resolve_support` goes away with them.
    - **So the objection is recorded as the real one, which is cost and
      dependency rather than possibility.** It is larger than the rest of the E
      track combined; it wants either a physics library (a dependency, against
      the no-bloat rule in `VISION.md`) or a hand-rolled solver, and that choice
      is itself an item; and nothing in the slice needs it. **It is deferred
      past v0.1 on those grounds and not on impossibility.** The previous text
      said it "may close as 'not possible without wrecking the pixel art'", and
      **that sentence is withdrawn rather than softened**: it is the kind of
      line that a reader a year from now takes as settled, and it would have
      closed a door that the engines this project is measured against walk
      through routinely. A deferral states its price; it does not claim the
      thing cannot be done.
    - **The cheap route is still worth trying first and is the one that fits
      before E5b.** A small piece pivots in whole-cell steps, so the move is a
      *permutation* of its own cells — every colour relocated, none invented or
      blended. The trap is that a rotation mapping two source cells onto one
      target cell is exactly where pixels get destroyed. If that cannot be
      avoided for pieces small enough to matter, the whole-cell route closes and
      the item becomes the full body extraction above, at its stated price.

- [x] **E9 — Fuel, and a clock for steam. The two thermal quantities that
  temperature was standing in for.** *(both halves done; the fuel half in waves
  2b/2c, the steam half on 2026-08-12)* *Observed:*
  [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 1, defects A3, A4 and A5.
  *Unlocks:* fire that reads as burning rather than as flickering, and it closes
  the last loose end the correctness pass left behind.
    - **Confirmed in play 2026-08-13, checklist step 5** ([spot
      check](PLAYTEST_LOG.md#spot-check--2026-08-13--the-two-owed-steps-run-together)).
      The pocket rises, gathers, waits, and drips from the top. **That retires
      A5, B3 and D5 — one symptom, reported three times across four sessions,
      and the only one on the record that was scheduled by repetition rather
      than by a plan.** The step was written so its two failure directions point
      at different mechanisms — condensing immediately would mean the lifetime
      is still a temperature, never dripping would mean the ceiling-contact rule
      is blind — and neither came back.
    - **Both halves are now done. The steam half shipped 2026-08-12 and this is
      what it turned out to be.** The item's own prediction was "give steam its
      own condensation counter"; that is what was built, and the two things it
      got wrong are worth more than the part it got right.
        - **The counter is on `Element::ticks`, and steam is the second material
          to spend it as a lifetime rather than a third role on the byte.** Fire
          already used it that way and Fire is a Gas; steam is a Gas; so the
          rule generalises rather than being extended. The old guard was a
          `static_assert` naming Fire, which was right and did not generalise —
          it is now `tick_role()` in [element.h](src/physics/element.h) plus an
          assertion over every row that nothing carrying a lifetime is
          structural. **This is the compiler that E10's entry asks for, arriving
          early and from a different direction**, and it is the reason the open
          `ticks` decision is not blocked by this item: the roles are now
          counted in one place, so whatever E5a's representation turns out to
          be, adding it is a change to a function that asserts rather than a
          belief about a byte.
        - **The condensing *reaction row* is deleted, and that deletion is the
          actual fix.** `{ Steam → Water, 0..26 }` is gone. The prediction above
          framed the problem as steam's life being too short; the sharper
          statement is that **it was measured in the wrong units**. Life was the
          span between spawn temperature and condensing point, so it depended on
          what steam was *touching* — and because `Empty` conducts nothing while
          stone conducts well, a pocket had its **shortest** life pressed
          against the ceiling it is supposed to collect on. Measured: a sealed
          pocket used to drain in **3 steps**. That is why the same complaint
          came back three times (A5, B3, D5) reading each time like a request to
          change a number, when no number in reach would have fixed it.
        - **"Condense at contact points rather than uniformly" was right, and
          one step short.** The bullet below predicted contact-point
          condensation and it was built that way first — a plain countdown,
          running faster where a cell touched a ceiling. It measured as drips
          and read as a puff: **the whole four-deep pocket drained in about a
          second**, because every cell was ageing on its own schedule whether or
          not it was touching anything. The rule that works is stricter and
          simpler: **only a cell in contact ages at all.** The interior of a
          pocket is not on a clock, it is waiting its turn. A pocket then drains
          from the top down and takes as long as it is deep, which is what
          "collects, waits, then drips" actually means. Same fixture after:
          **291 steps, first drop at 181.**
        - **Nothing in it writes a drop, and that is the design rather than an
          economy.** A contact cell that runs out becomes `Water` in place;
          `Water` is denser than the steam beneath it, so `can_displace` —
          already there — carries it down through the pocket and onto the floor.
          Writing the drop downward by hand was the obvious version and would
          have been a fourth write path reimplementing a decision the density
          rule already makes. **Drip rate scaling with pocket size is likewise
          not implemented**: a wider pocket has more cells against the ceiling,
          each on its own clock, so it drips faster and slows as it drains. Both
          of the behaviours B3 asked for are emergent.
        - **Two things it cost, both recorded where they will be found.** Steam
          no longer condenses because it is *cold* — cold-quenching is
          re-expressible as a catalyst row whenever something asks, and is filed
          in [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md) so it is not refiled
          as a bug. And the branch added to `step_cell` reads as **+3–4% on
          `churning` and `cascading`, neither of which contains a steam cell**;
          it merges under the frame-budget rule and the reading is in
          [PERFORMANCE.md](PERFORMANCE.md) with the reason it is probably layout
          rather than the branch. **The more useful finding there: no benchmark
          scenario contains steam, so this feature's cost where it is actually
          used is unmeasured, which is a named requirement for `P4`.**
        - *Verified:* four new checks in `grid_test`, and **the two that carry
          the feature were confirmed against the unfixed engine first** — the
          pocket-drain span reads 3 steps before and 291 after, which is the
          defect and the fix in one number. Ten suites green. **The manual
          checklist is owed**: step 5 (reactions and heat) is the one that
          matters, and it is the step all three original reports came out of.
    - **The fuel half is done, and it was the whole of what was left in this
      item until 2026-08-12.** Fire was rebuilt, tuned and confirmed good in
      play across [waves 2b and 2c](ROADMAP_ARCHIVE.md#-waves--sub-plans-that-preempt-the-tracks)
      — session 4 answered "does this read as burning, at a pace that feels
      right" with yes, and fire goes back to being a regression check rather
      than an open question. Everything about what those waves changed, cost and
      got wrong is recorded with them; this entry keeps the argument for *why*
      the model had to change. **The steam clock below has never been built**,
      and it was briefly carried in the wave table as a queued wave of its own,
      which meant two documents each believed they owned it. It is tracked here
      and only here.
    - **The fuel half is built, and the first build of it put the timer on the
      wrong cell.** A3 and A4 were both closed by a `burn_duration` column, with
      the flame seeded from the material it consumed and held in place while it
      still owed burning time. That is correct about duration and correct about
      propagation, and it models fire as *fuel that has become flame*. Reference
      footage of a scene burning says fire is the other thing: **the fuel stays,
      in a burning state, and emits short-lived flame into the air around it** —
      see [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 1 follow-up. Flame lives
      ~5–15 steps and is completely replaced between frames; the fuel lasts
      seconds. Two quantities, two cells. One 180-step stationary flame cell is
      an orange block regardless of how correctly it propagates.
        - **What survives the rebuild:** duration is a per-material number, fire
          crosses a horizontal beam, and the regression tests that pin both.
          What changes is ownership of the timer.
        - **`Wood` gains a burning state as its own row (`Charred`) and an
          `emits` column, so this stays a table edit.** Burning things throwing
          flame has to be a column and not a Wood special case, or the next
          flammable material needs engine work instead of a row. The one genuine
          engine addition is emission itself — a burning cell writing into a
          randomly chosen adjacent *Empty* — and it pays for itself immediately:
          "fire is a layer on the surface and interiors never burn" is not a
          rule anyone writes, it is what emission-into-empty does when a cell is
          buried.
        - **The byte budget decides char's lifetime, and the forced answer is
          the better one.** Burning wood must stay structural — a burning
          ceiling that drops instantly is worse than one that burns — and
          structural cells already spend `ticks` on the free-fall clock.
          `Element` has no spare byte, so char *cannot* hold a countdown, and
          its lifetime becomes a per-step decay chance instead. That needs
          `chance_pct` widened to per-mille, since a 180-step mean is 0.56%. The
          gain is variance: a plank's cells stop winking out in lockstep.
          **Flame keeps a real countdown**, because Fire is a Gas and never
          structural, and that countdown is what the colour ramp reads.
        - **The `static_assert` written for the first build is what made this
          analysable rather than a bug.** It forbids Fire from being structural,
          on the grounds that `ticks` means two things. Extending fire to a
          material that *is* structural walked straight into it, at compile
          time, with the reason attached — which is the whole argument for the
          correctness pass's rule, arriving from a direction it did not predict:
          the invariant was between a struct field and a table row, not between
          two tables.
    - *(done, kept for the reasoning)* **Fire has no fuel,~~ and that is why
      three separate defects look like tuning problems and are not.** Wood
      ignites at 100% on reaching 120° and the resulting Fire dies on a flat
      6%/step roll — about 17 steps, roughly a quarter second. Fire from wood,
      fire from oil and fire from nothing are the same cell with the same
      lifetime, so "wood burns too fast" has no number to change. It needs a
      `burn_duration` column on `MATERIALS` and a per-cell fuel counter that
      Fire inherits from whatever it consumed.
    - *(done, kept for the reasoning)* **The horizontal-beam defect is the same
      missing quantity, which is why these are one item and not two.** Fire is a
      `Gas` at density −10, so it rises: a flame on top of a horizontal beam
      floats off after a single step and the wood below never reaches its
      threshold, while alongside a *vertical* beam the flame rises parallel to
      its fuel and stays in contact the whole way up. Fire that is consuming
      fuel should stay where the fuel is; fire with none rises and expires. One
      rule, and it fixes propagation and duration together.
    - *(done 2026-08-12, kept for the reasoning)* **Steam's lifetime is the span
      between its spawn and condensing points, so temperature is doing double
      duty as a clock — and that coupling is where the ignition bug came from.**
      *(This and the two bullets under it were the open half of E9, and they
      diagnosed it correctly. Session 1 raised it twice, as defect A5 and as
      observation B3, and they are the same fix seen from two sides.)* Steam
      used to spawn at 220° purely to make a puff last, which put it 100° over
      Wood's ignition point and made dousing a fire a way of starting one. The
      fix dropped spawn to 88° and paid for it in lifetime; session 1 confirms
      the bill came due. **Give steam its own condensation counter and both ends
      are satisfied at once** — a long-lived puff, and a spawn temperature that
      stays under the ignition floor the `static_assert` at the bottom of
      `reaction.h` already guards.
    - *(done 2026-08-12, kept for the reasoning — and see the correction above:
      "at contact points" had to become "only at contact points" before it read
      as collecting rather than as a puff)* **What that unlocks is the behaviour
      session 1 actually asked for:** condense at contact points rather than
      uniformly, so steam collects against a ceiling, waits, and emits water
      downward while the pocket shrinks — drip rate scaling on pocket size,
      which is both what was asked for and the physically right shape.
    - **The trap, and this area has been burned by it once already — it was
      avoided, and by the route this bullet named.** No thermal column was added
      in the end, so the specific hazard did not arise; what did arise is its
      shape one level up, a *byte* gaining a third claimant across two files,
      and it was answered the way this bullet says to answer it: with an
      assertion next to the one already in `reaction.h`. Original text follows.
      `spawn_temperature` and the ignition thresholds live in two different
      tables, and nothing about editing either one suggests reading the other.
      Any new thermal column inherits that hazard. The correctness pass's rule
      applies directly — when a table gains a column, ask what invariant now
      spans two tables and assert it, next to the one already in `reaction.h`.

- [ ] **E5b — The air field.** *(the second half of what was E5, and it absorbs
  the item that used to sit below this list as "gas pressure — named and
  deliberately not scheduled")* *Observed:* the four searches named in this
  track's preamble, plus the gap that used to be its own note — steam in a
  sealed room builds no pressure and smoke does not fill a space it should,
  because gases rise and spread by the same per-cell rule as everything else.
  *Unlocks:* six things that are currently six separate gaps, which is the
  argument for it. **Scheduled after the playtest gate**, and the two costs
  below are why.
    - **A second coarse grid over the world — pressure and a velocity, one entry
      per 4×4 block. The pattern is already built, shipped and measured:**
      `LightField` is exactly this shape — a low-resolution, integer,
      deterministic field stretched over the scene and composited with a single
      `RenderCopy`. Reusing its block size, its padding rules and its "does this
      frame have any at all" early-out is most of the design already done.
    - **What one system delivers:** gas pressure; explosions that *push* rather
      than delete-and-throw; steam and smoke that advect and curl instead of
      rising in columns, which is the single thing that most makes a sandbox
      read as alive; wind, which V9's sparks need in order not to look like a
      screensaver; fire suffocating in a sealed space, which E9 cannot currently
      express at all; and the pressure term that **retires `find_lower_surface`,
      `vent_fluid` and `make_room_above` together**.
    - **It also buys back something the current design has ruled out
      permanently, and that is the strongest argument for it.** E1 levels water
      by teleporting a surface cell across the connected body — instant, and
      non-local by construction. Waves, sloshing, surges and water hammer are
      therefore not missing features, they are **impossible**, because the
      mechanism has no propagation delay for them to live in. Pressure diffuses
      over several steps, so they fall out for free rather than being added.
    - **Two honest costs, and both are why it is after the slice rather than
      before.** The reference implementation (The Powder Toy's) is
      floating-point, and `Grid` forbids floating point for the reproducibility
      F1 spent seven steps establishing and F1.7 wrote down — so this needs a
      fixed-point port, which is doable, which is non-negotiable, and which is
      real work that the reference does not do for you. And it is a **fixed**
      cost proportional to the awake area rather than one that scales with how
      much is moving. Every other cost in this engine has the second shape;
      chunking, dirty rects and the thermal early-out are all built on it. That
      difference deserves its own [PERFORMANCE.md](PERFORMANCE.md) entry with a
      measurement at the played size, not a bracketed number attached to an
      item.
    - **The four searches do not get deleted on the strength of the field
      existing.** Each one is currently the only thing preventing a specific,
      reproduced defect — A6, A6b and the U-tube case all have probes and tests.
      The order is: field lands, each search is removed one at a time, and the
      probe that motivated it is the thing that has to still pass. A search
      removed on the argument that the field "should" cover it is how A6b comes
      back.

### V — Visual identity

*Running order: **~~V7-emissive → V5 → V6 → V10 → V3 → V4-props~~ → V11 + V12 +
V13 → V19 → V16 → V9 → V14 → V15 → V8 → V7-rest**, with V10.1 alongside E6 and
V15 pulled by `S1`. **V19 is new on 2026-08-16 and is placed by request rather
than by inference** — the ask was for a `CnC_parallax`-shaped scene before the
split-view path — and **V16 may end up inside it**, since five new pan-sized
layers is the cost that finally makes a wrapping layer worth having. **V12–V16
are new as of 2026-08-11 and are the visual system this track was not planning
for** — see [The visual system this track is now building
toward](#the-visual-system-this-track-is-now-building-toward) immediately below,
which is the one place the four goals behind them are stated together. **V11 is
new as of 2026-08-09 and goes to the head of the open half**, and the reason is
a requirement this track was not serving: the art direction is expected to
change several times, and five specific things in the codebase make each change
expensive — the frame composition is 350 inline lines in `main.cpp` with a
hard-coded layer order, material colours are compile-time constants,
`Camera::SCALE` is a compile-time constant, the light layer can only add and
never subtract, and the parallax factors are duplicated across two files. None
of those is hard to fix today and every one of them gets harder. **V9 moves
ahead of V8** because E6 is what it exists to dress and E6 now lands before it.
The struck-through half is done and is left visible rather than deleted, because
the order is the argument and a reader needs to see which arrows were actually
followed. **V8 has moved from second in this line to second-last, and it moved
by everything else earning a place rather than by being demoted**; see the note
in "Where this stands" for why its remainder still cannot answer this tier's
admission test. **The five items now ahead of it are the standard it failed**:
each names something in the built game that is wrong, and V16 in particular
takes the half of "the backdrop needs work" that *can* — the backdrop does not
move — leaving V8 holding only the parts that still cannot. The order below is
not the numbering — see the note on stable IDs above. **A way to drive the
window is a prerequisite for everything from V3 onward and is promoted into this
track from [Sandbox / debug tooling](#sandbox--debug-tooling)**, where the
argument for moving it is written out; V10 is placed ahead of that line because
a HUD is the one visual item that can still be judged on a screenshot. V5 and V6
are what everything after them is authored **against**, so they run first even
though they were written last. **V7 is split by its gate rather than placed
once**: its emissive half runs immediately after E9's fire rebuild, because fire
is the one subject here whose look is what it emits rather than what colour it
is, and the argument for that is written into V7 itself. Everything in V7 that
needs authored albedo to respond to keeps the original gate and stays where it
was.*

***Reference footage is a primary input to this track, which is the opposite of
its standing in E.*** A look cannot be derived from your own bugs — nothing
about the fact that the player is a white rectangle tells you what should be
there instead, and that asymmetry is the whole reason the two tracks answer to
reference differently. V5 is the item that consumes it; V7, V8 and V9 each cite
it for a specific question they would otherwise guess at. Observations go in
[notes/reference_observations.txt](notes/reference_observations.txt), the frames
themselves are gitignored, and the caveat is craft rather than process: what
comes out is our direction informed by theirs, not their palette lifted. **V6 is
where that stops being a good intention and becomes checkable**, since a locked
palette is a constraint a validator can enforce and a resemblance is not.*

**This track is in progress as of 2026-08-16, and it displaced E10 to get
there.** The running order above is unchanged; what changed is that the head of
it is now being worked, as one block rather than as five items. The trigger was
a request for a split-view and parallax backdrop system, and the finding that
made it a block is that **V11, V17, V7-rest and V8's remainder are all the same
item wearing four hats** — every one of them is blocked on the 350 inline lines
of frame composition, and taking them separately means extracting that code
three times. The step order, sizes and current state live in
[ROADMAP_ITEMS.md](ROADMAP_ITEMS.md#-next-up); the reasons are here and at each
item.

Two things came out of the first step — rewriting the two `notes/` files, which
is documentation and was expected to return nothing:

- **The four depth bands do not separate by value, and the reference says they
  must.** Measured against the CnC frames: sky spans roughly `0x14`–`0x28`,
  mountains `0x20`–`0x30`, trees `0x18`–`0x59`, terrain `0x1B`–`0x78`. They
  overlap almost completely, so **nothing distinguishes one band from the next
  except the rim highlight** — which is why a busy frame reads flat. **This is a
  renderer defect and not a palette one**, and the distinction matters because
  the obvious repair is wrong: pushing a band's range down is a *multiply*, the
  light pass can only *add*, so no edit to `PALETTE` can express it. It is the
  concrete thing V7-rest's darkening now exists to fix, and the first time that
  item has had a measured symptom rather than an intention. **Fixed 2026-08-16
  at block step 3, and the re-measurement was worse than the channel ranges
  above suggested** — in luminance the sky averages 26 and the mountains are
  *flat 28*, p05 and p95 both 28, so the two most distant bands separate by two
  levels out of 255 and the far one is the brighter of the pair. A range is not
  a separation, and quoting one for the other is how this understated itself.
  The mountains now carry a 0.60 per-layer multiply. **One clause above is wrong
  and is corrected rather than deleted:** "pushing a band's range down" is
  right, but it cannot be done *globally* — a frame-wide multiply scales every
  band alike and leaves every ratio between them where it was, so separation is
  necessarily per-layer. The world-wide grade got built alongside it and is a
  different feature (night, fog, biome); reading this bullet as the argument for
  it would be reading it as the argument for the one knob that could not have
  answered it.
- **The parallax factors were never measured, and nearly got a false
  provenance.** The three "parallax" reference frames were taken to be one scene
  panning, which would have priced every band. They are not: region shifts come
  back non-monotonic in depth and *opposite in sign* at the two edges, with `dy`
  zero everywhere, which no camera translation produces. They are three
  separately generated lakes sharing a sprite vocabulary. So `PARALLAX_SKY_X`
  and friends remain what they have always been — chosen by eye — and **V11 must
  ship the existing factors unchanged**, with the golden frame proving the
  extraction was a no-op, and retune as a separate commit that can be pointed
  at. Recording an eyeballed number as a measured one is the failure this
  project's first rule is about, and it was one step away. **V11 shipped on
  2026-08-16 and did exactly this** — `0.04/0.02` and `0.15/0.06` are
  byte-for-byte what they were, and the golden checksum held unchanged across
  the whole restructure before the one deliberate change moved it. **The
  mid-ground band's new `0.40/0.16` is the case this rule had not covered**,
  since it is not a retune of an existing factor but a factor where there was
  none: it ships with a *stated derivation* (parallax is inverse depth, so
  sqrt(0.15 × 1.00) ≈ 0.40) and is labelled a derivation rather than a
  measurement everywhere it appears.

Both are written up in
[notes/reference_observations.txt](notes/reference_observations.txt); the
correction to V5's resolution is filed beside V5's own entry.

#### The visual system this track is now building toward

***Added 2026-08-11. Four things were named as the intended end state of the
visual system — varying pixel resolutions and sprite sizes in one scene,
procedural animation, backgrounds that are animated rather than static, and
entity bodies that granulate locally when damaged. This section is where that
commitment is recorded, because three of the five new items below cannot be read
without it and one of them is admitted by it rather than by an observation.***

**A commitment is not a licence to skip the admission test, and four of the five
still pass it on their own.** V12, V13 and V16 each name something in the built
game that is wrong or reads badly — a colour key that forbids a soft edge
anywhere, a draw path that makes a denser drawing a bigger object, a backdrop
whose only motion is parallax and therefore stops when the player does. V14
names an articulation limit the track has already written down twice. **V15 is
the one item admitted on the commitment itself, and it is admitted honestly**:
[notes/procedural_animation.md](notes/procedural_animation.md) wrote down, in
advance, the three things that would admit a skeletal rig, and the third is *"a
second character type is committed to."* That is exactly what happened. **The
note predicted the trigger and the trigger fired**, which is the strongest form
this document's admission test has ever taken — better than an observation,
because it was specified before the thing that satisfied it existed. The same
move admits `E12`'s material half through
[notes/granulating_enemies.md](notes/granulating_enemies.md), whose own closing
section names the condition under which it stops being notes.

**What the commitment does *not* do is move the running order.** None of this
displaces wave 4, E9's steam half, the fluid spike or `S0`; the visual block
sits where V11 already sat. The reason to write it all down now rather than when
each item comes up is the same reason V11 exists at all — every one of these
gets more expensive the more code reads the assumption it removes, and three of
them share draw sites, so taken apart the same two rectangles in `main.cpp` are
edited three times.

**Two things the commitment asks for are deliberately not items, and both are
recorded as decisions rather than left to be discovered mid-implementation** —
see [Decisions owed](ROADMAP_ITEMS.md#-decisions-owed). *Higher-resolution pixel
art* is not bought by making the world's cells smaller: at `Camera::SCALE` 2 the
viewport holds four times the cells, every asset is re-authored, and every
physics constant is retuned, because all of them are stated in cells against a
scale of 4 (`Player::WIDTH`/`HEIGHT`, `DigTool::RANGE`, `LightField`'s reach in
blocks). V13 buys denser *art* with none of that, and it is also the cheap
experiment that says whether denser art is wanted at all. *And the shader path
stays refused*, which V7 already required be decided deliberately if it is ever
decided: every item below has a route through `SDL_Renderer` as it stands —
`SDL_ComposeCustomBlendMode` for a multiply term, `SDL_RenderGeometry` for
textured triangles, `SDL_TEXTUREACCESS_TARGET` for a masked body, all present in
the pinned SDL 2.30.0 and none of them used anywhere yet. **The fork should be
bought by the first thing that has no such route**, and sub-cell terrain detail
is the one candidate on the horizon, because the terrain's resolution is the
simulation's resolution and no amount of asset work changes that.

- [x] **V1 — Transparent `Empty` and a backdrop layer.** *(done — see
  [Shipped](ROADMAP_ARCHIVE.md#v1v2-visual-foundation))*
- [x] **V2 — Palette and jitter pass on `MATERIALS`.** *(done — see
  [Shipped](ROADMAP_ARCHIVE.md#v1v2-visual-foundation))*

- [x] **V5 — Write the art direction down.** *(done — see
  [notes/art_direction.txt](notes/art_direction.txt))* *Observed:* there is no
  art direction in this repository. The only thing resembling one is the opening
  paragraph of the art-pipeline note — "detailed snowy Japanese railway-crossing
  scenes, roughly 640x400, cold desaturated blue-grey palette with warm sodium
  and signal accents, heavy ordered dithering in the sky and snow" — and it is a
  *reference target for one test scene*, written to justify a test fixture. It
  has since become the de facto direction for the whole game by nobody deciding
  anything.
    - **The finding that makes this urgent: the reference and the fiction are
      two different games' art, and nothing in this plan reconciles them.**
      `notes/story.txt` is a dystopian United States where a chatbot runs the
      government, humans are barred from the economy and live on rations, and
      "quantum magic" is how they reach other worlds. A snowy Japanese level
      crossing is not that. V2's palette has *already* been authored against the
      crossing reference — the world went cold-backdrop and warm-desaturated
      because of it — and V3, V4, V7 and V8 will each be authored against
      *something*. The only question is whether it is the same something, and
      right now the answer is being decided per-asset by whoever is looking at
      which file.
    - **The reconciliation is available and cheap, which is why this is a
      writing task and not a crisis.** By the fiction, quantum worlds are
      *other* worlds — so a snowy crossing is a perfectly legitimate trial
      location, and the dystopian US is the home base the player returns to.
      That is **two directions**, and the plan currently budgets for zero.
      Retiring the crossing reference is equally acceptable. What is not
      acceptable is drifting, because every asset authored under an unstated
      direction is an asset that has to be re-authored when it is stated.
    - **Deliverable:** `notes/art_direction.txt`, and it is the document every
      later V item is checked against. It has to answer, in writing and in this
      order: **which locations exist and what each one reads as**; **the
      palette's intent** (V2's set is already a decision — cool backdrop, warm
      desaturated world, saturation reserved for Fire and Water — and this is
      where it stops being an implementation detail of one table and becomes a
      rule); **the dithering rule**, since V2 cut `color_jitter` specifically to
      make room for hand-placed ordered dithering and nothing has yet placed
      any; **what the player reads as**, which V3 cannot start without; and
      **what is drawn versus what is simulated**, which the original three-layer
      model already sketched and which V4, V8 and V9 all depend on.
    - **Why it runs before V3 rather than after.** V3 is the player sprite: the
      single most direction-dependent asset in the game. You cannot draw the
      protagonist without knowing whether they are a figure in a snow suit at a
      level crossing or a ration-line human in an AI-run America, and a sprite
      authored against the wrong answer is not a tuning problem, it is a redraw.
    - **Reference footage is an input to this item specifically, and it is worth
      being concrete about what it is for.** Two reference points now exist
      rather than one — the level-crossing stills the art-pipeline note opened
      with, and gameplay capture of comparable games — and the second is useful
      *because* it is not the first: two directions that disagree force the
      choice to be made rather than absorbed. What to actually take from it is
      bounded, and it is not "a look": contrast and readability against a busy
      simulated background, how much of a frame is not simulated at all, and
      what a player's eye is drawn to when everything is moving. Those are the
      questions this document cannot answer from its own screenshots, because
      its own screenshots are of a debug palette. Extraction and the entry
      format are in
      [notes/reference_observations.txt](notes/reference_observations.txt);
      frames are gitignored and the written observation is the artifact.
    - *Verify.* No code, and the check is the same one F3.5 used: the answer is
      written down before anything that needs it can re-derive it,
      inconsistently, on its own. The concrete test is that V3, V4, V7, V8 and
      V9 can each name the line in it they are authored against.
    - **Resolved as: the crossing stills are retired, the Noita-forest
      screenshot in
      [notes/reference_observations.txt](notes/reference_observations.txt) is
      adopted as the first quantum world's direction, and the dystopian US stays
      the frame story per the reconciliation this item itself proposed.**
      Locations are per-world from here on; a second biome gets its own section
      when it exists rather than inheriting this one's palette. V3 (the player)
      is explicitly not answered by this pass — see the note at the bottom of
      art_direction.txt — and stays open.
    - **Correction, 2026-08-16 — half of that resolution had gone false and
      nothing noticed for three months.** Both deliverables,
      `notes/art_direction.txt` and `notes/reference_observations.txt`, were
      **deleted in commit `e05609d`** while roughly fifteen places — ASSETS.md,
      ENGINEERING_NOTES.md, this file in six places, `generate_test_scene.py`,
      `tests/rim_probe.cpp`, `src/main.cpp` twice, and five of the `tools/`
      scripts — went on citing them by name for a specific rule each. That is
      the exact failure this project's first rule names: a stated rule that
      stopped matching the tree and kept being believed. **Both files are
      rewritten from scratch as of 2026-08-16**, against the Cast n Chill frames
      in `resources/images/`, and each section a caller names is marked with who
      names it so the next deletion is visible. What changed in substance: the
      **Noita-forest screenshot no longer exists in `resources/` and no
      observation of it survived**, so that adoption is unverifiable and the CnC
      frames are adopted in its place — *the crossing stills stay retired, which
      was the half that was right*. V3 has since shipped, so the player section
      now records what was built rather than leaving the question open. Two new
      findings came out of the rewrite and are not V5's: the **band-value
      defect** (our four depth bands overlap almost completely, so nothing but
      the rim separates them — a renderer problem, not a palette one, because
      the light pass can only add) and the **failed parallax measurement** (the
      three "parallax" reference frames are three generated lakes, not one pan,
      so our `0.04`/`0.15` remain unmeasured and a "measured" factor came within
      one step of being fabricated). Both are written up at the notes.

- [x] **V6 — One locked palette, shared by the table and the art.** *(done — see
  [tools/pixel_art.py](tools/pixel_art.py),
  [tools/validate_palette.py](tools/validate_palette.py))* *Observed:* V2 found
  that authored terrain never reads `MATERIALS` at all — `load_scene` calls
  `grid.paint(x, y, type, color)` with the colour out of the albedo BMP and
  `paint` writes it verbatim. So the engine's eight colours and the art's
  colours are two independent sets that happen to have been chosen by the same
  person on the same day, and nothing anywhere checks that they agree.
  *Unlocks:* every authored asset after it, and it is what stops V2's palette
  from being a one-off.
    - **The seam V2 named is not a bug, it is the absence of a shared palette.**
      Authored wood burning into table-coloured fire is the visible symptom; the
      cause is that there is no set both sides draw from. Fixing it at the seam
      — carrying an authored colour across a reaction — is the wrong repair and
      would put art data into `src/physics/`, which nothing there currently
      reads and nothing there should.
    - **Built as** a named palette both sides index: a header the `MATERIALS`
      rows are written in terms of, and the same values exported in whatever
      form the external editor wants. Plus a **validator**, which is the half
      that actually holds the line: a check that every colour in an albedo BMP
      is in the palette, run the same way `generate_test_scene.py`'s legend
      match is already checked. An off-palette pixel that loads silently is the
      same failure mode as F4.3's size mismatch — the whole scene dropped
      without a warning — and it gets the same fix, which is to make it loud.
    - **What this deliberately does not do:** it does not derive per-cell colour
      from the material table. That is the optimisation `ENGINEERING_NOTES.md`
      prices at ~6x the hot loop's memory traffic and rejects outright, because
      it is mutually exclusive with authored per-cell colour, which is the
      visual pillar. A shared palette constrains what colours are *chosen*; it
      changes nothing about where they are *stored*.
    - **Built as** `tools/pixel_art.py` — the locked `PALETTE` dict, an
      ordered-dithering helper (`dither_mix`, over the `bayer_threshold` matrix)
      and the rim-light pre-process every generator in `tools/` now shares —
      plus `tools/validate_palette.py`, which fails loudly on an off-palette
      pixel the same way an unmatched legend colour already does (`main.cpp`'s
      `load_scene_from_bmp`), and `assets/palette.gpl` for loading the set into
      an external editor directly, regenerated by `tools/export_palette_gpl.py`.
      `MATERIALS` in `src/physics/material.h` and `src/scene/legend.h` are
      untouched, as specified. Not yet done: nothing currently *runs* the
      validator automatically (no pre-commit hook, no build step) — it is a
      script to run by hand until an off-palette asset actually ships once.

- [x] **V3 — Player sprite decoupled from its hitbox.** *(done — superseded by
  V3.1 below; its generator has been replaced by
  [tools/player_sheet.py](tools/player_sheet.py))* *Observed:* the body is drawn
  as a plain white rectangle, and a person cannot be drawn as one. *Unlocks:*
  the player reading as a character at all, and V9's animation follow-on.
  Collision stays 8x20 — it is tuned to the physics and 18 tests stand on it —
  and a larger sprite is drawn anchored to the box's bottom-centre with an
  offset. Touches no physics and breaks no tests.
    - **This item said "4x8" in three places and the code had said 8x20 since
      the Noita rescale**, which is exactly the failure mode the "point at code
      by name" convention at the top of this file exists to prevent — the
      convention covers line numbers and did not cover *quoted constants*, which
      go stale the same way and are harder to spot because they read as
      deliberate. `notes/art_direction.txt` carried the same stale pair. Both
      corrected against `player.h`.
    - **Built as** a 14x26 colour-keyed BMP from a hand-authored ASCII grid
      (`tools/generate_player.py`, since replaced), in the locked palette plus
      one new `char_*` group in `tools/pixel_art.py`. ASCII rather than a binary
      for the reason `hotbar.cpp`'s icons are hand-set bits: at this size the
      source form should be the one a diff can show, and a one-pixel change
      should review as a one-character change.
    - **The offsets are duplicated between the generator and `main.cpp` and
      nothing enforces agreement** — the same trap V8's parallax factors have,
      written down in both places for the same reason. Each side asserts the
      half it can see: the generator checks canvas size, that the bottom row is
      painted (a sprite whose lowest row is empty hovers above every floor
      forever, and reads as a physics bug), and that no box row is empty;
      `main.cpp` has a `static_assert` that the sprite is at least the box's
      size.
    - **Facing is tracked in `main.cpp`, not on `Player`,** and stickily — it
      holds the last direction actually pressed. Presentation state on the
      simulation body is state F1's determinism tests would then have to account
      for, for a flag that only ever picks an `SDL_FLIP`.
    - **`char_accent` (`0x945128`) is now the brightest value in the locked
      palette**, above `rim_grass`, which the palette had called "the one bright
      accent in the whole terrain layer". It is six pixels on the mask and is
      defensible at that area, but it sits in the warm-orange family reserved
      for Fire — the tension is recorded next to the entry in `pixel_art.py`
      rather than left to be rediscovered. **Check it against a burning scene
      before any more copper is added**, and specifically once V7's emissive
      pass covers more than temperature.
    - **Still open, and deliberately:** one static pose only, per the animation
      note below. The fallback rectangle is kept for a missing asset rather than
      deleted — a game you can still move around in is a better diagnostic than
      an invisible player.
    - **Authored against V5's answer, which is why it moved behind it.**
      Everything technical about this item is standard practice and was never
      the hard part; the hard part is what the figure *is*.
    - **What this does not do, named rather than left to be discovered:
      animation.** A sprite that slides while walking and holds one pose while
      falling reads worse in motion than a rectangle does, because a rectangle
      makes no claim. Walk, fall, land and dig are the four states the existing
      code can already distinguish without any new state being tracked —
      `Player` knows its velocity and whether it is on the ground, and `DigTool`
      knows its cooldown. That is the follow-on and it is deliberately not this
      item. *(Done — **V3.1** below.)*

- [x] **V3.1 — Animation, and one decomposed limb.** *(done — see
  [tools/player_sheet.py](tools/player_sheet.py),
  [src/render/player_anim.h](src/render/player_anim.h))* *Observed:* V3's single
  pose responded to nothing but facing, so the figure slid while walking — the
  exact defect V3's own text predicted would read worse than the rectangle.
  *Unlocks:* the player reading as *doing* something rather than as being
  somewhere.
    - **The finding that shaped this, from reading how Noita actually does it:**
      its expressiveness is not frame count. Its player sheet carries ~50
      animations, but the reason the wizard reads as alive at fourteen pixels
      wide is **decomposition** — the wand arm is a separate sprite that rotates
      toward the cursor over a body playing a short loop, and the cape is
      separate simulated cloth. Neither is baked into the sheet, so a handful of
      body loops covers every direction of aim. **The lesson was "decompose",
      not "draw fifty animations"**, and it is what decided this item's scope.
    - **Built as** Noita's model with the parts this game cannot yet use left
      out: a sheet whose rows are animations, and a table of `(row, col, frames,
      wait, loop)`. `col` exists so two animations can share a row — `rise` and
      `fall` are one row holding two poses, which is Noita's own "same row,
      several names".
    - **The decomposed limb was built and then pulled, and the sheet is what
      remains.** A rotating aiming arm drawn over the body loop, attached at a
      marker pixel in a per-frame hotspot image, is the thing the finding above
      actually argues for — and it went back out rather than shipping
      half-right. **What it cost while it was in was not the draw call; it was
      the second image.** Every frame drawn from then on had to carry a marker
      pixel or `--validate` failed, which is a standing tax on authoring in
      exchange for a limb the game was not yet using well.
      `tools/player_sheet.py` no longer emits a `SHOULDER` table, generates no
      hotspot image, and asks nothing of the artist beyond the sheet;
      `assets/player_hotspots.bmp` and `assets/player_arm.bmp` are deleted, and
      neither held hand-authored work — both were `--starter` output with one
      uniform shoulder position. **Bringing it back is: the hotspot image, the
      marker-pixel validation, a `SHOULDER` table beside `ANIMATIONS`, and a
      rotate-about-the-shoulder `SDL_RenderCopyExF`.** Nothing about the sheet
      format has to change to allow it.
        - **Its one non-obvious trap, recorded because it cost a bug the first
          time:** `SDL_RenderCopyEx` mirrors the *texture* and then rotates the
          quad, so flipping the sprite does not mirror the aim angle — it turns
          the arm's rest direction from `+x` into `-x`, which the rotation has
          to undo with a 180° offset. The reflection (`180 - angle`) is right
          for a cursor level with the shoulder and inverts the vertical
          everywhere else, so aiming up-left pointed the arm down-left, and both
          of the cases anyone checks by eye are the two it gets right.
    - **Five animations from state the code already had**, which was V3's stated
      bound and held: `idle`, `walk`, `rise`/`fall` (chosen by the sign of
      `velocity_y()`, so a long drop holds the falling pose instead of cycling),
      and `dig` as a one-shot. `Player` gained one read-only `velocity_x()` and
      nothing else.
    - **The clock is the fixed step, and this is the trap V10 names for screen
      shake arriving somewhere nobody expected it.** A walk cycle driven off the
      rendered frame runs at nearly 3x speed at 165 Hz — and presents as an
      *art* problem, which is what makes it dangerous. **The fix is not to move
      animation into the simulation:** F3.5 forbids rendering feeding
      simulation, and animation state on `Player` is state the determinism tests
      would then have to carry. It is advanced inside `main.cpp`'s existing
      fixed-step loop and lives entirely in `src/render/`.
    - **The selector is SDL-free and tested** (`anim_test`, the eighth suite),
      for the same reason V7's `LightField` is: what it produces is a row and a
      column. Its failure modes are all silent and all *look like art problems*
      — a cycle that restarts every step reads as "the walk animation is bad", a
      one-shot that never clears reads as "the dig sprite is stuck" — so none of
      them would send anyone to the file that caused them.
        - **One of those tests caught a bug that compiles, links and runs.** A
          `constexpr` variable at namespace scope in a header has internal
          linkage, so every translation unit gets its own object at its own
          address — and the selector identifies the current animation by
          pointer. `main.cpp`'s `IDLE` and `player_anim.cpp`'s `IDLE` were
          different addresses, so every comparison was false and the animation
          restarted every step. `inline constexpr` fixes it and the generator
          now emits it with the reason attached. **This is the argument for
          testing the selector rather than checking it by eye**: the symptom is
          a figure standing still twitching, which is indistinguishable from bad
          art.
    - **Authoring moved from ASCII to a hand-drawn sheet, reversing V3's own
      reasoning on purpose.** One pose as an ASCII grid meant a one-pixel change
      reviewed as a one-character change; twenty-four such blocks is where that
      argument inverts. `--starter` bootstraps the sheet with V3's pose stamped
      into every frame slot, so the game runs end to end from the first minute
      and each redrawn frame improves on a working baseline rather than being a
      prerequisite for one.
    - **`tools/generate_player.py` is gone and its layout duplication with it.**
      Everything about the sheet is emitted into a generated
      `src/render/player_sprite.h`, so the trap V3 shipped with — four numbers
      in two files, nothing enforcing agreement — is closed rather than
      documented, which is what a sheet's several dozen numbers made necessary.
    - **Deliberately not done, with reasons:** the **verlet cape** (off-grid
      float cloth; it needs an off-grid substrate that E5a deliberately does not
      provide, so it waits on E8's body extraction rather than on the E track),
      **sprite stains** (Noita maps the sprite's UVs so world materials splash
      and stain it — the most on-brand idea it has for an engine whose whole
      subject is materials; filed in `ENGINEERING_NOTES.md`), and **frame-tied
      gameplay events**, which Noita has and which this project cannot adopt:
      firing gameplay off an animation frame is rendering driving simulation,
      which F3.5 forbids outright.

- [ ] **V4 — Props overlay and the `Snow` material.** The non-simulated props
  layer (signage, fencing, anything that exercises no system and therefore
  belongs on top rather than in the grid), plus `Snow` as one `MATERIALS` row: a
  powder lighter than Sand, and the single most on-theme material for the
  reference art. **Sequenced after E2 on purpose** — with heat in the engine,
  snow melting into water is a thermal row rather than a special case someone
  has to hand-write, which is the whole argument for E2 arriving first. **The
  `Snow` row is also E7's first row**, and if E7 has run by the time this item
  comes up it belongs there rather than here; the props layer is what is
  genuinely V4's.
    - **The props half got a first slice alongside V5/V6: trees.**
      `tools/generate_props.py` generates colour-keyed tree sprites;
      `main.cpp`'s `Prop` struct anchors one to a world position and draws it
      before the cell texture, which is what lets authored terrain occlude a
      trunk's base for free (see notes/art_direction.txt's layer model for why
      that ordering, not the more obvious "props on top," is correct here).
    - **The prop format is now built and the hardcoded list is gone** —
      `src/scene/props.{h,cpp}`, `assets/test_props.txt`, and `props_test` as
      the ninth suite. The nine trees are unchanged on screen; what changed is
      that they are data. A second scene can have its own props now, which is
      what Quantum Worlds was going to need and did not have.
        - **Four decisions came out of building it and all four outlived the
          item, so they are recorded as standing rules in
          [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md) rather than here:**
          per-cell data gets an image and a list gets a list (the reflex is to
          copy F4's material map, and the frozen legend is what makes that not
          merely expensive but forbidden); a format must not carry a number its
          loader ignores, which is why there is no `y`; a malformed list costs
          every record rather than the bad line, and the test suite that follows
          from that asserts *empty* rather than *short*; and data that names a
          path gets its name validated. **What belongs here is the sequencing
          claim, not the rules:** this was the item taken instead of V8's
          remainder, and it is the worked example of the tier's admission test,
          because it could name what was wrong (no format, so no second scene
          could have props) and what it unlocked (Quantum Worlds).
        - **One consequence worth naming: an unplantable prop is now dropped
          rather than parked at a fallback.** The authored-y version had a y to
          fall back to and falling back to it is how the burial hid; with no
          authored y the only fallback is 0, the top of the world, so it would
          hang in the sky. Removing it and warning is the honest version of the
          same message.
            - **The startup count contradicted that warning for a whole
              revision, and moving it is the fix.** `Props: N of M placed`
              printed immediately after the textures loaded — 59 lines above the
              terrain scan, and therefore before either way a prop can be
              dropped — so it counted texture loads while saying "placed".
              Confirmed against the running binary rather than argued: a fixture
              with one unplantable prop printed `Props: 10 of 10 placed` on
              stdout while stderr warned that one of them was not drawn. **The
              warning was right and the number contradicted it, which is worse
              than no number** — [README](README.md)'s launch check makes this
              line the check *rather than* the eyeballing. The `printf` now runs
              after planting; the same fixture reads `9 of 10` and the clean one
              still reads `9 of 9`. The general form is in
              [ENGINEERING_NOTES.md](ENGINEERING_NOTES.md): a count taken before
              the last thing that can fail is measuring a different quantity
              than its label claims.
    - **Still open in V4:** `Snow` is untouched, and per this item's own text it
      belongs to **E7** rather than here if E7 has run by then. Props themselves
      are still one flat layer at parallax ~1 with no depth sorting among them,
      which is correct for the forest and is the thing to revisit if a scene
      ever wants props at two depths.

- [ ] **V8 — The backdrop: authored, and parallax.** *Observed:* V1 shipped the
  *layer* and a 64-band gradient placeholder, and said in as many words that
  authored backdrop art was not its job. F3.4 then landed a camera that
  genuinely pans, which is what turns the placeholder from plain into wrong.
  *Unlocks:* the depth the reference art gets most of its beauty from, at a cost
  the layer model already priced.
    - **A panning camera is what makes a single static backdrop impossible to
      get right.** It either slides 1:1 with the world, in which case it reads
      as painted onto the terrain, or it does not move at all, in which case it
      reads as a wall behind a moving world. Parallax layers are what a backdrop
      is *for* once the camera moves — and the original three-layer model was
      written before F3.4 existed, so it assumed a static image and did not say
      this.
    - **Cost is one `SDL_RenderCopy` per layer**, which is what that note
      already budgeted, and the offsets come from `Camera::view_x()`/`view_y()`
      scaled per layer — no new coordinate system, and specifically not a second
      one, which is the thing F3.2 spent a step consolidating.
    - **A first slice shipped alongside V5/V6, and it is a slice rather than the
      whole item.** Two static layers — sky+stars and a mountain silhouette,
      `tools/generate_backdrop.py` — replace V1's gradient in `main.cpp`, each
      offset by `camera.view_x()`/`view_y()` at its own parallax factor exactly
      as specced above. **Left open:** more than one biome's worth of backdrop
      (this is the forest world only), any time-of-day variation, and a third
      depth band beyond sky/mountains if a future reference asks for one. The
      factor pair for each layer lives in two places that must be kept in sync
      by hand — `main.cpp`'s `PARALLAX_SKY_X/Y`/`PARALLAX_MOUNTAIN_X/Y` and the
      matching constants at the top of `tools/generate_backdrop.py` — because
      the image has to be sized to the exact pan range its own factor implies;
      nothing enforces the two files agreeing, which is written down in both
      places as the trap it is.

- [x] **V7 — Per-cell emissive lighting.** *Observed:* fire is the brightest
  thing in the world and casts nothing. Terrain does not shadow itself, a flame
  in a pit lights nothing, and the only reason the scene reads at all is that
  every cell is drawn at full brightness regardless of what is around it.
  *Unlocks:* the part of this tier that would genuinely read as cutting-edge,
  and it is the one item here that a screenshot sells on its own.
    - **E2 is what makes this affordable rather than speculative, and that is
      the whole reason it is sequenced here.** "Which cells are emitting" is a
      question the engine already answers — a cell above ambient is a light
      source, for free, and nothing new has to be tracked to know it.
    - **The architectural decision this item forces, named now because the
      alternative is much larger.** The renderer is `SDL_Renderer` with no
      shader path, chosen by the same zero-dependency logic that picked BMP over
      PNG and immediate-mode over Dear ImGui. Per-pixel lighting at the played
      size is 256,000 cells on the CPU every frame, which is not affordable. The
      shape that is: a **downsampled light grid** — one value per small block of
      cells, propagated over a handful of iterations, uploaded as a small
      texture and stretched with linear filtering over the cell texture. That is
      a second `SDL_RenderCopy`, no new dependency, and a resolution knob to
      trade quality against cost. **The alternative is moving to a shader
      path**, which is a genuinely large decision about what this project's
      renderer is, and it should be made deliberately with its own entry in
      `ENGINEERING_NOTES.md` if it is made at all — not discovered halfway
      through a CPU implementation that turned out too slow.
    - **The trap: light must never become a simulation input.** F3.5 settled
      that rendering does not feed the simulation, and gave the sharper reason —
      two players on the same seed and the same input log must not diverge
      because of what was on screen. A light value read by anything in
      `src/physics/` reimports exactly the failure mode F1 spent seven steps
      ruling out, through a feature that was only ever supposed to touch
      rendering.
    - **The gate this item was given as a footnote is now broken on purpose, and
      here is the argument.** The gate said: lighting an unlit debug palette
      flatters nothing, so wait for V5, V6, V3, V4 and V8. That reasoning is
      about **albedo** — it is true of terrain, whose look is what colour it is,
      and it is false of fire, whose look is what it *emits*. Reference footage
      of a burning scene ([PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 1
      follow-up) makes the size of the difference plain: the flame cells occupy
      a modest band and the whole cavern is lit, walls tens of cells out picking
      up orange. Strip the glow and what is left is a ribbon of orange noise on
      black — which is precisely what E9's rebuilt fire will produce on its own.
      **Fire is the one subject in this tier where the light is the art**, so it
      is the one subject the gate was never really about.
        - **What this does not license is doing V7 early for everything else.**
          The gate stands for terrain, for the player and for the backdrop, and
          V7 stays after V8 in the running order for those. What is pulled
          forward is emissive light from cells that are hot, which E2 already
          tells the engine for free. Anything needing authored albedo to respond
          correctly waits where it was.
        - **The scope trap, named because this wave is the one most likely to
          grow.** V7's architecture note below is the affordable shape and it
          does not change: a downsampled light grid, a handful of propagation
          iterations, one extra `SDL_RenderCopy`. Pulling the item forward is a
          reorder, not a licence to reach for the shader path — that decision
          keeps its own entry in `ENGINEERING_NOTES.md` and is not made in
          passing while chasing a look.
    - **Built as specified, and the scope trap held.**
      `src/render/light.{h,cpp}` — a light grid at one texel per 4x4 cells,
      sixteen max-propagation iterations with a convergence early-out, packed to
      ARGB and drawn with one additive `SDL_RenderCopy` under linear filtering.
      **No shader path was reached for, so `ENGINEERING_NOTES.md` gains no
      entry**, which is the outcome that note was written to make checkable.
      Measured in `PERFORMANCE.md`: 0.20 ms/frame lit, 0.05 ms unlit, ~1.2% of a
      60 Hz frame — and the measurement needed a second instrument, because
      `grid_bench` times the simulation step and this is not in it.
        - **The "light must never be a simulation input" trap is enforced by the
          dependency direction and by the build file.** `light.h` includes
          `physics/grid.h`; nothing under `src/physics/` includes `light.h`, and
          it is kept out of `ENGINE_SOURCES` in its own `RENDER_SOURCES`
          variable so that the day something in the simulation needs it, the
          mistake has to be written down in `CMakeLists.txt` to compile at all.
        - **What is still open, and deliberately not done here:** emission is
          read from temperature alone, so this lights hot things and nothing
          else — the gate on terrain, player and backdrop albedo stands exactly
          where V8 left it. Additive only, so nothing in the scene is *darkened*
          by the absence of light; a world with no fire in it looks precisely as
          it did before. That is the reorder's boundary, not an oversight.

- [ ] **V9 — A non-simulated effects layer, and impact feel.** *Observed:* E3's
  fractures already happen in silence — a slab cracks in half and drops with no
  dust — and E6's explosions will be worse, because the bigger the event the
  more conspicuous the absence. *Unlocks:* the feedback that makes destruction
  read as destruction rather than as the world rearranging itself.
    - **Sparks, embers, dust and smoke wisps interact with nothing, so
      simulating them buys nothing and costs the hot loop.** They are drawn on
      top, on the same layer as V4's props. This is the distinction
      [notes/art_direction.txt](notes/art_direction.txt) already draws for
      scenery — if it exercises no system it belongs in a drawn layer rather
      than in the grid — applied to effects instead of to decoration. **Do not
      confuse this with E5a.** E5a is matter with a velocity that stays in the
      grid; this is a visual that expires and never enters it. A spark that has
      to conserve mass is a cell and belongs there.
    - **Bounded by construction:** a fixed-size pool with the oldest recycled,
      and no allocation per spark. The failure mode otherwise is a frame-rate
      cliff that appears exactly when the most is happening on screen, which is
      the worst possible time for it.
    - **This is where the free-particle idea E5a rejected actually belongs**,
      and the two items should be read together. E5a keeps moving matter in the
      grid precisely so that it does not need a second answer to every rule in
      the engine. A spark needs *no* answer to any of them — it conducts no
      heat, reacts with nothing, occludes nothing — so a sparse off-grid list
      with a sub-cell position is exactly right here and exactly wrong there.
      The dividing line is conservation: a thing that has to conserve mass is a
      cell, and a thing that expires is one of these.

- [ ] **V11 — The visual system is adaptable.** *Observed:* five places where
  changing the art direction is expensive out of proportion to the change, all
  found by reading the render path on 2026-08-09, and all cheaper to fix now
  than after another visual item lands on top of them. *Unlocks:* the ability to
  change direction repeatedly, which is a stated expectation of this project and
  is currently served by nothing. **Admitted on the first question rather than
  the second, which is unusual for this track and is the point:** none of the
  five is a look, all five are the cost of changing one.

    > **Status 2026-08-16: three of the six bullets are shipped and one is
    > partly.** The layer list, the parallax duplication and the sixth bullet's
    > lighting field are done; the mid-ground band has a slot and no art, for
    > reasons below. **The tint bullet — the light pass gaining a multiply — is
    > step 3 of the V block and is what remains of this item**, along with
    > runtime `Camera::SCALE` and the theme-loadable material colours, which
    > stay with V12/V13. Each bullet below carries its own outcome.

    - **~~There is no renderer.~~ ✅ Shipped 2026-08-16, in two commits by
      design.** *(Original text, kept because the estimate in it is what got
      corrected:)* Roughly 350 lines of frame composition sit inline in
      `main.cpp` with the layer order hard-coded — clear, sky, mountains, props,
      cells, player, light, reticle, HUD, hotbar, menu.
      [notes/reference_observations.txt](notes/reference_observations.txt) has
      **already** concluded that a mid-ground band is needed that this stack has
      no slot for, and V8's remainder is a third depth layer. Extract
      `render/frame.cpp` holding an explicit ordered list of layers. An
      afternoon, after which adding a band is one entry rather than surgery
      between two comments — and `main.cpp` stops being the file every visual
      change has to touch.
        - **It was ~175 lines, not ~350**, and the difference is that the larger
          figure counted the reticle, the HUD, the hotbar, the run-over wash and
          the settings menu. Those are UI, they stayed in `main.cpp`, and **the
          boundary is load-bearing rather than filing**: V7's light pass is the
          last thing drawn in the world and everything after it is deliberately
          unlit, which is what defect B1 was about.
        - **Split into V17 (move it, change nothing, checksum it) and V11
          (restructure it), and that order is the finding.** V17's own entry had
          to admit it could not retroactively prove the extraction was a no-op,
          there being no checksum before the move — the claim rested on the diff
          being verbatim. V11 is where that stopped being a limitation: the
          layer table, the camera method and the generated header were built and
          run **first**, against V17's number, which held unchanged at
          `0x3d729ad7fbcaa839`; the band was added after, and the number moved
          to `0x06f6412da7af6607`. **The general form: when a change has a no-op
          half and a visible half, build the no-op half first and prove it,
          because afterwards you cannot.**
        - **`FRAME_SOURCES` is a fifth source-set variable**, kept out of
          `RENDER_SOURCES` because `frame.cpp` is the only rendering source that
          calls SDL and folding it in would make nine headless suites link SDL2.
          `golden_frame_test` is the first test target in the project that links
          SDL, and still needs no display.
    - **Material colours are `constexpr`, so a retune is a recompile and a
      re-validation.** Give each `MATERIALS` row a palette *slot* and put slot →
      colour in a loadable theme. **The correctness pass already learned exactly
      this lesson once** — it separated the level file's colour codes from the
      render palette after V2's retune booted the game to a blank world for a
      whole commit — and this is one further step of the same move, from "the
      level does not depend on the palette" to "the palette is not in the
      binary". A second biome is then a file and time-of-day is two files and a
      blend. **The cost, stated rather than discovered:** `pixels` holds baked
      jittered colour, so a theme swap needs one pass over the world. That is a
      one-off at swap time, not a per-frame cost, and the hot loop is untouched
      — which is the only version of this that is affordable and should be built
      as that version deliberately.
    - **`Camera::SCALE` is a compile-time constant**, and `display.h` argues for
      keeping it one. That argument is about the window, and it is right about
      the window; it is now also load-bearing for the reticle's size, the sprite
      offsets and the prop rectangles. A zoomed-out biome, a different cell size
      and the already-scheduled resolution options all collide with it, and
      every system that reads it makes the change more expensive. Make it
      runtime while three things read it rather than six.
    - **The light layer can only add, so every biome will be the same
      brightness.** V7 says this in its own text as an open-by-design limitation
      — it lights only *hot* things and it only ever brightens. Generalising it
      to multiply-and-add (an exposure and tint term as well as an emissive one)
      gives night, underground, fog and per-biome grading without touching a
      single material colour. **Given that the reference finding was that the
      read comes from silhouette layering rather than detail, this is the
      highest-value visual control available**, and it is a strictly smaller
      change than V7-rest, which it partly absorbs. ✅ **Shipped 2026-08-16
      (block step 3), an afternoon against a week.** It came out as *two* knobs
      and not the one this bullet describes — a per-layer `Grade` on every row
      of the layer table, and a world-wide one as its own pass — for the reason
      recorded at the depth-band finding above: a global multiply cannot
      separate bands. **The estimate was wrong for a reason worth keeping**:
      this was priced as the item that would finally spend
      `SDL_ComposeCustomBlendMode`, the escape hatch the renderer-versus-shader
      refusal has held in reserve through two examinations. It needed no custom
      blend mode — `SDL_BLENDMODE_MOD` is stock and `SDL_SetTextureColorMod`
      does the per-layer half with no extra draw call. **All three named escape
      hatches are still unspent, and the first item that looked like it would
      spend one did not**, which is evidence for that refusal rather than merely
      an absence of evidence against it. The bullet's own prediction held: the
      mountains at 0.60 are the whole of the depth fix and they are one number.
    - **~~The parallax factors are duplicated~~ ✅ Shipped 2026-08-16.**
      *(Original:)* between `main.cpp` and `tools/generate_backdrop.py` with
      nothing enforcing agreement; the failure is a seam at the pan limit.
      Generate the header from the tool, exactly as V3.1 did for the player
      sheet. This one is already named in "Where this stands" as an afternoon
      that is not a tier item — it is folded in here because it is the same
      class of problem as the other four and it is silly to do separately. **V16
      retires the problem rather than documenting it** — a layer that wraps
      needs no size relationship to the pan range at all — so this bullet is the
      cheap version bought now and not the last word.
        - **Built as** `python tools/generate_backdrop.py --header` writing
          `src/render/backdrop_layers.h`, from the same table that sizes the
          images. The Python is the one copy.
        - **It bought a second thing the bullet did not ask for, and it is the
          better half.** The header carries each layer's *generated size* as
          well as its factors, so `main.cpp` can compare what it actually loaded
          against what the generator would have produced and warn at startup.
          **The seam stops being a pixel at the far edge of the map and becomes
          a line on stderr.** Retiring the duplication removes the way the two
          sides come apart; the size check catches the case where they came
          apart before the header existed, or where somebody hand-edits
          generated work. The first is prevention and the second is detection,
          and it turned out to be cheap to have both.
        - **A cost this bullet never priced, found by adding a band: image size
          grows with parallax factor, so the *nearest* layer is the most
          expensive one.** Sky is 16 MB, mountains 20 MB, and a mid-ground at
          0.40 would be 32 MB — more than both together, in a repo that tracks
          its assets. `--sizes` prints the table. **This is a considerably
          stronger argument for V16 than "the duplication is annoying" was**: a
          wrapping layer has no size relationship to the pan range at all, so it
          retires a growing asset bill and not just a stale-constant hazard.
    - **A sixth thing, added 2026-08-11, and it is the layer list earning its
      keep on the first day it exists.** The light pass is drawn over
      *everything* between the clear and the reticle, so the boundary between
      "in the world and therefore lit" and "UI and therefore not" is currently a
      comment and a position in 350 inline lines. **Every item after this one
      adds a layer that has to declare which side it is on** — V16's animated
      bands are behind the world and must not pick up firelight from in front of
      it, and a flat-shaded or vector element under V13 has no business being
      tinted by the same crude additive wash as a 4x4 cell. Make it a field on
      the layer record. It costs a bool now and it is a rewrite of the composite
      order later.
        - **✅ Shipped 2026-08-16 as `frame::Lighting`, and it is three values
          rather than the bool this bullet asked for — but it is *narrower* than
          what the bullet wanted, not wider, and the reason matters.** `Lit`,
          `Light` (the boundary, and there is exactly one) and `Unlit`.
          **Writing it exposed that this bullet's own example is not currently
          representable.** V16's animated bands are asked to be *behind* the
          world and *not* tinted by the light pass — but the light pass is one
          additive full-screen copy, so anything drawn before it is tinted, full
          stop. There is no bool that delivers that; it needs the light pass to
          stop being a single wash, which is this item's tint bullet (step 3) or
          a render target.
        - **So the field says only what the composition can enforce, which is an
          ordering**, and two `static_assert`s over the table hold it: exactly
          one boundary entry, and no layer whose declared side disagrees with
          its position. **Verified by reordering the table and watching the
          build fail with the right message**, per the rule about testing a
          guard against the code it is meant to catch. A fourth enum value
          meaning "behind the world but unlit" would have been a field that
          lies, which is the exact failure `CLAUDE.md`'s first rule names — **a
          stated rule that has gone false is worse than no rule, and one that
          was never true is worse still.**
        - The honest consequence, written here because it is a thing step 3 now
          owes: **the sky and mountains are currently tinted by firelight**, and
          by V16's standard they should not be. That is a real defect of the
          single-pass architecture, it predates this item, and it is now
          *declared* rather than merely true.

    - **A seventh, and it is the bullet the reference actually asked for: a
      mid-ground band. ❌ Built 2026-08-16 and removed the same day, on the
      played-frame check its own source note demanded. This is the most useful
      thing in the whole item and it is a deletion.**
      [notes/reference_observations.txt](notes/reference_observations.txt) entry
      4 finds a distinct band between "distant mountains" and "the ground the
      character is on" in five of eight reference frames, carrying the most
      silhouette detail and doing most of the depth work — while our stack goes
      from mountains at 0.15 straight to the world at 1.00 with nothing in
      between. A slot was built at 0.40/0.16.
        - **Entry 4 wrote its own disproof condition, it was checked, and it
          fired.** "A simulated world might fill that band with terrain by
          itself — our world is 1080 cells tall and the camera sees 270 of them.
          Worth checking against a played frame before building a band that
          terrain was already occupying." It went out as checklist step 11 and
          came back **no gap: the terrain already fills it.** The row came out;
          the entry is marked disproved rather than deleted, per the
          documentation rule about keeping wrong predictions beside their
          corrections.
        - **The reason it did not transfer is the transferable part.** Five of
          eight reference frames really do carry that band and it really is
          doing their depth work — the observation is right about *them*. What
          does not cross over is the **mechanism**: a painting has to author
          that band because nothing else will put anything there, and we
          simulate 800 cells of real ground into the same space.
          **`reference_observations.txt`'s own header names this constraint in
          advance** — "anything taken from these frames has to survive being
          composed by a simulation instead of by an artist" — and entry 4 is the
          first observation to fail exactly that test. **The lesson filed at the
          note: ask what in the reference is doing the work, not just what the
          work is.** "A mid-ground band carries the depth" is a result; "a human
          paints one because nothing else would" is the mechanism, and the
          mechanism is what tells you whether it transfers.
        - **It also mis-aimed the depth problem, and that is worth separating
          from the above.** Entry 2 measured our bands at sky 0x14–0x28,
          mountains 0x20–0x30, trees 0x18–0x59, terrain 0x1B–0x78 — near-total
          overlap, which is *why* a busy frame reads flat. A mid-ground drawn
          from today's palette sits in the same value range as the mountains
          behind it, so it would have added a band without adding depth. **Depth
          here is a renderer problem before it is a layer-count problem**, and
          the fix is the tint bullet's multiply (step 3). A band was very nearly
          shipped as the answer to a question it could not have answered.
        - **What the episode proves about this item, and it is the strongest
          evidence V11 has.** The claim admitting V11 was "adding a band is one
          entry rather than surgery between two comments". **The first thing
          that actually happened was a band being *removed* in one row** — the
          table, one draw function, one factor, one fixture texture, in and out
          inside a day. The expensive direction was always going to be changing
          your mind, and that is the direction that got measured. **The golden
          checksum returned to V17's exact value**, `0x3d729ad7fbcaa839`, which
          is a second and independent proof that everything else in V11 composes
          the identical frame the inline code did at V8. **Step 3 then moved it
          for the first time on purpose**, to `0x9d9e92a81c4df07b`, using the
          same procedure a third time: the whole grade mechanism was built at
          identity and run against the old number, which held, before the one
          real value went in. That procedure is now the house style for this
          repo, and it is stated as a rule at V17's entry.
        - **The cost that was never priced, and it survives the deletion as an
          argument for V16.** Layer image size grows with parallax factor, so a
          *near* band is the most expensive layer in the stack: the mid-ground
          would have been 32 MB, more than the sky and mountains together, in a
          repo that tracks its assets.
        - *Reopen trigger:* a location whose terrain does **not** fill the band
          — a flatter scene with a lower horizon than F4's snowbank, or a
          zoomed-out camera once `Camera::SCALE` is runtime, which is this
          item's own remaining bullet. Recorded at the layer table in
          `frame.cpp`, at `generate_backdrop.py`, and at entry 4.
    - **V12 and V13 are the same item type and should be taken as one block with
      this one.** All three are the cost of changing the visual direction rather
      than a direction; all three edit the same two destination rectangles in
      `main.cpp`; and V11's runtime `Camera::SCALE` and V13's per-asset scale
      are the two factors of one multiplication. Done separately, those
      rectangles are rewritten three times and the middle version is wrong in a
      way that only shows on one asset.

- [ ] **V12 — The asset layer: alpha, and more than one format.** *Observed:*
  transparency in this project is one exact colour, keyed at load
  (`load_art_texture` in [main.cpp](src/main.cpp), `COLOR_KEY` in
  [tools/pixel_art.py](tools/pixel_art.py)), so **nothing in the game can have a
  soft edge, a partial-opacity pixel or a fade**, and every drawing has to avoid
  `#FF00FF` or it acquires holes — a failure mode [ASSETS.md](ASSETS.md)
  documents because it has happened. `SDL_LoadBMP` is the only decoder and
  `load_art_texture` is the only loader, called four times with four
  hand-managed `SDL_DestroyTexture` calls at shutdown. *Unlocks:* V13's
  smooth-filtered art, V14's rotated parts, `S1`'s masked body, and per-sprite
  opacity through `SDL_SetTextureAlphaMod`, which SDL has always offered and
  nothing here has ever called.
    - **Rotation is the reason this comes before V14 rather than after it.** A
      colour-keyed edge is binary by construction: the key becomes alpha 0 or
      alpha 255 and there is nothing between. Rotate that through
      `SDL_RenderCopyExF` and every diagonal is a staircase of hard-edged 4x4
      blocks, which is precisely the "rotated parts stop sitting on the lattice"
      objection [notes/procedural_animation.md](notes/procedural_animation.md)
      says is a taste call checkable in an afternoon. **Checking it against
      colour-keyed art answers a different question than the one being asked**,
      and answers it in the wrong direction.
    - **This crosses the zero-dependency line and that is a decision, not an
      implementation detail.** `ENGINEERING_NOTES.md` records PNG losing to BMP
      at F4 and the UI library losing at V10, both under "zero new dependencies
      until a specific need can't be met without one". **The F4 verdict does not
      bind here and the reason is worth stating rather than assumed:** it was
      about a *test fixture*, where the need was "read an image" and BMP met it.
      The need now is alpha, which BMP cannot carry at all — the format's 32-bit
      variant is not what `read_bmp` accepts and not what SDL hands back.
      `stb_image.h` is a single vendored header with no build-system entry,
      which is the cheapest possible form of crossing the line, and the crossing
      goes in `ENGINEERING_NOTES.md` next to the entry it is departing from.
    - **The trap, and it is the one that would quietly undo V6.** The locked
      palette and `tools/validate_palette.py` only understand BMP. Allow PNG
      everywhere and half the art silently stops being checked, which is exactly
      the state V6 exists to end — and it would present a year later as "the
      palette drifted", with nothing able to say when. **The boundary is a rule,
      not a convention:** BMP-with-colour-key stays the format for everything
      inside the locked palette — terrain, props, the player sheet — and
      PNG-with-alpha is only for assets deliberately outside it, each one
      listed. `assets/sprites.txt` is where that list already wants to live,
      since it is the file that says what each key loads from.
    - **Built as** one small `TextureCache` in `src/render/` that owns load and
      destroy, keyed by path. Today the prop cache is a
      `std::vector<std::pair<std::string, SDL_Texture*>>` in `main.cpp` that
      doubles as the destroy list, which works and does not generalise: every
      new kind of asset after it adds a member, a load site and a matching line
      in the shutdown sequence, and the failure mode of forgetting the third is
      invisible.
    - *Verify.* An asset with a genuine alpha gradient loads and composites; the
      existing colour-keyed assets are byte-identical on screen (this item must
      change nothing that ships today); `validate_palette.py` refuses to be
      pointed at a file the boundary rule says it does not cover, rather than
      passing it vacuously.

- [ ] **V13 — Sprites carry their own resolution.** *Observed:* "one BMP pixel
  is one world cell" is not enforced anywhere and is assumed everywhere —
  `Prop`'s destination rect and the player's are both native size times
  `Camera::SCALE`, taken straight from `SDL_QueryTexture`. **So the only way to
  draw a more detailed sprite is to make it a bigger object**: a character drawn
  at twice the density is a character twice as tall. *Unlocks:* varying pixel
  resolution inside one scene, which is the first of the four goals above; a
  detailed protagonist over chunky terrain; and flat or vector art at any
  density, rasterised to a texture at load, which needs nothing new in the
  renderer.
    - **Built as** a `pixels_per_cell` field on the manifest record in
      [src/scene/sprites.h](src/scene/sprites.h), plus the filter mode
      (`nearest` or `linear`, already a per-texture setting via
      `SDL_SetTextureScaleMode` and already used for the light texture) on the
      same row. The format already carries an optional `[frame_w frame_h]`, so
      this is a third optional field and the parser's shape does not change.
      **It belongs in the manifest and not in code for the reason `sprites.h`
      already gives about the frame table pointing the other way:** how many
      cells tall a character is, is a fact about the game; how many pixels the
      drawing spends on each of those cells is a fact about the file, and the
      file is what gets swapped.
    - **Four traps, three of which are silent.** The prop planting scan in
      `main.cpp` walks `prop.w` columns of terrain looking for ground — that is
      a count of *world cells* and would be twice the footprint it draws.
      `player_sprite.h`'s two `static_assert`s compare `FRAME_W`/`FRAME_H`
      against `Player::WIDTH`/`HEIGHT`, which are cells; at 2x they compare
      pixels against cells and **pass while meaning nothing**, which is worse
      than failing. `build_player_sheet.py`'s "exactly 14 px wide, 26 or fewer
      tall" becomes a statement about cells that the tool measures in pixels.
      And the loud one: the terrain cannot participate at all, because its
      resolution *is* the simulation's — one `uint32_t` per cell in
      `Grid::pixels`. **Mixed density therefore has a floor and the floor is the
      world**, which is a design consequence to accept deliberately rather than
      meet on the first asset.
    - **Sequenced with V11 for the arithmetic rather than for tidiness.** V11
      makes `Camera::SCALE` a runtime value; this makes the per-asset factor it
      multiplies. Every draw site touches both, and the intermediate state where
      one is data and the other is a constant is the one where a zoom and a
      dense sprite disagree by exactly the factor nobody is tracking.
    - *Verify.* A 2x-density prop occupies the same world footprint as its 1x
      version and plants at the same y; the player sheet at 1x is
      pixel-identical to what ships today; and a deliberately mismatched sheet
      is *refused* rather than drawn sliced, which is the failure `main.cpp`'s
      existing sheet-size warning already exists to catch and which this item
      gives a second way to produce.

- [ ] **V14 — A part rig: rotation, and attachment from a table.** *(this is P1
  in [notes/procedural_animation.md](notes/procedural_animation.md), which is
  the design work and is not repeated here)* *Observed:* `player_anim::State`
  produces a row and a column, so the whole articulation available to the figure
  is which of nineteen drawn frames is showing. The aiming arm V3.1 built to fix
  exactly this was pulled, and its own entry records that **what killed it was
  the hotspot image, not the draw** — every frame drawn afterwards had to carry
  a marker pixel or `--validate` refused it. *Unlocks:* continuous aim without
  multiplying frame counts, head and limb motion the sheet cannot express, and
  the answer to whether rotated parts read at 4 screen pixels per cell — which
  V15 depends on and which nobody has looked at.
    - **The whole difference from the pulled version is where the attachment
      point lives.** A part attaches at a body-space offset per animation frame
      index, held in a table beside `ANIMATIONS` in `tools/player_sheet.py` and
      emitted into the generated `player_sprite.h`. That is a dozen numbers in
      the file that already generates numbers, against a parallel BMP the artist
      must not forget to paint. **The tax that killed the arm is a tax the sheet
      imposes and a table does not**, and that sentence is the entire case for
      this item over simply rebuilding V3.1's arm.
    - **The rotation trap has been paid for once and must not be paid again.**
      `SDL_RenderCopyEx` mirrors the texture and *then* rotates the quad, so
      flipping the sprite does not mirror the angle — it needs a 180° offset,
      and the naive `180 - angle` reflection is correct for a cursor level with
      the shoulder and inverts the vertical everywhere else. **Both of the cases
      anyone checks by eye are the two it gets right.** It is written down at
      V3.1 and it is written down here because this is the item that walks into
      it.
    - **The solver stays SDL-free and produces transforms, exactly as
      `player_anim` produces a row and a column and `LightField` produces a
      buffer.** `main.cpp` turns a transform into an `SDL_RenderCopyExF`. That
      is what makes it testable in `anim_test`, and V3.1's entry is the argument
      for why that matters here more than anywhere: **a rig's failure modes all
      look like art problems**, and a part attached one cell off reads as "the
      arm is drawn badly" rather than as a table being wrong.
    - **This does not retire `--validate`.** The sheet body survives, so the
      three checks that catch bugs presenting as physics — empty bottom row, gap
      inside the collision box, blank declared frame — all still apply to it.
      **That is the whole reason this is the cheap step and V15 is not.**
    - *Verify.* A part tracks a target through all four quadrants with the
      figure facing both ways (the trap above, tested rather than eyeballed);
      the body loop is unchanged with no parts declared; and the
      rotation-legibility question is answered in writing, with a screenshot,
      because it is the input V15 is priced against.

- [ ] **V15 — A skeletal rig, and feet that find the ground.** *(P2 in
  [notes/procedural_animation.md](notes/procedural_animation.md))* *Observed:* a
  drawn walk cycle assumes a flat floor, and **nothing in this game is a flat
  floor for long** — the terrain is per-cell, arbitrary and constantly destroyed
  by the one verb the game has. Wave 4's D6 and D7 are both the figure's
  relationship to ground it is not actually reading. *Unlocks:* feet that land
  on the terrain that is there, and a second character for the price of a
  re-pose instead of a second sheet — which is the thing `S1` needs and the
  reason this item exists now rather than later.
    - **Admitted on a trigger written in advance, and that is the unusual
      part.** `notes/procedural_animation.md`'s closing section names three
      things that would admit a rig; the third is *"a second character type is
      committed to → admits P2 on amortisation, and that is the case where
      'overhaul' stops being the wrong frame."* One has been. **The note also
      says what to do about the other two arguments and it still holds:** the
      slope-aware-feet case remains unproven by any playtest, so it is a reason
      this item is *shaped* the way it is, not the reason it is admitted.
    - **It must replace `--validate`'s three checks, and nothing currently
      does.** Under a rig there are no frames, so an empty bottom row, a gap
      inside the collision box and a blank declared frame all become
      unexpressible — and each of those exists because it catches a bug that
      **reads as a physics problem rather than an art one**. The rig equivalents
      are checkable and have to be written as part of this item, not after it:
      the lowest part touches the baseline in every reachable pose, no pose
      leaves a gap through the silhouette inside the collision box, and no part
      names a sprite that does not load. **A rig that ships without these buys
      expressiveness by making a class of bug invisible again.**
    - **The clock is the fixed step and any solver with state makes that
      stricter rather than looser.** A spring's stiffness and an IK smoothing
      rate are constants that silently mean something different at every frame
      rate — worse than the walk cycle V3.1 caught, because a wrong-speed cycle
      is visible and a wrong-stiffness limb just looks slightly bad.
      `player_anim.h`'s clock note covers this and is the thing to read first.
    - **Deliberately not P3.** Verlet or spring limbs remain the most expensive
      option on that page with nothing observed in play asking for them, and the
      note's own correction — that the "build it twice" deferral behind E5 is
      void since E5a keeps matter in the grid — makes them *unblocked* rather
      than *due*. **Held on cost, and the deferral states a price rather than
      claiming impossibility**, which is the form E8's entry settled on.
    - *Verify.* A figure standing on a two-cell step has both feet on terrain
      rather than one in the air; a figure walking across ground the player has
      just dug does not slide; the determinism suites are untouched, because
      none of this may reach `src/physics/`; and the three replacement checks
      fail on art that would have failed the originals.

- [ ] **V19 — The seven-band scene, and a ground plane where the reference has
  water.** *(new 2026-08-16, admitted by request)* *Observed:* the request was
  for a scene composed the way `CnC_parallax_*` is composed, built before the
  split-view path rather than after it. **This item is admitted by that request
  and not by the reference**, which matters because
  [notes/reference_observations.txt](notes/reference_observations.txt)'s own
  bound says reference answers "what is possible" and never "what is wrong here"
  — it cannot admit an item, and entry 7 says so at the top rather than being
  read as if it had. *Unlocks:* the first scene in this project composed as a
  depth stack rather than as
  sky-plus-mountains-plus-whatever-the-simulation-put-there, and the composition
  V18's split view is a modification of.
    - **The reference frame is seven layers and we ship two.** Entry 7 walked
      one column and segmented it: sky, far range, mid range, near ridge, shore
      treeline, the ground plane, and a near silhouette. We have `backdrop_sky`
      and `backdrop_mountains`, then the simulated world. **The mapping is not
      one new layer per missing row**, and working that out is most of this
      item:
        - sky → `backdrop_sky`, exists, regrade only
        - far range → **new**
        - mid range → `backdrop_mountains`, exists, already at 0.60 against a
          reference-implied 0.63
        - near ridge → **new**, and it lands in the band V11 built and deleted
          (see the trap below)
        - shore treeline → **new**, same band, same trap
        - ground plane → **new, and it is the item's centre of gravity** —
          *built 2026-08-16 as step 4b; the rest of this entry's bullets on it
          are marked with what came back*
        - near silhouette → **already filled by the simulated world and its
          props.** The reference's foreground rock is a painting the boat passes
          behind; ours is diggable terrain. **Do not author a painted band in
          front of the world** — it would occlude the one verb the game has.
    - **Art is cheap here on purpose and that is a scope decision, not a
      placeholder excuse.** One colour and one or two shades per layer, rough
      silhouettes, generated by `tools/generate_backdrop.py` exactly as sky and
      mountains already are. **Generated layers conform to `PALETTE` by
      construction**, which is the property the palette deferral (2026-08-12)
      explicitly kept for this half of the pipeline. Nothing here needs a drawn
      asset, so nothing here is blocked on V12's alpha channel.
    - **What the water layer actually does, since it is the thing being ported
      to land.** Entry 7 separates four mechanisms, and the split is the useful
      part because only three survive the water being removed: the value ladder
      above the horizon (0.78 per band, compounding); **the horizon being the
      darkest line in the frame** (row-mean luminance bottoms out at the
      waterline, 69 against 156 at the top and 140 at the bottom); contrast
      growing with nearness, with 87 of the frame's levels spent between the
      plane's near edge and the silhouette on it against 14-45 at every join
      further back; and a texture gradient, the plane's marks widening 1.3x-3.0x
      toward the viewer. **The reflection is the one that does not transfer**,
      and it was measured anyway (compressed to ~0.6 vertically, contrast cut to
      44%, lifted +61, desaturated) so that a later reader can tell which of the
      plane's properties we chose to drop.
    - **The land plane brightens toward the viewer for a reason that is not the
      reflection**, and this is the substitution the item turns on. The
      reference's water is bright near the camera because it mirrors the sky; a
      land plane cannot do that. But **a horizontal surface faces the sky and a
      vertical silhouette does not**, which is why moonlit ground reads brighter
      than the trees standing on it — same result, different mechanism, and it
      holds at night when the sky is the only source. That keeps mechanism 2
      intact: the plane still meets the horizon at low contrast and still
      diverges toward the viewer, so the dark pinch survives the water going
      away. **Stating the mechanism rather than copying the appearance is the
      rule entry 4's deleted mid-ground band was bought with** — the result
      transfers only if the mechanism does.
    - **The plane needs per-row parallax and that is the one piece of new
      rendering.** A receding plane has no single depth, so it has no single
      parallax factor; drawn flat at one factor it reads as a wall behind the
      world rather than as ground going away. For a ground plane, screen row
      distance below the horizon goes as inverse world distance, so **the scroll
      factor is linear in the distance below the horizon** — `f(y) = f_far +
      (f_near - f_far) * (y - y_horizon) / (H - y_horizon)`, which is arithmetic
      a strip loop can do in integers. Built as N horizontal strips, one
      `SDL_RenderCopy` each, source row height shrinking with distance so
      mechanism 4's texture gradient falls out of the same relation instead of
      being authored twice. **No custom blend mode, no render target, no
      shader** — so this does not spend any of the three named escape hatches
      either, which is now the second item in a row to have looked like it
      would.
        - **Built 2026-08-16 as step 4b, and the prediction above was right
          about the mechanism and backwards about the arithmetic.** The
          relation, the strip loop, the integer arithmetic and the escape-hatch
          claim all held; `backdrop_wrap::plane_strip` is the built form of the
          formula in the line above it. What is wrong is "source row height
          shrinking with distance" — it *grows* with distance, because a far
          strip spans a huge range of world distance in a handful of screen
          rows. The mechanism named in the same sentence (the reference's marks
          widening 1.3x-3.0x toward the viewer) is what the geometry produces
          and what shipped: measured on the built strips the near strip is
          magnified 3.46x against the far strip's 1.06x. **The sentence was
          wrong, not the mechanism**, and it is corrected rather than deleted
          because the shape is the one this project keeps meeting — a prediction
          that names the right effect and inverts the relation that causes it.
          `tests/test_backdrop.cpp`'s property 6 is what a linear mapping fails,
          and it was verified against one.
        - **One cheapness taken deliberately: only the vertical is scaled.** A
          true plane shrinks a mark in both axes; the tile's width is constant
          across every strip. Scaling it per strip is free arithmetically and
          looks worse — adjacent strips would tile at different widths, their
          phases would diverge, and the mark pattern would stair-step at all
          twenty-four strip boundaries. Twenty-four visible seams bought to fix
          a foreshortening nobody can see on a night-dark texture.
        - **`GROUND_STRIPS` is the item's one unpriced cost and it is unpriced
          for a reason worth stating.** The plane issues 24 strips times their
          tiling copies every frame, against one draw call for every other band.
          *Verify* below asks for both halves of the frame-budget rule — and
          **`grid_bench` times `Grid::update` and `Run::step` and cannot see a
          draw call at all**, so neither half can reach this. The synthetic rows
          and the replayed row are both blind to it. The honest instrument is
          the frame rate in the running game, which is checklist step 11, and
          this bullet exists so the next reader does not go looking for a bench
          row that cannot exist.
    - **The grades come out of the ladder and are a derivation from measurement,
      not a measurement.** Entry 7's compounded column gives 1.00 / 0.74 / 0.63
      / 0.46 / 0.37 for the five bands and 0.45 → 0.80 across the plane. Those
      are ratios against the reference's sky and our sky is a night sky at L 26,
      so they are a *shape* to author against and every one of them is a
      `TUNING.md` row with a dated History line the first time it is touched.
      **The corroboration worth keeping:** our mountains were set to 0.60 from a
      luminance measurement against our own art, and the reference's same band
      compounds to 0.63 — two independent routes to within 5%, which is the
      strongest evidence the ladder is a real thing and not a property of one
      painting.
        - **The plane's own two numbers came back inside 2% of the ladder**
          (2026-08-16). Authored with the 0.45 → 0.80 ramp baked into the tile
          and placed by a 0.53 grade, `assets/backdrop_ground.bmp` measures
          **0.44 → 0.81 of our sky**, at an internal ratio of 1.84 against the
          reference plane's 1.78. Its horizon edge lands at luminance 11.7,
          under the graded mountains at 16.9 and under the darkest sky row at
          18.1 — so **the horizon is the darkest line in the frame**, which is
          mechanism 2 surviving the water being removed. That is a derivation
          checked against our own art rather than a second measurement, and it
          is stated that way in the `TUNING.md` row.
        - **The ramp is in the art and the level is in the grade, and the split
          is forced rather than chosen.** A `Grade` multiplies uniformly, so it
          cannot make a surface brighten toward the viewer; only the tile can.
          What the grade does is place the band. Anyone reaching for the grade
          to fix a plane that reads flat is reaching for the wrong knob.
    - **One band gets the warm accent and the rest stay cool.** Entry 8 found
      the sky at 0.0% warm pixels and the shore treeline at 47%, with the warm
      population held at a near-constant luminance (96-104) while the cool
      population descends 174 → 73. So the accent **crosses** the ladder instead
      of obeying it. Applied here that is one band — the treeline — authored
      warm at a fixed value, everything else on the cool ramp. It is entry 6's
      hue-isolation trick at band scale and it is why our player is already cool
      against a warm world; **this inverts that locally and the two must be
      checked against each other**, because a warm treeline and a warm world is
      the player's isolation spent twice.
    - **Trap 1, and it is the one that can waste the whole item: two of the five
      bands land where V11 built a band and deleted it the same day.** The near
      ridge and the treeline sit in the mid-ground, and on 2026-08-16 a
      0.40/0.16 band went in there and came out on checklist step 11 because
      **our terrain already fills that space** — 1080 cells of world into a
      270-cell viewport. The reopen trigger recorded at the time is "a location
      whose terrain does not fill the band, or a zoomed-out camera once
      `Camera::SCALE` is runtime." **This item must fire that trigger before
      authoring anything into the band, and the check is one screenshot**: where
      does the terrain's skyline sit in the target scene? If it fills the band
      again, the honest outcome is a five-layer scene rather than a seven-layer
      one, and the deletion stands twice instead of being quietly reversed.
    - **Trap 2: the reference cannot supply a parallax factor and it nearly
      supplied one anyway.** Entry 1 established the three frames are three
      generated lakes rather than one pan — region shifts non-monotonic in depth
      and opposite in sign at the two edges. So every new factor here is a
      **stated derivation**, labelled as one everywhere it appears, exactly as
      the deleted band's 0.40 was. Parallax is inverse depth, so a geometric
      ladder between the two factors that already exist is the defensible
      construction: 0.04 → 0.08 → 0.15 → 0.28 → 0.52 → 1.00 is that ladder at a
      ratio of about 1.9, and **it lands the existing mountains at 0.15 without
      moving them**, which is a check on the construction rather than a
      coincidence to lean on.
    - **Trap 3: the composition being copied is a horizon composition and this
      game is a side-on cross-section.** The reference looks *across* a plane to
      a horizon; our camera looks at a wall of cells. The plane therefore cannot
      be our terrain — **it is a backdrop band drawn behind the world**, ground
      receding behind the play plane, and the terrain stays the near silhouette
      in front of it. This is the whole reason the mapping above has seven rows
      and only five new ones.
    - **Trap 4, and it is a decision rather than a hazard: the reference's plane
      is brighter than what stands on it, and our world layer is at grade 1.0.**
      Plane near edge 138 against foreground rock 52 is the frame's largest
      contrast and it runs in that direction. Ours would run the other way
      unless the world row takes a grade below the plane's — which is coherent
      (the table's order is Lit → Grade → Light → Unlit, so a darkened world is
      lit back up by fire and lamps rather than through them) and is also a
      change to how the play area reads. **Do not decide it inside this item's
      implementation.** Either the plane sits low enough that they do not
      compete, or the world gets a grade and that is its own row in `TUNING.md`
      with a playtest against legibility while digging.
    - **The cost is real and it is the best argument V16 has ever had.** A layer
      must cover the window plus the pan range at its own factor, so the nearer
      the band the larger the file: `--sizes` prices 0.08 at 17.3 MB, 0.25 at
      24.8 MB and 0.40 at 32.3 MB, against sky's 15.9 and mountains' 20.3 today.
      **Five new pan-sized layers roughly triples the asset directory.** The
      plane in particular is the nearest band and the worst case, and a plane is
      exactly the kind of texture that tiles — so **V16's wrapping layer may
      have to be pulled into this item rather than following it**, and that is a
      sequencing question to answer before authoring, not after. `python
      tools/generate_backdrop.py --sizes` before committing to any factor.
        - **Answered 2026-08-16: pulled in, and the price it paid off is larger
          than the estimate above.** `backdrop_ground.bmp` is a 256x256 tile at
          **0.2 MB**, against the 32 MB the same band costs priced flat — the
          plane alone is the whole of V16's argument recovered. `--sizes` now
          labels a wrapping row rather than printing a number that would be
          wrong, and `layer_size()` carries a refusal saying a wrapping row must
          not be run through it. The arithmetic landed at step 4a
          (`backdrop_wrap.h`, thirteenth suite, no source set linked at all) and
          the draw path at 4b.
    - **Two mechanical consequences that are expected and not breakages.** The
      golden checksum moves, and the new value goes in the same commit as the
      change — the house procedure applies in full: build every new layer at
      identity, run it against the current number, and only then put the real
      grades in, so the move has exactly one cause. And the layer table's three
      `static_assert`s hold the rank order; adding five `Lighting::Lit` rows
      must not disturb "exactly one light pass, at most one grade, and the grade
      row's own grade is identity."
        - **The house procedure had to be adapted at 4b, and the adaptation is
          the part to copy for the remaining three bands.** "Run it against the
          current number" is what the four previous uses did, and **a new band
          that draws pixels has no such half** — a layer that composes to the
          old checksum is a layer that is not in the frame, and leaving its
          texture null to arrange that is precisely the anti-pattern
          `.claude/rules/simulation.md` names. So the separation was bought with
          **two numbers instead of one**: the plane at identity gave
          `0xfd8e2f04b7037278`, and the grade then took it to
          `0x24eb769681836a0e`. The first number is the geometry, the step
          between them is the grade, and both are written into the constant's
          comment because the intermediate is not recoverable from the file
          afterwards. The `static_assert`s held untouched, as predicted.
    - *Verify.* The terrain-fills-the-band question is answered by screenshot
      **before** any band is authored, and written down whichever way it comes
      back; the frame composes byte-identically with every new grade at
      identity, before the grades go in; the per-row plane scroll is priced on
      both halves of the frame-budget rule — P4's replayed row for p99 and
      steps-over-budget, and the bracketed synthetic rows at 1920x1080, because
      N strips a frame is a per-frame cost the played row is too quiet to see;
      the parallax factors are labelled derivations in the header comment and in
      `TUNING.md`; and README checklist steps 11 and 12 are both owed — 11
      because the layer table changes shape again, 12 because the whole item is
      a retune of the thing step 12 exists to watch.

- [x] **V20 — The value ceiling, and the two defects V19 4b shipped.** *(new and
  done 2026-08-16, admitted by playtest)* *Observed:*
  [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 6, the first human eyes on V19 4b,
  returned three visual defects against the ground plane. *Unlocks:* every
  remaining V19 band, because all three causes are properties of the stack those
  bands would be authored into rather than of the plane.

    **The direction question was asked with the report — "are we going down the
    wrong route to achieve the Cast n Chill graphics" — and the answer is no.**
    Every mechanism entry 7 identifies is built and none is misconceived. What
    was wrong is amplitude, and one structural mistake produced it.

    - **The structural mistake: the ladder was built downward from a floor
      instead of downward from a ceiling.** `sky_deep` was authored at luminance
      **18 of 255**, and the only separation tool the project owns is `Grade`,
      which is a multiply and can therefore only darken — so every band added
      since V11 was pushed further toward zero from a sky that was already at
      the bottom. Entry 7's ladder is 0.78 per band over six bands, a factor of
      3.4; from L 18, band six lands at **L 5**.
        - **The numbers, measured on `resources/game_screenshots/plane_test
          (1-4).png` and on the shipped BMPs.** The whole composition occupied
          **L 15.5 to 24.5** against the reference's 51.6 to 173.6. Nine levels.
          **The reference's *smallest single band join* is 14** — larger than
          our entire frame's range — and its **night** frame has its sky at L
          163, nine times ours.
        - **The palette group was raised wholesale**, stated as post-grade
          targets since that is what reaches the screen: sky 95 → 62 top to
          horizon, mountain rim 71, mountain body 44, ground far **30** (the
          frame's darkest value, entry 7's mechanism 2) ramping to 78 at the
          near edge. The two grades were **not** touched — 0.60 and 0.53 keep
          their `TUNING.md` rows and their history, and the horizon join comes
          out at 14 levels, which is the reference's own.
        - **The sky ran the wrong way and nobody had looked.** `sky_deep` (the
          top) was darker than `sky_horizon` (the bottom), so the frame's
          brightest row sat immediately above the row mechanism 2 requires to be
          its darkest. Corrected by the palette alone; the generator loop is
          unchanged.
        - **The one-line version of the error is preserved in `pixel_art.py`
          rather than deleted**, because the shape recurs: the ground pair was
          justified as "ratio near/far 1.83 against the reference plane's 1.78".
          That is arithmetically true and it is the wrong quantity — entry 7's
          plane is a ratio of 1.78 **and a difference of 61 levels**, and
          matched as a ratio down at L 18 it bought a ramp of 18 levels before
          the grade and **9.8 after**. **When a mechanism is absolute contrast,
          matching its ratio is not matching it.**
    - **Defect F-2, "mountains are not visible just the plane", was not the
      grade and the checklist asked the right question about the wrong layer.**
      Measured, the mountains against the sky were the **largest contrast
      anywhere in the frame**. The plane was covering them. `draw_ground` placed
      the horizon at 0.55 of the *window*, justified as "where the played
      frame's terrain skyline sits" — which names the terrain, while the layer
      it collides with is the mountains, authored independently in their own
      image's coordinates by a different script. The plane's horizon ran between
      screen rows 594 and 238; the mountain silhouette began at 604. The plane
      is opaque RGB with no colour key and is drawn after the mountains in
      `LAYERS`. **The two constants were contradictory at every camera position
      the world reaches, and nothing could have said so** — step 12 was watching
      the grades.
        - **Fixed by making the contradiction unrepresentable, not by retuning
          either number.** The horizon is now derived from
          `backdrop_layers::MOUNTAINS_SKYLINE_MAX`, generated from the same
          seeded walk that draws the silhouette, so the plane's far edge *is*
          the mountains' deepest skyline row. The silhouette was also raised in
          the art (`MOUNTAIN_BASE_FRACTION` 0.58 → 0.31) so there is room below
          it for a plane at all.
        - **A second, independent half of the same defect: the horizon had the
          wrong parallax factor.** It moved at the plane's own 0.11 while the
          mountains move at 0.06 — **a receding plane's far edge is at infinity
          by construction, so its factor has to be the smallest in the scene,
          not the plane's near-edge one.** Given 0.11 it climbed past the
          mountains within a few hundred cells of descent even from a correct
          starting position. Now on the mountains' factor, which locks the two
          together.
        - **Stated as a fraction of the loaded mountains texture and not as a
          row index, and that distinction cost a diagnosis.** Stated as a row it
          was a number only the shipped 1642-row BMP could satisfy; against the
          golden fixture's 300-row synthetic mountain it put the whole plane
          below the window. The frame composed cleanly, every check in
          `test_golden_frame.cpp` passed, and **the checksum silently reverted
          to a value from before the plane existed.** It was caught only because
          that value was recognised. `.claude/rules/simulation.md` already warns
          that a checksum over a null-textured layer covers the layer's
          *absence*; this is the same hazard reached by geometry, and the
          fixture now asserts the plane's presence by composing once without its
          texture and requiring a different frame.
    - **Defect F-1, "black bands appearing in between the plane pixels", was two
      causes that produce one symptom, and each needed its own half of the
      pipeline.**
        - **The arithmetic half.** `backdrop_wrap.h`'s own comment states the
          requirement — "strip i's bottom source row is strip i+1's top source
          row, or the texture repeats or skips a row at every boundary" — and
          the draw call broke it: `SDL_Rect src{0, (int)s.src_y, w, (int)s.src_h
          + 1}` rounds each strip's start and its height *independently*. Run
          against the shipped geometry that draws **268 rows of a 256-row tile
          and misses at 12 of its 23 boundaries**, which is why the symptom was
          a set of bands rather than one line. Fixed by rounding the shared
          *boundaries* instead, in `plane_src_row`, so adjacent strips meet by
          construction.
        - **`test_backdrop.cpp` property 4 passed the whole time, and
          generalising why is worth more than the fix.** It checks the float
          rows meet, and they do. Nothing asked whether their *roundings* did.
          **When a continuous quantity is checked for a property and then
          quantised before use, the property has to be re-checked on the
          quantised value** — otherwise the passing test is measuring something
          the renderer never sees. The new property 7 asserts conservation on
          the integers, and was verified against the unfixed arithmetic.
        - **The art half, which no arithmetic could have fixed.** The tile was a
          **49/49 ordered dither between exactly two tones**, point-sampled at
          up to ten source rows per screen row near the horizon, so which rows a
          strip landed on decided whether it came out dark — a band that moves
          as the camera moves. Cause: `dither_mix` picks between two colours, so
          a "ten band" ramp built from it contains **two colours and ten
          proportions of them**. `banded_ramp` now produces ten distinct flat
          tones with dithered hand-offs, which is what V5's rule ("flat tones,
          not a per-pixel smooth blend") actually asked for and what it had
          quietly stopped being.
    - **Defect F-3, "dashes move smoothly but the effect isn't very convincing",
      was amplitude plus one specific loss.** The plane's ramp was **9.8 graded
      levels against the reference's 61**, and tile rows 160-255 — the near
      third, where entry 7's mechanism 3 says contrast should be *growing* —
      were flat at L 40.4, because the two-colour dither had already saturated.
      The ramp is now 30 → 81 post-grade, **50.6 levels**, gradient to the near
      edge. The marks were also weighted toward the viewer and cleared out of
      the top 30% of the tile: a mark near the horizon is being point-sampled at
      ten to one, which is a speckle that flickers as the camera moves rather
      than texture, and it is the wrong end of the frame for it under both
      mechanism 2 and mechanism 3.
    - **The golden checksum moved to `0xcde4dc1a39927fca` and the house
      procedure was set aside rather than claimed.** "Ship the no-op half first"
      needs a no-op half; this is three causes that all move pixels and none of
      which can be staged at identity. That is written into the constant's
      comment plainly, because **a procedure claimed and not followed is worse
      than one openly set aside.** The separation it does not have is carried
      instead by `backdrop_test`'s two new properties, which pin the rounding
      independently of any frame.
    - *Verify.* Full suite green (13/13). **Not verified and owed: every part of
      this that a human has to look at** — the raised palette is a large
      aesthetic swing that no test can judge, the mountains' new position is a
      composition, and F-1's art half is specifically a defect that only appears
      while the camera moves. Checklist steps 11 and 12 are both owed, and step
      12's question has changed: it now asks whether the frame is *too bright*,
      which is the opposite of what it was written to watch for.

- [x] **V21 — The ceiling comes back down a fifth, and a stale hand-copy goes
  with it.** *(new and done 2026-08-16, admitted by playtest)* *Observed:*
  [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 7 answered V20's checklist item 1
  **"too bright"** — the one answer that item was explicitly written to invite.
  Three of its five items closed outright (mountains visible, walking bands
  gone, frame rate fine), so V20's two defect fixes are confirmed and only its
  aesthetic swing overshot. *Unlocks:* nothing on its own; it settles the ladder
  that V19 4c's three bands would be authored against, which is why it goes
  first.

    **This is a ceiling move of the second kind and nothing else.**
    `.claude/rules/assets-and-formats.md` states the test: a change that alters
    the ratio between two bands is the refused move and belongs in a grade; a
    change that scales every band and leaves the ratios intact is a ceiling, and
    no grade can perform it. V21 multiplies the backdrop palette group by
    **0.80** and touches nothing else. `mountains` 0.60 and `ground` 0.53 are
    untouched for the second consecutive retune — which is the argument for
    keeping the ladder in the grades, made twice in one day.

    - **Post-grade the frame now reads sky 76 → 50 top to horizon, mountain rim
      57, mountain body 35, ground far 24 ramping to 62.** Every one of those is
      exactly 0.80 of V20's. `star` is deliberately not scaled: it is a point
      accent carrying no ratio, and the reference's night frame holds its stars
      and moon at full value while everything around them descends.
    - **The factor is 0.80 rather than lower, and the reason is the one number
      V20 existed to buy.** A ceiling is a multiply, so it scales every
      *absolute* separation — and absolute separation is precisely what V20
      learned to hold when matching the reference plane by ratio bought 9.8
      levels. The mountain/ground horizon join is the tightest and is the
      reference's own signature at **14 levels**; at 0.80 it goes to 11, at 0.72
      to 10, at 0.60 to 8 — under the reference's smallest single band join and
      most of the way back to the flat frame session 6 reported. **The ceiling
      is nearly out of downward room.** Recorded so the next session does not
      reach for a smaller factor by reflex: a second "too bright" is a
      hue-and-saturation item (this group is a strongly saturated violet, and
      saturation reads as brightness) or a grade item, and either is a different
      argument.
    - **The report was "too bright" and not "washed out", and the item offered
      both.** Kept because the discrimination is the whole justification for a
      uniform scale: the complaint is level, not separation and not hue, and a
      uniform scale is the only move that answers level without disturbing the
      other two.
    - **A stale hand-copied constant, found while measuring and fixed in the
      same commit.** `draw_clear` held `0x14, 0x10, 0x22` — luminance 18 — under
      a comment reading `sky_deep, tools/pixel_art.py`. **Both halves had gone
      false.** V20 raised the group and did not carry the change here, and V20
      had *also* inverted the sky ramp, which made `sky_deep` the brighter of
      the pair while the comment's stated intent is "the palette's darkest sky
      tone" — now `sky_horizon`. Corrected to `sky_horizon` at the V21 level.
        - **It was invisible, and why it was invisible is the point.** The sky
          texture covers the window, so the clear only shows through a missing
          or unreadable BMP — the exact failure the clear exists to make
          survivable. **A constant that only matters in a failure path will not
          be caught by anything watching the success path.**
        - It is still hand-copied and that is now the **second** duplicated
          backdrop constant to go stale silently, after V11's parallax factors.
          Generating it into `backdrop_layers.h` is the fix; deferred to
          ENGINEERING_NOTES.md rather than done here, because a retune that
          quietly grows a code path is how a measurement stops being bracketed.
    - **The golden checksum did not move, and that is the correct result rather
      than a miss.** V21 is outside its coverage in both halves: the fixture
      **generates its own textures instead of loading `assets/`**, so no palette
      edit can reach it, and the clear is fully overdrawn by the sky.
      `0xcde4dc1a39927fca` stands. **Stated positively because the inverse of
      the V20 trap is just as dangerous** — there, an unchanged checksum was a
      silent defect; here it is a proof of scope. The distinction is whether the
      unchanged value was *predicted before the run*, and this one was.
    - *Verify.* Full suite green (13/13). **Not verified and owed: the entire
      visible effect of the item.** No test in the project can see either change
      — one is asset colour the fixture does not load, one is a failure-path
      constant. The brightness re-check is item 1 of the owed list in
      MANUAL_TESTING.md and it is the only instrument there is.

- [x] **V23b — The camera goes back to centre, and the mechanism goes with
  it.** *(2026-08-17, hours after V23a)* *Observed:* playtest session 9, a
  direction rather than a result: *"lets go back to the camera always being
  centered."* *Unlocks:* nothing. It removes an item's worth of code and hands
  V22 back a constraint it had been relieved of.

    **What shipped is a deletion.** `src/game/camera_bias.h` and
    `tests/test_camera_bias.cpp` are gone; `Camera::set_vertical_anchor`,
    `vertical_anchor()` and the `anchor_y_` member are gone; `main.cpp` no longer
    updates a bias per frame. `Camera::follow`'s vertical expression is character
    for character the pre-V23 one, and the golden checksum went back to
    **`0xcde4dc1a39927fca`** — the value it held before V23, not a new number.
    **That the checksum returned to an earlier value is the evidence the revert
    is complete**, and it is worth expecting the next time `git log -S` on that
    constant lists the same value twice.

    - **Removed rather than neutralised, and that was the decision in this
      item.** The alternative was to leave the mechanism with both anchors at
      0.50 — a smaller diff that keeps the option warm. It was refused because
      this project's recurring failure is a stated rule that stopped matching the
      code: TUNING.md would have gone on listing three knobs, the checklist would
      have gone on describing a framing, and every one of them would have been
      describing behaviour nothing produces. **A knob nobody turns is worse than
      no knob, because it reads as a supported way to change the game.**
    - **The suite count is unchanged at 14.** The Camera half of the retired
      suite is now `tests/test_camera.cpp` — the centred framing, all four
      world-edge clamps, the A1 fractional split and the parallax endpoints. The
      framing checks went with the framing. **The centring assertion matters
      more than it did**: it is now the whole of the shipped composition rule and
      nothing else states it.
    - **What this costs is V22's route, and the cost is exactly the measurement
      V23 was built on.** A centred camera caps the receding ground plane's
      visible share below the player at **~50% by construction**, and it measured
      20.2% at the spawn against a reference reading of "clearly past half". That
      measurement is untouched by the revert. **So V22 cannot reach its target
      framing by moving the player down the frame any more**, and its remaining
      route is the fixture scene — which session 8 had already argued is the
      load-bearing half.
    - **Three sessions, three directions, one day, and that is a result about
      this project's process rather than about the camera.** V23 was built from
      a measurement and three reference stills, V23a from the first human look,
      V23b from the second. **The reference reading was never wrong about the
      references** — what it lacked was any evidence about play, and the item
      shipped with that stated and shipped anyway. The reusable form: when a feel
      item's whole evidence base is still images, its first playtest is not a
      verification step, it is the experiment, and the item should be sized as
      one that might be deleted.
    - **What is not settled and must not be quietly re-litigated.** Whether a
      non-centred framing is right *at all* was never separately answered — it
      was asked for, delivered wrong, corrected, and withdrawn. The two things
      the next attempt would otherwise rediscover are kept at `Camera::follow`:
      an anchor belongs on the camera and not at the caller (it is called twice
      per rendered frame and applying it at one site tears the backdrop), and a
      framing is a fraction of the viewport, never a count of cells.

- [x] **V23a — The dig framing was never delivered, and the report that caught
  it.** *(2026-08-17, the day after V23)* *Observed:* playtest session 8, the
  V23 feel report V22 was gated on. *Unlocks:* nothing new — it repairs V23 and
  re-owes the same report.

    **The finding is not a feel result and that is what makes it worth reading.**
    The tester's words were that the illusion "might be upside down", which is a
    third failure the checklist did not offer — it asked whether the camera read
    as *answering the dig* or as *wandering*. Checked against the code, the sign
    is right: `SURFACE_ANCHOR` 0.80 does draw the player low. **What was wrong is
    that the dig framing was requested and not delivered.** `Camera::follow`
    clamps the view at `world_h - viewport_h` = 810, so a `DIG_ANCHOR` of 0.30
    resolved to **0.51 on screen at the fixture floor** (row ~948) and to **0.70
    at row 1000**. The camera answered the dig *least* where there was most world
    below to see and most where there was least — **an inversion of the item's
    intent produced entirely by the clamp**, and a move that visibly starts and
    does not arrive.

    **The lesson generalises past this item and is the reason for the new test.**
    V23 asserted the constants and the arithmetic that chooses between them; it
    never asserted what a player is shown, because the clamp lives one call away
    in `Camera`. **A composition constant has to be tested as delivered, through
    `follow`, not as requested** — `camera_bias_test` now pins the on-screen
    fraction at the fixture floor, and that check fails against V23's code, which
    is how it was verified.

    **What shipped.** `DIG_ANCHOR` 0.30 → `COLUMN_ANCHOR` 0.50 —
    **renamed, because it gained a second trigger**: being airborne now takes the
    same framing whatever the cursor is doing, which session 8 asked for in the
    same breath ("digging or flying out of frame") and which V23 had no notion
    of. A falling player previously kept the surface framing and spent its ~55
    cells in under a second, which is the likeliest thing behind "flying out of
    frame". Four checks were written first and failed for the right reasons.

    - **The tester's number and the reference reading converged, and the
      convergence is not a coincidence.** 0.50 is what the report asked for, and
      0.50 is also what the clamp was already delivering at the surface (0.511).
      **The tester was describing the framing the game actually produced in the
      one place they could see it**, and V23's 0.30 was a number that existed
      only in the constant. Recorded because it is a reusable shape: when a
      report and a measurement disagree with a constant, check whether the
      constant is reaching the screen at all.
    - **`EASE_PER_SEC` did not move and its meaning did.** The swing is 0.30
      rather than 0.50, so the same 0.85 now crosses it in **0.35 s where the
      checklist asked the tester about 0.6 s**. The speed question is therefore
      still open and session 8's answer to it cannot be reused — flagged in
      MANUAL_TESTING.md rather than quietly re-asked.
    - **One consequence reasoned about and not measured, stated so the re-test
      can find it.** A jump is airborne, so the camera now leans on every hop.
      A hop is shorter than the swing, so it should read as a lean; **if it
      twitches, the fix is to trigger on descending rather than on airborne**,
      which is a different feel and not a retune of this one.
    - **What this does not touch.** The ground plane. Session 8 answered
      question 3 **no** for the fourth time, which is a finding about V22's
      premise and is written up there, not here.

- [x] **V23 — The camera leaves centre, and digging brings it back.** *(new and
  built 2026-08-17)* *Observed:* not a playtest report — a **measurement taken
  while starting V22's scene work**, which is the unusual thing about this item
  and the reason it exists at all. V22's plan was to author a flat open spawn so
  the ground plane's near portion would show; measuring the geometry first
  established that no scene could do it. *Unlocks:* V22, which was blocked on a
  trade this resolves, and with it V19 4c and 4d behind it.

    **The measurement, because the item is a consequence of it rather than of a
    preference.** `draw_ground` runs the plane from `ground_horizon_y` to the
    bottom of the window unconditionally, so the plane is never partly drawn and
    "not visible at spawn" meant the world was occluding **100%** of it.
    `Camera::follow` then centred strictly, with no vertical term, so wherever
    the view is unclamped the player is pinned at screen centre and the plane's
    band below them **cannot exceed ~50% by construction**; at the spawn, where
    the camera sits at its bottom clamp, it measured **20.2%**. The reference
    wants two thirds. **Three rounds of value tuning had been aimed at pixels
    the plane never drew**, which is the whole explanation for why they went
    nowhere, and no fourth round could have worked either.

    - **The trade this had to resolve, and the reading that dissolved it.**
      Anchoring the player low enough to match the reference leaves ~55 cells of
      world below them, which starves digging — the same collision as the
      near-foreground-silhouette refusal, in a new form: *the reference composes
      a scene you look across, and this is a world you dig down into.* It was
      filed as an open decision with a recommendation to split the difference.
      **The answer came back "match the reference, and let digging move the
      camera"**, and
      [notes/reference_observations.txt](notes/reference_observations.txt) entry
      10 is the reading that supports it: across the three frames the subject
      sits at 0.60, 0.36 and 0.27 down — but **the near volume holds the lower
      55-65% in all three.** What is constant is the composition, not the
      subject; the subject rides on the edge of whichever volume the frame is
      about, and rises when that volume goes underneath it. **The two framings
      were never in conflict — they are one rule in two states**, and the state
      is chosen by what the player is doing.
    - **So the anchor is a fraction of viewport height, not a bias in cells**,
      and that is the one design decision here worth defending on its own. What
      is being matched is a composition; a cell count expresses it only at the
      viewport height it was tuned at, and every other entry in `DISPLAY_MODES`
      would get a different picture from the same constant. `Camera::follow`
      already computed `center_y - viewport_h / 2`, so the anchor is that `/ 2`
      becoming a variable, and **0.5 is exactly the old expression** — which is
      what made a genuine no-op half available.
    - **The one real trap, and it is a feedback loop rather than a tuning
      problem.** The anchor moves the view; the view is what `screen_to_world_y`
      resolves the mouse through; the resolved cursor is what `Input` carries
      into the step; the aim is what picks the anchor. That closes, and it
      closes **positively**: digging downward raises the view, which slides the
      world under a stationary hand further down, which reads as a steeper dig.
      It saturates rather than oscillates, so it would not have looked like a
      bug — it would have looked like the camera crawling away under a still
      hand. **Cut by measuring the aim in the unbiased frame**, where the
      anchor's own term cancels exactly rather than being damped and hoped
      about. The suite simulates a held mouse at two anchors to check it, **and
      carries a negative control proving the uncorrected form fails that check**
      — without which the test passes for a reason nobody has verified.
    - **The golden checksum moved for the seventh time, to `0xf29c435ed9d923b1`,
      and this one had its no-op half and took it.** The mechanism shipped in
      its own commit at the 0.5 default and ran against `0xcde4dc1a39927fca`,
      which held; the step between the two numbers is the fixture adopting the
      shipped anchor and nothing else. **The fixture adopts it deliberately**: a
      hash over a configuration the game never runs covers nothing that ships,
      which is the null-texture lesson in a second form.
    - **Nothing was invalidated, and that was checked rather than assumed.**
      `Input::cursor_x/y` are stored in **world cells**, converted in `main.cpp`
      before the step, so a camera change cannot change what a recorded log
      replays — both `.rec` sessions survive, and P4's row stays lit. The scene
      is untouched: launch prints `334901`, matching `FIXTURE_SCENE_CELLS`.
      **This is the cost V22 was going to pay and this item does not**, which is
      worth noting before V22 resumes and pays it anyway.
    - **What is not settled is the feel, and it is the majority of the item's
      risk.** The two anchors are read off still frames; `EASE_PER_SEC` is read
      off nothing at all. **A still image cannot say how fast a camera should
      move**, and the failure at both ends of that constant has the same name
      for opposite reasons — too fast is a cut, too slow never composes either
      framing. Owed to the tester as its own checklist step.

- [ ] **V22 — The plane the player is in.** *(new 2026-08-16, **unblocked the
  same day**; retitled from "What the receding plane is made of" when the
  decision came back)* *Observed:* [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 7
  answered "does the plane recede" with **"no"**, and supplied the reading that
  explains why every previous attempt to fix it by shading failed: in
  `resources/images/CnC_parallax_1..3.jpg` **the reference's receding plane is a
  lake, and the boat floats on it.** *Unlocks:* V19 4c and 4d, which tune bands
  against a junction this item moves. **Nothing is built yet.**

    **The decision came back land, and the title change is the answer.** *"i
    want the first scene to be land not water, but i still want the land layer
    to act like the water layer with the player sitting on/in the plane."* The
    three options this entry offered each answered two questions at once — *what
    is the plane made of* and *where is the player relative to it* — and the
    answer separates them, taking land for the first and the lake's relationship
    for the second. **So the item is no longer about the plane's material at
    all**, which is why it is retitled: leaving the name pointed at the material
    is how an item quietly gets built to its own obsolete premise.

    - **⚠️ The gate answered "no" for the fourth time, on 2026-08-17, and this
      is the first thing to read before spending a week here.**
      [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 8 was the V23 feel report this
      item was gated on, and its third question — does the plane finally read as
      receding — came back **no**, in session 7b's exact terms: *"the character
      should be standing within the ground plane. currently the player is
      standing on the test albedo.bmp setup with the backdrop."* **V23 was the
      first attempt aimed at the cause the geometry actually had, it landed, and
      the answer did not move.** That is the finding, and it is a finding about
      this item's premise rather than about V23: **camera framing was necessary
      and is not sufficient either.**

      What it does *not* license is re-opening the closed decision — the plane
      stays land, and what transfers from the reference is the relationship.
      What it does say is that **two mechanisms have now each been proven
      necessary and insufficient in turn** (value continuity, then framing), and
      the one thing all three reports have named and nothing has yet changed is
      **the fixture scene itself**. Session 8 named it by its file for the first
      time, `albedo.bmp`. The fixture-scene rewrite was already this item's
      second half and was already the expensive one; **this is the evidence that
      it is the load-bearing half, not the dressing.** Sequence it first.

      **The cost of that has not changed and must be flagged before it is
      spent:** the rewrite costs both `.rec` recordings and takes P4's replayed
      row dark until the tester plays and presses `F9`.
    - **Value continuity is necessary and it is not sufficient, and that
      correction arrived hours after the bullet below was written.**
      [PLAYTEST_LOG.md session
      7b](PLAYTEST_LOG.md#session-7b--2026-08-16-the-plane-the-player-is-not-touching)
      reported that the player is not touching the plane at all — they are on
      the fixture scene's terrain, and the plane's near portion is not *visible*
      at spawn. **There is therefore no junction to tune.** The bullet below
      assumed the two surfaces meet at a line whose values could be brought
      together; they meet at the ragged silhouette of a test fixture, and no
      grade reconciles a surface with an object standing in front of it. The
      bullet is kept because everything in it stays true once the surfaces do
      meet — it was the right mechanism attached to the wrong stage of the
      problem.
    - **`CnC_lighthouse.jpg` was named specifically and measuring it produced
      the number this item was missing** —
      [notes/reference_observations.txt](notes/reference_observations.txt) entry
      9. The horizon sits 59.7% down, the boat's waterline 85.2% down, and the
      plane runs to the bottom of the frame: so **64% of the plane is behind the
      figure and 36% is in front of it.** The figure is not at the plane's near
      edge looking across it, it is two thirds of the way down it. That is what
      makes the plane recede *around* the figure, it is geometry rather than
      shading, and it is the reason three rounds of value work went nowhere. The
      second measured fact costs more: **between the boat and the horizon there
      is nothing but plane** — no clutter, no vertical feature except a
      lighthouse island deliberately pushed to one side.
    - **The third-in-front lands on the *world*, not on the backdrop, and that
      is what makes this expensive.** The plane in front of our player is not
      backdrop at all — it is the simulated, diggable world the player already
      stands on. So the requirement is that the terrain at spawn presents a flat
      open surface running to where the backdrop plane takes over, with the
      contact point around two thirds down the combined band. **This is the
      item's real cost and it was not visible when the direction was settled**:
      nothing about it is a rendering change.
    - **The two-thirds is not reachable by editing the scene, and the arithmetic
      says so before anybody redraws. (2026-08-17.)** The plane runs
      `ground_horizon_y` to the *bottom of the window*, unconditionally — it is
      never partly drawn, so "the plane is not visible at spawn" is the world
      **occluding** it, not the plane failing to reach.
      `generate_test_scene.py`'s `FLOOR_TOP = s(380)` is world row 950 of 1080
      and the slab is full width, so everything below the terrain skyline is
      world and the plane's share in front of the player is **exactly zero**,
      not a small number. Now the part that kills the cheap fix: at spawn the
      camera is at its **bottom clamp** (world 1080, viewport 270, so `view_fy`
      pins at 810), the horizon lands at screen 428.6 and the player's feet at
      560, putting the contact at **20.2% of the band**. `Camera::follow`
      centres strictly, with no vertical bias — so **anywhere the camera is
      unclamped the player is pinned at screen centre, and the contact can never
      exceed ~50% of the band by construction.** A surface two thirds down the
      band requires the player ~80 cells *below* screen centre. **No scene edit
      can produce that, because the scene does not control where on the screen
      the player is drawn — the camera does.**
    - **Which turns the item into a camera question, and the camera question has
      a cost that points the other way.** A vertical bias of ~80 cells puts the
      contact at 65.3% of the band (`view_fy` 735, horizon 446.6, feet at screen
      860) and — unlike a scene edit — it holds as the player traverses instead
      of only at the spawn. But 860 of 1080 is the player at **80% down the
      screen**, leaving ~55 cells of world visible below them. **That starves
      digging, which is the game's one verb**, and it is the *same* collision
      the near-foreground-silhouette refusal is about, arriving in a new form:
      the reference composes a scene you look **across**, and this is a world
      you dig **down** into. The two want opposite thirds of the frame. **A
      decision is owed on that trade and it is not one to settle inside this
      item** — filed in ROADMAP_ITEMS.md.
    - **That decision came back the same day: match the reference, and let
      digging move the camera. Built as V23 above.** The recommendation in the
      row was to split the difference at 20-30 cells, and it was **not** taken —
      worth recording as a wrong call rather than quietly dropping, because the
      reason it was wrong is instructive. It treated the two framings as
      endpoints of one axis to compromise along, and looked for the least-bad
      fixed point on it. The answer was that the framing does not have to be
      fixed at all: the reference's own three frames put their subject at 0.60,
      0.36 and 0.27 down, and hold their composition constant underneath. **A
      trade between two states is only a trade if you have to pick one**, and
      that is the question this entry never asked. The bullet above stands
      otherwise — the ~80-cell number and the ~55-cell cost were both right, and
      the second is exactly what the dig framing exists to pay back.
    - **It collides head-on with F4.4, and the collision is the finding rather
      than an obstacle to route around.** F4.4 deliberately made the first scene
      *a test fixture wearing art* — uneven stairs for step-up, fence posts for
      dig-the-base collapse, a pit with pillars for the flood fill, a water
      channel, jump ledges. Every one of those is a vertical feature standing
      between the player and the horizon, on a jagged surface, at the spawn.
      **The first scene now has two jobs and they are incompatible at the same
      location:** exercise every system where the player starts, and present an
      unbroken plane where the player starts. Nothing is wrong with either job.
      **A decision is owed on which one the spawn serves**, filed in
      ROADMAP_ITEMS.md, and the cheap answer is that the fixture keeps its job
      somewhere the camera does not open on.
    - **The mechanism is value continuity across one junction, and not a change
      of draw order.** The plane stays behind the world and still ends at the
      bottom of the window. What makes the player *in* it is that the world's
      surface has to read as the plane's near end rather than as a different
      object standing in front of it — the frame's one large junction that has
      never been tuned, because the world row sits at grade 1.00 and the plane's
      at 0.53, so today the near end of the plane is *darker* than the ground in
      front of it and the reference runs the other way. **This is why the fix is
      reachable at all**: the composition-order problem the entry below names is
      real, but it is soluble by value because the plane is already behind the
      world at the place it matters.
    - **It therefore consumes the deliberately-open TUNING question rather than
      sitting beside it**, and that has to be stated because the two were filed
      as independent. "Does the world row take a grade below the plane's?" was a
      look-and-see left open at 4b; it is now the load-bearing knob of a
      scheduled item. **It keeps its own row and its own playtest** — a grade on
      the world changes how the play area reads while digging, which is exactly
      the reason it was not settled inside 4b — but it stops being optional and
      stops being postponable.
    - **Reflection stays dropped and the replacement was written before the
      fact**, at entry 7: the lake is bright near the camera because it mirrors
      the sky, and land cannot, but a horizontal surface faces the sky where a
      vertical silhouette does not — the same brightening for a different
      reason. Land reflecting mountains would be the appearance without the
      mechanism, which is the deleted mid-ground band's mistake exactly.
    - **Changing the fixture scene invalidates both recorded sessions, and that
      is the consequence to state out loud before anyone redraws anything.**
      `src/game/input_log.h` names "the fixture scene changes" as the *first* of
      the three things that invalidate a log, and `tests/test_scene.cpp` pins
      `FIXTURE_SCENE_CELLS = 334901` specifically so this fails in `ctest`
      rather than in a benchmark nobody runs on the breaking commit. **So P4's
      replayed row — the only instrument in the project that proves the frame
      budget is intact on a real frame — goes dark until somebody plays and
      records again, and only the tester can do that.** The guard working as
      designed is the good news; the cost is real and it is a human's afternoon,
      not a build step. `tests/bench_grid.cpp` loads the fixture in two places
      and `rim_probe` in one, so those go quiet too until the new scene exists.
      **The scene is generated** (`generate_test_scene.py`), which is the one
      piece of cheap news: a new scene is a new generator, not pixel work.
    - **The refusal against a near foreground silhouette is untouched, and this
      item must not be cited as having bent it.** That refusal is about paint in
      front of the world occluding digging. Nothing here draws anything new in
      front of the world; if a later step reaches for that, it is a fresh
      argument and this entry does not supply it.

    *(Everything below is the entry as written while the decision was open, kept
    because the reading in it is what produced the answer.)* **This reopens a
    decision that was made deliberately the other way** — V19 is titled "the
    seven-band scene, with land where the reference has water" — so it is a
    direction change and not a defect, and it is not settled inside an item
    about something else. That rule already exists here for the world-grade
    question and applies identically.

    - **The mechanism we did not build is reflection.** The reference's plane
      shows every band standing above it a second time, mirrored and
      value-compressed. That is where its depth comes from. Of its three cues we
      built exactly one: the ripple dashes growing toward the viewer. **This is
      the same lesson as the deleted mid-ground band — ask what in the reference
      is doing the work.** Three sessions have now tried to buy that plane's
      recession with values, and values were never what it was made of.
    - **The second difference is occupancy, and it is the one the tester
      named.** The boat sits at roughly the plane's vertical midpoint with plane
      both above and below it. Ours is drawn *behind* the world with the player
      standing in front of it on terrain. **A band the player is in front of
      cannot recede around them**, which is a property of the composition order,
      not of the art.
    - **Three options are on the table and the cheapest is respectable.** (1)
      Leave it — ours is a digging game on solid ground and the plane stays a
      backdrop band. (2) Reflections only — keep the plane where it is and
      mirror the mountains into it; buys the missing cue without touching the
      world or the scene, and is reachable from the existing draw path. (3) The
      water plane — a new starting scene, the player on or beside water; reaches
      `src/scene/`, the world and possibly the fluid sim, and is much the
      largest.
    - **Option 3 is the one to price honestly before anyone agrees to it**,
      because the reference is a *painting of a boat on a lake* and ours would
      be a simulation with a diggable world in it. The refusal already recorded
      against a near foreground silhouette applies with the same force: a
      painted plane in front of the world occludes the one verb the game has.

- [ ] **V16 — The backdrop moves.** *Observed:* `backdrop_sky` and
  `backdrop_mountains` are two static textures whose only motion is the parallax
  offset, so **the backdrop's sole depth cue stops the moment the player does.**
  A wooded hillside at night is motionless: nothing drifts, nothing sways, no
  star varies. That is a thing in the built game that reads badly, which is
  precisely the question V8's remainder cannot answer and the reason this is a
  separate item from it rather than part of it. *Unlocks:* depth that survives
  the camera standing still, and the third of the four goals above.
    - **Built on V11's layer list and it is why that list comes first.** A layer
      gains an optional function of the step count — a scroll rate, a short
      frame sequence, a tint from V11's grading term — and stays one
      `SDL_RenderCopy`, or two for a layer that wraps. **No new coordinate
      system**, which is the constraint V8 already priced this whole layer
      against.
    - **The clock is the fixed simulation step and not the rendered frame**, for
      the third time in this document: V3.1's walk cycle ran at nearly 3x speed
      at 165 Hz, V10.1 is warned about the same thing, and a drifting cloud is
      the same shape. **It presents as an art problem in every one of the three
      cases**, which is what makes it worth writing down again rather than
      assuming it is known.
    - **A wrapping layer retires the parallax duplication rather than
      documenting it**, and that is a real second payoff. The seam at the pan
      limit exists because each layer's size has to be derived from the camera's
      pan range at that layer's factor — two constants in two languages with
      nothing enforcing agreement. A layer that tiles has no size relationship
      to the pan range at all. V11 generates the header because that is the
      afternoon available today; **this is the version where the failure mode
      cannot occur**, and the two are not redundant because the header is what
      keeps the existing hand-sized art correct in the meantime.
    - **The bound, stated because this is the easiest item here to let grow.**
      Motion in the backdrop is drawn, non-simulated, interacts with nothing,
      and is not V9 — [notes/art_direction.txt](notes/art_direction.txt)'s
      drawn/simulated split is the test, and a moving thing that a player could
      expect to interact with has failed it. Weather that lands on terrain is a
      simulation feature and is not this item.
    - *Verify.* The backdrop reads as alive with the player standing still —
      which is the observation, so it is the check; motion is identical at 60
      and 165 Hz, measured rather than assumed; and a missing backdrop BMP still
      degrades to the flat clear colour, since the `if` around each layer is
      what keeps a failed load from showing two-frames-ago garbage.

- [x] **V10 — The in-window UI layer: a dig reticle and a material hotbar.**
  *(done — see [src/ui/hotbar.cpp](src/ui/hotbar.cpp); screen shake and hit-stop
  are split out to **V10.1** below and are the only part still open)*
  *Observed:* [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 1, observations B1 and
  B2. *Unlocks:* the first player-facing UI in the project, and the reason it is
  here rather than in Presentation & Tooling is that everything in that section
  is dev-facing by its own preamble. A reticle the player aims with is not a
  debug convenience.
    - **Both halves came out of a playtest as feature requests and neither is
      taken at face value, because the stated fix and the real finding differ in
      both cases.** The reticle was asked for as four non-intersecting ticks;
      the finding underneath is that the current marker is a filled one-cell
      rect in the same orange family as Fire, so it vanishes against the one
      thing you most want to aim at. Contrast and shape are the requirement, and
      the tick design satisfies it. The hotbar was asked for because selecting
      materials felt slow — **and a measurable part of that was defect A2, a HUD
      that rebuilt once a second and showed a stale material for up to a full
      second after the key was pressed.** That is fixed, and it is a caution
      worth keeping: a chunk of this request was a rendering lag being read as
      an interaction problem. What remains is real, since eight materials behind
      eight number keys with no visible affordance still has to be memorised.
    - **Both halves of the UI are now built; what is left in this item is the
      two game-feel effects at the bottom of it.** The reticle shipped first, on
      its own. The hotbar is `src/ui/hotbar.h` / `.cpp`: a row of eight 8x8
      hand-authored icons, bottom centre, the selected slot framed in its
      material's own colour. **Ordered by `MoveKind` rather than by the order
      materials were added** — structural solids, powder, liquids, gases, eraser
      last — so neighbouring keys behave alike and a mis-hit lands near what was
      meant. That moved the eraser from `4` to `8`, which is the one binding in
      this project anyone had already learnt, and it is worth watching for in
      the next playtest rather than leaving to be discovered.
        - **The table moved into `src/ui/` and the keydown switch in `main.cpp`
          became a loop over it.** That was not tidying. Two lists — one saying
          key 5 places Oil, one drawing an oil icon over the fifth box — can
          disagree, and the failure is silent and worse than no hotbar at all,
          because the affordance is then actively lying. One table is read by
          both.
        - **Sized off `DisplayMode::ui_scale()`, which this entry predates.**
          Authored in fixed pixels it is a postage stamp at 3440x1440, which is
          the mode the art is measured against.
        - **One departure from "tinted from `MATERIALS`", stated rather than
          hidden:** Oil is `0xFF2C2620` and Charred is barely lighter, and both
          sit on a dark backing panel, so painted faithfully they are black
          squares — technically correct and functionally absent. A base below a
          brightness floor is lifted to it proportionally, keeping the hue.
          Every other row passes through untouched.
        - **Three shapes were changed after looking at them at real size rather
          than at the ASCII art**, which is the argument for authoring them as
          ASCII in the first place: the eraser's inner ring was in the shadow
          tone and vanished against the panel, Oil's sheen was two detached
          pixels that read as noise, and Oil's first silhouette was a mound that
          collided with Sand's.
    - **Built as** an icon atlas in `src/ui/`, laid out exactly like the bitmap
      font already there — hardcoded small bitmaps, tinted per material from
      `MATERIALS` so an icon can never drift from the palette it depicts. Not
      drawn with renderer primitives: an afternoon of fighting `SDL_Renderer`
      yields a worse pyramid than forty bytes of hand-set bits, and the tinting
      is what keeps this honest when V6 locks the palette.
    - **Sequenced after the simulation work rather than before it, for one
      concrete reason:** an icon depicts behaviour, and E9 changes what steam
      *does*. Authoring a steam plume against the current puff means authoring
      it twice.
    - **This does not do a menu, a settings screen, or a font beyond the one
      that exists.** Naming that here because a "UI layer" is the single easiest
      item in this document to let grow, and the bound is that everything in it
      is visible during play and costs no interaction to reach.
    - **Screen shake and hit-stop were scoped here and are split out as V10.1
      rather than carried as an unchecked bullet under a done item.** The two UI
      halves shipped and the two feel effects did not, and a `[ ]` box on a
      mostly-built item is exactly the state that made this document need a
      bookkeeping pass in the first place.

- [ ] **V10.1 — Screen shake and hit-stop.** *Observed:* split out of V10, which
  shipped its two UI halves without them. *Unlocks:* the impact half of E6 — an
  explosion that moves the camera reads as force, and the same effect applied to
  E3's fractures is what V9's dust is compensating for the absence of.
    - **They have a real trap.** Both are game feel rather than art, and the
      fixed timestep makes them easy to get wrong: a shake driven off the
      rendered frame and a simulation driven off the fixed step must not share a
      clock, or the effect changes with frame rate. That is the same class of
      bug F1 and F2.3 spent two sections retiring, and it would be reintroduced
      by a feature nobody thinks of as simulation. **V3.1 hit this exact trap
      and its entry is the worked example** — a walk cycle off the rendered
      frame ran at nearly 3x speed at 165 Hz and presented as an art problem.
    - **Sequenced with E6 rather than before it**, because a shake with nothing
      worth shaking for is tuned against the dig tool and then re-tuned against
      the first explosion.

### P — Performance

*Running order: **~~P2~~ → ~~P4~~ → P1 → P3** — `P4` was added on 2026-08-11 and
goes first of the remaining three because it is an instrument and the other two
are judged with it; `ROADMAP_ITEMS.md` has carried that order since, and this
line said `P2 → P1 → P3` until P4 shipped on 2026-08-13. It is not the numbering
— see the note on stable IDs above. **P2 is now the first item in the whole
plan, ahead of the E track and ahead of the session 5 playtest**, which is a
change from it merely being first within this section: it is half a day, and
every item in the new order carries a bracketed measurement that would otherwise
be quoted against a world a quarter the size of the one actually being played.
Taking it after E10 means measuring E10 twice. P1 is the layout change and wants
the field set settled, which means after E5a. P3 is new and is last because it
is the largest of the three and the only one that changes how the sweep is
structured.*

- [x] **P2 — Re-baseline the benchmark at the size the game actually runs.**
  *(done — 2026-08-10; table in [PERFORMANCE.md](PERFORMANCE.md))* `grid_bench`
  measured 960x540 and the game runs **1920x1080** — four times the cells, and
  nothing measured it. **The item inverted while nobody was looking:** 960x540
  was written down as the target the played size had yet to reach, and the Noita
  rescale took the world past it, so the benchmark became the *smaller* number
  and every budget in `PERFORMANCE.md` was quoted against a world a quarter the
  size of the real one. Both sizes now run; 960x540 stays as the historical
  series and as a control, with every scenario constant written to reproduce its
  old literal value exactly at that size.
    - **The stale comment was the cleanest evidence of how it inverted.** The
      bench justified its size as "1920x1080 at a 2px scale = 960x540 cells".
      `Camera::SCALE` is 4, not 2 — and since F3.1 decoupled world size from
      window size, `GRID_WIDTH`/`GRID_HEIGHT` are a cell count that no scale
      factor applies to at all. The arithmetic was true of a world that was the
      window, and it went on reading as a justification for two architectures
      after that world stopped existing.
    - **The finding is better than the item expected, and it is `sparse` at
      1.00x.** An ordinary gameplay frame — large static world, small patch of
      action — costs the *same* at 1920x1080 as at 960x540. `settled` is at the
      noise floor in both. **The engine pays for awake cells, not for cells**,
      so the size of the sleeping majority is free; there is no single "4x" to
      apply to a row, and the ratios run from 1.00x to 7.54x.
    - **Two scenarios are over budget at the played size and neither is a
      regression:** `churning` 211% and `cascading` 241%. That is what the
      played world has cost since the rescale; it had simply never been
      measured. Nothing got slower.
    - **Two measurement defects were found and fixed on the way**, both
      invisible while the bench ran a single size. The stepped scenarios kept
      their cadence in a function-scope `static int tick`, so a second run of
      the same scenario inherited the first's counter and was not the same
      scenario. And `shattering` needed a witness: fracture fires only on
      landing, a slab at 1920x1080 falls twice as far while the clear cycle
      stays at 60 steps, and "the slabs are wiped mid-flight and nothing lands"
      would have produced a confident number about a feature that ran zero times
      — which is exactly how E3's first measurement failed. `piece_tag` is
      counted untimed after the clock and reads 2,384 / 2,348, so it does land
      and does break.
    - **It also put a question mark over P1's grading criterion**, which is
      recorded in `PERFORMANCE.md` next to the prediction rather than here: that
      criterion rests on `churning` fitting in cache, which was observed at
      960x540 and cannot be assumed at four times the size.
- [x] **P4 — A benchmark scenario that is a real frame.** *(built and first
  session recorded 2026-08-13 — numbers and method in
  [PERFORMANCE.md](PERFORMANCE.md), how to record one in
  [README](README.md#the-replayed-row-and-recording-one-p4))* **This item
  retires an argument rather than a defect.** All seven scenarios in
  `grid_bench` are hand-built, and the plan has twice had to decide which of
  them counts as a realistic frame — most recently over `churning` at 211%,
  where "call it synthetic" was the convenient answer and sand sinking through
  water is the most ordinary thing a player does in a falling-sand game. **That
  argument is unwinnable by classification and does not have to be had:** F2.3
  made a run a seed plus a replayable list of inputs, so a row that *is* a
  played frame can simply be recorded.
    - **The design decision worth carrying: recording is always on, and `F9`
      saves.** The obvious build — `F9` starts recording — cannot work, and
      finding out why took one paragraph rather than an afternoon only because
      the fingerprint check was designed first. A log must begin at a world the
      replay can rebuild, and the only such world is the fixture scene before
      the first step. A recording started two minutes in would replay from the
      fixture into inputs that assume two minutes of dug tunnels, which is
      precisely the "silently measures nothing" failure the item exists to
      remove. Cost of always-on: one 24-byte `Input` per fixed step, ~1.4 MB an
      hour, capped at half an hour.
    - **The staleness trap is closed by construction rather than by a note in a
      file.** The log carries the seed, the fixture's placed-cell count and a
      fingerprint of every cell before the first step; the bench rebuilds the
      world and **refuses** on a mismatch. `scene_test` pins the fixture count
      too, so a changed scene fails in `ctest` on the commit that changed it
      rather than in a benchmark nobody runs that day. **A changed simulation is
      the third case and is deliberately not refused** — it moves the end state
      legitimately, the bench cannot distinguish it from a stale log, so it
      reports what it saw. An instrument that guesses which of two
      indistinguishable things happened is worse than one that says it cannot
      tell.
    - **It reports four statistics because a mean is the wrong one for a
      budget.** Mean, p99, worst step, and how many steps exceeded 16.67 ms. A
      session that sleeps through 95% of its steps and spends the rest at 40 ms
      stutters and has an excellent mean.
    - **It cost a refactor that was not in the estimate, and the refactor is the
      better half of the item.** The scene loader was 90 lines of `SDL_LoadBMP`
      inside `main.cpp` — so the world the game actually boots into could not be
      built by anything headless, which is why no test had ever loaded the
      shipped fixture and why V2's blank world went unnoticed for a commit. It
      is now `src/scene/bmp.cpp`; `main.cpp` calls it and its SDL path is
      deleted. **One reader, not a headless one beside the SDL one** — two
      implementations of one question is the shape of D1's two clocks and F6's
      two range tests, both of which drifted, and this is the first time that
      lesson has been applied *before* the drift. Checked by the count
      `README`'s launch step already pinned — 334,901 cells, unchanged,
      independently reproduced by `tools/pixel_art.py`'s reader.
    - ~~**What it owes: a played session, which the builder cannot supply.**~~
      **Supplied 2026-08-13.** 24,437 steps, 407 s of play: **mean 0.1212
      ms/step, p99 1.4745 ms, worst step 4.8193 ms, 0 steps over budget**,
      replayed to the recorded end state exactly. Both questions closed — see
      `ROADMAP_ITEMS.md`'s decisions table and `PERFORMANCE.md`.
    - **The number corrected the rule this item was built to serve, which is the
      outcome worth carrying.** The frame-budget rule was to be stated against
      this row *instead of* the synthetic ones. But a played session costs 0.12
      ms a step, so "under 10% on the replayed row" is twelve microseconds —
      under the noise floor, on a row with three times the budget spare. **A row
      that is realistic enough to settle a budget is, for that same reason, too
      quiet to price a per-cell change.** Both kinds of row are kept and given
      separate jobs. The item still succeeded at what it was for: the argument
      about which hand-built scenario counts as realistic is over, and
      `churning` turned out to be neither a defect nor an artifact but a real
      activity type at an intensity nobody sustained — 360 of 510 chunks awake
      against the session's 129.
    - **Then its own census overturned the reading of its first session, within
      the hour, and this is the part to carry.** The row was justified — here
      and in `ROADMAP_ITEMS.md` and `PERFORMANCE.md` — on the argument that a
      *played* frame is realistic **by construction**, which is true and which
      was silently treated as meaning representative. A census was added to the
      row (inputs counted exactly, world sampled once a second in a second
      untimed pass) and reported that the session **never dug once, never moved
      a grain of sand or a cell of water, never contained steam, and peaked at
      16 of 510 chunks awake**. It is a painting session. **Realistic by
      construction; representative only by evidence** — two properties this
      item's entire argument ran together, and no amount of "but it was really
      played" would ever have separated them. Only counting what was in it did.
      The budget rule stands (16 of 510 awake makes the case that a played row
      cannot price a per-cell change *stronger*); the `churning` question
      reopens; a second session is owed.
    - **And a result nobody asked it for: 24,437 steps of real play replay
      byte-exact.** `test_run.cpp` proved that for short synthetic sequences;
      this is seven minutes of a person digging, pouring and walking, rebuilt
      from a seed and an input list onto the same world fingerprint — the first
      end-to-end evidence that F5's and F6's fixed-point conversions hold over a
      long run rather than over a test.

- [ ] **P1 — Split the cell array hot from cold.**
  [PERFORMANCE.md](PERFORMANCE.md) establishes that `cascading` — the stated
  number to watch — is **bandwidth-bound**, and `ENGINEERING_NOTES.md` prices a
  12→2 byte cell at roughly 6x the hot loop's memory traffic while rejecting it
  outright as mutually exclusive with authored per-cell colour. **Splitting the
  array captures most of that win without giving up authored colour, which is
  the reason to reach for this one first:** `type` / `updated_tag` /
  `temperature` / `piece_tag` are read for nearly every awake cell every step,
  `color` is touched only when a cell is written and when the frame is uploaded.
  Two documents call that trade impossible; this is the thing that makes it not.
  Measured bracketed (on, off, on) with `churning` as the control, per the rules
  in that file — and **recorded honestly if it does not pay**, because both of
  the previous entries filed as cheap wins failed and were left on the record
  rather than deleted.
    - **This item has gained consumers since it was written, and they are the
      reason it is sequenced after E5a rather than before.** `element.h` records
      that E3 spent the last free byte: the struct is 12 with nothing spare, so
      the next field costs 500 KB at the target resolution and a wider stride.
      **The field set this has to be settled against changed on 2026-08-09 and
      it changed in P1's favour.** E10 and E5a add no field at all — ~~they
      claim `Element::ticks`, which is already in the struct and unused for
      non-structural, non-`Fire` cells~~ **they add three fields into the
      alignment hole between `type` and `color`, which the sitting on 2026-08-13
      found had been unused since the struct existed** *(the `ticks` plan is
      withdrawn; see E5a and `element.h`)* — so `Element` stays at 12 bytes
      either way and the hot/cold split is against the same twelve it was always
      going to be. **The split does change, though: the three new bytes are
      hot** — a velocity is read on every awake cell — so E5a moves the hot side
      from eight bytes to eleven against `color`'s four, which makes the ratio
      P1 is trying to improve *worse* rather than leaving it alone. That is an
      argument for P1, not against E5a, and it is the sharpest version of "the
      layout wants to be settled against the final field set" this entry has
      had. What the layout must still accommodate is `temperature`, V7's
      downsampled light buffer, and eventually E5b's air field, both of which
      are separate low-resolution grids rather than anything in the cell array.
      **The "sparse particle list" this bullet used to name no longer exists** —
      that was the old E5 design and P1 was waiting on a structure that is not
      being built.
    - **The prediction is already written down in
      [PERFORMANCE.md](PERFORMANCE.md) so it can be graded rather than
      reinterpreted afterwards**, including the part that matters more than the
      win: if `churning` improves as much as `cascading`, the explanation is
      wrong even if the numbers are good.

- [ ] **P3 — Run the chunks in parallel.** *Observed:* the simulation is
  single-threaded — there is no `<thread>`, no task system and no parallel loop
  anywhere in `src/`, and every measurement in [PERFORMANCE.md](PERFORMANCE.md)
  is one core's. *Unlocks:* the headroom that E5b in particular will want, and
  it is worth naming next to P1 for scale: **P1 buys tens of percent and this
  buys a multiple.**
    - **The reference approach fits what is already built, which is the whole
      reason this is worth scheduling rather than dismissing.** Noita updates
      chunks in four alternating passes, arranged so that two chunks being
      updated simultaneously are never neighbours. That is what makes an edge
      write safe without a lock — a cell can only ever write into a chunk that
      is not currently being touched — and it is why the pattern preserves
      reproducibility: the pass order is fixed, so the same seed and input still
      produce the same world. **The 64×64 chunks, the per-chunk dirty rects and
      the "writes go through `swap_elements`" discipline are already most of the
      prerequisite**, and none of them was built for this.
    - **Determinism is the thing to prove rather than assume, and F1 is what
      makes proving it cheap.** Randomness is a stateless hash of (position,
      tick, seed, purpose) with no generator state, so nothing about thread
      scheduling can perturb a draw — which is a property F1.3 bought for a
      different reason and which turns out to be the precondition for this. The
      test is the one that already exists: same seed, same inputs,
      byte-identical world, now run at every thread count the build supports.
    - **Two things that will not survive the change unexamined:** the
      row-alternating sweep direction keyed on `y`, and `frame_tag`, which is a
      single grid-wide counter written by every visited cell. Both are correct
      single-threaded and both need to be re-derived per pass rather than
      ported.
    - **Sequenced last of the three.** It restructures the sweep, so doing it
      before P1 settles the layout means restructuring twice, and doing it
      before the played-size baseline exists means optimising against the wrong
      number — which is the mistake P2 exists to stop.

---

## 🟡 Medium Term (Core Gameplay Loop)
*Tying the physics engine into the lore. This section is the v0.1 slice.*

*This list is longer than it used to be, and every addition came from an audit
rather than from an idea. Three items were previously absent even though the
Definition of Done depends on all three — a camera, a save file, and a way to
die. A fourth, the UI decision, was sitting in `ENGINEERING_NOTES.md` as a
warning with nothing scheduled to act on it. None of those were deferred
decisions; they were gaps, and a plan that hides its prerequisites is not a
plan. Assume the slice is at least twice the work the old five-line version
implied. Two of the additions have since moved out again, upward rather than
away: the camera is F3 and the level loader is F4, because neither was ever
slice content — they are what slice content stands on.*

***Engine & Visual Depth sits ahead of this section with one exception, and the
exception is new.** `S0` below is pulled out of this section and scheduled
between E4 and E10, for the reasons written at that item. Everything else here
still sits behind the engine tier, and the paragraph below is the argument for
that and is unchanged — it is worth reading with S0 in mind rather than replaced
by it, because the case for engine depth first was always about *depth*, never
about the slice having to be last.*

***The original argument, which still holds for the other seven items:** this is
a deliberate delay to the slice rather than an accident of ordering.* The engine
and how it looks are the selling point, so they are worth depth before the loop
is built on top of them — the argument in full is in that section's preamble and
in `VISION.md`'s Scope Discipline. Three consequences for the items below.
**Quantum Worlds and Objective + Extraction get a better engine to be built
against** — heat, levelling liquids, fracturing structures, thrown debris and
explosions are all things a level would otherwise have to be designed around the
absence of. **Player health has one of its two damage sources waiting for it**:
"what damages the player" is answered for free by E2, since fire that has a
temperature is fire the body can be asked to read, exactly the way collision
already asks what it is standing in. And **E6 makes the hook question answerable
by playing** rather than by argument — see that item, and see the two open
questions immediately below, which are the ones this section still owes an
answer to.*

**Two open questions this section has not answered, recorded here rather than
left to be discovered at the playtest gate.**

- **What the hook is.** `VISION.md` leaves this open on purpose and is explicit
  about why: naming it early would commit the design to whichever comparison got
  written down, and the Definition of Done currently cashes out "physics-based
  movement and destruction" as walking, jumping and one dig tool on a cooldown.
  That document is equally explicit that under-building looks exactly like
  discipline right up until the playtest gate. **E6 is the first thing in the
  plan that makes this a question you can answer by playing**, which is the only
  way this project has ever settled a design question. It still has to be
  *asked*, before the gate rather than at it.
- **Whether there is combat, and it is currently neither in nor out.**
  `notes/core_features.txt` names "physics-based combat and exploration" as a
  key mechanic. Nothing in this document schedules any combat, the Definition of
  Done does not mention it, and no item names its absence. That is the worst of
  the three available states: either combat is in the slice, in which case it is
  an item here and the slice grows by a real amount, or it is deliberately out
  of v0.1, in which case that belongs in writing next to the note's line.
  Deciding it costs a paragraph; discovering it costs a milestone.

**Both of those questions now have a due date rather than a gate, and `S0` is
why.** They were each written as "answerable by E6" and then left with nothing
scheduled to force the answer, which is how an open loop becomes a permanent
one. [ROADMAP_ITEMS.md](ROADMAP_ITEMS.md) carries them in a Decisions table with
a deadline against each: combat at the end of S0, the hook at the end of E6. A
decision with no deadline never closes, and this section has been carrying two
of them plus E4's for long enough to be evidence of that.

- [x] **S0 — The run can be lost.** *(new 2026-08-09, and it is the item that
  changed the running order; **built 2026-08-14**)* *Observed:* fire is fully
  simulated and cannot hurt the player, there is no failure state, and there is
  nothing in the built game to do. *Unlocks:* the two questions immediately
  above, by playing rather than by argument — which is the only way this project
  has ever settled a design question, and which is currently blocking every item
  in this section.

    **What shipped, against what the entry below promised: all of it, and
    nothing else.** Health on `Player` (`MAX_HEALTH`, `health()`, `is_alive()`);
    damage from contact heat and from landing too fast; one objective placed in
    the fixture scene by `main.cpp`; reaching it wins and dying loses, both
    through `Run::reset(seed)`; an `HP` readout on the HUD that already existed.
    The refusal list held — no generator, no save file, no second damage source,
    no death presentation, no UI beyond the readout. Tests are in
    `test_player.cpp` (the two damage rules, both sides of each) and
    `test_run.cpp` (outcomes, the objective, reset). **Both recorded sessions
    still replay byte for byte**, which is the claim that matters most: S0 reads
    the grid and the body and writes neither, so the simulation is untouched.

    **Five things the entry did not predict, and the first is the one worth
    carrying.**

    - **The spawn drop is a fall, and it was the hardest fall in the game.**
      `Run` puts the body a quarter of the world's height up in open air on
      purpose, and priced by the rule as written that is terminal velocity — 80
      of 100 health — before the player has touched a key. Nothing in this item,
      in the full "Player health and death" item, or in `player.h`'s spawn
      comment saw it coming, and it is obvious the moment it is stated. The fix
      is one bool on `Player` (`has_landed`): the first landing of a run is
      free, spent there rather than by moving the spawn, which `Run` cannot
      place on terrain it does not know about. **The generalisable form: a
      damage rule prices everything the world already does to the body,
      including the things it does before the run starts.** Every later hazard
      inherits this — E6's explosions, S1's contact damage — and each will have
      its own version of "something that was free is now charged".
    - **The obvious landing test does not detect landings, and this was measured
      rather than reasoned.** The natural way to write fall damage is to read
      `move_y` reporting a block. A body falling six cells a step onto a floor
      exactly six cells below walks the whole distance unblocked, ends the step
      flush on the ground with its velocity intact, and has `vel_y` quietly
      zeroed by the resting rule on the *next* step — so the landing never
      happens and the damage is zero. The correct edge is `on_ground` going
      false→true. **Written the obvious way, terminal-velocity falls do no
      damage whenever the arithmetic comes out even**, and the test that catches
      it was run against both forms: 100 health against the naive version, 20
      against the correct one. That is the same shape as D1's two clocks and
      F6's two range tests — a quantity with two plausible spellings that agree
      almost always.
    - **A restart would have silently invalidated P4's session log, and the fix
      is that a restart starts a new one.** A log replays by rebuilding the
      world from the seed and the scene and feeding the inputs back; a log
      spanning a `reset()` would replay into a world two minutes of play deep,
      and `grid_bench` cannot tell that from a stale log — which is precisely
      the "silently measures nothing" failure P4 exists to remove. **Why the new
      log is valid is asserted rather than argued**: reset on the same seed plus
      a re-stamp of the same scene reproduces the starting world exactly,
      checked on the fingerprint in `run_test`. The alternative — giving `Run`
      the scene so `reset()` could restore it — is the better answer when saves
      arrive and is recorded in `ENGINEERING_NOTES.md` rather than built here.
    - **The objective is a point in the world, not a cell**, and that is a limit
      rather than a shortcut. A cell means a row in `MATERIALS`, which means
      answering what happens when it is dug, burnt, displaced or buried — four
      questions the full item below exists to answer and which a spike has no
      business deciding by accident.
    - **It is ~740 cells from the spawn and the viewport is 480 cells wide, so
      it starts off-screen.** Without something pointing at it the run is "walk
      east until you find it", which is not a difficulty, it is a missing
      instrument — the same failure the free camera keeps being scheduled
      behind. Answered inside the item's own limit: a bearing (`GOAL:740E`)
      appended to the readout that already exists, not an arrow at the screen
      edge.

    **Two things it did on purpose that are worth reading as design and not as
    detail.** The objective sits east of F4's water channel, which is walled on
    both sides and full to within 175 cells of the top — **a traverse the
    character cannot walk**. Flight has been shipped since before this item and
    nothing in the built game had ever asked for it; this is what asks. And
    **death is checked before the objective**, so a body that reaches the marker
    on the step its last health goes has lost. That is an arbitrary call between
    two things that cannot both be true, written down at `Run::step` so it is a
    decision rather than an accident of ordering.

    **Two exceptions to "reset gives you a fresh run" now exist, and the second
    is this item's.** `Grid::vent_radius` survives because it is configuration;
    `Run`'s objective survives because it is a property of the *level*, and the
    caller re-stamps the same scene on reset — a cleared objective is one the
    caller has to remember to place again, and forgetting produces a run that is
    unwinnable and says nothing. Both are asserted in tests rather than left as
    prose.

    **What it could not answer, and this is the item's whole point rather than a
    shortfall.** The combat decision is due at the end of S0 and the hook at the
    end of E6; both are answered by playing and neither can be settled from the
    desk. **The playtest is owed** — the checklist gained a step 10 for it.
    Performance: this sitting could not price the change and says so, because
    every synthetic row in `grid_bench` — including the ones that never
    construct a `Player` — ran 2.2–2.9x above `PERFORMANCE.md`'s table, and the
    replayed row's 2.5x sits inside that band. What the sitting *does* establish
    is that the budget is intact at 0 of 20,415 steps over it even on a machine
    running two and a half times slow. **No number from it goes in
    `PERFORMANCE.md`.**
    - **It is the thin half of "Player health and death" and "Objective +
      Extraction", pulled forward and built now.** Health; damage from fire,
      whose temperature the engine already supplies, and from landing too fast,
      whose speed it already supplies; one hard-coded objective placed in the
      test scene; reaching it ends the run as a win and dying ends it as a loss,
      through `Run::reset(seed)` and not a second code path. A health readout on
      the HUD that already exists. **That is the whole of it.**
    - **What it explicitly is not:** no generator, no save file, no pet agent,
      no economy, no second damage source, no death presentation, no UI beyond
      the readout. Those stay in the full items below and are not started here.
      **The discipline that makes a spike safe is that it names what it is not
      building**, and this bullet is that.
    - **Why it is admitted ahead of most of the engine tier, stated against
      `VISION.md` rather than around it.** That document's Scope Discipline
      guards one direction — building the wrong things — and says outright that
      it does *not* defend against the slice being too thin to be fun, that
      under-building looks exactly like discipline until the playtest gate, and
      that this is the one place the cutting reflex is worth checking.
      Everything shipped to date is engine or visual foundation. This is that
      check, taken at a cost of about a week, and it is the cheapest available
      answer to "the game feels underdeveloped and without direction" — which is
      an observation about the built game and therefore the same class of
      evidence the E track admits items on.
    - **Keep the direction of the dependency.** This is the first gameplay
      system spanning both sides of the rule `tool.cpp` established: the grid
      does not know about bodies, bodies read the grid. Damage is the player
      *asking* what it is standing in, the same way collision does. It is not a
      health field on `Grid` and not a damage column on `Element`, and it will
      be tempting to make it one.

- [ ] **S1 — The enemy that granulates.** *(new 2026-08-11; the design is
  [notes/granulating_enemies.md](notes/granulating_enemies.md) Part B, path
  E-C)* *Observed:* **nothing, and that is stated rather than worked around.**
  There is no combat, so nothing in the built game is wrong for want of an enemy
  in the sense the E track means. **This item is blocked on the combat decision,
  which is due at the end of `S0`** — and the note it comes from says exactly
  that: the enemy *"becomes admissible when the Definition of Done's objective
  loop needs something to oppose the player."* *Unlocks:* the simulation being
  the combat system rather than a backdrop to one, which is the same argument
  `VISION.md` makes for the engine being the product.
    - **Built as E-C: a body, plus a `W×H` byte mask in body space.** Roughly
      364 bytes for a player-sized enemy, **per enemy and not per cell**, so
      `element.h`'s no-spare-byte constraint is untouched. Alive, the sprite
      draws *through* the mask and damage clears bits while spawning real `Grit`
      cells into the grid at those world positions — the sand pouring off a
      wounded enemy is genuine simulated powder that piles, gets wet and burns.
      Dead, the body deletes itself and writes its remaining mask into the grid
      as real `Crust`. **From that instant it is terrain, and E3 collapses the
      corpse for free.**
    - **This item is the consumer that reopens E4, and that is the most
      important thing on this entry.** E4 — does a body displace material — was
      closed **"no"** on 2026-08-10 by session 5, and the note this design comes
      from predicted the exact failure that decision now allows: *"the grit
      falls through the enemy that is producing it. The effect fails precisely
      at the moment it exists to be looked at."* **E4's "no" was decided on
      evidence gathered when nothing in the game depended on the answer**, and
      `ENGINEERING_NOTES.md` already says to re-ask at E5a. This is a second and
      stronger reason to re-ask, with a named consumer instead of a schedule
      position, and it is carried in [Decisions
      owed](ROADMAP_ITEMS.md#-decisions-owed) rather than left inside this
      bullet.
    - **Three things the note names that will be met on the first day**, kept
      here because each is cheap to design for and expensive to retrofit. *The
      mask is in body space, not sprite space* — a hole punched in the idle
      frame lands somewhere meaningless on walk frame 4, so it aligns to the
      collision box and every frame draws through it. *Damage location has to be
      real* — a hit that clears bits at the sprite's centre regardless of where
      you struck collapses the whole effect, and `Tool::aim_point`/`march`
      already resolve a world-space impact point to convert. *The corpse is
      gameplay, not decoration* — it can bury the player, block a tunnel or
      smother a fire, which is probably excellent and is a design consequence to
      accept deliberately.
    - **Its one new renderer requirement is a render target, and it needs no
      shader.** Drawing a sprite with holes in it is a per-enemy
      `SDL_TEXTUREACCESS_TARGET` texture — 14x26 at 1x — rebuilt only when the
      mask changes, which is on damage rather than per frame. **Nothing in this
      project has ever used a render target**, so it is worth naming as the one
      capability this item adds rather than discovering it mid-build. It depends
      on **V12** for a real alpha channel to punch, since a colour key cannot
      express a hole that is not exactly one colour.
    - **It depends on V15 for the reason V15 is admitted at all**, and the two
      should be read together: a rig is written once and re-posed, a sheet is
      drawn per character. An enemy built against a second hand-drawn sheet buys
      the animation twice and forfeits the amortisation that is V15's whole
      argument.
    - **What it is not:** not a generic actor framework. `ENGINEERING_NOTES.md`
      refuses an entity/component system on the grounds that *"there is one
      body, and there will be perhaps four things"* — **a second concrete body
      type is inside that budget and a system for arbitrary body types is the
      thing being refused**, and the difference is the whole of what keeps this
      item from becoming an engine rewrite.

- [x] **Pick the UI layer, in writing, before anything needs one.** Decided and
  recorded in `ENGINEERING_NOTES.md`: immediate-mode, drawn directly against
  `SDL_Renderer`, no new dependency — a library (Dear ImGui, Nuklear) loses for
  the same reason PNG lost to BMP in F4, and hand-rolled retained-mode loses for
  the same reason an ECS is off the table, both recorded in full there rather
  than repeated here. The one gap immediate-mode against `SDL_Renderer` opens on
  its own is text, since the renderer draws no glyphs — closed with a small
  hand-authored 3x5 bitmap font (`src/ui/text.h`/`text.cpp`, SDL-side and
  outside `ENGINE_SOURCES` like `main.cpp`), covering A-Z, 0-9 and a handful of
  punctuation rather than the whole ASCII table. *Verify, done exactly as
  written:* the frame-rate/brush/awake-chunk readout that the window title bar
  carried alone now draws inside the game window itself with the new font, over
  a translucent backing rect for legibility against whatever the simulation is
  doing underneath — the window title returns to being a plain, static label.
  This is the drawing primitive the health readout, pet agent panel and
  resolution options will each build their own screen on; none of those three
  screens exist yet.
- [ ] **Player health and death — the full version.** *(the thin half is `S0`
  above and **shipped 2026-08-14**; what is left here is everything S0
  explicitly does not build)* A real damage model rather than two hard-coded
  sources, more sources as the engine supplies them (E6 adds a third), and a
  death that is presented rather than merely applied. The dependency rule and
  the "not a field on `Grid`" constraint are stated at S0, held through the
  build, and hold for both halves.
    - **What S0 leaves this item, now that the thin half is real rather than
      planned.** The two sources are an `if` each in `Player::update` and that
      is correct at two; a third wants a table, and the moment to build one is
      the moment E6 supplies it rather than now. **The spawn-drop finding at S0
      is the thing to re-read before adding any source** — every new hazard has
      its own version of "something the world already did to the body for free
      is now charged", and the current one is spent on a single bool.
    - **`damage_this_step()` exists and nothing reads it.** It is the event half
      of the health number, built for the same reason `DigTool` reports the step
      a blow landed, and it is what a hit reaction, a sound or a screen shake
      each want. Kept because it costs a forwarded int; "presented rather than
      merely applied" is the item that spends it.
- [ ] **Playtest gate:** Put the slice in front of people who did not build it.
  Do not proceed past this line on the strength of your own opinion. Two things
  make this cheaper than it sounds and both are earned above: F1 plus F2.3 mean
  a bug report is a seed and an input log rather than a description, and the two
  open questions at the top of this section should have been asked *before*
  getting here rather than answered *by* getting here.

## 🔵 Presentation & Tooling (after the slice, before polish)
*Not slice-blocking — none of this makes the loop fun, and none of it should be
started while Engine & Visual Depth or Medium Term is open. Recorded here rather
than in the Long Term wish list (`VISION.md`) because these are concrete,
bounded, and genuinely expected to get built, unlike that list.*

**This section's rule has now been tested three times and has not bent.** The
scene loader left for F4, the art-pipeline visual work left for V1–V4, and V5–V9
were written directly into Engine & Visual Depth rather than here. In every case
the *item moved or was filed elsewhere* — nothing was ever built while still
sitting behind "do not start this yet". That is the only version of this rule
that survives contact with a schedule: a rule with one exception has no force
left, so the answer to "this needs to happen sooner" is to argue the item into a
different section, in writing, or to leave it alone.

### Sandbox / debug tooling
*Dev-facing, not player-facing — these speed up iterating on the physics engine
itself rather than polishing what ships. Each is small and self-contained in
`main.cpp`/`grid.h`, so the risk of scope creep is low, but they still wait
behind the sections above like everything else here.*

**One of these has stopped being purely a convenience and is worth promoting
when the time comes.** V2 could not be verified in the running window at all —
the startup camera sits below the F4 scene so only sky is visible, and synthetic
input never reached the SDL window, with the HUD's brush indicator never leaving
`SAND` as proof it was being dropped rather than mis-aimed. The palette was
signed off on a swatch sheet instead. **Every remaining V item has the same
problem**, and it is worse for them: a sprite, a backdrop and a lighting pass
cannot be judged on a static sheet at all. Whatever the answer is — a free
camera, a debug scene spawner, or a way to drive the window that actually works
— it is a prerequisite for verifying the V track rather than a nicety, and it
should be pulled forward the first time a V item cannot be checked.

**That trigger has now fired, and this note is the written argument the rule
above demands.** [PLAYTEST_LOG.md](PLAYTEST_LOG.md) session 1 was the first time
the manual checklist was actually run, and it returned eight defects against a
suite that was 6/6 green on 199 checks. Five of the eight are rendering or feel.
**A1 is the one that settles this**: the player rectangle jitters because the
renderer reads a truncated integer position and discards the sub-cell remainder,
and *no headless test can see that* — the simulation was correct the entire
time, collision included. It is the first defect in this project that was
invisible to the suites by construction rather than by omission, which is
exactly the category this item exists to cover, and the other six are only
reachable by tests nobody had written.

**The item therefore moves rather than being started here** — a free camera and
a way to drive the window are now listed as a prerequisite in the V track's own
running order, which is where the work that needs them lives. Left in this
section it would have to be built while this section says not to build it, and a
rule with one exception has no force left. **What has not changed:** it is still
dev-facing, so it is not V10, and it does not become slice-blocking by being
promoted. It becomes the thing V5's successors are checked *with*.

- [x] **World reset hotkey.** *(shipped 2026-08-14 as `T1.4`, on `Ctrl`+`R`
  rather than `R` — see [T1](ROADMAP_ARCHIVE.md#t1-the-debug-tooling-batch))* Wipes the grid back
  to Empty and respawns the player, so a messy test scene can be cleared without
  relaunching the exe. `Grid::reset(seed)` and `Run::reset(seed)` are built and
  tested in **F2.2**, so by the time this item comes up it is the key binding
  and a seed to pass — the current world's own seed keeps a debugging session
  reproducible; a freshly drawn one is the other reasonable choice, and this is
  where that gets decided.
- [x] **Pause and single-step (`P` to toggle, `.` to step once while paused).**
  *(shipped 2026-08-14 as `T1.1`; the brush-while-paused question is answered by
  construction and the third state is still open — see
  [T1](ROADMAP_ARCHIVE.md#t1-the-debug-tooling-batch))* Freezes the fixed-step loop so a collapse
  or a reaction can be inspected one frame at a time instead of flying past. The
  one thing to get right: `accumulator` must not keep growing while paused, or
  unpausing dumps a burst of queued steps at once. F2.4 confirmed the shape and
  left one policy question open on purpose: whether the brush still paints while
  paused, since a pause that also freezes editing forecloses using pause *to*
  set up a scene precisely.
- [x] **A free camera, or some other way to look at the world.** *(shipped
  2026-08-14 as `T1.2`, on `F` — see [T1](ROADMAP_ARCHIVE.md#t1-the-debug-tooling-batch))* Not in
  this list before, and it is the item the note above is about. The camera
  follows the player and nothing else, so anything the player is not standing
  next to is unverifiable by eye.
- [ ] **Continuous brush strokes.** The brush is stamped once per fixed step at
  the raw mouse position, so a fast drag leaves gaps in the line instead of a
  solid stroke. Fix is to track the last painted grid position and stamp the
  brush along the segment to the current one (Bresenham), not just at the
  endpoint — which also gives straight-line drawing for free on a deliberate
  fast drag.
- [ ] **Brush outline preview.** Draw a hollow ring at `brush_size` under the
  cursor at all times, not just while right-click is held, so the brush
  footprint is visible before paint is committed. Reuses the existing circle
  math from the paint loop, just as an outline test instead of a fill.
- [x] **Cell inspector.** *(shipped 2026-08-14 as `T1.3`, on `I`; it found the
  false claim on `Grid::active_chunk_count()` the first time it was used — see
  [T1](ROADMAP_ARCHIVE.md#t1-the-debug-tooling-batch))* Extend the in-window HUD with the material
  name — and now the temperature and piece tag — under the cursor via
  `grid.get_element(gridX, gridY)`. Cheap, and E2 and E3 both added per-cell
  state that is currently invisible and was debugged without it.

### Window and display

- [ ] **Display modes: fullscreen, borderless, windowed.** Industry-standard
  behaviour, which is more than three `SDL_SetWindowFullscreen` calls: exclusive
  fullscreen vs. borderless-windowed as separate options, alt-tab that doesn't
  corrupt the window state, the choice persisted between launches, and correct
  behaviour when the mode is changed mid-run rather than only at startup.
- [ ] **Resolution options, including 1920x1080, 2560x1440 and 3440x1440.**
  **The hard part of this item is already answered and should not be
  re-litigated here.** It used to read as an open modelling question, because
  the simulation grid was derived from the window size (`GRID_WIDTH =
  WINDOW_WIDTH / PIXEL_SCALE`) and a bigger window therefore meant a bigger,
  more expensive world rather than a bigger view of the same one. F3.1 separated
  the two constants, F3.3 sized the upload to the viewport rather than the
  world, and **F3.5 settled the model in writing**: world size and window size
  are independent in both directions, so growing the viewport reveals more of a
  world that was always fully simulated and never changes how much of it runs.
  F4.3 then made that concrete and the rescale went further — the world is
  1920x1080 against viewports of 480x270 to 860x360, so the camera genuinely
  pans today. What is left here is the window plumbing itself: the mode list,
  persisting the choice, and applying it mid-run rather than only at startup.
  Ultrawide keeps one open question that is *not* rendering — a wider viewport
  shows more of the world, which is a gameplay-fairness call rather than a
  display setting, and it is the one thing in this item still worth deciding
  rather than implementing.

### Art

- [ ] **Custom pixel art and animation generator for game assets.** A tool for
  authoring sprites and animations that fit the engine's palette and cell scale,
  so art is produced in-project instead of hand-drawn in an external editor and
  re-exported. Worth being blunt about the risk: this is a second application,
  with its own UI, file format and edit loop, and it is exactly the kind of
  seductive side-build `VISION.md`'s Scope Discipline section warns about. It
  only earns its keep once there is enough art volume that authoring by hand is
  measurably the bottleneck. Until then, an external editor plus a small import
  step is the cheaper answer. **V6 changes one thing about it and does not
  change the verdict:** a locked palette is exactly the constraint such a tool
  would exist to enforce, so if this is ever built, V6's validator is the half
  of it that already exists.
- **Making an authored scene cohere — moved out, nothing left here.** Both
  halves of the old art-pipeline plan have left this section: phases 0-2 became
  **F4** in Foundations, because the loader was a hard prerequisite for Quantum
  Worlds sitting behind this section's "do not start yet" rule; phases 3-6
  became **V1–V4** in Engine & Visual Depth, because the visual design became a
  stated selling point rather than a finish applied at the end. **V5–V9 were
  then written directly into that section and never passed through here at
  all**, which is the same rule holding in the easier direction. What remains
  genuinely deferred from that note is the editor above.

### Shipping
*`VISION.md`'s first Project Goal says "production-level" and none of this
existed anywhere in the document. Not slice-blocking — but "production-grade" is
a claim these five items are the evidence for, and a plan that omits them is
quietly redefining the goal.*

- [ ] **Packaging and a release build.** There is no answer today to "how does
  someone who is not you run this". SDL is already statically linked, which
  removes the DLL problem, so this is mostly a named artifact, a version string,
  and a decision about what a release actually consists of.
- [ ] **Build on macOS and Linux at least once.** `ENGINEERING_NOTES.md` has
  claimed cross-platform support for several revisions on the strength of using
  no platform-specific code. That is a reasonable expectation, not a verified
  fact, and the gap between the two is exactly the kind of thing this document
  is otherwise good about naming. CI would settle it permanently and is worth
  considering at the same time, since the whole build is CMake and FetchContent.
- [ ] **Crash diagnosis: assertions and a log.** Nothing in the project logs
  anything and nothing owns "what do we do with a crash we cannot reproduce" —
  there is already one unexplained `0xC0000409` (stack buffer overrun) from a
  prior session, seen twice under heavy load and never reproduced. **The payoff
  here is bought by F1 and F2.3 and should be spent on purpose:** once a run is
  a seed plus an input log, a crash report is a file that reproduces the crash
  rather than a description of it. That is also precisely what the playtest gate
  needs.
- [ ] **Audio.** No sound of any kind, mentioned in this document only as a
  clause inside the UI note. Correctly deferred — it makes nothing about the
  loop more or less fun to *test* — but a pixel physics game with no audio is
  not a production-grade game, and it should be a line item rather than a
  parenthetical. **E6 raises the stakes on this rather than changing the
  order:** an explosion is the one thing in the plan that is actively worse
  silent than absent.
- [ ] **Settle on one project name.** The README says "Toop / Xoco (working
  title)", the window title says "SLOP Pixel Physics", and the repo is
  `xoco-game`. Three names for one thing. Trivial to fix, and it stops being
  trivial the moment anything is published under one of them.

---

