/* gcompris - bonus.c
 *
 * Time-stamp: <2001/10/15 01:10:21 bruno>
 *
 * Copyright (C) 2001 Pascal George
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "bonus.h"

static GnomeCanvasItem *bonus_item = NULL;

static gint end_bonus_id = 0;


/* ==================================== */
void gcompris_display_bonus(int gamewon, GcomprisBoard *gcomprisBoard, int bonus_id) 
{
  switch(bonus_id) {
  case SMILEY_BONUS : 
    bonus_smiley(gamewon, gcomprisBoard); 
    break;
  default : 
    bonus_smiley(gamewon, gcomprisBoard); 
    break;
  }
}

/* ==================================== */
/* unfortunately, gcomprisBoard has to be passed as a parameter.
 * I tried to put it in bonus.h, but it does not get updated (remains NULL)
*/
void bonus_smiley(int gamewon, GcomprisBoard *gcomprisBoard) 
{
  char *str= NULL;
  int x,y;
  GdkPixbuf *pixmap = NULL;

  /* bonus_item must be a singleton */
  if (bonus_item != NULL)
  	return;

  if (gamewon == TRUE)
    str = g_strdup_printf("%s", "gcompris/bonus/smiley_good.png");
  else
    str = g_strdup_printf("%s", "gcompris/bonus/smiley_bad.png");

  pixmap = gcompris_load_pixmap(str);

  assert(gcomprisBoard != NULL);

  x = (gcomprisBoard->width - gdk_pixbuf_get_width(pixmap))/2;
  y = (gcomprisBoard->height - gdk_pixbuf_get_height(pixmap))/2;
  bonus_item = gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", pixmap,
				      "x", (double) x,
				      "y", (double) y,
				      "width", (double) gdk_pixbuf_get_width(pixmap),
				      "height", (double) gdk_pixbuf_get_height(pixmap),
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);
  gdk_pixbuf_unref(pixmap);
  g_free(str);
  end_bonus_id = gtk_timeout_add (2000, (GtkFunction) end_bonus, NULL);
}

/* ==================================== */
void end_bonus() {

  if (end_bonus_id) {
    gtk_timeout_remove (end_bonus_id);
    end_bonus_id = 0;
  }

  if(bonus_item)
    gtk_object_destroy (GTK_OBJECT(bonus_item));

  bonus_item = NULL;
}
