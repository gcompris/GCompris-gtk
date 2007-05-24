/* This file is part of libccc, criawips' cairo-based canvas
 *
 * AUTHORS
 *     Sven Herzberg  <herzi@gnome-de.org>
 *
 * Copyright (C) 2006  Sven Herzberg <herzi@gnome-de.org>
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

#include "cc-circle.h"

#include <math.h>

CcItem*
cc_circle_new(void) {
	return g_object_new(CC_TYPE_CIRCLE, NULL);
}

void
cc_circle_set_anchor(CcCircle* self, gdouble x, gdouble y) {
	g_return_if_fail(CC_IS_CIRCLE(self));

	if(self->x == x && self->y == y) {
		return;
	}

	self->x = x;
	self->y = y;

	cc_item_update_bounds(CC_ITEM(self), NULL);

	g_object_notify(G_OBJECT(self), "anchor");
}

void
cc_circle_set_radius(CcCircle* self, gdouble radius) {
	CcDRect dirty;
	CcItem* item;

	g_return_if_fail(CC_IS_CIRCLE(self));

	if(self->radius == radius) {
		return;
	}

	item = CC_ITEM(self);
	dirty.x1 = self->x - MAX(radius, self->radius);
	dirty.y1 = self->y - MAX(radius, self->radius);
	dirty.x2 = self->x + MAX(radius, self->radius);
	dirty.y2 = self->y + MAX(radius, self->radius);
	self->radius = radius;

	g_object_notify(G_OBJECT(self), "radius");
	cc_item_dirty(item, NULL, dirty);
	cc_item_update_bounds(item, NULL);
}

/* GType */
G_DEFINE_TYPE(CcCircle, cc_circle, CC_TYPE_SHAPE);

enum {
	PROP_O,
	PROP_ANCHOR,
	PROP_RADIUS
};

static void
cc_circle_init(CcCircle* self) {}

static void
cc_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	CcCircle* self = CC_CIRCLE(object);

	switch(prop_id) {
	case PROP_RADIUS:
		g_value_set_double(value, self->radius);
		break;
	case PROP_ANCHOR:
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cc_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	CcCircle* self = CC_CIRCLE(object);

	switch(prop_id) {
	case PROP_RADIUS:
		cc_circle_set_radius(self, g_value_get_double(value));
		break;
	case PROP_ANCHOR:
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gdouble
cc_distance(CcItem* item, gdouble x, gdouble y, CcItem** found) {
	gdouble   distance = 0.0;
	CcCircle* self = NULL;

	// search the children first
	distance = CC_ITEM_CLASS(cc_circle_parent_class)->distance(item, x, y, found);

	if(*found) {
		return distance;
	}

	self = CC_CIRCLE(item);
	distance = sqrt(pow(x - self->x, 2.0) + pow(y - self->y, 2));

	if(distance <= self->radius + 0.5 * CC_SHAPE(self)->width) {
		distance = 0.0;
		*found = item;
	} else {
		distance -= self->radius;
	}

	return distance;
}

static void
cc_update_bounds(CcItem* item, CcView const* view, gpointer data) {
	CcCircle* self = CC_CIRCLE(item);
	gdouble w_2 = 0.5 * CC_SHAPE(self)->width;
	CcDRect new_bounds = {
		self->x - self->radius - w_2,
		self->y - self->radius - w_2,
		self->x + self->radius + w_2,
		self->y + self->radius + w_2
	};
	CcDRect* bounds = cc_hash_map_lookup(item->bounds, view);

	if(!bounds || !cc_d_rect_equal(*bounds, new_bounds)) {
		if(bounds) {
			cc_item_dirty(item, view, *bounds);
			cc_hash_map_remove(item->bounds, view);
		}

		cc_hash_map_insert(item->bounds, (gpointer)view, cc_d_rect_copy(&new_bounds));
		cc_item_dirty(item, view, new_bounds);
		cc_item_bounds_changed(item, view);
	}
}

static void
cc_path(CcShape* shape, CcView* view, cairo_t* cr) {
	CcCircle* self = CC_CIRCLE(shape);
	gdouble x = self->x,
		y = self->y,
		r = self->radius,
		tmp = 0.0;
	cc_view_world_to_window(view, &x, &y);
	cc_view_world_to_window_distance(view, &r, &tmp);
	cairo_arc(cr, x, y, r, 0.0, 2*G_PI);
}

static void
cc_circle_class_init(CcCircleClass* self_class) {
	GObjectClass* go_class;
	CcItemClass * ci_class;
	CcShapeClass* cs_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->get_property = cc_get_property;
	go_class->set_property = cc_set_property;

	g_object_class_install_property(go_class,
					PROP_ANCHOR,
					g_param_spec_pointer("anchor",
							     "Anchor",
							     "The center of the circle",
							     0));
	g_object_class_install_property(go_class,
					PROP_RADIUS,
					g_param_spec_double("radius",
							    "Radius",
							    "Radius",
							    0.0,
							    G_MAXDOUBLE,
							    0.0,
							    G_PARAM_READWRITE));

	/* CcItemClass */
	ci_class = CC_ITEM_CLASS(self_class);
	ci_class->distance      = cc_distance;
	ci_class->update_bounds = cc_update_bounds;

	/* CcShapeClass */
	cs_class = CC_SHAPE_CLASS(self_class);
	cs_class->path = cc_path;
}

