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

#define CC_TYPE_COLOR_RGB (cc_color_rgb_get_type())
#define CC_COLOR_RGB(i)   (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_COLOR_RGB, CcColorRgb))

struct CcColorRgbPrivate {
	gdouble  red;
	gdouble  green;
	gdouble  blue;
};
#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE((i), CC_TYPE_COLOR_RGB, struct CcColorRgbPrivate))

typedef CcColorClass CcColorRgbClass;

G_DEFINE_TYPE(CcColorRgb, cc_color_rgb, CC_TYPE_COLOR);

CcColor*
cc_color_new_rgb(gdouble red, gdouble green, gdouble blue) {
	return cc_color_new_rgba(red, green, blue, 1.0);
}

CcColor*
cc_color_new_rgba(gdouble red, gdouble green, gdouble blue, gdouble alpha) {
	return g_object_new(CC_TYPE_COLOR_RGB,
			    "red",   red,
			    "green", green,
			    "blue",  blue,
			    "alpha", alpha,
			    NULL);
}

/* GType stuff */
enum {
	PROP_0,
	PROP_BLUE,
	PROP_GREEN,
	PROP_RED
};

static void
cc_color_rgb_init(CcColorRgb* self) {}

static void
ccr_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	switch(prop_id) {
	case PROP_BLUE:
		g_value_set_double(value, P(object)->blue);
		break;
	case PROP_GREEN:
		g_value_set_double(value, P(object)->green);
		break;
	case PROP_RED:
		g_value_set_double(value, P(object)->red);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ccr_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	switch(prop_id) {
	case PROP_BLUE:
		P(object)->blue = g_value_get_double(value);
		g_object_notify(object, "blue");
		break;
	case PROP_GREEN:
		P(object)->green = g_value_get_double(value);
		g_object_notify(object, "green");
		break;
	case PROP_RED:
		P(object)->red = g_value_get_double(value);
		g_object_notify(object, "red");
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
color_rgb_apply(CcColor const* color,
		gdouble      * red,
		gdouble      * green,
		gdouble      * blue,
		gdouble      * alpha)
{
	*red = P(color)->red;
	*green = P(color)->green;
	*blue = P(color)->blue;

	CC_COLOR_CLASS(cc_color_rgb_parent_class)->apply(color, red, green, blue, alpha);
}

static void
cc_color_rgb_class_init(CcColorRgbClass* self_class) {
	GObjectClass* go_class;
	CcColorClass* cc_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->get_property = ccr_get_property;
	go_class->set_property = ccr_set_property;
	
	g_object_class_install_property(go_class,
					PROP_RED,
					g_param_spec_double("red",
							    "Red",
							    "Red",
							    0.0,
							    1.0,
							    1.0,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(go_class,
					PROP_GREEN,
					g_param_spec_double("green",
							    "Green",
							    "Green",
							    0.0,
							    1.0,
							    1.0,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(go_class,
					PROP_BLUE,
					g_param_spec_double("blue",
							    "Blue",
							    "Blue",
							    0.0,
							    1.0,
							    1.0,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	/* CcColorClass */
	cc_class = CC_COLOR_CLASS(self_class);
	cc_class->apply = color_rgb_apply;

	g_type_class_add_private(self_class, sizeof(struct CcColorRgbPrivate));
}

