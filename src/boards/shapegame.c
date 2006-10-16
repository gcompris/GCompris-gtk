/* gcompris - shapegame.c
 *
 * Time-stamp: <2006/08/21 23:35:01 bruno>
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
#include <string.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

#define UNDEFINED "Undefined"
#define SQUARE_LIMIT_DISTANCE 50.0

static int gamewon;

static gboolean edit_mode = FALSE;

static gint addedname;	/* Defined the rules to apply to determine if the
			   board is done.
			   - by default it is puzzle like, each piece at its place
			   - If addedname is set, then this value is compared to the
			     sum of each xml name value of the placed pieces
			*/

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

#define POINT_COLOR_OFF 0xEf000080
#define POINT_COLOR_ON  0x00EF0080

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
  char  *tooltip;			/* optional tooltip for the shape */
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
  char  *soundfile;			/* relative sound file to be played when pressing mouse button */
  ShapeType type;			/* Type of shape */

  GnomeCanvasItem *item;     	  	/* Canvas item for this shape */
				  	/* Root index which this item is in the shapelist */
  GnomeCanvasGroup *shape_list_group_root;
  guint shapelistgroup_index;	  	/* Root index which this item is in the shapelist */
  Shape *icon_shape;			/* Temporary Canvas icon shape for this shape */
  Shape *target_shape;			/* If this is an icon shape then point to its shape */
  GnomeCanvasItem *bad_item;		/* Temporary bad placed Canvas item for this shape */

  gboolean found;			/* The user found this item */
  gboolean placed;			/* The user placed this item */
  GnomeCanvasItem *target_point;       	/* Target point item for this shape */
  GnomeCanvasItem *targetitem;       	/* Target item for this shape (if targetfile is defined) */
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
static GList *shape_list_init	= NULL;
static GList *shape_list	= NULL;
static GList *shape_list_group	= NULL;
static int current_shapelistgroup_index	= -1;

static GnomeCanvasItem *next_shapelist_item     = NULL;		/* Canvas item button for the next shapelist */
static GnomeCanvasItem *previous_shapelist_item = NULL;   	/* Canvas item button for the previous shapelist */

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

static GnomeCanvasItem	*shape_root_item;
static GnomeCanvasItem	*shape_list_root_item;

/* The tooltip */
static GnomeCanvasGroup	*tooltip_root_item;
static GnomeCanvasItem	*tooltip_text_item;
static GnomeCanvasItem	*tooltip_text_item_s;
static GnomeCanvasItem	*tooltip_bg_item;

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

static void              shapegame_init_canvas(GnomeCanvasGroup *parent);
static void 		 shapegame_destroy_all_items(void);
static void 		 setup_item(GnomeCanvasItem *item, Shape *shape);
static void 		 shapegame_next_level(void);
static gboolean 	 read_xml_file(char *fname);
static gboolean 	 write_xml_file(char *fname);
static Shape 		*find_closest_shape(double x, double y, double limit);
static Shape 		*create_shape(ShapeType type, char *name, char *tooltip,
				      char *pixmapfile,  GnomeCanvasPoints* points,
				      char *targetfile, double x, double y, double l, double h, double zoomx,
				      double zoomy, guint position, char *soundfile);
static gboolean 	 increment_sublevel(void);
static void 		 create_title(char *name, double x, double y, GtkJustification justification,
				      guint32 color_rgba);
static gint		 item_event_ok(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint		 item_event_edition(GnomeCanvasItem *item, GdkEvent *event, Shape *shape);

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
   process_ok,
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

  if (strcmp(agcomprisBoard->name, "imagename")==0){
    GHashTable *config = gc_db_get_board_conf();

    gc_locale_set(g_hash_table_lookup( config, "locale"));

    g_hash_table_destroy(config);
  }

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
							 gcomprisBoard->maxlevel++,
							 NULL)) )
	{
	  g_free(filename);

	}

      /**/
      gcomprisBoard->maxlevel--;

      if (strcmp(gcomprisBoard->name, "imagename")==0){
	gc_bar_set(GC_BAR_CONFIG|GC_BAR_LEVEL|GC_BAR_OK);
      } else
	gc_bar_set(GC_BAR_LEVEL|GC_BAR_OK);


      gcomprisBoard->sublevel = 0;

      /* In this board, the sublevels are dynamicaly discovered based on data files */
      gcomprisBoard->number_of_sublevel=G_MAXINT;


      if(gcomprisBoard->mode!=NULL)
	if(g_strncasecmp(gcomprisBoard->mode, "background=", 11)==0)
	  {
	    gchar *tmp = NULL;

	    tmp = g_malloc(strlen(gcomprisBoard->mode));
	    tmp = strcpy(tmp, gcomprisBoard->mode + 11);

	    gc_set_background(gnome_canvas_root(gcomprisBoard->canvas), tmp);
	    default_background = FALSE;
	    g_free(tmp);
	  }

      if(default_background)
	{
	  gchar *img;

	  // Default case, load the default background
	  img = gc_skin_image_get("gcompris-shapebg.jpg");
	  gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
				  img);
	  g_free(img);
	}

      shapegame_next_level();

      pause_board(FALSE);

      gc_cursor_set(GCOMPRIS_LINE_CURSOR);

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

  if (strcmp(gcomprisBoard->name, "imagename")==0){
    gc_locale_reset();
  }

  gcomprisBoard = NULL;
  gc_cursor_set(GCOMPRIS_DEFAULT_CURSOR);
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
	  BoardPlugin *bp_board = g_malloc0(sizeof(BoardPlugin));

	  bp_board->handle        = menu_bp.handle;
	  bp_board->filename      = menu_bp.filename;
	  bp_board->name          = menu_bp.name;
	  bp_board->description   = menu_bp.description;
	  bp_board->author        = menu_bp.author;
	  bp_board->init          = menu_bp.init;
	  bp_board->cleanup       = menu_bp.cleanup;
	  bp_board->about         = menu_bp.about;
	  bp_board->configure     = menu_bp.configure;
	  bp_board->start_board   = menu_bp.start_board;
	  bp_board->pause_board   = menu_bp.pause_board;
	  bp_board->end_board     = menu_bp.end_board;
	  bp_board->is_our_board  = menu_bp.is_our_board;
	  bp_board->key_press     = menu_bp.key_press;
	  bp_board->ok            = menu_bp.ok;
	  bp_board->set_level     = menu_bp.set_level;
	  bp_board->config        = menu_bp.config;
	  bp_board->repeat        = menu_bp.repeat;

	  if (strcmp(gcomprisBoard->name, "imagename")==0){
	    bp_board->config_start  = menu_bp.config_start;
	    bp_board->config_stop   = menu_bp.config_stop;
	  } else {
	    bp_board->config_start  = NULL;
	    bp_board->config_stop   = NULL;
	  }


	  /* Set the plugin entry */
	  gcomprisBoard->plugin = bp_board;

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
  guint c;

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
      process_ok();
      return TRUE;
    case GDK_Right:
    case GDK_Delete:
    case GDK_BackSpace:
    case GDK_Left:
      break;
    }

  c = tolower(keyval);

  switch (c)
    {
    case 'e':
      /* Enter Edit Mode */
      gc_dialog(_("You have entered Edit mode\nMove the puzzle items;\ntype 's' to save, and\n'd' to display all the shapes"), NULL);
      edit_mode = TRUE;
      break;
    case 's':
      /* Save the current board */
      if(edit_mode)
	{
	  write_xml_file("/tmp/gcompris-board.xml");
	  gc_dialog(_("The data from this activity are saved under\n/tmp/gcompris-board.xml"), NULL);
	}
      break;
    case 'd':
      /* Display all the shapes */
      if(edit_mode)
	{
	  GList *list;

	  /* loop through all our shapes */
	  for(list = shape_list; list != NULL; list = list->next) {
	    Shape *shape = list->data;

	    if(shape->type==SHAPE_TARGET)
	      {
		   /* You got it right perfect */
		   if(shape->bad_item!=NULL)
		     {
		       gnome_canvas_item_hide(shape->bad_item);
		       gtk_object_destroy (GTK_OBJECT(shape->bad_item));
		       shape->bad_item=NULL;
		     }
		   shape->found  = TRUE;
		   gnome_canvas_item_show(shape->item);
		   gnome_canvas_item_raise_to_top(shape->item);
		   gnome_canvas_item_raise_to_top(shape->target_point);
	      }
	  }
	}
      break;
    default:
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

    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
      gc_bonus_end_display(BOARD_FINISHED_RANDOM);
      return FALSE;
    }

  }

  return TRUE;
}

/* set initial values for the next level */
static void shapegame_next_level()
{
  char *filename;

  gamewon = FALSE;
  edit_mode = FALSE;

  shapegame_destroy_all_items();

  shapegame_init_canvas(gnome_canvas_root(gcomprisBoard->canvas));

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
	return;
    }

  gc_bar_set_level(gcomprisBoard);

  read_xml_file(filename);

  g_free(filename);
}


static void process_ok()
{
  GList *list;
  gboolean done = TRUE;

  /*
   * Here I implements the resolving rules.
   */
  if(addedname == INT_MAX)
    {
      /* - Default is to be puzzle like. Check that each piece is at its place */

      /* Loop through all the shapes to find if all target are found */
      for(list = shape_list; list != NULL; list = list->next) {
	Shape *shape = list->data;

	if(shape->type==SHAPE_TARGET)
	  {
	    if(shape->found==FALSE)
	      done=FALSE;
	  }
      }
    }
  else
    {
      /* - if addedname is set, then adding int name field of placed piece must
       *   equals addedname
       */
      gint total = 0;

      for(list = shape_list; list != NULL; list = list->next) {
	Shape *shape = list->data;
	gint   intname = 0;

	g_warning("   shape = %s\n", shape->name);
	if(shape->type==SHAPE_TARGET && shape->placed==TRUE)
	  {
	    intname = atoi(shape->name);
	    total += intname;
	    g_warning("      shape = %s   placed=TRUE\n", shape->name);
	  }

      }

      if(total != addedname)
	done = FALSE;

      g_warning("checking for addedname=%d done=%d total=%d\n", addedname, done, total);
    }


  if(done)
    {
      gamewon = TRUE;
      gc_bonus_display(gamewon, BONUS_FLOWER);
    }
  else
    {
      gc_bonus_display(gamewon, BONUS_FLOWER);
    }

}

static void destroy_shape (Shape *shape)
{
  g_free(shape->name);
  g_free(shape->pixmapfile);
  g_free(shape->targetfile);
  g_free(shape->soundfile);
  g_free(shape->tooltip);
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

  g_list_free(shape_list);

  if (shapelist_table)
    {
      /* Deleting the root item automatically deletes children items */
      gtk_object_destroy (GTK_OBJECT(shape_list_root_item));
      shape_list_root_item = NULL;
      gtk_object_destroy (GTK_OBJECT(shape_root_item));
      shape_root_item = NULL;
      gtk_object_destroy (GTK_OBJECT(tooltip_root_item));
      tooltip_root_item = NULL;

      g_hash_table_destroy (shapelist_table);
      shapelist_table=NULL;

      g_list_free(shape_list_group);
      shape_list_group = NULL;
      current_shapelistgroup_index = -1;
    }
}

static void shapegame_init_canvas(GnomeCanvasGroup *parent)
{
  GdkPixbuf       *pixmap = NULL;

  shape_list_root_item = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  shape_root_item = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_group_get_type (),
			   "x", (double)gcomprisBoard->width/SHAPE_BOX_WIDTH_RATIO,
			   "y", (double)0,
			   NULL);

  /* Create the tooltip area */
  pixmap = gc_skin_pixmap_load("button_large.png");
  tooltip_root_item = GNOME_CANVAS_GROUP(
					 gnome_canvas_item_new (parent,
								gnome_canvas_group_get_type (),
								"x", (double)10,
								"y", (double)gcomprisBoard->height-70,
								NULL)
					 );

  tooltip_bg_item = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(tooltip_root_item),
			   gnome_canvas_pixbuf_get_type (),
			   "pixbuf", pixmap,
			   "x", (double) 0,
			   "y", (double) 0,
			   NULL);
  gdk_pixbuf_unref(pixmap);

  tooltip_text_item_s = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(tooltip_root_item),
			   gnome_canvas_text_get_type (),
			   "text", "",
			   "font", gc_skin_font_board_small,
			   "x", (double)gdk_pixbuf_get_width(pixmap)/2 + 1.0,
			   "y", 24.0 + 1.0,
			   "anchor", GTK_ANCHOR_CENTER,
			   "justification", GTK_JUSTIFY_CENTER,
			   "fill_color_rgba", gc_skin_color_shadow,
			   NULL);
  tooltip_text_item = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(tooltip_root_item),
			   gnome_canvas_text_get_type (),
			   "text", "",
			   "font", gc_skin_font_board_small,
			   "x", (double)gdk_pixbuf_get_width(pixmap)/2,
			   "y", 24.0,
			   "anchor", GTK_ANCHOR_CENTER,
			   "justification", GTK_JUSTIFY_CENTER,
			   "fill_color_rgba", gc_skin_color_text_button,
			   NULL);

  /* Hide the tooltip */
  gnome_canvas_item_hide(GNOME_CANVAS_ITEM(tooltip_root_item));

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
  GnomeCanvasItem *item;
  GdkPixbuf   *pixmap = NULL;
  GnomeCanvasGroup *shape_list_group_root = NULL;
  double ICON_GAP    = 5.0;
  double ICON_HEIGHT = (double)(shapeBox.h / shapeBox.nb_shape_y) - ICON_GAP;
  double ICON_WIDTH  = (double)(shapeBox.w / shapeBox.nb_shape_x) - ICON_GAP;

  if(!shapelist_table)
    shapelist_table= g_hash_table_new (g_str_hash, g_str_equal);

  /*----------------------------------------------------------------------*/
  /* If the first list is full, add the previous/forward buttons          */
  if(g_hash_table_size(shapelist_table)==(shapeBox.nb_shape_x * shapeBox.nb_shape_y))
    {
      pixmap = gc_skin_pixmap_load("button_backward.png");
      previous_shapelist_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_list_root_item),
						       gnome_canvas_pixbuf_get_type (),
						       "pixbuf", pixmap,
						       "x", (double) shapeBox.x + (shapeBox.w/2) -
						       gdk_pixbuf_get_width(pixmap) - 2,
						       "y", (double) shapeBox.y + shapeBox.h,
						       NULL);

      gtk_signal_connect(GTK_OBJECT(previous_shapelist_item), "event",
			 (GtkSignalFunc) item_event_ok,
			 "previous_shapelist");
      gtk_signal_connect(GTK_OBJECT(previous_shapelist_item), "event",
			 (GtkSignalFunc) gc_item_focus_event,
			 NULL);
      gdk_pixbuf_unref(pixmap);

      pixmap = gc_skin_pixmap_load("button_forward.png");
      next_shapelist_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_list_root_item),
						   gnome_canvas_pixbuf_get_type (),
						   "pixbuf", pixmap,
						   "x", (double) shapeBox.x + (shapeBox.w/2) + 2,
						   "y", (double) shapeBox.y + shapeBox.h,
						   NULL);

      gtk_signal_connect(GTK_OBJECT(next_shapelist_item), "event",
			 (GtkSignalFunc) item_event_ok,
			 "next_shapelist");
      gtk_signal_connect(GTK_OBJECT(next_shapelist_item), "event",
			 (GtkSignalFunc) gc_item_focus_event,
			 NULL);
      gdk_pixbuf_unref(pixmap);
      gnome_canvas_item_hide(next_shapelist_item);

  }

  /*----------------------------------------------------------------------*/
  /* Do We need to create a new list                                      */
  if(g_hash_table_size(shapelist_table)%(shapeBox.nb_shape_x * shapeBox.nb_shape_y)==0)
    {
      current_shapelistgroup_index++;
      g_warning("Creation of the group of shape current_shapelistgroup_index=%d\n",
	     current_shapelistgroup_index);

      // Hide the previous group
      if(current_shapelistgroup_index>=1)
	{
	  g_warning(" Hide previous group\n");
	  shape_list_group_root = GNOME_CANVAS_GROUP(g_list_nth_data(shape_list_group,
								     current_shapelistgroup_index-1));
	  //gnome_canvas_item_hide(shape_list_group_root);
	  item = g_list_nth_data(shape_list_group, current_shapelistgroup_index-1);
	  gnome_canvas_item_hide(item);
	}

      // We need to start a new shape list group
      shape_list_group_root = \
	GNOME_CANVAS_GROUP(gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_list_root_item),
						 gnome_canvas_group_get_type (),
						 "x", (double)0,
						 "y", (double)0,
						 NULL));

      shape_list_group = g_list_append (shape_list_group, shape_list_group_root);
      g_warning(" current_shapelistgroup_index=%d\n", current_shapelistgroup_index);

    }
  else
    {
      // Get the current shapelist group
      g_warning(" get the current_shapelistgroup_index=%d\n", current_shapelistgroup_index);
      shape_list_group_root = g_list_nth_data(shape_list_group, current_shapelistgroup_index);
    }

  /*----------------------------------------------------------------------*/
  /* This pixmap is not yet in the list of shapes                         */
  if(g_hash_table_lookup (shapelist_table, shape->pixmapfile)==NULL)
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
      g_warning("  ICON_WIDTH = %f   ICON_HEIGHT = %f\n", ICON_WIDTH, ICON_HEIGHT);
      g_warning("x_offset = %f   y_offset = %f\n", x_offset, y_offset);

      /* So this shape is not yet in, let's put it in now */
      g_hash_table_insert (shapelist_table, shape->pixmapfile, shape);

      if(strcmp(shape->pixmapfile, UNDEFINED)!=0)
	{
	  pixmap = gc_pixmap_load(shape->pixmapfile);
	  if(pixmap)
	    {
	      double w, h;
	      Shape *icon_shape;

	      /* Calc a zoom factor so that the shape will fit in the shapelist
		 whatever its current size */
	      w = ICON_WIDTH;
	      h = gdk_pixbuf_get_height(pixmap) * (w / gdk_pixbuf_get_width(pixmap));

	      if(h > ICON_HEIGHT)
		{
		  h = ICON_HEIGHT;
		  w = gdk_pixbuf_get_width(pixmap) * ( h / gdk_pixbuf_get_height(pixmap));
		}

	      item = gnome_canvas_item_new (shape_list_group_root,
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", pixmap,
					    "x", (double)x_offset-w/2,
					    "y", (double)y_offset-h/2,
					    "width", (double) w,
					    "height", (double) h,
					    "width_set", TRUE,
					    "height_set", TRUE,
					    NULL);
	      gdk_pixbuf_unref(pixmap);

	      icon_shape = create_shape(SHAPE_ICON, shape->name, shape->tooltip,
					shape->pixmapfile, shape->points, shape->targetfile,
					(double)0, (double)y_offset,
					(double)w, (double)h,
					(double)shape->zoomx, (double)shape->zoomy,
					0, shape->soundfile);
	      icon_shape->item = item;
	      icon_shape->target_shape = shape;
	      icon_shape->shapelistgroup_index = current_shapelistgroup_index;
	      shape->shapelistgroup_index = current_shapelistgroup_index;
	      g_warning(" creation shape=%s shape->shapelistgroup_index=%d current_shapelistgroup_index=%d\n",
		     shape->name,
		     shape->shapelistgroup_index, current_shapelistgroup_index);
	      icon_shape->shape_list_group_root = shape_list_group_root;
	      setup_item(item, icon_shape);
	      gtk_signal_connect(GTK_OBJECT(item), "event",
				 (GtkSignalFunc) gc_item_focus_event,
				 NULL);
	    }
	}
    }
}

/*
 * Find the closest shape from the given point if it is located at
 * a distance under the given square limit.
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
	dist = sqrt(pow((shape->x-x),2) + pow((shape->y-y),2));
	if(dist<goodDist)
	  {
	    goodDist=dist;
	    candidateShape=shape;
	  }
      }
  }

  return candidateShape;
}

//#if 0
static void dump_shape(Shape *shape)
{
  g_warning("dump_shape name=%s found=%d type=%d ", shape->name, shape->found, shape->type);
  if(shape->bad_item)
    g_warning("bad_item=TRUE ");
  if(shape->icon_shape)
    g_warning("icon_shape=%s", shape->icon_shape->name);
  g_warning("\n");
}
//#endif

/*
 * Given the shape, if it has an icon, it puts it back to
 * the list of shapes
 */
static void shape_goes_back_to_list(Shape *shape, GnomeCanvasItem *item)
{
  g_warning("shape_goes_back_to_list shape=%s shape->shapelistgroup_index=%d current_shapelistgroup_index=%d\n", shape->name, shape->shapelistgroup_index, current_shapelistgroup_index);

  if(shape->icon_shape!=NULL)
    {
      if(shape->icon_shape->target_shape)
	{
	  shape->icon_shape->target_shape->placed = FALSE;
	  g_warning("shape_goes_back_to_list setting shape->name=%s to placed=%d\n",
		 shape->icon_shape->target_shape->name,
		 shape->icon_shape->target_shape->placed);
	}

      /* There was a previous icon here, put it back to the list */
      gnome_canvas_item_move(shape->icon_shape->item,
			     shape->icon_shape->x - shape->x,
			     shape->icon_shape->y - shape->y);
      gnome_canvas_item_show(shape->icon_shape->item);

      gc_item_focus_set(shape->icon_shape->item, TRUE);
      shape->icon_shape=NULL;

      gnome_canvas_item_hide(item);
      gc_sound_play_ogg ("sounds/gobble.ogg", NULL);
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
   static GnomeCanvasItem *target_point_previous;

  if(!gcomprisBoard)
    return FALSE;

  if(board_paused)
    return FALSE;


   if(shape==NULL) {
     g_warning("Shape is NULL : Should not happen");
     return FALSE;
   }

   /* This event is a non sense in the edit mode. Also, it will crash since the data structure are */
   /* filled differently in edit mode                                                              */
   /* Redirect this event to the edit mode one so that the user can drag the object directly       */
   if(edit_mode)
     {
       item_event_edition(item, event, shape);
       return FALSE;
     }

   item_x = event->button.x;
   item_y = event->button.y;
   gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

   switch (event->type)
     {
     case GDK_ENTER_NOTIFY:
       if(shape->tooltip && shape->type == SHAPE_ICON) {
	 gnome_canvas_item_raise_to_top(GNOME_CANVAS_ITEM(tooltip_root_item));
	 /* WARNING: This should not be needed but if I don't do it, it's not refreshed */
	 gnome_canvas_item_set(GNOME_CANVAS_ITEM(tooltip_bg_item),
			       "y", 0.0,
			       NULL);
	 gnome_canvas_item_set(GNOME_CANVAS_ITEM(tooltip_text_item_s),
			       "text", shape->tooltip,
			       NULL);
	 gnome_canvas_item_set(GNOME_CANVAS_ITEM(tooltip_text_item),
			       "text", shape->tooltip,
			       NULL);
	 gnome_canvas_item_show(GNOME_CANVAS_ITEM(tooltip_root_item));
       }
       break;
     case GDK_LEAVE_NOTIFY:
       if(shape->tooltip && shape->type == SHAPE_ICON)
       	 gnome_canvas_item_hide(GNOME_CANVAS_ITEM(tooltip_root_item));
       break;
     case GDK_BUTTON_PRESS:
       switch(event->button.button)
         {
         case 1:
           if (event->button.state & GDK_SHIFT_MASK)
             {
	     }
	   else
	     {
	       target_point_previous = NULL;

	       x = item_x;
	       y = item_y;

	       item_x = shape->x;
	       item_y = shape->y;

	       switch(shape->type)
		 {
		 case SHAPE_TARGET:
		   gnome_canvas_item_hide(GNOME_CANVAS_ITEM(item));
		   gc_item_focus_set(item, FALSE);

		   if( shape->icon_shape!=NULL)
		     {
		       item = shape->icon_shape->item;
		       item_x = x - (x - shape->x) * shape->icon_shape->w /
			  shape->w;
		       item_y = y - (y - shape->y) * shape->icon_shape->h /
			  shape->h;
		       gnome_canvas_item_move( item,
					       item_x - shape->x,
					       item_y - shape->y );
		       gnome_canvas_item_show( item );
		       gc_item_focus_set(item, TRUE);
		       shape->icon_shape=NULL;
		     }
		   break;
		 case SHAPE_ICON:
		   if (strcmp(shape->soundfile,UNDEFINED) != 0)
		   {
		     /* If the soundfile has space ' ' in it, then it is assumed that it is a list
		      * of sound rather than a single one
		      */

		     char *p = NULL;
		     char *soundfile = g_strdup(shape->soundfile);

		     while ((p = strstr (soundfile, " ")))
		       {
			 *p='\0';
			 gc_sound_play_ogg(soundfile, NULL);
			 soundfile=p+1;
			 g_warning("soundfile = %s\n", soundfile);
		       }

		     gc_sound_play_ogg(soundfile, NULL);

		   }
		   break;
		 default:
		   break;
		 }
	       /* This records the offset between the mouse pointer and the grabbed item center */
	       offset_x = x - item_x;
	       offset_y = y - item_y;
	       g_warning("offsetx=%f offsetx=%f\n", offset_x, offset_y);
	       if(item==NULL)
		 return FALSE;

	       fleur = gdk_cursor_new(GDK_FLEUR);

	       /* Make sure this item is on top */
	       gnome_canvas_item_raise_to_top(shape_list_root_item);
	       gnome_canvas_item_raise_to_top(item);

	       gc_canvas_item_grab(item,
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
	   /* Mark it not found */
	   shape->found = FALSE;
	   break;

         default:
           break;
         }
       break;

     case GDK_MOTION_NOTIFY:
       if (dragging && (event->motion.state & GDK_BUTTON1_MASK))
         {
	   Shape *targetshape = NULL;
	   new_x = item_x;
	   new_y = item_y;

	   gnome_canvas_item_move(item, new_x - x, new_y - y);

	   x = new_x;
	   y = new_y;

	   targetshape = find_closest_shape(item_x - offset_x,
					    item_y - offset_y,
					    SQUARE_LIMIT_DISTANCE);
	   if(targetshape!=NULL)
	     {
	       if(target_point_previous)
		 {
		   if(strcmp(shape->targetfile, UNDEFINED)==0)
		     {
		       gnome_canvas_item_set(GNOME_CANVAS_ITEM(target_point_previous),
					     "fill_color_rgba", POINT_COLOR_OFF,
					     NULL);

		       gnome_canvas_item_set(GNOME_CANVAS_ITEM(targetshape->target_point),
					     "fill_color_rgba", POINT_COLOR_ON,
					     NULL);
		     }
		   else
		     {
		       gc_item_focus_set(target_point_previous, FALSE);
		       gc_item_focus_set(targetshape->targetitem, TRUE);
		       target_point_previous = targetshape->targetitem;
		     }
		 }

	       if(strcmp(shape->targetfile, UNDEFINED)==0)
		 target_point_previous = targetshape->target_point;
	       else
		 target_point_previous = targetshape->targetitem;
	     }
	 }
       break;

     case GDK_BUTTON_RELEASE:
       if(dragging)
	 {
	   Shape *targetshape = NULL;

	   gc_canvas_item_ungrab(item, event->button.time);
	   dragging = FALSE;

	   if(target_point_previous)
	     {
	       if(strcmp(shape->targetfile, UNDEFINED)==0)
		 gnome_canvas_item_set(GNOME_CANVAS_ITEM(target_point_previous),
				       "fill_color_rgba", POINT_COLOR_OFF,
				       NULL);
	       else
		 gc_item_focus_set(target_point_previous, FALSE);
	     }
	   target_point_previous = NULL;

	   targetshape = find_closest_shape(item_x - offset_x,
					    item_y - offset_y,
					    SQUARE_LIMIT_DISTANCE);
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
		   targetshape->found  = TRUE;
		   shape->target_shape->placed = TRUE;
		   g_warning("setting shape->name=%s to placed=%d\n", shape->target_shape->name,
			  shape->target_shape->placed);
		   gnome_canvas_item_show(targetshape->item);
		   gnome_canvas_item_raise_to_top(targetshape->item);
		 }
	       else
		 {
		   /* Oups wrong position, create the wrong pixmap item */
		   GdkPixbuf *pixmap;
		   GnomeCanvasItem *item;

		   targetshape->found = FALSE;
		   shape->target_shape->placed = TRUE;
		   g_warning("setting shape->name=%s to placed=%d\n", shape->target_shape->name,
			  shape->target_shape->placed);
		   gnome_canvas_item_hide(targetshape->item);

		   /* There is already a bad item, delete it */
		   if(targetshape->bad_item!=NULL)
		     gtk_object_destroy (GTK_OBJECT(targetshape->bad_item));

		   pixmap = gc_pixmap_load(shape->pixmapfile);
		   item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
						 gnome_canvas_pixbuf_get_type (),
						 "pixbuf", pixmap,
						 "x", (double)targetshape->x - (gdk_pixbuf_get_width(pixmap)
										* shape->zoomx)/2,
						 "y", (double)targetshape->y - (gdk_pixbuf_get_height(pixmap)
										* shape->zoomy/2),
						 "width", (double) gdk_pixbuf_get_width(pixmap) * shape->zoomx,
						 "height", (double) gdk_pixbuf_get_height(pixmap) * shape->zoomy,
						 "width_set", TRUE,
						 "height_set", TRUE,
						 NULL);
		   gdk_pixbuf_unref(pixmap);

		   targetshape->bad_item = item;
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
	       shape->target_shape->placed = FALSE;
	       /* Mark it not found */
	       shape->target_shape->found = FALSE;
	       dump_shape(shape);
	       dump_shape(shape->target_shape);
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

  if(!gcomprisBoard)
    return FALSE;

  if(!edit_mode)
    return FALSE;

   if(shape==NULL) {
     g_warning("Shape is NULL : Should not happen");
     return FALSE;
   }

   if(shape->type != SHAPE_TARGET)
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

	       gc_canvas_item_grab(item,
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

	   gc_canvas_item_ungrab(item, event->button.time);
	   gnome_canvas_item_raise_to_top(item);
	   dragging = FALSE;

	 }
       break;

     default:
       break;
     }

   return FALSE;

}

/* Callback for the operations */
static gint
item_event_ok(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  GnomeCanvasItem	*root_item;

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      root_item = g_list_nth_data(shape_list_group, current_shapelistgroup_index);
      gnome_canvas_item_hide(root_item);

      g_warning(" item event current_shapelistgroup_index=%d\n", current_shapelistgroup_index);
      if(!strcmp((char *)data, "previous_shapelist"))
	{

	  if(current_shapelistgroup_index>0)
	    {
	      current_shapelistgroup_index--;
	    }

	  if(current_shapelistgroup_index == 0)
	    {
	      gc_item_focus_set(item, TRUE);
	      gnome_canvas_item_hide(previous_shapelist_item);
	    }

	  gnome_canvas_item_show(next_shapelist_item);

	}
      else if(!strcmp((char *)data, "next_shapelist"))
	{
	  if(current_shapelistgroup_index<g_list_length(shape_list_group)-1)
	    {
	      current_shapelistgroup_index++;
	    }

	  if(current_shapelistgroup_index == g_list_length(shape_list_group)-1)
	    {
	      gc_item_focus_set(item, TRUE);
	      gnome_canvas_item_hide(next_shapelist_item);
	    }

	  gnome_canvas_item_show(previous_shapelist_item);

	}

      root_item = g_list_nth_data(shape_list_group, current_shapelistgroup_index);
      gnome_canvas_item_show(root_item);

      /* FIXME : Workaround for bugged canvas */
      //      gnome_canvas_update_now(gcomprisBoard->canvas);

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
	  targetpixmap = gc_pixmap_load(shape->targetfile);
	  shape->w = (double)gdk_pixbuf_get_width(targetpixmap) * shape->zoomx;
	  shape->h = (double)gdk_pixbuf_get_height(targetpixmap) *shape->zoomy;

	  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
					gnome_canvas_pixbuf_get_type (),
					"pixbuf", targetpixmap,
					"x", shape->x - shape->w / 2,
					"y", shape->y - shape->h / 2,
					"width",  shape->w,
					"height", shape->h,
					"width_set", TRUE,
					"height_set", TRUE,
					NULL);
	  shape->targetitem = item;
	  gdk_pixbuf_unref(targetpixmap);
	}
      else
	{
	  int point_size = 6;

	  /* Display a point to highlight the target location of this shape */
	  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
					gnome_canvas_ellipse_get_type(),
					"x1", (double)shape->x-point_size,
					"y1", (double)shape->y-point_size,
					"x2", (double)shape->x+point_size,
					"y2", (double)shape->y+point_size,
					"fill_color_rgba", POINT_COLOR_OFF,
					"outline_color", "black",
					"width_pixels", 2,
					NULL);
	  shape->target_point = item;
	  gtk_signal_connect(GTK_OBJECT(item), "event",
			     (GtkSignalFunc) item_event_edition,
			     shape);
	}
      gnome_canvas_item_lower_to_bottom(item);
    }

  if(shape->points!=NULL)
    {
      g_warning("it's a point \n");
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
      g_warning("it's an image ? shape->pixmapfile=%s\n", shape->pixmapfile);
      if(strcmp(shape->pixmapfile, UNDEFINED)!=0)
	{
	  g_warning("  Yes it is an image \n");
	  pixmap = gc_pixmap_load(shape->pixmapfile);
	  if(pixmap)
	    {
	      shape->w = (double)gdk_pixbuf_get_width(pixmap) * shape->zoomx;
	      shape->h = (double)gdk_pixbuf_get_height(pixmap) * shape->zoomy;

	      /* Display the shape itself but hide it until the user puts the right shape on it */
	      /* I have to do it this way for the positionning (lower/raise) complexity          */
	      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", pixmap,
					    "x", shape->x - shape->w / 2,
					    "y", shape->y - shape->h / 2,
					    "width", shape->w,
					    "height", shape->h,
					    "width_set", TRUE,
					    "height_set", TRUE,
					    NULL);
	      gdk_pixbuf_unref(pixmap);
	    }
	}
    }

  /* Associate this item to this shape */
  shape->item = item;

  if(shape->type==SHAPE_TARGET || shape->type==SHAPE_DUMMY_TARGET)
    {
      setup_item(item, shape);

      gnome_canvas_item_hide(item);
      add_shape_to_list_of_shapes(shape);
    }
  else if(shape->type==SHAPE_BACKGROUND)
    {
      gnome_canvas_item_lower_to_bottom(item);
    }

}

static void create_title(char *name, double x, double y, GtkJustification justification,
			 guint32 color_rgba)
{
  GnomeCanvasItem *item;

  /* Shadow */
  item = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
			   gnome_canvas_text_get_type (),
			   "text", name,
			   "font", gc_skin_font_board_medium,
			   "x", x + 1.0,
			   "y", y + 1.0,
			   "anchor", GTK_ANCHOR_CENTER,
			   "justification", justification,
			   "fill_color_rgba", gc_skin_color_shadow,
			   NULL);

  gnome_canvas_item_raise_to_top(item);

  item = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
			   gnome_canvas_text_get_type (),
			   "text", name,
			   "font", gc_skin_font_board_medium,
			   "x", x,
			   "y", y,
			   "anchor", GTK_ANCHOR_CENTER,
			   "justification", justification,
			   "fill_color_rgba", color_rgba,
			   NULL);

  gnome_canvas_item_raise_to_top(item);
}

static Shape *
create_shape(ShapeType type, char *name, char *tooltip, char *pixmapfile, GnomeCanvasPoints* points,
	     char *targetfile, double x, double y,
	     double w, double h, double zoomx,
	     double zoomy, guint position, char *soundfile)
{
  Shape *shape;

  /* allocate a new shape */
  shape = g_new(Shape,1);

  shape->name = g_strdup(name);
  if(tooltip)
    shape->tooltip = g_strdup(tooltip);
  else
    shape->tooltip = NULL;

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
  shape->soundfile = g_strdup(soundfile);

  shape->bad_item = NULL;
  shape->icon_shape = NULL;
  shape->target_shape = NULL;

  shape->found  = FALSE;
  shape->placed = FALSE;

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
add_xml_shape_to_data(xmlDocPtr doc, xmlNodePtr xmlnode, GNode * child)
{
  char *name, *cd ,*ctype, *justification;
  char *tooltip;
  GtkJustification justification_gtk;
  char *pixmapfile = NULL;
  char *targetfile = NULL;
  char *soundfile = NULL;
  double x, y, zoomx, zoomy;
  GnomeCanvasPoints* points = NULL;
  gchar **d;
  gint i, j;
  guint position;
  ShapeType type = SHAPE_TARGET;
  Shape *shape;
  xmlNodePtr xmlnamenode;
  char *locale;
  char *color_text;
  guint color_rgba;

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
  if(pixmapfile) {
    /* If the pixmapfile starts with skin: then get the skin relative image instead */
    if(!strncmp(pixmapfile, "skin:", 5)) {
      gchar *oldpixmapfile = pixmapfile;
      pixmapfile = gc_skin_image_get(oldpixmapfile+5);
      g_free(oldpixmapfile);
    }
  }

  targetfile = (char *)xmlGetProp(xmlnode, BAD_CAST "targetfile");

  soundfile = (char *)xmlGetProp(xmlnode, BAD_CAST "sound");
  /*********************************/
  /* get the points for a polygone */
  /* The list of points is similar to the one define in the SVG standard */
  /* FIXME : The implementation is incomplete, a point still needs to be added
     to shapelist and add management for it's x/y coordinates */
  cd = (char *)xmlGetProp(xmlnode, BAD_CAST "points");
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
	  points->coords[i] = g_ascii_strtod(d[i], NULL);
	}
      g_strfreev(d);
    }

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

  /* get the POSITION of the shape */
  /* Position in the xml means:
   * 0 = BOTTOM
   * 1 or more = TOP
   */
  position = (guint) xmlGetProp_Double(xmlnode, BAD_CAST "position", 0);

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
      else if (g_strcasecmp(ctype,"SHAPE_COLORLIST")==0)
        type = SHAPE_COLORLIST;
   xmlFree(ctype);
  }
  else
      type = SHAPE_TARGET;

  /* get the JUSTIFICATION of the Title */
  justification_gtk = GTK_JUSTIFY_CENTER;	/* GTK_JUSTIFICATION_CENTER is default */
  justification = (char *)xmlGetProp(xmlnode, BAD_CAST "justification");
  if(justification) {
    if (strcmp(justification, "GTK_JUSTIFY_LEFT") == 0) {
      justification_gtk = GTK_JUSTIFY_LEFT;
    } else if (strcmp(justification, "GTK_JUSTIFY_RIGHT") == 0) {
      justification_gtk = GTK_JUSTIFY_RIGHT;
    } else if (strcmp(justification, "GTK_JUSTIFY_CENTER") == 0) {
      justification_gtk = GTK_JUSTIFY_CENTER;
    } else if (strcmp(justification, "GTK_JUSTIFY_FILL") == 0) {
      justification_gtk = GTK_JUSTIFY_FILL;
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
      shape = create_shape(type, name, tooltip, pixmapfile ? pixmapfile : UNDEFINED, points,
                targetfile ? targetfile : UNDEFINED, x, y,
			   (double)0, (double)0,
			   zoomx, zoomy, position,
               soundfile ? soundfile : UNDEFINED);

      /* add the shape to the list */
      shape_list_init = g_list_append(shape_list_init, shape);
    }
  else if (g_strcasecmp((char *)xmlnode->name, "Title")==0)
    {
      /* Readd \n is needed */
      gchar *newname;

      if(name != NULL) {
	newname = g_strcompress(name);

	create_title(newname, x, y, justification_gtk, color_rgba);
    g_free(newname);
      }
    }
  g_free(pixmapfile);
  g_free(soundfile);
  g_free(name);
  g_free(targetfile);
  g_free(tooltip);
}

/* parse the doc, add it to our internal structures and to the clist */
static void
parse_doc(xmlDocPtr doc)
{
  xmlNodePtr node;
  GList *list;
  GnomeCanvasItem *item;

  /* find <Shape> nodes and add them to the list, this just
     loops through all the children of the root of the document */
  for(node = doc->children->children; node != NULL; node = node->next) {
    /* add the shape to the list, there are no children so
       we pass NULL as the node of the child */
    add_xml_shape_to_data(doc, node, NULL);
  }

  shape_list = g_list_copy(shape_list_init);

  /* Insert each of the shapes randomly */
  while(g_list_length(shape_list_init)>0)
    {
      Shape *shape;

      shape = g_list_nth_data(shape_list_init, RAND(0, (g_list_length(shape_list_init)-1)));
      add_shape_to_canvas(shape);

      shape_list_init = g_list_remove (shape_list_init, shape);
    }

  g_list_free(shape_list_init);
  shape_list_init = NULL;

  if(current_shapelistgroup_index>0) { /* If at least on shape group */
    item = g_list_nth_data(shape_list_group, current_shapelistgroup_index);
    gnome_canvas_item_hide(item);
    item = g_list_nth_data(shape_list_group, 0);
    gnome_canvas_item_show(item);
    gnome_canvas_item_hide(previous_shapelist_item);
    gnome_canvas_item_show(next_shapelist_item);
    current_shapelistgroup_index = 0;
  }

  /* Loop through all the shapes and */
  /* Arrange the order (depth) of the shapes on the canvas */
  /* Depending on the xml given definition in the position property */
  for(list = shape_list; list != NULL; list = list->next) {
    Shape *shape = list->data;

    gnome_canvas_item_lower_to_bottom(shape->item);
    if(shape->position>=1)
      gnome_canvas_item_raise(shape->item, ABS(shape->position));
  }
}



/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set */
static gboolean
read_xml_file(char *fname)
{
  /* pointer to the new doc */
  xmlDocPtr  doc;
  gchar     *tmpstr;

  g_return_val_if_fail(fname!=NULL,FALSE);

  /* parse the new file and put the result into newdoc */
  doc = gc_net_load_xml(fname);
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
  /* Read OkIfAddedName property */
  tmpstr = (char *)xmlGetProp(doc->children, BAD_CAST "OkIfAddedName");
  /* if unspecified, make it INT_MAX */
  if(!tmpstr)
      addedname = INT_MAX;
  else
      addedname = atoi(tmpstr);
  g_warning("addedname=%d\n", addedname);

  /*--------------------------------------------------*/
  /* Read ShapeBox property */
  tmpstr = (char *)xmlGetProp(doc->children, BAD_CAST "shapebox_x");
  /* if unspecified, use the default value */
  if(!tmpstr)
      shapeBox.x = 15;
  else
    shapeBox.x = g_ascii_strtod(tmpstr, NULL);
  g_warning("shapeBox.x=%f\n", shapeBox.x);

  tmpstr = (char *)xmlGetProp(doc->children, BAD_CAST "shapebox_y");
  /* if unspecified, use the default value */
  if(!tmpstr)
      shapeBox.y = 25;
  else
    shapeBox.y = g_ascii_strtod(tmpstr, NULL);
  g_warning("shapeBox.y=%f\n", shapeBox.y);

  tmpstr = (char *)xmlGetProp(doc->children, BAD_CAST "shapebox_w");
  /* if unspecified, use the default value */
  if(!tmpstr)
      shapeBox.w = 80;
  else
    shapeBox.w = g_ascii_strtod(tmpstr, NULL);
  g_warning("shapeBox.w=%f\n", shapeBox.w);

  tmpstr = (char *)xmlGetProp(doc->children, BAD_CAST "shapebox_h");
  /* if unspecified, use the default value */
  if(!tmpstr)
      shapeBox.h = 430;
  else
    shapeBox.h = g_ascii_strtod(tmpstr, NULL);
  g_warning("shapeBox.h=%f\n", shapeBox.h);

  tmpstr = (char *)xmlGetProp(doc->children, BAD_CAST "shapebox_nb_shape_x");
  /* if unspecified, use the default value */
  if(!tmpstr)
      shapeBox.nb_shape_x = 1;
  else
      shapeBox.nb_shape_x = atoi(tmpstr);
  g_warning("shapeBox.nb_shape_x=%d\n", shapeBox.nb_shape_x);

  tmpstr = (char *)xmlGetProp(doc->children, BAD_CAST "shapebox_nb_shape_y");
  /* if unspecified, use the default value */
  if(!tmpstr)
      shapeBox.nb_shape_y = 5;
  else
      shapeBox.nb_shape_y = atoi(tmpstr);
  g_warning("shapeBox.nb_shape_y=%d\n", shapeBox.nb_shape_y);


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
  newxml = xmlNewChild(xmlnode,NULL, BAD_CAST "Shape",NULL);
  /* set properties on it */
  xmlSetProp(newxml, BAD_CAST "name", BAD_CAST shape->name);
  if(shape->tooltip)
    xmlSetProp(newxml, BAD_CAST "tooltip", BAD_CAST shape->tooltip);
  xmlSetProp(newxml, BAD_CAST "pixmapfile", BAD_CAST shape->pixmapfile);
  xmlSetProp(newxml, BAD_CAST "sound", BAD_CAST shape->soundfile);

  tmp = g_strdup_printf("%f", shape->x);
  xmlSetProp(newxml, BAD_CAST "x", BAD_CAST tmp);
  g_free(tmp);

  tmp = g_strdup_printf("%f", shape->y);
  xmlSetProp(newxml, BAD_CAST "y", BAD_CAST tmp);
  g_free(tmp);

  tmp = g_strdup_printf("%f", shape->zoomx);
  xmlSetProp(newxml, BAD_CAST "zoomx", BAD_CAST tmp);
  g_free(tmp);

  tmp = g_strdup_printf("%f", shape->zoomy);
  xmlSetProp(newxml, BAD_CAST "zoomy", BAD_CAST tmp);
  g_free(tmp);

  tmp = g_strdup_printf("%d", shape->position);
  xmlSetProp(newxml, BAD_CAST "position", BAD_CAST tmp);
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
  doc = xmlNewDoc( BAD_CAST "1.0");
  /* create a new root node "ShapeGame" */
  doc->children = xmlNewDocNode(doc, NULL, BAD_CAST "ShapeGame", NULL);

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

  if ((gcomprisBoard) && (strcmp(gcomprisBoard->name, "imagename")==0)){
    GHashTable *config;

    if (profile_conf)
      config = gc_db_get_board_conf();
    else
      config = table;

    gc_locale_reset();

    gc_locale_set(g_hash_table_lookup( config, "locale"));

    if (profile_conf)
      g_hash_table_destroy(config);

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

  gchar * label = g_strdup_printf("<b>%s</b> configuration\n for profile <b>%s</b>",
				  agcomprisBoard->name,
				  aProfile? aProfile->name : "");

  gc_board_config_window_display( label,
				 (GcomprisConfCallback )conf_ok);

  g_free(label);

  /* init the combo to previously saved value */
  GHashTable *config = gc_db_get_conf( profile_conf, board_conf);

  gchar *locale = g_hash_table_lookup( config, "locale");

  gc_board_config_combo_locales( locale);

}


/* ======================= */
/* = config_stop        = */
/* ======================= */
static void
config_stop()
{
}
