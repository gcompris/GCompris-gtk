/* GNU Chess 5.0 - ttable.c - transposition table code
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

void TTPut (uint8_t side, uint8_t depth, uint8_t ply, int alpha, int beta, 
	    int score, int move)
/****************************************************************************
 *
 *  Uses a two-tier depth-based transposition table.  The criteria for 
 *  replacement is as follows.
 *  1.  The first element is replaced whenever we have a position with
 *      at least the same depth. In that case the element is moved to
 *      the second slot.
 *  2.  The second slot is otherwise always replaced.
 *  Problem may be that the first elements eventually get filled with
 *  outdated entries. Might add an age counter later.
 *  The & ~1 is a trick to clear the last bit making the offset even. 
 *
 ****************************************************************************/
{
   HashSlot *t;

   t = HashTab[side] + ((HashKey & TTHashMask) & ~1); 
   if (depth < t->depth)
      t++;
   else if (t->flag)
      *(t+1) = *t;

   if (t->flag)
      CollHashCnt++;
   TotalPutHashCnt++;
   t->move = move;
   t->key = HashKey;
   t->depth = depth;
   if (t->depth == 0)
      t->flag = QUIESCENT;
   else if (score >= beta)
      t->flag = LOWERBOUND;         
   else if (score <= alpha)
      t->flag = UPPERBOUND;
   else  
      t->flag = EXACTSCORE;

   if (MATESCORE(score))
      t->score = score + ( score > 0 ? ply : -ply);
   else
      t->score = score;
}


uint8_t TTGet (uint8_t side, uint8_t depth, uint8_t ply, 
	       int *score, int *move)
/*****************************************************************************
 *
 *  Probe the transposition table.  There are 2 entries to be looked at as
 *  we are using a 2-tier transposition table.
 *
 *****************************************************************************/
{
   HashSlot *t;

   TotalGetHashCnt++;
   t = HashTab[side] + ((HashKey & TTHashMask) & ~1);  
   if (HashKey != t->key && HashKey != (++t)->key)
      return (0);

   GoodGetHashCnt++;
   *move = t->move;
   *score = t->score;
   if (t->depth == 0)
      return (QUIESCENT);
   if (t->depth < depth && !MATESCORE (t->score))
      return (POORDRAFT);
   if (MATESCORE(*score))
      *score -= (*score > 0 ? ply : -ply);
   return (t->flag);
}


short TTGetPV (uint8_t side, uint8_t ply, int score, int *move)
/*****************************************************************************
 *
 *  Probe the transposition table.  There are 2 entries to be looked at as
 *  we are using a 2-tier transposition table.  This routine merely wants to
 *  get the PV from the hash, nothing else.
 *
 *****************************************************************************/
{
   HashSlot *t;
   int s;

   t = HashTab[side] + ((HashKey & TTHashMask) & ~1);  
   s = t->score;
   if (MATESCORE(s))
      s -= (s > 0 ? ply : -ply);
   if (HashKey == t->key && ((ply & 1 && score == s)||(!(ply & 1) && score == -s)))
   {
      *move = t->move;
      return (1);
   }
   t++;
   s = t->score;
   if (MATESCORE(s))
      s -= (s > 0 ? ply : -ply);
   if (HashKey == t->key && ((ply & 1 && score == s)||(!(ply & 1) && score == -s)))
   {
      *move = t->move;
      return (1);
   }
   return (0); 
}


void TTClear (void)
/****************************************************************************
 *   
 *  Zero out the transposition table.
 *
 ****************************************************************************/
{
   memset (HashTab[white], 0, HashSize * sizeof (HashSlot));
   memset (HashTab[black], 0, HashSize * sizeof (HashSlot));
}


void PTClear (void)
/****************************************************************************
 *   
 *  Zero out the pawn transposition table.
 *
 ****************************************************************************/
{
   memset (PawnTab[white], 0, PAWNSLOTS * sizeof (PawnSlot));
   memset (PawnTab[black], 0, PAWNSLOTS * sizeof (PawnSlot));
}
