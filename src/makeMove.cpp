#include "board.h"
#include "castling.h"
#include "zobrist.h"

void Board::makeMove(Move &m)
{
  if (halfMoveCount == 0)
    m.prevState = Undo{hash, 15, NO_SQUARE, 0, EMPTY, true};
  else
    m.prevState = Undo{hash, castlingRights, enPassantSquare, halfMoveCount, squares[m.to], whiteToMove};

  hash ^= castlingRandoms[castlingRights];
  if (enPassantSquare != NO_SQUARE)
    hash ^= epFile[enPassantSquare % 8];

  if (m.isCastling)
  {
    if (m.isKingside && whiteToMove)
    {
      hash ^= squareRandoms[wKing][e1];
      hash ^= squareRandoms[wRook][h1];
      castlingRights &= ~(wK | wQ);
      bitboards[wKing] = (bitboards[wKing] & ~(1ULL << e1)) | (1ULL << g1);
      bitboards[wRook] = (bitboards[wRook] & ~(1ULL << h1)) | (1ULL << f1);
      squares[4] = EMPTY;
      squares[5] = wRook;
      squares[6] = wKing;
      squares[7] = EMPTY;
      hash ^= squareRandoms[wKing][g1];
      hash ^= squareRandoms[wRook][f1];
    }
    else if (!m.isKingside && whiteToMove)
    {
      hash ^= squareRandoms[wKing][e1];
      hash ^= squareRandoms[wRook][a1];
      castlingRights &= ~(wK | wQ);
      bitboards[wKing] = (bitboards[wKing] & ~(1ULL << e1)) | (1ULL << c1);
      bitboards[wRook] = (bitboards[wRook] & ~(1ULL << a1)) | (1ULL << d1);
      squares[0] = EMPTY;
      squares[1] = EMPTY;
      squares[2] = wKing;
      squares[3] = wRook;
      squares[4] = EMPTY;
      hash ^= squareRandoms[wKing][c1];
      hash ^= squareRandoms[wRook][d1];
    }
    else if (m.isKingside && !whiteToMove)
    {
      hash ^= squareRandoms[bKing][e8];
      hash ^= squareRandoms[bRook][h8];
      castlingRights &= ~(bK | bQ);
      bitboards[bKing] = (bitboards[bKing] & ~(1ULL << e8)) | (1ULL << g8);
      bitboards[bRook] = (bitboards[bRook] & ~(1ULL << h8)) | (1ULL << f8);
      squares[60] = EMPTY;
      squares[61] = bRook;
      squares[62] = bKing;
      squares[63] = EMPTY;
      hash ^= squareRandoms[bKing][g8];
      hash ^= squareRandoms[bRook][f8];
    }
    else if (!m.isKingside && !whiteToMove)
    {
      hash ^= squareRandoms[bKing][e8];
      hash ^= squareRandoms[bRook][a8];
      castlingRights &= ~(bK | bQ);
      bitboards[bKing] = (bitboards[bKing] & ~(1ULL << e8)) | (1ULL << c8);
      bitboards[bRook] = (bitboards[bRook] & ~(1ULL << a8)) | (1ULL << d8);
      squares[56] = EMPTY;
      squares[57] = EMPTY;
      squares[58] = bKing;
      squares[59] = bRook;
      squares[60] = EMPTY;
      hash ^= squareRandoms[bKing][c8];
      hash ^= squareRandoms[bRook][d8];
    }
  }
  else if (m.isEnPassant)
  {
    m.prevState.capturedPiece = whiteToMove ? bPawn : wPawn;
    Piece movingPawn = whiteToMove ? wPawn : bPawn;
    Piece capturedPawn = whiteToMove ? bPawn : wPawn;
    int capturedSq = whiteToMove ? m.to - 8 : m.to + 8;

    hash ^= squareRandoms[movingPawn][m.from];
    hash ^= squareRandoms[capturedPawn][capturedSq];

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

    hash ^= squareRandoms[movingPawn][m.to];
  }
  else if (m.promotionPiece != EMPTY)
  {
    Piece pawnColor = whiteToMove ? wPawn : bPawn;
    Piece pieceFrom = squares[m.from];
    Piece pieceTo = squares[m.to];
    u64 from = 1ULL << m.from;
    u64 to = 1ull << m.to;

    hash ^= squareRandoms[pawnColor][m.from];
    if (pieceTo != EMPTY)
      hash ^= squareRandoms[pieceTo][m.to];

    squares[m.to] = m.promotionPiece;
    squares[m.from] = EMPTY;
    bitboards[m.promotionPiece] |= to;
    bitboards[pawnColor] &= ~from;
    if (pieceTo != EMPTY)
      bitboards[pieceTo] &= ~to;
    castlingRights &= castlingRightsMask[m.to];

    hash ^= squareRandoms[m.promotionPiece][m.to];
  }
  else
  {
    Piece pieceFrom = squares[m.from];
    Piece pieceTo = squares[m.to];
    u64 from = 1ULL << m.from;
    u64 to = 1ULL << m.to;

    hash ^= squareRandoms[pieceFrom][m.from];
    if (pieceTo != EMPTY)
      hash ^= squareRandoms[pieceTo][m.to];

    squares[m.to] = squares[m.from];
    squares[m.from] = EMPTY;
    bitboards[pieceFrom] &= ~from;
    bitboards[pieceFrom] |= to;
    if (pieceTo != EMPTY)
      bitboards[pieceTo] &= ~to;
    castlingRights &= castlingRightsMask[m.from];
    castlingRights &= castlingRightsMask[m.to];

    hash ^= squareRandoms[pieceFrom][m.to];
  }

  halfMoveCount++;
  whiteToMove = !whiteToMove;
  enPassantSquare = m.setEpSquare;

  hash ^= castlingRandoms[castlingRights];
  if (enPassantSquare != NO_SQUARE)
    hash ^= epFile[enPassantSquare % 8];

  hash ^= sideKey;

  updatePosition();
}
