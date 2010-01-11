/* gcompris - bar.h
 *
 * Copyright (C) 2010, Aleksey Lim
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BAR_H
#define BAR_H

#include "gcompris.h"

typedef struct
{
  void (*start) (GtkContainer *workspace, GooCanvas *theCanvas);
  void (*set_level) (GcomprisBoard *gcomprisBoard);
  void (*set_repeat_icon) (RsvgHandle *svg_handle);
  void (*set_location) (int x, int y, double zoom);
  void (*set_flags) (const GComprisBarFlags flags);
  void (*set_hide) (gboolean hide);
} Bar;

void gc_bar_register (Bar *bar);

#endif
