/* gcompris - drag.h
 *
 * Copyright (C) 2006, 2008 Miguel de Izarra
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

#ifndef GCOMPIRS_DRAG_H
#define GCOMPIRS_DRAG_H
#include "gcompris.h"

typedef enum {
    GC_DRAG_MODE_DEFAULT = 0,
    GC_DRAG_MODE_GRAB = 1,
    GC_DRAG_MODE_2CLICKS = 2,
    GC_DRAG_MODE_BOTH = 3
} gc_drag_mode_type;

typedef gint (*GcDragFunc) (GooCanvasItem *item,
			    GooCanvasItem *target,
			    GdkEvent *event, gpointer data);

gboolean gc_drag_event (GooCanvasItem *item,
			GooCanvasItem *target,
			GdkEventButton *event,
			gpointer data);

void gc_drag_start(GooCanvasItem *root_item, GcDragFunc function, gc_drag_mode_type mode);
void gc_drag_stop(GooCanvasItem *root_item);

GooCanvasItem * gc_drag_item_get(void);
void gc_drag_item_set(GooCanvasItem *item);
void gc_drag_item_move(GdkEvent *event, GooCanvasItem *group);

void gc_drag_offset_save(GdkEvent *event);
void gc_drag_offset_get(double *x, double *y);
void gc_drag_offset_set(double x, double y);

void gc_drag_change_mode (gc_drag_mode_type mode);

#endif

