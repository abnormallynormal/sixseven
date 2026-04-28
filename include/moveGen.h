#ifndef MOVEGEN_H
#define MOVEGEN_H
#include <vector>
#include "types.h"
class Board;
struct Undo
{
  int castlingRights;
  int enPassantSquare;
  int halfMoveCount;
  Piece capturedPiece;
  bool whiteToMove;
  Undo() = default;
  Undo(int castling, int ep, int count, Piece captured, bool white) : castlingRights(castling), enPassantSquare(ep), halfMoveCount(count), capturedPiece(captured), whiteToMove(white) {};
};

struct Move
{
  int to;
  int from;
  bool isCastling;
  bool isKingside;
  bool isEnPassant;
  Piece promotionPiece;
  Piece piece;
  Undo prevState;
  int setEpSquare;
  Move() : from(0), to(0), setEpSquare(-1), isCastling(false), isKingside(false), isEnPassant(false), promotionPiece(EMPTY), piece(EMPTY) {};
  Move(int from, int to) : from(from), to(to), setEpSquare(-1), isCastling(false), isKingside(false), isEnPassant(false) {};
  Move(int from, int to, Piece promotion) : from(from), to(to), promotionPiece(promotion), isCastling(false), isEnPassant(false), setEpSquare(-1) {};
  Move(int from, int to, bool isEp) : from(from), to(to), isEnPassant(isEp), isCastling(false), setEpSquare(-1) {};
  Move(int from, int to, int epSquare) : from(from), to(to), setEpSquare(epSquare), isCastling(false), isEnPassant(false) {};

  Move(bool isCastling, bool kingside) : from(0), to(0), isCastling(isCastling), isKingside(kingside), isEnPassant(false), promotionPiece(EMPTY), piece(EMPTY), setEpSquare(-1) {};
};

struct MoveList
{
  Move moves[218];
  int count = 0;
};
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
  bool isAttacked(Board &board, bool white, int sq);
  bool isInCheck(Board &board, bool white);
};
#endif