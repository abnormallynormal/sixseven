#include "board.h"

void Board::unmakeMove(Move &m)
{
  hash = m.prevState.hash;
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
  else if (m.promotionPiece != EMPTY)
  {
    Piece pawnColor = whiteToMove ? wPawn : bPawn;
    u64 from = 1ULL << m.from;
    u64 to = 1ULL << m.to;
    bitboards[m.promotionPiece] &= ~to;
    bitboards[pawnColor] |= from;
    if (m.prevState.capturedPiece != EMPTY)
    {
      bitboards[m.prevState.capturedPiece] |= to;
    }
    squares[m.to] = m.prevState.capturedPiece;
    squares[m.from] = pawnColor;
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
