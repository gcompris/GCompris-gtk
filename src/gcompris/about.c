/* gcompris - about.c
 *
 * Copyright (C) 2000, 2008 Bruno Coudoin
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
  GooCanvasItem *item;
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

  item = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", "#DIALOG",
			     "pointer-events", GOO_CANVAS_EVENTS_NONE,
			     NULL);

  GooCanvasBounds bounds;
  goo_canvas_item_get_bounds(item, &bounds);
  x_start = bounds.x1;
  y_start = bounds.y1;

  y = bounds.y2 - 26;

  goo_canvas_text_new (rootitem,
		       _("About GCompris"),
		       (gdouble) BOARDWIDTH/2,
		       (gdouble) y_start + 35,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_title,
		       "fill-color-rgba", gc_skin_color_title,
		       NULL);

  goo_canvas_text_new (rootitem,
		       _("GCompris Home Page: http://gcompris.net"),
		       (gdouble) BOARDWIDTH/2,
		       (gdouble)  y_start + 58,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_content,
		       "fill-color-rgba", gc_skin_color_subtitle,
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

  // Copyright
  item = goo_canvas_text_new (rootitem,
			      "Copyright 2000-2008 Bruno Coudoin and Others",
			      (gdouble)  BOARDWIDTH/2,
			      (gdouble)  y - 55,
			      -1,
			      GTK_ANCHOR_CENTER,
			      "font", gc_skin_font_content,
			      "fill-color-rgba", gc_skin_color_content,
			      NULL);

  // License
  item = goo_canvas_text_new (rootitem,
			      _("This software is a GNU Package and is released under the GNU General Public License"),
			      (gdouble)  BOARDWIDTH/2,
			      (gdouble)  y - 40,
			      -1,
			      GTK_ANCHOR_CENTER,
			      "font", gc_skin_font_content,
			      "fill-color-rgba", gc_skin_color_content,
			      NULL);

  // OK
  gc_util_button_text_svg(rootitem,
			  BOARDWIDTH * 0.5,
			  y,
			  "#BUTTON_TEXT",
			  _("OK"),
			  (GtkSignalFunc) item_event_ok,
			  "ok");

  // The animation
  pixmap = gc_skin_pixmap_load("gcompris-about.png");

  plane_y = BOARDHEIGHT - gdk_pixbuf_get_height(pixmap);
  item = goo_canvas_image_new (rootitem,
			       pixmap,
			       -1 * gdk_pixbuf_get_width(pixmap),
			       plane_y,
			       NULL);

  goo_canvas_item_animate (item,
			   BOARDWIDTH + gdk_pixbuf_get_width(pixmap),
			   -1 * gdk_pixbuf_get_height(pixmap),
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
