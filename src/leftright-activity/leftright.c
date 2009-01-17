/* gcompris - leftright.c
 *
 * Copyright (C) 2002, 2008 Pascal Georges
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

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
static int gamewon;
static int last_hand = -1;

static void process_ok(void);
static void clicked_left(void);
static void clicked_right(void);
static void game_won();

#define LEFT 0
#define RIGHT 1

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
static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem *hand_image_item = NULL;

static GooCanvasItem *leftright_create_item(GooCanvasItem *parent);
static void leftright_destroy_all_items(void);
static void leftright_next_level(void);

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
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Find your left and right hands",
    "Given a picture of a hand, work out if it's a right or left hand",
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
    NULL,
    NULL,
    NULL
  };

/* =====================================================================
 *
 * =====================================================================*/
GET_BPLUGIN_INFO(leftright)

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
    gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
                      "leftright/leftright-bg.svgz");
    gcomprisBoard->level=1;
    gcomprisBoard->maxlevel=NUMBER_OF_LEVELS;
    gcomprisBoard->sublevel=1;

    gcomprisBoard->number_of_sublevel = NUMBER_OF_SUBLEVELS;
    gc_score_start(SCORESTYLE_NOTE, 10, 50, gcomprisBoard->number_of_sublevel);
    gc_bar_set(GC_BAR_LEVEL);

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
    gc_score_end();
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
static gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
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
  gc_bar_set_level(gcomprisBoard);

  leftright_destroy_all_items();
  gamewon = FALSE;

  gc_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  leftright_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));

}
/* =====================================================================
 * Destroy all the items
 * =====================================================================*/
static void leftright_destroy_all_items() {
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static GooCanvasItem *leftright_create_item(GooCanvasItem *parent) {
  GdkPixbuf *hand_pixmap = NULL;
  gchar *str;
  int i;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);

  gc_util_button_text_svg(boardRootItem,
			  BOARDWIDTH/3, BOARDHEIGHT*2/3 + 50,
			  "#BUTTON_TEXT",
			  _("left"),
			  (GtkSignalFunc) clicked_left, NULL);

  gc_util_button_text_svg(boardRootItem,
			  BOARDWIDTH*2/3, BOARDHEIGHT*2/3 + 50,
			  "#BUTTON_TEXT",
			  _("right"),
			  (GtkSignalFunc) clicked_right, NULL);

  // make sure that next hand is not the same as previous
  do {
    i = g_random_int_range(0,gcomprisBoard->level*8-1);
  } while ( i == last_hand );

  last_hand = i;

  if ( ((int) (i/2)) % 2 == 1)
    answer = LEFT;
  else
    answer = RIGHT;

  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, hands[i]);
  hand_pixmap = gc_pixmap_load(str);
  hand_image_item =
    goo_canvas_image_new (boardRootItem,
                          hand_pixmap,
                          HAND_X - (gdk_pixbuf_get_width(hand_pixmap)/2),
                          HAND_Y - (gdk_pixbuf_get_height(hand_pixmap)/2),
                          NULL);

  g_free(str);

  gdk_pixbuf_unref(hand_pixmap);

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
    if(gcomprisBoard->level>gcomprisBoard->maxlevel)
      gcomprisBoard->level = gcomprisBoard->maxlevel;
  }
  leftright_next_level();
}

/* =====================================================================
 *
 * =====================================================================*/
static gboolean process_ok_timeout() {
  gc_bonus_display(gamewon, GC_BONUS_SMILEY);
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
static void
clicked_left()
{
  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  board_paused = TRUE;
  gamewon = (LEFT == answer);
  process_ok();
}

static void
clicked_right()
{
  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  board_paused = TRUE;
  gamewon = (RIGHT == answer);
  process_ok();
}

