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

#include <ccc/cc-d-rect.h>

#include "gobject-helpers.h"

CcDRect*
cc_d_rect_copy(CcDRect const* one) {
	CcDRect* eva;

	g_return_val_if_fail(one, NULL);

	eva = g_new(CcDRect, 1);
	eva->x1 = one->x1;
	eva->y1 = one->y1;
	eva->x2 = one->x2;
	eva->y2 = one->y2;

	return eva;
}

gboolean
cc_d_rect_equal(CcDRect one, CcDRect two) {
	return one.x1 == two.x1 &&
	       one.y1 == two.y1 &&
	       one.x2 == two.x2 &&
	       one.y2 == two.y2;
}

gboolean
cc_d_rect_intersect(CcDRect one, CcDRect two) {
	// two rectangles intersect if either a corner of two lies in one of the other way round
	return (((one.x1 <= two.x1 && two.x1 <= one.x2) || (one.x1 <= two.x2 && two.x2 <= one.x2)) &&
	        ((one.y1 <= two.y1 && two.y1 <= one.y2) || (one.y1 <= two.y2 && two.y2 <= one.y2))) ||
	       (((two.x1 <= one.x1 && one.x1 <= two.x2) || (two.x1 <= one.x2 && one.x2 <= two.x2)) &&
	        ((two.y1 <= one.y1 && one.y1 <= two.y2) || (two.y1 <= one.y2 && one.y2 <= two.y2)));
}

void
cc_d_rect_union(CcDRect one, CcDRect two, CcDRect* merged) {
	g_return_if_fail(merged);

	merged->x1 = MIN(one.x1, two.x1);
	merged->y1 = MIN(one.y1, two.y1);
	merged->x2 = MAX(one.x2, two.x2);
	merged->y2 = MAX(one.y2, two.y2);
}

/* GType stuff */
GType
cc_d_rect_get_type(void) {
	static GType type = 0;

	if(G_UNLIKELY(!type)) {
		type = g_boxed_type_register_static("CcDRect",
						    G_BOXED_COPY_FUNC(cc_d_rect_copy),
						    G_BOXED_FREE_FUNC(g_free));
	}

	return type;
}

