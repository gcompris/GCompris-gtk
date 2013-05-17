/* gcompris - skin.h
 *
 * Copyright (C) 2003, 2008 GCompris Developpement Team
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

#ifndef SKIN_H
#define SKIN_H

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "gcompris.h"

extern guint32 gc_skin_color_title;
extern guint32 gc_skin_color_text_button;
extern guint32 gc_skin_color_content;
extern guint32 gc_skin_color_subtitle;
extern guint32 gc_skin_color_shadow;

extern gchar* gc_skin_font_title;
extern gchar* gc_skin_font_subtitle;
extern gchar* gc_skin_font_content;
extern gchar* gc_skin_font_helptext;

extern gchar* gc_skin_font_menu_title;
extern gchar* gc_skin_font_menu_description;

extern gchar* gc_skin_font_board_minuscule;
extern gchar* gc_skin_font_board_tiny;
extern gchar* gc_skin_font_board_small;
extern gchar* gc_skin_font_board_medium;
extern gchar* gc_skin_font_board_medium_bold;
extern gchar* gc_skin_font_board_big;
extern gchar* gc_skin_font_board_big_bold;
extern gchar* gc_skin_font_board_fixed;
extern gchar* gc_skin_font_board_title;
extern gchar* gc_skin_font_board_title_bold;
extern gchar* gc_skin_font_board_levelmenu;
extern gchar* gc_skin_font_board_huge;
extern gchar* gc_skin_font_board_huge_bold;


gchar		*gc_skin_image_get(gchar *imagename);
GdkPixbuf	*gc_skin_pixmap_load_or_null(char *pixmapfile);
GdkPixbuf	*gc_skin_pixmap_load(char *pixmapfile);
RsvgHandle	*gc_skin_rsvg_get();
RsvgHandle	*gc_skin_rsvg_load(char *pixmapfile);

gboolean        gc_skin_load (gchar* skin);
void            gc_skin_free (void);

guint32         gc_skin_get_color_default(gchar* id, guint32 def);
void		gc_skin_get_gdkcolor_default(gchar* id, guint32 def, GdkColor *gdkcolor);
gchar*          gc_skin_get_font_default(gchar* id, gchar* def);
guint32		gc_skin_get_number_default(gchar* id, guint32 def);

#define gc_skin_get_gdkcolor(id, gdkcolor) gc_skin_get_gdkcolor_default(id, 0x0D0DFA00, gdkcolor)
#define gc_skin_get_color(id)     gc_skin_get_color_default(id, 0x0D0DFA00)
#define gc_skin_get_font(id)      gc_skin_get_font_default(id, "Sans 12")
#define gc_skin_get_number(id)    gc_skin_get_number_default(id, 0)

#endif
