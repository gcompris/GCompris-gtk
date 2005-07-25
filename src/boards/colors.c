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
static void		 repeat(void);
static void		 colors_config_start(GcomprisBoard *agcomprisBoard,
					     GcomprisProfile *aProfile);
static void		 colors_config_stop(void);

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
static gchar *colors[LAST_COLOR*2] = {
  "blue", 	N_("Click on the blue toon"),
  "brown",	N_("Click on the brown toon"),
  "green",	N_("Click on the green toon"),
  "grey",	N_("Click on the grey toon"),
  "orange",	N_("Click on the orange toon"),
  "purple",	N_("Click on the purple toon"),
  "red",	N_("Click on the red toon"),
  "yellow",	N_("Click on the yellow toon"),
  "black",	N_("Click on the black toon"),
  "white",	N_("Click on the white toon")
};

static int X[] = {75,212,242,368,414,533,578,709};
static int Y[] = {25,170,180,335,337,500};

/* Description of this plugin */
static BoardPlugin menu_bp =
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
    repeat,
    colors_config_start,
    colors_config_stop
  };

/* =====================================================================
 *
 * =====================================================================*/
GET_BPLUGIN_INFO(colors)


/* ======================= */
/* = config_start        = */
/* ======================= */

static GtkVBox *config_vbox = NULL;

static GcomprisConfCallback conf_apply(GHashTable *table)
{
  printf ("Config Hashtable size %d\n", g_hash_table_size(table));
}


static void 
colors_config_start(GcomprisBoard *agcomprisBoard,
		    GcomprisProfile *aProfile)
{
  gchar *label;
  
  label = g_strdup_printf("<b>%s</b> configuration\n for profile <b>%s</b>",
			  agcomprisBoard->name, aProfile->name);

    gcompris_configuration_window(label, conf_apply);

    g_free(label);

    gcompris_boolean_box("Test Check Box", "key1", TRUE);
}

/* ======================= */
/* = config_stop        = */
/* ======================= */
static void 
colors_config_stop()
{
}


/* =====================================================================
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 * =====================================================================*/
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  gcompris_bar_hide(FALSE);
  if(gamewon == TRUE && pause == FALSE) /* the game is won */
    game_won();

  board_paused = pause;
}

/* =====================================================================
 *
 * =====================================================================*/
static void start_board (GcomprisBoard *agcomprisBoard) {
  GcomprisProperties	*properties = gcompris_get_properties();
  GList * list = NULL;
  int * item;
  int i;

  if(agcomprisBoard!=NULL) {
    gcomprisBoard=agcomprisBoard;
    gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "colors/colors_bg.jpg");
    gcomprisBoard->level=1;
    gcomprisBoard->maxlevel=1;

    if(properties->fx) {
      gcompris_bar_set(GCOMPRIS_BAR_REPEAT);
    } else {
      gcompris_bar_set(0);
    }

    gamewon = FALSE;

    // we generate a list of color indexes in a random order
    for (i=0; i<LAST_COLOR; i++)
      list = g_list_append(list, GINT_TO_POINTER(i));

    while ((g_list_length(list) > 0)) {
      i = RAND(0,g_list_length(list)-1);
      item = g_list_nth_data(list, i);
      listColors = g_list_append(listColors, item);
      list = g_list_remove(list, item);
    }
    g_list_free(list);

    gtk_signal_connect(GTK_OBJECT(gcomprisBoard->canvas), "event",  (GtkSignalFunc) item_event, NULL);
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
    g_list_free(listColors);
    listColors=NULL;
  }
  gcomprisBoard = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
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
    {
      char *str  = NULL;
      char *str1 = NULL;
      char *str2 = NULL;
      GcomprisProperties	*properties = gcompris_get_properties();

      str1 = g_strdup_printf("%s%s", colors[GPOINTER_TO_INT(g_list_nth_data(listColors, 0))*2],
			     ".ogg");
      str2 = gcompris_get_asset_file("gcompris colors", NULL, "audio/x-ogg", str1);

      /* If we don't find a sound in our locale or the sounds are disabled */
      if(str2 && properties->fx) {
	gcompris_play_ogg(str2, NULL);
      }
      else
	{
	  str = g_strdup_printf(gettext(colors[GPOINTER_TO_INT(g_list_nth_data(listColors, 0))*2+1]));
	  
	  gnome_canvas_item_new (boardRootItem,
				 gnome_canvas_text_get_type (),
				 "text", str,
				 "font", gcompris_skin_font_board_huge_bold,
				 "x", (double) BOARDWIDTH/2+2,
				 "y", (double) BOARDHEIGHT-25+2,
				 "anchor", GTK_ANCHOR_CENTER,
				 "fill_color", "black",
				 NULL);
	  
	  gnome_canvas_item_new (boardRootItem,
				 gnome_canvas_text_get_type (),
				 "text", str,
				 "font", gcompris_skin_font_board_huge_bold,
				 "x", (double) BOARDWIDTH/2,
				 "y", (double) BOARDHEIGHT-25,
				 "anchor", GTK_ANCHOR_CENTER,
				 "fill_color", "blue",
				 NULL);
	  
	  
	  g_free(str);
	}
      


      g_free(str1);
      g_free(str2);

    }
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

  gdk_pixbuf_unref(highlight_pixmap);

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
static gboolean process_ok_timeout() {
  gcompris_display_bonus(gamewon, BONUS_GNU);
  return FALSE;
}

static void process_ok() {
  gcompris_bar_hide(TRUE);
  // leave time to display the right answer
  g_timeout_add(TIME_CLICK_TO_BONUS, process_ok_timeout, NULL);
}
/* =====================================================================
 *
 * =====================================================================*/
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
  double x, y;
  int i, j, clicked;

  x = event->button.x;
  y = event->button.y;

  if (!gcomprisBoard || board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      gnome_canvas_c2w(gcomprisBoard->canvas, x, y, &x, &y);
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
	board_paused = TRUE;
	highlight_selected(clicked);
	gamewon = (clicked == GPOINTER_TO_INT(g_list_nth_data(listColors,0)));
        process_ok();
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
