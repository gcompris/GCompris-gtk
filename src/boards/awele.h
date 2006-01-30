/*
 * gcompris - awele.h Copyright (C) 2005 Frederic Mazzarol This program is
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


#define BOUTON "awele/bouton1.png"	//Chemin relatif vers fichiers boutons
#define BOUTON_NOTIFY "awele/bouton1_notify.png"	//Chemin relatif vers fichiers boutons cliqués
#define BOUTON_CLIC "awele/bouton1_clic.png"	//Chemin relatif vers fichiers boutons cliqués
#define BEAN "awele/graine1.png"	//Chemin relatif vers fichiers graines

#define Y_BOUTONS 412		//Abcisse des boutons

#define WIDTH 800 	// Largeur Fenetre
#define HEIGHT 600	// Hauteur Fenetre
/**
*	Tableau de graines
*/
typedef struct {
	GnomeCanvasItem *beanPixbuf;
	char hole;
}BEANHOLE_LINK;


typedef struct {
    GnomeCanvasItem *msg;	//Item message pour dialogue avec utilisateur   
    GnomeCanvasItem *nbBeansHole[12];	//Tableau d'item affichant le nbre de graine par trou.
    BEANHOLE_LINK *ptBeansHoleLink;	//pointeur sur structures stockant les item graines et la case dans laquelle elles se trouvent.
    GnomeCanvasItem *button[6];	//Item des boutons (affichés avec pixbufButton) 
    GdkPixbuf *pixbufButtonNotify[6];	//pixbux des boutons notifiés
    GdkPixbuf *pixbufButton[6];	//pixbuf des boutons(selection de la case a jouer)
    GdkPixbuf *pixbufButtonClicked[6];	//pixbuf des boutons cliqués
    GnomeCanvasItem *Captures[2];	//Tableau d'item affichage nbre graine capturées.
    GdkPixbuf *pixbufBeans[4];	//pixbufs des graines
} GRAPHICS_ELT;

typedef struct {
    short int numeroCase;
    GRAPHICS_ELT *graphsElt;
} CALLBACK_ARGS;

/* 
*	Fonctions de traitement des events, signaux et rappels
*/
static gint eventDelete (GtkWidget * widget, GdkEvent *event, gpointer data);
static gint eventDestroy (GtkWidget * widget, GdkEvent *event, gpointer data);
static gint buttonClick (GtkWidget *item, GdkEvent *event, gpointer data);
static gint buttonNewGameClick (GRAPHICS_ELT *graphsElt);

/**
*	Fonctions Mise a jour de l'affichage
*/
static BEANHOLE_LINK * updateNbBeans (GnomeCanvasItem *nbBeansHole[NBHOLE], GnomeCanvasGroup *rootGroup, BEANHOLE_LINK *ptLink, int alpha);
static void updateCapturedBeans (GnomeCanvasItem *Captures[2]);
static void initBoardGraphics (GRAPHICS_ELT *graphsElt);
