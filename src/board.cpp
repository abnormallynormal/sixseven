#include "board.h"
#include "types.h"
#include "constants.h"
#include <iostream>
Board::Board()
{
  castlingRights = 15;
  halfMoveCount = 0;
  Piece initial[64] = {
      // Rank 1
      // Rank 1
      wRook, EMPTY, EMPTY, wQueen, EMPTY, wRook, wKing, EMPTY,

      // Rank 2
      wPawn, bPawn, EMPTY, wPawn, EMPTY, EMPTY, wPawn, wPawn,

      // Rank 3
      bQueen, EMPTY, EMPTY, EMPTY, EMPTY, wKnight, EMPTY, EMPTY,

      // Rank 4
      wBishop, wBishop, wPawn, EMPTY, wPawn, EMPTY, EMPTY, EMPTY,

      // Rank 5
      bKnight, wPawn, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,

      // Rank 6
      EMPTY, bBishop, EMPTY, EMPTY, EMPTY, bKnight, bBishop, wKnight,

      // Rank 7
      wPawn, bPawn, bPawn, bPawn, EMPTY, bPawn, bPawn, bPawn,

      // Rank 8
      bRook, EMPTY, EMPTY, EMPTY, bKing, EMPTY, EMPTY, bRook};

  for (int i = 0; i < 64; i++)
    squares[i] = initial[i];

  for (int i = 0; i < 12; i++)
  {
    bitboards[i] = 0ULL;
  }

  for (int i = 0; i < 64; i++)
  {
    if (squares[i] != EMPTY)
    {
      bitboards[squares[i]] |= (1ULL << i);
    }
  }
  updatePosition();
  whiteToMove = true;
  castlingRights = wK | wQ | bK | bQ;
  halfMoveCount = 4;
  enPassantSquare = NO_SQUARE;
}

void Board::printBoard()
{
  for (int i = 7; i >= 0; i--)
  {
    for (int j = 0; j < 8; j++)
    {
      switch (squares[i * 8 + j])
      {
      case EMPTY:
        std::cout << ". ";
        break;
      case wRook:
        std::cout << "R ";
        break;
      case bRook:
        std::cout << "r ";
        break;
      case wKnight:
        std::cout << "N ";
        break;
      case bKnight:
        std::cout << "n ";
        break;
      case wBishop:
        std::cout << "B ";
        break;
      case bBishop:
        std::cout << "b ";
        break;
      case wQueen:
        std::cout << "Q ";
        break;
      case bQueen:
        std::cout << "q ";
        break;
      case wKing:
        std::cout << "K ";
        break;
      case bKing:
        std::cout << "k ";
        break;
      case wPawn:
        std::cout << "P ";
        break;
      case bPawn:
        std::cout << "p ";
        break;
      }
    }
    std::cout << "\n";
  }
}

void Board::makeMove(Move &m)
{
  if (halfMoveCount == 0)
  {
    m.prevState = Undo{15, NO_SQUARE, 0, EMPTY, true};
  }
  else
  {
    m.prevState = Undo{
        castlingRights,
        enPassantSquare,
        halfMoveCount,
        squares[m.to],
        whiteToMove};
  }

  if (m.isCastling)
  {
    if (m.isKingside && whiteToMove)
    {
      castlingRights &= ~(wK | wQ);
      bitboards[wKing] = (bitboards[wKing] & ~(1ULL << e1)) | (1ULL << g1);
      bitboards[wRook] = (bitboards[wRook] & ~(1ULL << h1)) | (1ULL << f1);
      squares[4] = EMPTY;
      squares[5] = wRook;
      squares[6] = wKing;
      squares[7] = EMPTY;
    }
    else if (!m.isKingside && whiteToMove)
    {
      castlingRights &= ~(wK | wQ);
      bitboards[wKing] = (bitboards[wKing] & ~(1ULL << e1)) | (1ULL << c1);
      bitboards[wRook] = (bitboards[wRook] & ~(1ULL << a1)) | (1ULL << d1);
      squares[0] = EMPTY;
      squares[1] = EMPTY;
      squares[2] = wKing;
      squares[3] = wRook;
      squares[4] = EMPTY;
    }
    else if (m.isKingside && !whiteToMove)
    {
      castlingRights &= ~(bK | bQ);
      bitboards[bKing] = (bitboards[bKing] & ~(1ULL << e8)) | (1ULL << g8);
      bitboards[bRook] = (bitboards[bRook] & ~(1ULL << h8)) | (1ULL << f8);
      squares[60] = EMPTY;
      squares[61] = bRook;
      squares[62] = bKing;
      squares[63] = EMPTY;
    }
    else if (!m.isKingside && !whiteToMove)
    {
      castlingRights &= ~(bK | bQ);
      bitboards[bKing] = (bitboards[bKing] & ~(1ULL << e8)) | (1ULL << c8);
      bitboards[bRook] = (bitboards[bRook] & ~(1ULL << a8)) | (1ULL << d8);
      squares[56] = EMPTY;
      squares[57] = EMPTY;
      squares[58] = bKing;
      squares[59] = bRook;
      squares[60] = EMPTY;
    }
  }
  else if (m.isEnPassant)
  {
    m.prevState.capturedPiece = whiteToMove ? bPawn : wPawn;
    u64 from = 1ULL << m.from;
    u64 to = 1ULL << m.to;
    u64 capturedSquare = whiteToMove ? 1ULL << (m.to - 8) : 1ULL << (m.to + 8);
    squares[m.to] = squares[m.from];
    squares[m.from] = EMPTY;
    if (whiteToMove)
    {
      bitboards[wPawn] &= ~from;
      bitboards[wPawn] |= to;
      bitboards[bPawn] &= ~capturedSquare;
      squares[m.to - 8] = EMPTY;
    }
    else
    {
      bitboards[bPawn] &= ~from;
      bitboards[bPawn] |= to;
      bitboards[wPawn] &= ~capturedSquare;
      squares[m.to + 8] = EMPTY;
    }
  }
  
  else
  {
    Piece pieceFrom = squares[m.from];
    Piece pieceTo = squares[m.to];
    u64 from = 1ULL << m.from;
    u64 to = 1ULL << m.to;
    squares[m.to] = squares[m.from];
    squares[m.from] = EMPTY;
    bitboards[pieceFrom] &= ~from;
    bitboards[pieceFrom] |= to;
    if (pieceTo != EMPTY)
    {
      bitboards[pieceTo] &= ~to;
    }
    castlingRights &= castlingRightsMask[m.from];
    castlingRights &= castlingRightsMask[m.to];
  }
  halfMoveCount++;
  whiteToMove = !whiteToMove;
  enPassantSquare = m.setEpSquare;
  updatePosition();
}

void Board::unmakeMove(Move &m)
{
  castlingRights = m.prevState.castlingRights;
  enPassantSquare = m.prevState.enPassantSquare;
  halfMoveCount = m.prevState.halfMoveCount;
  whiteToMove = m.prevState.whiteToMove;

  if (m.isCastling)
  {
    if (m.isKingside && whiteToMove)
    {
      bitboards[wKing] = (bitboards[wKing] & ~(1ULL << g1)) | (1ULL << e1);
      bitboards[wRook] = (bitboards[wRook] & ~(1ULL << f1)) | (1ULL << h1);
      squares[4] = wKing;
      squares[5] = EMPTY;
      squares[6] = EMPTY;
      squares[7] = wRook;
    }
    else if (!m.isKingside && whiteToMove)
    {
      bitboards[wKing] = (bitboards[wKing] & ~(1ULL << c1)) | (1ULL << e1);
      bitboards[wRook] = (bitboards[wRook] & ~(1ULL << d1)) | (1ULL << a1);
      squares[0] = wRook;
      squares[1] = EMPTY;
      squares[2] = EMPTY;
      squares[3] = EMPTY;
      squares[4] = wKing;
    }
    else if (m.isKingside && !whiteToMove)
    {
      bitboards[bKing] = (bitboards[bKing] & ~(1ULL << g8)) | (1ULL << e8);
      bitboards[bRook] = (bitboards[bRook] & ~(1ULL << f8)) | (1ULL << h8);
      squares[60] = bKing;
      squares[61] = EMPTY;
      squares[62] = EMPTY;
      squares[63] = bRook;
    }
    else if (!m.isKingside && !whiteToMove)
    {
      bitboards[bKing] = (bitboards[bKing] & ~(1ULL << c8)) | (1ULL << e8);
      bitboards[bRook] = (bitboards[bRook] & ~(1ULL << d8)) | (1ULL << a8);
      squares[56] = bRook;
      squares[57] = EMPTY;
      squares[58] = EMPTY;
      squares[59] = EMPTY;
      squares[60] = bKing;
    }
  }
  else if (m.isEnPassant)
  {
    Piece movingPawn = whiteToMove ? wPawn : bPawn;
    Piece capturedPawn = whiteToMove ? bPawn : wPawn;
    int capturedSq = whiteToMove ? m.to - 8 : m.to + 8;

    bitboards[movingPawn] = (bitboards[movingPawn] & ~(1ULL << m.to)) | (1ULL << m.from);
    bitboards[capturedPawn] |= (1ULL << capturedSq);
    squares[m.from] = movingPawn;
    squares[m.to] = EMPTY;
    squares[capturedSq] = capturedPawn;
  }
  else
  {
    Piece movingPiece = squares[m.to];
    bitboards[movingPiece] = (bitboards[movingPiece] & ~(1ULL << m.to)) | (1ULL << m.from);
    if (m.prevState.capturedPiece != EMPTY)
      bitboards[m.prevState.capturedPiece] |= (1ULL << m.to);
    squares[m.from] = movingPiece;
    squares[m.to] = m.prevState.capturedPiece;
  }

  updatePosition();
}