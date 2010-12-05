/* gcompris - status.c
 *
 * Copyright (C) 2010 Bruno Coudoin
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

#include <string.h>

#include "gcompris.h"

static GooCanvasItem *rootStatusItem = NULL;
static GooCanvasItem *itemStatusMsg = NULL;

/*
 * Close the status box if it was open. It not, do nothing.
 */
void gc_status_close() {

  /* If we already running delete the previous one */
  if(rootStatusItem) {
    goo_canvas_item_remove(rootStatusItem);
  }

  rootStatusItem = NULL;
  itemStatusMsg = NULL;

}

/*
 * Display a status box
 * with the given initial message
 */
void gc_status_init(gchar *msg)
{
  g_message("Status=%s\n", msg);

  /* If we are already running do nothing */
  if(rootStatusItem) {
    g_warning("Cannot run a status box, one is already running. Message = %s\n", msg);
    return;
  }

  if(gc_get_canvas() == NULL)
    {
      g_warning("Cannot run a status box, canvas not initialized yet = %s\n", msg);
      return;
    }

  rootStatusItem =
    goo_canvas_group_new (goo_canvas_get_root_item(gc_get_canvas()),
			  NULL);
  /* Set the new canvas to the background color or it's white */
  guint offsetx = 100;
  guint offsety = 200;
  GooCanvasItem *item =
    goo_canvas_rect_new (rootStatusItem,
			 offsetx,
			 offsety,
			 BOARDWIDTH - offsetx * 2,
			 BOARDHEIGHT - offsety * 2,
			 "fill-color-rgba", 0xD0D0D0D0L,
			 "stroke_color_rgba", 0x000000FFL,
			 "line-width", 1.0,
			 "radius-x", (double) 10,
			 "radius-y", (double) 10,
			 NULL);

   GooCanvasBounds bounds;
   goo_canvas_item_get_bounds(item, &bounds);

  itemStatusMsg = \
    goo_canvas_text_new (rootStatusItem,
			 msg,
			 BOARDWIDTH / 2,
			 bounds.y1 + (bounds.y2 - bounds.y1) / 2,
			 (bounds.x2 - bounds.x1) - 20,
			 GTK_ANCHOR_CENTER,
			 NULL);

  while (gtk_events_pending())
    gtk_main_iteration();
}

/*
 * Update the message in the status box
 * \param format: printf formating.
 * \param ...:    additional params for the format (printf like)
 */
void gc_status_set_msg(const gchar *format, ...)
{
  va_list args;
  gchar *msg;

  // No status bar means we are still at command line level.
  if (!itemStatusMsg || !format)
    return;
  g_assert(rootStatusItem);

  va_start (args, format);
  msg = g_strdup_vprintf (format, args);
  va_end (args);

  g_object_set (itemStatusMsg,
		"text", msg,
		NULL);
  g_free(msg);
  while (gtk_events_pending())
    gtk_main_iteration();
}
