#pragma once

#include "types.h"

struct Undo
{
  u64 hash;
  int castlingRights;
  int enPassantSquare;
  int halfMoveCount;
  Piece capturedPiece;
  bool whiteToMove;
  Undo() = default;
  Undo(u64 hash, int castling, int ep, int count, Piece captured, bool white) : hash(hash), castlingRights(castling), enPassantSquare(ep), halfMoveCount(count), capturedPiece(captured), whiteToMove(white) {};
};

struct Move
{
  int to;
  int from;
  bool isCastling;
  bool isKingside;
  bool isEnPassant;
  Piece promotionPiece;
  Undo prevState;
  int setEpSquare;
  Move() : from(0), to(0), setEpSquare(-1), isCastling(false), isKingside(false), isEnPassant(false), promotionPiece(EMPTY) {};
  Move(int from, int to) : from(from), to(to), setEpSquare(-1), isCastling(false), isKingside(false), isEnPassant(false), promotionPiece(EMPTY) {};
  Move(int from, int to, Piece promotion) : from(from), to(to), promotionPiece(promotion), isCastling(false), isEnPassant(false), setEpSquare(-1) {};
  Move(int from, int to, bool isEp) : from(from), to(to), isEnPassant(isEp), isCastling(false), setEpSquare(-1), promotionPiece(EMPTY) {};
  Move(int from, int to, int epSquare) : from(from), to(to), setEpSquare(epSquare), isCastling(false), isEnPassant(false), promotionPiece(EMPTY) {};

  Move(bool isCastling, bool kingside) : from(0), to(0), isCastling(isCastling), isKingside(kingside), isEnPassant(false), promotionPiece(EMPTY), setEpSquare(-1) {};
};

struct MoveList
{
  Move moves[218];
  int count = 0;
};
