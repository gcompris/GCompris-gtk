/* this file is part of libccc, criawips' cairo-based canvas
 *
 * AUTHORS
 *       Sven Herzberg        <herzi@gnome-de.org>
 *
 * Copyright (C) 2005 Sven Herzberg
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "pango-helpers.h"

#include <pango/pangocairo.h>

/**
 * pango_layout_new_cairo:
 *
 * Creates a new #PangoLayout to be rendered to a cairo context. This is a
 * stripped down version of pango_cairo_create_layout() which doesn't call
 * pango_cairo_update_context() on the layout.
 *
 * This should not be harmful as long as you update the layout EVERY time you
 * want to render it to a cairo context.
 *
 * This method can be used to make life easier for objects which are tied very
 * closely to a #PangoLayout so they can create the layout in the init function
 * of the instance.
 */
PangoLayout*
pango_layout_new_cairo(void) {
	PangoFontMap* font_map;
	PangoContext* context;
	PangoLayout * layout;

	font_map = pango_cairo_font_map_get_default();
	context  = pango_cairo_font_map_create_context(PANGO_CAIRO_FONT_MAP(font_map));
	layout   = pango_layout_new(context);

	g_object_unref(context);

	return layout;
}

