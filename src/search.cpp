#include "search.h"
#include "constants.h"
#include "evaluation.h"
#include <climits>
#include <algorithm>

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
int negamax(Board &board, MoveGenerator &moveGen, int alpha, int beta, int depth, int ply)
{
  int originalAlpha = alpha;
  Move bestMove;

  int entryScore;
  Move entryMove;
  if (probeEntry(board.hash, depth, alpha, beta, ply, entryScore, entryMove))
  {
    return entryScore;
  }
  if (depth == 0)
    return quiescence(board, moveGen, alpha, beta, ply);
  int greatestValue = INT_MIN;
  moveGen.generateMoves(board, ply);
  int scores[218] = {0};
  for (int i = 0; i < moveGen.moveLists[ply].count; i++)
  {
    if (board.isSameMove(moveGen.moveLists[ply].moves[i], entryMove)) scores[i] = INT_MAX;
    else
      scores[i] = scoreMove(board, moveGen.moveLists[ply].moves[i]);
  }
  for (int i = 0; i < moveGen.moveLists[ply].count; i++)
  {
    int best = i;
    for (int j = i + 1; j < moveGen.moveLists[ply].count; j++)
    {
      if (scores[j] > scores[i])
      {
        best = j;
      }
      std::swap(scores[i], scores[best]);
      std::swap(moveGen.moveLists[ply].moves[i], moveGen.moveLists[ply].moves[best]);
    }

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
      bestMove = m;
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
      return -MATE_THRESHOLD + ply;
    }
    else
      return 0;
  }
  int nodeType;
  if (greatestValue <= originalAlpha)
    nodeType = UPPER;
  else if (greatestValue >= beta)
    nodeType = LOWER;
  else
    nodeType = EXACT;
  storeEntry(board.hash, greatestValue, depth, bestMove, nodeType, ply);
  return greatestValue;
}

Move rootNegamax(Board &board, MoveGenerator &moveGen, int depth)
{
  Move bestMove;
  int bestScore = INT_MIN;

  moveGen.generateMoves(board, 0);
  for (int i = 0; i < moveGen.moveLists[0].count; i++)
  {
    Move m = moveGen.moveLists[0].moves[i];
    board.makeMove(m);
    int score = -negamax(board, moveGen, INT_MIN, INT_MAX, depth - 1, 1);
    board.unmakeMove(m);

    if (score > bestScore)
    {
      bestScore = score;
      bestMove = m;
    }
  }
  return bestMove;
}

int quiescence(Board &board, MoveGenerator &moveGen, int alpha, int beta, int ply)
{
  bool isInCheck = moveGen.isInCheck(board, board.isWhiteToMove());
  int greatestVal = INT_MIN;
  if (!isInCheck)
  {
    int standPat = evaluate(board);
    if (standPat > greatestVal)
      greatestVal = standPat;
    if (standPat > alpha)
      alpha = standPat;
    if (standPat >= beta)
      return standPat;
  }

  if (isInCheck)
  {
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
      int score = -quiescence(board, moveGen, -beta, -alpha, ply + 1);
      board.unmakeMove(m);
      if (score >= beta)
        return score;
      if (score > greatestVal)
        greatestVal = score;
      if (score > alpha)
        alpha = score;
    }
  }
  else
  {
    moveGen.generateCaptures(board, ply);
    for (int i = 0; i < moveGen.moveLists[ply].count; i++)
    {
      Move m = moveGen.moveLists[ply].moves[i];
      board.makeMove(m);
      if (moveGen.isInCheck(board, !board.isWhiteToMove()))
      {
        board.unmakeMove(m);
        continue;
      }
      int score = -quiescence(board, moveGen, -beta, -alpha, ply + 1);
      board.unmakeMove(m);
      if (score >= beta)
        return score;
      if (score > greatestVal)
        greatestVal = score;
      if (score > alpha)
        alpha = score;
    }
  }
  if (greatestVal == INT_MIN && isInCheck)
    return -MATE_THRESHOLD + ply;
  return greatestVal;
}