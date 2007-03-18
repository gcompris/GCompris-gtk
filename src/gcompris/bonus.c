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

#include "gcompris.h"
#include "gc_core.h"

#include <ctype.h>
#include <math.h>

#define SOUNDLISTFILE PACKAGE
#define GC_BONUS_DURATION 2000
#define TUX_TIME_STEP 300

static GnomeCanvasGroup *bonus_group   = NULL;
static GnomeCanvasItem  *door1_item    = NULL;
static GnomeCanvasItem  *door2_item    = NULL;
static GnomeCanvasItem  *tuxplane_item = NULL;

static gboolean gc_bonus_end_display_running = FALSE;
static gboolean bonus_display_running = FALSE;

static gint end_bonus_id = 0, gc_bonus_end_display_id = 0;

//static gint end_board_count = 0;
static int left_door_limit = 0;

// List of sounds to use for greetings
static gchar *greetingsList[] =
{
  "sounds/$LOCALE/misc/congratulation.ogg",
  "sounds/$LOCALE/misc/great.ogg",
  "sounds/$LOCALE/misc/good.ogg",
  "sounds/$LOCALE/misc/awesome.ogg",
  "sounds/$LOCALE/misc/fantastic.ogg",
  "sounds/$LOCALE/misc/waytogo.ogg",
  "sounds/$LOCALE/misc/super.ogg",
  "sounds/$LOCALE/misc/perfect.ogg"
};
#define NUMBER_OF_GREETINGS G_N_ELEMENTS(greetingsList)

// List of sounds to use for loosing
static gchar *loosingList[] =
{
  "sounds/$LOCALE/misc/check_answer.ogg",
};
#define NUMBER_OF_LOOSING 1

/*
 * Function definition
 * -------------------
 */
static void	 bonus_image(char *,GCBonusStatusList);
static void	 end_bonus(void);


/* ==================================== */
static void
end_gc_bonus_end_display() {
  double dx1, dy1, dx2, dy2;
  //end_board_count++;
  gnome_canvas_item_get_bounds(tuxplane_item,  &dx1, &dy1, &dx2, &dy2);
  // animates tuxplane
  if (/*end_board_count*/ dx2 +50.0 < (double) (left_door_limit)) {
    gnome_canvas_item_move(tuxplane_item, 50, 0);
    return;
  }

  if (gc_bonus_end_display_id) {
    gtk_timeout_remove(gc_bonus_end_display_id);
    gc_bonus_end_display_id = 0;
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

  gc_bonus_end_display_running = FALSE;

  gc_bar_hide(FALSE);

  // go back to previous board layout
  if (gc_board_get_current_board_plugin()->end_board)
    gc_board_get_current_board_plugin()->end_board();
  gc_board_end();
}
/* ==================================== */
#define OFFSET 100
void
gc_bonus_end_display(GCBoardFinishedList type) {
  GcomprisBoard *gcomprisBoard = gc_board_get_current();

  int x,y;
  GdkPixbuf *pixmap_door1 = NULL,*pixmap_door2 = NULL,*pixmap_tuxplane = NULL;
  char * str = NULL;

  g_assert(type < GC_BOARD_FINISHED_LAST);

  gc_bar_hide(TRUE);

  if (gc_bonus_end_display_running)
    return;
  else
    gc_bonus_end_display_running = TRUE;

  /* First pause the board */
  gc_board_pause(TRUE);

  /* WARNING: I remove 1 to the GC_BOARD_FINISHED_LAST because RANDOM is for GOOD end only */
  if(type==GC_BOARD_FINISHED_RANDOM)
    type = RAND(1,GC_BOARD_FINISHED_LAST-2);

  /* Record the end of board */
  gc_log_end (gcomprisBoard, GC_BOARD_COMPLETED);

  switch (type) {
	case GC_BOARD_FINISHED_TUXPLANE :
		str = g_strdup_printf("gcompris/misc/tuxplane.png");
		break;
	case GC_BOARD_FINISHED_TUXLOCO :
		str = g_strdup_printf("gcompris/misc/tuxloco.png");
		break;
	case GC_BOARD_FINISHED_TOOMANYERRORS :
		str = g_strdup_printf("gcompris/misc/toomanyerrors.png");
		break;
	default :
		str = g_strdup_printf("gcompris/misc/tuxplane.png");
		break;
  }

  pixmap_door1 = gc_pixmap_load("gcompris/misc/door1.png");
  pixmap_door2 = gc_pixmap_load("gcompris/misc/door2.png");
  pixmap_tuxplane = gc_pixmap_load(str);
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

  gc_bonus_end_display_id = gtk_timeout_add (TUX_TIME_STEP, (GtkFunction) end_gc_bonus_end_display, NULL);

}

/* ==================================== */
void
gc_bonus_display(GCBonusStatusList gamewon, GCBonusList bonus_id)
{
  g_warning("bonus display %d %d", (int) gamewon, (int) bonus_id);
  gchar *absolute_file;

  g_assert(bonus_id < GC_BONUS_LAST);

  g_warning("bar_hide...");
  gc_bar_hide(TRUE);
  g_warning("bar_hide... ok");

  if (bonus_display_running) {
    g_warning("error bonus_display_running !");
    return;
  }
  else
    bonus_display_running = TRUE;

  if(gamewon == GC_BOARD_WIN || gamewon == GC_BOARD_DRAW) {
    g_warning("bonus absolute filename... ");
    absolute_file = gc_file_find_absolute(greetingsList[RAND(0, NUMBER_OF_GREETINGS-1)]);

    g_warning("bonus absolute filename: %s", absolute_file );

    if (absolute_file)
      {
	gc_sound_play_ogg(absolute_file, NULL);
	g_free(absolute_file);
      }
    else
      gc_sound_play_ogg("sounds/wahoo.ogg", NULL);

  } else {
    absolute_file = gc_file_find_absolute(loosingList[RAND(0, NUMBER_OF_LOOSING-1)]);

    if (absolute_file)
      {
	gc_sound_play_ogg(absolute_file, NULL);
	g_free(absolute_file);
      }
    else
      gc_sound_play_ogg("sounds/crash.ogg", NULL);
  }

  /* First pause the board */
  g_warning("Pausing board ...");
  gc_board_pause(TRUE);
  g_warning("Pausing board ...ok");

  if(bonus_id==GC_BONUS_RANDOM)
    bonus_id = RAND(1, GC_BONUS_LAST);

  switch(bonus_id) {
  case GC_BONUS_SMILEY :
    bonus_image("smiley",gamewon);
    break;
  case GC_BONUS_FLOWER :
    bonus_image("flower",gamewon);
    break;
  case GC_BONUS_TUX :
    bonus_image("tux",gamewon);
    break;
  case GC_BONUS_GNU :
    bonus_image("gnu",gamewon);
    break;
  case GC_BONUS_LION :
    bonus_image("lion",gamewon);
    break;
  default :
    bonus_image("smiley",gamewon);
    break;
  }
}

/* ==================================== */
static void
bonus_image(char *image, GCBonusStatusList gamewon)
{
  char *str= NULL;
  int x,y;
  GdkPixbuf *pixmap = NULL;

  g_warning("Bonus_image: gc_board_get_current...");

  GcomprisBoard *gcomprisBoard = gc_board_get_current();

  g_assert(gcomprisBoard);

  g_warning("Bonus_image: gc_board_get_current... ok");

  /* check that bonus_group is a singleton */
  if (bonus_group != NULL) {
    bonus_display_running = FALSE;
    return;
  }


  switch (gamewon) {
  case GC_BOARD_WIN :
    str = g_strdup_printf("%s%s%s", "gcompris/bonus/",image,"_good.png");
    /* Record the end of board */
    gc_log_end (gcomprisBoard, gamewon);
    break;
  case GC_BOARD_LOOSE :
    str = g_strdup_printf("%s%s%s", "gcompris/bonus/",image,"_bad.png");
    /* Record the end of board */
    gc_log_end (gcomprisBoard, gamewon);
    break;
  case GC_BOARD_DRAW :
    /* We do not have draw image so a text message is displayed bellow under the
     * win image
     */
    str = g_strdup_printf("%s%s%s", "gcompris/bonus/",image,"_good.png");
    /* Record the end of board */
    gc_log_end (gcomprisBoard, gamewon);
    break;
  case GC_BOARD_COMPLETED:
    break;
  }

  /* Log the board start again*/
  gc_log_start(gcomprisBoard);

  g_assert(gcomprisBoard != NULL);

  pixmap = gc_pixmap_load(str);

  bonus_group = (GnomeCanvasGroup *) \
    gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  x = (gcomprisBoard->width - gdk_pixbuf_get_width(pixmap))/2;
  y = (gcomprisBoard->height - gdk_pixbuf_get_height(pixmap))/2;
  gnome_canvas_item_new (bonus_group,
			 gnome_canvas_pixbuf_get_type (),
			 "pixbuf", pixmap,
			 "x", (double) x,
			 "y", (double) y,
			 "width", (double) gdk_pixbuf_get_width(pixmap),
			 "height", (double) gdk_pixbuf_get_height(pixmap),
			 "width_set", TRUE,
			 "height_set", TRUE,
			 NULL);


  if(gamewon==GC_BOARD_DRAW) {
    gnome_canvas_item_new (bonus_group,
			   gnome_canvas_text_get_type (),
			   "text", _("Drawn game"),
			   "font", gc_skin_font_title,
			   "x", (double) BOARDWIDTH/2 + 1.0,
			   "y", (double) gdk_pixbuf_get_height(pixmap) + 1.0,
			   "anchor", GTK_ANCHOR_CENTER,
			   "fill_color", "black",
			   NULL);
    gnome_canvas_item_new (bonus_group,
			   gnome_canvas_text_get_type (),
			   "text", _("Drawn game"),
			   "font", gc_skin_font_title,
			   "x", (double) BOARDWIDTH/2,
			   "y", (double) gdk_pixbuf_get_height(pixmap),
			   "anchor", GTK_ANCHOR_CENTER,
			   "fill_color_rgba", gc_skin_color_title,
			   NULL);
  }

  gdk_pixbuf_unref(pixmap);

  g_free(str);
  end_bonus_id = gtk_timeout_add (GC_BONUS_DURATION, (GtkFunction) end_bonus, NULL);
}

/* ==================================== */
static void
end_bonus()
{
  if (end_bonus_id) {
    gtk_timeout_remove (end_bonus_id);
    end_bonus_id = 0;
  }

  if(bonus_group)
    gtk_object_destroy (GTK_OBJECT(bonus_group));

  bonus_group = NULL;
  bonus_display_running = FALSE;

  gc_bar_hide(FALSE);

  /* Re-Start the board */
  gc_board_pause(FALSE);
}
