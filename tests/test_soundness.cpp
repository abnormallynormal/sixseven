// Test A: Soundness on tactical positions.
// The engine uses a material-only evaluation, so we restrict ourselves to
// positions where the best move is forced by a short, direct material swing
// that depth 6 search can resolve.

#include "test_harness.h"

struct TacticalCase
{
  const char *name;
  const char *fen;
  std::vector<std::string> best_moves; // accept any of these UCI strings
};

static const TacticalCase tac_cases[] = {
    // Free queen capture (queen en prise; multiple pieces can take it)
    {"Free queen", "rnb1kbnr/pppp1ppp/8/4p3/4P3/2q5/PPPP1PPP/RNBQKBNR w KQkq - 0 1", {"b2c3", "d2c3", "b1c3"}},
    // WAC.005: mate threat — engine found this at depth 6
    {"WAC.005 Qd1+", "1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - - 0 1", {"d6d1"}},
    // Knight fork wins queen
    {"Knight fork", "4k3/8/8/3q4/8/2N5/8/4K3 w - - 0 1", {"c3d5"}}, // Nxd5 forks (well captures queen)
    // White takes hanging black queen
    {"Free queen 2", "4k3/8/8/3q4/8/8/3R4/4K3 w - - 0 1", {"d2d5"}},
    // Simple back-rank mate-in-1
    {"Back-rank M1", "6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1", {"a1a8"}},
};

void run_soundness_tests(int depth)
{
  std::cout << "\n=== Soundness Tests (A) - depth " << depth << " ===\n";
  SearchConfig cfg;

  for (const auto &c : tac_cases)
  {
    if (c.best_moves.empty())
    {
      std::cout << "  " << std::left << std::setw(28) << c.name << "  [SKIP]\n";
      continue;
    }
    SearchResult r = run_iterative(c.fen, depth, cfg);
    std::string uci = result_to_uci(r);
    bool match = false;
    for (const auto &m : c.best_moves)
      if (m == uci) { match = true; break; }
    std::cout << "  " << std::left << std::setw(28) << c.name
              << " got=" << uci
              << " score=" << r.score
              << " nodes=" << r.nodes
              << " ms=" << std::fixed << std::setprecision(0) << r.elapsed_ms
              << (match ? "  [PASS]" : "  [FAIL]")
              << "\n";
    std::string expected;
    for (size_t i = 0; i < c.best_moves.size(); i++)
    {
      if (i) expected += "|";
      expected += c.best_moves[i];
    }
    EXPECT_TRUE(match, std::string(c.name) + ": expected {" + expected + "} got " + uci);
  }
}
