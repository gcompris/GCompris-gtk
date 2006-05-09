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
#define IMG_DATA_SET PACKAGE_DATA_DIR "/dataset"
#define FILE_SELECTOR_ROOT "drawings"

/* Represent the drawing area */
static gint drawing_area_x1 = 0;
static gint drawing_area_y1 = 0;
static gint drawing_area_x2 = 0;
static gint drawing_area_y2 = 0;

/* Set the grid size. If 0 then no grid */
static gint grid_step = 0;
static GnomeCanvasItem	*gridItem = NULL;
static GnomeCanvasItem	*grid_root_item = NULL;
#define DEFAULT_GRID_STEP 20

/* Needed to keep a temporary location of the user click */
static double clicked_x, clicked_y;

static GcomprisBoard	*gcomprisBoard = NULL;
static GnomeCanvasItem	*shape_root_item = NULL;
static GnomeCanvasItem	*draw_root_item = NULL;
static GnomeCanvasItem	*current_color_item = NULL;
static guint		 currentColor = 0;
static gboolean		 board_paused = TRUE;

typedef enum
  {
    TOOL_SAVE		= 0,
    TOOL_LOAD		= 1,
    TOOL_RECT		= 2,
    TOOL_FILLED_RECT	= 3,
    TOOL_CIRCLE		= 4,
    TOOL_FILLED_CIRCLE	= 5,
    TOOL_LINE		= 6,
    TOOL_IMAGE		= 7,
    TOOL_TEXT		= 8,
    TOOL_GRID		= 9,
    TOOL_DELETE		= 10,
    TOOL_FILL		= 11,
    TOOL_RAISE		= 12,
    TOOL_LOWER		= 13,
    TOOL_SELECT		= 14,
    TOOL_FLIP		= 15,
    TOOL_ROTATE_CCW	= 16,
    TOOL_ROTATE_CW	= 17,
  } ToolList;

#define NUMBER_OF_TOOL	TOOL_ROTATE_CW + 1

static ToolList		 currentTool = TOOL_RECT;
static GnomeCanvasItem	*currentToolItem = NULL;
static GnomeCanvasItem	*selectionToolItem = NULL;

// Used to cross reference pixmap for the tools
static char *tool_pixmap_name[] =
  {
    "draw/tool-save.png",            "draw/tool-save.png",
    "draw/tool-load.png",            "draw/tool-load.png",
    "draw/tool-rectangle.png",       "draw/tool-rectangle_on.png",
    "draw/tool-filledrectangle.png", "draw/tool-filledrectangle_on.png",
    "draw/tool-circle.png",          "draw/tool-circle_on.png",
    "draw/tool-filledcircle.png",    "draw/tool-filledcircle_on.png",
    "draw/tool-line.png",            "draw/tool-line_on.png",
    "draw/tool-image.png",           "draw/tool-image_on.png",
    "draw/tool-text.png",            "draw/tool-text_on.png",
    "draw/tool-grid.png",            "draw/tool-grid_on.png",
    "draw/tool-del.png",             "draw/tool-del_on.png",
    "draw/tool-fill.png",            "draw/tool-fill_on.png",
    "draw/tool-up.png",              "draw/tool-up_on.png",
    "draw/tool-down.png",            "draw/tool-down_on.png",
    "draw/tool-select.png",          "draw/tool-select_on.png",
    "draw/tool-flip.png",            "draw/tool-flip_on.png",
    "draw/tool-rotation-ccw.png",    "draw/tool-rotation-ccw_on.png",
    "draw/tool-rotation-cw.png",     "draw/tool-rotation-cw_on.png"
  };

#define PIXMAP_OFF 0
#define PIXMAP_ON  1

/*
 * A Single Point
 */
typedef struct {
  double	   x;
  double	   y;
} Point;

/*
 * Contains the points that represents the anchors
 */
typedef struct {
  GnomeCanvasItem *rootitem;
  ToolList	  tool;
  GnomeCanvasItem *item;
  GnomeCanvasItem *nw;
  GnomeCanvasItem *n;
  GnomeCanvasItem *ne;
  GnomeCanvasItem *e;
  GnomeCanvasItem *w;
  GnomeCanvasItem *sw;
  GnomeCanvasItem *s;
  GnomeCanvasItem *se;
  double	   ref_x1;
  double	   ref_y1;
  double	   ref_x2;
  double	   ref_y2;
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

#define ANCHOR_COLOR		0x36ede480
#define DEFAULT_ITEM_SIZE	40
#define DEFAULT_ANCHOR_SIZE	8
static AnchorsItem *selected_anchors_item = NULL;

#define DRAW_WIDTH_PIXELS	6

#define GRID_COLOR		0XCECECEFF

#define MAX_TEXT_CHAR		50

/* Defines the displayed colors in the color selector */
static guint ext_colorlist [] =
  {
    0x000000FF, 0x202020FF, 0x404040FF, 0x505050FF, 
    0x815a38FF, 0xb57c51FF, 0xe5a370FF, 0xfcc69cFF, 
    0xb20c0cFF, 0xea2c2cFF, 0xf26363FF, 0xf7a3a3FF, 
    0xff6600FF, 0xff8a3dFF, 0xfcaf7bFF, 0xf4c8abFF, 
    0x9b8904FF, 0xd3bc10FF, 0xf4dd2cFF, 0xfcee85FF, 
    0x255b0cFF, 0x38930eFF, 0x56d11dFF, 0x8fe268FF, 
    0x142f9bFF, 0x2d52e5FF, 0x667eddFF, 0xa6b4eaFF, 
    0x328989FF, 0x37b2b2FF, 0x3ae0e0FF, 0x96e0e0FF, 
    0x831891FF, 0xc741d8FF, 0xde81eaFF, 0xeecdf2FF, 
    0x666666FF, 0x838384FF, 0xc4c4c4FF, 0xffffffFF, 
  };

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gint	 key_press(guint keyval, gchar *commit_str, gchar *preedit_str);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 config(void);

static void	 draw_destroy_all_items(void);
static void	 draw_next_level(void);
static ToolList	 get_tool(GnomeCanvasItem *item);
static void	 create_anchors(GnomeCanvasItem *item);
static void	 display_anchors(AnchorsItem *anchorsItem, gboolean visible);
static void	 display_color_selector(GnomeCanvasGroup *parent);
static void	 display_tool_selector(GnomeCanvasGroup *parent);
static void	 display_drawing_area(GnomeCanvasGroup *parent);
static void	 display_grid(gboolean status);
static gint	 color_event(GnomeCanvasItem *item, GdkEvent *event, gchar *color);
static gint	 ext_color_event(GnomeCanvasItem *item, GdkEvent *event, gpointer color_rgba);
static void	 set_current_tool(GnomeCanvasItem *item, gint tool);
static gint	 tool_event(GnomeCanvasItem *item, GdkEvent *event, gint tool);
static void	 set_selected_item(AnchorsItem *anchorsItem);
static gint	 item_event(GnomeCanvasItem *item, GdkEvent *event, void *shape);
static gint	 item_event_resize(GnomeCanvasItem *item, GdkEvent *event, AnchorsItem *anchorsItem);
static gint	 item_event_move(GnomeCanvasItem *item, GdkEvent *event, AnchorsItem *anchorsItem);
static void	 highlight_color_item(GnomeCanvasItem *item);
static guint	 get_tool_cursor(ToolList tool);
static guint	 get_resize_cursor(AnchorType anchor);
static void	 realign_to_grid(GnomeCanvasItem *item);
static void	 snap_to_grid(double *x, double *y);
static void	 image_selected(gchar *image);
static void	 load_drawing(gchar *file);
static void	 save_drawing(gchar *file);

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "A simple vector drawing tool",
    "Creative board where you can draw freely",
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
    NULL,
    config,
    NULL,
    NULL,
    NULL
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(draw)

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  board_paused = pause;

}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{

  if(agcomprisBoard!=NULL)
    {
      gchar *img;

      gcomprisBoard=agcomprisBoard;

      /* disable im_context */
      gcomprisBoard->disable_im_context = TRUE;

      img = gcompris_image_to_skin("gcompris-bg.jpg");
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), 
			      img);
      g_free(img);


      /* set initial values for this level */
      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel=1;
      gcompris_bar_set(0);

      gcomprisBoard->number_of_sublevel=0;
      gcomprisBoard->sublevel = 0;

      drawing_area_x1 = 124;
      drawing_area_y1 = 20;
      drawing_area_x2 = gcomprisBoard->width  - 15;
      drawing_area_y2 = gcomprisBoard->height - 78;

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
      gcompris_set_cursor(GCOMPRIS_DEFAULT_CURSOR);
      pause_board(TRUE);
      draw_destroy_all_items();
      gcomprisBoard->level = 1;       // Restart this game to zero
    }
  gcomprisBoard = NULL;
}

/* Get the user keys to use with the text tool */
static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
{
  char str[2];
  char utf8char[6];

  GnomeCanvasItem	*item = NULL;

  if(!gcomprisBoard)
    return TRUE;

  /* Add some filter for control and shift key */
  switch (keyval)
    {
    case GDK_F1:
      gcompris_file_selector_load(gcomprisBoard, FILE_SELECTOR_ROOT, "", load_drawing);
      break;
    case GDK_F2:
      gcompris_file_selector_save(gcomprisBoard, FILE_SELECTOR_ROOT, "", save_drawing);
      break;
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
    case GDK_Mode_switch:
    case GDK_dead_circumflex:
    case GDK_Num_Lock:
      return FALSE;
    }

  if(selected_anchors_item == NULL)
    return TRUE;

  sprintf(utf8char, "%c", gdk_keyval_to_unicode(keyval));

  g_unichar_to_utf8 (gdk_keyval_to_unicode(keyval),
		     utf8char);

  sprintf(str, "%c", keyval);

  item = selected_anchors_item->item;

  if(GNOME_IS_CANVAS_TEXT(item))
    {
      gchar *oldtext;
      gchar *newtext;

      gtk_object_get (GTK_OBJECT (item), "text", &oldtext, NULL);

      switch(keyval)
	{
	case GDK_BackSpace:
	case GDK_Delete:
	  if(oldtext[1] != '\0') {
	    gchar *ptr;
	    ptr = g_utf8_prev_char(oldtext+g_utf8_strlen(oldtext, MAX_TEXT_CHAR)+1);
	    *ptr = '\0';
	    newtext = g_strdup(oldtext);
	  } else
	    newtext = "?";

	  break;

	default:

	  if(oldtext[0] == '?' && strlen(oldtext)==1)
	    {
	      oldtext[0] = ' ';
	      g_strstrip(oldtext);
	    }

	  if(strlen(oldtext)<MAX_TEXT_CHAR)
	    newtext = g_strconcat(oldtext, &utf8char, NULL);
	  else
	    newtext = g_strdup(oldtext);
	  break;

	}

      gnome_canvas_item_set (item,
			     "text", newtext,
			     NULL);

      g_free(oldtext);
    }

  return TRUE;
}



static gboolean
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
  GnomeCanvasItem *item = NULL;

  item = gnome_canvas_item_new (parent,
				gnome_canvas_rect_get_type (),
				"x1", (double) drawing_area_x1,
				"y1", (double) drawing_area_y1,
				"x2", (double) drawing_area_x2,
				"y2", (double) drawing_area_y2,
				"fill_color", "white",
				NULL);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event,
		     NULL);

  display_grid(FALSE);

  draw_root_item = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
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
  guint color_pixmap_height = 0;

  pixmap = gcompris_load_pixmap("draw/color-selector.png");
  if(pixmap)
    {
      x = (drawing_area_x2 - drawing_area_x1
	   - gdk_pixbuf_get_width(pixmap))/2
	+ drawing_area_x1;

      color_pixmap_height = gdk_pixbuf_get_height(pixmap);
      y = gcomprisBoard->height - color_pixmap_height - 5;

      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap,
				    "x", (double) x,
				    "y", (double) y,
				    NULL);
      gdk_pixbuf_unref(pixmap);
    }

  for(x1=x+26; x1<(x+26)+55*10; x1+=56)
    {
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_rect_get_type (),
				    "x1", (double) x1,
				    "y1", (double) y + 8,
				    "x2", (double) x1 + 24,
				    "y2", (double) y + color_pixmap_height/2,
				    "fill_color_rgba", ext_colorlist[c*4],
				    "outline_color_rgba", 0x07a3e080,
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) ext_color_event,
			 GINT_TO_POINTER(ext_colorlist[c*4]));

      if(c==0)
	highlight_color_item(item);

      item = gnome_canvas_item_new (parent,
				    gnome_canvas_rect_get_type (),
				    "x1", (double) x1 + 26,
				    "y1", (double) y + 8,
				    "x2", (double) x1 + 50,
				    "y2", (double) y + color_pixmap_height/2,
				    "fill_color_rgba", ext_colorlist[c*4+1],
				    "outline_color_rgba", 0x07a3e080,
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) ext_color_event,
			 GINT_TO_POINTER(ext_colorlist[c*4+1]));

      item = gnome_canvas_item_new (parent,
				    gnome_canvas_rect_get_type (),
				    "x1", (double) x1,
				    "y1", (double) y + color_pixmap_height/2 + 2,
				    "x2", (double) x1 + 24,
				    "y2", (double) y + color_pixmap_height - 8,
				    "fill_color_rgba", ext_colorlist[c*4+2],
				    "outline_color_rgba", 0x07a3e080,
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) ext_color_event,
			 GINT_TO_POINTER(ext_colorlist[c*4+2]));

      item = gnome_canvas_item_new (parent,
				    gnome_canvas_rect_get_type (),
				    "x1", (double) x1 + 26,
				    "y1", (double) y + color_pixmap_height/2 + 2,
				    "x2", (double) x1 + 50,
				    "y2", (double) y + color_pixmap_height - 8,
				    "fill_color_rgba", ext_colorlist[c*4+3],
				    "outline_color_rgba", 0x07a3e080,
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) ext_color_event,
			 GINT_TO_POINTER(ext_colorlist[c*4+3]));

      c++;
    }

  currentColor = ext_colorlist[0];
}

#define SELECTOR_VERTICAL_SPACING 52

static void display_tool_selector(GnomeCanvasGroup *parent)
{
  GdkPixbuf *pixmap;
  GnomeCanvasItem *item = NULL;
  gint x   = 0;
  gint x2  = 0;
  gint y   = 0;
  guint toolIndex = 0;

  pixmap = gcompris_load_pixmap("draw/tool-selector.png");
  if(pixmap)
    {
      x = 3;
      y = drawing_area_y1;
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap,
				    "x", (double) x,
				    "y", (double) y,
				    NULL);
      gdk_pixbuf_unref(pixmap);
    }

  y += 15;
  x  = 10;
  x2 = 55;
  pixmap = gcompris_load_pixmap(tool_pixmap_name[0 + PIXMAP_ON]);
  if(pixmap)
    {
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap,
				    "x", (double) x,
				    "y", (double) y,
				    "width", (double) 40.0,
				    "height", (double) 40.0,
				    "width_set", TRUE,
				    "height_set", TRUE,
				    NULL);
      gdk_pixbuf_unref(pixmap);

      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) tool_event,
			 (void *)TOOL_SAVE);

    }
  currentTool = TOOL_SAVE;
  currentToolItem = item;

  for( toolIndex = 1 ; toolIndex < NUMBER_OF_TOOL ; toolIndex++)
    {
      y += (toolIndex%2 == 0 ? SELECTOR_VERTICAL_SPACING : 0);
      pixmap = gcompris_load_pixmap(tool_pixmap_name[(2*toolIndex) + PIXMAP_OFF]);

      if(pixmap)
	{
	  item = gnome_canvas_item_new (parent,
					gnome_canvas_pixbuf_get_type (),
					"pixbuf", pixmap,
					"x", (double) (toolIndex%2 == 0 ? x : x2),
					"y", (double) y,
					"width", (double) 40.0,
					"height", (double) 40.0,
					"width_set", TRUE,
					"height_set", TRUE,
					NULL);
	  gdk_pixbuf_unref(pixmap);

	  switch(toolIndex)
	    {
	    case TOOL_GRID:
	      gridItem = item;
	      break;
	    case TOOL_SELECT:
	      selectionToolItem = item;
	      break;
	    default:
	      break;
	    }

	  gtk_signal_connect(GTK_OBJECT(item), "event",
			     (GtkSignalFunc) tool_event,
			     (void *)toolIndex);
	}
    }
}

/*
 * Request the display of the grid if status is true
 *
 */
static void display_grid(gboolean status)
{
  guint x, y;
  GdkPixbuf *pixmap = NULL;

  pixmap = gcompris_load_pixmap(tool_pixmap_name[(TOOL_GRID*2) +
						 (status == TRUE ? PIXMAP_ON : PIXMAP_OFF)]);
  if(pixmap)
    {
      gnome_canvas_item_set (gridItem,
			     "pixbuf", pixmap,
			     NULL);
      gdk_pixbuf_unref(pixmap);
    }

  if(!status)
    {
      grid_step = 0;

      if(grid_root_item!=NULL)
	gnome_canvas_item_hide(grid_root_item);
      return;
    }

  grid_step = DEFAULT_GRID_STEP;

  if(grid_root_item!=NULL)
    {
      gnome_canvas_item_show(grid_root_item);
      return;
    }

  /* Creation of the grid */

  grid_root_item = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(shape_root_item),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);
  gnome_canvas_item_raise_to_top(grid_root_item);
  //  gnome_canvas_item_raise(grid_root_item, 50);

  for( x = drawing_area_x1 ; x < drawing_area_x2 ; x += grid_step)
    {
      GnomeCanvasPoints	*points;
      GnomeCanvasItem	*item;

      for( y = drawing_area_y1 ; y < drawing_area_y2 ; y += grid_step)
	{

	  points = gnome_canvas_points_new(2);
	  points->coords[0] = (double) x;
	  points->coords[1] = (double) y;
	  points->coords[2] = (double) x;
	  points->coords[3] = (double) y + 1;

	  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(grid_root_item),
					gnome_canvas_line_get_type (),
					"points", points,
					"fill_color_rgba", GRID_COLOR,
					"width_pixels", 1,
					NULL);

	  gtk_signal_connect(GTK_OBJECT(item), "event",
			     (GtkSignalFunc) item_event,
			     NULL);

	  gnome_canvas_points_unref(points);

	}
    }
}

/*
 * Given an object, realign it to the grid
 * if the grid is on
 * FIXME: Does not work yet as expected
 */
static void realign_to_grid(GnomeCanvasItem *item)
{

  if(grid_step!=0)
    {
      double x1, y1, x2, y2;
      double xsnap1, ysnap1;

      gnome_canvas_item_get_bounds  (item,
				     &x1,
				     &y1,
				     &x2,
				     &y2);

      xsnap1 = x1;
      ysnap1 = y1;
      snap_to_grid(&xsnap1, &ysnap1);

      // Realign our object on the grid
      gnome_canvas_item_move(item,
			     x1 - xsnap1,
			     y1 - ysnap1);
    }
}

/*
 * Given a pair (x,y) rewrite them so that (x,y) is on a grid node
 * Do nothing if grid_step is 0
 */
static void snap_to_grid(double *x, double *y)
{
  guint tmp;

  if(grid_step==0)
    return;

  tmp = (guint)((*x+(grid_step/2)) - drawing_area_x1)/grid_step;
  *x = (double)drawing_area_x1 + tmp*grid_step;

  tmp = (guint)((*y+(grid_step/2)) - drawing_area_y1)/grid_step;
  *y = (double)drawing_area_y1 + tmp*grid_step;
}

/* Destroy all the items */
static void draw_destroy_all_items()
{

  /* Deleting the root item automatically deletes children items */
  if(shape_root_item!=NULL)
    gtk_object_destroy (GTK_OBJECT(shape_root_item));

  shape_root_item = NULL;
  grid_root_item = NULL;
  current_color_item = NULL;
}

/*
 * Return the proper GDK cursor based on the given tool
 *
 */
static guint get_tool_cursor(ToolList tool)
{
  switch(tool)
    {
    case TOOL_RECT:
      return(GCOMPRIS_FILLRECT_CURSOR);
      break;
    case TOOL_FILLED_RECT:
      return(GCOMPRIS_RECT_CURSOR);
      break;
    case TOOL_CIRCLE:
      return(GCOMPRIS_CIRCLE_CURSOR);
      break;
    case TOOL_FILLED_CIRCLE:
      return(GCOMPRIS_FILLCIRCLE_CURSOR);
      break;
    case TOOL_RAISE:
    case TOOL_LOWER:
    case TOOL_IMAGE:
    case TOOL_LINE:
    case TOOL_TEXT:
      return(GCOMPRIS_LINE_CURSOR);
      break;
    case TOOL_FILL:
      return(GCOMPRIS_FILL_CURSOR);
      break;
    case TOOL_DELETE:
      return(GCOMPRIS_DEL_CURSOR);
      break;
    case TOOL_SELECT:
      return(GCOMPRIS_SELECT_CURSOR);
      break;
    default:
      return(GCOMPRIS_DEFAULT_CURSOR);
      break;
    }
  return(0);
}

/**
 * Set the current tool.
 */
static void set_current_tool(GnomeCanvasItem *item, gint tool)
{
  GdkPixbuf *pixmap = NULL;

  if(currentToolItem)
    {
      pixmap = gcompris_load_pixmap(tool_pixmap_name[(currentTool*2) + PIXMAP_OFF]);
      if(pixmap)
	{
	  gnome_canvas_item_set (currentToolItem,
				 "pixbuf", pixmap,
				 NULL);
	  gdk_pixbuf_unref(pixmap);
	}
    }

  currentTool = tool;
  currentToolItem = item;

  pixmap = gcompris_load_pixmap(tool_pixmap_name[(currentTool*2) + PIXMAP_ON]);
  if(pixmap)
    {
      gnome_canvas_item_set (item,
			     "pixbuf", pixmap,
			     NULL);
      gdk_pixbuf_unref(pixmap);
    }
}

/**
 * Event that comes when a tool button is selected
 */
static gint
tool_event(GnomeCanvasItem *item, GdkEvent *event, gint tool)
{

  if(!gcomprisBoard || board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      switch(event->button.button)
	{
	case 1:

	  switch(tool)
	    {
	    case TOOL_LOAD:
	      //	      gcompris_file_selector_load(gcomprisBoard, FILE_SELECTOR_ROOT, "", load_drawing);
	      break;
	    case TOOL_SAVE:
	      //	      gcompris_file_selector_save(gcomprisBoard, FILE_SELECTOR_ROOT, "", save_drawing);
	      break;
	    case TOOL_GRID:
	      display_grid((grid_step==0 ? TRUE : FALSE));
	      return TRUE;
	      break;
	    case TOOL_IMAGE:
	      gcompris_images_selector_start(gcomprisBoard, IMG_DATA_SET, image_selected);
	    break;
	    case TOOL_RAISE:
	    case TOOL_LOWER:
	      if(selected_anchors_item)
		display_anchors(selected_anchors_item, FALSE);

	      selected_anchors_item = NULL;
	      break;
	    default:
	      break;
	    }

	  set_current_tool(item, tool);

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
ext_color_event(GnomeCanvasItem *item, GdkEvent *event, gpointer color_rgba)
{
  guint color = GPOINTER_TO_INT(color_rgba);

  if(!gcomprisBoard || board_paused)
    return FALSE;

  if(color_rgba==NULL)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      switch(event->button.button)
	{
	case 1:
	  {
	    currentColor = color;
	    highlight_color_item(item);
	  }
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

  g_assert(anchorsItem != NULL);

  if(visible)
    {
      if(anchorsItem->nw && GNOME_IS_CANVAS_RECT(anchorsItem->nw))
	{
	  gnome_canvas_item_show(anchorsItem->nw);
	  gnome_canvas_item_raise_to_top(anchorsItem->nw);
	}

      if(anchorsItem->n && GNOME_IS_CANVAS_RECT(anchorsItem->n))
	{
	  gnome_canvas_item_show(anchorsItem->n);
	  gnome_canvas_item_raise_to_top(anchorsItem->n);
	}

      if(anchorsItem->ne && GNOME_IS_CANVAS_RECT(anchorsItem->ne))
	{
	  gnome_canvas_item_show(anchorsItem->ne);
	  gnome_canvas_item_raise_to_top(anchorsItem->ne);
	}

      if(anchorsItem->w && GNOME_IS_CANVAS_RECT(anchorsItem->w))
	{
	  gnome_canvas_item_show(anchorsItem->w);
	  gnome_canvas_item_raise_to_top(anchorsItem->w);
	}

      if(anchorsItem->e && GNOME_IS_CANVAS_RECT(anchorsItem->e))
	{
	  gnome_canvas_item_show(anchorsItem->e);
	  gnome_canvas_item_raise_to_top(anchorsItem->e);
	}

      if(anchorsItem->sw && GNOME_IS_CANVAS_RECT(anchorsItem->sw))
	{
	  gnome_canvas_item_show(anchorsItem->sw);
	  gnome_canvas_item_raise_to_top(anchorsItem->sw);
	}

      if(anchorsItem->s && GNOME_IS_CANVAS_RECT(anchorsItem->s))
	{
	  gnome_canvas_item_show(anchorsItem->s);
	  gnome_canvas_item_raise_to_top(anchorsItem->s);
	}

      if(anchorsItem->se && GNOME_IS_CANVAS_RECT(anchorsItem->se))
	{
	  gnome_canvas_item_show(anchorsItem->se);
	  gnome_canvas_item_raise_to_top(anchorsItem->se);
	}

    }
  else
    {
      if(anchorsItem->nw && GNOME_IS_CANVAS_RECT(anchorsItem->nw))
	gnome_canvas_item_hide(anchorsItem->nw);

      if(anchorsItem->n && GNOME_IS_CANVAS_RECT(anchorsItem->n))
	gnome_canvas_item_hide(anchorsItem->n);

      if(anchorsItem->ne && GNOME_IS_CANVAS_RECT(anchorsItem->ne))
	gnome_canvas_item_hide(anchorsItem->ne);

      if(anchorsItem->w && GNOME_IS_CANVAS_RECT(anchorsItem->w))
	gnome_canvas_item_hide(anchorsItem->w);

      if(anchorsItem->e && GNOME_IS_CANVAS_RECT(anchorsItem->e))
	gnome_canvas_item_hide(anchorsItem->e);

      if(anchorsItem->sw && GNOME_IS_CANVAS_RECT(anchorsItem->sw))
	gnome_canvas_item_hide(anchorsItem->sw);

      if(anchorsItem->s && GNOME_IS_CANVAS_RECT(anchorsItem->s))
	gnome_canvas_item_hide(anchorsItem->s);

      if(anchorsItem->se && GNOME_IS_CANVAS_RECT(anchorsItem->se))
	gnome_canvas_item_hide(anchorsItem->se);
    }
}

static void reset_anchors_text(AnchorsItem *anchorsItem)
{

  double x1, x2, y1, y2;

  gnome_canvas_item_get_bounds(anchorsItem->item,  &x1, &y1, &x2, &y2);

  if(anchorsItem->nw)
    gnome_canvas_item_set (anchorsItem->nw,
			   "x1", (double) x1+(x2-x1)/2 - DEFAULT_ANCHOR_SIZE,
			   "y1", (double) y2 - DEFAULT_ANCHOR_SIZE,
			   "x2", (double) x1+(x2-x1)/2 + DEFAULT_ANCHOR_SIZE,
			   "y2", (double) y2 + DEFAULT_ANCHOR_SIZE,
			   NULL);

}

static void reset_anchors_line(AnchorsItem *anchorsItem)
{
  GnomeCanvasItem *item;
  double x1, x2, y1, y2;
  double x, y;
  GnomeCanvasPoints *points;

  item = anchorsItem->item;

  g_object_get(G_OBJECT(item), "points", &points, NULL);
  if (points == NULL){
    g_warning ("ERROR: LINE points NULL\n");
    return;
  }
  x1 = points->coords[0];
  y1 = points->coords[1];
  x2 = points->coords[2];
  y2 = points->coords[3];

  if(anchorsItem->nw)
    gnome_canvas_item_set (anchorsItem->nw,
			   "x1", (double) x1 - DEFAULT_ANCHOR_SIZE,
			   "y1", (double) y1 - DEFAULT_ANCHOR_SIZE,
			   "x2", (double) x1 + DEFAULT_ANCHOR_SIZE,
			   "y2", (double) y1 + DEFAULT_ANCHOR_SIZE,
			   NULL);

  if(anchorsItem->se)
    gnome_canvas_item_set (anchorsItem->se,
			   "x1", (double) x2 - DEFAULT_ANCHOR_SIZE,
			   "y1", (double) y2 - DEFAULT_ANCHOR_SIZE,
			   "x2", (double) x2 + DEFAULT_ANCHOR_SIZE,
			   "y2", (double) y2 + DEFAULT_ANCHOR_SIZE,
			   NULL);

}


static void reset_anchors_bounded(AnchorsItem *anchorsItem)
{
  GnomeCanvasItem *item;
  double x1, x2, y1, y2;

  item = anchorsItem->item;

  //gnome_canvas_item_get_bounds(item,  &x1, &y1, &x2, &y2);

  /* For each type of item, get the bounding box
   * We do not use gnome_canvas_item_get_bounds because it creates lags
   */
  if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_RECT || 
      G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_ELLIPSE) {

    g_object_get(G_OBJECT(item), "x1", &x1, "y1", &y1, "x2", &x2, "y2", &y2, NULL);

  } else if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_PIXBUF ) {
    double width, height;

    g_object_get(G_OBJECT(item), "x", &x1, "y", &y1, NULL);
    g_object_get(G_OBJECT(item), "width", &width, "height", &height, NULL);
    x2 = x1 + width;
    y2 = y1 + height;
  }

  if(anchorsItem->nw)
    gnome_canvas_item_set (anchorsItem->nw,
			   "x1", (double) x1 - DEFAULT_ANCHOR_SIZE,
			   "y1", (double) y1 - DEFAULT_ANCHOR_SIZE,
			   "x2", (double) x1 + DEFAULT_ANCHOR_SIZE,
			   "y2", (double) y1 + DEFAULT_ANCHOR_SIZE,
			   NULL);

  if(anchorsItem->n)
    gnome_canvas_item_set (anchorsItem->n,
			   "x1", (double) x1+(x2-x1)/2 - DEFAULT_ANCHOR_SIZE,
			   "y1", (double) y1 - DEFAULT_ANCHOR_SIZE,
			   "x2", (double) x1+(x2-x1)/2 + DEFAULT_ANCHOR_SIZE,
			   "y2", (double) y1 + DEFAULT_ANCHOR_SIZE,
			   NULL);

  if(anchorsItem->ne)
    gnome_canvas_item_set (anchorsItem->ne,
			   "x1", (double) x2 - DEFAULT_ANCHOR_SIZE,
			   "y1", (double) y1 - DEFAULT_ANCHOR_SIZE,
			   "x2", (double) x2 + DEFAULT_ANCHOR_SIZE,
			   "y2", (double) y1 + DEFAULT_ANCHOR_SIZE,
			   NULL);

  if(anchorsItem->e)
    gnome_canvas_item_set (anchorsItem->e,
			   "x1", (double) x2 - DEFAULT_ANCHOR_SIZE,
			   "y1", (double) y1+(y2-y1)/2 - DEFAULT_ANCHOR_SIZE,
			   "x2", (double) x2 + DEFAULT_ANCHOR_SIZE,
			   "y2", (double) y1+(y2-y1)/2 + DEFAULT_ANCHOR_SIZE,
			   NULL);

  if(anchorsItem->w)
    gnome_canvas_item_set (anchorsItem->w,
			   "x1", (double) x1 - DEFAULT_ANCHOR_SIZE,
			   "y1", (double) y1+(y2-y1)/2 - DEFAULT_ANCHOR_SIZE,
			   "x2", (double) x1 + DEFAULT_ANCHOR_SIZE,
			   "y2", (double) y1+(y2-y1)/2 + DEFAULT_ANCHOR_SIZE,
			   NULL);

  if(anchorsItem->sw)
    gnome_canvas_item_set (anchorsItem->sw,
			   "x1", (double) x1 - DEFAULT_ANCHOR_SIZE,
			   "y1", (double) y2 - DEFAULT_ANCHOR_SIZE,
			   "x2", (double) x1 + DEFAULT_ANCHOR_SIZE,
			   "y2", (double) y2 + DEFAULT_ANCHOR_SIZE,
			   NULL);

  if(anchorsItem->se)
    gnome_canvas_item_set (anchorsItem->se,
			   "x1", (double) x2 - DEFAULT_ANCHOR_SIZE,
			   "y1", (double) y2 - DEFAULT_ANCHOR_SIZE,
			   "x2", (double) x2 + DEFAULT_ANCHOR_SIZE,
			   "y2", (double) y2 + DEFAULT_ANCHOR_SIZE,
			   NULL);

  if(anchorsItem->s)
    gnome_canvas_item_set (anchorsItem->s,
			   "x1", (double) x1+(x2-x1)/2 - DEFAULT_ANCHOR_SIZE,
			   "y1", (double) y2 - DEFAULT_ANCHOR_SIZE,
			   "x2", (double) x1+(x2-x1)/2 + DEFAULT_ANCHOR_SIZE,
			   "y2", (double) y2 + DEFAULT_ANCHOR_SIZE,
			   NULL);

}

static void resize_item(AnchorsItem *anchorsItem, AnchorType anchor, double x, double y)
{
  double		 x1, y1, x2, y2;
  GnomeCanvasPoints	*points = NULL;
  GnomeCanvasItem	*item = NULL;

  item = anchorsItem->item;
  gnome_canvas_item_get_bounds  (item,
				 &x1,
				 &y1,
				 &x2,
				 &y2);

  if(GNOME_IS_CANVAS_PIXBUF(item))
    {
      switch(anchor)
	{
	case ANCHOR_E:
	  if(x>x1+1)
	    gnome_canvas_item_set (item,
				   "width", (double) x-x1,
				   NULL);
	  break;
	case ANCHOR_W:
	  if(x<x2-1)
	    {
	      gnome_canvas_item_set (item,
				     "x", (double) x,
				     NULL);
	      gnome_canvas_item_set (item,
				     "width", (double) x2-x,
				     NULL);
	    }
	  break;
	case ANCHOR_N:
	  if(y<y2-1)
	    {
	      gnome_canvas_item_set (item,
				     "y", (double) y,
				     NULL);
	      gnome_canvas_item_set (item,
				     "height", (double) y2-y,
				     NULL);
	    }
	  break;
	case ANCHOR_S:
	  if(y>y1+1)
	    gnome_canvas_item_set (item,
				   "height", (double) y-y1,
				   NULL);
	  break;
	case ANCHOR_NW:
	  if(y<y2-1)
	    {
	      gnome_canvas_item_set (item,
				     "y", (double) y,
				     NULL);
	      gnome_canvas_item_set (item,
				     "height", (double) y2-y,
				     NULL);
	    }
	  if(x<x2-1)
	    {
	      gnome_canvas_item_set (item,
				     "x", (double) x,
				     NULL);
	      gnome_canvas_item_set (item,
				     "width", (double) x2-x,
				     NULL);
	    }
	  break;
	case ANCHOR_NE:
	  if(y<y2-1)
	    {
	      gnome_canvas_item_set (item,
				     "y", (double) y,
				     NULL);
	      gnome_canvas_item_set (item,
				     "height", (double) y2-y,
				     NULL);
	    }
	  if(x>x1+1)
	    {
	      gnome_canvas_item_set (item,
				     "width", (double) x,
				     NULL);
	      gnome_canvas_item_set (item,
				     "width", (double) x-x1,
				     NULL);
	    }
	  break;
	case ANCHOR_SW:
	  if(y>y1+1)
	    {
	      gnome_canvas_item_set (item,
				     "height", (double) y,
				     NULL);
	    gnome_canvas_item_set (item,
				   "height", (double) y-y1,
				   NULL);
	    }
	  if(x<x2-1)
	    {
	      gnome_canvas_item_set (item,
				     "x", (double) x,
				     NULL);
	      gnome_canvas_item_set (item,
				     "width", (double) x2-x,
				     NULL);
	    }
	  break;
	case ANCHOR_SE:
	  if(y>y1+1)
	    {
	      gnome_canvas_item_set (item,
				     "height", (double) y,
				     NULL);
	      gnome_canvas_item_set (item,
				     "height", (double) y-y1,
				     NULL);
	    }
	  if(x>x1+1)
	    {
	      gnome_canvas_item_set (item,
				     "width", (double) x,
				     NULL);
	      gnome_canvas_item_set (item,
				     "width", (double) x-x1,
				     NULL);
	    }
	  break;
	}

      reset_anchors_bounded(anchorsItem);
    }
  else if(GNOME_IS_CANVAS_RECT(item) || GNOME_IS_CANVAS_ELLIPSE(item))
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

      reset_anchors_bounded(anchorsItem);
    }
  else if(GNOME_IS_CANVAS_LINE(item))
    {

      /* I don't know why, I need to shrink the bounding box */
      x1 += DRAW_WIDTH_PIXELS;
      y1 += DRAW_WIDTH_PIXELS;
      x2 -= DRAW_WIDTH_PIXELS;
      y2 -= DRAW_WIDTH_PIXELS;

      switch(anchor)
	{
	case ANCHOR_NW:
	  points = gnome_canvas_points_new(2);
	  points->coords[0] = (double) x;
	  points->coords[1] = (double) y;
	  points->coords[2] = (double) anchorsItem->ref_x2;
	  points->coords[3] = (double) anchorsItem->ref_y2;
	  break;
	case ANCHOR_SE:
	  points = gnome_canvas_points_new(2);
	  points->coords[0] = (double) anchorsItem->ref_x1;
	  points->coords[1] = (double) anchorsItem->ref_y1;
	  points->coords[2] = (double) x;
	  points->coords[3] = (double) y;
	  break;
	default:
	  break;
	}

      if(points)
	{
	  anchorsItem->ref_x1 = points->coords[0];
	  anchorsItem->ref_y1 = points->coords[1];
	  anchorsItem->ref_x2 = points->coords[2];
	  anchorsItem->ref_y2 = points->coords[3];

	  gnome_canvas_item_set (item,
				 "points", points,
				 NULL);

	  gnome_canvas_points_unref(points);

	  reset_anchors_line(anchorsItem);
	}
    }
  else if(GNOME_IS_CANVAS_TEXT(item))
    {
      /* FIXME : Should take care of not moving the text outside the drawing area */
      /* Resizing a text is just moving it */
      gnome_canvas_item_set (item,
			     "y", (double) y,
			     "x", (double) x,
			     NULL);
      reset_anchors_text(anchorsItem);
    }
}

/*
 * Set the color of the item based on the tool with which it has been
 * created
 *
 */
static void set_item_color(AnchorsItem *anchorsItem, guint color)
{
  GnomeCanvasItem *item = anchorsItem->item;

  switch(anchorsItem->tool)
    {
    case TOOL_RECT:
    case TOOL_CIRCLE:
      gnome_canvas_item_set (GNOME_CANVAS_ITEM(item),
			     "outline_color_rgba", color,
			     NULL);
      break;
    case TOOL_FILLED_RECT:
    case TOOL_FILLED_CIRCLE:
    case TOOL_TEXT:
      gnome_canvas_item_set (GNOME_CANVAS_ITEM(item),
			     "fill_color_rgba", color,
			     NULL);
      break;
    case TOOL_LINE:
      gnome_canvas_item_set (GNOME_CANVAS_ITEM(item),
			     "fill_color_rgba", color,
			     NULL);
      break;
    default:
      break;
    }
}

/*
 * Given an item, return the tool as a ToolList used to create it
 *
 */
static ToolList get_tool(GnomeCanvasItem *item)
{
  if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_GROUP )
    return -1;
  else if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_RECT )
    return TOOL_RECT;
  else if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_ELLIPSE )
    return TOOL_CIRCLE;
  else if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_LINE )
    return TOOL_LINE;
  else if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_TEXT )
    return TOOL_TEXT;
  else if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_PIXBUF )
    return TOOL_IMAGE;

  return -1;
}

/*
 * Create the anchors
 * (The item MUST BE in a group. The anchors are created in this group also)
 */
static void create_anchors(GnomeCanvasItem *item)
{
  GnomeCanvasGroup	*item_root_item = NULL;
  GnomeCanvasItem	*anchorItem = NULL;
  AnchorsItem		*anchorsItem = NULL;
  ToolList		 currentTool;

  g_assert(item != NULL);

  item_root_item = GNOME_CANVAS_GROUP(item->parent);

  /* Let the new item be on top */
  gnome_canvas_item_raise_to_top(item);

  currentTool = get_tool(item);

  /* Create the Anchors */
  anchorsItem = g_new0(AnchorsItem, 1);

  anchorsItem->rootitem = GNOME_CANVAS_ITEM(item_root_item);
  anchorsItem->item = item;
  anchorsItem->tool = currentTool;


  /*----------------------------------------*/
  switch(currentTool)
    {
    case TOOL_LINE:
      {
	GnomeCanvasPoints* points = NULL;

	// Keep track of the original size. It helps the resize operation for the line
	g_object_get(G_OBJECT(item), "points", &points, NULL);
	if (points == NULL){
	  g_warning ("ERROR: LINE points NULL \n");
	  return;
	} else {
	  anchorsItem->ref_x1 = points->coords[0];
	  anchorsItem->ref_y1 = points->coords[1];
	  anchorsItem->ref_x2 = points->coords[2];
	  anchorsItem->ref_y2 = points->coords[3];
	}

	anchorItem = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
					    gnome_canvas_rect_get_type (),
					    "fill_color_rgba", ANCHOR_COLOR,
					    "outline_color", "black",
					    "width_pixels", 1,
					    NULL);
	anchorsItem->nw = anchorItem;
	gtk_object_set_user_data(GTK_OBJECT(anchorItem), (void *)ANCHOR_NW);
	gtk_object_set_data(GTK_OBJECT(anchorItem),"anchors", GINT_TO_POINTER(TRUE));
	
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
	gtk_object_set_data(GTK_OBJECT(anchorItem),"anchors", GINT_TO_POINTER(TRUE));

	gtk_signal_connect(GTK_OBJECT(anchorItem), "event",
			   (GtkSignalFunc) item_event_resize,
			   anchorsItem);

	anchorsItem->n  = NULL;
	anchorsItem->s  = NULL;
	anchorsItem->e  = NULL;
	anchorsItem->w  = NULL;
	anchorsItem->ne = NULL;
	anchorsItem->sw = NULL;

	reset_anchors_line(anchorsItem);
	break;
      }
    case TOOL_IMAGE:
    case TOOL_RECT:
    case TOOL_CIRCLE:
    case TOOL_FILLED_RECT:
    case TOOL_FILLED_CIRCLE:
      anchorItem = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
					  gnome_canvas_rect_get_type (),
					  "fill_color_rgba", ANCHOR_COLOR,
					  "outline_color", "black",
					  "width_pixels", 1,
					  NULL);
      anchorsItem->nw = anchorItem;
      gtk_object_set_user_data(GTK_OBJECT(anchorItem), (void *)ANCHOR_NW);
      gtk_object_set_data(GTK_OBJECT(anchorItem),"anchors", GINT_TO_POINTER(TRUE));

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
      gtk_object_set_data(GTK_OBJECT(anchorItem),"anchors", GINT_TO_POINTER(TRUE));

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
      gtk_object_set_data(GTK_OBJECT(anchorItem),"anchors", GINT_TO_POINTER(TRUE));

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
      gtk_object_set_data(GTK_OBJECT(anchorItem),"anchors", GINT_TO_POINTER(TRUE));

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
      gtk_object_set_data(GTK_OBJECT(anchorItem),"anchors", GINT_TO_POINTER(TRUE));

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
      gtk_object_set_data(GTK_OBJECT(anchorItem),"anchors", GINT_TO_POINTER(TRUE));

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
      gtk_object_set_data(GTK_OBJECT(anchorItem),"anchors", GINT_TO_POINTER(TRUE));

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
      gtk_object_set_data(GTK_OBJECT(anchorItem),"anchors", GINT_TO_POINTER(TRUE));

      gtk_signal_connect(GTK_OBJECT(anchorItem), "event",
			 (GtkSignalFunc) item_event_resize,
			 anchorsItem);

      reset_anchors_bounded(anchorsItem);
      break;

      /* Anchors is needed to show the text widget that has the focus */
    case TOOL_TEXT:
      anchorItem = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
					  gnome_canvas_rect_get_type (),
					  "fill_color_rgba", ANCHOR_COLOR,
					  "outline_color", "black",
					  "width_pixels", 1,
					  NULL);
      anchorsItem->nw = anchorItem;
      gtk_object_set_user_data(GTK_OBJECT(anchorItem), (void *)ANCHOR_NW);
      gtk_object_set_data(GTK_OBJECT(anchorItem),"anchors", GINT_TO_POINTER(TRUE));

      gtk_signal_connect(GTK_OBJECT(anchorItem), "event",
			 (GtkSignalFunc) item_event_resize,
			 anchorsItem);

      anchorsItem->n  = NULL;
      anchorsItem->s  = NULL;
      anchorsItem->e  = NULL;
      anchorsItem->w  = NULL;
      anchorsItem->ne = NULL;
      anchorsItem->se = NULL;
      anchorsItem->sw = NULL;

      reset_anchors_text(anchorsItem);
      break;

    default:
      /* No anchors in these cases */
      anchorsItem->n  = NULL;
      anchorsItem->s  = NULL;
      anchorsItem->e  = NULL;
      anchorsItem->w  = NULL;
      anchorsItem->ne = NULL;
      anchorsItem->sw = NULL;
      anchorsItem->nw = NULL;
      anchorsItem->se = NULL;
      break;
    }

  if(selected_anchors_item)
    display_anchors(selected_anchors_item, FALSE);

  selected_anchors_item = anchorsItem;
  display_anchors(anchorsItem, TRUE);

  /* Move is performed on the item itself */
  gtk_signal_connect(GTK_OBJECT(anchorsItem->rootitem), "event",
		     (GtkSignalFunc) item_event_move,
		     anchorsItem);
}

static GnomeCanvasItem *create_item(double x, double y, gchar *imagename)
{
  GnomeCanvasItem *item = NULL;
  GnomeCanvasPoints* points = NULL;
  GnomeCanvasItem *item_root_item = NULL;
  guint item_size_x = 0;
  guint item_size_y = 0;
  GdkPixbuf *pixmap = NULL;

  item_root_item = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(draw_root_item),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  item_size_x = MIN(DEFAULT_ITEM_SIZE, drawing_area_x2 - x);
  item_size_y = MIN(DEFAULT_ITEM_SIZE, drawing_area_y2 - y);

  switch(currentTool)
    {
    case TOOL_IMAGE:
      // This is an image
      x = (drawing_area_x2-drawing_area_x1)/2;
      y = (drawing_area_y2-drawing_area_y1)/2;
      pixmap = gcompris_load_pixmap(imagename);
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap,
				    "x", (double)x-gdk_pixbuf_get_width(pixmap)/2,
				    "y", (double)y-gdk_pixbuf_get_height(pixmap)/2,
				    "width", (double) gdk_pixbuf_get_width(pixmap),
				    "height", (double) gdk_pixbuf_get_height(pixmap),
				    "width_set", TRUE,
				    "height_set", TRUE,
				NULL);
      //				    "anchor", GTK_ANCHOR_CENTER,

      /* Tell svg_save the filename */
      gtk_object_set_data(GTK_OBJECT(item), "filename", g_strdup(imagename));

      gdk_pixbuf_unref(pixmap);
      break;
    case TOOL_RECT:
      // This is a rectangle
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
				    gnome_canvas_rect_get_type (),
				    "x1", (double) x,
				    "y1", (double) y,
				    "x2", (double) x + item_size_x,
				    "y2", (double) y + item_size_y,
				    "outline_color_rgba", currentColor,
				    "width_pixels", DRAW_WIDTH_PIXELS,
				    NULL);
      /* Tell svg_save this is a transparent item */
      gtk_object_set_data(GTK_OBJECT(item),"empty", GINT_TO_POINTER(TRUE));
      break;
    case TOOL_FILLED_RECT:
      // This is a filled rectangle
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
				    gnome_canvas_rect_get_type (),
				    "x1", (double) x,
				    "y1", (double) y,
				    "x2", (double) x + item_size_x,
				    "y2", (double) y + item_size_y,
				    "fill_color_rgba", currentColor,
				    NULL);
      break;
    case TOOL_CIRCLE:
      // This is an ellipse
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
				    gnome_canvas_ellipse_get_type (),
				    "x1", (double) x,
				    "y1", (double) y,
				    "x2", (double) x + item_size_x,
				    "y2", (double) y + item_size_y,
				    "outline_color_rgba", currentColor,
				    "width_pixels", DRAW_WIDTH_PIXELS,
				    NULL);
      /* Tell svg_save this is a transparent item */
      gtk_object_set_data(GTK_OBJECT(item),"empty", GINT_TO_POINTER(TRUE));
      break;
    case TOOL_FILLED_CIRCLE:
      // This is a filled ellipse
      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
				    gnome_canvas_ellipse_get_type (),
				    "x1", (double) x,
				    "y1", (double) y,
				    "x2", (double) x + item_size_x,
				    "y2", (double) y + item_size_y,
				    "fill_color_rgba", currentColor,
				    NULL);
      break;
    case TOOL_LINE:
      // This is a line
      points = gnome_canvas_points_new(2);
      points->coords[0] = (double) x;
      points->coords[1] = (double) y;
      points->coords[2] = (double) x + item_size_x;
      points->coords[3] = (double) y + item_size_y;

      snap_to_grid(&points->coords[0], &points->coords[1]);
      snap_to_grid(&points->coords[2], &points->coords[3]);

      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
				    gnome_canvas_line_get_type (),
				    "points", points,
				    "fill_color_rgba", currentColor,
				    "width_pixels", DRAW_WIDTH_PIXELS,
				    NULL);
      break;
    case TOOL_TEXT:
      // This is text
      {
	item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(item_root_item),
				      gnome_canvas_text_get_type (),
				      "text", "?",
				      "font", gcompris_skin_font_board_big_bold,
				      "x", (double) x,
				      "y", (double) y,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color_rgba", currentColor,
				      NULL);
      }
      break;
    default:
      break;
    }

  /*
   * Create the anchors now
   */
  if(item)
    {
      create_anchors(item);
    }

  return item;
}

/*
 * Return the proper GDK cursor based on the given anchor
 *
 */
static guint get_resize_cursor(AnchorType anchor)
{
  switch(anchor)
    {
    case  ANCHOR_NW:
      return(GDK_TOP_LEFT_CORNER);
      break;
    case  ANCHOR_N:
      return(GDK_TOP_SIDE);
      break;
    case  ANCHOR_NE:
      return(GDK_TOP_RIGHT_CORNER);
      break;
    case  ANCHOR_E:
      return(GDK_RIGHT_SIDE);
      break;
    case  ANCHOR_W:
      return(GDK_LEFT_SIDE);
      break;
    case  ANCHOR_SW:
      return(GDK_BOTTOM_LEFT_CORNER);
      break;
    case  ANCHOR_S:
      return(GDK_BOTTOM_SIDE);
      break;
    case  ANCHOR_SE:
      return(GDK_BOTTOM_RIGHT_CORNER);
      break;
    }

  return 0;
}

/*
 * Special event callback for the resize operation
 */
static gint
item_event_resize(GnomeCanvasItem *item, GdkEvent *event, AnchorsItem *anchorsItem)
{
  static double			 x, y;
  static GnomeCanvasItem	*draggingItem = NULL;
  static int			 dragging;
  double			 item_x, item_y;
  GdkCursor			*fleur = NULL;
  AnchorType			 anchor;

  if(!gcomprisBoard || board_paused)
    return FALSE;

  anchor = (AnchorType)gtk_object_get_user_data(GTK_OBJECT(item));

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      switch(event->button.button)
	{
	case 1:
	  fleur = gdk_cursor_new(get_resize_cursor(anchor));
	  
	  gcompris_canvas_item_grab(item,
				 GDK_POINTER_MOTION_MASK |
				 GDK_BUTTON_RELEASE_MASK,
				 fleur,
				 event->button.time);
	  gdk_cursor_destroy(fleur);
	  draggingItem = item;
	  dragging = TRUE;

	  item_x = event->button.x;
	  item_y = event->button.y;
	  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);
	  snap_to_grid(&item_x, &item_y);
	  x = item_x;
	  y = item_y;
	  break;
	default:
	  break;
	}
      break;
    case GDK_BUTTON_RELEASE:
      if(dragging)
	{
	  gcompris_canvas_item_ungrab(item, event->button.time);
	  dragging = FALSE;
	  draggingItem = NULL;
	}
      break;
    case GDK_ENTER_NOTIFY:
      gcompris_set_cursor(get_resize_cursor(anchor));
      break;

    case GDK_LEAVE_NOTIFY:
      gcompris_set_cursor(get_tool_cursor(currentTool));
      break;

    case GDK_MOTION_NOTIFY:
      if (dragging && (event->motion.state & GDK_BUTTON1_MASK))
	{
	  double parent_x, parent_y;

	  item_x = event->button.x;
	  item_y = event->button.y;
	  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

	  parent_x = event->button.x;
	  parent_y = event->button.y;
	  gnome_canvas_item_w2i(anchorsItem->rootitem->parent, &parent_x, &parent_y);

	  snap_to_grid(&x, &y);
	  resize_item(anchorsItem, anchor, x, y);

	  /* Manage border limits */
	  if((parent_x > drawing_area_x1) && (parent_x < drawing_area_x2))
	    x = item_x;

	  if((parent_y > drawing_area_y1) && (parent_y < drawing_area_y2))
	    y = item_y;

	}
      break;
    default:
      break;
    }

  return(TRUE);
}

/**
 * Display the anchors to the selected item
 */
static void set_selected_item(AnchorsItem *anchorsItem)
{
  if(selected_anchors_item!=anchorsItem)
    {
      if(selected_anchors_item)
	display_anchors(selected_anchors_item, FALSE);

      if(anchorsItem!=NULL)
	display_anchors(anchorsItem, TRUE);

      selected_anchors_item = anchorsItem;
    }
}

/**
 * Callback of the selected image
 */
static void image_selected(gchar *image)
{
  GnomeCanvasItem *item = NULL;
  item = create_item(clicked_x, clicked_y, image);
  set_current_tool(selectionToolItem, TOOL_SELECT);
}

/*
 * Recreate an item after it has been saved and is being reloaded
 *
 */
static void recreate_item(GnomeCanvasItem *item)
{

  if(item == NULL) 
    return;

  if (G_OBJECT_TYPE(item) == GNOME_TYPE_CANVAS_GROUP ) {
    /* If it's a group, then the first object is the real object to create the anchor for */
    recreate_item(GNOME_CANVAS_ITEM(g_list_nth_data(GNOME_CANVAS_GROUP(item)->item_list, 0)));
    return;
  }

  /* Rebuild the anchors */
  create_anchors(item);
}

/**
 * Callback for the drawing load
 */
static void load_drawing(gchar *file)
{

}

/**
 * Callback for the drawing save
 */
static void save_drawing(gchar *file)
{

}

/*
 * Special event callback for the move operation
 */
static gint
item_event_move(GnomeCanvasItem *item, GdkEvent *event, AnchorsItem *anchorsItem)
{
  static double x, y;
  static double start_x, start_y;
  static GnomeCanvasItem *draggingItem = NULL;
  static int dragging;
  double new_x, new_y;
  double item_x, item_y;
  GdkCursor *fleur = NULL;

  if(!gcomprisBoard || board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      switch(event->button.button)
	{
	case 1:
	  switch(currentTool) {
	  case TOOL_IMAGE:
	  case TOOL_RECT:
	  case TOOL_FILLED_RECT:
	  case TOOL_CIRCLE:
	  case TOOL_FILLED_CIRCLE:
	  case TOOL_LINE:
	  case TOOL_TEXT:
	    /* In this case, we simply redirect to the item creation */
	    item_event(item, event, NULL);
	    break;
	  case TOOL_SELECT:
	    // Move an item
	    set_selected_item(anchorsItem);

	    fleur = gdk_cursor_new(GDK_FLEUR);

	    gcompris_canvas_item_grab(item,
				   GDK_POINTER_MOTION_MASK |
				   GDK_BUTTON_RELEASE_MASK,
				   fleur,
				   event->button.time);
	    gdk_cursor_destroy(fleur);
	    draggingItem = item;
	    dragging = TRUE;

	    item_x = event->button.x;
	    item_y = event->button.y;
	    gnome_canvas_item_w2i(item->parent, &item_x, &item_y);
	    snap_to_grid(&item_x, &item_y);

	    x = item_x;
	    y = item_y;
	    start_x = item_x;
	    start_y = item_y;

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
	    set_item_color(anchorsItem, currentColor);
	    break;

	  case TOOL_RAISE:
	    gnome_canvas_item_raise(anchorsItem->rootitem, 1);
	    break;

	  case TOOL_LOWER:
	    gnome_canvas_item_lower(anchorsItem->rootitem, 1);
	    break;

	  case TOOL_ROTATE_CW:
	    item_rotate_relative(anchorsItem->rootitem, 10);
	    reset_anchors_bounded(anchorsItem);
	    break;

	  case TOOL_ROTATE_CCW:
	    item_rotate_relative(anchorsItem->rootitem, -10);
	    reset_anchors_bounded(anchorsItem);
	    break;

	  case TOOL_FLIP:
	    {
	      GdkPixbuf *pixbuf;
	      /* Only implemented for images */
	      if(anchorsItem->tool == TOOL_IMAGE) {
		gtk_object_get (GTK_OBJECT (anchorsItem->item), "pixbuf", &pixbuf, NULL);
		if(pixbuf) {
		  GdkPixbuf *pixbuf2;
		  pixbuf2 = (GdkPixbuf *)pixbuf_copy_mirror(pixbuf, TRUE, FALSE);
		  gdk_pixbuf_unref(pixbuf);
		  gnome_canvas_item_set (anchorsItem->item,
					 "pixbuf", pixbuf2,
					 NULL);
		  gdk_pixbuf_unref(pixbuf2);
		}
	      }
	    }
	    break;
	  default:
	    break;
	  }
	  break;

	case 2:
	  // Shortcut for the Delete operation
	  if(selected_anchors_item == anchorsItem)
	    selected_anchors_item = NULL;

	  gtk_object_destroy (GTK_OBJECT(anchorsItem->rootitem));
	  g_free(anchorsItem);
	  break;

	case 3:
	  switch(currentTool) {
	    /* Perform the reverse operation when it makes sense */
	  case TOOL_ROTATE_CW:
	    item_rotate_relative(anchorsItem->rootitem, -10);
	    reset_anchors_bounded(anchorsItem);
	    break;
	  case TOOL_ROTATE_CCW:
	    item_rotate_relative(anchorsItem->rootitem, 10);
	    reset_anchors_bounded(anchorsItem);
	    break;
	  case TOOL_RAISE:
	    gnome_canvas_item_lower(anchorsItem->rootitem, 1);
	    break;
	  case TOOL_LOWER:
	    gnome_canvas_item_raise(anchorsItem->rootitem, 1);
	    break;

	  default:
	    // Shortcut for the Raise operation
	    gnome_canvas_item_raise_to_top(item);
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
	  gcompris_canvas_item_ungrab(item, event->button.time);
	  dragging = FALSE;
	  draggingItem = NULL;

	  /* Moving back the anchors around their shape */
	  display_anchors(anchorsItem, TRUE);

	}
      break;
    case GDK_ENTER_NOTIFY:
      switch(currentTool) {
      case TOOL_RECT:
      case TOOL_FILLED_RECT:
      case TOOL_CIRCLE:
      case TOOL_FILLED_CIRCLE:
      case TOOL_LINE:
      case TOOL_FLIP:
      case TOOL_DELETE:
      case TOOL_FILL:
      case TOOL_TEXT:
	gcompris_set_cursor(get_tool_cursor(currentTool));
	break;
      case TOOL_SELECT:
	gcompris_set_cursor(GDK_FLEUR);
	break;
      default:
	break;
      }
      break;
    case GDK_LEAVE_NOTIFY:
      gcompris_set_cursor(get_tool_cursor(currentTool));
      break;

    case GDK_MOTION_NOTIFY:
      if (dragging && (event->motion.state & GDK_BUTTON1_MASK))
	{
	  double x1, y1, x2, y2;

	  item_x = event->button.x;
	  item_y = event->button.y;
	  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

	  new_x = item_x;
	  new_y = item_y;

	  gnome_canvas_item_get_bounds  (item,
					 &x1,
					 &y1,
					 &x2,
					 &y2);

	  /* Manage border limits */
	  if((x1 + new_x - x) < drawing_area_x1 && (new_x - x)<0)
	    new_x = x;
	  else if((x2 + new_x -x) > drawing_area_x2 && (new_x - x)>0)
	    new_x = x;

	  if((y1 + new_y - y) < drawing_area_y1 && (new_y - y)<0)
	    new_y = y;
	  else if((y2 + new_y - y) > drawing_area_y2 && (new_y - y)>0)
	    new_y = y;

	  snap_to_grid(&new_x, &new_y);
	  gnome_canvas_item_move(item, new_x - x, new_y - y);

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
 * Event that occur when a creation of object is requested
 *
 */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, void *shape)
{
  static double x, y;
  double item_x, item_y;
  GnomeCanvasItem *newItem = NULL;

  if(!gcomprisBoard || board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      switch(event->button.button)
	{
	case 1:
	case 2:
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
	  case TOOL_TEXT:
	    // Create a new item
	    if(event->button.button==1)
	      {
		snap_to_grid(&x, &y);
		newItem = create_item(x, y, NULL);

		if(newItem==NULL)
		  return FALSE;
	      }
	    set_current_tool(selectionToolItem, TOOL_SELECT);
	    break;
	  case TOOL_SELECT:
	    set_selected_item(NULL);
	    break;
	  default:
	    break;
	  }

	default:
	  break;
	}
      break;

    case GDK_ENTER_NOTIFY:
      gcompris_set_cursor(get_tool_cursor(currentTool));
      break;

    case GDK_LEAVE_NOTIFY:
      gcompris_set_cursor(GCOMPRIS_DEFAULT_CURSOR);
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
