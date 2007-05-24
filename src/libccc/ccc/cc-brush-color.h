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

#ifndef CC_BRUSH_COLOR_H
#define CC_BRUSH_COLOR_H

#include <ccc/cc-brush.h>
#include <ccc/cc-color.h>

G_BEGIN_DECLS

typedef struct _CcBrushColor      CcBrushColor;
typedef struct _CcBrushColorClass CcBrushColorClass;

#define CC_TYPE_BRUSH_COLOR         (cc_brush_color_get_type())
#define CC_BRUSH_COLOR(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_BRUSH_COLOR, CcBrushColor))
#define CC_BRUSH_COLOR_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_BRUSH_COLOR, CcBrushColorClass))
#define CC_IS_BRUSH_COLOR(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_BRUSH_COLOR))
#define CC_IS_BRUSH_COLOR_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_BRUSH_COLOR))
#define CC_BRUSH_COLOR_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_BRUSH_COLOR, CcBrushColorClass))

GType cc_brush_color_get_type(void);

CcBrush      * cc_brush_color_new      (CcColor     * color);
CcColor      * cc_brush_color_get_color(CcBrushColor* self);
void           cc_brush_color_set_color(CcBrushColor* self,
					CcColor     * color);

struct _CcBrushColor {
	CcBrush   base;
	gboolean  disposed;
	CcColor * color;
};

struct _CcBrushColorClass {
	CcBrushClass base_class;
};

G_END_DECLS

#endif /* !CC_BRUSH_COLOR_H */

