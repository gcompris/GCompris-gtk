/* This file is part of libccc, criawips' cairo-based canvas
 *
 * AUTHORS
 *     Sven Herzberg  <herzi@gnome-de.org>
 *
 * Copyright (C) 2006  Sven Herzberg <herzi@gnome-de.org>
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

#ifndef CC_CIRCLE_H
#define CC_CIRCLE_H

#include <ccc/cc-shape.h>

G_BEGIN_DECLS

typedef struct _CcCircle      CcCircle;
typedef struct _CcCircleClass CcCircleClass;

#define CC_TYPE_CIRCLE         (cc_circle_get_type())
#define CC_CIRCLE(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_CIRCLE, CcCircle))
#define CC_CIRCLE_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_CIRCLE, CcCircleClass))
#define CC_IS_CIRCLE(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_CIRCLE))
#define CC_IS_CIRCLE_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_CIRCLE))
#define CC_CIRCLE_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_CIRCLE, CcCircleClass))

GType cc_circle_get_type(void);

CcItem* cc_circle_new       (void);
void    cc_circle_set_anchor(CcCircle* self,
			     gdouble   x,
			     gdouble   y);
void    cc_circle_set_radius(CcCircle* self,
			     gdouble   radius);

struct _CcCircle {
	CcShape base_instance;

	gdouble x;
	gdouble y;

	gdouble radius;
};

struct _CcCircleClass {
	CcShapeClass base_class;
};

G_END_DECLS

#endif /* !CC_CIRCLE_H */
