#include "moveGen.h"
#include "board.h"

MoveGenerator::MoveGenerator()
{
  rookAttackTable = new u64 *[64];
  for (int i = 0; i < 64; i++)
    rookAttackTable[i] = new u64[4096];

  bishopAttackTable = new u64 *[64];
  for (int i = 0; i < 64; i++)
    bishopAttackTable[i] = new u64[512];

  init();
}

MoveGenerator::~MoveGenerator()
{
  for (int i = 0; i < 64; i++)
    delete[] rookAttackTable[i];
  delete[] rookAttackTable;

  for (int i = 0; i < 64; i++)
    delete[] bishopAttackTable[i];
  delete[] bishopAttackTable;
}

void MoveGenerator::generateMoves(Board &board, int ply)
{
  currentPly = ply;
  moveLists[ply].count = 0;
  generateKnightMoves(board);
  generateKingMoves(board);
  generatePawnMoves(board);
  generateRookMoves(board);
  generateBishopMoves(board);
  generateQueenMoves(board);
  if (board.whiteToMove)
  {
    if ((board.castlingRights & wK) && !isAttacked(board, true, 4) && !isAttacked(board, true, 5) && !isAttacked(board, true, 6) && board.squares[5] == EMPTY && board.squares[6] == EMPTY)
    {
      addMove(Move(true, true));
    }
    if ((board.castlingRights & wQ) && !isAttacked(board, true, 4) && !isAttacked(board, true, 3) && !isAttacked(board, true, 2) && board.squares[3] == EMPTY && board.squares[2] == EMPTY && board.squares[1] == EMPTY)
    {
      addMove(Move(true, false));
    }
  }
  else
  {
    if ((board.castlingRights & bK) && !isAttacked(board, false, 60) && !isAttacked(board, false, 61) && !isAttacked(board, false, 62) && board.squares[61] == EMPTY && board.squares[62] == EMPTY)
    {
      addMove(Move(true, true));
    }
    if ((board.castlingRights & bQ) && !isAttacked(board, false, 60) && !isAttacked(board, false, 59) && !isAttacked(board, false, 58) && board.squares[59] == EMPTY && board.squares[58] == EMPTY && board.squares[57] == EMPTY)
    {
      addMove(Move(true, false));
    }
  }
}
void MoveGenerator::generateCaptures(Board &board, int ply){
  currentPly = ply;
  moveLists[ply].count = 0;
  generateKnightCaptures(board);
  generateKingCaptures(board);
  generatePawnCaptures(board);
  generateRookCaptures(board);
  generateBishopCaptures(board);
  generateQueenCaptures(board);
}
