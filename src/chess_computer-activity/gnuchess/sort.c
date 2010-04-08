/* GNU Chess 5.0 - sort.c - move sorting code
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
/*
 *
 */

#include <config.h>
#include <stdio.h>

#include "common.h"

#define WEIGHT  12
#define HASHSORTSCORE   INFINITY
#define KILLERSORTSCORE 1000
#define CASTLINGSCORE   500

void SortCaptures (int ply)
/***************************************************************************
 *
 *  Actually no sorting is done.  Just scores are assigned to the captures.
 *  When we need a move, we will select the highest score move from the
 *  list, place it at the top and try it.  This move might give us a cut
 *  in which case, there is no reason to sort.
 *  
 ***************************************************************************/
{
   leaf *p;
   int temp, f, t;

   for (p = TreePtr[ply]; p < TreePtr[ply+1]; p++)
   {
      f = Value[cboard[FROMSQ(p->move)]];
      t = Value[cboard[TOSQ(p->move)]];
      if (f < t)
         p->score = t - f;
      else
      {
         temp = SwapOff (p->move);
	 p->score = (temp < 0 ? -INFINITY : temp);
      }

   }
}


void SortMoves (int ply)
/*****************************************************************************
 *
 *  Sort criteria is as follows.
 *  1.  The move from the hash table
 *  2.  Captures as above.
 *  3.  Killers.
 *  4.  History.
 *  5.  Moves to the centre.
 *
 *****************************************************************************/
{
   leaf *p;
   int f, t, m, tovalue;
   int side, xside;
   BitBoard enemyP;

   side = board.side;
   xside = 1^side;
   enemyP = board.b[xside][pawn];

   for (p = TreePtr[ply]; p < TreePtr[ply+1]; p++)
   {
      p->score = -INFINITY;
      f = FROMSQ (p->move);
      t = TOSQ (p->move);
      m = p->move & MOVEMASK;

      /* Hash table move (highest score) */
      if (m == Hashmv[ply])
         p->score += HASHSORTSCORE;

      else if (cboard[t] != 0 || p->move & PROMOTION)
      {

	/* ***** SRW My Interpretation of this code *************
           * Captures normally in other places but.....         *
           *                                                    *
           * On capture we generally prefer to capture with the *
	   * with the lowest value piece so to chose between    *
           * pieces we should subtract the piece value .... but *
           *                                                    *
           * The original code was looking at some captures     *
           * last, especially where the piece was worth more    *
           * than the piece captured - KP v K in endgame.epd    *
           *                                                    *
           * So code modified to prefer any capture by adding   *
           * ValueK                                             *
           ****************************************************** */

        tovalue = (Value[cboard[t]] + Value[PROMOTEPIECE (p->move)]);
        p->score += tovalue + ValueK - Value[cboard[f]];


      }
      /* Killers */
      else if (m == killer1[ply] || m == killer2[ply])
         p->score += KILLERSORTSCORE; 
      else if (ply > 2 && (m == killer1[ply-2] || m == killer2[ply-2]))
         p->score += KILLERSORTSCORE; 

      p->score += history[side][(p->move & 0x0FFF)] + taxicab[f][D5] - taxicab[t][E4];

      if ( cboard[f] == pawn ) {
        /* Look at pushing Passed pawns first */
        if ( (enemyP & PassedPawnMask[side][t]) == NULLBITBOARD )
           p->score +=50;
      } 
  }
}


void SortRoot (void)
/*****************************************************************************
 *
 *  Sort the moves at the root.  The heuristic is simple.  Try captures/
 *  promotions first.  Other moves are ordered based on their swapoff values.
 *
 *****************************************************************************/
{
   leaf *p;
   int f, t ;
   int side, xside;
   BitBoard enemyP;

   side = board.side;
   xside = 1^side;
   enemyP = board.b[xside][pawn];

   for (p = TreePtr[1]; p < TreePtr[2]; p++)
   {
      f = Value[cboard[FROMSQ(p->move)]];
      if (cboard[TOSQ(p->move)] != 0 || (p->move & PROMOTION))
      {
         t = Value[cboard[TOSQ(p->move)]];
         if (f < t)
            p->score = -1000 + t - f;
         else
            p->score = -1000 + SwapOff (p->move);
      }
      else 
         p->score = -3000 + SwapOff (p->move);

      p->score += taxicab[FROMSQ(p->move)][D5] - taxicab[TOSQ(p->move)][E4];

      if ( f == ValueP ) {
        /* Look at pushing Passed pawns first */
        if ( (enemyP & PassedPawnMask[side][TOSQ(p->move)]) == NULLBITBOARD )
           p->score +=50;
      } 
   }
}


void pick (leaf *head, short ply)
/***************************************************************************
 *
 *  This pick routine searches the movelist and swap the high score entry
 *  with the one currently at the head.
 *
 ***************************************************************************/
{
   int best;
   leaf *p, *pbest, tmp;

   best = head->score;
   pbest = head;
   for (p = head+1; p < TreePtr[ply+1]; p++) 
   {
      if (p->score > best)
      {
         pbest = p;
	 best = p->score;
      }
   }

   /*  Swap pbest with the head  */
   tmp = *head;
   *head = *pbest;
   *pbest = tmp; 
}


int PhasePick (leaf **p1, int ply)
/***************************************************************************
 *
 *  A phase style routine which returns the next move to the search.
 *  Hash move is first returned.  If it doesn't fail high, captures are
 *  generated, sorted and tried.  If no fail high still occur, the rest of
 *  the moves are generated and tried.
 *  The idea behind all this is to save time generating moves which might
 *  not be needed.
 *  CAVEAT: To implement this, the way that genmoves & friends are called
 *  have to be modified.  In particular, TreePtr[ply+1] = TreePtr[ply] must
 *  be set before the calls can be made.
 *  If the board ever gets corrupted during the search, then there is a bug
 *  in IsLegalMove() which has to be fixed.
 *
 ***************************************************************************/
{
   static leaf* p[MAXPLYDEPTH];
   leaf *p2;
   int mv;
   int side;

   side = board.side;
   switch (pickphase[ply])
   {
      case PICKHASH:
         TreePtr[ply+1] = TreePtr[ply];
         pickphase[ply] = PICKGEN1;
         if (Hashmv[ply] && IsLegalMove (Hashmv[ply]))
         {
            TreePtr[ply+1]->move = Hashmv[ply];
            *p1 = TreePtr[ply+1]++;
            return (true);
         }

      case PICKGEN1:
         pickphase[ply] = PICKCAPT;
         p[ply] = TreePtr[ply+1];
         GenCaptures (ply);
         for (p2 = p[ply]; p2 < TreePtr[ply+1]; p2++)
            p2->score = SwapOff(p2->move) * WEIGHT + 
				Value[cboard[TOSQ(p2->move)]];

      case PICKCAPT:
         while (p[ply] < TreePtr[ply+1])
         {
            pick (p[ply], ply);
            if ((p[ply]->move & MOVEMASK) == Hashmv[ply])
            {
	       p[ply]++;
	       continue;
            } 
            *p1 = p[ply]++;
            return (true);
         }

      case PICKKILL1:
         pickphase[ply] = PICKKILL2;
         if (killer1[ply] && killer1[ply] != Hashmv[ply] && 
				IsLegalMove (killer1[ply]))
         {
            TreePtr[ply+1]->move = killer1[ply];
            *p1 = TreePtr[ply+1];
            TreePtr[ply+1]++;
            return (true);
         }
         
      case PICKKILL2:
         pickphase[ply] = PICKGEN2;
         if (killer2[ply] && killer2[ply] != Hashmv[ply] && 
				IsLegalMove (killer2[ply]))
         {
            TreePtr[ply+1]->move = killer2[ply];
            *p1 = TreePtr[ply+1];
            TreePtr[ply+1]++;
            return (true);
         }

      case PICKGEN2:
         pickphase[ply] = PICKREST;
         p[ply] = TreePtr[ply+1];
         GenNonCaptures (ply);
         for (p2 = p[ply]; p2 < TreePtr[ply+1]; p2++)
	 {
            p2->score = history[side][(p2->move & 0x0FFF)] + 
		taxicab[FROMSQ(p2->move)][D5]  - taxicab[TOSQ(p2->move)][E4];
	    if (p2->move & CASTLING)
	       p2->score += CASTLINGSCORE;
         }
	 
      case PICKREST:
         while (p[ply] < TreePtr[ply+1])
         {
            pick (p[ply], ply);
            mv = p[ply]->move & MOVEMASK;
            if (mv == Hashmv[ply] || mv == killer1[ply] || 
		mv == killer2[ply])
	    {
	       p[ply]++;
               continue;
	    }
            *p1 = p[ply]++;
            return (true);
         }
   }
   return (false);
} 


int PhasePick1 (leaf **p1, int ply)
/***************************************************************************
 *
 *  Similar to phase pick, but only used when the King is in check.
 *
 ***************************************************************************/
{
   static leaf* p[MAXPLYDEPTH];

   switch (pickphase[ply])
   {
      case PICKHASH:
         pickphase[ply] = PICKREST;
         p[ply] = TreePtr[ply];

      case PICKREST:
	 while (p[ply] < TreePtr[ply+1])
         {
            pick (p[ply], ply);
            *p1 = p[ply]++;
            return (true);
         }
   }
   return (false);
}
