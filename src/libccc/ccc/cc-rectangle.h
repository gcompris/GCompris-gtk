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

#ifndef CC_RECTANGLE_H
#define CC_RECTANGLE_H

#include <ccc/cc-shape.h>

G_BEGIN_DECLS

typedef struct _CcRectangle      CcRectangle;
typedef struct _CcRectangleClass CcRectangleClass;

#define CC_TYPE_RECTANGLE         (cc_rectangle_get_type())
#define CC_RECTANGLE(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_RECTANGLE, CcRectangle))
#define CC_RECTANGLE_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_RECTANGLE, CcRectangleClass))
#define CC_IS_RECTANGLE(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_RECTANGLE))
#define CC_IS_RECTANGLE_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_RECTNAGLE))
#define CC_RECTANGLE_GET_CLASS(c) (G_TYPE_INTERFACE_GET_CLASS((i), CC_TYPE_RECTANGLE, CcRectangleClass))

GType cc_rectangle_get_type(void);

CcItem*  cc_rectangle_new              (void);
void     cc_rectangle_set_position     (CcRectangle* self,
					gdouble      x,
					gdouble      y,
					gdouble      w,
					gdouble      h);
void     cc_rectangle_set_x            (CcRectangle* self,
					gdouble      x);
void     cc_rectangle_set_y            (CcRectangle* self,
					gdouble      y);
void     cc_rectangle_set_width        (CcRectangle* self,
					gdouble      width);
void     cc_rectangle_set_height       (CcRectangle* self,
					gdouble      height);

struct _CcRectangle {
	CcShape    base;

	gdouble   x,y,w,h;
};

struct _CcRectangleClass {
	CcShapeClass base_class;
};

G_END_DECLS

#endif /* !CC_RECTANGLE_H */

