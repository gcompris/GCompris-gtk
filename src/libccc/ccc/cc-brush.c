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

#include <ccc/cc-brush.h>

#include <ccc/cc-utils.h>
#include "gobject-helpers.h"

G_DEFINE_IFACE (CcBrush, cc_brush, G_TYPE_INTERFACE);

/**
 * cc_brush_apply:
 * @self: a #CcBrush
 * @view: a #CcView
 * @item: a #CcItem
 * @cr: a cairo context
 *
 * Apply a brush to a cairo context. The brush will be used to display @item in
 * @view.
 *
 * This is usually used from item implementations while rendering to a context.
 */
void
cc_brush_apply(CcBrush* self,
	       CcView * view,
	       CcItem * item,
	       cairo_t* cr)
{
	g_return_if_fail(CC_IS_BRUSH(self));
	g_return_if_fail(cr);

	cc_return_if_unimplemented_code(CC_BRUSH_GET_CLASS(self), apply,
					cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0));

	CC_BRUSH_GET_CLASS(self)->apply(self, view, item, cr);
}

