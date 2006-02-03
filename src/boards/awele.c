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

#include "gcompris/gcompris.h"
#include <awele_utils.h>
#include <awele.h>
#include <string.h>
#include <unistd.h>


static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

char errorMsg[30];
AWALE *staticAwale;
int caseCoord[12] =
	{ 102, 206, 309, 413, 522, 628, 626, 520, 411, 307, 201, 100 };
GRAPHICS_ELT *graphsElt = NULL;
CALLBACK_ARGS *buttonClickArgs[6];

static void start_board (GcomprisBoard * agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static void set_level (guint level);
static int gamewon;
static void game_won (void);
static void repeat(void);

static GnomeCanvasGroup *boardRootItem = NULL;

static gboolean is_our_board (GcomprisBoard * gcomprisBoard);
static GnomeCanvasItem *awele_create_item (GnomeCanvasGroup * parent);
static void awele_destroy_all_items (void);
static void awele_next_level (void);


/*
 * Description of this plugin 
 */
static BoardPlugin menu_bp = {
	NULL,
	NULL,
	"The awele game",	/* The name that describe this board */
	"African strategy board game",	/* The description that describes
					 * this board */
	"Frederic Mazzarol <fmazzarol@gmail.com>",	/* The author of this
							 * board */
	NULL,			/* Called when the plugin is loaded */
	NULL,			/* Called when gcompris exit */
	NULL,			/* Show the about box */
	NULL,			/* Show the configuration dialog */
	start_board,		/* Callback to start_board implementation */
	pause_board,
	end_board,
	is_our_board,		/* Return 1 if the plugin can handle the board file */
	NULL,
	NULL,
	set_level,
	NULL,
	repeat,
	NULL,
	NULL
};

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO (awele)
/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{
	if (gcomprisBoard == NULL)
		return;

	if (gamewon == TRUE && pause == FALSE)
	{			/* the game is won */
		game_won ();
	}

	board_paused = pause;
}

/*
 */
static void
start_board (GcomprisBoard * agcomprisBoard)
{

	if (agcomprisBoard != NULL)
	{
		gchar *str;
		GdkPixbuf *pixmap = NULL;

		gcomprisBoard = agcomprisBoard;
		gcomprisBoard->level = 1;
		gcomprisBoard->maxlevel = 4;
		gcomprisBoard->sublevel = 1;
		gcomprisBoard->number_of_sublevel = 1;	/* Go to next level after
							 * this number of 'play' */

		str = gcompris_image_to_skin("button_reload.png");
		pixmap = gcompris_load_pixmap(str);
		g_free(str);
		if(pixmap) {
		  gcompris_bar_set_repeat_icon(pixmap);
		  gdk_pixbuf_unref(pixmap);
		  gcompris_bar_set(GCOMPRIS_BAR_REPEAT_ICON);
		} else {
		  gcompris_bar_set(GCOMPRIS_BAR_REPEAT);
		}
		
		awele_next_level ();

		gamewon = FALSE;
		pause_board (FALSE);
	}
}

/*
 * ======================================= 
 */
static void
end_board ()
{
	if (gcomprisBoard != NULL)
	{
		pause_board (TRUE);
		awele_destroy_all_items ();
	}
	gcomprisBoard = NULL;
}

static gboolean
is_our_board (GcomprisBoard * gcomprisBoard)
{
	if (gcomprisBoard)
	{
		if (g_strcasecmp (gcomprisBoard->type, "awele") == 0)
		{
			/*
			 * Set the plugin entry 
			 */
			gcomprisBoard->plugin = &menu_bp;

			return TRUE;
		}
	}
	return FALSE;
}

/*
 * Repeat let the user restart the current level
 *
 */
static void repeat (){

  buttonNewGameClick(graphsElt);
}


static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel = 1;
      awele_next_level();
    }
}


/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*
 * set initial values for the next level 
 */
static void
awele_next_level ()
{

	gcompris_set_background (gnome_canvas_root (gcomprisBoard->canvas),
				 gcompris_image_to_skin ("gcompris-bg.jpg"));

	gcompris_bar_set_level (gcomprisBoard);

	awele_destroy_all_items ();
	gamewon = FALSE;

	/*
	 * Create the level 
	 */
	awele_create_item (gnome_canvas_root (gcomprisBoard->canvas));


}

/*
 * ==================================== 
 */
/*
 * Destroy all the items 
 */
static void
awele_destroy_all_items ()
{
	if (boardRootItem != NULL)
		gtk_object_destroy (GTK_OBJECT (boardRootItem));

	boardRootItem = NULL;
}

/*
 * ==================================== 
 */
static GnomeCanvasItem *
awele_create_item (GnomeCanvasGroup * parent)
{

	GdkPixbuf *pixmap = NULL;
	gint i = 0, x1 = 0;
	gchar buffer[2];
	gchar xpmFile[35] = BOUTON;
	gchar xpmFileNotify[35] = BOUTON_NOTIFY;
	gchar xpmFileClic[35] = BOUTON_CLIC;

	boardRootItem =
		GNOME_CANVAS_GROUP (gnome_canvas_item_new
				    (gnome_canvas_root
				     (gcomprisBoard->canvas),
				     gnome_canvas_group_get_type (), "x",
				     (double) 0, "y", (double) 0, NULL));

	/*
	 * Load the cute frame 
	 */
	pixmap = gcompris_load_pixmap ("awele/awele_frame.png");

	gnome_canvas_item_new (boardRootItem,
			       gnome_canvas_pixbuf_get_type (),
			       "pixbuf", pixmap,
			       "x", (double) 0,
			       "y", (double) 0,
			       "width",
			       (double) gdk_pixbuf_get_width (pixmap),
			       "height",
			       (double) gdk_pixbuf_get_height (pixmap),
			       "width_set", TRUE, "height_set", TRUE, NULL);

	/*
	 * Display text
	 */
	{
	  int x, y;

	  x = 35;
	  y = 190;
	  gnome_canvas_item_new (boardRootItem,
				 gnome_canvas_text_get_type (),
				 "text", _("NORTH"),
				 "font", gcompris_skin_font_board_medium,
				 "x", (double) x + 1,
				 "y", (double) y + 1,
				 "anchor", GTK_ANCHOR_CENTER,
				 "fill_color_rgba", gcompris_skin_color_shadow,
				 NULL);

	  gnome_canvas_item_new (boardRootItem,
				 gnome_canvas_text_get_type (),
				 "text", _("NORTH"),
				 "font", gcompris_skin_font_board_medium,
				 "x", (double) x,
				 "y", (double) y,
				 "anchor", GTK_ANCHOR_CENTER,
				 "fill_color_rgba", gcompris_skin_color_text_button,
				 NULL);

	  x = 765;
	  y = 295;
	  gnome_canvas_item_new (boardRootItem,
				 gnome_canvas_text_get_type (),
				 "text", _("SOUTH"),
				 "font", gcompris_skin_font_board_medium,
				 "x", (double) x + 1,
				 "y", (double) y + 1,
				 "anchor", GTK_ANCHOR_CENTER,
				 "fill_color_rgba", gcompris_skin_color_shadow,
				 NULL);

	  gnome_canvas_item_new (boardRootItem,
				 gnome_canvas_text_get_type (),
				 "text", _("SOUTH"),
				 "font", gcompris_skin_font_board_medium,
				 "x", (double) x,
				 "y", (double) y,
				 "anchor", GTK_ANCHOR_CENTER,
				 "fill_color_rgba", gcompris_skin_color_text_button,
				 NULL);

	}

	staticAwale = (AWALE *) malloc (sizeof (AWALE));

	if (!staticAwale)
		exit (1);

	for (i = 0; i < NBHOLE; i++)
	{
		staticAwale->board[i] = NBBEANSPERHOLE;
	}

	staticAwale->player = HUMAN;

	for (i = 0; i < NBPLAYER; i++)
	{
		staticAwale->CapturedBeans[i] = 0;
	}

	graphsElt = (GRAPHICS_ELT *) malloc (sizeof (GRAPHICS_ELT));

	for (i = 0; i < 6; i++)
	{
		buttonClickArgs[i] =
			(CALLBACK_ARGS *) malloc (sizeof (CALLBACK_ARGS));
		buttonClickArgs[i]->graphsElt = graphsElt;
		buttonClickArgs[i]->numeroCase = i;
	}

	/*
	 * Boucle pour creer et positionner les boutons qui serviront 
	 * a selectionner la case a jouer
	 */
	for (i = 0; i < NBHOLE / 2; i++)
	{
		sprintf (buffer, "%d", i + 1);
		xpmFile[12] = buffer[0];
		graphsElt->pixbufButton[i] = gcompris_load_pixmap (xpmFile);
		xpmFileNotify[12] = buffer[0];
		graphsElt->pixbufButtonNotify[i] =
			gcompris_load_pixmap (xpmFileNotify);
		xpmFileClic[12] = buffer[0];
		graphsElt->pixbufButtonClicked[i] =
			gcompris_load_pixmap (xpmFileClic);

		/*
		 * Ajustement de l'ordonnee x, pour positionner le bouton sur la barre de boutons.
		 */
		switch (i)
		{
		case 0:
			x1 = 120;
			break;
		case 1:
			x1 = 220;
			break;
		case 2:
			x1 = 325;
			break;
		case 3:
			x1 = 432;
			break;
		case 4:
			x1 = 539;
			break;
		case 5:
			x1 = 643;
			break;
		}

		/*
		 * Ajout des boutons comme items sur le rootGroup du canevas.
		 * et sauvegarde dans tableau button de type Gnome Canvas Item
		 * pour attacher les pointeurs de la fonction de rappel buttonClick
		 * qui servira a detecter quel est l'evenement souris, et en fonction
		 * declencher la procedure associee. Passage en argument a cette fonction
		 * du numero de case selectionne par tableau chaine
		 */
		graphsElt->button[i] = gnome_canvas_item_new (boardRootItem,
							      gnome_canvas_pixbuf_get_type
							      (), "x",
							      (double) x1,
							      "y",
							      (double)
							      Y_BOUTONS,
							      "pixbuf",
							      graphsElt->
							      pixbufButton[i],
							      NULL);

		gtk_signal_connect (GTK_OBJECT (graphsElt->button[i]),
				    "event", GTK_SIGNAL_FUNC (buttonClick),
				    (gpointer) buttonClickArgs[i]);


	}

	/**
	*	Affichage initial du nombre de graine courant dans le trou i.
	*	Sauvegarde des items d'affichage dans nbBeansHole, pour mise a jour
	*	pdt la partie.
	*/
	for (i = 11; i >= 0; i--)
	{

		sprintf (buffer, "%d", staticAwale->board[i]);

		graphsElt->nbBeansHole[i] =
			gnome_canvas_item_new (boardRootItem,
					       gnome_canvas_text_get_type (),
					       "text", buffer, "font",
					       "12x24", "size", 14000, "x",
					       (double) (caseCoord[i] + 45),
					       "y",
					       (double) ((i < 6) ? 378 : 94),
					       "fill_color", "black", NULL);
	}

	/**
	*	Affichage initial du nombre de graine capturees par chaque joueur.
	*	Sauvegarde des items d'affichage dans Captures[i], pour mise a jour
	*	pdt la partie.
	*/
	for (i = 0; i < 2; i++)
	{

		x1 = (i == 1) ? 32 : 762;

		sprintf (buffer, "%d", staticAwale->CapturedBeans[i]);

		graphsElt->Captures[i] = gnome_canvas_item_new (boardRootItem,
								gnome_canvas_text_get_type
								(), "text",
								buffer,
								"font",
								"12x24",
								"size", 20000,
								"x",
								(double) x1,
								"y",
								(double) 246,
								"fill_color",
								"black",
								NULL);
	}

	/**
	*	Initialisation du buffer xpmFile avec le chemin relatif des fichiers graine
	*	Creation des pixbuf et sauvegarde dans variable pixbufBeans
	*/
	strcpy (xpmFile, BEAN);
	for (i = 0; i < 4; i++)
	{
		sprintf (buffer, "%d", i + 1);
		xpmFile[12] = buffer[0];
		graphsElt->pixbufBeans[i] = gcompris_load_pixmap (xpmFile);
	}

	/**
	*	Reservation d'un espace memoire egal a NBTOTALBEAN x taille struct BEANHOLE_LINK
	*	pour y stocker chaque item de graine ainsi que la case dans laquelle se trouve la graine.
	*	Puis creation de toutes les graines et affichage sur le plateau.
	*/
	initBoardGraphics (graphsElt);

	graphsElt->msg = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_text_get_type (),
						"text", "Selectionne une case a jouer",
						"font", "12x24",
						"size", 20000,
						"x", (double) 400,
						"y", (double) 500,
						"fill_color", "red",
						"anchor", GTK_ANCHOR_CENTER,
						NULL);

	return NULL;
}


/*
 * ==================================== 
 */
static void
game_won ()
{
	gcomprisBoard->sublevel++;

	if (gcomprisBoard->sublevel > gcomprisBoard->number_of_sublevel)
	{
		/*
		 * Try the next level 
		 */
		gcomprisBoard->sublevel = 1;
		gcomprisBoard->level++;
		if (gcomprisBoard->level > gcomprisBoard->maxlevel)
		{		// the
			// current 
			// board
			// is
			// finished 
			// : bail
			// out
			board_finished (BOARD_FINISHED_RANDOM);
			return;
		}

	}
	awele_next_level ();
}

/**
*  Fonction effectuant l'initialisation des graines sur le plateau
*  Cette fonction est appelee a chaque debut de partie
*  @param data un pointeur de type void, pour passer en argument a la fonction\n
*  les elements graphiques a modifier.
*  @return void
*/
static void
initBoardGraphics (GRAPHICS_ELT * graphsElt)
{

	int i, j, k, idxTabBeans = 0;


	//if (graphsElt->ptBeansHoleLink != NULL)
	//      free(graphsElt->ptBeansHoleLink);       

	graphsElt->ptBeansHoleLink =
		(BEANHOLE_LINK *) malloc (NBTOTALBEAN *
					  sizeof (BEANHOLE_LINK));

	for (i = NBHOLE - 1; i >= 0; i--)
	{
		for (j = 0;
		     j < staticAwale->board[i] && idxTabBeans < NBTOTALBEAN;
		     j++, idxTabBeans++)
		{
			k = 0 + rand () % 4;
			graphsElt->ptBeansHoleLink[idxTabBeans].beanPixbuf =
				gnome_canvas_item_new (boardRootItem,
						       gnome_canvas_pixbuf_get_type
						       (), "x",
						       (double) caseCoord[i] +
						       rand () % 50, "y",
						       (double) (((i <
								   6) ? 260 :
								  130) +
								 rand () %
								 60),
						       "pixbuf",
						       graphsElt->
						       pixbufBeans[k], NULL);

			graphsElt->ptBeansHoleLink[idxTabBeans].hole = i;
		}
	}
}

/**
*  Fonction effectuant la procedure associe a un clic sur pixmap
*  Cette fonction est appelee quand un clic sur un bouton est effectue.\n
*  Selon l'event->Type declenchement de procedure differentes, modification de l'aspect des boutons\n
*  et declenchement d'un mouvement choisi par le joueur, puis lancement du coup de la machine.
*  @param widget pointeur sur le widget ayant declenche l'evenement eventDelete
*  @param event pointeur sur le type d'evenement
*  @param data un pointeur de type void, pour passer en argument a la fonction\n
*  les elements graphiques a modifier.
*  @return un entier
*/
static gint
buttonClick (GtkWidget * item, GdkEvent * event, gpointer data)
{

	CALLBACK_ARGS *args;
	short int retourMove;
	short int returnMove;
	short int coup;

	args = (CALLBACK_ARGS *) data;

	switch (event->type)
	{
	case GDK_ENTER_NOTIFY:
		g_object_set (GTK_OBJECT
			      (args->graphsElt->button[args->numeroCase]),
			      "pixbuf",
			      args->graphsElt->pixbufButtonNotify[args->
								  numeroCase],
			      "y", (double) Y_BOUTONS, NULL);
		break;
	case GDK_LEAVE_NOTIFY:
		g_object_set (GTK_OBJECT
			      (args->graphsElt->button[args->numeroCase]),
			      "pixbuf",
			      args->graphsElt->pixbufButton[args->numeroCase],
			      "y", (double) Y_BOUTONS, NULL);
		break;
	case GDK_BUTTON_PRESS:

		g_object_set (GTK_OBJECT
			      (args->graphsElt->button[args->numeroCase]),
			      "pixbuf",
			      args->graphsElt->pixbufButtonClicked[args->
								   numeroCase],
			      "y", (double) Y_BOUTONS + 3, NULL);

		g_object_set (args->graphsElt->msg, "text", "", NULL);

		if (isValidMove (args->numeroCase, staticAwale) != TRUE)
		{
			g_object_set (args->graphsElt->msg, "text", errorMsg,
				      NULL);
		}
		else
		{
			retourMove = move (args->numeroCase, staticAwale);
			if (retourMove == TRUE || retourMove == GAMEOVER)
			{
				updateNbBeans (args->graphsElt->nbBeansHole,
					       boardRootItem,
					       args->graphsElt->
					       ptBeansHoleLink, 0);
				updateCapturedBeans (args->graphsElt->
						     Captures);
				staticAwale->player = COMPUTER;
			}
			if (retourMove == GAMEOVER)
				g_object_set (args->graphsElt->msg, "text",
					      "FIN DE LA PARTIE !!!", NULL);
		}

		break;
	case GDK_BUTTON_RELEASE:
		g_object_set (GTK_OBJECT
			      (args->graphsElt->button[args->numeroCase]),
			      "pixbuf",
			      args->graphsElt->pixbufButtonNotify[args->
								  numeroCase],
			      "y", (double) Y_BOUTONS, NULL);


		if (staticAwale->player == COMPUTER)
		{
			sleep (1);
			coup = think (staticAwale, gcomprisBoard->level);
			returnMove = move (coup, staticAwale);
			updateNbBeans (args->graphsElt->nbBeansHole,
				       boardRootItem,
				       args->graphsElt->ptBeansHoleLink, 0);
			updateCapturedBeans (args->graphsElt->Captures);
			staticAwale->player = HUMAN;
			if (returnMove == GAMEOVER)
				g_object_set (args->graphsElt->msg, "text",
					      "FIN DE LA PARTIE !!!", NULL);
			else g_object_set (args->graphsElt->msg, "text",
					      "A toi de jouer ...", NULL);

		}
		break;
	default:
		break;
	}

	return FALSE;
}

/**
*  Fonction de gestion des graines dessinees sur le plateau
*  Cette fonction est appelee apres chaque mouvement, \n
*  pour remettre a jour le nombre de graines dessinees sur le plateau, \n
*  et diminuer la zone d'allocation ou les pixmap des graines sont stockees.
*  @param nbBeansHole[NBHOLE] Tableau de pointeur sur les gnomeCanvasItem\n
*  affichant le nombre de graine par case
*  @param rootGroup Pointeur sur le groupe contenant tous les items inseres dans le canevas
*  @param ptLink pointeur sur zone memoire ou sont stockees toutes les images des graines du plateau.
*  @param alpha entier pour differencier une mise a jour du plateau ou le lancement d'une nouvelle partie.
*  @return Renvoi du pointeur sur la zone memoire apres redimension (n'a probablement pas changÃ© d'adresse).
*/
static BEANHOLE_LINK *
updateNbBeans (GnomeCanvasItem * nbBeansHole[NBHOLE],
	       GnomeCanvasGroup * rootGroup,
	       BEANHOLE_LINK * ptLink, int alpha)
{

	char buffer[3];		//Manipulation chaines de caracteres            
	int i, j, k, idxTabBeans = 0;	//Compteur Boucle Manipulation Elements graphiques
	static short int nbActiveBean = NBTOTALBEAN;	//nbre graine restant sur plateau
	static short int nbOldActiveBean;	//nbre graine restant sur plateau au tour precedent
	BEANHOLE_LINK *ptBeansHoleLink = NULL;	//pointeur sur structures stockant les item graines et la case dans laquelle elles se trouvent.

	/**
	*	Sauvegarde du nombre de graines restantes sur le plateau de jeu 
	*	pour le prochain appel a la fonction. 
	*	Mise a jour de nbActiveBean avec nouvelle configuration du plateau de jeu.
	*/
	if (alpha)
	{
		nbOldActiveBean = 48;
	}
	else
	{
		nbOldActiveBean = nbActiveBean;
	}

	nbActiveBean =
		NBTOTALBEAN - (staticAwale->CapturedBeans[HUMAN] +
			       staticAwale->CapturedBeans[COMPUTER]);

	/**
	*	Destruction d'autant d'elements graphiques graines
	*	qu'il y a eu de captures pdt ce tour de jeu
	*/
	for (ptBeansHoleLink = &ptLink[nbActiveBean], i = 0;
	     i < nbOldActiveBean - nbActiveBean; i++, ptBeansHoleLink++)
	{
		gtk_object_destroy (GTK_OBJECT (ptBeansHoleLink->beanPixbuf));
	}


	/**
	*	Allocation d'un nouvel espace memoire stockant les item graines 
	*	et la case dans laquelle elles se trouvent. Puis liberation de la fin de 
	*	l'ancien espace memoire.
	*/

	ptBeansHoleLink =
		(BEANHOLE_LINK *) realloc (ptLink,
					   nbActiveBean *
					   sizeof (BEANHOLE_LINK));

	/**
	*	Pour chaque case du plateau, mise a jour du nbre de graines qu'elle contient. 
	*	Et pour chaque graine de cette case, deplacement d'un element graphique type graine
	*	dans cette case. Et mise a jour de l'information hole dans la structure BEANHOLE_LINK.
	*/
	for (i = NBHOLE - 1; i >= 0; i--)
	{
		sprintf (buffer, "%d", staticAwale->board[i]);
		gnome_canvas_item_set (nbBeansHole[i], "text", buffer, NULL);

		for (j = 0;
		     j < staticAwale->board[i] && idxTabBeans < nbActiveBean;
		     j++, idxTabBeans++)
		{

			k = 0 + rand () % 4;

			gnome_canvas_item_set (ptBeansHoleLink[idxTabBeans].
					       beanPixbuf, "x",
					       (double) caseCoord[i] +
					       rand () % 50, "y",
					       (double) (((i <
							   6) ? 260 : 130) +
							 rand () % 60), NULL);

			ptBeansHoleLink[idxTabBeans].hole = i;
		}

	}

	/**
	*	Renvoi du pointeur sur la zone memoire retaillee (n'a probablement pas change d'adresse).
	*/
	return ptBeansHoleLink;
}


/**
*  Fonction de gestion de l'affichage des scores
*  Cette fonction est appelee apres chaque mouvement, \n
*  pour remettre a jour le score des joueurs
*  @param Captures[2] pointeur sur les gnomeCanvasItem d'affichage des scores
*/
static void
updateCapturedBeans (GnomeCanvasItem * Captures[2])
{

	short int i;
	char buffer[3];

	for (i = 0; i < 2; i++)
	{
		sprintf (buffer, "%d", staticAwale->CapturedBeans[i]);
		g_object_set (Captures[i], "text", buffer, NULL);
		if (staticAwale->CapturedBeans[i] > 24)
		  {
		    gamewon = TRUE;
		    gcompris_display_bonus(i==0, BONUS_FLOWER);
		  }
	}
}


/**
*  Fonction effectuant la procedure associe a un clic sur le bouton nouvelle Partie
*  Cette fonction est appelee quand un clic sur le bouton nouvelle Partie est effectue.\n
*  Selon l'event->Type declenchement de procedure differentes, modification de l'aspect des boutons\n
*  et declenchement de la reinitialisation du plateau de jeu.
*  @param widget pointeur sur le widget ayant declenche l'evenement eventDelete
*  @param event pointeur sur le type d'evenement
*  @param data un pointeur de type void, pour passer en argument a la fonction\n
*  les elements graphiques a modifier.
*  @return un entier
*/
static gint
buttonNewGameClick (GRAPHICS_ELT *graphsElt)
{
	short int nbCapturedBean = 0;
	BEANHOLE_LINK *ptLink;
	int i;

	nbCapturedBean =
	  staticAwale->CapturedBeans[HUMAN] +
	  staticAwale->CapturedBeans[COMPUTER];

	/**
	 *	Destruction de toute les graines et remise a zero compteur de capture 
	 *	et indicateur nbre de graine par case
	*/
	for (ptLink = graphsElt->ptBeansHoleLink, i = 0;
	     i < NBTOTALBEAN - nbCapturedBean; i++, ptLink++)
	  {
	    gtk_object_destroy (GTK_OBJECT (ptLink->beanPixbuf));
	  }
	
	for (i = 0; i < NBHOLE; i++)
	  {
	    staticAwale->board[i] = NBBEANSPERHOLE;
	    g_object_set (GTK_OBJECT
			  (graphsElt->nbBeansHole[i]),
			  "text", "4", NULL);
	  }
	
	staticAwale->player = HUMAN;

	for (i = 0; i < NBPLAYER; i++)
	  {
	    staticAwale->CapturedBeans[i] = 0;
	  }
	
	updateCapturedBeans (graphsElt->Captures);
	initBoardGraphics (graphsElt);
}
