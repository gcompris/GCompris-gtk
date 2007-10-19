/* gcompris - timer.c
 *
 * Time-stamp: <2001/10/15 01:10:21 bruno>
 *
 * Copyright (C) 2002 Bruno coudoin
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
static gint		 timer_increment (GtkWidget *widget, gpointer data);
static gint		 subtimer_increment (GtkWidget *widget, gpointer data);
static void		 start_animation();
static GooCanvasItem    *boardRootItem = NULL;
static gint		 animate_id = 0;
static gint		 subanimate_id = 0;
static GcomprisTimerEnd	 gcomprisTimerEnd;

GooCanvasItem	*gc_timer_item;

void
gc_timer_display(int ax, int ay, TimerList atype, int second, GcomprisTimerEnd agcomprisTimerEnd)
{
  GdkFont *gdk_font;
  GdkPixbuf *pixmap = NULL;
  GcomprisProperties *properties = gc_prop_get();

  /* Timer is not requested */
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
					      y);

	gdk_pixbuf_unref(pixmap);
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

      /* Calc the number of step needed to reach the sea based on user y and second */
      ystep = (BOARDHEIGHT-y-gdk_pixbuf_get_height(pixmap))/second;

      gdk_pixbuf_unref(pixmap);

      pixmap = gc_skin_pixmap_load("timers/sea.png");
      goo_canvas_image_new (boardRootItem,
			    pixmap,
			    0,
			    BOARDHEIGHT - gdk_pixbuf_get_height(pixmap),
			    "width", (double) gdk_pixbuf_get_width(pixmap),
			    "height", (double) gdk_pixbuf_get_height(pixmap),
			    NULL);
      gdk_pixbuf_unref(pixmap);

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
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);
  boardRootItem = NULL;

  paused = TRUE;

  if (animate_id)
    gtk_timeout_remove (animate_id);
  animate_id = 0;

  if (subanimate_id)
    gtk_timeout_remove (subanimate_id);
  subanimate_id = 0;
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
	gtk_timeout_remove (animate_id);
      animate_id = 0;

      if (subanimate_id)
	gtk_timeout_remove (subanimate_id);
      subanimate_id = 0;
    }
  else
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
      subanimate_id = gtk_timeout_add (1000/subratio, (GtkFunction) subtimer_increment, NULL);
      break;
    }

  animate_id = gtk_timeout_add (1000, (GtkFunction) timer_increment, NULL);
}


static void
display_time_ellapsed()
{
  switch(type)
    {
    case GCOMPRIS_TIMER_TEXT:
      /* Display the value for this timer */
      if(gc_timer_item)
	g_object_set(gc_timer_item,
		     _("Time Elapsed"),
		     NULL);
      break;
    default:
      break;
    }

}


static gint
subtimer_increment(GtkWidget *widget, gpointer data)
{
  if(paused)
    return(FALSE);

  switch(type)
    {
    case GCOMPRIS_TIMER_BALLOON:
      /* Display the value for this timer */
      y+=ystep/subratio;
      if(gc_timer_item)
	g_object_set(gc_timer_item,
		     "y", y,
		     NULL);
      //      goo_canvas_update_now(gc_get_canvas());
      break;
    default:
      break;
    }
  return (TRUE);
}

static gint
timer_increment(GtkWidget *widget, gpointer data)
{
  if(paused)
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
      if(gc_timer_item)
	{
	  GdkPixbuf	*pixmap = NULL;
	  gchar		*filename = NULL;
	  gint		fileid;

	  fileid = (gint)timer;
	  if(type==GCOMPRIS_TIMER_SAND)
	    filename = g_strdup_printf("timers/sablier%d.png", fileid);
	  else
	    filename = g_strdup_printf("timers/clock%d.png", fileid);

	  pixmap = gc_skin_pixmap_load(filename);
	  g_object_set(gc_timer_item,
				"pixbuf", pixmap,
				NULL);
	  gdk_pixbuf_unref(pixmap);
	  g_free(filename);
	}
      break;
    case GCOMPRIS_TIMER_TEXT:
      /* Display the value for this timer */
      if(gc_timer_item)
	{
	  char *tmpstr = g_strdup_printf(_("Remaining Time = %d"), timer);
	  g_object_set_data (G_OBJECT(gc_timer_item),
			     "text", tmpstr);
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
