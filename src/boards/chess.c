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

#include <ctype.h>
#include <math.h>
#include <assert.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "chess_notation.h"

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

static void	 engine_local_destroy ();

static gboolean  engine_local_cb     (GIOChannel *source,
				      GIOCondition condition,
				      gpointer data);
static gboolean  engine_local_err_cb (GIOChannel *source,
				      GIOCondition condition,
				      gpointer data);
static void	 display_white_turn  (gboolean whiteturn);
static void	 display_info	     (gchar *info);
static int	 get_square          (double x, double y);
static int	 get_square_from_coord (double x, double y);

#define CHESSBOARD_X	50
#define CHESSBOARD_Y	20
#define SQUARE_WIDTH	60
#define SQUARE_HEIGHT	60
#define BLACK_COLOR	0x206070FF
#define WHITE_COLOR	0x4ACCFAFF
#define BLACK_COLOR_H	0x6B96A2FF
#define WHITE_COLOR_H	0xA6E7FFFF

#define INFO_COLOR	0xA0303FFF

#define TURN_X		(BOARDWIDTH-(BOARDWIDTH-(CHESSBOARD_X+(SQUARE_WIDTH*8)))/2)
#define TURN_Y		(CHESSBOARD_Y+15)

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

static pid_t childpid;

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
static int		 start_child (char        *cmd,
				      char       **arg,
				      GIOChannel **read_chan,
				      GIOChannel **write_chan,
				      pid_t       *childpid);

static void		 write_child (GIOChannel  *write_chan,
				      char        *format,
				      ...);

static int		 stop_child  (pid_t        childpid);

/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Learning Chess"),
    N_("Play chess against tux in a learning mode"),
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
  
  if (g_file_exists ("/usr/bin/gnuchessx")) {
    
    gcompris_dialog(_("Error: /usr/bin/gnuchessx is installed\nwhich means you run an old version\nof gnuchess.\nPlease upgrade to gnuchess 5 or above."), gcompris_end_board);
    
    return;
  }
  
  if (!g_file_exists (GNUCHESS)) {
    
    gcompris_dialog(_("Error: The external program gnuchess is mandatory\nto play chess in gcompris.\nFind this program on http://www.rpmfind.net or in your\nGNU/Linux distribution\nAnd check it is in "GNUCHESS), gcompris_end_board);
    
    return;
  }
  
  if(agcomprisBoard!=NULL)
    {
      char *param[2];

      param[0] = "xboard";
      param[1] = NULL;

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
	  gcompris_bar_set(GCOMPRIS_BAR_LEVEL);
	  break;
	default:
	  gcompris_bar_set(0);
	}
      
      start_child ("gnuchess", param, &read_chan,
		   &write_chan, &childpid);
      
      read_cb = g_io_add_watch (read_chan, G_IO_IN,
				engine_local_cb, NULL);
      err_cb = g_io_add_watch (read_chan, G_IO_HUP,
			       engine_local_err_cb, NULL);
      
      write_child (write_chan, "xboard\n");
      write_child (write_chan, "protover 2\n");
      write_child (write_chan, "post\n");
      write_child (write_chan, "easy\n");
      write_child (write_chan, "level 100 1 0\n");
      write_child (write_chan, "depth 1\n");
      
      chess_next_level();
      
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
      chess_destroy_all_items();
    }
  gcomprisBoard = NULL;

  turn_item     = NULL;
  info_item     = NULL;


  engine_local_destroy();
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
gboolean is_our_board (GcomprisBoard *gcomprisBoard)
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
  register Piece piece;
  register gshort rank;

  gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), 
			  gcompris_image_to_skin("gcompris-bg.jpg"));

  gcompris_bar_set_level(gcomprisBoard);

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
  register Piece piece;
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
static GnomeCanvasItem *chess_create_item(GnomeCanvasGroup *parent)
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
				     "x1", (double) CHESSBOARD_X + (x * SQUARE_WIDTH),
				     "y1", (double) CHESSBOARD_Y + ((7-y) * SQUARE_HEIGHT),
				     "x2", (double) CHESSBOARD_X + (x * SQUARE_WIDTH) + SQUARE_WIDTH -1,
				     "y2", (double)  CHESSBOARD_Y + ((7-y) * SQUARE_HEIGHT) + SQUARE_HEIGHT -1,
				     "fill_color_rgba", color,
				     "outline_color", "black",
				     "width_units", (double)1,
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
	char *temp;
	char *san;

	piece = position->square[square];

	x = square % 10 - 1;
	y = square / 10 - 2;

	/* Destination square */
	san = g_new0 (char, 12);
	temp = san;
	square_to_ascii (&temp, square);
	//	printf ( "%c%s\n", piece_to_ascii(piece), san);

	if(need_slash)
	  {
	    write_child (write_chan, "/");
	    need_slash = FALSE;
	  }

	if(piece!=NONE)
	  {

	    if(white_side && BPIECE(piece) ||
	       !white_side && WPIECE(piece)) 
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

  	temp = san;
	san = g_strdup (temp);
	g_free (temp);

	//	printf("square=%d piece=%d x=%d y=%d\n", square, piece, x, y);
	if(piece != EMPTY)
	  {
	    str = g_strdup_printf("chess/%c.png", piece_to_ascii(piece));
	    
	    pixmap = gcompris_load_pixmap(str);
	    //	    printf("loading piece %s\n",   str);
	    g_free(str);
	    item = gnome_canvas_item_new (boardRootItem,
					  gnome_canvas_pixbuf_get_type (),
					  "pixbuf", pixmap, 
					  "x", (double)CHESSBOARD_X + (x * SQUARE_WIDTH) +
					  (guint)((SQUARE_WIDTH-gdk_pixbuf_get_width(pixmap))/2),
					  "y", (double) CHESSBOARD_Y + ((7-y) * SQUARE_HEIGHT) +
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

  /* Quit the gnuchessx edit mode */
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
      board_finished(BOARD_FINISHED_RANDOM);
      return;
    }
    gcompris_play_ogg ("bonus", NULL);
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
					 "font",       gcompris_skin_font_board_big,
					 "x", (double) TURN_X,
					 "y", (double) TURN_Y,
					 "anchor",     GTK_ANCHOR_CENTER,
					 "fill_color", "white",
					 NULL);
    }

  gnome_canvas_item_set(turn_item, "text", (whiteturn ? _("White Turn") : _("Black Turn")),
			NULL);
}


static void display_info(gchar *info)
{

  if(info_item == NULL)
    {
      info_item = gnome_canvas_item_new (boardRootItem,
					 gnome_canvas_text_get_type (),
					 "text",       " ",
					 "font",       gcompris_skin_font_board_big,
					 "x", (double) INFO_X,
					 "y", (double) INFO_Y,
					 "anchor",     GTK_ANCHOR_CENTER,
					 "fill_color_rgba", INFO_COLOR,
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
      

  printf("move_piece_to from=%d to=%d\n", from, to);

  source_square = chessboard[from];
  item = source_square->piece_item;
  source_square->piece_item = NULL;

  if(item == NULL)
    {
      printf("Warning: Problem in chess.c, bad move request in move_piece_to\n");
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
			(BPIECE(position->square[to])?"red":"green"),
			NULL);

  display_white_turn(BPIECE(position->square[to]));

  x = to % 10;
  y = to / 10 -1;
  
  printf("   move_piece_to to    x=%d y=%d\n", x, y);

  dest_square = chessboard[to];

  /* Show the moved piece */
  gnome_canvas_item_set(dest_square->square_item,
			"outline_color", 
			(BPIECE(position->square[to])?"red":"green"),
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


  ofset_x = (CHESSBOARD_X + SQUARE_WIDTH  * (x-1)) - x1 + (SQUARE_WIDTH  - (x2-x1))/2;
  ofset_y = (CHESSBOARD_Y + SQUARE_HEIGHT * (8-y)) - y1 + (SQUARE_HEIGHT - (y2-y1))/2;

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
      printf("  WARNING promoting a pawn from=%d to=%d piece=%d\n", from, to, piece);
      printf("  piece_to_ascii returns %c\n", piece_to_ascii(piece));

      str = g_strdup_printf("chess/%c.png", piece_to_ascii(piece));
	      
      pixmap = gcompris_load_pixmap(str);
      g_free(str);
      printf("loading piece %c\n",  piece_to_ascii(piece));
      gnome_canvas_item_set (dest_square->piece_item,
			     "pixbuf", pixmap, 
			     NULL);

    }

  /* Display check info */
  if(position_white_king_attack(position))
    display_info(_("White check"));
  else if(position_black_king_attack(position))
    display_info(_("Black check"));
  else
    display_info(" ");

}

/*
 * Return a square suitable for position functions
 */
static int
get_square (double x, double y)
{

  return (A1 + (Square) (x-1)
	  + 10 * (Square)(y-1));

}

/*
 * Return a square suitable for position functions
 */
static int
get_square_from_coord (double x, double y)
{

  return (A1 + (Square) ((x - CHESSBOARD_X) / SQUARE_WIDTH)
	  + 10 * (7 - (Square)((y - CHESSBOARD_Y) / SQUARE_HEIGHT)));

}

void hightlight_possible_moves(GSquare *gsquare)
{
  Square square_test;
  guint color;
  register Square square;
  register Piece piece;
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
			(BPIECE(position->square[gsquare->square])?"red":"green"),
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
	gnome_canvas_item_grab(item,
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
	   printf("===== Source square = %d Destination square = %d\n", gsquare->square, 
		  to);

	   to = position_move_normalize (position, gsquare->square, to);
	   if (to) {
	     position_move (position, gsquare->square, to);

	     x = 1 + (event->button.x - CHESSBOARD_X) / SQUARE_WIDTH;
	     y = 1 + (event->button.y - CHESSBOARD_Y) / SQUARE_HEIGHT;
	     move_to_ascii((char *)&pos, gsquare->square, to);

	     /* Tell gnuchess what our move is */
	     write_child (write_chan, (char *)&pos);
	     write_child (write_chan, "\n");
	     move_piece_to(gsquare->square, to);
	   }
	   else
	     {
	       printf("====== MOVE from %d REFUSED\n", gsquare->square);
	   
	       /* Find the ofset to move the piece back to where it was*/
	       gnome_canvas_item_get_bounds  (item,
					      &x1,
					      &y1,
					      &x2,
					      &y2);
	       
	       x = gsquare->square % 10;
	       y = gsquare->square / 10 -1;
	       
	       ofset_x = (CHESSBOARD_X + SQUARE_WIDTH  * (x-1)) - x1 + (SQUARE_WIDTH  - (x2-x1))/2;
	       ofset_y = (CHESSBOARD_Y + SQUARE_HEIGHT * (8-y)) - y1 + (SQUARE_HEIGHT - (y2-y1))/2;
	       printf("ofset = x=%f y=%f\n", ofset_x, ofset_y);
	       
	       gnome_canvas_item_move(item, ofset_x, ofset_y);
	     }

	   gnome_canvas_item_ungrab(item, event->button.time);
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
    }
}

/*======================================================================*/
/*======================================================================*/
/*======================================================================*/
/*======================================================================*/
static void
engine_local_destroy () 
{

  printf("engine_local_destroy () \n");
  //  write_child (write_chan, "quit\n");

  g_source_remove(read_cb);
  g_source_remove(err_cb);

  g_io_channel_close (read_chan);
  g_io_channel_unref (read_chan);
  
  g_io_channel_close (write_chan);
  g_io_channel_unref (write_chan);
  
  stop_child (childpid);  
}

static gboolean
engine_local_cb (GIOChannel *source,
		 GIOCondition condition,
		 gpointer data)
{
  static char buf[1024];
  static char *b=buf;
  
  char *p,*q;
  ssize_t len;
  
  g_io_channel_read (read_chan, b, sizeof (buf) - 1 - (b - buf), &len);
  
  if (len > 0) {
    b[len] = 0;
    b += len;
  }
  
  while (1) {
    char tmp;
    
    q = strchr (buf,'\n');
    if (q == NULL) break;
    tmp = *(q+1);
    *(q+1) = 0;
    
    *q=0;
    *(q+1) = tmp;
    
    printf("engine_local_cb read=%s\n", &buf);
    
    /* parse for  NUMBER ... MOVE */
    if (isdigit (*buf))
      if ((p = strstr (buf, "...")))
	{
	  Square from, to;
	  
	  printf("computer number moves to %s\n", p+4);
	  
	  if (san_to_move (position, p+4, &from, &to))
	    ascii_to_move (position, p+4, &from, &to);
	  
	  position_move (position, from, to);
	  move_piece_to(from , to);
	}
      else if ((p = strstr (buf, " ")))
	{
	  /* It's a legal move case */
	  printf("Legal move to %s\n", p+1);
	}
    
    /* parse for move MOVE */
    if (!strncmp ("My move is : ",buf,13))
      {
	Square from, to;

	p = strstr (buf, ":");
	printf("computer moves to %s\n", p+1);

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
  g_error ("Local Engine connection died");
  
  return FALSE;
}

/*
 * FIXME : This should be centralised in gcompris core because only 
 *         the last signal command is kept at system level
 *         there is a potential risk because signal child is also used
 *         to manage ogg123 sounds.
 */

/* =====================================================================
 * Process the cleanup of the child (no zombies)
 * =====================================================================*/
void chess_child_end(int  signum)
{
  pid_t pid;

  printf("chess child_end signal=%d\n", signum);

  pid = waitpid(childpid, NULL, WNOHANG);
  printf("chess child_end pid=%d\n", pid);

  if (pid == -1)
    g_error("chess_child_end Error waitpid");

}

/*----------------------------------------
 * Subprocess creation
 *----------------------------------------*/

static int 
start_child (char *cmd, 
	     char **arg, 
	     GIOChannel **read_chan, 
	     GIOChannel **write_chan, 
	     pid_t *childpid)
{
  int pipe1[2], pipe2[2];

  if ((pipe (pipe1) <0) || (pipe (pipe2) < 0)) {
    perror ("pipe");
    exit (-1);
  }

  signal(SIGCHLD, chess_child_end);

  if ((*childpid = fork ()) < 0) {
    perror ("fork");
    exit (-1);
  } else if (*childpid > 0) /* Parent */ {
    close (pipe1[0]);
    close (pipe2[1]);

    *read_chan = g_io_channel_unix_new (pipe2[0]);
    *write_chan = g_io_channel_unix_new (pipe1[1]);

    return *childpid;
  } else   /* Child */ {
    close (pipe1[1]);
    close (pipe2[0]);

    dup2 (pipe1[0],0);
    dup2 (pipe2[1],1);

    close (pipe1[0]);
    close (pipe2[1]);

    if (execvp (cmd, arg) < 0)
      {
	g_warning("In order to play chess, you need to have gnuchess package installed with gnuchessx in the path");
	perror (cmd);
      }
    _exit (1);
  }

  g_assert_not_reached ();

  return 0;
}

static void 
write_child (GIOChannel *write_chan, char *format, ...) 
{
  GIOError err;
  va_list ap;
  char *buf;
  int len;	

  va_start (ap, format);

  buf = g_strdup_vprintf (format, ap);

  err = g_io_channel_write (write_chan, buf, strlen (buf), &len);
  if (err != G_IO_ERROR_NONE)
    g_warning ("Writing to child process failed");

  printf ("%s", buf);  

  va_end (ap);

  g_free (buf);
}

/* Kill Child */
static int 
stop_child (pid_t childpid) 
{

  printf("stop_child (childpid=%d) () \n", childpid);

  if (childpid && kill (childpid, SIGTERM) ) { 
    g_message ("Failed to kill child!\n");
    return 1;
  }

  return 0;
}
