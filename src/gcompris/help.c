/* gcompris - help.c
 *
 * Time-stamp: <2006/08/20 23:37:50 bruno>
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
#include <string.h>

#include "gcompris.h"
#include "gc_core.h"

#if defined _WIN32 || defined __WIN32__
# undef WIN32   /* avoid warning on mingw32 */
# define WIN32
#endif

#define SOUNDLISTFILE PACKAGE

#define GAP_TO_BUTTON -20

static gint	 item_event_help(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static void	 select_item(GnomeCanvasItem *item, GnomeCanvasItem *item_text);
static void	 set_content(char *text);

static gboolean help_displayed			= FALSE;

static GnomeCanvasItem *rootitem		= NULL;

static gchar *prerequisite			= NULL;
static gchar *goal				= NULL;
static gchar *manual				= NULL;
static gchar *credit				= NULL;

static GnomeCanvasItem *item_prerequisite	= NULL;
static GnomeCanvasItem *item_goal		= NULL;
static GnomeCanvasItem *item_manual		= NULL;
static GnomeCanvasItem *item_credit		= NULL;

static GnomeCanvasItem *item_prerequisite_text  = NULL;
static GnomeCanvasItem *item_goal_text          = NULL;
static GnomeCanvasItem *item_manual_text        = NULL;
static GnomeCanvasItem *item_credit_text        = NULL;

static GnomeCanvasItem *item_selected		= NULL;
static GnomeCanvasItem *item_selected_text	= NULL;

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
  GnomeCanvasItem *item, *item2;
  gint y = 0;
  gint y_start = 0;
  gint x_start = 0;
  gchar   *name = NULL;
  gchar   *text_to_display = NULL;

  if(rootitem)
    return;

  board_pause(TRUE);

  item_selected = NULL;
  item_selected_text = NULL;

  name = gcomprisBoard->title;
  gc_help_has_board(gcomprisBoard);

  rootitem = \
    gnome_canvas_item_new (gnome_canvas_root(gcompris_get_canvas()),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  pixmap = gcompris_load_skin_pixmap("help_bg.png");
  y_start = (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  x_start = (BOARDWIDTH - gdk_pixbuf_get_width(pixmap))/2;
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) x_start,
				"y", (double) y_start,
				NULL);
  y = BOARDHEIGHT - (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  gdk_pixbuf_unref(pixmap);

  y_start += 14;
  if(gcomprisBoard->section && gcomprisBoard->name) {
    text_to_display = g_strdup_printf("%s/%s", gcomprisBoard->section, gcomprisBoard->name);
     gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			    gnome_canvas_text_get_type (),
			    "text", text_to_display,
			    "font", gcompris_skin_font_board_tiny,
			    "x", (double) BOARDWIDTH*0.10 + 1.0,
			    "y", (double) y_start + 1.0,
			    "anchor", GTK_ANCHOR_NW,
			    "fill_color_rgba", gcompris_skin_color_shadow,
			    NULL);
     gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			    gnome_canvas_text_get_type (),
			    "text", text_to_display,
			    "font", gcompris_skin_font_board_tiny,
			    "x", (double) BOARDWIDTH*0.10,
			    "y", (double) y_start,
			    "anchor", GTK_ANCHOR_NW,
			    "fill_color_rgba", gcompris_skin_color_title,
			    NULL);
    g_free(text_to_display);
  }

  y_start += 35;
  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", name, 
			 "font", gcompris_skin_font_title,
			 "x", (double) BOARDWIDTH/2 + 1.0,
			 "y", (double) y_start + 1.0,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color_rgba", gcompris_skin_color_shadow,
			 "weight", PANGO_WEIGHT_HEAVY,
			 NULL);
  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", name, 
			 "font", gcompris_skin_font_title,
			 "x", (double) BOARDWIDTH/2,
			 "y", (double) y_start,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color_rgba", gcompris_skin_color_title,
			 "weight", PANGO_WEIGHT_HEAVY,
			 NULL);


  y_start += 120;

  pixmap = gcompris_load_skin_pixmap("button_up.png");

  // Prerequisite Button
  if(prerequisite)
    {
      item_prerequisite = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
						 gnome_canvas_pixbuf_get_type (),
						 "pixbuf", pixmap, 
						 "x", (double) (BOARDWIDTH*0.2) - gdk_pixbuf_get_width(pixmap)/2,
						 "y", (double) y_start - gdk_pixbuf_get_height(pixmap) - 10,
						 NULL);
      
      gtk_signal_connect(GTK_OBJECT(item_prerequisite), "event",
			 (GtkSignalFunc) item_event_help,
			 "prerequisite");
      
      item_prerequisite_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				    gnome_canvas_text_get_type (),
				    "text", _("Prerequisite"),
				    "font", gcompris_skin_font_content,
				    "x", (double)  BOARDWIDTH*0.20,
				    "y", (double)  y_start - gdk_pixbuf_get_height(pixmap)  + GAP_TO_BUTTON,
				    "anchor", GTK_ANCHOR_CENTER,
				    "fill_color_rgba", gcompris_skin_get_color("gcompris/helpunselect"),
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item_prerequisite_text), "event",
			 (GtkSignalFunc) item_event_help,
			 "prerequisite");
    }

  // Goal Button
  if(goal)
    {
      item_goal = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					 gnome_canvas_pixbuf_get_type (),
					 "pixbuf", pixmap, 
					 "x", (double) (BOARDWIDTH*0.4) - gdk_pixbuf_get_width(pixmap)/2,
					 "y", (double) y_start - gdk_pixbuf_get_height(pixmap) - 10,
					 NULL);
      
      gtk_signal_connect(GTK_OBJECT(item_goal), "event",
			 (GtkSignalFunc) item_event_help,
			 "goal");
      
      item_goal_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				    gnome_canvas_text_get_type (),
				    "text", _("Goal"),
				    "font", gcompris_skin_font_content,
				    "x", (double)  BOARDWIDTH*0.4,
				    "y", (double)  y_start - gdk_pixbuf_get_height(pixmap)  + GAP_TO_BUTTON,
				    "anchor", GTK_ANCHOR_CENTER,
				    "fill_color_rgba", gcompris_skin_get_color("gcompris/helpunselect"),
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item_goal_text), "event",
			 (GtkSignalFunc) item_event_help,
			 "goal");
    }

  // Manual Button
  if(manual)
    {
      item_manual = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					   gnome_canvas_pixbuf_get_type (),
					   "pixbuf", pixmap, 
					   "x", (double) (BOARDWIDTH*0.6) - gdk_pixbuf_get_width(pixmap)/2,
					   "y", (double) y_start - gdk_pixbuf_get_height(pixmap) - 10,
					   NULL);
      
      gtk_signal_connect(GTK_OBJECT(item_manual), "event",
			 (GtkSignalFunc) item_event_help,
			 "manual");
      
      item_manual_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				    gnome_canvas_text_get_type (),
				    "text", _("Manual"),
				    "font", gcompris_skin_font_content,
				    "x", (double)  BOARDWIDTH*0.6,
				    "y", (double)  y_start - gdk_pixbuf_get_height(pixmap)  + GAP_TO_BUTTON,
				    "anchor", GTK_ANCHOR_CENTER,
				    "fill_color_rgba", gcompris_skin_get_color("gcompris/helpunselect"),
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item_manual_text), "event",
			 (GtkSignalFunc) item_event_help,
			 "manual");
    }

  // Credit Button
  if(credit)
    {
      item_credit = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					   gnome_canvas_pixbuf_get_type (),
					   "pixbuf", pixmap, 
					   "x", (double) (BOARDWIDTH*0.8) - gdk_pixbuf_get_width(pixmap)/2,
					   "y", (double) y_start - gdk_pixbuf_get_height(pixmap) - 10,
					   NULL);
      
      gtk_signal_connect(GTK_OBJECT(item_credit), "event",
			 (GtkSignalFunc) item_event_help,
			 "credit");
      
      item_credit_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				    gnome_canvas_text_get_type (),
				    "text", _("Credit"),
				    "font", gcompris_skin_font_content,
				    "x", (double)  BOARDWIDTH*0.8,
				    "y", (double)  y_start - gdk_pixbuf_get_height(pixmap)  + GAP_TO_BUTTON,
				    "anchor", GTK_ANCHOR_CENTER,
				    "fill_color_rgba", gcompris_skin_get_color("gcompris/helpunselect"),
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item_credit_text), "event",
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

  PangoFontDescription *font_desc;
  font_desc = pango_font_description_from_string (gcompris_skin_font_content);
  gtk_widget_modify_font (view, font_desc);
  pango_font_description_free (font_desc);

  GdkColor fg_color;
  GdkColor bg_color;
  gcompris_skin_get_gdkcolor("gcompris/helpfg", &fg_color);
  gcompris_skin_get_gdkcolor("gcompris/helpbg", &bg_color);
  gtk_widget_modify_base(view, GTK_STATE_NORMAL, &bg_color);
  gtk_widget_modify_text(view, GTK_STATE_NORMAL, &fg_color);

  buffer_content = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (sw), view);

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_widget_get_type (),
			 "widget", GTK_WIDGET(sw),
			 "x", (double)  x_start + 40,
			 "y", (double)  y_start,
			 "width",  618.0,
			 "height", 280.0,
			 NULL);
  gtk_widget_show_all (sw);

  set_content(text_to_display);
  // OK
  pixmap = gcompris_load_skin_pixmap("button_large.png");

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH*0.5) - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y - gdk_pixbuf_get_height(pixmap) - 5,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_help,
		     "ok");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", _("OK"),
			 "font", gcompris_skin_font_title,
			 "x", (double)  BOARDWIDTH*0.5 + 1.0,
			 "y", (double)  y - gdk_pixbuf_get_height(pixmap) + 20 + 1.0,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color_rgba", gcompris_skin_color_shadow,
			 "weight", PANGO_WEIGHT_HEAVY,
			 NULL);
  item2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				 gnome_canvas_text_get_type (),
				 "text", _("OK"),
				 "font", gcompris_skin_font_title,
				 "x", (double)  BOARDWIDTH*0.5,
				 "y", (double)  y - gdk_pixbuf_get_height(pixmap) + 20,
				 "anchor", GTK_ANCHOR_CENTER,
				 "fill_color_rgba", gcompris_skin_color_text_button,
				 "weight", PANGO_WEIGHT_HEAVY,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) item_event_help,
		     "ok");
  gtk_signal_connect(GTK_OBJECT(item2), "event",
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
	  gtk_object_destroy(GTK_OBJECT(rootitem));
	  rootitem = NULL;	  
	}
      board_pause(FALSE);
    }

  gc_bar_hide(FALSE);
  help_displayed = FALSE;
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

static void select_item(GnomeCanvasItem *item, GnomeCanvasItem *item_text)
{
  GdkPixbuf   *pixmap = NULL;
  
  if(item_selected)
    {
      pixmap = gcompris_load_skin_pixmap("button_up.png");
      /* Warning changing the image needs to update pixbuf_ref for the focus usage */
      g_object_set_data (G_OBJECT (item_selected), "pixbuf_ref", pixmap);
      gnome_canvas_item_set(item_selected, 
			    "pixbuf", pixmap,
			    NULL);
      gnome_canvas_item_set(item_selected_text, 
			    "fill_color_rgba", gcompris_skin_get_color("gcompris/helpunselect"),
			    NULL);

      gdk_pixbuf_unref(pixmap);
    }

  pixmap = gcompris_load_skin_pixmap("button_up_selected.png");
  /* Warning changing the image needs to update pixbuf_ref for the focus usage */
  g_object_set_data (G_OBJECT (item), "pixbuf_ref", pixmap);
  gnome_canvas_item_set(item, 
			"pixbuf", pixmap,
			NULL);
  gdk_pixbuf_unref(pixmap);
  gnome_canvas_item_set(item_text, 
			"fill_color_rgba", gcompris_skin_get_color("gcompris/helpselect"),
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
static gint
item_event_help(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
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
