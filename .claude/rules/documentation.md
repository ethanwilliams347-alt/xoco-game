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

## Per-file conventions

**ROADMAP.md** — the authority on *why*, and 280 KB, so search it rather than
reading it through.
- **It is the only document that carries development steps.** A plan split across
  two files is a plan that disagrees with itself.
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

**README.md** — build, run, test, and the ten-step Manual Tester Checklist. Each
checklist step names the regression it exists to catch, most of which have
actually happened once. When a new class of regression is found by hand, add the
step; when a step's expectation is wrong, fix the wording — one step once told
testers to expect a frame rate the engine cannot deliver, which would have had
them file the known cost of the played world as a new defect.

**`notes/`** — informal lore, brainstorming and reference observations, upstream
of anything scheduled. Ideas land here for free. A note that names in advance
what would stop it being a note is the strongest form of admission an item can
later have.
