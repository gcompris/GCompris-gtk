/* gcompris - shapegame.c
 *
 * Time-stamp: <2002/01/13 17:56:32 bruno>
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

#define UNDEFINED "Undefined"

static GList *item_list = NULL;

static GcomprisBoard *gcomprisBoard = NULL;

typedef enum
{
  SHAPE_TARGET		= 1 << 0,
  SHAPE_DUMMY_TARGET	= 1 << 1,
  SHAPE_ICON		= 1 << 2,
  SHAPE_BACKGROUND	= 1 << 3,
  SHAPE_COLORLIST	= 1 << 4
} ShapeType;

/* Let's define the structure for a single shape */
typedef struct _Shape Shape;
struct _Shape {
  char  *name;				/* name of the shape */
  char  *pixmapfile;			/* relative pixmap file name of the shape */
  GnomeCanvasPoints* points; 		/* OR list of points for this shape */
  char  *targetfile;			/* OPTIONAL relative pixmap file name of the target shape, by default 
					   a red point is displayed */
  double x;				/* x coordinate */
  double y;				/* y coordinate */
  double w;				/* width */
  double h;				/* height */
  double zoomx;				/* x zoom factor */
  double zoomy;				/* y zoom factor */
  gint   position;			/* depth position 0=bottom other=intermediate */
  ShapeType type;			/* Type of shape */

  GnomeCanvasItem *item;     	  	/* Canvas item for this shape */
  Shape *icon_shape;			/* Temporary Canvas icon shape for this shape */
  GnomeCanvasItem *bad_item;		/* Temporary bad placed Canvas item for this shape */

  gboolean found;			/* The user found this item */
  GnomeCanvasItem *target_point;       	/* Target point item for this shape */
};

gchar *colorlist [] = 
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

/* This is the list of shape for the current game */
static GList *shape_list = NULL;

/* Hash table of all the shapes in the list of shapes (one by different pixmap plus icon list) */
static GHashTable *shapelist_table = NULL;
static gint SHAPE_BOX_WIDTH_RATIO = 18;

static GnomeCanvasItem *shape_root_item;
static GnomeCanvasItem *shape_list_root_item;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
static void process_ok(void);

static GnomeCanvasItem *shapegame_init_canvas(GnomeCanvasGroup *parent);
static void shapegame_destroy_all_items(void);
static void setup_item(GnomeCanvasItem *item, Shape *shape);
static void shapegame_next_level(void);
static gboolean read_xml_file(char *fname);
static gboolean write_xml_file(char *fname);
static Shape *find_closest_shape(double x, double y, double limit);
static Shape *create_shape(ShapeType type, char *name, char *pixmapfile,  GnomeCanvasPoints* points,
			   char *targetfile, double x, double y, double l, double h, double zoomx, 
			   double zoomy, guint position);
static void display_color_selector(GnomeCanvasGroup *parent);

/* Description of this plugin */
BoardPlugin menu_bp =
{
   NULL,
   NULL,
   N_("Make the puzzle"),
   N_("Drag and Drop the items to rebuild the object"),
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
   process_ok,
   set_level,
   NULL,
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

  if(gcomprisBoard==NULL)
    return;

}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "gcompris/gcompris-shapebg.jpg");

      /* set initial values for this level */
      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel=6;
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL|GCOMPRIS_BAR_OK);

      gcomprisBoard->number_of_sublevel=6;
      gcomprisBoard->sublevel = 0;

      shapegame_next_level();

      pause_board(FALSE);

    }

}

static void
end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      shapegame_destroy_all_items();
      gcomprisBoard->level = 1;       // Restart this game to zero
    }
  gcomprisBoard = NULL;
}

static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=0;
      shapegame_next_level();
    }
}

gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "shapegame")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

/*
 * Returns true if increment is done, false is end of board
 */
static gboolean increment_sublevel()
{
  gcomprisBoard->sublevel++;
  
  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
      board_finished(BOARD_FINISHED_RANDOM);
      return FALSE;
    }
    
    gcomprisBoard->sublevel=0;
  }

  gcompris_bar_set_level(gcomprisBoard);
  return TRUE;
}

/* set initial values for the next level */
static void shapegame_next_level() 
{
  char *filename;

  gcompris_bar_set_level(gcomprisBoard);

  shapegame_destroy_all_items();

  shapegame_init_canvas(gnome_canvas_root(gcomprisBoard->canvas));

  filename = g_strdup_printf("%s/%s/board%d_%d.xml",  
			     PACKAGE_DATA_DIR, gcomprisBoard->boarddir, 
			     gcomprisBoard->level, gcomprisBoard->sublevel);

  printf("1 gcomprisBoard->level %d   filename=%s\n", gcomprisBoard->level, filename);

  while(!g_file_exists(filename)
	&& ((gcomprisBoard->level != 1) || (gcomprisBoard->sublevel!=0)))
    {
  printf("2 gcomprisBoard->level %d\n", gcomprisBoard->level);
      /* Try the next level */
      gcomprisBoard->sublevel=gcomprisBoard->number_of_sublevel;
      if(!increment_sublevel())
	return;
      
      g_free(filename);
      filename = g_strdup_printf("%s/%s/board%d_%d.xml",  
				 PACKAGE_DATA_DIR, gcomprisBoard->boarddir, 
				 gcomprisBoard->level, gcomprisBoard->sublevel);
  printf("3 gcomprisBoard->level %d   filename=%s\n", gcomprisBoard->level, filename);
    }
  read_xml_file(filename);
  
  g_free(filename);
}


static void process_ok()
{
  GList *list;
  gboolean done = TRUE;

  /* Loop through all the shapes to find if all target are found */
  for(list = shape_list; list != NULL; list = list->next) {
    Shape *shape = list->data;

    if(shape->type==SHAPE_TARGET)
      {
	if(shape->found==FALSE)
	  done=FALSE;
      }
  }

  if(done)
    {
      increment_sublevel();

      gcompris_play_sound (SOUNDLISTFILE, "bonus");
      shapegame_next_level();
    }
  else
    gcompris_play_sound (SOUNDLISTFILE, "crash");

}

static void destroy_shape (Shape *shape)
{
  g_free(shape->name);
  g_free(shape->pixmapfile);
  g_free(shape->targetfile);
  if(shape->points!=NULL)
    gnome_canvas_points_unref(shape->points);
  g_free(shape);
}

/* Destroy all the items */
static void shapegame_destroy_all_items()
{
  Shape *shape;

  /* Cleanup of the shapes */
  while(g_list_length(shape_list)>0) 
    {
      shape = g_list_nth_data(shape_list, 0);
      shape_list = g_list_remove (shape_list, shape);
      destroy_shape(shape);
    }
  
  if (shapelist_table)
    {
      /* Deleting the root item automatically deletes children items */
      gtk_object_destroy (GTK_OBJECT(shape_list_root_item));
      gtk_object_destroy (GTK_OBJECT(shape_root_item));

      g_hash_table_destroy (shapelist_table);
      shapelist_table=NULL;
    }
}

static GnomeCanvasItem *shapegame_init_canvas(GnomeCanvasGroup *parent)
{
  GnomeCanvasItem *item = NULL;


  shape_list_root_item = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_group_get_type (),
			   "x", (double)gcomprisBoard->width/SHAPE_BOX_WIDTH_RATIO,
			   "y", (double)0,
			   NULL);
  item_list = g_list_append (item_list, shape_list_root_item);

  shape_root_item = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_group_get_type (),
			   "x", (double)gcomprisBoard->width/SHAPE_BOX_WIDTH_RATIO,
			   "y", (double)0,
			   NULL);
  item_list = g_list_append (item_list, shape_root_item);

  return (item);
}

/**************************************************************
 * Shape list management
 */

/*
 * Shuffle the shape list
 */
static void 
shuffle_shape_list()
{
  GList *icon_list = NULL;
  GList *list = NULL;
  int i;

  /* Create the list of icons */
  for(list = shape_list; list != NULL; list = list->next) {
    Shape *shape = list->data;
    if(shape->type==SHAPE_ICON)
      icon_list = g_list_append (icon_list, shape);
  }
  g_list_free(list);

  /* Loop through all the shapes to swap them eventually */
  for(list = icon_list; list != NULL; list = list->next) {
    Shape *shape1 = list->data;
    Shape *shape2;

    i=rand()%(g_list_length(icon_list));

    shape2 = (Shape *)g_list_nth_data(icon_list, i);
    if(shape2!=NULL)
      {
	if(shape1!=shape2)
	  {
	    /* Swap the shapes */
	    double y;

	    y         = shape1->y;
	    shape1->y = shape2->y;
	    shape2->y = y;

	    gnome_canvas_item_set (shape1->item,
				   "y", shape1->y - shape1->h/2,
				   NULL);
	    gnome_canvas_item_set (shape2->item,
				   "y", shape2->y - shape2->h/2,
				   NULL);
	  }
      }
  }
  g_list_free(icon_list);
}

/* Add the given shape to the list of shapes
 * Do nothing if the shape is already in
 */
static void
add_shape_to_list_of_shapes(Shape *shape)
{
  GnomeCanvasItem *item;
  gint MAX_NUMBER_OF_SHAPES = 8;
  double SHAPE_ELEM_NUMBER_IN_HEIGHT = (double)gcomprisBoard->height/MAX_NUMBER_OF_SHAPES;

  if(!shapelist_table)
    shapelist_table= g_hash_table_new (g_str_hash, g_str_equal);

  /* This pixmap is not yet in the list of shapes */
  if(g_hash_table_lookup (shapelist_table, shape->pixmapfile)==NULL)
    {
      double y_offset = 0;
      GdkPixbuf *pixmap = NULL;

      y_offset = SHAPE_ELEM_NUMBER_IN_HEIGHT/2 +
	g_hash_table_size(shapelist_table)*SHAPE_ELEM_NUMBER_IN_HEIGHT;

      /* So this shape is not yet in, let's put it in now */
      g_hash_table_insert (shapelist_table, shape->pixmapfile, shape);

      if(strcmp(shape->pixmapfile, UNDEFINED)!=0)
	{
	  pixmap = gcompris_load_pixmap(shape->pixmapfile);
	  if(pixmap)
	    {
	      double w, h;
	      guint SHAPE_SIZE = SHAPE_ELEM_NUMBER_IN_HEIGHT - 20;
	      Shape *icon_shape;
	      
	      /* Calc a zoom factor so that the shape will fit in the shapelist
		 whatever its current size */
	      w = SHAPE_SIZE;
	      h = gdk_pixbuf_get_height(pixmap) * ( w / gdk_pixbuf_get_width(pixmap));
	      
	      if(h > SHAPE_SIZE)
		{
		  h = SHAPE_SIZE;
		  w = gdk_pixbuf_get_width(pixmap) * ( h / gdk_pixbuf_get_height(pixmap));
		}
	      
	      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_list_root_item),
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", pixmap, 
					    "x", (double)-w/2,
					    "y", (double)y_offset-h/2,
					    "width", (double) w,
					    "height", (double) h,
					    "width_set", TRUE, 
					    "height_set", TRUE,
					    NULL);
	      gdk_pixbuf_unref(pixmap);
	      
	      icon_shape = create_shape(SHAPE_ICON, shape->name, shape->pixmapfile, shape->points, shape->targetfile,
					(double)0, (double)y_offset,
					(double)w, (double)h, 
					(double)1, (double)1,
					0);
	      icon_shape->item = item;
	      
	      setup_item(item, icon_shape);
	      item_list = g_list_append (item_list, item);
	    }
	}
    }
}

/*
 * Find the closest shape from the given point if it is located at
 * a distance under the given limit.
 */
static Shape *find_closest_shape(double x, double y, double limit)
{
  GList *list;
  double goodDist = limit;
  Shape *candidateShape = NULL;

  /* loop through all our shapes */
  for(list = shape_list; list != NULL; list = list->next) {
    Shape *shape = list->data;
    double dist;

    if(shape->type==SHAPE_TARGET)
      {
	/* Calc the distance between this shape and the given coord */
	dist = sqrtf(powf((shape->x-x),2)*powf((shape->y-y),2));
	
	if(dist<goodDist)
	  {
	    goodDist=dist;
	    candidateShape=shape;
	  }
      }
  }

  return candidateShape;
}

#if 0
static void dump_shape(Shape *shape)
{
  printf("dump_shape name=%s found=%d type=%d ", shape->name, shape->found, shape->type);
  if(shape->bad_item)
    printf("bad_item=TRUE ");
  if(shape->icon_shape)
    printf("icon_shape=%s", shape->icon_shape->name);
  printf("\n");
}
#endif

/*
 * Given the shape, if it has an icon, it puts it back to
 * the list of shapes
 */
static void shape_goes_back_to_list(Shape *shape, GnomeCanvasItem *item)
{
  if(shape->icon_shape!=NULL)
    {
      /* There was a previous icon here, put it back to the list */
      gnome_canvas_item_move(shape->icon_shape->item, 
			     shape->icon_shape->x - shape->x,
			     shape->icon_shape->y - shape->y);
      gnome_canvas_item_show(shape->icon_shape->item);
      gcompris_set_image_focus(shape->icon_shape->item, TRUE);
      shape->icon_shape=NULL;

      gnome_canvas_item_hide(item);
      gcompris_play_sound (SOUNDLISTFILE, "gobble");
    }
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
      x = (gcomprisBoard->width
	   - gcomprisBoard->width/SHAPE_BOX_WIDTH_RATIO*2
	   - gdk_pixbuf_get_width(pixmap))/2 
	+ gcomprisBoard->width/SHAPE_BOX_WIDTH_RATIO;
      
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
      c++;
    }
}

static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, Shape *shape)
{
   static double x, y;
   static double offset_x, offset_y;
   double new_x, new_y;
   GdkCursor *fleur;
   static int dragging;
   double item_x, item_y;

   if(!get_board_playing())
     return FALSE;

   if(shape==NULL) {
     g_warning("Shape is NULL : Should not happen");
     return FALSE;
   }

   item_x = event->button.x;
   item_y = event->button.y;
   gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

   switch (event->type) 
     {
     case GDK_BUTTON_PRESS:
       switch(event->button.button) 
         {
         case 1:
           if (event->button.state & GDK_SHIFT_MASK)
             {
	       /* Cheat code to save an XML file */
	       write_xml_file("/tmp/gcompris-board.xml");
	     }
	   else
	     {
	       x = item_x;
	       y = item_y;
	       
	       switch(shape->type)
		 {
		 case SHAPE_TARGET:
		   gnome_canvas_item_hide(item);
		   gcompris_set_image_focus(item, FALSE);

		   if( shape->icon_shape!=NULL)
		     {
		       item = shape->icon_shape->item;
		       gnome_canvas_item_show(item);
		       gcompris_set_image_focus(item, TRUE);
		       shape->icon_shape=NULL;
		     }
		   break;
		 default:
		   break;
		 }
	       /* This records the offset between the mouse pointer and the grabbed item center */
	       offset_x = x - shape->x;
	       offset_y = y - shape->y;
	       
	       if(item==NULL)
		 return FALSE;
	       
	       fleur = gdk_cursor_new(GDK_FLEUR);

	       /* In order to have our item above the others, I need to reparent it */
	       gnome_canvas_item_reparent (item, (GnomeCanvasGroup *)shape_root_item);
	       gnome_canvas_item_raise_to_top(item);

	       gnome_canvas_item_grab(item,
				      GDK_POINTER_MOTION_MASK | 
				      GDK_BUTTON_RELEASE_MASK,
				      fleur,
				      event->button.time);
	       gdk_cursor_destroy(fleur);
	       dragging = TRUE;
	     }
	   break;
	   
         case 3:
	   /* If There was a previous icon here, put it back to the list */
	   shape_goes_back_to_list(shape, item);
	   shape->found = FALSE;
	   break;

         default:
           break;
         }
       break;

     case GDK_MOTION_NOTIFY:
       if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) 
         {
	   new_x = item_x;
	   new_y = item_y;
	       
	   gnome_canvas_item_move(item, new_x - x, new_y - y);

	   x = new_x;
	   y = new_y;
         }
       break;
           
     case GDK_BUTTON_RELEASE:
       if(dragging) 
	 {
	   Shape *targetshape = NULL;

	   gnome_canvas_item_ungrab(item, event->button.time);
	   dragging = FALSE;

	   gnome_canvas_item_reparent (item, (GnomeCanvasGroup *)shape_list_root_item);

	   targetshape = find_closest_shape(item_x, item_y, 1000);
	   if(targetshape!=NULL)
	     {
	       /* Finish the placement of the grabbed item anyway */
	       gnome_canvas_item_move(item, 
				      targetshape->x - x + offset_x, 
				      targetshape->y - y + offset_y);
	       /* Hide it */
	       gnome_canvas_item_hide(item);

	       if(strcmp(targetshape->name, shape->name)==0)
		 {
		   /* You got it right perfect */
		   if(targetshape->bad_item!=NULL)
		     {
		       gnome_canvas_item_hide(targetshape->bad_item);
		       gtk_object_destroy (GTK_OBJECT(targetshape->bad_item));
		       targetshape->bad_item=NULL;
		     }
		   targetshape->found = TRUE;
		   gnome_canvas_item_show(targetshape->item);
		 }
	       else
		 {
		   /* Oups wrong position, create the wrong pixmap item */
		   GdkPixbuf *pixmap;
		   GnomeCanvasItem *item;

		   targetshape->found = FALSE;
		   gnome_canvas_item_hide(targetshape->item);

		   /* There is already a bad item, delete it */
		   if(targetshape->bad_item!=NULL)
		     gtk_object_destroy (GTK_OBJECT(targetshape->bad_item));

		   pixmap = gcompris_load_pixmap(shape->pixmapfile);

		   item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
						 gnome_canvas_pixbuf_get_type (),
						 "pixbuf", pixmap, 
						 "x", (double)targetshape->x - (gdk_pixbuf_get_width(pixmap) 
										* targetshape->zoomx)/2,
						 "y", (double)targetshape->y - (gdk_pixbuf_get_height(pixmap) 
										* targetshape->zoomy/2),
						 "width", (double) gdk_pixbuf_get_width(pixmap) * targetshape->zoomx,
						 "height", (double) gdk_pixbuf_get_height(pixmap) * targetshape->zoomy,
						 "width_set", TRUE, 
						 "height_set", TRUE,
						 NULL);
		   gdk_pixbuf_unref(pixmap);

		   targetshape->bad_item = item;
		   item_list = g_list_append (item_list, item);
		   setup_item(item, targetshape);
		 }

	       /* If There was a previous icon here, put it back to the list */
	       shape_goes_back_to_list(targetshape, item);

	       /* Set the icon item */
	       targetshape->icon_shape=shape;

	     }
	   else
	     {
	       /* The item has no close position */
	       switch(shape->type)
		 {
		 case SHAPE_TARGET:
		   gnome_canvas_item_hide(item);
		   if( shape->icon_shape!=NULL)
		     {
		       item = shape->icon_shape->item;
		       gnome_canvas_item_show(item);
		     }
		   break;
		 case SHAPE_ICON:
		   break;
		 default:
		   break;
		 }
	       /* Move back the item home */
	       gnome_canvas_item_move(item, 
				      shape->x - x + offset_x,
				      shape->y - y + offset_y);
	     }
	 }
       break;

     default:
       break;
     }
         
   return FALSE;
 }

/*
 * Special mode to edit the board by moving the target point
 */
static gint
item_event_edition(GnomeCanvasItem *item, GdkEvent *event, Shape *shape)
{
   static double x, y;
   double new_x, new_y;
   GdkCursor *fleur;
   static int dragging;
   double item_x, item_y;

   if(!get_board_playing())
     return FALSE;

   if(shape==NULL) {
     g_warning("Shape is NULL : Should not happen");
     return FALSE;
   }

   item_x = event->button.x;
   item_y = event->button.y;
   gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

   switch (event->type) 
     {
     case GDK_BUTTON_PRESS:
       switch(event->button.button) 
         {
         case 1:
           if (event->button.state & GDK_SHIFT_MASK)
             {
	       /* Cheat code to save an XML file */
	       write_xml_file("/tmp/gcompris-board.xml");
	     }
	   else
	     {
	       x = item_x;
	       y = item_y;
	       
	       item = shape->target_point;

	       fleur = gdk_cursor_new(GDK_FLEUR);

	       gnome_canvas_item_grab(item,
				      GDK_POINTER_MOTION_MASK | 
				      GDK_BUTTON_RELEASE_MASK,
				      fleur,
				      event->button.time);
	       gdk_cursor_destroy(fleur);
	       dragging = TRUE;
	     }
	   break;
	   
         default:
           break;
         }
       break;

     case GDK_MOTION_NOTIFY:
       if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) 
         {
	   new_x = item_x;
	   new_y = item_y;
	       
	   gnome_canvas_item_move(item, new_x - x, new_y - y);
	   gnome_canvas_item_move(shape->item, new_x - x, new_y - y);

	   // Update the shape's coordinate
	   shape->x = shape->x + new_x - x;
	   shape->y = shape->y + new_y - y;

	   x = new_x;
	   y = new_y;
         }
       break;
           
     case GDK_BUTTON_RELEASE:
       if(dragging) 
	 {

	   gnome_canvas_item_ungrab(item, event->button.time);
	   gnome_canvas_item_raise_to_top(item);
	   dragging = FALSE;

	 }
       break;

     default:
       break;
     }
         
   return FALSE;

}


static void
setup_item(GnomeCanvasItem *item, Shape *shape)
{
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event,
		     shape);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
}

/*
 * Thanks for George Lebl <jirka@5z.com> for his Genealogy example
 * for all the XML stuff there
 */

/* Adds a shape to the canvas */
static void
add_shape_to_canvas(Shape *shape)
{
  GdkPixbuf *pixmap;
  GdkPixbuf *targetpixmap;
  GnomeCanvasItem *item = NULL;

  /* checking of preconditions, this is usefull for tracking down bugs,
     but you should not count on these always being compiled in */
  g_return_if_fail(shape != NULL);

  if(shape->type==SHAPE_TARGET)
    {
      if(strcmp(shape->targetfile, UNDEFINED)!=0)
	{
	  targetpixmap = gcompris_load_pixmap(shape->targetfile);
	  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
					gnome_canvas_pixbuf_get_type (),
					"pixbuf", targetpixmap, 
					"x", (double)shape->x - 
					(gdk_pixbuf_get_width(targetpixmap) * shape->zoomx)/2,
					"y", (double)shape->y -
					(gdk_pixbuf_get_height(targetpixmap) * shape->zoomy)/2,
					"width", (double) gdk_pixbuf_get_width(targetpixmap) 
					* shape->zoomx,
					"height", (double) gdk_pixbuf_get_height(targetpixmap) 
					* shape->zoomy,
					"width_set", TRUE, 
					"height_set", TRUE,
					NULL);
	  gdk_pixbuf_unref(targetpixmap);
	} 
      else
	{
	  /* Display a point to highlight the target location of this shape */
	  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
					gnome_canvas_ellipse_get_type(),
					"x1", (double)shape->x-2.5,
					"y1", (double)shape->y-2.5,
					"x2", (double)shape->x+2.5,
					"y2", (double)shape->y+2.5,
					"fill_color_rgba", 0xEf000080,
					"outline_color", "black",
					"width_pixels", 0,
					NULL);
	  shape->target_point = item;
	  gtk_signal_connect(GTK_OBJECT(item), "event",
			     (GtkSignalFunc) item_event_edition,
			     shape);
	}
      gnome_canvas_item_lower_to_bottom(item);
      item_list = g_list_append (item_list, item);
    }
  
  if(shape->points!=NULL)
    {
      printf("it's a point \n");
      item = gnome_canvas_item_new(GNOME_CANVAS_GROUP(shape_root_item),
				   gnome_canvas_polygon_get_type (),
				   "points", shape->points,
				   "fill_color", "grey",
				   "outline_color", "black",
				   "width_units", 1.0,
				   NULL);
    }
  else
    {
      printf("it's an image ? shape->pixmapfile=%s\n", shape->pixmapfile);
      if(strcmp(shape->pixmapfile, UNDEFINED)!=0)
	{
	  printf("it's an image \n");
	  pixmap = gcompris_load_pixmap(shape->pixmapfile);
	  if(pixmap)
	    {	
	      /* Display the shape itself but hide it until the user puts the right shape on it */
	      /* I have to do it this way for the positionning (lower/raise) complexity          */
	      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", pixmap, 
					    "x", (double)shape->x-(gdk_pixbuf_get_width(pixmap) * shape->zoomx)/2,
					    "y", (double)shape->y-(gdk_pixbuf_get_height(pixmap) * shape->zoomy)/2,
					    "width", (double) gdk_pixbuf_get_width(pixmap) * shape->zoomx,
					    "height", (double) gdk_pixbuf_get_height(pixmap) * shape->zoomy,
					    "width_set", TRUE, 
					    "height_set", TRUE,
					    NULL);
	      gdk_pixbuf_unref(pixmap);
	    }
	}
    }

  if(shape->position>=1)
    gnome_canvas_item_lower(item, ABS(shape->position)); 
  
  /* Associate this item to this shape */
  shape->item = item;

  if(shape->type==SHAPE_TARGET || shape->type==SHAPE_DUMMY_TARGET)
    {
      setup_item(item, shape);
      item_list = g_list_append (item_list, item);

      gnome_canvas_item_hide(item);
      add_shape_to_list_of_shapes(shape);
    }
  else if(shape->type==SHAPE_BACKGROUND)
    gnome_canvas_item_lower_to_bottom(item);

}

void create_title(char *name, double x, double y, char *justification)
{
  GnomeCanvasItem *item;
  GdkFont *gdk_font;

  /* Load a gdk font */
  gdk_font = gdk_font_load ("-adobe-times-medium-r-normal--*-180-*-*-*-*-*-*");

  item = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
			   gnome_canvas_text_get_type (),
			   "text", name,
			   "font_gdk", gdk_font,
			   "x", x,
			   "y", y,
			   "anchor", GTK_ANCHOR_CENTER,
			   "justification", justification,
			   "fill_color", "white",
			   NULL);

  gnome_canvas_item_raise_to_top(item);
}

static Shape *
create_shape(ShapeType type, char *name, char *pixmapfile, GnomeCanvasPoints* points,
	     char *targetfile, double x, double y, 
	     double w, double h, double zoomx, 
	     double zoomy, guint position)
{
  Shape *shape;
  
  /* allocate a new shape */
  shape = g_new(Shape,1);
  
  shape->name = g_strdup(name);
  shape->pixmapfile = g_strdup(pixmapfile);
  shape->points = points;
  shape->targetfile = g_strdup(targetfile);
  shape->x = x;
  shape->y = y;
  shape->w = w;
  shape->h = h;
  shape->zoomx = zoomx;
  shape->zoomy = zoomy;
  shape->position = position;
  shape->type = type;

  shape->bad_item = NULL;
  shape->icon_shape = NULL;

  shape->found = FALSE;

  /* add the shape to the list */
  shape_list = g_list_append(shape_list, shape);
  
  return shape;
}


static void
add_xml_shape_to_data(xmlNodePtr xmlnode, GNode * child)
{
  char *name, *cx, *cy, *cd, *czoomx, *czoomy, *cposition, *ctype, *justification;
  char *pixmapfile = NULL;
  char *targetfile = NULL;
  double x, y, zoomx, zoomy;
  GnomeCanvasPoints* points = NULL;
  gchar **d;
  gint i, j;
  guint position;
  ShapeType type = SHAPE_TARGET;
  Shape *shape;

  if(/* if the node has no name */
     !xmlnode->name ||
     /* or if the name is not "Shape" */
     (g_strcasecmp(xmlnode->name,"Shape")!=0 &&
     /* or if the name is not "Title" */
      g_strcasecmp(xmlnode->name,"Title")!=0 &&
     /* or if the name is not "Option" */
      g_strcasecmp(xmlnode->name,"Option")!=0)
     )
    return;
  
  /* get the name of the shape */
  name = xmlGetProp(xmlnode,"name");

  pixmapfile = xmlGetProp(xmlnode,"pixmapfile");
  /* if unspecified, make it UNDEFINED */
  if(!pixmapfile) pixmapfile = UNDEFINED;

  targetfile = xmlGetProp(xmlnode,"targetfile");
  /* if unspecified, make it UNDEFINED */
  if(!targetfile) targetfile = UNDEFINED;

  /*********************************/
  /* get the points for a polygone */
  /* The list of points is similar to the one define in the SVG standard */
  /* FIXME : The implementation is incomplete, a point still needs to be added
     to shapelist and add management for it's x/y coordinates */
  cd = xmlGetProp(xmlnode,"points");
  if(!cd) 
    {
      cd = "";
    }
  else
    {
      d = g_strsplit(cd, " ", 64);

      j=0;
      while(d[j]!=NULL) 
	{
	  j++;
	}

      points = gnome_canvas_points_new(j/2);

      for(i=0; i<j; i++)
	{
	  points->coords[i] = atof(d[i]);
	}
      g_strfreev(d);
    }

  /* get the X coord of the shape */
  cx = xmlGetProp(xmlnode,"x");
  if(!cx) cx = "100";
  x = atof(cx);

  /* get the Y coord of the shape */
  cy = xmlGetProp(xmlnode,"y");
  if(!cy) cy = "100";
  y = atof(cy);

  /* get the ZOOMX coord of the shape */
  czoomx = xmlGetProp(xmlnode,"zoomx");
  if(!czoomx) czoomx = "1";
  zoomx = atof(czoomx);

  /* get the ZOOMY coord of the shape */
  czoomy = xmlGetProp(xmlnode,"zoomy");
  if(!czoomy) czoomy = "1";
  zoomy = atof(czoomy);

  /* get the POSITION coord of the shape */
  cposition = xmlGetProp(xmlnode,"position");
  if(!cposition) cposition = "0";
  position = atoi(cposition);

  /* get the TYPE of the shape */
  ctype = xmlGetProp(xmlnode,"type");
  if(!ctype) ctype = "SHAPE_TARGET"; /* SHAPE_TARGET is default */
  if(g_strcasecmp(ctype,"SHAPE_TARGET")==0)
    type = SHAPE_TARGET;
  else if(g_strcasecmp(ctype,"SHAPE_DUMMY_TARGET")==0)
    type = SHAPE_DUMMY_TARGET;
  else if (g_strcasecmp(ctype,"SHAPE_BACKGROUND")==0)
    type = SHAPE_BACKGROUND;
  else if (g_strcasecmp(ctype,"SHAPE_COLORLIST")==0)
    type = SHAPE_COLORLIST;

  /* get the JUSTIFICATION of the Title */
  justification = xmlGetProp(xmlnode,"justification");
  if(!justification) justification = "GTK_JUSTIFICATION_CENTER"; /* GTK_JUSTIFICATION_CENTER is default */


  if(g_strcasecmp(xmlnode->name,"Shape")==0)
    {
      /* add the shape to the database */
      /* WARNING : I do not initialize the width and height since I don't need them */
      shape = create_shape(type, name, pixmapfile, points, targetfile, x, y, 
			   (double)0, (double)0,
			   zoomx, zoomy, position);
      add_shape_to_canvas(shape);
    } 
  else if (g_strcasecmp(xmlnode->name,"Title")==0)
    {
      create_title(name, x, y, justification);
    }
  else if (g_strcasecmp(xmlnode->name,"Option")==0)
    {
      display_color_selector(GNOME_CANVAS_GROUP(shape_root_item));
    }
  
}

/* parse the doc, add it to our internal structures and to the clist */
static void
parse_doc(xmlDocPtr doc)
{
  xmlNodePtr node;
  
  /* find <Shape> nodes and add them to the list, this just
     loops through all the children of the root of the document */
  for(node = doc->children->children; node != NULL; node = node->next) {
    /* add the shape to the list, there are no children so
       we pass NULL as the node of the child */
    add_xml_shape_to_data(node,NULL);
  }
  shuffle_shape_list();
}



/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set */
static gboolean
read_xml_file(char *fname)
{
  /* pointer to the new doc */
  xmlDocPtr doc;
  
  g_return_val_if_fail(fname!=NULL,FALSE);
  
  /* if the file doesn't exist */
  if(!g_file_exists(fname)) 
    {
      g_warning(_("Couldn't find file %s !"), fname);
      return FALSE;
    }
      g_warning(_("find file %s !"), fname);

  /* parse the new file and put the result into newdoc */
  doc = xmlParseFile(fname);
  /* in case something went wrong */
  if(!doc)
    return FALSE;
  
  if(/* if there is no root element */
     !doc->children ||
     /* if it doesn't have a name */
     !doc->children->name ||
     /* if it isn't a ShapeGame node */
     g_strcasecmp(doc->children->name,"ShapeGame")!=0) {
    xmlFreeDoc(doc);
    return FALSE;
  }
  
  /* parse our document and replace old data */
  parse_doc(doc);
  
  xmlFreeDoc(doc);
  
  return TRUE;
}

/* add a shape to xmlnode from node */
static void
write_shape_to_xml(xmlNodePtr xmlnode, Shape *shape)
{
  xmlNodePtr newxml;
  gchar *tmp;

  g_return_if_fail(xmlnode!=NULL);
  g_return_if_fail(shape!=NULL);
  
  /* make a new xml node (as a child of xmlnode) with an
     empty content */
  newxml = xmlNewChild(xmlnode,NULL,"Shape",NULL);
  /* set properties on it */
  xmlSetProp(newxml,"name",shape->name);
  xmlSetProp(newxml,"pixmapfile",shape->pixmapfile);

  tmp = g_strdup_printf("%f", shape->x);
  xmlSetProp(newxml,"x",tmp);
  g_free(tmp);

  tmp = g_strdup_printf("%f", shape->y);
  xmlSetProp(newxml,"y",tmp);
  g_free(tmp);

  tmp = g_strdup_printf("%f", shape->zoomx);
  xmlSetProp(newxml,"zoomx",tmp);
  g_free(tmp);

  tmp = g_strdup_printf("%f", shape->zoomy);
  xmlSetProp(newxml,"zoomy",tmp);
  g_free(tmp);

  tmp = g_strdup_printf("%d", shape->position);
  xmlSetProp(newxml,"position",tmp);
  g_free(tmp);

}

/* write an xml file 
 * This is used only for creating shapegame boards
 */
static gboolean
write_xml_file(char *fname)
{
  /* pointer to the new doc */
  xmlDocPtr doc;
  GList *list;
  
  g_return_val_if_fail(fname!=NULL,FALSE);
  
  /* create new xml document with version 1.0 */
  doc = xmlNewDoc("1.0");
  /* create a new root node "ShapeGame" */
  doc->children = xmlNewDocNode(doc, NULL, "ShapeGame", NULL);
  
  /* loop through all our shapes */
  for(list = shape_list; list != NULL; list = list->next) {
    Shape *shape = list->data;
    if(shape->type!=SHAPE_ICON)
      write_shape_to_xml(doc->children, shape);
  }
  
  /* try to save the file */
  if(xmlSaveFile(fname,doc) == -1) {
    xmlFreeDoc(doc);
    return FALSE;
  }
  
  xmlFreeDoc(doc);
  
  return TRUE;
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
