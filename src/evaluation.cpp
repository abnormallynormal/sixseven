#include "evaluation.h"
#include <algorithm>

int material_odds(Board &board)
{
  return piece_vals[wPawn] * (count_pieces(wPawn, board) - count_pieces(bPawn, board)) + piece_vals[wKnight] * (count_pieces(wKnight, board) - count_pieces(bKnight, board)) + piece_vals[wBishop] * (count_pieces(wBishop, board) - count_pieces(bBishop, board)) + piece_vals[wRook] * (count_pieces(wRook, board) - count_pieces(bRook, board)) + piece_vals[wQueen] * (count_pieces(wQueen, board) - count_pieces(bQueen, board));
}

void precompute_psqt(Board &board)
{
  for (int i = 0, j = 48; i < 16, j < 64; i++, j++)
  {
    Piece p = board.squares[i];
    bool isWhite = p < 6;
    if (isWhite)
    {
      board.opening_material += opening_piece_vals[p];
      board.opening_psqt += opening_tables[p % 6][i];
      board.end_material += end_piece_vals[p];
      board.end_psqt += end_tables[p % 6][i];
      board.phase += phase_weights[p];
    }
    else
    {
      board.opening_material += opening_piece_vals[p];
      board.opening_psqt += opening_tables[p % 6][i ^ 56] * -1;
      board.end_material += end_piece_vals[p];
      board.end_psqt += end_tables[p % 6][i ^ 56] * -1;
      board.phase += phase_weights[p];
    }
  }
}

int evaluate_position(Board &board)
{
  int opening_eval = board.opening_material + board.opening_psqt;
  int end_eval = board.end_material + board.end_psqt;
  int phase = std::min(24, board.phase);

  int eval = (opening_eval * phase + end_eval * (24 - phase)) / 24;

  if(!board.is_white_to_move()) eval *= -1;

  
}