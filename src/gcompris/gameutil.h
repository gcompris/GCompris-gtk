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


GdkPixbuf	*gcompris_load_pixmap(char *pixmapfile);
void		 gcompris_set_image_focus(GnomeCanvasItem *item, gboolean focus);
gint		 gcompris_item_event_focus(GnomeCanvasItem *item, GdkEvent *event, 
					   GnomeCanvasItem *dest_item);
GcomprisBoard	*gcompris_read_xml_file(GcomprisBoard *gcomprisBoard, char *fname);

GcomprisBoard   *gcompris_get_board_from_section(gchar *section);
GList		*gcompris_get_menulist(gchar *section);
void		 gcompris_load_menus();

gchar		*convertUTF8Toisolat1(gchar * text);
void		 item_absolute_move(GnomeCanvasItem *item, int x, int y);
void		 item_rotate(GnomeCanvasItem *item, double angle);
void		 item_rotate_relative(GnomeCanvasItem *item, double angle);
void		 item_rotate_with_center(GnomeCanvasItem *item, double angle, int x, int y);
void		 item_rotate_relative_with_center(GnomeCanvasItem *item, double angle, int x, int y);

typedef void     (*DialogBoxCallBack)     ();
void		 gcompris_dialog(gchar *str, DialogBoxCallBack dbcb);
void		 gcompris_dialog_close();
GnomeCanvasGroup *display_difficulty_stars(GnomeCanvasGroup *parent, double x, double y, int difficulty);

/*! \fn GdkPixbuf	*gcompris_load_pixmap_asset(gchar *dataset, gchar* categories, gchar* name)
    \brief Returns a pixmap from the assetml base. 
    \warning If there is more than one answer, the first pixmap is returned

    \param dataset The assetml dataset
    \param categories The category filter
    \param mimetype The mimetype for the asset to search
    \param name The name filter
*/
GdkPixbuf	*gcompris_load_pixmap_asset(gchar *dataset, gchar* categories, 
					    gchar* mimetype, gchar* name);

/*! \fn gchar	*gcompris_get_asset_file(gchar *dataset, gchar* categories, gchar* name)
    \brief Returns a filename path found from the assetml base. 
    \warning If there is more than one answer, the first asset found is returned

    \param dataset The assetml dataset
    \param categories The category filter
    \param mimetype The mimetype for the asset to search
    \param file The file filter
*/
gchar		*gcompris_get_asset_file(gchar *dataset, gchar* categories, 
					 gchar* mimetype, gchar* file);

#endif
