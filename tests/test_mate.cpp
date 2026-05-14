// Test B: Mate-finding tests for known mate-in-N positions.
// Designing forced-mate positions for a material-only engine is tricky, so
// the set is intentionally small and well-vetted. Each FEN must have the
// side-to-move NOT in check (otherwise the engine, lacking root-legality
// filtering, will "capture" the king and crash).

#include "test_harness.h"

struct MateCase
{
  const char *name;
  const char *fen;
  int n;
};

static const MateCase mate_cases[] = {
    // Back-rank rook mate in 1: Rd8#
    {"M1 Rd8#",        "6k1/5ppp/8/8/8/8/8/3R2K1 w - - 0 1", 1},
    // King + Queen corner mate in 1: Qc8#
    {"M1 Qc8#",        "k7/8/1K6/8/8/8/8/2Q5 w - - 0 1", 1},
    // King + Queen mate in 2 (Wk b6, Wq e5; Bk a8) — Q delivers mate in two
    {"M2 Q vs K",      "k7/8/1K6/4Q3/8/8/8/8 w - - 0 1", 2},
};

void run_mate_tests()
{
  std::cout << "\n=== Mate-finding Tests (B) ===\n";
  SearchConfig cfg;

  for (const auto &c : mate_cases)
  {
    int search_depth = 2 * c.n + 1;
    if (search_depth < 3) search_depth = 3;
    std::cout << "  [trying " << c.name << " at depth " << search_depth << "]\n" << std::flush;
    SearchResult r = run_iterative(c.fen, search_depth, cfg);

    int expected = MATE_THRESHOLD - (2 * c.n - 1);
    bool in_mate_range = (r.score >= MATE_THRESHOLD - 1000);
    // Allow a quicker mate than declared (>=expected) but not a slower one.
    bool length_ok = (r.score >= expected) && (r.score <= MATE_THRESHOLD - 1);

    std::cout << "  " << std::left << std::setw(18) << c.name
              << " M" << c.n
              << " score=" << r.score
              << " expected>=" << expected
              << " nodes=" << r.nodes
              << (in_mate_range && length_ok ? "  [PASS]" : "  [FAIL]")
              << "\n";

    EXPECT_TRUE(in_mate_range, std::string(c.name) + ": score not in mate range");
    EXPECT_TRUE(length_ok, std::string(c.name) + ": score does not reflect mate <=" + std::to_string(c.n));

    SearchResult r2 = run_iterative(c.fen, search_depth + 2, cfg);
    bool deeper_consistent = (r2.score >= MATE_THRESHOLD - 1000) && (r2.score >= expected);
    EXPECT_TRUE(deeper_consistent, std::string(c.name) + ": deeper search mate score regressed: "
                                       + std::to_string(r2.score));
  }
}
