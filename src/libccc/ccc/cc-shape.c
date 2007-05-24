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

#include <ccc/cc-shape.h>

#include <ccc/cc-utils.h>

typedef struct {
	CcDistance* width;
} CcShapePrivate;

#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE ((i), CC_TYPE_SHAPE, CcShapePrivate))

CcBrush*
cc_shape_get_brush_border(CcShape* self) {
	g_return_val_if_fail(CC_IS_SHAPE(self), NULL);

	return self->brush_border;
}

static void
cs_redraw_bounds(CcItem* item, CcView* view) {
	CcDRect* bounds;

	g_return_if_fail(CC_IS_ITEM(item));
	g_return_if_fail(CC_IS_VIEW(view));

	bounds = cc_hash_map_lookup(item->bounds, view);
	if(bounds) {
		cc_item_dirty(item, view, *bounds);
	}
}

void
cc_shape_set_brush_border(CcShape* self, CcBrush* brush) {
	CcItem* item;

	g_return_if_fail(CC_IS_SHAPE(self));
	g_return_if_fail(!brush || CC_IS_BRUSH(brush));

	if(self->brush_border == brush) {
		return;
	}

	item = CC_ITEM(self);

	if(self->brush_border) {
		g_object_unref(self->brush_border);
		self->brush_border = NULL;
	}

	if(brush) {
		self->brush_border = g_object_ref_sink(brush);
	}

	g_object_notify(G_OBJECT(self), "brush-border");
	cc_item_foreach_view(item, CC_ITEM_FUNC(cs_redraw_bounds), NULL);
}

CcBrush*
cc_shape_get_brush_content(CcShape* self) {
	g_return_val_if_fail(CC_IS_SHAPE(self), NULL);

	return self->brush_content;
}

void
cc_shape_set_brush_content(CcShape* self, CcBrush* brush) {
	CcItem* item;

	g_return_if_fail(CC_IS_SHAPE(self));
	g_return_if_fail(!brush || CC_IS_BRUSH(brush));

	if(self->brush_content == brush) {
		return;
	}

	item = CC_ITEM(self);

	if(self->brush_content) {
		g_object_unref(self->brush_content);
		self->brush_content = NULL;
	}

	if(brush) {
		self->brush_content = g_object_ref_sink(brush);
	}

	g_object_notify(G_OBJECT(self), "brush-content");
	cc_item_foreach_view(item, CC_ITEM_FUNC(cs_redraw_bounds), NULL);
}

gdouble
cc_shape_get_width(CcShape* self, CcView const* view) {
	gdouble retval;

	g_return_val_if_fail(CC_IS_SHAPE(self), 0.0);
	g_return_val_if_fail(CC_IS_VIEW(view), 0.0);

	retval = P(self)->width->value;

//#warning  "FIXME: convert with cc_distance_...() function"
	if(P(self)->width->unit != CC_UNIT_DISPLAY_PIXEL) {
		gdouble tmp = 0.0;
		cc_view_world_to_window_distance(view, &retval, &tmp);
	}

	return retval;
}

/**
 * cc_shape_set_line_width:
 * @self: a #CcShape
 * @width: a #CcDistance
 *
 * Specify the line width of a canvas item. @self will take the ownership of
 * @width.
 */
void
cc_shape_set_line_width (CcShape   * self,
			 CcDistance* width)
{
	CcShapePrivate* private;

	g_return_if_fail (CC_IS_SHAPE (self));
	g_return_if_fail (width);

	private = P(self);

	if (width == private->width) {
		return;
	}

	cc_distance_free (private->width);
	private->width = width;

	// FIXME: don't forget the dirty stuff

	g_object_notify (G_OBJECT (self), "width");
}

// FIXME: deprecated
void
cc_shape_set_width_pixels (CcShape* self,
			   gdouble  width)
{
	CcDistance* real_width = cc_distance_new (width, CC_UNIT_DISPLAY_PIXEL);

	cc_shape_set_line_width (self, real_width);
}

// FIXME: deprecated
void
cc_shape_set_width_units (CcShape* self,
			  gdouble  width)
{
	CcDistance* real_width = cc_distance_new (width, CC_UNIT_POINT);

	cc_shape_set_line_width (self, real_width);
}

/* GType stuff */
G_DEFINE_ABSTRACT_TYPE(CcShape, cc_shape, CC_TYPE_ITEM);

enum {
	PROP_0,
	PROP_BRUSH_BORDER,
	PROP_BRUSH_CONTENT,
	PROP_WIDTH
};

static void
cc_shape_init(CcShape* self) {
	P(self)->width = cc_distance_new (1.0, CC_UNIT_POINT);
}

static void
cs_dispose (GObject* object)
{
	CcShape* self = CC_SHAPE(object);

	if(self->brush_border) {
		g_object_unref(self->brush_border);
		self->brush_border = NULL;
	}

	if(self->brush_content) {
		g_object_unref(self->brush_content);
		self->brush_content = NULL;
	}

	G_OBJECT_CLASS(cc_shape_parent_class)->dispose(object);
}

static void
shape_finalize (GObject* object)
{
	CcShapePrivate* private = P(object);

	cc_distance_free (private->width);
	private->width = NULL;

	G_OBJECT_CLASS(cc_shape_parent_class)->finalize (object);
}

static void
cs_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	CcShape* self = CC_SHAPE(object);

	switch(prop_id) {
	case PROP_BRUSH_BORDER:
		g_value_set_object(value, cc_shape_get_brush_border(self));
		break;
	case PROP_BRUSH_CONTENT:
		g_value_set_object(value, cc_shape_get_brush_content(self));
		break;
	case PROP_WIDTH:
		g_value_set_boxed (value,
				   P(object)->width);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cs_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	CcShape* self = CC_SHAPE(object);

	switch(prop_id) {
	case PROP_BRUSH_BORDER:
		cc_shape_set_brush_border(self, g_value_get_object(value));
		break;
	case PROP_BRUSH_CONTENT:
		cc_shape_set_brush_content(self, g_value_get_object(value));
		break;
	case PROP_WIDTH:
		cc_shape_set_line_width (self, g_value_get_boxed (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cs_render(CcItem* item, CcView* view, cairo_t* cr) {
	CcShape* self = CC_SHAPE(item);

	g_return_if_fail(CC_SHAPE_GET_CLASS(self)->path);

	cairo_save(cr);
	  CC_SHAPE_GET_CLASS(self)->render_content(self, view, cr);

	  CC_ITEM_CLASS(cc_shape_parent_class)->render(item, view, cr);

	  CC_SHAPE_GET_CLASS(self)->render_border (self, view, cr);
	cairo_restore(cr);
}

static void
cs_render_border(CcShape* self, CcView* view, cairo_t* cr) {
	if(self->brush_border) {
		gdouble width = cc_shape_get_width(self, view);
		gdouble foo = 0.0, bar = 0.0;
		cc_point_grid_align(&foo, &bar, &width);

		cairo_save(cr);
		  cairo_new_path(cr);
		  CC_SHAPE_GET_CLASS(self)->path(self, view, cr);
		  cairo_set_line_width(cr, width);
		  cc_brush_apply(self->brush_border, view, CC_ITEM(self), cr);
		  cairo_stroke(cr);
		cairo_restore(cr);
	}
}

static void
cs_render_content(CcShape* self, CcView* view, cairo_t* cr) {
	if(self->brush_content) {
		cairo_save(cr);
		  cairo_new_path(cr);
		  CC_SHAPE_GET_CLASS(self)->path(self, view, cr);
		  cc_brush_apply(self->brush_content, view, CC_ITEM(self), cr);
		  cairo_fill(cr);
		cairo_restore(cr);
	}
}

static void
cc_shape_class_init(CcShapeClass* self_class) {
	GObjectClass* go_class;
	CcItemClass * ci_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->dispose      = cs_dispose;
	go_class->finalize     = shape_finalize;
	go_class->get_property = cs_get_property;
	go_class->set_property = cs_set_property;

	g_object_class_install_property(go_class,
					PROP_BRUSH_BORDER,
					g_param_spec_object("brush-border",
							    "Brush Border",
							    "The brush for drawing the border",
							    CC_TYPE_BRUSH,
							    G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_BRUSH_CONTENT,
					g_param_spec_object("brush-content",
							    "Brush Content",
							    "The brush for drawing the content",
							    CC_TYPE_BRUSH,
							    G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_WIDTH,
					g_param_spec_boxed("width",
							   "line width",
							   "The width of the outline",
							   CC_TYPE_DISTANCE,
							   G_PARAM_READWRITE));

	/* CcItemClass */
	ci_class = CC_ITEM_CLASS(self_class);
	ci_class->render = cs_render;

	/* CcShapeClass */
	self_class->render_content = cs_render_content;
	self_class->render_border  = cs_render_border;

	g_type_class_add_private (self_class, sizeof (CcShapePrivate));
}

