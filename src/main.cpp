#include "board.h"
#include "moveGen.h"
#include "perft.h"
#include <iostream>
int main()
{
  MoveGenerator mg;
  
  // Test kiwipete
  Board b;
  std::cout << "depth 4: " << perft(b, 5, true, mg);
  
  return 0;
}