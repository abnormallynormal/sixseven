#pragma once
#include "types.h"
#include "moveGen.h"
#include "board.h"
#include "constants.h"

extern u64 seed;

inline u64 next()
{
  u64 z = (seed += 0x9e3779b97f4a7c15);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
  z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
  return z ^ (z >> 31);
}

const int tableSize = 1 << 20;

struct HashEntry
{
  u64 key;
  int score;
  int depth;
  Move bestMove;
  int nodeType;
};

extern HashEntry table[tableSize];

extern u64 squareRandoms[12][64];
extern u64 sideKey;
extern u64 castlingRandoms[16];
extern u64 epFile[8];

void generatePseudorandom();

u64 initHash(Board &b);

inline void storeEntry(u64 hash, int score, int depth, Move bestMove, int nodeType, int ply)
{
  HashEntry &entry = table[hash & (tableSize - 1)];

  if (score > MATE_THRESHOLD - 10000)
  {
    score += ply;
  }
  else if (score < -MATE_THRESHOLD + 10000)
  {
    score -= ply;
  }

  entry.key = hash;
  entry.score = score;
  entry.depth = depth;
  entry.bestMove = bestMove;
  entry.nodeType = nodeType;
}

inline bool probeEntry(u64 hash, int depth, int alpha, int beta, int ply, int &score, Move &m)
{
  HashEntry &entry = table[hash & (tableSize - 1)];
  if (entry.key != hash)
    return false;
  m = entry.bestMove;
  if (entry.depth >= depth)
  {
    int s = entry.score;
    if (s > MATE_THRESHOLD - 10000)
      s -= ply;
    else if (s < -MATE_THRESHOLD + 10000)
      s += ply;
    if (entry.nodeType == EXACT)
    {
      score = s;
      return true;
    }
    if (entry.nodeType == UPPER && score <= alpha)
    {
      score = s;
      return true;
    }
    if (entry.nodeType == LOWER && score >= beta)
    {
      score = s;
      return true;
    }
  }
  return false;
}
