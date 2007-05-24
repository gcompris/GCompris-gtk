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

#ifndef CC_LINE_H
#define CC_LINE_H

#include <ccc/cc-shape.h>

G_BEGIN_DECLS

typedef CcShape      CcLine;
typedef CcShapeClass CcLineClass;

#define CC_TYPE_LINE         (cc_line_get_type())
#define CC_LINE(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_LINE, CcLine))
#define CC_LINE_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_LINE, CcLineClass))
#define CC_IS_LINE(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_LINE))
#define CC_IS_LINE_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_LINE))
#define CC_LINE_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_LINE, CcLineClass))

GType   cc_line_get_type(void);

CcItem* cc_line_new (void);
void    cc_line_clear(CcLine * self);
void    cc_line_line (CcLine * self,
		      gdouble  x,
		      gdouble  y);
void    cc_line_move (CcLine * self,
		      gdouble  x,
		      gdouble  y);

guint   cc_line_get_num_data(CcLine * self);

void    cc_line_data_set_position (CcLine *self,
				   guint index,
				   gdouble x,
				   gdouble y);

void    cc_line_data_get_position (CcLine *self,
				   guint index,
				   gdouble *x,
				   gdouble *y);


G_END_DECLS

#endif /* !CC_LINE_H */
