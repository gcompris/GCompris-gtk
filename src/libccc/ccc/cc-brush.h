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

#ifndef CC_BRUSH_H
#define CC_BRUSH_H

#include <cairo.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _CcBrush      CcBrush;
typedef struct _CcBrushClass CcBrushClass;

#define CC_TYPE_BRUSH         (cc_brush_get_type())
#define CC_BRUSH(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_BRUSH, CcBrush))
#define CC_BRUSH_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_BRUSH, CcBrushClass))
#define CC_IS_BRUSH(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_BRUSH))
#define CC_IS_BRUSH_CLASS(c)  (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_BRUSH))
#define CC_BRUSH_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_BRUSH, CcBrushClass))

GType cc_brush_get_type(void);

void cc_brush_apply(CcBrush* self,
		    cairo_t* cr);

struct _CcBrush {
	GInitiallyUnowned      base;
};

struct _CcBrushClass {
	GInitiallyUnownedClass base_class;

	/* vtable */
	void (*apply) (CcBrush* brush,
		       cairo_t* cr);
};

G_END_DECLS

#endif /* !CC_BRUSH_H */

