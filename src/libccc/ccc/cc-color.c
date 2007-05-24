/* this file is part of libccc
 *
 * AUTHORS
 *       Sven Herzberg        <herzi@gnome-de.org>
 *
 * Copyright (C) 2005,2007 Sven Herzberg
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

#include <ccc/cc-color.h>

#include <ccc/cc-brush.h>
#include <ccc/cc-utils.h>

static void color_implement_brush (CcBrushIface* iface);
G_DEFINE_TYPE_WITH_CODE (CcColor, cc_color, G_TYPE_INITIALLY_UNOWNED,
			 G_IMPLEMENT_INTERFACE (CC_TYPE_BRUSH, color_implement_brush));

/**
 * cc_color_apply:
 * @self: a #CcColor
 * @red: target for red component
 * @green: target for green component
 * @blue: target for blue component
 * @alpha: target for alpha value
 *
 * Initializes @red, @green, @blue and @alpha to represent the color specified
 * by @self:
 */
void
cc_color_apply(CcColor const* self,
	       gdouble      * red,
	       gdouble      * green,
	       gdouble      * blue,
	       gdouble      * alpha)
{
	g_return_if_fail(CC_IS_COLOR(self));
	g_return_if_fail(red && green && blue && alpha);

	cc_return_if_unimplemented_code(CC_COLOR_GET_CLASS(self), apply,
					*red = *green = *blue = *alpha = 0.0);

	CC_COLOR_GET_CLASS(self)->apply(self, red, green, blue, alpha);
}

/**
 * cc_color_stop:
 * @self: a #CcColor
 * @pattern: a #cairo_pattern_t
 * @offset: [0.0..1.0]
 *
 * Specify a color stop in @pattern at @offset with the color of @self.
 */
void
cc_color_stop(CcColor const  * self,
	      cairo_pattern_t* pattern,
	      gdouble          offset)
{
	gdouble red,
		green,
		blue,
		alpha;

	g_return_if_fail(CC_IS_COLOR(self));
	g_return_if_fail(cairo_pattern_status(pattern) == CAIRO_STATUS_SUCCESS);

	cc_color_apply(self, &red, &green, &blue, &alpha);

	cairo_pattern_add_color_stop_rgba(pattern, offset,
					  red, green,
					  blue, alpha);
}

/* CcColor */
enum {
	PROP_0,
	PROP_ALPHA
};

static void
cc_color_init(CcColor* self) {}

static void
cc_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	CcColor* self = CC_COLOR(object);

	switch(prop_id) {
	case PROP_ALPHA:
		g_value_set_double(value, self->alpha);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cc_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	CcColor* self = CC_COLOR(object);

	switch(prop_id) {
	case PROP_ALPHA:
		self->alpha = g_value_get_double(value);
		g_object_notify(object, "alpha");
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
color_apply(CcColor const* self,
	    gdouble      * red,
	    gdouble      * green,
	    gdouble      * blue,
	    gdouble      * alpha)
{
	*alpha = self->alpha;
}

static void
cc_color_class_init(CcColorClass* self_class) {
	GObjectClass* go_class = G_OBJECT_CLASS(self_class);
	go_class->get_property = cc_get_property;
	go_class->set_property = cc_set_property;

	g_object_class_install_property(go_class,
					PROP_ALPHA,
					g_param_spec_double("alpha",
							    "Alpha",
							    "Alpha",
							    0.0, 1.0,
							    1.0,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	self_class->apply = color_apply;
}

static void
color_apply_brush (CcBrush* brush,
		   CcView * view  G_GNUC_UNUSED,
		   CcItem * item  G_GNUC_UNUSED,
		   cairo_t* cr)
{
	gdouble red, green, blue, alpha;
	cc_color_apply (CC_COLOR (brush), &red, &green, &blue, &alpha);
	cairo_set_source_rgba (cr, red, green, blue, alpha);
}

static void
color_implement_brush (CcBrushIface* iface)
{
	iface->apply = color_apply_brush;
}

