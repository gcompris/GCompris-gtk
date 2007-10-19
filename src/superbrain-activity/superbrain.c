/* gcompris - superbrain.c
 *
 * Copyright (C) 2002 Bruno Coudoin
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

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

/*
 * Contains the points that represents the anchors
 */
typedef struct {
  GooCanvasItem	*rootitem;
  GList			*listitem;
  guint			 selecteditem;
  GooCanvasItem	*good;
  GooCanvasItem	*misplaced;
  gboolean		completed;
} Piece;

static GList * listPieces = NULL;

static guint colors[] =
  {
    0x0000FF80,
    0x00FF0080,
    0xFF000080,
    0x00FFFF80,
    0xFF00FF80,
    0xFFFF0080,
    0x00007F80,
    0x007F0080,
    0x7F000080,
    0x7F007F80,
  };

#define MAX_COLORS	10

#define PIECE_WIDTH	20
#define PIECE_HEIGHT	20
#define PIECE_GAP	18
#define PIECE_GAP_GOOD	5
#define SCROLL_LIMIT	160
#define PLAYING_AREA_X	190
#define PLAYING_HELP_X	(BOARDWIDTH-80)

#define COLOR_GOOD	0x00000080
#define COLOR_MISPLACED	0xFFFFFF80

#define PIECE_DISPLAY_X	40
#define PIECE_DISPLAY_Y	35

static void	 process_ok(void);
static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static GooCanvasItem *boardRootItem = NULL;
static GooCanvasItem	*boardLogoItem = NULL;


static GooCanvasItem	*superbrain_create_item(GooCanvasItem *parent);
static void		 superbrain_destroy_all_items(void);
static void		 superbrain_next_level(void);
static gint		 item_event(GooCanvasItem *item, GdkEvent *event, Piece *piece);
static void		 mark_pieces(void);

static guint number_of_color    = 0;
static guint number_of_piece    = 0;
static gint current_y_position  = 0;

#define MAX_PIECES	10
static guint solution[MAX_PIECES];

#define LEVEL_MAX_FOR_HELP	4

#define Y_STEP	(PIECE_HEIGHT+PIECE_GAP)

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Super Brain"),
    N_("Tux has hidden several items. Find them again in the correct order"),
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
    NULL,
    NULL,
    NULL
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(superbrain)

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
      gc_bar_set(GC_BAR_OK|GC_BAR_LEVEL);

      superbrain_next_level();

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
      superbrain_destroy_all_items();
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
      superbrain_next_level();
    }
}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "superbrain")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

/* ======================================= */
static void process_ok() {

  mark_pieces();

  if(gamewon)
    gc_bonus_display(gamewon, GC_BONUS_SMILEY);
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* set initial values for the next level */
static void superbrain_next_level()
{
  guint i;
  gboolean selected_color[MAX_COLORS];

  gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			  "superbrain/superbrain_background.jpg");

  gc_bar_set_level(gcomprisBoard);

  superbrain_destroy_all_items();
  gamewon = FALSE;

  /* Select level difficulty : should not go above MAX_PIECES */
  /* number_of_color must be upper than number_of_piece to allow unicity */

  if(gcomprisBoard->level < LEVEL_MAX_FOR_HELP)
    {
      number_of_piece = gcomprisBoard->level + 2;
      number_of_color = gcomprisBoard->level + 4;
    }
  else
    {
      number_of_piece = gcomprisBoard->level - LEVEL_MAX_FOR_HELP + 3;
      number_of_color = gcomprisBoard->level - LEVEL_MAX_FOR_HELP + 5;
    }


  current_y_position = BOARDHEIGHT - 50;

  /* Init a random solution : colors choosen are uniquely choosen */
  for(i=0; i<number_of_color; i++)
    {
      selected_color[i] = FALSE;
    }

  for(i=0; i<number_of_piece; i++)
    {
      guint j;

      j = (guint)g_random_int_range(1, number_of_color);
      while(selected_color[j])
	j = (guint)g_random_int_range(1, number_of_color);

      solution[i] = j;
      selected_color[j] = TRUE;
    }



  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  boardLogoItem = GOO_CANVAS_GROUP(
				     goo_canvas_item_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
							    goo_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  /* The list of the pieces */
  for(i=0; i<number_of_color; i++)
    {
      goo_canvas_item_new (boardLogoItem,
			     goo_canvas_ellipse_get_type(),
			     "x1", (double) PIECE_DISPLAY_X,
			     "y1", (double) PIECE_DISPLAY_Y + i*PIECE_WIDTH*1.2 + (i*PIECE_GAP*1.2),
			     "x2", (double) PIECE_DISPLAY_X + PIECE_WIDTH*1.2,
			     "y2", (double) PIECE_DISPLAY_Y + i*PIECE_WIDTH*1.2 + PIECE_HEIGHT*1.2 + (i*PIECE_GAP*1.2),
			     "fill_color_rgba", colors[i],
			     "stroke-color", "white",
			     "line-width", (double)1,
			     NULL);
    }


  superbrain_create_item(boardRootItem);
}
/* ==================================== */
/* Destroy all the items */
static void superbrain_destroy_all_items()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;

  if(boardLogoItem!=NULL)
    goo_canvas_item_remove(boardLogoItem);

  boardLogoItem = NULL;
}
/* ==================================== */
static GooCanvasItem *superbrain_create_item(GooCanvasItem *parent)
{
  int i, j, x;
  double x1, x2;
  GooCanvasItem *item = NULL;
  Piece *piece = NULL;
  GooCanvasPoints	*points;

  if(current_y_position < SCROLL_LIMIT)
    {
      goo_canvas_item_translate(GNOME_CANVAS_ITEM(boardRootItem), 0.0, (double)Y_STEP);
    }

  x = (BOARDWIDTH - number_of_piece*(PIECE_WIDTH+PIECE_GAP))/2 + PLAYING_AREA_X;

  /* Draw a line to separate cleanly */
  x1 = x + PIECE_WIDTH/2;
  x2 = (BOARDWIDTH + (number_of_piece-1)*(PIECE_WIDTH+PIECE_GAP))/2 - PIECE_WIDTH/2 + PLAYING_AREA_X;

  points = goo_canvas_points_new(2);
  points->coords[0] = (double) x1;
  points->coords[1] = (double) current_y_position + PIECE_HEIGHT + PIECE_GAP/2;
  points->coords[2] = (double) x2;
  points->coords[3] = (double) current_y_position + PIECE_HEIGHT + PIECE_GAP/2;

  goo_canvas_item_new (boardRootItem,
			 goo_canvas_line_get_type (),
			 "points", points,
			 "fill-color", "white",
			 "width_pixels", 1,
			 NULL);

  points->coords[0] = (double) x1 + 2;
  points->coords[1] = (double) current_y_position + PIECE_HEIGHT + PIECE_GAP/2 + 1;
  points->coords[2] = (double) x2 + 2;
  points->coords[3] = (double) current_y_position + PIECE_HEIGHT + PIECE_GAP/2 + 1;

  goo_canvas_item_new (boardRootItem,
			 goo_canvas_line_get_type (),
			 "points", points,
			 "fill-color", "black",
			 "width_pixels", 1,
			 NULL);

  /* Continuing the line */
  //  x1 = (BOARDWIDTH + (number_of_piece+2)*(PIECE_WIDTH+PIECE_GAP))/2 + PLAYING_AREA_X;
  x1 = PLAYING_HELP_X;
  x2 = x1 + number_of_piece*PIECE_WIDTH/2;

  points->coords[0] = (double) x1;
  points->coords[1] = (double) current_y_position + PIECE_HEIGHT + PIECE_GAP/2;
  points->coords[2] = (double) x2;
  points->coords[3] = (double) current_y_position + PIECE_HEIGHT + PIECE_GAP/2;

  goo_canvas_item_new (boardRootItem,
			 goo_canvas_line_get_type (),
			 "points", points,
			 "fill-color", "white",
			 "width_pixels", 1,
			 NULL);

  points->coords[0] = (double) x1 + 2;
  points->coords[1] = (double) current_y_position + PIECE_HEIGHT + PIECE_GAP/2 + 1;
  points->coords[2] = (double) x2 + 2;
  points->coords[3] = (double) current_y_position + PIECE_HEIGHT + PIECE_GAP/2 + 1;

  goo_canvas_item_new (boardRootItem,
			 goo_canvas_line_get_type (),
			 "points", points,
			 "fill-color", "black",
			 "width_pixels", 1,
			 NULL);

  goo_canvas_points_unref(points);

  /* Draw the pieces */
  listPieces = g_list_alloc();

  for(i=0; i<number_of_piece; i++)
    {

      piece = g_new(Piece, 1);
      piece->listitem = g_list_alloc();
      piece->completed = FALSE;
      listPieces = g_list_append(listPieces, piece);

      piece->rootitem = GOO_CANVAS_GROUP(
					   goo_canvas_item_new (parent,
								  goo_canvas_group_get_type (),
								  "x", (double) 0,
								  "y", (double) 0,

								  NULL));


      // Good
      piece->good = goo_canvas_item_new (piece->rootitem,
					   goo_canvas_rect_get_type (),
					   "x1", (double) x + i*PIECE_WIDTH + (i*PIECE_GAP) - PIECE_GAP_GOOD,
					   "y1", (double) current_y_position - PIECE_GAP_GOOD,
					   "x2", (double) x + i*PIECE_WIDTH  + PIECE_WIDTH + (i*PIECE_GAP) + PIECE_GAP_GOOD,
					   "y2", (double) current_y_position + PIECE_HEIGHT + PIECE_GAP_GOOD,
					   "fill_color_rgba", COLOR_GOOD,
					   "stroke-color", "white",
					   "line-width", (double)1,
					   NULL);
      g_object_set (piece->good, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

      // Misplaced
      piece->misplaced = goo_canvas_item_new (piece->rootitem,
						goo_canvas_rect_get_type (),
						"x1", (double) x + i*PIECE_WIDTH + (i*PIECE_GAP) - PIECE_GAP_GOOD,
						"y1", (double) current_y_position - PIECE_GAP_GOOD,
						"x2", (double) x + i*PIECE_WIDTH  + PIECE_WIDTH + (i*PIECE_GAP) + PIECE_GAP_GOOD,
						"y2", (double) current_y_position + PIECE_HEIGHT + PIECE_GAP_GOOD,
						"fill_color_rgba", COLOR_MISPLACED,
						"stroke-color", "white",
						"line-width", (double)1,
						NULL);
      g_object_set (piece->misplaced, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

      for(j=0; j<number_of_color; j++)
	{
	  item = goo_canvas_item_new (piece->rootitem,
					goo_canvas_ellipse_get_type(),
					"x1", (double) x + i*PIECE_WIDTH + (i*PIECE_GAP),
					"y1", (double) current_y_position,
					"x2", (double) x + i*PIECE_WIDTH  + PIECE_WIDTH + (i*PIECE_GAP),
					"y2", (double) current_y_position + PIECE_HEIGHT,
					"fill_color_rgba", colors[j],
					"stroke-color", "white",
					"line-width", (double)1,
					NULL);

	  g_object_set (item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
	  piece->listitem = g_list_append(piece->listitem, item);

	  g_signal_connect(GTK_OBJECT(item), "enter_notify_event", (GtkSignalFunc) item_event, piece);
	}

      piece->selecteditem = 1;
      item = g_list_nth_data(piece->listitem,
			     piece->selecteditem);
      g_object_set (item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);

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
      gc_bonus_end_display(GC_BOARD_FINISHED_RANDOM);
      return;
    }
    gc_sound_play_ogg ("sounds/bonus.wav", NULL);
  }
  superbrain_next_level();
}

/* ==================================== */
static gint
item_event(GooCanvasItem *item, GdkEvent *event, Piece *piece)
{
  GooCanvasItem *newitem;

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      if(!piece->completed)
	{
	  g_object_set (item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

	  switch(event->button.button)
	    {
	    case 1:
	    case 4:
	      piece->selecteditem++;
	      if(piece->selecteditem>=g_list_length(piece->listitem))
		piece->selecteditem = 1;
	      break;
	    case 2:
	    case 3:
	    case 5:
	      piece->selecteditem--;
	      if(piece->selecteditem<=0)
		piece->selecteditem = g_list_length(piece->listitem)-1;
	      break;
	    }

	  newitem = g_list_nth_data(piece->listitem,
				    piece->selecteditem);
	  g_object_set (newitem, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
	}
      break;
    default:
      break;
    }

  return FALSE;
}

static void mark_pieces()
{
  guint i, j;
  guint x;
  Piece	*piece = NULL;
  guint nbgood = 0;
  guint nbmisplaced = 0;
  guint solution_tmp[MAX_PIECES];

  for(i=0; i<number_of_piece; i++)
    {
      solution_tmp[i] = solution[i];
    }

  gamewon = TRUE;

  /* Mark good placed */
  for(i=1; i<g_list_length(listPieces);  i++)
    {
      piece = g_list_nth_data(listPieces, i);
      if(piece->selecteditem == solution_tmp[i-1])
	{
	  if(gcomprisBoard->level<LEVEL_MAX_FOR_HELP)
	    g_object_set (piece->good, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
	  nbgood++;
	  solution_tmp[i-1] = G_MAXINT;
	}
      else
	{
	  gamewon = FALSE;
	}

      piece->completed = TRUE;
    }

  /* Mark misplaced */
  for(i=1; i<=number_of_piece;  i++)
    {
      gboolean done;

      piece = g_list_nth_data(listPieces, i);

      /* Search if this color is elsewhere */
      j = 1;
      done = FALSE;
      do {
	if(piece->selecteditem != solution[i-1] &&
	   piece->selecteditem == solution_tmp[j-1])
	  {
	    nbmisplaced++;
	    solution_tmp[j-1] = G_MAXINT;
	    if(gcomprisBoard->level<LEVEL_MAX_FOR_HELP)
	      g_object_set (piece->misplaced, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
	    done = TRUE;
	  }
      } while (!done && j++!=number_of_piece);
    }

  /* Display the matermind information to the user */
  //  x = (BOARDWIDTH + (number_of_piece+2)*(PIECE_WIDTH+PIECE_GAP))/2;
  x = PLAYING_HELP_X;
  for(i=0; i<nbgood;  i++)
    {
      goo_canvas_item_new (boardRootItem,
			     goo_canvas_ellipse_get_type(),
			     "x1", (double) x + i*PIECE_WIDTH/2 + (i*PIECE_GAP/2),
			     "y1", (double) current_y_position,
			     "x2", (double) x + i*PIECE_WIDTH/2  + PIECE_WIDTH/2 + (i*PIECE_GAP/2),
			     "y2", (double) current_y_position + PIECE_HEIGHT/2,
			     "fill-color", "black",
			     "stroke-color", "white",
			     "line-width", (double)1,
			     NULL);
    }

  for(i=0; i<nbmisplaced;  i++)
    {
      goo_canvas_item_new (boardRootItem,
			     goo_canvas_ellipse_get_type(),
			     "x1", (double) x + i*PIECE_WIDTH/2 + (i*PIECE_GAP/2),
			     "y1", (double) current_y_position + PIECE_HEIGHT/2 + PIECE_GAP/3,
			     "x2", (double) x + i*PIECE_WIDTH/2  + PIECE_WIDTH/2 + (i*PIECE_GAP/2),
			     "y2", (double) current_y_position + PIECE_HEIGHT + PIECE_GAP/3,
			     "fill-color", "white",
			     "stroke-color", "black",
			     "line-width", (double)1,
			     NULL);
    }

  current_y_position -= Y_STEP;

  g_list_free(listPieces);

  superbrain_create_item(boardRootItem);

}
