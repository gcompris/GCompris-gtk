/* GNU Chess 5.0 - swap.c - static exchange evaluator code
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
#include <string.h>
#include "common.h"

static const int xray[7] = { 0, 1, 0, 1, 1, 1, 0 };

int SwapOff (int move)
/****************************************************************************
 *
 *  A Static Exchange Evaluator (or SEE for short).
 *  First determine the target square.  Create a bitboard of all squares
 *  attacking the target square for both sides.  Using these 2 bitboards,
 *  we take turn making captures from smallest piece to largest piece.
 *  When a sliding piece makes a capture, we check behind it to see if
 *  another attacker piece has been exposed.  If so, add this to the bitboard
 *  as well.  When performing the "captures", we stop if one side is ahead
 *  and doesn't need to capture, a form of pseudo-minimaxing.
 *
 ****************************************************************************/
{
   int f, t, sq, piece, lastval;
   int side, xside; 
   int swaplist[MAXPLYDEPTH], n;
   BitBoard b, c, *d, *e, r;

   f = FROMSQ (move);
   t = TOSQ (move);
   side = ((board.friends[white] & BitPosArray[f]) ? white : black);
   xside = 1^side;

   /*  Squares attacking t for side and xside  */
   b = AttackTo (t, side);
   c = AttackTo (t, xside);
   CLEARBIT(b, f);
   if (xray[cboard[f]])
      AddXrayPiece (t, f, side, &b, &c);

   d = board.b[side];
   e = board.b[xside]; 
   if (move & PROMOTION)
   {
      swaplist[0] = Value[PROMOTEPIECE (move)] - ValueP;
      lastval = -Value[PROMOTEPIECE (move)];
   }
   else
   {
      swaplist[0] = (move & ENPASSANT ? ValueP : Value[cboard[t]]);
      lastval = -Value[cboard[f]];
   }
   n = 1;
   while (1)
   {
      if (c == NULLBITBOARD)
         break;
      for (piece = pawn; piece <= king; piece++)
      {
         r = c & e[piece]; 
	 if (r)
         {
	    sq = leadz (r);
	    CLEARBIT (c, sq);
	    if (xray[piece])
	       AddXrayPiece (t, sq, xside, &c, &b);
	    swaplist[n] = swaplist[n-1] + lastval;
            n++;
	    lastval = Value[piece];
	    break;
         }
      }

      if (b == NULLBITBOARD)
         break;
      for (piece = pawn; piece <= king; piece++)
      {
         r = b & d[piece]; 
	 if (r)
         {
	    sq = leadz (r);
	    CLEARBIT (b, sq);
	    if (xray[piece])
	       AddXrayPiece (t, sq, side, &b, &c);
	    swaplist[n] = swaplist[n-1] + lastval;
            n++;
	    lastval = -Value[piece];
	    break;
         }
      }
   }

/****************************************************************************
 *
 *  At this stage, we have the swap scores in a list.  We just need to 
 *  mini-max the scores from the bottom up to the top of the list.
 *
 ****************************************************************************/
   --n;
   while (n)
   {
      if (n & 1)
      {
         if (swaplist[n] <= swaplist[n-1])
	    swaplist[n-1] = swaplist[n]; 
      }
      else
      {
         if (swaplist[n] >= swaplist[n-1])
	    swaplist[n-1] = swaplist[n]; 
      }
      --n;
   }
   return (swaplist[0]);
}


void AddXrayPiece (int t, int sq, int side, BitBoard *b, BitBoard *c)
/***************************************************************************
 *
 *  The purpose of this routine is to find a piece which attack through
 *  another piece (e.g. two rooks, Q+B, B+P, etc.)  Side is the side attacking
 *  the square where the swapping is to be done.
 *
 ***************************************************************************/
{
   int dir, nsq, piece;
   BitBoard a;

   dir = directions[t][sq];
   a = Ray[sq][dir] & board.blocker;
   if (a == NULLBITBOARD)
      return;
   nsq = (t < sq ? leadz (a) : trailz (a));
   piece = cboard[nsq];
   if ((piece == queen) || (piece == rook && dir > 3) || 
			    (piece == bishop && dir < 4))
   {
      if (BitPosArray[nsq] & board.friends[side])
         *b |= BitPosArray[nsq];
      else
         *c |= BitPosArray[nsq];
   }
   return;
}
