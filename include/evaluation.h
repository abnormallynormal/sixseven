#include "board.h"
#pragma once

const int pieceVals[] = {100, 300, 310, 500, 900, -1, 100, 300, 310, 500, 900, -1};

int countPieces(Piece p, Board &board)
{
  int count = 0;
  u64 bitboard = board.bitboards[p];
  while (bitboard > 0)
  {
    count = count + 1;
    bitboard = bitboard & (bitboard - 1);
  }
  return count;
}

int materialOdds(Board& board) {}
int knightOdds(Board& board) {}

int evaluate(Board&board) {
  return materialOdds(board);
}

