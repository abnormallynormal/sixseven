#pragma once

#include "board.h"
#include "moveGen.h"
#include "types.h"
#include <atomic>

struct SearchConfig
{
  bool use_tt = true;                // transposition table cutoffs + TT move ordering
  bool use_mvv_lva = true;           // capture ordering
  bool use_killer = true;            // killer move ordering
  bool use_history = true;           // history heuristic ordering
  bool use_nmp = true;               // null move pruning
  bool use_lmr = true;               // late move reductions
  bool use_quiescence = true;        // quiescence search (when false, return eval at depth 0)
  bool use_static_eval_cache = true; // cache static_eval in TT entries
};

extern const SearchConfig DEFAULT_CONFIG;

extern Move killer_table[2][256];
extern int history_table[12][64];

extern std::atomic<u64> g_node_count;

extern std::atomic<u64> g_cutoff_count;
extern std::atomic<u64> g_cutoff_index_sum;
extern std::atomic<u64> g_cutoff_first_count;
// Per-index cutoff histogram (test instrumentation only). Index 0 is unused;
// index k counts cutoffs where the k-th legal move (1-based) caused the cutoff.
// Anything past 255 is bucketed into [255].
constexpr int CUTOFF_HIST_SIZE = 256;
extern std::atomic<u64> g_cutoff_by_index[CUTOFF_HIST_SIZE];

inline void reset_node_count() { g_node_count.store(0, std::memory_order_relaxed); }
inline u64 get_node_count() { return g_node_count.load(std::memory_order_relaxed); }

inline void reset_cutoff_stats()
{
  g_cutoff_count.store(0, std::memory_order_relaxed);
  g_cutoff_index_sum.store(0, std::memory_order_relaxed);
  g_cutoff_first_count.store(0, std::memory_order_relaxed);
  for (int i = 0; i < CUTOFF_HIST_SIZE; i++)
    g_cutoff_by_index[i].store(0, std::memory_order_relaxed);
}
inline u64 get_cutoff_count() { return g_cutoff_count.load(std::memory_order_relaxed); }
inline u64 get_cutoff_index_sum() { return g_cutoff_index_sum.load(std::memory_order_relaxed); }
inline u64 get_cutoff_first_count() { return g_cutoff_first_count.load(std::memory_order_relaxed); }

void reset_killer_table();
void reset_history_table();
void reset_tt();
void reset_search_state();

Move root_negamax(Board &board, MoveGenerator &move_gen, int depth, const SearchConfig &cfg = DEFAULT_CONFIG);

int negamax(Board &board, MoveGenerator &move_gen, int alpha, int beta, int depth, int ply, bool can_null, const SearchConfig &cfg = DEFAULT_CONFIG);

int score_move(Board &board, Move m, int ply, const SearchConfig &cfg);

int mvv_lva(Piece attack, Piece victim);

int quiescence(Board &board, MoveGenerator &move_gen, int alpha, int beta, int ply, int static_eval, int depth, const SearchConfig &cfg = DEFAULT_CONFIG);

inline Move iterative_deepening(Board &board, MoveGenerator &move_gen, int max_depth, const SearchConfig &cfg = DEFAULT_CONFIG)
{
  Move best_move;
  for (int i = 1; i <= max_depth; i++)
  {
    best_move = root_negamax(board, move_gen, i, cfg);
    for (int p = 0; p < 12; p++)
    {
      for (int s = 0; s < 64; s++)
      {
        history_table[p][s] /= 2;
      }
    }
  }
  return best_move;
}
