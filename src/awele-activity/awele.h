/*
 * gcompris - awele.h
 *
 * Copyright (C) 2005, 2008 Frederic Mazzarol
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */


#define BOUTON "awele/bouton1.png"	//Chemin relatif vers fichiers boutons
#define BOUTON_NOTIFY "awele/bouton1_notify.png"	//Chemin relatif vers fichiers boutons cliqués
#define BOUTON_CLIC "awele/bouton1_clic.png"	//Chemin relatif vers fichiers boutons cliqués
#define BEAN "awele/graine1.png"	//Chemin relatif vers fichiers graines

#define WIDTH 800 	// Largeur Fenetre
#define HEIGHT 600	// Hauteur Fenetre
/**
*	Tableau de graines
*/
typedef struct {
	GooCanvasItem *beanPixbuf;
	char hole;
}BEANHOLE_LINK;


typedef struct {
    GooCanvasItem *msg;	//Item message pour dialogue avec utilisateur
    GooCanvasItem *nbBeansHole[12];	//Tableau d'item affichant le nbre de graine par trou.
    BEANHOLE_LINK *ptBeansHoleLink;	//pointeur sur structures stockant les item graines et la case dans laquelle elles se trouvent.
    GooCanvasItem *button[6];	//Item des boutons (affichés avec pixbufButton)
    GdkPixbuf *pixbufButtonNotify[6];	//pixbux des boutons notifiés
    GdkPixbuf *pixbufButton[6];	//pixbuf des boutons(selection de la case a jouer)
    GdkPixbuf *pixbufButtonClicked[6];	//pixbuf des boutons cliqués
    GooCanvasItem *Captures[2];	//Tableau d'item affichage nbre graine capturées.
    GdkPixbuf *pixbufBeans[4];	//pixbufs des graines
} GRAPHICS_ELT;

typedef struct {
    short int numeroCase;
    GRAPHICS_ELT *graphsElt;
} CALLBACK_ARGS;

/*
*	Fonctions de traitement des events, signaux et rappels
*/
static gboolean buttonClick (GooCanvasItem  *item,
			     GooCanvasItem  *target,
			     GdkEventButton *event,
			     gchar *data);

/**
*	Fonctions Mise a jour de l'affichage
*/
static BEANHOLE_LINK * updateNbBeans (int alpha);
static void updateCapturedBeans (void);
static void initBoardGraphics (GRAPHICS_ELT *graphsElt);
