/* gcompris - bonus.c
 *
 * Time-stamp: <2001/10/15 01:10:21 bruno>
 *
 * Copyright (C) 2001 Pascal George
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "bonus.h"

#define SOUNDLISTFILE PACKAGE
#define BONUS_DURATION 2000
#define TUX_TIME_STEP 300

static GnomeCanvasItem *bonus_item = NULL;
static GnomeCanvasItem *door1_item = NULL;
static GnomeCanvasItem *door2_item = NULL;
static GnomeCanvasItem *tuxplane_item = NULL;

static gboolean board_finished_running = FALSE;
static gboolean bonus_display_running = FALSE;

static gint end_bonus_id = 0, board_finished_id = 0;

//static gint end_board_count = 0;
static int left_door_limit = 0;

// List of sounds to use for greetings
static gchar *greetingsList[] =
{
  "congratulation.ogg",
  "great.ogg",
  "good.ogg",
  "awesome.ogg",
  "fantastic.ogg",
  "waytogo.ogg",
  "super.ogg",
  "perfect.ogg"
};
#define NUMBER_OF_GREETINGS 3

/* ==================================== */
void end_board_finished() {
  double dx1, dy1, dx2, dy2;
  //end_board_count++;
  gnome_canvas_item_get_bounds(tuxplane_item,  &dx1, &dy1, &dx2, &dy2);
  // animates tuxplane
  if (/*end_board_count*/ dx2 +50.0 < (double) (left_door_limit)) {
    gnome_canvas_item_move(tuxplane_item, 50, 0);
    return;
  }

  if (board_finished_id) {
    gtk_timeout_remove(board_finished_id);
    board_finished_id = 0;
  }

  if(door1_item)
    gtk_object_destroy (GTK_OBJECT(door1_item));
  if(door2_item)
    gtk_object_destroy (GTK_OBJECT(door2_item));
  if(tuxplane_item)
    gtk_object_destroy (GTK_OBJECT(tuxplane_item));

  door1_item = NULL;
  door2_item = NULL;
  tuxplane_item = NULL;

  board_finished_running = FALSE;
  
  gcompris_bar_hide(FALSE);

  // go back to previous board layout
  if (get_current_board_plugin()->end_board)
    get_current_board_plugin()->end_board();
  gcompris_end_board();
}
/* ==================================== */
#define OFFSET 100
void board_finished(int type) {
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();
  int x,y;
  GdkPixbuf *pixmap_door1 = NULL,*pixmap_door2 = NULL,*pixmap_tuxplane = NULL;
  char * str = NULL;

  gcompris_bar_hide(TRUE);

  if (board_finished_running)
    return;
  else
    board_finished_running = TRUE;

  /* First pause the board */
  if(gcomprisBoard->plugin->pause_board != NULL)
      gcomprisBoard->plugin->pause_board(TRUE);

  /* WARNING: I remove 1 to the BOARD_FINISHED_LAST because RANDOM is for GOOD end only */
  if(type==BOARD_FINISHED_RANDOM)
    type = RAND(1,BOARD_FINISHED_LAST-1);

  /* Record the end of board */
  gcompris_log_end (gcomprisBoard, GCOMPRIS_LOG_STATUS_COMPLETED);

  switch (type) {
	case BOARD_FINISHED_TUXPLANE :
		str = g_strdup_printf("gcompris/misc/tuxplane.png");
		break;
	case BOARD_FINISHED_TUXLOCO :
		str = g_strdup_printf("gcompris/misc/tuxloco.png");
		break;
	case BOARD_FINISHED_TOOMANYERRORS :
		str = g_strdup_printf("gcompris/misc/toomanyerrors.png");
		break;
	default :
		str = g_strdup_printf("gcompris/misc/tuxplane.png");
		break;
  }

  pixmap_door1 = gcompris_load_pixmap("gcompris/misc/door1.png");
  pixmap_door2 = gcompris_load_pixmap("gcompris/misc/door2.png");
  pixmap_tuxplane = gcompris_load_pixmap(str);
  g_free(str);

  g_assert(gcomprisBoard != NULL);

  x = gcomprisBoard->width - OFFSET - gdk_pixbuf_get_width(pixmap_door1);
  y = OFFSET;
  left_door_limit = x + gdk_pixbuf_get_width(pixmap_door1);

  door1_item = gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", pixmap_door1,
				      "x", (double) x,
				      "y", (double) y,
				      "width", (double) gdk_pixbuf_get_width(pixmap_door1),
				      "height", (double) gdk_pixbuf_get_height(pixmap_door1),
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);

  x = OFFSET;
  y = (gcomprisBoard->height - gdk_pixbuf_get_height(pixmap_tuxplane)) /2;
  tuxplane_item = gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", pixmap_tuxplane,
				      "x", (double) x,
				      "y", (double) y,
				      "width", (double) gdk_pixbuf_get_width(pixmap_tuxplane),
				      "height", (double) gdk_pixbuf_get_height(pixmap_tuxplane),
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);

  x = gcomprisBoard->width - OFFSET - gdk_pixbuf_get_width(pixmap_door2);
  y = OFFSET;
  door2_item = gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", pixmap_door2,
				      "x", (double) x,
				      "y", (double) y,
				      "width", (double) gdk_pixbuf_get_width(pixmap_door2),
				      "height", (double) gdk_pixbuf_get_height(pixmap_door2),
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);

  gdk_pixbuf_unref(pixmap_door1);
  gdk_pixbuf_unref(pixmap_door2);
  gdk_pixbuf_unref(pixmap_tuxplane);

  board_finished_id = gtk_timeout_add (TUX_TIME_STEP, (GtkFunction) end_board_finished, NULL);
}

/* ==================================== */
void gcompris_display_bonus(int gamewon, int bonus_id)
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();
  
  gcompris_bar_hide(TRUE);

  if (bonus_display_running)
    return;
  else
    bonus_display_running = TRUE;
  
  if(gamewon == TRUE) {
    gchar *str = gcompris_get_asset_file("gcompris misc", NULL, 
					 "audio/x-ogg", 
					 greetingsList[RAND(0, NUMBER_OF_GREETINGS-1)]);
    gcompris_play_ogg(str, NULL);
    g_free(str);
  } else {
    gcompris_play_ogg ("crash", NULL);
  }

  /* First pause the board */
  if(gcomprisBoard->plugin->pause_board != NULL)
      gcomprisBoard->plugin->pause_board(TRUE);

  if(bonus_id==BONUS_RANDOM)
    bonus_id = RAND(1, BONUS_LAST);

  switch(bonus_id) {
  case BONUS_SMILEY :
    bonus_image("smiley",gamewon);
    break;
  case BONUS_FLOWER :
    bonus_image("flower",gamewon);
    break;
  case BONUS_TUX :
    bonus_image("tux",gamewon);
    break;
  case BONUS_GNU :
    bonus_image("gnu",gamewon);
    break;
  default :
    bonus_image("smiley",gamewon);
    break;
  }
}

/* ==================================== */
void bonus_image(char *image, int gamewon)
{
  char *str= NULL;
  int x,y;
  GdkPixbuf *pixmap = NULL;
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  /* check that bonus_item is a singleton */
  if (bonus_item != NULL) {
    bonus_display_running = FALSE;
    return;
  }



  if (gamewon == TRUE) {
    str = g_strdup_printf("%s%s%s", "gcompris/bonus/",image,"_good.png");
    /* Record the end of board */
    gcompris_log_end (gcomprisBoard, GCOMPRIS_LOG_STATUS_PASSED);
  } else {
    str = g_strdup_printf("%s%s%s", "gcompris/bonus/",image,"_bad.png");
    /* Record the end of board */
    gcompris_log_end (gcomprisBoard, GCOMPRIS_LOG_STATUS_FAILED);
  }
  /* Log the board start again*/
  gcompris_log_start(gcomprisBoard);

  pixmap = gcompris_load_pixmap(str);

  g_assert(gcomprisBoard != NULL);

  x = (gcomprisBoard->width - gdk_pixbuf_get_width(pixmap))/2;
  y = (gcomprisBoard->height - gdk_pixbuf_get_height(pixmap))/2;
  bonus_item = gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", pixmap,
				      "x", (double) x,
				      "y", (double) y,
				      "width", (double) gdk_pixbuf_get_width(pixmap),
				      "height", (double) gdk_pixbuf_get_height(pixmap),
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);

  gdk_pixbuf_unref(pixmap);
  g_free(str);
  end_bonus_id = gtk_timeout_add (BONUS_DURATION, (GtkFunction) end_bonus, NULL);
}

/* ==================================== */
void end_bonus()
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  if (end_bonus_id) {
    gtk_timeout_remove (end_bonus_id);
    end_bonus_id = 0;
  }

  if(bonus_item)
    gtk_object_destroy (GTK_OBJECT(bonus_item));

  bonus_item = NULL;
  bonus_display_running = FALSE;

  gcompris_bar_hide(FALSE);

  /* Re-Start the board */
  if(gcomprisBoard->plugin->pause_board != NULL)
      gcomprisBoard->plugin->pause_board(FALSE);
}
