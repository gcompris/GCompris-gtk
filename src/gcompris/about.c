/* gcompris - about.c
 *
 * Time-stamp: <2002/01/07 00:18:34 bruno>
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
#include "about.h"

static GnomeCanvasItem	*rootitem		= NULL;
static GnomeCanvasItem	*plane_item		= NULL;
static GdkPixbuf	*pixmap_about		= NULL;
static gint		 move_plane_id		= 0;
static gint		 plane_x		= 0;
static gint		 plane_y		= 0;
static gint		 plane_speed		= 0;

static gint item_event_ok(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint move_plane (GtkWidget *widget, gpointer item);


/*
 * Do all the bar display and register the events
 */
void gcompris_about_start ()
{
  GcomprisBoard		*gcomprisBoard = get_current_gcompris_board();
  GdkFont *gdk_font;
  GdkFont *gdk_font_small;
  GdkPixbuf   *pixmap = NULL;
  gint y_start = 0;
  gint x_start = 0;
  gint x_text_start = 0;
  gint x_flag_start = 0;
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
				"text", N_("About GCompris"), 
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

  // About box content
  gdk_font_small = gdk_font_load ("-adobe-times-medium-r-normal--*-180-*-*-*-*-*-*");

  pixmap_about = gcompris_load_pixmap("gcompris/gcompris-about.png");

  plane_x = gdk_pixbuf_get_width(pixmap_about)/2;
  plane_y = gdk_pixbuf_get_height(pixmap_about)/2 + 40;
  plane_speed = 1;
  plane_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", pixmap_about, 
				      "x", (double) plane_x,
				      "y", (double) plane_y,
				      NULL);
  move_plane_id = gtk_timeout_add (500,
				   (GtkFunction) move_plane, NULL);

  x_start += 150;
  x_flag_start = x_start + 50;
  x_text_start = x_start + 120;


}

void gcompris_about_stop ()
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  if (move_plane_id) {
    gtk_timeout_remove (move_plane_id);
    move_plane_id = 0;
  }

  // Destroy the help box
  if(rootitem!=NULL)
    {
      gtk_object_destroy(GTK_OBJECT(rootitem));
      gcomprisBoard->plugin->pause_board(FALSE);
    }
  rootitem = NULL;	  

  gdk_pixbuf_unref(pixmap_about);

  /* UnPause the board */
  if(gcomprisBoard->plugin->pause_board != NULL)
    gcomprisBoard->plugin->pause_board(FALSE);

}


/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/


/*
 * This does the moves of the game items on the play canvas
 *
 */
static gint move_plane (GtkWidget *widget, gpointer data)
{
  GcomprisBoard		*gcomprisBoard = get_current_gcompris_board();

  /* Manage the wrapping */
  if(plane_x>gcomprisBoard->width) {
    double x1, y1, x2, y2;
    gnome_canvas_item_get_bounds    (plane_item,
				     &x1,
				     &y1,
				     &x2,
				     &y2);
    gnome_canvas_item_move(plane_item, (double)-gcomprisBoard->width-(x2-x1), (double)0);
    plane_x = plane_x - gcomprisBoard->width - (x2-x1);
  }

  plane_x += plane_speed;
  gnome_canvas_item_move(plane_item, plane_speed, 0);

  /* Return true to be called again */
  return(TRUE);
}

/* Callback for the OK operations */
static gint
item_event_ok(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
      if(!strcmp((char *)data, "ok"))
	{
	  gcompris_about_stop();
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
