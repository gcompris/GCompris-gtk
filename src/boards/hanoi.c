/* gcompris - hanoi.c
 *
 * Copyright (C) 2003 Bruno Coudoin
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

GcomprisBoard *gcomprisBoard = NULL;
gboolean board_paused = TRUE;

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem	*hanoi_create_item(GnomeCanvasGroup *parent);
static void		 hanoi_destroy_all_items(void);
static void		 hanoi_next_level(void);

/*
 * Contains the piece information
 */
typedef struct {
  GnomeCanvasItem *item;
  gint x;
  gint y;
  gboolean on_top;
  gint color;
} PieceItem;

static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, PieceItem *data);

/* This contains the layout of the pieces */
#define MAX_NUMBER_X 10
#define MAX_NUMBER_Y 10
static PieceItem *position[MAX_NUMBER_X][MAX_NUMBER_Y];

static int number_of_item = 0;
static int number_of_item_x = 0;
static int number_of_item_y = 0;
static int item_width;
static int item_height;

static gchar *colorlist [] = 
  {
    "red",
    "orange",
    "yellow",
    "green",
    "blue",
    "purple",
    "grey",
    "brown",
    "cyan",
  };
#define NUMBER_OF_COLOR 8


/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Simplified Tower of Hanoi"),
    N_("Reproduce the given tower"),
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
      gcomprisBoard->maxlevel=5;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

      hanoi_next_level();

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
      hanoi_destroy_all_items();
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
      hanoi_next_level();
    }
}
/* ======================================= */
gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "hanoi")==0)
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
static void hanoi_next_level()
{

  gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			  gcompris_image_to_skin("gcompris-bg.jpg"));

  gcompris_bar_set_level(gcomprisBoard);

  hanoi_destroy_all_items();
  gamewon = FALSE;

  /* Select level difficulty */
  switch(gcomprisBoard->level)
    {
    case 1:
      number_of_item_x = 3;
      number_of_item_y = 3;
      break;
    case 2:
      number_of_item_x = 4;
      number_of_item_y = 4;
      break;
    case 3: 
      number_of_item_x = 5;
      number_of_item_y = 5;
    case 4:
      number_of_item_x = 5;
      number_of_item_y = 6;
      break;
    case 5:
      number_of_item_x = 5;
      number_of_item_y = 7;
      break;
    default:
      number_of_item_x = 5;
      number_of_item_y = 7;
    }
      

  /* Try the next level */
  hanoi_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void hanoi_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}

static void dump_solution()
{
  guint i, j;

  printf("Dumping solution\n");
  for(i=0; i<(number_of_item_x+2); i++)
    {
      for(j=0; j<number_of_item_y; j++)
	{
	  printf("%2d ", position[i][j]->color);
	}
      printf("\n");
    }

}

/* ==================================== */
static GnomeCanvasItem *hanoi_create_item(GnomeCanvasGroup *parent)
{
  int i,j;
  double gap_x, gap_y;
  double baseline;
  GnomeCanvasItem *item = NULL;
  guint color_to_place;

  /*----------------------------------------*/
  /* Empty the solution */
  for(i=0; i<(number_of_item_x+2); i++)
    {
      for(j=0; j<number_of_item_y; j++)
	{
	  position[i][j] = g_malloc(sizeof(PieceItem));
	  position[i][j]->color  = -1;
	  position[i][j]->x      = i;
	  position[i][j]->y      = j;
	  position[i][j]->on_top = FALSE;
	}
    }
  /* Initialize a random goal and store the color index in position[number_of_item_x] */
  for(i=0; i<(number_of_item_y); i++)
    {
      position[number_of_item_x+1][i]->color = (guint)RAND(0, NUMBER_OF_COLOR);
    }

  dump_solution();
  /* Randomly place the solution */
  for (color_to_place=0; color_to_place<number_of_item_y; color_to_place++)
    {
      gboolean done;

      do
	{
	  done = FALSE;
	  i = (guint)RAND(0, number_of_item_x-2);
	  j = (guint)RAND(0, number_of_item_y-2);
	  printf("%d %d\n", i, j);
	  if(position[i][j]->color == -1)
	    {
	      done = TRUE;
	      position[i][j]->color = position[number_of_item_x+1][color_to_place]->color;
	      printf("DONE color=%d\n", position[number_of_item_x+1][color_to_place]->color);
	    }
	}
      while(!done);
    }

  dump_solution();
  /* Initialize the left open positions */
  for(i=0; i<(number_of_item_x); i++)
    {
      for(j=0; j<number_of_item_y-1; j++)
	{
	  if(position[i][j]->color == -1)
	    {
	      position[i][j]->color = (guint)RAND(0, NUMBER_OF_COLOR);
	    }
	}
    }
  dump_solution();

  /* Mark the top pieces */
  for(i=0; i<(number_of_item_x); i++)
    {
      position[i][number_of_item_y-1]->on_top = TRUE;
    }
  /*----------------------------------------*/
  /* Display it now */
  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,

							    NULL));

  item_width  = BOARDWIDTH / (number_of_item_x + 2);
  item_height = 30;

  gap_x = item_width  * 0.1;
  gap_y = item_height * 0.1;

  baseline = BOARDHEIGHT/2 + item_height * number_of_item_y/2;

  number_of_item = 0;

  for(i=0; i<(number_of_item_x+2); i++)
    {
      if(i==number_of_item_x+1)
	{
	  /* Create the backgound for the target */
	  gnome_canvas_item_new (boardRootItem,
				 gnome_canvas_rect_get_type (),
				 "x1", (double) item_width * i + gap_x/2,
				 "y1", (double) baseline - item_height * number_of_item_y - gap_y - 50,
				 "x2", (double) item_width * (i+1) - gap_x/2,
				 "y2", (double) baseline + 50,
				 "fill_color", "blue",
				 "outline_color", "black",
				 "width_units", (double)1,
				 NULL);
	}
      else if (i==number_of_item_x)
	{
	  /* Create the backgound for the empty area */
	  gnome_canvas_item_new (boardRootItem,
				 gnome_canvas_rect_get_type (),
				 "x1", (double) item_width * i + gap_x/2,
				 "y1", (double) baseline - item_height * number_of_item_y - gap_y - 50,
				 "x2", (double) item_width * (i+1) - gap_x/2,
				 "y2", (double) baseline + 50,
				 "fill_color", "cyan",
				 "outline_color", "black",
				 "width_units", (double)1,
				 NULL);
	}

      /* Create the vertical line */
      guint w = 10;
      gnome_canvas_item_new (boardRootItem,
			     gnome_canvas_rect_get_type (),
			     "x1", (double) item_width * i + item_width/2 - w,
			     "y1", (double) baseline - item_height * number_of_item_y - gap_y,
			     "x2", (double) item_width * i + item_width/2 + w,
			     "y2", (double) baseline,
			     "fill_color", "grey",
			     "outline_color", "black",
			     "width_units", (double)1,
			     NULL);

      for(j=0; j<number_of_item_y; j++)
	{
	  if(position[i][j]->color != -1)
	    {
	      item = gnome_canvas_item_new (boardRootItem,
					    gnome_canvas_rect_get_type (),
					    "x1", (double) item_width * i + gap_x,
					    "y1", (double) baseline - item_height * j 
					    - item_height + gap_y,
					    "x2", (double) item_width * i + item_width - gap_x,
					    "y2", (double) baseline - item_height * j,
					    "fill_color", colorlist[position[i][j]->color],
					    "outline_color", "black",
					    "width_units", (double)1,
					    NULL);
	      position[i][j]->item = item;
	    }

	  if(i!=number_of_item_x+1)
	    gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event,  position[i][j]);
	}
    }

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
    gcompris_play_ogg ("bonus", NULL);
  }
  hanoi_next_level();
}

/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, PieceItem *data)
{
   static double x, y;
   double new_x, new_y;
   GdkCursor *fleur;
   static int dragging;
   double item_x, item_y;

   if(!gcomprisBoard)
     return FALSE;

  if(board_paused)
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

	   if(!data->on_top)
	     return FALSE;

	   x = item_x;
	   y = item_y;

	   gnome_canvas_item_raise_to_top(item);

	   fleur = gdk_cursor_new(GDK_FLEUR);
	   gnome_canvas_item_grab(item,
				  GDK_POINTER_MOTION_MASK | 
				  GDK_BUTTON_RELEASE_MASK,
				  fleur,
				  event->button.time);
	   gdk_cursor_destroy(fleur);
	   dragging = TRUE;
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
	   gnome_canvas_item_ungrab(item, event->button.time);
	   dragging = FALSE;
	 }
       break;

     default:
       break;
     }
         

  return FALSE;
}

