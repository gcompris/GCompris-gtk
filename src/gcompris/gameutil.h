/* gcompris - gameutil.h
 *
 * Time-stamp: <2000/07/16 00:32:26 bruno>
 *
 * Copyright (C) 2000 Bruno Coudoin
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

/*! \file gameutil.h
  \brief Function usefull to board developpers
*/

#ifndef GAMEUTIL_H
#define GAMEUTIL_H

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgnomecanvas/gnome-canvas-pixbuf.h>

#include "gcompris.h"
#include "soundutil.h"

// returns a random integer in range [A,B]
#define RAND(A,B) (A + (int)((float)(B-A+1)*rand()/(RAND_MAX+1.0)))

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

gchar 		*g_utf8_strndup(gchar*,gint);
gchar           *reactivate_newline(char *str);

GdkPixbuf	*gc_pixmap_load(const gchar *filename, ...);
void		 gc_item_focus_set(GnomeCanvasItem *item, gboolean focus);
gint		 gc_item_focus_event(GnomeCanvasItem *item, GdkEvent *event, 
					   GnomeCanvasItem *dest_item);

gchar		*convertUTF8Toisolat1(gchar * text);
void		 gc_item_absolute_move(GnomeCanvasItem *item, int x, int y);
void		 gc_item_rotate(GnomeCanvasItem *item, double angle);
void		 gc_item_rotate_relative(GnomeCanvasItem *item, double angle);
void		 gc_item_rotate_with_center(GnomeCanvasItem *item, double angle, int x, int y);
void		 gc_item_rotate_relative_with_center(GnomeCanvasItem *item, double angle, int x, int y);

GnomeCanvasGroup *gc_difficulty_display(GnomeCanvasGroup *parent, double x, double y, 
						    double ratio,
						    int difficulty);

/* find the complete filename looking for the file everywhere (printf formatting supported) */
gchar		 *gc_file_find_absolute(const gchar *filename, ...);

#endif
