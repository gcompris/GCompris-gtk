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

#define CC_TYPE_COLOR_HSV (cc_color_hsv_get_type())
#define CC_COLOR_HSV(i)   (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_COLOR_HSV, CcColorHsv))

/* CcColorHsv */
struct CcColorHsvPrivate {
	gdouble  hue;
	gdouble  value;
	gdouble  saturation;
};
#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE((i), CC_TYPE_COLOR_HSV, struct CcColorHsvPrivate))

typedef struct _CcColorClass CcColorHsvClass;

G_DEFINE_TYPE(CcColorHsv, cc_color_hsv, CC_TYPE_COLOR);

CcColor*
cc_color_new_hsv(gdouble hue, gdouble saturation, gdouble value) {
	return cc_color_new_hsva(hue, saturation, value, 1.0);
}

CcColor*
cc_color_new_hsva(gdouble hue, gdouble saturation, gdouble value, gdouble alpha) {
	return g_object_new(cc_color_hsv_get_type(),
			    "hue",        hue,
			    "value",      value,
			    "saturation", saturation,
			    "alpha",      alpha,
			    NULL);
}

/* GType stuff */
enum {
	PROP_0,
	PROP_HUE,
	PROP_SATURATION,
	PROP_VALUE
};

static void
cc_color_hsv_init(CcColorHsv* self) {}

static void
cch_get_property() {}

static void
cch_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	switch(prop_id) {
	case PROP_HUE:
		P(object)->hue = g_value_get_double(value);
		g_object_notify(object, "hue");
		break;
	case PROP_SATURATION:
		P(object)->saturation = g_value_get_double(value);
		g_object_notify(object, "saturation");
		break;
	case PROP_VALUE:
		P(object)->value = g_value_get_double(value);
		g_object_notify(object, "value");
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
color_hsv_apply(CcColor const* color,
		gdouble      * red,
		gdouble      * green,
		gdouble      * blue,
		gdouble      * alpha)
{
	if(P(color)->saturation == 0.0) {
		*red = *green = *blue = P(color)->value;
	} else {
		gdouble hue = P(color)->hue * 6.0,
			f, p, q, t;

		if(hue >= 6.0) hue = 0.0;

		f = hue - (int)hue;
		p = P(color)->value * (1.0 - P(color)->saturation);
		q = P(color)->value * (1.0 - P(color)->saturation * f);
		t = P(color)->value * (1.0 - P(color)->saturation * (1.0 - f));

		switch((int)hue) {
		case 0:
			*red = P(color)->value; *green = t; *blue = p;
			break;
		case 1:
			*red = q; *green = P(color)->value; *blue = p;
			break;
		case 2:
			*red = p; *green = P(color)->value; *blue = t;
			break;
		case 3:
			*red = p; *green = q; *blue = P(color)->value;
			break;
		case 4:
			*red = t; *green = p; *blue = P(color)->value;
			break;
		case 5:
			*red = P(color)->value; *green = p; *blue = q;
			break;
		default:
			g_warning("hue should not be bigger than 5");
			break;
		};
	}

	CC_COLOR_CLASS(cc_color_hsv_parent_class)->apply(color, red, green, blue, alpha);
}

static void
cc_color_hsv_class_init(CcColorHsvClass* self_class) {
	GObjectClass* go_class;
	CcColorClass* cc_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->get_property = cch_get_property;
	go_class->set_property = cch_set_property;

	g_object_class_install_property(go_class,
					PROP_HUE,
					g_param_spec_double("hue",
							    "Hue",
							    "Hue",
							    0.0, 1.0,
							    0.0,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(go_class,
					PROP_SATURATION,
					g_param_spec_double("saturation",
							    "Saturation",
							    "Saturation",
							    0.0, 1.0,
							    0.0,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(go_class,
					PROP_VALUE,
					g_param_spec_double("value",
							    "Value",
							    "Value",
							    0.0, 1.0,
							    1.0,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	/* CcColorClass */
	cc_class = CC_COLOR_CLASS(self_class);
	cc_class->apply = color_hsv_apply;

	/* CcColorHsvClass */
	g_type_class_add_private(self_class, sizeof(struct CcColorHsvPrivate));
}

