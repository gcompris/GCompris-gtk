/* gcompris - gcompris_alphabeta.c
 *
 * Time-stamp: <2006/08/21 23:28:14 bruno>
 *
 * Copyright (C) 2000 Bruno Coudoin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "gcompris.h"

/* gcompris_alphabeta returns the best value of evaluation functions */
/* set the bestChild to the index of child with this value */
/* maximize : TRUE if depth is maximize one, neither FALSE. */
/* treeGame : pointer on game to pass to others functions, */
/* bestChild : pointer on GList wich will contains list of indexes fo best plays. */
/* isLeaf : TRUE if game correspond to a Leaf of Tree. */
/* firstChild : return pointer on first  child of a node. */
/* nextSibling : return pointer on next sibling of a node. */
/* heuristic : evaluation function of game. */
/* depth is max depth of recursion */
gint gcompris_alphabeta (gboolean maximize,
			 gpointer treeGame,
			 EvalFunction heuristic,
			 gint *bestChild,
			 FirstChildGameFunction firstChild,
			 NextSiblingGameFunction nextSibling,
			 gint alpha,
			 gint beta,
			 gint depth
			 )
{
  gpointer child;
  gint m, t, nextBest, index;

  g_assert (depth >= 0);
  
  child = firstChild(treeGame);

  *bestChild = -1;

/*   g_warning("gcompris_alphabeta %d %d %d", depth, alpha, beta); */
  
  /* directly return value for leaf node*/
  if ((!child) || (depth == 0)){
/*     g_warning("gcompris_alphabeta %d returns %d bestChild %d", depth, heuristic(treeGame), *bestChild); */
    return heuristic(treeGame);
  }

  index = 0;
      
  if (maximize) {
    m = alpha;
    while (child){
      t = gcompris_alphabeta (!maximize,
			      child,
			      heuristic,
			      &nextBest,
			      firstChild,
			      nextSibling,
			      m,
			      beta,
			      depth - 1
			      );
      if (t > m){
	m = t;
	*bestChild = index;
      }
      if ( m >= beta){
/* 	g_warning("gcompris_alphabeta %d returns %d bestChild %d", depth, heuristic(treeGame), *bestChild); */
	return m;
      }
      index++;
      child = nextSibling(child);
    }
/*     g_warning("gcompris_alphabeta %d returns %d bestChild %d", depth, heuristic(treeGame), *bestChild); */
    return m;
  }
  else {
    /* minimize */
    m = beta;
    while (child){
      t = gcompris_alphabeta (!maximize,
			      child,
			      heuristic,
			      &nextBest,
			      firstChild,
			      nextSibling,
			      alpha,
			      m,
			      depth - 1
			      );
      if (t < m){
	m = t;
	*bestChild = index;
      }
      if ( m <= alpha){
/* 	g_warning("gcompris_alphabeta %d returns %d bestChild %d", depth, heuristic(treeGame), *bestChild); */
	return m;
      }
      index++;
      child = nextSibling(child);
    }
/* 	g_warning("gcompris_alphabeta %d returns %d bestChild %d", depth, heuristic(treeGame), *bestChild); */
    return m;

  }

}
