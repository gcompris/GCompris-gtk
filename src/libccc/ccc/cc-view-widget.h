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

#ifndef CC_VIEW_WIDGET_H
#define CC_VIEW_WIDGET_H

#include <gtk/gtkdrawingarea.h>
#include <ccc/cc-item.h>
#include <ccc/cc-utils.h>

G_BEGIN_DECLS

typedef struct _CcViewWidget      CcViewWidget;
typedef struct _CcViewWidgetClass CcViewWidgetClass;

#define CC_TYPE_VIEW_WIDGET         (cc_view_widget_get_type())
#define CC_VIEW_WIDGET(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_VIEW_WIDGET, CcViewWidget))
#define CC_VIEW_WIDGET_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_VIEW_WIDGET, CcViewWidgetClass))
#define CC_IS_VIEW_WIDGET(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_VIEW_WIDGET))
#define CC_IS_VIEW_WIDGET_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_VIEW_WIDGET))
#define CC_VIEW_WIDGET_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_VIEW_WIDGET, CcViewWidgetClass))

GType cc_view_widget_get_type(void);

GtkWidget* cc_view_widget_new            (void);
GtkWidget* cc_view_widget_new_root       (CcItem* root);

gboolean   cc_view_widget_get_center_view(CcViewWidget const* self);
void       cc_view_widget_set_center_view(CcViewWidget* self,
					  gboolean      center_view);

CcZoomMode cc_view_widget_get_zoom_mode  (CcViewWidget const* self);
void       cc_view_widget_set_zoom_mode  (CcViewWidget      * self,
					  CcZoomMode          zoom_mode);

struct _CcViewWidget {
	GtkDrawingArea  base_instance;
	gboolean        disposed;

	CcDRect       * scrolled_region;
	gdouble         zoom;
	CcZoomMode      zoom_mode;
	cairo_matrix_t  display_matrix;
	CcItem        * root;

	gboolean        center_view;
	GtkAdjustment * h_adjustment;
	GtkAdjustment * v_adjustment;

	CcItem        * current;
	CcItem        * focus;
	CcItem        * grabbed;
	gboolean        over_grabbed;
	GdkEventMask    grab_mask;

	struct CcViewWidgetPrivate* priv;
};

struct _CcViewWidgetClass {
	GtkDrawingAreaClass base_class;

	/* signals */
	void (*set_scroll_adjustments) (CcViewWidget * self,
					GtkAdjustment* hadjustment,
					GtkAdjustment* vadjustment);
};

G_END_DECLS

#endif /* !CC_VIEW_WIDGET_H */

