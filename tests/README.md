# Search Test Suite

## How to run

From the repository root:

```powershell
C:/msys64/mingw64/bin/g++.exe -std=c++17 -O2 -g -I include -I tests `
  src/board.cpp src/makeMove.cpp src/unmakeMove.cpp src/moveGen.cpp `
  src/attacks.cpp src/pseudoMoves.cpp src/captures.cpp src/search.cpp `
  src/transposition.cpp src/zobrist.cpp src/evaluation.cpp `
  tests/test_main.cpp tests/test_soundness.cpp tests/test_mate.cpp `
  tests/test_equivalence.cpp tests/test_node_counts.cpp `
  tests/test_move_ordering.cpp tests/test_quiescence.cpp `
  -o tests/test_search.exe

tests/test_search.exe
```

A bash helper (`tests/build_and_run.sh`) does the same thing.

Wall-clock on a typical machine: **~55 seconds**. Total assertions: 47.

## Layout

| File | What it does |
|------|--------------|
| [test_harness.h](test_harness.h) | `run_search` / `run_iterative` / `eval_position`, mini assertion macro, summary printers |
| [fen.h](fen.h) | FEN parser (writes into `Board`, recomputes Zobrist hash) |
| [test_main.cpp](test_main.cpp) | Driver — calls each suite, prints final summary |
| [test_soundness.cpp](test_soundness.cpp) | (A) engine picks the known best move on simple tactical positions |
| [test_mate.cpp](test_mate.cpp) | (B) mate-in-N — score lands in mate window at expected depth, stays stable when searched deeper |
| [test_equivalence.cpp](test_equivalence.cpp) | (C) for every sound flag, toggling it must not change the score |
| [test_node_counts.cpp](test_node_counts.cpp) | (D) for every sound flag, toggling it OFF should *increase* node count |
| [test_move_ordering.cpp](test_move_ordering.cpp) | (E) average beta-cutoff index ≈ 1.0 when ordering is healthy |
| [test_quiescence.cpp](test_quiescence.cpp) | (F) at depth 0, quiescence resolves a hanging piece that the static eval misses |
| [BUGS.md](BUGS.md) | Pre-existing engine bugs the harness uncovered |

## Engine changes I made to wire this up

- [include/search.h](../include/search.h): added `SearchConfig` (eight boolean flags, all defaulting to `true`), `g_node_count` (atomic `u64`), `g_cutoff_count/_index_sum/_first_count` (for test E), `reset_search_state()` that wipes killers, history, TT, and counters.
- [src/search.cpp](../src/search.cpp): each feature now reads its `cfg` flag (TT probe/store, MVV-LVA, killer, history, NMP, LMR, quiescence, static-eval cache). `negamax` and `quiescence` increment the node counter; `negamax` also records cutoff index on each fail-high. Default behaviour (all flags `true`) matches the pre-change engine exactly.
- [include/move.h](../include/move.h): one bug-fix patch (default `set_ep_square` to `NO_SQUARE`, not `-1`) — see [BUGS.md](BUGS.md) #1. Without this the test suite crashes on any search reaching a position with a pawn on g7.

## Result summary (last run)

```
=== Soundness Tests (A) - depth 5 ===           5/5 PASS
=== Mate-finding Tests (B) ===                  9/9 PASS  (3 mates × 3 assertions)
=== Equivalence Tests (C) - depth 4 ===         21/21 PASS  (7 flags × 3 positions)
=== Node-count Sanity Tests (D) - depth 5 ===   5/6 PASS  (1 flag fails — see below)
=== Move Ordering Quality Tests (E) - depth 5 ===  4/4 PASS  (avg cutoff index 2.4)
=== Quiescence Regression Tests (F) ===         2/2 PASS

Feature node-count ratios (off/on; >1 means feature reduces work):
  use_tt           x1.03   marginal
  use_mvv_lva      x2.32   strong
  use_killer       x2.61   strong
  use_history      x0.90   *negative* — feature hurts
  use_nmp          x1.40   modest
  use_lmr          x11.41  huge

Move ordering quality:
  pos#0 (start)            avg-index 1.40  first-move cutoff 96.7%
  pos#1 (Italian-style)    avg-index 2.80  first-move cutoff 80.6%
  pos#2 (quiet middlegame) avg-index 2.12  first-move cutoff 83.3%
  OVERALL                  avg-index 2.44  first-move cutoff 82.5%

Total passed: 46
Total failed: 1   (use_history: see BUGS.md #8 and #8b)
```

The single failure is the history heuristic actively *hurting* search at
depth 5 — the assertion is doing its job by surfacing this.

## Notes on test choices

- All FENs in (A) and (B) were vetted to ensure the side-to-move is not
  already in check. The engine doesn't validate this and will crash via
  `__builtin_ctzll(0)` if you feed it an illegal FEN (BUGS.md #9).
- Tests (C) and (D) intentionally use small position sets (3 each) to keep
  the total runtime under a minute. (E) and (D) at depth 5 are the dominant
  cost; (A) at depth 5 on the harder tactic (WAC.005) is the single
  longest-running individual case.
- Suite (G) (repetition / 50-move) is skipped: [src/search.cpp](../src/search.cpp)
  doesn't track repetitions and `half_move_count` is never read for
  fifty-move-rule purposes.
