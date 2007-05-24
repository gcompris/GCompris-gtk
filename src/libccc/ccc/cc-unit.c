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

#include "cc-unit.h"

#include <gobject-helpers.h>

/**
 * CcUnit:
 * @CC_UNIT_PIXEL: The value is given in display pixels. It doesn't depend on
 * the resolution or the current zoom level.
 * @CC_UNIT_PX: an alias for %CC_UNIT_PIXEL
 * @CC_UNIT_POINT: 1/72 of an inch
 * @CC_UNIT_PT: an alias for %CC_UNIT_POINT
 *
 * #CcUnit represents a length unit. It is used to represent sizes that are not
 * given in pixels. It's also very useful for resolution-independent rendering
 * of sizes and positions.
 */

G_DEFINE_BOXED_TYPE (CcDistance, cc_distance);

/**
 * CcDistance:
 *
 * #CcDistance represents a one-dimensional length measured in a certain unit.
 * You shouldn't access the attributes directly.
 */

/**
 * cc_distance_new:
 * @value: the size
 * @unit: the unit belonging to the size
 *
 * Create a new distance representation.
 *
 * Returns a newly allocated #CcDistance.
 */
CcDistance*
cc_distance_new (gdouble value,
		 CcUnit  unit)
{
	CcDistance self = {value, unit};
	return cc_distance_copy (&self);
}

/**
 * cc_distance_copy:
 * @self: a #CcDistance
 *
 * Create a new #CcDistance by copying another.
 *
 * Returns a copy of @self.
 */
CcDistance*
cc_distance_copy (CcDistance const* self)
{
	CcDistance* copy = g_slice_new (CcDistance);
	copy->value = self->value;
	copy->unit  = self->unit;
	return copy;
}

/**
 * cc_distance_free:
 * @self: a #CcDistance
 *
 * Free the memory that was occupied by @self.
 */
void
cc_distance_free (CcDistance* self)
{
	g_slice_free (CcDistance, self);
}

