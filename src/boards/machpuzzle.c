/* gcompris - machpuzzle.c
 *
 * Copyright (C) 2001 Bruno Coudoin
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

#include <ctype.h>
#include <math.h>
#include <assert.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

typedef enum {
  MACH_HORZ_WALL = 0,
  MACH_VERT_WALL,
  MACH_DIAG_WALL,
  MACH_BILLARD_BALL,
  MACH_BASKET_BALL,
  MACH_FLYING_BALL,
} MachItemType;

struct _MachItem {
  MachItemType	   type;
  gboolean	   moving;
  GnomeCanvasItem *item;		/* The canvas item      */
  double	   x1, y1, x2, y2;	/* Bounding of the item */
  double	   times;
  double	   ax, ay;
  double	   xposo, xpos, vxo;
  double	   yposo, ypos, vyo;
  double	   bounce;
  double	   hotspotx[16];
  double	   hotspoty[16];
};

typedef struct _MachItem MachItem;

static GList *item_list = NULL;

GcomprisBoard *gcomprisBoard = NULL;
gboolean board_paused = TRUE;
static gint move_id = 0;
static gint score = 0;
static GnomeCanvasItem *score_item = NULL;
static  double		times_inc  = 0.1;
static double		gravity = 9.8;
static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem	*machpuzzle_create_item(GnomeCanvasGroup *parent);
static void		 machpuzzle_destroy_all_items(void);
static void		 machpuzzle_next_level(void);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, MachItem *machItem);
static void		 machpuzzle_move(GList *item_list);

static MachItem		*create_machine_item(MachItemType machItemType, double x, double y);

// List of images to use in the game
static gchar *imageList[] =
{
  "gcompris/animals/bear001.jpg",
  "gcompris/animals/malaybear002.jpg",
  "gcompris/animals/polabear011.jpg",
  "gcompris/animals/spectbear001.jpg",
  "gcompris/animals/joybear001.jpg",
  "gcompris/animals/polarbear001.jpg",
  "gcompris/animals/joybear002.jpg",
  "gcompris/animals/poolbears001.jpg"
};
#define NUMBER_OF_IMAGES 8

/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Move the mouse"),
    N_("Move the mouse to machpuzzle the area and discover the background"),
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

  if(gamewon == TRUE && pause == FALSE) /* the game is won */
    {
      game_won();
    }

  board_paused = pause;
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=6;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

      machpuzzle_next_level();

      gamewon = FALSE;
      pause_board(FALSE);
    }
}
/* ======================================= */
static void end_board ()
{
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      machpuzzle_destroy_all_items();
    }
  gcomprisBoard = NULL;
}

/* ======================================= */
static void set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      machpuzzle_next_level();
    }
}
/* ======================================= */
gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "machpuzzle")==0)
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
/* set initial values for the next level */
static void machpuzzle_next_level()
{

  gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			  imageList[RAND(0, NUMBER_OF_IMAGES-1)]);

  gcompris_bar_set_level(gcomprisBoard);

  machpuzzle_destroy_all_items();
  gamewon = FALSE;

  /* Try the next level */
  machpuzzle_create_item(gnome_canvas_root(gcomprisBoard->canvas));

  move_id = gtk_timeout_add (40, (GtkFunction) machpuzzle_move, item_list);

}
/* ==================================== */
/* Destroy all the items */
static void machpuzzle_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  if (move_id) {
    gtk_timeout_remove (move_id);
    move_id = 0;
  }

  boardRootItem = NULL;
}
/* ==================================== */
static GnomeCanvasItem *machpuzzle_create_item(GnomeCanvasGroup *parent)
{
  int i,j;
  GnomeCanvasPoints	*points;
  GnomeCanvasItem	*item;
  MachItem		*machItem;
  GdkFont		*gdk_font;

  gdk_font = gdk_font_load (FONT_BOARD_BIG);

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,

							    NULL));
  //  machItem = create_machine_item(MACH_BASKET_BALL, 20.0, 20.0);
  machItem = create_machine_item(MACH_BASKET_BALL, 200.0, 100.0);
  machItem = create_machine_item(MACH_BILLARD_BALL, 250.0, 100.0);

  //  machItem = create_machine_item(MACH_FLYING_BALL, 450.0, 400.0);

  machItem = create_machine_item(MACH_HORZ_WALL, 300.0, 300.0);

  /* The border */

  points = gnome_canvas_points_new(2);
  points->coords[0] = (double) 1;
  points->coords[1] = (double) 1;
  points->coords[2] = (double) BOARDWIDTH;
  points->coords[3] = (double) 1;
  
  item = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_line_get_type (),
				"points", points,
				"fill_color", "white",
				"width_pixels", 5,
				NULL);
  gnome_canvas_points_unref(points);

  points = gnome_canvas_points_new(2);
  points->coords[0] = (double) 1;
  points->coords[1] = (double) 1;
  points->coords[2] = (double) 1;
  points->coords[3] = (double) BOARDHEIGHT;
  
  item = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_line_get_type (),
				"points", points,
				"fill_color", "red",
				"width_pixels", 5,
				NULL);
  gnome_canvas_points_unref(points);

  points = gnome_canvas_points_new(2);
  points->coords[0] = (double) BOARDWIDTH-1;
  points->coords[1] = (double) 1;
  points->coords[2] = (double) BOARDWIDTH-1;
  points->coords[3] = (double) BOARDHEIGHT;
  
  item = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_line_get_type (),
				"points", points,
				"fill_color", "red",
				"width_pixels", 5,
				NULL);
  gnome_canvas_points_unref(points);

  points = gnome_canvas_points_new(2);
  points->coords[0] = (double) 1;
  points->coords[1] = (double) BOARDHEIGHT-2;
  points->coords[2] = (double) BOARDWIDTH;
  points->coords[3] = (double) BOARDHEIGHT-2;
  
  item = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_line_get_type (),
				"points", points,
				"fill_color", "red",
				"width_pixels", 5,
				NULL);
  gnome_canvas_points_unref(points);


  score_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", "SCORE : 0",
				      "font_gdk", gdk_font,
				      "x", (double) BOARDWIDTH  - 100,
				      "y", (double) BOARDHEIGHT - 50,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color", "red",
				      NULL);

  return NULL;
}
/* ==================================== */
static void game_won()
{
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
      board_finished(BOARD_FINISHED_RANDOM);
      return;
    }
    gcompris_play_sound (SOUNDLISTFILE, "bonus");
  }
  machpuzzle_next_level();
}

/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, MachItem *machItem)
{
  double item_x, item_y;
  double x1, y1, x2, y2;
  double width;
  item_x = event->button.x;
  item_y = event->button.y;
  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);
  gnome_canvas_item_get_bounds    (item,
				   &x1,
				   &y1,
				   &x2,
				   &y2);

  if(board_paused)
    return FALSE;

   switch (event->type) 
     {

     case GDK_BUTTON_PRESS:
       width = x2-x1;

       //       machItem->vyo   = (y1 - machItem->ypos) * machItem->bounce;
       machItem->times = 0;
       machItem->yposo = machItem->ypos;
       machItem->xposo = machItem->xpos;

       machItem->vyo = ((item_y-y1)<width/2?(width/2-(item_y-y1))*5:-1*(width/2-(y2-item_y))*5);
       machItem->vxo = ((item_x-x1)<width/2?(width/2-(item_x-x1))*5:-1*(width/2-(x2-item_x))*5);
       break;
       
     default:
       break;
     }
   
   
   return FALSE;
}

static void dump_machItem(MachItem *machItem)
{

  printf("Type		= %d\n", machItem->type);
  printf("        times = %f\n", machItem->times);
  printf("           ax = %f\n", machItem->ax);
  printf("           ay = %f\n", machItem->ay);
  printf("        xposo = %f\n", machItem->xposo);
  printf("         xpos = %f\n", machItem->xpos);
  printf("          vxo = %f\n", machItem->vxo);
  printf("        yposo = %f\n", machItem->yposo);
  printf("         ypos = %f\n", machItem->ypos);
  printf("          vyo = %f\n", machItem->vyo);

}

/* Create a machine item */
static MachItem *create_machine_item(MachItemType machItemType, double x, double y)
{
  MachItem		*machItem;
  GnomeCanvasItem	*item;
  guint			 width;
  guint			 height;
  guint			 hs = 0;

  machItem = g_new (MachItem, 1);

  machItem->type	= machItemType;

  switch (machItemType) 
    {
    case MACH_HORZ_WALL:
      width = 100;
      height = 20;
      machItem->moving	= FALSE;
      machItem->times   = 0.0;

      machItem->xposo	= x;
      machItem->xpos	= x;
      machItem->vxo	= 0;
      machItem->ax	= 0;

      machItem->yposo	= y;
      machItem->ypos	= y;
      machItem->vyo	= 0;
      machItem->ay	= 0;

      machItem->bounce  = 5;

      machItem->item = gnome_canvas_item_new (boardRootItem,
					      gnome_canvas_rect_get_type (),
					      "x1", (double) machItem->xposo,
					      "y1", (double) machItem->yposo,
					      "x2", (double) machItem->xposo + width,
					      "y2", (double) machItem->yposo + height,
					      "outline_color", "black",
					      "fill_color_rgba", 0xFF10C0FF,
					      "width_units", (double)1,
					      NULL);

      gtk_signal_connect(GTK_OBJECT(machItem->item), "event",
			 (GtkSignalFunc) item_event,
			 machItem);
      break;
    case MACH_VERT_WALL:
      break;
    case MACH_DIAG_WALL:
      break;
    case MACH_BILLARD_BALL:
      width = 30;
      machItem->moving	= TRUE;
      machItem->times   = 0.0;

      machItem->xposo	= x;
      machItem->xpos	= x;
      machItem->vxo	= 10;
      machItem->ax	= 0;

      machItem->yposo	= y;
      machItem->ypos	= y;
      machItem->vyo	= 0;
      machItem->ay	= gravity;

      machItem->bounce  = 2;

      machItem->item = gnome_canvas_item_new (boardRootItem,
					      gnome_canvas_ellipse_get_type (),
					      "x1", (double) machItem->xposo,
					      "y1", (double) machItem->yposo,
					      "x2", (double) machItem->xposo + width,
					      "y2", (double) machItem->yposo + width,
					      "outline_color", "black",
					      "fill_color_rgba", 0x1010FFFF,
					      "width_units", (double)1,
					      NULL);

      gtk_signal_connect(GTK_OBJECT(machItem->item), "event",
			 (GtkSignalFunc) item_event,
			 machItem);

      //      machItem->hotspotx[hs]   = machItem->xposo - 1;
      //      machItem->hotspoty[hs++] = machItem->yposo - 1;
      machItem->hotspotx[hs]   = machItem->xposo + width + 1;
      machItem->hotspoty[hs++] = machItem->yposo + width + 1;

      break;
    case MACH_BASKET_BALL:
      width = 40;
      machItem->moving	= TRUE;
      machItem->times   = 0.0;

      machItem->xposo	= x;
      machItem->xpos	= x;
      machItem->vxo	= 10;
      machItem->ax	= 0;

      machItem->yposo	= y;
      machItem->ypos	= y;
      machItem->vyo	= 0;
      machItem->ay	= gravity;

      machItem->bounce  = 7;

      machItem->item = gnome_canvas_item_new (boardRootItem,
					      gnome_canvas_ellipse_get_type (),
					      "x1", (double) machItem->xposo,
					      "y1", (double) machItem->yposo,
					      "x2", (double) machItem->xposo + width,
					      "y2", (double) machItem->yposo + width,
					      "outline_color", "black",
					      "fill_color_rgba", 0xFFFF00FF,
					      "width_units", (double)1,
					      NULL);

      gtk_signal_connect(GTK_OBJECT(machItem->item), "event",
			 (GtkSignalFunc) item_event,
			 machItem);
     
      machItem->hotspotx[hs]   = machItem->xposo + width + 1;
      machItem->hotspoty[hs++] = machItem->yposo + width + 1;

      break;
    case MACH_FLYING_BALL:
      width = 40;
      machItem->moving	= TRUE;
      machItem->times   = 0.0;

      machItem->xposo	= x;
      machItem->xpos	= x;
      machItem->vxo	= 10;
      machItem->ax	= 0;

      machItem->yposo	= y;
      machItem->ypos	= y;
      machItem->vyo	= -5;
      machItem->ay	= -5;

      machItem->bounce  = 2;

      machItem->item = gnome_canvas_item_new (boardRootItem,
					      gnome_canvas_ellipse_get_type (),
					      "x1", (double) machItem->xposo,
					      "y1", (double) machItem->yposo,
					      "x2", (double) machItem->xposo + width,
					      "y2", (double) machItem->yposo + width * 1.5,
					      "outline_color", "black",
					      "fill_color_rgba", 0xE03000FF,
					      "width_units", (double)1,
					      NULL);

      gtk_signal_connect(GTK_OBJECT(machItem->item), "event",
			 (GtkSignalFunc) item_event,
			 machItem);
      break;
    }

  /* End of hot spot list for collision detection */
  machItem->hotspotx[hs]   = - 1;
  machItem->hotspoty[hs++] = - 1;

  gtk_object_set_data(GTK_OBJECT(machItem->item),"mach",(gpointer)machItem);

  item_list = g_list_append (item_list, machItem);

  return machItem;
}

/* Move */
static void machpuzzle_move(GList *item_list)
{
  double		 x1, y1, x2, y2;
  MachItem		*machItem;
  GnomeCanvasItem	*item;
  guint			 i;
  gboolean		 collision = FALSE;

  for(i=0; i<g_list_length(item_list); i++)
    {
      gint j = 0;

      machItem = g_list_nth_data(item_list, i);

      //     dump_machItem(machItem);
      item = machItem->item;

      if(machItem->moving)
	{

	  gnome_canvas_item_get_bounds(item, &x1, &y1, &x2, &y2);

	  machItem->times += times_inc;
	  
	  /* Collision detection */
	  while(machItem->hotspotx[j]!=-1)
	    {
	      GnomeCanvasItem *collItem;
	      MachItem	      *collMachItem;
	      double	       wx, wy;

	      wx = machItem->hotspotx[j];
	      wy = machItem->hotspoty[j];
	      gnome_canvas_item_i2w(machItem->item, &wx,&wy);

	      printf("Checking coll detec %d at x=%f y=%f\n", j, wx, wy);
	      printf("    hotspotx=%f  hotspoty=%f\n", 
		     machItem->hotspotx[j], 
		     machItem->hotspoty[j]);
	      collItem = gnome_canvas_get_item_at(gcomprisBoard->canvas,
						  wx, wy);
	      if (collItem)
		{
		  printf("  got an item\n");
		  collMachItem=(MachItem*)gtk_object_get_data(GTK_OBJECT(collItem),
							      "mach");
		  if(collMachItem)
		    {
		      printf("!!! Collision detected with:\n");
		      dump_machItem(collMachItem);

		      collision = TRUE;
		    }
		}

	      j++;

	    }

	  machItem->ypos=machItem->yposo 
	    + (machItem->vyo*machItem->times) 
	    + (.5*machItem->ay * (machItem->times*machItem->times));
	  
	  machItem->xpos=machItem->xposo 
	    + (machItem->vxo*machItem->times) 
	    + (.5*machItem->ax * (machItem->times*machItem->times));
	  
	  
	  item_absolute_move(item, machItem->xpos, machItem->ypos);
	  
	  if(y2>=BOARDHEIGHT-5 && (y1 - machItem->ypos)<=0 || collision == TRUE)
	    {
	      machItem->vyo   = (y1 - machItem->ypos) * machItem->bounce;
	      machItem->times = 0;
	      machItem->yposo = machItem->ypos;
	      machItem->xposo = machItem->xpos;
	      
	      /* Floor touch */
	      machItem->vxo *= 0.9;
	    }
	  
	  if(y1<=5 && (y1 - machItem->ypos)>0)
	    {
	      machItem->vyo = (y1 - machItem->ypos) * machItem->bounce;
	      machItem->times=0;
	      machItem->yposo=machItem->ypos;
	      machItem->xposo=machItem->xpos;
	      
	      score ++;
	      gnome_canvas_item_set (score_item,
				     "text", g_strdup_printf("Score : %d", score),
				     NULL);
	      
	    }
	  if(x1<=5 && (x1 - machItem->xpos)>0)
	    {
	      machItem->vyo = -1 * (y1 - machItem->ypos) * machItem->bounce;
	      machItem->vxo = (x1 - machItem->xpos) * machItem->bounce;
	      machItem->times=0;
	      machItem->yposo=machItem->ypos;
	      machItem->xposo=machItem->xpos;
	    }
	  if(x2>=BOARDWIDTH-5 && machItem->vxo>0)
	    {
	      machItem->vyo = -1 * (y1 - machItem->ypos) * machItem->bounce;
	      machItem->vxo = (x1 - machItem->xpos) * machItem->bounce;
	      machItem->times=0;
	      machItem->yposo=machItem->ypos;
	      machItem->xposo=machItem->xpos;
	    }
	}

    }

}

