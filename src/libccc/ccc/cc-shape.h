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

#ifndef CC_SHAPE_H
#define CC_SHAPE_H

#include <ccc/cc-brush.h>
#include <ccc/cc-item.h>
#include <ccc/cc-unit.h>

G_BEGIN_DECLS

typedef struct _CcShape      CcShape;
typedef struct _CcShapeClass CcShapeClass;

#define CC_TYPE_SHAPE         (cc_shape_get_type())
#define CC_SHAPE(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_SHAPE, CcShape))
#define CC_SHAPE_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_SHAPE, CcShapeClass))
#define CC_IS_SHAPE(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_SHAPE))
#define CC_IS_SHAPE_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_SHAPE))
#define CC_SHAPE_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_SHAPE, CcShapeClass))

GType    cc_shape_get_type(void);

CcBrush* cc_shape_get_brush_border (CcShape* self);
void     cc_shape_set_brush_border (CcShape* self,
				    CcBrush* brush);
CcBrush* cc_shape_get_brush_content(CcShape* self);
void     cc_shape_set_brush_content(CcShape* self,
				    CcBrush* brush);
gdouble  cc_shape_get_width        (CcShape* shape,
				    CcView const* view);
void     cc_shape_set_line_width   (CcShape     * shape,
				    CcDistance  * width);
void     cc_shape_set_width_pixels (CcShape* self,
				    gdouble  width);
void     cc_shape_set_width_units  (CcShape* self,
				    gdouble  width);

struct _CcShape {
	CcItem base_instance;

	CcBrush * brush_border;
	CcBrush * brush_content;
};

struct _CcShapeClass {
	CcItemClass base_class;

	void (*path)           (CcShape* self,
				CcView * view,
				cairo_t* cr);
	void (*render_content) (CcShape* self,
				CcView * view,
				cairo_t* cr);
	void (*render_border)  (CcShape* self,
				CcView * view,
				cairo_t* cr);
};

G_END_DECLS

#endif /* !CC_SHAPE_H */

