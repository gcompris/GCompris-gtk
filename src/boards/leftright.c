/* gcompris - leftright.c
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

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
static int gamewon;
static int last_hand = -1;

static void process_ok(void);
static void highlight_selected(int);
static void game_won();

#define LEFT 0
#define RIGHT 1

// the values are taken from the backgound image colors-bg.jpg
#define BUTTON_AREA_X1 83
#define BUTTON_AREA_X2 487
#define BUTTON_AREA_Y1 380

#define CENTER_LEFT_X 185
#define CENTER_LEFT_Y 430
#define CENTER_RIGHT_X 600
#define CENTER_RIGHT_Y CENTER_LEFT_Y

#define HAND_X 400
#define HAND_Y 200

// Defines the clickable areas
#define CLICKABLE_X1 90
#define CLICKABLE_X2 300
#define CLICKABLE_X3 490
#define CLICKABLE_X4 700
#define CLICKABLE_Y1 390
#define CLICKABLE_Y2 480

#define NUMBER_OF_SUBLEVELS 6
#define NUMBER_OF_LEVELS 4

#define TEXT_COLOR "yellow"

/* ================================================================ */
static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem *hand_image_item = NULL, *left_text_item = NULL, *right_text_item = NULL;
static GnomeCanvasItem *left_highlight_image_item = NULL, *right_highlight_image_item = NULL;

static GnomeCanvasItem *leftright_create_item(GnomeCanvasGroup *parent);
static void leftright_destroy_all_items(void);
static void leftright_next_level(void);
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static int answer;

static char *hands[32] = {"main_droite_dessus_0.png","main_droite_paume_0.png",
													"main_gauche_dessus_0.png","main_gauche_paume_0.png",
													"main_droite_dessus_90.png","main_droite_paume_90.png",
													"main_gauche_dessus_90.png","main_gauche_paume_90.png",
													"main_droite_dessus_180.png","main_droite_paume_180.png",
													"main_gauche_dessus_180.png","main_gauche_paume_180.png",
													"main_droite_dessus_270.png","main_droite_paume_270.png",
													"main_gauche_dessus_270.png","main_gauche_paume_270.png",
													"poing_droit_dessus_0.png", "poing_droit_paume_0.png",
													"poing_gauche_dessus_0.png", "poing_gauche_paume_0.png",
													"poing_droit_dessus_90.png", "poing_droit_paume_90.png",
													"poing_gauche_dessus_90.png", "poing_gauche_paume_90.png",
													"poing_droit_dessus_180.png", "poing_droit_paume_180.png",
													"poing_gauche_dessus_180.png", "poing_gauche_paume_180.png",
													"poing_droit_dessus_270.png", "poing_droit_paume_270.png",
													"poing_gauche_dessus_270.png", "poing_gauche_paume_270.png"
													};

/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Find your left and right hands"),
    N_("Given hands pictures, find if it's a right or left one"),
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
    set_level,
    NULL,
    NULL
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
  if(agcomprisBoard!=NULL) {
      gcomprisBoard=agcomprisBoard;
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "leftright/leftright-bg.jpg");
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=NUMBER_OF_LEVELS;
      gcomprisBoard->sublevel=1;

      gcomprisBoard->number_of_sublevel = NUMBER_OF_SUBLEVELS;
      gcompris_score_start(SCORESTYLE_NOTE, 10, 50, gcomprisBoard->number_of_sublevel);
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

      leftright_next_level();

      gamewon = FALSE;
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
      leftright_destroy_all_items();
    }
  gcomprisBoard = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static void set_level (guint level) {
  if(gcomprisBoard!=NULL) {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      leftright_next_level();
    }
}

/* =====================================================================
 *
 * =====================================================================*/
gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
	if (gcomprisBoard) {
      if(g_strcasecmp(gcomprisBoard->type, "leftright")==0) {
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
static void leftright_next_level() {
  gcompris_bar_set_level(gcomprisBoard);

  leftright_destroy_all_items();
  gamewon = FALSE;

  gcompris_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  leftright_create_item(gnome_canvas_root(gcomprisBoard->canvas));

}
/* =====================================================================
 * Destroy all the items
 * =====================================================================*/
static void leftright_destroy_all_items() {
  if(boardRootItem!=NULL)
      gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static GnomeCanvasItem *leftright_create_item(GnomeCanvasGroup *parent) {
  GdkPixbuf *highlight_pixmap = NULL;
  GdkPixbuf *hand_pixmap = NULL;
  char *str = NULL;
	int i;

	boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  left_text_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", _("left"),
				      "font", gcompris_skin_font_board_big,
				      "x", (double) CENTER_LEFT_X,
				      "y", (double) CENTER_LEFT_Y,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color", TEXT_COLOR,
				      NULL);

  right_text_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", _("right"),
				      "font", gcompris_skin_font_board_big,
				      "x", (double) CENTER_RIGHT_X,
				      "y", (double) CENTER_RIGHT_Y,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color", TEXT_COLOR,
				      NULL);

  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, "highlight_torus.png");
  highlight_pixmap = gcompris_load_pixmap(str);

	left_highlight_image_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", highlight_pixmap,
				      "x", (double) BUTTON_AREA_X1,
				      "y", (double) BUTTON_AREA_Y1,
				      "width", (double) gdk_pixbuf_get_width(highlight_pixmap),
				      "height", (double) gdk_pixbuf_get_height(highlight_pixmap),
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);

	right_highlight_image_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", highlight_pixmap,
				      "x", (double) BUTTON_AREA_X2,
				      "y", (double) BUTTON_AREA_Y1,
				      "width", (double) gdk_pixbuf_get_width(highlight_pixmap),
				      "height", (double) gdk_pixbuf_get_height(highlight_pixmap),
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);

  g_free(str);
	gnome_canvas_item_hide(right_highlight_image_item);
	gnome_canvas_item_hide(left_highlight_image_item);

	// make sure that next hand is not the same as previous
	do {
		i = RAND(0,gcomprisBoard->level*8-1);
	} while ( i == last_hand );

	last_hand = i;

	if ( ((int) (i/2)) % 2 == 1)
		answer = LEFT;
		else
			answer = RIGHT;

	str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, hands[i]);
  hand_pixmap = gcompris_load_pixmap(str);
	hand_image_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", hand_pixmap,
				      "x", (double) HAND_X - (gdk_pixbuf_get_width(hand_pixmap)/2),
				      "y", (double) HAND_Y - (gdk_pixbuf_get_height(hand_pixmap)/2),
				      "width", (double) gdk_pixbuf_get_width(hand_pixmap),
				      "height", (double) gdk_pixbuf_get_height(hand_pixmap),
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);

  g_free(str);

  gdk_pixbuf_unref(highlight_pixmap);
  gdk_pixbuf_unref(hand_pixmap);

  gtk_signal_connect(GTK_OBJECT(gcomprisBoard->canvas), "event",  (GtkSignalFunc) item_event, NULL);

  return NULL;
}
/* =====================================================================
 *
 * =====================================================================*/
static void game_won() {
  gcomprisBoard->sublevel++;
  if(gcomprisBoard->sublevel > gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
			board_finished(BOARD_FINISHED_TUXLOCO);
			return;
      }
  }
  leftright_next_level();
}

/* =====================================================================
 *
 * =====================================================================*/
static gboolean process_ok_timeout() {
  gcompris_display_bonus(gamewon, BONUS_SMILEY);
	return FALSE;
}

static void process_ok() {
	// leave time to display the right answer
  g_timeout_add(TIME_CLICK_TO_BONUS, process_ok_timeout, NULL);
}
/* =====================================================================
 *
 * =====================================================================*/
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
  double x, y;
  int side;

  x = event->button.x;
  y = event->button.y;

  if (!gcomprisBoard || board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      gnome_canvas_c2w(gcomprisBoard->canvas, x, y, &x, &y);

      if (y>CLICKABLE_Y1 && y<CLICKABLE_Y2) {
	if (x>CLICKABLE_X1 && x<CLICKABLE_X2) { // the left button is clicked
	  side = LEFT;
	  highlight_selected(side);
	  gamewon = (side == answer);
          process_ok();
	}
	if (x>CLICKABLE_X3 && x<CLICKABLE_X4) { // the left button is clicked
	  side = RIGHT;
	  highlight_selected(side);
	  gamewon = (side == answer);
          process_ok();
	}
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
static void highlight_selected(int side) {
	if (side == LEFT) {
		gnome_canvas_item_hide(right_highlight_image_item);
		gnome_canvas_item_show(left_highlight_image_item);
	}
	if (side == RIGHT) {
		gnome_canvas_item_show(right_highlight_image_item);
		gnome_canvas_item_hide(left_highlight_image_item);
	}
}
