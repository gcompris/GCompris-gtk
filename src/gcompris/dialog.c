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

static GnomeCanvasGroup *rootDialogItem = NULL;
static GnomeCanvasItem *itemDialogText = NULL;
static gint item_event_ok(GnomeCanvasItem *item, GdkEvent *event, DialogBoxCallBack dbcb);

extern GnomeCanvas *canvas;

typedef void (*sighandler_t)(int);


/*
 * Close the dialog box if it was open. It not, do nothing.
 */
void gc_dialog_close() {

  /* If we already running delete the previous one */
  if(rootDialogItem) {
    /* WORKAROUND: There is a bug in the richtex item and we need to remove it first */
    while (g_idle_remove_by_data (itemDialogText));
    gtk_object_destroy (GTK_OBJECT(itemDialogText));
    itemDialogText = NULL;

    gtk_object_destroy(GTK_OBJECT(rootDialogItem));
  }
  
  rootDialogItem = NULL;

}

/*
 * Display a dialog box and an OK button
 * When the box is closed, the given callback is called if any
 */
void gc_dialog(gchar *str, DialogBoxCallBack dbcb)
{
  GcomprisBoard   *gcomprisBoard = get_current_gcompris_board();
  GnomeCanvasItem *item_text   = NULL;
  GnomeCanvasItem *item_text_ok   = NULL;
  GdkPixbuf       *pixmap_dialog = NULL;
  GtkTextIter      iter_start, iter_end;
  GtkTextBuffer   *buffer;
  GtkTextTag      *txt_tag;

  g_warning("Dialog=%s\n", str);

  if(!gcomprisBoard)
    return;

  /* If we are already running do nothing */
  if(rootDialogItem) {
    g_warning("Cannot run a dialog box, one is already running. Message = %s\n", str);
    return;
  }

  /* First pause the board */
  board_pause(TRUE);

  gc_bar_hide(TRUE);

  rootDialogItem = GNOME_CANVAS_GROUP(
				      gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							     gnome_canvas_group_get_type (),
							     "x", (double) 0,
							     "y", (double) 0,
							     NULL));
      
  pixmap_dialog = gcompris_load_skin_pixmap("dialogbox.png");

  itemDialogText = gnome_canvas_item_new (rootDialogItem,
				       gnome_canvas_pixbuf_get_type (),
				       "pixbuf", pixmap_dialog,
				       "x", (double) (BOARDWIDTH - gdk_pixbuf_get_width(pixmap_dialog))/2,
				       "y", (double) (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap_dialog))/2,
				      NULL);

  /* OK Text */
  item_text_ok = gnome_canvas_item_new (rootDialogItem,
					gnome_canvas_text_get_type (),
					"text", _("OK"),
					"font", gcompris_skin_font_title,
					"x", (double)  BOARDWIDTH*0.5,
					"y", (double)  (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap_dialog))/2 +
					gdk_pixbuf_get_height(pixmap_dialog) - 35,
					"anchor", GTK_ANCHOR_CENTER,
					"fill_color_rgba", gcompris_skin_color_text_button,
					"weight", PANGO_WEIGHT_HEAVY,
				NULL);

  gdk_pixbuf_unref(pixmap_dialog);

  gtk_signal_connect(GTK_OBJECT(itemDialogText), "event",
		     (GtkSignalFunc) item_event_ok,
		     dbcb);

  item_text = gnome_canvas_item_new (rootDialogItem,
				     gnome_canvas_rich_text_get_type (),
				     "x", (double) BOARDWIDTH/2,
				     "y", (double) 100.0,
				     "width", (double)BOARDWIDTH-260.0,
				     "height", 400.0,
				     "anchor", GTK_ANCHOR_NORTH,
				     "justification", GTK_JUSTIFY_CENTER,
				     "grow_height", FALSE,
				     "cursor_visible", FALSE,
				     "cursor_blink", FALSE,
				     "editable", FALSE,
				     NULL);

  gnome_canvas_item_set (item_text,
			 "text", str,
			 NULL);

  buffer  = gnome_canvas_rich_text_get_buffer(GNOME_CANVAS_RICH_TEXT(item_text));
  txt_tag = gtk_text_buffer_create_tag(buffer, NULL, 
				       "font",       gcompris_skin_font_board_medium,
				       "foreground", "blue",
				       "family-set", TRUE,
				       NULL);
  gtk_text_buffer_get_end_iter(buffer, &iter_end);
  gtk_text_buffer_get_start_iter(buffer, &iter_start);
  gtk_text_buffer_apply_tag(buffer, txt_tag, &iter_start, &iter_end);

  gtk_signal_connect(GTK_OBJECT(item_text), "event",
		     (GtkSignalFunc) item_event_ok,
		     dbcb);
  gtk_signal_connect(GTK_OBJECT(item_text_ok), "event",
		     (GtkSignalFunc) item_event_ok,
		     dbcb);

}

/* Callback for the bar operations */
static gint
item_event_ok(GnomeCanvasItem *item, GdkEvent *event, DialogBoxCallBack dbcb)
{
  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
      if(rootDialogItem)
	gc_dialog_close();

      /* restart the board */
      board_pause(FALSE);
      
      gc_bar_hide(FALSE);

      if(dbcb != NULL)
	dbcb();
	  
    default:
      break;
    }
  return TRUE;
}
