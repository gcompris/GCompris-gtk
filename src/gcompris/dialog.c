/* gcompris - dialog.c
 *
 * Time-stamp: <2006/08/20 09:52:47 bruno>
 *
 * Copyright (C) 2000-2006 Bruno Coudoin
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
  GooCanvasItem *item_text   = NULL;
  GooCanvasItem *item_text_ok   = NULL;
  GdkPixbuf     *pixmap_dialog = NULL;

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

  pixmap_dialog = gc_skin_pixmap_load("dialogbox.png");

  itemDialogText = goo_canvas_image_new (rootDialogItem,
					 pixmap_dialog,
					 (BOARDWIDTH - gdk_pixbuf_get_width(pixmap_dialog))/2,
					 (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap_dialog))/2,
					 NULL);

  /* OK Text */
  item_text_ok = goo_canvas_text_new (rootDialogItem,
				      _("OK"),
				      BOARDWIDTH * 0.5,
				      BOARDHEIGHT - 30 -
				      (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap_dialog))/2,
				      -1,
				      GTK_ANCHOR_CENTER,
				      "font", gc_skin_font_title,
				      "fill-color-rgba", gc_skin_color_text_button,
				      NULL);

  g_signal_connect(itemDialogText, "button_press_event",
		   (GtkSignalFunc) item_event_ok,
		   dbcb);

  item_text = goo_canvas_text_new (rootDialogItem,
				   str,
				   BOARDWIDTH / 2,
				   (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap_dialog))/2 + 40,
				   BOARDWIDTH / 2,
				   GTK_ANCHOR_CENTER,
				   "alignment", PANGO_ALIGN_CENTER,
				   NULL);

  gdk_pixbuf_unref(pixmap_dialog);

  g_signal_connect(item_text, "button_press_event",
		   (GtkSignalFunc) item_event_ok,
		   dbcb);
  g_signal_connect(item_text_ok, "button_press_event",
		   (GtkSignalFunc) item_event_ok,
		   dbcb);

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
