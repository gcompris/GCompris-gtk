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

/**
*  Fonction test si famine
*  Test si le mouvement demandee provoque une \n
*  famine dans le camp oppose. Met a jour la variable string errorMsg\n
*  pour affichage sur le plateau de jeu.
*  @param aw un pointeur sur la structure awale sur laquelle faire le test
*  @param start un entier donnant la premiere case de l'opposant
*  @param end un entier donnant la derniere case de l'opposant
*  @return TRUE si ce mouvement ne declenche pas une famine, FALSE sinon
*  player est le dernier à avoir joué. C'est son coté qui peut être vide.
*/
short int isOpponentHungry(short int player, AWALE * aw)
{
    short int i, total, start, end;

    start = (player == HUMAN)? START_HUMAN : START_COMPUTER;
    end   = (player == HUMAN)? END_HUMAN : END_COMPUTER;

    for (total = 0, i = start; i <= end; i++) {
	total += aw->board[i];
    }

    if (!total)
	return TRUE;

    return FALSE;
}

/**
*  Fonction de test si case non vide
*  Test si la case choisie n'est pas vide
*  @param hole entier designant la case du plateau choisie
*  @param aw pointeur sur la structure AWALE courante.
*/
AWALE *moveAwale(short int hole, AWALE * aw)
{
  AWALE *tempAw, *tempAwGs;
  gboolean has_capture = FALSE;

  if (!aw->board[hole]){
    return NULL;
  }

  short int nbBeans, j, last;
  
  tempAw = g_malloc(sizeof(AWALE));
  
  memcpy(tempAw, aw, sizeof(AWALE));

  tempAw->last_play = hole;

  nbBeans = tempAw->board[hole];
  tempAw->board[hole] = 0;
  
  // Déplacement des graines
  for (j = 1, last = (hole+1)%12 ; j <= nbBeans; j++) {
    tempAw->board[last] += 1;
    last = (last + 1) % 12;
    if (last == hole)
      last = (last +1)% 12;
  }

  last = (last +11) %12;

  /* Grand Slam (play and no capture because this let other player hungry */
  tempAwGs = g_malloc(sizeof(AWALE));
  memcpy(tempAwGs, tempAw, sizeof(AWALE));

  // capture
  while ((last >= ((tempAw->player == HUMAN)? 0 : 6))
	  && (last < ((tempAw->player == HUMAN)? 6 : 12))){
    if ((tempAw->board[last] == 2) || (tempAw->board[last] == 3)){
      has_capture = TRUE;
      tempAw->CapturedBeans[switch_player(tempAw->player)] += tempAw->board[last];
      tempAw->board[last] = 0;
      last = (last+11)%12;
      continue;
    }
    break;
  }

  if (isOpponentHungry(tempAw->player, tempAw)){
    if (has_capture){
      /* Grand Slam case */
      //g_warning("Grand Slam: no capture");
      g_free(tempAw);
      return tempAwGs;
    } else{
      /* No capture and  opponent hungry -> forbidden */
      //g_warning("isOpponentHungry %s TRUE",(tempAw->player == HUMAN)? "HUMAN" : "COMPUTER" );
      g_free(tempAw);
      g_free(tempAwGs);
      return NULL;
    }
  }    
  else {
    tempAw->player = switch_player(tempAw->player);
    return tempAw;
  }
}

/**
* Fonction de chgt de joueur
* Cette fonction permet de renvoyer la valeur de l'opposant
* @param player un entier representant le joueur courant
* @return un entier representant l'opposant
*/
short int switch_player(short int player)
{
    return (player == HUMAN) ? COMPUTER : HUMAN;
}

/**
* Fonction coup Aleatoire
* Cette fonction permet de generer un coup aleatoire
* @param a pointeur sur la structure AWALE courante
* @return un entier representant le coup a jouer
*/
short int randplay(AWALE * a)
{
    short int i;
    AWALE *tmp = NULL;

    do {
	i = 6 + g_random_int() % 6;
    } while (a->board[i] == 0 && !(tmp = moveAwale(i, a)));

    g_free(tmp);
    return (i);
}

/* last player is hungry and cannot be served ? */
gboolean diedOfHunger(AWALE *aw)
{
  gint begin = (aw->player == HUMAN) ? 6 : 0;
  gint k;

  if (isOpponentHungry(switch_player(aw->player), aw)){
    for (k=0; k <6; k++){
      if ( aw->board[begin+k] > 6 - k)
	return FALSE;
    }
    g_warning("%s is died of hunger", (aw->player == HUMAN) ? "HUMAN" : "COMPUTER");
    return TRUE;
  }
  else
    return FALSE;
}

