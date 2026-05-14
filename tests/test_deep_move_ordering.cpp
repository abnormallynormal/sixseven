// Deep test E: per-position cutoff-index measurement at fixed depth across the
// full deep-position set. Adds a histogram showing the *tail* of the cutoff-
// index distribution — averages alone hide whether ordering is "almost always
// fine" vs "usually fine but a long tail of bad cases."

#include "test_harness.h"
#include "test_positions.h"
#include <sstream>

struct OrderingRow
{
  std::string label;
  std::string kind;
  u64 cutoffs;
  double avg_index;
  double first_pct;
};
static std::vector<OrderingRow> g_deep_ordering;

// Aggregated per-bucket cutoffs across all positions.
static u64 g_total_buckets[6] = {0, 0, 0, 0, 0, 0};
// Bucket boundaries: [1], [2], [3], [4..5], [6..10], [11..]
static const char *bucket_labels[6] = {"1", "2", "3", "4-5", "6-10", "11+"};

static int bucket_for(int index)
{
  if (index == 1) return 0;
  if (index == 2) return 1;
  if (index == 3) return 2;
  if (index <= 5) return 3;
  if (index <= 10) return 4;
  return 5;
}

void run_deep_move_ordering_tests(int depth)
{
  std::cout << "\n=== Deep Move-Ordering Tests (E-deep) - depth " << depth << " ===\n";
  std::cout << "  histogram buckets: " << bucket_labels[0] << " " << bucket_labels[1] << " "
            << bucket_labels[2] << " " << bucket_labels[3] << " " << bucket_labels[4] << " "
            << bucket_labels[5] << "\n\n";

  SearchConfig cfg;
  const auto &positions = deep_positions();
  u64 grand_cutoffs = 0;
  u64 grand_first = 0;
  u64 grand_idx_sum = 0;

  for (const auto &p : positions)
  {
    if (p.kind == PositionKind::TranspositionHeavy)
    {
      std::cout << "  " << std::left << std::setw(28) << p.name
                << "  [skipped — measured in TT diagnostic]\n" << std::flush;
      continue;
    }
    SearchResult r = run_search(p.fen, depth, cfg);
    u64 cuts = get_cutoff_count();
    u64 first = get_cutoff_first_count();
    u64 idx_sum = get_cutoff_index_sum();
    double avg = cuts > 0 ? (double)idx_sum / (double)cuts : 0.0;
    double fpct = cuts > 0 ? 100.0 * (double)first / (double)cuts : 0.0;

    u64 buckets[6] = {0, 0, 0, 0, 0, 0};
    for (int i = 1; i < CUTOFF_HIST_SIZE; i++)
    {
      u64 c = g_cutoff_by_index[i].load(std::memory_order_relaxed);
      if (c == 0) continue;
      buckets[bucket_for(i)] += c;
    }

    std::cout << "  " << std::left << std::setw(28) << p.name
              << " cuts=" << std::setw(8) << cuts
              << " avg-idx=" << std::fixed << std::setprecision(3) << std::setw(7) << avg
              << " first%=" << std::fixed << std::setprecision(1) << std::setw(5) << fpct
              << "   hist: ";
    for (int b = 0; b < 6; b++)
    {
      std::cout << std::setw(8) << buckets[b];
      g_total_buckets[b] += buckets[b];
    }
    std::cout << " (" << kind_name(p.kind) << ")\n" << std::flush;
    g_deep_ordering.push_back({p.name, kind_name(p.kind), cuts, avg, fpct});

    grand_cutoffs += cuts;
    grand_first += first;
    grand_idx_sum += idx_sum;
  }

  double grand_avg = grand_cutoffs > 0 ? (double)grand_idx_sum / (double)grand_cutoffs : 0.0;
  double grand_first_pct = grand_cutoffs > 0 ? 100.0 * (double)grand_first / (double)grand_cutoffs : 0.0;
  std::cout << "\n  OVERALL cuts=" << grand_cutoffs
            << " avg-idx=" << std::fixed << std::setprecision(3) << grand_avg
            << " first%=" << std::fixed << std::setprecision(1) << grand_first_pct << "\n";

  EXPECT_TRUE(grand_avg < 3.5, "Deep-E: overall avg cutoff index > 3.5");
}

void print_deep_move_ordering_summary()
{
  std::cout << "\n--- Deep-E Move-Ordering Summary ---\n";
  std::cout << "  histogram buckets: ";
  for (int b = 0; b < 6; b++)
    std::cout << bucket_labels[b] << "=" << g_total_buckets[b] << "  ";
  std::cout << "\n";
  u64 grand = 0;
  for (int b = 0; b < 6; b++) grand += g_total_buckets[b];
  std::cout << "  histogram fractions: ";
  for (int b = 0; b < 6; b++)
  {
    double frac = grand > 0 ? 100.0 * (double)g_total_buckets[b] / (double)grand : 0.0;
    std::cout << bucket_labels[b] << "=" << std::fixed << std::setprecision(1) << frac << "%  ";
  }
  std::cout << "\n";
}
