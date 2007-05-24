/* this file is part of libccc, criawips' cairo-based canvas
 *
 * AUTHORS
 *       Sven Herzberg        <herzi@gnome-de.org>
 *
 * Copyright (C) 2005,2006 Sven Herzberg
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

#include <ccc/cc-rounded-rectangle.h>

#include <math.h>
#include <ccc/cc-utils.h>

struct CcRoundedRectanglePrivate {
	gdouble radius;
};
#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE((i), CC_TYPE_ROUNDED_RECTANGLE, struct CcRoundedRectanglePrivate))

CcItem*
cc_rounded_rectangle_new(void)
{
	return g_object_new(CC_TYPE_ROUNDED_RECTANGLE, NULL);
}

/* GType stuff */
G_DEFINE_TYPE(CcRoundedRectangle, cc_rounded_rectangle, CC_TYPE_RECTANGLE);

enum {
	PROP_0,
	PROP_RADIUS
};

static void
cc_rounded_rectangle_init(CcRoundedRectangle* self) {
	P(self)->radius = 5.0;
}

static void
crr_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	CcRoundedRectangle* self = CC_ROUNDED_RECTANGLE(object);

	switch(prop_id) {
	case PROP_RADIUS:
		g_value_set_double(value, P(self)->radius);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
crr_dirty_corners(CcRectangle* self, CcView* view, gdouble* old_radius) {
	CcDRect dirty = {self->x, self->y, self->x + self->w, self->y + self->h};
	cc_item_dirty(CC_ITEM(self), view, dirty);
}

void
cc_rounded_rectangle_set_radius(CcRoundedRectangle* self,
				gdouble             radius) {
	CcItem * item;
	gdouble old_radius;
	if(P(self)->radius == radius) {
		return;
	}

	old_radius = P(self)->radius;
	P(self)->radius = radius;

	item = CC_ITEM(self);
	cc_item_foreach_view(item, CC_ITEM_FUNC(crr_dirty_corners), &old_radius);

	g_object_notify(G_OBJECT(self), "corner-radius");
}

static void
crr_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	CcRoundedRectangle* self = CC_ROUNDED_RECTANGLE(object);

	switch(prop_id) {
	case PROP_RADIUS:
		cc_rounded_rectangle_set_radius(self, g_value_get_double(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
crr_path(CcShape* shape, CcView* view, cairo_t* cr) {
	CcRectangle* rect = CC_RECTANGLE(shape);
	CcRoundedRectangle* self = CC_ROUNDED_RECTANGLE(shape);
	gdouble width  = cc_shape_get_width(shape, view);
	gdouble radius = P(self)->radius;
	gdouble points[][2] = {
		{rect->x + radius, rect->y + rect->h - radius},
		{rect->x + rect->w - radius, rect->y + radius}
	};
	guint i;

	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

	cc_view_world_to_window_distance(view, &radius, NULL);
	if(CC_ITEM_GRID_ALIGNED(shape)) {
		cc_point_grid_align(NULL, NULL, &radius);
	}

	for(i = 0; i < G_N_ELEMENTS(points); i++) {
		cc_view_world_to_window(view, &points[i][0], &points[i][1]);
		if(CC_ITEM_GRID_ALIGNED(shape)) {
			cc_point_grid_align(&points[i][0], &points[i][1], &width);
		}
	}

	cairo_arc(cr, points[1][0], points[1][1], radius, G_PI/-2.0, 0.0);
	cairo_arc(cr, points[1][0], points[0][1], radius, 0.0,       G_PI/2.0);
	cairo_arc(cr, points[0][0], points[0][1], radius, G_PI/2.0,  G_PI);
	cairo_arc(cr, points[0][0], points[1][1], radius, G_PI,      G_PI/-2.0);
	cairo_close_path(cr);
}

static void
cc_rounded_rectangle_class_init(CcRoundedRectangleClass* self_class) {
	GObjectClass* go_class;
	CcShapeClass* cs_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->get_property = crr_get_property;
	go_class->set_property = crr_set_property;

	g_object_class_install_property(go_class,
					PROP_RADIUS,
					g_param_spec_double("corner-radius",
							    "Corner Radius",
							    "The radius used for the rounded corners",
							    0.0, G_MAXDOUBLE,
							    5.0,
							    G_PARAM_READWRITE));

	// FIXME: override CcItem::distance

	/* CcShapeClass */
	cs_class = CC_SHAPE_CLASS(self_class);
	cs_class->path = crr_path;

	/* CcRoundedRectangleClass */
	g_type_class_add_private(self_class, sizeof(struct CcRoundedRectanglePrivate));
}

