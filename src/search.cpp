#include "search.h"
#include "board.h"
#include "evaluation.h"
#include "moveGen.h"
#include <climits>

int scoreMove(Board &board, Move m)
{
  if (board.squares[m.to] != EMPTY)
  {

    return mvv_lva(board.squares[m.from], board.squares[m.to]);
  }
  else
    return 0;
}
int mvv_lva(Piece attack, Piece victim) { return pieceVals[victim] - pieceVals[attack]; }
int negamax(Board &board, MoveGenerator &moveGen, int alpha, int beta, int depth, int ply = 0)
{
  if (depth == 0)
    return evaluate(board);
  int greatestValue = INT_MIN;
  moveGen.generateMoves(board, ply);
  for (int i = 0; i < moveGen.moveLists[ply].count; i++)
  {
    Move m = moveGen.moveLists[ply].moves[i];
    board.makeMove(m);
    if (moveGen.isInCheck(board, !board.isWhiteToMove()))
    {
      board.unmakeMove(m);
      continue;
    }
    int score = -negamax(board, moveGen, -beta, -alpha, depth - 1, ply + 1);
    if (score > greatestValue)
    {
      greatestValue = score;
      if (score > alpha)
      {
        alpha = score;
      }
    }
    if (score >= beta)
    {
      board.unmakeMove(m);
      break;
    }
    board.unmakeMove(m);
  }
  if (greatestValue == INT_MIN)
  {
    if (moveGen.isInCheck(board, board.isWhiteToMove()))
    {
      return -100000 * ply;
    }
    else
      return 0;
  }
  return greatestValue;
}