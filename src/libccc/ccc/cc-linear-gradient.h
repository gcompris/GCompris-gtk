/* This file is part of CCC
 *
 * AUTHORS
 *     Sven Herzberg  <herzi@gnome-de.org>
 *
 * Copyright (C) 2006  Sven Herzberg
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

#ifndef CC_LINEAR_GRADIENT_H
#define CC_LINEAR_GRADIENT_H

#include <ccc/cc-gradient.h>

G_BEGIN_DECLS

typedef CcGradient      CcLinearGradient;
typedef CcGradientClass CcLinearGradientClass;

#define CC_TYPE_LINEAR_GRADIENT         (cc_linear_gradient_get_type())

GType    cc_linear_gradient_get_type(void);
CcBrush* cc_linear_gradient_new(gdouble x1,
				gdouble y1,
				gdouble x2,
				gdouble y2);

G_END_DECLS

#endif /* !CC_LINEAR_GRADIENT_H */
