---
paths:
  - "**/ROADMAP.md"
  - "**/ROADMAP_ARCHIVE.md"
  - "**/PLAYTEST_LOG.md"
  - "**/TUNING.md"
  - "**/PERFORMANCE.md"
  - "**/ENGINEERING_NOTES.md"
  - "**/VISION.md"
  - "**/README.md"
  - "**/ASSETS.md"
  - "**/notes/**"
---

# Writing in this project's documents

Loaded when editing the project docs. `CLAUDE.md` has the routing table for
*which* file; this is how to write in each one without breaking a convention that
was learned the hard way.

## Conventions that apply everywhere

**Point at code by name — a function, or a line quoted out of it — never by line
number.** One roadmap step shifted `grid.cpp` by five lines, `grid.h` by
seventeen and `main.cpp` by fourteen, silently falsifying every numeric reference
in the document *including the one describing the bug that step had just fixed*.
A plan read across many sessions cannot use an anchor that moves every time the
plan is executed. (`TUNING.md` is the deliberate exception: it cites line numbers
because its job is to find a constant fast, and its rows are re-checked when
retuned.)

**Do not delete a wrong prediction — record the correction beside it.** This
project keeps its mistakes on the record next to the numbers that corrected them,
because recognising the *shape* of a wrong prediction again is worth more than a
clean document. Several entries in `ENGINEERING_NOTES.md` and `PERFORMANCE.md`
exist only for that reason. File a correction as a correction, not as a rewrite.

**A stated rule that does not match the path in use is worse than no rule**,
because it stops people checking. If a document claims something is enforced,
verify the enforcement still points where the claim says.

**Wrap prose at 80 columns.** Adopted 2026-08-17 as `W1`; the argument is
retrieval cost, not neatness. These files are searched far more often than they
are read, and a grep hit is atomic at the line — so a 3,000-character line
returns 3,000 characters whether or not the match needed them. Measured: a
`grep -C2` into `ROADMAP.md` returned **9,930 bytes for 20 lines**, against about
1.3 KB for the same query shape against wrapped text. It also stops a one-word
change rendering as a whole-line rewrite in `git diff` and `git blame`.
- **`W1` shipped 2026-08-17 and the four files now comply**, so this is a rule
  the tree obeys rather than one it visibly violates. Before: `ROADMAP.md` avg
  394 chars/line, `ROADMAP_ITEMS.md` 285, `ENGINEERING_NOTES.md` 601,
  `PERFORMANCE.md` 205. After: 68, 77, 71, 62 — the two above 70 are table rows,
  which are exempt. `README.md` (63) and `PLAYTEST_LOG.md` (83) already complied,
  which is why this was an inconsistency to remove rather than a new convention
  to import. Measured gain, same three queries before and after against
  `ROADMAP.md`: `EASE_PER_SEC` 2,014 → 372 bytes, `determinism` 29,474 → 5,062,
  `parallax` 54,607 → 9,726 — **about a fifth to a sixth**, which is what the
  item predicted.
- **Tables, code blocks and headings are exempt** — wrapping any of them breaks
  it. A handful of lines still exceed 80 because a single token does (a long
  anchor link); breaking those would break the link.
- **Wrap new prose as you write it in every one of these files.** The reflow was
  a one-time pass over a corpus, not a step to repeat — a file drifts back one
  unwrapped paragraph at a time.

## Per-file conventions

**ROADMAP.md** — the one live plan: what is next, how big it is, and why it is
ordered that way, **one entry per item**. **346 KB**, so search it rather than
reading it through — except its **The plan** block at the top, which is the only
part that has to be re-read to know what to do. *(This line said 280 KB, then 399
KB; `W4` took it to 346 KB, against a 319 KB archive, by moving closed work out.)*
- **It is the only document that carries development steps.** This was false from
  the day `ROADMAP_ITEMS.md` was split out until `W4` merged it back on
  2026-08-17 — the sentence's own warning was what came true: a plan split across
  two files is a plan that disagrees with itself, and all 48 item IDs ended up in
  both. **It is true again, and the way to keep it true is to file an item's
  order and its argument in the same entry**, never in a second file.
- Live work first; finished work leaves the file for `ROADMAP_ARCHIVE.md`.
- **Item IDs (`F1`, `E5a`, `V13`, `P2`, `S0`) are never renumbered.** New items get
  new IDs even when that leaves the numbering ragged.

**ROADMAP_ARCHIVE.md** — closed work, and **nothing is ever required to read
it**. That promise is the whole point of the file and it is easy to break.
- **The boundary is "finished *and* nothing open depends on the reasoning"**, not
  "marked done". Before an entry moves, any finding in it that still constrains
  an open item gets written **into that open item** — `V22` carrying `V23b`'s
  ~50% camera cap is the worked example, and the absorbed text says where it came
  from and when.
- **Never cite the archive from a live entry as required reading.** A stub
  pointing at it for the full history is fine; "see the archive for why" is the
  failure mode, and it turns the archive back into part of the plan.
- Nothing is deleted here, including wrong predictions and superseded plans.

**PLAYTEST_LOG.md** — **questions and answers only.** A defect's symptom is
recorded here; why it happened, what was done and what it cost live in
ROADMAP.md, in the wave that spent it. Do not write fixes or root causes here.
- When writing a checklist row, prefer one whose two answers point at two
  different subsystems over one that can only pass or fail. It costs the same to
  write and is worth far more.

**TUNING.md** — feel knobs only: weight, speed, timing, animation. Correctness
constants are documented at the point they are used, not here. Retuning something
means **a row updated *and* a dated line in History, newest first.**

**PERFORMANCE.md** — numbers plus the methodology they are only meaningful
under. Stale tables are replaced wholesale rather than appended to. Every number
needs its measurement conditions; an unbracketed reading is not evidence.

**ENGINEERING_NOTES.md** — decisions deliberately made and then deferred, or
refused, so they are not rediscovered or reversed by accident. Reference
material, not a task list. A refusal belongs here **with its reasoning**; "we
don't do that" without an argument will not survive the next session that wants it.

**VISION.md** — goals, scope discipline, the Long Term wish list. The wish list is
for adding to freely; the discipline is only ever about what gets *built*. If
something needs to happen sooner, the move is to argue it into a different
section **in writing** — never to build it while it is still filed under "do not
start this yet."

**README.md** — build, run, test, and `## General Testing`: a **public-facing**
fundamentals checklist, one or two lines per item, for someone looking at the
project who wants to see whether it works. Keep it that way — **no reasoning, no
regression history, no roadmap IDs.** Anything that needs a paragraph belongs in
MANUAL_TESTING.md.

**MANUAL_TESTING.md** — the Manual Tester Checklist in full, **thirteen steps as
of 2026-08-17** (split out
of README on 2026-08-16). Each checklist step names the regression it exists to
catch, most of which have actually happened once. When a new class of regression
is found by hand, add the step; when a step's expectation is wrong, fix the
wording — one step once told testers to expect a frame rate the engine cannot
deliver, which would have had them file the known cost of the played world as a
new defect.
- **It opens with "Owed to the tester" and that list is maintained, not
  appended to.** Add a row the moment a session asks the human for something;
  delete the row the moment it comes back. A list that only grows is one nobody
  reads, and the whole point of putting it above the steps is that it is short
  enough to act on.

**`notes/`** — informal lore, brainstorming and reference observations, upstream
of anything scheduled. Ideas land here for free. A note that names in advance
what would stop it being a note is the strongest form of admission an item can
later have.
