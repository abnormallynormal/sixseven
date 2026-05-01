#pragma once

#include "evaluation.h"

Move rootNegamax(Board &board, MoveGenerator &moveGen, int depth){}

int negamax(Board &board, MoveGenerator &moveGen, int alpha, int beta, int depth, int ply) {}

int scoreMove(Board &board, Move m) {}

int mvv_lva(Piece attack, Piece victim) {}

int quiescence(Board& board, MoveGenerator &moveGen, int alpha, int beta){}