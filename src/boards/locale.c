/* gcompris - locale.c
 *
 * Copyright (C) 2001 Bruno Coudoin
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

#define VERTICAL_SEPARATION 30
#define HORIZONTAL_SEPARATION 30
#define TEXT_COLOR "white"

static GnomeCanvasGroup *boardRootItem = NULL;

static void locale_destroy_all_items(void);

/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Select the locale"),
    N_("Select the language for the messages in GCompris"),
    "Bruno Coudoin <bruno.coudoin@free.fr>",
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
    NULL
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

BoardPlugin
*get_bplugin_info(void)
{
  return &menu_bp;
}

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  board_paused = pause;
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{
  GdkFont *gdk_font;
  GdkFont *gdk_font_small;
  GdkPixbuf *menu_pixmap = NULL;

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      "gcompris/gcompris-bg.jpg");
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=1;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gcompris_bar_set(0);
      gcompris_bar_set_timer(0);
      gcompris_bar_set_maxtimer(gcomprisBoard->maxlevel * gcomprisBoard->number_of_sublevel);

      /* Use the given mode to set the locale */
      if(!gcomprisBoard->mode)
	gcompris_set_locale(NULL);
      else 
	gcompris_set_locale(gcomprisBoard->mode);

      boardRootItem = GNOME_CANVAS_GROUP(
					 gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
								gnome_canvas_group_get_type (),
								"x", (double) 0,
								"y", (double) 0,
								
								NULL));


      menu_pixmap = gcompris_load_pixmap(gcomprisBoard->icon_name);

      gnome_canvas_item_new (boardRootItem,
			     gnome_canvas_pixbuf_get_type (),
			     "pixbuf", menu_pixmap,
			     "x", (double)BOARDWIDTH/2 
			     - gdk_pixbuf_get_width(menu_pixmap)/2,
			     "y", (double)BOARDHEIGHT/3 
			     - gdk_pixbuf_get_height(menu_pixmap)/2,
			     "width", (double) gdk_pixbuf_get_width(menu_pixmap),
			     "height", (double) gdk_pixbuf_get_height(menu_pixmap),
			     NULL);

      gdk_font = gdk_font_load ("-adobe-times-medium-r-normal--*-240-*-*-*-*-*-*");

      gnome_canvas_item_new (boardRootItem,
			     gnome_canvas_text_get_type (),
			     "text", _("GCompris will now display its messages in the selected language"),
			     "font_gdk", gdk_font,
			     "x", (double) BOARDWIDTH/2,
			     "y", (double) BOARDHEIGHT/3 + 80,
			     "anchor", GTK_ANCHOR_CENTER,
			     "fill_color", "white",
			     NULL);



      /* Load a gdk font */
      gdk_font_small = gdk_font_load ("-adobe-times-medium-r-normal--*-180-*-*-*-*-*-*");

      gnome_canvas_item_new (boardRootItem,
			     gnome_canvas_text_get_type (),
			     "text", _("If this message is not translated, \nit means that probably GCompris is not translated in this language"),
			     "font_gdk", gdk_font_small,
			     "x", (double) BOARDWIDTH/2,
			     "y", (double) BOARDHEIGHT/3 + 160,
			     "anchor", GTK_ANCHOR_CENTER,
			     "fill_color", "white",
			     NULL);

      pause_board(FALSE);
    }
}
/* ======================================= */
static void end_board ()
{
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      locale_destroy_all_items();
    }
}

/* ======================================= */
static void set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
    }
}
/* ======================================= */
gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "locale")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* ==================================== */
/* Destroy all the items */
static void locale_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
