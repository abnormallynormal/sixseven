#include "evaluation.h"
#include <algorithm>

const int opening_passed_pawn_bonuses[] = {0, 5, 10, 20, 35, 50, 60};
const int end_passed_pawn_bonuses[] = {0, 15, 15, 25, 40, 70, 120};
const int opening_isolated_pawn_maluses[] = {0, -10, -25, -50, -80, -85, -90, -95, -100};
const int end_isolated_pawn_maluses[] = {0, -20, -45, -80, -120, -130, -140, -150, -160};

void precompute_eval(Board &board)
{
  board.opening_material = 0;
  board.end_material = 0;
  board.opening_psqt = 0;
  board.end_psqt = 0;
  board.phase = 0;
  for (int i = 0; i < 64; i++)
  {
    Piece p = board.squares[i];
    if (p != EMPTY)
    {
      bool isWhite = p < 6;
      int sign = isWhite ? 1 : -1;
      int square = isWhite ? i : (i ^ 56);
      board.opening_material += opening_piece_vals[p] * sign;
      board.end_material += end_piece_vals[p] * sign;
      board.opening_psqt += opening_tables[p % 6][square] * sign;
      board.end_psqt += end_tables[p % 6][square] * sign;
      board.phase += phase_weights[p];
    }
  }
}

int evaluate_position(Board &board)
{
  int opening_eval = board.opening_material + board.opening_psqt;
  int end_eval = board.end_material + board.end_psqt;
  int phase = std::min(24, board.phase);

  int eval = ((opening_eval * phase + end_eval * (24 - phase)) / 24);

  eval += evaluate_pawn_struct(board, phase);

  if (!board.is_white_to_move())
    eval *= -1;

  return eval + TEMPO_BONUS;
}

u64 compute_passed_pawn_mask(Board &board, int sq, bool white)
{
  u64 a_file = 0x0101010101010101;
  u64 file_on = a_file << (sq % 8);
  u64 ranks_ahead = white ? ~0ULL << ((sq / 8 + 1) * 8) : ~0ULL >> (64 - (sq / 8) * 8);
  return ranks_ahead & (compute_neighboring_files(sq) | file_on);
}

int evaluate_pawn_struct(Board &board, int phase)
{
  PawnEntry &entry = pawn_table[board.pawns_hash & (pawn_table_size - 1)];
  if (entry.key == board.pawns_hash)
  {
    return entry.score;
  }

  u64 white = board.bitboards[wPawn];
  u64 white_pawns = white;
  u64 black = board.bitboards[bPawn];
  u64 black_pawns = black;
  int opening_eval = 0;
  int end_eval = 0;
  int total_w_isolated = 0;
  int total_b_isolated = 0;
  int opening_passed_eval = 0;
  int end_passed_eval = 0;
  int doubled = 0;
  while (white_pawns)
  {
    int pawn = __builtin_ctzll(white_pawns);
    u64 neighbors = compute_neighboring_files(pawn);
    if ((neighbors & white) == 0)
    {
      total_w_isolated++;
    }
    white_pawns &= white_pawns - 1;
    u64 passed_mask = compute_passed_pawn_mask(board, pawn, true);
    if ((passed_mask & black) == 0)
    {
      opening_passed_eval += opening_passed_pawn_bonuses[pawn / 8];
      end_passed_eval += end_passed_pawn_bonuses[pawn / 8];
    }
    u64 file = 0x0101010101010101 << (pawn % 8);
    u64 file_without_pawn = file & ~(1ULL << pawn);
    if (file_without_pawn & white)
    {
      doubled++;
    }
  }
  while (black_pawns)
  {
    int pawn = __builtin_ctzll(black_pawns);
    u64 neighbors = compute_neighboring_files(pawn);
    if ((neighbors & black) == 0)
    {
      total_b_isolated++;
    }
    u64 passed_mask = compute_passed_pawn_mask(board, pawn, false);
    if ((passed_mask & white) == 0)
    {
      opening_passed_eval -= opening_passed_pawn_bonuses[7 - pawn / 8];
      end_passed_eval -= end_passed_pawn_bonuses[7 - pawn / 8];
    }
    black_pawns &= black_pawns - 1;
    u64 file = 0x0101010101010101 << (pawn % 8);
    u64 file_without_pawn = file & ~(1ULL << pawn);
    if (file_without_pawn & black)
    {
      doubled--;
    }
  }
  opening_eval += (opening_isolated_pawn_maluses[total_w_isolated] - opening_isolated_pawn_maluses[total_b_isolated]);
  end_eval += (end_isolated_pawn_maluses[total_w_isolated] - end_isolated_pawn_maluses[total_b_isolated]);
  opening_eval += opening_passed_eval / 2;
  end_eval += end_passed_eval / 2;
  opening_eval -= doubled * 10;
  end_eval -= doubled * 25;

  int eval = ((opening_eval * phase + end_eval * (24 - phase)) / 24);
  store_pawn_entry(board.pawns_hash, eval);

  return eval;
}
