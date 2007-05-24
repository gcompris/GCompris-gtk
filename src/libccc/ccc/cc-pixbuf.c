/* This file is part of libccc
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

#include "cc-pixbuf.h"

#include <gdk/gdkcairo.h>
#include <ccc/cc-utils.h>

struct CcPixbufPrivate {
	gdouble    pos_x;
	gdouble    pos_y;

	gdouble    width;
	gdouble    height;

	GdkPixbuf* pixbuf;
	gdouble    rotation;

	gboolean   height_set : 1;
	gboolean   width_set : 1;
};
#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE((i), CC_TYPE_PIXBUF, struct CcPixbufPrivate))

CcItem*
cc_pixbuf_new(void) {
	return g_object_new(CC_TYPE_PIXBUF, NULL);
}

void
cc_pixbuf_set_pixbuf(CcPixbuf* self, GdkPixbuf* pixbuf) {
	g_return_if_fail(CC_IS_PIXBUF(self));
	g_return_if_fail(!pixbuf || GDK_IS_PIXBUF(pixbuf));

	if(pixbuf == P(self)->pixbuf) {
		return;
	}

	// FIXME: redraw on pixbuf changes

	if(P(self)->pixbuf) {
		g_object_unref(P(self)->pixbuf);
		P(self)->pixbuf = NULL;
	}

	if(pixbuf) {
		P(self)->pixbuf = g_object_ref(pixbuf);
	}

	cc_item_update_bounds(CC_ITEM(self), NULL);
	g_object_notify(G_OBJECT(self), "pixbuf");
}

void
cc_pixbuf_set_position_x(CcPixbuf* self,
			 gdouble   pos_x)
{
	g_return_if_fail(CC_IS_PIXBUF(self));

	if(P(self)->pos_x == pos_x) {
		return;
	}

	P(self)->pos_x = pos_x;

	cc_item_update_bounds(CC_ITEM(self), NULL);
	g_object_notify(G_OBJECT(self), "position-x");
}

void
cc_pixbuf_set_position_y(CcPixbuf* self,
			 gdouble   pos_y)
{
	g_return_if_fail(CC_IS_PIXBUF(self));

	if(P(self)->pos_y == pos_y) {
		return;
	}

	P(self)->pos_y = pos_y;

	cc_item_update_bounds(CC_ITEM(self), NULL);
	g_object_notify(G_OBJECT(self), "position-y");
}

void
cc_pixbuf_set_position(CcPixbuf* self, gdouble x, gdouble y, gdouble w, gdouble h) {
	g_return_if_fail(CC_IS_PIXBUF(self));

#warning "FIXME: complete..."
	g_object_set(self,
		     "position-x", x,
		     "position-y", y,
		     "position-w", w,
		     "position-h", h,
		     NULL);
}

gdouble
cc_pixbuf_get_rotation(CcPixbuf const* self)
{
	g_return_val_if_fail(CC_IS_PIXBUF(self), 0.0);

	return P(self)->rotation;
}

static void
pixbuf_set_dirty(CcItem  * item,
		 CcView  * view,
		 gpointer  unused G_GNUC_UNUSED)
{
	CcDRect* bbox = cc_item_get_all_bounds(item, view);
	if(bbox) {
		cc_item_dirty(item, view, *bbox);
	}
}

void
cc_pixbuf_set_rotation(CcPixbuf* self,
		       gdouble   rotation)
{
	g_return_if_fail(CC_IS_PIXBUF(self));

	if(P(self)->rotation == rotation) {
		return;
	}

	P(self)->rotation = rotation;

	cc_item_foreach_view(CC_ITEM(self), pixbuf_set_dirty, NULL);
	cc_item_update_bounds(CC_ITEM(self), NULL);
	g_object_notify(G_OBJECT(self), "rotation");
}

/* GType */
G_DEFINE_TYPE(CcPixbuf, cc_pixbuf, CC_TYPE_ITEM);

enum {
	PROP_0,
	PROP_PIXBUF,
	PROP_POS_X,
	PROP_POS_Y,
	PROP_POS_W,
	PROP_POS_W_SET,
	PROP_POS_H,
	PROP_POS_H_SET,
	PROP_ROTATION
};

static void
cc_pixbuf_init(CcPixbuf* self) {
}

static void
cp_dispose(GObject* object) {
	if(CC_ITEM_DISPOSED(object)) {
		return;
	}

	if(P(object)->pixbuf) {
		g_object_unref(P(object)->pixbuf);
		P(object)->pixbuf = NULL;
	}

	G_OBJECT_CLASS(cc_pixbuf_parent_class)->dispose(object);
}

static void
cp_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	CcPixbuf* self = CC_PIXBUF(object);

	switch(prop_id) {
	case PROP_PIXBUF:
		g_value_set_object(value, P(self)->pixbuf);
		break;
	case PROP_POS_X:
#warning "implement"
		break;
	case PROP_POS_Y:
#warning "implement"
		break;
	case PROP_POS_W:
		g_value_set_double(value, P(self)->width);
		break;
	case PROP_POS_W_SET:
#warning "implement"
		break;
	case PROP_POS_H:
		g_value_set_double(value, P(self)->height);
		break;
	case PROP_POS_H_SET:
#warning "implement"
		break;
	case PROP_ROTATION:
#warning "implement"
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
pixbuf_set_width_set(CcPixbuf* self, gboolean width_set)
{
	if(P(self)->width_set == width_set) {
		return;
	}

	P(self)->width_set = width_set;
	g_object_notify(G_OBJECT(self), "position-w-set");
}

static void
cp_set_width(CcPixbuf* self, gdouble width) {
	pixbuf_set_width_set(self, TRUE);

	if(width == P(self)->width) {
		return;
	}

	P(self)->width = width;

	cc_item_update_bounds(CC_ITEM(self), NULL);
	g_object_notify(G_OBJECT(self), "position-w");
}

static void
pixbuf_set_height_set(CcPixbuf* self, gboolean height_set)
{
	if(P(self)->height_set == height_set) {
		return;
	}

	P(self)->height_set = height_set;
	g_object_notify(G_OBJECT(self), "position-h-set");
}

static void
cp_set_height(CcPixbuf* self, gdouble height) {
	pixbuf_set_height_set(self, TRUE);

	if(height == P(self)->height) {
		return;
	}

	P(self)->height = height;

	cc_item_update_bounds(CC_ITEM(self), NULL);
	g_object_notify(G_OBJECT(self), "position-h");
}

static void
cp_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	CcPixbuf* self = CC_PIXBUF(object);

	switch(prop_id) {
	case PROP_PIXBUF:
		cc_pixbuf_set_pixbuf(self, g_value_get_object(value));
		break;
	case PROP_POS_X:
#warning "implement"
		break;
	case PROP_POS_Y:
#warning "implement"
		break;
	case PROP_POS_W:
		cp_set_width(self, g_value_get_double(value));
		break;
	case PROP_POS_W_SET:
#warning "implement"
		break;
	case PROP_POS_H:
		cp_set_height(self, g_value_get_double(value));
		break;
	case PROP_POS_H_SET:
#warning "implement"
		break;
	case PROP_ROTATION:
#warning "implement"
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cp_render(CcItem* item, CcView* view, cairo_t* cr) {
	CcPixbuf* self = CC_PIXBUF(item);

	if(P(self)->pixbuf) {
		gdouble width   = P(self)->width,
			height  = P(self)->height;
		gdouble scale_x = 1.0,
			scale_y = 1.0;
		gdouble x = P(self)->pos_x,
			y = P(self)->pos_y;
		CcDRect* bounds;
		if(!P(item)->width_set) {
			width = gdk_pixbuf_get_width(P(self)->pixbuf);
		}
		if(!P(item)->height_set) {
			height = gdk_pixbuf_get_width(P(self)->pixbuf);
		}
		cc_view_world_to_window(view, &x, &y);
		bounds = cc_d_rect_copy(cc_item_get_all_bounds(item, view));
		cc_view_world_to_window(view, &bounds->x1, &bounds->y1);
		cc_view_world_to_window(view, &bounds->x2, &bounds->y2);
		cc_view_world_to_window_distance(view, &width, &height);
		cairo_save(cr);
		cairo_rectangle(cr, bounds->x1, bounds->y1, bounds->x2 - bounds->x1, bounds->y2 - bounds->y1);
		cairo_translate(cr, x, y);
		cairo_translate(cr, width / 2, height / 2);
		cairo_rotate(cr, P(self)->rotation);
		scale_x = (width) / gdk_pixbuf_get_width(P(self)->pixbuf);
		scale_y = (height) / gdk_pixbuf_get_height(P(self)->pixbuf);
		cairo_scale(cr, scale_x, scale_y);
		gdk_cairo_set_source_pixbuf(cr, P(self)->pixbuf, -width/2/scale_x, -height/2/scale_y);
		cairo_fill(cr);
		g_free(bounds);
		cairo_restore(cr);
	}
}

static void
cp_update_bounds(CcItem* item, CcView const* view, gpointer user_data) {
	CcDRect  new_bounds_;
	CcPixbuf* self = CC_PIXBUF(item);
	CcDRect* new_bounds = NULL;
	CcDRect* bounds = cc_hash_map_lookup(item->bounds, view);

	if(P(self)->pixbuf) {
		new_bounds_.x1 = P(self)->pos_x;
		if(!P(self)->width_set) {
			new_bounds_.x2 = new_bounds_.x1 + gdk_pixbuf_get_width(P(self)->pixbuf);
		}
		else {
			new_bounds_.x2 = new_bounds_.x1 + P(item)->width;
		}
		new_bounds_.y1 = P(self)->pos_y;
		if(!P(self)->height_set) {
			new_bounds_.y2 = P(self)->pos_y + gdk_pixbuf_get_height(P(self)->pixbuf);
		}
		else {
			new_bounds_.y2 = P(self)->pos_y + P(item)->height;
		}
		new_bounds = &new_bounds_;

		if(P(self)->rotation != 0.0) {
			CcDRect temp;
			CcDRect temp2;
			cairo_matrix_t matrix;
			cairo_matrix_init_translate(&matrix, new_bounds_.x2 / 2, new_bounds_.y2 / 2);
			cairo_matrix_rotate(&matrix, P(self)->rotation);
			cairo_matrix_translate(&matrix, new_bounds_.x2 / -2, new_bounds_.y2 / -2);
			temp = new_bounds_;
			cairo_matrix_transform_point(&matrix, &temp.x1, &temp.y1);
			cairo_matrix_transform_point(&matrix, &temp.x2, &temp.y2);
			temp2 = new_bounds_;
			cairo_matrix_transform_point(&matrix, &temp2.x1, &temp2.y2);
			cairo_matrix_transform_point(&matrix, &temp2.x2, &temp2.y1);
			new_bounds_.x1 = MIN(MIN(temp.x1, temp.x2), MIN(temp2.x1, temp2.x2));
			new_bounds_.x2 = MAX(MAX(temp.x1, temp.x2), MAX(temp2.x1, temp2.x2));
			new_bounds_.y1 = MIN(MIN(temp.y1, temp.y2), MIN(temp2.y1, temp2.y2));
			new_bounds_.y2 = MAX(MAX(temp.y1, temp.y2), MAX(temp2.y1, temp2.y2));
		}
	}

	if(new_bounds != bounds && (!new_bounds || !bounds || !cc_d_rect_equal(*new_bounds, *bounds))) {
		if(bounds) {
			cc_item_dirty(item, view, *bounds);
			cc_hash_map_remove(item->bounds, view);
		}

		if(new_bounds) {
			cc_hash_map_insert(item->bounds, (gpointer)view, cc_d_rect_copy(new_bounds));
			cc_item_dirty(item, view, *new_bounds);
			cc_item_bounds_changed(item, view);
		}
	}
}

static void
cc_pixbuf_class_init(CcPixbufClass* self_class) {
	GObjectClass* go_class;
	CcItemClass * ci_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->dispose      = cp_dispose;
	go_class->get_property = cp_get_property;
	go_class->set_property = cp_set_property;

	g_object_class_install_property(go_class,
					PROP_PIXBUF,
					g_param_spec_object("pixbuf",
							    "pixbuf",
							    "pixbuf",
							    GDK_TYPE_PIXBUF,
							    G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_POS_X,
					g_param_spec_double("position-x",
							    "X Position",
							    "The horizontal position of the pixbuf",
							    -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
							    G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_POS_Y,
					g_param_spec_double("position-y",
							    "Y Position",
							    "The vertical position of the pixbuf",
							    -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
							    G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_POS_W,
					g_param_spec_double("position-w",
							    "position-w",
							    "The width of the image",
							    -G_MAXDOUBLE, G_MAXDOUBLE,
							    0.0,
							    G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_POS_W_SET,
					g_param_spec_boolean("position-w-set",
							     "position-w-set",
							     "Use the specified width instead of the image width",
							     FALSE,
							     G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_POS_H,
					g_param_spec_double("position-h",
							    "position-h",
							    "The height of the image",
							    -G_MAXDOUBLE, G_MAXDOUBLE,
							    0.0,
							    G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_POS_H_SET,
					g_param_spec_boolean("position-h-set",
							     "position-h-set",
							     "Use the specified height instead of the image height",
							     FALSE,
							     G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_ROTATION,
					g_param_spec_double("rotation",
							    "Rotation",
							    "The rotation of this image",
							    -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
							    G_PARAM_READWRITE));

	/* CcItemClass */
	ci_class = CC_ITEM_CLASS(self_class);
	// FIXME: implement distance
	ci_class->render        = cp_render;
	ci_class->update_bounds = cp_update_bounds;

	g_type_class_add_private(self_class, sizeof(struct CcPixbufPrivate));
}

