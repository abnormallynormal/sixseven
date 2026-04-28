#include "board.h"
#include "moveGen.h"
#include "perft.h"
#include <iostream>
int main()
{
  MoveGenerator mg;
  
  Board b;
  std::cout << perft(b, 5, true, mg);
  
  return 0;
}