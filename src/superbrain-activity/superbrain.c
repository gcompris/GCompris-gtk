/* gcompris - superbrain.c
 *
 * Copyright (C) 2002, 2008 Bruno Coudoin
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

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

/*
 * Contains the points that represents the anchors
 */
typedef struct {
  GooCanvasItem	*rootitem;
  GList		*listitem;
  guint		 selecteditem;
  GooCanvasItem	*good;
  GooCanvasItem	*misplaced;
} Piece;

/* The active list of pieces */
static GList * listPieces = NULL;

static guint colors[] =
  {
    0x0000FFC0,
    0x00FF00C0,
    0xFF0000C0,
    0x00FFFFC0,
    0xFF00FFC0,
    0xFFFF00C0,
    0x00007FC0,
    0x007F00C0,
    0x7F0000C0,
    0x7F007FC0,
  };

#define MAX_COLORS	10

#define PIECE_WIDTH	9.0
#define PIECE_HEIGHT	9.0
#define PIECE_GAP	20.0
#define PIECE_GAP_GOOD	5
#define SCROLL_LIMIT	20
#define PLAYING_AREA_X	190
#define PLAYING_HELP_X	(BOARDWIDTH-80)

#define COLOR_GOOD	0x000000C0
#define COLOR_MISPLACED	0xFFFFFFC0

#define PIECE_DISPLAY_X	50.0
#define PIECE_DISPLAY_Y	47.0

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
static gboolean		 item_event (GooCanvasItem  *item,
				     GooCanvasItem  *target,
				     GdkEventButton *event,
				     Piece *piece);
static void		 mark_pieces(void);
static void		 listPiecesClear();

static guint number_of_color    = 0;
static guint number_of_piece    = 0;
static double current_y_position  = 0;

#define MAX_PIECES	10
static guint solution[MAX_PIECES];

/* After this level, we provide less feedback to the user */
#define LEVEL_MAX_FOR_HELP	4

#define Y_STEP	(PIECE_HEIGHT*2+PIECE_GAP)

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
    NULL,
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
      gcomprisBoard->number_of_sublevel=6; /* Go to next level after this number of 'play' */
      gc_bar_set(GC_BAR_LEVEL);
      gc_bar_location(30, -1, -1);

      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			"superbrain/superbrain_background.svgz");

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

      j = (guint)g_random_int_range(0, number_of_color);
      while(selected_color[j])
	j = (guint)g_random_int_range(0, number_of_color);

      solution[i] = j;
      selected_color[j] = TRUE;
    }

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  boardLogoItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);

  /* The OK Button */
  GooCanvasItem *item = goo_canvas_svg_new (boardLogoItem,
					    gc_skin_rsvg_get(),
					    "svg-id", "#OK",
					    NULL);
  SET_ITEM_LOCATION(item, 270, 50);
  g_signal_connect(item, "button_press_event",
		   (GtkSignalFunc) process_ok, NULL);
  gc_item_focus_init(item, NULL);

  /* The list of the pieces */
  for(i=0; i<number_of_color; i++)
    {
      goo_canvas_ellipse_new (boardLogoItem,
			      PIECE_DISPLAY_X,
			      PIECE_DISPLAY_Y + i*((PIECE_WIDTH*1.5)*2 + PIECE_GAP/2),
			      PIECE_WIDTH * 1.5,
			      PIECE_WIDTH * 1.5,
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
  listPiecesClear();

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
  int i, j;
  double x;
  double x1, x2;
  GooCanvasItem *item = NULL;
  Piece *piece = NULL;

  listPiecesClear();

  if(current_y_position < SCROLL_LIMIT)
    {
      goo_canvas_item_translate(boardRootItem, 0.0, Y_STEP);
    }

  x = (BOARDWIDTH - number_of_piece*(PIECE_WIDTH*2+PIECE_GAP))/2 + PLAYING_AREA_X;

  /* Draw a line to separate cleanly */
  x1 = x + PIECE_WIDTH;
  x2 = (BOARDWIDTH + (number_of_piece-1)*(PIECE_WIDTH*2+PIECE_GAP))/2 - PIECE_WIDTH + PLAYING_AREA_X;

  goo_canvas_polyline_new (boardRootItem, FALSE, 2,
			   x1, current_y_position + PIECE_HEIGHT + PIECE_GAP/2,
			   x2, current_y_position + PIECE_HEIGHT + PIECE_GAP/2,
			   "stroke-color", "white",
			   "line-width", 1.0,
			   NULL);


  goo_canvas_polyline_new (boardRootItem, FALSE, 2,
			   x1 + 2, current_y_position + PIECE_HEIGHT + PIECE_GAP/2 + 1,
			   x2 + 2, current_y_position + PIECE_HEIGHT + PIECE_GAP/2 + 1,
			   "stroke-color", "black",
			   "line-width", 1.0,
			   NULL);

  /* Continuing the line */
  x1 = PLAYING_HELP_X;
  x2 = x1 + number_of_piece*PIECE_WIDTH;


  goo_canvas_polyline_new (boardRootItem, FALSE, 2,
			   x1, current_y_position + PIECE_HEIGHT + PIECE_GAP/2,
			   x2, current_y_position + PIECE_HEIGHT + PIECE_GAP/2,
			   "stroke-color", "white",
			   "line-width", 1.0,
			   NULL);


  goo_canvas_polyline_new (boardRootItem, FALSE, 2,
			   x1 + 2, current_y_position + PIECE_HEIGHT + PIECE_GAP/2 + 1,
			   x2 + 2, current_y_position + PIECE_HEIGHT + PIECE_GAP/2 + 1,
			   "stroke-color", "black",
			   "line-width", 1.0,
			   NULL);

  /* Draw the pieces */

  for(i=0; i<number_of_piece; i++)
    {

      piece = g_new(Piece, 1);
      piece->listitem = NULL;
      listPieces = g_list_append(listPieces, piece);

      piece->rootitem = goo_canvas_group_new (parent,
					      NULL);


      // Good
      piece->good = goo_canvas_rect_new (piece->rootitem,
					 x + i*(PIECE_WIDTH*2 + PIECE_GAP) - PIECE_WIDTH - PIECE_GAP_GOOD,
					 current_y_position - PIECE_HEIGHT - PIECE_GAP_GOOD,
					 PIECE_WIDTH*2 + PIECE_GAP_GOOD*2,
					 PIECE_HEIGHT*2 + PIECE_GAP_GOOD*2,
					 "fill_color_rgba", COLOR_GOOD,
					 "stroke-color", "white",
					 "line-width", 1.0,
					 NULL);
      g_object_set (piece->good, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

      // Misplaced
      piece->misplaced = goo_canvas_rect_new (piece->rootitem,
					      x + i*(PIECE_WIDTH*2 + PIECE_GAP) - PIECE_WIDTH - PIECE_GAP_GOOD,
					      current_y_position - PIECE_HEIGHT - PIECE_GAP_GOOD,
					      PIECE_WIDTH*2 + PIECE_GAP_GOOD*2,
					      PIECE_HEIGHT*2 + PIECE_GAP_GOOD*2,
					      "fill_color_rgba", COLOR_MISPLACED,
					      "stroke-color", "black",
					      "line-width", 1.0,
					      NULL);
      g_object_set (piece->misplaced, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

      for(j=0; j<number_of_color; j++)
	{
	  item = goo_canvas_ellipse_new (piece->rootitem,
					 x + i*(PIECE_WIDTH*2 + PIECE_GAP),
					 current_y_position,
					 PIECE_WIDTH,
					 PIECE_HEIGHT,
					 "fill_color_rgba", colors[j],
					 "stroke-color", "white",
					 "line-width", (double)1,
					 NULL);
	  gc_item_focus_init(item, NULL);

	  g_object_set (item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
	  piece->listitem = g_list_append(piece->listitem, item);

	  g_signal_connect(item, "button-press-event",
			   (GtkSignalFunc) item_event, piece);
	}

      piece->selecteditem = 0;
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
    if(gcomprisBoard->level>gcomprisBoard->maxlevel)
      gcomprisBoard->level = gcomprisBoard->maxlevel;

    gc_sound_play_ogg ("sounds/bonus.wav", NULL);
  }
  superbrain_next_level();
}

/* ==================================== */
static gboolean item_event (GooCanvasItem  *item,
			    GooCanvasItem  *target,
			    GdkEventButton *event,
			    Piece *piece)
{
  GooCanvasItem *newitem;
  guint j;

  if(board_paused)
    return FALSE;

  switch(event->button)
    {
    case 1:
    case 4:
      piece->selecteditem++;
      if(piece->selecteditem >= g_list_length(piece->listitem))
	piece->selecteditem = 0;
      break;
    case 2:
    case 3:
    case 5:
      if(piece->selecteditem == 0)
	piece->selecteditem = g_list_length(piece->listitem)-1;
      else
	piece->selecteditem--;
      break;
    }

  for(j=0; j<g_list_length(piece->listitem);  j++)
    {
      g_object_set (g_list_nth_data(piece->listitem, j),
		    "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
    }

	  newitem = g_list_nth_data(piece->listitem,
			    piece->selecteditem);
  g_object_set (newitem, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);

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
  for(i=0; i<g_list_length(listPieces);  i++)
    {
      piece = g_list_nth_data(listPieces, i);
      if(piece->selecteditem == solution_tmp[i])
	{
	  if(gcomprisBoard->level<LEVEL_MAX_FOR_HELP)
	    g_object_set (piece->good, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
	  nbgood++;
	  solution_tmp[i] = G_MAXINT;
	}
      else
	{
	  gamewon = FALSE;
	}

    }

  /* Mark misplaced */
  for(i=0; i<g_list_length(listPieces);  i++)
    {
      gboolean done;

      piece = g_list_nth_data(listPieces, i);

      /* Search if this color is elsewhere */
      j = 0;
      done = FALSE;
      do {
	if(piece->selecteditem != solution[i] &&
	   piece->selecteditem == solution_tmp[j])
	  {
	    nbmisplaced++;
	    solution_tmp[j] = G_MAXINT;
	    if(gcomprisBoard->level<LEVEL_MAX_FOR_HELP)
	      g_object_set (piece->misplaced, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
	    done = TRUE;
	  }
      } while (!done && ++j!=number_of_piece);
    }

  /* Display the matermind information to the user */
  x = PLAYING_HELP_X;
  for(i=0; i<nbgood;  i++)
    {
      goo_canvas_ellipse_new (boardRootItem,
			      x + i*(PIECE_WIDTH + PIECE_GAP/2),
			      current_y_position,
			      PIECE_WIDTH/2,
			      PIECE_HEIGHT/2,
			      "fill-color", "black",
			      "stroke-color", "white",
			      "line-width", (double)1,
			      NULL);
    }

  for(i=0; i<nbmisplaced;  i++)
    {
      goo_canvas_ellipse_new (boardRootItem,
			      x + i*(PIECE_WIDTH + PIECE_GAP/2),
			      current_y_position + PIECE_HEIGHT/2 + PIECE_GAP/3,
			      PIECE_WIDTH/2,
			      PIECE_HEIGHT/2,
			      "fill-color", "white",
			      "stroke-color", "black",
			      "line-width", (double)1,
			      NULL);
    }

  current_y_position -= Y_STEP;

  superbrain_create_item(boardRootItem);

}

void listPiecesClear()
{
  guint i, j;
  for(i=0; i<g_list_length(listPieces);  i++)
    {
      Piece *piece = g_list_nth_data(listPieces, i);

      for(j=0; j<g_list_length(piece->listitem);  j++)
	{
	  GooCanvasItem *item = g_list_nth_data(piece->listitem, j);
	  g_signal_handlers_disconnect_by_func(item,
					       (GtkSignalFunc) item_event, piece);
	  gc_item_focus_remove(item, NULL);
	}
      g_free(piece);
    }
  g_list_free(listPieces);
  listPieces = NULL;
}

