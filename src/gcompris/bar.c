/* gcompris - bar.c
 *
 * Copyright (C) 2000-2003 Bruno Coudoin
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

/**
 * The bar button common to each games
 *
 */

#include <string.h>

#include "gcompris.h"
#include "gc_core.h"
#include "gcompris_config.h"
#include "about.h"

#define SOUNDLISTFILE PACKAGE

#define BAR_GAP		15	/* Value used to fill space above and under icons in the bar */
#define NUMBER_OF_ITEMS 10	/* Number of buttons in the bar                              */
#define HIDE_BAR_TIMOUT 3000    /* The time before we hide the bar in ms */

static void	 update_exit_button();
static gboolean  on_enter_notify (GooCanvasItem *item,
				  GooCanvasItem *target,
				  GdkEventCrossing *event,
				  char *data);
static gboolean  on_leave_notify (GooCanvasItem *item,
				  GooCanvasItem *target,
				  GdkEventCrossing *event,
				  char *data);
static gboolean item_event_bar (GooCanvasItem  *item,
				GooCanvasItem  *target,
				GdkEventButton *event,
				gchar *data);
static void	 bar_reset_sound_id (void);
static void	 setup_item_signals (GooCanvasItem *item, gchar* name);
static gboolean	 _bar_down(void *ignore);
static void	 _bar_up();
static void	 _force_bar_down();

static gint current_level = -1;
static gint current_flags = -1;
static GooCanvasItem *bar_item  = NULL;
static GooCanvasItem *exit_item = NULL;
static GooCanvasItem *home_item = NULL;
static GooCanvasItem *level_item = NULL;
static GooCanvasItem *ok_item = NULL;
static GooCanvasItem *help_item = NULL;
static GooCanvasItem *repeat_item = NULL;
static GooCanvasItem *config_item = NULL;
static GooCanvasItem *about_item = NULL;
static GooCanvasItem *rootitem = NULL;

static gint sound_play_id = 0;
static gint bar_down_id = 0;
static gboolean _hidden;

static void  confirm_quit(gboolean answer);

/*
 * Main entry point
 * ----------------
 *
 */


/*
 * Do all the bar display and register the events
 */
void gc_bar_start (GooCanvas *theCanvas)
{
  GcomprisProperties *properties = gc_prop_get();
  GdkPixbuf   *pixmap = NULL;
  gint16           width, height;
  double           zoom;

  width  = BOARDWIDTH;
  height = BARHEIGHT-2;

  bar_reset_sound_id();

  rootitem = goo_canvas_group_new (goo_canvas_get_root_item(theCanvas), NULL);
  goo_canvas_item_translate(rootitem, 0, BOARDHEIGHT - BARHEIGHT);

  pixmap = gc_skin_pixmap_load("bar_bg.png");
  bar_item = goo_canvas_image_new (rootitem,
				   pixmap,
				   0,
				   0,
				NULL);
  setup_item_signals(bar_item, "bar");
  gdk_pixbuf_unref(pixmap);

  // EXIT
  if(properties->disable_quit == 0)
    {
      pixmap = gc_skin_pixmap_load("button_exit.png");
      zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
      exit_item = goo_canvas_image_new (rootitem,
					pixmap,
					(width/NUMBER_OF_ITEMS) * 1 -
					 gdk_pixbuf_get_width(pixmap)/2,
					(height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
					 NULL);
      gdk_pixbuf_unref(pixmap);

      setup_item_signals(exit_item, "quit");
    }

  // HOME
  pixmap = gc_skin_pixmap_load("home.png");
  zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
  home_item = goo_canvas_image_new (rootitem,
				    pixmap,
				    (double) (width/NUMBER_OF_ITEMS) * 9 -
				     gdk_pixbuf_get_width(pixmap)/2,
				    (double) (height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
				     NULL);
  gdk_pixbuf_unref(pixmap);

  setup_item_signals(home_item, "back");


  // OK
  pixmap = gc_skin_pixmap_load("ok.png");
  zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
  ok_item = goo_canvas_image_new (rootitem,
				  pixmap,
				  (double) (width/NUMBER_OF_ITEMS) * 7 -
				   gdk_pixbuf_get_width(pixmap)/2,
				  (double) (height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
				   NULL);
  gdk_pixbuf_unref(pixmap);

  setup_item_signals(ok_item, "ok");

  // LEVEL
  pixmap = gc_skin_pixmap_load("level1.png");
  zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
  level_item = goo_canvas_image_new (rootitem,
				     pixmap,
				     (double) (width/NUMBER_OF_ITEMS) * 5 -
				      gdk_pixbuf_get_width(pixmap)/2,
				     (double) (height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
				      NULL);
  gdk_pixbuf_unref(pixmap);

  current_level = 1;

  setup_item_signals(level_item, "level");

  // REPEAT
  pixmap = gc_skin_pixmap_load("repeat.png");
  zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
  repeat_item = goo_canvas_image_new (rootitem,
				      pixmap,
				      (double) (width/NUMBER_OF_ITEMS) * 6 -
				       gdk_pixbuf_get_width(pixmap)/2,
				      (double) (height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
				       NULL);
  gdk_pixbuf_unref(pixmap);

  setup_item_signals(repeat_item, "repeat");


  // HELP
  pixmap = gc_skin_pixmap_load("help.png");
  zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
  help_item = goo_canvas_image_new (rootitem,
				    pixmap,
				    (double) (width/NUMBER_OF_ITEMS) * 4 -
				     gdk_pixbuf_get_width(pixmap)/2,
				    (double) (height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
				     NULL);
  gdk_pixbuf_unref(pixmap);

  setup_item_signals(help_item, "help");

  // CONFIG
  if(properties->disable_config == 0)
    {
      pixmap = gc_skin_pixmap_load("config.png");
      zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
      config_item = goo_canvas_image_new (rootitem,
					  pixmap,
					  (double) (width/NUMBER_OF_ITEMS) * 3 -
					   gdk_pixbuf_get_width(pixmap)/2,
					  (double) (height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
					   NULL);
      gdk_pixbuf_unref(pixmap);

      setup_item_signals(config_item, "configuration");
    }

  // ABOUT
  pixmap = gc_skin_pixmap_load("about.png");
  zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
  about_item = goo_canvas_image_new (rootitem,
				     pixmap,
				     (double) (width/NUMBER_OF_ITEMS) * 2 -
				     gdk_pixbuf_get_width(pixmap)/2,
				     (double) (height-gdk_pixbuf_get_height(pixmap)*zoom)/2,
				     NULL);
  gdk_pixbuf_unref(pixmap);

  setup_item_signals(about_item, "about");

  // Show them all
  update_exit_button();
  g_object_set (level_item,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);
  g_object_set (ok_item,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);
  g_object_set (repeat_item,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);

  if(config_item)
    g_object_set (config_item,
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);

  g_object_set (about_item,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);

  _hidden = FALSE;
  _force_bar_down(NULL);
}


void gc_bar_set_level(GcomprisBoard *gcomprisBoard)
{
  char *str = NULL;
  GdkPixbuf *pixmap = NULL;

  goo_canvas_item_raise(rootitem, NULL);
  /* Non yet initialized : Something Wrong */
  if(level_item==NULL)
    {
      g_message("in bar_set_level, level_item uninitialized : should not happen\n");
      return;
    }

  if(gcomprisBoard!=NULL)
    {

      str = g_strdup_printf("level%d.png", gcomprisBoard->level);
      pixmap = gc_skin_pixmap_load(str);

      g_free(str);

      g_object_set (level_item,
		    "pixbuf", pixmap,
		    NULL);
      gdk_pixbuf_unref(pixmap);

    }

  current_level=gcomprisBoard->level;
}



/* gc_bar_set_repeat_icon
 *
 * Override the repeat icon to a new one specific to your current board.
 * This must be called before calling gc_bar_set with GC_BAR_REPEAT_ICON
 * the given pixmap is not freed.
 *
 * Next call to gc_bar_set with GC_BAR_REPEAT will restore the default icon.
 *
 */
void
gc_bar_set_repeat_icon (GdkPixbuf *pixmap)
{
  goo_canvas_item_raise(rootitem, NULL);
  /* Non yet initialized : Something Wrong */
  if(level_item==NULL)
    {
      g_message("in bar_set_level, level_item uninitialized : should not happen\n");
      return;
    }

  g_object_set (repeat_item,
		"pixbuf", pixmap,
		NULL);
}

/* Setting list of available icons in the control bar */
void
gc_bar_set (const GComprisBarFlags flags)
{

  _hidden = FALSE;
  goo_canvas_item_raise(rootitem, NULL);

  /* Non yet initialized : Something Wrong */
  if(level_item==NULL)
    {
      g_message("in bar_set_level, level_item uninitialized : should not happen\n");
      return;
    }

  current_flags = flags;

  update_exit_button();

  if(flags&GC_BAR_LEVEL)
    g_object_set (level_item,
		  "visibility", GOO_CANVAS_ITEM_VISIBLE,
		  NULL);
  else
    g_object_set (level_item,
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);

  if(flags&GC_BAR_OK)
    g_object_set (ok_item,
		  "visibility", GOO_CANVAS_ITEM_VISIBLE,
		  NULL);
  else
    g_object_set (ok_item,
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);

  if(gc_help_has_board(gc_board_get_current()))
    g_object_set (help_item,
		  "visibility", GOO_CANVAS_ITEM_VISIBLE,
		  NULL);
  else
    g_object_set (help_item,
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);

  if(flags&GC_BAR_REPEAT) {
    GdkPixbuf *pixmap;

    /* Set the repeat icon to the original one */
    pixmap = gc_skin_pixmap_load("repeat.png");
    g_object_set (repeat_item,
		  "pixbuf", pixmap,
		  NULL);
    gdk_pixbuf_unref(pixmap);

    g_object_set (repeat_item,
		  "visibility", GOO_CANVAS_ITEM_VISIBLE,
		  NULL);
  } else {

    if(flags&GC_BAR_REPEAT_ICON)
      g_object_set (repeat_item,
		    "visibility", GOO_CANVAS_ITEM_VISIBLE,
		    NULL);
    else
      g_object_set (repeat_item,
		    "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		    NULL);
  }

  if(flags&GC_BAR_CONFIG && config_item)
    g_object_set (config_item,
		  "visibility", GOO_CANVAS_ITEM_VISIBLE,
		  NULL);
  else
    g_object_set (config_item,
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);

  if(flags&GC_BAR_ABOUT)
    g_object_set(about_item,
		 "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
  else
    g_object_set(about_item,
		 "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

  /* FIXME : Workaround for bugged canvas */
  //  goo_canvas_update_now(gc_board_get_current()->canvas);

}

static gboolean
_bar_down(void *ignore)
{
  bar_down_id = 0;
  goo_canvas_item_animate(rootitem,
			  0,
			  BOARDHEIGHT - 20,
			  1,
			  0,
			  TRUE,
			  1000,
			  80,
			  GOO_CANVAS_ANIMATE_FREEZE);

  return(FALSE);
}

static void
_force_bar_down()
{
  if(bar_down_id)
    g_source_remove (bar_down_id);

  bar_down_id=0;

  /* Hide it faster than normal */
  goo_canvas_item_animate(rootitem,
			  0,
			  BOARDHEIGHT - 20,
			  1,
			  0,
			  TRUE,
			  100,
			  10,
			  GOO_CANVAS_ANIMATE_FREEZE);
}

static void
_bar_up()
{
  goo_canvas_item_raise(rootitem, NULL);
  goo_canvas_item_animate(rootitem,
			  0,
			  BOARDHEIGHT - BARHEIGHT,
			  1,
			  0,
			  TRUE,
			  700,
			  80,
			  GOO_CANVAS_ANIMATE_FREEZE);
}

/* Hide all icons in the control bar
 * or retore the icons to the previous value
 */
void
gc_bar_hide (gboolean hide)
{
  /* Non yet initialized : Something Wrong */
  g_assert(level_item);

  _hidden = hide;

  if(hide)
    {
      if(exit_item)
	g_object_set(exit_item,
		     "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      g_object_set(home_item,
		   "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      g_object_set(level_item,
		   "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      g_object_set(ok_item,
		   "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      g_object_set(help_item,
		   "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      g_object_set(repeat_item,
		   "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      if(config_item)
	g_object_set(config_item,
		     "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      g_object_set(about_item,
		   "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

      _force_bar_down();
    }
  else
    {
      g_object_set(home_item,
		   "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
      gc_bar_set(current_flags);

    }

}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

/*
 * Display or not the exit button
 */
static void update_exit_button()
{

  if (!gc_board_get_current() || gc_board_get_current()->previous_board == NULL)
    {
      /* We are in the upper menu: show it */
      if(exit_item)
	g_object_set(exit_item,
		     "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
      g_object_set(home_item,
		   "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
    }
  else
    {
      if(exit_item)
	g_object_set(exit_item,
		     "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      g_object_set(home_item,
		   "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
    }
}

/*
 * This is called to play sound
 *
 */
static gint bar_play_sound (gchar *sound)
{
  int policy = gc_sound_policy_get();
  gchar *str;
  gc_sound_policy_set(PLAY_ONLY_IF_IDLE);

  str = g_strdup_printf("voices/$LOCALE/misc/%s.ogg", sound);

  gc_sound_play_ogg(str, NULL);

  g_free(str);

  gc_sound_policy_set(policy);
  sound_play_id = 0;
  return (FALSE);
}

static void bar_reset_sound_id ()
{
  if(sound_play_id)
    g_source_remove (sound_play_id);

  sound_play_id=0;
}

static gboolean
on_enter_notify (GooCanvasItem  *item,
		 GooCanvasItem  *target,
		 GdkEventCrossing *event,
		 char *data)
{
  if(_hidden)
    return FALSE;

  bar_reset_sound_id();
  sound_play_id = g_timeout_add (1000, (GtkFunction) bar_play_sound, data);
  _bar_up();

  if(bar_down_id)
    g_source_remove (bar_down_id);

  bar_down_id=0;

  return FALSE;
}

static gboolean
on_leave_notify (GooCanvasItem  *item,
		 GooCanvasItem  *target,
		 GdkEventCrossing *event,
		 char *data)
{
  bar_reset_sound_id();

  if(!bar_down_id)
    bar_down_id = g_timeout_add (HIDE_BAR_TIMOUT, (GtkFunction) _bar_down, NULL);

  return FALSE;
}

/* Callback for the bar operations */
static gboolean
item_event_bar (GooCanvasItem  *item,
		GooCanvasItem  *target,
		GdkEventButton *event,
		gchar *data)
{
  GcomprisBoard *gcomprisBoard = gc_board_get_current();

  if(_hidden)
    return(FALSE);

  bar_reset_sound_id();
  gc_sound_play_ogg ("sounds/bleep.wav", NULL);

  if(!strcmp((char *)data, "ok"))
    {
      if(gcomprisBoard && gcomprisBoard->plugin->ok != NULL)
	gcomprisBoard->plugin->ok();
    }
  else if(!strcmp((char *)data, "level"))
    {
      gint tmp = current_level;

      if(event->button == 1)
	{
	  current_level++;
	  if(gcomprisBoard && current_level>gcomprisBoard->maxlevel)
	    current_level=1;
	}
      else
	{
	  /* Decrease the level */
	  current_level--;
	  if(current_level<1)
	    current_level=1;
	}

      if(tmp!=current_level)
	{
	  gchar *str_number;

	  gchar *number_str = g_strdup_printf("%d", current_level);
	  gchar *current_level_str = gc_sound_alphabet(number_str);
	  g_free(number_str);

	  str_number = g_strdup_printf("voices/$LOCALE/alphabet/%s", current_level_str);

	  gc_sound_play_ogg("voices/$LOCALE/misc/level.ogg", str_number, NULL);

	  g_free(str_number);
	  g_free(current_level_str);

	  if(gcomprisBoard && gcomprisBoard->plugin->set_level != NULL)
	    {
	      gcomprisBoard->plugin->set_level(current_level);
	    }
	}
    }
  else if(!strcmp((char *)data, "back"))
    {
      gc_bar_hide (TRUE);
      gc_board_stop();
    }
  else if(!strcmp((char *)data, "help"))
    {
      gc_help_start(gcomprisBoard);
    }
  else if(!strcmp((char *)data, "repeat"))
    {
      if(gcomprisBoard && gcomprisBoard->plugin->repeat != NULL)
	{
	  gcomprisBoard->plugin->repeat();
	}
    }
  else if(!strcmp((char *)data, "configuration"))
    {
      if(gcomprisBoard && gcomprisBoard->plugin->config_start != NULL)
	{
	  gcomprisBoard->plugin->config_start(gcomprisBoard,
					      gc_profile_get_current());
	}
    }
  else if(!strcmp((char *)data, "about"))
    {
      gc_about_start();
    }
  else if(!strcmp((char *)data, "quit"))
    {
      GcomprisProperties *properties = gc_prop_get();

      if(strlen(properties->root_menu) == 1)
	gc_confirm_box( _("GCompris confirmation"),
			_("Are you sure you want to quit?"),
			_("Yes, I am sure!"),
			_("No, I want to keep going"),
			(ConfirmCallBack) confirm_quit);
      else
	confirm_quit(TRUE);
    }
  else if(!strcmp((char *)data, "bar"))
    {
      _force_bar_down();
    }

  return TRUE;
}

static void
confirm_quit(gboolean answer)
{
  if (answer)
    gc_exit();
}

static void
setup_item_signals (GooCanvasItem *item, gchar* name)
{
  g_signal_connect (item, "enter_notify_event",
		    (GtkSignalFunc) on_enter_notify, name);
  g_signal_connect (item, "leave_notify_event",
		    (GtkSignalFunc) on_leave_notify, name);
  g_signal_connect(item, "button_press_event",
		   (GtkSignalFunc) item_event_bar,
		   name);

  if(strcmp(name, "bar"))
    gc_item_focus_init(item, NULL);
}
