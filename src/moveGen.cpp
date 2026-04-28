#include "moveGen.h"
#include "board.h"
#include "constants.h"
#include <bits/stdc++.h>

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

void MoveGenerator::precomputeKnightAttacks()
{
  for (int i = 0; i < 64; i++)
  {
    u64 knightPos = 1ULL << i;
    u64 nnE = (knightPos << 17) & notAFile;
    u64 neE = (knightPos << 10) & notABFile;
    u64 seE = (knightPos >> 6) & notABFile;
    u64 ssE = (knightPos >> 15) & notAFile;
    u64 ssW = (knightPos >> 17) & notHFile;
    u64 swW = (knightPos >> 10) & notGHFile;
    u64 nwW = (knightPos << 6) & notGHFile;
    u64 nnW = (knightPos << 15) & notHFile;
    u64 knightAttacks = nnE | neE | seE | ssE | ssW | swW | nwW | nnW;
    knightAttackTable[i] = knightAttacks;
  }
}
void MoveGenerator::precomputeKingAttacks()
{
  for (int i = 0; i < 64; i++)
  {
    u64 kingPos = 1ULL << i;
    u64 ne = (kingPos << 9) & notAFile;
    u64 e = (kingPos << 1) & notAFile;
    u64 se = (kingPos >> 7) & notAFile;
    u64 s = (kingPos >> 8);
    u64 sw = (kingPos >> 9) & notHFile;
    u64 w = (kingPos >> 1) & notHFile;
    u64 nw = (kingPos << 7) & notHFile;
    u64 n = (kingPos << 8);
    u64 kingAttacks = ne | e | se | s | sw | w | nw | n;
    kingAttackTable[i] = kingAttacks;
  }
}
void MoveGenerator::precomputeWhitePawnAttacks()
{
  for (int i = 8; i < 56; i++)
  {
    u64 pawnPos = 1ULL << i;
    u64 ne = (pawnPos << 9) & notAFile;
    u64 nw = (pawnPos << 7) & notHFile;
    u64 whitePawnAttacks = ne | nw;
    whitePawnAttackTable[i] = whitePawnAttacks;
  }
}
void MoveGenerator::precomputeBlackPawnAttacks()
{
  for (int i = 8; i < 56; i++)
  {
    u64 pawnPos = 1ULL << i;
    u64 se = (pawnPos >> 7) & notAFile;
    u64 sw = (pawnPos >> 9) & notHFile;
    u64 blackPawnAttacks = se | sw;
    blackPawnAttackTable[i] = blackPawnAttacks;
  }
}
void MoveGenerator::precomputeRookMasks()
{
  for (int i = 0; i < 64; i++)
  {
    int x = i % 8;
    int y = i / 8;
    u64 mask = 0ULL;
    for (int j = 0; j < 7 - y - 1; j++)
    {
      mask |= (1ULL << (i + 8 * (j + 1)));
    }
    for (int j = 0; j < y - 1; j++)
    {
      mask |= (1ULL << (i - 8 * (j + 1)));
    }
    for (int j = 0; j < 7 - x - 1; j++)
    {
      mask |= (1ULL << (i + j + 1));
    }
    for (int j = 0; j < x - 1; j++)
    {
      mask |= (1ULL << (i - j - 1));
    }
    rookMasks[i] = mask;
  }
}
void MoveGenerator::precomputeBishopMasks()
{
  for (int i = 0; i < 64; i++)
  {
    int x = i % 8;
    int y = i / 8;
    int neBound = std::min(7 - x - 1, 7 - y - 1);
    int nwBound = std::min(x - 1, 7 - y - 1);
    int seBound = std::min(7 - x - 1, y - 1);
    int swBound = std::min(x - 1, y - 1);
    u64 mask = 0ULL;
    for (int j = 0; j < neBound; j++)
    {
      mask |= (1ULL << (i + 9 * (j + 1)));
    }
    for (int j = 0; j < nwBound; j++)
    {
      mask |= (1ULL << (i + 7 * (j + 1)));
    }
    for (int j = 0; j < seBound; j++)
    {
      mask |= (1ULL << (i - 7 * (j + 1)));
    }
    for (int j = 0; j < swBound; j++)
    {
      mask |= (1ULL << (i - 9 * (j + 1)));
    }
    bishopMasks[i] = mask;
  }
}
void MoveGenerator::precomputeRookAttacks()
{
  precomputeRookMasks();
  for (int i = 0; i < 64; i++)
  {
    u64 mask = rookMasks[i];
    u64 subset = 0;
    do
    {
      u64 attacks = calculateRookAttacks(i, subset);
      u64 index = (subset * rookMagic[i]) >> (64 - rookShifts[i]);
      rookAttackTable[i][index] = attacks;
      subset = (subset - mask) & mask;
    } while (subset != 0);
  }
}
void MoveGenerator::precomputeBishopAttacks()
{
  precomputeBishopMasks();
  for (int i = 0; i < 64; i++)
  {
    u64 mask = bishopMasks[i];
    u64 subset = 0;
    do
    {
      u64 attacks = calculateBishopAttacks(i, subset);
      u64 index = (subset * bishopMagic[i]) >> (64 - bishopShifts[i]);
      bishopAttackTable[i][index] = attacks;
      subset = (subset - mask) & mask;
    } while (subset != 0);
  }
}
u64 MoveGenerator::calculateRookAttacks(int sq, u64 blockers)
{
  u64 attacks = 0ULL;
  int file = sq % 8;
  int rank = sq / 8;
  for (int r = rank + 1; r <= 7; r++)
  {
    attacks |= (1ULL << (r * 8 + file));
    if (blockers & (1ULL << (r * 8 + file)))
      break;
  }
  for (int r = rank - 1; r >= 0; r--)
  {
    attacks |= (1ULL << (r * 8 + file));
    if (blockers & (1ULL << (r * 8 + file)))
      break;
  }
  for (int f = file + 1; f <= 7; f++)
  {
    attacks |= (1ULL << (rank * 8 + f));
    if (blockers & (1ULL << (rank * 8 + f)))
      break;
  }
  for (int f = file - 1; f >= 0; f--)
  {
    attacks |= (1ULL << (rank * 8 + f));
    if (blockers & (1ULL << (rank * 8 + f)))
      break;
  }
  return attacks;
}
u64 MoveGenerator::calculateBishopAttacks(int sq, u64 blockers)
{
  u64 attacks = 0ULL;
  int file = sq % 8;
  int rank = sq / 8;

  int neBound = std::min(7 - rank, 7 - file);
  int nwBound = std::min(rank, 7 - file);
  int seBound = std::min(7 - rank, file);
  int swBound = std::min(rank, file);
  u64 mask = 0ULL;
  for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++)
  {
    attacks |= (1ULL << (r * 8 + f));
    if (blockers & (1ULL << (r * 8 + f)))
      break;
  }
  for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--)
  {
    attacks |= (1ULL << (r * 8 + f));
    if (blockers & (1ULL << (r * 8 + f)))
      break;
  }
  for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++)
  {
    attacks |= (1ULL << (r * 8 + f));
    if (blockers & (1ULL << (r * 8 + f)))
      break;
  }
  for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--)
  {
    attacks |= (1ULL << (r * 8 + f));
    if (blockers & (1ULL << (r * 8 + f)))
      break;
  }
  return attacks;
}
u64 MoveGenerator::getRookAttacks(int sq, Board &board)
{
  u64 blockers = (board.whitePieces | board.blackPieces) & rookMasks[sq];
  u64 index = (blockers * rookMagic[sq]) >> (64 - rookShifts[sq]);
  u64 maxIndex = 1ULL << rookShifts[sq];
  if (index >= maxIndex)
  {
    std::cerr << "Rook attack OOB! sq=" << sq
              << " index=" << index
              << " maxIndex=" << maxIndex
              << " blockers=" << blockers << std::endl;
    throw std::runtime_error("Rook OOB");
  }
  return rookAttackTable[sq][index];
}
u64 MoveGenerator::getBishopAttacks(int sq, Board &board)
{
  u64 blockers = (board.whitePieces | board.blackPieces) & bishopMasks[sq];
  u64 index = (blockers * bishopMagic[sq]) >> (64 - bishopShifts[sq]);
  return bishopAttackTable[sq][index];
}
u64 MoveGenerator::getQueenAttacks(int sq, Board &board)
{
  return getRookAttacks(sq, board) | getBishopAttacks(sq, board);
}
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
bool MoveGenerator::isAttacked(Board &board, bool white, int sq)
{
  if (white)
  {
    if (knightAttackTable[sq] & board.bitboards[bKnight])
      return true;
    if (kingAttackTable[sq] & board.bitboards[bKing])
      return true;
    if (sq < 56)
    {
      u64 potentialBlackPawns = ((1ULL << (sq + 7)) & notHFile) | ((1ULL << (sq + 9)) & notAFile);
      if (board.bitboards[bPawn] & potentialBlackPawns)
        return true;
    }
    if (getRookAttacks(sq, board) & (board.bitboards[bRook] | board.bitboards[bQueen]))
      return true;
    if (getBishopAttacks(sq, board) & (board.bitboards[bBishop] | board.bitboards[bQueen]))
      return true;
  }
  else
  {
    if (knightAttackTable[sq] & board.bitboards[wKnight])
      return true;
    if (kingAttackTable[sq] & board.bitboards[wKing])
      return true;
    if (sq >= 8)
    {
      u64 potentialWhitePawns = ((1ULL << (sq - 7)) & notAFile) | ((1ULL << (sq - 9)) & notHFile);
      if (board.bitboards[wPawn] & potentialWhitePawns)
        return true;
    }
    if (getRookAttacks(sq, board) & (board.bitboards[wRook] | board.bitboards[wQueen]))
      return true;
    if (getBishopAttacks(sq, board) & (board.bitboards[wBishop] | board.bitboards[wQueen]))
      return true;
  }
  return false;
}
bool MoveGenerator::isInCheck(Board &board, bool white)
{
  int kingSq = __builtin_ctzll(white ? board.bitboards[wKing] : board.bitboards[bKing]);

  if (white)
  {
    if (knightAttackTable[kingSq] & board.bitboards[bKnight])
      return true;
    if (kingAttackTable[kingSq] & board.bitboards[bKing])
      return true;
    u64 potentialBlackPawns = 0;
    if (kingSq < 56)
    {
      potentialBlackPawns |= ((1ULL << (kingSq + 7)) & notHFile) | ((1ULL << (kingSq + 9)) & notAFile);
      if (board.bitboards[bPawn] & potentialBlackPawns)
        return true;
    }
    if (getRookAttacks(kingSq, board) & (board.bitboards[bRook] | board.bitboards[bQueen]))
      return true;
    if (getBishopAttacks(kingSq, board) & (board.bitboards[bBishop] | board.bitboards[bQueen]))
      return true;
  }
  else
  {
    if (knightAttackTable[kingSq] & board.bitboards[wKnight])
      return true;
    if (kingAttackTable[kingSq] & board.bitboards[wKing])
      return true;
    u64 potentialWhitePawns = 0;
    if (kingSq >= 8)
    {
      potentialWhitePawns |= ((1ULL << (kingSq - 7)) & notAFile) | ((1ULL << (kingSq - 9)) & notHFile);
      if (board.bitboards[wPawn] & potentialWhitePawns)
        return true;
    }
    if (getRookAttacks(kingSq, board) & (board.bitboards[wRook] | board.bitboards[wQueen]))
      return true;
    if (getBishopAttacks(kingSq, board) & (board.bitboards[wBishop] | board.bitboards[wQueen]))
      return true;
  }
  return false;
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