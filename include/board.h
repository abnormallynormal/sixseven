#pragma once

#include "types.h"
#include "moveGen.h"
enum Square : int
{
  a1,
  b1,
  c1,
  d1,
  e1,
  f1,
  g1,
  h1,
  a2,
  b2,
  c2,
  d2,
  e2,
  f2,
  g2,
  h2,
  a3,
  b3,
  c3,
  d3,
  e3,
  f3,
  g3,
  h3,
  a4,
  b4,
  c4,
  d4,
  e4,
  f4,
  g4,
  h4,
  a5,
  b5,
  c5,
  d5,
  e5,
  f5,
  g5,
  h5,
  a6,
  b6,
  c6,
  d6,
  e6,
  f6,
  g6,
  h6,
  a7,
  b7,
  c7,
  d7,
  e7,
  f7,
  g7,
  h7,
  a8,
  b8,
  c8,
  d8,
  e8,
  f8,
  g8,
  h8,
  NO_SQUARE
};

class Board
{
  friend class MoveGenerator;

public:
  u64 bitboards[12];
  u64 whitePieces;
  u64 blackPieces;
  u64 occupiedSquares;
  bool whiteToMove;
  int castlingRights;
  int halfMoveCount;
  int enPassantSquare;
  Piece squares[64];


  Board();
  void printBoard();
  void makeMove(Move &m);
  void setColorToMove(bool white)
  {
    whiteToMove = white;
  }
  bool isWhiteToMove() const { return whiteToMove; }
  void unmakeMove(Move& m);
  int getFile(int square) { return square % 8; }
  int getRank(int square) { return square / 8; }

  void updatePosition()
  {
    whitePieces = bitboards[wPawn] | bitboards[wKnight] | bitboards[wBishop] | bitboards[wRook] | bitboards[wQueen] | bitboards[wKing];
    blackPieces = bitboards[bPawn] | bitboards[bKnight] | bitboards[bBishop] | bitboards[bRook] | bitboards[bQueen] | bitboards[bKing];
    occupiedSquares = whitePieces | blackPieces;
  }
  u64 getOccupiedSquares() { return occupiedSquares; }
};

