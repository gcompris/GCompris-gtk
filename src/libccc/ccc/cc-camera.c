/* This file is part of CCC, a cairo-based canvas
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

#include "cc-camera.h"

#include <ccc/cc-item-view.h>

struct CcCameraPrivate {
	CcItem* root;
};
#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE((i), CC_TYPE_CAMERA, struct CcCameraPrivate))

/* * FIXME
 * SECTION:camera
 *
 *
 */

CcItem*
cc_camera_new(void)
{
	return cc_camera_new_root(NULL);
}

CcItem*
cc_camera_new_root(CcItem* root)
{
	//g_warning("CcCamera is not implemented completely yet. Feel free to complete it.");
	return g_object_new(CC_TYPE_CAMERA, "root", root, NULL);
}

void
cc_camera_set_root(CcCamera* self,
		   CcItem  * root)
{
	g_return_if_fail(CC_IS_CAMERA(self));
	g_return_if_fail(!root || CC_IS_ITEM(root));

	if(P(self)->root == root) {
		return;
	}

	if(P(self)->root) {
		cc_item_view_unregister(CC_ITEM_VIEW(self), P(self)->root);
		g_object_unref(P(self)->root);
		P(self)->root = NULL;
	}

	if(root) {
		P(self)->root = g_object_ref(root);
		cc_item_view_register(CC_ITEM_VIEW(self), P(self)->root);
	}

	g_object_notify(G_OBJECT(self), "root");
}

CcItem*
cc_camera_get_root(CcItem const* self)
{
	return NULL;
}

/* GType */
enum {
	PROP_0,
	PROP_FOCUS,
	PROP_MODE,
	PROP_ROOT,
	PROP_SCROLL,
	PROP_ZOOM
};
static void camera_implement_view(CcViewIface* iface);
G_DEFINE_TYPE_WITH_CODE(CcCamera, cc_camera, CC_TYPE_ITEM,
			G_IMPLEMENT_INTERFACE(CC_TYPE_VIEW, camera_implement_view));

static void
cc_camera_init(CcCamera* self G_GNUC_UNUSED)
{}

static void
camera_dispose(GObject* object)
{
	if(P(object)->root) {
		cc_camera_set_root(CC_CAMERA(object), NULL);
	}

	if(G_OBJECT_CLASS(cc_camera_parent_class)->dispose) {
		G_OBJECT_CLASS(cc_camera_parent_class)->dispose(object);
	}
}

static void
camera_get_property()
{
	g_warning("FIXME: implement %s", __PRETTY_FUNCTION__);
}

static void
camera_set_zoom(CcCamera* self,
		gdouble   zoom)
{
	// FIXME: implement
}

static void
camera_set_property(GObject     * object,
		    guint         prop_id,
		    GValue const* value,
		    GParamSpec  * pspec)
{
	switch(prop_id) {
	case PROP_FOCUS:
		g_warning("Implement 'focus' for CcCamera");
		break;
	case PROP_MODE:
		g_warning("Implement 'zoom-mode' for CcCamera");
		break;
	case PROP_ROOT:
		cc_camera_set_root(CC_CAMERA(object), g_value_get_object(value));
		break;
	case PROP_SCROLL:
		g_warning("Implement 'scrolled-region' for CcCamera");
		break;
	case PROP_ZOOM:
		camera_set_zoom(CC_CAMERA(object), g_value_get_double(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
camera_render(CcItem * item,
	      CcView * view,
	      cairo_t* cr)
{
	cairo_surface_t* surface;
	cairo_t        * surface_cr;

	if(CC_ITEM_CLASS(cc_camera_parent_class)->render) {
		CC_ITEM_CLASS(cc_camera_parent_class)->render(item, view, cr);
	}

	if(!CC_IS_ITEM(P(item)->root)) {
		return;
	}

	surface = cairo_surface_create_similar(cairo_get_target(cr),
					       CAIRO_CONTENT_COLOR_ALPHA,
					       101, 101);
	surface_cr = cairo_create(surface);
	cairo_save(surface_cr);
	  cairo_rectangle(surface_cr, 0.0, 0.0, 101.0, 101.0);
	  cairo_set_source_rgba(surface_cr, 0.0, 0.0, 0.0, 0.5);
	  cairo_fill(surface_cr);
	  // FIXME: render the canvas
	  cc_item_render(P(item)->root, CC_VIEW(item), surface_cr);
	cairo_restore(surface_cr);
	cairo_destroy(surface_cr);

	cairo_save(cr);
	  cairo_set_source_surface(cr, surface, 0.0, 0.0);
	  cairo_rectangle(cr, 0.0, 0.0, 101.0, 101.0);
	  cairo_clip(cr);
	  cairo_paint_with_alpha(cr, 0.5);
	cairo_restore(cr);

	cairo_surface_destroy(surface);
}

static void
cc_camera_class_init(CcCameraClass* self_class)
{
	GObjectClass* object_class = G_OBJECT_CLASS(self_class);
	CcItemClass * item_class = CC_ITEM_CLASS(self_class);

	/* GObjectClass */
	object_class->dispose      = camera_dispose;
	object_class->get_property = camera_get_property;
	object_class->set_property = camera_set_property;

	_cc_view_install_property(object_class,
				  PROP_ROOT, PROP_SCROLL,
				  PROP_ZOOM, PROP_MODE,
				  PROP_FOCUS);

	/* CcItemClass */
	item_class->render = camera_render;

	/* CcCameraClass */
	g_type_class_add_private(self_class, sizeof(struct CcCameraPrivate));
}

/* CcViewIface */
static void
camera_init_matrix(cairo_matrix_t* matrix)
{
	cairo_matrix_init_translate(matrix, 0.0, 0.0);
	cairo_matrix_init_scale(matrix, 0.25, 0.25);
}

static void
camera_world_to_window(CcView * view,
		       gdouble* x,
		       gdouble* y)
{
	cairo_matrix_t matrix;
	gdouble real_x = x ? *x : 0.0,
		real_y = y ? *y : 0.0;

	camera_init_matrix(&matrix);
	cairo_matrix_transform_point(&matrix, &real_x, &real_y);

	if(x) {
		*x = real_x;
	}
	if(y) {
		*y = real_y;
	}
}

static void
camera_world_to_window_distance(CcView * view,
				gdouble* x,
				gdouble* y)
{
	cairo_matrix_t matrix;
	gdouble real_x = x ? *x : 0.0,
		real_y = y ? *y : 0.0;
	camera_init_matrix(&matrix);
	cairo_matrix_transform_distance(&matrix, &real_x, &real_y);
	if(x) {
		*x = real_x;
	}
	if(y) {
		*y = real_y;
	}
}

static void
camera_implement_view(CcViewIface* iface)
{
	iface->world_to_window          = camera_world_to_window;
	iface->world_to_window_distance = camera_world_to_window_distance;
}

