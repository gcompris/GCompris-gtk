/* gcompris - gc_confirm_box.c
 *
 * Time-stamp: <2006/08/20 10:14:18 bruno>
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
#include <librsvg/rsvg.h>

static GooCanvasItem *text;

static void              display_confirm(gchar *title,
					 gchar *question_text,
					 gchar *yes_text,
					 gchar *no_text,
					 ConfirmCallBack iscb);

static gboolean button_event (GooCanvasItem  *item,
			      GooCanvasItem  *target,
			      GdkEventButton *event,
			      gchar *answer);

static void              set_content(GooCanvasText *item_content,
				     gchar *text);

static gboolean		 confirm_displayed = FALSE;

static GooCanvasItem	*rootitem = NULL;
static GooCanvasItem	*no_button = NULL;
static GooCanvasItem	*yes_button = NULL;
static GooCanvasItem	*yes_stick = NULL;
static GooCanvasItem	*no_cross = NULL;

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

void gc_confirm_box (gchar *title,
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

void gc_confirm_box_stop ()
{
  GcomprisBoard *gcomprisBoard = gc_board_get_current();

  // Destroy the box
  if(rootitem!=NULL)
    goo_canvas_item_remove(rootitem);

  rootitem = NULL;

  if(gcomprisBoard!=NULL && confirm_displayed)
    gc_board_pause(FALSE);

  gc_bar_hide(FALSE);
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

  GooCanvasItem  *item, *item2;
  GdkPixbuf	 *pixmap = NULL;
  GdkPixbuf	 *pixmap_cross = NULL;
  GdkPixbuf	 *pixmap_stick = NULL;
  RsvgHandle	 *svg_handle = NULL;

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

  gc_bar_hide(TRUE);

  gc_board_pause(TRUE);

  confirmCallBack=iscb;

  rootitem = goo_canvas_group_new (goo_canvas_get_root_item(gc_get_canvas()),
				   NULL);

  svg_handle = gc_skin_rsvg_load("help_bg.svgz");
  item = goo_svg_item_new (rootitem, svg_handle, NULL);

  RsvgDimensionData dimension;
  rsvg_handle_get_dimensions(svg_handle, &dimension);
  goo_canvas_item_translate(item, (BOARDWIDTH - dimension.width)/2,
			    (BOARDHEIGHT - dimension.height)/2);
  g_object_unref (svg_handle);

  /* Title */
  goo_canvas_text_new (rootitem,
		       title,
		       titre_x + titre_w/2,
		       titre_y + titre_h/2,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_title,
		       "fill-color-rgba",  gc_skin_color_title,
		       NULL);

  text = goo_canvas_text_new (rootitem,
			      "",
			      text_zone_x,
			      text_zone_y,
			      text_zone_w,
			      GTK_JUSTIFY_CENTER);

  set_content (GOO_CANVAS_TEXT(text), question_text);


  /*
   * Buttons
   * -------
   */

  pixmap = gc_skin_pixmap_load("button_large.png");
  pixmap_stick = gc_skin_pixmap_load("button_checked.png");
  pixmap_cross = gc_skin_pixmap_load("bad.png");

  // CANCEL
  no_button = goo_canvas_image_new (rootitem,
				    pixmap,
				    (double) button_x ,
				    (double) button_y + 2*button_h/3,
				     NULL);

  //gdk_pixbuf_unref(pixmap);

  g_signal_connect(no_button, "button_press_event",
		     (GtkSignalFunc) button_event,
		     "/no/");

  g_signal_connect(no_button, "button_press_event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

  // CANCEL CROSS
  no_cross = goo_canvas_image_new (rootitem,
				   pixmap_cross,
				   button_x  + gdk_pixbuf_get_width(pixmap)/2
				   - gdk_pixbuf_get_width(pixmap_cross)/2,
				   button_y + 2*button_h/3,
				    NULL);

  gdk_pixbuf_unref(pixmap_cross);

  g_signal_connect(no_cross, "button_press_event",
		     (GtkSignalFunc) button_event,
		     "/no/");
  g_signal_connect(no_cross, "button_press_event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);


  goo_canvas_text_new (rootitem,
		       no_text,
		       (gdouble)  button_x + gdk_pixbuf_get_width(pixmap) + button_x_int ,
		       (gdouble)  button_y + 2*button_h/3 + 20,
		       -1,
		       GTK_ANCHOR_WEST,
		       "font", gc_skin_font_subtitle,
		       "fill-color-rgba", gc_skin_get_color("gcompris/helpfg"),
		       NULL);

  // OK
  yes_button = goo_canvas_image_new (rootitem,
				     pixmap,
				     (double) button_x ,
				     (double) button_y + button_h/3,
				      NULL);

  gdk_pixbuf_unref(pixmap);

  g_signal_connect(yes_button, "button_press_event",
		     (GtkSignalFunc) button_event,
		     "/yes/");

  g_signal_connect(yes_button, "button_press_event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

  // OK stick
  yes_stick = goo_canvas_image_new (rootitem,
				    pixmap_stick,
				    button_x + gdk_pixbuf_get_width(pixmap)/2
				    - gdk_pixbuf_get_width(pixmap_stick)/2,
				    button_y + button_h/3,
				     NULL);

  gdk_pixbuf_unref(pixmap_stick);

  g_signal_connect(yes_stick, "button_press_event",
		     (GtkSignalFunc) button_event,
		     "/yes/");
  g_signal_connect(yes_stick, "button_press_event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);


  item2 = goo_canvas_text_new (rootitem,
			       yes_text,
			       (gdouble)  button_x + gdk_pixbuf_get_width(pixmap) + button_x_int ,
			       (gdouble)  button_y + button_h/3 + 20,
			       -1,
			       GTK_ANCHOR_WEST,
			       "font", gc_skin_font_subtitle,
			       "fill-color-rgba", gc_skin_get_color("gcompris/helpfg"),
			       NULL);

  confirm_displayed = TRUE;

}

/* Apply the style to the given Text item  */
static void
set_content(GooCanvasText *item_content,
	    gchar *text) {


  /*
   * Set the new text in the 2 items
   */
  g_object_set(GOO_CANVAS_ITEM(item_content),
	       "text", text,
	       NULL);

}

static gboolean
button_event (GooCanvasItem  *item,
	      GooCanvasItem  *target,
	      GdkEventButton *event,
	      gchar *answer)
{

  /* Callback with the proper params */
  if (confirmCallBack){
    if (strcmp(answer, "/no/")==0)
      confirmCallBack(FALSE);
    else
      confirmCallBack(TRUE);
  }
  gc_confirm_box_stop();

  return TRUE;

}
