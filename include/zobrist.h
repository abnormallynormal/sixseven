#pragma once
#include "types.h"

class Board;

extern u64 seed;

inline u64 next()
{
  u64 z = (seed += 0x9e3779b97f4a7c15);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
  z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
  return z ^ (z >> 31);
}

extern u64 squareRandoms[12][64];
extern u64 sideKey;
extern u64 castlingRandoms[16];
extern u64 epFile[8];

void generatePseudorandom();

u64 initHash(Board &b);
