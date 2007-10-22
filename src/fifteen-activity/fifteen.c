/* gcompris - fifteen.c
 *
 * Copyright (C) 2003 Bruno Coudoin
 *
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
#include <time.h>

#define PIECE_SIZE 50

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem	*fifteen_create_item(GooCanvasItem *parent);
static void		 fifteen_destroy_all_items(void);
static void		 fifteen_next_level(void);

static void		 free_stuff (GtkObject *obj, gpointer data);
static gboolean		 piece_event (GooCanvasItem  *item,
				      GooCanvasItem  *target,
				      GdkEventButton *event,
				      gpointer data);
static void		 scramble (GooCanvasItem **board, guint number_of_scrambles);
static char		*get_piece_color (int piece);

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "The fifteen game",
    "Move the items one by one, to rearrange them in increasing order",
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

GET_BPLUGIN_INFO(fifteen)

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
      gc_bar_set(GC_BAR_LEVEL);

      fifteen_next_level();

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
      fifteen_destroy_all_items();
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
      fifteen_next_level();
    }
}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "fifteen")==0)
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
static void fifteen_next_level()
{
  gchar *img;

  img = gc_skin_image_get("gcompris-bg.jpg");
  gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			  img);
  g_free(img);

  gc_bar_set_level(gcomprisBoard);

  fifteen_destroy_all_items();
  gamewon = FALSE;

  /* Create the level */
  fifteen_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));


}
/* ==================================== */
/* Destroy all the items */
static void fifteen_destroy_all_items()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}
/* ==================================== */
static GooCanvasItem *fifteen_create_item(GooCanvasItem *parent)
{
  int i;
  int x, y;
  GooCanvasItem **board;
  GooCanvasItem *text;
  char buf[20];
  GdkPixbuf *pixmap = NULL;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
				   NULL);

  goo_canvas_item_translate(boardRootItem,
			    (BOARDWIDTH-(4*PIECE_SIZE))/2,
			    (BOARDHEIGHT-(4*PIECE_SIZE))/2);

  /* Load the cute frame */
  pixmap = gc_pixmap_load("fifteen/fifteen_frame.png");

  goo_canvas_image_new (boardRootItem,
			pixmap,
			-1*((gdk_pixbuf_get_width(pixmap)-(4*PIECE_SIZE))/2),
			-1*((gdk_pixbuf_get_height(pixmap)-(4*PIECE_SIZE))/2)-2,
			NULL);
  gdk_pixbuf_unref(pixmap);


  board = g_new (GooCanvasItem *, 16);
  g_object_set_data (G_OBJECT (boardRootItem), "board", board);
  g_signal_connect (boardRootItem, "destroy",
		    G_CALLBACK (free_stuff),
		    board);

  for (i = 0; i < 15; i++) {
    y = i / 4;
    x = i % 4;

    board[i] = goo_canvas_group_new (boardRootItem, NULL);

    goo_canvas_item_translate(board[i],
			      (x * PIECE_SIZE),
			      (y * PIECE_SIZE));

    goo_canvas_rect_new (board[i],
			 0.0,
			 0.0,
			 PIECE_SIZE,
			 PIECE_SIZE,
			 "fill-color", get_piece_color (i),
			 "stroke-color", "black",
			 NULL);

    sprintf (buf, "%d", i + 1);

    text = goo_canvas_text_new (board[i],
				buf,
				(double) PIECE_SIZE / 2.0,
				(double) PIECE_SIZE / 2.0,
				-1,
				GTK_ANCHOR_CENTER,
				"font", gc_skin_font_board_medium,
				"fill-color", "black",
				NULL);

    g_object_set_data (G_OBJECT (board[i]), "piece_num", GINT_TO_POINTER (i));
    g_object_set_data (G_OBJECT (board[i]), "piece_pos", GINT_TO_POINTER (i));
    g_object_set_data (G_OBJECT (board[i]), "text", text);
    g_signal_connect (board[i], "button-press-event",
		      G_CALLBACK (piece_event),
		      NULL);
  }

  board[15] = NULL;

  /* Select level difficulty */
  switch(gcomprisBoard->level)
    {
    case 1:
      scramble(board, 10);
      break;
    case 2:
      scramble(board, 50);
      break;
    case 3:
    case 4:
      scramble(board, 100);
      break;
    case 5:
      scramble(board, 150);
      break;
    default:
      scramble(board, 256);
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
  fifteen_next_level();
}

/*==================================================*/
/*   Code taken from libgnomecanvas demo fifteen    */


static void
free_stuff (GtkObject *obj, gpointer data)
{
  g_free (data);
}

static void
test_win (GooCanvasItem **board)
{
  int i;

  for (i = 0; i < 15; i++)
    if (!board[i] || (GPOINTER_TO_INT (g_object_get_data (G_OBJECT (board[i]), "piece_num")) != i))
      return;


  gamewon = TRUE;
  fifteen_destroy_all_items();
  gc_bonus_display(gamewon, GC_BONUS_SMILEY);

}

static char *
get_piece_color (int piece)
{
  static char buf[50];
  int x, y;
  int r, g, b;

  y = piece / 4;
  x = piece % 4;

  r = ((4 - x) * 255) / 4;
  g = ((4 - y) * 255) / 4;
  b = 128;

  sprintf (buf, "#%02x%02x%02x", r, g, b);

  return buf;
}

static gboolean
piece_event (GooCanvasItem  *item,
	     GooCanvasItem  *target,
	     GdkEventButton *event,
	     gpointer data)
{
  GooCanvasItem **board;
  GooCanvasItem *text;
  int num, pos, newpos;
  int x, y;
  double dx = 0.0, dy = 0.0;
  int move;

  board = g_object_get_data (G_OBJECT (goo_canvas_item_get_parent(item)), "board");
  num = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item), "piece_num"));
  pos = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item), "piece_pos"));
  text = g_object_get_data (G_OBJECT (item), "text");

  switch (event->type) {
  case GDK_ENTER_NOTIFY:
    g_object_set (text,
		  "fill-color", "white",
		  NULL);
    break;

  case GDK_LEAVE_NOTIFY:
    g_object_set (text,
		  "fill-color", "black",
		  NULL);
    break;

  case GDK_BUTTON_PRESS:
    y = pos / 4;
    x = pos % 4;

    move = TRUE;

    if ((y > 0) && (board[(y - 1) * 4 + x] == NULL)) {
      dx = 0.0;
      dy = -1.0;
      y--;
    } else if ((y < 3) && (board[(y + 1) * 4 + x] == NULL)) {
      dx = 0.0;
      dy = 1.0;
      y++;
    } else if ((x > 0) && (board[y * 4 + x - 1] == NULL)) {
      dx = -1.0;
      dy = 0.0;
      x--;
    } else if ((x < 3) && (board[y * 4 + x + 1] == NULL)) {
      dx = 1.0;
      dy = 0.0;
      x++;
    } else
      move = FALSE;

    if (move) {
      newpos = y * 4 + x;
      board[pos] = NULL;
      board[newpos] = item;
      g_object_set_data (G_OBJECT (item), "piece_pos", GINT_TO_POINTER (newpos));
      goo_canvas_item_translate (item, dx * PIECE_SIZE, dy * PIECE_SIZE);

      test_win (board);
    }

    break;

  default:
    break;
  }

  return FALSE;
}

static void
scramble (GooCanvasItem **board, guint number_of_scrambles)
{
  int i;
  int pos, oldpos;
  int dir;
  int x, y;

  /* g_random are initialised in gcompris launch */
  /* srand (time (NULL)); */

  /* First, find the blank spot */

  for (pos = 0; pos < 16; pos++)
    if (board[pos] == NULL)
      break;

  /* "Move the blank spot" around in order to scramble the pieces */

  for (i = 0; i < number_of_scrambles; i++) {
  retry_scramble:
    dir = g_random_int () % 4;

    x = y = 0;

    if ((dir == 0) && (pos > 3)) /* up */
      y = -1;
    else if ((dir == 1) && (pos < 12)) /* down */
      y = 1;
    else if ((dir == 2) && ((pos % 4) != 0)) /* left */
      x = -1;
    else if ((dir == 3) && ((pos % 4) != 3)) /* right */
      x = 1;
    else
      goto retry_scramble;

    oldpos = pos + y * 4 + x;
    board[pos] = board[oldpos];
    board[oldpos] = NULL;
    g_object_set_data (G_OBJECT (board[pos]), "piece_pos", GINT_TO_POINTER (pos));
    goo_canvas_item_translate (board[pos], -x * PIECE_SIZE, -y * PIECE_SIZE);
    pos = oldpos;
  }
}

