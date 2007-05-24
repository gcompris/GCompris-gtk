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

#ifndef CC_UTILS_H
#define CC_UTILS_H

#include <glib-object.h>

G_BEGIN_DECLS

gboolean cc_accumulator_boolean(GSignalInvocationHint* hint,
				GValue		     * return_accu,
				GValue const         * handler_return,
				gpointer               data);

void cc_point_grid_align(gdouble        * x,
			 gdouble        * y,
			 gdouble        * width);

typedef enum {
	CC_ZOOM_PIXELS,
	CC_ZOOM_WIDTH,
	CC_ZOOM_HEIGHT,
	CC_ZOOM_AUTO
} CcZoomMode;

#define cc_return_if_unimplemented_code(klass, member, CODE) G_STMT_START {\
	if(!klass->member) {\
		g_warning("%s doesn't implement %s", g_type_name(G_TYPE_FROM_CLASS(klass)), G_STRINGIFY(member));\
		CODE;\
		return;\
	}\
} G_STMT_END;

#define cc_return_if_unimplemented(klass, member) cc_return_if_unimplemented_code(klass, member, {})

/* distance frome M (xm, ym) to [AB] A(xa, ya) & B(xb,yb) */
gdouble math_distance_point_segment(gdouble xm,
				    gdouble ym,
				    gdouble xa,
				    gdouble ya,
				    gdouble xb,
				    gdouble yb);

G_END_DECLS

#endif /* !CC_UTILS_H */

