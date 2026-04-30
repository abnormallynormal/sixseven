#include "evaluation.h"


int materialOdds(Board &board)
{
  return pieceVals[wPawn] * (countPieces(wPawn, board) - countPieces(bPawn, board)) 
  + pieceVals[wKnight] * (countPieces(wKnight, board) - countPieces(bKnight, board)) 
  + pieceVals[wBishop] * (countPieces(wBishop, board) - countPieces(bBishop, board)) 
  + pieceVals[wRook] * (countPieces(wRook, board) - countPieces(bRook, board)) 
  + pieceVals[wQueen] * (countPieces(wQueen, board) - countPieces(bQueen, board));
}