#pragma once

#include <cstdint>
typedef uint64_t u64;
typedef uint8_t u8;

enum Piece : int
{
  wPawn = 0,
  wKnight,
  wBishop,
  wRook,
  wQueen,
  wKing,
  bPawn,
  bKnight,
  bBishop,
  bRook,
  bQueen,
  bKing,
  EMPTY,
};

enum CastlingRights : int {
  wK = 1,
  wQ = 2,
  bK = 4,
  bQ = 8
};
