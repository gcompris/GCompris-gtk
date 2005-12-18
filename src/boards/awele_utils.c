#include <stdlib.h>
#include <string.h>
#include <awele_utils.h>

/**
*  Fonction test si famine
*  Test si le mouvement demandÃ© provoque une \n
*  famine dans le camp opposÃ©. Met a jour la variable string errorMsg\n
*  pour affichage sur le plateau de jeu.
*  @param aw un pointeur sur la structure awalé sur laquelle faire le test
*  @param start un entier donnant la premiere case de l'opposant
*  @param end un entier donnant la derniere case de l'opposant
*  @return TRUE si ce mouvement ne declenche pas une famine, FALSE sinon
*/
short int isOpponentHungry(AWALE * aw, short int start, short int end)
{

    short int i, total;
    extern char errorMsg[30];

    for (total = 0, i = start; i <= end; i++) {
	total += aw->board[i];
    }

    if (total)
	return TRUE;

    strcpy(errorMsg, MSG_FAMINE);
    return FALSE;
}

/**
*  Fonction de test si case non vide
*  Test si la case choisie n'est pas vide
*  @param hole entier désignant la case du plateau choisie
*  @param aw pointeur sur la structure AWALE courante.
*/
short int isValidMove(short int hole, AWALE * aw)
{

    extern char errorMsg[30];
    if (aw->board[hole])
	return TRUE;

    strcpy(errorMsg, MSG_EMPTYHOLE);
    return FALSE;
}


/**
*  Fonction deplacement des graines
*  Cette fonction est appelé a chaque déplacement de graines
*  @param hole la case à partir de laquelle commencer le déplacement
*  @param un pointeur sur la structure AWALE courante, pour laquelle efetuer le déplacement
*  @return TRUE si mouvement valide, le test de famine ne peut etre fait qu'apres deplacement\n
*  FALSE sinon.
*/
short int move(short int hole, AWALE * aw)
{

    AWALE tempAw;
    short int i, j, nbBeans, start, end;


	/**
	*	Sauvegarde de l'awale courant dans un awale temporaire
	*	pour effectuer le mouvement et tester s'il est valide.
	*/
    memcpy(&tempAw, aw, sizeof(AWALE));


    start = (tempAw.player == HUMAN) ? START_COMPUTER : START_HUMAN;
    end = (tempAw.player == HUMAN) ? END_COMPUTER : END_HUMAN;

    nbBeans = tempAw.board[hole];
    tempAw.board[hole] = 0;

    // DÃ©placement des graines
    for (i = nbBeans, j = hole + 1; i > 0; i--, j++) {
	if (j == NBHOLE)
	    j = 0;
	if (j != hole)
	    tempAw.board[j] += 1;
	else
	    i++;
    }

    j--;
    // Comptage des points et mise a zÃ©ro ou il y a capture
    if ((j >= start && j <= end)
	&& (tempAw.board[j] == 2 || tempAw.board[j] == 3)) {
	for (i = j; i >= start; i--) {
	    if (tempAw.board[i] == 2 || tempAw.board[i] == 3) {
		tempAw.CapturedBeans[tempAw.player] += tempAw.board[i];
		tempAw.board[i] = 0;
	    } else
		break;
	}
    }



    if (isOpponentHungry(&tempAw, start, end) != TRUE)
	return FALSE;
    else
	memcpy(aw, &tempAw, sizeof(AWALE));

    if (isEndOfGame(aw) == GAMEOVER)
	return GAMEOVER;

    return TRUE;
}


/**
* Fonction de chgt de joueur
* Cette fonction permet de renvoyer la valeur de l'opposant
* @param player un entier représentant le joueur courant
* @return un entier représentant l'opposant
*/
short int switch_player(short int player)
{
    return (player == HUMAN) ? COMPUTER : HUMAN;
}

/**
* Fonction coup Aléatoire
* Cette fonction permet de générer un coup aléatoire
* @param a pointeur sur la structure AWALE courante
* @return un entier représentant le coup a jouer
*/
short int randplay(AWALE * a)
{
    short int i;

    do {
	i = 6 + rand() % 6;
    } while (a->board[i] == 0 && !testMove(i, a));

    return (i);
}

/**
* Fonction de test si Fin de partie
* Cette fonction est appelée après chaque mvt\
* pour tester si c'est la fin de la partie.
* @param aw pointeur sur la structure AWALE a evaluer
* @return GAMEOVER si c'est la fin de la partie, NOT_GAMEOVER sinon
*/
short int isEndOfGame(AWALE * aw)
{

    short int start, end, i, iCpt, otherPlayer, oppICpt;

    start = (aw->player == HUMAN) ? START_HUMAN : START_COMPUTER;
    end = (aw->player == HUMAN) ? END_HUMAN : END_COMPUTER;
    otherPlayer = (aw->player == HUMAN) ? COMPUTER : HUMAN;

    // Comptage du nbre de graine dans le camp du joueur courant.   
    iCpt = 0;
    for (i = start; i <= end; i++) {
	iCpt += aw->board[i];
    }

    start = (aw->player == HUMAN) ? START_COMPUTER : START_HUMAN;
    end = (aw->player == HUMAN) ? END_COMPUTER : END_HUMAN;

    switch (iCpt) {
    case 0:			//si le joueur courant n'a plus de graine dans son camp
	for (i = start; i <= end; i++)
	    if (aw->board[i] > (end - i))
		// si l'adversaire peut rengrainer : 
		return NOT_GAMEOVER;
	/* Sinon, l'adversaire ne peut pas rengrainer : 
	   comptage des points, mise a zÃ©ro des cases */
	aw->CapturedBeans[otherPlayer] +=
	    NBTOTALBEAN - (aw->CapturedBeans[HUMAN] +
			   aw->CapturedBeans[COMPUTER]);
	for (i = START_HUMAN; i < NBHOLE; i++)
	    aw->board[i] = 0;
	return GAMEOVER;

    case 1:			// S'il ne reste plus q'une graine dans chaque camp alors fin de partie.
	oppICpt = 0;
	for (i = start; i <= end; i++) {
	    oppICpt += aw->board[i];
	}
	if (oppICpt == 1 || oppICpt == 0)
	    return GAMEOVER;
    }

    return NOT_GAMEOVER;
}

/**
* Fonction de test mouvement pour les coups de la machine
* Cette fonction est appelée avt chaque mouvement de la machine pour connaitre\n
* tous les coups possibles.
* @param coup un entier représentant le coup a tester
* @param aw Un pointeur su la structure AWALE courante
* @return TRUE si le mouvement est possible, FALSE sinon
*/
short int testMove(short int coup, AWALE * aw)
{

    AWALE tempAw;
    short int returnMove = FALSE;

    memcpy(&tempAw, aw, sizeof(AWALE));

    if (isValidMove(coup, &tempAw))
	returnMove = move(coup, &tempAw);

    if (returnMove == TRUE || returnMove == GAMEOVER)
	return TRUE;
    else
	return FALSE;

}

/**
* Fonction de creation d'un awale
* Cette fonction reserve en memoire l'espace d'un awale\n
* et renvoi un pointeur sur la zone réservée
* @param void
* @return un pointeur sur la zone nouvellement créee contenant la structure awalé
*/
AWALE *create_awale()
{
    AWALE *a;

    a = (AWALE *) malloc(sizeof(AWALE));
    if (a == NULL) {
	exit(0);
    }


    return a;

}

//ATTENTION : cette fonction n'est plus utilisÃ©e
void awale_equal(AWALE * inp, AWALE * out)
{
    memcpy(out, inp, sizeof(AWALE));
}

/**
* Fonction de création d'un noeud de l'arbre
* Cette fonction est appelée a chaque création d'un noeud de l'arbre\n
* Elle initialise une structure de type TREE, contenant un plateau de jeu,\n
* et les six fils possibles de ce plateau de jeu (representant les coups directement dérivés)
* @param prof entier représentant la profondeur du noeud a crée
* @param player représentant le joueur pour lequel le noeud est crée
* @param aw pointeur sur la structure AWALE mère insérée dans le noeud
* @return un pointeur sur la structure TREE fraichement crée.
*/
TREE *create_tree(short int prof, short int player, AWALE * aw)
{
    TREE *t;
    short int i;

    t = (TREE *) malloc(sizeof(TREE));
    if (t == NULL) {
	exit(0);
    }
    t->aw = create_awale();
    memcpy(t->aw, aw, sizeof(AWALE));
    t->prof = prof;
    t->aw->player = player;
    t->note = t->best = 0;
    for (i = 0; i < HALF_SIZE; i++)
	t->son[i] = NULL;
    return t;
}

/**
* Fonction de destruction d'un noeud de l'arbre
* Cette fonction est appelée pour détruire un noeud de l'arbre, et donc libéré l'espace mémoire
* @param t un pointeur de pointeur de structure TREE
* @return void
*/
void destroy_tree(TREE ** t)
{
    short int i;

    for (i = 0; i < HALF_SIZE; i++) {
	if ((*t)->son[i] != NULL)
	    destroy_tree(&((*t)->son[i]));
    }

    destroy_awale(&((*t)->aw));

    free(*t);
    *t = NULL;
}

/**
*  Fonction de destruction d'un awale.
*  Cette fonction libère l'espace mémoire pris par le pointeur de pointeur de structure AWALE
*  @param a Un pointeur de pointeur de structure AWALE
*  @return void
*/
void destroy_awale(AWALE ** a)
{
    free(*a);

    (*a) = NULL;
}
