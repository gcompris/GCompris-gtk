/* gcompris - cursor.c
 *
 * Copyright (C) 2002 Pascal Georges
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

#include "gcompris.h"
#include "cursor.h"

static GdkCursor *hand_cursor;
GtkWidget *window;

/* =====================================================================
 *
 * =====================================================================*/
/*void init_cursor() {
  GdkCursor *hand_cursor;
	GdkColor fg, bg;

  gdk_color_parse("rgb:0000/0000/0000",&fg);
  gdk_color_parse("rgb:FFFF/3FFF/0000",&bg);

	hand_cursor = gdk_cursor_new_from_data(handcursor_bits, 40, 40, &fg, &bg, 0, 0);
	gdk_window_set_cursor(gcompris_get_window(), hand_cursor);
  gdk_cursor_destroy(hand_cursor);
}
*/
/* =====================================================================
 * This function is taken from stickers game
 * by Paul Kienzle and Tanya Riseman <stickers@kienzle.powernet.co.uk>
 * =====================================================================*/
/* A routine that ought to be in the gdk library: build a cursor from
 * a text layout with '1' for foreground, '0' for background and ' '
 * for transparent.
 */
GdkCursor *gdk_cursor_new_from_data(const gchar *bits[],
				    gint width, gint height,
				    GdkColor *fg, GdkColor *bg,
				    gint hot_x, gint hot_y)
{
  GdkBitmap *bitmap, *mask;
  GdkCursor *cursor;
  guchar *data, *ptr;
  gint i, j;

  /* Though it does not say so on the X11 manual pages, the bitmap
   * format consists of scan lines padded to byte boundaries with the
   * bits in "reverse" order within each byte (lo bits come before hi
   * bits in the bitmap).  I assume this representation is platform
   * independent.  Let me know if it doesn't work for you. */

  /* Create space for the bitmap, padding the scanlines to byte boundaries. */
  data = g_new(guchar, ((width+7)/8)*height);

  /* Build bitmap */
  ptr = data;
  for (i=0; i < height; i++) {
    for (j=0; j < width; j++) {
      *ptr = (*ptr >> 1)|(bits[i][j]=='1'?0x80:0);
      if (j%8 == 7) ptr++;
    }
    if (j%8) *ptr++ >>= 8-j%8;
  }
  bitmap = gdk_bitmap_create_from_data(NULL, data, width, height);

  /* Build mask */
  ptr = data;
  for (i=0; i < height; i++) {
    for (j=0; j < width; j++) {
      *ptr = (*ptr >> 1)|(bits[i][j]==' '?0:0x80);
      if (j%8 == 7) ptr++;
    }
    if (j%8) *ptr++ >>= 8-j%8;
  }
  mask = gdk_bitmap_create_from_data(NULL, data, width, height);

  /* Build cursor from bitmap and mask */
  cursor = gdk_cursor_new_from_pixmap(bitmap, mask, /* Image and mask */
				      fg, bg, /* colors */
				      hot_x, hot_y); /* Hot point */

  /* No longer need bitmap or mask */
  gdk_pixmap_unref(bitmap);
  gdk_pixmap_unref(mask);
  g_free(data);

  return cursor;
}
