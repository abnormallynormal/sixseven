#include "moveGen.h"
#include "board.h"
#include "bitboard_masks.h"

void MoveGenerator::generateKnightMoves(Board &board)
{
  u64 knights = board.whiteToMove ? board.bitboards[wKnight] : board.bitboards[bKnight];
  while (knights)
  {
    int knightPos = __builtin_ctzll(knights);
    u64 ownPieces = board.whiteToMove ? board.whitePieces : board.blackPieces;
    u64 moves = knightAttackTable[knightPos] & ~ownPieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      addMove(Move(knightPos, to));
      moves &= moves - 1;
    }
    knights &= knights - 1;
  }
}
void MoveGenerator::generateKingMoves(Board &board)
{
  u64 king = board.whiteToMove ? board.bitboards[wKing] : board.bitboards[bKing];
  while (king)
  {
    int kingPos = __builtin_ctzll(king);
    u64 ownPieces = board.whiteToMove ? board.whitePieces : board.blackPieces;
    u64 moves = kingAttackTable[kingPos] & ~ownPieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      addMove(Move(kingPos, to));
      moves &= moves - 1;
    }
    king &= king - 1;
  }
}
void MoveGenerator::generateRookMoves(Board &board)
{
  u64 rook = board.whiteToMove ? board.bitboards[wRook] : board.bitboards[bRook];
  while (rook)
  {
    int rookPos = __builtin_ctzll(rook);
    u64 ownPieces = board.whiteToMove ? board.whitePieces : board.blackPieces;
    u64 moves = getRookAttacks(rookPos, board) & ~ownPieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      addMove(Move(rookPos, to));
      moves &= moves - 1;
    }
    rook &= rook - 1;
  }
}
void MoveGenerator::generateBishopMoves(Board &board)
{
  u64 bishop = board.whiteToMove ? board.bitboards[wBishop] : board.bitboards[bBishop];
  while (bishop)
  {
    int bishopPos = __builtin_ctzll(bishop);
    u64 ownPieces = board.whiteToMove ? board.whitePieces : board.blackPieces;
    u64 moves = getBishopAttacks(bishopPos, board) & ~ownPieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      addMove(Move(bishopPos, to));
      moves &= moves - 1;
    }
    bishop &= bishop - 1;
  }
}
void MoveGenerator::generateQueenMoves(Board &board)
{
  u64 queen = board.whiteToMove ? board.bitboards[wQueen] : board.bitboards[bQueen];
  while (queen)
  {
    int queenPos = __builtin_ctzll(queen);
    u64 ownPieces = board.whiteToMove ? board.whitePieces : board.blackPieces;
    u64 moves = getQueenAttacks(queenPos, board) & ~ownPieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      addMove(Move(queenPos, to));
      moves &= moves - 1;
    }
    queen &= queen - 1;
  }
}
void MoveGenerator::generatePawnMoves(Board &board)
{
  u64 pawns = board.whiteToMove ? board.bitboards[wPawn] : board.bitboards[bPawn];
  while (pawns)
  {
    int pawnPos = __builtin_ctzll(pawns);
    u64 pawnPosBit = 1ULL << pawnPos;
    bool isWhite = board.whiteToMove ? true : false;
    u64 oppPieces = isWhite ? board.blackPieces : board.whitePieces;
    u64 ownPieces = !isWhite ? board.blackPieces : board.whitePieces;
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
    u64 pawnPush = isWhite ? ((1ULL << pawnPos + 8) & ~(oppPieces | ownPieces)) : ((1ULL << pawnPos - 8) & ~(oppPieces | ownPieces));
    if (pawnPush != 0ULL)
    {
      int to = __builtin_ctzll(pawnPush);
      if ((pawnPush & promotionRanks) == 0ULL)
      {
        addMove(Move(pawnPos, to));
      }
      else
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
    }
    if (isWhite && ((pawnPosBit & secondRank) != 0ULL))
    {
      if ((pawnPosBit << 8 & ~(oppPieces | ownPieces)) != 0ULL && (pawnPosBit << 16 & ~(oppPieces | ownPieces)) != 0ULL)
      {
        int to = __builtin_ctzll(((pawnPosBit << 16) & ~(oppPieces | ownPieces)));
        addMove(Move(pawnPos, to, pawnPos + 8));
      }
    }
    else if (!isWhite && ((pawnPosBit & seventhRank) != 0ULL))
    {
      if ((pawnPosBit >> 8 & ~(oppPieces | ownPieces)) != 0ULL && (pawnPosBit >> 16 & ~(oppPieces | ownPieces)) != 0ULL)
      {
        int to = __builtin_ctzll(((pawnPosBit >> 16) & ~(oppPieces | ownPieces)));
        addMove(Move(pawnPos, to, pawnPos - 8));
      }
    }

    pawns &= pawns - 1;
  }
}
