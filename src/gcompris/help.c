/* gcompris - help.c
 *
 * Time-stamp: <2001/12/19 00:29:19 bruno>
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
#include <gtkhtml/gtkhtml.h>

#define SOUNDLISTFILE PACKAGE

static gint item_event_help(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

GnomeCanvasItem *rootitem = NULL;

/*
 * Main entry point 
 * ----------------
 *
 */


/*
 * Do all the help display and register the events
 */

void gcompris_help_start (gchar *title, gchar *content)
{

  GdkPixbuf   *pixmap = NULL;
  GnomeCanvasItem *item;
  gint y = 0;
  gint y_start = 0;
  gint x_start = 0;
  gint yhtml_start = 0;
  gint xhtml_start = 0;
  GdkFont *gdk_font;
  GtkHTML *gtkHtml;
  gchar *tmp;
  gchar *htmlHeader = "<html>
<head>
<title>GCompris Help</title>
</head>
<body text=\"#000000\" bgcolor=\"#00b7fe\" MARGINHEIGHT=\"0\" MARGINWIDTH=\"0\" 
TOPMARGIN=\"0\" LEFTMARGIN=\"0\">";
  gchar * htmlFooter = "</body></html>";



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
				"text", title, 
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
		     (GtkSignalFunc) item_event_help,
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
		     (GtkSignalFunc) item_event_help,
		     "ok");

  // GTKHTML Content

  tmp = g_strconcat(htmlHeader, content, htmlFooter);
  gtkHtml = GTK_HTML(gtk_html_new_from_string (tmp, strlen(tmp)));
  g_free(tmp);

  xhtml_start = x_start + 40;
  yhtml_start = y_start + 90;
  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_widget_get_type (),
			 "widget", gtkHtml, 
			 "x", (double) xhtml_start,
			 "y", (double) yhtml_start,
			 "width", (double) BOARDWIDTH - xhtml_start*2,
			 "height", (double) BOARDHEIGHT - yhtml_start*2,
			 "size_pixels", TRUE,
			 NULL);
  

  gtk_widget_show (GTK_WIDGET(gtkHtml));

}

/*
 * Remove the displayed help.
 * Do nothing is none is currently being dislayed
 */
void gcompris_help_stop ()
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  if(gcomprisBoard!=NULL)
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
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/



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
