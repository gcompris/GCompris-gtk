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

gchar 		*g_utf8_strndup(gchar*,gint);
gchar           *reactivate_newline(char *str);

GdkPixbuf	*gc_pixmap_load(const gchar *filename, ...);
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
int               gc_util_create_rootdir (gchar *rootdir);

void		 gc_activity_intro_play (GcomprisBoard *gcomprisBoard);
void		 gc_util_button_text(GooCanvasItem *rootitem,
				     guint x, guint y,
				     char *button_file,
				     char *text,
				     GtkSignalFunc process,
				     gpointer data);
#endif
