#!/usr/bin/env bash
# Build & run the search test suite.
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GXX="${CXX:-C:/msys64/mingw64/bin/g++.exe}"

SOURCES=(
  "$ROOT/src/board.cpp"
  "$ROOT/src/makeMove.cpp"
  "$ROOT/src/unmakeMove.cpp"
  "$ROOT/src/moveGen.cpp"
  "$ROOT/src/attacks.cpp"
  "$ROOT/src/pseudoMoves.cpp"
  "$ROOT/src/captures.cpp"
  "$ROOT/src/search.cpp"
  "$ROOT/src/transposition.cpp"
  "$ROOT/src/zobrist.cpp"
  "$ROOT/src/evaluation.cpp"
  "$ROOT/tests/test_main.cpp"
  "$ROOT/tests/test_soundness.cpp"
  "$ROOT/tests/test_mate.cpp"
  "$ROOT/tests/test_equivalence.cpp"
  "$ROOT/tests/test_node_counts.cpp"
  "$ROOT/tests/test_move_ordering.cpp"
  "$ROOT/tests/test_quiescence.cpp"
)

OUT="$ROOT/tests/test_search.exe"

"$GXX" -std=c++17 -O2 -g \
  -I "$ROOT/include" -I "$ROOT/tests" \
  "${SOURCES[@]}" \
  -o "$OUT"

echo "Built: $OUT"
"$OUT"
