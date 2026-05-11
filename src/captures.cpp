#include "moveGen.h"
#include "board.h"
#include "bitboard_masks.h"

void MoveGenerator::generateKnightCaptures(Board &board)
{
  u64 knights = board.whiteToMove ? board.bitboards[wKnight] : board.bitboards[bKnight];
  while (knights)
  {
    int knightPos = __builtin_ctzll(knights);
    u64 oppPieces = !board.whiteToMove ? board.whitePieces : board.blackPieces;
    u64 moves = knightAttackTable[knightPos] & oppPieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      addMove(Move(knightPos, to));
      moves &= moves - 1;
    }
    knights &= knights - 1;
  }
}
void MoveGenerator::generateKingCaptures(Board &board)
{
  u64 king = board.whiteToMove ? board.bitboards[wKing] : board.bitboards[bKing];
  while (king)
  {
    int kingPos = __builtin_ctzll(king);
    u64 oppPieces = !board.whiteToMove ? board.whitePieces : board.blackPieces;
    u64 moves = kingAttackTable[kingPos] & oppPieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      addMove(Move(kingPos, to));
      moves &= moves - 1;
    }
    king &= king - 1;
  }
}
void MoveGenerator::generateRookCaptures(Board &board)
{
  u64 rook = board.whiteToMove ? board.bitboards[wRook] : board.bitboards[bRook];
  while (rook)
  {
    int rookPos = __builtin_ctzll(rook);
    u64 oppPieces = !board.whiteToMove ? board.whitePieces : board.blackPieces;
    u64 moves = getRookAttacks(rookPos, board) & oppPieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      addMove(Move(rookPos, to));
      moves &= moves - 1;
    }
    rook &= rook - 1;
  }
}
void MoveGenerator::generateBishopCaptures(Board &board)
{
  u64 bishop = board.whiteToMove ? board.bitboards[wBishop] : board.bitboards[bBishop];
  while (bishop)
  {
    int bishopPos = __builtin_ctzll(bishop);
    u64 oppPieces = !board.whiteToMove ? board.whitePieces : board.blackPieces;
    u64 moves = getBishopAttacks(bishopPos, board) & oppPieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      addMove(Move(bishopPos, to));
      moves &= moves - 1;
    }
    bishop &= bishop - 1;
  }
}
void MoveGenerator::generateQueenCaptures(Board &board)
{
  u64 queen = board.whiteToMove ? board.bitboards[wQueen] : board.bitboards[bQueen];
  while (queen)
  {
    int queenPos = __builtin_ctzll(queen);
    u64 oppPieces = !board.whiteToMove ? board.whitePieces : board.blackPieces;
    u64 moves = getQueenAttacks(queenPos, board) & oppPieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      addMove(Move(queenPos, to));
      moves &= moves - 1;
    }
    queen &= queen - 1;
  }
}
void MoveGenerator::generatePawnCaptures(Board &board)
{
  u64 pawns = board.whiteToMove ? board.bitboards[wPawn] : board.bitboards[bPawn];
  while (pawns)
  {
    int pawnPos = __builtin_ctzll(pawns);
    u64 pawnPosBit = 1ULL << pawnPos;
    bool isWhite = board.whiteToMove ? true : false;
    u64 oppPieces = isWhite ? board.blackPieces : board.whitePieces;
    u64 captures = isWhite ? whitePawnAttackTable[pawnPos] & oppPieces : blackPawnAttackTable[pawnPos] & oppPieces;
    if (board.enPassantSquare != NO_SQUARE)
    {
      u64 ep = 1ULL << board.enPassantSquare;
      if (isWhite)
      {
        ep &= whitePawnAttackTable[pawnPos];
      }
      else
      {
        ep &= blackPawnAttackTable[pawnPos];
      }
      if (ep != 0ULL)
      {
        int to = __builtin_ctzll(ep);
        addMove(Move(pawnPos, to, true));
      }
    }
    while (captures)
    {
      int to = __builtin_ctzll(captures);
      u64 toBit = 1ULL << to;
      if (toBit & promotionRanks)
      {
        if (isWhite)
        {
          addMove(Move(pawnPos, to, wKnight));
          addMove(Move(pawnPos, to, wBishop));
          addMove(Move(pawnPos, to, wRook));
          addMove(Move(pawnPos, to, wQueen));
        }
        else
        {
          addMove(Move(pawnPos, to, bKnight));
          addMove(Move(pawnPos, to, bBishop));
          addMove(Move(pawnPos, to, bRook));
          addMove(Move(pawnPos, to, bQueen));
        }
      }
      else
      {
        addMove(Move(pawnPos, to));
      }
      captures &= captures - 1;
    }
    pawns &= pawns - 1;
  }
}
