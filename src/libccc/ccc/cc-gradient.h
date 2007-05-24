/* This file is part of libccc
 *
 * AUTHORS
 *     Sven Herzberg  <herzi@gnome-de.org>
 *
 * Copyright (C) 2006,2007  Sven Herzberg
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

#ifndef CC_GRADIENT_H
#define CC_GRADIENT_H

#include <ccc/cc-brush.h>
#include <ccc/cc-color.h>

G_BEGIN_DECLS

typedef struct _CcGradient        CcGradient;
typedef struct _CcGradientPrivate CcGradientPrivate;
typedef struct _CcGradientClass   CcGradientClass;

#define CC_TYPE_GRADIENT         (cc_gradient_get_type())
#define CC_GRADIENT(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_GRADIENT, CcGradient))
#define CC_GRADIENT_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_GRADIENT, CcGradientClass))
#define CC_IS_GRADIENT(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_GRADIENT))
#define CC_IS_GRADIENT_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_GRADIENT))
#define CC_GRADIENT_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_GRADIENT, CcGradientClass))

GType		 cc_gradient_get_type	   (void);
void		 cc_gradient_add_stop	   (CcGradient      * self,
					    gdouble           offset,
					    CcColor         * color);
cairo_pattern_t* cc_gradient_create_pattern(CcGradient const* self,
					    CcView const    * view,
					    CcItem const    * item);

struct _CcGradient {
	GInitiallyUnowned  base_instance;
	CcGradientPrivate* _private;
};

struct _CcGradientClass {
	GInitiallyUnownedClass base_class;

	/* vtable */
	cairo_pattern_t* (*create_pattern)(CcGradient const* self,
					   CcView const    * view,
					   CcItem const    * item);
};

G_END_DECLS

#endif /* !CC_GRADIENT_H */
