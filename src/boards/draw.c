/* gcompris - draw.c
 *
 * Time-stamp: <2001/08/20 00:54:45 bruno>
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

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include <math.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static gint drawing_area_x1 = 0;
static gint drawing_area_y1 = 0;
static gint drawing_area_x2 = 0;
static gint drawing_area_y2 = 0;

static GcomprisBoard *gcomprisBoard = NULL;
static GnomeCanvasItem *shape_root_item = NULL;
static GnomeCanvasItem *draw_root_item = NULL;
static GnomeCanvasItem *current_color_item = NULL;
static gchar *currentColor = NULL;

/*
 * Contains the points that represents the anchors
 */
typedef struct {
  GnomeCanvasItem *rootitem;
  GnomeCanvasItem *item;
  GnomeCanvasItem *nw;
  GnomeCanvasItem *n;
  GnomeCanvasItem *ne;
  GnomeCanvasItem *e;
  GnomeCanvasItem *w;
  GnomeCanvasItem *sw;
  GnomeCanvasItem *s;
  GnomeCanvasItem *se;
} AnchorsItem;

typedef enum
{
  ANCHOR_NW		= 1,
  ANCHOR_N 		= 2,
  ANCHOR_NE		= 3,
  ANCHOR_E 		= 4,
  ANCHOR_W 		= 5,
  ANCHOR_SW		= 6,
  ANCHOR_S 		= 7,
  ANCHOR_SE		= 8,
} AnchorType;

#define ANCHOR_COLOR		0x36ede400
#define DEFAULT_ITEM_SIZE	40
#define DEFAULT_ANCHOR_SIZE	5
static AnchorsItem *selected_anchors_item = NULL;


static gchar *colorlist [] = 
  {
    "black",
    "brown",
    "red",
    "orange",
    "yellow",
    "green",
    "blue",
    "purple",
    "grey",
    "white",
  };

typedef enum
{
  TOOL_RECT		= 0,
  TOOL_FILLED_RECT	= 1,
  TOOL_CIRCLE		= 2,
  TOOL_FILLED_CIRCLE	= 3,
  TOOL_LINE		= 4,
  TOOL_DELETE		= 5,
  TOOL_FILL		= 6,
} ToolList;

static ToolList		 currentTool = TOOL_RECT;
static GnomeCanvasItem	*currentToolItem = NULL;

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 config(void);

static void	 draw_destroy_all_items(void);
static void	 draw_next_level(void);
static void	 display_color_selector(GnomeCanvasGroup *parent);
static void	 display_tool_selector(GnomeCanvasGroup *parent);
static void	 display_drawing_area(GnomeCanvasGroup *parent);
static gint	 color_event(GnomeCanvasItem *item, GdkEvent *event, gchar *color);
static gint	 tool_event(GnomeCanvasItem *item, GdkEvent *event, gint tool);
static gint	 item_event(GnomeCanvasItem *item, GdkEvent *event, void *shape);
static gint	 item_event_resize(GnomeCanvasItem *item, GdkEvent *event, AnchorsItem *anchorsItem);
static gint	 item_event_move(GnomeCanvasItem *item, GdkEvent *event, AnchorsItem *anchorsItem);
static void	 highlight_color_item(GnomeCanvasItem *item);

/* Description of this plugin */
BoardPlugin menu_bp =
{
   NULL,
   NULL,
   N_("A simple vector drawing tool"),
   N_("Creative board where you can freely draw"),
   "Bruno Coudoin <bruno.coudoin@free.fr>",
   NULL,
   NULL,
   NULL,
   NULL,
   start_board,
   pause_board,
   end_board,
   is_our_board,
   NULL,
   NULL,
   NULL,
   config,
   NULL
};

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

BoardPlugin 
*get_bplugin_info(void)
{
  return &menu_bp;
}

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{

}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "draw/draw-bg.jpg");


      /* set initial values for this level */
      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel=1;
      gcompris_bar_set(0);

      gcomprisBoard->number_of_sublevel=0;
      gcomprisBoard->sublevel = 0;

      drawing_area_x1 = 93;
      drawing_area_y1 = 23;
      drawing_area_x2 = gcomprisBoard->width  - 28;
      drawing_area_y2 = gcomprisBoard->height - 80;

      gcompris_bar_set(0);

      draw_next_level();

      /* Special Init for this board */
      selected_anchors_item = NULL;

      pause_board(FALSE);

    }

}

static void
end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      draw_destroy_all_items();
      gcomprisBoard->level = 1;       // Restart this game to zero
    }
  gcomprisBoard = NULL;
}


gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "draw")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

static void
config ()
{
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
    }
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

/* set initial values for the next level */
static void draw_next_level() 
{

  gcompris_bar_set_level(gcomprisBoard);

  draw_destroy_all_items();

  shape_root_item = \
    gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  display_color_selector(GNOME_CANVAS_GROUP(shape_root_item));
  display_tool_selector(GNOME_CANVAS_GROUP(shape_root_item));
  display_drawing_area(GNOME_CANVAS_GROUP(shape_root_item));
}


static void display_drawing_area(GnomeCanvasGroup *parent)
{
  draw_root_item = gnome_canvas_item_new (parent,
				gnome_canvas_rect_get_type (),
				"x1", (double) drawing_area_x1,
				"y1", (double) drawing_area_y1,
				"x2", (double) drawing_area_x2,
				"y2", (double) drawing_area_y2,
				"fill_color", "white",
				NULL);
  gtk_signal_connect(GTK_OBJECT(draw_root_item), "event",
		     (GtkSignalFunc) item_event,
		     NULL);
  
}

static void display_color_selector(GnomeCanvasGroup *parent)
{
  GdkPixbuf *pixmap;
  GnomeCanvasItem *item = NULL;
  gint x  = 0;
  gint y  = 0;
  gint x1 = 0;
  gint c  = 0;

  pixmap = gcompris_load_pixmap("draw/color-selector.jpg");
  if(pixmap)
    {
      x = (drawing_area_x2 - drawing_area_x1
	   - gdk_pixbuf_get_width(pixmap))/2 
	+ drawing_area_x1;
      
      y = gcomprisBoard->height - gdk_pixbuf_get_height(pixmap) - 5;

      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) x,
				    "y", (double) y,
				    NULL);
      gdk_pixbuf_unref(pixmap);
    }

  for(x1=x+26; x1<(x+26)+55*10; x1+=55)
    {
      item = gnome_canvas_item_new (parent,
			     gnome_canvas_rect_get_type (),
			     "x1", (double) x1,
			     "y1", (double) y + 8,
			     "x2", (double) x1 + 50,
			     "y2", (double) y + gdk_pixbuf_get_height(pixmap) - 8,
			     "fill_color", colorlist[c],
			     NULL);

      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) color_event,
			 colorlist[c]);

      if(c==0)
	highlight_color_item(item);

      c++;
    }

  currentColor = colorlist[0];
}

static void display_tool_selector(GnomeCanvasGroup *parent)
{
  GdkPixbuf *pixmap;
  GnomeCanvasItem *item = NULL;
  gint x  = 0;
  gint y  = 0;

  pixmap = gcompris_load_pixmap("draw/tool-selector.jpg");
  if(pixmap)
    {
      x = 20;
      y = (drawing_area_y2 - drawing_area_y1 - gdk_pixbuf_get_height(pixmap)) / 2
	+  drawing_area_y1;
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) x,
				    "y", (double) y,
				    NULL);
      gdk_pixbuf_unref(pixmap);
    }

  y += 15;
  x = 24;
  pixmap = gcompris_load_pixmap("draw/tool-rectangle.png");
  if(pixmap)
    {
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) x,
				    "y", (double) y,
				    NULL);
      gdk_pixbuf_unref(pixmap);

      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) tool_event,
			 (void *)TOOL_RECT);
      gcompris_set_image_focus(item, TRUE);

    }
  currentTool = TOOL_RECT;
  currentToolItem = item;

  y += 50;
  pixmap = gcompris_load_pixmap("draw/tool-filledrectangle.png");
  if(pixmap)
    {
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) x,
				    "y", (double) y,
				    NULL);
      gdk_pixbuf_unref(pixmap);

      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) tool_event,
			 (void *)TOOL_FILLED_RECT);
    }
  y += 50;

  pixmap = gcompris_load_pixmap("draw/tool-circle.png");
  if(pixmap)
    {
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) x,
				    "y", (double) y,
				    NULL);
      gdk_pixbuf_unref(pixmap);

      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) tool_event,
			 (void *)TOOL_CIRCLE);
    }
  y += 50;

  pixmap = gcompris_load_pixmap("draw/tool-filledcircle.png");
  if(pixmap)
    {
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) x,
				    "y", (double) y,
				    NULL);
      gdk_pixbuf_unref(pixmap);

      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) tool_event,
			 (void *)TOOL_FILLED_CIRCLE);
    }
  y += 50;

  pixmap = gcompris_load_pixmap("draw/tool-line.png");
  if(pixmap)
    {
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) x,
				    "y", (double) y,
				    NULL);
      gdk_pixbuf_unref(pixmap);

      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) tool_event,
			 (void *)TOOL_LINE);
    }
  y += 50;

  pixmap = gcompris_load_pixmap("draw/tool-move.png");
  if(pixmap)
    {
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) x,
				    "y", (double) y,
				    NULL);
      gdk_pixbuf_unref(pixmap);

      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) tool_event,
			 (void *)TOOL_FILL);
    }
  y += 50;

  pixmap = gcompris_load_pixmap("draw/tool-del.png");
  if(pixmap)
    {
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) x,
				    "y", (double) y,
				    NULL);
      gdk_pixbuf_unref(pixmap);

      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) tool_event,
			 (void *)TOOL_DELETE);
    }
  y += 50;

}

/* Destroy all the items */
static void draw_destroy_all_items()
{
  
  /* Deleting the root item automatically deletes children items */
  if(shape_root_item!=NULL)
    gtk_object_destroy (GTK_OBJECT(shape_root_item));
  shape_root_item = NULL;

  if(draw_root_item!=NULL)
    gtk_object_destroy (GTK_OBJECT(draw_root_item));
  draw_root_item = NULL;

  current_color_item = NULL;
}

static gint
tool_event(GnomeCanvasItem *item, GdkEvent *event, gint tool)
{

  switch (event->type) 
    {
    case GDK_BUTTON_PRESS:
      switch(event->button.button) 
	{
	case 1:
	  if(currentToolItem)
	    gcompris_set_image_focus(currentToolItem, FALSE);

	  currentTool = tool;
	  currentToolItem = item;
	  gcompris_set_image_focus(item, TRUE);
	  break;
	default:
	  break;
	}
    default:
      break;
    }
  return FALSE;
}

static void highlight_color_item(GnomeCanvasItem *item)
{
  // Highligh the selected color
  if(current_color_item)
    gnome_canvas_item_set (current_color_item,
			   "outline_color_rgba", 0x07a3e080,
			   "width_pixels", 0,
			   NULL);

  gnome_canvas_item_set (item,
			 "outline_color_rgba", 0xaef45880,
			 "width_pixels", 3,
			 NULL);
  current_color_item=item;
}

static gint
color_event(GnomeCanvasItem *item, GdkEvent *event, gchar *color)
{
  if(color==NULL)
    return FALSE;

  switch (event->type) 
    {
    case GDK_BUTTON_PRESS:
      switch(event->button.button) 
	{
	case 1:
	  currentColor = color;
	  highlight_color_item(item);
	  gcompris_play_ogg(color, NULL);
	  break;
	default:
	  break;
	}
    default:
      break;
    }
  return FALSE;
}

static void display_anchors(AnchorsItem *anchorsItem, gboolean visible)
{
  if(visible)
    {
      gnome_canvas_item_show(anchorsItem->nw);
      gnome_canvas_item_raise_to_top(anchorsItem->nw);
      gnome_canvas_item_show(anchorsItem->n);
      gnome_canvas_item_raise_to_top(anchorsItem->n);
      gnome_canvas_item_show(anchorsItem->ne);
      gnome_canvas_item_raise_to_top(anchorsItem->ne);
      gnome_canvas_item_show(anchorsItem->w);
      gnome_canvas_item_raise_to_top(anchorsItem->w);
      gnome_canvas_item_show(anchorsItem->e);
      gnome_canvas_item_raise_to_top(anchorsItem->e);
      gnome_canvas_item_show(anchorsItem->sw);
      gnome_canvas_item_raise_to_top(anchorsItem->sw);
      gnome_canvas_item_show(anchorsItem->s);
      gnome_canvas_item_raise_to_top(anchorsItem->s);
      gnome_canvas_item_show(anchorsItem->se);
      gnome_canvas_item_raise_to_top(anchorsItem->se);
    } 
  else
    {
      gnome_canvas_item_hide(anchorsItem->nw);
      gnome_canvas_item_hide(anchorsItem->n);
      gnome_canvas_item_hide(anchorsItem->ne);
      gnome_canvas_item_hide(anchorsItem->w);
      gnome_canvas_item_hide(anchorsItem->e);
      gnome_canvas_item_hide(anchorsItem->sw);
      gnome_canvas_item_hide(anchorsItem->s);
      gnome_canvas_item_hide(anchorsItem->se);
    }
}

static void reset_anchors(AnchorsItem *anchorsItem)
{
  double x1, x2, y1, y2;
  
  gnome_canvas_item_get_bounds(anchorsItem->item,  &x1, &y1, &x2, &y2); 

  gnome_canvas_item_set (anchorsItem->nw,
			 "x1", (double) x1 - DEFAULT_ANCHOR_SIZE,
			 "y1", (double) y1 - DEFAULT_ANCHOR_SIZE,
			 "x2", (double) x1 + DEFAULT_ANCHOR_SIZE,
			 "y2", (double) y1 + DEFAULT_ANCHOR_SIZE,
			 NULL);

  gnome_canvas_item_set (anchorsItem->n,
			 "x1", (double) x1+(x2-x1)/2 - DEFAULT_ANCHOR_SIZE,
			 "y1", (double) y1 - DEFAULT_ANCHOR_SIZE,
			 "x2", (double) x1+(x2-x1)/2 + DEFAULT_ANCHOR_SIZE,
			 "y2", (double) y1 + DEFAULT_ANCHOR_SIZE,
			 NULL);

  gnome_canvas_item_set (anchorsItem->ne,
			 "x1", (double) x2 - DEFAULT_ANCHOR_SIZE,
			 "y1", (double) y1 - DEFAULT_ANCHOR_SIZE,
			 "x2", (double) x2 + DEFAULT_ANCHOR_SIZE,
			 "y2", (double) y1 + DEFAULT_ANCHOR_SIZE,
			 NULL);

  gnome_canvas_item_set (anchorsItem->e,
			 "x1", (double) x2 - DEFAULT_ANCHOR_SIZE,
			 "y1", (double) y1+(y2-y1)/2 - DEFAULT_ANCHOR_SIZE,
			 "x2", (double) x2 + DEFAULT_ANCHOR_SIZE,
			 "y2", (double) y1+(y2-y1)/2 + DEFAULT_ANCHOR_SIZE,
			 NULL);

  gnome_canvas_item_set (anchorsItem->w,
			 "x1", (double) x1 - DEFAULT_ANCHOR_SIZE,
			 "y1", (double) y1+(y2-y1)/2 - DEFAULT_ANCHOR_SIZE,
			 "x2", (double) x1 + DEFAULT_ANCHOR_SIZE,
			 "y2", (double) y1+(y2-y1)/2 + DEFAULT_ANCHOR_SIZE,
			 NULL);

  gnome_canvas_item_set (anchorsItem->sw,
			 "x1", (double) x1 - DEFAULT_ANCHOR_SIZE,
			 "y1", (double) y2 - DEFAULT_ANCHOR_SIZE,
			 "x2", (double) x1 + DEFAULT_ANCHOR_SIZE,
			 "y2", (double) y2 + DEFAULT_ANCHOR_SIZE,
			 NULL);

  gnome_canvas_item_set (anchorsItem->se,
			 "x1", (double) x2 - DEFAULT_ANCHOR_SIZE,
			 "y1", (double) y2 - DEFAULT_ANCHOR_SIZE,
			 "x2", (double) x2 + DEFAULT_ANCHOR_SIZE,
			 "y2", (double) y2 + DEFAULT_ANCHOR_SIZE,
			 NULL);

  gnome_canvas_item_set (anchorsItem->s,
			 "x1", (double) x1+(x2-x1)/2 - DEFAULT_ANCHOR_SIZE,
			 "y1", (double) y2 - DEFAULT_ANCHOR_SIZE,
			 "x2", (double) x1+(x2-x1)/2 + DEFAULT_ANCHOR_SIZE,
			 "y2", (double) y2 + DEFAULT_ANCHOR_SIZE,
			 NULL);

}

static void resize_item(AnchorsItem *anchorsItem, AnchorType anchor, double x, double y)
{
  double x1, y1, x2, y2;
  GnomeCanvasPoints	*points = NULL;
  GnomeCanvasItem	*item = NULL;

  /* Manage border limits */
  if(x<drawing_area_x1)
    x = drawing_area_x1;
  else if(x>drawing_area_x2)
    x = drawing_area_x2;

  if(y<drawing_area_y1)
    y = drawing_area_y1;
  else if(y>drawing_area_y2)
    y = drawing_area_y2;

  item = anchorsItem->item;
  gnome_canvas_item_get_bounds  (item,
				 &x1,
				 &y1,
				 &x2,
				 &y2);

  printf("Resize Anchor = %d\n", anchor);

  if(GNOME_IS_CANVAS_RECT(item) || GNOME_IS_CANVAS_ELLIPSE(item))
    {
      switch(anchor)
	{
	case ANCHOR_E:
	  if(x>x1+1)
	    gnome_canvas_item_set (item,
				   "x2", (double) x,
				   NULL);
	  break;
	case ANCHOR_W:
	  if(x<x2-1)
	    gnome_canvas_item_set (item,
				   "x1", (double) x,
				   NULL);
	  break;
	case ANCHOR_N:
	  if(y<y2-1)
	    gnome_canvas_item_set (item,
				   "y1", (double) y,
				   NULL);
	  break;
	case ANCHOR_S:
	  if(y>y1+1)
	    gnome_canvas_item_set (item,
				   "y2", (double) y,
				   NULL);
	  break;
	case ANCHOR_NW:
	  if(y<y2-1)
	    gnome_canvas_item_set (item,
				   "y1", (double) y,
				   NULL);
	  if(x<x2-1)
	    gnome_canvas_item_set (item,
				   "x1", (double) x,
				   NULL);
	  break;
	case ANCHOR_NE:
	  if(y<y2-1)
	    gnome_canvas_item_set (item,
				   "y1", (double) y,
				   NULL);
	  if(x>x1+1)
	    gnome_canvas_item_set (item,
				   "x2", (double) x,
				   NULL);
	  break;
	case ANCHOR_SW:
	  if(y>y1+1)
	    gnome_canvas_item_set (item,
				   "y2", (double) y,
				   NULL);
	  if(x<x2-1)
	    gnome_canvas_item_set (item,
				   "x1", (double) x,
				   NULL);
	  break;
	case ANCHOR_SE:
	  if(y>y1+1)
	    gnome_canvas_item_set (item,
				   "y2", (double) y,
				   NULL);
	  if(x>x1+1)
	    gnome_canvas_item_set (item,
				   "x2", (double) x,
				   NULL);
	  break;
	}
    }
  else if(GNOME_IS_CANVAS_LINE(item))
    {

      /* I don't know why, I need to shrink the bounding box */
      x1 += 2;
      y1 += 2;
      x2 -= 2;
      y2 -= 2;

      switch(anchor)
	{
	case ANCHOR_E:
	  if(x>x1+1)
	    {
	      points = gnome_canvas_points_new(2);
	      points->coords[0] = (double) x1;
	      points->coords[1] = (double) y1;
	      points->coords[2] = (double) x;
	      points->coords[3] = (double) y2;
	    } 
	  break;
	case ANCHOR_W:
	  if(x<x2-1)
	    {
	      points = gnome_canvas_points_new(2);
	      points->coords[0] = (double) x;
	      points->coords[1] = (double) y1;
	      points->coords[2] = (double) x2;
	      points->coords[3] = (double) y2;
	    }
	  break;
	case ANCHOR_N:
	  if(y<y2-1)
	    {
	      points = gnome_canvas_points_new(2);
	      points->coords[0] = (double) x1;
	      points->coords[1] = (double) y;
	      points->coords[2] = (double) x2;
	      points->coords[3] = (double) y2;
	    }
	  break;
	case ANCHOR_S:
	  if(y>y1+1)
	    {
	      points = gnome_canvas_points_new(2);
	      points->coords[0] = (double) x1;
	      points->coords[1] = (double) y1;
	      points->coords[2] = (double) x2;
	      points->coords[3] = (double) y;
	    }
	  break;
	case ANCHOR_NW:
	  if((y<y2-1) && (x<x2-1))
	    {
	      points = gnome_canvas_points_new(2);
	      points->coords[0] = (double) x;
	      points->coords[1] = (double) y;
	      points->coords[2] = (double) x2;
	      points->coords[3] = (double) y2;
	    }
	  break;
	case ANCHOR_NE:
	  if((y<y2-1) && (x>x1+1))
	    {
	      points = gnome_canvas_points_new(2);
	      points->coords[0] = (double) x1;
	      points->coords[1] = (double) y;
	      points->coords[2] = (double) x;
	      points->coords[3] = (double) y2;
	    }
	  break;
	case ANCHOR_SW:
	  if((y>y1+1) && (x<x2-1))
	    {
	      points = gnome_canvas_points_new(2);
	      points->coords[0] = (double) x;
	      points->coords[1] = (double) y1;
	      points->coords[2] = (double) x2;
	      points->coords[3] = (double) y;
	    }
	  break;
	case ANCHOR_SE:
	  if((y>y1+1) && (x>x1+1))
	    {
	      points = gnome_canvas_points_new(2);
	      points->coords[0] = (double) x1;
	      points->coords[1] = (double) y1;
	      points->coords[2] = (double) x;
	      points->coords[3] = (double) y;
	    }
	  break;
	}

      if(points)
	{
	  gnome_canvas_item_set (item,
				 "points", points,
				 NULL);
      
	  gnome_canvas_points_unref(points);
	}
    }

  reset_anchors(anchorsItem);
}

static void set_item_color(GnomeCanvasItem *item, gchar *color)
{
  gnome_canvas_item_set (GNOME_CANVAS_ITEM(item),
				    "fill_color", currentColor,
				    "outline_color", currentColor,
				    NULL);
}

static GnomeCanvasItem *create_item(double x, double y)
{
  GnomeCanvasItem *item = NULL;
  GnomeCanvasPoints* points = NULL;
  GnomeCanvasItem *item_root_item = NULL;

  item_root_item = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  switch(currentTool)
    {
    case TOOL_RECT:
      // This is a rectangle
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
				    gnome_canvas_rect_get_type (),
				    "x1", (double) x,
				    "y1", (double) y,
				    "x2", (double) x + DEFAULT_ITEM_SIZE,
				    "y2", (double) y + DEFAULT_ITEM_SIZE,
				    "outline_color", currentColor,
				    "width_pixels", 2,
				    NULL);
      break;
    case TOOL_FILLED_RECT:
      // This is a filled rectangle
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
				    gnome_canvas_rect_get_type (),
				    "x1", (double) x,
				    "y1", (double) y,
				    "x2", (double) x + DEFAULT_ITEM_SIZE,
				    "y2", (double) y + DEFAULT_ITEM_SIZE,
				    "fill_color", currentColor,
				    NULL);
      break;      
    case TOOL_CIRCLE:
      // This is an ellipse
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
				    gnome_canvas_ellipse_get_type (),
				    "x1", (double) x,
				    "y1", (double) y,
				    "x2", (double) x + DEFAULT_ITEM_SIZE,
				    "y2", (double) y + DEFAULT_ITEM_SIZE,
				    "outline_color", currentColor,
				    "width_pixels", 2,
				    NULL);
      break;
    case TOOL_FILLED_CIRCLE:
      // This is a filled ellipse
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
				    gnome_canvas_ellipse_get_type (),
				    "x1", (double) x,
				    "y1", (double) y,
				    "x2", (double) x + DEFAULT_ITEM_SIZE,
				    "y2", (double) y + DEFAULT_ITEM_SIZE,
				    "fill_color", currentColor,
				    NULL);
      break;      
    case TOOL_LINE:
      // This is a line
      points = gnome_canvas_points_new(2);
      points->coords[0] = (double) x;
      points->coords[1] = (double) y;
      points->coords[2] = (double) x + DEFAULT_ITEM_SIZE;
      points->coords[3] = (double) y + DEFAULT_ITEM_SIZE;

      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
				    gnome_canvas_line_get_type (),
				    "points", points,
				    "fill_color", currentColor,
				    "width_pixels", 2,
				    NULL);

      gnome_canvas_points_unref(points);
      break;      
    default:
      break;
    }

  if(item)
    {
      double x1, x2, y1, y2;
      GnomeCanvasItem		*anchorItem = NULL;
      AnchorsItem		*anchorsItem = NULL;

      /* Let the new item be on top */
      gnome_canvas_item_raise_to_top(item);

      /* Create the Anchors */
      anchorsItem = g_new(AnchorsItem, 1);

      anchorsItem->rootitem = item_root_item;
      anchorsItem->item = item;

      gnome_canvas_item_get_bounds(item,  &x1, &y1, &x2, &y2); 

      /*----------------------------------------*/
      anchorItem = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
					  gnome_canvas_rect_get_type (),
					  "fill_color_rgba", ANCHOR_COLOR,
					  "outline_color", "black",
					  "width_pixels", 1,
					  NULL);
      anchorsItem->nw = anchorItem;
      gtk_object_set_user_data(GTK_OBJECT(anchorItem), (void *)ANCHOR_NW);

      gtk_signal_connect(GTK_OBJECT(anchorItem), "event",
			 (GtkSignalFunc) item_event_resize,
			 anchorsItem);

      /*----------------------------------------*/
      anchorItem = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
					  gnome_canvas_rect_get_type (),
					  "fill_color_rgba", ANCHOR_COLOR,
					  "outline_color", "black",
					  "width_pixels", 1,
					  NULL);
      anchorsItem->n = anchorItem;
      gtk_object_set_user_data(GTK_OBJECT(anchorItem), (void *)ANCHOR_N);

      gtk_signal_connect(GTK_OBJECT(anchorItem), "event",
			 (GtkSignalFunc) item_event_resize,
			anchorsItem);

      /*----------------------------------------*/
      anchorItem = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
					  gnome_canvas_rect_get_type (),
					  "fill_color_rgba", ANCHOR_COLOR,
					  "outline_color", "black",
					  "width_pixels", 1,
					  NULL);
      anchorsItem->ne = anchorItem;
      gtk_object_set_user_data(GTK_OBJECT(anchorItem), (void *)ANCHOR_NE);

      gtk_signal_connect(GTK_OBJECT(anchorItem), "event",
			 (GtkSignalFunc) item_event_resize,
			 anchorsItem);

      /*----------------------------------------*/
      anchorItem = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
					  gnome_canvas_rect_get_type (),
					  "fill_color_rgba", ANCHOR_COLOR,
					  "outline_color", "black",
					  "width_pixels", 1,
					  NULL);
      anchorsItem->sw = anchorItem;
      gtk_object_set_user_data(GTK_OBJECT(anchorItem), (void *)ANCHOR_SW);

      gtk_signal_connect(GTK_OBJECT(anchorItem), "event",
			 (GtkSignalFunc) item_event_resize,
			 anchorsItem);

      /*----------------------------------------*/
      anchorItem = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
					  gnome_canvas_rect_get_type (),
					  "fill_color_rgba", ANCHOR_COLOR,
					  "outline_color", "black",
					  "width_pixels", 1,
					  NULL);
      anchorsItem->s = anchorItem;
      gtk_object_set_user_data(GTK_OBJECT(anchorItem), (void *)ANCHOR_S);

      gtk_signal_connect(GTK_OBJECT(anchorItem), "event",
			 (GtkSignalFunc) item_event_resize,
			anchorsItem);

      /*----------------------------------------*/
      anchorItem = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
					  gnome_canvas_rect_get_type (),
					  "fill_color_rgba", ANCHOR_COLOR,
					  "outline_color", "black",
					  "width_pixels", 1,
					  NULL);
      anchorsItem->se = anchorItem;
      gtk_object_set_user_data(GTK_OBJECT(anchorItem), (void *)ANCHOR_SE);

      gtk_signal_connect(GTK_OBJECT(anchorItem), "event",
			 (GtkSignalFunc) item_event_resize,
			 anchorsItem);

      /*----------------------------------------*/
      anchorItem = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
					  gnome_canvas_rect_get_type (),
					  "fill_color_rgba", ANCHOR_COLOR,
					  "outline_color", "black",
					  "width_pixels", 1,
					  NULL);
      anchorsItem->w = anchorItem;
      gtk_object_set_user_data(GTK_OBJECT(anchorItem), (void *)ANCHOR_W);

      gtk_signal_connect(GTK_OBJECT(anchorItem), "event",
			 (GtkSignalFunc) item_event_resize,
			 anchorsItem);

      /*----------------------------------------*/
      anchorItem = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
					  gnome_canvas_rect_get_type (),
					  "fill_color_rgba", ANCHOR_COLOR,
					  "outline_color", "black",
					  "width_pixels", 1,
					  NULL);
      anchorsItem->e = anchorItem;
      gtk_object_set_user_data(GTK_OBJECT(anchorItem), (void *)ANCHOR_E);

      reset_anchors(anchorsItem);

      if(selected_anchors_item)
	display_anchors(selected_anchors_item, FALSE);

      selected_anchors_item = anchorsItem;

      display_anchors(anchorsItem, TRUE);

      gtk_signal_connect(GTK_OBJECT(anchorItem), "event",
			 (GtkSignalFunc) item_event_resize,
			 anchorsItem);

      /* Move is performed on the item itself */
      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) item_event_move,
			 anchorsItem);

    }

  return item;
}

/*
 * Special event callback for the resize operation
 */
static gint
item_event_resize(GnomeCanvasItem *item2move, GdkEvent *event, AnchorsItem *anchorsItem)
{
   static double x, y;
   static GnomeCanvasItem *draggingItem = NULL;
   GnomeCanvasItem *item = NULL;
   static int dragging;
   double new_x, new_y;
   double item_x, item_y;
   GdkCursor *fleur = NULL;
   AnchorType anchor;

   if(!gcomprisBoard)
     return FALSE;

   anchor = (AnchorType)gtk_object_get_user_data(GTK_OBJECT(item2move));
   printf("Anchor = %d\n", anchor);

   item = anchorsItem->rootitem;

   switch (event->type) 
     {
     case GDK_BUTTON_PRESS:
       switch(event->button.button) 
         {
         case 1:
	   fleur = gdk_cursor_new(GDK_FLEUR);
	   
	   gnome_canvas_item_grab(item2move,
				  GDK_POINTER_MOTION_MASK | 
				  GDK_BUTTON_RELEASE_MASK,
				  fleur,
				  event->button.time);
	   gdk_cursor_destroy(fleur);
	   draggingItem = item;
	   printf("draggingItem = %p\n", draggingItem);
	   dragging = TRUE;

	   item_x = event->button.x;
	   item_y = event->button.y;
	   gnome_canvas_item_w2i(item2move->parent, &item_x, &item_y);
	   x = item_x;
	   y = item_y;
	   printf("RESIZE BUTTON PRESS:     x = %f    y = %f\n", x, y);
	   break;
	 default:
	   break;
	 }
       break;
     case GDK_BUTTON_RELEASE:
       if(dragging) 
	 {
	   gnome_canvas_item_ungrab(item2move, event->button.time);
	   dragging = FALSE;
	   draggingItem = NULL;	   
	 }
       break;
     case GDK_ENTER_NOTIFY:
       break;

     case GDK_LEAVE_NOTIFY:
       break;

     case GDK_MOTION_NOTIFY:
       printf("RESIZE: GDK_MOTION_NOTIFY dragging = %d event->motion.state=%d\n", dragging, event->motion.state);

       if (dragging && (event->motion.state & GDK_BUTTON1_MASK))
         {
	   item_x = event->button.x;
	   item_y = event->button.y;
	   gnome_canvas_item_w2i(item2move->parent, &item_x, &item_y);

	   new_x = item_x;
	   new_y = item_y;
	       
	   resize_item(anchorsItem, anchor, x, y);

	   x = new_x;
	   y = new_y;
         }
       break;
     default:
       break;
     }

   return(TRUE);
}

/*
 * Special event callback for the move operation
 */
static gint
item_event_move(GnomeCanvasItem *item2move, GdkEvent *event, AnchorsItem *anchorsItem)
{
   static double x, y;
   static double start_x, start_y;
   static GnomeCanvasItem *draggingItem = NULL;
   GnomeCanvasItem *item = NULL;
   static int dragging;
   double new_x, new_y;
   double item_x, item_y;
   GdkCursor *fleur = NULL;

   if(!gcomprisBoard)
     return FALSE;

   item = anchorsItem->rootitem;

   switch (event->type) 
     {
     case GDK_BUTTON_PRESS:
       switch(event->button.button) 
         {
         case 1:
	   switch(currentTool) {
	   case TOOL_RECT:
	   case TOOL_FILLED_RECT:
	   case TOOL_CIRCLE: 
	   case TOOL_FILLED_CIRCLE:
	   case TOOL_LINE:
	     // Move an item
	     if(selected_anchors_item!=anchorsItem)
	       {
		 display_anchors(selected_anchors_item, FALSE);
		 display_anchors(anchorsItem, TRUE);
		 selected_anchors_item = anchorsItem;
	       }
	     
	     fleur = gdk_cursor_new(GDK_FLEUR);
	     
	     gnome_canvas_item_grab(item2move,
				    GDK_POINTER_MOTION_MASK | 
				    GDK_BUTTON_RELEASE_MASK,
				    fleur,
				    event->button.time);
	     gdk_cursor_destroy(fleur);
	     draggingItem = item;
	     printf("draggingItem = %p\n", draggingItem);
	     dragging = TRUE;
	     
	     item_x = event->button.x;
	     item_y = event->button.y;
	     gnome_canvas_item_w2i(item2move->parent, &item_x, &item_y);
	     x = item_x;
	     y = item_y;
	     start_x = item_x;
	     start_y = item_y;
	     printf("MOVE BUTTON PRESS:     x = %f    y = %f\n", x, y);

	     display_anchors(anchorsItem, FALSE);
	     break;

	   case TOOL_DELETE:
	     if(selected_anchors_item == anchorsItem)
	       selected_anchors_item = NULL;

	     gtk_object_destroy (GTK_OBJECT(anchorsItem->rootitem));
	     g_free(anchorsItem);
	     return FALSE;
	     break;

	   case TOOL_FILL:
	     set_item_color(item2move, currentColor);
	     break;

	   default:
	     break;
	   }	     
	   break;
	 default:
	   break;
	 }
       break;
     case GDK_BUTTON_RELEASE:
       if(dragging) 
	 {
	   gnome_canvas_item_ungrab(item2move, event->button.time);
	   dragging = FALSE;
	   draggingItem = NULL;	   

	   /* Moving back the anchors around their shape */
	   gnome_canvas_item_move(item, x - start_x, y - start_y);
	   gnome_canvas_item_move(item2move, start_x - x, start_y - y);
	   display_anchors(anchorsItem, TRUE);
	 }
       break;
     case GDK_ENTER_NOTIFY:
       break;

     case GDK_LEAVE_NOTIFY:
       break;

     case GDK_MOTION_NOTIFY:
       printf("MOVE: GDK_MOTION_NOTIFY dragging = %d event->motion.state=%d\n", dragging, event->motion.state);

       if (dragging && (event->motion.state & GDK_BUTTON1_MASK))
         {
	   double x1, y1, x2, y2;

	   item_x = event->button.x;
	   item_y = event->button.y;
	   gnome_canvas_item_w2i(item2move->parent, &item_x, &item_y);
	   printf("MOVE:     item_x = %f    item_y = %f\n", item_x, item_y);

	   new_x = item_x;
	   new_y = item_y;

	   gnome_canvas_item_get_bounds  (item,
					  &x1,
					  &y1,
					  &x2,
					  &y2);
	  	       
	   if((x1 + new_x - x) < drawing_area_x1 && (new_x - x)<0)
	     new_x = x;
	   else if((x2 + new_x -x) > drawing_area_x2 && (new_x - x)>0)
	     new_x = x;
	   
	   if((y1 + new_y - y) < drawing_area_y1 && (new_y - y)<0)
	     new_y = y;
	   else if((y2 + new_y - y) > drawing_area_y2 && (new_y - y)>0)
	     new_y = y;
	  
	   printf("MOVE:     x = %f    y = %f\n", x, y);
	   printf("MOVE: new x = %f    y = %f\n", new_x, new_y);
	   printf("MOVE:    mx = %f   my = %f\n", new_x - x, new_y - y);

	   gnome_canvas_item_move(item2move, new_x - x, new_y - y);

	   x = new_x;
	   y = new_y;
         }
       break;
     default:
       break;
     }

   return(TRUE);
}

static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, void *shape)
{
   static double x, y;
   double item_x, item_y;
   GnomeCanvasItem *newItem = NULL;

   if(!gcomprisBoard)
     return FALSE;

   switch (event->type) 
     {
     case GDK_BUTTON_PRESS:
       switch(event->button.button) 
         {
         case 1:
         case 3:
	   item_x = event->button.x;
	   item_y = event->button.y;
	   gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

	   x = item_x;
	   y = item_y;
	   
	   switch(currentTool) {
	   case TOOL_RECT:
	   case TOOL_FILLED_RECT:
	   case TOOL_CIRCLE: 
	   case TOOL_FILLED_CIRCLE:
	   case TOOL_LINE:
	       // Create a new item
	     if(event->button.button==1)
	       {
		 printf("Create an item\n");
		 newItem = create_item(x, y);

		 if(newItem==NULL)
		   return FALSE;
	       }
	     break;

	   default:
	     break;
	   }

         default:
           break;
         }
       break;

     case GDK_MOTION_NOTIFY:
       break;
           
     case GDK_BUTTON_RELEASE:
       break;

     default:
       break;
     }
         
   return TRUE;
}
