/* This file is part of CCC
 *
 * AUTHORS
 *     Sven Herzberg  <herzi@gnome-de.org>
 *
 * Copyright (C) 2006  Sven Herzberg
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

#include "cc-radial-gradient.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <glib/gi18n-lib.h>

struct CcRadialGradientPrivate {
	gdouble x;
	gdouble y;
	gdouble radius;
};
#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE((i), CC_TYPE_RADIAL_GRADIENT, struct CcRadialGradientPrivate))

/**
 * cc_radial_gradient_new:
 * @x: the x position of the gradient's center
 * @y: the y position of the gradient's center
 * @radius: the radius of the gradient's circle
 *
 * Create a new radial gradient.
 *
 * @x, @y and @radius are given in a relation to the item's size and position:
 * eg. 0.5 means either "in the middle" (for @x and @y) or "half the size" (for
 * @radius).
 *
 * Returns a new #CcRadialGradient.
 */
CcBrush*
cc_radial_gradient_new(gdouble x,
		       gdouble y,
		       gdouble radius)
{
	return g_object_new(CC_TYPE_RADIAL_GRADIENT,
			    "x", x,
			    "y", y,
			    "radius", radius,
			    NULL);
}

/* GType */
G_DEFINE_TYPE(CcRadialGradient, cc_radial_gradient, CC_TYPE_GRADIENT);

enum {
	PROP_0,
	PROP_POS_X,
	PROP_POS_Y,
	PROP_RADIUS
};

static void
radial_gradient_get_property(GObject   * object,
			     guint       prop_id,
			     GValue    * value,
			     GParamSpec* pspec)
{
	switch(prop_id) {
	case PROP_POS_X:
		g_value_set_double(value, P(object)->x);
		break;
	case PROP_POS_Y:
		g_value_set_double(value, P(object)->y);
		break;
	case PROP_RADIUS:
		g_value_set_double(value, P(object)->radius);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
radial_gradient_set_property(GObject     * object,
			     guint         prop_id,
			     GValue const* value,
			     GParamSpec  * pspec)
{
	switch(prop_id) {
	case PROP_POS_X:
		P(object)->x = g_value_get_double(value);
		g_object_notify(object, "x");
		break;
	case PROP_POS_Y:
		P(object)->y = g_value_get_double(value);
		g_object_notify(object, "y");
		break;
	case PROP_RADIUS:
		P(object)->radius = g_value_get_double(value);
		g_object_notify(object, "radius");
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
cc_radial_gradient_init(CcRadialGradient* self)
{
	P(self)->x = 0.5;
	P(self)->y = 0.5;
	P(self)->radius = 0.5;
}

static cairo_pattern_t*
radial_gradient_create_pattern(CcGradient const* gradient,
			       CcView const    * view,
			       CcItem const    * item)
{
	CcDRect const* bounds = cc_hash_map_lookup(item->bounds, view);
	CcDRect nul = {0.0, 0.0, 0.0, 0.0};

	if(G_LIKELY(bounds)) {
		nul = *bounds;
	}

	cc_view_world_to_window(view, &nul.x1, &nul.y1);
	cc_view_world_to_window(view, &nul.x2, &nul.y2);

	nul.x2 -= nul.x1 + 1.0;
	nul.y2 -= nul.y1 + 1.0;

	return cairo_pattern_create_radial(nul.x1 + P(gradient)->x * nul.x2,
					   nul.y1 + P(gradient)->y * nul.y2,
					   0.0,
					   nul.x1 + P(gradient)->x * nul.x2,
					   nul.y1 + P(gradient)->y * nul.y2,
					   P(gradient)->radius * MAX(nul.x2, nul.y2));
}

static void
cc_radial_gradient_class_init(CcRadialGradientClass* self_class)
{
	GObjectClass* object_class = G_OBJECT_CLASS(self_class);
	CcGradientClass* gradient_class = CC_GRADIENT_CLASS(self_class);

	/* GObjectClass */
	object_class->get_property = radial_gradient_get_property;
	object_class->set_property = radial_gradient_set_property;

	g_object_class_install_property(object_class,
					PROP_POS_X,
					g_param_spec_double("x",
							    _("X Position"),
							    _("Horizontal position of the gradient center"),
							    -G_MAXDOUBLE, G_MAXDOUBLE, 0.5,
							    G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_POS_Y,
					g_param_spec_double("y",
							    _("Y Position"),
							    _("Vertical position of the gradient center"),
							    -G_MAXDOUBLE, G_MAXDOUBLE, 0.5,
							    G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_RADIUS,
					g_param_spec_double("radius",
							    _("Radius"),
							    _("Gradient Radius"),
							    -G_MAXDOUBLE, G_MAXDOUBLE, 0.5,
							    G_PARAM_READWRITE));

	/* CcGradientClass */
	gradient_class->create_pattern = radial_gradient_create_pattern;

	/* CcRadialGradientClass */
	g_type_class_add_private(self_class, sizeof(struct CcRadialGradientPrivate));
}

