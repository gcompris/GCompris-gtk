/* gcompris - help.c
 *
 * Copyright (C) 2000 Bruno Coudoin
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

#define GAP_TO_BUTTON -20

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

static GtkTextBuffer   *buffer_content;
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

  GdkPixbuf   *pixmap = NULL;
  GooCanvasItem *item, *item2;
  gint y = 0;
  gint y_start = 0;
  gint x_start = 0;
  gchar   *name = NULL;
  gchar   *text_to_display = NULL;

  if(rootitem)
    return;

  gc_board_pause(TRUE);

  item_selected = NULL;
  item_selected_text = NULL;

  name = gcomprisBoard->title;
  gc_help_has_board(gcomprisBoard);

  rootitem = goo_canvas_group_new (goo_canvas_get_root_item(gc_get_canvas()),
				   NULL);

  pixmap = gc_skin_pixmap_load("help_bg.png");
  y_start = (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  x_start = (BOARDWIDTH - gdk_pixbuf_get_width(pixmap))/2;
  item = goo_canvas_image_new (rootitem,
			       pixmap,
			       x_start,
			       y_start,
			       NULL);
  y = BOARDHEIGHT - (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  gdk_pixbuf_unref(pixmap);

  y_start += 14;
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

  y_start += 35;
  goo_canvas_text_new (rootitem,
		       name,
		       BOARDWIDTH/2,
		       y_start,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_title,
		       "fill-color-rgba", gc_skin_color_title,
		       NULL);


  y_start += 120;

  pixmap = gc_skin_pixmap_load("button_up.png");

  // Prerequisite Button
  if(prerequisite)
    {
      item_prerequisite = goo_canvas_image_new (rootitem,
						pixmap,
						(BOARDWIDTH*0.2) - gdk_pixbuf_get_width(pixmap)/2,
						y_start - gdk_pixbuf_get_height(pixmap) - 10,
						NULL);

      g_signal_connect(item_prerequisite, "button_press_event",
			 (GtkSignalFunc) item_event_help,
			 "prerequisite");

      item_prerequisite_text = \
	goo_canvas_text_new (rootitem,
			     _("Prerequisite"),
			     BOARDWIDTH*0.20,
			     y_start - gdk_pixbuf_get_height(pixmap)  + GAP_TO_BUTTON,
			     -1,
			     GTK_ANCHOR_CENTER,
			     "font", gc_skin_font_content,
			     "fill-color-rgba", gc_skin_get_color("gcompris/helpunselect"),
			     NULL);
      g_signal_connect(item_prerequisite_text, "button_press_event",
			 (GtkSignalFunc) item_event_help,
			 "prerequisite");
    }

  // Goal Button
  if(goal)
    {
      item_goal = goo_canvas_image_new (rootitem,
					pixmap,
					(BOARDWIDTH*0.4) - gdk_pixbuf_get_width(pixmap)/2,
					y_start - gdk_pixbuf_get_height(pixmap) - 10,
					NULL);

      g_signal_connect(item_goal, "button_press_event",
			 (GtkSignalFunc) item_event_help,
			 "goal");

      item_goal_text = goo_canvas_text_new (rootitem,
					    _("Goal"),
					    BOARDWIDTH*0.4,
					    y_start - gdk_pixbuf_get_height(pixmap)  + GAP_TO_BUTTON,
					    -1,
					    GTK_ANCHOR_CENTER,
					    "font", gc_skin_font_content,
					    "fill-color-rgba", gc_skin_get_color("gcompris/helpunselect"),
					    NULL);
      g_signal_connect(item_goal_text, "button_press_event",
			 (GtkSignalFunc) item_event_help,
			 "goal");
    }

  // Manual Button
  if(manual)
    {
      item_manual = goo_canvas_image_new (rootitem,
					  pixmap,
					  (BOARDWIDTH*0.6) - gdk_pixbuf_get_width(pixmap)/2,
					  y_start - gdk_pixbuf_get_height(pixmap) - 10,
					   NULL);

      g_signal_connect(item_manual, "button_press_event",
			 (GtkSignalFunc) item_event_help,
			 "manual");

      item_manual_text = goo_canvas_text_new (rootitem,
					      _("Manual"),
					      BOARDWIDTH*0.6,
					      y_start - gdk_pixbuf_get_height(pixmap)  + GAP_TO_BUTTON,
					      -1,
					      GTK_ANCHOR_CENTER,
					      "font", gc_skin_font_content,
					      "fill-color-rgba", gc_skin_get_color("gcompris/helpunselect"),
					      NULL);
      g_signal_connect(item_manual_text, "button_press_event",
			 (GtkSignalFunc) item_event_help,
			 "manual");
    }

  // Credit Button
  if(credit)
    {
      item_credit = goo_canvas_image_new (rootitem,
					  pixmap,
					  (BOARDWIDTH*0.8) - gdk_pixbuf_get_width(pixmap)/2,
					  y_start - gdk_pixbuf_get_height(pixmap) - 10,
					   NULL);

      g_signal_connect(item_credit, "button_press_event",
			 (GtkSignalFunc) item_event_help,
			 "credit");

      item_credit_text = goo_canvas_text_new (rootitem,
					      _("Credit"),
					      BOARDWIDTH*0.8,
					      y_start - gdk_pixbuf_get_height(pixmap)  + GAP_TO_BUTTON,
					      -1,
					      GTK_ANCHOR_CENTER,
					      "font", gc_skin_font_content,
					      "fill-color-rgba", gc_skin_get_color("gcompris/helpunselect"),
					      NULL);
      g_signal_connect(item_credit_text, "button_press_event",
			 (GtkSignalFunc) item_event_help,
			 "credit");
    }

  gdk_pixbuf_unref(pixmap);

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

  y_start += 5;

  /* Create a scrolled area for the text content */
  GtkWidget *view;
  GtkWidget *sw;
  view = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
  gtk_text_view_set_editable(GTK_TEXT_VIEW (view), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW (view), FALSE);

  gtk_signal_connect(GTK_OBJECT(view), "button-press-event",
		     (GtkSignalFunc) event_disable_right_click_popup, NULL);

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
			 280.0);
  gtk_widget_show_all (sw);

  set_content(text_to_display);
  // OK
  pixmap = gc_skin_pixmap_load("button_large.png");

  item = goo_canvas_image_new (rootitem,
			       pixmap,
			       (BOARDWIDTH*0.5) - gdk_pixbuf_get_width(pixmap)/2,
			       y - gdk_pixbuf_get_height(pixmap) - 5,
				NULL);

  g_signal_connect(item, "button_press_event",
		     (GtkSignalFunc) item_event_help,
		     "ok");
  g_signal_connect(item, "button_press_event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

  item2 = goo_canvas_text_new (rootitem,
			       _("OK"),
			       BOARDWIDTH*0.5,
			       y - gdk_pixbuf_get_height(pixmap) + 20,
			       -1,
			       GTK_ANCHOR_CENTER,
			       "font", gc_skin_font_title,
			       "fill-color-rgba", gc_skin_color_text_button,
			       NULL);
  g_signal_connect(item2, "button_press_event",
		     (GtkSignalFunc) item_event_help,
		     "ok");
  g_signal_connect(item2, "button_press_event",
		     (GtkSignalFunc) gc_item_focus_event,
		     item);
  gdk_pixbuf_unref(pixmap);

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
  GdkPixbuf   *pixmap = NULL;

  if(item_selected)
    {
      pixmap = gc_skin_pixmap_load("button_up.png");
      /* Warning changing the image needs to update pixbuf_ref for the focus usage */
      gc_item_focus_free(item_selected, NULL);
      g_object_set(item_selected,
		   "pixbuf", pixmap,
		   NULL);
      g_object_set(item_selected_text,
		   "fill-color-rgba", gc_skin_get_color("gcompris/helpunselect"),
		   NULL);

      gdk_pixbuf_unref(pixmap);
    }

  pixmap = gc_skin_pixmap_load("button_up_selected.png");
  /* Warning changing the image needs to update pixbuf_ref for the focus usage */
  gc_item_focus_free(item, NULL);
  g_object_set(item,
	       "pixbuf", pixmap,
	       NULL);
  gdk_pixbuf_unref(pixmap);
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
