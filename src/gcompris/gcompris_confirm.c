/* gcompris - gcompris_confirm.c
 *
 * Time-stamp: <2006/04/10 00:00:58 bruno>
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
#include <string.h>

/**
 * A confirmation popup for gcompris
 *
 */
#include "gcompris.h"

static void              display_confirm(gchar *title,
					 gchar *question_text,
					 gchar *yes_text,
					 gchar *no_text,
					 ConfirmCallBack iscb);

static gint
button_event(GnomeCanvasItem *item, GdkEvent *event,  gchar *answer);

static void              set_content(GnomeCanvasRichText *item_content, 
				     GnomeCanvasRichText *item_content_s,
				     gchar *text);

static gboolean		 confirm_displayed = FALSE;

static GnomeCanvasItem	*rootitem = NULL;
static GnomeCanvasItem	*no_button = NULL;
static GnomeCanvasItem	*yes_button = NULL;
static GnomeCanvasItem	*yes_stick = NULL;
static GnomeCanvasItem	*no_cross = NULL;

static ConfirmCallBack  confirmCallBack = NULL;

static gdouble bg_x;
static gdouble bg_y;

static gdouble titre_x;
static gdouble titre_w;
static gdouble titre_y;
static gdouble titre_h;


static gdouble text_zone_x;
static gdouble text_zone_w;
static gdouble text_zone_y;
static gdouble text_zone_h;


static gdouble button_x;
static gdouble button_w;
static gdouble button_y;
static gdouble button_h;
static gdouble button_x_int;

/* background */
#define BG_X 400
#define BG_Y 0

/* TITLE */
#define T_X 80
#define T_Y 24
#define T_W 620
#define T_H 30

/* Text */
#define T_Z_X 80
#define T_Z_Y 120
#define T_Z_W 620
#define T_Z_H 115

/* Buttons */
#define T_B_X 80 
#define T_B_Y 250
#define T_B_W 620 
#define T_B_H 192
#define T_B_X_INT 30


/*
 * Main entry point 
 * ----------------
 *
 */

/*
 * Do all the file_selector display and register the events
 * file_types is A Comma separated text explaining the different file types
 */

void gcompris_confirm (gchar *title, 
		       gchar *question_text,
		       gchar *yes_text,
		       gchar *no_text,
		       ConfirmCallBack iscb)
{
  display_confirm(title, 
		  question_text, 
		  yes_text,
		  no_text,
		  iscb);
}

/*
 * Remove the displayed file_selector.
 * Do nothing if none is currently being dislayed
 */

void gcompris_confirm_stop ()
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  // Destroy the box
  /* FIXME: Crashes randomly */
  if(rootitem!=NULL)
    gtk_object_destroy(GTK_OBJECT(rootitem));

  rootitem = NULL;	  

  if(gcomprisBoard!=NULL && confirm_displayed)
    {
      if(gcomprisBoard->plugin->pause_board != NULL)
	{
	  gcomprisBoard->plugin->pause_board(FALSE);
	}
    }

  gcompris_bar_hide(FALSE);
  confirm_displayed = FALSE;
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/


static void
display_confirm(gchar *title,
		gchar *question_text,
		gchar *yes_text,
		gchar *no_text,
		ConfirmCallBack iscb) {

  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();


  GnomeCanvasItem  *item, *item2, *richtext_s, *richtext;
  GdkPixbuf	   *pixmap = NULL;
  GdkPixbuf	   *pixmap_cross = NULL;
  GdkPixbuf	   *pixmap_stick = NULL;
  gint		    y_start = 0;
  gint		    x_start = 0;

  if(rootitem)
    return;

  bg_x = BG_X;
  bg_y = BG_Y;

  titre_x = T_X;
  titre_w = T_W;
  titre_y = T_Y;;
  titre_h = T_H;


  text_zone_x = T_Z_X;
  text_zone_w = T_Z_W;
  text_zone_y = T_Z_Y;
  text_zone_h = T_Z_H;


  button_x = T_B_X;
  button_w = T_B_W;
  button_y = T_B_Y;
  button_h = T_B_Y;
  button_x_int = T_B_X_INT;

  gcompris_bar_hide(TRUE);

  if(gcomprisBoard!=NULL)
    {
      if(gcomprisBoard->plugin->pause_board != NULL)
	gcomprisBoard->plugin->pause_board(TRUE);
    }

  confirmCallBack=iscb;

  rootitem = \
    gnome_canvas_item_new (gnome_canvas_root(gcompris_get_canvas()),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  pixmap = gcompris_load_skin_pixmap("help_bg.png");
  y_start = (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  if (y_start < 0)
    y_start = 0;
  x_start = (BOARDWIDTH)/2;
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) x_start,
				"y", (double) y_start,
				"anchor", GTK_ANCHOR_NORTH,
				NULL);

  gdk_pixbuf_unref(pixmap);

  /* Title */
  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", title,
			 "x", (gdouble) titre_x + titre_w/2 + 1.0,
			 "y", (gdouble) titre_y + titre_h/2 + 1.0,
			 "font", gcompris_skin_font_title,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color_rgba",  gcompris_skin_color_shadow,
			 NULL);

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", title,
			 "x", (gdouble) titre_x + titre_w/2,
			 "y", (gdouble) titre_y + titre_h/2,
			 "font", gcompris_skin_font_title,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color_rgba",  gcompris_skin_color_title,
			 NULL);

  richtext_s = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				     gnome_canvas_rich_text_get_type (),
				     "x", (double) text_zone_x + 1.0,
				     "y", (double)  text_zone_y + 1.0,
				     "width",  text_zone_w,
				     "height", text_zone_h,
				     "anchor", GTK_ANCHOR_NW,
				     "justification", GTK_JUSTIFY_CENTER,
				     "grow_height", FALSE,
				     "cursor_visible", FALSE,
				     "cursor_blink", FALSE,
				     "editable", FALSE,
				     NULL);


  richtext = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				     gnome_canvas_rich_text_get_type (),
				     "x", (double) text_zone_x ,
				     "y", (double)  text_zone_y,
				     "width",  text_zone_w,
				     "height", text_zone_h,
				     "anchor", GTK_ANCHOR_NW,
				     "justification", GTK_JUSTIFY_CENTER,
				     "grow_height", FALSE,
				     "cursor_visible", FALSE,
				     "cursor_blink", FALSE,
				     "editable", FALSE,
				     NULL);

  set_content (GNOME_CANVAS_RICH_TEXT(richtext), 
	       GNOME_CANVAS_RICH_TEXT(richtext_s), 
	       question_text);
	 
  
  /*
   * Buttons
   * -------
   */

  pixmap = gcompris_load_skin_pixmap("button_large.png");
  pixmap_stick = gcompris_load_skin_pixmap("button_checked.png");
  pixmap_cross = gcompris_load_skin_pixmap("bad.png");

  // CANCEL
  no_button = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				     gnome_canvas_pixbuf_get_type (),
				     "pixbuf", pixmap, 
				     "x", (double) button_x ,
				     "y", (double) button_y + 2*button_h/3,
				     "anchor", GTK_ANCHOR_WEST,
				     NULL);

  //gdk_pixbuf_unref(pixmap);

  gtk_signal_connect(GTK_OBJECT(no_button), "event",
		     (GtkSignalFunc) button_event,
		     "/no/");

  gtk_signal_connect(GTK_OBJECT(no_button), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  // CANCEL CROSS
  no_cross = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap_cross, 
				    "x", (double) button_x  + gdk_pixbuf_get_width(pixmap)/2,
				    "y", (double) button_y + 2*button_h/3,
				    "anchor", GTK_ANCHOR_CENTER,
				    NULL);

  gdk_pixbuf_unref(pixmap_cross);

  gtk_signal_connect(GTK_OBJECT(no_cross), "event",
		     (GtkSignalFunc) button_event,
		     "/no/");
  gtk_signal_connect(GTK_OBJECT(no_cross), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);


  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", no_text,
			 "font", gcompris_skin_font_title,
			 "x", (double)  button_x + gdk_pixbuf_get_width(pixmap) + button_x_int ,
			 "y", (double)  button_y + 2*button_h/3,
			 "anchor", GTK_ANCHOR_WEST,
			 "fill_color_rgba", gcompris_skin_get_color("gcompris/helpfg"),
			 NULL);

  // OK
  yes_button = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", pixmap, 
				      "x", (double) button_x ,
				      "y", (double) button_y + button_h/3,
				      "anchor", GTK_ANCHOR_WEST,
				      NULL);
  
  gdk_pixbuf_unref(pixmap);
  
  gtk_signal_connect(GTK_OBJECT(yes_button), "event",
		     (GtkSignalFunc) button_event,
		     "/yes/");

  gtk_signal_connect(GTK_OBJECT(yes_button), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  // OK stick
  yes_stick = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				     gnome_canvas_pixbuf_get_type (),
				     "pixbuf", pixmap_stick, 
				     "x", (double) button_x + gdk_pixbuf_get_width(pixmap)/2,
				     "y", (double) button_y + button_h/3,
				     "anchor", GTK_ANCHOR_CENTER,
				     NULL);

  gdk_pixbuf_unref(pixmap_stick);

  gtk_signal_connect(GTK_OBJECT(yes_stick), "event",
		     (GtkSignalFunc) button_event,
		     "/yes/");
  gtk_signal_connect(GTK_OBJECT(yes_stick), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);


  item2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				 gnome_canvas_text_get_type (),
				 "text", yes_text,
				 "font", gcompris_skin_font_title,
				 "x", (double)  button_x + gdk_pixbuf_get_width(pixmap) + button_x_int ,
				 "y", (double)  button_y + button_h/3,
				 "anchor", GTK_ANCHOR_WEST,
				 "fill_color_rgba", gcompris_skin_get_color("gcompris/helpfg"),
				 NULL);

  confirm_displayed = TRUE;

}

static void
free_stuff (GtkObject *obj, gchar *data)
{
  g_free(data);
}

/* Apply the style to the given RichText item  */
static void
set_content(GnomeCanvasRichText *item_content, 
	    GnomeCanvasRichText *item_content_s,
	    gchar *text) {

  GtkTextIter    iter_start, iter_end;
  GtkTextBuffer *buffer;
  GtkTextTag    *txt_tag;
  gboolean success; 
  gchar *color_string;
  GdkColor *color_s = (GdkColor *)malloc(sizeof(GdkColor));
  GdkColor *color   = (GdkColor *)malloc(sizeof(GdkColor));

  /*
   * Set the new text in the 2 items
   */
  gnome_canvas_item_set(GNOME_CANVAS_ITEM(item_content),
			"text", text,
			NULL);

  gnome_canvas_item_set(GNOME_CANVAS_ITEM(item_content_s),
			"text", text,
			NULL);

  /*
   * Set the shadow
   */

  color_string = g_strdup_printf("#%x", gcompris_skin_color_shadow >> 8);
  gdk_color_parse(color_string, color_s);
  g_free(color_string);
  success = gdk_colormap_alloc_color(gdk_colormap_get_system(), 
				     color_s,
  				     FALSE, TRUE); 

  buffer  = gnome_canvas_rich_text_get_buffer(GNOME_CANVAS_RICH_TEXT(item_content_s));
  txt_tag = gtk_text_buffer_create_tag(buffer, NULL, 
				       "foreground-gdk", color_s,
				       "font",       gcompris_skin_font_board_medium,
				       NULL);
  gtk_text_buffer_get_end_iter(buffer, &iter_end);
  gtk_text_buffer_get_start_iter(buffer, &iter_start);
  gtk_text_buffer_apply_tag(buffer, txt_tag, &iter_start, &iter_end);


  /* 
   * Set the text
   */
  color_string = g_strdup_printf("#%x", gcompris_skin_get_color("gcompris/helpunselect") >> 8);
  gdk_color_parse(color_string, color);
  g_free(color_string);
  success = gdk_colormap_alloc_color(gdk_colormap_get_system(), 
				     color,
  				     FALSE, TRUE); 

  buffer  = gnome_canvas_rich_text_get_buffer(GNOME_CANVAS_RICH_TEXT(item_content));
  txt_tag = gtk_text_buffer_create_tag(buffer, NULL, 
				       "foreground-gdk", color,
				       "font",        gcompris_skin_font_board_medium,
				       NULL);
  gtk_text_buffer_get_end_iter(buffer, &iter_end);
  gtk_text_buffer_get_start_iter(buffer, &iter_start);
  gtk_text_buffer_apply_tag(buffer, txt_tag, &iter_start, &iter_end);

}

static gint
button_event(GnomeCanvasItem *item, GdkEvent *event,  gchar *answer)
{
  if (event->type != GDK_BUTTON_PRESS)
    return FALSE;

  /* Callback with the proper params */
  if (confirmCallBack){
    if (strcmp(answer, "/no/")==0)
      confirmCallBack(FALSE);
    else
      confirmCallBack(TRUE);
  }
  gcompris_confirm_stop();

  return TRUE;
  
}

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
