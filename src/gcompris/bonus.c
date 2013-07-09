/* gcompris - bonus.c
 *
 * Copyright (C) 2001, 2008 Pascal George
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

#include "gcompris.h"
#include "gc_core.h"

#include <ctype.h>
#include <math.h>

#define GC_BONUS_DURATION 2000

static GooCanvasItem *bonus_group   = NULL;

static gboolean bonus_display_running = FALSE;

static gint end_bonus_id = 0;

// List of sounds to use for greetings
static gchar *greetingsList[] =
{
  "voices/$LOCALE/misc/congratulation.ogg",
  "voices/$LOCALE/misc/great.ogg",
  "voices/$LOCALE/misc/good.ogg",
  "voices/$LOCALE/misc/awesome.ogg",
  "voices/$LOCALE/misc/fantastic.ogg",
  "voices/$LOCALE/misc/waytogo.ogg",
  "voices/$LOCALE/misc/super.ogg",
  "voices/$LOCALE/misc/perfect.ogg"
};
#define NUMBER_OF_GREETINGS G_N_ELEMENTS(greetingsList)

// List of sounds to use for loosing
static gchar *loosingList[] =
{
  "voices/$LOCALE/misc/check_answer.ogg",
};
#define NUMBER_OF_LOOSING 1

/*
 * Function definition
 * -------------------
 */
static void	 bonus_image(char *,GCBonusStatusList);
static void	 end_bonus(void);

/* ==================================== */
void
gc_bonus_display(GCBonusStatusList gamewon, GCBonusList bonus_id)
{
  gchar *absolute_file;

  g_assert(bonus_id < GC_BONUS_LAST);

  gc_bar_hide(TRUE);

  if (bonus_display_running) {
    return;
  }
  else
    bonus_display_running = TRUE;

  if(gamewon == GC_BOARD_WIN || gamewon == GC_BOARD_DRAW) {
    absolute_file = gc_file_find_absolute(greetingsList[RAND(0, NUMBER_OF_GREETINGS-1)]);

    if (absolute_file)
      {
	gc_sound_play_ogg(absolute_file, NULL);
	g_free(absolute_file);
      }
    else
      gc_sound_play_ogg("sounds/bonus.wav", NULL);

  } else {
    absolute_file = gc_file_find_absolute(loosingList[RAND(0, NUMBER_OF_LOOSING-1)]);

    if (absolute_file)
      {
	gc_sound_play_ogg(absolute_file, NULL);
	g_free(absolute_file);
      }
    else
      gc_sound_play_ogg("sounds/crash.wav", NULL);
  }

  /* First pause the board */
  gc_board_pause(TRUE);

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
  case GC_BONUS_NOTE :
    bonus_image("note",gamewon);
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

  GcomprisBoard *gcomprisBoard = gc_board_get_current();

  g_assert(gcomprisBoard);

  /* check that bonus_group is a singleton */
  if (bonus_group != NULL) {
    bonus_display_running = FALSE;
    return;
  }


  switch (gamewon) {
  case GC_BOARD_WIN :
    str = g_strdup_printf("%s%s%s", "bonus/",image,"_good.png");
    /* Record the end of board */
    gc_log_end (gcomprisBoard, gamewon);
    break;
  case GC_BOARD_LOOSE :
    str = g_strdup_printf("%s%s%s", "bonus/",image,"_bad.png");
    /* Record the end of board */
    gc_log_end (gcomprisBoard, gamewon);
    break;
  case GC_BOARD_DRAW :
    /* We do not have draw image so a text message is displayed bellow under the
     * win image
     */
    str = g_strdup_printf("%s%s%s", "bonus/",image,"_good.png");
    /* Record the end of board */
    gc_log_end (gcomprisBoard, gamewon);
    break;
  case GC_BOARD_COMPLETED:
    break;
  }

  /* Log the board start again*/
  gc_log_start(gcomprisBoard);

  g_assert(gcomprisBoard != NULL);

  pixmap = gc_skin_pixmap_load(str);

  bonus_group = goo_canvas_group_new (goo_canvas_get_root_item(GOO_CANVAS(gcomprisBoard->canvas)),
				      NULL);

  x = (BOARDWIDTH - gdk_pixbuf_get_width(pixmap))/2;
  y = (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  goo_canvas_image_new (bonus_group,
			pixmap,
			x,
			y,
			NULL);


  if(gamewon==GC_BOARD_DRAW) {
    goo_canvas_text_new (bonus_group,
			 _("Drawn game"),
			 (gdouble) BOARDWIDTH/2 + 1.0,
			 (gdouble) gdk_pixbuf_get_height(pixmap) + 1.0,
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_title,
			 "fill_color", "black",
			 NULL);
    goo_canvas_text_new (bonus_group,
			 _("Drawn game"),
			 (gdouble) BOARDWIDTH/2,
			 (gdouble) gdk_pixbuf_get_height(pixmap),
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_title,
			 "fill-color-rgba", gc_skin_color_title,
			 NULL);
  }

#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
  gdk_pixbuf_unref(pixmap);
#else
  g_object_unref(pixmap);
#endif

  g_free(str);
  end_bonus_id = g_timeout_add (GC_BONUS_DURATION, (GSourceFunc) end_bonus, NULL);
}

/* ==================================== */
static void
end_bonus()
{
  if (end_bonus_id) {
    g_source_remove (end_bonus_id);
    end_bonus_id = 0;
  }

  if(bonus_group)
    goo_canvas_item_remove(bonus_group);

  bonus_group = NULL;
  bonus_display_running = FALSE;

  gc_bar_hide(FALSE);

  /* Re-Start the board */
  gc_board_pause(FALSE);
}
