// Test D: Node-count sanity tests.
// For each feature, verify enabling it reduces node counts on average.

#include "test_harness.h"

static const char *nc_positions[] = {
    // Starting position
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    // Italian-ish middlegame
    "r1bq1rk1/pp3ppp/2nb1n2/3p4/3P4/2NB1N2/PP3PPP/R1BQ1RK1 w - - 0 1",
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

static const FlagEntry node_flags[] = {
    {"use_tt", off_tt},
    {"use_mvv_lva", off_mvv_lva},
    {"use_killer", off_killer},
    {"use_history", off_history},
    {"use_nmp", off_nmp},
    {"use_lmr", off_lmr},
};

// Collected for the summary
struct FeatureRatio
{
  std::string name;
  double avg_ratio_off_over_on; // >1 means feature reduces nodes
};
std::vector<FeatureRatio> g_feature_ratios;

void run_node_count_tests(int depth)
{
  std::cout << "\n=== Node-count Sanity Tests (D) - depth " << depth << " ===\n";
  size_t n_pos = sizeof(nc_positions) / sizeof(nc_positions[0]);

  for (const auto &fe : node_flags)
  {
    u64 total_on = 0, total_off = 0;
    int reduced_count = 0;
    for (size_t p = 0; p < n_pos; p++)
    {
      SearchConfig cfg_on;
      SearchResult r_on = run_search(nc_positions[p], depth, cfg_on);

      SearchConfig cfg_off;
      fe.disable(cfg_off);
      SearchResult r_off = run_search(nc_positions[p], depth, cfg_off);

      total_on += r_on.nodes;
      total_off += r_off.nodes;
      if (r_on.nodes < r_off.nodes) reduced_count++;
      std::cout << "  " << std::left << std::setw(16) << fe.name
                << " pos#" << p
                << " on=" << std::setw(9) << r_on.nodes
                << " off=" << std::setw(9) << r_off.nodes
                << " ratio(off/on)=" << std::fixed << std::setprecision(2)
                << (r_on.nodes > 0 ? (double)r_off.nodes / r_on.nodes : 0.0)
                << "\n";
    }

    double avg_ratio = total_on > 0 ? (double)total_off / (double)total_on : 0.0;
    g_feature_ratios.push_back({fe.name, avg_ratio});

    // Sanity: total nodes with feature ON must be lower than OFF
    bool ok = total_on < total_off;
    std::cout << "  -> " << fe.name << " total on=" << total_on
              << " off=" << total_off
              << " avg-ratio(off/on)=" << std::fixed << std::setprecision(2) << avg_ratio
              << " positions-reduced=" << reduced_count << "/" << n_pos
              << (ok ? "  [PASS]" : "  [FAIL]") << "\n";
    EXPECT_TRUE(ok, std::string(fe.name) + ": enabling feature did not reduce total nodes ("
                      + std::to_string(total_on) + " vs " + std::to_string(total_off) + ")");
  }
}

void print_node_count_summary()
{
  std::cout << "\n--- Feature Node-Count Ratios (off/on, higher = bigger savings) ---\n";
  for (const auto &fr : g_feature_ratios)
  {
    std::cout << "  " << std::left << std::setw(20) << fr.name
              << " x" << std::fixed << std::setprecision(2) << fr.avg_ratio_off_over_on << "\n";
  }
}
