/* gcompris - help.c
 *
 * Time-stamp: <2004/05/11 23:54:44 bcoudoin>
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

#define GAP_TO_BUTTON -20

#define COLOR_SELECTED   0x34682aFF
#define COLOR_UNSELECTED 0x4252ffFF

static gint	 item_event_help(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static void	 select_item(GnomeCanvasItem *item, GnomeCanvasItem *item_text);
static void	 set_content(char *text);

static gboolean help_displayed			= FALSE;

static GnomeCanvasItem *rootitem		= NULL;
static GnomeCanvasItem *item_content		= NULL;
static GnomeCanvasItem *item_content_shadow	= NULL;

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


/*
 * Main entry point 
 * ----------------
 *
 */

/**
 * Return true wether the given board has at least one help field defined
 *
 */
gboolean gcompris_board_has_help (GcomprisBoard *gcomprisBoard)
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

void gcompris_help_start (GcomprisBoard *gcomprisBoard)
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

  if(gcomprisBoard!=NULL)
    {
      if(gcomprisBoard->plugin->pause_board != NULL)
	  gcomprisBoard->plugin->pause_board(TRUE);
    }

  item_selected = NULL;
  item_selected_text = NULL;

  name = gcomprisBoard->title;
  gcompris_board_has_help(gcomprisBoard);

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

  y_start += 40;
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", name, 
				"font", gcompris_skin_font_title,
				"x", (double) BOARDWIDTH/2,
				"y", (double) y_start,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_title,
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
      gtk_signal_connect(GTK_OBJECT(item_prerequisite), "event",
			 (GtkSignalFunc) gcompris_item_event_focus,
			 NULL);
      
      item_prerequisite_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				    gnome_canvas_text_get_type (),
				    "text", _("Prerequisite"),
				    "font", gcompris_skin_font_content,
				    "x", (double)  BOARDWIDTH*0.20,
				    "y", (double)  y_start - gdk_pixbuf_get_height(pixmap)  + GAP_TO_BUTTON,
				    "anchor", GTK_ANCHOR_CENTER,
				    "fill_color_rgba", gcompris_skin_color_text_button,
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item_prerequisite_text), "event",
			 (GtkSignalFunc) item_event_help,
			 "prerequisite");
      gtk_signal_connect(GTK_OBJECT(item_prerequisite_text), "event",
			 (GtkSignalFunc) gcompris_item_event_focus,
			 item_prerequisite);
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
      gtk_signal_connect(GTK_OBJECT(item_goal), "event",
			 (GtkSignalFunc) gcompris_item_event_focus,
			 NULL);
      
      item_goal_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				    gnome_canvas_text_get_type (),
				    "text", _("Goal"),
				    "font", gcompris_skin_font_content,
				    "x", (double)  BOARDWIDTH*0.4,
				    "y", (double)  y_start - gdk_pixbuf_get_height(pixmap)  + GAP_TO_BUTTON,
				    "anchor", GTK_ANCHOR_CENTER,
				    "fill_color_rgba", gcompris_skin_color_text_button,
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item_goal_text), "event",
			 (GtkSignalFunc) item_event_help,
			 "goal");
      gtk_signal_connect(GTK_OBJECT(item_goal_text), "event",
			 (GtkSignalFunc) gcompris_item_event_focus,
			 item_goal);
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
      gtk_signal_connect(GTK_OBJECT(item_manual), "event",
			 (GtkSignalFunc) gcompris_item_event_focus,
			 NULL);
      
      item_manual_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				    gnome_canvas_text_get_type (),
				    "text", _("Manual"),
				    "font", gcompris_skin_font_content,
				    "x", (double)  BOARDWIDTH*0.6,
				    "y", (double)  y_start - gdk_pixbuf_get_height(pixmap)  + GAP_TO_BUTTON,
				    "anchor", GTK_ANCHOR_CENTER,
				    "fill_color_rgba", gcompris_skin_color_text_button,
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item_manual_text), "event",
			 (GtkSignalFunc) item_event_help,
			 "manual");
      gtk_signal_connect(GTK_OBJECT(item_manual_text), "event",
			 (GtkSignalFunc) gcompris_item_event_focus,
			 item_manual);
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
      gtk_signal_connect(GTK_OBJECT(item_credit), "event",
			 (GtkSignalFunc) gcompris_item_event_focus,
			 NULL);
      
      item_credit_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				    gnome_canvas_text_get_type (),
				    "text", _("Credit"),
				    "font", gcompris_skin_font_content,
				    "x", (double)  BOARDWIDTH*0.8,
				    "y", (double)  y_start - gdk_pixbuf_get_height(pixmap)  + GAP_TO_BUTTON,
				    "anchor", GTK_ANCHOR_CENTER,
				    "fill_color_rgba", gcompris_skin_color_text_button,
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item_credit_text), "event",
			 (GtkSignalFunc) item_event_help,
			 "credit");
      gtk_signal_connect(GTK_OBJECT(item_credit_text), "event",
			 (GtkSignalFunc) gcompris_item_event_focus,
			 item_credit);
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

  item_content_shadow = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					gnome_canvas_rich_text_get_type (),
					"x", (double)  x_start + 40 + 1,
					"y", (double)  y_start + 1,
					"width", 620.0,
					"height", 400.0,
					"anchor", GTK_ANCHOR_NW,
					"grow_height", FALSE,
					"cursor_visible", FALSE,
					"cursor_blink", FALSE,
					"editable", FALSE,
					NULL);

  item_content = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					gnome_canvas_rich_text_get_type (),
					"x", (double)  x_start + 40,
					"y", (double)  y_start,
					"width", 620.0,
					"height", 400.0,
					"anchor", GTK_ANCHOR_NW,
					"grow_height", FALSE,
					"cursor_visible", FALSE,
					"cursor_blink", FALSE,
					"editable", FALSE,
					NULL);

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
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  item2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("OK"),
				"font", gcompris_skin_font_title,
				"x", (double)  BOARDWIDTH*0.5,
				"y", (double)  y - gdk_pixbuf_get_height(pixmap) + 20,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_text_button,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) item_event_help,
		     "ok");
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     item);
  gdk_pixbuf_unref(pixmap);

  gcompris_bar_hide(TRUE);

  help_displayed = TRUE;
}

/*
 * Remove the displayed help.
 * Do nothing is none is currently being dislayed
 */
void gcompris_help_stop ()
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  if(gcomprisBoard!=NULL && help_displayed)
    {
      if(gcomprisBoard->plugin->pause_board != NULL)
	{
	  // Destroy the help box
	  if(rootitem!=NULL)
	    {
	      gtk_object_destroy(GTK_OBJECT(rootitem));
	      gcomprisBoard->plugin->pause_board(FALSE);
	    }
	  rootitem = NULL;	  
	}
    }

  gcompris_bar_hide(FALSE);
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
      gnome_canvas_item_set(item_selected, 
			    "pixbuf", pixmap,
			    NULL);
      gnome_canvas_item_set(item_selected_text, 
			    "fill_color_rgba", COLOR_UNSELECTED,
			    NULL);
      gdk_pixbuf_unref(pixmap);
    }

  pixmap = gcompris_load_skin_pixmap("button_up_selected.png");
  gnome_canvas_item_set(item, 
			"pixbuf", pixmap,
			NULL);
  gdk_pixbuf_unref(pixmap);
  gnome_canvas_item_set(item_text, 
			"fill_color_rgba", COLOR_SELECTED,
			NULL);
  item_selected = item;
  item_selected_text = item_text;
}

/* Apply the style to the given RichText item  */
static void set_content(gchar *text) {

  GtkTextIter    iter_start, iter_end;
  GtkTextBuffer *buffer;
  GtkTextTag    *txt_tag;

  gnome_canvas_item_set(item_content, 
			"text", text,
			NULL);

  buffer  = gnome_canvas_rich_text_get_buffer(GNOME_CANVAS_RICH_TEXT(item_content));
  txt_tag = gtk_text_buffer_create_tag(buffer, NULL, 
				       "foreground", "black",
				       "font",       "Sans 10",
				       NULL);
  gtk_text_buffer_get_end_iter(buffer, &iter_end);
  gtk_text_buffer_get_start_iter(buffer, &iter_start);
  gtk_text_buffer_apply_tag(buffer, txt_tag, &iter_start, &iter_end);


  gnome_canvas_item_set(item_content_shadow, 
			"text", text,
			NULL);

  buffer  = gnome_canvas_rich_text_get_buffer(GNOME_CANVAS_RICH_TEXT(item_content));
  txt_tag = gtk_text_buffer_create_tag(buffer, NULL, 
				       "foreground", "blue",
				       "font",       "Sans 10",
				       NULL);
  gtk_text_buffer_get_end_iter(buffer, &iter_end);
  gtk_text_buffer_get_start_iter(buffer, &iter_start);
  gtk_text_buffer_apply_tag(buffer, txt_tag, &iter_start, &iter_end);

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
	  gcompris_help_stop();
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
