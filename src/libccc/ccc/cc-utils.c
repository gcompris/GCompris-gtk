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

#include <ccc/cc-utils.h>

#include <math.h>
#include <glib/gmessages.h>

/**
 * cc_point_grid_align:
 * @x: an x coordinate
 * @y: a y coordinate
 * @width: the width of a line
 *
 * Rounds the coordinates @x and @y. Rounds @width to the nearest integer, the
 * result of this conversion is needed for the other calculation.
 *
 * If the rounded @width is a multiple of 2.0, then @x and @y will be set to be
 * multiples of 1.0. They will be set to values of the form (x + 0.5)
 * otherwise.
 */

static inline void
round_even(gdouble* x, gdouble* y) {
	*x = round(*x);
	*y = round(*y);
}

static inline void
round_odd(gdouble* x, gdouble* y) {
	*x = ceil(*x) - 0.5;
	*y = ceil(*y) - 0.5;
}

void
cc_point_grid_align(gdouble* x, gdouble* y, gdouble* width) {
	g_return_if_fail(width);

	if(!x || !y) {
		return;
	}

	if(G_UNLIKELY(*width <= 1.0)) {
		round_odd(x, y);
	} else {
		*width = round(*width);

		if(((int)*width) % 2 != 0) {
			// let the coordinates be of the form n+0.5
			round_odd(x, y);
		} else {
			// let the coordinates be integer numbers
			round_even(x, y);
		}
	}
}

/**
 * cc_accumulator_boolean:
 * @hint: a #GSignalInvocationHint
 * @return_accu: a #GValue for the emission
 * @handler_return: a #GValue
 * @data: unused
 *
 * Serves as a simple accumulator for signal emission. If you set
 * cc_accumulator_boolean() as the accumulator for a signal, the signal will
 * abort execution once a signal handler returns %TRUE.
 *
 * Returns %FALSE to stop, %TRUE to continue.
 */
gboolean
cc_accumulator_boolean(GSignalInvocationHint* hint, GValue* return_accu, GValue const* handler_return, gpointer data) {
	gboolean retval = g_value_get_boolean(handler_return);

	g_value_set_boolean(return_accu, retval);

	return !retval;
}

