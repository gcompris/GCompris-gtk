/* gcompris - skin.h
 *
 * Copyright (C) 2003 GCompris Developpement Team
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
 
#ifndef SKIN_H
#define SKIN_H

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "gcompris.h"

extern guint32 gcompris_skin_color_title;
extern guint32 gcompris_skin_color_text_button;
extern guint32 gcompris_skin_color_content;
extern guint32 gcompris_skin_color_subtitle;
extern guint32 gcompris_skin_color_shadow;

extern gchar* gcompris_skin_font_title;
extern gchar* gcompris_skin_font_subtitle;
extern gchar* gcompris_skin_font_content;

extern gchar* gcompris_skin_font_board_tiny;
extern gchar* gcompris_skin_font_board_small;
extern gchar* gcompris_skin_font_board_medium;
extern gchar* gcompris_skin_font_board_big;
extern gchar* gcompris_skin_font_board_big_bold;
extern gchar* gcompris_skin_font_board_fixed;
extern gchar* gcompris_skin_font_board_title;
extern gchar* gcompris_skin_font_board_title_bold;
extern gchar* gcompris_skin_font_board_huge;
extern gchar* gcompris_skin_font_board_huge_bold;


gchar		*gcompris_image_to_skin(gchar *imagename);
GdkPixbuf	*gcompris_load_skin_pixmap(char *pixmapfile);

void            gcompris_skin_load (gchar* skin);
void            gcompris_skin_free (void);

guint32         gcompris_skin_get_color_default(gchar* id, guint32 def);
void		gcompris_skin_get_gdkcolor_default(gchar* id, guint32 def, GdkColor *gdkcolor);
gchar*          gcompris_skin_get_font_default(gchar* id, gchar* def);

#define gcompris_skin_get_gdkcolor(id, gdkcolor) gcompris_skin_get_gdkcolor_default(id, 0x0D0DFA00, gdkcolor)
#define gcompris_skin_get_color(id)     gcompris_skin_get_color_default(id, 0x0D0DFA00)
#define gcompris_skin_get_font(id)      gcompris_skin_get_font_default(id, "Sans 12")
#define gcompris_skin_get_number(id)    gcompris_skin_get_number_default(id, 0)

#endif
