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

static double resize_item_ref_x = 0;
static double resize_item_ref_y = 0;

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
  TOOL_MOVE		= 6,
  TOOL_RESIZE		= 7
} ToolList;

static ToolList		 currentTool = TOOL_RECT;
static GnomeCanvasItem	*currentToolItem = NULL;

static gboolean doubleclick_lock = FALSE;

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
      drawing_area_y2 = gcomprisBoard->height - 83;

      gcompris_bar_set(0);

      draw_next_level();

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
			 (void *)TOOL_MOVE);
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

  pixmap = gcompris_load_pixmap("draw/tool-resize.png");
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
			 (void *)TOOL_RESIZE);
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

static void resize_item(GnomeCanvasItem *item, double x, double y)
{
  double x1, y1, x2, y2;
  GnomeCanvasPoints* points = NULL;

  /* Manage border limits */
  if(x<drawing_area_x1)
    x = drawing_area_x1;
  else if(x>drawing_area_x2)
    x = drawing_area_x2;

  if(y<drawing_area_y1)
    y = drawing_area_y1;
  else if(y>drawing_area_y2)
    y = drawing_area_y2;


  gnome_canvas_item_get_bounds  (item,
				 &x1,
				 &y1,
				 &x2,
				 &y2);

  switch(currentTool)
    {
    case TOOL_RECT:
    case TOOL_FILLED_RECT:
    case TOOL_CIRCLE:
    case TOOL_FILLED_CIRCLE:
    case TOOL_LINE:
    case TOOL_RESIZE:
      if(GNOME_IS_CANVAS_RECT(item) || GNOME_IS_CANVAS_ELLIPSE(item))
	{
	  if(x>resize_item_ref_x)
	    gnome_canvas_item_set (item,
				   "x2", (double) x,
				   NULL);
	  else
	    gnome_canvas_item_set (item,
				   "x1", (double) x,
				   NULL);
	  
	  if(y>resize_item_ref_y)
	    gnome_canvas_item_set (item,
				   "y2", (double) y,
				   NULL);
	  else
	    gnome_canvas_item_set (item,
				   "y1", (double) y,
				   NULL);
	}
      else if(GNOME_IS_CANVAS_LINE(item))
	{
	  points = gnome_canvas_points_new(2);
	  points->coords[0] = (double) resize_item_ref_x;
	  points->coords[1] = (double) resize_item_ref_y;
	  points->coords[2] = (double) x;
	  points->coords[3] = (double) y;
	  
	  gnome_canvas_item_set (item,
				 "points", points,
				 NULL);
	  
	  gnome_canvas_points_unref(points);
	}
      break;
    case TOOL_DELETE:
    case TOOL_MOVE:
      break;
    default:
      break;
    }
}

static GnomeCanvasItem *create_item(double x, double y)
{
  GnomeCanvasItem *item = NULL;
  GnomeCanvasPoints* points = NULL;

  switch(currentTool)
    {
    case TOOL_RECT:
      // This is a rectangle
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
				    gnome_canvas_rect_get_type (),
				    "x1", (double) x,
				    "y1", (double) y,
				    "x2", (double) x + 2,
				    "y2", (double) y + 2,
				    "outline_color", currentColor,
				    "width_pixels", 2,
				    NULL);
      break;
    case TOOL_FILLED_RECT:
      // This is a filled rectangle
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
				    gnome_canvas_rect_get_type (),
				    "x1", (double) x,
				    "y1", (double) y,
				    "x2", (double) x + 2,
				    "y2", (double) y + 2,
				    "fill_color", currentColor,
				    NULL);
      break;      
    case TOOL_CIRCLE:
      // This is an ellipse
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
				    gnome_canvas_ellipse_get_type (),
				    "x1", (double) x,
				    "y1", (double) y,
				    "x2", (double) x + 2,
				    "y2", (double) y + 2,
				    "outline_color", currentColor,
				    "width_pixels", 2,
				    NULL);
      break;
    case TOOL_FILLED_CIRCLE:
      // This is a filled ellipse
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
				    gnome_canvas_ellipse_get_type (),
				    "x1", (double) x,
				    "y1", (double) y,
				    "x2", (double) x + 2,
				    "y2", (double) y + 2,
				    "fill_color", currentColor,
				    NULL);
      break;      
    case TOOL_LINE:
      // This is a line
      points = gnome_canvas_points_new(2);
      points->coords[0] = (double) x;
      points->coords[1] = (double) y;
      points->coords[2] = (double) x + 2;
      points->coords[3] = (double) y + 2;

      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
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
    gtk_signal_connect(GTK_OBJECT(item), "event",
		       (GtkSignalFunc) item_event,
		       NULL);

  return item;
}

static gint release_doubleclick_lock (GtkWidget *widget, gpointer data)
{
  doubleclick_lock = FALSE;
  return (FALSE);
}

static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, void *shape)
{
   static double x, y;
   double new_x, new_y;
   GdkCursor *fleur = NULL;
   static int dragging;
   double item_x, item_y;
   GnomeCanvasItem *newItem = NULL;

   if(!gcomprisBoard)
     return FALSE;

   item_x = event->button.x;
   item_y = event->button.y;
   gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

   switch (event->type) 
     {
     case GDK_BUTTON_PRESS:
       switch(event->button.button) 
         {
         case 1:
         case 3:
	   if(doubleclick_lock)
	     return FALSE;

	   // FIXME: Should not happen but actually helps
	   if(dragging) 
	     {
	       gnome_canvas_item_ungrab(item, event->button.time);
	       dragging = FALSE;
	     }
	   x = item_x;
	   y = item_y;
	   
	   resize_item_ref_x = item_x;
	   resize_item_ref_y = item_y;

	   switch(currentTool) {
	   case TOOL_RECT:
	   case TOOL_FILLED_RECT:
	   case TOOL_CIRCLE: 
	   case TOOL_FILLED_CIRCLE:
	   case TOOL_LINE:
	       // Create a new item
	     if(event->button.button==1)
	       {
		 newItem = create_item(x, y);
		 if(newItem==NULL)
		   return FALSE;
	       }
	     break;

	   case TOOL_DELETE:
	     if(item!=draw_root_item)
	       gtk_object_destroy (GTK_OBJECT(item));
	     return FALSE;
	     break;

	   case TOOL_RESIZE:
	     break;

	   case TOOL_MOVE:
	     break;
	   }

	 if(currentTool==TOOL_MOVE || event->button.button==3)
	     {
	       double x1, y1, x2, y2;


	       if(item==draw_root_item)
		 return FALSE;
		 
	       // Just resize or move the seleted item
	       newItem = item;
	       
	       gnome_canvas_item_get_bounds  (item,
					      &x1,
					      &y1,
					      &x2,
					      &y2);
	       
	       resize_item_ref_x = x1;
	       resize_item_ref_y = y1;
	     }

	   gnome_canvas_item_raise_to_top(newItem);
	   fleur = gdk_cursor_new(GDK_FLEUR);

	   // Double click never send us the release. Workaround with a timer
	   gtk_timeout_add (1200,
			    (GtkFunction) release_doubleclick_lock, NULL);
	   doubleclick_lock = TRUE;

	   gnome_canvas_item_grab(newItem,
				  GDK_POINTER_MOTION_MASK | 
				  GDK_BUTTON_RELEASE_MASK,
				  fleur,
				  event->button.time);
	   gdk_cursor_destroy(fleur);
	   dragging = TRUE;
	   break;

	 case 2:
	   // Mouse button 2 = delete item
	   if(item!=draw_root_item)
	     gtk_object_destroy (GTK_OBJECT(item));
	   break;

         default:
           break;
         }
       break;

     case GDK_MOTION_NOTIFY:
       if (dragging && ((event->motion.state & GDK_BUTTON3_MASK) || currentTool==TOOL_MOVE)) 
         {
	   double x1, y1, x2, y2;

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

	   gnome_canvas_item_move(item, new_x - x, new_y - y);

	   x = new_x;
	   y = new_y;

	   resize_item_ref_x = x;
	   resize_item_ref_y = y;
         }
       else if (dragging && ((event->motion.state & GDK_BUTTON1_MASK) || currentTool==TOOL_RESIZE)) 
         {
	   new_x = item_x;
	   new_y = item_y;
	       
	   resize_item(item, x, y);

	   x = new_x;
	   y = new_y;
         }
       break;
           
     case GDK_BUTTON_RELEASE:
       if(dragging) 
	 {
	   gnome_canvas_item_ungrab(item, event->button.time);
	   dragging = FALSE;
	 }
       break;

     default:
       break;
     }
         
   return FALSE;
 }



