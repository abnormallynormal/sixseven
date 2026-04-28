#ifndef PERFT_H
#define PERFT_H
#include "moveGen.h"
#include "board.h"
#include "string"
#include <iostream>
std::string moveToString(Move &m)
{
  if (m.isCastling)
  {
    if (m.isKingside)
      return "e1g1"; // or e8g8, doesn't matter for split debugging
    else
      return "e1c1";
  }
  std::string result = "";
  result += (char)('a' + (m.from % 8));
  result += (char)('1' + (m.from / 8));
  result += (char)('a' + (m.to % 8));
  result += (char)('1' + (m.to / 8));
  return result;
}

u64 perft(Board &board, int depth, bool white, MoveGenerator &moveGen, int ply = 0)
{
  if (depth == 0)
    return 1;

  moveGen.generateMoves(board, ply);
  int count = moveGen.moveLists[ply].count;

  u64 nodes = 0;
  for (int i = 0; i < count; i++)
  {
    Move m = moveGen.moveLists[ply].moves[i];
    board.makeMove(m);
    if (!moveGen.isInCheck(board, white))
    {
      int currentNodes = perft(board, depth - 1, !white, moveGen, ply + 1);
      // if(ply == 0)
      //   std::cout << "\n" << moveToString(m) << " " << currentNodes;
      nodes += currentNodes;
    }
    board.unmakeMove(m);
  }
  return nodes;
}

#endif
