/* gcompris - point.c
 *
 * Time-stamp: <2001/12/01 23:51:03 bruno>
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
 * This includes an API to count points in gcompris
 *
 */

#include "gcompris.h"

static GnomeCanvasGroup *boardRootItem = NULL;

static guint x, y, max;
static PointStyleList currentStyle;

/*
 * Forward declarations
 */
static void display_number(GnomeCanvasGroup *parent, 
			   guint x,
			   guint y, 
			   char *operand_str);

/*
 * Main entry point 
 * ----------------
 *
 */


/*
 * Do all the point display
 */
void gcompris_point_start (PointStyleList style, guint gx, guint gy, guint gmax)
{

  currentStyle = style;
  x = gx;
  y = gy;
  max = gmax;


  switch(currentStyle) {
  case POINTSTYLE_NOTE :
    break;
  default:
    break;
  }
}


void gcompris_point_end()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));
  
  boardRootItem=NULL;
}

void gcompris_point_set(guint value)
{

  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (
							    gnome_canvas_root(get_current_gcompris_board()->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) x,
							    "y", (double) y,
							    NULL));

  switch(currentStyle) {
  case POINTSTYLE_NOTE :
    {
      gchar *tmp;
      GdkPixbuf *button_pixmap = NULL;

      button_pixmap = gcompris_load_pixmap("gcompris/buttons/button_large.png");
      gnome_canvas_item_new (boardRootItem,
			     gnome_canvas_pixbuf_get_type (),
			     "pixbuf",  button_pixmap,
			     "x",  (double) 0,
			     "y",  (double) -gdk_pixbuf_get_height(button_pixmap)/2,
			     NULL);
      gdk_pixbuf_unref(button_pixmap);

      tmp = g_strdup_printf("%d:%d", value, max);
      display_number(boardRootItem, gdk_pixbuf_get_width(button_pixmap)+20, 0, tmp);
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


#define NUMBERSWIDTH       35
#define NUMBERGAP          0
#define ZOOMFACTOR         0.8

static void display_number(GnomeCanvasGroup *parent, 
			   guint x,
			   guint y, 
			   char *operand_str)
{
  GdkPixbuf *algebra_pixmap = NULL;
  GnomeCanvasItem *item;
  int i;

  x -= NUMBERSWIDTH/2 + 15;

  for(i=strlen(operand_str)-1; i>=0; i--)
    {
      guint gap;

      algebra_pixmap = gcompris_load_number_pixmap(operand_str[i]);

      // Calculate a gap to center the text
      gap = (5-strlen(operand_str))*(NUMBERSWIDTH+NUMBERGAP)/2;

      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", algebra_pixmap, 
				    "x", (double)x-gap-((strlen(operand_str)-i)*(NUMBERSWIDTH+NUMBERGAP)),
				    "y", (double)y - gdk_pixbuf_get_height(algebra_pixmap)*ZOOMFACTOR/2,
				    "width", (double) NUMBERSWIDTH,
				    "height", (double) gdk_pixbuf_get_height(algebra_pixmap)*ZOOMFACTOR,
				    "width_set", TRUE, 
				    "height_set", TRUE,
				    NULL);
      gdk_pixbuf_unref(algebra_pixmap);
    }
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
