#include "board.h"
#include "moveGen.h"
#include "perft.h"
#include <iostream>

MoveGenerator mg;
Board b;
int main()
{
  std::cout << perft(b, 5, true, mg);
  return 0;
}