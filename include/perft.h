#pragma once

#include "moveGen.h"
#include "board.h"
#include "evaluation.h"
#include "string"
#include <iostream>
#include <assert.h>
inline std::string move_to_string(Move &m, Board &board)
{
  if (m.is_castling)
  {
    if (m.is_kingside)
      if (board.is_white_to_move())
        return "e1g1";
      else
        return "e8g8";
    else
    {
      if (board.is_white_to_move())
        return "e1c1";
      else
        return "e8c8";
    }
  }
  std::string result = "";
  result += (char)('a' + (m.from % 8));
  result += (char)('1' + (m.from / 8));
  result += (char)('a' + (m.to % 8));
  result += (char)('1' + (m.to / 8));
  if(m.promotion_piece != EMPTY){
    if(m.promotion_piece % 6 == 1) result += 'n';
    if(m.promotion_piece % 6 == 2) result += 'b';
    if(m.promotion_piece % 6 == 3) result += 'r';
    if(m.promotion_piece % 6 == 4) result += 'q';
  }
  return result;
}

inline u64 perft(Board &board, int depth, bool white, MoveGenerator &move_gen, int ply = 0)
{
  if (depth == 0)
    return 1;

  move_gen.generate_moves(board, ply);
  int count = move_gen.move_lists[ply].count;

  u64 nodes = 0;
  for (int i = 0; i < count; i++)
  {
    Move m = move_gen.move_lists[ply].moves[i];
    u64 hash_before = board.hash;
    u64 pawns_before = board.pawns_hash;

    Undo undo;
    board.make_move(m, undo);

    u64 hash_expected = init_hash(board);
    u64 pawns_expected = init_pawns_hash(board);
    if (board.hash != hash_expected || board.pawns_hash != pawns_expected)
    {
      std::cerr << "INCREMENTAL HASH BUG after make_move " << move_to_string(m, board)
                << " at ply " << ply << "\n"
                << "  hash       got=0x" << std::hex << board.hash
                << " want=0x" << hash_expected << "\n"
                << "  pawns_hash got=0x" << board.pawns_hash
                << " want=0x" << pawns_expected << std::dec << "\n";
      board.print_board();
      std::abort();
    }

    if (!move_gen.is_in_check(board, white))
    {
      nodes += perft(board, depth - 1, !white, move_gen, ply + 1);
    }
    board.unmake_move(m, undo);

    if (board.hash != hash_before || board.pawns_hash != pawns_before)
    {
      std::cerr << "ROUND-TRIP BUG across make/unmake of "
                << move_to_string(m, board) << " at ply " << ply << "\n"
                << "  hash       before=0x" << std::hex << hash_before
                << " after=0x" << board.hash << "\n"
                << "  pawns_hash before=0x" << pawns_before
                << " after=0x" << board.pawns_hash << std::dec << "\n";
      std::abort();
    }
  }
  return nodes;
}


