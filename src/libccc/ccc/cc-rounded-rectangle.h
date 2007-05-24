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

#ifndef CC_ROUNDED_RECTANGLE_H
#define CC_ROUNDED_RECTANGLE_H

#include <ccc/cc-rectangle.h>

G_BEGIN_DECLS

typedef struct _CcRoundedRectangle      CcRoundedRectangle;
typedef struct _CcRoundedRectangleClass CcRoundedRectangleClass;

#define CC_TYPE_ROUNDED_RECTANGLE         (cc_rounded_rectangle_get_type())
#define CC_ROUNDED_RECTANGLE(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_ROUNDED_RECTANGLE, CcRoundedRectangle))
#define CC_ROUNDED_RECTANGLE_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_ROUNDED_RECTANGLE, CcRoundedRectangleClass))
#define CC_IS_ROUNDED_RECTANGLE(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_ROUNDED_RECTANGLE))
#define CC_IS_ROUNDED_RECTANGLE_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_ROUNDED_RECTANGLE))
#define CC_ROUNDED_RECTANGLE_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_ROUNDED_RECTANGLE, CcRoundedRectangleClass))

GType cc_rounded_rectangle_get_type(void);

struct _CcRoundedRectangle {
	CcRectangle base_instance;

	gdouble corner_radius;
};

struct _CcRoundedRectangleClass {
	CcRectangleClass base_class;
};

G_END_DECLS

#endif /* !CC_ROUNDED_RECTANGLE_H */

