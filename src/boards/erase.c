/* gcompris - erase.c
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

static GnomeCanvasItem	*erase_create_item(GnomeCanvasGroup *parent);
static void		 erase_destroy_all_items(void);
static void		 erase_next_level(void);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

static int number_of_item = 0;
static int number_of_item_x = 0;
static int number_of_item_y = 0;

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
    N_("Move the mouse to erase the area and discover the background"),
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

      erase_next_level();

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
      erase_destroy_all_items();
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
      erase_next_level();
    }
}
/* ======================================= */
gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "erase")==0)
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
static void erase_next_level()
{

  gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			  imageList[RAND(0, NUMBER_OF_IMAGES-1)]);

  gcompris_bar_set_level(gcomprisBoard);

  erase_destroy_all_items();
  gamewon = FALSE;

  /* Select level difficulty */
  switch(gcomprisBoard->level)
    {
    case 1:
      number_of_item_x = gcomprisBoard->level*5;
      number_of_item_y = gcomprisBoard->level*5;
      break;
    case 2:
      number_of_item_x = gcomprisBoard->level*5;
      number_of_item_y = gcomprisBoard->level*5;
      break;
    case 3: 
    case 4:
      number_of_item_x = 4*5;
      number_of_item_y = 4*5;
      break;
    case 5:
      number_of_item_x = gcomprisBoard->level*5;
      number_of_item_y = gcomprisBoard->level*5;
      break;
    default:
      number_of_item_x = 5*5;
      number_of_item_y = 5*5;
    }
      

  /* Try the next level */
  erase_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void erase_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
static GnomeCanvasItem *erase_create_item(GnomeCanvasGroup *parent)
{
  int i,j;
  GnomeCanvasItem *item = NULL;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,

							    NULL));

  number_of_item = 0;

  for(i=0; i<BOARDWIDTH; i+=BOARDWIDTH/number_of_item_x)
    {
      for(j=0; j<BOARDHEIGHT; j+=BOARDHEIGHT/number_of_item_y)
	{

	  item = gnome_canvas_item_new (boardRootItem,
					gnome_canvas_rect_get_type (),
					"x1", (double) i,
					"y1", (double) j,
					"x2", (double) i+BOARDWIDTH/number_of_item_x,
					"y2", (double)  j+BOARDHEIGHT/number_of_item_y,
					"fill_color", "blue",
					"outline_color", "green",
					"width_units", (double)1,
					NULL);

	  gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event, NULL);
	  number_of_item++;
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
    gcompris_play_sound (SOUNDLISTFILE, "bonus");
  }
  erase_next_level();
}

/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{

  if(board_paused)
    return FALSE;

  gtk_object_destroy(GTK_OBJECT(item));

  if(--number_of_item == 0)
    {
      gamewon = TRUE;
      erase_destroy_all_items();
      gcompris_display_bonus(gamewon, BONUS_SMILEY);
    }
  
  return FALSE;
}
