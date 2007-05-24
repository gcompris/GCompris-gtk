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

#include <ccc/cc-color.h>

G_DEFINE_TYPE(CcColor, cc_color, G_TYPE_INITIALLY_UNOWNED);

void
cc_color_apply(CcColor* self, cairo_t* cr) {
	g_return_if_fail(CC_IS_COLOR(self));
	g_return_if_fail(cr);

	g_return_if_fail(CC_COLOR_GET_CLASS(self)->apply);

	CC_COLOR_GET_CLASS(self)->apply(self, cr);
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
}

