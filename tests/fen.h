#pragma once

#include "board.h"
#include "zobrist.h"
#include "square.h"
#include <string>

inline bool load_fen(Board &board, const std::string &fen)
{
  for (int i = 0; i < 12; i++)
    board.bitboards[i] = 0ULL;
  for (int i = 0; i < 64; i++)
    board.squares[i] = EMPTY;

  size_t i = 0;
  int rank = 7, file = 0;
  for (; i < fen.size() && fen[i] != ' '; i++)
  {
    char c = fen[i];
    if (c == '/')
    {
      rank--;
      file = 0;
      continue;
    }
    if (c >= '1' && c <= '8')
    {
      file += (c - '0');
      continue;
    }
    Piece p = EMPTY;
    switch (c)
    {
    case 'P': p = wPawn;   break;
    case 'N': p = wKnight; break;
    case 'B': p = wBishop; break;
    case 'R': p = wRook;   break;
    case 'Q': p = wQueen;  break;
    case 'K': p = wKing;   break;
    case 'p': p = bPawn;   break;
    case 'n': p = bKnight; break;
    case 'b': p = bBishop; break;
    case 'r': p = bRook;   break;
    case 'q': p = bQueen;  break;
    case 'k': p = bKing;   break;
    default: return false;
    }
    int sq = rank * 8 + file;
    board.squares[sq] = p;
    board.bitboards[p] |= (1ULL << sq);
    file++;
  }

  while (i < fen.size() && fen[i] == ' ') i++;
  if (i >= fen.size()) return false;
  board.white_to_move = (fen[i] == 'w');
  i++;
  while (i < fen.size() && fen[i] == ' ') i++;

  board.castling_rights = 0;
  if (i < fen.size() && fen[i] == '-')
  {
    i++;
  }
  else
  {
    while (i < fen.size() && fen[i] != ' ')
    {
      char c = fen[i];
      if (c == 'K') board.castling_rights |= wK;
      else if (c == 'Q') board.castling_rights |= wQ;
      else if (c == 'k') board.castling_rights |= bK;
      else if (c == 'q') board.castling_rights |= bQ;
      i++;
    }
  }
  while (i < fen.size() && fen[i] == ' ') i++;

  if (i < fen.size() && fen[i] == '-')
  {
    board.en_passant_square = NO_SQUARE;
    i++;
  }
  else if (i + 1 < fen.size())
  {
    int f = fen[i] - 'a';
    int r = fen[i + 1] - '1';
    board.en_passant_square = r * 8 + f;
    i += 2;
  }
  else
  {
    board.en_passant_square = NO_SQUARE;
  }
  while (i < fen.size() && fen[i] == ' ') i++;

  board.half_move_count = 0;
  if (i < fen.size() && fen[i] >= '0' && fen[i] <= '9')
  {
    int n = 0;
    while (i < fen.size() && fen[i] >= '0' && fen[i] <= '9')
    {
      n = n * 10 + (fen[i] - '0');
      i++;
    }
    board.half_move_count = n;
  }

  board.update_position();
  board.hash = init_hash(board);
  return true;
}

inline std::string move_to_uci(const Move &m, const Board &board)
{
  if (m.is_castling)
  {
    if (m.is_kingside)
      return board.is_white_to_move() ? "e1g1" : "e8g8";
    return board.is_white_to_move() ? "e1c1" : "e8c8";
  }
  std::string s;
  s += (char)('a' + (m.from % 8));
  s += (char)('1' + (m.from / 8));
  s += (char)('a' + (m.to % 8));
  s += (char)('1' + (m.to / 8));
  if (m.promotion_piece != EMPTY)
  {
    char c = 'q';
    switch (m.promotion_piece)
    {
    case wQueen: case bQueen: c = 'q'; break;
    case wRook: case bRook:   c = 'r'; break;
    case wBishop: case bBishop: c = 'b'; break;
    case wKnight: case bKnight: c = 'n'; break;
    default: break;
    }
    s += c;
  }
  return s;
}
