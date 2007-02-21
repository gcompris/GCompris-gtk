/* gcompris - gcompris_alphabeta.h
 *
 * Time-stamp: <2006/08/28 21:20:56 bruno>
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

#ifndef _GOMPRIS_ALPHABETA_H_
#define _GOMPRIS_ALPHABETA_H_

#include "gcompris.h"

/* gc_alphabeta returns the best value of evaluation functions */
/* set the bestChild to the index of child with this value */
/* maximize : TRUE if depth is maximize one, neither FALSE. */
/* treeGame : pointer on game to pass to others functions, */
/* bestChild : pointer on GList wich will contains list of indexes fo best plays. */
/* isLeaf : TRUE if game correspond to a Leaf of Tree. */
/* firstChild : return pointer on first  child of a node. */
/* nextSibling : return pointer on next sibling of a node. */
/* heuristic : evaluation function of game. */
/* depth is max depth of recursion */

typedef gint         (*EvalFunction)              (gpointer treeGame);
typedef gboolean     (*LeafFunction)              (gpointer treeGame);
typedef gpointer     (*FirstChildGameFunction)    (gpointer treeGame);
typedef gpointer     (*NextSiblingGameFunction)     (gpointer treeGame);

gint gc_alphabeta (gboolean maximize,
		   gpointer treeGame,
		   EvalFunction heuristic,
		   gint *bestChild,
		   FirstChildGameFunction firstChild,
		   NextSiblingGameFunction nextSibling,
		   gint alpha,
		   gint beta,
		   gint depth
		   );

#endif
