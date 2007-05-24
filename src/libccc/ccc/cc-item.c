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

#include <ccc/cc-item.h>

#include <string.h>
#include <gdk/gdk.h>
#include <gdk/gdkenumtypes.h>
#include <gtk/gtktypebuiltins.h>
#include <ccc/cc-marshallers.h>
#include <ccc/cc-item-view.h>
#include <ccc/cc-utils.h>
#define CDEBUG_TYPE cc_item_get_type
#include "glib-helpers.h"
#include "cdebug.h"

/* Copied from glib/trunk/gobject/gtype.c:
 * FIXME: report a bug against GLib to make these macros visible
 * --------------8<------------ */
/* The 2*sizeof(size_t) alignment here is borrowed from
 * GNU libc, so it should be good most everywhere.
 * It is more conservative than is needed on some 64-bit
 * platforms, but ia64 does require a 16-byte alignment.
 * The SIMD extensions for x86 and ppc32 would want a
 * larger alignment than this, but we don't need to
 * do better than malloc.
 */
#define STRUCT_ALIGNMENT (2 * sizeof (gsize))
#define ALIGN_STRUCT(offset) \
	      ((offset + (STRUCT_ALIGNMENT - 1)) & -STRUCT_ALIGNMENT)
/* --------------8<------------ */

typedef struct {
	CcHashMap* view_data;
} CcItemPrivate;

#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE ((i), CC_TYPE_ITEM, CcItemPrivate))

typedef struct {
	CcDRect* all_bounds;
	CcDRect* children_bounds;
} CcItemViewData;

#define V(i,v) (CC_ITEM_GET_VIEW_DATA ((i), (v), CC_TYPE_ITEM, CcItemViewData))

enum {
	ALL_BOUNDS_CHANGED,
	BUTTON_PRESS_EVENT,
	BUTTON_RELEASE_EVENT,
	ENTER_NOTIFY_EVENT,
	EVENT,
	FOCUS,
	FOCUS_ENTER,
	FOCUS_LEAVE,
	ITEM_DIRTY,
	KEY_PRESS_EVENT,
	KEY_RELEASE_EVENT,
	LEAVE_NOTIFY_EVENT,
	MOTION_NOTIFY_EVENT,

	ITEM_ADDED,
	ITEM_REMOVED,

	VIEW_REGISTER,
	VIEW_UNREGISTER,

	N_SIGNALS
};

static guint cc_item_signals[N_SIGNALS] = {0};

static void ci_init_item_view(CcItemViewIface* iface);

G_DEFINE_TYPE_WITH_CODE(CcItem, cc_item, G_TYPE_INITIALLY_UNOWNED,
			G_IMPLEMENT_INTERFACE(CC_TYPE_ITEM_VIEW, ci_init_item_view));

/**
 * CcItemFlags:
 * @CC_DISPOSED: specifies whether dispose has already been run on an item.
 * @CC_GRID_ALIGNED: specifies whether an item should be rendered to match an
 * existing grid.
 */

/**
 * CC_ITEM_FLAGS:
 * @i: a #CcItem
 *
 * Get the flags which are set on the #CcItem @i.
 *
 * Returns the mask of flags that are set on @i.
 */

/**
 * CC_ITEM_SET_FLAGS:
 * @i: a #CcItem
 * @m: a CcItemFlags mask
 *
 * Sets on @i the #CcItemFlags that have been specified in @m.
 */

/**
 * CC_ITEM_UNSET_FLAGS:
 * @i: a #CcItem
 * @m: a #CcItemFlags mask
 *
 * Unsets from @i the #CcItemFlags that have been specified in @m.
 */

/**
 * CC_ITEM_DISPOSED:
 * @i: a #CcItem
 *
 * Queries the #CcItem @i whether it has the %CC_DISPOSED flag set. This is
 * very useful in the dispose handler for the GObject derived types. This flag
 * gets set automatically if you chain up your dispose calls correctly.
 *
 * Returns %TRUE is the flag was set, %FALSE if not.
 */

/**
 * CC_ITEM_GRID_ALIGNED:
 * @i: a #CcItem
 *
 * Queries the #CcItem @i whether it has the %CC_GRID_ALIGNED flag set.
 *
 * Returns %TRUE if the flag was set, %FALSE if not.
 */

/**
 * cc_item_class_add_view_data:
 * @item_class: a #CcItemClass
 * @view_data_size: the size to be allocated for the view data
 *
 * Registers the size of view data for a certain item class. See also
 * <xref linkend="ccc-View-Specific-Data"/>.
 */
void
cc_item_class_add_view_data (gpointer item_class,
			     gsize    view_data_size)
{
	CcItemClass* self_class;

	g_return_if_fail (CC_IS_ITEM_CLASS (item_class));

	self_class = CC_ITEM_CLASS (item_class);

	if (G_OBJECT_CLASS_TYPE (self_class) != CC_TYPE_ITEM) {
		CcItemClass* parent_class = g_type_class_peek_parent (self_class);
		if (self_class->view_data_size != parent_class->view_data_size) {
			g_warning ("cc_item_class_add_view_data() called multiple times for the same type");
			return;
		}
	}

	self_class->view_data_size = ALIGN_STRUCT(self_class->view_data_size) + view_data_size;
}

/**
 * cc_item_new:
 *
 * Create a new #CcItem that can be used to group several items together.
 *
 * Returns a new #CcItem.
 */
CcItem*
cc_item_new(void) {
	return g_object_new(CC_TYPE_ITEM, NULL);
}

/**
 * cc_item_append:
 * @self: a #CcItem
 * @child: another #CcItem
 *
 * Insert an item at the end of this item. A #CcItem can serve as a container
 * for several items.
 */
void
cc_item_append(CcItem* self, CcItem* child) {
	cc_item_insert(self, child, -1);
}

/**
 * cc_item_add_view:
 * @self: a #CcItem
 * @view: a #CcView
 *
 * Registers a view at the item. The item and its children can now set up data
 * for this view.
 *
 * This function should not be called by application code. Use
 * cc_item_view_register() instead.
 */
void
cc_item_add_view (CcItem* self,
		  CcView* view)
{
	g_return_if_fail(CC_IS_ITEM(self));
	g_return_if_fail(CC_IS_VIEW(view));

	self->views = g_list_prepend(self->views, view);
	// FIXME: add a weak reference on the view, and then add a FIXME for
	// removal in _remove_view()
//#warning  "FIXME: register the memory for the view and implement quick-lookup for the last view that was looked for in CcHashMap"
	g_signal_emit(self, cc_item_signals[VIEW_REGISTER], 0, view);
}

/**
 * cc_item_remove_view:
 * @self: a #CcItem
 * @view: a #CcView
 *
 * Unregisters a view at the item. The item and the children can now free data
 * kept for this view.
 *
 * This function should not be called by application code. Use
 * cc_item_view_unregister() instead.
 */
void
cc_item_remove_view(CcItem* self, CcView* view) {
	g_return_if_fail(CC_IS_ITEM(self));
	g_return_if_fail(CC_IS_VIEW(view));

	self->views = g_list_remove(self->views, view);
	// FIXME: add an extra check to make sure that the view is definitely
	// not in the list anymore (and disable it with G_DISABLE_CHECKS)
	g_signal_emit(self, cc_item_signals[VIEW_UNREGISTER], 0, view);
}

static void
ci_update_all_bounds(CcView const* view, CcItem* self) {
	CcItemViewData* view_data = V(self, view);
	CcDRect const* bounds     = cc_hash_map_lookup(self->bounds, view);

	g_return_if_fail(CC_IS_VIEW(view));

	if(G_UNLIKELY(!bounds && !view_data->children_bounds && view_data->all_bounds)) {
		g_free (view_data->all_bounds);
		view_data->all_bounds = NULL;
		g_signal_emit(self, cc_item_signals[ALL_BOUNDS_CHANGED], 0, view, NULL);
	} else if(G_LIKELY(bounds || view_data->children_bounds)) {
		CcDRect new_bounds;
		if(bounds && view_data->children_bounds) {
			cc_d_rect_union(*bounds, *view_data->children_bounds, &new_bounds);
		} else if(bounds) {
			new_bounds = *bounds;
		} else {
			new_bounds = *view_data->children_bounds;
		}

		if(!view_data->all_bounds || !cc_d_rect_equal(*view_data->all_bounds, new_bounds)) {
			g_free (view_data->all_bounds);
			view_data->all_bounds = cc_d_rect_copy (&new_bounds);
			g_signal_emit(self, cc_item_signals[ALL_BOUNDS_CHANGED], 0, view, &new_bounds);
		}
	}
}

void
cc_item_bounds_changed(CcItem* self, CcView const* view) {
	g_return_if_fail(CC_IS_ITEM(self));
	g_return_if_fail(CC_IS_VIEW(view));

	ci_update_all_bounds(view, self);
}

static void
item_repaint_cb (CcItem* item,
		 CcView* view)
{
	CcDRect* bounds = cc_hash_map_lookup (item->bounds, view);

	if (bounds) {
		cc_item_dirty (item, view, *bounds);
	}
}

/**
 * cc_item_repaint:
 * @self: a #CcItem
 *
 * Request a redraw operation for all the views.
 */
void
cc_item_repaint (CcItem* item)
{
	cc_item_foreach_view (item, CC_ITEM_FUNC (item_repaint_cb), NULL);
}

/**
 * cc_item_dirty:
 * @self: a #CcItem
 * @view: a #CcView
 * @dirty_region: the region that needs to be redrawn
 *
 * Let emit a signal to tell @view that it needs to be redrawn.
 */
void
cc_item_dirty(CcItem* self, CcView const* view, CcDRect dirty_region) {
	g_signal_emit(self, cc_item_signals[ITEM_DIRTY], 0, view, &dirty_region);
}

/**
 * cc_item_distance:
 * @self: a #CcItem
 * @view: a #CcView
 * @x: the x coordinate to query for
 * @y: the y coordinate to query for
 * @found: the return location for a found #CcItem
 *
 * Calculates the distance from @self to (@x,@y). If (@x,@y) is within @self or
 * one of its children @found is set to the matching element.
 *
 * Returns the distance between @self (or one of its children) to (@x,@y).
 */
gdouble
cc_item_distance (CcItem      * self,
		  CcView const* view,
		  gdouble       x,
		  gdouble       y,
		  CcItem      **found)
{
	gdouble distance = 0.0;
	g_return_val_if_fail(CC_IS_ITEM(self), G_MAXDOUBLE);
	g_return_val_if_fail(CC_ITEM_GET_CLASS(self)->distance, G_MAXDOUBLE);
	g_return_val_if_fail(found, G_MAXDOUBLE);
	g_return_val_if_fail(!CC_IS_ITEM(*found), G_MAXDOUBLE);

	distance = CC_ITEM_GET_CLASS(self)->distance(self, view, x, y, found);

#ifndef G_DISABLE_CHECKS
	// enable this on experimental builds, but not on performance builds
	if(distance <= 0.0 && !CC_IS_ITEM(*found)) {
		g_warning("%sClass->distance() should set *found if the returned distance is <= 0.0",
			  G_OBJECT_TYPE_NAME(self));
		*found = self;
	} else if(distance > 0 && CC_IS_ITEM(*found)) {
		g_warning("%sClass->distance() should not set *found if distance is larger than 0.0",
			  G_OBJECT_TYPE_NAME(self));
		*found = NULL;
	}
#endif

	return distance;
}

static void
ci_foreach_view(CcView* view, gpointer self_func_and_data[3]) {
	CcItem   * self = self_func_and_data[0];
	CcItemFunc func = self_func_and_data[1];
	gpointer   data = self_func_and_data[2];

	func(self, view, data);
}

void
cc_item_foreach_view(CcItem* self, CcItemFunc func, gpointer data) {
	CcItem* item;
	gpointer self_func_and_data[] = {self, func, data};
	for(item = self; item; item = item->parent) {
		g_list_foreach(item->views, G_FUNC(ci_foreach_view), self_func_and_data);
	}
}

/**
 * cc_item_get_all_bounds:
 * @self: a #CcItem
 * @view: a #CcView
 *
 * Get the bounds of @self as it would e displayed by @view (including all of its children).
 *
 * Returns the bounds of the item, may be %NULL (in this case the item is not
 * tied visible area).
 */
CcDRect const*
cc_item_get_all_bounds(CcItem const* self, CcView const* view) {
	g_return_val_if_fail(CC_IS_ITEM(self), NULL);
	g_return_val_if_fail(CC_IS_VIEW(view), NULL);

	return V(self, view)->all_bounds;
}

/**
 * cc_item_get_view_data:
 * @self: a #CcItem
 * @type: the #GType of @self which should be searched for the view
 * @view: a #CcView
 *
 * Get a pointer to the view data for @view in @self. See also
 * <xref linkend="ccc-View-Specific-Data"/>.
 *
 * Returns a pointer to the requested memory.
 */
gpointer
cc_item_get_view_data (CcItem const* self,
		       CcView const* view,
		       GType         self_type)
{
	gpointer view_data = NULL;

	g_return_val_if_fail (CC_IS_ITEM (self), NULL);
	g_return_val_if_fail (CC_IS_VIEW (view), NULL);
	g_return_val_if_fail (g_type_is_a (self_type, CC_TYPE_ITEM), NULL);

	cdebug ("get_view_data()", "for %s in %s",
		G_OBJECT_TYPE_NAME (view),
		G_OBJECT_TYPE_NAME (self));
	view_data = cc_hash_map_lookup (P(self)->view_data, view);

	if (G_UNLIKELY (!view_data)) {
		g_warning ("Didn't find data for view %p (%s) in %p (%s).",
			   view, G_OBJECT_TYPE_NAME (view),
			   self, G_OBJECT_TYPE_NAME (self));
	} else if (self_type != CC_TYPE_ITEM) {
		CcItemClass* parent_class = g_type_class_peek_parent (CC_ITEM_GET_CLASS (self));
		view_data = ((gchar*)view_data) + ALIGN_STRUCT (parent_class->view_data_size);
	}
	return view_data;
}

void
cc_item_grab_focus(CcItem* self, CcView* view) {
	GdkEventFocus ev = {
		GDK_FOCUS_CHANGE,
		NULL,
		FALSE,
		FALSE
	};
	gboolean retval;
	CcItem* focus = cc_view_get_focus(view);

	if(focus) {
		g_signal_emit(focus, cc_item_signals[FOCUS_LEAVE], 0, view, &ev, &retval);
	}

	cc_view_set_focus(view, self);
	ev.in = TRUE;
	g_signal_emit(self, cc_item_signals[FOCUS_ENTER], 0, view, &ev, &retval);
}

static void
ci_add_parent_view_to_child(CcItem* self, CcView* view, gpointer data) {
	cc_item_view_register(CC_ITEM_VIEW(view), CC_ITEM(data));
}

/**
 * cc_item_insert:
 * @self: a #CcItem
 * @child: another #CcItem
 * @position: the position to insert the child at (0 means at the bottom,
 * bigger numbers mean higher layers, -1 means top layer)
 *
 * Inserts @child at @position into @self.
 */
void
cc_item_insert(CcItem* self, CcItem* child, gint position) {
	g_return_if_fail(CC_IS_ITEM(self));
	g_return_if_fail(CC_IS_ITEM(child));
	g_return_if_fail(!CC_IS_ITEM(child->parent));

	self->children = g_list_insert(self->children, g_object_ref_sink(child), position);
	child->parent = self;
	position = g_list_index(self->children, child);
	cc_item_foreach_view(self, ci_add_parent_view_to_child, child);

	// FIXME: let's have a test case that makes sure that the views are registered before
	// the item registers with the child
	cc_item_view_register(CC_ITEM_VIEW(self), child);

	g_signal_emit(self, cc_item_signals[ITEM_ADDED], 0, position, child);
}

/**
 * cc_item_is_child_of:
 * @child: a #CcItem
 * @parent: another #CcItem
 *
 * Find out whether @child is a child of @parent.
 *
 * Returns %TRUE is @child is a child of @parent.
 */
gboolean
cc_item_is_child_of(CcItem const* child, CcItem const* parent) {
	gboolean retval = FALSE;

	g_return_val_if_fail(CC_IS_ITEM(child), FALSE);
	g_return_val_if_fail(CC_IS_ITEM(parent), FALSE);

	for(child = child->parent; CC_IS_ITEM(child) ; child = child->parent) {
		if(child == parent) {
			retval = TRUE;
			break;
		}
	}

	return retval;
}

static void
ci_remove_parent_view_from_child(CcItem* self, CcView* view, gpointer data) {
	CcItem* child = CC_ITEM(data);

	cc_item_view_unregister(CC_ITEM_VIEW(view), child);
}

/**
 * cc_item_remove:
 * @self: a #CcItem
 * @child: another #CcItem
 *
 * Removes @child from @self. Does nothing if @child was not a child of @self
 * or if @child is a child of one of @self's children.
 */
void
cc_item_remove(CcItem* self, CcItem* child) {
	gint position;
	g_return_if_fail(CC_IS_ITEM(self) && CC_IS_ITEM(child));

	position = g_list_index(self->children, child);
	self->children = g_list_remove(self->children, child);
	cc_item_foreach_view(self, ci_remove_parent_view_from_child, child);
	cc_item_view_unregister(CC_ITEM_VIEW(self), child);

	g_signal_emit(self, cc_item_signals[ITEM_REMOVED], 0, position, child);
	g_object_unref(child);
}

/**
 * cc_item_render:
 * @self: a #CcItem
 * @view: a #Ccview
 * @cr: a cairo context (#cairo_t)
 *
 * Renders @self to @cr which is supposed to be a cairo context of @view as
 * the transformation matrix of @view will be used for rendering.
 */
void
cc_item_render(CcItem* self, CcView* view, cairo_t* cr) {
	g_return_if_fail(CC_IS_ITEM(self));
	g_return_if_fail(CC_IS_VIEW(view));
	g_return_if_fail(cr);

	g_return_if_fail(CC_ITEM_GET_CLASS(self)->render);

	CC_ITEM_GET_CLASS(self)->render(self, view, cr);
}

/**
 * cc_item_set_grid_aligned:
 * @self: a #CcItem
 * @grid_aligned: a gboolean
 *
 * Sets @self to be rendered grid-aligned if @grid_aligned is %TRUE, sets it to
 * be allowed to render fuzzyly if %FALSE.
 */
// FIXME: link to the grid align section of the reference
void
cc_item_set_grid_aligned(CcItem* self, gboolean grid_aligned) {
	if(grid_aligned) {
		CC_ITEM_SET_FLAGS(self, CC_GRID_ALIGNED);
	} else {
		CC_ITEM_UNSET_FLAGS(self, CC_GRID_ALIGNED);
	}
}

static void
ci_update_bounds_per_view(CcView* view, gpointer* self_and_data) {
	g_return_if_fail(CC_IS_VIEW(view));
	g_return_if_fail(CC_IS_ITEM(self_and_data[0]));

	if(CC_ITEM_GET_CLASS(self_and_data[0])->update_bounds) {
		CC_ITEM_GET_CLASS(self_and_data[0])->update_bounds(self_and_data[0], view, self_and_data[1]);
	}
}

void
cc_item_update_bounds(CcItem* self, gpointer data) {
	CcItem* item;
	gpointer self_and_data[2];

	g_return_if_fail(CC_IS_ITEM(self));

	self_and_data[0] = self;
	self_and_data[1] = data;
	for(item = self; item; item = item->parent) {
		g_list_foreach(item->views, G_FUNC(ci_update_bounds_per_view), self_and_data);
	}
}

void
cc_item_update_bounds_for_view(CcItem* self, CcView* view) {
	gpointer self_and_data[] = {
		self, NULL
	};
	ci_update_bounds_per_view(view, self_and_data);
}

static void
ci_update_view(CcItem* self, CcView* view, gpointer data) {
	CcDRect const *dirty_region = cc_item_get_all_bounds(self, view);
	g_return_if_fail(dirty_region != NULL);
	cc_item_dirty(self, view, *dirty_region);
}

/**
 * cc_item_set_position:
 * @child: a #CcItem
 * @parent: another #CcItem, parent of @child
 * @position: the new position of the #CcItem (0 means at the bottom,
 * bigger numbers mean higher layers, -1 means top layer)
 *
 * Changes the position of @child relative to @parent.
 */
void
cc_item_set_position(CcItem* child, CcItem* parent, gint position)
{
	g_return_if_fail(CC_IS_ITEM(child));
	g_return_if_fail(CC_IS_ITEM(parent));
	g_return_if_fail(cc_item_is_child_of(child, parent));

	parent->children = g_list_remove(parent->children, child);
	parent->children = g_list_insert(parent->children, child, position);
	cc_item_foreach_view(child, ci_update_view, NULL);
}

/**
 * cc_item_raise_to_top:
 * @child: a #CcItem
 * @parent: another #CcItem, parent of @child
 *
 * Raises an @child to the top. It will be displayed over all of the other
 * children of @parent.
 */
void
cc_item_raise_to_top(CcItem* child, CcItem* parent)
{
	g_return_if_fail(CC_IS_ITEM(child));
	g_return_if_fail(CC_IS_ITEM(parent));
	g_return_if_fail(cc_item_is_child_of(child, parent));

	if(child == CC_ITEM(g_list_last(parent->children)->data)) {
		return;
	}

	parent->children = g_list_remove(parent->children, child);
	parent->children = g_list_append(parent->children, child);
	cc_item_foreach_view(child, ci_update_view, NULL);
}

/**
 * cc_item_raise:
 * @child: a #CcItem
 * @parent: another #CcItem, parent of @child
 *
 * Raises @child by one level.
 */
void
cc_item_raise(CcItem* child, CcItem* parent)
{
	gint position;

	g_return_if_fail(CC_IS_ITEM(child));
	g_return_if_fail(CC_IS_ITEM(parent));
	g_return_if_fail(cc_item_is_child_of(child, parent));

	if(child == CC_ITEM(g_list_last(parent->children)->data)) {
		return;
	}

	position = g_list_index(parent->children, child);
	parent->children = g_list_remove(parent->children, child);
	parent->children = g_list_insert(parent->children, child, position+1);
	cc_item_foreach_view(child, ci_update_view, NULL);
}

/**
 * cc_item_lower:
 * @child: a #CcItem
 * @parent: another #CcItem, parent of @child
 *
 * Lowers @child by one level.
 */
void
cc_item_lower(CcItem* child, CcItem* parent)
{
	gint position;

	g_return_if_fail(CC_IS_ITEM(child));
	g_return_if_fail(CC_IS_ITEM(parent));
	g_return_if_fail(cc_item_is_child_of(child, parent));

	if(child == CC_ITEM(parent->children->data)) {
		return;
	}

	position = g_list_index(parent->children, child);
	parent->children = g_list_remove(parent->children, child);
	parent->children = g_list_insert(parent->children, child, position-1);
	cc_item_foreach_view(child, ci_update_view, NULL);
}

/**
 * cc_item_lower_to_bottom:
 * @child: a #CcItem
 * @parent: another #CcItem, parent of @child
 *
 * Lowers @child to be displayed behind all the other children of @parent.
 */
void
cc_item_lower_to_bottom(CcItem* child, CcItem* parent)
{
	g_return_if_fail(CC_IS_ITEM(child));
	g_return_if_fail(CC_IS_ITEM(parent));
	g_return_if_fail(cc_item_is_child_of(child, parent));

	if(child == CC_ITEM(parent->children->data)) {
		return;
	}

	parent->children = g_list_remove(parent->children, child);
	parent->children = g_list_prepend(parent->children, child);
	cc_item_foreach_view(child, ci_update_view, NULL);
}

/* GType stuff */
enum {
	PROP_0,
	PROP_ALL_BOUNDS,
	PROP_BOUNDS,
	PROP_CHILDREN_BOUNDS
};

static void
cc_item_init(CcItem* self) {
	self->parent = NULL;

	P(self)->view_data    = cc_hash_map_new (G_TYPE_POINTER);
	self->bounds          = cc_hash_map_new(CC_TYPE_D_RECT);
}

static void
ci_remove_swapped(CcItem* child, CcItem* self) {
	cc_item_remove(self, child);
}

static void
ci_dispose (GObject* object)
{
	CcItem* self = CC_ITEM (object);

	CC_ITEM_SET_FLAGS (self, CC_DISPOSED);

	g_list_foreach (self->children, G_FUNC (ci_remove_swapped), self);
	// self->children is NULL now

	if (P(self)->view_data) {
		g_object_unref (P(self)->view_data);
		P(self)->view_data = NULL;
	}

	if (self->bounds) {
		g_object_unref(self->bounds);
		self->bounds = NULL;
	}

	G_OBJECT_CLASS(cc_item_parent_class)->dispose (object);
}

static void
ci_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	switch(prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gdouble
item_distance(CcItem      * self,
	      CcView const* view,
	      gdouble       x,
	      gdouble       y,
	      CcItem      **found)
{
	gdouble  distance = G_MAXDOUBLE;
	GList  * child;
	CcItem * child_found = NULL;

	g_return_val_if_fail(CC_IS_ITEM(self), distance);
	g_return_val_if_fail(found && !CC_IS_ITEM(*found), distance);

	cdebug("distance()", "start on %p", self);

	for(child = g_list_last(self->children); child ; child = child->prev, child_found = NULL) {
		gdouble new_dist = cc_item_distance(CC_ITEM(child->data), view, x, y, &child_found);

		if(distance > new_dist) {
			distance = new_dist;
			if (child_found) {
				*found = child_found;
			}
		}
	}

	cdebug("distance()", "end: distance to %s (%p) is %f",
	       *found ? G_OBJECT_TYPE_NAME(*found) : "NULL",
	       *found, distance);
	return distance;
}

static gboolean
item_event(CcItem  * self,
	   CcView  * view,
	   GdkEvent* event)
{
	gboolean retval = FALSE;

	g_return_val_if_fail(CC_IS_ITEM(self), FALSE);
	g_return_val_if_fail(CC_IS_VIEW(view), FALSE);

	CDEBUG(GEnumClass* clss = g_type_class_ref(GDK_TYPE_EVENT_TYPE);
	       cdebug("event()", "%s (%p) received an event of type \"%s\": %p",
		      G_OBJECT_TYPE_NAME(self), self,
		      g_enum_get_value(clss, event->type)->value_name,
		      event);
	       g_type_class_unref(clss);
	);

	switch(event->type) {
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
		g_signal_emit(self, cc_item_signals[BUTTON_PRESS_EVENT], 0, view, event, &retval);
		break;
	case GDK_BUTTON_RELEASE:
		g_signal_emit(self, cc_item_signals[BUTTON_RELEASE_EVENT], 0, view, event, &retval);
		break;
	case GDK_MOTION_NOTIFY:
		g_signal_emit(self, cc_item_signals[MOTION_NOTIFY_EVENT], 0, view, event, &retval);
		break;
	case GDK_ENTER_NOTIFY:
		g_signal_emit(self, cc_item_signals[ENTER_NOTIFY_EVENT], 0, view, event, &retval);
		break;
	case GDK_LEAVE_NOTIFY:
		g_signal_emit(self, cc_item_signals[LEAVE_NOTIFY_EVENT], 0, view, event, &retval);
		break;
	case GDK_KEY_PRESS:
		g_signal_emit(self, cc_item_signals[KEY_PRESS_EVENT], 0, view, event, &retval);
		break;
	case GDK_KEY_RELEASE:
		g_signal_emit(self, cc_item_signals[KEY_RELEASE_EVENT], 0, view, event, &retval);
		break;
	case GDK_FOCUS_CHANGE:
		if(!event->focus_change.in) {
			g_signal_emit(self, cc_item_signals[FOCUS_LEAVE], 0, view, event, &retval);
		}
		break;
	default:
		{
			GEnumClass* klass = g_type_class_ref(GDK_TYPE_EVENT_TYPE);
			GEnumValue* value = g_enum_get_value(klass, event->type);
			g_type_class_unref(klass);

			g_message("CcItem::event(): got an unhandled '%s' event", value->value_name);
		}
		break;
	}

	if(!retval && self->parent) {
		// FIXME: make sure item->parent is displayed in view
		g_signal_emit(self->parent, cc_item_signals[EVENT], 0, view, event, &retval);
	}

	return retval;
}

static gboolean
ci_move_focus(CcItem* self, CcItem* last_child, CcView* view, GtkDirectionType dir) {
	GList* child = g_list_find(self->children, last_child);
	gboolean retval = FALSE;

	g_return_val_if_fail(child, FALSE);

	for(child = child->prev; !retval && child; child = child->prev) {
		g_signal_emit(child->data, cc_item_signals[FOCUS], 0, view, dir, &retval);
	}

	return retval;
}

static gboolean
ci_focus(CcItem* self, CcView* view, GtkDirectionType dir) {
	gboolean    retval = FALSE;

	if(CC_ITEM_CAN_FOCUS(self) && cc_view_get_focus(view) != self) {
		cc_item_grab_focus(self, view);
		retval = TRUE;
	} else {
		// try to move the focus into a child, if that fails, try
		// to move the focus from a parent item
		// => emit focus events into the children elements
		GList* child = g_list_last(self->children);
		for(child = g_list_last(self->children); !retval && child; child = child->prev) {
			g_signal_emit(child->data, cc_item_signals[FOCUS], 0, view, dir, &retval);
		}
	}

	if(!retval && self->parent) {
		//do {
			// tell our parent to move the focus on
			retval = ci_move_focus(self->parent, self, view, dir);
			self = self->parent;
		//} while(self->parent && !retval);
	}

	return retval;
}

static void
ci_render_child(CcItem* child, gpointer view_and_cr[2]) {
	cc_item_render(child, CC_VIEW(view_and_cr[0]), view_and_cr[1]);
}

static void
ci_render(CcItem* self, CcView* view, cairo_t* cr) {
	gpointer view_and_cr[2] = {
		view, cr
	};
	g_list_foreach(self->children, G_FUNC(ci_render_child), view_and_cr);
}

static void
ci_merge_child_bounds(CcItem* child, gpointer rect_and_view[2]) {
	CcDRect      * all_bounds = V(child, rect_and_view[1])->all_bounds;

	if (all_bounds) {
		CcDRect* rect = rect_and_view[0];
		cc_d_rect_union(*all_bounds, *rect, rect);
	}
}

static void
ci_notify_child_bounds_real(CcItem       * self,
			    CcItem       * child,
			    CcView       * view,
			    CcDRect const* bounds)
{
	CcItemViewData* view_data;
	GList   * it = self->children;

	view_data = V(self, view);

	if(G_UNLIKELY(!bounds && self->bounds)) {
		bounds = cc_hash_map_lookup(self->bounds, view);
	} else if(G_UNLIKELY(!bounds)) {
		for(it = self->children; it; it = it->next) {
			bounds = V(it->data, view)->all_bounds;
			if(bounds) {
				it = it->next;
				break;
			}
		}
	}

	if(G_LIKELY(bounds)) {
		CcDRect drect = *bounds;
		gpointer rect_and_view[] = {&drect, view};
		g_list_foreach(it, G_FUNC(ci_merge_child_bounds), rect_and_view);

		if(!view_data->children_bounds || !cc_d_rect_equal(*view_data->children_bounds, drect)) {
			g_free (view_data->children_bounds);
			view_data->children_bounds = cc_d_rect_copy(&drect);
			ci_update_all_bounds(view, self);
		}
	} else {
		g_free (view_data->children_bounds);
		view_data->children_bounds = NULL;
		ci_update_all_bounds(view, self);
	}
}

static void
ci_child_emit_view_register(CcItem* item, CcView* view) {
	g_signal_emit(item, cc_item_signals[VIEW_REGISTER], 0, view);
}

static void
ci_view_register(CcItem* self, CcView* view) {
	gpointer view_data = NULL;
	cdebug ("view_register()", "for %s in %s",
		G_OBJECT_TYPE_NAME (view),
		G_OBJECT_TYPE_NAME (self));
	view_data = g_slice_alloc0 (CC_ITEM_GET_CLASS(self)->view_data_size);
	cc_hash_map_insert (P(self)->view_data, view, view_data);
	g_list_foreach(self->children, G_FUNC(ci_child_emit_view_register), view);
}

static void
ci_child_emit_view_unregister(CcItem* item, CcView* view) {
	g_signal_emit(item, cc_item_signals[VIEW_UNREGISTER], 0, view);
}

static void
ci_view_unregister(CcItem* self, CcView* view) {
	CcItemViewData* view_data = V(self, view);
	g_list_foreach(self->children, G_FUNC(ci_child_emit_view_unregister), view);
	if(view_data->all_bounds) {
		cc_item_dirty(self, view, *view_data->all_bounds);
		g_free (view_data->all_bounds);
		view_data->all_bounds = NULL;
	}

	if (view_data->children_bounds) {
		g_free (view_data->children_bounds);
		view_data->children_bounds = NULL;
	}

	g_slice_free1 (CC_ITEM_GET_CLASS(self)->view_data_size, view_data);
	cc_hash_map_remove (P(self)->view_data, view);
	cc_hash_map_remove (self->bounds, view);
	cdebug ("view_unregister()", "for %s in %s",
		G_OBJECT_TYPE_NAME (view),
		G_OBJECT_TYPE_NAME (self));
}

static void
cc_item_class_init(CcItemClass* self_class) {
	GObjectClass* go_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->dispose      = ci_dispose;
	go_class->get_property = ci_get_property;

	/**
	 * CcItem::all-bounds-changed
	 * @self: a #Ccitem
	 * @view: the #CcView belonging to this event
	 * @box: a #CcDRect specifying the new bounding box
	 *
	 * This signal gets emitted when the total bounding box of an item
	 * changed. See also <xref linkend="ccc-Bounds-Handling"/>.
	 */
	cc_item_signals[ALL_BOUNDS_CHANGED] =
		g_signal_new("all-bounds-changed", CC_TYPE_ITEM,
			     0,0, NULL, NULL,
			     cc_marshal_VOID__OBJECT_BOXED,
			     G_TYPE_NONE, 2,
			     CC_TYPE_VIEW, CC_TYPE_D_RECT);
	/**
	 * CcItem::button-press-event
	 * @self: a #CcItem
	 * @view: the #CcView which emitted this event
	 * @event: a #GdkEventButton with coordinates translated to canvas
	 * coordinates.
	 *
	 * This signal gets emitted when a mouse button got pressed on top of
	 * an element.
	 *
	 * Returns %TRUE to stop other signal handlers from being called.
	 */
	cc_item_signals[BUTTON_PRESS_EVENT] =
		g_signal_new("button-press-event", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, button_press_event),
			     cc_accumulator_boolean, NULL,
			     cc_marshal_BOOLEAN__OBJECT_BOXED,
			     G_TYPE_BOOLEAN, 2,
			     CC_TYPE_VIEW, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);
	/**
	 * CcItem::button-release-event
	 * @self: a #CcItem
	 * @view: the #CcView which emitted this event
	 * @event: a #GdkEventButton with coordinates translated to canvas
	 * coordinates
	 *
	 * This signal gets emitted when a mouse button got released on top of
	 * an item.
	 *
	 * Returns %TRUE to stop other signal handlers from being called.
	 */
	cc_item_signals[BUTTON_RELEASE_EVENT] =
		g_signal_new("button-release-event", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, button_release_event),
			     cc_accumulator_boolean, NULL,
			     cc_marshal_BOOLEAN__OBJECT_BOXED,
			     G_TYPE_BOOLEAN, 2,
			     CC_TYPE_VIEW, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);
	/**
	 * CcItem::dirty
	 * @self: a #CcItem
	 * @dirty_region: a #CcDRect that specifies the dirty region
	 *
	 * This signal gets emitted when an item changes. It's used to request
	 * redraws of the affected items.
	 */
	cc_item_signals[ITEM_DIRTY] =
		g_signal_new("dirty", CC_TYPE_ITEM,
			     0, 0,
			     NULL, NULL,
			     cc_marshal_VOID__OBJECT_BOXED,
			     G_TYPE_NONE, 2,
			     CC_TYPE_VIEW, CC_TYPE_D_RECT);
	/**
	 * CcItem::enter-notify-event
	 * @self: a #CcItem
	 * @view: the #CcView which emitted this event
	 * @event: a #GdkEventCrossing with x and y coordinates translated into
	 * canvas coordinates
	 *
	 * This signal gets emitted when the mouse pointer enters an item.
	 *
	 * Returns %TRUE to stop other signal handlers from being invoked.
	 */
	cc_item_signals[ENTER_NOTIFY_EVENT] =
		g_signal_new("enter-notify-event", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, enter_notify_event),
			     cc_accumulator_boolean, NULL,
			     cc_marshal_BOOLEAN__OBJECT_BOXED,
			     G_TYPE_BOOLEAN, 2,
			     CC_TYPE_VIEW, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);
	/**
	 * CcItem::event
	 * @self: a #CcItem
	 * @view: the #CcView which emitted this event
	 * @event: the event that got emitted (with canvas coordinates)
	 *
	 * This signal catches all events before they are distributes into more
	 * specified signals.
	 *
	 * Returns %TRUE to stop other signal handlers from being called.
	 */
	cc_item_signals[EVENT] =
		g_signal_new("event", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, event),
			     cc_accumulator_boolean, NULL,
			     cc_marshal_BOOLEAN__OBJECT_BOXED,
			     G_TYPE_BOOLEAN, 2,
			     CC_TYPE_VIEW, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);
	/**
	 * CcItem::focus
	 * @self: a #CcItem
	 * @view: a #CcView
	 * @dir: the focus direction
	 *
	 * This signal gets emitted when a focus event on the view happended to
	 * let focus-handling happen.
	 *
	 * Returns %TRUE to stop other signal handlers from being called (such
	 * as the one which lets the focus leave the canvas view)
	 */
	cc_item_signals[FOCUS] =
		g_signal_new("focus", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, focus),
			     cc_accumulator_boolean, NULL,
			     cc_marshal_BOOLEAN__OBJECT_ENUM,
			     G_TYPE_BOOLEAN, 2,
			     CC_TYPE_VIEW, GTK_TYPE_DIRECTION_TYPE);
	/**
	 * CcItem::focus-enter
	 * @self: a #CcItem
	 * @view: a #CcView
	 * @event: a #GdkEventFocus
	 *
	 * This signal gets emitted when the focus enters this item.
	 *
	 * Returns %TRUE to stop other signal handlers, %FALSE to allow their
	 * execution.
	 */
	cc_item_signals[FOCUS_ENTER] =
		g_signal_new("focus-enter", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, focus_enter),
			     NULL, NULL,
			     cc_marshal_BOOLEAN__OBJECT_BOXED,
			     G_TYPE_BOOLEAN, 2,
			     CC_TYPE_VIEW, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);
	/**
	 * CcItem::focus-leave
	 * @self: a #CcItem
	 * @view: a #CcView
	 * @event: a #GdkEventFocus
	 *
	 * This signal gets emitted when the focus leaves this item.
	 *
	 * Returns %TRUE to stop other signal handlers, %FALSE to allow their
	 * execution.
	 */
	cc_item_signals[FOCUS_LEAVE] =
		g_signal_new("focus-leave", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, focus_leave),
			     NULL, NULL,
			     cc_marshal_BOOLEAN__OBJECT_BOXED,
			     G_TYPE_BOOLEAN, 2,
			     CC_TYPE_VIEW, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);
	/**
	 * CcItem::key-press-event
	 * @self: a #CcItem
	 * @view: a #CcView
	 * @event: a #GdkEventKey
	 *
	 * This signal gets emitted when a key gets pressed.
	 *
	 * Returns %TRUE to stop other signal handlers, %FALSE to allow their
	 * execution.
	 */
	cc_item_signals[KEY_PRESS_EVENT] =
		g_signal_new("key-press-event", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, key_press_event),
			     cc_accumulator_boolean, NULL,
			     cc_marshal_BOOLEAN__OBJECT_BOXED,
			     G_TYPE_BOOLEAN, 2,
			     CC_TYPE_VIEW, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);
	/**
	 * CcItem::key-release-event
	 * @self: a #CcItem
	 * @view: a #CcView
	 * @event: a #GdkEventKey
	 *
	 * This signal gets emitted when a key gets released.
	 *
	 * Returns %TRUE to stop other signal handlers, %FALSE to allow their
	 * execution.
	 */
	cc_item_signals[KEY_RELEASE_EVENT] =
		g_signal_new("key-release-event", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, key_release_event),
			     cc_accumulator_boolean, NULL,
			     cc_marshal_BOOLEAN__OBJECT_BOXED,
			     G_TYPE_BOOLEAN, 2,
			     CC_TYPE_VIEW, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);
	/**
	 * CcItem::leave-notify-event
	 * @self: a #CcItem
	 * @view: the #CcView which emitted this event
	 * @event: a #GdkEventCrossing
	 *
	 * This signal gets emitted when the mouse pointer leaves an element.
	 *
	 * Returns %TRUE to stop other signal handlers from being invoked.
	 */
	cc_item_signals[LEAVE_NOTIFY_EVENT] =
		g_signal_new("leave-notify-event", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, leave_notify_event),
			     cc_accumulator_boolean, NULL,
			     cc_marshal_BOOLEAN__OBJECT_BOXED,
			     G_TYPE_BOOLEAN, 2,
			     CC_TYPE_VIEW, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);
	/**
	 * CcItem::motion-notify-event
	 * @self: a #CcItem
	 * @view: the #CcView which emitted this event
	 * @event: the event that got emitted (with canvas coordinates)
	 *
	 * This signal gets emitted when the pointer (mouse cursor) moves while
	 * being over this item.
	 *
	 * Returns %TRUE to stop other signal handlers from being called.
	 */
	cc_item_signals[MOTION_NOTIFY_EVENT] =
		g_signal_new("motion-notify-event", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, motion_notify_event),
			     cc_accumulator_boolean, NULL,
			     cc_marshal_BOOLEAN__OBJECT_BOXED,
			     G_TYPE_BOOLEAN, 2,
			     CC_TYPE_VIEW, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);
	/**
	 * CcItem::item-added
	 * @self: a #CcItem
	 * @position: the new index of the child
	 * @child: the child
	 *
	 * This signal gets emitted when an element is added to another element.
	 */
	cc_item_signals[ITEM_ADDED] =
		g_signal_new("item-added", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, item_added),
			     NULL, NULL,
			     cc_marshal_VOID__INT_OBJECT,
			     G_TYPE_NONE, 2,
			     G_TYPE_INT, CC_TYPE_ITEM);
	/**
	 * CcItem::item-removed
	 * @self: a #CcItem
	 * @position: the former index of the child
	 * @child: the child
	 *
	 * This signal gets emitted when an element is removed from another element.
	 */
	cc_item_signals[ITEM_REMOVED] =
		g_signal_new("item-removed", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, item_removed),
			     NULL, NULL,
			     cc_marshal_VOID__INT_OBJECT,
			     G_TYPE_NONE, 2,
			     G_TYPE_INT, CC_TYPE_ITEM);
	/**
	 * CcItem::view-register
	 * @self: a #CcItem
	 * @view: a #CcView
	 *
	 * Registeres the view for this item. This signal can be used to create
	 * view-specific data for the #CcItem. Take a look at #CcText to see
	 * how this can be used.
	 */
	cc_item_signals[VIEW_REGISTER] =
		g_signal_new("view-register", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(CcItemClass, view_register),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__OBJECT,
			     G_TYPE_NONE, 1,
			     CC_TYPE_VIEW);
	/**
	 * CcItem::view-unregister
	 * @self: a #CcItem
	 * @view: a #CcView
	 *
	 * Registeres the view for this item. This signal can be used to create
	 * view-specific data for the #CcItem. Take a look at #CcText to see
	 * how this can be used.
	 */
	cc_item_signals[VIEW_UNREGISTER] =
		g_signal_new("view-unregister", CC_TYPE_ITEM,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcItemClass, view_unregister),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__OBJECT,
			     G_TYPE_NONE, 1,
			     CC_TYPE_VIEW);

	/* CcItemClass */
	self_class->distance            = item_distance;
	self_class->event               = item_event;
	self_class->focus               = ci_focus;
	self_class->render              = ci_render;
	self_class->notify_child_bounds = ci_notify_child_bounds_real;

	self_class->view_register       = ci_view_register;
	self_class->view_unregister     = ci_view_unregister;

	self_class->view_data_size      = 0;
	cc_item_class_add_view_data (self_class, sizeof (CcItemViewData));

	g_type_class_add_private (self_class, sizeof (CcItemPrivate));
}

/* CcItemViewIface */
static void
ci_item_dirty(CcItemView* iview, CcItem* item, CcView const* view, CcDRect const* dirty) {
	g_return_if_fail(dirty);

	cc_item_dirty(CC_ITEM(iview), view, *dirty);
}

static void
ci_notify_child_bounds(CcItemView* iview, CcItem* item, CcView* view, CcDRect const* all_bounds) {
	CC_ITEM_GET_CLASS(iview)->notify_child_bounds(CC_ITEM(iview), item, view, all_bounds);
}

static void
ci_init_item_view(CcItemViewIface* iface) {
	iface->item_dirty        = ci_item_dirty;
	iface->notify_all_bounds = ci_notify_child_bounds;
}

