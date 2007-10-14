/* gcompris - drag.c
 *
 * Copyright (C) 2006 Miguel de Izarra
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "drag.h"

static gc_Drag_Func gc_drag_func;
static int gc_drag_status;

/*  Values of status are :
    0 - waiting for a BUTTON_PRESS
    1 - follow mouse pointer
    2 - follow mouse pointer and waiting for a BUTTON_RELEASE */

static gc_drag_mode_type gc_drag_mode;
static GooCanvasItem *gc_drag_item;
static double gc_drag_offset_x, gc_drag_offset_y;

GooCanvasItem * gc_drag_item_get(void)
{
    return gc_drag_item;
}

void gc_drag_item_set(GooCanvasItem *item)
{
    if(gc_drag_status != 0)
        g_warning("Don't use gc_drag_set_item during a dragging");
    if(item)
        gc_drag_item = item;
}

void gc_drag_item_move(GdkEvent *event)
{
    double item_x, item_y;

    item_x = event->button.x;
    item_y = event->button.y;
    goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(gc_drag_item),
				     gc_drag_item, &item_x, &item_y);

    g_object_set(gc_drag_item,
            "x", item_x - gc_drag_offset_x,
            "y", item_y - gc_drag_offset_y, NULL);
}

void gc_drag_offset_get(double *x, double *y)
{
    *x = gc_drag_offset_x;
    *y = gc_drag_offset_y;
}

void gc_drag_offset_set(double x, double y)
{
    gc_drag_offset_x = x;
    gc_drag_offset_y = y;
}

void gc_drag_offset_save(GdkEvent *event)
{
    double x, y;
    GooCanvasBounds bounds;

    x=event->button.x;
    y=event->button.y;
    goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(gc_drag_item),
				     gc_drag_item, &x, &y);

    goo_canvas_item_get_bounds(gc_drag_item, &bounds);

    gc_drag_offset_set(x - bounds.x1, y - bounds.y1);
}

gint gc_drag_event(GooCanvasItem *item, GdkEvent *event, gpointer data)
{
    switch(event->type)
    {
        case GDK_BUTTON_PRESS:
            if(gc_drag_status ==0 && event->button.button == 1)
            {
                gc_drag_item = item;
                if(gc_drag_mode == GC_DRAG_MODE_GRAB)
                    gc_drag_status = 2;
                else
                    gc_drag_status = 1;

                gc_drag_func(item, event, data);
            }
            break;
        case GDK_BUTTON_RELEASE:
            if(gc_drag_status == 2)
            {
                gc_drag_func(gc_drag_item, event, data);
                gc_drag_status = 0;
                gc_drag_item = NULL;
            }
            else if (gc_drag_status == 1 && gc_drag_mode & GC_DRAG_MODE_2CLICKS)
                gc_drag_status = 2;
            break;
        default:
            break;
    }
    return FALSE;
}

gint gc_drag_event_root(GooCanvasItem * item, GdkEvent *event, gpointer data)
{
    switch(event->type)
    {
        case GDK_MOTION_NOTIFY:
            if(gc_drag_status > 0)
            {
                if(gc_drag_status==1 && gc_drag_mode & GC_DRAG_MODE_GRAB)
                    gc_drag_status=2;
                gc_drag_func(gc_drag_item, event, data);
            }
            break;
        default:
            break;
    }
    return FALSE;
}

void gc_drag_start(GooCanvasGroup *root_item, gc_Drag_Func function, gc_drag_mode_type mode)
{
    g_signal_connect(root_item, "button_press_event",
            (GtkSignalFunc) gc_drag_event_root, NULL);
    gc_drag_func = function;
    gc_drag_status = 0;
    gc_drag_item = NULL;
    gc_drag_offset_x = gc_drag_offset_y = 0;
    if (mode == GC_DRAG_MODE_DEFAULT)
      gc_drag_mode = gc_prop_get()->drag_mode;
    else
      gc_drag_mode = mode;
}

void gc_drag_stop(GooCanvasGroup *root_item)
{
    if(gc_drag_status>0)
    {
        GdkEvent event;
        event.type = GDK_BUTTON_RELEASE;
        event.button.x = event.button.y =0;
        event.button.button = 1;
        gc_drag_func(gc_drag_item, &event, NULL);
    }
    gtk_signal_disconnect_by_func(GTK_OBJECT(root_item),
            (GtkSignalFunc) gc_drag_event_root, NULL);
    gc_drag_func = NULL;
    gc_drag_status = -1;
    gc_drag_item = NULL;
    gc_drag_mode = 0;
}

void gc_drag_change_mode (gc_drag_mode_type mode)
{
    if(gc_drag_status != 0)
        g_warning("Don't use gc_change_mode during a dragging");
    else {
      if (mode == GC_DRAG_MODE_DEFAULT)
	gc_drag_mode = gc_prop_get()->drag_mode;
      else
	gc_drag_mode = mode;
    }
}
