/* gcompris - score.c
 *
 * Time-stamp: <2006/08/21 23:27:04 bruno>
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
 * This includes an API to count scores in gcompris
 *
 */

#include "gcompris.h"

static GnomeCanvasGroup *boardRootItem = NULL;

static guint x, y, max;
static ScoreStyleList currentStyle;

/*
 * Forward declarations
 */
static void display_number(GnomeCanvasGroup *parent, 
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
    gtk_object_destroy (GTK_OBJECT(boardRootItem));
  
  boardRootItem=NULL;
}

void
gc_score_set(guint value)
{

  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (
							    gnome_canvas_root(gc_board_get_current()->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) x,
							    "y", (double) y,
							    NULL));

  switch(currentStyle) {
  case SCORESTYLE_NOTE :
    {
      gchar *tmp;
      GdkPixbuf *button_pixmap = NULL;

      button_pixmap = gc_skin_pixmap_load("button_large.png");
      gnome_canvas_item_new (boardRootItem,
			     gnome_canvas_pixbuf_get_type (),
			     "pixbuf",  button_pixmap,
			     "x",  (double) 0,
			     "y",  (double) -gdk_pixbuf_get_height(button_pixmap)/2,
			     NULL);
      gdk_pixbuf_unref(button_pixmap);

      tmp = g_strdup_printf("%d/%d", value, max);
      display_number(boardRootItem, gdk_pixbuf_get_width(button_pixmap)+10, 0, tmp);
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


#define NUMBERSWIDTH       110

static void
display_number(GnomeCanvasGroup *parent, 
	       guint x,
	       guint y, 
	       char *operand_str)
{

  x -= NUMBERSWIDTH;

  gnome_canvas_item_new (parent,
			 gnome_canvas_text_get_type (),
			 "text", operand_str,
			 "font", gc_skin_font_board_huge_bold,
			 "x", (double) x+2,
			 "y", (double) y+2,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color_rgba", 0x7a8699FF,
			 NULL);
  gnome_canvas_item_new (parent,
			 gnome_canvas_text_get_type (),
			 "text", operand_str,
			 "font", gc_skin_font_board_huge_bold,
			 "x", (double) x,
			 "y", (double) y,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color_rgba", 0xe5e532FF,
			 NULL);
}
