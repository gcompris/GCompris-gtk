/* gcompris - bar.c
 *
 * Time-stamp: <2001/12/01 23:51:03 bruno>
 *
 * Copyright (C) 2000 Bruno Coudoin
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

/**
 * The bar button common to each games
 *
 */

#include "gcompris.h"

#define SOUNDLISTFILE PACKAGE

#define BAR_GAP 15		/* Value used to fill space above and under icons in the bar */

static gint item_event_bar(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

static gint current_level = -1;
static GnomeCanvasItem *level_item = NULL;
static GnomeCanvasItem *ok_item = NULL;
static GnomeCanvasItem *help_item = NULL;
static GnomeCanvasItem *repeat_item = NULL;
static guint level_handler_id;
static gfloat maxtimer;

GtkWidget *statusbar= NULL;

/*
 * Main entry point 
 * ----------------
 *
 */


/*
 * Do all the bar display and register the events
 */
void gcompris_bar_start (GnomeCanvas *theCanvas, GtkWidget *theStatusbar)
{

  GdkPixbuf   *pixmap = NULL;
  GnomeCanvasItem *item;
  GnomeCanvasItem *rootitem;
  gint16           width, height;
  double           zoom;

  statusbar = theStatusbar;

  width  = BOARDWIDTH-1;
  height = BARHEIGHT-2;

  rootitem = \
    gnome_canvas_item_new (gnome_canvas_root(theCanvas),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  pixmap = gcompris_load_pixmap("gcompris/buttons/bar_bg.jpg");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) 0,
				"y", (double) 0,
				"width", (double)  gdk_pixbuf_get_width(pixmap),
				"height", (double) gdk_pixbuf_get_height(pixmap),
				NULL);

  // HOME
  pixmap = gcompris_load_pixmap("gcompris/buttons/home.png");
  zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) width*0.9,
				"y", (double) (height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
				"width", (double)  gdk_pixbuf_get_width(pixmap)*zoom,
				"height", (double) gdk_pixbuf_get_height(pixmap)*zoom,
				"width_set", TRUE, 
				"height_set", TRUE,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_bar,
		     "home");
  gcompris_set_image_focus(pixmap, FALSE);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     pixmap);


  // OK
  pixmap = gcompris_load_pixmap("gcompris/buttons/ok.png");
  zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
  ok_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) width*0.7,
				"y", (double) (height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
				"width", (double)  gdk_pixbuf_get_width(pixmap)*zoom,
				"height", (double) gdk_pixbuf_get_height(pixmap)*zoom,
				"width_set", TRUE, 
				"height_set", TRUE,
				NULL);

  gtk_signal_connect(GTK_OBJECT(ok_item), "event",
		     (GtkSignalFunc) item_event_bar,
		     "ok");
  gcompris_set_image_focus(pixmap, FALSE);
  gtk_signal_connect(GTK_OBJECT(ok_item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     pixmap);


  // LEVEL
  pixmap = gcompris_load_pixmap("gcompris/buttons/level1.png");
  zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
  level_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", pixmap, 
				      "x", (double) width*0.6,
				      "y", (double) (height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
				      "width", (double)  gdk_pixbuf_get_width(pixmap)*zoom,
				      "height", (double) gdk_pixbuf_get_height(pixmap)*zoom,
				      "width_set", TRUE, 
				      "height_set", TRUE,
				      NULL);

  current_level = 1;
  gtk_signal_connect(GTK_OBJECT(level_item), "event",
		     (GtkSignalFunc) item_event_bar,
		     "level");
  gcompris_set_image_focus(pixmap, FALSE);
  level_handler_id =   gtk_signal_connect(GTK_OBJECT(level_item), "event",
					  (GtkSignalFunc) gcompris_item_event_focus,
					  pixmap);

  // HELP
  pixmap = gcompris_load_pixmap("gcompris/buttons/help.png");
  zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
  help_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				     gnome_canvas_pixbuf_get_type (),
				     "pixbuf", pixmap, 
				     "x", (double) width*0.5,
				     "y", (double) (height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
				     "width", (double)  gdk_pixbuf_get_width(pixmap)*zoom,
				     "height", (double) gdk_pixbuf_get_height(pixmap)*zoom,
				     "width_set", TRUE, 
				     "height_set", TRUE,
				     NULL);
  gtk_signal_connect(GTK_OBJECT(help_item), "event",
		     (GtkSignalFunc) item_event_bar,
		     "help");
  gcompris_set_image_focus(pixmap, FALSE);
  gtk_signal_connect(GTK_OBJECT(help_item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     pixmap);

  // REPEAT
  pixmap = gcompris_load_pixmap("gcompris/buttons/repeat.png");
  zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
  repeat_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				     gnome_canvas_pixbuf_get_type (),
				     "pixbuf", pixmap, 
				     "x", (double) width*0.4,
				     "y", (double) (height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
				     "width", (double)  gdk_pixbuf_get_width(pixmap)*zoom,
				     "height", (double) gdk_pixbuf_get_height(pixmap)*zoom,
				     "width_set", TRUE, 
				     "height_set", TRUE,
				     NULL);
  gtk_signal_connect(GTK_OBJECT(repeat_item), "event",
		     (GtkSignalFunc) item_event_bar,
		     "repeat");
  gcompris_set_image_focus(pixmap, FALSE);
  gtk_signal_connect(GTK_OBJECT(repeat_item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     pixmap);

  /* Default value for the timer */
  gcompris_bar_set_maxtimer (0);
  
  gnome_canvas_item_show(level_item);
  gnome_canvas_item_show(ok_item);
  gnome_canvas_item_show(help_item);
  gnome_canvas_item_show(repeat_item);
}


void gcompris_bar_set_level(GcomprisBoard *gcomprisBoard)
{
  char *str = NULL;
  GdkPixbuf *pixmap = NULL;
      
  /* Non yet initialized : Something Wrong */
  if(level_item==NULL)
    {
      g_message("in bar_set_level, level_item uninitialized : should not happen\n");
      return;
    }

  if(gcomprisBoard!=NULL)
    {

      str = g_strdup_printf("gcompris/buttons/level%d.png", gcomprisBoard->level);
      pixmap = gcompris_load_pixmap(str);
      g_free(str);
      gnome_canvas_item_set (level_item,
      			     "pixbuf", pixmap,
      			     NULL);
      gdk_pixbuf_unref(pixmap);

      /* Warning, I have to reconnect to highlight the new image */
      gtk_signal_disconnect	(GTK_OBJECT(level_item), level_handler_id);
      level_handler_id = gtk_signal_connect(GTK_OBJECT(level_item), "event",
					    (GtkSignalFunc) gcompris_item_event_focus,
					    pixmap);
    }
  
  current_level=gcomprisBoard->level;
}



void
gcompris_bar_set_timer (guint time)
{
  if(maxtimer>0 && time<maxtimer)
    gnome_appbar_set_progress (GNOME_APPBAR (statusbar), (gfloat) time/maxtimer);
}


void
gcompris_bar_set_maxtimer (guint time)
{
  maxtimer= time;
  gnome_appbar_set_progress (GNOME_APPBAR (statusbar), 0.0);
}

/* Setting list of available icons in the control bar */
void
gcompris_bar_set (const GComprisBarFlags flags)
{
  if(flags&GCOMPRIS_BAR_LEVEL)
    gnome_canvas_item_show(level_item);
  else
    gnome_canvas_item_hide(level_item);

  if(flags&GCOMPRIS_BAR_OK)
    gnome_canvas_item_show(ok_item);
  else
    gnome_canvas_item_hide(ok_item);

  if(flags&GCOMPRIS_BAR_HELP)
    gnome_canvas_item_show(help_item);
  else
    gnome_canvas_item_hide(help_item);

  if(flags&GCOMPRIS_BAR_REPEAT)
    gnome_canvas_item_show(repeat_item);
  else
    gnome_canvas_item_hide(repeat_item);
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/



/* Callback for the bar operations */
static gint
item_event_bar(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
      /* This is not perfect clean but it makes it easy to remove the help window
	 by clicking on any button in the bar */
      if(strcmp((char *)data, "help"))
	gcompris_help_stop ();
      
      if(!strcmp((char *)data, "ok"))
	{
	  if(gcomprisBoard!=NULL)
	    {
	      if(gcomprisBoard->plugin->ok != NULL)
		{
		  gcomprisBoard->plugin->ok();
		}
	    }
	}
      else if(!strcmp((char *)data, "level"))
	{
	  gint tmp = current_level;

	  current_level++;
	  if(current_level>gcomprisBoard->maxlevel)
	    current_level=1;
	  
	  if(tmp!=current_level)
	    gcompris_play_sound (SOUNDLISTFILE, "level");

	  if(gcomprisBoard->plugin->set_level != NULL)
	    {
	      gcomprisBoard->plugin->set_level(current_level);
	    }
	}
      else if(!strcmp((char *)data, "home"))
	{
	  gcompris_play_sound (SOUNDLISTFILE, "gobble");
	  board_stop();
	}
      else if(!strcmp((char *)data, "help"))
	{
	  gcompris_play_sound (SOUNDLISTFILE, "gobble");
	  if(gcomprisBoard->plugin->help != NULL)
	    {
	      gcomprisBoard->plugin->help();
	    }
	}
      else if(!strcmp((char *)data, "repeat"))
	{
	  if(gcomprisBoard->plugin->repeat != NULL)
	    {
	      gcomprisBoard->plugin->repeat();
	    }
	}
      break;
      
    default:
      break;
    }
  return FALSE;

}




/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
