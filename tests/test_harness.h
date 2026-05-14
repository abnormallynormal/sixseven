#pragma once

#include "board.h"
#include "moveGen.h"
#include "search.h"
#include "searchConstants.h"
#include "transposition.h"
#include "evaluation.h"
#include "fen.h"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <iomanip>
#include <cmath>
#include <climits>

struct SearchResult
{
  Move best_move;
  int score;
  u64 nodes;
  double elapsed_ms;
  bool white_to_move = true;
};

inline MoveGenerator &harness_move_gen()
{
  // MoveGenerator contains move_lists[256] (~3.3MB) which is too large for
  // a Windows default 1MB stack, so we heap-allocate a single shared instance.
  static MoveGenerator *mg = new MoveGenerator();
  return *mg;
}

inline int negamax_root_score(Board &board, MoveGenerator &move_gen, int depth, const SearchConfig &cfg, Move &out_best)
{
  out_best = Move();
  int best_score = -INF;
  move_gen.generate_moves(board, 0);
  int n = move_gen.move_lists[0].count;
  Move root_moves[218];
  for (int i = 0; i < n; i++)
    root_moves[i] = move_gen.move_lists[0].moves[i];
  for (int i = 0; i < n; i++)
  {
    Move m = root_moves[i];
    board.make_move(m);
    if (move_gen.is_in_check(board, !board.is_white_to_move()))
    {
      board.unmake_move(m);
      continue;
    }
    int score = -negamax(board, move_gen, -INF, INF, depth - 1, 1, true, cfg);
    board.unmake_move(m);
    if (score > best_score)
    {
      best_score = score;
      out_best = m;
    }
  }
  return best_score;
}

inline SearchResult run_search(const std::string &fen, int depth, const SearchConfig &cfg)
{
  Board board;
  MoveGenerator &move_gen = harness_move_gen();
  if (!load_fen(board, fen))
  {
    std::cerr << "Failed to parse FEN: " << fen << std::endl;
    return {Move(), 0, 0, 0.0};
  }

  reset_search_state();

  auto t0 = std::chrono::high_resolution_clock::now();
  Move best;
  int score = negamax_root_score(board, move_gen, depth, cfg, best);
  auto t1 = std::chrono::high_resolution_clock::now();

  SearchResult r;
  r.best_move = best;
  r.score = score;
  r.nodes = get_node_count();
  r.elapsed_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
  r.white_to_move = board.white_to_move;
  return r;
}

inline SearchResult run_iterative(const std::string &fen, int depth, const SearchConfig &cfg)
{
  Board board;
  MoveGenerator &move_gen = harness_move_gen();
  if (!load_fen(board, fen))
  {
    std::cerr << "Failed to parse FEN: " << fen << std::endl;
    return {Move(), 0, 0, 0.0};
  }

  reset_search_state();

  auto t0 = std::chrono::high_resolution_clock::now();
  Move best;
  int last_score = 0;
  for (int d = 1; d <= depth; d++)
  {
    last_score = negamax_root_score(board, move_gen, d, cfg, best);
  }
  auto t1 = std::chrono::high_resolution_clock::now();

  SearchResult r;
  r.best_move = best;
  r.score = last_score;
  r.nodes = get_node_count();
  r.elapsed_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
  r.white_to_move = board.white_to_move;
  return r;
}

inline std::string result_to_uci(const SearchResult &r)
{
  Board tmp;
  tmp.white_to_move = r.white_to_move;
  return move_to_uci(r.best_move, tmp);
}

// Evaluate a position from side-to-move POV at "depth 0" semantics: either
// quiescence (when use_quiescence=true) or raw static evaluate().
inline int eval_position(const std::string &fen, const SearchConfig &cfg)
{
  Board board;
  MoveGenerator &move_gen = harness_move_gen();
  load_fen(board, fen);
  reset_search_state();
  if (cfg.use_quiescence)
    return quiescence(board, move_gen, -INF, INF, 0, NO_EVAL, 0, cfg);
  return evaluate(board);
}

struct TestStats
{
  int passed = 0;
  int failed = 0;
  std::vector<std::string> failures;
};

inline TestStats &g_stats()
{
  static TestStats s;
  return s;
}

#define EXPECT_TRUE(cond, msg)                                                                 \
  do                                                                                           \
  {                                                                                            \
    if (cond)                                                                                  \
    {                                                                                          \
      g_stats().passed++;                                                                      \
    }                                                                                          \
    else                                                                                       \
    {                                                                                          \
      g_stats().failed++;                                                                      \
      std::string m = std::string(__FUNCTION__) + ": " + std::string(msg);                     \
      g_stats().failures.push_back(m);                                                         \
      std::cerr << "  FAIL [" << __FUNCTION__ << "] " << msg << std::endl;                     \
    }                                                                                          \
  } while (0)

inline void print_summary()
{
  std::cout << "\n=========================================\n";
  std::cout << "Total passed: " << g_stats().passed << "\n";
  std::cout << "Total failed: " << g_stats().failed << "\n";
  if (!g_stats().failures.empty())
  {
    std::cout << "\nFailures:\n";
    for (const auto &f : g_stats().failures)
      std::cout << "  - " << f << "\n";
  }
  std::cout << "=========================================\n";
}
