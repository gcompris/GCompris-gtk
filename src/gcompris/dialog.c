/* gcompris - dialog.c
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

#include <string.h>

#include "gcompris.h"

static GooCanvasItem *rootDialogItem = NULL;
static GooCanvasItem *itemDialogText = NULL;
static gboolean item_event_ok (GooCanvasItem  *item,
			       GooCanvasItem  *target,
			       GdkEventButton *event,
			       DialogBoxCallBack dbcb);

typedef void (*sighandler_t)(int);


/*
 * Close the dialog box if it was open. It not, do nothing.
 */
void gc_dialog_close() {

  /* If we already running delete the previous one */
  if(rootDialogItem) {
    goo_canvas_item_remove(rootDialogItem);
  }

  rootDialogItem = NULL;

}

/*
 * Display a dialog box and an OK button
 * When the box is closed, the given callback is called if any
 */
void gc_dialog(gchar *str, DialogBoxCallBack dbcb)
{
  gint y_start;
  gint x_start;

  g_warning("Dialog=%s\n", str);

  /* If we are already running do nothing */
  if(rootDialogItem) {
    g_warning("Cannot run a dialog box, one is already running. Message = %s\n", str);
    return;
  }

  if(gc_get_canvas() == NULL)
    {
      g_warning("Cannot run a dialog box, canvas not initialized yet = %s\n", str);
      return;
    }

  /* First pause the board */
  gc_board_pause(TRUE);

  gc_bar_hide(TRUE);

  rootDialogItem = goo_canvas_group_new (goo_canvas_get_root_item(gc_get_canvas()),
					 NULL);

  itemDialogText = goo_canvas_svg_new (rootDialogItem,
				       gc_skin_rsvg_get(),
				       "svg-id", "#DIALOG",
				       NULL);
  g_signal_connect(itemDialogText, "button_press_event",
		   (GtkSignalFunc) item_event_ok,
		   dbcb);

  GooCanvasBounds bounds;
  goo_canvas_item_get_bounds(itemDialogText, &bounds);
  x_start = bounds.x1;
  y_start = bounds.y1;

  goo_canvas_text_new (rootDialogItem,
		       str,
		       BOARDWIDTH / 2,
		       (bounds.y2 - bounds.y1)/2 - 100,
		       BOARDWIDTH / 2,
		       GTK_ANCHOR_CENTER,
		       "alignment", PANGO_ALIGN_CENTER,
		       NULL);


  /* OK Button */
  gc_util_button_text_svg(rootDialogItem,
			  BOARDWIDTH * 0.5,
			  bounds.y2 - 30,
			  "#BUTTON_TEXT",
			  _("OK"),
			  (GtkSignalFunc) item_event_ok,
			  "ok");
}

/* Callback for the bar operations */
static gboolean
item_event_ok (GooCanvasItem  *item,
	       GooCanvasItem  *target,
	       GdkEventButton *event,
	       DialogBoxCallBack dbcb)
{
  if(rootDialogItem)
    gc_dialog_close();

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);

  /* restart the board */
  gc_board_pause(FALSE);

  gc_bar_hide(FALSE);

  if(dbcb != NULL)
    dbcb();

  return TRUE;
}
