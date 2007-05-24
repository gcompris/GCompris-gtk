/* This file is part of libccc
 *
 * AUTHORS
 *     Sven Herzberg  <herzi@gnome-de.org>
 *
 * Copyright (C) 2006,2007  Sven Herzberg
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

#include "cc-gradient.h"

#include <ccc/cc-utils.h>

struct CcGradientStop {
	gdouble  offset;
	CcColor* color;
};

struct _CcGradientPrivate {
	GSList* stops;
};
#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE((i), CC_TYPE_GRADIENT, CcGradientPrivate))

static gint
gradient_compare_stops(struct CcGradientStop* first,
		       struct CcGradientStop* second)
{
	if(first->offset > second->offset) {
		return 1;
	} else if(first->offset < second->offset) {
		return -1;
	}
	return 0;
}

/**
 * cc_gradient_add_stop:
 * @self: a #CcGradient
 * @offset: the offset of the color stop (in [0.0;1.0])
 * @color: the #CcColor for this stop
 *
 * Adds a color stop to a gradient. Color stops don't need to be inserted in
 * order sorting happens automatically.
 */
void
cc_gradient_add_stop(CcGradient* self,
		     gdouble     offset,
		     CcColor   * color)
{
	struct CcGradientStop* stop;

	g_return_if_fail(CC_IS_GRADIENT(self));
	g_return_if_fail(0.0 <= offset && offset <= 1.0);
	g_return_if_fail(CC_IS_COLOR(color));

	stop = g_new0(struct CcGradientStop, 1);
	stop->offset = offset;
	stop->color  = g_object_ref_sink(color);
	P(self)->stops = g_slist_insert_sorted(P(self)->stops, stop, (GCompareFunc)gradient_compare_stops);

	// FIXME: emit a changed signal to update items
}

/**
 * cc_gradient_create_pattern:
 * @self: a #CcGradient
 * @view: a #CcView
 * @item: a #CcItem
 *
 * Creates a #cairo_pattern_t for rendering a gradient on @item in @view.
 *
 * Returns a #cairo_pattern_t.
 */
cairo_pattern_t*
cc_gradient_create_pattern(CcGradient const* self,
			   CcView const    * view,
			   CcItem const    * item)
{
	g_return_val_if_fail(CC_GRADIENT_GET_CLASS(self)->create_pattern, NULL);

	return CC_GRADIENT_GET_CLASS(self)->create_pattern(self, view, item);
}

/* GType */
static void gradient_implement_brush (CcBrushIface* iface);
G_DEFINE_ABSTRACT_TYPE_WITH_CODE (CcGradient, cc_gradient, G_TYPE_INITIALLY_UNOWNED,
				  G_IMPLEMENT_INTERFACE (CC_TYPE_BRUSH, gradient_implement_brush));

static void
cc_gradient_init(CcGradient* self G_GNUC_UNUSED)
{}

static void
gradient_apply(CcBrush* brush,
	       CcView * view,
	       CcItem * item,
	       cairo_t* cr)
{
	cairo_pattern_t* pattern = cc_gradient_create_pattern(CC_GRADIENT(brush), view, item);

	if(G_LIKELY(pattern)) {
		struct CcGradientStop* stop;
		GSList* it;
		for(it = P(brush)->stops; it; it = g_slist_next(it)) {
			stop = it->data;
			cc_color_stop(stop->color, pattern, stop->offset);
		}
		cairo_set_source(cr, pattern);
		cairo_pattern_destroy(pattern);
	}
	else {
		cairo_set_source_rgba(cr, 0.925, 0.16, 0.16, 1.0);
	}
}

static void
cc_gradient_class_init (CcGradientClass* self_class)
{
	g_type_class_add_private (self_class, sizeof (CcGradientPrivate));
}

static void
gradient_implement_brush (CcBrushIface* iface)
{
	iface->apply = gradient_apply;
}

