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

static void	 update_exit_button();
static gboolean  on_enter_notify (GooCanvasItem *item,
				  GooCanvasItem *target,
				  GdkEventCrossing *event,
                                  GComprisBarFlags flag);
static gboolean  on_leave_notify (GooCanvasItem *item,
				  GooCanvasItem *target,
				  GdkEventCrossing *event,
                                  GComprisBarFlags flag);
static gboolean item_event_bar (GooCanvasItem  *item,
				GooCanvasItem  *target,
				GdkEventButton *event,
                                GComprisBarFlags flag);
static void	 bar_reset_sound_id (void);
static gint	 bar_play_sound (GooCanvasItem *item);
static void	 play_level_voice(int level);

static gint current_level = -1;
static gint current_flags = 0;
static GooCanvasItem *bar_item  = NULL;
static GooCanvasItem *rootitem = NULL;
static GooCanvasItem *level_item;

static gint sound_play_id = 0;
static gboolean _hidden;     /* Dialog boxes request a bar hide */

/* Default position for the bar */
static int _default_y;
static int _default_zoom;

static void  confirm_quit(gboolean answer);

static GSList *buttons = NULL;

/*

 * Main entry point
 * ----------------
 *
 */

gint
compare_flag(GooCanvasItem *item,
             GComprisBarFlags flag)
{
  return (GPOINTER_TO_UINT(g_object_get_data(G_OBJECT (item),
                                             "flag")) != flag);
}

/* Return the item for the given flag or NULL if non existant */
GooCanvasItem *
get_item(GComprisBarFlags flag)
{
  GSList *node =
    g_slist_find_custom (buttons,
                         GUINT_TO_POINTER(flag),
                         (GCompareFunc) compare_flag);
  if (!node)
    return NULL;

  return (GooCanvasItem *)node->data;
}

void
item_visibility(GComprisBarFlags flag,
                gboolean visible)
{
  GooCanvasItem *item = get_item(flag);
  g_assert(item);

  if (visible)
    g_object_set (item,
                  "visibility", GOO_CANVAS_ITEM_VISIBLE,
                  NULL);
  else
    g_object_set (item,
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);
}

/* Return a new button item */
GooCanvasItem *
new_button(GooCanvasItem *rootitem,
	   RsvgHandle *svg_handle,
           GComprisBarFlags flag,
           gchar *svg_id)
{
  GooCanvasItem *item =
    goo_canvas_svg_new (rootitem,
			svg_handle,
			"svg-id", svg_id,
			NULL);
  SET_ITEM_LOCATION(item, 0, 0);

  g_object_set (item,
                "visibility", GOO_CANVAS_ITEM_VISIBLE,
                NULL);
  g_object_set_data (G_OBJECT(item), "flag",
		     GUINT_TO_POINTER(flag));
  g_signal_connect (item, "enter_notify_event",
		    (GtkSignalFunc) on_enter_notify,
                    GUINT_TO_POINTER(flag));
  g_signal_connect (item, "leave_notify_event",
		    (GtkSignalFunc) on_leave_notify,
                    GUINT_TO_POINTER(flag));
  g_signal_connect(item, "button_press_event",
		   (GtkSignalFunc) item_event_bar,
		   GUINT_TO_POINTER(flag));
  return item;
}

/*
 * Do all the bar display and register the events
 */
void gc_bar_start (GooCanvas *theCanvas)
{
  GcomprisProperties *properties = gc_prop_get();
  gint16       height;
  gint16       buttony;
  double       zoom;
  RsvgHandle  *svg_handle;
  GooCanvasBounds bounds;

  height = BARHEIGHT-2;

  _default_y = BOARDHEIGHT - BARHEIGHT;
  _default_zoom = 1.0;

  bar_reset_sound_id();

  rootitem = goo_canvas_group_new (goo_canvas_get_root_item(theCanvas), NULL);

  svg_handle = gc_skin_rsvg_get();
  bar_item = goo_canvas_svg_new (rootitem,
				 svg_handle,
				 "svg-id", "#BAR_BG",
				 "pointer-events", GOO_CANVAS_EVENTS_NONE,
				 NULL);
  SET_ITEM_LOCATION(bar_item, 0, 0);

  goo_canvas_item_get_bounds(bar_item, &bounds);
  zoom = (double)(height-BAR_GAP)/(bounds.y2 - bounds.y1);
  buttony = (height-(bounds.y2 - bounds.y1)*zoom)/2 - 20;

  /*
   * The Order in which buttons are created represents
   * also the order in which they will be displayed
   */

  // EXIT
  if(properties->disable_quit == 0)
    buttons = g_slist_append(buttons,
                             new_button(rootitem,
					svg_handle,
                                        GC_BAR_EXIT,
                                        "#EXIT"));

  // ABOUT
  buttons = g_slist_append(buttons,
                           new_button(rootitem,
				      svg_handle,
                                      GC_BAR_ABOUT,
                                      "#ABOUT"));

  // CONFIG
  if(properties->disable_config == 0)
    buttons = g_slist_append(buttons,
                             new_button(rootitem,
					svg_handle,
                                        GC_BAR_CONFIG,
                                        "#CONFIG"));

  // HELP
  buttons = g_slist_append(buttons,
                           new_button(rootitem,
				      svg_handle,
                                      GC_BAR_HELP,
                                      "#HELP"));

  // LEVEL (Multiple buttons for this one)
  GooCanvasItem *rootitem_level = goo_canvas_group_new (rootitem, NULL);
  g_object_set (rootitem_level,
                "visibility", GOO_CANVAS_ITEM_INVISIBLE,
                NULL);
  g_object_set_data (G_OBJECT(rootitem_level), "flag",
		     GUINT_TO_POINTER(GC_BAR_LEVEL));
  buttons = g_slist_append(buttons, rootitem_level);

  GooCanvasItem *item = new_button(rootitem_level,
				   svg_handle,
				   GC_BAR_LEVEL_DOWN,
				   "#LEVEL_DOWN");

  g_object_set (item,
                "visibility", GOO_CANVAS_ITEM_VISIBLE,
                NULL);

  item = new_button(rootitem_level,
		    svg_handle,
                    GC_BAR_LEVEL,
                    "#LEVEL_UP");
  goo_canvas_item_translate(item, 50, 0);
  g_object_set (item,
                "visibility", GOO_CANVAS_ITEM_VISIBLE,
                NULL);

  goo_canvas_item_get_bounds(item, &bounds);

  level_item =
    goo_canvas_text_new (rootitem_level,
                         "",
                         bounds.x1 - 10,
                         (bounds.y2 - bounds.y1) / 2 + 8,
                         -1,
                         GTK_ANCHOR_CENTER,
                         "font", gc_skin_font_board_title_bold,
                         "fill-color-rgba", gc_skin_color_text_button,
                         NULL);
  current_level = 1;

  // REPEAT (Default)
  buttons = g_slist_append(buttons,
                           new_button(rootitem,
				      svg_handle,
                                      GC_BAR_REPEAT,
                                      "#REPEAT"));

  // REPEAT ICON
  buttons = g_slist_append(buttons,
                           new_button(rootitem,
				      svg_handle,
                                      GC_BAR_REPEAT_ICON,
                                      "#REPEAT"));

  // HOME
  buttons = g_slist_append(buttons,
                           new_button(rootitem,
				      svg_handle,
                                      GC_BAR_HOME,
                                      "#HOME"));

  update_exit_button();

  gc_bar_set(0);

  _hidden = FALSE;
}


void gc_bar_set_level(GcomprisBoard *gcomprisBoard)
{

  goo_canvas_item_raise(rootitem, NULL);
  /* Non yet initialized : Something Wrong */
  if(get_item(GC_BAR_LEVEL)==NULL)
    {
      g_message("in bar_set_level, level_item uninitialized : should not happen\n");
      return;
    }

  if(gcomprisBoard!=NULL)
    {
      char *str = NULL;
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
 * the given svg_handle is not freed.
 */
void
gc_bar_set_repeat_icon (RsvgHandle *svg_handle)
{
  GooCanvasItem *item;
  goo_canvas_item_raise(rootitem, NULL);
  /* Non yet initialized : Something Wrong */
  if( (item = get_item(GC_BAR_REPEAT_ICON)) == NULL)
    {
      g_message("in bar_set_level, level_item uninitialized : should not happen\n");
      return;
    }

  g_object_set (item,
		"svg-handle", svg_handle,
		"svg-id", NULL,
		NULL);
}

/** Setting the bar location
 * @param[in] x the bar x coordinate, -1 to set the default
 * @param[in] y the bar y coordinate, -1 to set the default
 * @param[in] zoom the bar zoom factor, -1 to set the default
 */
void
gc_bar_location (int x, int y, double zoom)
{
  // Make the y coord be assigned at its bottom
  int ny = (y == -1 ? _default_y : y);
  ny += BARHEIGHT - (zoom == -1 ? _default_zoom : zoom) * BARHEIGHT;

  goo_canvas_item_set_transform(rootitem, NULL);

  GooCanvasBounds bounds;
  goo_canvas_item_get_bounds(rootitem, &bounds);
  int nx = (x == -1 ? (BOARDWIDTH - (bounds.x2 - bounds.x1))/2 : x);
  goo_canvas_item_translate(rootitem, nx, ny);
  goo_canvas_item_scale(rootitem,
			(zoom == -1 ? _default_zoom : zoom),
			(zoom == -1 ? _default_zoom : zoom));
  //#endif
}

/* Setting list of available icons in the control bar */
void
gc_bar_set (const GComprisBarFlags flags)
{
  // Always reset the zoom factor or the calculation
  // will be wrong
  goo_canvas_item_set_transform(rootitem, NULL);

  _hidden = FALSE;
  goo_canvas_item_raise(rootitem, NULL);

  /* Non yet initialized : Something Wrong */
  if(get_item(GC_BAR_LEVEL) == NULL)
    {
      g_message("in bar_set_level, level_item uninitialized : should not happen\n");
      return;
    }

  current_flags = flags;

  if(gc_help_has_board(gc_board_get_current()))
    current_flags |= GC_BAR_HELP;

  if(flags&GC_BAR_ABOUT)
    current_flags |= GC_BAR_ABOUT;

  if(flags&GC_BAR_CONFIG)
    current_flags |= GC_BAR_CONFIG;

  if(flags&GC_BAR_REPEAT_ICON)
    current_flags |= GC_BAR_REPEAT_ICON;

  if(flags&GC_BAR_REPEAT)
    current_flags |= GC_BAR_REPEAT;

  update_exit_button();

  GSList *list;
  double x = 0;
  for (list = buttons; list != NULL; list = list->next)
    {
      GooCanvasItem *item = (GooCanvasItem *)list->data;
      GComprisBarFlags flag =
        GPOINTER_TO_UINT(g_object_get_data(G_OBJECT (item), "flag"));

      if (flag & current_flags)
        {
          GooCanvasBounds bounds;
	  SET_ITEM_LOCATION(item, x, -20);
          goo_canvas_item_get_bounds(item, &bounds);
          gc_item_focus_init(item, NULL);
          x += bounds.x2 - bounds.x1 + BAR_GAP;

          g_object_set (item,
                        "visibility", GOO_CANVAS_ITEM_VISIBLE,
                        NULL);
        }
      else
          g_object_set (item,
                        "visibility", GOO_CANVAS_ITEM_INVISIBLE,
                        NULL);

    }

  /* Scale the bar back to fit the buttons, no more */
  SET_ITEM_LOCATION(bar_item, 0, 0);
  GooCanvasBounds bounds;
  goo_canvas_item_get_bounds(bar_item, &bounds);
  goo_canvas_item_scale(bar_item,
                        x / (bounds.x2 - bounds.x1),
                        1);

  // Always center the bar with its new bounds
  //SET_ITEM_LOCATION(rootitem, 0, _default_y);
  gc_bar_location (-1, -1, -1);
}

/* Hide all icons in the control bar
 * or restore the icons to the previous value
 */
void
gc_bar_hide (gboolean hide)
{
  /* Non yet initialized : Something Wrong */
  g_assert(rootitem);

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

  if (gc_board_get_current() &&
      gc_board_get_current()->previous_board == NULL)
    {
      /* We are in the upper menu: show it */
      current_flags |= GC_BAR_EXIT;
      current_flags &= ~GC_BAR_HOME;
    }
  else
    {
      current_flags &= ~GC_BAR_EXIT;
      current_flags |= GC_BAR_HOME;
    }
}

/*
 * This is called to play sound
 *
 */
static gint bar_play_sound (GooCanvasItem *item)
{
  int policy = gc_sound_policy_get();
  gchar *str;
  gc_sound_policy_set(PLAY_ONLY_IF_IDLE);

  GComprisBarFlags flag =
    GPOINTER_TO_UINT(g_object_get_data(G_OBJECT (item), "flag"));
  str = g_strdup_printf("voices/$LOCALE/misc/%d.ogg", flag);

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
                 GComprisBarFlags flag)
{
  if(_hidden)
    return FALSE;

  sound_play_id =
    g_timeout_add (1000, (GtkFunction) bar_play_sound, item);

  return FALSE;
}

static gboolean
on_leave_notify (GooCanvasItem  *item,
		 GooCanvasItem  *target,
		 GdkEventCrossing *event,
                 GComprisBarFlags flag)
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
		GComprisBarFlags flag)
{
  GcomprisBoard *gcomprisBoard = gc_board_get_current();

  if(_hidden)
    return(FALSE);

  bar_reset_sound_id();
  gc_sound_play_ogg ("sounds/bleep.wav", NULL);

  switch (flag)
    {
    case GC_BAR_LEVEL:
      {
        current_level++;
        if(gcomprisBoard && current_level > gcomprisBoard->maxlevel)
          current_level=1;

        /* Set the level */
        if(gcomprisBoard && gcomprisBoard->plugin->set_level != NULL)
          gcomprisBoard->plugin->set_level(current_level);

        play_level_voice(current_level);
      }
      break;
    case GC_BAR_LEVEL_DOWN:
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
      break;
    case GC_BAR_HOME:
      {
        gc_bar_hide (TRUE);
        gc_board_stop();
      }
      break;
    case GC_BAR_HELP:
      {
        gc_help_start(gcomprisBoard);
      }
      break;
    case GC_BAR_REPEAT_ICON:
    case GC_BAR_REPEAT:
      {
        if(gcomprisBoard && gcomprisBoard->plugin->repeat != NULL)
          {
            gcomprisBoard->plugin->repeat();
          }
      }
      break;
    case GC_BAR_CONFIG:
      {
        if(gcomprisBoard && gcomprisBoard->plugin->config_start != NULL)
          {
            gcomprisBoard->plugin->config_start(gcomprisBoard,
                                                gc_profile_get_current());
          }
      }
      break;
    case GC_BAR_ABOUT:
      {
        gc_about_start();
      }
      break;
    case GC_BAR_EXIT:
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
      break;
    default:
      break;
    }
  return TRUE;
}

  static void
confirm_quit(gboolean answer)
{
  if (answer)
    gc_exit();
}
