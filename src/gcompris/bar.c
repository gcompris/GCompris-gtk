/* gcompris - bar.c
 *
 * Copyright (C) 2000, 2008 Bruno Coudoin
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

#define BAR_GAP		10	/* Value used to fill space above and under icons in the bar */
#define NUMBER_OF_ITEMS 5	/* Max Number of buttons in the bar                              */

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
static gint	 bar_play_sound (gchar *sound);
static void	 play_level_voice(int level);

static gint current_level = -1;
static gint current_flags = -1;
static GooCanvasItem *bar_item  = NULL;
static GooCanvasItem *exit_item = NULL;
static GooCanvasItem *home_item = NULL;
static GooCanvasItem *level_up_item = NULL;
static GooCanvasItem *level_down_item = NULL;
static GooCanvasItem *level_item = NULL;
static GooCanvasItem *help_item = NULL;
static GooCanvasItem *repeat_item = NULL;
static GooCanvasItem *config_item = NULL;
static GooCanvasItem *about_item = NULL;
static GooCanvasItem *rootitem = NULL;

static gint sound_play_id = 0;
static gboolean _hidden;     /* Dialog boxes request a bar hide */

/* Default position for the bar */
static int _default_x;
static int _default_y;
static int _default_zoom;

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
  gint16       width, height;
  gint16       buttony;
  double       zoom;

  width  = BARWIDTH;
  height = BARHEIGHT-2;

  _default_x = width / 2;
  _default_y = BOARDHEIGHT - BARHEIGHT;
  _default_zoom = 1.0;

  bar_reset_sound_id();

  rootitem = goo_canvas_group_new (goo_canvas_get_root_item(theCanvas), NULL);
  goo_canvas_item_translate(rootitem, _default_x, _default_y);

  pixmap = gc_skin_pixmap_load("bar_bg.png");
  bar_item = goo_canvas_image_new (rootitem,
				   pixmap,
				   0,
				   0,
				NULL);
  setup_item_signals(bar_item, "bar");
  gdk_pixbuf_unref(pixmap);

  zoom = (double)(height-BAR_GAP)/(double)gdk_pixbuf_get_height(pixmap);
  buttony = (height-gdk_pixbuf_get_height(pixmap)*zoom)/2 - 20;

  // EXIT
  if(properties->disable_quit == 0)
    {
      pixmap = gc_skin_pixmap_load("button_exit.png");
      exit_item = goo_canvas_image_new (rootitem,
					pixmap,
					(width/NUMBER_OF_ITEMS) * 0,
					buttony,
					NULL);
      gdk_pixbuf_unref(pixmap);

      setup_item_signals(exit_item, "quit");
    }

  // HOME
  pixmap = gc_skin_pixmap_load("home.png");
  home_item = goo_canvas_image_new (rootitem,
				    pixmap,
				    (width/NUMBER_OF_ITEMS) * 4,
				    buttony,
				     NULL);
  gdk_pixbuf_unref(pixmap);

  setup_item_signals(home_item, "back");


  // LEVEL
  pixmap = gc_skin_pixmap_load("level_up.png");
  int level_up_height = gdk_pixbuf_get_height(pixmap);
  int level_up_width = gdk_pixbuf_get_width(pixmap);
  level_up_item = goo_canvas_image_new (rootitem,
				     pixmap,
				     (width/NUMBER_OF_ITEMS) * 3,
				     buttony,
				     NULL);
  gdk_pixbuf_unref(pixmap);
  setup_item_signals(level_up_item, "level_up");

  pixmap = gc_skin_pixmap_load("level_down.png");
  level_down_item = goo_canvas_image_new (rootitem,
				     pixmap,
				     (width/NUMBER_OF_ITEMS) * 3,
				     buttony + level_up_height,
				     NULL);
  gdk_pixbuf_unref(pixmap);
  setup_item_signals(level_down_item, "level_down");

  current_level = 1;

  level_item =
    goo_canvas_text_new (rootitem,
			 "1",
			 (width/NUMBER_OF_ITEMS) * 3 + level_up_width/2,
			 buttony + 30,
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_board_title_bold,
			 "fill-color-rgba", gc_skin_get_color("menu/text"),
			 "alignment", PANGO_ALIGN_CENTER,
			 NULL);

  // REPEAT
  pixmap = gc_skin_pixmap_load("repeat.png");
  repeat_item = goo_canvas_image_new (rootitem,
				      pixmap,
				      (width/NUMBER_OF_ITEMS) * 0,
				      buttony,
				       NULL);
  gdk_pixbuf_unref(pixmap);

  setup_item_signals(repeat_item, "repeat");


  // HELP
  pixmap = gc_skin_pixmap_load("help.png");
  help_item = goo_canvas_image_new (rootitem,
				    pixmap,
				    (width/NUMBER_OF_ITEMS) * 1,
				    buttony,
				    NULL);
  gdk_pixbuf_unref(pixmap);

  setup_item_signals(help_item, "help");

  // CONFIG
  if(properties->disable_config == 0)
    {
      pixmap = gc_skin_pixmap_load("config.png");
      config_item = goo_canvas_image_new (rootitem,
					  pixmap,
					  (width/NUMBER_OF_ITEMS) * 2,
					  buttony,
					  NULL);
      gdk_pixbuf_unref(pixmap);

      setup_item_signals(config_item, "configuration");
    }

  // ABOUT
  pixmap = gc_skin_pixmap_load("about.png");
  about_item = goo_canvas_image_new (rootitem,
				     pixmap,
				     (width/NUMBER_OF_ITEMS) * 3,
				     buttony,
				     NULL);
  gdk_pixbuf_unref(pixmap);

  setup_item_signals(about_item, "about");

  // Show them all
  update_exit_button();
  g_object_set (level_up_item,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);
  g_object_set (level_down_item,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);
  g_object_set (level_up_item,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);
  g_object_set (level_down_item,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);
  g_object_set (level_item,
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
}


void gc_bar_set_level(GcomprisBoard *gcomprisBoard)
{
  char *str = NULL;

  goo_canvas_item_raise(rootitem, NULL);
  /* Non yet initialized : Something Wrong */
  if(level_item==NULL)
    {
      g_message("in bar_set_level, level_item uninitialized : should not happen\n");
      return;
    }

  if(gcomprisBoard!=NULL)
    {

      str = g_strdup_printf("%d", gcomprisBoard->level);

      g_object_set (level_item,
		    "text", str,
		    NULL);

      g_free(str);

    }

  current_level = gcomprisBoard->level;
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

/** Setting the bar location
 * @param[in] x the bar x coordinate, -1 to set the default
 * @param[in] y the bar x coordinate, -1 to set the default
 * @param[in] zoom the bar zoom factor, -1 to set the default
 */
void
gc_bar_location (int x, int y, double zoom)
{
  goo_canvas_item_set_transform(rootitem, NULL);
  goo_canvas_item_translate(rootitem,
			    (x == -1 ? _default_x : x),
			    (y == -1 ? _default_y : y));
  goo_canvas_item_scale(rootitem,
			(zoom == -1 ? _default_zoom : zoom),
			(zoom == -1 ? _default_zoom : zoom));

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
    {
    g_object_set (level_up_item,
		  "visibility", GOO_CANVAS_ITEM_VISIBLE,
		  NULL);
    g_object_set (level_down_item,
		  "visibility", GOO_CANVAS_ITEM_VISIBLE,
		  NULL);
    g_object_set (level_item,
		  "visibility", GOO_CANVAS_ITEM_VISIBLE,
		  NULL);
    }
  else
    {
    g_object_set (level_up_item,
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);
    g_object_set (level_down_item,
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);
    g_object_set (level_item,
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);
    }

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
}

/* Hide all icons in the control bar
 * or restore the icons to the previous value
 */
void
gc_bar_hide (gboolean hide)
{
  /* Non yet initialized : Something Wrong */
  g_assert(level_item);

  _hidden = hide;

  if(hide)
    {
      g_object_set(rootitem,
		   "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
    }
  else
    {
      g_object_set(rootitem,
		   "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
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

  sound_play_id = g_timeout_add (1000, (GtkFunction) bar_play_sound, data);

  return FALSE;
}

static gboolean
on_leave_notify (GooCanvasItem  *item,
		 GooCanvasItem  *target,
		 GdkEventCrossing *event,
		 char *data)
{
  bar_reset_sound_id();

  return FALSE;
}

/** Play the audio number given in @level
 */
static void
play_level_voice(int level)
{
  /* Play the audio level number */
  gchar *number_str = g_strdup_printf("%d", level);

  if ( level < 10 )
    {
      /* Set the number as unicode */
      gchar *level_str = gc_sound_alphabet(number_str);
      g_free(number_str);
      number_str = level_str;
    }

  gchar *audio_str = g_strdup_printf("voices/$LOCALE/alphabet/%s", number_str);

  gc_sound_play_ogg("voices/$LOCALE/misc/level.ogg", audio_str, NULL);

  g_free(number_str);
  g_free(audio_str);
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
  else if(!strcmp((char *)data, "level_up"))
    {
	  current_level++;
	  if(gcomprisBoard && current_level > gcomprisBoard->maxlevel)
	    current_level=1;

	  /* Set the level */
	  if(gcomprisBoard && gcomprisBoard->plugin->set_level != NULL)
	    gcomprisBoard->plugin->set_level(current_level);

	  play_level_voice(current_level);
    }
  else if(!strcmp((char *)data, "level_down"))
    {
      /* Decrease the level */
      current_level--;
      if(current_level < 1)
	current_level = gcomprisBoard->maxlevel;

      /* Set the level */
      if(gcomprisBoard && gcomprisBoard->plugin->set_level != NULL)
	gcomprisBoard->plugin->set_level(current_level);

      play_level_voice(current_level);
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
