/* gcompris - shapegame.c
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

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include <math.h>
#include <string.h>

#include "gcompris/gcompris.h"
#include "gcompris/pixbuf_util.h"

#define SOUNDLISTFILE PACKAGE

#define SQUARE_LIMIT_DISTANCE 100.0

static int gamewon;

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;
static gboolean shadow_enable;

#define POINT_COLOR_OFF 0xEf000080
#define POINT_COLOR_ON  0x00EF0080

typedef enum
  {
    SHAPE_TARGET	= 1 << 0,
    SHAPE_DUMMY_TARGET	= 1 << 1,
    SHAPE_ICON		= 1 << 2,
    SHAPE_BACKGROUND	= 1 << 3,
  } ShapeType;

/* Let's define the structure for a single shape */
typedef struct _Shape Shape;
struct _Shape {
  char  *name;				/* name of the shape */
  char  *tooltip;			/* optional tooltip for the shape */
  char  *pixmapfile;			/* relative pixmap file name of the shape */
  char  *targetfile;			/* OPTIONAL relative pixmap file name of the target shape, by default
					   a red point is displayed */
  double x;				/* x coordinate */
  double y;				/* y coordinate */
  double w;				/* width */
  double h;				/* height */
  double zoomx;				/* x zoom factor */
  double zoomy;				/* y zoom factor */
  gint   position;			/* depth position 0=bottom other=intermediate */
  char  *soundfile;			/* relative sound file to be played when pressing mouse button */
  ShapeType type;			/* Type of shape */

  GooCanvasItem *item;     	  	/* Canvas item for this shape */
				  	/* Root index which this item is in the shapelist */
  GdkPixbuf *pixmap;			/* The pixmap of the target item */
  guint shapelistgroup_index;	  	/* Root index which this item is in the shapelist */
  Shape *icon_shape;			/* Temporary Canvas icon shape for this shape */
  Shape *target_shape;			/* If this is an icon shape then point to its shape */

  GooCanvasItem *target_point;       	/* Target point item for this shape */
  GooCanvasItem *targetitem;       	/* Target item for this shape (if targetfile is defined) */
  double offset_x, offset_y;
  Shape *shape_place;			/* the shape place in this place */
  Shape *placed ;			/* where is placed this shape */
  };

/* This is the list of shape for the current game */
static GList *shape_list	= NULL;
static GList *shape_list_group	= NULL;
static int current_shapelistgroup_index	= -1;

static GooCanvasItem *next_shapelist_item     = NULL;		/* Canvas item button for the next shapelist */
static GooCanvasItem *previous_shapelist_item = NULL;   	/* Canvas item button for the previous shapelist */

/* Let's define the structure for the shapebox list that contains the icons of the shapes */
typedef struct _ShapeBox ShapeBox;
struct _ShapeBox {
  double x;				/* x coordinate */
  double y;				/* y coordinate */
  double w;				/* width */
  double h;				/* height */
  guint  nb_shape_x;			/* Number of shape on x */
  guint  nb_shape_y;			/* Number of shape on y */
  };
static ShapeBox shapeBox;

#define BUTTON_SPACE		40

/* Hash table of all the shapes in the list of shapes (one by different pixmap plus icon list) */
static GHashTable *shapelist_table = NULL;
static gint SHAPE_BOX_WIDTH_RATIO = 18;

static GooCanvasItem	*shape_root_item;
static GooCanvasItem	*shape_list_root_item;

/* The tooltip */
static GooCanvasItem	*tooltip_root_item;
static GooCanvasItem	*tooltip_text_item;
static GooCanvasItem	*tooltip_bg_item;

/* The continue button */
static GooCanvasItem	*continue_root_item;

static GooCanvasItem	*selector_item;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void 		 pause_board (gboolean pause);
static void 		 end_board (void);
static gboolean 	 is_our_board (GcomprisBoard *gcomprisBoard);
static void 		 set_level (guint level);
static void 		 process_ok(void);
static gint		 key_press(guint keyval, gchar *commit_str, gchar *preedit_str);
static void	         config_start (GcomprisBoard *agcomprisBoard,
				       GcomprisProfile *aProfile);
static void	         config_stop (void);

static void              shapegame_init_canvas(GooCanvasItem *parent);
static void 		 shapegame_destroy_all_items(void);
static void 		 setup_item(GooCanvasItem *item, Shape *shape);
static void 		 shapegame_next_level(void);
static gboolean 	 read_xml_file(char *fname);
static Shape 		*find_closest_shape(double x, double y, double limit);
static Shape 		*create_shape(ShapeType type, char *name, char *tooltip,
				      char *pixmapfile,
				      char *targetfile, double x, double y,
				      double w, double h, double zoomx,
				      double zoomy, guint position, char *soundfile);
static gboolean 	 increment_sublevel(void);
static void 		 create_title(char *name, double x, double y,
				      GtkAnchorType anchor,
				      guint32 color_rgba,
				      gchar *color_background);
static gint		 item_event_ok(GooCanvasItem *item, GooCanvasItem *target,
				       GdkEvent *event,
				       gchar *data);
static gint item_event_drag(GooCanvasItem *item, GooCanvasItem *target,
			    GdkEvent *event, Shape *shape);
#if DEBUG
static void dump_shapes(void);
static void dump_shape(Shape *shape);
#endif
static void update_shapelist_item(void);
static void auto_process(void);

static gint drag_mode;
/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Make the puzzle",
    "Drag and Drop the items to rebuild the object",
    "Bruno Coudoin <bruno.coudoin@free.fr>",
    NULL,
    NULL,
    NULL,
    NULL,
    start_board,
    pause_board,
    end_board,
    is_our_board,
    key_press,
    NULL,
    set_level,
    NULL,
    NULL,
    config_start,
    config_stop
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(shapegame)

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{

  if(gcomprisBoard==NULL)
    return;

  if(gamewon == TRUE && pause == FALSE) /* the game is won */
    {
      if(increment_sublevel())
	shapegame_next_level();
    }

  board_paused = pause;
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{
  gchar *filename = NULL;
  gboolean default_background = TRUE;

  GHashTable *config = gc_db_get_board_conf();

  if (strcmp(agcomprisBoard->name, "imagename")==0){
    gc_locale_change(g_hash_table_lookup( config, "locale"));
  }

  gchar *drag_mode_str = g_hash_table_lookup( config, "drag_mode");

  if (drag_mode_str && (strcmp (drag_mode_str, "NULL") != 0))
    drag_mode = g_ascii_strtod(drag_mode_str, NULL);
  else
    drag_mode = 0;

  g_hash_table_destroy(config);

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard = agcomprisBoard;

      /* disable im_context */
      gcomprisBoard->disable_im_context = TRUE;

      /* set initial values for this level */
      gcomprisBoard->level = 1;

      /* Calculate the maxlevel based on the available data file for this board */
      gcomprisBoard->maxlevel=1;
      /**/
      while( (filename = gc_file_find_absolute("%s/board%d_0.xml",
					       gcomprisBoard->boarddir,
					       gcomprisBoard->maxlevel,
					       NULL)) )
	{
	  gcomprisBoard->maxlevel++;
	  g_free(filename);
	}
      g_free(filename);

      gcomprisBoard->maxlevel--;

      gc_bar_set(GC_BAR_CONFIG|GC_BAR_LEVEL);
      gc_bar_location(10, -1, 0.6);


      gcomprisBoard->sublevel = 0;

      /* In this board, the sublevels are dynamicaly discovered based on data files */
      gcomprisBoard->number_of_sublevel=G_MAXINT;


      if(gcomprisBoard->mode!=NULL)
	if(g_strncasecmp(gcomprisBoard->mode, "background=", 11)==0)
	  {
	    gchar *tmp = NULL;

	    tmp = g_malloc(strlen(gcomprisBoard->mode));
	    tmp = strcpy(tmp, gcomprisBoard->mode + 11);

	    gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas), tmp);
	    default_background = FALSE;
	    g_free(tmp);
	  }

      if(default_background)
	{
	  // Default case, load the default background
	  gc_set_default_background(goo_canvas_get_root_item(gcomprisBoard->canvas));
	}

      // And the vertical selector
      selector_item =
	goo_canvas_svg_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
			    gc_skin_rsvg_get(),
			    "svg-id", "#SELECTOR",
			    "pointer-events", GOO_CANVAS_EVENTS_NONE,
			    NULL);

      /* FIXME: This no more works ! */
      gc_drag_start(goo_canvas_get_root_item(gcomprisBoard->canvas),
		    (GcDragFunc) item_event_drag, drag_mode);
      shapegame_next_level();

      pause_board(FALSE);

    }
}

static void
end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      gc_drag_stop(goo_canvas_get_root_item(gcomprisBoard->canvas));
      pause_board(TRUE);
      shapegame_destroy_all_items();
      gcomprisBoard->level = 1;       // Restart this game to zero
    }

  if (strcmp(gcomprisBoard->name, "imagename")==0){
    gc_locale_reset();
  }

  if (selector_item)
    goo_canvas_item_remove(selector_item);
  selector_item = NULL;


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
	  gcomprisBoard->plugin = &menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

/*
 * Keypress here are use for entering the editing mode
 */
static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
{
  if(!gcomprisBoard)
    return FALSE;

  /* Add some filter for control and shift key */
  switch (keyval)
    {
      /* Avoid all this keys to be interpreted by this game */
    case GDK_Shift_L:
    case GDK_Shift_R:
    case GDK_Control_L:
    case GDK_Control_R:
    case GDK_Caps_Lock:
    case GDK_Shift_Lock:
    case GDK_Meta_L:
    case GDK_Meta_R:
    case GDK_Alt_L:
    case GDK_Alt_R:
    case GDK_Super_L:
    case GDK_Super_R:
    case GDK_Hyper_L:
    case GDK_Hyper_R:
    case GDK_Num_Lock:
      return FALSE;
    case GDK_KP_Enter:
    case GDK_Return:
      return FALSE;
    case GDK_Right:
    case GDK_Delete:
    case GDK_BackSpace:
    case GDK_Left:
      break;
    }

  return TRUE;
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
    gcomprisBoard->sublevel=0;

    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : restart
      gcomprisBoard->level = 1;
      gcomprisBoard->sublevel=0;
    }

  }

  return TRUE;
}

/* set initial values for the next level */
static void shapegame_next_level()
{
  char *filename;

  gamewon = FALSE;
  gc_drag_stop(goo_canvas_get_root_item(gcomprisBoard->canvas));

  shapegame_destroy_all_items();
  next_shapelist_item = previous_shapelist_item = NULL;
  shapegame_init_canvas(goo_canvas_get_root_item(gcomprisBoard->canvas));

  while( ((filename = gc_file_find_absolute("%s/board%d_%d.xml",
					    gcomprisBoard->boarddir,
					    gcomprisBoard->level,
					    gcomprisBoard->sublevel,
					    NULL)) == NULL)
	 && ((gcomprisBoard->level != 1) || (gcomprisBoard->sublevel!=0)))
    {
      /* Try the next level */
      gcomprisBoard->sublevel = gcomprisBoard->number_of_sublevel;
      if(!increment_sublevel())
	{
	  g_free(filename);
	  return;
	}
    }

  gc_bar_set_level(gcomprisBoard);

  read_xml_file(filename);
  gc_drag_start(goo_canvas_get_root_item(gcomprisBoard->canvas),
		(GcDragFunc) item_event_drag, drag_mode);
  g_free(filename);
}


static void process_ok()
{
  gamewon = TRUE;

  /* Show the tooltip to let the user continue the game */
  g_object_set (continue_root_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
}

static void destroy_shape (Shape *shape)
{
  g_free(shape->name);
  g_free(shape->pixmapfile);
  g_free(shape->targetfile);
  g_free(shape->soundfile);
  g_free(shape->tooltip);
  if(shape->pixmap)
    gdk_pixbuf_unref(shape->pixmap);
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

  g_list_free(shape_list);

  if (shapelist_table)
    {
      /* Deleting the root item automatically deletes children items */
      goo_canvas_item_remove(shape_list_root_item);
      shape_list_root_item = NULL;

      goo_canvas_item_remove(shape_root_item);
      shape_root_item = NULL;

      goo_canvas_item_remove(tooltip_root_item);
      tooltip_root_item = NULL;

      gc_item_focus_remove(continue_root_item, NULL);
      goo_canvas_item_remove(continue_root_item);
      continue_root_item = NULL;

      g_hash_table_destroy (shapelist_table);
      shapelist_table=NULL;

      g_list_free(shape_list_group);
      shape_list_group = NULL;
      current_shapelistgroup_index = -1;
    }
}

static void shapegame_init_canvas(GooCanvasItem *parent)
{

  shape_root_item = goo_canvas_group_new (parent, NULL);
  goo_canvas_item_translate(shape_root_item,
			    BOARDWIDTH/SHAPE_BOX_WIDTH_RATIO,
			    0);

  shape_list_root_item = goo_canvas_group_new (parent, NULL);

  /* Create the tooltip area */
  tooltip_root_item = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					    NULL);
  goo_canvas_item_translate(tooltip_root_item, 10, BOARDHEIGHT-70);


  tooltip_bg_item = \
    goo_canvas_rect_new (tooltip_root_item,
			 0,
			 0,
			 0,
			 0,
			 "stroke_color_rgba", 0xFFFFFFFFL,
			 "fill_color_rgba", 0x0000FF90L,
			 "line-width", (double) 2,
			 "radius-x", (double) 10,
			 "radius-y", (double) 10,
			 NULL);

  tooltip_text_item = \
    goo_canvas_text_new (tooltip_root_item,
			 "",
			 15,
			 15,
			 -1,
			 GTK_ANCHOR_WEST,
			 "font", gc_skin_font_board_small,
			 "fill_color_rgba", gc_skin_color_text_button,
			 NULL);

  /* Hide the tooltip */
  g_object_set (tooltip_root_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);


  /* Create the continue button */
  continue_root_item = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					    NULL);
  goo_canvas_item_translate(continue_root_item, 5, 5);


  continue_root_item = \
    goo_canvas_svg_new (continue_root_item,
			gc_skin_rsvg_get(),
			"svg-id", "#OK",
			NULL);
  SET_ITEM_LOCATION(continue_root_item, 15, 15);
  gc_item_focus_init(continue_root_item, NULL);

  g_signal_connect(continue_root_item,
		   "button_press_event",
		   (GtkSignalFunc) item_event_ok,
		   "continue_click");

  /* Hide the continue */
  g_object_set (continue_root_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

}

/**************************************************************
 * Shape list management
 */

/* Add the given shape to the list of shapes
 * Do nothing if the shape is already in
 */
static void
add_shape_to_list_of_shapes(Shape *shape)
{
  GooCanvasItem *item;
  GooCanvasItem *shape_list_group_root = NULL;
  double ICON_GAP    = 5.0;
  double ICON_HEIGHT = (double)(shapeBox.h / shapeBox.nb_shape_y) - ICON_GAP;
  double ICON_WIDTH  = (double)(shapeBox.w / shapeBox.nb_shape_x) - ICON_GAP;

  if(!shapelist_table)
    shapelist_table= g_hash_table_new (g_str_hash, g_str_equal);

  /*----------------------------------------------------------------------*/
  /* If the first list is full, add the previous/forward buttons          */
  if(g_hash_table_size(shapelist_table) == (shapeBox.nb_shape_x * shapeBox.nb_shape_y))
    {
      previous_shapelist_item = \
	goo_canvas_svg_new (shape_list_root_item,
			      gc_skin_rsvg_get(),
			      "svg-id", "#PREVIOUS",
			      NULL);
      SET_ITEM_LOCATION(previous_shapelist_item,
			shapeBox.x - 5,
			shapeBox.h);

      g_signal_connect(previous_shapelist_item,
		       "button_press_event",
		       (GtkSignalFunc) item_event_ok,
		       "previous_shapelist");
      gc_item_focus_init(previous_shapelist_item, NULL);

      next_shapelist_item = \
	goo_canvas_svg_new (shape_list_root_item,
			    gc_skin_rsvg_get(),
			    "svg-id", "#NEXT",
			    NULL);

      SET_ITEM_LOCATION(next_shapelist_item,
			shapeBox.x + shapeBox.w / 2,
			shapeBox.h);

      g_signal_connect(next_shapelist_item, "button_press_event",
		       (GtkSignalFunc) item_event_ok,
		       "next_shapelist");
      gc_item_focus_init(next_shapelist_item, NULL);

      g_object_set (next_shapelist_item, "visibility",
		    GOO_CANVAS_ITEM_INVISIBLE, NULL);

    }

  /*----------------------------------------------------------------------*/
  /* Do We need to create a new list                                      */
  if(g_hash_table_size(shapelist_table)%(shapeBox.nb_shape_x * shapeBox.nb_shape_y)==0)
    {
      current_shapelistgroup_index++;

      // Hide the previous group
      if(current_shapelistgroup_index>=1)
	{
	  shape_list_group_root = GOO_CANVAS_ITEM(g_list_nth_data(shape_list_group,
								  current_shapelistgroup_index-1));
	  //g_object_set (shape_list_group_root, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
	  item = g_list_nth_data(shape_list_group, current_shapelistgroup_index-1);
	  g_object_set (item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
	}

      // We need to start a new shape list group
      shape_list_group_root = \
	goo_canvas_group_new (shape_list_root_item,
			      NULL);

      shape_list_group = g_list_append (shape_list_group, shape_list_group_root);

    }
  else
    {
      // Get the current shapelist group
      shape_list_group_root = g_list_nth_data(shape_list_group,
					      current_shapelistgroup_index);
    }

  /*----------------------------------------------------------------------*/
  /* This pixmap is not yet in the list of shapes                         */
  if(g_hash_table_lookup (shapelist_table, shape->pixmapfile) == NULL)
    {
      double y_offset = 0;
      double x_offset = 0;
      GdkPixbuf *pixmap = NULL;

      y_offset = shapeBox.y + (ICON_HEIGHT/2 +
			       (g_hash_table_size(shapelist_table) % shapeBox.nb_shape_y) *
			       ICON_HEIGHT);

      x_offset = shapeBox.x + (ICON_WIDTH/2 +
			       ((g_hash_table_size(shapelist_table) %
				 ( shapeBox.nb_shape_x * shapeBox.nb_shape_y)) /
				shapeBox.nb_shape_y) *
			       ICON_WIDTH);

      /* So this shape is not yet in, let's put it in now */
      g_hash_table_insert (shapelist_table, shape->pixmapfile, shape);

      if(shape->pixmapfile)
	{
	  pixmap = gc_pixmap_load(shape->pixmapfile);
	  if(pixmap)
	    {
	      double w, h, z;
	      Shape *icon_shape;

	      /* Calc a zoom factor so that the shape will fit in the shapelist
		 whatever its current size */
	      w = ICON_WIDTH;
	      h = gdk_pixbuf_get_height(pixmap) * (w / gdk_pixbuf_get_width(pixmap));
	      z = ICON_WIDTH / gdk_pixbuf_get_width(pixmap);

	      if(h > ICON_HEIGHT)
		{
		  h = ICON_HEIGHT;
		  w = gdk_pixbuf_get_width(pixmap) * ( h / gdk_pixbuf_get_height(pixmap));
		  z = ICON_HEIGHT / gdk_pixbuf_get_height(pixmap);
		}
	      if(h < 20 || w < 20)
		{
		  GdkPixbuf *scale, *hand;

		  scale = gdk_pixbuf_scale_simple(pixmap, w, h, GDK_INTERP_BILINEAR);
		  gdk_pixbuf_unref(pixmap);

		  pixmap = gdk_pixbuf_new( GDK_COLORSPACE_RGB, TRUE, 8,
					   ICON_WIDTH, ICON_HEIGHT);
		  gdk_pixbuf_fill(pixmap, 0xffffff00);
		  // add the shape
		  gdk_pixbuf_copy_area( scale, 0, 0, w, h,
					pixmap, (ICON_WIDTH-w )/2, (ICON_HEIGHT-h)/2 );
		  gdk_pixbuf_unref(scale);

		  // add the hand
		  hand = gc_pixmap_load("shapegame/hand.svg");
		  h = ICON_HEIGHT/3;
		  w = gdk_pixbuf_get_width(hand) * h / gdk_pixbuf_get_height(hand);
		  scale = gdk_pixbuf_scale_simple(hand, w, h, GDK_INTERP_BILINEAR);
		  gdk_pixbuf_copy_area(scale, 0, 0, w, h,
				       pixmap, ICON_WIDTH-w,0);
		  gdk_pixbuf_unref(hand);
		  gdk_pixbuf_unref(scale);

		  w = ICON_WIDTH;
		  h = ICON_HEIGHT;
		  z = 1;
		}

	      item = goo_canvas_image_new (shape_list_group_root,
					   pixmap,
					   0,
					   0,
					   NULL);
	      goo_canvas_item_translate(item,
					x_offset - w/2,
					y_offset - h/2);
	      goo_canvas_item_scale(item, z, z);
	      g_object_set_data(G_OBJECT(item), "z", GINT_TO_POINTER((int)(z * 1000)));
	      gdk_pixbuf_unref(pixmap);

	      icon_shape = create_shape(SHAPE_ICON, shape->name, shape->tooltip,
					shape->pixmapfile, shape->targetfile,
					(double)x_offset -w/2, (double)y_offset -h/2,
					(double)w, (double)h,
					(double)shape->zoomx, (double)shape->zoomy,
					0, shape->soundfile);
	      icon_shape->item = item;
	      icon_shape->target_shape = shape;
	      shape->icon_shape = icon_shape;
	      icon_shape->shapelistgroup_index = current_shapelistgroup_index;
	      shape->shapelistgroup_index = current_shapelistgroup_index;
	      setup_item(item, icon_shape);
	      gc_item_focus_init(item, NULL);
	    }
	}
    }
}

/*
 * Find the closest shape from the given point if it is located at
 * a distance under the given square limit.
 */
static Shape *
find_closest_shape(double x, double y, double limit)
{
  GList *list;
  double goodDist = limit;
  Shape *candidateShape = NULL;

  /* loop through all our shapes */
  for(list = shape_list; list != NULL; list = list->next)
    {
      Shape *shape = list->data;
      double dist;

      if(shape->type == SHAPE_TARGET)
	{
	  /* Calc the distance between this shape and the given coord */
	  dist = sqrt(pow((shape->x-x),2) + pow((shape->y-y),2));
	  if(dist<goodDist)
	    {
	      goodDist=dist;
	      candidateShape = shape;
	    }
	}
    }

  return candidateShape;
}

#if DEBUG

static void dump_shapes(void)
{
  GList *list;

  printf("---- Shapes ----\n");
  for(list = shape_list; list ; list = list->next)
    {
      Shape * s = list->data;
      if(s->type == SHAPE_TARGET || s->type == SHAPE_ICON)
	dump_shape(s);
    }
}

static void
dump_shape(Shape *shape)
{
  if(shape->type == SHAPE_TARGET && (shape->placed || shape->shape_place))
    {
      printf("%s :", shape->name);
      if(shape->placed)
	printf(" %s -> %s", shape->name, shape->placed->name);
      else
	printf("       ");
      if(shape->shape_place)
	printf(" %s -> %s", shape->shape_place->name, shape->name);
      printf("\n");
    }
}
#endif

/* it puts a shape back to the list of shapes */
static void
shape_goes_back_to_list(Shape *shape)
{
  gdouble z;

  if(shape->type == SHAPE_ICON)
    shape = shape->target_shape;

  g_object_set (shape->item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
  /* replace the icon */
  gc_item_absolute_move(shape->icon_shape->item,
			shape->icon_shape->x,
			shape->icon_shape->y);

  z = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (shape->icon_shape->item),
					  "z"));
  z = z / 1000;

  goo_canvas_item_scale(shape->icon_shape->item, z, z);

  g_object_set (shape->icon_shape->item, "visibility",
		GOO_CANVAS_ITEM_VISIBLE, NULL);

  if(shape->placed)
    {
      shape->placed->shape_place = NULL;
      shape->placed = NULL;
    }

  update_shapelist_item();

  gc_sound_play_ogg ("sounds/flip.wav", NULL);
}

/* switch off all point, and switch on this point
   if shape is NULL, switch off all */
void
target_point_switch_on(Shape *shape_on)
{
  GList *list;
  Shape *shape;

  for(list = shape_list; list ; list = list ->next)
    {
      shape = list -> data;
      if(shape->type == SHAPE_TARGET && ! shape->targetfile)
	g_object_set(shape->target_point,
		     "fill_color_rgba",
		     shape == shape_on ? POINT_COLOR_ON : POINT_COLOR_OFF,
		     NULL);
    }
}

static gint
item_event_drag(GooCanvasItem *item,
		GooCanvasItem *target,
		GdkEvent *event, Shape *shape)
{
  static GooCanvasItem *shadow_item = NULL;
  static GooCanvasItem *dragged;
  double item_x, item_y;
  Shape *found_shape;

  if(board_paused)
    return FALSE;

  switch(event->type)
    {
    case GDK_BUTTON_PRESS:
      gc_sound_play_ogg ("sounds/bleep.wav", NULL);
      switch(shape->type)
	{
	case SHAPE_TARGET:
	  /* unplace this shape */
	  if (shape->placed && shape->placed->target_point)
	    g_object_set (shape->placed->target_point, "visibility",
			  GOO_CANVAS_ITEM_VISIBLE, NULL);
	  shape->placed->shape_place = NULL;
	  shape->placed = NULL;
	  /* No break on purpose */
	case SHAPE_ICON:
	  gc_drag_offset_save(event);
	  gc_drag_offset_get(&shape->offset_x, &shape->offset_y);
	  if (shape->soundfile)
	    {
	      /* If the soundfile has space ' ' in it, then it is assumed that it is a list
	       * of sound rather than a single one */
	      char *p = NULL;
	      char *soundfile = g_strdup(shape->soundfile);
	      char *soundfiles = soundfile;

	      while ((p = strstr (soundfiles, " ")))
		{
		  *p='\0';
		  gc_sound_play_ogg(soundfiles, NULL);
		  soundfiles = p + 1;
		}

	      if (soundfiles != soundfile)
		gc_sound_play_ogg(soundfiles, NULL);
	      else
		gc_sound_play_ogg(soundfile, NULL);
	      g_free(soundfile);
	    }
	  break;

	default:
	  break;
	}

      if(shadow_enable)
	{
	  if(shadow_item)
	    goo_canvas_item_remove(shadow_item);

	  // initialise shadow shape
	  GdkPixbuf *dest;

	  dest = gdk_pixbuf_copy(shape->target_shape->pixmap);
	  pixbuf_add_transparent(dest, 100);
	  shadow_item = goo_canvas_image_new(shape_root_item,
					     dest,
					     0,
					     0,
					     NULL);
	  g_object_set(shadow_item, "visibility",
		       GOO_CANVAS_ITEM_INVISIBLE, NULL);
	  gdk_pixbuf_unref(dest);
	}
      dragged = shape->item;
      gc_drag_item_move(event, NULL);
      break;

    case GDK_MOTION_NOTIFY:
      if (item != dragged)
	break;

      gc_drag_item_move(event, NULL);

      item_x = event->button.x;
      item_y = event->button.y;
      goo_canvas_convert_from_item_space(goo_canvas_item_get_canvas(item),
					 item, &item_x, &item_y);

      found_shape = find_closest_shape(item_x - BOARDWIDTH/SHAPE_BOX_WIDTH_RATIO,
				       item_y, SQUARE_LIMIT_DISTANCE);
      if(shadow_enable)
	{
	  if(found_shape)
	    {
	      GooCanvasBounds bounds;

	      goo_canvas_item_get_bounds (shadow_item, &bounds);
	      gc_item_absolute_move(shadow_item,
				    found_shape->x - (bounds.x2 - bounds.x1) / 2
				    + BOARDWIDTH/SHAPE_BOX_WIDTH_RATIO,
				    found_shape->y - (bounds.y2 - bounds.y1) / 2);
	      g_object_set (shadow_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
	    }
	  else
	    g_object_set (shadow_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
	}

      target_point_switch_on(found_shape);
      break;

    case GDK_BUTTON_RELEASE:
      if (item != dragged)
	break;

      item_x = event->button.x;
      item_y = event->button.y;
      goo_canvas_convert_from_item_space(goo_canvas_item_get_canvas(item),
					 item, &item_x, &item_y);

      if(shadow_enable && shadow_item)
	{
	  goo_canvas_item_remove(shadow_item);
	  shadow_item = NULL;
	}

      target_point_switch_on(NULL);

      found_shape = find_closest_shape(item_x - BOARDWIDTH/SHAPE_BOX_WIDTH_RATIO,
				       item_y,
				       SQUARE_LIMIT_DISTANCE);

      if(found_shape)
	{
	  GooCanvasBounds bounds;
	  GooCanvasItem *target_item;

	  target_item = shape->target_shape->item;

	  if(found_shape->shape_place)
	    shape_goes_back_to_list(found_shape->shape_place);

	  gc_sound_play_ogg ("sounds/line_end.wav", NULL);

	  /* place the target item */
	  goo_canvas_item_get_bounds(target_item, &bounds);

	  gc_item_absolute_move(target_item,
				found_shape->x - (bounds.x2 - bounds.x1) / 2
				+ BOARDWIDTH/SHAPE_BOX_WIDTH_RATIO,
				found_shape->y - (bounds.y2 - bounds.y1) / 2);

	  if(found_shape->target_point)
	    g_object_set (found_shape->target_point, "visibility",
			  GOO_CANVAS_ITEM_INVISIBLE, NULL);
	  if(target_item)
	    {
	      g_object_set (target_item, "visibility",
			    GOO_CANVAS_ITEM_VISIBLE, NULL);
	      goo_canvas_item_raise(target_item, NULL);
	    }

	  if(shape->type == SHAPE_ICON)
	    g_object_set (shape->item, "visibility",
			  GOO_CANVAS_ITEM_INVISIBLE, NULL);

	  shape->target_shape->placed = found_shape;
	  found_shape->shape_place = shape->target_shape;

	  auto_process();
	  update_shapelist_item();
	}
      else
	{
	  shape_goes_back_to_list(shape);
	}
      break;

    default:
      break;
    }
  return FALSE;
}

static gint
item_event(GooCanvasItem *item, GooCanvasItem *target,
	   GdkEvent *event, Shape *shape)
{
  if(!gcomprisBoard || board_paused)
    return FALSE;

  g_assert(shape);

  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
      if(shape->tooltip) {
	g_object_set(tooltip_text_item,
		     "text", gettext(shape->tooltip),
		     NULL);
	g_object_set (tooltip_root_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);

	/* Set the background */
	GooCanvasBounds bounds;
	goo_canvas_item_get_bounds (tooltip_text_item, &bounds);
	g_object_set(tooltip_bg_item,
		     "width", bounds.x2 - bounds.x1 + 30,
		     "height", bounds.y2 - bounds.y1 + 15,
		     NULL);

      }
      break;

    case GDK_LEAVE_NOTIFY:
      if(shape->tooltip)
	g_object_set (tooltip_root_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      break;

    case GDK_BUTTON_PRESS:
      if(event->button.button == 3)
	shape_goes_back_to_list(shape);

    default:
      break;
    }

  return FALSE;
}

static int
get_element_count_listgroup(int listgroup_index)
{
  int count=0, i;
  Shape *sh;
  for (i=0;i<g_list_length(shape_list);i++)
    {
      sh = g_list_nth_data(shape_list,i);
      if( sh->shapelistgroup_index == listgroup_index &&
	  sh->type == SHAPE_TARGET && ! sh->placed)
	count ++;
    }
  return count;
}

static int
get_no_void_group(int direction)
{
  int index = current_shapelistgroup_index;

  direction = direction>0 ? 1 : -1;

  index += direction;
  while(0 <= index && index < g_list_length(shape_list_group))
    {
      if(get_element_count_listgroup(index))
	return index;
      index += direction;
    }
  return current_shapelistgroup_index;
}

static void
auto_process(void)
{
  GList *list;
  gboolean done = TRUE;

  /* Loop through all the shapes to find if all target are in place */
  for(list = shape_list; list != NULL; list = list->next)
    {
      Shape *shape = list->data;

      if(shape->type == SHAPE_TARGET)
	{
	  if(shape->placed != shape)
	    done=FALSE;
	}
    }

  if(done)
    process_ok();
}

static void
update_shapelist_item(void)
{
  if(! next_shapelist_item || !previous_shapelist_item)
    return;
  if(get_element_count_listgroup(current_shapelistgroup_index) ==0)
    {
      int index;
      GooCanvasItem *root_item;

      index = get_no_void_group(-1);
      if(index == current_shapelistgroup_index)
	index = get_no_void_group(1);
      if(index != current_shapelistgroup_index)
        {
	  root_item = g_list_nth_data(shape_list_group, current_shapelistgroup_index);
	  g_object_set (root_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
	  root_item = g_list_nth_data(shape_list_group, index);
	  g_object_set (root_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
	  current_shapelistgroup_index = index;
        }
    }

  if(get_no_void_group(1) == current_shapelistgroup_index)
    g_object_set (next_shapelist_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
  else
    g_object_set (next_shapelist_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
  if(get_no_void_group(-1) == current_shapelistgroup_index)
    g_object_set (previous_shapelist_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
  else
    g_object_set (previous_shapelist_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
#if DEBUG
  dump_shapes();
#endif
}

/* Callback for the previous / next shape operations */
static gint
item_event_ok(GooCanvasItem *item, GooCanvasItem *target,
	      GdkEvent *event, gchar *data)
{
  GooCanvasItem	*root_item;

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      gc_sound_play_ogg ("sounds/bleep.wav", NULL);
      root_item = g_list_nth_data(shape_list_group, current_shapelistgroup_index);
      g_object_set (root_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

      if(!strcmp(data, "previous_shapelist"))
	{
	  current_shapelistgroup_index = get_no_void_group(-1);
	  update_shapelist_item();
	}
      else if(!strcmp(data, "next_shapelist"))
	{
	  current_shapelistgroup_index = get_no_void_group(1);
	  update_shapelist_item();
	}
      else if(!strcmp(data, "continue_click"))
	if(gamewon == TRUE)
	  gc_bonus_display(gamewon, GC_BONUS_FLOWER);

      root_item = g_list_nth_data(shape_list_group, current_shapelistgroup_index);
      g_object_set (root_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);

    default:
      break;
    }
  return FALSE;
}

static void
setup_item(GooCanvasItem *item, Shape *shape)
{
  g_signal_connect(item, "enter_notify_event",
		   (GtkSignalFunc) item_event,
		   shape);
  g_signal_connect(item, "leave_notify_event",
		   (GtkSignalFunc) item_event,
		   shape);
  g_signal_connect(item, "button_press_event",
		   (GtkSignalFunc) item_event,
		   shape);
  g_signal_connect(item, "button_press_event",
		   (GtkSignalFunc) gc_drag_event, shape);
  g_signal_connect(item, "button_release_event",
		   (GtkSignalFunc) gc_drag_event, shape);
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
  GooCanvasItem *item = NULL;

  g_return_if_fail(shape != NULL);

  if(shape->type == SHAPE_TARGET)
    {
      if(shape->targetfile)
	{
	  if(shape->targetfile[0] != '\0')
	    {
	      targetpixmap = gc_pixmap_load(shape->targetfile);
	      shape->w = (double)gdk_pixbuf_get_width(targetpixmap);
	      shape->h = (double)gdk_pixbuf_get_height(targetpixmap);

	      item = goo_canvas_image_new (shape_root_item,
					   targetpixmap,
					   0, 0,
					   NULL);
	      goo_canvas_item_translate(item,
					shape->x - shape->w / 2,
					shape->y - shape->h / 2);
	      goo_canvas_item_scale(item,
				    shape->zoomx, shape->zoomy);

	      shape->targetitem = item;
	      gdk_pixbuf_unref(targetpixmap);
	    }
	  // An empty targetfile means no target and no point
	}
      else
	{
	  int point_size = 6;
	  /* Display a point to highlight the target location of this shape */
	  item = goo_canvas_ellipse_new (shape_root_item,
					 shape->x,
					 shape->y,
					 point_size,
					 point_size,
					 "fill_color_rgba", POINT_COLOR_OFF,
					 "stroke-color", "black",
					 "line-width", 2.0,
					 NULL);
	  shape->target_point = item;
	}
      if (item)
	goo_canvas_item_lower(item, NULL);
    }

  if(shape->pixmapfile)
    {
      pixmap = gc_pixmap_load(shape->pixmapfile);
      if(pixmap)
	{
	  shape->w = (double)gdk_pixbuf_get_width(pixmap);
	  shape->h = (double)gdk_pixbuf_get_height(pixmap);

	  /* Display the shape itself but hide it until the user puts the right shape on it */
	  /* I have to do it this way for the positionning (lower/raise) complexity          */
	  item = goo_canvas_image_new (shape_root_item,
				       pixmap,
				       0,
				       0,
				       NULL);
	  goo_canvas_item_translate(item,
				    shape->x - shape->w / 2,
				    shape->y - shape->h / 2);
	  goo_canvas_item_scale(item, shape->zoomx, shape->zoomy);

	  shape->pixmap = pixmap;
	}
    }

  /* Associate this item to this shape */
  shape->item = item;

  if(shape->type==SHAPE_TARGET || shape->type==SHAPE_DUMMY_TARGET)
    {
      setup_item(item, shape);

      g_object_set (item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      add_shape_to_list_of_shapes(shape);
    }
  else if(shape->type==SHAPE_BACKGROUND)
    {
      goo_canvas_item_lower(item, NULL);
    }

}

static void
create_title(char *name, double x, double y,
	     GtkAnchorType anchor,
	     guint32 color_rgba,
	     gchar *color_background)
{
  GooCanvasItem *item;

  item = \
    goo_canvas_text_new (shape_root_item,
			 gettext(name),
			 x,
			 y,
			 -1,
			 anchor,
			 "font", gc_skin_font_board_small,
			 "fill_color_rgba", color_rgba,
			 NULL);

  /* Display a background if a color is provided */
  if(color_background) {
    guint32 color_background_rgba = gc_skin_get_color(color_background);
    GooCanvasBounds bounds;
    int gap = 8;

    goo_canvas_item_get_bounds (item, &bounds);
    goo_canvas_rect_new (shape_root_item,
			 x - (bounds.x2 - bounds.x1)/2 - gap,
			 y - (bounds.y2 - bounds.y1)/2 - gap,
			 bounds.x2 - bounds.x1 + gap*2,
			 bounds.y2 - bounds.y1 + gap*2,
			 "stroke_color_rgba", 0xFFFFFFFFL,
			 "fill_color_rgba", color_background_rgba,
			 "line-width", (double) 2,
			 "radius-x", (double) 10,
			 "radius-y", (double) 10,
			 NULL);
  }

  goo_canvas_item_raise(item, NULL);
}

static Shape *
create_shape(ShapeType type, char *name, char *tooltip, char *pixmapfile,
	     char *targetfile, double x, double y,
	     double w, double h, double zoomx,
	     double zoomy, guint position, char *soundfile)
{
  Shape *shape;

  /* allocate a new shape */
  shape = g_new0(Shape,1);

  shape->name = g_strdup(name);
  if(tooltip)
    shape->tooltip = g_strdup(tooltip);
  else
    shape->tooltip = NULL;

  shape->pixmapfile = g_strdup(pixmapfile);
  shape->targetfile = g_strdup(targetfile);
  shape->x = x;
  shape->y = y;
  shape->w = w;
  shape->h = h;
  shape->zoomx = zoomx;
  shape->zoomy = zoomy;
  shape->position = position;
  shape->type = type;
  shape->soundfile = g_strdup(soundfile);

  if(type != SHAPE_ICON)
    shape->target_shape = shape;

  /* add the shape to the list */
  shape_list = g_list_append(shape_list, shape);

  return shape;
}

/** return a double value from an nodePtr
 *
 * \param node
 * \param prop
 * \param def_value :  default value
 *
 * \return a double value found in the 'prop' property of the 'node' or
 *         the default value if not found
 */
static double
xmlGetProp_Double(xmlNodePtr node, xmlChar *prop, double def_value)
{
  double value;
  char *str;

  str = (char *)xmlGetProp(node, prop);
  if(str)
    {
      value = g_ascii_strtod(str, NULL);
      free(str);
    }
  else
    value = def_value;

  return value;
}

static void
add_xml_shape_to_data(xmlDocPtr doc, xmlNodePtr xmlnode, GNode * child, GList **list)
{
  char *name,*ctype, *justification;
  char *tooltip;
  GtkAnchorType anchor_gtk;
  char *pixmapfile = NULL;
  char *targetfile = NULL;
  char *soundfile = NULL;
  double x, y, zoomx, zoomy;
  guint position;
  ShapeType type = SHAPE_TARGET;
  Shape *shape;
  xmlNodePtr xmlnamenode;
  char *locale;
  char *color_text;
  guint color_rgba;
  char *color_background = NULL;

  if(/* if the node has no name */
     !xmlnode->name ||
     /* or if the name is not "Shape" */
     ((g_strcasecmp((const char *)xmlnode->name,"Shape")!=0) &&
      /* or if the name is not "Title" */
      (g_strcasecmp((const char *)xmlnode->name,"Title")!=0) &&
      /* or if the name is not "Option" */
      (g_strcasecmp((const char *)xmlnode->name,"Option")!=0) )
     )
    return;

  pixmapfile = (char *)xmlGetProp(xmlnode, BAD_CAST "pixmapfile");

  targetfile = (char *)xmlGetProp(xmlnode, BAD_CAST "targetfile");

  soundfile = (char *)xmlGetProp(xmlnode, BAD_CAST "sound");

  /* get the X coord of the shape */
  x = xmlGetProp_Double(xmlnode, BAD_CAST "x", 100);
  /* get the Y coord of the shape */
  y = xmlGetProp_Double(xmlnode, BAD_CAST "y", 100);

  /* Back up the current locale to be sure to load well C formated numbers */
  locale = g_strdup(gc_locale_get());
  gc_locale_set("C");

  /* get the ZOOMX coord of the shape */
  zoomx = xmlGetProp_Double(xmlnode, BAD_CAST "zoomx", 1);

  /* get the ZOOMY coord of the shape */
  zoomy = xmlGetProp_Double(xmlnode, BAD_CAST "zoomy", 1);

  /* get the POSITION of the shape : DEPRECATED */
  /* Position in the xml means:
   * 0 = BOTTOM
   * 1 or more = TOP
   */
  position = 0;

  /* Back to the user locale */
  gc_locale_set(locale);
  g_free(locale);

  /* get the TYPE of the shape */
  ctype = (char *)xmlGetProp(xmlnode, BAD_CAST "type");
  if(ctype) {
    if(g_strcasecmp(ctype,"SHAPE_TARGET")==0)
      type = SHAPE_TARGET;
    else if(g_strcasecmp(ctype,"SHAPE_DUMMY_TARGET")==0)
      type = SHAPE_DUMMY_TARGET;
    else if (g_strcasecmp(ctype,"SHAPE_BACKGROUND")==0)
      type = SHAPE_BACKGROUND;
    xmlFree(ctype);
  }
  else
    type = SHAPE_TARGET;

  /* get the JUSTIFICATION of the Title */
  anchor_gtk = GTK_ANCHOR_CENTER;	/* GTK_ANCHOR_CENTER is default */
  justification = (char *)xmlGetProp(xmlnode, BAD_CAST "justification");
  if(justification) {
    if (strcmp(justification, "GTK_JUSTIFY_LEFT") == 0) {
      anchor_gtk = GTK_ANCHOR_WEST;
    } else if (strcmp(justification, "GTK_JUSTIFY_RIGHT") == 0) {
      anchor_gtk = GTK_ANCHOR_EAST;
    } else if (strcmp(justification, "GTK_JUSTIFY_CENTER") == 0) {
      anchor_gtk = GTK_ANCHOR_CENTER;
    } else {
    }
    xmlFree(justification);
  }

  /* get the COLOR of the Title Specified by skin reference */
  color_text = (char *)xmlGetProp(xmlnode, BAD_CAST "color_skin");
  if(color_text) {
    color_rgba = gc_skin_get_color(color_text);
    xmlFree(color_text);
  } else {
    color_rgba = gc_skin_get_color("gcompris/content");	/* the default */
  }

  /* get the COLOR of the Title Specified by skin reference */
  /* Default is NULL */
  color_background = (char *)xmlGetProp(xmlnode, BAD_CAST "color_background_skin");

  /* get the name and tooltip of the shape */
  name    = NULL;
  tooltip = NULL;

  xmlnamenode = xmlnode->xmlChildrenNode;
  while (xmlnamenode != NULL) {
    gchar *lang = (char *)xmlGetProp(xmlnamenode, BAD_CAST "lang");
    /* get the name of the shape */
    if (!strcmp((char *)xmlnamenode->name, "name")
	&& (lang==NULL
	    || !strcmp(lang, gc_locale_get())
	    || !strncmp(lang, gc_locale_get(), 2)))
      {
	if (name)
	  xmlFree(name);
	name = (char *)xmlNodeListGetString(doc, xmlnamenode->xmlChildrenNode, 1);
      }

    /* get the tooltip of the shape */
    if (!strcmp((char *)xmlnamenode->name, "tooltip")
	&& (lang==NULL
	    || !strcmp(lang, gc_locale_get())
	    || !strncmp(lang, gc_locale_get(), 2)))
      {
	if (tooltip)
	  xmlFree(tooltip);
	tooltip = (char *)xmlNodeListGetString(doc, xmlnamenode->xmlChildrenNode, 1);
      }
    xmlFree(lang);
    xmlnamenode = xmlnamenode->next;
  }

  /* If name is not given as an element, try to get it as a property */
  if(!name)
    name = (char *)xmlGetProp(xmlnode, BAD_CAST "name");

  if(g_strcasecmp((char *)xmlnode->name, "Shape")==0)
    {
      /* add the shape to the database */
      /* WARNING : I do not initialize the width and height since I don't need them */
      shape = create_shape(type, name, tooltip, pixmapfile,
			   targetfile, x, y,
			   0, 0,
			   zoomx, zoomy, position,
			   soundfile);

      /* add the shape to the list */
      *list = g_list_append(*list, shape);
    }
  else if (g_strcasecmp((char *)xmlnode->name, "Title")==0)
    {
      /* Read \n is needed */
      gchar *newname;

      if(name != NULL) {
	newname = g_strcompress(name);

	create_title(newname, x, y, anchor_gtk,
		     color_rgba, color_background);
	g_free(newname);
      }
    }
  g_free(pixmapfile);
  g_free(soundfile);
  g_free(name);
  g_free(targetfile);
  g_free(tooltip);
  xmlFree(color_background);
}

static void
insert_shape_random(GList *shapes_, int shapeMask)
{
  int list_length, i;

  GList *shapes = g_list_copy(shapes_);

  /* Insert each of the shapes randomly */
  while((list_length = g_list_length(shapes)))
    {
      Shape *shape;

      i = g_random_int_range(0, list_length);
      shape = g_list_nth_data(shapes, i);
      if (shape->type & shapeMask)
	add_shape_to_canvas(shape);

      shapes = g_list_remove (shapes, shape);
    }
  g_list_free(shapes);
}

/* parse the doc, add it to our internal structures and to the clist */
static void
parse_doc(xmlDocPtr doc)
{
  GList *shape_list_init = NULL;
  xmlNodePtr node;
  GooCanvasItem *item;

  /* find <Shape> nodes and add them to the list, this just
     loops through all the children of the root of the document */
  for(node = doc->children->children; node != NULL; node = node->next) {
    /* add the shape to the list, there are no children so
       we pass NULL as the node of the child */
    add_xml_shape_to_data(doc, node, NULL, &shape_list_init);
  }

  shape_list = g_list_copy(shape_list_init);
  insert_shape_random(shape_list_init, 0xFF ^ SHAPE_BACKGROUND);
  insert_shape_random(shape_list_init, SHAPE_BACKGROUND);
  g_list_free(shape_list_init);
  shape_list_init = NULL;

  if(current_shapelistgroup_index > 0)
    { /* If at least on shape group */
      item = g_list_nth_data(shape_list_group, current_shapelistgroup_index);
      g_object_set (item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      item = g_list_nth_data(shape_list_group, 0);
      g_object_set (item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
      g_object_set (previous_shapelist_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      g_object_set (next_shapelist_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
      current_shapelistgroup_index = 0;
    }
}



/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set */
static gboolean
read_xml_file(char *fname)
{
  /* pointer to the new doc */
  xmlDocPtr  doc;

  g_return_val_if_fail(fname!=NULL,FALSE);

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
     g_strcasecmp((char *)doc->children->name, "ShapeGame")!=0) {
    xmlFreeDoc(doc);
    return FALSE;
  }

  /*--------------------------------------------------*/
  /* Read ShapeBox property */
  shapeBox.x = xmlGetProp_Double(doc->children, BAD_CAST "shapebox_x", 15);

  shapeBox.y = xmlGetProp_Double(doc->children, BAD_CAST "shapebox_y", 25);

  shapeBox.w = xmlGetProp_Double(doc->children, BAD_CAST "shapebox_w", 80);

  shapeBox.h = xmlGetProp_Double(doc->children, BAD_CAST "shapebox_h", 430);

  shapeBox.nb_shape_x = xmlGetProp_Double(doc->children, BAD_CAST "shapebox_nb_shape_x", 1);

  shapeBox.nb_shape_y = xmlGetProp_Double(doc->children, BAD_CAST "shapebox_nb_shape_y", 5);

  /* Read shadow enable property */
  shadow_enable = xmlGetProp_Double(doc->children, BAD_CAST "shadow_enable", 1);

  /* parse our document and replace old data */
  parse_doc(doc);

  xmlFreeDoc(doc);

  return TRUE;
}

/* ************************************* */
/* *            Configuration          * */
/* ************************************* */


/* ======================= */
/* = config_start        = */
/* ======================= */

static GcomprisProfile *profile_conf;
static GcomprisBoard   *board_conf;

static void save_table (gpointer key,
			gpointer value,
			gpointer user_data)
{
  gc_db_set_board_conf ( profile_conf,
			 board_conf,
			 (gchar *) key,
			 (gchar *) value);
}

static void conf_ok(GHashTable *table)
{
  if (!table){
    if (gcomprisBoard)
      pause_board(FALSE);
    return;
  }

  g_hash_table_foreach(table, (GHFunc) save_table, NULL);

  if (gcomprisBoard){
    GHashTable *config;

    if (profile_conf)
      config = gc_db_get_board_conf();
    else
      config = table;

    if (strcmp(gcomprisBoard->name, "imagename")==0){
      gc_locale_set(g_hash_table_lookup( config, "locale"));
    }

    gchar *drag_mode_str = g_hash_table_lookup( config, "drag_mode");

    if (drag_mode_str && (strcmp (drag_mode_str, "NULL") != 0))
      drag_mode = (gint ) g_ascii_strtod(drag_mode_str, NULL);
    else
      drag_mode = 0;

    if (profile_conf)
      g_hash_table_destroy(config);

    gc_drag_change_mode( drag_mode);

    shapegame_next_level();

    pause_board(FALSE);
  }

  board_conf = NULL;
  profile_conf = NULL;

}

static void
config_start(GcomprisBoard *agcomprisBoard,
	     GcomprisProfile *aProfile)
{
  board_conf = agcomprisBoard;
  profile_conf = aProfile;

  if (gcomprisBoard)
    pause_board(TRUE);

  gchar * label = g_strdup_printf(_("<b>%1$s</b> configuration\n for profile <b>%2$s</b>"),
				  agcomprisBoard->name,
				  aProfile? aProfile->name : "");

  GcomprisBoardConf *bconf;
  bconf = \
    gc_board_config_window_display( label,
				    (GcomprisConfCallback )conf_ok);

  g_free(label);

  /* init the combo to previously saved value */
  GHashTable *config = gc_db_get_conf( profile_conf, board_conf);

  if (strcmp(agcomprisBoard->name, "imagename")==0){
    gchar *locale = g_hash_table_lookup( config, "locale");

    gc_board_config_combo_locales( bconf, locale);
  }

  gchar *drag_mode_str = g_hash_table_lookup( config, "drag_mode");
  gint drag_previous;

  if (drag_mode_str && (strcmp (drag_mode_str, "NULL") != 0))
    drag_previous = (gint ) g_ascii_strtod(drag_mode_str, NULL);
  else
    drag_previous = 0;

  gc_board_config_combo_drag(bconf, drag_mode);

}


/* ======================= */
/* = config_stop        = */
/* ======================= */
static void
config_stop()
{
}
