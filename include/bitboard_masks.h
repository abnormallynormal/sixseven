#pragma once
#include "types.h"

const u64 notAFile = 0xFEFEFEFEFEFEFEFE;
const u64 notHFile = 0x7F7F7F7F7F7F7F7F;
const u64 notABFile = 0xFCFCFCFCFCFCFCFC;
const u64 notGHFile = 0x3F3F3F3F3F3F3F3F;
const u64 secondRank = 0x000000000000FF00;
const u64 seventhRank = 0x00FF000000000000;
const u64 firstRank = 0x00000000000000FF;
const u64 eighthRank = 0xFF00000000000000;
const u64 boardEdges = 0xFF818181818181FF;
const u64 notEdges = ~boardEdges;
const u64 promotionRanks = 0xFF000000000000FF;
