/* gcompris - about.c
 *
 * Copyright (C) 2000, 2010 Bruno Coudoin
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

  gchar *text = g_strdup_printf("%s\n%s\n%s",
				_(content),
				_("Translators:"),
				translators);

  y_start += 100;
  goo_canvas_text_new (rootitem,
		       text,
		       (gdouble)  BOARDWIDTH/2-320,
		       (gdouble)  y_start,
		       -1,
		       GTK_ANCHOR_NORTH_WEST,
		       "font", gc_skin_font_content,
		       "fill-color-rgba", gc_skin_color_content,
		       NULL);
  g_free(text);

  // Version
  item = \
    goo_canvas_text_new (rootitem,
		       "GCompris V" VERSION,
		       (gdouble)  0,
		       (gdouble)  y_start,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_title,
		       "fill-color-rgba", gc_skin_color_subtitle,
		       NULL);
  goo_canvas_item_get_bounds(item, &bounds);
  gdouble x = BOARDWIDTH - (bounds.x2 - bounds.x1) - 20;
  g_object_set( (GooCanvasItem*)item,
		"x",
		x,
		NULL);

  y_start += 180;
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
				   (gdouble) y_start - gdk_pixbuf_get_height(pixmap) + 15,
                   NULL);

      gdk_pixbuf_unref(pixmap);
      g_free(sponsor_image);
    }
  else
    {
      // Default sponsor is the FSF
      RsvgHandle  *svg_handle;
      svg_handle = gc_skin_rsvg_get();
      item = goo_canvas_svg_new (rootitem,
				     svg_handle,
				     "svg-id", "#FSF_LOGO",
				     "pointer-events", GOO_CANVAS_EVENTS_NONE,
				     NULL);
      goo_canvas_item_get_bounds(item, &bounds);
      SET_ITEM_LOCATION_CENTER(item, (BOARDWIDTH*0.75),
			       y_start + 50 - (bounds.y2 - bounds.y1) / 2);

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
			      "Copyright 2000-2010 Bruno Coudoin and Others",
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
			      "font", "sans 8",
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
