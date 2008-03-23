/* gcompris - score.c
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
 * This includes an API to count scores in gcompris
 *
 */

#include "gcompris.h"

static GooCanvasItem *boardRootItem = NULL;

static guint x, y, max;
static ScoreStyleList currentStyle;

/*
 * Forward declarations
 */
static void display_number(GooCanvasItem *parent,
			   guint x,
			   guint y,
			   char *operand_str);

/*
 * Main entry score
 * ----------------
 *
 */


/*
 * Do all the score display
 */
void
gc_score_start (ScoreStyleList style, guint gx, guint gy, guint gmax)
{

  currentStyle = style;
  x = gx;
  y = gy;
  max = gmax;


  switch(currentStyle) {
  case SCORESTYLE_NOTE :
    break;
  default:
    break;
  }
}


void
gc_score_end()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem=NULL;
}

void
gc_score_set(guint value)
{

  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = \
    goo_canvas_group_new (goo_canvas_get_root_item(GOO_CANVAS(gc_board_get_current()->canvas)),
			  NULL);

  switch(currentStyle) {
  case SCORESTYLE_NOTE :
    {
      gchar *tmp;
      GdkPixbuf *button_pixmap = NULL;

      button_pixmap = gc_skin_pixmap_load("button_large.png");
      goo_canvas_image_new (boardRootItem,
			    button_pixmap,
			    x,
			    y-gdk_pixbuf_get_height(button_pixmap)/2,
			    NULL);

      tmp = g_strdup_printf("%d/%d", value, max);
      display_number(boardRootItem,
		     x + gdk_pixbuf_get_width(button_pixmap)/2,
		     y - gdk_pixbuf_get_height(button_pixmap)/2,
		     tmp);
      gdk_pixbuf_unref(button_pixmap);
      g_free(tmp);
    }
    break;
  default:
    break;
  }
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/


static void
display_number(GooCanvasItem *parent,
	       guint x,
	       guint y,
	       char *operand_str)
{

  goo_canvas_text_new (parent,
		       operand_str,
		       x+2,
		       y+2,
		       -1,
		       GTK_ANCHOR_NORTH,
		       "alignment", PANGO_ALIGN_CENTER,
		       "font", gc_skin_font_board_huge_bold,
		       "fill-color-rgba", 0x7a8699FF,
		       NULL);

  goo_canvas_text_new (parent,
		       operand_str,
		       x,
		       y,
		       -1,
		       GTK_ANCHOR_NORTH,
		       "alignment", PANGO_ALIGN_CENTER,
		       "font", gc_skin_font_board_huge_bold,
		       "fill-color-rgba", 0xe5e532FF,
		       NULL);
}
