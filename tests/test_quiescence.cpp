// Test F: Quiescence regression.
// In positions where the static eval is misleading (a piece is en prise), a
// depth-0 call should return the resolved value (via quiescence), not the
// noisy static eval.

#include "test_harness.h"

struct QuiesCase
{
  const char *name;
  const char *fen;
  // Static eval is misleading; with quiescence enabled the depth=0 score should
  // be substantially different (closer to true material balance after captures).
};

static const QuiesCase qcases[] = {
    // White queen on d4 is en prise — black pawn on e5 captures it via exd4.
    {"hanging queen w", "rnbqkbnr/pppp1ppp/8/4p3/3Q4/8/PPPP1PPP/RNB1KBNR b KQkq - 0 1"},
    // White rook on d4 is en prise — black pawn on e5 captures via exd4.
    {"hanging rook w",  "rnbqkbnr/pppp1ppp/8/4p3/3R4/8/PPPPPPPP/1NBQKBNR b Kkq - 0 1"},
};

void run_quiescence_tests()
{
  std::cout << "\n=== Quiescence Regression Tests (F) ===\n";

  for (const auto &c : qcases)
  {
    SearchConfig with_q;
    SearchConfig no_q;
    no_q.use_quiescence = false;

    int score_q  = eval_position(c.fen, with_q);
    int score_nq = eval_position(c.fen, no_q);

    int diff = std::abs(score_q - score_nq);
    bool meaningful_change = diff >= 150; // ~1.5 pawn difference is enough

    std::cout << "  " << std::left << std::setw(20) << c.name
              << " static_eval=" << score_nq
              << " quiesc=" << score_q
              << " diff=" << diff
              << (meaningful_change ? "  [PASS]" : "  [FAIL]")
              << "\n";
    EXPECT_TRUE(meaningful_change, std::string(c.name) + ": quiescence did not resolve a hanging piece (diff=" + std::to_string(diff) + ")");
  }
}
