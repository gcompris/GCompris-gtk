/* GNU Chess 5.0 - eval.c - evaluation code
   Copyright (c) 1999-2002 Free Software Foundation, Inc.

   GNU Chess is based on the two research programs 
   Cobalt by Chua Kong-Sian and Gazebo by Stuart Cracraft.

   GNU Chess is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GNU Chess is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Chess; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Contact Info: 
     bug-gnu-chess@gnu.org
     cracraft@ai.mit.edu, cracraft@stanfordalumni.org, cracraft@earthlink.net
*/

/****************************************************************************
 *
 *
 *
 *****************************************************************************/


#include <config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "eval.h"

int LoneKing (int, int);
int ScoreKBNK (int, int);
int KPK (int);
int BishopTrapped (short);
int DoubleQR7 (short);

BitBoard passed[2];
BitBoard weaked[2];

static int PawnSq[2][64] = 
{
{  0,  0,  0,  0,  0,  0,  0,  0,
   5,  5,  5,-10,-10,  5,  5,  5,
  -2, -2, -2,  6,  6, -2, -2, -2,
   0,  0,  0, 25, 25,  0,  0,  0,
   2,  2, 12, 16, 16, 12,  2,  2,
   4,  8, 12, 16, 16, 12,  4,  4,
   4,  8, 12, 16, 16, 12,  4,  4,
   0,  0,  0,  0,  0,  0,  0,  0},
{  0,  0,  0,  0,  0,  0,  0,  0,
   4,  8, 12, 16, 16, 12,  4,  4,
   4,  8, 12, 16, 16, 12,  4,  4,
   2,  2, 12, 16, 16, 12,  2,  2,
   0,  0,  0, 25, 25,  0,  0,  0,
  -2, -2, -2,  6,  6, -2, -2, -2,
   5,  5,  5,-10,-10,  5,  5,  5,
   0,  0,  0,  0,  0,  0,  0,  0}
};

static const int Passed[2][8] =
{ { 0, 48, 48, 120, 144, 192, 240, 0}, {0, 240, 192, 144, 120, 48, 48, 0} };
/* Penalties for one or more isolated pawns on a given file */
static const int isolani_normal[8] = {
  -8, -10, -12, -14, -14, -12, -10, -8
};
/* Penalties if the file is half-open (i.e. no enemy pawns on it) */
static const int isolani_weaker[8] = {
  -22, -24, -26, -28, -28, -26, -24, -22
};

static const BitBoard d2e2[2] =
		 { ULL(0x0018000000000000), ULL(0x0000000000001800) };
static const BitBoard brank7[2]  = { ULL(0x000000000000FF00),
				     ULL(0x00FF000000000000) };
static const BitBoard brank8[2]  = { ULL(0x00000000000000FF),
				     ULL(0xFF00000000000000) };
static const BitBoard brank67[2] = { ULL(0x0000000000FFFF00),
				     ULL(0x00FFFF0000000000) };
static const BitBoard brank58[2] = { ULL(0x00000000FFFFFFFF),
				     ULL(0xFFFFFFFF00000000) };

int ScoreP (short side)
/***************************************************************************
 *
 *  Pawn evaluation is based on the following factors (which is being
 *  constantly updated!).
 *
 *  1.  Pawn square tables.
 *  2.  Passed pawns.
 *  3.  Backward pawns.
 *  4.  Pawn base under attack.
 *  5.  Doubled pawns 
 *  6.  Isolated pawns 
 *  7.  Connected passed pawns on 6/7th rank.
 *  8.  Unmoved & blocked d, e pawn
 *  9.  Passed pawn which cannot be caught.
 *  10. Pawn storms.
 *
 ***************************************************************************/
{
   int xside;
   int s, sq, i, i1;
   int n1, n2, backward;
   int nfile[8];
   int EnemyKing;
   BitBoard c, t, p, blocker, *e;
   PawnSlot *ptable;

   if (board.b[side][pawn] == NULLBITBOARD)
      return (0);
   xside = 1^side;
   EnemyKing = board.king[xside];
   p = board.b[xside][pawn];
   c = t = board.b[side][pawn];
   ptable = PawnTab[side] + (PawnHashKey & PHashMask);
   TotalPawnHashCnt++;
   if (ptable->phase == phase && ptable->pkey == KEY(PawnHashKey))
   {
      GoodPawnHashCnt++;
      s = ptable->score;
      passed[side] = ptable->passed;
      weaked[side] = ptable->weaked;
      goto phase2; 
   }

   s = 0;
   passed[side] = NULLBITBOARD;
   weaked[side] = NULLBITBOARD;
   memset (nfile, 0, sizeof (nfile));
   while (t)
   {
      sq = leadz (t);
      CLEARBIT (t, sq);
      s += PawnSq[side][sq]; 

      /*  Passed pawns  */
      if ((p & PassedPawnMask[side][sq]) == NULLBITBOARD)
      {
	 if ((side == white && (FromToRay[sq][sq|56] & c) == 0) ||
	     (side == black && (FromToRay[sq][sq&7] & c) == 0)) 
         {
            passed[side] |= BitPosArray[sq];
            s += (Passed[side][RANK(sq)] * phase) / 12;
         }
      }

      /*  Backward pawns */
      backward = false;
    /*   i = sq + (side == white ? 8 : -8); */
      if ( side == white ) {
 	 i = sq + 8;  }
      else {
         i= sq - 8; }

      if (!(PassedPawnMask[xside][i] & ~FileBit[ROW(sq)] & c) &&
	  cboard[i] != pawn)
      {
         n1 = nbits (c & MoveArray[ptype[xside]][i]);
         n2 = nbits (p & MoveArray[ptype[side]][i]);
         if (n1 < n2)
            backward = true;
      }
      if (!backward && (BitPosArray[sq] & brank7[xside]))
      {
         i1 = 1;
         i = i + (side == white ? 8 : -8);
         if (!(PassedPawnMask[xside][i] & ~FileBit[ROW(i1)] & c))
         {
            n1 = nbits (c & MoveArray[ptype[xside]][i]);
            n2 = nbits (p & MoveArray[ptype[side]][i]);
            if (n1 < n2)
               backward = true;
         }
      }
      if (backward)
      {
         weaked[side] |= BitPosArray[sq];
         s += BACKWARDPAWN;
      }

      /* Pawn base under attack */
      if ((MoveArray[ptype[side]][sq] & p) && (MoveArray[ptype[side]][sq] & c))
         s += PAWNBASEATAK;
 
      /*  Increment file count for isolani & doubled pawn evaluation */
      nfile[ROW(sq)]++;
   }

   for (i = 0; i <= 7; i++)
   {
      /* Doubled pawns */
      if (nfile[i] > 1)
         s += DOUBLEDPAWN;

      /* Isolated pawns */
      if (nfile[i] && (!(c & IsolaniMask[i])))
      {
	 /* Isolated on a half-open file */
         if (!(FileBit[i] & board.b[xside][pawn]))
	   s += isolani_weaker[i] * nfile[i];
	 else /* Normal isolated pawn */
           s += isolani_normal[i] * nfile[i];
	 weaked[side] |= (c & FileBit[i]);
      }
   }


  if (computerplays == side) {

    /* Penalize having eight pawns */
    if (nbits(board.b[computerplays][pawn]) == 8)
        s += EIGHT_PAWNS;

    /* Detect stonewall formation in enemy */
    if (nbits(stonewall[xside] & board.b[xside][pawn]) == 3)
      s += STONEWALL;

    /* Locked pawns */
    n = 0;
    if (side == white)
      n = nbits((c >> 8) & board.b[xside][pawn] &
               boxes[1]);
    else
      n = nbits((c << 8) & board.b[xside][pawn] &
               boxes[1]);
    if (n > 1)
      s += n * LOCKEDPAWNS;
  }


   /* Save the score into the pawn hash table */ 
   ptable->pkey = KEY(PawnHashKey);
   ptable->passed = passed[side];
   ptable->weaked = weaked[side];
   ptable->score = s;
   ptable->phase = phase;

/***************************************************************************
 *  
 *  This section of the pawn code cannot be saved into the pawn hash as
 *  they depend on the position of other pieces.  So they have to be 
 *  calculated again.
 *
 ***************************************************************************/
phase2:

   /* Pawn on f6/c6 with Queen against castled king is very strong */
   c = board.b[side][pawn];
   sq = board.king[xside];
   if (side == white && board.b[side][queen] && 
	(BitPosArray[C6] | BitPosArray[F6]) & c)
   {
      if (c & BitPosArray[F6] && sq > H6 && distance[sq][G7]==1)
         s += PAWNNEARKING;
      if (c & BitPosArray[C6] && sq > H6 && distance[sq][B7]==1)
         s += PAWNNEARKING;
   }
   else if (side == black && board.b[side][queen] &&
	(BitPosArray[C3] | BitPosArray[F3]) & c)
   {
      if (c & BitPosArray[F3] && sq < A3 && distance[sq][G2]==1)
         s += PAWNNEARKING;
      if (c & BitPosArray[C3] && sq < A3 && distance[sq][B2]==1)
         s += PAWNNEARKING;
   }

   /* Connected passed pawns on 6th or 7th rank */
   t = passed[side] & brank67[side];
   if (t && (board.pmaterial[xside] == ValueR || 
	(board.pmaterial[xside] == ValueN &&
	pieces[xside] == board.b[xside][knight])))
   {
      n1 = ROW(board.king[xside]);
      n2 = RANK(board.king[xside]);
      for (i = 0; i <= 6; i++)
      {
	 if (t & FileBit[i] && t & FileBit[i+1] && (n1 < i-1 || n1 > i+1 ||
		(side == white && n2 < 4) || (side == black && n2 > 3)))
            s += CONNECTEDPP;
      }
   }

   /* Pawn on d2,e2/d7,e7 is blocked  */
   blocker = board.friends[side] | board.friends[xside];
   if (side == white && (((c & d2e2[white]) >> 8) & blocker))
      s += BLOCKDEPAWN;
   if (side == black && (((c & d2e2[black]) << 8) & blocker))
      s += BLOCKDEPAWN;

   /* Enemy has no pieces & King is outside of passed pawn square */
   if (passed[side] && board.pmaterial[xside]==0)
   {
      e = board.b[xside];
      i1 = board.king[xside];
      p = passed[side];
      while (p)
      {
         sq = leadz (p);
	 CLEARBIT (p, sq);
	 if (board.side == side)
         {
	    if (!(SquarePawnMask[side][sq] & board.b[xside][king]))
	       s += ValueQ * Passed[side][RANK(sq)] / PFACTOR;
         }
         else if (!(MoveArray[king][i1] & SquarePawnMask[side][sq]))
	    s += ValueQ * Passed[side][RANK(sq)] / PFACTOR;
      }
   }

  /* If both sides are castled on different sides, bonus for pawn storms */
  c = board.b[side][pawn];
  if (abs (ROW (board.king[side]) - ROW (board.king[xside])) >= 4 &&
	PHASE < 6)
  {
     n1 = ROW (board.king[xside]);
     p = (IsolaniMask[n1] | FileBit[n1]) & c;
     while (p)
     {
        sq = leadz (p);
        CLEARBIT (p, sq);
        s += 10 * (5 - distance[sq][board.king[xside]]);
     }
  }

   return (s);
}

static const int Outpost[2][64] =
{
  { 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0 }
};


static inline int CTL(short sq, short piece __attribute__ ((unused)), short side)
/***************************************************************************
 *
 *  Return a score corresponding to the number of squares in the bitboard
 *  target multiplied by a specified bonus for controlling each square.
 *
 *  Can be used for control of the center and attacks around the king.
 *
 ***************************************************************************/
{
  int s, n, EnemyKing, FriendlyKing;
  BitBoard controlled;

  s = 0;

  EnemyKing = board.king[1^side];
  FriendlyKing = board.king[side];

  controlled = AttackXFrom (sq, side);

  /* Center control */
  n = nbits (controlled & boxes[0]);
  s += 4*n;

  /* Attacks against enemy king */
  n = nbits (controlled & DistMap[EnemyKing][2]);
  s += n;

  /* Defenses for friendly king */
  n = nbits (controlled & DistMap[FriendlyKing][2]);
  s += n;

  /* Mobility */
  n = nbits(controlled);
  s += 4*n;

  return (s);
}

int ScoreN (short side)
/***************************************************************************
 *
 *  1.  central knight - distance from enemy king.
 *  2.  mobility/control/attack
 *  3.  outpost knight protected by pawn.
 *  4.  knight attacking weak pawns.
 *
 ***************************************************************************/
{
   int xside;
   int s, s1, sq;
   int EnemyKing;
   BitBoard c, t;

   if (board.b[side][knight] == NULLBITBOARD)
      return (0);
   xside = side^1;
   s = s1 = 0;
   c = board.b[side][knight];
   t = board.b[xside][pawn]; 
   EnemyKing = board.king[xside];

   if ( c & pinned )
   {
	s += PINNEDKNIGHT * nbits(c & pinned);
   }

   while (c)
   {
      sq = leadz (c);
      CLEARBIT (c, sq);

      /* Control */
      s1 = CTL(sq,knight,side);
  
      if ( (BitPosArray[sq] & rings[3]) != NULLBITBOARD)
	s1 += KNIGHTONRIM;

      if (Outpost[side][sq] && 
	  !(t & IsolaniMask[ROW(sq)] & PassedPawnMask[side][sq]) )
      {
         s1 += OUTPOSTKNIGHT;

    	 /* Knight defended by own pawn */
         if (MoveArray[ptype[xside]][sq] & board.b[side][pawn])
            s1 += OUTPOSTKNIGHT;
      }
    
      /* Attack on weak opponent pawns */
      if (MoveArray[knight][sq] & weaked[xside])
         s1 += ATAKWEAKPAWN;

      s += s1;
   }

   return (s);
}


int ScoreB (short side)
/****************************************************************************
 *
 *  1.  double bishops.
 *  2.  mobility/control/attack
 *  3.  outpost bishop
 *  4.  fianchetto bishop
 *  5.  Bishop pair
 *
 ****************************************************************************/
{
   int xside;
   int s, s1, n, sq, EnemyKing;
   BitBoard c, t;

   if (board.b[side][bishop] == NULLBITBOARD)
      return (0);
   s = s1 = 0;
   c = board.b[side][bishop];
   xside = side ^ 1;
   EnemyKing = board.king[xside];
   n = 0;
   t = board.b[xside][pawn];

   if ( c & pinned )
   {
	s += PINNEDBISHOP * nbits(c & pinned);
   }

   while (c)
   {
      sq = leadz (c);
      CLEARBIT (c, sq);
      n++;

      /* Control */
      s1 = CTL(sq,bishop,side);

      /*  Outpost bishop */
      if (Outpost[side][sq] && 
	  !(t & IsolaniMask[ROW(sq)] & PassedPawnMask[side][sq]))
      {
         s1 += OUTPOSTBISHOP;

    	 /* Bishop defended by own pawn */
	 if (MoveArray[ptype[xside]][sq] & board.b[side][pawn])
            s1 += OUTPOSTBISHOP;
      }

      /*  Fianchetto bishop */
      if (side == white)
      {
         if (board.king[side] >= F1 && board.king[side] <= H1 && sq == G2)
	    s1 += FIANCHETTO;
         if (board.king[side] >= A1 && board.king[side] <= C1 && sq == B2)
	    s1 += FIANCHETTO;
      }
      else if (side == black)
      {
         if (board.king[side] >= F8 && board.king[side] <= H8 && sq == G7)
	    s1 += FIANCHETTO;
         if (board.king[side] >= A8 && board.king[side] <= C8 && sq == B7)
	    s1 += FIANCHETTO;
      }

      /* Attack on weak opponent pawns */
      if (BishopAttack(sq) & weaked[xside])
         s1 += ATAKWEAKPAWN;

      s += s1;
   }

   /* Doubled bishops */
   if (n > 1)            
      s += DOUBLEDBISHOPS;

   return (s);

}


int BishopTrapped (short side)
/****************************************************************************
 *
 *  Check for bishops trapped at A2/H2/A7/H7
 *
 ****************************************************************************/
{
   int s = 0;

   /* Don't waste time */
   if (board.b[side][bishop] == NULLBITBOARD)
     return (0); 

   if (side == white)
   {
      if ((board.b[white][bishop] & BitPosArray[A7]) &&
	  (board.b[black][pawn] & BitPosArray[B6]) && SwapOff(MOVE(A7,B6)) < 0)
         s += BISHOPTRAPPED;
      if ((board.b[white][bishop] & BitPosArray[H7]) &&
	  (board.b[black][pawn] & BitPosArray[G6]) && SwapOff(MOVE(H7,G6)) < 0)
         s += BISHOPTRAPPED;
   }
   else
   {
      if ((board.b[black][bishop] & BitPosArray[A2]) &&
	  (board.b[white][pawn] & BitPosArray[B3]) && SwapOff(MOVE(A2,B3)) < 0)
         s += BISHOPTRAPPED;
      if ((board.b[black][bishop] & BitPosArray[H2]) &&
	  (board.b[white][pawn] & BitPosArray[G3]) && SwapOff(MOVE(H2,G3)) < 0)
         s += BISHOPTRAPPED;
   }

   return (s);
}

int ScoreR (short side)
/****************************************************************************
 *
 *  1.  rook on 7th rank and Enemy king on 8th rank or pawns on 7th rank.
 *  2.  rook on open/half-open file.
 *  3.  rook in front/behind passed pawns (pawn >= 5th rank)
 *
 ****************************************************************************/
{
   int s, s1, sq, xside, fyle, EnemyKing;
   BitBoard c;

   if (board.b[side][rook] == NULLBITBOARD)
      return (0);
   s = s1 = 0;
   c = board.b[side][rook];
   xside = side ^ 1;
   EnemyKing = board.king[xside];

   if ( c & pinned )
   {
	s += PINNEDROOK * nbits(c & pinned);
   }

   while (c)
   {
      sq = leadz (c);
      CLEARBIT (c, sq);

      /* Control */
      s1 = CTL(sq,rook,side);

      fyle = ROW(sq);
      if (PHASE < 7)
      {
         if (!(board.b[side][pawn] & FileBit[fyle]))
         {
	    if (fyle == 5 && ROW(board.king[xside])>=E_FILE)
	      s1 += ROOKLIBERATED;
            s1 += ROOKHALFFILE;
            if (!(board.b[xside][pawn] & FileBit[fyle]))
               s1 += ROOKOPENFILE;
         }
      }

      if (phase > 6 && (FileBit[fyle] & passed[white] & brank58[white]))
      {
	 if (nbits (Ray[sq][7] & passed[white]) == 1)
	    s1 += ROOKBEHINDPP;
	 else if (Ray[sq][4] & passed[white])
            s1 += ROOKINFRONTPP;
      }
      if (FileBit[fyle] & passed[black] & brank58[black])
      {
	 if (nbits (Ray[sq][4] & passed[black]) == 1)
	    s1 += ROOKBEHINDPP;
	 else if (Ray[sq][7] & passed[black])
            s1 += ROOKINFRONTPP;
      }

      /* Attack on weak opponent pawns */
      if (RookAttack(sq) & weaked[xside])
         s1 += ATAKWEAKPAWN;

      /* Rook on 7th rank */
      if (RANK (sq) == rank7[side] && (RANK (EnemyKing) == rank8[side] ||
	  board.b[xside][pawn] & RankBit[RANK(sq)]))
         s1 += ROOK7RANK;

      s += s1;
   }

   return (s);
}

int DoubleQR7 (short side)
/***************************************************************************
 *
 *  This code just check to see if there is a QQ or QR or RR combo on the
 *  7th rank.  This is very strong and given quite a big bonus.  This 
 *  routine is called by the lazy section.
 *
 ***************************************************************************/
{
   int xside;

   xside = 1^side;
   if (nbits ((board.b[side][queen]|board.b[side][rook]) & brank7[side]) > 1
      && ((board.b[xside][king] & brank8[side]) || 
	  (board.b[xside][pawn] & brank7[side])))

      return (ROOKS7RANK);
   else
      return (0);
}

int ScoreQ (short side)
/***************************************************************************
 *
 *  1. queen centralization.
 *  2. king tropism.
 *  3. Bonus if opponent king is exposed.
 *
 ***************************************************************************/
{
   int xside;
   int s, s1, sq, EnemyKing;
   BitBoard c;
   
   s = s1 = 0;

   /* Try to keep our queen on the board for attacking purposes. */
   if (board.b[side][queen] == NULLBITBOARD) {
       if (side == computer) {
         s += QUEEN_NOT_PRESENT;
       }
       return(s);
    }                                                                           

   xside = 1 ^ side;
   c = board.b[side][queen];
   EnemyKing = board.king[xside];

   if ( c & pinned )
   {
	s += PINNEDQUEEN * nbits(c & pinned);
   }

   while (c)
   {
      sq = leadz (c);
      CLEARBIT (c, sq);

      /* Control */
      s1 = CTL(sq,queen,side);

      if (distance[sq][EnemyKing] <= 2)
         s1 += QUEENNEARKING;

      /* Attack on weak opponent pawns */
      if (QueenAttack(sq) & weaked[xside])
         s1 += ATAKWEAKPAWN;

      s += s1;
   }

   return (s);
}


static const int KingSq[64] =
{
   24, 24, 24, 16, 16,  0, 32, 32,
   24, 20, 16, 12, 12, 16, 20, 24,
   16, 12,  8,  4,  4,  8, 12, 16,
   12,  8,  4,  0,  0,  4,  8, 12,
   12,  8,  4,  0,  0,  4,  8, 12,
   16, 12,  8,  4,  4,  8, 12, 16,
   24, 20, 16, 12, 12, 16, 20, 24,
   24, 24, 24, 16, 16,  0, 32, 32
};

static const int EndingKing[64] =
{
   0,  6, 12, 18, 18, 12,  6,  0,
   6, 12, 18, 24, 24, 18, 12,  6,
  12, 18, 24, 32, 32, 24, 18, 12,
  18, 24, 32, 48, 48, 32, 24, 18,
  18, 24, 32, 48, 48, 32, 24, 18,
  12, 18, 24, 32, 32, 24, 18, 12,
   6, 12, 18, 24, 24, 18, 12,  6,
   0,  6, 12, 18, 18, 12,  6,  0
};

static const int pawncover[9] = { -60, -30, 0, 5, 30, 30, 30, 30, 30 };
static const int factor[9] = { 7, 8, 8, 7, 6, 5, 4, 2, 0, };

int ScoreK (short side)
/***************************************************************************
 *
 *  1.  king in the corner. ?? SRW 2002-08-02 Unclear if implemented
 *  2.  pawns around king.
 *  3.  king on open file.
 *  4.  Uncastled king.
 *  5.  Open rook file via Ng5 or Bxh7 sac.
 *  6.  No major or minor piece in the king's quadrant.
 *
 ***************************************************************************/
{
   int xside;
   int s, sq, sq1, n, n1, n2, file, fsq, rank;
   BitBoard b, x;

   s = 0;
   xside = 1^side;
   sq = board.king[side];
   file = ROW (sq);
   rank = RANK (sq);
   KingSafety[side] = 0;
   if (!ENDING)
   { 

      s += ((6 - phase) * KingSq[sq] + phase * EndingKing[sq]) / 6;

      /* After castling kingside, reward having all 3 pawns in front but not if
	 there is a threatening pawn. This permits the freeing move F3/F6. */
	 
      if (side == white)
        n = nbits (MoveArray[king][sq] & board.b[side][pawn] & RankBit[rank+1]);
      else
        n = nbits (MoveArray[king][sq] & board.b[side][pawn] & RankBit[rank-1]);
      s += pawncover[n];

      /* Penalize compromised wing pawn formations prior to castle. */
      if (!board.castled[side]) {
        n = -1;
        if (side == white) {
          /* King on original square and unmoved */
	  if (sq == 4 && Mvboard[sq] == 0) {
	    /* Kingside - rook on original square and unmoved. */
	    if ( (board.b[side][rook] & BitPosArray[H1])!=NULLBITBOARD &&
		 Mvboard[H1] == 0)
                   n = nbits (MoveArray[king][G1] & 
			      board.b[side][pawn] & RankBit[rank+1]);
	    /* Queenside */
	    if ( (board.b[side][rook] & BitPosArray[A1])!=NULLBITBOARD &&
		 Mvboard[A1] == 0)
                   n = nbits (MoveArray[king][C1] & 
			      board.b[side][pawn] & RankBit[rank+1]);
          }
   	} else {
	  if (sq == 60 && Mvboard[sq] == 0) {
	    /* Kingside */
	    if ( (board.b[side][rook] & BitPosArray[H8])!=NULLBITBOARD &&
		 Mvboard[H8] == 0)
                   n = nbits (MoveArray[king][G8] & 
			      board.b[side][pawn] & RankBit[rank-1]);
	    /* Queenside */
	    if ( (board.b[side][rook] & BitPosArray[A8])!=NULLBITBOARD &&
		 Mvboard[A8] == 0)
                   n = nbits (MoveArray[king][C8] & 
			      board.b[side][pawn] & RankBit[rank-1]);
          }
	}
        
	/* Penalize breaking the wing pawn formations prior to castle */
	if (n != -1) s += pawncover[n];
      }

      if (side == computer && file >= F_FILE && 
		!(FileBit[G_FILE] & board.b[side][pawn]))
      {
         if (side == white && cboard[F2] == pawn)
            s += GOPEN;
         else if (side == black && cboard[F7] == pawn)
            s += GOPEN;
      }

      /* No friendly pawns on this king file */
      if (!(FileBit[file] & board.b[side][pawn]))
         s += KINGOPENFILE;

      /* No enemy pawns on this king file */
      if (!(FileBit[file] & board.b[xside][pawn]))
         s += KINGOPENFILE1;

      switch (file)
      {
         case A_FILE :
         case E_FILE :
         case F_FILE :
	 case G_FILE : if (!(FileBit[file+1] & board.b[side][pawn]))
		          s += KINGOPENFILE;
		       if (!(FileBit[file+1] & board.b[xside][pawn]))
		          s += KINGOPENFILE1;
	               break;
         case H_FILE :
         case D_FILE :
         case C_FILE :
	 case B_FILE : if (!(FileBit[file-1] & board.b[side][pawn]))
		          s += KINGOPENFILE;
		       if (!(FileBit[file-1] & board.b[xside][pawn]))
		          s += KINGOPENFILE1;
	               break;
	 default :
	               break;
      }

      if (board.castled[side]) {
        if (side == white) {
          if (file > E_FILE) {
            if (!(BitPosArray[F2] & board.b[side][pawn]) ||
                !(BitPosArray[G2] & board.b[side][pawn]) ||
                !(BitPosArray[H2] & board.b[side][pawn]) )
                s += RUPTURE;
          } else if (file < E_FILE) {
            if (!(BitPosArray[A2] & board.b[side][pawn]) ||
                !(BitPosArray[B2] & board.b[side][pawn]) ||
                !(BitPosArray[C2] & board.b[side][pawn]) )
                s += RUPTURE;
          }
        } else {
          if (file > E_FILE) {
            if (!(BitPosArray[F7] & board.b[side][pawn]) ||
                !(BitPosArray[G7] & board.b[side][pawn]) ||
                !(BitPosArray[H7] & board.b[side][pawn]) )
                s += RUPTURE;
          } else if (file < E_FILE) {
            if (!(BitPosArray[A7] & board.b[side][pawn]) ||
                !(BitPosArray[B7] & board.b[side][pawn]) ||
                !(BitPosArray[C7] & board.b[side][pawn]) )
                s += RUPTURE;
	  }
	}
      }
      if (side == computer) {

	/* Stock piece sacrifice on h file */

	if (file >= E_FILE && board.b[xside][queen] && board.b[xside][rook] &&
	    !((board.b[side][pawn]|board.b[xside][pawn]) & FileBit[7]))
         s += HOPEN;
	
	/* King trapping rook */
        if (side == white) {
	  if (file > E_FILE) {
	    if (board.b[side][rook]&mask_kr_trapped_w[H_FILE-file]) {
		s += ROOKTRAPPED;
	    }
	  } else if (file < D_FILE) {
	    if (board.b[side][rook]&mask_qr_trapped_w[file]) {
		s += ROOKTRAPPED;
	    }
	  }
	} else {
	  if (file > E_FILE) {
	    if (board.b[side][rook]&mask_kr_trapped_b[H_FILE-file]) {
		s += ROOKTRAPPED;
	    }
	  } else if (file < D_FILE) {
	    if (board.b[side][rook]&mask_qr_trapped_b[file]) {
		s += ROOKTRAPPED;
	    }
	  }
	}
      }

      /* Don't give fianchetto target for advanced pawns */
      if (file > E_FILE && ROW(board.king[xside]) < D_FILE) {
         if (side == white) fsq = G3; else fsq = G6;
	 if ((BitPosArray[fsq] & board.b[side][pawn]) != NULLBITBOARD)
            if (((BitPosArray[F4]|BitPosArray[H4]|
	         BitPosArray[F5]|BitPosArray[H5])
	      & board.b[xside][pawn]) != NULLBITBOARD) 
	        s += FIANCHETTO_TARGET;	
      }
      if (file < E_FILE && ROW(board.king[xside]) > E_FILE) {
         if (side == white) fsq = B3; else fsq = B6;
	 if ((BitPosArray[fsq] & board.b[side][pawn]) != NULLBITBOARD)
            if (((BitPosArray[A4]|BitPosArray[C4]|
	         BitPosArray[A5]|BitPosArray[C5])
	      & board.b[xside][pawn]) != NULLBITBOARD) 
	        s += FIANCHETTO_TARGET;	
      }

      /* No major/minor piece in king's quadrant */
      /* First identify the quadrant */
      x = boardhalf[side] & boardside[file<=D_FILE];
      /* Now identify the number of non-pawn enemy in quadrant */
      n1 = nbits(x & (board.friends[xside]));
      if (n1 > 0) {
        /* Now identify the number of non-pawn friends in quadrant */
        n2 = nbits(x & (board.friends[side] & ~board.b[side][pawn] & 
	 	~board.b[side][king]));
        if (n1 > n2)
	  s += (n1 - n2) * KING_DEFENDER_DEFICIT;
      }
      
      KingSafety[side] = s;
      s = (s * factor[phase]) / 8;
   }
   else
   {
      s += EndingKing[sq];
      s += CTL(sq,king,side);
      b = (board.b[white][pawn] | board.b[black][pawn]);
      while (b)
      {
         sq1 = leadz (b);
         CLEARBIT (b, sq1);
	 if (BitPosArray[sq1] & board.b[white][pawn])
            s -= distance[sq][sq1+8] * 10 - 5;
	 else if (BitPosArray[sq1] & board.b[white][pawn])
            s -= distance[sq][sq1-8] * 10 - 5;
         else
	    s -= distance[sq][sq1] - 5;
      }

      /* Attack on weak opponent pawns */
      if (MoveArray[king][sq] & weaked[xside])
         s += ATAKWEAKPAWN * 2;

   }

    if (phase >= 4) {
      /* Weak back rank */
      if (side == white) {
        if (sq < A2) 
	  if (!(MoveArray[king][sq] & (~board.b[side][pawn] & RankBit[1]))) 
	    s += KING_BACK_RANK_WEAK;
      } else {
	if (sq > H7) 
	  if (!(MoveArray[king][sq] & (~board.b[side][pawn] & RankBit[6]))) 
	    s += KING_BACK_RANK_WEAK;
      }
   }

   return (s);
}


int LoneKing (int side, int loser)
/**************************************************************************
 *
 *  One side has a lonely king and the other has no pawns, but enough
 *  mating material.  We give an additional bonus of 150 points for the 
 *  winning side to attract the search to such positions.
 *
 **************************************************************************/
{
   int s, winer, sq1, sq2;

   winer = 1^loser;
   if (board.material[winer] == ValueB+ValueN && 
	nbits(board.b[winer][bishop]) == 1 &&
	nbits(board.b[winer][knight]) == 1)
      return (ScoreKBNK (side, loser));

   sq1 = board.king[winer];
   sq2 = board.king[loser];
   s = 150 - 6 * taxicab[sq1][sq2] - EndingKing[sq2];
   if (side == loser)
      s = -s;
   s += MATERIAL;

   return (s);
}


int KPK (int side)
/**************************************************************************
 *
 *  A KPK endgame evaluator.  Side is the one on the move.
 *  This is not a perfect evaluator, it merely identifies SOME positions
 *  as wins or draw.  Some WON positions could be seen as draws; the search
 *  will be able to use the knowledge here to identify more positions.
 *
 **************************************************************************/
{
   int winer, loser, sq, sqw, sql;
   int s;

   winer = (board.b[white][pawn] ? white : black);
   loser = 1 ^ winer;
   sq  = leadz (board.b[winer][pawn]);
   sqw = board.king[winer];
   sql = board.king[loser];
   s = ValueP + (ValueQ * Passed[winer][RANK(sq)] / PFACTOR) + 
	 4 * (winer == white ? RANK(sqw) : 7-RANK(sqw));

/**************************************************************************
 *
 * Pawn is outside the square of the king 
 *
 **************************************************************************/
   if (~SquarePawnMask[winer][sq] & board.b[loser][king])
   {
      if (!(MoveArray[king][sql] & SquarePawnMask[winer][sq]))
         return (winer == side ? s : -s);
      if (winer == side)
         return (s);
   }

/**************************************************************************
 *
 *  Friendly king is on same or adjacent file to the pawn, and the pawn is 
 *  on a file other than a rook file and ...
 *
 **************************************************************************/
   if (ROW(sq) != 0 && ROW(sq) != 7 &&
        ((IsolaniMask[ROW(sq)] | FileBit[ROW(sq)]) & board.b[winer][king]))
   {

/**************************************************************************
 *
 * a. friendly king is 2 ranks more advanced than the pawn 
 * b. friendly king is 1 rank more advanced than the pawn 
 *    i.  The friendly king is on the sixth rank.
 *    ii. The enemy king does not have direct opposition by being 2 ranks
 *        in front of the friendly king and on the same file.
 * c. friendly king is same rank as pawn
 *    i.  The enemy king is not 2-4 ranks more advanced that the pawn.
 *    ii. The pawn is on the sixth rank and the enemy king does not have
 *        direct opposition.
 * d. pawn is on the 7th rank, friendly king is on sixth rank and
 *    i.  The enemy king is not on the queening square.
 *    ii. The enemy is on the queening square but both kings are in the same
 *        file.
 * 
 **************************************************************************/
      if (winer == white)
      {
         if (RANK(sqw) == RANK(sq) + 2)
            return (winer == side ? s : -s);
         if (RANK(sqw) == RANK(sq) + 1)
         {
	    if (RANK(sqw) == 5)
               return (winer == side ? s : -s);
            if (sqw < A6) 
	    {
	       if (sqw+16 == sql && winer == side)
		  return (0);
	       else
                  return (winer == side ? s : -s);
	    }
         }
         if (RANK(sqw) == RANK(sq))
         {
            if ((RANK(sql) - RANK(sq) < 2 || RANK(sql) - RANK(sq) > 4) &&
		 winer == side)
	       return (s);
            if ((RANK(sql) - RANK(sq) < 1 || RANK(sql) - RANK(sq) > 5) &&
		 loser == side)
               return (-s);
	    if (RANK(sq) == 5 && sqw+16 != sql)
               return (winer == side ? s : 0);
	 }
	 if (RANK(sq) == 6 && RANK(sqw) == 5)
         {
	    if (sql != sq+8)
               return (winer == side ? s : 0);
	    if (sql == sq+8 && sql == sqw+16)
               return (winer == side ? s : 0);
	 }
      } 
      else
      {
         if (RANK(sqw) == RANK(sq) - 2)
            return (winer == side ? s : -s);
         if (RANK(sqw) == RANK(sq) - 1)
         {
	    if (RANK(sqw) == 2)
               return (winer == side ? s : -s);
	    if (sqw > H3)
	    {
	       if (sqw-16 == sql && winer == side)
	          return (0);
	       else
                  return (winer == side ? s : -s);
	    }	
	 }
         if (RANK(sqw) == RANK(sq))
         {
            if ((RANK(sq) - RANK(sql) < 2 || RANK(sq) - RANK(sql) > 4) &&
		 winer == side)
	       return (s);
            if ((RANK(sq) - RANK(sql) < 1 || RANK(sq) - RANK(sql) > 5) &&
		 loser == side)
	       return (-s);
	    if (RANK(sq) == 5 && sqw+16 != sql)
               return (winer == side ? s : 0);
	 }
	 if (RANK(sq) == 1 && RANK(sqw) == 2)
         {
	    if (sql != sq-8)
               return (winer == side ? s : 0);
	    if (sql == sq-8 && sql == sqw-16)
               return (winer == side ? s : 0);
	 }
      } 
   }  

   return (0);
}


int KBNK[64] = 
{
   0, 10, 20, 30, 40, 50, 60, 70,
  10, 20, 30, 40, 50, 60, 70, 60,
  20, 30, 40, 50, 60, 70, 60, 50,
  30, 40, 50, 60, 70, 60, 50, 40,
  40, 50, 60, 70, 60, 50, 40, 30,
  50, 60, 70, 60, 50, 40, 30, 20,
  60, 70, 60, 50, 40, 30, 20, 10,
  70, 60, 50, 40, 30, 20, 10,  0
};

int ScoreKBNK (int side, int loser)
/****************************************************************************
 *
 *  My very own KBNK routine!
 *
 ****************************************************************************/
{
   int s, winer, sq1, sq2, sqB;

   winer = 1^loser;
   sqB = board.king[loser];
   if (board.b[winer][bishop] & WHITESQUARES)
      sqB = RANK(sqB)*8 + 7 - ROW(sqB);
   sq1 = board.king[winer];
   sq2 = board.king[loser];
   s = 300 - 6 * taxicab[sq1][sq2];
   s -= KBNK[sqB];
   s -= EndingKing[sq2];
   s -= taxicab[leadz(board.b[winer][knight])][sq2];
   s -= taxicab[leadz(board.b[winer][bishop])][sq2];

   /*  King in the central 4x4 region is good! */
   if (board.b[winer][king] & ULL(0x00003C3C3C3C0000))
      s += 20;
   if (side == loser)
      s = -s;
   s += MATERIAL;

   return (s); 
}


static const BitBoard nn[2] = { ULL(0x4200000000000000), ULL(0x0000000000000042) };
static const BitBoard bb[2] = { ULL(0x2400000000000000), ULL(0x0000000000000024) };

int ScoreDev (short side)
/***************************************************************************
 *
 *  Calculate the development score for side (for opening only).
 *  Penalize the following.
 *  .  Uncastled and cannot castled
 *  .  Undeveloped knights and bishops
 *  .  Early queen move.
 *
 ***************************************************************************/
{
   int s;
   int sq;
   BitBoard c;

   /* Calculate whether we are developed */
   c = (board.b[side][knight] & nn[side]) | (board.b[side][bishop] & bb[side]);
   s = nbits(c) * -8;

   /* If we are castled or beyond the 20th move, no more ScoreDev */
   if (board.castled[side] || GameCnt >= 38)
      return (s);

   s += NOTCASTLED;

   /* If the king is moved, nail it, otherwise check rooks */
   if (Mvboard[board.king[side]] > 0) 
      s += KINGMOVED;

   /* Discourage rook moves */
   c = board.b[side][rook];
   while (c) {
     sq = leadz(c);
     CLEARBIT(c, sq);
     if (Mvboard[sq] > 0)
       s += ROOKMOVED;
   }

   /* Penalize a queen that moves at all */
   if (board.b[side][queen])
   {
      sq = leadz (board.b[side][queen]);
      if (Mvboard[sq] > 0)
         s += EARLYQUEENMOVE;
         /* s += Mvboard[sq] * EARLYQUEENMOVE; */
   }

   /* Discourage repeat minor piece moves */
   c = board.b[side][knight] | board.b[side][bishop];
   while (c) {
     sq = leadz(c);
     CLEARBIT(c, sq);
     if (Mvboard[sq] > 1)
	s += EARLYMINORREPEAT;
	/* s += Mvboard[sq] * EARLYMINORREPEAT; */
   }

   /* Discourage any wing pawn moves */
/*   c = board.b[side][pawn] & (FileBit[0]|FileBit[1]|FileBit[6]|FileBit[7]); */
   c = board.b[side][pawn] & ULL(0xc3c3c3c3c3c3c3c3);
   while (c) {
     sq = leadz(c);
     CLEARBIT(c, sq);
     if (Mvboard[sq] > 0) 
	s += EARLYWINGPAWNMOVE;
   }

   /* Discourage any repeat center pawn moves */
/*   c = board.b[side][pawn] & (FileBit[2]|FileBit[3]|FileBit[4]|FileBit[5]); */
   c = board.b[side][pawn] & ULL(0x3c3c3c3c3c3c3c3c);
   while (c) {
     sq = leadz(c);
     CLEARBIT(c, sq);
     if (Mvboard[sq] > 1) 
	s += EARLYCENTERPREPEAT;
   }

   return (s);
}


/*  Array of pointer to functions  */
static int (*ScorePiece[7]) (short) =
{ NULL, ScoreP, ScoreN, ScoreB, ScoreR, ScoreQ, ScoreK };


int Evaluate (int alpha, int beta)
/****************************************************************************
 *
 *  First check to see if this position can be specially dealt with.
 *  E.g. if our bounds indicate that we are looking for a mate score,
 *  then just return the material score.  Nothing else is important.
 *  If its a KPK endgame, call our KPK routine.
 *  If one side has a lone king & the winning side has no pawns then call
 *  the LoneKing() mating driver routine.  Note that there is enough
 *  mating material as we have already check for insufficient mating material
 *  in the call to EvaluateDraw() in search()/quiesce().
 *
 ****************************************************************************/
{
   int side, xside;
   int piece, s, s1, score;
   int npiece[2];
   BitBoard *b;

   side = board.side;
   xside = 1 ^ side;

   /*  If we are looking for a MATE, just return the material */
   if (alpha > MATE-255 || beta < -MATE+255)
      return (MATERIAL); 

   /*  A KPK endgame. */
   if (board.material[white]+board.material[black] == ValueP)
      return (KPK (side));  

   /*  One side has a lone king and other side has no pawns */
   if (board.material[xside] == 0 && board.b[side][pawn] == NULLBITBOARD)
      return LoneKing (side, xside);
   if (board.material[side] == 0 && board.b[xside][pawn] == NULLBITBOARD)
      return LoneKing (side, side);

/****************************************************************************
 *
 *  Lets try a lazy evaluation.  In this stage, we should evaluate all those
 *  features that gives big bonus/penalties.  E.g. squares around king is
 *  attacked by enemy pieces, 2 rooks on 7th rank, runaway passed pawns etc.
 *  This will be the direction, so things will continue to change in this
 *  section.
 *
 ****************************************************************************/
   EvalCall++;
   phase = PHASE;
   b = board.b[white];
   pieces[white] = b[knight] | b[bishop] | b[rook] | b[queen];
   npiece[white] = nbits (pieces[white]);
   b = board.b[black];
   pieces[black] = b[knight] | b[bishop] | b[rook] | b[queen];
   npiece[black] = nbits (pieces[black]);
   s1 = MATERIAL;

   if ((s1 + maxposnscore[side] < alpha || s1 - maxposnscore[xside] > beta) &&
	phase <= 6)
   {
      score = s1;
      goto next;
   }
   s = 0;
   s += ScoreDev (side) - ScoreDev (xside);
   s += ScoreP (side) - ScoreP (xside);
   s += ScoreK (side) - ScoreK (xside);
   s += BishopTrapped (side) - BishopTrapped (xside);
   s += DoubleQR7 (side) - DoubleQR7 (xside);

   s1 = s + MATERIAL;

/**************************************************************************
 *
 *  See if we can have a lazy evaluation cut.  Otherwise its a slow eval.
 * 
 **************************************************************************/

   if (s1 + lazyscore[side] < alpha || s1 - lazyscore[side] > beta)
   {
      score = s1;
   }
   else
   {
      EvalCnt++;

	GenAtaks();
	s1 = HUNGPENALTY * ( EvalHung(side) - EvalHung(xside) );
	FindPins(&pinned);

      for (piece = knight; piece < king; piece++)
      {
         s1 += (*ScorePiece[piece]) (side) - (*ScorePiece[piece]) (xside);
      }
      lazyscore[side] = MAX (s1, lazyscore[side]);
      maxposnscore[side] = MAX (maxposnscore[side], s + s1);
      score = s + s1 + MATERIAL;
   }

/***************************************************************************
 *
 *  Trade down bonus code.  When ahead, trade pieces & not pawns;
 *
 ***************************************************************************/
next:
   if (MATERIAL >= 200)
   {
      score += (RootPieces - nbits(pieces[white] | pieces[black])) * TRADEPIECE;
      score -= (RootPawns - nbits(board.b[white][pawn] | board.b[black][pawn])) 
			* TRADEPAWNS;
   }
   else if (MATERIAL <= -200)
   {
      score -= (RootPieces - nbits(pieces[white] | pieces[black])) * TRADEPIECE;
      score += (RootPawns - nbits(board.b[white][pawn] | board.b[black][pawn]))
			 * TRADEPAWNS;
   }
      
/***************************************************************************
 *
 *  Opposite color bishops is drawish.
 *
 ***************************************************************************/
   if (ENDING && pieces[white] == board.b[white][bishop] && 
                 pieces[black] == board.b[black][bishop] &&
       ((pieces[white] & WHITESQUARES && pieces[black] & BLACKSQUARES) ||
	(pieces[white] & BLACKSQUARES && pieces[black] & WHITESQUARES)))
   {
      score /= 2;
   }
    
/***************************************************************************
 *
 *  When one side has no mating material, then his score can never be > 0.
 *
 ***************************************************************************/
   if (score > 0 && !board.b[side][pawn] && (board.material[side] < ValueR
        || pieces[side] == board.b[side][knight]))
      score = 0;
   if (score < 0 && !board.b[xside][pawn] && (board.material[xside] < ValueR
        || pieces[xside] == board.b[xside][knight]))
      score = 0;
   
   return (score);
}


short EvaluateDraw (void)
/***************************************************************************
 *
 *  This routine is called by search() and quiesce() before anything else
 *  is done.  Its purpose it to check if the current position is a draw.
 *  0.  50-move draw.
 *  1.  If there are any pawns, it is not.
 *  2.  If both sides has anything less than a rook, draw.
 *  3.  If both sides has <= 2 knights only, draw.
 *  4.  If its a KBBK and bishops of same color, draw.
 *
 ***************************************************************************/
{
   BitBoard *w, *b;
   int wm, bm, wn, bn;

   /* 
    * Exception - if we are close to a pawn move, promotion 
    * or capture it is possible a forced mate will follow.
    * So we assume not drawn for 2 moves.
    */

   if ( (GameCnt-Game50) < 5 )
     return (false);

   /* 50 move rule */
   if ( (GameCnt-Game50) > 100 )
     return (true);

   w = board.b[white];
   b = board.b[black];
   if (w[pawn] != 0 || b[pawn] != 0)
      return (false);

   wm = board.material[white];
   bm = board.material[black];
   wn = nbits (w[knight]);
   bn = nbits (b[knight]);
   if  ((wm<ValueR || (wm==2*ValueN && wn==2)) &&
        (bm<ValueR || (bm==2*ValueN && bn==2)))
      return (true); 

   if (wm < ValueR)
   {
      if (bm == 2*ValueB && 
         ( nbits(board.b[black][bishop] & WHITESQUARES) == 2 ||
           nbits(board.b[black][bishop] & BLACKSQUARES) == 2 ))
      return (true);
   }
   if (bm < ValueR)
   {
      if (wm == 2*ValueB && 
         ( nbits(board.b[white][bishop] & WHITESQUARES) == 2 ||
           nbits(board.b[white][bishop] & BLACKSQUARES) == 2 ))
      return (true);
   }

   return (false);
}
