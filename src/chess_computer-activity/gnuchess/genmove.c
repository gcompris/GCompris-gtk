/* GNU Chess 5.0 - genmove.c - move generator code
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

#include <config.h>
#include <stdio.h>

#include "common.h"

const short raybeg[7] = { 0, 0, 0, 0, 4, 0, 0 };
const short rayend[7] = { 0, 0, 0, 4, 8, 8, 0 };

static leaf *node;

#define ADDMOVE(a,b,c)            \
  do {                            \
    node->move = MOVE(a,b) | (c); \
    node++;                       \
  } while (0)

#define ADDPROMOTE(a,b)           \
  do {                            \
    ADDMOVE (a, b, QUEENPRM);     \
    ADDMOVE (a, b, KNIGHTPRM);    \
    ADDMOVE (a, b, ROOKPRM);      \
    ADDMOVE (a, b, BISHOPPRM);    \
  } while (0)

static inline void BitToMove (short f, BitBoard b)
/***************************************************************************
 *
 *  Convert a bitboard into a list of moves.  These are stored
 *  into the tree.  f is the origin square.
 *
 ***************************************************************************/
{
   int t;

   while (b)
   {
      t = leadz (b);
      CLEARBIT (b, t);
      ADDMOVE (f, t, 0);
   }
}



void GenMoves (short ply)
/****************************************************************************
 *
 *  My bitboard move generator.  Let's see how fast we can go!
 *  This routine generates pseudo-legal moves.
 *
 ****************************************************************************/
{
   int side;
   int piece, sq, t, ep;
   BitBoard b, c, d, e, friends, notfriends, blocker, notblocker;
   BitBoard *a;

   side = board.side;
   a = board.b[side];
   friends = board.friends[side];
   notfriends = ~friends;
   blocker = board.blocker;
   notblocker = ~blocker;
   node = TreePtr[ply + 1];
   ep = board.ep;

   /* Knight & King */
   for (piece = knight; piece <= king; piece += 4)
   {
      b = a[piece];
      while (b)
      {
         sq = leadz (b);
         CLEARBIT (b, sq);
         BitToMove (sq, MoveArray[piece][sq] & notfriends);
      }
   }

   /* Bishops */
   b = a[bishop];
   while (b)
   {
      sq = leadz (b);
      CLEARBIT (b, sq);
      d = BishopAttack(sq);
      BitToMove (sq, d & notfriends);
   }

   /* Rooks */
   b = a[rook];
   while (b)
   {
      sq = leadz (b);
      CLEARBIT (b, sq);
      d = RookAttack(sq);
      BitToMove (sq, d & notfriends);
   }

   /* Queen */
   b = a[queen];
   while (b)
   {
      sq = leadz (b);
      CLEARBIT (b, sq);
      d = QueenAttack(sq);
      BitToMove (sq, d & notfriends);
   }

   /*  White pawn moves  */
   e = (board.friends[1^side] | (ep > -1 ? BitPosArray[ep] : NULLBITBOARD));
   if (side == white)
   {
      c = (a[pawn] >> 8) & notblocker;		/* 1 square forward */
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         if (t >= 56)				/* promotion */
         {
            ADDPROMOTE (t-8, t);
         }
         else
            ADDMOVE (t-8, t, 0);
      }

      b = a[pawn] & RankBit[1];			/* all pawns on 2nd rank */
      c = (b >> 8) & notblocker;
      c = (c >> 8) & notblocker;		/* 2 squares forward */
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         ADDMOVE (t-16, t, 0);
      }

      b = a[pawn] & ~FileBit[0]; 		/* captures to the left */
      c = (b >> 7) & e;
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         if (t >= 56)				/* promotion */
         {
            ADDPROMOTE (t-7, t);
         }
         else if (ep == t)
	 {
	    ADDMOVE (t-7, t, ENPASSANT);
         }
	 else
	 {
            ADDMOVE (t-7, t, 0);
	 }
      }

      b = a[pawn] & ~FileBit[7]; 		/* captures to the right */
      c = (b >> 9) & e;
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         if (t >= 56)				/* promotion */
         {
            ADDPROMOTE (t-9, t);
         }
	 else if (ep == t)
	 {
	    ADDMOVE (t-9, t, ENPASSANT);
	 }
         else
	 {
            ADDMOVE (t-9, t, 0);
	 }
      }
   }


   /*  Black pawn forward moves   */
   if (side == black)
   {
      c = (a[pawn] << 8) & notblocker;		
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         if (t <= 7)				/* promotion */
         {
            ADDPROMOTE (t+8, t);
         }
         else
            ADDMOVE (t+8, t, 0);
      }

      b = a[pawn] & RankBit[6];                 /* all pawns on 7th rank */
      c = (b << 8) & notblocker;
      c = (c << 8) & notblocker;
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         ADDMOVE (t+16, t, 0);
      }

      b = a[pawn] & ~FileBit[7];		/* captures to the left */
      c = (b << 7) & e;
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         if (t <= 7)				/* promotion */
         {
            ADDPROMOTE (t+7, t);
         }
	 else if (ep == t)
         {
	    ADDMOVE (t+7, t, ENPASSANT);
         }
         else
	 {
            ADDMOVE (t+7, t, 0);
	 }
      }

      b = a[pawn] & ~FileBit[0];		/* captures to the right */
      c = (b << 9) & e;
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         if (t <= 7)				/* promotion */
         {
            ADDPROMOTE (t+9, t);
         }
         else if (ep == t)
         {
            ADDMOVE (t+9, t, ENPASSANT);
	 }
	 else
	 {
            ADDMOVE (t+9, t, 0);
	 }
      }
   }

   /* Castling code */
   b = board.b[side][rook];
   if (side == white && (board.flag & WKINGCASTLE) && (b & BitPosArray[H1]) &&
       !(FromToRay[E1][G1] & blocker) && 
       !SqAtakd (E1, black) && !SqAtakd (F1, black) && !SqAtakd (G1, black))
   {
           ADDMOVE (E1, G1, CASTLING);  
   }
   if (side == white && (board.flag & WQUEENCASTLE) && (b & BitPosArray[A1]) &&
       !(FromToRay[E1][B1] & blocker) &&
       !SqAtakd (E1, black) && !SqAtakd (D1, black) && !SqAtakd (C1, black))
   {
           ADDMOVE (E1, C1, CASTLING);  
   }
   if (side == black && (board.flag & BKINGCASTLE) && (b & BitPosArray[H8]) &&
       !(FromToRay[E8][G8] & blocker) &&
       !SqAtakd (E8, white) && !SqAtakd (F8, white) && !SqAtakd (G8, white))
   {
           ADDMOVE (E8, G8, CASTLING);  
   }
   if (side == black && (board.flag & BQUEENCASTLE) && (b & BitPosArray[A8]) &&
       !(FromToRay[E8][B8] & blocker) &&
       !SqAtakd (E8, white) && !SqAtakd (D8, white) && !SqAtakd (C8, white))
   {
           ADDMOVE (E8, C8, CASTLING);  
   }

   /* Update tree pointers and count */
   TreePtr[ply + 1] = node;
   GenCnt += TreePtr[ply + 1] - TreePtr[ply];
}


void GenNonCaptures (short ply)
/****************************************************************************
 *
 *  Here I generate only non-captures.  Promotions are considered
 *  as captures and are not generated.
 *
 ****************************************************************************/
{
   int side;
   int piece, sq, t, ep;
   BitBoard b, c, d, friends, notfriends, blocker, notblocker;
   BitBoard *a;

   side = board.side;
   a = board.b[side];
   friends = board.friends[side];
   notfriends = ~friends;
   blocker = board.blocker;
   notblocker = ~blocker;
   node = TreePtr[ply + 1];
   ep = board.ep;

   /* Knight & King */
   for (piece = knight; piece <= king; piece += 4)
   {
      b = a[piece];
      while (b)
      {
         sq = leadz (b);
         CLEARBIT (b, sq);
         BitToMove (sq, MoveArray[piece][sq] & notblocker);
      }
   }

   /* Bishops */
   b = a[bishop];
   while (b)
   {
      sq = leadz (b);
      CLEARBIT (b, sq);
      d = BishopAttack(sq);
      BitToMove (sq, d & notblocker);
   }

   /* Rooks */
   b = a[rook];
   while (b)
   {
      sq = leadz (b);
      CLEARBIT (b, sq);
      d = RookAttack(sq);
      BitToMove (sq, d & notblocker);
   }

   /* Queen */
   b = a[queen];
   while (b)
   {
      sq = leadz (b);
      CLEARBIT (b, sq);
      d = QueenAttack(sq);
      BitToMove (sq, d & notblocker);
   }

   /*  White pawn moves  */
   if (side == white)
   {
      c = (a[pawn] >> 8) & notblocker;		/* 1 square forward */
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
	 if (t < 56)
            ADDMOVE (t-8, t, 0);
      }

      b = a[pawn] & RankBit[1];			/* all pawns on 2nd rank */
      c = (b >> 8) & notblocker;
      c = (c >> 8) & notblocker;		/* 2 squares forward */
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         ADDMOVE (t-16, t, 0);
      }
   }


   /*  Black pawn forward moves   */
   if (side == black)
   {
      c = (a[pawn] << 8) & notblocker;		
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
	 if ( t > 7)
            ADDMOVE (t+8, t, 0);
      }

      b = a[pawn] & RankBit[6];                 /* all pawns on 7th rank */
      c = (b << 8) & notblocker;
      c = (c << 8) & notblocker;
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         ADDMOVE (t+16, t, 0);
      }
   }

   /* Castling code */
   b = board.b[side][rook];
   if (side == white && (board.flag & WKINGCASTLE) && (b & BitPosArray[7]) &&
       !(FromToRay[4][6] & blocker) &&
       !SqAtakd (4, black) && !SqAtakd (5, black) && !SqAtakd (6, black))
   {
           ADDMOVE (4, 6, CASTLING);  
   }
   if (side == white && (board.flag & WQUEENCASTLE) && (b & BitPosArray[0]) &&
       !(FromToRay[4][1] & blocker) &&
       !SqAtakd (4, black) && !SqAtakd (3, black) && !SqAtakd (2, black))
   {
           ADDMOVE (4, 2, CASTLING);  
   }
   if (side == black && (board.flag & BKINGCASTLE) && (b & BitPosArray[63]) &&
       !(FromToRay[60][62] & blocker) &&
       !SqAtakd (60, white) && !SqAtakd (61, white) && !SqAtakd (62, white))
   {
           ADDMOVE (60, 62, CASTLING);  
   }
   if (side == black && (board.flag & BQUEENCASTLE) && (b & BitPosArray[56]) &&
       !(FromToRay[60][57] & blocker) &&
       !SqAtakd (60, white) && !SqAtakd (59, white) && !SqAtakd (58, white))
   {
           ADDMOVE (60, 58, CASTLING);  
   }

   /* Update tree pointers and count */
   TreePtr[ply + 1] = node;
   GenCnt += TreePtr[ply + 1] - TreePtr[ply];
}


void GenCaptures (short ply)
/****************************************************************************
 *
 *  This routine generates captures.  En passant and pawn promotions
 *  are included.
 *
 ****************************************************************************/
{
   int side;
   int piece, sq, t, ep;
   BitBoard b, c, friends, notfriends, enemy, blocker;
   BitBoard *a;

   side = board.side;
   a = board.b[side];
   friends = board.friends[side];
   notfriends = ~friends;
   enemy = board.friends[1^side];
   blocker = board.blocker;
   node = TreePtr[ply + 1];
   ep = board.ep;

   /* Knight  & King */
   for (piece = knight; piece <= king; piece += 4)
   {
      b = a[piece];
      while (b)
      {
         sq = leadz (b);
         CLEARBIT (b, sq);
         BitToMove (sq, MoveArray[piece][sq] & enemy);
      }
   }

   /* Bishop */
   b = a[bishop];
   while (b)
   {
      sq = leadz (b);
      CLEARBIT (b, sq);
      c = BishopAttack(sq);
      BitToMove (sq, c & enemy);
   }

   /* Rook */
   b = a[rook];
   while (b)
   {
      sq = leadz (b);
      CLEARBIT (b, sq);
      c = RookAttack(sq);
      BitToMove (sq, c & enemy);
   }

   /* Queen */
   b = a[queen];
   while (b)
   {
      sq = leadz (b);
      CLEARBIT (b, sq);
      c = QueenAttack(sq);
      BitToMove (sq, c & enemy);
   }

   /*  White pawn moves  */
   if (side == white)
   {
      b = a[pawn] & RankBit[6];			/* all pawns on 7 rank */
      c = (b >> 8) & ~blocker;			/* 1 square forward */
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         ADDPROMOTE (t-8, t);
      }

      b = a[pawn] & ~FileBit[0]; 		/* captures to the left */
      c = (b >> 7) & (board.friends[1^side] | (ep > -1 ? BitPosArray[ep] : ULL(0)));
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         if (t >= 56)				/* promotion */
         {
            ADDPROMOTE (t-7, t);
         }
         else if (ep == t)
	 {
	    ADDMOVE (t-7, t, ENPASSANT);
         }
	 else
	 {
            ADDMOVE (t-7, t, 0);
	 }
      }

      b = a[pawn] & ~FileBit[7]; 		/* captures to the right */
      c = (b >> 9) & (board.friends[1^side] | (ep > -1 ? BitPosArray[ep] : ULL(0)));
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         if (t >= 56)				/* promotion */
         {
            ADDPROMOTE (t-9, t);
         }
	 else if (ep == t)
	 {
	    ADDMOVE (t-9, t, ENPASSANT);
	 }
         else
	 {
            ADDMOVE (t-9, t, 0);
	 }
      }
   }

   /*  Black pawn forward moves   */
   if (side == black)
   {
      b = a[pawn] & RankBit[1];			/* all pawns on 2nd rank */
      c = (b << 8) & ~blocker;		
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         ADDPROMOTE (t+8, t);
      }

      b = a[pawn] & ~FileBit[7];		/* captures to the left */
      c = (b << 7) & (board.friends[1^side] | (ep > -1 ? BitPosArray[ep] : ULL(0)));
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         if (t <= 7)				/* promotion */
         {
            ADDPROMOTE (t+7, t);
         }
	 else if (ep == t)
         {
	    ADDMOVE (t+7, t, ENPASSANT);
         }
         else
	 {
            ADDMOVE (t+7, t, 0);
	 }
      }

      b = a[pawn] & ~FileBit[0];		/* captures to the right */
      c = (b << 9) & (board.friends[1^side] | (ep > -1 ? BitPosArray[ep] : ULL(0)));
      while (c)
      {
         t = leadz (c);
         CLEARBIT (c, t);
         if (t <= 7)				/* promotion */
         {
            ADDPROMOTE (t+9, t);
         }
         else if (ep == t)
         {
            ADDMOVE (t+9, t, ENPASSANT);
	 }
	 else
	 {
            ADDMOVE (t+9, t, 0);
	 }
      }
   }


   /* Update tree pointers and count */
   TreePtr[ply + 1] = node;
   GenCnt += TreePtr[ply + 1] - TreePtr[ply];
}


void GenCheckEscapes (short ply)
/**************************************************************************
 *
 *  The king is in check, so generate only moves which get the king out
 *  of check.  
 *  Caveat:  The special case of an enpassant capture must be taken into
 *  account too as the captured pawn could be the checking piece.
 *
 **************************************************************************/
{
   int side, xside;
   int kingsq, chksq, sq, sq1, epsq, dir;
   BitBoard checkers, b, c, p, escapes; 
   escapes = NULLBITBOARD;
   side = board.side;
   xside = 1 ^ side;
/*   TreePtr[ply + 1] = TreePtr[ply];  */
   node = TreePtr[ply + 1];
   kingsq = board.king[side];
   checkers = AttackTo (kingsq, xside);
   p = board.b[side][pawn];

   if (nbits (checkers) == 1)
   {
      /*  Captures of checking pieces (except by king) */
      chksq = leadz (checkers);
      b = AttackTo (chksq, side); 
      b &= ~board.b[side][king];
      while (b)
      {
         sq = leadz (b);
         CLEARBIT (b, sq);
         if (!PinnedOnKing (sq, side))
	 {
	    if (cboard[sq] == pawn && (chksq <= H1 || chksq >= A8))
	    {
               ADDPROMOTE (sq, chksq);
	    }
	    else
            ADDMOVE (sq, chksq, 0);
	 }
      }

      /*  Maybe enpassant can help  */
      if (board.ep > -1)
      {
         epsq = board.ep;
         if (epsq + (side == white ? -8 : 8) == chksq)
         {
	    b = MoveArray[ptype[1^side]][epsq] & p;
            while (b)
	    {
	       sq = leadz (b);
	       CLEARBIT (b, sq);
               if (!PinnedOnKing (sq, side))
	          ADDMOVE (sq, epsq, ENPASSANT);
            }
         }
      }

      /* Lets block/capture the checking piece */
      if (slider[cboard[chksq]])
      {
         c = FromToRay[kingsq][chksq] & NotBitPosArray[chksq];
         while (c)
         {
            sq = leadz (c);
            CLEARBIT (c, sq);
            b = AttackTo (sq, side); 
            b &= ~(board.b[side][king] | p);

            /* Add in pawn advances */
            if (side == white && sq > H2)
            {
               if (BitPosArray[sq-8] & p) 
	          b |= BitPosArray[sq-8];
	       if (RANK(sq) == 3 && cboard[sq-8] == empty && 
			(BitPosArray[sq-16] & p))
	          b |= BitPosArray[sq-16];
            }
            if (side == black && sq < H7)
            {
               if (BitPosArray[sq+8] & p) 
	          b |= BitPosArray[sq+8];
	       if (RANK(sq) == 4 && cboard[sq+8] == empty && 
			(BitPosArray[sq+16] & p))
	          b |= BitPosArray[sq+16];
            }
            while (b)
            {
               sq1 = leadz (b);
               CLEARBIT (b, sq1);
               if (!PinnedOnKing (sq1, side))
	       {
	          if (cboard[sq1] == pawn && (sq > H7 || sq < A2))
		  {
                     ADDPROMOTE (sq1, sq);
		  }
		  else
                     ADDMOVE (sq1, sq, 0);
	       }
            }
         }
      }
   }

   /* If more than one checkers, move king to get out of check */
   if (checkers)
    escapes = MoveArray[king][kingsq] & ~board.friends[side];
   while (checkers)
   {
      chksq = leadz (checkers);
      CLEARBIT (checkers, chksq);
      dir = directions[chksq][kingsq];
      if (slider[cboard[chksq]])
         escapes &= ~Ray[chksq][dir];
   }
   while (escapes)
   {
      sq = leadz (escapes);
      CLEARBIT (escapes, sq);
      if (!SqAtakd (sq, xside))
         ADDMOVE (kingsq, sq, 0);
   }

   /* Update tree pointers and count */
   TreePtr[ply + 1] = node;
   GenCnt += TreePtr[ply + 1] - TreePtr[ply];
   return;
}


void FilterIllegalMoves (short ply)
/**************************************************************************
 *
 *  All the illegal moves in the current ply is removed.
 *
 **************************************************************************/
{
   leaf *p;
   int side, xside;
   int check, sq;

   side = board.side;
   xside = 1^side;
   sq = board.king[side];
   for (p = TreePtr[ply]; p < TreePtr[ply+1]; p++)
   {
      MakeMove (side, &p->move);
      if (cboard[TOSQ(p->move)] != king)
         check = SqAtakd (sq, xside); 
      else 
         check = SqAtakd (TOSQ(p->move), xside); 
      UnmakeMove (xside, &p->move);

      if (check)	/* its an illegal move */
      {
         --TreePtr[ply+1];
         *p = *TreePtr[ply+1];
         --p; 
	 GenCnt--;
      } 
   }
}
