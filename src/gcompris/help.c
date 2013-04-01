/* gcompris - help.c
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

#define SOUNDLISTFILE PACKAGE

#define GAP_TO_BUTTON -30

static gboolean item_event_help (GooCanvasItem  *item,
				 GooCanvasItem  *target,
				 GdkEventButton *event,
				 gchar *data);
static int	 event_disable_right_click_popup(GtkWidget *w, GdkEvent *event, gpointer data);
static void	 select_item(GooCanvasItem *item, GooCanvasItem *item_text);
static void	 set_content(char *text);

static gboolean help_displayed			= FALSE;

static GooCanvasItem *rootitem		= NULL;

static gchar *prerequisite			= NULL;
static gchar *goal				= NULL;
static gchar *manual				= NULL;
static gchar *credit				= NULL;

static GooCanvasItem *item_prerequisite	= NULL;
static GooCanvasItem *item_goal		= NULL;
static GooCanvasItem *item_manual		= NULL;
static GooCanvasItem *item_credit		= NULL;

static GooCanvasItem *item_prerequisite_text  = NULL;
static GooCanvasItem *item_goal_text          = NULL;
static GooCanvasItem *item_manual_text        = NULL;
static GooCanvasItem *item_credit_text        = NULL;

static GooCanvasItem *item_selected		= NULL;
static GooCanvasItem *item_selected_text	= NULL;

static GtkTextBuffer *buffer_content;
static guint	      caller_cursor;

/*
 * Main entry point
 * ----------------
 *
 */

/**
 * Return true wether the given board has at least one help field defined
 *
 */
gboolean gc_help_has_board (GcomprisBoard *gcomprisBoard)
{
  if(!gcomprisBoard)
    return FALSE;

  prerequisite	= gcomprisBoard->prerequisite;
  goal		= gcomprisBoard->goal;
  manual	= gcomprisBoard->manual;
  credit	= gcomprisBoard->credit;
  return(prerequisite!=NULL || goal!=NULL || manual!=NULL || credit!=NULL);
}

/*
 * Do all the help display and register the events
 */

void gc_help_start (GcomprisBoard *gcomprisBoard)
{

  gchar *item_id = "#UP";
  GooCanvasItem *item;
  gint y = 0;
  gint y_start = 0;
  gint x_start = 0;
  gchar   *name = NULL;
  gchar   *text_to_display = NULL;

  if(rootitem)
  {
    gc_help_stop();
    return;
  }

  gc_board_pause(TRUE);

  caller_cursor = gc_cursor_get();
  gc_cursor_set(GCOMPRIS_DEFAULT_CURSOR);

  item_selected = NULL;
  item_selected_text = NULL;

  name = gcomprisBoard->title;
  gc_help_has_board(gcomprisBoard);

  rootitem = goo_canvas_group_new (goo_canvas_get_root_item(gc_get_canvas()),
				   NULL);

  item = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", "#DIALOG",
			     "pointer-events", GOO_CANVAS_EVENTS_NONE,
			     NULL);

  GooCanvasBounds bounds;
  guint pixmap_width = 40;
  goo_canvas_item_get_bounds(item, &bounds);
  x_start = bounds.x1;
  y_start = bounds.y1;

  y = bounds.y2 - 26;

  y_start += 15;
  if(gcomprisBoard->section && gcomprisBoard->name) {
    text_to_display = g_strdup_printf("%s/%s", gcomprisBoard->section, gcomprisBoard->name);
     goo_canvas_text_new (rootitem,
			  text_to_display,
			  BOARDWIDTH*0.10,
			  y_start,
			  -1,
			  GTK_ANCHOR_NW,
			  "font", gc_skin_font_board_tiny,
			  "fill-color-rgba", gc_skin_color_title,
			  NULL);
    g_free(text_to_display);
  }

  y_start += 30;
  goo_canvas_text_new (rootitem,
		       name,
		       BOARDWIDTH/2,
		       y_start,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_subtitle,
		       "fill-color-rgba", gc_skin_color_title,
		       NULL);


  y_start += 80;

  // Prerequisite Button
  if(prerequisite)
    {
      item_prerequisite = goo_canvas_svg_new (rootitem,
					      gc_skin_rsvg_get(),
					      "svg-id", item_id,
					      NULL);
      SET_ITEM_LOCATION(item_prerequisite,
			(BOARDWIDTH*0.2) - pixmap_width/2,
			y_start  - 10)

      g_signal_connect(item_prerequisite, "button_press_event",
			 (GCallback) item_event_help,
			 "prerequisite");
      gc_item_focus_init(item_prerequisite, NULL);

      item_prerequisite_text = \
	goo_canvas_text_new (rootitem,
			     _("Prerequisite"),
			     BOARDWIDTH*0.20,
			     y_start   + GAP_TO_BUTTON,
			     -1,
			     GTK_ANCHOR_CENTER,
			     "font", gc_skin_font_content,
			     "fill-color-rgba", gc_skin_get_color("gcompris/helpunselect"),
			     NULL);
      g_signal_connect(item_prerequisite_text, "button_press_event",
			 (GCallback) item_event_help,
			 "prerequisite");
    }

  // Goal Button
  if(goal)
    {
      item_goal = goo_canvas_svg_new (rootitem,
				      gc_skin_rsvg_get(),
				      "svg-id", item_id,
				      NULL);
      SET_ITEM_LOCATION(item_goal,
			(BOARDWIDTH*0.4) - pixmap_width/2,
			y_start  - 10);

      g_signal_connect(item_goal, "button_press_event",
			 (GCallback) item_event_help,
			 "goal");
      gc_item_focus_init(item_goal, NULL);

      item_goal_text = goo_canvas_text_new (rootitem,
					    _("Goal"),
					    BOARDWIDTH*0.4,
					    y_start   + GAP_TO_BUTTON,
					    -1,
					    GTK_ANCHOR_CENTER,
					    "font", gc_skin_font_content,
					    "fill-color-rgba", gc_skin_get_color("gcompris/helpunselect"),
					    NULL);
      g_signal_connect(item_goal_text, "button_press_event",
			 (GCallback) item_event_help,
			 "goal");
    }

  // Manual Button
  if(manual)
    {
      item_manual = goo_canvas_svg_new (rootitem,
					gc_skin_rsvg_get(),
					"svg-id", item_id,
					NULL);
      SET_ITEM_LOCATION(item_manual,
			(BOARDWIDTH*0.6) - pixmap_width/2,
			y_start  - 10);

      g_signal_connect(item_manual, "button_press_event",
			 (GCallback) item_event_help,
			 "manual");
      gc_item_focus_init(item_manual, NULL);

      item_manual_text = goo_canvas_text_new (rootitem,
					      _("Manual"),
					      BOARDWIDTH*0.6,
					      y_start   + GAP_TO_BUTTON,
					      -1,
					      GTK_ANCHOR_CENTER,
					      "font", gc_skin_font_content,
					      "fill-color-rgba", gc_skin_get_color("gcompris/helpunselect"),
					      NULL);
      g_signal_connect(item_manual_text, "button_press_event",
			 (GCallback) item_event_help,
			 "manual");
    }

  // Credit Button
  if(credit)
    {
      item_credit = goo_canvas_svg_new (rootitem,
					gc_skin_rsvg_get(),
					"svg-id", item_id,
					NULL);
      SET_ITEM_LOCATION(item_credit,
			(BOARDWIDTH*0.8) - pixmap_width/2,
			y_start  - 10);

      g_signal_connect(item_credit, "button_press_event",
			 (GCallback) item_event_help,
			 "credit");
      gc_item_focus_init(item_credit, NULL);

      item_credit_text = goo_canvas_text_new (rootitem,
					      _("Credit"),
					      BOARDWIDTH*0.8,
					      y_start   + GAP_TO_BUTTON,
					      -1,
					      GTK_ANCHOR_CENTER,
					      "font", gc_skin_font_content,
					      "fill-color-rgba", gc_skin_get_color("gcompris/helpunselect"),
					      NULL);
      g_signal_connect(item_credit_text, "button_press_event",
			 (GCallback) item_event_help,
			 "credit");
    }

  // CONTENT

  // default text to display

  if(prerequisite)
    {
      text_to_display = prerequisite;
      select_item(item_prerequisite, item_prerequisite_text);
    }
  else if(goal)
    {
      text_to_display = goal;
      select_item(item_goal, item_goal_text);
    }
  else if(manual)
    {
      text_to_display = manual;
      select_item(item_manual, item_manual_text);
    }
  else if(credit)
    {
      text_to_display = credit;
      select_item(item_credit, item_credit_text);
    }

  y_start += 45;

  /* Create a scrolled area for the text content */
  GtkWidget *view;
  GtkWidget *sw;
  view = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
  gtk_text_view_set_editable(GTK_TEXT_VIEW (view), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW (view), FALSE);

  g_signal_connect(GTK_OBJECT(view), "button-press-event",
		   (GCallback) event_disable_right_click_popup, NULL);

  PangoFontDescription *font_desc;
  font_desc = pango_font_description_from_string (gc_skin_font_content);
  gtk_widget_modify_font (view, font_desc);
  pango_font_description_free (font_desc);

  GdkColor fg_color;
  GdkColor bg_color;
  gc_skin_get_gdkcolor("gcompris/helpfg", &fg_color);
  gc_skin_get_gdkcolor("gcompris/helpbg", &bg_color);
  gtk_widget_modify_base(view, GTK_STATE_NORMAL, &bg_color);
  gtk_widget_modify_text(view, GTK_STATE_NORMAL, &fg_color);

  buffer_content = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (sw), view);

  goo_canvas_widget_new (rootitem,
			 sw,
			 x_start + 40,
			 y_start,
			 618.0,
			 270.0,
			 NULL);
  gtk_widget_show_all (sw);

  set_content(text_to_display);
  // OK
  gc_util_button_text_svg(rootitem,
			  BOARDWIDTH * 0.5,
			  y,
			  "#BUTTON_TEXT",
			  _("OK"),
			  (GCallback) item_event_help,
			  "ok");

  gc_bar_hide(TRUE);

  help_displayed = TRUE;
}

/*
 * Remove the displayed help.
 * Do nothing if none is currently being dislayed
 */
void gc_help_stop ()
{
  if(help_displayed)
    {
      // Destroy the help box
      if(rootitem!=NULL)
	{
	  goo_canvas_item_remove(rootitem);
	  rootitem = NULL;
	}
      gc_cursor_set(caller_cursor);
      gc_board_pause(FALSE);
    }

  gc_bar_hide(FALSE);
  help_displayed = FALSE;
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

static void select_item(GooCanvasItem *item, GooCanvasItem *item_text)
{
  gchar *item_id;

  if(item_selected)
    {
      item_id = "#UP";
      g_object_set(item_selected,
		   "svg-id", item_id,
		   NULL);
      g_object_set(item_selected_text,
		   "fill-color-rgba", gc_skin_get_color("gcompris/helpunselect"),
		   NULL);

    }

  item_id = "#UP_SELECTED";
  g_object_set(item,
	       "svg-id", item_id,
	       NULL);
  g_object_set(item_text,
	       "fill-color-rgba", gc_skin_get_color("gcompris/helpselect"),
	       NULL);
  item_selected = item;
  item_selected_text = item_text;
}

/* Apply the style to the given RichText item  */
static void set_content(gchar *text)
{
  gtk_text_buffer_set_text (buffer_content, "", 0);
  gtk_text_buffer_insert_at_cursor(buffer_content, text, -1);
}

/* Callback for the bar operations */
static gboolean
item_event_help (GooCanvasItem  *item,
		 GooCanvasItem  *target,
		 GdkEventButton *event,
		 gchar *data)
{

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  if(!strcmp((char *)data, "ok"))
    {
      gc_help_stop();
    }
  else if(!strcmp((char *)data, "prerequisite"))
    {
      select_item(item_prerequisite, item_prerequisite_text);
      set_content(prerequisite);
    }
  else if(!strcmp((char *)data, "goal"))
    {
      select_item(item_goal, item_goal_text);
      set_content(goal);
    }
  else if(!strcmp((char *)data, "manual"))
    {
      select_item(item_manual, item_manual_text);
      set_content(manual);
    }
  else if(!strcmp((char *)data, "credit"))
    {
      select_item(item_credit, item_credit_text);
      set_content(credit);
    }

  return TRUE;
}

/* Textview have a popup on right click to copy/paste. We don't want it */
static int
event_disable_right_click_popup(GtkWidget *w, GdkEvent *event, gpointer data)
{
  if(event->button.button == 3)
    return TRUE;
  return FALSE;
}
