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

#include <ccc/cc-view-widget.h>

#include <math.h>
#include <string.h>
#include <ccc/cc-marshallers.h>
#include <ccc/cc-item-view.h>
#include <ccc/cc-view.h>

#include "glib-helpers.h"

// define to get some nice output
#undef DEBUG_SAVED_FRAMES

struct CcViewWidgetPrivate {
	gint       redraw_timeout;
	GdkRegion* dirty_region;
#ifdef DEBUG_SAVED_FRAMES
	gint       updates, requests;
#endif
};

#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE ((i), CC_TYPE_VIEW_WIDGET, struct CcViewWidgetPrivate))

/**
 * cc_view_widget_get_center_view:
 * @self: a #CcViewWidget
 *
 * Find out whether the view is centered.
 *
 * Returns %TRUE if @self displays its model centered once there's more space
 * available than the model requires.
 */
gboolean
cc_view_widget_get_center_view(CcViewWidget const* self) {
	g_return_val_if_fail(CC_IS_VIEW_WIDGET(self), FALSE);

	return self->center_view;
}

/**
 * cc_view_widget_get_zoom_mode:
 * @self: a #CcViewWidget
 *
 * Get the zoom mode of the view widget.
 *
 * Returns the zoom mode of the widget.
 */
CcZoomMode
cc_view_widget_get_zoom_mode(CcViewWidget const* self) {
	g_return_val_if_fail(CC_IS_VIEW_WIDGET(self), CC_ZOOM_PIXELS);

	return self->zoom_mode;
}

/**
 * cc_view_widget_new:
 *
 * Create a new #CcViewWidget.
 *
 * Returns a newly created #CcViewWidget which can be used to display a canvas.
 */
GtkWidget*
cc_view_widget_new(void) {
	return g_object_new(CC_TYPE_VIEW_WIDGET, NULL);
}

/**
 * cc_view_widget_new_root:
 * @root: a #CcItem, the model of the new view
 *
 * Create a new #CcViewWidget and set @root to be the model.
 *
 * Returns a new #CcViewWidget with @root as the model.
 */
GtkWidget*
cc_view_widget_new_root(CcItem* root) {
	GtkWidget* self = cc_view_widget_new();

	cc_view_set_root(CC_VIEW(self), root);

	return self;
}

static gboolean
cvw_redraw_timeout (CcViewWidget* self)
{
	g_return_val_if_fail (GTK_IS_WIDGET (self), TRUE);

	gdk_window_invalidate_region(GTK_WIDGET(self)->window, self->priv->dirty_region, FALSE);

	gdk_region_destroy(self->priv->dirty_region);
	self->priv->dirty_region   = NULL;

	self->priv->redraw_timeout = 0;
#ifdef DEBUG_SAVED_FRAMES
	self->priv->updates++;
#endif
	return FALSE; // we're done (for now)
}

static void
cvw_set_dirty(CcViewWidget* self, CcDRect dirty) {
	if(G_UNLIKELY(!GTK_WIDGET_REALIZED(self))) {
		return;
	}

	if(G_UNLIKELY(!self->root)) {
		return;
	}

	{
		GtkWidget* widget = GTK_WIDGET(self);
		CcDRect visible = {
			0.0, 0.0,
			1.0 * widget->allocation.width, 1.0 * widget->allocation.height
		};

		cc_view_world_to_window(CC_VIEW(self), &dirty.x1, &dirty.y1);
		cc_view_world_to_window(CC_VIEW(self), &dirty.x2, &dirty.y2);

		dirty.x1 = floor(dirty.x1);
		dirty.y1 = floor(dirty.y1);
		dirty.x2 = ceil (dirty.x2);
		dirty.y2 = ceil (dirty.y2);

		if(cc_d_rect_intersect(dirty, visible)) {
			GdkRectangle rect = {((gint)dirty.x1),
					      ((gint)dirty.y1),
					      ((gint)dirty.x2 - dirty.x1),
					      ((gint)dirty.y2 - dirty.y1)};

			if(G_UNLIKELY(!self->priv->redraw_timeout)) {
				self->priv->dirty_region = gdk_region_new();
				self->priv->redraw_timeout = g_timeout_add(40, // ~25 FPS
									   (GSourceFunc)cvw_redraw_timeout,
									   self);
			}

			gdk_region_union_with_rect(self->priv->dirty_region, &rect);
#ifdef DEBUG_SAVED_FRAMES
			self->priv->requests++;
#endif
		}
	}
}

static CcDRect const*
cvw_get_real_view(CcViewWidget* self) {
	static const CcDRect default_view = {0.0, 0.0, 0.0, 0.0};
	CcDRect const* retval = NULL;

	if(self->scrolled_region) {
		return self->scrolled_region;
	} else if(CC_IS_ITEM(self->root)) {
		retval = cc_item_get_all_bounds(self->root, CC_VIEW(self));
	}

	if(!retval) {
		retval = &default_view;
	}

	return retval;
}

static gdouble
cvw_get_real_zoom(CcViewWidget* self) {
	gdouble zoom_width;
	gdouble zoom_height;
	CcDRect const* view = cvw_get_real_view(self);

	if(!view || self->zoom_mode == CC_ZOOM_PIXELS) {
		return self->zoom;
	}

	zoom_width  = self->zoom * GTK_WIDGET(self)->allocation.width  / (view->x2 + 1.0 - view->x1);
	zoom_height = self->zoom * GTK_WIDGET(self)->allocation.height / (view->y2 + 1.0 - view->y1);

	switch(self->zoom_mode) {
	case CC_ZOOM_WIDTH:
		return zoom_width;
	case CC_ZOOM_HEIGHT:
		return zoom_height;
	case CC_ZOOM_AUTO:
		return MIN(zoom_width, zoom_height);
	case CC_ZOOM_PIXELS:
	default:
		// cannot happen - mention it to make the compiler happy
		g_assert_not_reached();
		return 0.0;
	}
}

static void
cvw_update_matrix(CcViewWidget* self) {
	CcDRect dirty;
	CcDRect const* view = cvw_get_real_view(self);

	// avoid 0.0 leaving the widget by rounding
	gdouble h_offset = 0.5,
		v_offset = 0.5;

	gdouble zoom = cvw_get_real_zoom(self);

	dirty.x1 = 0.0;
	dirty.x2 = GTK_WIDGET(self)->allocation.width;
	dirty.y1 = 0.0;
	dirty.y2 = GTK_WIDGET(self)->allocation.height;

	cairo_matrix_init_scale(&self->display_matrix, zoom, zoom);

	if(self->h_adjustment && self->h_adjustment->page_size < self->h_adjustment->upper - self->h_adjustment->lower) {
		h_offset -= self->h_adjustment->value;
	} else if(self->center_view && view) {
		h_offset += (dirty.x2 / zoom - (view->x2 + view->x1)) / 2.0;
	} else if(view) {
		h_offset -= view->x1;
	}

	if(self->v_adjustment && self->v_adjustment->page_size < self->v_adjustment->upper - self->v_adjustment->lower) {
		v_offset -= self->v_adjustment->value;
	} else if(self->center_view && view) {
		v_offset += (dirty.y2 / zoom - (view->y2 + view->y1)) / 2.0;
	} else if(view) {
		v_offset -= view->y1;
	}

	cairo_matrix_translate(&self->display_matrix, h_offset, v_offset);

	cc_view_window_to_world(CC_VIEW(self), &dirty.x1, &dirty.y1);
	cc_view_window_to_world(CC_VIEW(self), &dirty.x2, &dirty.y2);
	cvw_set_dirty(self, dirty);
}

/**
 * cc_view_widget_set_center_view:
 * @self: a #CcViewWidget
 * @center_view: a #gboolean
 *
 * Specify whether the model should be displayed centered if there is as much
 * space.
 */
void
cc_view_widget_set_center_view(CcViewWidget* self, gboolean center_view) {
	g_return_if_fail(CC_IS_VIEW_WIDGET(self));

	if(self->center_view == center_view) {
		return;
	}

	self->center_view = center_view;

	cvw_update_matrix(self);
	g_object_notify(G_OBJECT(self), "center-view");
}

static void
cvw_update_adjustment_allocation(GtkAdjustment* adjustment, gdouble size, gboolean update_adjustment) {
	if(!GTK_IS_ADJUSTMENT(adjustment)) {
		return;
	}

	g_object_set(adjustment,
		     "page-size",      size,
		     "page-increment", size,
		     "step-increment", size > 20 ? 10.0 : 1.0,
		     NULL);

	if(update_adjustment) {
		gtk_adjustment_changed(adjustment);
	}

	gtk_adjustment_set_value(adjustment, MIN(adjustment->value, adjustment->upper - adjustment->page_size));
}

static void
cvw_update_hadjustment_allocation(CcViewWidget* self, gboolean update_adjustment) {
	cvw_update_adjustment_allocation(self->h_adjustment, GTK_WIDGET(self)->allocation.width / cvw_get_real_zoom(self), update_adjustment);
}

static void
cvw_update_vadjustment_allocation(CcViewWidget* self, gboolean update_adjustment) {
	cvw_update_adjustment_allocation(self->v_adjustment, GTK_WIDGET(self)->allocation.height / cvw_get_real_zoom(self), update_adjustment);
}

/**
 * cc_view_widget_set_zoom_mode:
 * @self: a #CcViewWidget
 * @zoom_mode: a #CcZoomMode
 *
 * Set the zoom mode of the view widget.
 */
void
cc_view_widget_set_zoom_mode(CcViewWidget* self, CcZoomMode zoom_mode) {
	g_return_if_fail(CC_IS_VIEW_WIDGET(self));

	if(self->zoom_mode == zoom_mode) {
		return;
	}

	self->zoom_mode = zoom_mode;

	g_object_notify(G_OBJECT(self), "zoom-mode");

	cvw_update_hadjustment_allocation(self, TRUE);
	cvw_update_vadjustment_allocation(self, TRUE);

	cvw_update_matrix(self);
}

/* GType */
static void cvw_init_item_view(CcItemViewIface* iface);
static void cvw_init_view     (CcViewIface    * iface);
G_DEFINE_TYPE_WITH_CODE(CcViewWidget, cc_view_widget, GTK_TYPE_DRAWING_AREA,
			G_IMPLEMENT_INTERFACE(CC_TYPE_ITEM_VIEW, cvw_init_item_view);
			G_IMPLEMENT_INTERFACE(CC_TYPE_VIEW, cvw_init_view));

enum {
	PROP_0,
	PROP_CENTER_VIEW,
	PROP_FOCUS,
	PROP_SCROLLED_REGION,
	PROP_ROOT,
	PROP_ZOOM,
	PROP_ZOOM_MODE
};

#ifdef DEBUG_SAVED_FRAMES
static gboolean
cvw_print_stats(CcViewWidget* self) {
	if(G_LIKELY(self->priv->requests || self->priv->updates)) {
		g_print("%d redraws by %d redraw requests\n", self->priv->updates, self->priv->requests);
		self->priv->updates = self->priv->requests = 0;
	}
	return TRUE;
}
#endif

static void
cc_view_widget_init(CcViewWidget* self) {
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, CC_TYPE_VIEW_WIDGET, struct CcViewWidgetPrivate);

	self->center_view = TRUE;
	gtk_widget_add_events(GTK_WIDGET(self), GDK_ALL_EVENTS_MASK & ~GDK_POINTER_MOTION_HINT_MASK);

#ifdef DEBUG_SAVED_FRAMES
	g_timeout_add(1000, (GSourceFunc)cvw_print_stats, self);
#endif
}

static void
cvw_dispose (GObject* object)
{
	cc_view_set_root (CC_VIEW (object), NULL);

	G_OBJECT_CLASS(cc_view_widget_parent_class)->dispose(object);
}

static void
view_widget_finalize (GObject* object)
{
	if (P(object)->redraw_timeout) {
		g_source_remove (P(object)->redraw_timeout);
		P(object)->redraw_timeout = 0;
	}

	G_OBJECT_CLASS (cc_view_widget_parent_class)->finalize (object);
}

static void
cvw_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	CcViewWidget* self = CC_VIEW_WIDGET(object);

	switch(prop_id) {
	case PROP_CENTER_VIEW:
		g_value_set_boolean(value, cc_view_widget_get_center_view(self));
		break;
	case PROP_FOCUS:
		g_value_set_object(value, self->focus);
		break;
	case PROP_ROOT:
		g_value_set_object(value, self->root);
		break;
	case PROP_SCROLLED_REGION:
		g_value_set_boxed(value, self->scrolled_region);
		break;
	case PROP_ZOOM:
		g_value_set_double(value, self->zoom);
		break;
	case PROP_ZOOM_MODE:
		g_value_set_enum(value, cc_view_widget_get_zoom_mode(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cvw_set_focus(CcViewWidget* self, CcItem* focus) {
	g_return_if_fail(CC_IS_VIEW_WIDGET(self));
	g_return_if_fail(GTK_WIDGET_CAN_FOCUS(self));
	g_return_if_fail(!focus || CC_IS_ITEM(focus));

	if(self->focus == focus) {
		return;
	}

	if(self->focus) {
		g_object_unref(self->focus);
		self->focus = NULL;
	}

	if(focus) {
		self->focus = g_object_ref_sink(focus);
		if(!gtk_widget_is_focus(GTK_WIDGET(self))) {
			gtk_widget_grab_focus(GTK_WIDGET(self));
		}
	}

	g_object_notify(G_OBJECT(self), "focus");
}

static void
cvw_set_root(CcViewWidget* self, CcItem* root) {
	g_return_if_fail(!root || CC_IS_ITEM(root));

	if(self->root == root) {
		return;
	}

	if(self->root) {
		cc_item_view_unregister(CC_ITEM_VIEW(self), self->root);
		g_object_unref(self->root);
		self->root = NULL;
	}

	if(root) {
		self->root = g_object_ref_sink(root);
		cc_item_view_register(CC_ITEM_VIEW(self), self->root);
	}

	g_object_notify(G_OBJECT(self), "root");
}

static void
cvw_update_adjustment_root(GtkAdjustment* adjustment, gdouble lower, gdouble upper, gboolean update_adjustment) {
	g_return_if_fail(GTK_IS_ADJUSTMENT(adjustment));

	upper += 1.0; // avoid leaving the canvas by rounding

	g_object_set(adjustment,
		     "lower", lower,
		     "upper", upper,
		     NULL);

	if(update_adjustment) {
		gtk_adjustment_set_value(adjustment, MIN(MAX(adjustment->value, lower), upper));

		gtk_adjustment_changed(adjustment);
	}
}

static void
cvw_update_hadjustment_root(CcViewWidget* self, gboolean update_adjustment) {
	CcDRect const* view = cvw_get_real_view(self);
	if(self->h_adjustment && view) {
		cvw_update_adjustment_root(self->h_adjustment,
					   view->x1,
					   view->x2,
					   update_adjustment);
	}
}

static void
cvw_update_vadjustment_root(CcViewWidget* self, gboolean update_adjustment) {
	CcDRect const* view = cvw_get_real_view(self);
	if(self->v_adjustment && view) {
		cvw_update_adjustment_root(self->v_adjustment,
					   view->y1,
					   view->y2,
					   update_adjustment);
	}
}

static void
cvw_set_scrolled_region(CcViewWidget * self,
			CcDRect const* scrolled_region)
{
	if(self->scrolled_region == scrolled_region) {
		return;
	}

	g_free(self->scrolled_region);
	self->scrolled_region = NULL;

	if(scrolled_region) {
		self->scrolled_region = cc_d_rect_copy(scrolled_region);
	}

	g_object_notify(G_OBJECT(self), "scrolled-region");
	cvw_update_hadjustment_root(self, TRUE);
	cvw_update_vadjustment_root(self, TRUE);
	cvw_update_matrix(self);
	gtk_widget_queue_draw(GTK_WIDGET(self));
}

static void
cvw_set_zoom(CcViewWidget* self, gdouble zoom) {
	if(self->zoom == zoom) {
		return;
	}

	self->zoom = zoom;

	g_object_notify(G_OBJECT(self), "zoom");

	cvw_update_hadjustment_allocation(self, TRUE);
	cvw_update_vadjustment_allocation(self, TRUE);

	cvw_update_matrix(self);
}

static void
cvw_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	CcViewWidget* self = CC_VIEW_WIDGET(object);

	switch(prop_id) {
	case PROP_CENTER_VIEW:
		cc_view_widget_set_center_view(self, g_value_get_boolean(value));
		break;
	case PROP_FOCUS:
		cvw_set_focus(self, g_value_get_object(value));
		break;
	case PROP_ROOT:
		cvw_set_root(self, g_value_get_object(value));
		break;
	case PROP_SCROLLED_REGION:
		cvw_set_scrolled_region(self, g_value_get_boxed(value));
		break;
	case PROP_ZOOM:
		cvw_set_zoom(self, g_value_get_double(value));
		break;
	case PROP_ZOOM_MODE:
		cc_view_widget_set_zoom_mode(self, g_value_get_enum(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean
cvw_event(GtkWidget* widget, GdkEvent* event) {
	CcViewWidget* self = CC_VIEW_WIDGET(widget);
	CcView      * view = CC_VIEW(widget);
	gboolean      retval = FALSE;
	GdkEvent    * item_event = gdk_event_copy(event);
	CcItem      * item = NULL;
#if 0
	{
		GEnumClass* c = g_type_class_ref(GDK_TYPE_EVENT_TYPE);
		GEnumValue* v = g_enum_get_value(c, event->type);
		g_message("cvw_event(): %s", v->value_name);
		g_type_class_unref(c);
	}
#endif
	switch(event->type) {
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:
		cc_view_window_to_world(CC_VIEW(self), &item_event->button.x, &item_event->button.y);
		if(!self->root) {
			break;
		}
		if(self->grabbed) {
			item = self->grabbed;
		} else {
			cc_item_distance(self->root, CC_VIEW (self), item_event->button.x, item_event->button.y, &item);
		}
		if(item) {
			g_signal_emit_by_name(item, "event", view, item_event, &retval);
		}
		break;
	case GDK_ENTER_NOTIFY:
		// we don't need this one
		break;
	case GDK_LEAVE_NOTIFY:
		// unset the current item, but not if it's just grabbed
		if(self->current && self->current != self->grabbed) {
			cc_view_world_to_window(CC_VIEW(self), &item_event->crossing.x, &item_event->crossing.y);
			g_signal_emit_by_name(self->current, "event", view, item_event, &retval);
			self->current = NULL;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if(self->root) {
			GdkEventCrossing cr = {
				GDK_LEAVE_NOTIFY,
				event->motion.window,
				FALSE,
				event->motion.window,
				event->motion.time,
				event->motion.x, event->motion.y,
				event->motion.x, event->motion.y,
				GDK_CROSSING_NORMAL,
				GDK_NOTIFY_UNKNOWN, // FIXME: implement this one
				FALSE, // FIXME: implement this one
				event->motion.state
			};
			gboolean retval2 = FALSE;

			cc_view_window_to_world(CC_VIEW(self), &item_event->motion.x, &item_event->motion.y);
			if(self->grabbed) {
				if(0.0 < cc_item_distance(self->grabbed, CC_VIEW (self), item_event->motion.x, item_event->motion.y, &item)) {
					if(self->over_grabbed) {
						g_signal_emit_by_name(self->grabbed, "event", view, &cr, &retval2);
						self->over_grabbed = FALSE;
					}
				} else {
					if(!self->over_grabbed) {
						retval2 = FALSE;
						cr.type = GDK_ENTER_NOTIFY;
						g_signal_emit_by_name(self->grabbed, "event", view, &cr, &retval2);
					}
				}
				item = self->grabbed;
			} else {
				cc_item_distance(self->root, CC_VIEW (self), item_event->motion.x, item_event->motion.y, &item);
				if(item != self->current) {
					cc_view_window_to_world(CC_VIEW(self), &cr.x, &cr.y);
					if(self->current) {
						g_signal_emit_by_name(self->current, "event", view, &cr, &retval2);
						self->current = NULL;
					}
					if(item) {
						retval2 = FALSE;
						cr.type = GDK_ENTER_NOTIFY;
						self->current = item;
						g_signal_emit_by_name(self->current, "event", view, &cr, &retval2);
					}
				}
			}
			if(item) {
				g_signal_emit_by_name(item, "event", view, item_event, &retval);
			}
			gdk_window_get_pointer(GTK_WIDGET(self)->window, NULL, NULL, NULL);
		}
		break;
	case GDK_SCROLL:
		// dispatch this one on our own, heh
		break;
	case GDK_GRAB_BROKEN:
		// read documentation and find out whether we need this one
		break;
	case GDK_KEY_PRESS:
	case GDK_KEY_RELEASE:
		// we're going to deliver these down to the focused child
		if(self->focus) {
			g_signal_emit_by_name(self->focus, "event", view, item_event, &retval);
		}
		break;
	case GDK_FOCUS_CHANGE:
		if(!item_event->focus_change.in && self->focus) {
			g_signal_emit_by_name(self->focus, "event", view, item_event, &retval);
			cvw_set_focus(self, NULL);
		}
	case GDK_CONFIGURE:
	case GDK_EXPOSE:
	case GDK_MAP:
	case GDK_PROXIMITY_IN:
	case GDK_PROXIMITY_OUT:
	case GDK_UNMAP:
	case GDK_VISIBILITY_NOTIFY:
		// we don't need those
		break;
	default:
		{
			GTypeClass* class = g_type_class_ref(GDK_TYPE_EVENT_TYPE);
			GEnumValue* value = g_enum_get_value(G_ENUM_CLASS(class), event->type);
			g_message("CcViewWidget.event(): Unhandled event of type '%s'", value->value_name);
			g_type_class_unref(class);
		}
		break;
	}

	if(GTK_WIDGET_CLASS(cc_view_widget_parent_class)->event) {
		retval |= GTK_WIDGET_CLASS(cc_view_widget_parent_class)->event(widget, event);
	}

	gdk_event_free(item_event);

	return retval;
}

static gboolean
cvw_expose_event(GtkWidget* widget, GdkEventExpose* ev) {
	CcViewWidget* self = CC_VIEW_WIDGET(widget);
	CcDRect const* all_bounds;

	g_return_val_if_fail(!self->root || CC_IS_ITEM(self->root), FALSE);

	all_bounds = !self->root ? NULL : cc_item_get_all_bounds(self->root, CC_VIEW(widget));
	if(all_bounds) {
		// draw only if the area that should be repainted overlaps with the canvas area
		GdkRectangle rect;
		gdouble x = all_bounds->x1,
			y = all_bounds->y1,
			w = all_bounds->x2,
			h = all_bounds->y2;

		cc_view_world_to_window(CC_VIEW(self), &x, &y);
		cc_view_world_to_window(CC_VIEW(self), &w, &h);

		w -= x;
		h -= y;

		rect.x = x;
		rect.y = y;
		rect.width = w;
		rect.height = h;

		if(GDK_OVERLAP_RECTANGLE_OUT != gdk_region_rect_in(ev->region, &rect)) {
			cairo_t* cr = gdk_cairo_create(widget->window);
			gdk_cairo_region(cr, ev->region);
			cairo_clip(cr);
			cairo_save(cr);
			 cc_item_render(self->root, CC_VIEW(self), cr);
			cairo_restore(cr);
			cairo_destroy(cr);
		}
	}

	// we don't chain up by intention
	return FALSE;
}

static gboolean
cvw_focus(GtkWidget* widget, GtkDirectionType dir) {
	CcViewWidget* self;
	gboolean      retval = FALSE;
	CcItem*       victim;

	if(!GTK_WIDGET_CAN_FOCUS(widget) || !self->root) {
		return retval;
	}

	self = CC_VIEW_WIDGET(widget);
	victim = self->focus ? self->focus : self->root;
	g_signal_emit_by_name(victim, "focus", self, dir, &retval);

	g_return_val_if_fail(!retval || gtk_widget_is_focus(widget), retval);

	// This function is built like gtk_widget_real_focus, so no chaining up here.
	return retval;
}


static void
cvw_size_request(GtkWidget* widget, GtkRequisition* requisition) {
	CcViewWidget* self = CC_VIEW_WIDGET(widget);

	CcDRect const* all_bounds = !self->root ? NULL : cc_item_get_all_bounds(self->root, CC_VIEW(self));

	if(!self->h_adjustment && all_bounds) {
		requisition->width = self->zoom * all_bounds->x2 + 1.0;
	} else {
		requisition->width = 0.0;
	}

	if(!self->v_adjustment && all_bounds) {
		requisition->height = self->zoom * all_bounds->y2 + 1.0;
	} else {
		requisition->height = 0.0;
	}
}

static void
cvw_size_allocate(GtkWidget* widget, GtkAllocation* allocation) {
	CcViewWidget* self = CC_VIEW_WIDGET(widget);

	GTK_WIDGET_CLASS(cc_view_widget_parent_class)->size_allocate(widget, allocation);

	cvw_update_hadjustment_allocation(self, TRUE);
	cvw_update_vadjustment_allocation(self, TRUE);

	cvw_update_matrix(self);
}

static void
cvw_set_adjustment(CcViewWidget* self, GtkAdjustment* new_adjustment, GtkAdjustment**old_adjustment) {
	g_return_if_fail(old_adjustment);

	if(!new_adjustment) {
		new_adjustment = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
	}

	if(*old_adjustment != new_adjustment) {
		if(*old_adjustment) {
			g_signal_handlers_disconnect_by_func(*old_adjustment, cvw_update_matrix, self);
			g_object_unref(*old_adjustment);
		}

		*old_adjustment = g_object_ref_sink (new_adjustment);

		g_signal_connect_swapped(*old_adjustment, "changed",
					 G_CALLBACK(cvw_update_matrix), self);
		g_signal_connect_swapped(*old_adjustment, "value-changed",
					 G_CALLBACK(cvw_update_matrix), self);
		cvw_update_matrix(self);
	}
}

static void
cvw_set_hadjustment(CcViewWidget* self, GtkAdjustment* hadjustment) {
	cvw_set_adjustment(self, hadjustment, &self->h_adjustment);

	if(self->h_adjustment) {
		cvw_update_hadjustment_allocation(self, FALSE);
		cvw_update_hadjustment_root(self, TRUE);
	}
}

static void
cvw_set_vadjustment(CcViewWidget* self, GtkAdjustment* vadjustment) {
	cvw_set_adjustment(self, vadjustment, &self->v_adjustment);

	if(self->v_adjustment) {
		cvw_update_vadjustment_allocation(self, FALSE);
		cvw_update_vadjustment_root(self, TRUE);
	}
}

static void
cvw_set_scroll_adjustments(CcViewWidget* self, GtkAdjustment* hadjustment, GtkAdjustment* vadjustment) {
	cvw_set_hadjustment(self, hadjustment);
	cvw_set_vadjustment(self, vadjustment);
}

static void
cc_view_widget_class_init(CcViewWidgetClass* self_class) {
	GObjectClass  * go_class;
	GtkWidgetClass* gw_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->dispose      = cvw_dispose;
	go_class->finalize     = view_widget_finalize;
	go_class->get_property = cvw_get_property;
	go_class->set_property = cvw_set_property;

	g_object_class_install_property(go_class,
					PROP_CENTER_VIEW,
					g_param_spec_boolean("center-view",
							     "Center View",
							     "Display the model centered if there is more space than "
							     "the model needs.",
							     TRUE,
							     G_PARAM_READWRITE));

	/* GtkWidgetClass */
	gw_class = GTK_WIDGET_CLASS(self_class);
	gw_class->event         = cvw_event;
	gw_class->expose_event  = cvw_expose_event;
	gw_class->focus         = cvw_focus;
	gw_class->size_allocate = cvw_size_allocate;
	gw_class->size_request  = cvw_size_request;

	/**
	 * CcViewWidget::set-scroll-adjustments:
	 * @hadjustment: a #GtkAdjustment
	 * @vadjustment: a #GtkAdjustment
	 *
	 * This signal gets emitted when the widget is being added to a
	 * scrollable container.
	 */
	gw_class->set_scroll_adjustments_signal =
		g_signal_new("set-scroll-adjustments", CC_TYPE_VIEW_WIDGET,
			     G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(CcViewWidgetClass, set_scroll_adjustments),
			     NULL, NULL,
			     cc_marshal_VOID__BOXED_BOXED,
			     G_TYPE_NONE, 2,
			     GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);

	/* CcViewIface */
	_cc_view_install_property(go_class, PROP_ROOT, PROP_SCROLLED_REGION, PROP_ZOOM, PROP_ZOOM_MODE, PROP_FOCUS);

	/* CcViewWidgetClass */
	self_class->set_scroll_adjustments = cvw_set_scroll_adjustments;

	g_type_class_add_private(self_class, sizeof(struct CcViewWidgetPrivate));
}

/* CcItemViewIface */
static void
cvw_item_dirty(CcItemView* iview, CcItem* item, CcView const* view, CcDRect const* dirty) {
	if(dirty) {
		cvw_set_dirty(CC_VIEW_WIDGET(view), *dirty);
	}
}

static void
cvw_notify_all_bounds(CcItemView* iview, CcItem* item, CcView* view, CcDRect const* all_bounds) {
	CcViewWidget* self = CC_VIEW_WIDGET(iview);
	cvw_update_hadjustment_root(self, TRUE);
	cvw_update_vadjustment_root(self, TRUE);

	//g_return_if_fail(CC_IS_VIEW(view));
	//g_return_if_fail(CC_VIEW_WIDGET(iview) == CC_VIEW(view));

	if(all_bounds) {
		cvw_set_dirty(self, *all_bounds);
	}
	cvw_update_matrix(self);
	gtk_widget_queue_resize_no_redraw(GTK_WIDGET(iview));
}

static void
cvw_init_item_view(CcItemViewIface* iface) {
	iface->item_dirty        = cvw_item_dirty;
	iface->notify_all_bounds = cvw_notify_all_bounds;
}

/* CcViewIface */
static GdkGrabStatus
cvw_grab_item(CcView* view, CcItem* item, GdkEventMask mask, GdkCursor* cursor, guint32 time) {
	GdkGrabStatus retval;
	CcViewWidget* self = CC_VIEW_WIDGET(view);

	g_return_val_if_fail(CC_IS_VIEW_WIDGET(view), GDK_GRAB_NOT_VIEWABLE);
	g_return_val_if_fail(CC_IS_ITEM(item), GDK_GRAB_NOT_VIEWABLE);

	if(self->grabbed) {
		return GDK_GRAB_ALREADY_GRABBED;
	}

	// FIXME: check whether the item is visible

	retval = gdk_pointer_grab(GTK_WIDGET(self)->window,
				  FALSE,
				  mask,
				  NULL,
				  cursor,
				  time);

	if(retval != GDK_GRAB_SUCCESS) {
		return retval;
	}

	self->grabbed = item;
	self->grab_mask = mask;
	self->over_grabbed = (self->current && cc_item_is_child_of(self->current, self->grabbed)) || self->current == self->grabbed;
	self->current = item; /* So all events go to the grabbed item */

	return retval;
}

static void
cvw_ungrab_item(CcView* view, CcItem* item, guint32 time) {
	CcViewWidget* self = CC_VIEW_WIDGET(view);
	g_return_if_fail(CC_IS_VIEW_WIDGET(view));
	g_return_if_fail(CC_IS_ITEM(item));

	if(self->grabbed != item) {
		return;
	}

	self->grabbed = NULL;

	if(!self->over_grabbed) {
		self->current = NULL;
	}

	gdk_pointer_ungrab(time);
}

static void
cvw_window_to_world(CcView const* view,
		    gdouble     * x,
		    gdouble     * y)
{
	cairo_matrix_t inverse;

	memcpy(&inverse, &CC_VIEW_WIDGET(view)->display_matrix, sizeof(cairo_matrix_t));

	cairo_matrix_invert(&inverse);
	cairo_matrix_transform_point(&inverse, x, y);
}

static void
cvw_world_to_window(CcView const* view,
		    gdouble     * x,
		    gdouble     * y)
{
	gdouble tx, ty;
	cairo_matrix_transform_point(&CC_VIEW_WIDGET(view)->display_matrix, x ? x : &tx, y ? y : &ty);
}

static void
cvw_world_to_window_distance(CcView const* view,
			     gdouble     * x,
			     gdouble     * y) {
	gdouble tx, ty;
	cairo_matrix_transform_distance(&CC_VIEW_WIDGET(view)->display_matrix, x ? x : &tx, y ? y : &ty);
}

static void
cvw_init_view(CcViewIface* iface) {
	iface->grab_item                = cvw_grab_item;
	iface->ungrab_item              = cvw_ungrab_item;
	iface->window_to_world          = cvw_window_to_world;
	iface->world_to_window          = cvw_world_to_window;
	iface->world_to_window_distance = cvw_world_to_window_distance;
}

