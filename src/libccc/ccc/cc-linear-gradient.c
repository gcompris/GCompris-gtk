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

#include "cc-linear-gradient.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <glib/gi18n-lib.h>

#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE((i), CC_TYPE_LINEAR_GRADIENT, CcDRect))

/**
 * SECTION:cc-linear-gradient
 *
 * Linear gradients are a very nice pattern to fill items with. To permit
 * gradients to be applied to different items, the API got designed a bit
 * tricky (but still easy to understand).
 *
 * Instead of working with exact coordinates on items, you specify two points
 * for the gradient (the start and the end) relative to the item's bounding
 * box.
 *
 * A gradient from (0.0; 0.0) to (1.0; 1.0) applied to a square will result in
 * a linear gradient from the north-western corner of the square to the
 * south-eastern corner.
 *
 * A gradient from (0.5; 0.0) to (0.5; 1.0) applied to a square will result in
 * a gradient from the top to the bottom.
 *
 * Of course, you are not limited to the bounding box of your item. A gradient
 * like this can also be applied: (-0.25; 0.0) to (0.5; 1.0).
 */

CcBrush*
cc_linear_gradient_new(gdouble x1,
		       gdouble y1,
		       gdouble x2,
		       gdouble y2)
{
	return g_object_new(CC_TYPE_LINEAR_GRADIENT,
			    "x1", x1, "y1", y1,
			    "x2", x2, "y2", y2,
			    NULL);
}

/* GType */
G_DEFINE_TYPE(CcLinearGradient, cc_linear_gradient, CC_TYPE_GRADIENT);

enum {
	PROP_0,
	PROP_POS_X1,
	PROP_POS_Y1,
	PROP_POS_X2,
	PROP_POS_Y2
};

static void
cc_linear_gradient_init(CcLinearGradient* self G_GNUC_UNUSED)
{
	P(self)->x1 = 0.5;
	P(self)->x2 = 0.5;
	P(self)->y1 = 0.0;
	P(self)->y2 = 1.0;
}

static void
linear_gradient_get_property(GObject   * object,
			     guint       prop_id,
			     GValue    * value,
			     GParamSpec* pspec)
{
	switch(prop_id) {
	case PROP_POS_X1:
		g_value_set_double(value, P(object)->x1);
		break;
	case PROP_POS_X2:
		g_value_set_double(value, P(object)->x2);
		break;
	case PROP_POS_Y1:
		g_value_set_double(value, P(object)->y1);
		break;
	case PROP_POS_Y2:
		g_value_set_double(value, P(object)->y2);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
linear_gradient_set_property(GObject     * object,
			     guint         prop_id,
			     GValue const* value,
			     GParamSpec  * pspec)
{
	switch(prop_id) {
	case PROP_POS_X1:
		P(object)->x1 = g_value_get_double(value);
		g_object_notify(object, "x1");
		break;
	case PROP_POS_X2:
		P(object)->x2 = g_value_get_double(value);
		g_object_notify(object, "x2");
		break;
	case PROP_POS_Y1:
		P(object)->y1 = g_value_get_double(value);
		g_object_notify(object, "y1");
		break;
	case PROP_POS_Y2:
		P(object)->y2 = g_value_get_double(value);
		g_object_notify(object, "y2");
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static cairo_pattern_t*
linear_gradient_create_pattern(CcGradient const* self,
			       CcView const    * view,
			       CcItem const    * item)
{
	CcDRect const* bounds = cc_hash_map_lookup(item->bounds, view);
	CcDRect        nul = {0.0, 0.0, 0.0, 0.0};

	if(G_LIKELY(bounds)) {
		nul = *bounds;
	}

	cc_view_world_to_window(view, &nul.x1, &nul.y1);
	cc_view_world_to_window(view, &nul.x2, &nul.y2);

	return cairo_pattern_create_linear(nul.x2 - (1.0-P(self)->x1)*(nul.x2 - nul.x1),
					   nul.y2 - (1.0-P(self)->y1)*(nul.y2 - nul.y1),
					   nul.x1 + P(self)->x2*(nul.x2 - nul.x1),
					   nul.y1 + P(self)->y2*(nul.y2 - nul.y1));
}

static void
cc_linear_gradient_class_init(CcLinearGradientClass* self_class)
{
	GObjectClass* object_class = G_OBJECT_CLASS(self_class);
	CcGradientClass* gradient_class = CC_GRADIENT_CLASS(self_class);

	object_class->get_property = linear_gradient_get_property;
	object_class->set_property = linear_gradient_set_property;

	g_object_class_install_property(object_class,
					PROP_POS_X1,
					g_param_spec_double("x1",
							    _("X1"),
							    _("The first horizontal point."),
							    -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
							    G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_POS_Y1,
					g_param_spec_double("y1",
							    _("Y1"),
							    _("The first vertical point."),
							    -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
							    G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_POS_X2,
					g_param_spec_double("x2",
							    _("X2"),
							    _("The second horizontal point."),
							    -G_MAXDOUBLE, G_MAXDOUBLE, 1.0,
							    G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_POS_Y2,
					g_param_spec_double("y2",
							    _("Y2"),
							    _("The second vertical point."),
							    -G_MAXDOUBLE, G_MAXDOUBLE, 1.0,
							    G_PARAM_READWRITE));

	/* CcGradientClass */
	gradient_class->create_pattern = linear_gradient_create_pattern;

	/* CcLinearGradientClass */
	g_type_class_add_private(self_class, sizeof(CcDRect));
}

