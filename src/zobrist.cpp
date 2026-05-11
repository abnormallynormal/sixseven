#include "zobrist.h"
#include "board.h"

u64 seed = 67;

u64 squareRandoms[12][64];
u64 sideKey;
u64 castlingRandoms[16];
u64 epFile[8];

void generatePseudorandom()
{
  for (int i = 0; i < 12; i++)
  {
    for (int j = 0; j < 64; j++)
    {
      squareRandoms[i][j] = next();
    }
  }
  sideKey = next();
  for (int i = 0; i < 16; i++)
  {
    castlingRandoms[i] = next();
  }
  for (int i = 0; i < 8; i++)
  {
    epFile[i] = next();
  }
}

u64 initHash(Board &b)
{
  static bool initialized = false;
  if (!initialized)
  {
    generatePseudorandom();
    initialized = true;
  }
  u64 hash = 0;
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      int index = 8 * i + j;
      if (b.squares[index] != EMPTY)
      {
        Piece p = b.squares[index];
        hash ^= squareRandoms[p][index];
      }
    }
  }
  if (!b.whiteToMove)
    hash ^= sideKey;

  hash ^= castlingRandoms[b.castlingRights];
  if (b.enPassantSquare != NO_SQUARE)
  {
    int file = b.enPassantSquare % 8;
    hash ^= epFile[file];
  }
  return hash;
}
