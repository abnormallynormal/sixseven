#pragma once
#include "types.h"
#include "move.h"
#include "searchConstants.h"
#include <climits>
#include <atomic>

const int table_size = 1 << 20;
const int NO_EVAL = INT_MIN;

struct HashEntry
{
  u64 key;
  int score;
  int depth;
  Move best_move;
  int node_type;
  int static_eval;
};

extern HashEntry table[table_size];

// Test-only instrumentation. Incremented by probe_entry below.
// No effect on search behaviour.
extern std::atomic<u64> g_tt_probes;
extern std::atomic<u64> g_tt_probe_hash_matches;
extern std::atomic<u64> g_tt_probe_cutoffs;
extern std::atomic<u64> g_tt_stores;

inline void reset_tt_stats()
{
  g_tt_probes.store(0, std::memory_order_relaxed);
  g_tt_probe_hash_matches.store(0, std::memory_order_relaxed);
  g_tt_probe_cutoffs.store(0, std::memory_order_relaxed);
  g_tt_stores.store(0, std::memory_order_relaxed);
}

inline void store_entry(u64 hash, int score, int depth, Move best_move, int node_type, int ply, int eval)
{
  g_tt_stores.fetch_add(1, std::memory_order_relaxed);
  HashEntry &entry = table[hash & (table_size - 1)];

  if (entry.key == hash || depth >= entry.depth)
  {
    if (score > MATE_THRESHOLD - 10000)
    {
      score += ply;
    }
    else if (score < -MATE_THRESHOLD + 10000)
    {
      score -= ply;
    }

    entry.key = hash;
    entry.score = score;
    entry.depth = depth;
    entry.best_move = best_move;
    entry.node_type = node_type;
    entry.static_eval = eval;
  }
}

inline bool probe_entry(u64 hash, int depth, int alpha, int beta, int ply, int &score, int &eval, Move &m)
{
  g_tt_probes.fetch_add(1, std::memory_order_relaxed);
  HashEntry &entry = table[hash & (table_size - 1)];
  if (entry.key != hash)
    return false;
  g_tt_probe_hash_matches.fetch_add(1, std::memory_order_relaxed);
  m = entry.best_move;
  eval = entry.static_eval;
  if (entry.depth >= depth)
  {
    int s = entry.score;
    if (s > MATE_THRESHOLD - 10000)
      s -= ply;
    else if (s < -MATE_THRESHOLD + 10000)
      s += ply;
    if (entry.node_type == EXACT)
    {
      g_tt_probe_cutoffs.fetch_add(1, std::memory_order_relaxed);
      score = s;
      return true;
    }
    if (entry.node_type == UPPER && s <= alpha)
    {
      g_tt_probe_cutoffs.fetch_add(1, std::memory_order_relaxed);
      score = s;
      return true;
    }
    if (entry.node_type == LOWER && s >= beta)
    {
      g_tt_probe_cutoffs.fetch_add(1, std::memory_order_relaxed);
      score = s;
      return true;
    }
  }
  return false;
}
