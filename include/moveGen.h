#pragma once

#include <vector>
#include "types.h"
#include "move.h"

class Board;

class MoveGenerator
{
public:
  MoveList moveLists[64];
  int currentPly;
  MoveGenerator();
  ~MoveGenerator();
  u64 knightAttackTable[64];
  u64 kingAttackTable[64];
  u64 **rookAttackTable;
  u64 **bishopAttackTable;
  u64 rookMasks[64];
  u64 bishopMasks[64];
  u64 whitePawnAttackTable[64];
  u64 blackPawnAttackTable[64];
  void precomputeKnightAttacks();
  void precomputeKingAttacks();
  void precomputeWhitePawnAttacks();
  void precomputeBlackPawnAttacks();
  void precomputeRookMasks();
  void precomputeBishopMasks();
  void precomputeRookAttacks();
  void precomputeBishopAttacks();
  u64 calculateRookAttacks(int sq, u64 blockers);
  u64 calculateBishopAttacks(int sq, u64 blockers);
  u64 getRookAttacks(int sq, Board &board);
  u64 getBishopAttacks(int sq, Board &board);
  u64 getQueenAttacks(int sq, Board &board);
  void init()
  {
    precomputeKnightAttacks();
    precomputeKingAttacks();
    precomputeWhitePawnAttacks();
    precomputeBlackPawnAttacks();
    precomputeRookAttacks();
    precomputeBishopAttacks();
  }
  void generateKnightMoves(Board &board);
  void generateKingMoves(Board &board);
  void generatePawnMoves(Board &board);
  void generateRookMoves(Board &board);
  void generateBishopMoves(Board &board);
  void generateQueenMoves(Board &board);
  void addMove(Move m)
  {
    moveLists[currentPly].moves[moveLists[currentPly].count++] = m;
  }
  void generateMoves(Board &board, int ply);
  void generateKnightCaptures(Board &board);
  void generateKingCaptures(Board &board);
  void generatePawnCaptures(Board &board);
  void generateRookCaptures(Board &board);
  void generateBishopCaptures(Board &board);
  void generateQueenCaptures(Board &board);
  void generateCaptures(Board &board, int ply);
  bool isAttacked(Board &board, bool white, int sq);
  bool isInCheck(Board &board, bool white);
};
