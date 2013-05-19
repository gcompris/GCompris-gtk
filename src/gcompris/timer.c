/* gcompris - timer.c
 *
 * Copyright (C) 2002, 2008 Bruno coudoin
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

#include "gcompris_config.h"
#include "timer.h"

#define SOUNDLISTFILE PACKAGE

static gboolean		 paused;
static double		 x;
static double		 y;
static double		 ystep;
static int		 timer;
static double		 subratio;
static TimerList	 type;
static gint		 timer_increment (GooCanvasItem *item);
static gint		 subtimer_increment (GooCanvasItem *item);
static void		 start_animation();
static GooCanvasItem    *boardRootItem = NULL;
static gint		 animate_id = 0;
static gint		 subanimate_id = 0;
static GcomprisTimerEnd	 gcomprisTimerEnd;

GooCanvasItem	*gc_timer_item;

void
gc_timer_display(int ax, int ay,
		 TimerList atype, int second,
		 GcomprisTimerEnd agcomprisTimerEnd)
{
  GdkFont *gdk_font;
  GdkPixbuf *pixmap = NULL;
  GcomprisProperties *properties = gc_prop_get();

  /* Timer is user disabled */
  if(properties->timer==0)
    return;

  gdk_font = gdk_font_load (FONT_BOARD_MEDIUM);

  gc_timer_end();

  paused = FALSE;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gc_get_canvas()),
					NULL);

  x = ax;
  y = ay;
  second = second / properties->timer;
  timer = second;

  type = atype;
  gcomprisTimerEnd = agcomprisTimerEnd;

  switch(type)
    {
    case GCOMPRIS_TIMER_SAND:
    case GCOMPRIS_TIMER_CLOCK:
      {
	gchar		*filename = NULL;
	gint		fileid;

	fileid = (gint)timer;
	if(type==GCOMPRIS_TIMER_SAND)
	  filename = g_strdup_printf("timers/sablier%d.png", fileid);
	else
	  filename = g_strdup_printf("timers/clock%d.png", fileid);

	pixmap = gc_skin_pixmap_load(filename);

	gc_timer_item = goo_canvas_image_new (boardRootItem,
					      pixmap,
					      x,
					      y,
                          NULL);

#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
	gdk_pixbuf_unref(pixmap);
#else
	g_object_unref(pixmap);
#endif
	g_free(filename);
      }
      break;
    case GCOMPRIS_TIMER_TEXT:
      {
	gchar *tmpstr = g_strdup_printf("Remaining Time = %d", timer);
	/* Display the value for this timer */
	gc_timer_item = goo_canvas_text_new (boardRootItem,
					     tmpstr,
					     x,
					     y,
					     -1,
					     GTK_ANCHOR_CENTER,
					     "font_gdk", gdk_font,
					     "fill_color", "white",
					     NULL);
	g_free(tmpstr);
      }
      break;
    case GCOMPRIS_TIMER_BALLOON:
      pixmap = gc_skin_pixmap_load("timers/tuxballoon.png");
      gc_timer_item = goo_canvas_image_new (boardRootItem,
					    pixmap,
					    x,
					    y,
					    NULL);

      /* Calc the number of step needed to reach
       * the sea based on user y and second
       */
      ystep = (BOARDHEIGHT-y-gdk_pixbuf_get_height(pixmap))/second;

#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
      gdk_pixbuf_unref(pixmap);
#else
      g_object_unref(pixmap);
#endif

      pixmap = gc_skin_pixmap_load("timers/sea.png");
      goo_canvas_image_new (boardRootItem,
			    pixmap,
			    0,
			    BOARDHEIGHT - gdk_pixbuf_get_height(pixmap),
			    "width", (double) gdk_pixbuf_get_width(pixmap),
			    "height", (double) gdk_pixbuf_get_height(pixmap),
			    NULL);
#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
      gdk_pixbuf_unref(pixmap);
#else
      g_object_unref(pixmap);
#endif

      break;
    default:
      break;
    }

  start_animation();
}

void
gc_timer_add(int second)
{
  timer +=second;
}

void
gc_timer_end()
{
  paused = TRUE;

  if (animate_id)
    g_source_remove (animate_id);
  animate_id = 0;

  if (subanimate_id)
    g_source_remove (subanimate_id);
  subanimate_id = 0;

  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);
  boardRootItem = NULL;
}

void
gc_timer_pause(gboolean pause)
{
  if(boardRootItem==NULL)
    return;

  paused = pause;

  if(pause)
    {
      if (animate_id)
	g_source_remove (animate_id);
      animate_id = 0;

      if (subanimate_id)
	g_source_remove (subanimate_id);
      subanimate_id = 0;
    }
  else if(paused)
    {
      start_animation();
    }

}

guint
gc_timer_get_remaining()
{
  return(timer);
}

/*
 * Local functions
 * ---------------
 */
static void
start_animation()
{

  switch(type)
    {
    case GCOMPRIS_TIMER_SAND:
    case GCOMPRIS_TIMER_CLOCK:
      /* No subanimation */
      break;
    case GCOMPRIS_TIMER_TEXT:
      /* No subanimation */
      break;
    case GCOMPRIS_TIMER_BALLOON:
      /* Perform under second animation */
      subratio = 5;
      subanimate_id = g_timeout_add (1000/subratio,
				     (GSourceFunc) subtimer_increment,
				     gc_timer_item);
      break;
    }

  animate_id = g_timeout_add (1000,
			      (GSourceFunc) timer_increment,
			      gc_timer_item);
}


static void
display_time_ellapsed()
{
  if(paused || !boardRootItem)
    return;

  switch(type)
    {
    case GCOMPRIS_TIMER_TEXT:
      /* Display the value for this timer */
      if(gc_timer_item)
	g_object_set(gc_timer_item,
		     D_(GETTEXT_GUI,"Time Elapsed"),
		     NULL);
      break;
    default:
      break;
    }

}


static gint
subtimer_increment(GooCanvasItem *item)
{
  if(paused || !boardRootItem)
    return(FALSE);

  switch(type)
    {
    case GCOMPRIS_TIMER_BALLOON:
      y += ystep/subratio;
      g_object_set(item,
		   "y", y,
		   NULL);
      break;
    default:
      break;
    }
  return (TRUE);
}

static gint
timer_increment(GooCanvasItem *item)
{
  if(paused || !boardRootItem)
    return(FALSE);

  timer--;

  if(timer==-1)
    {
      display_time_ellapsed();
      if(gcomprisTimerEnd!=NULL)
	{
	  /* Call the callback */
	  gcomprisTimerEnd();

	}
      return (FALSE);
    }

  switch(type)
    {
    case GCOMPRIS_TIMER_SAND:
    case GCOMPRIS_TIMER_CLOCK:
      if(item)
	{
	  GdkPixbuf	*pixmap = NULL;
	  gchar		*filename = NULL;
	  gint		 fileid;

	  fileid = (gint)timer;
	  if(type == GCOMPRIS_TIMER_SAND)
	    filename = g_strdup_printf("timers/sablier%d.png", fileid);
	  else
	    filename = g_strdup_printf("timers/clock%d.png", fileid);

	  pixmap = gc_skin_pixmap_load(filename);
	  g_object_set(item,
		       "pixbuf", pixmap,
		       NULL);
#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
	  gdk_pixbuf_unref(pixmap);
#else
	  g_object_unref(pixmap);
#endif
	  g_free(filename);
	}
      break;
    case GCOMPRIS_TIMER_TEXT:
      /* Display the value for this timer */
      if(item)
	{
	  char *tmpstr = g_strdup_printf(D_(GETTEXT_GUI,"Remaining Time = %d"), timer);
	  g_object_set (item,
			"text", tmpstr,
			NULL);
	  g_free(tmpstr);
	}
      break;
    case GCOMPRIS_TIMER_BALLOON:
      break;
    default:
      break;
    }

  return (TRUE);
}
