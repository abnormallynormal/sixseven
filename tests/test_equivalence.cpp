// Test C: Equivalence tests — single feature toggled off must produce the same
// score as all-features-on. This verifies each feature is sound (only changes
// the work done, not the answer).

#include "test_harness.h"

static const char *eq_positions[] = {
    // Starting position
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    // Italian middlegame
    "r1bq1rk1/pp3ppp/2nb1n2/3p4/3P4/2NB1N2/PP3PPP/R1BQ1RK1 w - - 0 1",
    // Endgame-ish
    "8/p3kpp1/1p1q1n1p/3p4/3P4/PQ3N1P/1P3PP1/6K1 w - - 0 1",
};

struct FlagEntry
{
  const char *name;
  void (*disable)(SearchConfig &);
};

static void off_tt(SearchConfig &c) { c.use_tt = false; }
static void off_mvv_lva(SearchConfig &c) { c.use_mvv_lva = false; }
static void off_killer(SearchConfig &c) { c.use_killer = false; }
static void off_history(SearchConfig &c) { c.use_history = false; }
static void off_nmp(SearchConfig &c) { c.use_nmp = false; }
static void off_lmr(SearchConfig &c) { c.use_lmr = false; }
static void off_static_eval_cache(SearchConfig &c) { c.use_static_eval_cache = false; }

static const FlagEntry equivalence_flags[] = {
    {"use_tt", off_tt},
    {"use_mvv_lva", off_mvv_lva},
    {"use_killer", off_killer},
    {"use_history", off_history},
    {"use_nmp", off_nmp},
    {"use_lmr", off_lmr},
    {"use_static_eval_cache", off_static_eval_cache},
};

void run_equivalence_tests(int depth)
{
  std::cout << "\n=== Equivalence Tests (C) - depth " << depth << " ===\n";
  std::cout << "  Each flag is toggled off individually; score must match baseline.\n";

  // Compute baseline scores once per position with all features on
  std::vector<int> baseline_score(sizeof(eq_positions) / sizeof(eq_positions[0]));
  for (size_t p = 0; p < baseline_score.size(); p++)
  {
    SearchConfig cfg; // all on
    SearchResult r = run_search(eq_positions[p], depth, cfg);
    baseline_score[p] = r.score;
  }

  for (const auto &fe : equivalence_flags)
  {
    int n_pass = 0, n_total = 0;
    for (size_t p = 0; p < baseline_score.size(); p++)
    {
      SearchConfig cfg;
      fe.disable(cfg);
      SearchResult r = run_search(eq_positions[p], depth, cfg);
      n_total++;
      bool ok = (r.score == baseline_score[p]);
      if (ok) n_pass++;
      if (!ok)
      {
        std::cerr << "  FAIL " << fe.name << " pos#" << p
                  << " baseline=" << baseline_score[p]
                  << " toggled=" << r.score << "\n";
      }
      EXPECT_TRUE(ok, std::string(fe.name) + " pos#" + std::to_string(p)
        + " baseline=" + std::to_string(baseline_score[p])
        + " toggled=" + std::to_string(r.score));
    }
    std::cout << "  " << std::left << std::setw(24) << fe.name
              << " " << n_pass << "/" << n_total << " positions equivalent\n";
  }
}
