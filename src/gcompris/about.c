/* gcompris - about.c
 *
 * Time-stamp: <2006/08/21 23:27:24 bruno>
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
#include <string.h>

static GooCanvasItem	*rootitem		= NULL;

static gboolean is_displayed			= FALSE;


static gboolean item_event_ok (GooCanvasItem  *item,
			       GooCanvasItem  *target,
			       GdkEventButton *event,
			       gchar *data);

/*
 * Do all the bar display and register the events
 */
void gc_about_start ()
{
  GdkPixbuf   *pixmap = NULL;
  gdouble y_start = 0;
  gdouble x_start = 0;
  gint y = 0;
  GooCanvasItem *item, *item2;
  gint plane_y;

  static gchar *content =
    N_("Author: Bruno Coudoin\n"
       "Contribution: Pascal Georges, Jose Jorge, Yves Combe\n"
       "Graphics: Renaud Blanchard, Franck Doucet\n"
       "Intro Music: Djilali Sebihi\n"
       "Background Music: Rico Da Halvarez\n"
       );

  /* TRANSLATORS: Replace this string with your names, one name per line. */
  gchar *translators = _("translator_credits");

  /* Pause the board */
  gc_board_pause(TRUE);

  if(rootitem)
    return;

  gc_bar_hide (TRUE);

  rootitem = goo_canvas_group_new (goo_canvas_get_root_item(gc_get_canvas()),
				   NULL);

  pixmap = gc_skin_pixmap_load("help_bg.png");
  y_start = (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  x_start = (BOARDWIDTH - gdk_pixbuf_get_width(pixmap))/2;
  item = goo_canvas_image_new (rootitem,
			       pixmap,
			       x_start,
			       y_start);
  y = BOARDHEIGHT - (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  gdk_pixbuf_unref(pixmap);

  goo_canvas_text_new (rootitem,
		       _("About GCompris"),
		       (gdouble) BOARDWIDTH/2 + 1.0,
		       (gdouble) y_start + 40 + 1.0,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_title,
		       "fill-color-rgba", gc_skin_color_shadow,
		       NULL);
  goo_canvas_text_new (rootitem,
		       _("About GCompris"),
		       (gdouble) BOARDWIDTH/2,
		       (gdouble) y_start + 40,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_title,
		       "fill-color-rgba", gc_skin_color_title,
		       NULL);

  goo_canvas_text_new (rootitem,
		       _("Translators:"),
		       (gdouble) BOARDWIDTH/2-320,
		       (gdouble) y_start + 90,
		       -1,
		       GTK_ANCHOR_NORTH_WEST,
		       "font", gc_skin_font_subtitle,
		       "fill-color-rgba", gc_skin_color_subtitle,
		       NULL);

  goo_canvas_text_new (rootitem,
		       translators,
		       (gdouble)  BOARDWIDTH/2-320,
		       (gdouble)  y_start + 120,
		       -1,
		       GTK_ANCHOR_NORTH_WEST,
		       "font", gc_skin_font_content,
		       "fill-color-rgba", gc_skin_color_content,
		       NULL);
  // Version
  y_start += 100;

  goo_canvas_text_new (rootitem,
		       "GCompris V" VERSION,
		       (gdouble)  BOARDWIDTH/2,
		       (gdouble)  y_start,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_title,
		       "fill-color-rgba", gc_skin_color_subtitle,
		       NULL);

  y_start += 140;
  goo_canvas_text_new (rootitem,
		       gettext(content),
		       (gdouble) BOARDWIDTH/2-320,
		       (gdouble)  y_start,
		       -1,
		       GTK_ANCHOR_NORTH_WEST,
		       "font", gc_skin_font_content,
		       "fill-color-rgba", gc_skin_color_content,
		       NULL);

  y_start += 40;
  /* Location for a potential sponsor */
  gchar *sponsor_image = gc_file_find_absolute("sponsor.png");
  if(sponsor_image)
    {
      pixmap = gc_pixmap_load("sponsor.png");
      goo_canvas_text_new (rootitem,
			   "Version parrainée par",
			   (gdouble)  BOARDWIDTH*0.75,
			   (gdouble)  y_start - gdk_pixbuf_get_height(pixmap),
			   -1,
			   GTK_ANCHOR_CENTER,
			   "font", gc_skin_font_content,
			   "fill-color-rgba", gc_skin_color_content,
			   NULL);
      goo_canvas_text_new (rootitem,
			   "Version parrainée par",
			   (gdouble)  BOARDWIDTH*0.75 + 1,
			   (gdouble)  y_start - gdk_pixbuf_get_height(pixmap),
			   -1,
			   GTK_ANCHOR_CENTER,
			   "font", gc_skin_font_content,
			   "fill_color", "black",
			   NULL);

      item = goo_canvas_image_new (rootitem,
				   pixmap,
				   (gdouble) (BOARDWIDTH*0.75) - gdk_pixbuf_get_width(pixmap)/2,
				   (gdouble) y_start - gdk_pixbuf_get_height(pixmap) + 15);

      gdk_pixbuf_unref(pixmap);
      g_free(sponsor_image);
    }
  else
    {
      // Default sponsor is the FSF
      pixmap = gc_skin_pixmap_load("fsflogo.png");
      item = goo_canvas_image_new (rootitem,
				   pixmap,
				   (gdouble) (BOARDWIDTH*0.8) - gdk_pixbuf_get_width(pixmap)/2,
				   (gdouble) y_start - gdk_pixbuf_get_height(pixmap)/2);
      gdk_pixbuf_unref(pixmap);

      item = goo_canvas_text_new (rootitem,
				  "Free Software Foundation\nhttp://www.fsf.org",
				  (gdouble)  (BOARDWIDTH*0.75),
				  (gdouble)  y_start + 80,
				  -1,
				  GTK_ANCHOR_CENTER,
				  "font", gc_skin_font_content,
				  "fill-color-rgba", gc_skin_color_subtitle,
				  NULL);
    }

  // GCompris Reference
  y_start += 80;
  pixmap = gc_skin_pixmap_load("gcomprislogo.png");
  item = goo_canvas_image_new (rootitem,
			       pixmap,
			       (double) (BOARDWIDTH*0.3) - gdk_pixbuf_get_width(pixmap)/2,
			       (double) y_start - gdk_pixbuf_get_height(pixmap)/2,
				NULL);

  gdk_pixbuf_unref(pixmap);

  item = goo_canvas_text_new (rootitem,
			      _("GCompris Home Page\nhttp://gcompris.net"),
			      (gdouble) BOARDWIDTH/2-320,
			      (gdouble)  y_start + 5,
			      -1,
			      GTK_ANCHOR_NORTH_WEST,
			      "font", gc_skin_font_content,
			      "fill-color-rgba", gc_skin_color_subtitle,
			      NULL);

  // Copyright
  item = goo_canvas_text_new (rootitem,
			      "Copyright 2000-2007 Bruno Coudoin and Others",
			      (gdouble)  BOARDWIDTH/2,
			      (gdouble)  y - 95,
			      -1,
			      GTK_ANCHOR_CENTER,
			      "font", gc_skin_font_content,
			      "fill-color-rgba", gc_skin_color_content,
			      NULL);

  // License
  item = goo_canvas_text_new (rootitem,
			      _("This software is a GNU Package and is released under the GNU General Public License"),
			      (gdouble)  BOARDWIDTH/2,
			      (gdouble)  y - 80,
			      -1,
			      GTK_ANCHOR_CENTER,
			      "font", gc_skin_font_content,
			      "fill-color-rgba", gc_skin_color_content,
			      NULL);

  // OK
  pixmap = gc_skin_pixmap_load("button_large.png");
  item = goo_canvas_image_new (rootitem,
			       pixmap,
			       (double) BOARDWIDTH*0.5 - gdk_pixbuf_get_width(pixmap)/2,
			       (double) y - gdk_pixbuf_get_height(pixmap) - 5,
				NULL);

  g_signal_connect(item, "button_press_event",
		   (GtkSignalFunc) item_event_ok,
		   "ok");
  g_signal_connect(item, "button_press_event",
		   (GtkSignalFunc) gc_item_focus_event,
		   NULL);
  gdk_pixbuf_unref(pixmap);


  goo_canvas_text_new (rootitem,
		       _("OK"),
		       (gdouble)  BOARDWIDTH*0.5 + 1.0,
		       (gdouble)  y - gdk_pixbuf_get_height(pixmap) + 20 + 1.0,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_title,
		       "fill-color-rgba", gc_skin_color_shadow,
		       NULL);
  item2 = goo_canvas_text_new (rootitem,
			       _("OK"),
			       (gdouble)  BOARDWIDTH*0.5,
			       (gdouble)  y - gdk_pixbuf_get_height(pixmap) + 20,
			       -1,
			       GTK_ANCHOR_CENTER,
			       "font", gc_skin_font_title,
			       "fill-color-rgba", gc_skin_color_text_button,
			       NULL);
  g_signal_connect(item2, "button_press_event",
		     (GtkSignalFunc) item_event_ok,
		     "ok");
  g_signal_connect(item2, "button_press_event",
		   (GtkSignalFunc) gc_item_focus_event,
		   item);

  pixmap = gc_skin_pixmap_load("gcompris-about.png");

  plane_y = BOARDHEIGHT - gdk_pixbuf_get_height(pixmap);
  item = goo_canvas_image_new (rootitem,
			       pixmap,
			       -gdk_pixbuf_get_width(pixmap),
			       plane_y);

  goo_canvas_item_animate (item,
			   BOARDWIDTH + gdk_pixbuf_get_width(pixmap),
			   -gdk_pixbuf_get_height(pixmap),
			   0.4,
			   -30,
			   TRUE,
			   40 * BOARDWIDTH, 40,
			   GOO_CANVAS_ANIMATE_RESTART);

  gdk_pixbuf_unref(pixmap);

  is_displayed = TRUE;

}

void gc_about_stop ()
{
  // Destroy the help box
  if(rootitem!=NULL)
    goo_canvas_item_remove(rootitem);
  rootitem = NULL;

  /* UnPause the board */
  if(is_displayed)
    gc_board_pause(FALSE);

  gc_bar_hide (FALSE);

  is_displayed = FALSE;
}


/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/


/* Callback for the OK operations */
static gboolean item_event_ok (GooCanvasItem  *item,
			       GooCanvasItem  *target,
			       GdkEventButton *event,
			       gchar *data)
{
  if(!strcmp((char *)data, "ok"))
    gc_about_stop();

  return TRUE;
}
