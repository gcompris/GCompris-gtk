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

#include "chess_position.h"
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

#define CHESSBOARD_X	100
#define CHESSBOARD_Y	20
#define SQUARE_WIDTH	60
#define SQUARE_HEIGHT	60
#define BLACK_COLOR	0x206070FF
#define WHITE_COLOR	0x4ACCFAFF
#define BLACK_COLOR_H	0x6B96A2FF
#define WHITE_COLOR_H	0xA6E7FFFF

#define TURN_X		(BOARDWIDTH-(BOARDWIDTH-(CHESSBOARD_X+(SQUARE_WIDTH*8)))/2)
#define TURN_Y		(CHESSBOARD_Y+15)

#define INFO_X		TURN_X
#define INFO_Y		(TURN_Y+40)

#define IS_WHITE_PIECE(p)	(isupper(p))


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
  gchar			 position[2];
  gchar			 piece;
  GnomeCanvasItem	*piece_item;
  Square		 from;
  Square		 to;
} GSquare;

static GnomeCanvasItem	*turn_item = NULL;
static GnomeCanvasItem	*info_item = NULL;

static GSquare *chessboard[9][9];

static GnomeCanvasItem	*chess_create_item(GnomeCanvasGroup *parent);
static void		 chess_destroy_all_items(void);
static void		 chess_next_level(void);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static void		 display_square(GSquare *square);
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

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=1;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gcompris_bar_set(0);

      chess_next_level();

      gamewon = FALSE;
      pause_board(FALSE);

      start_child ("gnuchessx", NULL, &read_chan,
		   &write_chan, &childpid);

      read_cb = g_io_add_watch (read_chan, G_IO_IN,
				engine_local_cb, NULL);
      err_cb = g_io_add_watch (read_chan, G_IO_HUP,
			       engine_local_err_cb, NULL);

      write_child (write_chan, "post\n");
      write_child (write_chan, "easy\n");
      write_child (write_chan, "level 100 1 0\n");
      write_child (write_chan, "depth\n");
      write_child (write_chan, "1\n");
      
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
  guint x,y;

  gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			  "gcompris/gcompris-bg.jpg");

  gcompris_bar_set_level(gcomprisBoard);

  chess_destroy_all_items();
  gamewon = FALSE;

  position = POSITION (position_new_initial ());

  /* Initial position */
  for(x=1; x<=8; x++)
    for(y=1; y<=8; y++)
      {
	GSquare *square;

	square = g_malloc(sizeof(GSquare));

	square->position[0] = 'a' + x - 1;
	square->position[1] = '0' + y;

	if(y==7)
	  {
	    square->piece='p';
	  }
	else if(y==2) 
	  {
	    square->piece='P';
	  }
	else if(y==8)
	  {
	    if(x==1) square->piece='r';
	    if(x==2) square->piece='n';
	    if(x==3) square->piece='b';
	    if(x==4) square->piece='q';
	    if(x==5) square->piece='k';
	    if(x==6) square->piece='b';
	    if(x==7) square->piece='n';
	    if(x==8) square->piece='r';
	  }		
	else if(y==1)
	  {
	    if(x==1) square->piece='R';
	    if(x==2) square->piece='N';
	    if(x==3) square->piece='B';
	    if(x==4) square->piece='Q';
	    if(x==5) square->piece='K';
	    if(x==6) square->piece='B';
	    if(x==7) square->piece='N';
	    if(x==8) square->piece='R';
	  }		
	else
	  square->piece=' ';

	chessboard[x][y] = square;
	chessboard[x][y]->piece_item = NULL;

	chessboard[x][y]->from = get_square(x, y);
	printf("get_square(%d, %d) = %d\n", x, y, chessboard[x][y]->from);
      }

  /* Try the next level */
  chess_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}

/* ==================================== */
/* Destroy all the items */
static void chess_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;

  if(position!=NULL)
    gtk_object_destroy (GTK_OBJECT (position));

  position = NULL;
}

/* ==================================== */
static GnomeCanvasItem *chess_create_item(GnomeCanvasGroup *parent)
{
  int x,y;
  guint color;
  GnomeCanvasItem *item = NULL;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,

							    NULL));

  for(x=0; x<8; x++)
    {
      for(y=0; y<8; y++)
	{

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
	  chessboard[x+1][y+1]->square_item = item;
	}
    }

  /* Display the pieces */
  for(x=0; x<8; x++)
    {
      for(y=0; y<8; y++)
	{
	  if(chessboard[x+1][y+1]->piece!=' ')
	    {
	      GdkPixbuf *pixmap = NULL;
	      char *str;
	      str = g_strdup_printf("chess/%c.png", chessboard[x+1][y+1]->piece);
	      
	      pixmap = gcompris_load_pixmap(str);
	      g_free(str);
	      printf("loading piece %c\n",  chessboard[x+1][y+1]->piece);
	      item = gnome_canvas_item_new (boardRootItem,
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", pixmap, 
					    "x", (double)CHESSBOARD_X + (x * SQUARE_WIDTH) +
					    (guint)((SQUARE_WIDTH-gdk_pixbuf_get_width(pixmap))/2),
					    "y", (double) CHESSBOARD_Y + ((7-y) * SQUARE_HEIGHT) +
					    (guint)((SQUARE_HEIGHT-gdk_pixbuf_get_height(pixmap))/2),
					    NULL);

	      chessboard[x+1][y+1]->piece_item = item;
	      if(IS_WHITE_PIECE(chessboard[x+1][y+1]->piece))
		gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event, NULL);
	      gdk_pixbuf_unref(pixmap);

	    }
	}
    }

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
    gcompris_play_sound (SOUNDLISTFILE, "bonus");
  }
  chess_next_level();
}

static void
display_square(GSquare *square)
{
  printf("---------\n");
  printf("piece=%c position=%c%c\n", square->piece, square->position[0], square->position[1]);
}

static void display_white_turn(gboolean whiteturn)
{

  if(turn_item == NULL)
    {
      GdkFont *gdk_font;
      gdk_font = gdk_font_load (FONT_BOARD_BIG);

      turn_item = gnome_canvas_item_new (boardRootItem,
					 gnome_canvas_text_get_type (),
					 "text",       " ",
					 "font_gdk",   gdk_font,
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
      GdkFont *gdk_font;
      gdk_font = gdk_font_load (FONT_BOARD_BIG);

      info_item = gnome_canvas_item_new (boardRootItem,
					 gnome_canvas_text_get_type (),
					 "text",       " ",
					 "font_gdk",   gdk_font,
					 "x", (double) INFO_X,
					 "y", (double) INFO_Y,
					 "anchor",     GTK_ANCHOR_CENTER,
					 "fill_color", "red",
					 NULL);
    }

  gnome_canvas_item_set(info_item, "text", info,
			NULL);
}


/*
 * Move a piece to the given position
 * dest is a chess notation like c2c4
 *
 */
static void move_piece_to(Square from, Square to)
{
  GSquare *source_square, *dest_square;
  GnomeCanvasItem *item;
  guint x, y;
  double ofset_x, ofset_y;
  double x1, y1, x2, y2;
      

  printf("move_piece_to %d %d\n", from, to);

  x = from % 10;
  y = from / 10 -1;
  
  printf("   move_piece_to from x=%d y=%d\n", x, y);

  source_square = chessboard[x][y];
  item = source_square->piece_item;
  source_square->piece_item = NULL;

  if(item == NULL)
    {
      printf("Warning: Problem in chess.c, bad move request in move_piece_to\n");
      return;
    }

  /* Show the moved piece */
  gnome_canvas_item_set(source_square->square_item,
			"outline_color", "red",
			NULL);

  display_white_turn(!IS_WHITE_PIECE(source_square->piece));

  x = to % 10;
  y = to / 10 -1;
  
  printf("   move_piece_to to    x=%d y=%d\n", x, y);

  dest_square = chessboard[x][y];

  /* Show the moved piece */
  gnome_canvas_item_set(dest_square->square_item,
			"outline_color", "red",
			NULL);

  if(dest_square->piece_item != NULL)
    /* Oups I loose a piece */
    gtk_object_destroy (GTK_OBJECT(dest_square->piece_item));

  dest_square->piece_item    = item;
  dest_square->piece         = source_square->piece;
  source_square->piece       = ' ';

  /* Find the ofset to move the piece */
  gnome_canvas_item_get_bounds  (item,
				 &x1,
				 &y1,
				 &x2,
				 &y2);


  ofset_x = (CHESSBOARD_X + SQUARE_WIDTH  * (x-1)) - x1 + (SQUARE_WIDTH  - (x2-x1))/2;
  ofset_y = (CHESSBOARD_Y + SQUARE_HEIGHT * (8-y)) - y1 + (SQUARE_HEIGHT - (y2-y1))/2;
  printf("ofset = x=%f y=%f\n", ofset_x, ofset_y);
  gnome_canvas_item_move(item, ofset_x, ofset_y);

  /* Manage rock */
  if(dest_square->piece=='K' && source_square->from==E1 && dest_square->from==C1)
    move_piece_to(A1, D1);
  else if(dest_square->piece=='K' && source_square->from==E1 && dest_square->from==G1)
    move_piece_to(H1, F1);
  else if(dest_square->piece=='k' && source_square->from==E8 && dest_square->from==C8)
    move_piece_to(A8, D8);
  else if(dest_square->piece=='k' && source_square->from==E8 && dest_square->from==G8)
    move_piece_to(H8, F8);

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

void hightlight_possible_moves(GSquare *square)
{
  guint x, y;
  Square square_test;
  guint color;

  for(x=1; x<=8; x++)
    for(y=1; y<=8; y++)
      {
	square_test = position_move_normalize (position, square->from, chessboard[x][y]->from);
	printf("Testing position %d %d from %d to %d returned %d\n", x, y,
	       square->from, chessboard[x][y]->from, square_test);

	if (square_test) 
	  {
	    color=((x+y)%2?BLACK_COLOR_H:WHITE_COLOR_H);

	    gnome_canvas_item_set(chessboard[x][y]->square_item,
				  "fill_color_rgba", color,
				  "outline_color", "black",
				  NULL);
	  }
	else
	  {
	    color=((x+y)%2?BLACK_COLOR:WHITE_COLOR);

	    gnome_canvas_item_set(chessboard[x][y]->square_item,
				  "fill_color_rgba", color,
				  "outline_color", "black",
				  NULL);
	  }
      }      

  /* Show the current piece */
  gnome_canvas_item_set(square->square_item,
			"outline_color", "red",
			NULL);
  
}

/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
   static double x, y;
   static GSquare *square;
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
	Square square_from;

	square_from = get_square_from_coord(event->button.x, event->button.y);
	x1 = square_from % 10;
	y1 = square_from / 10 -1;
	square = chessboard[x1][y1];

	display_square(square);

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

	hightlight_possible_moves(square);
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

	   square->to = get_square_from_coord(event->button.x, event->button.y);
	   printf("===== Source square = %d Destination square = %d\n", square->from, 
		  square->to);

	   square->to = position_move_normalize (position, square->from, square->to);
	   if (square->to) {
	     position_move (position, square->from, square->to);
	     display_square(square);
	     x = 1 + (event->button.x - CHESSBOARD_X) / SQUARE_WIDTH;
	     y = 1 + (event->button.y - CHESSBOARD_Y) / SQUARE_HEIGHT;
	     pos[0] = square->position[0];
	     pos[1] = square->position[1];
	     pos[2] = 'a' + x - 1;
	     pos[3] = '9' - y;
	     pos[4] = '\n';
	     pos[5] = 0;
	     
	     /* Tell gnuchess what our move is */
	     write_child (write_chan, (char *)&pos);
	     move_piece_to(square->from, square->to);
	   }
	   else
	     {
	       printf("====== MOVE from %d REFUSED\n", square->from);
	   
	       /* Find the ofset to move the piece back to where it was*/
	       gnome_canvas_item_get_bounds  (item,
					      &x1,
					      &y1,
					      &x2,
					      &y2);
	       
	       x = square->position[0] - 'a' + 1;
	       y = '9' - square->position[1];
	       
	       ofset_x = (CHESSBOARD_X + SQUARE_WIDTH  * (x-1)) - x1 + (SQUARE_WIDTH  - (x2-x1))/2;
	       ofset_y = (CHESSBOARD_Y + SQUARE_HEIGHT * (y-1)) - y1 + (SQUARE_HEIGHT - (y2-y1))/2;
	       printf("ofset = x=%f y=%f\n", ofset_x, ofset_y);
	       
	       gnome_canvas_item_move(item, ofset_x, ofset_y);
	     }

	   gnome_canvas_item_ungrab(item, event->button.time);
	   dragging = FALSE;
	 }
       break;

    default:
      break;
    }

  return FALSE;
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
    if (!strncmp ("move",buf,4))
      {
	Square from, to;

	printf("computer moves to %s\n", p+5);

	if (san_to_move (position, p+5, &from, &to))
	  ascii_to_move (position, p+5, &from, &to);

	position_move (position, from, to);
	move_piece_to(from , to);
      }
    
    /* parse for illegal move */
    if (!strncmp ("Illegal move",buf,12))
      {
	printf("Illegal move to %s\n", buf+31);
      }

    if (!strncmp ("Black mates!",buf,12))
      {
	display_info(_("Black mates"));
      }

    if (!strncmp ("White mates!",buf,12))
      {
	display_info(_("White mates"));
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

/* =====================================================================
 * Process the cleanup of the child (no zombies)
 * =====================================================================*/
void chess_child_end(int  signum)
{
  pid_t pid;
  int i;
  tsSound * tmpSound;

  pid = waitpid(-1, NULL, WNOHANG);
  printf("chess child_end pid=%d", pid);

  if (pid == -1)
    g_error("Error waitpid");
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

  g_message (buf);  

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
