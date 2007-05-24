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

#ifndef CC_VIEW_CELL_RENDERER_H
#define CC_VIEW_CELL_RENDERER_H

#include <gtk/gtkcellrenderer.h>
#include <ccc/cc-item.h>

G_BEGIN_DECLS

typedef struct _CcViewCellRenderer      CcViewCellRenderer;
typedef struct _CcViewCellRendererClass CcViewCellRendererClass;

#define CC_TYPE_VIEW_CELL_RENDERER         (cc_view_cell_renderer_get_type())
#define CC_VIEW_CELL_RENDERER(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_VIEW_CELL_RENDERER, CcViewCellRenderer))
#define CC_VIEW_CELL_RENDERER_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_VIEW_CELL_RENDERER, CcViewCellRendererClass))
#define CC_IS_VIEW_CELL_RENDERER(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_VIEW_CELL_RENDERER))
#define CC_IS_VIEW_CELL_REDNERER_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_VIEW_CELL_RENDERER))
#define CC_VIEW_CELL_RENDERER_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_VIEW_CELL_RENDERER, CcCellRendererClass))

GType cc_view_cell_renderer_get_type(void);

GtkCellRenderer* cc_view_cell_renderer_new(void);

struct _CcViewCellRenderer {
	GtkCellRenderer  base_instance;
	gboolean         disposed;

	gdouble          zoom;
};

struct _CcViewCellRendererClass {
	GtkCellRendererClass base_class;
};

G_END_DECLS

#endif /* !CC_VIEW_CELL_RENDERER_H */

