/* gcompris - colors.c
 *
 * Copyright (C) 2002 Pascal Georges
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <ctype.h>
#include <assert.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

GcomprisBoard *gcomprisBoard = NULL;
gboolean board_paused = TRUE;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static int gamewon;

static void		 process_ok(void);
static void		 highlight_selected(int);
static void		 game_won(void);
static void		 repeat(void);

/* ================================================================ */
static GnomeCanvasGroup *boardRootItem = NULL;
static GnomeCanvasItem *highlight_image_item = NULL;

static GnomeCanvasItem *colors_create_item(GnomeCanvasGroup *parent);
static void colors_destroy_all_items(void);
static void colors_next_level(void);
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static int highlight_width, highlight_height;
static GList * listColors = NULL;

#define LAST_COLOR 10
static gchar *colors[LAST_COLOR] = {	"blue","brown",
														"green","grey",
														"orange","purple",
														"red","yellow",
														"black","white"
													};
static int X[] = {75,212,242,368,414,533,578,709};
static int Y[] = {25,170,180,335,337,500};

/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Colors"),
    N_("Click on the right color"),
    "Pascal Georges pascal.georges1@free.fr>",
    NULL,
    NULL,
    NULL,
    NULL,
    start_board,
    pause_board,
    end_board,
    is_our_board,
    NULL,
    process_ok,
    NULL,//set_level,
    NULL,
    repeat
  };

/* =====================================================================
 *
 * =====================================================================*/
BoardPlugin *get_bplugin_info(void) {
  return &menu_bp;
}

/* =====================================================================
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 * =====================================================================*/
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if(gamewon == TRUE && pause == FALSE) /* the game is won */
      game_won();

  board_paused = pause;
}

/* =====================================================================
 *
 * =====================================================================*/
static void start_board (GcomprisBoard *agcomprisBoard) {
	GList * list = NULL;
	int * item;
	int i;

  if(agcomprisBoard!=NULL) {
      gcomprisBoard=agcomprisBoard;
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "colors/colors_bg.jpg");
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=1;
      gcompris_bar_set(GCOMPRIS_BAR_OK|GCOMPRIS_BAR_REPEAT);

      gamewon = FALSE;

			// we generate a list of color indexes in a random order
  		while (g_list_length(listColors) > 0)
    		listColors = g_list_remove(listColors, g_list_nth_data(listColors,0));

			for (i=0; i<LAST_COLOR; i++)
				list = g_list_append(list, GINT_TO_POINTER(i));

			while ((g_list_length(list) > 0)) {
				i = RAND(0,g_list_length(list)-1);
				item = g_list_nth_data(list, i);
				listColors = g_list_append(listColors, item);
				list = g_list_remove(list, item);
			}

			colors_next_level();
      pause_board(FALSE);
    }
}

/* =====================================================================
 *
 * =====================================================================*/
static void end_board () {

  if(gcomprisBoard!=NULL){
      pause_board(TRUE);
      gcompris_score_end();
      colors_destroy_all_items();
		// free list
		while (g_list_length(listColors) > 0)
 			listColors = g_list_remove(listColors, g_list_nth_data(listColors,0));
  }
  gcomprisBoard = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
	if (gcomprisBoard) {
      if(g_strcasecmp(gcomprisBoard->type, "colors")==0) {
	  		/* Set the plugin entry */
	  		gcomprisBoard->plugin=&menu_bp;
	  		return TRUE;
			}
  }
  return FALSE;
}
/* =====================================================================
 * set initial values for the next level
 * =====================================================================*/
static void colors_next_level() {
  colors_destroy_all_items();
  gamewon = FALSE;

  /* Try the next level */
  colors_create_item(gnome_canvas_root(gcomprisBoard->canvas));
	repeat();
}
/* ======================================= */
static void repeat (){
  if(gcomprisBoard!=NULL)
      gcompris_play_ogg(colors[GPOINTER_TO_INT(g_list_nth_data(listColors, 0))], NULL);
}
/* =====================================================================
 * Destroy all the items
 * =====================================================================*/
static void colors_destroy_all_items() {
  if(boardRootItem!=NULL)
      gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static GnomeCanvasItem *colors_create_item(GnomeCanvasGroup *parent) {
  GdkPixbuf *highlight_pixmap = NULL;
  char *str = NULL;
	int i;

	boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, "colors_highlight.png");
  highlight_pixmap = gcompris_load_pixmap(str);

	highlight_image_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", highlight_pixmap,
				      "x", (double) 0,
				      "y", (double) 0,
				      "width", (double) gdk_pixbuf_get_width(highlight_pixmap),
				      "height", (double) gdk_pixbuf_get_height(highlight_pixmap),
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);

	highlight_width = gdk_pixbuf_get_width(highlight_pixmap);
	highlight_height = gdk_pixbuf_get_height(highlight_pixmap);

  g_free(str);
	gnome_canvas_item_hide(highlight_image_item);
	i = RAND(0,LAST_COLOR);

  gdk_pixbuf_unref(highlight_pixmap);
  gtk_signal_connect(GTK_OBJECT(gcomprisBoard->canvas), "event",  (GtkSignalFunc) item_event, NULL);

  return NULL;
}
/* =====================================================================
 *
 * =====================================================================*/
static void game_won() {
  gcomprisBoard->sublevel++;

	listColors = g_list_remove(listColors, g_list_nth_data(listColors,0));

  if( g_list_length(listColors) <= 0 ) { // the current board is finished : bail out
		board_finished(BOARD_FINISHED_TUXLOCO);
		return;
  }

	colors_next_level();
}
/* =====================================================================
 *
 * =====================================================================*/
static void process_ok() {
  gcompris_display_bonus(gamewon, BONUS_SMILEY);
}
/* =====================================================================
 *
 * =====================================================================*/
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
  double x, y;
  int i, j, clicked;
	x = event->button.x;
  y = event->button.y;

	if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
			clicked = -1;
			for (i=0; i<4; i++) {
				for (j=0; j<2; j++) {
					if (x>X[i*2] && x<X[i*2+1] && y>Y[j*2] && y<Y[j*2+1]) {
						clicked = j*4 + i;
					}
				}
			}
			if (x>X[2] && x<X[3] && y>Y[4] && y<Y[5])
				clicked = 8;
			if (x>X[4] && x<X[5] && y>Y[4] && y<Y[5])
				clicked = 9;

			if (clicked >= 0) {
				highlight_selected(clicked);
				gamewon = (clicked == GPOINTER_TO_INT(g_list_nth_data(listColors,0)));
				}
			break;

    default:
      break;
    }
  return FALSE;
}

/* =====================================================================
 *
 * =====================================================================*/
static void highlight_selected(int c) {
	int x, y;

	assert(c>=0 && c<=9);

	if (c<8) {
		x = (X[(c%4)*2] + X[(c%4)*2+1]) /2;
		y = (Y[(int)(c/4)*2] + Y[(int)(c/4)*2+1]) /2;
	} else {
		y = (Y[4]+Y[5]) /2;
		if (c==8)
			x = (X[2] + X[3]) /2;
			else
				x = (X[4] + X[5]) /2;
	}
	x -= highlight_width/2;
	y -= highlight_height/2;
	gnome_canvas_item_show(highlight_image_item);
	item_absolute_move(highlight_image_item, x, y);
}
