/* gcompris - help.c
 *
 * Time-stamp: <2002/05/05 22:32:20 bruno>
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
 * The bar button common to each games
 *
 */

#include "gcompris.h"

#define SOUNDLISTFILE PACKAGE

static gint item_event_help(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

static gboolean help_displayed = FALSE;

static GnomeCanvasItem *rootitem = NULL;
static GnomeCanvasItem *item_content = NULL;

static gchar   *prerequisite = NULL;
static gchar   *goal = NULL;
static gchar   *manual = NULL;

static GnomeCanvasItem *item_prerequisite = NULL;
static GnomeCanvasItem *item_goal = NULL;
static GnomeCanvasItem *item_manual = NULL;

static GnomeCanvasItem *item_selected = NULL;


/*
 * Main entry point 
 * ----------------
 *
 */

/**
 * Return true wether the given board has at least one help field defined
 *
 */
gboolean gcompris_board_has_help (GcomprisBoard *gcomprisBoard)
{

  prerequisite	= gcomprisBoard->prerequisite;
  goal		= gcomprisBoard->goal;
  manual	= gcomprisBoard->manual;
  return(prerequisite!=NULL || goal!=NULL || manual!=NULL);
}

/*
 * Do all the help display and register the events
 */

void gcompris_help_start (GcomprisBoard *gcomprisBoard)
{

  GdkPixbuf   *pixmap = NULL;
  GnomeCanvasItem *item, *item2;
  gint y = 0;
  gint y_start = 0;
  gint x_start = 0;
  GdkFont *gdk_font;
  GdkFont *gdk_font2;
  gchar   *name = NULL;

  if(rootitem)
    return;

  gcompris_bar_hide(TRUE);

  if(gcomprisBoard!=NULL)
    {
      if(gcomprisBoard->plugin->pause_board != NULL)
	  gcomprisBoard->plugin->pause_board(TRUE);
    }

  name = gcomprisBoard->title;
  gcompris_board_has_help(gcomprisBoard);

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

  // NAME
  gdk_font = gdk_font_load (FONT_TITLE);
  if(!gdk_font)
    // Fallback to a more usual font
    gdk_font = gdk_font_load (FONT_TITLE_FALLBACK);

  y_start += 40;
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", name, 
				"font_gdk", gdk_font,
				"x", (double) BOARDWIDTH/2,
				"y", (double) y_start,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", COLOR_TITLE,
				NULL);


  y_start += 110;

  pixmap = gcompris_load_pixmap("gcompris/buttons/button_large_selected.png");

  // Prerequisite Button
  item_prerequisite = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					     gnome_canvas_pixbuf_get_type (),
					     "pixbuf", pixmap, 
					     "x", (double) (BOARDWIDTH*0.25) - gdk_pixbuf_get_width(pixmap)/2,
					     "y", (double) y_start - gdk_pixbuf_get_height(pixmap) - 10,
					     NULL);
  
  gtk_signal_connect(GTK_OBJECT(item_prerequisite), "event",
		     (GtkSignalFunc) item_event_help,
		     "prerequisite");
  gtk_signal_connect(GTK_OBJECT(item_prerequisite), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  item_selected = item_prerequisite;
  gdk_pixbuf_unref(pixmap);

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("Prerequisite"),
				"font_gdk", gdk_font,
				"x", (double)  BOARDWIDTH*0.25,
				"y", (double)  y_start - gdk_pixbuf_get_height(pixmap) + 12,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", COLOR_TEXT_BUTTON,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_help,
		     "prerequisite");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     item_prerequisite);


  // Goal Button
  pixmap = gcompris_load_pixmap("gcompris/buttons/button_large.png");

  item_goal = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				     gnome_canvas_pixbuf_get_type (),
				     "pixbuf", pixmap, 
				     "x", (double) (BOARDWIDTH*0.5) - gdk_pixbuf_get_width(pixmap)/2,
				     "y", (double) y_start - gdk_pixbuf_get_height(pixmap) - 10,
				     NULL);

  gtk_signal_connect(GTK_OBJECT(item_goal), "event",
		     (GtkSignalFunc) item_event_help,
		     "goal");
  gtk_signal_connect(GTK_OBJECT(item_goal), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("Goal"),
				"font_gdk", gdk_font,
				"x", (double)  BOARDWIDTH*0.5,
				"y", (double)  y_start - gdk_pixbuf_get_height(pixmap) + 12,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", COLOR_TEXT_BUTTON,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_help,
		     "goal");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     item_goal);

  // Manual Button
  item_manual = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH*0.75) - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y_start - gdk_pixbuf_get_height(pixmap) - 10,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item_manual), "event",
		     (GtkSignalFunc) item_event_help,
		     "manual");
  gtk_signal_connect(GTK_OBJECT(item_manual), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("Manual"),
				"font_gdk", gdk_font,
				"x", (double)  BOARDWIDTH*0.75,
				"y", (double)  y_start - gdk_pixbuf_get_height(pixmap) + 12,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", COLOR_TEXT_BUTTON,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_help,
		     "manual");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     item_manual);


  // CONTENT
  y_start += 10;
  gdk_font2 = gdk_font_load (FONT_CONTENT);
  if(!gdk_font2)
    // Fallback to a more usual font
    gdk_font2 = gdk_font_load (FONT_CONTENT_FALLBACK);

  item_content = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					gnome_canvas_text_get_type (),
					"text", prerequisite,
					"font_gdk", gdk_font2,
					"x", (double)  x_start + 45,
					"y", (double)  y_start,
					"anchor", GTK_ANCHOR_NW,
					"fill_color_rgba", COLOR_CONTENT,
					NULL);

  // OK
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH*0.5) - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y - gdk_pixbuf_get_height(pixmap) - 5,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_help,
		     "ok");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  item2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("OK"),
				"font_gdk", gdk_font,
				"x", (double)  BOARDWIDTH*0.5,
				"y", (double)  y - gdk_pixbuf_get_height(pixmap) + 20,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", COLOR_TEXT_BUTTON,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) item_event_help,
		     "ok");
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     item);
  gdk_pixbuf_unref(pixmap);


  help_displayed = TRUE;
}

/*
 * Remove the displayed help.
 * Do nothing is none is currently being dislayed
 */
void gcompris_help_stop ()
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  if(gcomprisBoard!=NULL && help_displayed)
    {
      if(gcomprisBoard->plugin->pause_board != NULL)
	{
	  // Destroy the help box
	  if(rootitem!=NULL)
	    {
	      gtk_object_destroy(GTK_OBJECT(rootitem));
	      gcomprisBoard->plugin->pause_board(FALSE);
	    }
	  rootitem = NULL;	  
	}
    }
  gcompris_bar_hide(FALSE);
  help_displayed = FALSE;
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

static void select_item(GnomeCanvasItem *item)
{
  GdkPixbuf   *pixmap = NULL;
  
  if(item_selected)
    {
      pixmap = gcompris_load_pixmap("gcompris/buttons/button_large.png");
      gnome_canvas_item_set(item_selected, 
			    "pixbuf", pixmap,
			    NULL);
      gdk_pixbuf_unref(pixmap);
    }

  pixmap = gcompris_load_pixmap("gcompris/buttons/button_large_selected.png");
  gnome_canvas_item_set(item, 
			"pixbuf", pixmap,
			NULL);
  gdk_pixbuf_unref(pixmap);
  item_selected = item;
}

/* Callback for the bar operations */
static gint
item_event_help(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
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
	  gcompris_help_stop();
	}      
      else if(!strcmp((char *)data, "prerequisite"))
	{
	  select_item(item_prerequisite);

	  gnome_canvas_item_set(item_content, 
				"text", prerequisite,
				NULL);
	}      
      else if(!strcmp((char *)data, "goal"))
	{
	  select_item(item_goal);

	  gnome_canvas_item_set(item_content, 
				"text", goal,
				NULL);
	}      
      else if(!strcmp((char *)data, "manual"))
	{
	  select_item(item_manual);
	  
	  gnome_canvas_item_set(item_content, 
				"text", manual,
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
