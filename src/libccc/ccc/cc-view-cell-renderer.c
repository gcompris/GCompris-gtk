/* this file is part of libccc
 *
 * AUTHORS
 *       Sven Herzberg        <herzi@gnome-de.org>
 *
 * Copyright (C) 2005,2006,2007 Sven Herzberg
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

#include <ccc/cc-view-cell-renderer.h>

#include <ccc/cc-item-view.h>
#include <ccc/cc-view.h>

struct CcViewCellRendererPrivate {
	CcItem        * root;
	cairo_matrix_t  matrix; 
};
#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE((i), CC_TYPE_VIEW_CELL_RENDERER, struct CcViewCellRendererPrivate))

/**
 * cc_view_cell_renderer_new:
 *
 * Creates a #GtkCellRenderer that can be used to display a canvas in a
 * #GtkCellLayout (like #GtkTreeView or #GtkComboBox).
 *
 * Returns a new instance of #CcViewCellRenderer.
 */
GtkCellRenderer*
cc_view_cell_renderer_new(void) {
	return g_object_new(CC_TYPE_VIEW_CELL_RENDERER, NULL);
}

/* GType stuff */
static void cell_render_implement_item_view(CcItemViewIface* iface);
static void cell_renderer_implement_view(CcViewIface* iface);
G_DEFINE_TYPE_WITH_CODE(CcViewCellRenderer, cc_view_cell_renderer, GTK_TYPE_CELL_RENDERER,
			G_IMPLEMENT_INTERFACE(CC_TYPE_ITEM_VIEW, cell_render_implement_item_view);
			G_IMPLEMENT_INTERFACE(CC_TYPE_VIEW, cell_renderer_implement_view));

enum {
	PROP_0,
	PROP_FOCUS,
	PROP_ROOT,
	PROP_SCROLLED_REGION,
	PROP_ZOOM,
	PROP_ZOOM_MODE
};

static void
cc_view_cell_renderer_init(CcViewCellRenderer* self) {}

static void
cvcr_dispose (GObject* object)
{
	cc_view_set_root(CC_VIEW(object), NULL);

	G_OBJECT_CLASS(cc_view_cell_renderer_parent_class)->dispose(object);
}

static void
cvcr_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	CcViewCellRenderer* self = CC_VIEW_CELL_RENDERER(object);

	switch(prop_id) {
	case PROP_ROOT:
		g_value_set_object(value, P(self)->root);
		break;
	case PROP_ZOOM:
		g_value_set_double(value, self->zoom);
		break;
	case PROP_ZOOM_MODE:
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cvcr_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	CcViewCellRenderer* self = CC_VIEW_CELL_RENDERER(object);

	switch(prop_id) {
	case PROP_ROOT:
		if(P(self)->root == g_value_get_object(value)) {
			break;
		}
		if(P(self)->root) {
			cc_item_view_unregister(CC_ITEM_VIEW(self), P(self)->root);
			g_object_unref(P(self)->root);
			P(self)->root = NULL;
		}
		if(g_value_get_object(value)) {
			P(self)->root = g_object_ref(g_value_get_object(value));
			cc_item_view_register(CC_ITEM_VIEW(self), P(self)->root);
			cc_item_update_bounds_for_view(P(self)->root, CC_VIEW(self));
		}
		g_object_notify(object, "root");
		break;
	case PROP_ZOOM:
		// apply value
		// emit notify
		break;
	case PROP_ZOOM_MODE:
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cvcr_get_size(GtkCellRenderer* renderer,
	      GtkWidget      * widget,
	      GdkRectangle   * renderer_area,
	      gint           * x_offset,
	      gint           * y_offset,
	      gint           * width,
	      gint           * height)
{
	CcDRect const* bounds = cc_item_get_all_bounds(P(renderer)->root, CC_VIEW(renderer));

	if(!bounds) {
		return;
	}

	if(width) {
		*width = bounds->x2 - bounds->x1;
	}

	if(height) {
		*height = bounds->y2 - bounds->y1;
	}
}

static void
cvcr_render(GtkCellRenderer     * renderer,
	    GdkDrawable         * window,
	    GtkWidget           * widget,
	    GdkRectangle        * background_area,
	    GdkRectangle        * cell_area,
	    GdkRectangle        * expose_area,
	    GtkCellRendererState  flags)
{
	CcDRect const* bounds = cc_item_get_all_bounds(P(renderer)->root, CC_VIEW(renderer));
	cairo_t* cr = gdk_cairo_create(window);

	if(bounds) {
		gdk_cairo_rectangle(cr, expose_area);
		cairo_clip(cr);
		cairo_matrix_init_translate(&P(renderer)->matrix,
					    cell_area->x - bounds->x1,
					    cell_area->y - bounds->y1);
		cc_item_render(P(renderer)->root, CC_VIEW(renderer), cr);
	}
	cairo_destroy(cr);
}

static void
cc_view_cell_renderer_class_init(CcViewCellRendererClass* self_class) {
	GObjectClass        * go_class;
	GtkCellRendererClass* gcr_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->dispose      = cvcr_dispose;
	go_class->get_property = cvcr_get_property;
	go_class->set_property = cvcr_set_property;

	/* GtkCellRendererClass */
	gcr_class = GTK_CELL_RENDERER_CLASS(self_class);
	gcr_class->get_size = cvcr_get_size;
	gcr_class->render   = cvcr_render;

	/* CcViewIface */
	_cc_view_install_property(go_class, PROP_ROOT, PROP_SCROLLED_REGION, PROP_ZOOM, PROP_ZOOM_MODE, PROP_FOCUS);

	g_type_class_add_private(self_class, sizeof(struct CcViewCellRendererPrivate));
}

/* CcItemViewIface */
static void
cell_render_implement_item_view(CcItemViewIface* iface G_GNUC_UNUSED)
{}

/* CcViewIface */
static void
cell_renderer_world_to_window(CcView const* view,
			      gdouble     * x,
			      gdouble     * y)
{
	cairo_matrix_transform_point(&P(view)->matrix, x, y);
}

static void
cell_renderer_implement_view(CcViewIface* iface)
{
	iface->world_to_window = cell_renderer_world_to_window;
}
