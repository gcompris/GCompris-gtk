/* gcompris - config.c
 *
 * Time-stamp: <2001/12/27 01:25:27 bruno>
 *
 * Copyright (C) 2000 Bruno Coudoin
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

/**
 * Configuration of gcompris
 */

#include "gcompris.h"
#include "gcompris_config.h"

static GnomeCanvasItem	*rootitem		= NULL;
static GdkPixbuf	*pixmap_checked		= NULL;
static GdkPixbuf	*pixmap_unchecked	= NULL;

#define SOUNDLISTFILE PACKAGE

static gint item_event_ok(GnomeCanvasItem *item, GdkEvent *event, gpointer data);


/*
 * Do all the bar display and register the events
 */
void gcompris_config_start ()
{
  GcomprisBoard		*gcomprisBoard = get_current_gcompris_board();
  GcomprisProperties	*properties = gcompris_get_properties();
  GdkFont *gdk_font;
  GdkFont *gdk_font_small;
  GdkPixbuf   *pixmap = NULL;
  gint y_start = 0;
  gint x_start = 0;
  gint x_text_start = 0;
  gint y = 0;
  GnomeCanvasItem *item;

  /* Pause the board */
  if(gcomprisBoard->plugin->pause_board != NULL)
    gcomprisBoard->plugin->pause_board(TRUE);

  if(rootitem)
    return;

  rootitem = \
    gnome_canvas_item_new (gnome_canvas_root(gcompris_get_canvas()),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  pixmap = gcompris_load_pixmap("gcompris/help_bg.png");
  y_start = (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  x_start = (BOARDWIDTH - gdk_pixbuf_get_width(pixmap))/2;
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) x_start,
				"y", (double) y_start,
				NULL);
  y = BOARDHEIGHT - (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  gdk_pixbuf_unref(pixmap);

  // TITLE
  gdk_font = gdk_font_load ("-adobe-times-medium-r-normal--*-240-*-*-*-*-*-*");

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", N_("GCompris Configuration"), 
				"font_gdk", gdk_font,
				"x", (double) BOARDWIDTH/2,
				"y", (double) y_start + 40,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color", "white",
				NULL);

  // OK
  pixmap = gcompris_load_pixmap("gcompris/buttons/button_small.png");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH - gdk_pixbuf_get_width(pixmap))/2,
				"y", (double) y - gdk_pixbuf_get_height(pixmap) - 10,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "ok");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
  gdk_pixbuf_unref(pixmap);


  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", N_("OK"),
				"font_gdk", gdk_font,
				"x", (double)  BOARDWIDTH/2,
				"y", (double)  y - gdk_pixbuf_get_height(pixmap) + 8,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color", "white",
				NULL);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "ok");

  // Configuration content
  gdk_font_small = gdk_font_load ("-adobe-times-medium-r-normal--*-180-*-*-*-*-*-*");

  pixmap_checked   = gcompris_load_pixmap("gcompris/buttons/button_checked.png");
  pixmap_unchecked = gcompris_load_pixmap("gcompris/buttons/button_unchecked.png");


  x_start += 150;
  x_text_start = x_start + 50;

  //--------------------------------------------------
  // Locale
  y_start += 140;

  pixmap = gcompris_load_pixmap("gcompris/buttons/button_backward.png");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) x_start - gdk_pixbuf_get_width(pixmap) - 10,
				"y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "locale_backward");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
  gdk_pixbuf_unref(pixmap);


  pixmap = gcompris_load_pixmap("gcompris/buttons/button_forward.png");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) x_start,
				"y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "locale_forward");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
  gdk_pixbuf_unref(pixmap);


  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", N_("Locale"), 
			 "font_gdk", gdk_font_small,
			 "x", (double) x_text_start,
			 "y", (double) y_start,
			 "anchor", GTK_ANCHOR_WEST,
			 "fill_color", "white",
			 NULL);

  // Fullscreen / Window
  y_start += 50;

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", (properties->fullscreen ? pixmap_checked : pixmap_unchecked), 
				"x", (double) x_start,
				"y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "fullscreen");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);


  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", N_("Fullscreen"), 
			 "font_gdk", gdk_font_small,
			 "x", (double) x_text_start,
			 "y", (double) y_start,
			 "anchor", GTK_ANCHOR_WEST,
			 "fill_color", "white",
			 NULL);

  // Music
  y_start += 50;

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", (properties->music ? pixmap_checked : pixmap_unchecked), 
				"x", (double) x_start,
				"y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "music");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);


  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", N_("Music"), 
			 "font_gdk", gdk_font_small,
			 "x", (double) x_text_start,
			 "y", (double) y_start,
			 "anchor", GTK_ANCHOR_WEST,
			 "fill_color", "white",
			 NULL);

  // Effect
  y_start += 50;

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", (properties->fx ? pixmap_checked : pixmap_unchecked), 
				"x", (double) x_start,
				"y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "effect");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);


  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", N_("Effect"), 
			 "font_gdk", gdk_font_small,
			 "x", (double) x_text_start,
			 "y", (double) y_start,
			 "anchor", GTK_ANCHOR_WEST,
			 "fill_color", "white",
			 NULL);

}

void gcompris_config_stop ()
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  // Destroy the help box
  if(rootitem!=NULL)
    {
      gtk_object_destroy(GTK_OBJECT(rootitem));
      gcomprisBoard->plugin->pause_board(FALSE);
    }
  rootitem = NULL;	  

  gdk_pixbuf_unref(pixmap_unchecked);
  gdk_pixbuf_unref(pixmap_checked);

  /* UnPause the board */
  if(gcomprisBoard->plugin->pause_board != NULL)
    gcomprisBoard->plugin->pause_board(FALSE);

}


/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/



/* Callback for the bar operations */
static gint
item_event_ok(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  GcomprisProperties	*properties = gcompris_get_properties();

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
      if(!strcmp((char *)data, "ok"))
	{
	  gcompris_config_stop();
	}
      else if(!strcmp((char *)data, "fullscreen"))
	{
	  properties->fullscreen = (properties->fullscreen ? 0 : 1);
	  gnome_canvas_item_set (item,
				 "pixbuf", (properties->fullscreen ? pixmap_checked : pixmap_unchecked),
				 NULL);

	}
      else if(!strcmp((char *)data, "music"))
	{
	  properties->music = (properties->music ? 0 : 1);
	  gnome_canvas_item_set (item,
				 "pixbuf", (properties->music ? pixmap_checked : pixmap_unchecked),
				 NULL);

	}
      else if(!strcmp((char *)data, "effect"))
	{
	  properties->fx = (properties->fx ? 0 : 1);
	  gnome_canvas_item_set (item,
				 "pixbuf", (properties->fx ? pixmap_checked : pixmap_unchecked),
				 NULL);

	}
    default:
      break;
    }
  return FALSE;

}




/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
