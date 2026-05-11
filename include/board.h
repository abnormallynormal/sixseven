#pragma once

#include "types.h"
#include "square.h"
#include "move.h"

class Board
{

public:
  u64 bitboards[12];
  u64 whitePieces;
  u64 blackPieces;
  u64 occupiedSquares;
  bool whiteToMove;
  int castlingRights;
  int halfMoveCount;
  int enPassantSquare;
  u64 hash;
  Piece squares[64];

  Board();
  void printBoard();
  void makeMove(Move &m);
  void updateHash(Move &m);
  void setColorToMove(bool white)
  {
    whiteToMove = white;
  }
  bool isWhiteToMove() const { return whiteToMove; }
  void unmakeMove(Move &m);
  int getFile(int square) { return square % 8; }
  int getRank(int square) { return square / 8; }
  inline bool isSameMove(Move &a, Move &b)
  {
    if (a.isCastling != b.isCastling)
      return false;
    if (a.isKingside != b.isKingside)
      return false;
    if (a.promotionPiece != b.promotionPiece)
      return false;
    if (a.from != b.from)
      return false;
    if (a.to != b.to)
      return false;
    return true;
  }
  void updatePosition()
  {
    whitePieces = bitboards[wPawn] | bitboards[wKnight] | bitboards[wBishop] | bitboards[wRook] | bitboards[wQueen] | bitboards[wKing];
    blackPieces = bitboards[bPawn] | bitboards[bKnight] | bitboards[bBishop] | bitboards[bRook] | bitboards[bQueen] | bitboards[bKing];
    occupiedSquares = whitePieces | blackPieces;
  }
  u64 getOccupiedSquares() { return occupiedSquares; }
};
