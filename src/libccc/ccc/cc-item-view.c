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

#include <ccc/cc-item-view.h>

#include "glib-helpers.h"
#include "gobject-helpers.h"

G_DEFINE_IFACE(CcItemView, cc_item_view, G_TYPE_INTERFACE);

static void
civ_notify_all_bounds (CcItemView   * self,
		       CcView       * view,
		       CcDRect const* rect,
		       CcItem       * item)
{
	CC_ITEM_VIEW_GET_CLASS(self)->notify_all_bounds(self, item, view, rect);
}

static void
emit_notify_all_bounds (CcItem* parent,
			CcView* view,
			CcItem* item)
{
	civ_notify_all_bounds (CC_ITEM_VIEW (parent),
			       view,
			       cc_item_get_all_bounds (item, view),
			       item);
}

static void
civ_item_dirty(CcItemView* self, CcView* view, CcDRect const* dirty, CcItem* item) {
	CC_ITEM_VIEW_GET_CLASS(self)->item_dirty(self, item, view, dirty);
}

static void
civ_update_all_bounds(CcItem* item, CcView* view) {
	g_list_foreach(item->children, G_FUNC(civ_update_all_bounds), view);

	cc_item_update_bounds_for_view(item, view);
}

static void
civ_item_added(CcItemView* self, gint position, CcItem* child, CcItem* item) {
	CC_ITEM_VIEW_GET_CLASS(self)->item_added(self, item, position, child);
}

static void
civ_item_removed(CcItemView* self, gint position, CcItem* child, CcItem* item) {
	CC_ITEM_VIEW_GET_CLASS(self)->item_removed(self, item, position, child);
}

void
cc_item_view_register(CcItemView* self, CcItem* item) {
	GCallback callback;

	g_return_if_fail(CC_IS_ITEM_VIEW(self));
	g_return_if_fail(CC_IS_ITEM(item));

	if(CC_IS_VIEW(self)) {
		cc_item_add_view(item, CC_VIEW(self));
	}

	callback = G_CALLBACK(CC_ITEM_VIEW_GET_CLASS(self)->notify_all_bounds);
	if(callback) {
		g_signal_connect_swapped(item, "all-bounds-changed",
					 G_CALLBACK(civ_notify_all_bounds), self);

		if(CC_IS_VIEW(self)) {
			civ_update_all_bounds(item, CC_VIEW(self));
		} else if(CC_IS_ITEM(self)) {
			cc_item_foreach_view (CC_ITEM (self), CC_ITEM_FUNC (emit_notify_all_bounds), item);
		}
	}

	callback = G_CALLBACK(CC_ITEM_VIEW_GET_CLASS(self)->item_dirty);
	if(callback) {
		g_signal_connect_swapped(item, "dirty",
					 G_CALLBACK(civ_item_dirty), self);
		if(CC_IS_VIEW(self) && cc_item_get_all_bounds(item, CC_VIEW(self))) {
			CcDRect const* all_bounds = cc_item_get_all_bounds(item, CC_VIEW(self));
			CC_ITEM_VIEW_GET_CLASS(self)->item_dirty(self, item, CC_VIEW(self), all_bounds);
		}
	}

	callback = G_CALLBACK(CC_ITEM_VIEW_GET_CLASS(self)->item_added);
	if(callback) {
		g_signal_connect_swapped(item, "item-added",
					 G_CALLBACK(civ_item_added), self);

		// FIXME: emit for each child
	}

	callback = G_CALLBACK(CC_ITEM_VIEW_GET_CLASS(self)->item_removed);
	if(callback) {
		g_signal_connect_swapped(item, "item-removed",
					 G_CALLBACK(civ_item_removed), self);
	}
}

static void
civ_emit_dirty_per_view(CcItem* item, CcView* view, CcItemView* self) {
	CcDRect const* all_bounds = cc_item_get_all_bounds(item, view);
	if(all_bounds) {
		civ_item_dirty(self, view, all_bounds, item);
	}
}

void
cc_item_view_unregister(CcItemView* self, CcItem* item) {
	CcItemViewIface* iface;

	g_return_if_fail(CC_IS_ITEM_VIEW(self));
	g_return_if_fail(CC_IS_ITEM(item));

	iface = CC_ITEM_VIEW_GET_CLASS(self);

	if(iface->item_dirty) {
		if(CC_IS_VIEW(self)) {
			civ_emit_dirty_per_view(item, CC_VIEW(self), self);
		}
		g_signal_handlers_disconnect_by_func(item, civ_item_dirty, self);
	}
	g_signal_handlers_disconnect_by_func(item, civ_notify_all_bounds, self);

	if(iface->item_removed) {
		// FIXME: emit for each child in reverse order

		g_signal_handlers_disconnect_by_func(item, civ_item_removed, self);
	}
	g_signal_handlers_disconnect_by_func(item, civ_item_added, self);

	if(CC_IS_VIEW(self)) {
		cc_item_remove_view(item, CC_VIEW(self));
	}
}

