/* GNU Chess 5.0 - quiesce.c - quiescence search code
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

int Quiesce (uint8_t ply, int alpha, int beta)
/**************************************************************************
 *
 *  Our quiescent search.  This quiescent search is able to recognize
 *  mates.
 *
 **************************************************************************/
{
   uint8_t side, xside;
   int best, delta, score, savealpha;
   leaf *p, *pbest;

   if (EvaluateDraw ())
      return (DRAWSCORE);

   side = board.side;
   xside = 1^side;
   InChk[ply] = SqAtakd (board.king[side], xside);
   best = Evaluate (alpha, beta);
   if (best >= beta && !InChk[ply])
      return (best);
   TreePtr[ply+1] = TreePtr[ply];
   if (InChk[ply])
   {
      GenCheckEscapes (ply);
      if (TreePtr[ply] == TreePtr[ply+1])
         return (-MATE+ply-2);
      if (best >= beta)
         return (best);
      SortMoves (ply);
   }
   else
   {
      GenCaptures (ply);
      if (TreePtr[ply] == TreePtr[ply+1])
         return (best);
      SortCaptures (ply); 
   }

   savealpha = alpha;
   pbest = NULL;
   alpha = MAX(best, alpha);
   delta = MAX (alpha - 150 - best, 0);

   for (p = TreePtr[ply]; p < TreePtr[ply+1]; p++)
   {
      pick (p, ply);

      /* We are in check or capture cannot bring score near alpha, give up */
      if (!InChk[ply] && SwapOff (p->move) < delta)
         continue;

      /* If capture cannot bring score near alpha, give up */
      if (p->score == -INFINITY)
	 continue;

      MakeMove (side, &p->move);
      QuiesCnt++;
      if (SqAtakd (board.king[side], xside))
      {
         UnmakeMove (xside, &p->move);
         continue;
      }
      score = -Quiesce (ply+1, -beta, -alpha);
      UnmakeMove (xside, &p->move);
      if (score > best)
      {
         best = score;
	 pbest = p;
         if (best >= beta)
	    goto done;
         alpha = MAX (alpha, best);
      }
   }

done:
   if (flags & USEHASH && pbest != NULL)
      TTPut (side, 0, ply, savealpha, beta, best, pbest->move);

   return (best);
}
