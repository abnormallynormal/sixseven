// Deep test D: per-feature node-count comparison across the full deep-position
// set. Reports per-position ratios plus the OVERALL geometric mean (more
// honest for ratio data than arithmetic mean).

#include "test_harness.h"
#include "test_positions.h"
#include <cmath>

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

static const FlagEntry deep_flags[] = {
    {"use_tt", off_tt},
    {"use_mvv_lva", off_mvv_lva},
    {"use_killer", off_killer},
    {"use_history", off_history},
    {"use_nmp", off_nmp},
    {"use_lmr", off_lmr},
};

// Geometric mean of a vector of positive doubles.
static double geomean(const std::vector<double> &xs)
{
  if (xs.empty()) return 0.0;
  double sum_log = 0.0;
  int n = 0;
  for (double x : xs)
  {
    if (x <= 0) continue;
    sum_log += std::log(x);
    n++;
  }
  if (n == 0) return 0.0;
  return std::exp(sum_log / n);
}

struct DeepFeatureSummary
{
  std::string name;
  double geomean_ratio;
  int positions_helped;
  int positions_total;
};
static std::vector<DeepFeatureSummary> g_deep_summary;

void run_deep_node_count_tests(int depth)
{
  std::cout << "\n=== Deep Node-count Tests (D-deep) - depth " << depth << " ===\n";
  std::cout << "  geomean is the right summary for ratio data; arithmetic gets pulled around by\n"
               "  a single explosive position.\n\n";
  const auto &positions = deep_positions();

  // Pre-compute baseline (all flags on) once per non-transposition position.
  // Transposition-heavy positions are measured separately in the TT diagnostic;
  // including them in D/E pushes runtime past the budget without adding signal.
  std::vector<u64> baseline_nodes(positions.size(), 0);
  std::vector<bool> include(positions.size(), false);
  for (size_t p = 0; p < positions.size(); p++)
  {
    if (positions[p].kind == PositionKind::TranspositionHeavy)
    {
      std::cout << "  baseline " << std::left << std::setw(28) << positions[p].name
                << "  [skipped — measured in TT diagnostic]\n" << std::flush;
      continue;
    }
    include[p] = true;
    SearchConfig cfg_on;
    SearchResult r = run_search(positions[p].fen, depth, cfg_on);
    baseline_nodes[p] = r.nodes;
    std::cout << "  baseline " << std::left << std::setw(28) << positions[p].name
              << " nodes=" << std::setw(12) << r.nodes
              << " ms=" << (int)r.elapsed_ms << "\n" << std::flush;
  }

  std::cout << "\n  per-feature ratios (off/on > 1 means feature reduces nodes):\n";
  for (const auto &fe : deep_flags)
  {
    std::cout << "\n  --- " << fe.name << " ---\n" << std::flush;
    std::vector<double> ratios;
    int helped = 0;
    for (size_t p = 0; p < positions.size(); p++)
    {
      if (!include[p]) continue;
      SearchConfig cfg_off;
      fe.disable(cfg_off);
      SearchResult r_off = run_search(positions[p].fen, depth, cfg_off);
      double ratio = baseline_nodes[p] > 0
                         ? (double)r_off.nodes / (double)baseline_nodes[p]
                         : 0.0;
      ratios.push_back(ratio);
      if (ratio > 1.0) helped++;
      std::cout << "    " << std::left << std::setw(28) << positions[p].name
                << " on=" << std::setw(11) << baseline_nodes[p]
                << " off=" << std::setw(11) << r_off.nodes
                << " ratio(off/on)=" << std::fixed << std::setprecision(2) << ratio
                << " (" << kind_name(positions[p].kind) << ")"
                << "\n" << std::flush;
    }
    double gm = geomean(ratios);
    g_deep_summary.push_back({fe.name, gm, helped, (int)ratios.size()});
    std::cout << "    -> geomean(off/on)=" << std::fixed << std::setprecision(3) << gm
              << "  positions-helped=" << helped << "/" << positions.size() << "\n";
    // Assertion on the geomean — feature should reduce nodes overall.
    EXPECT_TRUE(gm > 1.0, std::string("Deep-D ") + fe.name + ": geomean off/on = "
                              + std::to_string(gm) + " (feature did not reduce nodes overall)");
  }
}

void print_deep_node_count_summary()
{
  std::cout << "\n--- Deep-D Feature Geomean Ratios (off/on; >1 = feature saves work) ---\n";
  std::cout << "  " << std::left << std::setw(20) << "feature"
            << " " << std::setw(12) << "geomean"
            << " " << "positions-helped\n";
  for (const auto &fr : g_deep_summary)
  {
    std::cout << "  " << std::left << std::setw(20) << fr.name
              << " x" << std::fixed << std::setprecision(3) << std::setw(11) << fr.geomean_ratio
              << " " << fr.positions_helped << "/" << fr.positions_total << "\n";
  }
}
