/* gcompris - read_colors.c
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

#include <string.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static int gamewon;

static void		 process_ok(void);
static void		 highlight_selected(int);
static void		 game_won(void);

/* ================================================================ */
static GooCanvasItem *boardRootItem = NULL;
static GooCanvasItem *highlight_image_item = NULL;
static GooCanvasItem *color_item = NULL;
static GooCanvasItem *clock_image_item = NULL;
static GdkPixbuf *clock_pixmap = NULL;

static GooCanvasItem *read_colors_create_item(GooCanvasItem *parent);
static void read_colors_destroy_all_items(void);
static void read_colors_next_level(void);
static gboolean	item_event (GooCanvasItem  *item,
			    GooCanvasItem  *target,
			    GdkEventButton *event,
			    gpointer data);
static void update_clock();

static int highlight_width, highlight_height, errors;
static GList * listColors = NULL;

#define MAX_ERRORS 10
#define CLOCK_X 40
#define CLOCK_Y 420

static gchar* colors[] =
  {
    N_("blue"),
    N_("brown"),
    N_("green"),
    N_("grey"),
    N_("orange"),
    N_("violet"),
    N_("red"),
    N_("yellow"),
    N_("black"),
    N_("white"),
    N_("pink"),
  };

#define LAST_COLOR G_N_ELEMENTS(colors)

static int X[] = {100,300,500,700,200,400,600,100,300,500,700};
static int Y[] = {90,90,90,90,180,180,180,270,270,270,270};

/* (x1,y1) and (x2, y2) are the coordinates of the rectangle where to
   draw the color's name */
static int color_x1 = 240, color_x2 = 570;
static int color_y1 = 395, color_y2 = 490;

#define RADIUS 60

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Read Colors",
    "Click on the corresponding color",
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
    NULL,
    NULL,
    NULL
  };

/* =====================================================================
 *
 * =====================================================================*/
GET_BPLUGIN_INFO(read_colors)

/* =====================================================================
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 * =====================================================================*/
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  gc_bar_hide(FALSE);
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
  int i,list_length;

  if(agcomprisBoard!=NULL) {
    gcomprisBoard=agcomprisBoard;
    gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
		      "read_colors/read_colors_background.png");
    gcomprisBoard->level=1;
    gcomprisBoard->maxlevel=1;
    gc_bar_set(0);

    gamewon = FALSE;
    errors = MAX_ERRORS;

    // we generate a list of color indexes in a random order
    while (g_list_length(listColors) > 0)
      listColors = g_list_remove(listColors, g_list_nth_data(listColors,0));

    for (i=0; i<LAST_COLOR; i++)
      list = g_list_append(list, GINT_TO_POINTER(i));

    while ((list_length = g_list_length(list))) {
      i = list_length == 1 ? 0 : g_random_int_range(0,g_list_length(list)-1);
      item = g_list_nth_data(list, i);
      listColors = g_list_append(listColors, item);
      list = g_list_remove(list, item);
    }
    g_list_free(list);

    g_signal_connect(goo_canvas_get_root_item(gcomprisBoard->canvas),
		     "button_press_event", (GtkSignalFunc) item_event,
		     NULL);
    read_colors_next_level();
    pause_board(FALSE);
  }
}

/* =====================================================================
 *
 * =====================================================================*/
static void
end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      g_signal_handlers_disconnect_by_func(goo_canvas_get_root_item(gcomprisBoard->canvas),
					   (GtkSignalFunc) item_event, NULL);

      pause_board(TRUE);
      gc_score_end();
      read_colors_destroy_all_items();
      // free list
      while (g_list_length(listColors) > 0)
	listColors = g_list_remove(listColors, g_list_nth_data(listColors,0));

      g_list_free(listColors);
      listColors = NULL;
    }
  gcomprisBoard = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
  if (gcomprisBoard) {
    if(g_strcasecmp(gcomprisBoard->type, "read_colors")==0) {
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
static void read_colors_next_level() {

  read_colors_destroy_all_items();
  gamewon = FALSE;

  /* Try the next level */
  read_colors_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));

  /* show text of color to find */
  color_item = goo_canvas_text_new (boardRootItem,
				    gettext(colors[GPOINTER_TO_INT(g_list_nth_data(listColors,0))]),
				    (double) (color_x1+color_x2)/2,
				    (double) (color_y1+color_y2)/2,
				    -1,
				    GTK_ANCHOR_CENTER,
				    "font", gc_skin_font_board_title_bold,
				    "fill-color", "darkblue",
				    NULL);

}
/* =====================================================================
 * Destroy all the items
 * =====================================================================*/
static void read_colors_destroy_all_items() {
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static GooCanvasItem *read_colors_create_item(GooCanvasItem *parent) {
  GdkPixbuf *highlight_pixmap = NULL;
  char *str = NULL;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, "read_colors_highlight.png");
  highlight_pixmap = gc_pixmap_load(str);

  highlight_image_item = goo_canvas_image_new (boardRootItem,
					       highlight_pixmap,
					       0,
					       0,
					       NULL);

  highlight_width = gdk_pixbuf_get_width(highlight_pixmap);
  highlight_height = gdk_pixbuf_get_height(highlight_pixmap);

  g_free(str);
  g_object_set (highlight_image_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

  gdk_pixbuf_unref(highlight_pixmap);

  /* setup the clock */
  str = g_strdup_printf("%s%d.png", "timers/clock",errors);
  clock_pixmap = gc_skin_pixmap_load(str);

  clock_image_item = goo_canvas_image_new (boardRootItem,
					   clock_pixmap,
					   CLOCK_X,
					   CLOCK_Y,
					   NULL);

  g_free(str);

  return NULL;
}
/* =====================================================================
 *
 * =====================================================================*/
static void update_clock() {
  char *str = g_strdup_printf("%s%d.png", "timers/clock",errors);

  goo_canvas_item_remove(clock_image_item);

  clock_pixmap = gc_skin_pixmap_load(str);

  clock_image_item = goo_canvas_image_new (boardRootItem,
					   clock_pixmap,
					   CLOCK_X,
					   CLOCK_Y,
					   NULL);

  gdk_pixbuf_unref(clock_pixmap);
  g_free(str);
}
/* =====================================================================
 *
 * =====================================================================*/
static void game_won() {
  gcomprisBoard->sublevel++;

  listColors = g_list_remove(listColors, g_list_nth_data(listColors,0));

  if( g_list_length(listColors) <= 0 ) { // the current board is finished : bail out
    gc_bonus_end_display(GC_BOARD_FINISHED_TUXLOCO);
    return;
  }

  read_colors_next_level();
}
/* =====================================================================
 *
 * =====================================================================*/
static gboolean process_ok_timeout() {
  gc_bonus_display(gamewon, GC_BONUS_SMILEY);
  if (!gamewon)
    errors--;
  if (errors <1)
    errors = 1;
  update_clock();

  if (errors <= 1) {
    gc_bonus_end_display(GC_BOARD_FINISHED_TOOMANYERRORS);
  }

	return FALSE;
}

static void process_ok() {
  gc_bar_hide(TRUE);
  // leave time to display the right answer
  g_timeout_add(TIME_CLICK_TO_BONUS, process_ok_timeout, NULL);
}

/* =====================================================================
 *
 * =====================================================================*/
static gboolean item_event (GooCanvasItem  *item,
			    GooCanvasItem  *target,
			    GdkEventButton *event,
			    gpointer data)
{
  double x, y;
  int i, clicked;

  x = event->x;
  y = event->y;

  if (!gcomprisBoard || board_paused)
    return FALSE;

  clicked = -1;
  for (i=0; i<LAST_COLOR; i++) {
    if (hypot((double) (X[i]-x),(double)(Y[i]-y)) < RADIUS) {
      clicked = i;
      break;
    }
  }

  if (clicked >= 0) {
    gc_sound_play_ogg ("sounds/bleep.wav", NULL);
    board_paused = TRUE;
    highlight_selected(clicked);
    gamewon = (clicked == GPOINTER_TO_INT(g_list_nth_data(listColors,0)));
    process_ok();
  }

  return FALSE;
}

/* =====================================================================
 *
 * =====================================================================*/
static void highlight_selected(int c) {
  int x, y;

  g_assert(c>=0 && c<LAST_COLOR);

  x = X[c];
  y = Y[c];

  x -= highlight_width/2;
  y -= highlight_height/2;
  g_object_set (highlight_image_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
  gc_item_absolute_move(highlight_image_item, x, y);
}
