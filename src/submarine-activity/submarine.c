/* gcompris - submarine.c
 *
 * Copyright (C) 2003, 2008 Pascal Georges
 * Copyright (C) 2009, 2011 Bruno Coudoin
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

#include "gcompris/gcompris.h"

#define DEG_TO_RAD(x) M_PI*(x)/180.0

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
static int gamewon;

static void ok(void);
static void game_won();

#define PURGE_AR 215
#define PURGE_AV 428
#define REGLEUR 320
#define CHASSE_BALLAST_AR_X 214
#define CHASSE_BALLAST_AR_Y 97
#define CHASSE_BALLAST_AV_X 427
#define CHASSE_BALLAST_AV_Y 98
#define CHASSE_REGLEUR_X 321
#define CHASSE_REGLEUR_Y 72
#define BARRE_AR_X 80
#define BARRE_AR_Y 30
#define BARRE_AV_X 500
#define BARRE_AV_Y 90

#define BALLAST_AV_AIR_TEXT_X 440
#define BALLAST_AV_AIR_TEXT_Y 50
#define BALLAST_AV_AIR_X1 393
#define BALLAST_AV_AIR_Y1 20
#define BALLAST_AV_AIR_W 90
#define BALLAST_AV_AIR_H 60

#define BALLAST_AR_AIR_TEXT_X 220
#define BALLAST_AR_AIR_TEXT_Y 50
#define BALLAST_AR_AIR_X1 180
#define BALLAST_AR_AIR_Y1 20
#define BALLAST_AR_AIR_W 90
#define BALLAST_AR_AIR_H 60

#define SURFACE_IN_BACKGROUND 40
#define SURFACE_DEPTH 20.0
#define IP_DEPTH 13.0
#define SECURITY_DEPTH 55.0
#define MAX_DEPTH 250.0
#define SPEED_MAX 10
#define SPEED_STEP 1
#define AIR_INITIAL 30000
#define BATTERY_INITIAL 3000
#define MAX_BALLAST 10000
#define MAX_REGLEUR 800
#define REGLEUR_INITIAL 500.0
#define WEIGHT_INITIAL -300.0

#define SUBMARINE_INITIAL_X 100
#define WRAP_X BOARDWIDTH
#define SUBMARINE_INITIAL_DEPTH SURFACE_DEPTH

#define RUDDER_STEP 5
#define RUDDER_MAX 15
#define RUDDER_CENTER_X 68
#define RUDDER_CENTER_Y 8
#define ENGINE_DOWN_X 32
#define ENGINE_DOWN_Y 94
#define ENGINE_UP_X 114
#define ENGINE_UP_Y 94
#define AIR_X 328
#define AIR_Y 109
#define BATTERY_X 330
#define BATTERY_Y 156

#define REGLEUR_TEXT_X 330
#define REGLEUR_TEXT_Y 37
#define REGLEUR_X1 325
#define REGLEUR_Y1 18
#define REGLEUR_W  12
#define REGLEUR_H  34

#define AIR_TRIGGER_X 154
#define AIR_TRIGGER_Y 108
#define BATTERY_TRIGGER_X 184
#define BATTERY_TRIGGER_Y 108
#define TRIGGER_CENTER_X 7
#define TRIGGER_CENTER_Y 23
#define ALERT_SUBMARINE_X 719
#define ALERT_SUBMARINE_Y 368

#define UP 1
#define DOWN 0

#define WHALE_DETECTION_RADIUS 45.0
#define TREASURE_DETECTION_RADIUS 70.0

#define UPDATE_DELAY 200
#define UPDATE_DELAY_SLOW 300
#define UPDATE_DELAY_VERY_SLOW 1500

#define TEXT_COLOR_FRONT "red"
#define TEXT_COLOR_BACK "orange"

/* ================================================================ */
static GooCanvasItem *backgroundRootItem = NULL;
static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem *submarine_item,
  *ballast_av_purge_item, *ballast_ar_purge_item, *regleur_purge_item;
static GooCanvasItem *regleur_chasse_item;
gboolean ballast_av_purge_open, ballast_ar_purge_open, regleur_purge_open;
gboolean ballast_av_chasse_open, ballast_ar_chasse_open, regleur_chasse_open;
gboolean air_charging, battery_charging;
gboolean submarine_destroyed;
gboolean treasure_captured;

static GooCanvasItem *barre_av_item, *barre_ar_item,
  *speed_item_back, *speed_item_front,
  *air_item_back, *air_item_front,
  *regleur_item_back, *regleur_item_front,
  *regleur_item_rect,
  *battery_item_back, *battery_item_front,
  *ballast_av_air_item_back, *ballast_av_air_item_front,
  *ballast_ar_air_item_back, *ballast_ar_air_item_front,
  *ballast_av_air_item_rect, *ballast_ar_air_item_rect,
  *air_compressor_item, *battery_charger_item, *alert_submarine,
  *bubbling[3], *big_explosion, *whale, *treasure, *top_gate_item;

static GooCanvasItem *frigate_item = NULL;
/* submarine parameters */
static gdouble barre_av_angle, barre_ar_angle, depth, weight, resulting_weight;
static gdouble submarine_x, air, battery, regleur;
static gdouble submarine_horizontal_speed, submarine_vertical_speed;
static gdouble speed_ordered, assiette;
static gdouble ballast_av_air, ballast_ar_air;

static gdouble whale_x, whale_y;
static gdouble treasure_x, treasure_y;
static guint schema_x, schema_y;

/* Defines the right gate */
static guint gate_top_y, gate_bottom_y, gate_top_current_y;

/* updated from submarine.png */
static guint submarine_width;
static guint submarine_height;

static GooCanvasItem *submarine_drawbackground(GooCanvasItem *parent);
static GooCanvasItem *submarine_create_item(GooCanvasItem *parent);
static void submarine_destroy_all_items(void);
static void submarine_next_level(void);
static gboolean ballast_av_purge_event (GooCanvasItem  *item,
					GooCanvasItem  *target,
					GdkEventButton *event,
					gpointer data);
static gboolean ballast_ar_purge_event(GooCanvasItem *item, GooCanvas *target,
				       GdkEventButton *event, gpointer data);
static gboolean regleur_purge_event(GooCanvasItem *item, GooCanvas *target,
				GdkEventButton *event, gpointer data);
static gboolean ballast_ar_chasse_event(GooCanvasItem *item, GooCanvas *target,
				    GdkEventButton *event, gpointer data);
static gboolean ballast_av_chasse_event(GooCanvasItem *item, GooCanvas *target,
				    GdkEventButton *event, gpointer data);
static gboolean regleur_chasse_event(GooCanvasItem *item, GooCanvas *target,
				 GdkEventButton *event, gpointer data);
static gboolean barre_av_event(GooCanvasItem *item, GooCanvas *target,
			   GdkEventButton *event, gpointer data);
static gboolean barre_ar_event(GooCanvasItem *item, GooCanvas *target,
			   GdkEventButton *event, gpointer data);
static gboolean engine_event(GooCanvasItem *item, GooCanvas *target,
			 GdkEventButton *event, gpointer data);
static gboolean air_compressor_event(GooCanvasItem *item, GooCanvas *target,
				 GdkEventButton *event, gpointer data);
static gboolean battery_charger_event(GooCanvasItem *item, GooCanvas *target,
				  GdkEventButton *event, gpointer data);

static void start_frigate_anim(void);
static void stop_frigate_anim(void);
static void setSpeed(gdouble value);
static void setBattery(gdouble value);
static void setAir(gdouble value);
static void setRegleur(gdouble value);
static void setBallastAV(gdouble value);
static void setBallastAR(gdouble value);

static void submarine_explosion();
static void open_door();

static gboolean update_timeout();
static gboolean update_timeout_slow();
static gboolean update_timeout_very_slow();
static guint timer_id, timer_slow_id, timer_very_slow_id;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Submarine",
    "Control the depth of a submarine",
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
    NULL,
    set_level,
    NULL,
    NULL,
    NULL,
    NULL
  };

/* =====================================================================
 *
 * =====================================================================*/
GET_BPLUGIN_INFO(submarine)

/* =====================================================================
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 * =====================================================================*/
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if(!gamewon)
    {
      if(pause == TRUE)
	stop_frigate_anim();
      else
	start_frigate_anim();
    }

  board_paused = pause;
}

/* =====================================================================
 *
 * =====================================================================*/
static void start_board (GcomprisBoard *agcomprisBoard) {
  if(agcomprisBoard!=NULL) {
    gcomprisBoard=agcomprisBoard;
    gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
    		      "submarine/sub_bg.svgz");
    gcomprisBoard->level=1;
    gcomprisBoard->maxlevel=4;
    gcomprisBoard->sublevel=1;
    gc_bar_set(GC_BAR_LEVEL);
    gc_bar_location(0, -1, 0.6);
    backgroundRootItem =						\
	 submarine_drawbackground(goo_canvas_get_root_item(gcomprisBoard->canvas));
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
  gcomprisBoard = NULL;

  if(backgroundRootItem != NULL){
    goo_canvas_item_remove(backgroundRootItem);
  }
  backgroundRootItem = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static void set_level (guint level) {
  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      submarine_next_level();
    }
}

/* =====================================================================
 *
 * =====================================================================*/
static gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
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
static void submarine_next_level()
{
  gamewon = FALSE;
  ballast_av_purge_open = ballast_ar_purge_open = regleur_purge_open = FALSE;
  ballast_av_chasse_open = ballast_ar_chasse_open = regleur_chasse_open = FALSE;
  air_charging = battery_charging = FALSE;
  barre_av_angle = barre_ar_angle = 0.0;
  depth = SUBMARINE_INITIAL_DEPTH;
  submarine_horizontal_speed = speed_ordered = 0.0;
  submarine_x = SUBMARINE_INITIAL_X;
  weight = WEIGHT_INITIAL;
  regleur = REGLEUR_INITIAL;
  air = AIR_INITIAL;
  battery = BATTERY_INITIAL;
  ballast_av_air = ballast_ar_air = MAX_BALLAST/10.0;
  assiette = 0.0;
  submarine_destroyed = FALSE;
  treasure_captured = FALSE;

  submarine_destroy_all_items();

  /* Try the next level */
  boardRootItem = submarine_create_item(backgroundRootItem);
  start_frigate_anim();

  gc_bar_set_level(gcomprisBoard);

}
/* =====================================================================
 * Destroy all the items
 * =====================================================================*/
static void submarine_destroy_all_items() {
  /* kill pending timers */
  if(timer_id)
    g_source_remove(timer_id);
  timer_id = 0;

  if(timer_slow_id)
    g_source_remove(timer_slow_id);
  timer_slow_id = 0;

  if(timer_very_slow_id)
    g_source_remove(timer_very_slow_id);
  timer_very_slow_id = 0;

  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);
  boardRootItem = NULL;

  stop_frigate_anim();
}

static GooCanvasItem *submarine_drawbackground(GooCanvasItem *parent) {

  GooCanvasItem *backgroundRootItem = goo_canvas_group_new (parent, NULL);
  GooCanvasItem *item;
  GdkPixbuf *pixmap = NULL;
  int w, h;

  pixmap = gc_pixmap_load("submarine/sub_schema.svg");

  w = gdk_pixbuf_get_width(pixmap);
  h = gdk_pixbuf_get_height(pixmap);

  schema_x = (BOARDWIDTH - w)/2 ;
  schema_y = BOARDHEIGHT - h;
  goo_canvas_image_new (backgroundRootItem, pixmap,
  			schema_x, schema_y, NULL);

  gdk_pixbuf_unref(pixmap);

#define COMMAND_OFFSET 20.0
  pixmap = gc_pixmap_load("submarine/up.png");
  int w2 = gdk_pixbuf_get_width(pixmap);
  w = 80; // The rudder width
  item = goo_canvas_image_new (backgroundRootItem,
			       pixmap,
			       schema_x + BARRE_AV_X + w - w2,
			       schema_y + BARRE_AV_Y - COMMAND_OFFSET,
			       NULL);
  g_signal_connect(item, "button-press-event",
		     (GtkSignalFunc) barre_av_event, GINT_TO_POINTER(UP));

  item = goo_canvas_image_new (backgroundRootItem,
			       pixmap,
			       schema_x + BARRE_AR_X + w - w2,
			       schema_y + BARRE_AR_Y - COMMAND_OFFSET,
			       NULL);
  g_signal_connect(item, "button-press-event",
		   (GtkSignalFunc) barre_ar_event, GINT_TO_POINTER(UP));

  item = goo_canvas_image_new (backgroundRootItem,
			       pixmap,
			       schema_x + ENGINE_UP_X,
			       schema_y + ENGINE_UP_Y,
			       NULL);
  g_signal_connect(item, "button-press-event",
		   (GtkSignalFunc) engine_event, GINT_TO_POINTER(UP));

  gdk_pixbuf_unref(pixmap);

  pixmap = gc_pixmap_load("submarine/down.png");
  item = goo_canvas_image_new (backgroundRootItem,
			       pixmap,
			       schema_x + BARRE_AV_X + w - w2,
			       schema_y + BARRE_AV_Y + COMMAND_OFFSET,
			       NULL);
  g_signal_connect(item, "button-press-event",
		   (GtkSignalFunc) barre_av_event, GINT_TO_POINTER(DOWN));

  item = goo_canvas_image_new (backgroundRootItem,
			       pixmap,
			       schema_x + BARRE_AR_X + w - w2,
			       schema_y + BARRE_AR_Y + COMMAND_OFFSET,
			       NULL);
  g_signal_connect(item, "button-press-event",
		   (GtkSignalFunc) barre_ar_event, GINT_TO_POINTER(DOWN));

  item = goo_canvas_image_new (backgroundRootItem,
			       pixmap,
			       schema_x + ENGINE_DOWN_X,
			       schema_y + ENGINE_DOWN_Y,
			       NULL);
  g_signal_connect(item, "button-press-event",
		     (GtkSignalFunc) engine_event, GINT_TO_POINTER(DOWN));

  gdk_pixbuf_unref(pixmap);

  return backgroundRootItem;
}

/* =====================================================================
 *
 * =====================================================================*/
static GooCanvasItem *submarine_create_item(GooCanvasItem *parent) {
  GdkPixbuf *pixmap = NULL;
  GooCanvasItem *item = NULL;
  char s12[12];
  int i, w, h;

  GooCanvasItem *rootItem = goo_canvas_group_new (parent, NULL);

  pixmap = gc_pixmap_load("submarine/submarine.png");
  submarine_width = gdk_pixbuf_get_width(pixmap);
  submarine_height = gdk_pixbuf_get_height(pixmap);
  submarine_item = goo_canvas_image_new (rootItem,
					 pixmap,
					 0,
					 0,
					 NULL);

  gdk_pixbuf_unref(pixmap);


  pixmap = gc_pixmap_load("submarine/vanne.svg");
  ballast_ar_purge_item = goo_canvas_image_new (rootItem,
						pixmap,
						PURGE_AR + schema_x,
						schema_y -1.0,
						 NULL);
  g_signal_connect(ballast_ar_purge_item, "button-press-event",
		   (GtkSignalFunc) ballast_ar_purge_event, NULL);

  ballast_av_purge_item = goo_canvas_image_new (rootItem,
						pixmap,
						PURGE_AV + schema_x,
						schema_y -1.0,
						NULL);
  g_signal_connect(ballast_av_purge_item, "button-press-event",
		   (GtkSignalFunc) ballast_av_purge_event, NULL);

  regleur_purge_item = goo_canvas_image_new (rootItem,
					     pixmap,
					     REGLEUR + schema_x,
					     schema_y -2.0,
					     NULL);
  g_signal_connect(regleur_purge_item, "button-press-event",
		   (GtkSignalFunc) regleur_purge_event, NULL);

  item = goo_canvas_image_new (rootItem,
			       pixmap,
			       schema_x + CHASSE_BALLAST_AV_X,
			       schema_y +  CHASSE_BALLAST_AV_Y,
			       NULL);
  g_signal_connect(item, "button-press-event",
		   (GtkSignalFunc) ballast_av_chasse_event, NULL);

  item = goo_canvas_image_new (rootItem,
			       pixmap,
			       schema_x + CHASSE_BALLAST_AR_X,
			       schema_y +  CHASSE_BALLAST_AR_Y,
			       NULL);
  g_signal_connect(item, "button-press-event",
		   (GtkSignalFunc) ballast_ar_chasse_event, NULL);

  regleur_chasse_item = goo_canvas_image_new (rootItem,
					      pixmap,
					      schema_x + CHASSE_REGLEUR_X,
					      schema_y + CHASSE_REGLEUR_Y,
					      NULL);
  g_signal_connect(regleur_chasse_item, "button-press-event",
		   (GtkSignalFunc) regleur_chasse_event, NULL);

  gdk_pixbuf_unref(pixmap);

  // DEPTH RUDDERS
  pixmap = gc_pixmap_load("submarine/rudder.png");
  w = gdk_pixbuf_get_width(pixmap);
  h = gdk_pixbuf_get_height(pixmap);
  barre_av_item = goo_canvas_image_new (rootItem,
					pixmap,
					schema_x + BARRE_AV_X,
					schema_y + BARRE_AV_Y,
					 NULL);
  barre_ar_item = goo_canvas_image_new (rootItem,
					pixmap,
					schema_x + BARRE_AR_X,
					schema_y + BARRE_AR_Y,
					 NULL);
  gdk_pixbuf_unref(pixmap);

  // displays the speed on the engine
  sprintf(s12,"%d",(int)submarine_horizontal_speed);
  speed_item_back = goo_canvas_text_new (rootItem,
					 s12,
					 (gdouble) schema_x + ENGINE_UP_X - ENGINE_DOWN_X +1,
					 (gdouble) schema_y + ENGINE_UP_Y + 10 + 1,
					 -1,
					 GTK_ANCHOR_CENTER,
					 "font", gc_skin_font_board_title_bold,
					 "alignment", PANGO_ALIGN_CENTER,
					 "fill-color", TEXT_COLOR_BACK,
					 NULL);
  speed_item_front = goo_canvas_text_new (rootItem,
					  s12,
					  (gdouble) schema_x + ENGINE_UP_X - ENGINE_DOWN_X,
					  (gdouble) schema_y + ENGINE_UP_Y + 10,
					  -1,
					  GTK_ANCHOR_CENTER,
					  "font", gc_skin_font_board_title_bold,
					  "alignment", PANGO_ALIGN_CENTER,
					  "fill-color", TEXT_COLOR_FRONT,
					  NULL);

  // displays the ballast_av_air value
  ballast_av_air_item_rect = goo_canvas_rect_new (rootItem,
						  schema_x + BALLAST_AV_AIR_X1,
						  schema_y + BALLAST_AV_AIR_Y1
						  + BALLAST_AV_AIR_H,
						  BALLAST_AV_AIR_W,
						  BALLAST_AV_AIR_H,
						  "fill-color", "blue",
						  "line-width", 0.0,
						  NULL);

  sprintf(s12,"%d",(int)ballast_av_air);
  ballast_av_air_item_back = goo_canvas_text_new (rootItem,
						  s12,
						  (gdouble) schema_x + BALLAST_AV_AIR_TEXT_X + 1,
						  (gdouble) schema_y + BALLAST_AV_AIR_TEXT_Y + 1,
						  -1,
						  GTK_ANCHOR_CENTER,
						  "font", gc_skin_font_board_title_bold,
						  "fill-color", TEXT_COLOR_BACK,
						  NULL);
  ballast_av_air_item_front = goo_canvas_text_new (rootItem,
						   s12,
						   (gdouble) schema_x + BALLAST_AV_AIR_TEXT_X,
						   (gdouble) schema_y + BALLAST_AV_AIR_TEXT_Y,
						   -1,
						   GTK_ANCHOR_CENTER,
						   "font", gc_skin_font_board_title_bold,
						   "fill-color", TEXT_COLOR_FRONT,
						   NULL);
  setBallastAV(ballast_av_air);

  // displays the ballast_ar_air value
  ballast_ar_air_item_rect = goo_canvas_rect_new (rootItem,
						  schema_x + BALLAST_AR_AIR_X1,
						  schema_y + BALLAST_AR_AIR_Y1
						  + BALLAST_AR_AIR_H,
						  BALLAST_AR_AIR_W,
						  BALLAST_AR_AIR_H,
						  "fill-color", "blue",
						  "line-width", 0.0,
						  NULL);

  sprintf(s12,"%d",(int)ballast_ar_air);
  ballast_ar_air_item_back = goo_canvas_text_new (rootItem,
						  s12,
						  (gdouble) schema_x + BALLAST_AR_AIR_TEXT_X + 1,
						  (gdouble) schema_y + BALLAST_AR_AIR_TEXT_Y + 1,
						  -1,
						  GTK_ANCHOR_CENTER,
						  "font", gc_skin_font_board_title_bold,
						  "fill-color", TEXT_COLOR_BACK,
						  NULL);
  ballast_ar_air_item_front = goo_canvas_text_new (rootItem,
						   s12,
						   (gdouble) schema_x + BALLAST_AR_AIR_TEXT_X,
						   (gdouble) schema_y + BALLAST_AR_AIR_TEXT_Y,
						   -1,
						   GTK_ANCHOR_CENTER,
						   "font", gc_skin_font_board_title_bold,
						   "fill-color", TEXT_COLOR_FRONT,
						   NULL);
  setBallastAR(ballast_ar_air);

  // displays the remaining air value
  sprintf(s12,"%d", (int)air);
  air_item_back = goo_canvas_text_new (rootItem,
				       s12,
				       (gdouble) schema_x + AIR_X +1,
				       (gdouble) schema_y + AIR_Y + 1,
				       -1,
				       GTK_ANCHOR_CENTER,
				       "font", gc_skin_font_board_title_bold,
				       "fill-color", TEXT_COLOR_BACK,
				       NULL);
  air_item_front = goo_canvas_text_new (rootItem,
					s12,
					(gdouble) schema_x + AIR_X,
					(gdouble) schema_y + AIR_Y,
					-1,
					GTK_ANCHOR_CENTER,
					"font", gc_skin_font_board_title_bold,
					"fill-color", TEXT_COLOR_FRONT,
					NULL);

  // displays the remaining battery value
  sprintf(s12,"%d", (int)battery);
  battery_item_back = goo_canvas_text_new (rootItem,
					   s12,
					   (gdouble) schema_x + BATTERY_X +1,
					   (gdouble) schema_y + BATTERY_Y + 1,
					   -1,
					   GTK_ANCHOR_CENTER,
					   "font", gc_skin_font_board_title_bold,
					   "alignment", PANGO_ALIGN_CENTER,
					   "fill-color", TEXT_COLOR_BACK,
					   NULL);
  battery_item_front = goo_canvas_text_new (rootItem,
					    s12,
					    (gdouble) schema_x + BATTERY_X,
					    (gdouble) schema_y + BATTERY_Y,
					    -1,
					    GTK_ANCHOR_CENTER,
					    "font", gc_skin_font_board_title_bold,
					    "alignment", PANGO_ALIGN_CENTER,
					    "fill-color", TEXT_COLOR_FRONT,
					    NULL);

  // displays the remaining regleur value
  regleur_item_rect = goo_canvas_rect_new (rootItem,
					   schema_x + REGLEUR_X1,
					   schema_y + REGLEUR_Y1 + REGLEUR_H,
					   REGLEUR_W,
					   REGLEUR_H,
					   "fill-color", "blue",
					   "line-width", 0.0,
					   NULL);

  sprintf(s12,"%d", (int)regleur);
  regleur_item_back = goo_canvas_text_new (rootItem,
					   s12,
					   (gdouble) schema_x + REGLEUR_TEXT_X +1,
					   (gdouble) schema_y + REGLEUR_TEXT_Y + 1,
					   -1,
					   GTK_ANCHOR_CENTER,
					   "font", gc_skin_font_board_title_bold,
					   "fill-color", TEXT_COLOR_BACK,
					   NULL);
  regleur_item_front = goo_canvas_text_new (rootItem,
					    s12,
					    (gdouble) schema_x + REGLEUR_TEXT_X,
					    (gdouble) schema_y + REGLEUR_TEXT_Y,
					    -1,
					    GTK_ANCHOR_CENTER,
					    "font", gc_skin_font_board_title_bold,
					    "fill-color", TEXT_COLOR_FRONT,
					    NULL);
  setRegleur(regleur);

  // displays an alert when some parameters are bad
  pixmap = gc_pixmap_load("submarine/alert_submarine.png");
  w = gdk_pixbuf_get_width(pixmap);
  h = gdk_pixbuf_get_height(pixmap);
  alert_submarine = goo_canvas_image_new (rootItem,
					  pixmap,
					  ALERT_SUBMARINE_X,
					  ALERT_SUBMARINE_Y,
					  NULL);
  gdk_pixbuf_unref(pixmap);
  g_object_set (alert_submarine,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);

  // when the submarine makes some bubbles ...
  pixmap = gc_pixmap_load("submarine/bubbling.png");

  for (i=0; i<3; i++) {
    bubbling[i] = goo_canvas_image_new (rootItem,
					pixmap,
					0,
					0,
					NULL);
    g_object_set (bubbling[i],
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);
  }
  gdk_pixbuf_unref(pixmap);

  // whale item
  switch(gcomprisBoard->level)
    {
    case 1:
      whale_x = 50;
      whale_y = MAX_DEPTH - 100;
      break;
    case 2:
      whale_x = 150;
      whale_y = MAX_DEPTH - 100;
      break;
    case 3:
      whale_x = 250;
      whale_y = MAX_DEPTH - 100;
      break;
    default:
      whale_x = 380;
      whale_y = MAX_DEPTH - 100;
      break;
    }

  pixmap = gc_pixmap_load("submarine/whale.png");
  whale = goo_canvas_image_new (rootItem,
				pixmap,
				whale_x,
				whale_y,
				NULL);
  gdk_pixbuf_unref(pixmap);

  //  whale being hit
  pixmap = gc_pixmap_load("submarine/whale_hit.png");
  big_explosion = goo_canvas_image_new (rootItem,
					pixmap,
					whale_x,
					whale_y,
					NULL);
  g_object_set (big_explosion,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);

  gdk_pixbuf_unref(pixmap);

  // treasure item
  pixmap = gc_pixmap_load("submarine/crown.png");
  treasure_x = (BOARDWIDTH*3)/4;
  treasure_y = MAX_DEPTH;
  treasure = goo_canvas_image_new (rootItem,
				   pixmap,
				   0,
				   0,
				   NULL);
  goo_canvas_item_translate(treasure, treasure_x, 0);
  goo_canvas_item_animate(treasure,
			  treasure_x,
			  treasure_y,
			  0.6,
			  0,
			  TRUE,
			  6*1000,
			  40,
			  GOO_CANVAS_ANIMATE_FREEZE);
  gdk_pixbuf_unref(pixmap);

  // the triggers for air compressor and battery charger
  pixmap = gc_pixmap_load("submarine/manette.png");
  air_compressor_item = goo_canvas_image_new (rootItem,
					      pixmap,
					      schema_x + AIR_TRIGGER_X,
					      schema_y + AIR_TRIGGER_Y,
					      NULL);
  battery_charger_item = goo_canvas_image_new (rootItem,
					       pixmap,
					       schema_x + BATTERY_TRIGGER_X,
					       schema_y + BATTERY_TRIGGER_Y,
					       NULL);
  gdk_pixbuf_unref(pixmap);

  g_signal_connect(air_compressor_item, "button-press-event",
		   (GtkSignalFunc) air_compressor_event, NULL);
  g_signal_connect(battery_charger_item, "button-press-event",
		   (GtkSignalFunc) battery_charger_event, NULL);

  /*
   * Set the right wall
   * ------------------
   */

  switch(gcomprisBoard->level)
    {
    case 1:
      gate_top_y = 80;
      gate_bottom_y = schema_y - 100;
      break;
    case 2:
      gate_top_y = 100;
      gate_bottom_y = schema_y - 120;
      break;
    default:
      gate_top_y = 120;
      gate_bottom_y = schema_y - 120;
      break;
    }

  /* At startup, the gate is closed */
  gate_top_current_y = gate_bottom_y;
  top_gate_item = goo_canvas_rect_new (rootItem,
				       BOARDWIDTH - 25,
				       40,
				       27,
				       gate_top_current_y - 40,
				       "fill_color_rgba", 0x989677FF,
				       "stroke-color", "black",
				       "line-width", 2.0,
				       NULL);

  goo_canvas_rect_new (rootItem,
		       BOARDWIDTH - 25,
		       gate_bottom_y,
		       27,
		       schema_y - gate_bottom_y,
		       "fill_color_rgba", 0x989677FF,
		       "stroke-color", "black",
		       "line-width", 2.0,
		       NULL);


  timer_id = g_timeout_add(UPDATE_DELAY, update_timeout, NULL);
  timer_slow_id = g_timeout_add(UPDATE_DELAY_SLOW, update_timeout_slow, NULL);
  timer_very_slow_id = g_timeout_add(UPDATE_DELAY_VERY_SLOW, update_timeout_very_slow, NULL);

  return rootItem;
}

static void start_frigate_anim()
{
  GdkPixbuf *pixmap;
  int w;
  if (frigate_item)
    stop_frigate_anim();

  // the antisubmarine warfare frigate
  pixmap = gc_pixmap_load("submarine/asw_frigate.png");
  w = gdk_pixbuf_get_width(pixmap);

  frigate_item = goo_canvas_image_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
				       pixmap,
				       0,
				       0,
				       NULL);
  goo_canvas_item_translate(frigate_item, BOARDWIDTH, 6);
  goo_canvas_item_animate(frigate_item,
			  -w,
			  2.0,
			  1,
			  0,
			  TRUE,
			  100*1000,
			  100,
			  GOO_CANVAS_ANIMATE_RESTART);
  gdk_pixbuf_unref(pixmap);
}

static void stop_frigate_anim()
{
  if (frigate_item)
    {
      goo_canvas_item_stop_animation (frigate_item);
      goo_canvas_item_remove(frigate_item);
    }
  frigate_item = NULL;
}

/* =====================================================================
 * Periodically recalculate the submarine parameters
 * =====================================================================*/
static gboolean update_timeout() {
  gdouble delta_air;
  gboolean regleur_dirty = FALSE;
  gboolean air_dirty = FALSE;

  if(!boardRootItem)
    return FALSE;

  if(board_paused)
    return TRUE;

  /* air in ballasts */
  if (ballast_av_purge_open) {
    ballast_av_air -= UPDATE_DELAY/1000.0 *500.0; // 500 liters go out per second
    if (ballast_av_air < 0.0)
      ballast_av_air = 0.0;
    setBallastAV(ballast_av_air);
  }
  if (ballast_ar_purge_open) {
    ballast_ar_air -= UPDATE_DELAY/1000.0 *500.0;
    if (ballast_ar_air < 0.0)
      ballast_ar_air = 0.0;
    setBallastAR(ballast_ar_air);
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
    setBallastAV(ballast_av_air);
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
    setBallastAR(ballast_ar_air);
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
  gdouble delta_assiette;

  if(!boardRootItem)
    return FALSE;

  if(board_paused)
    return TRUE;

  /* speed : don't reach instantly the ordered speed */
  if (speed_ordered != submarine_horizontal_speed) {
    submarine_horizontal_speed += (speed_ordered-submarine_horizontal_speed)/10.0;
    if (fabs(speed_ordered - submarine_horizontal_speed) < 0.1)
      submarine_horizontal_speed = speed_ordered;
  }

  /* assiette */
  delta_assiette = (ballast_ar_air - ballast_av_air)/200.0 +
    (barre_av_angle - barre_ar_angle)/5.0*submarine_horizontal_speed;
  assiette -= delta_assiette*UPDATE_DELAY/10000.0;
  if (assiette < -30.0)
    assiette = -30.0;
  if (assiette > 30.0)
    assiette = 30.0;

  /* If surfacing, diminish the 'assiette' */
  if ( depth <= 5.0 + SURFACE_DEPTH) {
    assiette *= depth/(depth+1.0);
  }

  /* update some dynamic parameters */
  /* resulting_weight > 0 ==> the sub goes deeper
     regleur : this is the qty of water */
  resulting_weight = weight - ballast_av_air - ballast_ar_air + regleur;
  submarine_vertical_speed = resulting_weight/300.0
    + submarine_horizontal_speed*sin(DEG_TO_RAD(-assiette));

  /* if depth rudders are in the same direction */
  if (barre_ar_angle != 0.0 && barre_av_angle != 0.0) {
    if (fabs(barre_ar_angle)/barre_ar_angle == fabs(barre_av_angle)/barre_av_angle) {
      gdouble a = (fabs(barre_ar_angle) > fabs(barre_av_angle)) ? barre_av_angle : barre_ar_angle;
      submarine_vertical_speed += a * submarine_horizontal_speed/30.0;
    }
  }

  /* position & depth */
  submarine_x += submarine_horizontal_speed
    * cos(DEG_TO_RAD(assiette))
    * UPDATE_DELAY_SLOW/1000.0;

  depth += submarine_vertical_speed * UPDATE_DELAY_SLOW/1000.0;

  if (depth < SURFACE_DEPTH)
    depth = SURFACE_DEPTH;
  if (depth > MAX_DEPTH)
    depth = MAX_DEPTH;

  // show an alert if some parameters reach the limit
  if (depth >= MAX_DEPTH-20.0 || assiette == -30.0
      || assiette == 30.0 || air == 0.0 || battery == 0.0)
    g_object_set (alert_submarine,
		  "visibility", GOO_CANVAS_ITEM_VISIBLE,
		  NULL);
  else
    g_object_set (alert_submarine,
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);

  /* if the submarine dives, stop charging air tanks and batteries */
  if ( depth >= SURFACE_DEPTH+10.0 ) {
    if (air_charging) {
      air_charging = FALSE;
      gc_item_rotate_with_center(air_compressor_item, 0,
				 TRIGGER_CENTER_X, TRIGGER_CENTER_Y );
    }
    if (battery_charging) {
      battery_charging = FALSE;
      gc_item_rotate_with_center(battery_charger_item, 0,
				 TRIGGER_CENTER_X, TRIGGER_CENTER_Y );
    }
  }

  /* Check the submarine passed the right door */
  if ( submarine_x > WRAP_X && !gamewon )
    {
      /* Check its within the gate range */
      GooCanvasBounds bounds;

      goo_canvas_item_get_bounds (submarine_item, &bounds);

      guint antena_height = 30;
      if(bounds.y1 + antena_height < gate_top_current_y ||
	 bounds.y2 > gate_bottom_y)
	{
	  /* It's a crash */
	  submarine_explosion();
	}
      else
	{
	  gamewon = TRUE;
	  ok();
	}
    }

  /* Open the door */
  if(treasure_captured && gate_top_current_y > gate_top_y)
    open_door();

  /* display the submarine */
  goo_canvas_item_set_simple_transform(submarine_item,
				       submarine_x - submarine_width/2.0,
				       depth + SURFACE_DEPTH - submarine_height/2.0,
				       1, -assiette);

  /* the frigate */
  if ( frigate_item ) {
    /* detects a collision between the frigate and the submarine */
    if (depth <= 30.0 && !submarine_destroyed) {
      GooCanvasBounds bounds;
      goo_canvas_item_get_bounds(frigate_item, &bounds);
      gdouble frigate_x = bounds.x1 + (bounds.x2 - bounds.x1) / 2;
      if ( abs(submarine_x - frigate_x) < 100 ) {
        submarine_explosion();
	return TRUE;
      }
    }
  }

  /* whale detection */
  {
    gdouble dist1 = hypot( submarine_x - whale_x,  depth - whale_y);
    if ( ( dist1 < WHALE_DETECTION_RADIUS ) && !submarine_destroyed ) {
      g_object_set (whale, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      g_object_set (big_explosion, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
      submarine_explosion();
      return TRUE;
    }
  }

  /* treasure detection */
  {
    gdouble dist1 = hypot( submarine_x - treasure_x, depth - treasure_y);
    if ( (dist1 < TREASURE_DETECTION_RADIUS)
	 && !treasure_captured ) {
      gc_sound_play_ogg("sounds/tuxok.wav", NULL);
      g_object_set (treasure, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      treasure_captured = TRUE;
      open_door();
    }
  }

  return TRUE;
}
/* =====================================================================
 * Periodically recalculate some submarine parameters, with a slow delay
 * =====================================================================*/
static gboolean update_timeout_very_slow() {
  /* charging */

  if(!boardRootItem)
    return FALSE;

  if(board_paused)
    return TRUE;

  if (air_charging && depth < SURFACE_DEPTH+5.0) {
    air += 100.0*UPDATE_DELAY_VERY_SLOW/1000.0;
    setAir(air);
  }

  if (battery_charging && depth < SURFACE_DEPTH+5.0) {
    if (battery < 0.3*battery)
      battery += 500.0*UPDATE_DELAY_VERY_SLOW/1000.0;
    else
      if (battery < 0.6*battery)
	battery += 200.0*UPDATE_DELAY_VERY_SLOW/1000.0;
      else
	if (battery < 0.8*battery)
	  battery += 100.0*UPDATE_DELAY_VERY_SLOW/1000.0;
	else
	  battery += 50.0*UPDATE_DELAY_VERY_SLOW/1000.0;
  }

  /* battery */
  battery -= submarine_horizontal_speed * submarine_horizontal_speed/3.0
    * UPDATE_DELAY_VERY_SLOW/1000.0;

  if (battery < 0.0) {
    battery = 0.0;
    speed_ordered = 0;
    setSpeed(speed_ordered);
  }

  setBattery( battery );

  /* bubbling */
  if ( (ballast_av_purge_open && ballast_av_air > 0.0) ||
       ( ballast_av_chasse_open && ballast_av_air == MAX_BALLAST ) )
    {
      gc_item_absolute_move( bubbling[0],
			     submarine_x + submarine_width/2,
			     depth - 30.0);
      g_object_set ( bubbling[0] , "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
      gc_sound_play_ogg ("sounds/bubble.wav", NULL);
    }
  else
    g_object_set ( bubbling[0] , "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

  if ( (ballast_ar_purge_open && ballast_ar_air > 0.0) ||
       ( ballast_ar_chasse_open && ballast_ar_air == MAX_BALLAST ) )
    {
      gc_item_absolute_move( bubbling[2],
			     submarine_x - submarine_width/2,
			     depth - 30.0);
      gc_sound_play_ogg ("sounds/bubble.wav", NULL);
      g_object_set ( bubbling[2] , "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
    }
  else
    g_object_set ( bubbling[2] , "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

  if (regleur_purge_open && regleur < MAX_REGLEUR)
    {
      gc_item_absolute_move( bubbling[1],
			     submarine_x,
			     depth - 30.0);
      gc_sound_play_ogg ("sounds/bubble.wav", NULL);
      g_object_set ( bubbling[1] , "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
    }
  else
    g_object_set ( bubbling[1] , "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

  return TRUE;
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
    if(gcomprisBoard->level>gcomprisBoard->maxlevel)
      gcomprisBoard->level = gcomprisBoard->maxlevel;
  }
  submarine_next_level();
}

/* =====================================================================
 *
 * =====================================================================*/
static gboolean quit_after_delay() {

  if(!boardRootItem)
    return FALSE;

  if(board_paused)
    return TRUE;

  if (gamewon)
    game_won();
  else
    submarine_next_level();

  gc_bar_hide(FALSE);
  return FALSE;
}

static gboolean ok_timeout() {
  gc_bonus_display(gamewon, GC_BONUS_SMILEY);
  g_timeout_add(TIME_CLICK_TO_BONUS*5, quit_after_delay, NULL);
  return FALSE;
}

static void ok() {
  gc_bar_hide(TRUE);
  // leave time to display the right answer
  g_timeout_add(TIME_CLICK_TO_BONUS, ok_timeout, NULL);
}

/* =====================================================================
 *		ballast_av_purge_event
 * =====================================================================*/
static gboolean
ballast_av_purge_event (GooCanvasItem  *item,
			GooCanvasItem  *target,
			GdkEventButton *event,
			gpointer data)
{
  GooCanvasBounds bounds;

  if(board_paused || !boardRootItem)
    return FALSE;

  goo_canvas_item_get_bounds(item, &bounds);

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  ballast_av_purge_open = !ballast_av_purge_open;

  goo_canvas_item_rotate(item, 90.0,
			 bounds.x1 + (bounds.x2 - bounds.x1) / 2,
			 bounds.y1 + (bounds.y2 - bounds.y1) / 2);


  return FALSE;
}

/* =====================================================================
 *		ballast_ar_purge_event
 * =====================================================================*/
static gboolean
ballast_ar_purge_event(GooCanvasItem *item, GooCanvas *target,
		       GdkEventButton *event, gpointer data)
{
  GooCanvasBounds bounds;

  if(board_paused || !boardRootItem)
    return FALSE;

  goo_canvas_item_get_bounds(item, &bounds);

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  ballast_ar_purge_open = !ballast_ar_purge_open;

  goo_canvas_item_rotate(item, 90.0,
			 bounds.x1 + (bounds.x2 - bounds.x1) / 2,
			 bounds.y1 + (bounds.y2 - bounds.y1) / 2);

  return FALSE;
}
/* =====================================================================
 *		regleur_purge_event
 * =====================================================================*/
static gboolean
regleur_purge_event(GooCanvasItem *item, GooCanvas *target,
		    GdkEventButton *event, gpointer data)
{
  GooCanvasBounds bounds;

  if(board_paused || !boardRootItem)
    return FALSE;

  goo_canvas_item_get_bounds(item, &bounds);

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  regleur_purge_open = !regleur_purge_open;

  goo_canvas_item_rotate(item, 90.0,
			 bounds.x1 + (bounds.x2 - bounds.x1) / 2,
			 bounds.y1 + (bounds.y2 - bounds.y1) / 2);

  return FALSE;
}
/* =====================================================================
 *		ballast_ar_chasse_event
 * =====================================================================*/
static gboolean
ballast_ar_chasse_event(GooCanvasItem *item, GooCanvas *target,
			GdkEventButton *event, gpointer data)
{
  GooCanvasBounds bounds;

  if(board_paused || !boardRootItem)
    return FALSE;

  goo_canvas_item_get_bounds(item, &bounds);

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  ballast_ar_chasse_open = !ballast_ar_chasse_open;

  goo_canvas_item_rotate(item, 90.0,
			 bounds.x1 + (bounds.x2 - bounds.x1) / 2,
			 bounds.y1 + (bounds.y2 - bounds.y1) / 2);

  return FALSE;
}
/* =====================================================================
 *		ballast_av_chasse_event
 * =====================================================================*/
static gboolean
ballast_av_chasse_event(GooCanvasItem *item, GooCanvas *target,
			GdkEventButton *event, gpointer data)
{
  GooCanvasBounds bounds;

  if(board_paused || !boardRootItem)
    return FALSE;

  goo_canvas_item_get_bounds(item, &bounds);

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  ballast_av_chasse_open = !ballast_av_chasse_open;

  goo_canvas_item_rotate(item, 90.0,
			 bounds.x1 + (bounds.x2 - bounds.x1) / 2,
			 bounds.y1 + (bounds.y2 - bounds.y1) / 2);


  return FALSE;
}
/* =====================================================================
 *
 * =====================================================================*/
static gboolean
regleur_chasse_event(GooCanvasItem *item, GooCanvas *target,
		     GdkEventButton *event, gpointer data)
{
  GooCanvasBounds bounds;

  if(board_paused || !boardRootItem)
    return FALSE;

  goo_canvas_item_get_bounds(item, &bounds);

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  regleur_chasse_open = !regleur_chasse_open;

  goo_canvas_item_rotate(item, 90.0,
			 bounds.x1 + (bounds.x2 - bounds.x1) / 2,
			 bounds.y1 + (bounds.y2 - bounds.y1) / 2);

  return FALSE;
}

/* =====================================================================
 *		barre_av_event
 * =====================================================================*/
static gboolean
barre_av_event(GooCanvasItem *item, GooCanvas *target,
	       GdkEventButton *event, gpointer data)
{

  int d = GPOINTER_TO_INT(data);

  if(board_paused || !boardRootItem)
    return FALSE;

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  if (d == UP && barre_av_angle < RUDDER_MAX) {
    barre_av_angle += RUDDER_STEP;
    gc_item_rotate_relative_with_center( barre_av_item, RUDDER_STEP,
					 RUDDER_CENTER_X, RUDDER_CENTER_Y);
  }
  if (d == DOWN && barre_av_angle > -RUDDER_MAX) {
    barre_av_angle -= RUDDER_STEP;
    gc_item_rotate_relative_with_center( barre_av_item, -RUDDER_STEP,
					 RUDDER_CENTER_X, RUDDER_CENTER_Y);
  }

  return FALSE;
}
/* =====================================================================
 *		barre_ar_event
 * =====================================================================*/
static gboolean
barre_ar_event(GooCanvasItem *item, GooCanvas *target,
	       GdkEventButton *event, gpointer data)
{
  int d = GPOINTER_TO_INT(data);

  if(board_paused || !boardRootItem)
    return FALSE;

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  if (d == UP && barre_ar_angle < RUDDER_MAX) {
    barre_ar_angle += RUDDER_STEP;
    gc_item_rotate_relative_with_center(barre_ar_item, RUDDER_STEP,
					RUDDER_CENTER_X, RUDDER_CENTER_Y);
  }
  if (d == DOWN && barre_ar_angle > -RUDDER_MAX) {
    barre_ar_angle -= RUDDER_STEP;
    gc_item_rotate_relative_with_center( barre_ar_item, -RUDDER_STEP,
					 RUDDER_CENTER_X, RUDDER_CENTER_Y);
  }

  return FALSE;
}
/* =====================================================================
 *		engine_event
 * =====================================================================*/
static gboolean
engine_event(GooCanvasItem *item, GooCanvas *target,
	     GdkEventButton *event, gpointer data)
{
  int d = GPOINTER_TO_INT(data);

  if(board_paused || !boardRootItem)
    return FALSE;

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  if (d == UP) {
    speed_ordered += SPEED_STEP;
  }
  if (d == DOWN) {
    speed_ordered -= SPEED_STEP;
  }
  if (speed_ordered > SPEED_MAX)
    speed_ordered = SPEED_MAX;
  if (speed_ordered < 0)
    speed_ordered = 0;
  setSpeed(speed_ordered);

  return FALSE;
}
/* =====================================================================
 *		air_compressor_event
 * =====================================================================*/
static gboolean
air_compressor_event(GooCanvasItem *item, GooCanvas *target,
		     GdkEventButton *event, gpointer data)
{

  if(board_paused || !boardRootItem)
    return FALSE;

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  if (air_charging)
    air_charging = FALSE;
  else
    air_charging = TRUE;

  gc_item_rotate_relative_with_center(item, air_charging ? 180 : 0 ,
				      TRIGGER_CENTER_X, TRIGGER_CENTER_Y );

  return FALSE;
}
/* =====================================================================
 *		battery_charger_event
 * =====================================================================*/
static gboolean
battery_charger_event(GooCanvasItem *item, GooCanvas *target,
		      GdkEventButton *event, gpointer data)
{

  if(board_paused || !boardRootItem)
    return FALSE;

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  if (battery_charging)
    battery_charging = FALSE;
  else
    battery_charging = TRUE;

  gc_item_rotate_with_center(item, battery_charging ? 180 : 0 , TRIGGER_CENTER_X, TRIGGER_CENTER_Y );

  return FALSE;
}
/* =====================================================================
 * Helper functions to update the graphical display
 * =====================================================================*/
static void setSpeed(gdouble value) {
  char s12[12];
  sprintf(s12,"%d",(int)value);
  g_object_set(speed_item_back, "text", s12, NULL);
  g_object_set(speed_item_front, "text", s12, NULL);
}

static void setBattery(gdouble value) {
  char s12[12];
  sprintf(s12,"%d",(int)value);
  g_object_set(battery_item_back, "text", s12, NULL);
  g_object_set(battery_item_front, "text", s12, NULL);
}

static void setAir(gdouble value) {
  char s12[12];
  sprintf(s12,"%d",(int)value);
  g_object_set(air_item_back, "text", s12, NULL);
  g_object_set(air_item_front, "text", s12, NULL);
}

/* Value is the water level in the small tank */
static void setRegleur(gdouble value) {
  char s12[12];
  gdouble height, y_new;

  sprintf(s12,"%d", (int)value);
  g_object_set(regleur_item_back, "text", s12, NULL);
  g_object_set(regleur_item_front, "text", s12, NULL);

  y_new = \
    schema_y + REGLEUR_Y1 + REGLEUR_H +
    ( value * REGLEUR_H * -1) / MAX_REGLEUR;

  height = \
    schema_y + REGLEUR_Y1 + REGLEUR_H - y_new;

  g_object_set(regleur_item_rect,
	       "y", y_new,
	       "height", height,
	       NULL);
}

static void setBallastAV(gdouble value) {
  char s12[12];
  gdouble height, y_new;

  sprintf(s12,"%d", MAX_BALLAST - (int)value);
  g_object_set(ballast_av_air_item_back, "text", s12, NULL);
  g_object_set(ballast_av_air_item_front, "text", s12, NULL);

  y_new = \
    schema_y + BALLAST_AV_AIR_Y1 + BALLAST_AV_AIR_H +
    ( (MAX_BALLAST - value) * BALLAST_AV_AIR_H * -1) / MAX_BALLAST;

  height = \
    schema_y + BALLAST_AV_AIR_Y1 + BALLAST_AV_AIR_H - y_new;

  g_object_set(ballast_av_air_item_rect,
	       "y", y_new,
	       "height", height,
	       NULL);
}

static void setBallastAR(gdouble value) {
  char s12[12];
  gdouble height, y_new;

  sprintf(s12,"%d", MAX_BALLAST - (int)value);
  g_object_set(ballast_ar_air_item_back, "text", s12, NULL);
  g_object_set(ballast_ar_air_item_front, "text", s12, NULL);

  y_new = \
    schema_y + BALLAST_AR_AIR_Y1 + BALLAST_AR_AIR_H +
    ( (MAX_BALLAST - value) * BALLAST_AR_AIR_H * -1) / MAX_BALLAST;

  height = \
    schema_y + BALLAST_AR_AIR_Y1 + BALLAST_AR_AIR_H - y_new;

  g_object_set(ballast_ar_air_item_rect,
	       "y", y_new,
	       "height", height,
	       NULL);
}

static void open_door()
{
  gdouble height;

  g_object_get(top_gate_item, "height", &height, NULL);
  gate_top_current_y--;
  height--;
  g_object_set(top_gate_item,
	       "height", height,
	       NULL);
}
  /* =====================================================================
 *	Submarine explosion
 * =====================================================================*/
static void submarine_explosion()
{
  GdkPixbuf *pixmap = NULL;

  if (submarine_destroyed)
    return;

  submarine_destroyed = TRUE;
  gamewon = FALSE;
  gc_sound_play_ogg("sounds/crash.wav", NULL);

  /* make the submarine die */
  setSpeed(speed_ordered = submarine_horizontal_speed = 0.0);
  setBattery(battery = 0.0);
  setAir(air = 0.0);
  regleur = MAX_REGLEUR;
  weight = 2000.0;

  /* Stop the frigate */
  goo_canvas_item_stop_animation (frigate_item);

  /* display the boken submarine */
  pixmap = gc_pixmap_load("submarine/submarine-broken.png");
  g_object_set(submarine_item,
	       "pixbuf", pixmap,
	       NULL);
  gdk_pixbuf_unref(pixmap);

  ok();
}
