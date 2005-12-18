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



int maxprof;
static void alphabeta( TREE * t , short int alpha , short int beta );

/**
* Fonction d'evaluation d'un plateau
* La fonction d'evaluation va evaluer la difference du nombre de graines capturees (Facteur preponderant),\n
* la difference de la mobilite des deux joueurs, la difference des cases menacantes,\n
* et la difference du nombre de graine active de chaque joueur.\n
* @param AWALE *aw Pointeur sur la structure AWALE a evaluer
* @return Une note d'evaluation du plateau.
*/
int eval (AWALE *aw){
	
	int score;//, attacDelta, mobilityDelta; 	

 	score = aw->CapturedBeans[COMPUTER] - aw->CapturedBeans[HUMAN];
 	//attacDelta = threatenDelta(aw)*33;
	
	/*if ((aw->CapturedBeans[COMPUTER] + aw->CapturedBeans[HUMAN]) > 30)
		mobilityDelta = 0;
	else
		mobilityDelta	= moveDelta(aw)*22;*/
	
	if (aw->player == HUMAN){
		score = -score;
		//attacDelta = -attacDelta;
		//mobilityDelta = -mobilityDelta;
	}
	
	//score = score*100 +  mobilityDelta + attacDelta;

	return score;
}


/**
* Fonction de jeu de la machine
* Cette Fonction est appelee pour faire jouer l'ordinateur, \n
* la racine de l'arbre est cree, puis passe en argument a la fonction AlphaBeta\n
* La profondeur augmente au fur et mesure de la partie quand le nombre de graines diminue.\n
* @param aw Un pointeur sur le plateau a partir duquel reflechir
* @return Le meilleur coup calcule par la machine
*/
short int  think( AWALE *a, short int level){
    TREE * t ;
    int npris ;
	short int best;
    
	maxprof = level ;

     /*augmente la profondeur quand le nombre de pieces diminue */
   	npris = a->CapturedBeans[HUMAN] + a->CapturedBeans[COMPUTER] ;
    if ( npris > 20 ) maxprof ++ ;
    if ( npris > 25 ) maxprof ++ ;
    if ( npris > 30 ) maxprof ++ ;
    if ( npris > 35 ) maxprof ++ ;
    if ( npris > 40 ) maxprof ++ ;

    /* initialisation de l'arbre */
    t = create_tree( 0 , a->player , a ) ;

    /* recherche meilleur coup */
    alphabeta( t , -INFINI , INFINI ) ;
	
    best = t->best;
	
    destroy_tree( &t ) ;
	
	return (best);
}

/**
* Algorithme MiniMax (amelioration AlphaBeta)
* Cette fonction va etudier les meilleurs coups possibles a jouer  \n
* de facon recursive pour tous les coups possibles\n
* @param t La racine de l'arbre
* @param alpha optimisation alphaBeta coupure de type Max
* @param beta  optimisation alphaBeta coupure de type Min
*/
static void alphabeta( TREE * t , short int alpha , short int beta ){
    
	short int i,j, n,is,ie,isOtherPlayer,ieOtherPlayer;
	short int m,note;
	char prune=FALSE ;
	char FinPartie = TRUE;
	
	is = (t->aw->player == HUMAN)?START_HUMAN:START_COMPUTER;
	ie = (t->aw->player == HUMAN)?END_HUMAN:END_COMPUTER;
	isOtherPlayer = (t->aw->player == HUMAN)?START_COMPUTER:START_HUMAN;
	ieOtherPlayer = (t->aw->player == HUMAN)?END_COMPUTER:END_HUMAN;
	
	/**
	*	Test si fin de partie par famine
	*/
	for (n =0, i=is; i<=ie; i++)
		n += t->aw->board[i];
			
	if (!n)
		for (j=isOtherPlayer; j<=ieOtherPlayer; j++){
			if (t->aw->board[j] <= ieOtherPlayer -j)
				FinPartie = FinPartie & TRUE;
			else {
				FinPartie = FinPartie & FALSE; 
				break;
			}
		}
		
    /* si noeud terminal calcul note */
    if ( t->prof == DEF_DEPTH || t->aw->CapturedBeans[HUMAN]+t->aw->CapturedBeans[COMPUTER] >= NBTOTALBEAN -2 || FinPartie != TRUE) {
		t->note = eval (t->aw);
		return ;	
    } 
	
	
	/* remontee note des fils */
    m = alpha ;
	
    for ( n=0,i=is ; i<=ie && !prune ; n++,i++ )
		
		if (testMove(i, t->aw)) {
			
			if (t->prof >0)
				t->son[n] = create_tree( t->prof+1, switch_player(t->aw->player), t->aw  ) ; 
			else 
				t->son[n] = create_tree( t->prof+1, t->aw->player, t->aw  ) ;
			
			if (testMove(i, t->son[n]->aw)){
				move ( i , t->son[n]->aw);
				t->son[n]->aw->player = switch_player (t->son[n]->aw->player);
				alphabeta( t->son[n] , -beta , -m ) ;
				note = -t->son[n]->note ;
				if ( note > m ) {
					m = note ;
					t->best = i ;
				}
			
				if ( m >= beta ) prune=TRUE ;
			}
					
			destroy_tree( &(t->son[n]) ) ;
    }
   
	t->note = m ;
}
/**
* Fonction de calcul de la difference du nombre de cases menacantes
* Cette fonction va calculer le nombre de cases menacantes pour les deux joueurs\n
* puis faire la diffrence des deux.
* @param aw Un pointeur sur le plateau a evaluer
* @return un entier egal a la difference des cases menacantes
*/
short int threatenDelta (AWALE *aw){
	
	short int i, tempo;
	short int threatenHuman = 0, threatenComputer = 0;
	AWALE tmpAw[13];
	
	
	memcpy(&tmpAw[12], aw, sizeof(AWALE));
	
	for (i=START_HUMAN; i<=END_COMPUTER; i++){
		if (i<=END_HUMAN){
			if (testMove (i, &tmpAw[12])){
				memcpy(&tmpAw[i], aw, sizeof(AWALE));
				tempo = tmpAw[i].CapturedBeans[HUMAN];
				move(i, tmpAw);
				if (tmpAw[i].CapturedBeans[HUMAN] > tempo)
					threatenHuman++;
			}
		}
		else {
			tmpAw[i].player = switch_player(aw->player);
			tmpAw[12].player = COMPUTER;
			if (testMove (i, &tmpAw[12])){
				memcpy(&tmpAw[i], aw, sizeof(AWALE));
				tempo = tmpAw[i].CapturedBeans[COMPUTER];
				move(i, tmpAw);
				if (tmpAw[i].CapturedBeans[COMPUTER] > tempo)
					threatenComputer++;
			}
		}
	
	
	}

	return (threatenComputer - threatenHuman);
}


/**
* Fonction de calcul de la difference de la mobilite des deux joueurs
* Cette fonction va calculer le nombre de cases non vides pour les deux joueurs\n
* puis faire la diffrence des deux.
* @param aw Un pointeur sur le plateau a evaluer
* @return un entier egal a la difference des cases non vides des deux joueurs
*/
short int moveDelta (AWALE *aw){
	
	short int i;
	short int moveHuman = 0, moveComputer = 0;
	AWALE tmpAw;	
	
	memcpy (&tmpAw, aw, sizeof (AWALE));
	
	for (i=START_HUMAN; i<=END_COMPUTER; i++)
		
		if (i == START_COMPUTER)
			tmpAw.player = COMPUTER;
		
		if (testMove (i, &tmpAw)){
			if (i<=END_HUMAN)
				moveHuman++;
			else
				moveComputer++;
			
		}

	return (moveComputer - moveHuman);
}
