// Test E: Move ordering quality — measure the average beta-cutoff index.
// Good move ordering should produce most cutoffs on the first move tried.

#include "test_harness.h"
#include <sstream>

static const char *mo_positions[] = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",         // start
    "r1bq1rk1/pp3ppp/2nb1n2/3p4/3P4/2NB1N2/PP3PPP/R1BQ1RK1 w - - 0 1",  // Italian-ish
    "r1b2rk1/pp2qppp/2nbpn2/3p4/3P4/2NBPN2/PP1B1PPP/R2Q1RK1 w - - 0 1", // Quiet
};

struct OrderingRow
{
  std::string label;
  double avg_index;
  double first_pct;
  u64 cutoffs;
};
std::vector<OrderingRow> g_ordering_rows;

void run_move_ordering_tests(int depth)
{
  std::cout << "\n=== Move Ordering Quality Tests (E) - depth " << depth << " ===\n";
  SearchConfig cfg;

  size_t n_pos = sizeof(mo_positions) / sizeof(mo_positions[0]);
  u64 grand_cutoffs = 0, grand_first = 0, grand_index_sum = 0;

  for (size_t p = 0; p < n_pos; p++)
  {
    // run_search resets state including cutoff stats
    SearchResult r = run_search(mo_positions[p], depth, cfg);
    u64 cuts = get_cutoff_count();
    u64 first = get_cutoff_first_count();
    u64 idx_sum = get_cutoff_index_sum();
    double avg_idx = cuts > 0 ? (double)idx_sum / (double)cuts : 0.0;
    double first_pct = cuts > 0 ? 100.0 * (double)first / (double)cuts : 0.0;

    std::ostringstream label;
    label << "pos#" << p;
    g_ordering_rows.push_back({label.str(), avg_idx, first_pct, cuts});

    std::cout << "  pos#" << p
              << " cutoffs=" << std::setw(9) << cuts
              << " avg-index=" << std::fixed << std::setprecision(3) << avg_idx
              << " first-move-cutoff=" << std::fixed << std::setprecision(1) << first_pct << "%"
              << "\n";

    grand_cutoffs += cuts;
    grand_first += first;
    grand_index_sum += idx_sum;

    // Sanity check: average should be reasonably close to 1.0
    EXPECT_TRUE(avg_idx < 5.0, std::string("pos#") + std::to_string(p)
                  + " avg cutoff index " + std::to_string(avg_idx) + " > 5.0 (move ordering looks broken)");
  }

  double grand_avg = grand_cutoffs > 0 ? (double)grand_index_sum / (double)grand_cutoffs : 0.0;
  double grand_first_pct = grand_cutoffs > 0 ? 100.0 * (double)grand_first / (double)grand_cutoffs : 0.0;
  std::cout << "  --------------------------------\n";
  std::cout << "  OVERALL cutoffs=" << grand_cutoffs
            << " avg-index=" << std::fixed << std::setprecision(3) << grand_avg
            << " first-move-cutoff=" << std::fixed << std::setprecision(1) << grand_first_pct << "%\n";

  g_ordering_rows.push_back({"OVERALL", grand_avg, grand_first_pct, grand_cutoffs});

  // The engine's move ordering has known weaknesses (killer moves outrank
  // captures; see BUGS.md #7). Use a permissive threshold and surface the
  // numbers as a metric.
  EXPECT_TRUE(grand_avg < 3.5, "Overall avg cutoff index > 3.5 — move ordering looks broken");
}

void print_move_ordering_summary()
{
  std::cout << "\n--- Move Ordering Quality Summary ---\n";
  std::cout << "  (avg-index near 1.0 means cutoffs usually happen on first move tried)\n";
  for (const auto &row : g_ordering_rows)
  {
    std::cout << "  " << std::left << std::setw(10) << row.label
              << " cutoffs=" << std::setw(8) << row.cutoffs
              << " avg-index=" << std::fixed << std::setprecision(3) << row.avg_index
              << " first%=" << std::fixed << std::setprecision(1) << row.first_pct << "\n";
  }
}
