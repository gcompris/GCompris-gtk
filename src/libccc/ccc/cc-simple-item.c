/* This file is part of libccc
 *
 * AUTHORS
 *     Sven Herzberg  <herzi@gnome-de.org>
 *
 * Copyright (C) 2007  Sven Herzberg
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

#include "cc-simple-item.h"

#include <ccc/cc-cairo.h>
#include <ccc/cc-marshallers.h>

enum {
	SIGNAL_DISTANCE,
	SIGNAL_RENDER,
	SIGNAL_UPDATE_BOUNDS,
	N_SIGNALS
};

static gint simple_item_signals[N_SIGNALS] = {0};

G_DEFINE_TYPE (CcSimpleItem, cc_simple_item, CC_TYPE_ITEM);

static void
cc_simple_item_init (CcSimpleItem* self)
{}

static gdouble
simple_item_distance (CcItem      * item,
		      CcView const* view,
		      gdouble       x,
		      gdouble       y,
		      CcItem      **found)
{
	gdouble retval = G_MAXDOUBLE;
	g_signal_emit (item, simple_item_signals[SIGNAL_DISTANCE], 0, view, x, y, found, &retval);
	// FIXME: check children

#ifndef G_DISABLE_CHECKS
	if (retval <= 0.0 && !*found) {
		g_message ("Please set the return value in your dirty signal on %s (%p)",
			   G_OBJECT_TYPE_NAME (item), item);
		*found = item;
	}
#endif
	return retval;
}

static void
simple_item_render (CcItem * item,
		    CcView * view,
		    cairo_t* cr)
{
	g_signal_emit (item, simple_item_signals[SIGNAL_RENDER], 0, view, cr);

	if (CC_ITEM_CLASS (cc_simple_item_parent_class)->render) {
		CC_ITEM_CLASS (cc_simple_item_parent_class)->render (item, view, cr);
	}
}

static void
simple_item_update_bounds (CcItem      * item,
			   CcView const* view,
			   gpointer      user_data)
{
	g_signal_emit (item, simple_item_signals[SIGNAL_UPDATE_BOUNDS], 0, view, user_data);
}

static void
cc_simple_item_class_init (CcSimpleItemClass* self_class)
{
	CcItemClass* item_class = CC_ITEM_CLASS (self_class);

	/**
	 * CcSimpleItem::distance:
	 * @self: the #CcSimpleItem which received the signal
	 * @view: the #CcView where the signal was received
	 * @x: the horizontal coordinate of the tested point
	 * @y: the vertical coordinate of the tested point
	 * @retval: a return location for a hit item
	 *
	 * Hit-Test an item. This signal gets emitted when the view is trying
	 * to locate an item at a specific position. See also <xref
	 * linkend="ccc-Rapid-Prototyping-of-custom-Items"/>.
	 *
	 * Returns the distance of (@x, @y) to @self.
	 */
	simple_item_signals[SIGNAL_DISTANCE] =
		g_signal_new ("distance", CC_TYPE_SIMPLE_ITEM,
			      0, 0,
			      NULL, NULL,
			      cc_marshal_DOUBLE__OBJECT_DOUBLE_DOUBLE_POINTER,
			      G_TYPE_DOUBLE,
			      4,
			      CC_TYPE_VIEW,
			      G_TYPE_DOUBLE,
			      G_TYPE_DOUBLE,
			      G_TYPE_POINTER);
	/**
	 * CcSimpleItem::render:
	 * @self: a #CcSimpleItem
	 * @view: a #CcView
	 * @cairo: a #cairo_t for rendering
	 *
	 * Render the simple item to a view. See also <xref
	 * linkend="ccc-Rapid-Prototyping-of-custom-Items"/>.
	 */
	simple_item_signals[SIGNAL_RENDER] =
		g_signal_new ("render", CC_TYPE_SIMPLE_ITEM,
			      0, 0,
			      NULL, NULL,
			      cc_marshal_VOID__OBJECT_BOXED,
			      G_TYPE_NONE, 2,
			      CC_TYPE_VIEW,
			      CC_TYPE_CAIRO);
	/**
	 * CcSimpleItem::update-bounds:
	 * @self: a #CcSimpleItem
	 * @view: a #CcView
	 *
	 * Update the bounding box for a view. See also <xref
	 * linkend="ccc-Rapid-Prototyping-of-custom-Items"/>.
	 */
	simple_item_signals[SIGNAL_UPDATE_BOUNDS] =
		g_signal_new ("update-bounds", CC_TYPE_SIMPLE_ITEM,
			      0, 0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1,
			      CC_TYPE_VIEW);

	item_class->distance      = simple_item_distance;
	item_class->render        = simple_item_render;
	item_class->update_bounds = simple_item_update_bounds;
}

/**
 * cc_simple_item_new:
 *
 * Creates a new item for rapid prototyping.
 *
 * Returns a new #CcItem.
 */
CcItem*
cc_simple_item_new (void)
{
	return g_object_new (CC_TYPE_SIMPLE_ITEM, NULL);
}

