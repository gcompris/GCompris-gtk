/* gcompris - timer.h
 *
 * Time-stamp: <2001/10/15 01:10:21 bruno>
 *
 * Copyright (C) 2001 Pascal Georges
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef TIMER_H
#define TIMER_H

#include <ctype.h>
#include <math.h>
#include <assert.h>
#include "gcompris.h"

typedef enum
{
  GCOMPRIS_TIMER_TEXT		= 0,
  GCOMPRIS_TIMER_SAND		= 1,
  GCOMPRIS_TIMER_BALLOON	= 2,
  GCOMPRIS_TIMER_CLOCK		= 3,
} TimerList;

/* Callback when the timer is completed */
typedef void          (*GcomprisTimerEnd)       ();

void	 gcompris_timer_display(int x, int y, TimerList type, int second, GcomprisTimerEnd gcomprisTimerEnd);
void	 gcompris_timer_add(int second);
void	 gcompris_timer_end(void);
guint	 gcompris_timer_get_remaining();
void	 gcompris_timer_pause(gboolean pause);

#endif
