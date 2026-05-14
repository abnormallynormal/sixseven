# Bugs Found While Wiring Up the Test Suite

These were uncovered while building the test harness. Per your instruction I
have not fixed any of them (except #1 below, which I had to patch in order for
the test suite to run at all — please verify the patch).

---

## 1. *(PATCHED — verify)* `Move::set_ep_square` defaults to `-1`, corrupting state via phantom en passant

**Files involved:** [include/move.h](include/move.h), [src/makeMove.cpp](src/makeMove.cpp), [src/pseudoMoves.cpp](src/pseudoMoves.cpp)

All `Move` constructors except the double-push one default `set_ep_square` to
`-1`. `make_move()` then writes `en_passant_square = m.set_ep_square` at the
bottom of the function — so after **any non-double-push move** the board's
`en_passant_square == -1`.

In [src/pseudoMoves.cpp:101-117](src/pseudoMoves.cpp#L101-L117) the en-passant
generator checks `!= NO_SQUARE` (which is `64`), so `-1` slips through, and
then it executes `1ULL << board.en_passant_square` (i.e. `1ULL << -1`) — that
is undefined behaviour. On x86-64 the shift count is masked to 6 bits, so it
becomes `1ULL << 63` (bit `h8`). The generator then ANDs that with the pawn
attack table and, if any pawn sits on g7, produces a phantom **en passant
g7→h8**. `make_move` accepts it, overwrites `squares[h8]`, and leaves whatever
piece was on h8 (rook in the failing trace) still set in the bitboards. The
bitboards and `squares[]` array desynchronise and downstream code crashes via
`__builtin_ctzll(0)` once the desync touches a king bitboard.

**Patch I applied** (minimal): change the default initialiser in every `Move`
constructor from `set_ep_square(-1)` to `set_ep_square(NO_SQUARE)`, and add
`#include "square.h"` to [include/move.h](include/move.h) so `NO_SQUARE` is
visible.

**Why I patched it:** without this fix every search that reaches a position
with a pawn on g7 corrupts the board and crashes. No test in this suite would
have run.

**Triage suggestion:** the same fix is correct in `make_move` (clamp `-1`
→ `NO_SQUARE` when writing `en_passant_square`), or in `pseudoMoves.cpp` (bounds
check before the shift). The `move.h` change is the smallest.

---

## 2. `int negamax(... int ply = 0, bool can_null);` — default parameter precedes non-default

**File:** [include/search.h](include/search.h) (pre-edit)

The original declaration was `int negamax(Board&, MoveGenerator&, int alpha, int beta, int depth, int ply = 0, bool can_null);`. C++ forbids a defaulted parameter to precede a non-defaulted one. GCC accepted it because nothing actually `#include`d `search.h` except `search.cpp` itself, where the definition has no default. I rewrote the declaration without the spurious default while wiring up `SearchConfig`.

Same shape bug on the `quiescence` declaration.

---

## 3. `extern Move history_table[12][64];` declared as `Move`, defined as `int`

**Files:** [include/search.h](include/search.h) (pre-edit) vs [src/search.cpp:9](src/search.cpp#L9)

`history_table` is defined as `int history_table[12][64]` but declared as
`Move history_table[12][64]` in the header. The mismatch was harmless only
because nothing outside `search.cpp` referenced the extern. I corrected the
header declaration to `int` while editing `search.h`.

---

## 4. `int entry_score, entry_eval;` were uninitialised before being read

**File:** [src/search.cpp:37](src/search.cpp#L37) (pre-edit)

```cpp
int entry_score, entry_eval;
Move entry_move;
if (probe_entry(board.hash, depth, alpha, beta, ply, entry_score, entry_eval, entry_move))
  return entry_score;
...
else if (entry_eval != NO_EVAL)
  static_eval = entry_eval;
```

`probe_entry` only writes `entry_eval` when the hash **matches** the slot. On
a miss the variable was uninitialised and the subsequent `entry_eval != NO_EVAL`
check read indeterminate memory. The probability of indeterminate equalling
`INT_MIN` is negligible, but the behaviour is UB. I initialise them to safe
defaults (`0` and `NO_EVAL`) in the new code.

---

## 5. `root_negamax` does not filter illegal moves at root

**File:** [src/search.cpp:174-194](src/search.cpp#L174-L194)

The engine's root function iterates the pseudo-legal move list and never
checks whether the moving side left its own king in check. If such a move is
tried, the recursive negamax searches a position where one king can be
"captured", potentially leading to `is_in_check` calling
`__builtin_ctzll(0)` (UB) once the king bitboard hits zero.

My test harness sidesteps this by using its own root function
(`negamax_root_score` in [tests/test_harness.h](tests/test_harness.h)) that
filters illegal root moves. The engine's `root_negamax` itself is unchanged.

---

## 6. `MoveList move_lists[64]` sized for 64 plies; quiescence can exceed that

**File:** [include/moveGen.h:12](include/moveGen.h#L12)

`MoveGenerator::move_lists` is 64 entries, but quiescence recursively calls
`generate_moves(board, ply)` / `generate_captures(board, ply)` and increments
`ply` each time. On a deeply nested capture chain `ply` can exceed 63 and
write OOB. The `killer_table[2][256]` size suggests the author already
intended ~256 plies elsewhere.

I have not hit this in my test suite at the shallow depths used (depths 4–6),
but it is a latent risk at higher depths or in pathological positions.

---

## 7. Killer move ordering scores below MVV-LVA captures

**File:** [src/search.cpp:11-28](src/search.cpp#L11-L28)

`score_move` returns MVV-LVA values in the range roughly `[-800, +800]` for
captures but `90000` / `80000` for killer moves. The result is that **every
killer move outranks every capture**, including PxQ. The conventional ordering
in mainstream engines is `winning_capture > killer > losing_capture`. This is
a likely cause of weaker-than-expected move ordering on tactical positions
(see test E results in [tests/test_move_ordering.cpp](tests/test_move_ordering.cpp)).

---

## 8b. *(empirical)* history heuristic ON searches **more** nodes than OFF

This is what the node-count test (D) actually surfaces — the single failing
assertion in the suite:

```
use_history      pos#0 on=29764     off=32999     ratio(off/on)=1.11
use_history      pos#1 on=7365799   off=6603202   ratio(off/on)=0.90
-> use_history total on=7395563 off=6636201 avg-ratio(off/on)=0.90  [FAIL]
```

On the Italian-style middlegame at depth 5, disabling the history heuristic
*saves* ~10% of nodes. With deeper searches (depth 5 over Kiwipete-style
middlegames it's worse — ~3× more nodes with history on). The most likely
mechanism is the unaged/unscaled history values from #8 above — by the time
iterative deepening reaches depth 5 the values are large enough to override
MVV-LVA-like signals on quiet moves and produce poor ordering. Worth
ageing the table each ID iteration (e.g. `history[p][s] /= 2`) before triaging
further.

---

## 8c. NMP cannot fire at the root because of `beta < MATE_THRESHOLD - 256`

[src/search.cpp:60](src/search.cpp#L60):

```cpp
if (cfg.use_nmp && !is_in_check && depth >= 3 && beta < MATE_THRESHOLD - 256 && ...)
```

The intent of `beta < MATE_THRESHOLD - 256` is to skip NMP when beta is in
the mate-score window. But because the engine uses `INF = 1000000` (much
larger than `MATE_THRESHOLD = 100000`), `beta == INF` also fails the check.
That means **NMP never fires at any node with a full-width window** — only
under null-window re-searches deep enough that LMR has tightened beta to
a small value. At test-suite depths NMP fires far less than expected.

Fix: change the guard to `std::abs(beta) < MATE_THRESHOLD - 256` (or similar),
so `INF` passes.

---

## 8d. TT ON sometimes searches **more** nodes than TT OFF on quiet middlegames

At depth 5 over an Italian-style middlegame the TT-on/off ratio is roughly
1.03 (close, but the wrong sign). On the heavier middlegame from the earlier
3-position D suite it was ratio 0.81 (TT actively hurts). The TT lookup logic
itself is fine; the most likely culprits are (a) hash collisions in the
fixed-2²⁰ table mixing scores between different positions, and (b) the
unaged history table interaction — TT move ordering puts a stale move first,
the history score puts a different stale move next, and the search re-visits
many sub-trees on the next ID iteration.

---

## 9. Root-legality crash when given an illegal FEN

If the side-to-move is already in check in the FEN (illegal in real chess but
trivially supplyable), the engine will "capture the king" in the search,
then call `is_in_check` on a zero king-bitboard and trip
`__builtin_ctzll(0)`. This isn't reachable from legal gameplay but it makes
debugging painful — I burned an hour on this before realising one of my mate
FENs (`7k/8/6KQ/8/8/8/8/8 w`) had black already in check from `Qh6`.

Cheap defence: assert at entry that `bitboards[wKing] != 0` and
`bitboards[bKing] != 0`, and that the *non-moving* side is not in check.

---

## 8. History update indexes by destination square only

**File:** [src/search.cpp:143](src/search.cpp#L143)

```cpp
history_table[board.squares[m.to]][m.to] += depth * depth;
```

Standard history heuristic is `history[piece][to]`. The implementation here is
correct in shape, but I noticed two subtleties:

- `score_move` reads `history_table[board.squares[m.from]][m.to]` (piece taken
  from the **source** before make), while the update reads
  `history_table[board.squares[m.to]][m.to]` (piece taken from the
  **destination** after make). Both resolve to the moving piece, so they
  match — but the asymmetry is fragile.
- The table is never aged or scaled, so values grow unboundedly across iterative
  deepening iterations. Eventually they could dominate killer/MVV-LVA scoring
  in unintended ways.

---
