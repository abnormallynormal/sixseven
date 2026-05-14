#pragma once
// Deep-measurement position set for tests D, E and the TT diagnostic.
// All FENs vetted: side-to-move is NOT already in check (engine BUGS.md #9
// would crash otherwise).
//
// Several positions from the originally-planned 15 were dropped after timing
// the engine — their tactical density makes quiescence explode and a single
// depth-7 search would take several minutes. The set below is 11 positions.
// Slow positions excluded: quiet_qgd_locked, quiet_kid_locked, tactic_q_vs_q,
// tactic_sicilian_open (each >10s at depth 5 / >70s at depth 6).

#include <string>
#include <vector>

enum class PositionKind
{
  Opening,
  QuietMiddle,
  TacticalMiddle,
  Endgame,
  TranspositionHeavy,
  PawnEndgame,
};

struct DeepPosition
{
  const char *name;
  const char *fen;
  PositionKind kind;
};

inline const std::vector<DeepPosition> &deep_positions()
{
  static const std::vector<DeepPosition> positions = {
      // ---- Openings (2) ----
      {"open_italian_setup",
       "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
       PositionKind::Opening},
      {"open_sicilian_5_Nc3",
       "rnbqkb1r/pp2pppp/3p1n2/8/3NP3/2N5/PPP2PPP/R1BQKB1R b KQkq - 0 5",
       PositionKind::Opening},

      // ---- Quiet middlegames (3): closed/semi-closed, calmer tactics ----
      {"quiet_italian_closed",
       "r1bq1rk1/pp3ppp/2nb1n2/3p4/3P4/2NB1N2/PP3PPP/R1BQ1RK1 w - - 0 1",
       PositionKind::QuietMiddle},
      {"quiet_french_closed",
       "r1bq1rk1/pp2nppp/2n1p3/2bpP3/3P4/2P2N2/PP1NBPPP/R1BQ1RK1 w - - 0 1",
       PositionKind::QuietMiddle},
      {"quiet_caro_solid",
       "r1bqkb1r/pp1npppp/2p2n2/3p4/3P4/3B1N2/PPPN1PPP/R1BQK2R w KQkq - 0 1",
       PositionKind::QuietMiddle},

      // ---- Tactical middlegames (3): open lines but not pathologically deep ----
      {"tactic_wac005",
       "1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - - 0 1",
       PositionKind::TacticalMiddle},
      {"tactic_wac014",
       "r1b1kb1r/3q1ppp/pBp1pn2/8/Np3P2/5B2/PPP3PP/R2Q1RK1 w kq - 0 1",
       PositionKind::TacticalMiddle},
      // (tactic_sicilian_open dropped — 76 s for a single depth-6 search makes
      //  the full feature matrix infeasible.)

      // ---- Endgames with pieces (2) ----
      {"endgame_rook",
       "8/5k2/3R4/4K3/8/8/8/8 w - - 0 1",
       PositionKind::Endgame},
      // Minor-piece endgame (KBN vs K). Original "8/8/4k3/8/2BN4/4K3/8/8" was
      // illegal — Bc4 was attacking Bk on e6 on white's turn (BUGS.md #9).
      {"endgame_bishop_knight",
       "8/8/8/k7/8/2N5/3B4/3K4 w - - 0 1",
       PositionKind::Endgame},

      // ---- Transposition-heavy positions (2) — used in the TT diagnostic ----
      {"trans_kiwipete",
       "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
       PositionKind::TranspositionHeavy},
      {"trans_open_mg",
       "r1bq1rk1/ppp2ppp/2n2n2/3pp3/4P3/2NPB3/PPP1NPPP/R2QK2R w KQ - 0 1",
       PositionKind::TranspositionHeavy},

      // ---- Pawn endgame (1) ----
      {"pawn_zugzwang",
       "8/p1p1k1p1/1p1p4/3P1P2/1PP1P3/P5P1/4K3/8 w - - 0 1",
       PositionKind::PawnEndgame},
  };
  return positions;
}

inline const char *kind_name(PositionKind k)
{
  switch (k)
  {
  case PositionKind::Opening:            return "opening";
  case PositionKind::QuietMiddle:        return "quiet-middle";
  case PositionKind::TacticalMiddle:     return "tactical-middle";
  case PositionKind::Endgame:            return "endgame";
  case PositionKind::TranspositionHeavy: return "transposition";
  case PositionKind::PawnEndgame:        return "pawn-endgame";
  }
  return "?";
}
