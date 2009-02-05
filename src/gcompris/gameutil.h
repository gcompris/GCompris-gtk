/* gcompris - gameutil.h
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

/*! \file gameutil.h
  \brief Function usefull to board developpers
*/

#ifndef GAMEUTIL_H
#define GAMEUTIL_H

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "gcompris.h"
#include "soundutil.h"

// returns a random integer in range [A,B]
#define RAND(A,B) (A == B ? A : g_random_int_range((guint32) A,(guint32) B))

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

// This is especially usefull for SVG images that are loaded
// by their Ids and that are not at the 0,0 coordinate in the
// source file.
// This reset the item at its 0,0 coordinate and then translate
// it to x,y
#define SET_ITEM_LOCATION(item, x, y)					\
  {									\
  GooCanvasBounds bounds;						\
    goo_canvas_item_set_transform(item, NULL);				\
    goo_canvas_item_get_bounds(item, &bounds);				\
    goo_canvas_item_translate(item,					\
			      -1 * bounds.x1 + x,			\
			      -1 * bounds.y1 + y);			\
  }

/* Same as above but centerer around x,y */
#define SET_ITEM_LOCATION_CENTER(item, x, y)				\
  {									\
    GooCanvasBounds bounds;						\
    goo_canvas_item_set_transform(item, NULL);				\
    goo_canvas_item_get_bounds(item, &bounds);				\
    goo_canvas_item_translate(item,					\
			      -1 * bounds.x1 + x - (bounds.x2 - bounds.x1)/2, \
			      -1 * bounds.y1 + y - (bounds.y2 - bounds.y1)/2); \
  }

gchar 		*g_utf8_strndup(gchar*,gint);
gchar           *reactivate_newline(char *str);

GdkPixbuf	*gc_pixmap_load(const gchar *filename, ...);
GdkPixbuf       *gc_pixmap_load_or_null(const gchar *format, ...);
RsvgHandle	*gc_rsvg_load(const gchar *format, ...);
void		 gc_item_focus_init(GooCanvasItem *source_item,
				    GooCanvasItem *target_item);
void		 gc_item_focus_remove(GooCanvasItem *source_item,
				      GooCanvasItem *target_item);

gchar		*convertUTF8Toisolat1(gchar * text);
void		 gc_item_absolute_move(GooCanvasItem *item, int x, int y);
void		 gc_item_rotate(GooCanvasItem *item, double angle);
void		 gc_item_rotate_relative(GooCanvasItem *item, double angle);
void		 gc_item_rotate_with_center(GooCanvasItem *item, double angle, int x, int y);
void		 gc_item_rotate_relative_with_center(GooCanvasItem *item, double angle, int x, int y);

/* find the complete filename looking for the file everywhere (printf formatting supported) */
gchar		 *gc_file_find_absolute(const gchar *filename, ...);
gchar		 *gc_file_find_absolute_writeable(const gchar *filename, ...);
int               gc_util_create_rootdir (gchar *rootdir);

void		 gc_activity_intro_play (GcomprisBoard *gcomprisBoard);
void		 gc_util_button_text(GooCanvasItem *rootitem,
				     guint x, guint y,
				     char *button_file,
				     char *text,
				     GtkSignalFunc process,
				     gpointer data);
void		 gc_util_button_text_svg(GooCanvasItem *rootitem,
					 guint x, guint y,
					 char *button_id,
					 char *text,
					 GtkSignalFunc process,
					 gpointer data);
#endif
