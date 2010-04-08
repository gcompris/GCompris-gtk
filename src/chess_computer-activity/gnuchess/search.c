/* GNU Chess 5.0 - search.c - tree-search code
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
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define TIMECHECK 	1023
#define HISTSCORE(d)	((d)*(d))

#define FUTSCORE        (MATERIAL+fdel)
#define GETNEXTMOVE  (InChk[ply] ? PhasePick1 (&p, ply) : PhasePick (&p, ply))

static inline void ShowThinking (leaf *p, uint8_t ply)
{
   if (flags & XBOARD)
      return;
   if (!(flags & POST))
      return;
   if (NodeCnt < 500000 && (flags & SOLVE)) {
      return;
   }
   SANMove (p->move, ply);
   printf ("\r%2d.         %2d/%2d%10s    ", Idepth, 
      (int) (p-TreePtr[ply]+1), (int) (TreePtr[ply+1]-TreePtr[ply]), SANmv);
   fflush (stdout);
}

static int ply1score;

int SearchRoot (short depth, int alpha, int beta)
/**************************************************************************
 *
 *  This perform searches at ply=1.  For ply>1, it calls the more generic
 *  search() routine.  The rationale for splitting these is because at
 *  ply==1, things are done slightly differently than from the other plies,
 *  e.g. print PVs, not testing null move etc.
 *
 **************************************************************************/
{
   int best, score, savealpha;
   uint8_t side, xside, ply;
   short nodetype;
   leaf *p, *pbest;

   ply = 1; 
   side = board.side;
   xside = 1^side;
   ChkCnt[2] = ChkCnt[1];
   ThrtCnt[2] = ThrtCnt[1];
   KingThrt[white][ply] = MateScan (white);
   KingThrt[black][ply] = MateScan (black);
   InChk[ply] = SqAtakd (board.king[side], xside);
   if (InChk[ply] && ChkCnt[ply] < 3*Idepth)
   {
      ChkExtCnt++;
      ChkCnt[ply+1]++;
      depth += 1;
   }
   best = -INFINITY;
   savealpha = alpha;
   nodetype = PV;
   pbest = NULL;

   for (p = TreePtr[1]; p < TreePtr[2]; p++)
   {
      pick (p, 1); 
      ShowThinking (p, ply);
      MakeMove (side, &p->move);
      NodeCnt++;
      
      /*  If first move, search against full alpha-beta window  */
      if (p == TreePtr[1])
      {
         score = -Search (2, depth-1, -beta, -alpha, nodetype);
         /*
	    The following occurs when we are re-searching a fail high move
            and now it has fail low.  This can be disastrous, so immediately
	    adjust alpha and research.
          */
	 if (beta == INFINITY && score <= alpha)
	 {
	    alpha = -INFINITY;
            score = -Search (2, depth-1, -beta, -alpha, nodetype);
         }
      }

      /*  Else search against zero window  */
      else
      {
	 nodetype = CUT;
         alpha = MAX (best, alpha);            
         score = -Search (2, depth-1, -alpha-1, -alpha, nodetype);
         if (score > best)
         {
            if (alpha < score && score < beta)
	    {
	       nodetype = PV;
               score = -Search (2, depth-1, -beta, -score, nodetype);
	    }
         }
      }
      UnmakeMove (xside, &p->move);

      ply1score = p->score = score;
      if (score > best)
      {
         best = score;
	 pbest = p;
         if (best > alpha)
         {
            rootscore = best;
            RootPV = p->move;
	    if (best >= beta)
	       goto done;
            ShowLine (RootPV, best, '&');
         }
      }

      if (flags & TIMEOUT)
      {
	/* ply == 1 always at this point, but code
	 * copied from Search
	 */
         best = (ply & 1 ? rootscore : -rootscore );
	 return (best);
      }

      if (((flags & PONDER) || SearchDepth == 0) && (NodeCnt & TIMECHECK) == 0)
      {
	 if (flags & PONDER) {
	    if (input_status != INPUT_NONE)
	       SET(flags, TIMEOUT);
	 } else {
	    ElapsedTime = GetElapsed (StartTime);
	    if ((ElapsedTime >= SearchTime && (
		    rootscore == -INFINITY-1 
		    || ply1score > lastrootscore - 25 
		    || flags & SOLVE))
		|| ElapsedTime >= maxtime)
	       SET (flags, TIMEOUT);
	 }
      }

      if (MATE+1 == best+1)
         return (best);
   }

/*  If none of the move is good, we still want to try the same first move */
   if (best <= savealpha)
      TreePtr[1]->score = savealpha;

/*****************************************************************************
 *
 *  Out of main search loop.
 *
 *****************************************************************************/
done:

   /*  Update history  */
   if (best > savealpha)
      history[side][pbest->move & 0x0FFF] += HISTSCORE(depth);

   rootscore = best;
   return (best);
}


int Search (uint8_t ply, short depth, int alpha, int beta, short nodetype)
/**************************************************************************
 *
 *  The basic algorithm for this search routine came from Anthony 
 *  Marsland.  It is a PVS (Principal Variation Search) algorithm.
 *  The fail-soft alpha-beta technique is also used for improved
 *  pruning.
 *
 **************************************************************************/
{
   int best, score, nullscore, savealpha;
   int side, xside;
   int rc, t0, t1, firstmove;
   int fcut, fdel, donull, savenode, extend;
   leaf *p, *pbest;
   int g0, g1;
   int upperbound;

   /* Check if this position is a known draw */
   if (EvaluateDraw ())
      return (DRAWSCORE);
   if (GameCnt >= Game50+3 && Repeat())
   {
      RepeatCnt++;
      return (DRAWSCORE); 
   }

   side = board.side;
   xside = 1^side;
   donull = true;

/*************************************************************************
 *
 *  Perform some basic search extensions.
 *  1.  One reply extensions.  
 *  2.  If in check, extend (maximum of Idepth-1).
 *  3.  If there is a threat to the King, extend (not beyond 2*Idepth)
 *  4.  If recapture to same square and not beyond Idepth+2
 *  5.  If pawn move to 7th rank at the leaf node, extend.
 *
 *************************************************************************/
   extend = false;
   InChk[ply] = SqAtakd (board.king[side], xside);
   if (InChk[ply])
   {
      TreePtr[ply+1] = TreePtr[ply];
      GenCheckEscapes (ply);
      if (TreePtr[ply] == TreePtr[ply+1])
         return (-MATE+ply-2);
      if (TreePtr[ply]+1 == TreePtr[ply+1])
      {
         depth += 1;
	 extend = true;
         OneRepCnt++;
      }
   }

/*
   We've already found a mate at the next ply.  If we aren't being mated by 
   a shorter line, so just return the current material value.
*/
   if (rootscore + ply >= MATE)
      return (MATERIAL);

   g0 = Game[GameCnt].move;
   g1 = GameCnt > 0 ? Game[GameCnt-1].move : 0;
   t0 = TOSQ(g0); 
   t1 = TOSQ(g1);
   ChkCnt[ply+1] = ChkCnt[ply];
   ThrtCnt[ply+1] = ThrtCnt[ply];
   KingThrt[white][ply] = MateScan (white);
   KingThrt[black][ply] = MateScan (black);
   if (InChk[ply]  && /* ChkCnt[ply] < Idepth-1*/ ply <= 2*Idepth)
   {
      ChkExtCnt++;
      ChkCnt[ply+1]++;
      depth += 1;
      extend = true;
   }
   else if (!KingThrt[side][ply-1] && KingThrt[side][ply] && ply <= 2*Idepth)
   {
      KingExtCnt++;
      extend = true;
      depth += 1;
      extend = true;
      donull = false;
   }
   /* Promotion extension */
   else if (g0 & PROMOTION)
   {
      PawnExtCnt++;
      depth += 1; /* Not reached, but why?! */
      extend = true;
   }
   /* Recapture extension */
   else if ((g0 & CAPTURE) && (board.material[computer] - 
	board.material[1^computer] == RootMaterial))
   {
      RcpExtCnt++;
      depth += 1;
      extend = true;
   }
   /* 6th or 7th rank extension */
   else if (depth <= 1 && cboard[t0] == pawn && (RANK(t0) == rank7[xside] || RANK(t0) == rank6[xside]))
   {
      PawnExtCnt++;
      depth += 1;
      extend = true;
   }

/**************************************************************************** 
 *
 *  The following extension is to handle cases when the opposing side is 
 *  delaying the mate by useless interposing moves. 
 *
 ****************************************************************************/
   if (ply > 2 && InChk[ply-1] && cboard[t0] != king && t0 != t1 && 
	 !SqAtakd (t0, xside))
   {
      HorzExtCnt++;
      depth += 1;
      extend = true;
   }

/***************************************************************************
 *
 *  This is a new code to perform search reductiion.  We introduce some
 *  form of selectivity here.
 *
 **************************************************************************/

   if (depth <= 0)
      return (Quiesce (ply, alpha, beta)); 

/**************************************************************************** 
 *
 *  Probe the transposition table for a score and a move.
 *  If the score is an upperbound, then we can use it to improve the value
 *  of beta.  If a lowerbound, we improve alpha.  If it is an exact score,
 *  if we now get a cut-off due to the new alpha/beta, return the score.
 *
 ***************************************************************************/
   Hashmv[ply] = 0;
   upperbound = INFINITY;
   if (flags & USEHASH)
   {
      rc = TTGet (side, depth, ply, &score, &g1);
      if (rc)
      {
         Hashmv[ply] = g1 & MOVEMASK;
         switch (rc)
         {
	 case POORDRAFT  :  /* Not reached */ break;
	 case EXACTSCORE :  /* Not reached */ return (score);
            case UPPERBOUND :  beta = MIN (beta, score);
			       upperbound = score;
			       donull = false;
	                       break;
            case LOWERBOUND :  /*alpha = MAX (alpha, score);*/
			       alpha = score;
	                       break;
	    case QUIESCENT  :  Hashmv[ply] = 0;
	                       break;
	    default : break;
         }
	 if (alpha >= beta)
	    return (score);
      }
   }

/*****************************************************************************
 *
 *  Perform the null move here.  There are certain cases when null move
 *  is not done.  
 *  1.  When the previous move is a null move.
 *  2.  At the frontier (depth == 1)
 *  3.  At a PV node.
 *  4.  If side to move is in check.
 *  5.  If the material score + pawn value is still below beta.
 *  6.  If we are being mated at next ply.
 *  7.  If hash table indicate the real score is below beta (UPPERBOUND).
 *  8.  If side to move has less than or equal to a bishop in value.
 *  9.  If Idepth <= 3.  This allows us to find mate-in 2 problems quickly.
 *  10. We are looking for a null threat.
 *
 *****************************************************************************/
   if (ply > 4 && InChk[ply-2] && InChk[ply-4])
      donull = false;
   if (flags & USENULL && g0 != NULLMOVE && depth > 1 && nodetype != PV &&
       !InChk[ply] && MATERIAL+ValueP > beta && beta > -MATE+ply && donull &&
	board.pmaterial[side] > ValueB )
   {
      TreePtr[ply+1] = TreePtr[ply];
      MakeNullMove (side);
      nullscore = -Search (ply+1, depth-3, -beta, -beta+1, nodetype);
      UnmakeNullMove (xside); 
      if (nullscore >= beta)
      {
         NullCutCnt++;
         return (nullscore);
      }
      if ( depth-3 >= 1 && MATERIAL > beta && nullscore <= -MATE+256)
      {
         depth += 1;
	 extend = true;
      }
   }

   if (InChk[ply] && TreePtr[ply]+1 < TreePtr[ply+1])
      SortMoves (ply);

   pickphase[ply] = PICKHASH;
   GETNEXTMOVE;

/*************************************************************************
 *
 *  Razoring + Futility.
 *  At depth 3, if there is no extensions and we are really bad, decrease
 *  the search depth by 1.
 *  At depth 2, if there is no extensions and we are quite bad, then we
 *  prune all non checking moves and capturing moves that don't bring us up
 *  back to alpha.
 *  Caveat: Skip all this if we are in the ending.
 *
 *************************************************************************/
   fcut = false;
   fdel = MAX (ValueQ, maxposnscore[side]);
   if (!extend && nodetype != PV && depth == 3 && FUTSCORE <= alpha)
   {
      depth = 2;
      RazrCutCnt++;
   }
   fdel = MAX (ValueR, maxposnscore[side]);
   fcut = (!extend && nodetype != PV && depth == 2 && FUTSCORE <= alpha);
   if (!fcut)
   {
      fdel = MAX (3*ValueP, maxposnscore[side]);
      fcut = (nodetype != PV && depth == 1 && FUTSCORE <= alpha);
   }

   MakeMove (side, &p->move);
   NodeCnt++;
   g0 = g1 = 0;
   while ((g0 = SqAtakd (board.king[side], xside)) > 0 ||
      	 (fcut && FUTSCORE < alpha && !SqAtakd (board.king[xside], side) &&
	  !MateScan (xside)))
   {
      if (g0 == 0) g1++;
      UnmakeMove (xside, &p->move);
      if (GETNEXTMOVE == false)
         return (g1 ? Evaluate(alpha,beta) : DRAWSCORE);
      MakeMove (side, &p->move);
      NodeCnt++;
   }
   firstmove = true;
   pbest = p;
   best = -INFINITY;
   savealpha = alpha;
   nullscore = INFINITY;
   savenode = nodetype;
   if (nodetype != PV)
      nodetype = (nodetype == CUT) ? ALL : CUT;

   while (1)
   {
      /* We have already made the move before the loop. */
      if (firstmove)
      {
         firstmove = false;
         score = -Search (ply+1, depth-1, -beta, -alpha, nodetype);
      }

      /* Zero window search for rest of moves */
      else
      {
	 if (GETNEXTMOVE == false)
	    break;

         MakeMove (side, &p->move);
         NodeCnt++;
         if (SqAtakd (board.king[side], xside)) 
         {
            UnmakeMove (xside, &p->move);
            continue;
         }

/*****************************************************************************
 *
 *  Futility pruning.  The idea is that at the frontier node (depth == 1),
 *  if the side on the move is materially bad, then if the move doesn't win
 *  back material or the move isn't a check or doesn't threatened the king, 
 *  then there is no point in searching this move.  So skip it.  
 *  Caveat:  However if the node is a PV, we skip this test.
 *
 *****************************************************************************/
      	 if (fcut && FUTSCORE <= alpha && !SqAtakd (board.king[xside], side) &&
		!MateScan (xside))
		
         {
            UnmakeMove (xside, &p->move);
	    FutlCutCnt++;
	    NodeCnt--;
            continue;
         }
         NodeCnt++;

         if (nodetype == PV)
            nodetype = CUT;
         alpha = MAX (best, alpha);                /* fail-soft condition */
         score = -Search (ply+1, depth-1, -alpha-1, -alpha, nodetype);
         if (score > best)
         {
	    if (savenode == PV)
	       nodetype = PV;
            if (alpha < score && score < beta)
	    {
               score = -Search (ply+1, depth-1, -beta, -score, nodetype);
	    } 
	    if (nodetype == PV && score <= alpha &&
		Game[GameCnt+1].move == NULLMOVE)
	    {
               score = -Search (ply+1, depth-1, -alpha, INFINITY, nodetype);
	    }
         }
      }

      UnmakeMove (xside, &p->move);

      if (score > best)
      {
         best = score;
         pbest = p;
	 if (best >= beta)
	    goto done;
      }

      if (flags & TIMEOUT)
      {
         best = (ply & 1 ? rootscore : -rootscore);
	 return (best);
      }

      if (((flags & PONDER) || SearchDepth == 0) && (NodeCnt & TIMECHECK) == 0)
      {	
	 if (flags & PONDER) {
	    if (input_status != INPUT_NONE)
	       SET(flags, TIMEOUT);
	 } else {
	    ElapsedTime = GetElapsed (StartTime);
	    if ((ElapsedTime >= SearchTime && 
		 (rootscore == -INFINITY-1 || 
		  ply1score > lastrootscore - 25 || flags & SOLVE)) ||
		ElapsedTime >= maxtime)
	       SET (flags, TIMEOUT);        
	 }
      }

/*  The following line should be explained as I occasionally forget too :) */
/*  This code means that if at this ply, a mating move has been found,     */
/*  then we can skip the rest of the moves!  				   */
      if (MATE+1 == best+ply)
         goto done;
   } 

/*****************************************************************************
 *
 *  Out of main search loop.
 *
 *****************************************************************************/
done:

/*
   if (upperbound < best)
      printf ("Inconsistencies %d %d\n", upperbound, best);
*/

   /*  Save the best move inside the transposition table  */
   if (flags & USEHASH){
/*
 * Nasty temporary hack to try and work around timeout problem
 * If we are pondering and timeout don't save incomplete answers
 * Must look at failure of TIMEOUT condition more carefully!
 */
	if ( !(flags & TIMEOUT))
          TTPut (side, depth, ply, savealpha, beta, best, pbest->move); 
      }

   /*  Update history  */
   if (best > savealpha)
      history[side][pbest->move & 0x0FFF] += HISTSCORE(depth);

   /*  Don't store captures as killers as they are tried before killers */
   if (!(pbest->move & (CAPTURE | PROMOTION)) && best > savealpha)
   {
      if (killer1[ply] == 0)
         killer1[ply] = pbest->move & MOVEMASK;
      else if ((pbest->move & MOVEMASK) != killer1[ply])
         killer2[ply] = pbest->move & MOVEMASK;
   }

   return (best);
}


void ShowLine (int move __attribute__ ((unused)), int score, char c)
/*****************************************************************************
 *
 *  Print out the latest PV found during the search.
 *  The only move we know is the root move.  The rest of the PV is taken
 *  from the hash table.  This strategy avoids all the headaches associated
 *  with returning the PV up from the leaf to the root.
 *
 *****************************************************************************/
{
   int i, len;
   int pvar[MAXPLYDEPTH];

   /* SMC */
   if (!(flags & POST))
     return;
   if (NodeCnt < 500000 && (flags & SOLVE)) {
      /* printf("NodeCnt = %d\n",NodeCnt); getchar(); */
      return;
   }
   if (Idepth == 1 && c == '&')
      return;
   if ((flags & XBOARD) && c == '&')
      return;
   if (rootscore == -INFINITY-1)
      return;
   ElapsedTime = GetElapsed (StartTime);

   /*
    * The different output formats for Xboard and GNU Chess are documented
    * in the engine protocol guide.
    *
    * In particular if the character after ply is not a space, Xboard
    * assume it is talking to a GNU Chess compatible engine and
    * uses time in seconds, not centiseconds.
    *
    * This code should be simplified!
    *
    */
   if (flags & XBOARD) {
     if (score > MATE-255) {
       printf ("%d%c Mat%d %d %ld\t", Idepth, c,
                (int)(MATE+2-abs(score))/2, (int)(ElapsedTime), NodeCnt+QuiesCnt);
       if (ofp != stdout)
	 fprintf (ofp,"%2d%c%7.2f  Mat%02d%10ld\t", Idepth, c, ElapsedTime,
                (MATE+2-abs(score))/2, NodeCnt+QuiesCnt);
     } else if (score < -MATE+255) {
       printf ("%d%c -Mat%2d %d %ld\t", Idepth, c,
                (int)(MATE+2-abs(score))/2, (int)(ElapsedTime), NodeCnt+QuiesCnt);
       if (ofp != stdout)
       fprintf (ofp,"%2d%c%7.2f -Mat%02d%10ld\t", Idepth, c, ElapsedTime,
		 (MATE+2-abs(score))/2, NodeCnt+QuiesCnt);
     } else {
	 printf ("%d%c %d %d %ld\t", Idepth, c, (int)score, 
		 (int)(ElapsedTime), NodeCnt+QuiesCnt);
	 if (ofp != stdout) 
	   fprintf (ofp,"%2d%c%7.2f%7d%10ld\t", Idepth, c, 
		    ElapsedTime, score, NodeCnt+QuiesCnt);	 
       }
   }
   else {              /* Not XBOARD */
      if (score > MATE-255) {
	 printf ("\r%2d%c%7.2f  Mat%02d%10ld\t", Idepth, c, ElapsedTime,
		 (MATE+2-abs(score))/2, NodeCnt+QuiesCnt);
	 if (ofp != stdout)
	    fprintf (ofp,"\r%2d%c%7.2f  Mat%02d%10ld\t", Idepth, c, ElapsedTime,
		     (MATE+2-abs(score))/2, NodeCnt+QuiesCnt);
      } else if (score < -MATE+255) {
	 printf ("\r%2d%c%7.2f -Mat%02d%10ld\t", Idepth, c, ElapsedTime,
		 (MATE+2-abs(score))/2, NodeCnt+QuiesCnt);
	 if (ofp != stdout)
	    fprintf (ofp,"\r%2d%c%7.2f -Mat%02d%10ld\t", Idepth, c, ElapsedTime,
		     (MATE+2-abs(score))/2, NodeCnt+QuiesCnt);
      } else {
	 printf ("\r%2d%c%7.2f%7d%10ld\t", Idepth, c, ElapsedTime,
		 score, NodeCnt+QuiesCnt);
	 if (ofp != stdout) 
	    fprintf (ofp,"\r%2d%c%7.2f%7d%10ld\t", Idepth, c, ElapsedTime,
		    score, NodeCnt+QuiesCnt);	 
      }
   }

   if (c == '-')
   {
      printf ("\n");
      if (ofp != stdout) fprintf(ofp, "\n");
      return;
   }
   else if (c == '+')
   {
      SANMove (RootPV, 1);
      printf (" %s\n", SANmv);
      if (ofp != stdout) fprintf (ofp," %s\n", SANmv);
      return;
   }

   SANMove (RootPV, 1);
   printf (" %s", SANmv);
   if (ofp != stdout) fprintf (ofp," %s", SANmv);
   MakeMove (board.side, &RootPV);
   TreePtr[3] = TreePtr[2];
   GenMoves (2);
   len = strlen (SANmv);
   i = 2;
   pvar[1] = RootPV;

   /*  We fill the rest of the PV with moves from the hash table */
   if ((flags & USEHASH))
   {
      while (TTGetPV (board.side, i, rootscore, &pvar[i]))
      {
         if ((MATESCORE(score) && abs(score) == MATE+2-i) || Repeat ())
            break;
  
         if (len >= 32)
         {
            printf ("\n\t\t\t\t");
	    if (ofp != stdout) fprintf (ofp,"\n\t\t\t\t");
            len = 0;
         }
         SANMove (pvar[i], i);
         printf (" %s", SANmv);
	 if (ofp != stdout) fprintf (ofp," %s", SANmv);
         MakeMove (board.side, &pvar[i]);
         TreePtr[i+2] = TreePtr[i+1];
         GenMoves (++i);
         len += strlen (SANmv);
      }
   }

   printf ("\n");
   if (ofp != stdout) fprintf(ofp,"\n");
   for (--i; i; i--)
      UnmakeMove (board.side, &pvar[i]);
   fflush (stdout);
   if (ofp != stdout) fflush (ofp);
}
