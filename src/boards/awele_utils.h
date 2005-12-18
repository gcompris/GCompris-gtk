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


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define NBHOLE 12		// Nombre de cases de l'awele
#define NBTOTALBEAN 48		// Nombre de graines totale de l'awale
#define NBPLAYER 2		// Nombre de joueur
#define HUMAN 0			// Numero du joueur Humain (camp Sud)
#define COMPUTER 1		// Numero du joueur virtuel (camp nord)
#define START_HUMAN 0		// Premiere case du camp Sud
#define START_COMPUTER 6	// Premiere Case du camp Nord
#define END_HUMAN 5		// Derniere case du camp Sud
#define END_COMPUTER 11		// Derniere case du camp Nord
#define NBBEANSPERHOLE 4	// Nbre de graine par case en debut de partie
#define HALF_SIZE 6		// nbre de case dans un camp
#define DEF_DEPTH 3		// Profondeur de recherche de l'algo minimax
#define INFINI 50		// Infini pour alphaBeta

/*
*	Code erreur retour des fonctions de test mouvements
*/
#define EMPTY_HOLE 51
#define HUNGRY 52
#define GAMEOVER 53
#define NOT_GAMEOVER 54

#define MSG_FAMINE "declenche une famine"
#define MSG_AREA "clic dans ton camp"
#define MSG_EMPTYHOLE "Cette case est vide, selectionne une autre case"


/**
*	Definition nouveaux types de variables
*/
typedef struct {
    short int board[NBHOLE];
    short int CapturedBeans[NBPLAYER];
    short int player;
} AWALE;

typedef struct tree {
    short int prof;		/* nb de 1/2 coup depuis racine */
    short int note;		/* note calculee */
    short int best;		/* meilleur fils */
    AWALE *aw;			/* awele courant */
    struct tree *son[HALF_SIZE];
} TREE;


/**
*	Fonction de manipulation de l'awale
*/
short int isValidMove(short int hole, AWALE * aw);	// test si mouvement valide
short int move(short int hole, AWALE * aw);
short int think(AWALE * a, short int level);
short int isEndOfGame(AWALE * aw);
short int randplay(AWALE * a);
short int testMove(short int coup, AWALE * aw);

void destroy_tree(TREE ** t);
void destroy_awale(AWALE ** a);

short int switch_player(short int player);
TREE *create_tree(short int prof, short int player, AWALE * aw);
AWALE *create_awale();
void awale_equal(AWALE * inp, AWALE * out);

short int threatenDelta(AWALE * aw);
short int moveDelta(AWALE * aw);
int eval(AWALE * aw);
