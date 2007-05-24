/* This file is part of libccc
 *
 * AUTHORS
 *     Sven Herzberg  <herzi@gnome-de.org>
 *
 * Copyright (C) 2007  Sven Herzberg
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

#ifndef CC_UNIT_H
#define CC_UNIT_H

#include <glib-object.h>

G_BEGIN_DECLS

/* http://www.w3.org/TR/css3-values/#lengths */

typedef enum {
	/* relative sizes */
	CC_UNIT_DISPLAY_PIXEL,
	/* EM */
	/* EX */
	/* CC_UNIT_PIXEL, */
	/* GD */
	/* REM */
	/* VW */
	/* VH */
	/* VM */
	/* CH */
	/* absolute size */
	/* IN */
	/* CM */
	/* MM */
	CC_UNIT_POINT,
	/* PC */
	/* aliases */
	/* CC_UNIT_PX = CC_UNIT_PIXEL, */
	CC_UNIT_PT = CC_UNIT_POINT
} CcUnit;

#define CC_TYPE_DISTANCE (cc_distance_get_type ())
typedef struct _CcDistance CcDistance;

struct _CcDistance {
	/*< private >*/
	gdouble value;
	CcUnit  unit;
};

GType       cc_distance_get_type (void);
CcDistance* cc_distance_new      (gdouble           value,
			          CcUnit            unit);
CcDistance* cc_distance_copy     (CcDistance const* self);
void        cc_distance_free     (CcDistance      * self);

G_END_DECLS

#endif /* !CC_UNIT_H */
