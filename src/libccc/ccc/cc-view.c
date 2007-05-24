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

#include <ccc/cc-view.h>

#include <ccc/cc-enumerations.h>
#include <ccc/cc-item-view.h>
#include <ccc/cc-utils.h>
#include "gobject-helpers.h"

CcItem*
cc_view_get_focus(CcView const* self) {
	CcItem* focus = NULL;
	g_return_val_if_fail(CC_IS_VIEW(self), NULL);

	g_object_get((CcView*)self, "focus", &focus, NULL);

	return focus;
}

/**
 * cc_view_get_root:
 * @self: a #CcView
 *
 * Get the model that's displayed by @self.
 *
 * Returns the #CcItem that's displayed by @self, may be %NULL.
 */
CcItem*
cc_view_get_root(CcView* self) {
	CcItem* retval = NULL;
	g_return_val_if_fail(CC_IS_VIEW(self), NULL);
	g_object_get(self, "root", &retval, NULL);
	return retval;
}

/**
 * cc_view_get_scrolled_region:
 * @self: a #CcView
 *
 * Get the scrolled region of this view.
 *
 * Returns the scrolled region of this view.
 */
CcDRect*
cc_view_get_scrolled_region(CcView* self) {
	CcDRect* retval;
	g_return_val_if_fail(CC_IS_VIEW(self), NULL);
	g_object_get(self, "scrolled-region", &retval, NULL);
	return retval;
}

/**
 * cc_view_get_zoom:
 * @self: a #CcView
 *
 * Get the zoom of the view.
 *
 * Returns the zoom of the view.
 */
gdouble
cc_view_get_zoom(CcView* self) {
	gdouble retval;
	g_return_val_if_fail(CC_IS_VIEW(self), 1.0);
	g_object_get(self,
		     "zoom", &retval,
		     NULL);
	return retval;
}

/**
 * cc_view_grab_item:
 * @self: a #CcView
 * @item: a #CcItem
 * @mask: a #GdkEventMask
 * @cursor: a #GdkCursor
 * @time: the time
 *
 * Grab the item.
 *
 * Returns %GDK_GRAB_NOT_VIEWABLE if the view cannot grab at all.
 */
gint
cc_view_grab_item(CcView* self, CcItem* item, GdkEventMask mask, GdkCursor* cursor, guint32 time) {
	if(!CC_VIEW_GET_CLASS(self)->grab_item) {
		return GDK_GRAB_NOT_VIEWABLE;
	}

	return CC_VIEW_GET_CLASS(self)->grab_item(self, item, mask, cursor, time);
}

void
cc_view_ungrab_item(CcView* self, CcItem* item, guint32 time) {
	if(CC_VIEW_GET_CLASS(self)->ungrab_item) {
		CC_VIEW_GET_CLASS(self)->ungrab_item(self, item, time);
	}
}

void
cc_view_set_focus(CcView* self, CcItem* item) {
	g_return_if_fail(CC_IS_VIEW(self));
	g_return_if_fail(!item || CC_IS_ITEM(item));

	g_object_set(self, "focus", item, NULL);
}

/**
 * cc_view_set_root:
 * @self: a #CcView
 * @root: a #CcItem
 *
 * Set @root as the model to be displayed in @self.
 */
void
cc_view_set_root(CcView* self, CcItem* new_root) {
	g_return_if_fail(CC_IS_VIEW(self));
	g_object_set(self, "root", new_root, NULL);
}

/**
 * cc_view_set_scrolled_region:
 * @self: a #CcView
 * @scrolled_region: a #CcItem
 *
 * Set @scrolled_region as the region to be displayed and/or reachable with
 * scrollbars.
 */
void
cc_view_set_scrolled_region(CcView* self, CcDRect* scrolled_region) {
	g_return_if_fail(CC_IS_VIEW(self));
	g_object_set(self, "scrolled-region", scrolled_region, NULL);
}

/**
 * cc_view_set_zoom:
 * @self: a #CcView
 * @zoom: a new zoom value
 *
 * Set the zoom of the canvas.
 */
void
cc_view_set_zoom(CcView* self, gdouble zoom) {
	g_return_if_fail(CC_IS_VIEW(self));
	g_object_set(self, "zoom", zoom, NULL);
}

/**
 * cc_view_window_to_world:
 * @self: a #CcView
 * @x: target for a world coordinate, may be %NULL
 * @y: target for a world coordinate, may be %NULL
 *
 * Convert @window_x and @window_y to @world_x and @world_y if given.
 */
void
cc_view_window_to_world(CcView* self, gdouble* x, gdouble* y) {
	g_return_if_fail(CC_VIEW_GET_CLASS(self)->window_to_world);

	CC_VIEW_GET_CLASS(self)->window_to_world(self, x, y);
}

/**
 * cc_view_world_to_window:
 * @self: a #CcView
 * @x: target for a window coordinate, may be %NULL
 * @y: target for a window coordinate, may be %NULL
 *
 * Convert @x and @y to window coordinates if given.
 */
void
cc_view_world_to_window(CcView* self, gdouble* x, gdouble* y) {
	cc_return_if_unimplemented(CC_VIEW_GET_CLASS(self), world_to_window);

	CC_VIEW_GET_CLASS(self)->world_to_window(self, x, y);
}

/**
 * cc_view_world_to_window_distance:
 * @self: a #CcView
 * @x: target for a window distance, may be %NULL
 * @y: target for a window distance, may be %NULL
 *
 * Convert @x and @y to window coordinates
 */
void
cc_view_world_to_window_distance(CcView* self, gdouble* x, gdouble* y) {
	g_return_if_fail(CC_VIEW_GET_CLASS(self)->world_to_window_distance);

	CC_VIEW_GET_CLASS(self)->world_to_window_distance(self, x, y);
}

/* GType stuff */
static void cc_view_iface_init(gpointer iface);
G_DEFINE_IFACE_EXTENDED(CcView, cc_view, G_TYPE_INTERFACE, cc_view_iface_init,
			g_type_interface_add_prerequisite(CC_TYPE_VIEW, CC_TYPE_ITEM_VIEW));

void
_cc_view_install_property(GObjectClass* go_class, guint root_id, guint scroll_id, guint zoom_id, guint mode_id, guint focus_id) {
	g_object_class_override_property(go_class, focus_id,  "focus");
	g_object_class_override_property(go_class, root_id,   "root");
	g_object_class_override_property(go_class, scroll_id, "scrolled-region");
	g_object_class_override_property(go_class, zoom_id,   "zoom");
	g_object_class_override_property(go_class, mode_id,   "zoom-mode");
}

static void
cc_view_iface_init(gpointer iface) {
	g_object_interface_install_property(iface,
					    g_param_spec_object("focus",
								"focus",
								"Focus",
								CC_TYPE_ITEM,
								G_PARAM_READWRITE));
	g_object_interface_install_property(iface,
					    g_param_spec_object("root",
								"Root Item",
								"The root element of this canvas",
								CC_TYPE_ITEM,
								G_PARAM_READWRITE));
	g_object_interface_install_property(iface,
					    g_param_spec_boxed("scrolled-region",
							       "scrolled-region",
							       "scrolled-region",
							       CC_TYPE_D_RECT,
							       G_PARAM_READWRITE));
	g_object_interface_install_property(iface,
					    g_param_spec_double("zoom",
								"Zoom",
								"Zoom",
								G_MINDOUBLE,
								G_MAXDOUBLE,
								1.0,
								G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_interface_install_property(iface,
					    g_param_spec_enum("zoom-mode",
							      "Zoom Mode",
							      "BLURB",
							      CC_TYPE_ZOOM_MODE,
							      CC_ZOOM_PIXELS,
							      G_PARAM_READWRITE));
}

