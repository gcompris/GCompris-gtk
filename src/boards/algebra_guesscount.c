/* gcompris - algebra_guesscount.c
 *
 * Copyright (C) 2001 Pascal Georges
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
#include <math.h>
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
static gint process_time_id = 0;
static void process_ok(void);
static void process_time(void);
static void game_won();

static void destroy_board();

/* 4 levels :
 * 1evel 1 : 2 numbers and 1 operation
 * 1evel 2 : 3 numbers and 2 operations
 * 1evel 3 : 4 numbers and 3 operations
 * 1evel 4 : 5 numbers and 4 operations
 */
#define NUMBER_OF_SUBLEVELS 2
#define NUMBER_OF_LEVELS 4
#define MAX_NUMBER 5

#define BUTTON_WIDTH 81
#define BUTTON_HEIGHT 64
#define HORIZONTAL_SEPARATION 20

static const char oper_values[] = {'+', '-', 'x', ':', '='};
static const int num_values[] = {1,2,3,4,5,6,7,8,9,10,25,50,100};
#define NUM_VALUES 13
#define Y_OPE 20
#define Y_NUM 100
#define Y_ANS 400

static char answer_oper[MAX_NUMBER-1];
static int answer_num[MAX_NUMBER];
static int click_count;

/* ================================================================ */
static GnomeCanvasGroup *boardRootItem = NULL;

static GdkPixbuf * num_pixmap[NUM_VALUES];
static GdkPixbuf * oper_pixmap[4];
static GdkPixbuf *button_pixmap = NULL;

static GnomeCanvasItem *oper_item[4];
static GnomeCanvasItem *num_item[MAX_NUMBER];

static GnomeCanvasItem *algebra_guesscount_create_item(GnomeCanvasGroup *parent);
static void algebra_guesscount_destroy_all_items(void);
static void algebra_guesscount_next_level(void);
static gint item_event_num(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint item_event_oper(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Guess operations"),
    N_("Guess operations"),
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

/* ==================================== */
/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 */
BoardPlugin * get_bplugin_info(void){
  return &menu_bp;
}

/* ==================================== */
// in : boolean TRUE = PAUSE : FALSE = CONTINUE
static void pause_board (gboolean pause){
  if(gcomprisBoard==NULL)
    return;

  if(gamewon == TRUE && pause == FALSE) {
      game_won();
    }

  board_paused = pause;
}

/* ==================================== */
static void start_board (GcomprisBoard *agcomprisBoard) {
	int i;
  gchar *str;

  if(agcomprisBoard!=NULL){
      gcomprisBoard=agcomprisBoard;

			// load pixmap files
			for (i=0; i<NUM_VALUES; i++) {
  			str = g_strdup_printf("%s/%d.png", gcomprisBoard->boarddir,num_values[i]);
				num_pixmap[i] = gcompris_load_pixmap(str);
			  g_free(str);
				}
			for (i=0; i<5; i++) {
  			str = g_strdup_printf("%s/%c.png", gcomprisBoard->boarddir,oper_values[i]);
				oper_pixmap[i] = gcompris_load_pixmap(str);
			  g_free(str);
				}

			str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir,"button.png");
   		button_pixmap = gcompris_load_pixmap(str);
			g_free(str);

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			     "gcompris/animals/tiger1_by_Ralf_Schmode.jpg");
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=NUMBER_OF_LEVELS;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=NUMBER_OF_SUBLEVELS; /* Go to next level after this number of 'play' */
      gcompris_score_start(SCORESTYLE_NOTE,
			   50,
			   gcomprisBoard->height - 50,
			   gcomprisBoard->number_of_sublevel);
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL|GCOMPRIS_BAR_OK);

      algebra_guesscount_next_level();

      gamewon = FALSE;
      pause_board(FALSE);
    }
}

/* ==================================== */
static void end_board () {
  if(gcomprisBoard!=NULL) {
      pause_board(TRUE);
      gcompris_score_end();
			destroy_board();
			algebra_guesscount_destroy_all_items();
    }
}

/* ==================================== */
static void set_level (guint level) {
  if(gcomprisBoard!=NULL) {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      algebra_guesscount_next_level();
    }
}

/* ==================================== */
gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
  if (gcomprisBoard) {
      if(g_strcasecmp(gcomprisBoard->type, "algebra_guesscount")==0) {
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;
	  return TRUE;
	}
    }
  return FALSE;
}

/* ==================================== */
/* set initial values for the next level */
static void algebra_guesscount_next_level() {
  gcompris_bar_set_level(gcomprisBoard);

  algebra_guesscount_destroy_all_items();
  gamewon = FALSE;
	click_count = 0;

  gcompris_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  algebra_guesscount_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void algebra_guesscount_destroy_all_items() {
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
static GnomeCanvasItem *algebra_guesscount_create_item(GnomeCanvasGroup *parent) {
  GdkFont *gdk_font;
//  double dx1, dy1, dx2, dy2;
  int i,j, result, r, xOffset;
  gboolean minus, divide;

  for (i=0; i<gcomprisBoard->level+1; i++) {
    j = (int)((NUM_VALUES-1)*rand()/(RAND_MAX+1.0));
    answer_num[i] = num_values[j];
  }

  result = answer_num[0];
  for (i=0; i<gcomprisBoard->level-1; i++) {
    // + and x can always be chosen, but we must ensure - and / are valid
    if (result - answer_num[i+1] < 0)
	minus = FALSE;
	else
	  minus = TRUE;
    if (result % answer_num[i+1] != 0)
	divide = FALSE;
	else
	  divide = TRUE;
    r = 2 + (minus ? 1 : 0) + (divide ? 1 : 0);
    j  = (int)(r*rand()/(RAND_MAX+1.0));
    assert(j>=0 && j<r);
    switch (j) {
    case 0 : answer_oper[i] = '+'; break;
    case 1 : answer_oper[i] = 'x'; break;
    case 2 : 	if (minus)
			answer_oper[i] = '-';
		else
			answer_oper[i] = '/';
    		break;
    case 3 : if ((int)(2*rand()/(RAND_MAX+1.0)) == 0)
							answer_oper[i] = '/';
							else
							answer_oper[i] = '-';
    		break;
    default : printf("Bug in guesscount\n"); break;
    }
  }

// dump for DEBUG
  for (i=0; i<gcomprisBoard->level; i++) {
		printf("oper:%c ",answer_oper[i]);
		printf("num:%d ",answer_num[i]);
	}
	printf("num:%d\n", answer_num[gcomprisBoard->level]);
// end DEBUG

  gdk_font = gdk_font_load ("-adobe-times-medium-r-normal--*-240-*-*-*-*-*-*");

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));
/*
	text = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_text_get_type (),
				"text", _(board->question),
				"font_gdk", gdk_font,
				"x", (double) txt_area_x,
				"y", (double) txt_area_y,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color", TEXT_COLOR,
				NULL);

  gnome_canvas_item_get_bounds(text, &dx1, &dy1, &dx2, &dy2);
  yOffset += VERTICAL_SEPARATION + dy2-dy1;
*/
  xOffset = (gcomprisBoard->width - 4 * BUTTON_WIDTH - 3 * HORIZONTAL_SEPARATION)/2;
	for (i=0; i<4; i++) {
		oper_item[i] = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", oper_pixmap[i],
				      "x", (double) xOffset ,
				      "y", (double) Y_OPE,
				      "width", (double) BUTTON_WIDTH,
				      "height", (double) BUTTON_HEIGHT,
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);
		xOffset += BUTTON_WIDTH+HORIZONTAL_SEPARATION;
  	gtk_signal_connect(GTK_OBJECT(oper_item[i]), "event", (GtkSignalFunc) item_event_oper, (void *) &oper_values[i]);
		}

	xOffset = (gcomprisBoard->width - (gcomprisBoard->level+1) * BUTTON_WIDTH - gcomprisBoard->level * HORIZONTAL_SEPARATION)/2;
	for (i=0; i<gcomprisBoard->level+1; i++) {
		num_item[i] = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", num_pixmap[i],
				      "x", (double) xOffset ,
				      "y", (double) Y_NUM,
				      "width", (double) BUTTON_WIDTH,
				      "height", (double) BUTTON_HEIGHT,
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);
		xOffset += BUTTON_WIDTH+HORIZONTAL_SEPARATION;
  	gtk_signal_connect(GTK_OBJECT(num_item[i]), "event", (GtkSignalFunc) item_event_num, GINT_TO_POINTER(num_values[i]));
		}

  return NULL;
}
/* ==================================== */
static void game_won() {
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) {
	board_finished(BOARD_FINISHED_TUXPLANE);
	return;
    }
  }
  algebra_guesscount_next_level();
}

/* ==================================== */
static void process_ok()
{
  if (gamewon) {
  }
  process_time_id = gtk_timeout_add (2000, (GtkFunction) process_time, NULL);
}
/* ==================================== */
static void process_time(){
  if (process_time_id) {
    gtk_timeout_remove (process_time_id);
    process_time_id = 0;
  }
  gcompris_display_bonus(gamewon, BONUS_FLOWER);
}
/* ==================================== */
static gint item_event_oper(GnomeCanvasItem *item, GdkEvent *event, gpointer data){
	char oper;
  if(board_paused)
    return FALSE;

	oper = *((char*)data);
  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
	printf("clicked %c\n", oper);
/*	gamewon = TRUE;
      } else {
	gamewon = FALSE;
      }*/
      break;
    }
  return FALSE;
}
/* ==================================== */
static gint item_event_num(GnomeCanvasItem *item, GdkEvent *event, gpointer data){
	int num;

  if(board_paused)
    return FALSE;

	num = GPOINTER_TO_INT(data);
  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
	printf("clicked %d\n", num);
/*	gamewon = TRUE;
      } else {
	gamewon = FALSE;
      }*/
      break;
    }
  return FALSE;
}

/* ======================================= */
// causes a segfault, why ?
// FIXME : potential memory leak
static void destroy_board() {
	return;
	int i;
	for (i=0; i<NUM_VALUES; i++)
  	gdk_pixbuf_unref(num_pixmap[i]);
	for (i=0; i<5; i++)
		gdk_pixbuf_unref(oper_pixmap[i]);
}
