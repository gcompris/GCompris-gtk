/*
 * gcompris - awele.c Copyright (C) 2005 Frederic Mazzarol This program is
 * free software; you can redistribute it and/or modify it under the terms 
 * of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version.  This program is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.  You should have received a
 * copy of the GNU General Public License along with this program; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, Suite
 * 330, Boston, MA 02111-1307 USA 
 */

#include "awele_utils.h"
#include <string.h>
#include <stdlib.h>
#include "gcompris/gcompris.h"


static gint maxprof;

/**
* Fonction d'evaluation d'un plateau
* La fonction d'evaluation va evaluer la difference du nombre de graines capturees (Facteur preponderant),\n
* la difference de la mobilite des deux joueurs, la difference des cases menacantes,\n
* et la difference du nombre de graine active de chaque joueur.\n
* @param AWALE *aw Pointeur sur la structure AWALE a evaluer
* @return Une note d'evaluation du plateau.
*/
gint eval (GNode *node){
  AWALE *aw = node->data;

  if (aw->CapturedBeans[COMPUTER] > 24)
    return 25;

  if (aw->CapturedBeans[HUMAN] > 24)
    return -25;

  return (aw->CapturedBeans[COMPUTER] - aw->CapturedBeans[HUMAN]);
}

/*
 * Evaluation function for level 1-2
 * this function returns always 0. The play is random, 
 * because tree building is randomised.
 *
 */
gint eval_to_null (GNode *node){
  return 0;
}


gint eval_to_best_capture (GNode *node){
  AWALE *aw = node->data;

  return (aw->CapturedBeans[COMPUTER]);
}

/*
 * firstChild. create all the childs and return first one
 */
GNode *firstChild(GNode *node)
{
  AWALE *aw = node->data;
  AWALE *tmpaw;
  GNode *tmpnode;
  gint eval_node = eval(node);
  gint rand_play;

  /* Case node is winning one */
  if ((eval_node == 25) || (eval_node == -25))
    return NULL;

  gint i;
  rand_play = 1 + random()%6;

  for (i = 0 ; i < 6; i++)
    {
      tmpaw = moveAwale((rand_play + i)%6 + ((aw->player == HUMAN )? 6 : 0), aw);
      if (tmpaw){
	tmpnode = g_node_new(tmpaw);
	g_node_insert (node, -1, tmpnode);
      }
    }
  
  return g_node_first_child(node);
}

/* next sibling */
GNode *nextSibling(GNode *node)
{
  return g_node_next_sibling(node);
}


gboolean free_awale(GNode *node,
		    gpointer data){
  g_free(data);
}


/**
* Fonction de jeu de la machine
* Cette Fonction est appelee pour faire jouer l'ordinateur, \n
* la racine de l'arbre est cree, puis passe en argument a la fonction AlphaBeta\n
* La profondeur augmente au fur et mesure de la partie quand le nombre de graines diminue.\n
* @param aw Un pointeur sur le plateau a partir duquel reflechir
* @return Le meilleur coup calcule par la machine
* le player est celui qui a joué le dernier coup.
*/

short int  think( AWALE *static_awale, short int level){

  AWALE *aw = g_malloc(sizeof(AWALE));
  memcpy (aw, static_awale, sizeof(AWALE));

  GNode *t = g_node_new(aw) ;

  int npris ;
  int best = -1;
  int value = 0;
  EvalFunction use_eval = NULL;

  switch (level) {
  case 1:
    maxprof = 1;
    use_eval = &eval_to_null;
    g_warning("search depth 1, evaluation null");
    break;
  case 2:
    maxprof = 1;
    use_eval = &eval_to_best_capture;
    g_warning("search depth 1, evaluation best capture");
    break;
  case 3:
  case 4:
    maxprof = 2;
    use_eval = &eval;
    g_warning("search depth %d, evaluation best difference", maxprof);
    break;
  case 5:
  case 6:
    maxprof = 4;
    use_eval = &eval;
    g_warning("search depth %d, evaluation best difference", maxprof);
    break;
  case 7:
  case 8:
    maxprof = 6;
    use_eval = &eval;
    g_warning("search depth %d, evaluation best difference", maxprof);
    break;
  case 9:
    maxprof = 8;
    use_eval = &eval;
    g_warning("search depth %d, evaluation best difference", maxprof);
    break;
  default:
    maxprof = 8;
    use_eval = &eval;
    g_warning("search depth %d, evaluation best difference", maxprof);
    break;
  }

  value = gcompris_alphabeta( TRUE, 
			      t, 
			      use_eval, 
			      &best, 
			      (FirstChildGameFunction) firstChild, 
			      (NextSiblingGameFunction) nextSibling,
			      -INFINI , 
			      INFINI,
			      maxprof) ;
  
  if (best < 0){
    g_warning("Leaf node, game is over");
    return -1;
  }
  GNode *tmpNode = g_node_nth_child (t, best);
  
  AWALE *tmpaw = tmpNode->data;
  
  g_warning("THINK best : %d, play: %d", value, tmpaw->last_play);
  
  best = tmpaw->last_play;
  
  /* free awales*/
  g_node_traverse (t,
		   G_IN_ORDER,
		   G_TRAVERSE_ALL,
		   -1,
		   (GNodeTraverseFunc) free_awale,
		   NULL);

  /* free tree */
  g_node_destroy(t);

  return (best);
}

