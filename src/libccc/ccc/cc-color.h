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

#ifndef CC_COLOR_H
#define CC_COLOR_H

#include <cairo.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _CcColor      CcColor;
typedef struct _CcColorClass CcColorClass;

typedef CcColor CcColorHsv;
typedef CcColor CcColorRgb;

#define CC_TYPE_COLOR         (cc_color_get_type())
#define CC_COLOR(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_COLOR, CcColor))
#define CC_COLOR_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_COLOR, CcColorClass))
#define CC_IS_COLOR(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_COLOR))
#define CC_IS_COLOR_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_COLOR))
#define CC_COLOR_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_COLOR, CcColorClass))

GType    cc_color_get_type    (void);
GType    cc_color_hsv_get_type(void);
GType    cc_color_rgb_get_type(void);

void     cc_color_apply   (CcColor const  * self,
			   gdouble        * red,
			   gdouble        * green,
			   gdouble        * blue,
			   gdouble        * alpha);
void     cc_color_stop    (CcColor const  * self,
			   cairo_pattern_t* pattern,
			   gdouble          offset);
CcColor* cc_color_new_rgb (gdouble  red,
			   gdouble  green,
			   gdouble  blue);
CcColor* cc_color_new_rgba(gdouble  red,
			   gdouble  green,
			   gdouble  blue,
			   gdouble  alpha);
CcColor* cc_color_new_hsv (gdouble  hue,
			   gdouble  saturation,
		           gdouble  value);
CcColor* cc_color_new_hsva(gdouble  hue,
			   gdouble  saturation,
		           gdouble  value,
			   gdouble  alpha);

struct _CcColor {
	GInitiallyUnowned base_instance;
	gdouble alpha;
};

struct _CcColorClass {
	GInitiallyUnownedClass base_class;

	/* vtable */
	void (*apply) (CcColor const* self,
		       gdouble      * red,
		       gdouble      * green,
		       gdouble      * blue,
		       gdouble      * alpha);
};

G_END_DECLS

#endif /* !CC_COLOR_H */

