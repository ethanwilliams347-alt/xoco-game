---
paths:
  - "**/ROADMAP.md"
  - "**/ROADMAP_ITEMS.md"
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
- **Four files do not comply yet and `W1` is the item that fixes them:**
  `ROADMAP.md` (avg 394 chars/line), `ROADMAP_ITEMS.md` (285),
  `ENGINEERING_NOTES.md` (601), `PERFORMANCE.md` (205). `README.md` (63) and
  `PLAYTEST_LOG.md` (83) already comply, which is why this is an inconsistency to
  remove rather than a new convention to import. **Named here rather than stated
  as universal, because a rule the tree visibly violates is one people stop
  checking.**
- **Tables and code blocks are exempt** — wrapping either breaks it.
- **New prose written into the four non-compliant files should be wrapped
  anyway.** `W1` then has less to do, and nothing is lost if it never runs.

## Per-file conventions

**ROADMAP.md** — the authority on *why*, and **399 KB**, so search it rather than
reading it through. *(This line said 280 KB until 2026-08-17; the file had grown
43% past the number that was telling people how carefully to search it.)*
- **It is the only document that carries development steps.** ⚠️ **This has been
  false since `ROADMAP_ITEMS.md` was split out, and the sentence's own warning is
  what came true:** a plan split across two files is a plan that disagrees with
  itself, and **all 48 item IDs now appear in both.** `W4` merges them back to one
  live plan plus a dated archive. **Until it lands, keep filing both halves** —
  a half-migrated plan is worse than a duplicated one.
- Live work first; finished work moves to *Shipped* at the bottom, kept for its
  reasoning rather than its status.
- **Item IDs (`F1`, `E5a`, `V13`, `P2`, `S0`) are never renumbered.** New items get
  new IDs even when that leaves the numbering ragged.

**ROADMAP_ITEMS.md** — the authority on *what is next*: the running order as a
table with sizes. Split out deliberately, because an order that lives in six
preambles scattered through a 280 KB file is a force against changing direction.
**Update this one when priorities move.**

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
