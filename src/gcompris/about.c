/* gcompris - about.c
 *
 * Time-stamp: <2004/05/19 21:28:25 bcoudoin>
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

static gboolean is_displayed			= FALSE;


static gint item_event_ok(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint move_plane (GtkWidget *widget, gpointer item);

void gcompris_about_start (void);
void gcompris_about_stop (void);

/*
 * Do all the bar display and register the events
 */
void gcompris_about_start ()
{
  GcomprisBoard		*gcomprisBoard = get_current_gcompris_board();
  GdkPixbuf   *pixmap = NULL;
  gint y_start = 0;
  gint x_start = 0;
  gint x_text_start = 0;
  gint x_flag_start = 0;
  gint y = 0;
  GnomeCanvasItem *item, *item2;
  static gchar *content =
    N_("Author: Bruno Coudoin\n"
       "Contribution: Pascal Georges, Jose Jorge\n"
       "Graphics: Renaud Blanchard\n"
       "Intro Music: Djilali Sebihi\n"
       "Background Music: Rico Da Halvarez\n"
       );

  /* TRANSLATORS: Replace this string with your names, one name per line. */
  gchar *translators = _("translator_credits");

  /* Pause the board */
  if(gcomprisBoard->plugin->pause_board != NULL)
    gcomprisBoard->plugin->pause_board(TRUE);

  if(rootitem)
    return;

  gcompris_bar_hide (TRUE);

  rootitem = \
    gnome_canvas_item_new (gnome_canvas_root(gcompris_get_canvas()),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  pixmap = gcompris_load_skin_pixmap("help_bg.png");
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

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("About GCompris"), 
				"font", gcompris_skin_font_title,
				"x", (double) BOARDWIDTH/2,
				"y", (double) y_start + 40,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_title,
				NULL);

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", _("Translators:"), 
			 "font", gcompris_skin_font_subtitle,
			 "x", (double) BOARDWIDTH/2-320,
			 "y", (double) y_start + 90,
			 "anchor", GTK_ANCHOR_NORTH_WEST,
			 "fill_color_rgba", gcompris_skin_color_subtitle,
			 NULL);

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", translators,
			 "font", gcompris_skin_font_content,
			 "x", (double)  BOARDWIDTH/2-320,
			 "y", (double)  y_start + 120,
			 "anchor", GTK_ANCHOR_NORTH_WEST,
			 "fill_color_rgba", gcompris_skin_color_content,
			 NULL);
  // Version
  y_start += 100;

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", "GCompris V" VERSION,
				"font", gcompris_skin_font_title,
				"x", (double)  BOARDWIDTH/2,
				"y", (double)  y_start,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_subtitle,
				NULL);

  y_start += 180;
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", gettext(content),
				"font", gcompris_skin_font_content,
				"x", (double)  BOARDWIDTH/2,
				"y", (double)  y_start,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_content,
				NULL);

  // Ofset Reference
  pixmap = gcompris_load_skin_pixmap("ofsetlogo.png");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH*0.20) - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y_start - gdk_pixbuf_get_height(pixmap)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "ofset");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
  gdk_pixbuf_unref(pixmap);
  
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", "OFSET",
				"font", gcompris_skin_font_content,
				"x", (double)  (BOARDWIDTH*0.25),
				"y", (double)  y_start + 80,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba",gcompris_skin_color_subtitle, 
				NULL);

  // FSF Reference
  pixmap = gcompris_load_skin_pixmap("fsflogo.png");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH*0.8) - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y_start - gdk_pixbuf_get_height(pixmap)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "fsf");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
  gdk_pixbuf_unref(pixmap);
  
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", "Free Software Foundation",
				"font", gcompris_skin_font_content,
				"x", (double)  (BOARDWIDTH*0.75),
				"y", (double)  y_start + 80,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_subtitle,
				NULL);

  // GCompris Reference
  y_start += 70;
  pixmap = gcompris_load_skin_pixmap("gcomprislogo.png");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH*0.5) - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y_start - gdk_pixbuf_get_height(pixmap)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "gcompris");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
  gdk_pixbuf_unref(pixmap);

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("GCompris Home Page"),
				"font", gcompris_skin_font_content,
				"x", (double)  (BOARDWIDTH*0.5),
				"y", (double)  y_start + 30,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_subtitle,
				NULL);

  // Copyright
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", "Copyright 2000-2003 Bruno Coudoin",
				"font", gcompris_skin_font_content,
				"x", (double)  BOARDWIDTH/2,
				"y", (double)  y - 85,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_content,
				NULL);

  // License
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("This software is a GNU Package and is released under the GNU Public Licence"),
				"font", gcompris_skin_font_content,
				"x", (double)  BOARDWIDTH/2,
				"y", (double)  y - 70,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_content,
				NULL);

  // GCompris HELP
  pixmap = gcompris_load_skin_pixmap("button_large.png");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) BOARDWIDTH*0.3 - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y - gdk_pixbuf_get_height(pixmap) - 5,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "help");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
  gdk_pixbuf_unref(pixmap);


  item2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("Help"),
				"font", gcompris_skin_font_title,
				"x", (double)  BOARDWIDTH*0.3,
				"y", (double)  y - gdk_pixbuf_get_height(pixmap) + 20,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_title,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) item_event_ok,
		     "help");
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     item);

  // OK
  pixmap = gcompris_load_skin_pixmap("button_large.png");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) BOARDWIDTH*0.7 - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y - gdk_pixbuf_get_height(pixmap) - 5,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "ok");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
  gdk_pixbuf_unref(pixmap);


  item2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("OK"),
				"font", gcompris_skin_font_title,
				"x", (double)  BOARDWIDTH*0.7,
				"y", (double)  y - gdk_pixbuf_get_height(pixmap) + 20,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_title,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) item_event_ok,
		     "ok");
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     item);

  pixmap_about = gcompris_load_skin_pixmap("gcompris-about.png");

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

  is_displayed = TRUE;

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
      gtk_object_destroy(GTK_OBJECT(rootitem));
  rootitem = NULL;	  

  if(pixmap_about)
    gdk_pixbuf_unref(pixmap_about);
  pixmap_about = NULL;

  /* UnPause the board */
  if(gcomprisBoard->plugin->pause_board != NULL && is_displayed)
    {
      gcomprisBoard->plugin->pause_board(FALSE);
    }

  gcompris_bar_hide (FALSE);

  is_displayed = FALSE;
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
      else if (!strcmp((char *)data, "ofset"))
	{
	  /* FIXME: Should inform the user that we don't find mozilla. */
	  system("mozilla http://ofset.sf.net &");
	}
      else if (!strcmp((char *)data, "fsf"))
	{
	  /* FIXME: Should inform the user that we don't find mozilla. */
	  system("mozilla http://www.fsf.org &");
	}
      else if (!strcmp((char *)data, "gcompris"))
	{
	  /* FIXME: Should inform the user that we don't find mozilla. */
	  system("mozilla http://ofset.sourceforge.net/gcompris &");
	}
      else if (!strcmp((char *)data, "help"))
	{
	  /* It implements gcompris's own way to load help in order to be
	   * Package relocatable.
	   * Unfortunatly, this does not supports I18N
	   */
	  /* FIXME: Should inform the user that we don't find mozilla. */
	  system("mozilla "PACKAGE_HELP_DIR"/C/gcompris.html &");

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
