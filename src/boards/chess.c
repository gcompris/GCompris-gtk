/* gcompris - chess.c
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "chess_notation.h"

#include "gcompris/gcompris.h"

#if defined _WIN32 || defined __WIN32__
# undef WIN32   /* avoid warning on mingw32 */
# define WIN32
#endif

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static GPid	 gnuchess_pid;
static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static void	 engine_local_destroy (GPid gnuchess_pid);

static gboolean  engine_local_cb     (GIOChannel *source,
				      GIOCondition condition,
				      gpointer data);
static gboolean  engine_local_err_cb (GIOChannel *source,
				      GIOCondition condition,
				      gpointer data);
static void	 display_white_turn  (gboolean whiteturn);
static void	 display_info	     (gchar *info);
static int	 get_square_from_coord (double x, double y);

#define CHESSGC_BOARD_X	50
#define CHESSGC_BOARD_Y	20
#define SQUARE_WIDTH	60
#define SQUARE_HEIGHT	60
#define WHITE_COLOR	0xFFFF99FF
#define BLACK_COLOR	0x9999FFFF
#define WHITE_COLOR_H	0x99FF99FF
#define BLACK_COLOR_H	0x99FF99FF

#define TURN_X		(BOARDWIDTH-(BOARDWIDTH-(CHESSGC_BOARD_X+(SQUARE_WIDTH*8)))/2)
#define TURN_Y		(CHESSGC_BOARD_Y+15)

#define INFO_X		TURN_X
#define INFO_Y		(TURN_Y+40)

/* Game Type */
#define COMPUTER	1
#define PARTYEND	2
#define MOVELEARN	3

static char gameType = COMPUTER;

static GnomeCanvasGroup *boardRootItem = NULL;

static GIOChannel *read_chan;
static GIOChannel *write_chan;

static gint read_cb;
static gint err_cb;

static Position *position;

/*
 * Contains the squares structure
 */
typedef struct {
  GnomeCanvasItem	*square_item;
  GnomeCanvasItem	*piece_item;
  Square		 square;
} GSquare;

static GSquare  *currentHighlightedGsquare;

static GnomeCanvasItem	*turn_item = NULL;
static GnomeCanvasItem	*info_item = NULL;

/* Need more space to fit notation.h definition */
static GSquare *chessboard[100];

static GnomeCanvasItem	*chess_create_item(GnomeCanvasGroup *parent);
static void		 chess_destroy_all_items(void);
static void		 chess_next_level(void);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint		 item_event_black(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gboolean		 start_child (char        *cmd,
				      GIOChannel **read_chan,
				      GIOChannel **write_chan,
				      GPid	 *gnuchess_pid);

static void		 write_child (GIOChannel  *write_chan,
				      char        *format,
				      ...);


/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Learning Chess",
    "Play chess against tux in a learning mode",
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

GET_BPLUGIN_INFO(chess)

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

void gnuchess_died(int signum)
{
  gnuchess_pid = 0;
  gc_dialog(_("Error: The external program gnuchess died unexpectedly"), gc_board_stop);
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{

  gnuchess_pid = 0;
  gchar *gnuchess_bin = GNUCHESS;

#ifndef WIN32
  signal(SIGTRAP, gnuchess_died);
  signal(SIGPIPE, gnuchess_died);
  if (!g_file_test (gnuchess_bin, G_FILE_TEST_EXISTS))
    {
      /* check in same bin/ as we are */
      extern gchar *exec_prefix;
      gnuchess_bin = g_build_filename( exec_prefix, "bin", "gnuchess", NULL);
      if (!g_file_test (gnuchess_bin, G_FILE_TEST_EXISTS)) {
	gc_dialog(_("Error: The external program gnuchess is required\nto play chess in gcompris.\n"
		    "Find this program on http://www.rpmfind.net or in your\nGNU/Linux distribution\n"
		    "And check it is located here: "GNUCHESS),
		  gc_board_stop);
	return;
      }
  }

  g_warning("GNUCHESS found %s", gnuchess_bin);
#endif

  if(agcomprisBoard!=NULL)
    {

      gcomprisBoard=agcomprisBoard;

      /* Default mode */
      if(!gcomprisBoard->mode)
	gameType=COMPUTER;
      else if(g_strncasecmp(gcomprisBoard->mode, "computer", 1)==0)
	gameType=COMPUTER;
      else if(g_strncasecmp(gcomprisBoard->mode, "partyend", 1)==0)
	gameType=PARTYEND;
      else if(g_strncasecmp(gcomprisBoard->mode, "movelearn", 1)==0)
	gameType=MOVELEARN;

      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=1;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */

      switch(gameType)
	{
	case PARTYEND:
	case MOVELEARN:
	  gcomprisBoard->maxlevel=9;
	  gc_bar_set(GC_BAR_LEVEL);
	  break;
	default:
	  gc_bar_set(0);
	}

      if(start_child (gnuchess_bin, &read_chan,
		      &write_chan, &gnuchess_pid)==FALSE)
	{
	  gc_dialog(_("Error: The external program gnuchess is mandatory\n"
		      "to play chess in gcompris.\n"
		      "First install it, and check it is in "GNUCHESS), gc_board_stop);
	  return;
	}

      read_cb = g_io_add_watch (read_chan, G_IO_IN|G_IO_PRI,
				engine_local_cb, NULL);
      err_cb = g_io_add_watch (read_chan, G_IO_HUP,
			       engine_local_err_cb, NULL);

      write_child (write_chan, "xboard\n");
      write_child (write_chan, "protover 2\n");
      write_child (write_chan, "post\n");
      write_child (write_chan, "easy\n");
      write_child (write_chan, "level 100 1 0\n");
      write_child (write_chan, "depth 1\n");
      write_child (write_chan, "time 500\n");

      chess_next_level();

      gamewon = FALSE;
      pause_board(FALSE);

    }
}

/* ======================================= */
static void end_board ()
{
#ifndef WIN32
  signal(SIGTRAP, NULL);
  signal(SIGPIPE, NULL);
#endif
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      chess_destroy_all_items();
    }
  gcomprisBoard = NULL;

  turn_item     = NULL;
  info_item     = NULL;


  engine_local_destroy(gnuchess_pid);
}

/* ======================================= */
static void set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      chess_next_level();
    }
}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "chess")==0)
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
static void chess_next_level()
{
  register Square square;
  register gshort rank;
  gchar *img;

  img = gc_skin_image_get("gcompris-bg.jpg");
  gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			  img);
  g_free(img);

  gc_bar_set_level(gcomprisBoard);

  chess_destroy_all_items();
  gamewon = FALSE;

  /* Initial position */
  position = POSITION (position_new_initial ());

  switch(gameType)
    {
    case PARTYEND:
      position_set_initial_partyend(position, gcomprisBoard->level);
      break;
    case MOVELEARN:
      position_set_initial_movelearn(position, gcomprisBoard->level);
      break;
    }
  /* Init our internal chessboard */
  for (rank = 1; rank <= 8; rank++) {
    for (square = A1 + ((rank - 1) * 10);
	 square <= H1 + ((rank - 1) * 10);
	 square++) {

	GSquare *gsquare;

	gsquare = g_malloc(sizeof(GSquare));

	chessboard[square] = gsquare;
	chessboard[square]->piece_item = NULL;
	chessboard[square]->square = square;

    }
  }

  /* Try the next level */
  chess_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}

/* ==================================== */
/* Destroy all the items */
static void chess_destroy_all_items()
{
  register Square square;
  register gshort rank;

  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
  turn_item     = NULL;
  info_item     = NULL;

  if(position!=NULL)
    gtk_object_destroy (GTK_OBJECT (position));

  position = NULL;

  for (rank = 1; rank <= 8; rank++) {
    for (square = A1 + ((rank - 1) * 10);
	 square <= H1 + ((rank - 1) * 10);
	 square++) {

      if(chessboard[square]!=NULL)
	{
	  g_free(chessboard[square]);
	  chessboard[square]=NULL;
	}
    }
  }
}

/* ==================================== */
static GnomeCanvasItem *
chess_create_item(GnomeCanvasGroup *parent)
{
  guint color;
  GnomeCanvasItem *item = NULL;
  Square square;
  Piece piece;
  gshort rank;
  gboolean white_side = TRUE;
  guint empty_case = 0;
  gboolean need_slash = TRUE;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,

							    NULL));

  for (rank = 1; rank <= 8; rank++) {
    for (square = A1 + ((rank - 1) * 10);
	 square <= H1 + ((rank - 1) * 10);
	 square++) {
      int x,y;

      x = square % 10 - 1;
      y = square / 10 - 2;

      color=((x+y)%2?BLACK_COLOR:WHITE_COLOR);

      item  = gnome_canvas_item_new (boardRootItem,
				     gnome_canvas_rect_get_type (),
				     "x1", (double) CHESSGC_BOARD_X + (x * SQUARE_WIDTH),
				     "y1", (double) CHESSGC_BOARD_Y + ((7-y) * SQUARE_HEIGHT),
				     "x2", (double) CHESSGC_BOARD_X + (x * SQUARE_WIDTH) + SQUARE_WIDTH -1,
				     "y2", (double)  CHESSGC_BOARD_Y + ((7-y) * SQUARE_HEIGHT) + SQUARE_HEIGHT -1,
				     "fill_color_rgba", color,
				     "outline_color", "black",
				     "width_units", (double)2,
				     NULL);
      chessboard[square]->square_item = item;
    }
  }

  /* Enter the gnuchess edit mode */
  write_child (write_chan, "force\n");
  write_child (write_chan, "new\n");
  write_child (write_chan, "setboard ");

  empty_case = 0;
  need_slash = FALSE;

  /* Display the pieces */
  for (rank = 8; rank >= 1; rank--) {
    for (square = A1 + ((rank - 1) * 10);
	 square <= H1 + ((rank - 1) * 10);
	 square++)
      {
	GdkPixbuf *pixmap = NULL;
	char *str;
	gint x, y;

	piece = position->square[square];

	x = square % 10 - 1;
	y = square / 10 - 2;

	//	printf ( "%c%s\n", piece_to_ascii(piece), san);

	if(need_slash)
	  {
	    write_child (write_chan, "/");
	    need_slash = FALSE;
	  }

	if(piece!=NONE)
	  {

	    if( (white_side && BPIECE(piece)) ||
		(!white_side && WPIECE(piece)) )
	      {
		white_side = !white_side;
		//		write_child (write_chan, "c\n");
	      }
	    if(empty_case>0)
	      write_child (write_chan, "%d", empty_case);

	    empty_case=0;

	    write_child (write_chan, "%c", piece_to_ascii(piece));
	  }
	else
	  {
	    empty_case++;
	  }

	if(x==7)
	  {
	    if(empty_case>0)
	      write_child (write_chan, "%d", empty_case);

	    empty_case=0;

	    need_slash = TRUE;
	  }

	//	printf("square=%d piece=%d x=%d y=%d\n", square, piece, x, y);
	if(piece != EMPTY)
	  {
	    if(BPIECE(piece))
	      str = g_strdup_printf("chess/B%c.png", piece_to_ascii(piece));
	    else
	      str = g_strdup_printf("chess/W%c.png", piece_to_ascii(piece));

	    pixmap = gc_pixmap_load(str);
	    //	    g_warning("loading piece %s\n",   str);
	    g_free(str);
	    item = gnome_canvas_item_new (boardRootItem,
					  gnome_canvas_pixbuf_get_type (),
					  "pixbuf", pixmap,
					  "x", (double)CHESSGC_BOARD_X + (x * SQUARE_WIDTH) +
					  (guint)((SQUARE_WIDTH-gdk_pixbuf_get_width(pixmap))/2),
					  "y", (double) CHESSGC_BOARD_Y + ((7-y) * SQUARE_HEIGHT) +
					  (guint)((SQUARE_HEIGHT-gdk_pixbuf_get_height(pixmap))/2),
					  NULL);

	    chessboard[square]->piece_item = item;
	    if(WPIECE(piece))
	      gtk_signal_connect(GTK_OBJECT(item), "event",
				 (GtkSignalFunc) item_event, NULL);
	    else
	      gtk_signal_connect(GTK_OBJECT(item), "event",
				 (GtkSignalFunc) item_event_black, NULL);

	    gdk_pixbuf_unref(pixmap);
	  }
      }
  }

  /* Quit the gnuchess edit mode */
  write_child (write_chan, " w KQkq\n");

  display_white_turn(TRUE);

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
    gc_sound_play_ogg ("sounds/bonus.ogg", NULL);
  }
  chess_next_level();
}

static void display_white_turn(gboolean whiteturn)
{

  if(turn_item == NULL)
    {

      turn_item = gnome_canvas_item_new (boardRootItem,
					 gnome_canvas_text_get_type (),
					 "text",       " ",
					 "font",       gc_skin_font_board_big,
					 "x", (double) TURN_X,
					 "y", (double) TURN_Y,
					 "anchor",     GTK_ANCHOR_CENTER,
					 "fill_color_rgba", gc_skin_color_content,
					 NULL);
    }

  gnome_canvas_item_set(turn_item, "text", (whiteturn ? _("White's Turn") : _("Black's Turn")),
			NULL);
}


static void display_info(gchar *info)
{

  if(info_item == NULL)
    {
      info_item = gnome_canvas_item_new (boardRootItem,
					 gnome_canvas_text_get_type (),
					 "text",       " ",
					 "font",       gc_skin_font_board_big,
					 "x", (double) INFO_X,
					 "y", (double) INFO_Y,
					 "anchor",     GTK_ANCHOR_CENTER,
					 "fill_color_rgba", gc_skin_color_subtitle,
					 NULL);
    }

  gnome_canvas_item_set(info_item, "text", info,
			NULL);
}


/*
 * Move a piece to the given position using chess_notation notation
 *
 */
static void move_piece_to(Square from, Square to)
{
  GSquare *source_square, *dest_square;
  GnomeCanvasItem *item;
  guint x, y;
  double ofset_x, ofset_y;
  double x1, y1, x2, y2;
  Piece piece = NONE;


  g_warning("move_piece_to from=%d to=%d\n", from, to);

  source_square = chessboard[from];
  item = source_square->piece_item;
  source_square->piece_item = NULL;

  if(item == NULL)
    {
      g_warning("Warning: Problem in chess.c, bad move request in move_piece_to\n");
      return;
    }

  /* If we are promoting a pawn */
  if(position_get_color_to_move(position)==BLACK)
    {
      if (to & 128) {
	piece = ((to & 127) >> 3 ) + WP - 1;
	to = (to & 7) + A8;
	printf("  Promoting white piece to %d\n", piece);
      }
    }
  else
    {
      if (to & 128) {
	piece = ((to & 127) >> 3) + BP - 1;
	to = (to & 7) + A1;
	printf("  Promoting black piece to %d\n", piece);
      }
    }

  /* Show the moved piece */
  gnome_canvas_item_set(source_square->square_item,
			"outline_color",
			(BPIECE(position->square[to])?"red":"blue"),
			NULL);

  display_white_turn(BPIECE(position->square[to]));

  x = to % 10;
  y = to / 10 -1;

  g_warning("   move_piece_to to    x=%d y=%d\n", x, y);

  dest_square = chessboard[to];

  /* Show the moved piece */
  gnome_canvas_item_set(dest_square->square_item,
			"outline_color",
			(BPIECE(position->square[to])?"red":"blue"),
			NULL);

  if(dest_square->piece_item != NULL)
    /* Oups I loose a piece */
    gtk_object_destroy (GTK_OBJECT(dest_square->piece_item));

  dest_square->piece_item    = item;

  /* Find the ofset to move the piece */
  gnome_canvas_item_get_bounds  (item,
				 &x1,
				 &y1,
				 &x2,
				 &y2);


  ofset_x = (CHESSGC_BOARD_X + SQUARE_WIDTH  * (x-1)) - x1 + (SQUARE_WIDTH  - (x2-x1))/2;
  ofset_y = (CHESSGC_BOARD_Y + SQUARE_HEIGHT * (8-y)) - y1 + (SQUARE_HEIGHT - (y2-y1))/2;

  gnome_canvas_item_move(item, ofset_x, ofset_y);

  /* Manage rock */
  if(position->square[to]==WK && from==E1 && to==C1)
    move_piece_to(A1, D1);
  else if(position->square[to]==WK && from==E1 && to==G1)
    move_piece_to(H1, F1);
  else if(position->square[to]==BK && from==E8 && to==C8)
    move_piece_to(A8, D8);
  else if(position->square[to]==BK && from==E8 && to==G8)
    move_piece_to(H8, F8);

  /* Manage promotion */
  if(piece != NONE)
    {
      GdkPixbuf *pixmap = NULL;
      char *str;
      g_warning("  WARNING promoting a pawn from=%d to=%d piece=%d\n", from, to, piece);
      g_warning("  piece_to_ascii returns %c\n", piece_to_ascii(piece));

      if(BPIECE(piece))
	str = g_strdup_printf("chess/B%c.png", piece_to_ascii(piece));
      else
	str = g_strdup_printf("chess/W%c.png", piece_to_ascii(piece));

      pixmap = gc_pixmap_load(str);
      g_free(str);
      g_warning("loading piece %c\n",  piece_to_ascii(piece));
      gnome_canvas_item_set (dest_square->piece_item,
			     "pixbuf", pixmap,
			     NULL);

    }

  /* Display check info */
  if(position_white_king_attack(position))
    display_info(_("White checks"));
  else if(position_black_king_attack(position))
    display_info(_("Black checks"));
  else
    display_info(" ");

}

/*
 * Return a square suitable for position functions
 */
static int
get_square_from_coord (double x, double y)
{

  return (A1 + (Square) ((x - CHESSGC_BOARD_X) / SQUARE_WIDTH)
	  + 10 * (7 - (Square)((y - CHESSGC_BOARD_Y) / SQUARE_HEIGHT)));

}

void hightlight_possible_moves(GSquare *gsquare)
{
  Square square_test;
  guint color;
  register Square square;
  register gshort rank;
  short    current_color;

  if(currentHighlightedGsquare == gsquare)
    return;

  /* Remember the current color to move */
  current_color = position_get_color_to_move(position);

  if(WPIECE(position->square[gsquare->square]))
    position_set_color_to_move(position, WHITE);
  else
    position_set_color_to_move(position, BLACK);

  for (rank = 1; rank <= 8; rank++) {
    for (square = A1 + ((rank - 1) * 10);
	 square <= H1 + ((rank - 1) * 10);
	 square++) {


	square_test = position_move_normalize (position, gsquare->square, chessboard[square]->square);

	if (square_test)
	  {
	    color=((rank+square)%2?BLACK_COLOR_H:WHITE_COLOR_H);

	    gnome_canvas_item_set(chessboard[square]->square_item,
				  "fill_color_rgba", color,
				  "outline_color", "black",
				  NULL);
	  }
	else
	  {
	    color=((rank+square)%2?BLACK_COLOR:WHITE_COLOR);

	    gnome_canvas_item_set(chessboard[square]->square_item,
				  "fill_color_rgba", color,
				  "outline_color", "black",
				  NULL);
	  }
      }
  }

  /* Set back the current color to move */
  position_set_color_to_move(position, current_color);

  /* Show the current piece */
  gnome_canvas_item_set(gsquare->square_item,
			"outline_color",
			(BPIECE(position->square[gsquare->square])?"red":"blue"),
			NULL);

}

/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
   static double x, y;
   static GSquare *gsquare;
   double new_x, new_y;
   GdkCursor *fleur;
   static int dragging;
   double item_x, item_y;

  if(board_paused)
    return FALSE;

  item_x = event->button.x;
  item_y = event->button.y;
  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      {
	guint x1, y1;
	Square square;

	square = get_square_from_coord(event->button.x, event->button.y);
	x1 = square % 10;
	y1 = square / 10 -1;
	gsquare = chessboard[square];

	x = item_x;
	y = item_y;

	fleur = gdk_cursor_new(GDK_FLEUR);
	gnome_canvas_item_raise_to_top(item);
	gc_canvas_item_grab(item,
			       GDK_POINTER_MOTION_MASK |
			       GDK_BUTTON_RELEASE_MASK,
			       fleur,
			       event->button.time);
	gdk_cursor_destroy(fleur);
	dragging = TRUE;

	hightlight_possible_moves(gsquare);
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
	   guint x, y;
	   double ofset_x, ofset_y;
	   double x1, y1, x2, y2;
	   char pos[6];
	   Square to;

	   to = get_square_from_coord(event->button.x, event->button.y);
	   g_warning("===== Source square = %d Destination square = %d\n", gsquare->square,
		  to);

	   to = position_move_normalize (position, gsquare->square, to);
	   if (to) {
	     position_move (position, gsquare->square, to);

	     x = 1 + (event->button.x - CHESSGC_BOARD_X) / SQUARE_WIDTH;
	     y = 1 + (event->button.y - CHESSGC_BOARD_Y) / SQUARE_HEIGHT;
	     move_to_ascii((char *)&pos, gsquare->square, to);

	     /* Tell gnuchess what our move is */
	     write_child (write_chan, (char *)&pos);
	     write_child (write_chan, "\n");
	     move_piece_to(gsquare->square, to);
	   }
	   else
	     {
	       g_warning("====== MOVE from %d REFUSED\n", gsquare->square);

	       /* Find the ofset to move the piece back to where it was*/
	       gnome_canvas_item_get_bounds  (item,
					      &x1,
					      &y1,
					      &x2,
					      &y2);

	       x = gsquare->square % 10;
	       y = gsquare->square / 10 -1;

	       ofset_x = (CHESSGC_BOARD_X + SQUARE_WIDTH  * (x-1)) - x1 + (SQUARE_WIDTH  - (x2-x1))/2;
	       ofset_y = (CHESSGC_BOARD_Y + SQUARE_HEIGHT * (8-y)) - y1 + (SQUARE_HEIGHT - (y2-y1))/2;
	       g_warning("ofset = x=%f y=%f\n", ofset_x, ofset_y);

	       gnome_canvas_item_move(item, ofset_x, ofset_y);
	     }

	   gc_canvas_item_ungrab(item, event->button.time);
	   dragging = FALSE;

	   position_display(position);

	 }
       break;

    default:
      break;
    }

  return FALSE;
}

/* ==================================== */
/* The user clicked on a black piece    */
static gint
item_event_black(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
   static GSquare *gsquare;

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      {
	Square square;

	square = get_square_from_coord(event->button.x, event->button.y);
	gsquare = chessboard[square];

	hightlight_possible_moves(gsquare);
      }
      break;
    default:
      break;
    }
  return(FALSE);
}

/*======================================================================*/
/*======================================================================*/
/*======================================================================*/
/*======================================================================*/
static void
engine_local_destroy (GPid gnuchess_pid)
{

  if(!gnuchess_pid)
    return;

  g_warning("engine_local_destroy () \n");
  write_child (write_chan, "quit\n");

  g_source_remove(read_cb);
  g_source_remove(err_cb);

  g_io_channel_close (read_chan);
  g_io_channel_unref (read_chan);

  g_io_channel_close (write_chan);
  g_io_channel_unref (write_chan);

  if(gnuchess_pid)
    g_spawn_close_pid(gnuchess_pid);
}

/** We got data back from gnuchess, we parse them here
 *
 */
static gboolean
engine_local_cb (GIOChannel *source,
		 GIOCondition condition,
		 gpointer data)
{
  gchar buf[1000];
  char *b=buf;
  GError *err = NULL;
  char *p,*q;
  gsize len = 0;
  GIOStatus status = G_IO_STATUS_NORMAL;

  g_warning("engine_local_cb");

  status = g_io_channel_read_chars(source,
				   buf,
				   1000,
				   &len,
				   &err);

  g_warning("g_io_channel_read_line len=%d", (int)len);
  if(status == G_IO_STATUS_ERROR)
    {
      g_warning("g_io_channel_read_chars error=%s",
		err->message);
      /* FIXME: Not sure what to do */
      return FALSE;
    }

  if(status != G_IO_STATUS_NORMAL)
    {
      g_warning("g_io_channel_read_chars error=%d",
		status);
      /* FIXME: Not sure what to do */
      return FALSE;
    }

  g_warning("engine_local_cb read=%s\n", buf);

  if (len > 0) {
    b[len] = 0;
    b += len;
  }

  while (1) {
    char tmp;

    q = strchr (buf,'\n');
    if (q == NULL) break;
    tmp = *(q+1);
    *(q+1) = '\0';

    *q='\0';
    *(q+1) = tmp;

    g_warning("engine_local_cb read=%s\n", buf);

    /* parse for  NUMBER ... MOVE */
    if (isdigit (*buf))
      {
	if ((p = strstr (buf, "...")))
	  {
	    Square from, to;

	    g_warning("computer number moves to %s\n", p+4);

	    if (san_to_move (position, p+4, &from, &to))
	      ascii_to_move (position, p+4, &from, &to);

	    position_move (position, from, to);
	    move_piece_to(from , to);
	  }
	else if ((p = strstr (buf, " ")))
	  {
	    /* It's a legal move case */
	    g_warning("Legal move to %s\n", p+1);
	  }
      }

    /* parse for move MOVE */
    if (!strncmp ("My move is : ",buf,13))
      {
	Square from, to;

	p = strstr (buf, ":");
	g_warning("computer moves to %s\n", p+1);

	if (san_to_move (position, p+1, &from, &to))
	  ascii_to_move (position, p+1, &from, &to);

	position_move (position, from, to);
	move_piece_to(from , to);
      }

    /* parse for illegal move */
    if (!strncmp ("Illegal move",buf,12))
      {
	g_warning("Illegal move to %s : SHOULD NOT HAPPEN", buf+31);
      }

    if (!strncmp ("0-1",buf,3))
      {
	display_info(_("Black mates"));
      }

    if (!strncmp ("1-0",buf,3))
      {
	display_info(_("White mates"));
      }

    if (!strncmp ("1/2-1/2",buf,7))
      {
	display_info(_("Drawn game"));
      }

    /* parse for feature */
    if (!strncmp ("feature",buf,7))
      {
	write_child(write_chan, "accepted setboard\n");
	write_child(write_chan, "accepted analyze\n");
	write_child(write_chan, "accepted ping\n");
	write_child(write_chan, "accepted draw\n");
	write_child(write_chan, "accepted variants\n");
	write_child(write_chan, "accepted myname\n");
	write_child(write_chan, "accepted done\n");
      }

    memmove (buf, q+1, sizeof(buf) - ( q + 1 - buf));
    b -= (q + 1 - buf);
  }

  return TRUE;
}

static gboolean
engine_local_err_cb (GIOChannel *source,
		     GIOCondition condition,
		     gpointer data)
{
  gnuchess_pid = 0;
  gc_dialog(_("Error: The external program gnuchess died unexpectingly"), gc_board_stop);
  return FALSE;
}

/*----------------------------------------
 * Subprocess creation
 * Return TRUE if gnuchess is started, false instead
 *----------------------------------------*/

static gboolean
start_child (char *cmd,
	     GIOChannel **read_chan,
	     GIOChannel **write_chan,
	     GPid *Child_Process)
{
  gint   Child_In, Child_Out, Child_Err;
  GError *gerror = NULL;

  gchar *Child_Argv[]={ cmd, "-e", NULL };

  g_warning("Ready to start child");

  if (!g_spawn_async_with_pipes(NULL, Child_Argv, NULL,
				G_SPAWN_SEARCH_PATH,
				NULL, NULL, Child_Process, &Child_In, &Child_Out,
				&Child_Err, &gerror)) {

    g_warning("Error message '%s'", gerror->message);
    g_warning("Error code    '%d'", gerror->code);
    g_error_free (gerror);
    g_warning("In order to play chess, you need to have gnuchess installed as " GNUCHESS);
    return(FALSE);

  }

  g_warning("gnuchess subprocess is started");

  *read_chan = g_io_channel_unix_new (Child_Out);
  *write_chan = g_io_channel_unix_new (Child_In);

  if(g_io_channel_set_encoding(*write_chan, NULL, NULL) != G_IO_STATUS_NORMAL)
    g_warning("Failed to set NULL encoding");

  if(g_io_channel_set_flags (*read_chan, G_IO_FLAG_NONBLOCK, NULL) != G_IO_STATUS_NORMAL)
    g_warning("Failed to set NON BLOCKING IO");

  if(g_io_channel_set_flags (*write_chan, G_IO_FLAG_NONBLOCK, NULL) != G_IO_STATUS_NORMAL)
    g_warning("Failed to set NON BLOCKING IO");

  return(TRUE);
}

/** Write a command to the gnuchess backend
 *
 */
static void
write_child (GIOChannel *write_chan, char *format, ...)
{
  GIOStatus err;
  va_list ap;
  gchar *buf;
  gsize len;
  GError *error = NULL;

  va_start (ap, format);

  buf = g_strdup_vprintf (format, ap);

  err = g_io_channel_write_chars (write_chan, buf, strlen (buf), &len, &error);

  if (err == G_IO_STATUS_ERROR)
    g_error ("Error writing: %s\n", error->message);

  if (err != G_IO_STATUS_NORMAL)
    g_warning ("Writing to child process failed");
  else
    g_warning ("Wrote '%s' to gnuchess", buf);

  err = g_io_channel_flush (write_chan, &error);
  if (err == G_IO_STATUS_ERROR)
    g_error ("Error flushing: %s\n", error->message);

  va_end (ap);

  g_free (buf);
}
