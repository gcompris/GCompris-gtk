/* gcompris - submarine.c
 *
 * Copyright (C) 2003 Pascal Georges
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

#define DEG_TO_RAD(x) M_PI*(x)/180.0

#define SOUNDLISTFILE PACKAGE

GcomprisBoard *gcomprisBoard = NULL;
gboolean board_paused = TRUE;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
static int gamewon;

static void process_ok(void);
static void game_won();

#define NUMBER_OF_SUBLEVELS 6
#define NUMBER_OF_LEVELS 4

#define PURGE_AR 225
#define PURGE_AV 438
#define REGLEUR 330
#define CHASSE_BALLAST_AR_X 227
#define CHASSE_BALLAST_AR_Y 97
#define CHASSE_BALLAST_AV_X 440
#define CHASSE_BALLAST_AV_Y 98
#define CHASSE_REGLEUR_X 331
#define CHASSE_REGLEUR_Y 72
#define BARRE_AR_X 100
#define BARRE_AR_Y 50
#define BARRE_AV_X 530
#define BARRE_AV_Y 100

#define SUBMARINE_HEEL 10
#define SURFACE_IN_BACKGROUND 30
#define SURFACE_DEPTH 5.0
#define IP_DEPTH 13.0
#define SECURITY_DEPTH 55.0
#define MAX_DEPTH 300.0
#define SPEED_MAX 10
#define SPEED_STEP 1
#define AIR_INITIAL 50000
#define BATTERY_INITIAL 5000
#define MAX_BALLAST 10000
#define MAX_REGLEUR 500

#define SUBMARINE_INITIAL_X 10
#define WRAP_X 700
#define SUBMARINE_INITIAL_DEPTH SECURITY_DEPTH

#define RUDDER_STEP 5
#define RUDDER_MAX 15
#define RUDDER_CENTER_X 72
#define RUDDER_CENTER_Y 7
#define ENGINE_DOWN_X 42
#define ENGINE_DOWN_Y 104
#define ENGINE_UP_X 124
#define ENGINE_UP_Y 104
#define AIR_X 328
#define AIR_Y 109
#define BATTERY_X 285
#define BATTERY_Y 156
#define REGLEUR_X 330
#define REGLEUR_Y 37
#define UP 1
#define DOWN 0

#define UPDATE_DELAY 300
#define UPDATE_DELAY_SLOW 1500

#define TEXT_COLOR_FRONT "red"
#define TEXT_COLOR_BACK "orange"

/* ================================================================ */
static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem *sub_schema_image_item, *submarine_item,
	*ballast_av_purge_item, *ballast_ar_purge_item, *regleur_purge_item;
static GnomeCanvasItem *ballast_av_chasse_item, *ballast_ar_chasse_item, *regleur_chasse_item;
gboolean ballast_av_purge_open, ballast_ar_purge_open, regleur_purge_open;
gboolean ballast_av_chasse_open, ballast_ar_chasse_open, regleur_chasse_open;

static GnomeCanvasItem *barre_av_item, *barre_ar_item,
	*barre_av_up_item, *barre_av_down_item, *barre_ar_up_item, *barre_ar_down_item,
  *engine_up_item, *engine_down_item,
  *speed_item_back, *speed_item_front,
	*air_item_back, *air_item_front,
	*regleur_item_back, *regleur_item_front,
  *battery_item_back, *battery_item_front;

/* submarine parameters */
static double barre_av_angle, barre_ar_angle, depth, weight, resulting_weight, submarine_x, air, battery, regleur;
static double submarine_horizontal_speed, submarine_vertical_speed, assiette;
static double ballast_av_air, ballast_ar_air;

static GnomeCanvasItem *submarine_create_item(GnomeCanvasGroup *parent);
static void submarine_destroy_all_items(void);
static void submarine_next_level(void);
static gint ballast_av_purge_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint ballast_ar_purge_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint regleur_purge_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint ballast_ar_chasse_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint ballast_av_chasse_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint regleur_chasse_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint barre_av_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint barre_ar_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint engine_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

static void setSpeed(double value);
static void setBattery(double value);
static void setAir(double value);
static void setRegleur(double value);
static void setAssiette(double value);

static void updateDynamic();

static gboolean update_timeout();
static gboolean update_timeout_slow();
static guint timer_id, timer_slow_id;

/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Submarine"),
    N_("Control the depth of a submarine"),
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
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "submarine/sub_bg.jpg");
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=NUMBER_OF_LEVELS;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel = NUMBER_OF_SUBLEVELS;
      submarine_next_level();
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
      submarine_destroy_all_items();
    }
  /* kill pending timers */
  g_source_remove(timer_id);
	g_source_remove(timer_slow_id);

  gcomprisBoard = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static void set_level (guint level) {
  if(gcomprisBoard!=NULL) {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      submarine_next_level();
    }
}

/* =====================================================================
 *
 * =====================================================================*/
gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
	if (gcomprisBoard) {
      if(g_strcasecmp(gcomprisBoard->type, "submarine")==0) {
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
static void submarine_next_level() {
	ballast_av_purge_open = ballast_ar_purge_open = regleur_purge_open = FALSE;
	ballast_av_chasse_open = ballast_ar_chasse_open = regleur_chasse_open = FALSE;
	barre_av_angle = barre_ar_angle = 0.0;
  depth = SUBMARINE_INITIAL_DEPTH;
  submarine_horizontal_speed = 0.0;
  submarine_x = SUBMARINE_INITIAL_X;
  weight = -200.0;
  regleur = 300.0;
  air = AIR_INITIAL;
  battery = BATTERY_INITIAL;
	ballast_av_air = ballast_ar_air = 0.0;

  submarine_destroy_all_items();
  gamewon = FALSE;

  /* Try the next level */
  submarine_create_item(gnome_canvas_root(gcomprisBoard->canvas));

}
/* =====================================================================
 * Destroy all the items
 * =====================================================================*/
static void submarine_destroy_all_items() {
  if(boardRootItem!=NULL)
      gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static GnomeCanvasItem *submarine_create_item(GnomeCanvasGroup *parent) {
  GdkPixbuf *pixmap = NULL;
  char *str = NULL;
  char s12[12];
	int i, w, h, schema_x, schema_y;

	boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, "submarine.png");
  pixmap = gcompris_load_pixmap(str);
	submarine_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) SUBMARINE_INITIAL_X,
						"y", (double) SUBMARINE_INITIAL_DEPTH + SURFACE_IN_BACKGROUND - SUBMARINE_HEEL,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            //"anchor", GTK_ANCHOR_CENTER, // to be removed or item_move does not work
						NULL);

  g_free(str);
  gdk_pixbuf_unref(pixmap);

  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, "sub_schema.png");
  pixmap = gcompris_load_pixmap(str);

  w = gdk_pixbuf_get_width(pixmap);
  h = gdk_pixbuf_get_height(pixmap);

  schema_x = (gcomprisBoard->width - w)/2 ;
  schema_y = gcomprisBoard->height - h;
  sub_schema_image_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) schema_x + w/2.0,
						"y", (double) schema_y + h/2.0,
						"width", (double) w,
						"height", (double) h,
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);

  g_free(str);
  gdk_pixbuf_unref(pixmap);

  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, "vanne.png");
  pixmap = gcompris_load_pixmap(str);
  w = gdk_pixbuf_get_width(pixmap);
  h = gdk_pixbuf_get_height(pixmap);
  ballast_ar_purge_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) PURGE_AR + schema_x,
						"y", (double) schema_y + h/2.0 -1.0 ,
						"width", (double) w,
						"height", (double) h,
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  gtk_signal_connect(GTK_OBJECT(ballast_ar_purge_item), "event",  (GtkSignalFunc) ballast_ar_purge_event, NULL);

  ballast_av_purge_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) PURGE_AV + schema_x,
						"y", (double) schema_y + h/2.0 -1.0  ,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  gtk_signal_connect(GTK_OBJECT(ballast_av_purge_item), "event",  (GtkSignalFunc) ballast_av_purge_event, NULL);

  regleur_purge_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) REGLEUR + schema_x,
						"y", (double) schema_y + h/2.0 -2.0,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  gtk_signal_connect(GTK_OBJECT(regleur_purge_item), "event",  (GtkSignalFunc) regleur_purge_event, NULL);

  ballast_av_chasse_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) schema_x + CHASSE_BALLAST_AV_X,
						"y", (double) schema_y +  CHASSE_BALLAST_AV_Y,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  gtk_signal_connect(GTK_OBJECT(ballast_av_chasse_item), "event",  (GtkSignalFunc) ballast_av_chasse_event, NULL);

  ballast_ar_chasse_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) schema_x + CHASSE_BALLAST_AR_X,
						"y", (double) schema_y +  CHASSE_BALLAST_AR_Y,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  gtk_signal_connect(GTK_OBJECT(ballast_ar_chasse_item), "event",  (GtkSignalFunc) ballast_ar_chasse_event, NULL);

  regleur_chasse_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) schema_x + CHASSE_REGLEUR_X,
						"y", (double) schema_y + CHASSE_REGLEUR_Y,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  gtk_signal_connect(GTK_OBJECT(regleur_chasse_item), "event",  (GtkSignalFunc) regleur_chasse_event, NULL);

  g_free(str);
  gdk_pixbuf_unref(pixmap);

	// DEPTH RUDDERS
  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, "rudder.png");
  pixmap = gcompris_load_pixmap(str);
  barre_av_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) schema_x + BARRE_AV_X,
						"y", (double) schema_y + BARRE_AV_Y,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  barre_ar_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) schema_x + BARRE_AR_X,
						"y", (double) schema_y + BARRE_AR_Y,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  g_free(str);
  gdk_pixbuf_unref(pixmap);

#define COMMAND_OFFSET 20.0
  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, "up.png");
  pixmap = gcompris_load_pixmap(str);
  barre_av_up_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) schema_x + BARRE_AV_X + COMMAND_OFFSET,
						"y", (double) schema_y + BARRE_AV_Y - COMMAND_OFFSET,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  barre_ar_up_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) schema_x + BARRE_AR_X + COMMAND_OFFSET,
						"y", (double) schema_y + BARRE_AR_Y - COMMAND_OFFSET,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  engine_up_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) schema_x + ENGINE_UP_X,
						"y", (double) schema_y + ENGINE_UP_Y,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  g_free(str);
  gdk_pixbuf_unref(pixmap);

  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, "down.png");
  pixmap = gcompris_load_pixmap(str);
  barre_av_down_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) schema_x + BARRE_AV_X + COMMAND_OFFSET,
						"y", (double) schema_y + BARRE_AV_Y + COMMAND_OFFSET,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  barre_ar_down_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) schema_x + BARRE_AR_X + COMMAND_OFFSET,
						"y", (double) schema_y + BARRE_AR_Y + COMMAND_OFFSET,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  engine_down_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", pixmap,
						"x", (double) schema_x + ENGINE_DOWN_X,
						"y", (double) schema_y + ENGINE_DOWN_Y,
						"width", (double) gdk_pixbuf_get_width(pixmap),
						"height", (double) gdk_pixbuf_get_height(pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
            "anchor", GTK_ANCHOR_CENTER,
						NULL);
  g_free(str);
  gdk_pixbuf_unref(pixmap);

  gtk_signal_connect(GTK_OBJECT(barre_av_up_item), "event",  (GtkSignalFunc) barre_av_event, GINT_TO_POINTER(UP));
  gtk_signal_connect(GTK_OBJECT(barre_ar_up_item), "event",  (GtkSignalFunc) barre_ar_event, GINT_TO_POINTER(UP));
  gtk_signal_connect(GTK_OBJECT(barre_av_down_item), "event",  (GtkSignalFunc) barre_av_event, GINT_TO_POINTER(DOWN));
  gtk_signal_connect(GTK_OBJECT(barre_ar_down_item), "event",  (GtkSignalFunc) barre_ar_event, GINT_TO_POINTER(DOWN));
  gtk_signal_connect(GTK_OBJECT(engine_up_item), "event",  (GtkSignalFunc) engine_event, GINT_TO_POINTER(UP));
  gtk_signal_connect(GTK_OBJECT(engine_down_item), "event",  (GtkSignalFunc) engine_event, GINT_TO_POINTER(DOWN));

  // displays the speed on the engine
  sprintf(s12,"%d",(int)submarine_horizontal_speed);
  speed_item_back = gnome_canvas_item_new (boardRootItem,
					    gnome_canvas_text_get_type (),
					    "text", s12,
					    "font", FONT_BOARD_TITLE_BOLD,
					    "x", (double) schema_x + (ENGINE_UP_X + ENGINE_DOWN_X)/2 +1,
					    "y", (double) schema_y + ENGINE_UP_Y + 1,
					    "anchor", GTK_ANCHOR_CENTER,
					    "fill_color", TEXT_COLOR_BACK,
					    NULL);
  speed_item_front = gnome_canvas_item_new (boardRootItem,
					     gnome_canvas_text_get_type (),
					     "text", s12,
					     "font", FONT_BOARD_TITLE_BOLD,
					     "x", (double) schema_x + (ENGINE_UP_X + ENGINE_DOWN_X)/2,
					     "y", (double) schema_y + ENGINE_UP_Y,
					     "anchor", GTK_ANCHOR_CENTER,
					     "fill_color", TEXT_COLOR_FRONT,
					     NULL);

  // displays the remaining air value
  sprintf(s12,"%d", (int)air);
  air_item_back = gnome_canvas_item_new (boardRootItem,
					    gnome_canvas_text_get_type (),
					    "text", s12,
					    "font", FONT_BOARD_TITLE_BOLD,
					    "x", (double) schema_x + AIR_X +1,
					    "y", (double) schema_y + AIR_Y + 1,
					    "anchor", GTK_ANCHOR_CENTER,
					    "fill_color", TEXT_COLOR_BACK,
					    NULL);
  air_item_front = gnome_canvas_item_new (boardRootItem,
					     gnome_canvas_text_get_type (),
					     "text", s12,
					     "font", FONT_BOARD_TITLE_BOLD,
					     "x", (double) schema_x + AIR_X,
					     "y", (double) schema_y + AIR_Y,
					     "anchor", GTK_ANCHOR_CENTER,
					     "fill_color", TEXT_COLOR_FRONT,
					     NULL);

  // displays the remaining battery value
  sprintf(s12,"%d", (int)battery);
  battery_item_back = gnome_canvas_item_new (boardRootItem,
					    gnome_canvas_text_get_type (),
					    "text", s12,
					    "font", FONT_BOARD_TITLE_BOLD,
					    "x", (double) schema_x + BATTERY_X +1,
					    "y", (double) schema_y + BATTERY_Y + 1,
					    "anchor", GTK_ANCHOR_CENTER,
					    "fill_color", TEXT_COLOR_BACK,
					    NULL);
  battery_item_front = gnome_canvas_item_new (boardRootItem,
					     gnome_canvas_text_get_type (),
					     "text", s12,
					     "font", FONT_BOARD_TITLE_BOLD,
					     "x", (double) schema_x + BATTERY_X,
					     "y", (double) schema_y + BATTERY_Y,
					     "anchor", GTK_ANCHOR_CENTER,
					     "fill_color", TEXT_COLOR_FRONT,
					     NULL);

  // displays the remaining battery value
  sprintf(s12,"%d", (int)regleur);
  regleur_item_back = gnome_canvas_item_new (boardRootItem,
					    gnome_canvas_text_get_type (),
					    "text", s12,
					    "font", FONT_BOARD_TITLE_BOLD,
					    "x", (double) schema_x + REGLEUR_X +1,
					    "y", (double) schema_y + REGLEUR_Y + 1,
					    "anchor", GTK_ANCHOR_CENTER,
					    "fill_color", TEXT_COLOR_BACK,
					    NULL);
  regleur_item_front = gnome_canvas_item_new (boardRootItem,
					     gnome_canvas_text_get_type (),
					     "text", s12,
					     "font", FONT_BOARD_TITLE_BOLD,
					     "x", (double) schema_x + REGLEUR_X,
					     "y", (double) schema_y + REGLEUR_Y,
					     "anchor", GTK_ANCHOR_CENTER,
					     "fill_color", TEXT_COLOR_FRONT,
					     NULL);

  timer_id = g_timeout_add(UPDATE_DELAY, update_timeout, NULL);
  timer_slow_id = g_timeout_add(UPDATE_DELAY_SLOW, update_timeout_slow, NULL);
  return NULL;
}
/* =====================================================================
 * Periodically recalculate the submarine parameters
 * =====================================================================*/
static gboolean update_timeout() {
  double delta_air;
  gboolean regleur_dirty = FALSE;
	gboolean air_dirty = FALSE;

  /* air in ballasts */
	if (ballast_av_purge_open) {
		ballast_av_air -= UPDATE_DELAY/1000.0 *500.0; // 500 liters go out per second
    if (ballast_av_air < 0.0)
			ballast_av_air = 0.0;
  }
	if (ballast_ar_purge_open) {
		ballast_ar_air -= UPDATE_DELAY/1000.0 *500.0;
    if (ballast_ar_air < 0.0)
			ballast_ar_air = 0.0;
  }
	if (ballast_av_chasse_open && air>0.0) {
  	delta_air = UPDATE_DELAY/1000.0 *500.0; // 200 liters are injected each second
		ballast_av_air += delta_air;
    air -= delta_air;
    if (air<0.0)
    	air = 0.0;
    if (ballast_av_air > MAX_BALLAST)
			ballast_av_air = MAX_BALLAST;
    air_dirty = TRUE;
  }
	if (ballast_ar_chasse_open && air>0.0) {
  	delta_air = UPDATE_DELAY/1000.0 *500.0;
		ballast_ar_air += delta_air;
    air -= delta_air;
    if (air<0.0)
    	air = 0.0;
    if (ballast_ar_air > MAX_BALLAST)
			ballast_ar_air = MAX_BALLAST;
    air_dirty = TRUE;
  }

  if (air_dirty)
  	setAir(air);

  /* air in "regleur" (small ballast to finely balance the submarine) */
	if (regleur_purge_open) {
		regleur += UPDATE_DELAY/1000.0 *50.0; // 100 liters enter per second
    if (regleur > MAX_REGLEUR)
			regleur = MAX_REGLEUR;
   	regleur_dirty = TRUE;
  }
	if (regleur_chasse_open && air>0.0 && regleur > 0.0) {
  	delta_air = UPDATE_DELAY/1000.0 *50.0; // 50 liters are injected each second
		regleur -= delta_air;
    air -= delta_air;
    if (air<0.0)
    	air = 0.0;
    if (regleur < 0.0)
			regleur = 0.0;
   	regleur_dirty = TRUE;
    setAir(air);
  }

	if (regleur_dirty)
  	setRegleur(regleur);

	return TRUE;
}
/* =====================================================================
 * Periodically recalculate some submarine parameters, with a larger delay
 * =====================================================================*/
static gboolean update_timeout_slow() {
	double delta_assiette;
	/* assiette */
  delta_assiette = (ballast_ar_air - ballast_av_air)/300.0 +
  	(barre_av_angle - barre_ar_angle)/10.0*submarine_horizontal_speed;
  assiette += delta_assiette*UPDATE_DELAY/10000.0;
  if (assiette < -30.0)
  	assiette = -30.0;
  if (assiette > 30.0)
  	assiette = 30.0;
  setAssiette(assiette);

	/* battery */
  setBattery(battery -= submarine_horizontal_speed*submarine_horizontal_speed/10.0);

  /* position */
  updateDynamic();
  submarine_x += submarine_horizontal_speed;
  depth += submarine_vertical_speed;
  if (depth < 0.0)
  	depth = 0.0;
  if (depth > 300.0)
  	depth = 300.0;

  printf("depth = %d\n", (int) depth);

  /* if the submarine is too close from right, put it at left */
  if ( submarine_x > WRAP_X )
		submarine_x = SUBMARINE_INITIAL_X;

  item_absolute_move( submarine_item,submarine_x, depth + SURFACE_IN_BACKGROUND-SUBMARINE_HEEL);

  return TRUE;
}
/* =====================================================================
 * Updates submarine physics : the horizontal and vertical speeds,
 * the speed of rotation and global balance
 * =====================================================================*/
static void updateDynamic() {
	/* resulting_weight > 0 ==> the sub goes deeper
     regleur : this is the qty of water */
	resulting_weight = weight - ballast_av_air - ballast_ar_air + regleur;
  submarine_vertical_speed = resulting_weight/300.0 + submarine_horizontal_speed*sin(DEG_TO_RAD(assiette));
  printf("vspeed=%.2f hspeed=%.2f res_weight=%.2f asst=%.2f balav=%.0f balar=%.0f\n",
  	submarine_vertical_speed,submarine_horizontal_speed, resulting_weight, assiette,
    ballast_av_air, ballast_ar_air);
  /* if depth rudders are in the same direction */
  if (barre_ar_angle != 0.0 && barre_av_angle != 0.0) {
  	if (fabs(barre_ar_angle)/barre_ar_angle == fabs(barre_av_angle)/barre_av_angle) {
    	double a = (fabs(barre_ar_angle) > fabs(barre_av_angle)) ? barre_av_angle : barre_ar_angle;
      printf("2 barres dans le meme sens, prise en compte de %.2f\n", a);
			submarine_vertical_speed += a * submarine_horizontal_speed/30.0;
      }
  }
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
  submarine_next_level();
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
 *		ballast_av_purge_event
 * =====================================================================*/
static gint ballast_av_purge_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
	if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
			ballast_av_purge_open = !ballast_av_purge_open;
      if (ballast_av_purge_open)
  			item_rotate(item, 90.0);
        else
					item_rotate(item, 0.0);
			break;

    default:
      break;
    }
  return FALSE;
}

/* =====================================================================
 *		ballast_ar_purge_event
 * =====================================================================*/
static gint ballast_ar_purge_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
	if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
			ballast_ar_purge_open = !ballast_ar_purge_open;
      if (ballast_ar_purge_open)
  			item_rotate(item, 90.0);
        else
					item_rotate(item, 0.0);
			break;

    default:
      break;
    }
  return FALSE;
}
/* =====================================================================
 *		regleur_purge_event
 * =====================================================================*/
static gint regleur_purge_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
	if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
			regleur_purge_open = !regleur_purge_open;
      if (regleur_purge_open)
  			item_rotate(item, 90.0);
        else
					item_rotate(item, 0.0);
			break;

    default:
      break;
    }
  return FALSE;
}
/* =====================================================================
 *		ballast_ar_chasse_event
 * =====================================================================*/
static gint ballast_ar_chasse_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
	if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
			ballast_ar_chasse_open = !ballast_ar_chasse_open;
      if (ballast_ar_chasse_open)
  			item_rotate(item, 90.0);
        else
					item_rotate(item, 0.0);
			break;

    default:
      break;
    }
  return FALSE;
}
/* =====================================================================
 *		ballast_av_chasse_event
 * =====================================================================*/
static gint ballast_av_chasse_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
	if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
			ballast_av_chasse_open = !ballast_av_chasse_open;
      if (ballast_av_chasse_open)
  			item_rotate(item, 90.0);
        else
					item_rotate(item, 0.0);
			break;

    default:
      break;
    }
  return FALSE;
}
/* =====================================================================
 *
 * =====================================================================*/
static gint regleur_chasse_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
	if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
			regleur_chasse_open = !regleur_chasse_open;
      if (regleur_chasse_open)
  			item_rotate(item, 90.0);
        else
					item_rotate(item, 0.0);
			break;

    default:
      break;
    }
  return FALSE;
}

/* =====================================================================
 *		barre_av_event
 * =====================================================================*/
static gint barre_av_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
	int d = GPOINTER_TO_INT(data);
	if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
    	if (d == UP && barre_av_angle < RUDDER_MAX) {
        barre_av_angle += RUDDER_STEP;
        item_rotate_with_center( barre_av_item, barre_av_angle,RUDDER_CENTER_X,RUDDER_CENTER_Y);
      }
    	if (d == DOWN && barre_av_angle > -RUDDER_MAX) {
        barre_av_angle -= RUDDER_STEP;
        item_rotate_with_center( barre_av_item, barre_av_angle,RUDDER_CENTER_X,RUDDER_CENTER_Y);
      }
			break;

    default:
      break;
    }
  return FALSE;
}
/* =====================================================================
 *		barre_ar_event
 * =====================================================================*/
static gint barre_ar_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
	int d = GPOINTER_TO_INT(data);

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
    	if (d == UP && barre_ar_angle < RUDDER_MAX) {
        barre_ar_angle += RUDDER_STEP;
        item_rotate_with_center( barre_ar_item, barre_ar_angle,RUDDER_CENTER_X,RUDDER_CENTER_Y);
      }
    	if (d == DOWN && barre_ar_angle > -RUDDER_MAX) {
        barre_ar_angle -= RUDDER_STEP;
        item_rotate_with_center( barre_ar_item, barre_ar_angle,RUDDER_CENTER_X,RUDDER_CENTER_Y);
      }
			break;

    default:
      break;
    }
  return FALSE;
}
/* =====================================================================
 *		engine_event
 * =====================================================================*/
static gint engine_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
	int d = GPOINTER_TO_INT(data);

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
    	if (d == UP) {
        submarine_horizontal_speed += SPEED_STEP;
      }
    	if (d == DOWN) {
        submarine_horizontal_speed -= SPEED_STEP;
      }
      if (submarine_horizontal_speed > SPEED_MAX)
				submarine_horizontal_speed = SPEED_MAX;
    	if (submarine_horizontal_speed < 0)
      	submarine_horizontal_speed = 0;
			setSpeed(submarine_horizontal_speed);
			break;

    default:
      break;
    }
  return FALSE;
}
/* =====================================================================
 * Helper functions to update the graphical display
 * =====================================================================*/
static void setSpeed(double value) {
	char s12[12];
  sprintf(s12,"%d",(int)value);
	gnome_canvas_item_set(speed_item_back, "text", s12, NULL);
	gnome_canvas_item_set(speed_item_front, "text", s12, NULL);
}
static void setBattery(double value) {
	char s12[12];
  sprintf(s12,"%d",(int)value);
	gnome_canvas_item_set(battery_item_back, "text", s12, NULL);
	gnome_canvas_item_set(battery_item_front, "text", s12, NULL);
}
static void setAir(double value) {
	char s12[12];
  sprintf(s12,"%d",(int)value);
	gnome_canvas_item_set(air_item_back, "text", s12, NULL);
	gnome_canvas_item_set(air_item_front, "text", s12, NULL);
}
static void setRegleur(double value) {
	char s12[12];
  sprintf(s12,"%d",(int)value);
	gnome_canvas_item_set(regleur_item_back, "text", s12, NULL);
	gnome_canvas_item_set(regleur_item_front, "text", s12, NULL);
}
static void setAssiette(double value) {
	item_rotate(submarine_item, value);
}
