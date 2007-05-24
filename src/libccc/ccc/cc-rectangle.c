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

#include <ccc/cc-rectangle.h>

#include <math.h>
#include <ccc/cc-utils.h>

#include "glib-helpers.h"

G_DEFINE_TYPE(CcRectangle, cc_rectangle, CC_TYPE_SHAPE);

CcItem*
cc_rectangle_new(void) {
	return g_object_new(CC_TYPE_RECTANGLE, NULL);
}

/**
 * cc_rectangle_set_position:
 * @self: a #CcRectangle
 * @x: the horizontal position
 * @y: the vertical position
 * @w: the width of the rectangle
 * @h: the eight of the rectangle
 *
 * Specify the position of a rectangle.
 */
void
cc_rectangle_set_position(CcRectangle* self, gdouble x, gdouble y, gdouble w, gdouble h) {
	g_object_set(self,
		     "position-x", x,
		     "position-y", y,
		     "position-w", w,
		     "position-h", h,
		     NULL);
}

void
cc_rectangle_set_x(CcRectangle* self, gdouble x) {
	g_return_if_fail(CC_IS_RECTANGLE(self));

	if(self->x == x) {
		return;
	}

	self->x = x;

	g_object_notify(G_OBJECT(self), "position-x");

	cc_item_update_bounds(CC_ITEM(self), NULL);
}

void
cc_rectangle_set_y(CcRectangle* self, gdouble y) {
	g_return_if_fail(CC_IS_RECTANGLE(self));

	if(self->y == y) {
		return;
	}

	self->y = y;

	g_object_notify(G_OBJECT(self), "position-y");

	cc_item_update_bounds(CC_ITEM(self), NULL);
}

void
cc_rectangle_set_width(CcRectangle* self, gdouble width) {
	g_return_if_fail(CC_IS_RECTANGLE(self));

	if(self->w == width) {
		return;
	}

	self->w = width;

	g_object_notify(G_OBJECT(self), "position-w");

	cc_item_update_bounds(CC_ITEM(self), NULL);
}

void
cc_rectangle_set_height(CcRectangle* self, gdouble height) {
	g_return_if_fail(CC_IS_RECTANGLE(self));

	if(self->h == height) {
		return;
	}

	self->h = height;

	g_object_notify(G_OBJECT(self), "position-h");

	cc_item_update_bounds(CC_ITEM(self), NULL);
}

/* GType stuff */
enum {
	PROP_0,
	PROP_POS_X,
	PROP_POS_Y,
	PROP_POS_W,
	PROP_POS_H
};

static void
cc_rectangle_init(CcRectangle* self) {}

static void
cr_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	CcRectangle* self = CC_RECTANGLE(object);

	switch(prop_id) {
	case PROP_POS_X:
		g_value_set_double(value, self->x);
		break;
	case PROP_POS_Y:
		g_value_set_double(value, self->y);
		break;
	case PROP_POS_W:
		g_value_set_double(value, self->w);
		break;
	case PROP_POS_H:
		g_value_set_double(value, self->h);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cr_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	CcRectangle* self = CC_RECTANGLE(object);

	switch(prop_id) {
	case PROP_POS_X:
		cc_rectangle_set_x(self, g_value_get_double(value));
		break;
	case PROP_POS_Y:
		cc_rectangle_set_y(self, g_value_get_double(value));
		break;
	case PROP_POS_W:
		cc_rectangle_set_width(self, g_value_get_double(value));
		break;
	case PROP_POS_H:
		cc_rectangle_set_height(self, g_value_get_double(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gdouble
cr_distance(CcItem      * item,
	    CcView const* view,
	    gdouble       x,
	    gdouble       y,
	    CcItem      **found)
{
	CcRectangle* self = CC_RECTANGLE(item);
	gdouble      distance = G_MAXDOUBLE;

	g_return_val_if_fail(found, distance);
	g_return_val_if_fail(!CC_IS_ITEM(*found), distance);

	// search the children first
	distance = CC_ITEM_CLASS(cc_rectangle_parent_class)->distance(item, view, x, y, found);

	if(CC_IS_ITEM(*found)) {
		return distance;
	}

	if (!CC_SHAPE(self)->brush_content && !CC_SHAPE(self)->brush_border)
		return MIN(distance, G_MAXDOUBLE);

	gdouble half_width = cc_shape_get_width (CC_SHAPE (self), view);
	gdouble delta_width = CC_SHAPE(self)->brush_border ? half_width : 0;

	if(( self->x - delta_width <= x )  && ( x <= self->x + self->w +  + delta_width ) &&
	   ( self->y - delta_width <= y && y <= self->y + self->h + delta_width )) {
		if (CC_SHAPE(self)->brush_content) {
			distance = 0.0;
			if(found) {
				*found = item;
			}
		} else {
			/* border is present, but no content */
			gdouble new_dist = MAX(
					       MIN(
						   MIN(x - self->x - delta_width,
						       self->x + self->w - x - delta_width),
						   MIN(y - self->y - delta_width,
						       self->y + self->h - y - delta_width)),
					       0.0
					       );

			if (new_dist == 0.0) {
				if(found) {
					*found = item;
				}
			}
			distance = MIN(new_dist, distance);
			
		}
	} else {
		// calculate the distance
		if((self->x - delta_width <= x) && (x <= self->x + self->w + delta_width)) {
			// point differs only in y
			if(y < self->y) {
				distance = self->y - y - delta_width;
			} else {
				distance = y - self->y - self->h -delta_width;
			}
		} else if((self->y -delta_width) <= y && y <= (self->y + self->h + delta_width)) {
			// point differs only in x
			if(x < self->x) {
				distance = self->x - x - delta_width;
			} else {
				distance = x - self->x - self->w - delta_width;
			}
		} else {
			// point is completely out of the rect area
			gdouble new_dist = sqrt(pow(MAX(self->x - x - delta_width, x - self->x - self->w -delta_width), 2.0) +
						pow(MAX(self->y - y - delta_width, y - self->y - self->h - delta_width),2.0));
			distance = MIN(distance, new_dist);
		}
		
	}
	return distance;
}

static void
cr_update_bounds(CcItem* item, CcView const* view, gpointer data) {
	// check whether the bounds have changed, if so, emit a signal
	CcRectangle* self = CC_RECTANGLE(item);
	gdouble half_width = cc_shape_get_width(CC_SHAPE(item), view) / 2.0;
	CcDRect drect = {
		self->x - half_width, self->y - half_width,
		self->x + self->w + half_width, self->y + self->h + half_width
	};
	CcDRect* bounds = cc_hash_map_lookup(item->bounds, view);

	if(!bounds || !cc_d_rect_equal(*bounds, drect)) {
		if(bounds) {
			cc_item_dirty(item, view, *bounds);
			cc_hash_map_remove(item->bounds, view);
		}

		cc_hash_map_insert(item->bounds, (gpointer)view, cc_d_rect_copy(&drect));
		cc_item_dirty(item, view, drect);
		cc_item_bounds_changed(item, view);
	}
}

static void
cr_path(CcShape* shape, CcView* view, cairo_t* cr) {
	CcRectangle* self = CC_RECTANGLE(shape);
	gdouble x1 = self->x,
		x2 = self->x + self->w,
		y1 = self->y,
		y2 = self->y + self->h;
	gdouble width = cc_shape_get_width(shape, view);

	cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);

	cc_view_world_to_window(view, &x1, &y1);
	cc_view_world_to_window(view, &x2, &y2);

	if(CC_ITEM_GRID_ALIGNED(shape)) {
		cc_point_grid_align(&x1, &y1, &width);
		cc_point_grid_align(&x2, &y2, &width);
	}

	cairo_rectangle(cr, x1, y1, x2 - x1, y2 - y1);

}

static void
cc_rectangle_class_init(CcRectangleClass* self_class) {
	GObjectClass* go_class;
	CcItemClass * ci_class;
	CcShapeClass* cs_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->get_property = cr_get_property;
	go_class->set_property = cr_set_property;

	g_object_class_install_property(go_class,
					PROP_POS_X,
					g_param_spec_double("position-x",
							    "position x",
							    "The x position of the rectangle",
							    -G_MAXDOUBLE,
							    G_MAXDOUBLE,
							    0.0,
							    G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_POS_Y,
					g_param_spec_double("position-y",
							    "position y",
							    "The y position of the rectangle",
							    -G_MAXDOUBLE,
							    G_MAXDOUBLE,
							    0.0,
							    G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_POS_W,
					g_param_spec_double("position-w",
							    "position w",
							    "The width of the rectangle",
							    -G_MAXDOUBLE,
							    G_MAXDOUBLE,
							    0.0,
							    G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_POS_H,
					g_param_spec_double("position-h",
							    "position h",
							    "The height of the rectangle",
							    -G_MAXDOUBLE,
							    G_MAXDOUBLE,
							    0.0,
							    G_PARAM_READWRITE));

	/* CcItemClass */
	ci_class = CC_ITEM_CLASS(self_class);
	ci_class->distance      = cr_distance;
	ci_class->update_bounds = cr_update_bounds;

	/* CcShapeClass */
	cs_class = CC_SHAPE_CLASS(self_class);
	cs_class->path = cr_path;
}

