/* gcompris - drag.c
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

#include "drag.h"

static GcDragFunc gc_drag_func;
static gpointer gc_drag_user_data;
static int gc_drag_status;

/*  Values of status are :
    0 - waiting for a BUTTON_PRESS
    1 - follow mouse pointer
    2 - follow mouse pointer and waiting for a BUTTON_RELEASE */

static gc_drag_mode_type gc_drag_mode;
static GooCanvasItem *gc_drag_item;
static GooCanvasItem *gc_drag_target;
static double gc_drag_offset_x, gc_drag_offset_y;

GooCanvasItem*
gc_drag_item_get(void)
{
  return gc_drag_item;
}

void
gc_drag_item_set(GooCanvasItem *item)
{
  /* Don't use gc_drag_set_item during a dragging */
  g_assert(gc_drag_status == 0);

  if(item)
    gc_drag_item = item;
}

/* Move the current dragging item of the group if given */
void
gc_drag_item_move(GdkEvent *event, GooCanvasItem *group)
{
  double new_x = event->button.x;
  double new_y = event->button.y;
  GooCanvasBounds bounds;

  /* Check board boundaries */
  goo_canvas_item_get_bounds(gc_drag_item, &bounds);
  if((bounds.x1 < 0 && new_x < gc_drag_offset_x)
     || (bounds.x2 > BOARDWIDTH && new_x > gc_drag_offset_x))
      new_x = gc_drag_offset_x;

  if((bounds.y1 < 0 && new_y < gc_drag_offset_y)
     || (bounds.y2 > BOARDHEIGHT && new_y > gc_drag_offset_y))
      new_y = gc_drag_offset_y;

  goo_canvas_item_translate ((group ? group : gc_drag_item),
			     new_x - gc_drag_offset_x,
			     new_y - gc_drag_offset_y);
}

void
gc_drag_offset_get(double *x, double *y)
{
  *x = gc_drag_offset_x;
  *y = gc_drag_offset_y;
}

void
gc_drag_offset_set(double x, double y)
{
  gc_drag_offset_x = x;
  gc_drag_offset_y = y;
}

void
gc_drag_offset_save(GdkEvent *event)
{
  double x, y;
  GooCanvasBounds bounds;

  x = event->button.x;
  y = event->button.y;

  goo_canvas_item_get_bounds(gc_drag_item, &bounds);
  goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(gc_drag_item),
  				   gc_drag_item, &bounds.x1, &bounds.y1);

  gc_drag_offset_set(x - bounds.x1, y - bounds.y1);
}

gboolean
gc_drag_event (GooCanvasItem *item,
	       GooCanvasItem *target,
	       GdkEventButton *event,
	       gpointer data)
{
  switch(event->type)
    {
    case GDK_BUTTON_PRESS:
      if(gc_drag_status == 0 && event->button == 1)
	{
	  gc_drag_item = item;
	  gc_drag_target = target;
	  if(gc_drag_mode == GC_DRAG_MODE_GRAB)
	    gc_drag_status = 2;
	  else
	    gc_drag_status = 1;

	  gc_drag_user_data = data;
	  gc_drag_func(item, target, (GdkEvent*)event, data);
	}
      break;
    case GDK_BUTTON_RELEASE:
      if(gc_drag_status == 2)
	{
	  gc_drag_func(gc_drag_item,
		       item,
		       (GdkEvent*)event,
		       data);
	  gc_drag_status = 0;
	  gc_drag_item = NULL;
	  gc_drag_target = NULL;
	  gc_drag_user_data = NULL;
	}
      else if (gc_drag_status == 1 && gc_drag_mode & GC_DRAG_MODE_2CLICKS)
	gc_drag_status = 2;
      break;
    default:
      break;
    }
  return FALSE;
}

gint
gc_drag_event_root(GooCanvasItem * item,
		   GooCanvasItem *target,
		   GdkEventMotion *event,
		   gpointer data)
{
  if(gc_drag_mode & GC_DRAG_MODE_2CLICKS) {
    gdouble new_x = event->x_root;
    gdouble new_y = event->y_root;
    goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(item),
                                    gc_drag_item, &new_x, &new_y);
    event->x = new_x;
    event->y = new_y;
    target = gc_drag_target;
  }

  if(gc_drag_target != target)
    return FALSE;

  switch(event->type)
    {
    case GDK_MOTION_NOTIFY:
      if(gc_drag_status > 0)
	{
	  if(gc_drag_status==1 && gc_drag_mode & GC_DRAG_MODE_GRAB)
	    gc_drag_status=2;

	  gc_drag_func(gc_drag_item,
		       item,
		       (GdkEvent*)event,
		       gc_drag_user_data);
	}
      break;
    default:
      break;
    }
  return FALSE;
}

void
gc_drag_start(GooCanvasItem *root_item,
	      GcDragFunc function,
	      gc_drag_mode_type mode)
{
  g_signal_connect(root_item, "motion_notify_event",
  		   (GCallback) gc_drag_event_root, NULL);
  gc_drag_func = function;
  gc_drag_user_data = NULL;
  gc_drag_status = 0;
  gc_drag_item = NULL;
  gc_drag_target = NULL;
  gc_drag_offset_x = gc_drag_offset_y = 0;
  if (mode == GC_DRAG_MODE_DEFAULT)
    gc_drag_mode = gc_prop_get()->drag_mode;
  else
    gc_drag_mode = mode;
}

void
gc_drag_stop(GooCanvasItem *root_item)
{
  if(gc_drag_status > 0)
    {
      GdkEvent event;
      event.type = GDK_BUTTON_RELEASE;
      event.button.x = event.button.y =0;
      event.button.button = 1;
      gc_drag_func(gc_drag_item, NULL, &event, gc_drag_user_data);
    }
  g_signal_handlers_disconnect_by_func(root_item,
				       (GCallback) gc_drag_event_root,
				       NULL);
  gc_drag_func = NULL;
  gc_drag_user_data = NULL;
  gc_drag_status = -1;
  gc_drag_item = NULL;
  gc_drag_target = NULL;
  gc_drag_mode = 0;
}

void
gc_drag_change_mode (gc_drag_mode_type mode)
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
