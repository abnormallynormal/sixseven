// TT diagnostic: depth-7 and depth-8 measurements on the two transposition-
// heavy positions. Reports node count on/off, ratio, probe count, cutoff rate,
// hash-match-without-cutoff count, and TT fill rate.

#include "test_harness.h"
#include "test_positions.h"

static double tt_fill_rate()
{
  u64 used = 0;
  for (int i = 0; i < table_size; i++)
    if (table[i].key != 0) used++;
  return (double)used / (double)table_size * 100.0;
}

void run_tt_diagnostic(int depth)
{
  std::cout << "\n=== TT Diagnostic - depth " << depth << " ===\n";

  for (const auto &p : deep_positions())
  {
    if (p.kind != PositionKind::TranspositionHeavy)
      continue;

    std::cout << "\n  " << p.name << "  (depth " << depth << ")\n";

    // TT off baseline
    SearchConfig cfg_off;
    cfg_off.use_tt = false;
    SearchResult r_off = run_search(p.fen, depth, cfg_off);
    u64 nodes_off = r_off.nodes;

    // TT on, capture probe stats afterwards
    SearchConfig cfg_on; // all default-on
    SearchResult r_on = run_search(p.fen, depth, cfg_on);
    u64 nodes_on = r_on.nodes;
    u64 probes = g_tt_probes.load(std::memory_order_relaxed);
    u64 matches = g_tt_probe_hash_matches.load(std::memory_order_relaxed);
    u64 cutoffs = g_tt_probe_cutoffs.load(std::memory_order_relaxed);
    u64 stores  = g_tt_stores.load(std::memory_order_relaxed);
    double fill = tt_fill_rate();

    double ratio       = nodes_on > 0 ? (double)nodes_off / (double)nodes_on : 0.0;
    double match_pct   = probes  > 0 ? 100.0 * (double)matches / (double)probes : 0.0;
    double cutoff_pct  = probes  > 0 ? 100.0 * (double)cutoffs / (double)probes : 0.0;
    double match_cutoff_pct = matches > 0 ? 100.0 * (double)cutoffs / (double)matches : 0.0;
    u64 match_no_cutoff = matches - cutoffs;

    std::cout << "    nodes  off="     << std::setw(11) << nodes_off
              << " on=" << std::setw(11) << nodes_on
              << " ratio(off/on)=" << std::fixed << std::setprecision(2) << ratio << "\n";
    std::cout << "    TT     probes="  << std::setw(11) << probes
              << " stores=" << std::setw(11) << stores
              << " hash-matches=" << matches
              << " cutoffs=" << cutoffs << "\n";
    std::cout << "    rates  match%="  << std::fixed << std::setprecision(2) << std::setw(6) << match_pct
              << "  cutoff%(of probes)=" << std::setw(6) << cutoff_pct
              << "  cutoff%(of matches)=" << std::setw(6) << match_cutoff_pct
              << "  match-no-cutoff=" << match_no_cutoff << "\n";
    std::cout << "    fill   table-fill=" << std::fixed << std::setprecision(2) << fill << "%"
              << "  (table_size=" << table_size << ")\n";

    std::cout << "    time   off=" << (int)r_off.elapsed_ms << "ms"
              << "  on=" << (int)r_on.elapsed_ms << "ms\n";
  }
}
