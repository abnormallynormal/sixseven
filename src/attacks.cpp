#include "moveGen.h"
#include "board.h"
#include "bitboard_masks.h"
#include "magics.h"

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
