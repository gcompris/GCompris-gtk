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

#include "gcompris/gcompris.h"

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

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem	*hanoi_create_item(GnomeCanvasGroup *parent);
static void		 hanoi_destroy_all_items(void);
static void		 hanoi_next_level(void);

/*
 * Contains the piece information
 */
typedef struct {
  GnomeCanvasItem *item;
  GnomeCanvasItem *item_text;
  gint i;
  gint j;
  double x;
  double y;
  double xt;
  double yt;
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

static guint colorlist [] =
  {
    0x00FFFFFF,
    0xA00000FF,
    0xF00000FF,
    0x00A000FF,
    0x00F000FF,
    0x0000AAFF,
    0x0000FFFF,
    0x505000FF,
    0xA0A000FF,
    0xF0F000FF,
    0x005050FF,
    0x00A0A0FF,
    0x500050FF,
    0xA000A0FF,
    0xF000F0FF
  };
#define NUMBER_OF_COLOR G_N_ELEMENTS(colorlist)


/* Description of this plugin */
static BoardPlugin menu_bp =
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
    NULL,
    NULL,
    NULL
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(hanoi)

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
    gchar *img;

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=6;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gc_bar_set(GC_BAR_LEVEL);

      gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      img = gc_skin_image_get("gcompris-bg.jpg"));
      g_free(img);
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
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
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

  gc_bar_set_level(gcomprisBoard);

  hanoi_destroy_all_items();
  gamewon = FALSE;

  /* Select level difficulty */
  switch(gcomprisBoard->level)
    {
    case 1:
      number_of_item_x = 3;
      number_of_item_y = 5;
      break;
    case 2:
      number_of_item_x = 4;
      number_of_item_y = 5;
      break;
    case 3:
      number_of_item_x = 5;
      number_of_item_y = 6;
    case 4:
      number_of_item_x = 6;
      number_of_item_y = 7;
      break;
    case 5:
      number_of_item_x = 6;
      number_of_item_y = 8;
      break;
    case 6:
      number_of_item_x = 5;
      number_of_item_y = 9;
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
  guint i,j;

  if(boardRootItem!=NULL)
    {
      gtk_object_destroy (GTK_OBJECT(boardRootItem));

      /* Cleanup our memory structure */
      for(i=0; i<(number_of_item_x+2); i++)
	{
	  for(j=0; j<number_of_item_y; j++)
	    {
	      g_free(position[i][j]);
	    }
	}
    }
  boardRootItem = NULL;

}

#if 0
static void dump_solution()
{
  guint i, j;

  g_warning("Dumping solution\n");
  for(i=0; i<(number_of_item_x+2); i++)
    {
      for(j=0; j<number_of_item_y; j++)
	{
	  g_warning("(%d,%d=%2d/%d) ",  position[i][j]->i,  position[i][j]->j, position[i][j]->color,
		 position[i][j]->on_top);
	}
      g_warning("\n");
    }

}

static void print_piece(PieceItem *piece)
{
  g_warning("Piece: (%d,%d=%2d/%d)\n",  piece->i,  piece->j, piece->color, piece->on_top);
}
#endif

/* ==================================== */
static GnomeCanvasItem *hanoi_create_item(GnomeCanvasGroup *parent)
{
  int i,j;
  double gap_x, gap_y;
  double baseline;
  GnomeCanvasItem *item = NULL;
  guint color_to_place;
  guint used_colors[NUMBER_OF_COLOR];
  GnomeCanvasPathDef *path;
  guint w;
  GdkPixbuf *pixmap = NULL;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  pixmap = gc_skin_pixmap_load("gcompris-shapelabel.png");
  if(pixmap) {
    gnome_canvas_item_new (boardRootItem,
			   gnome_canvas_pixbuf_get_type (),
			   "pixbuf", pixmap,
			   "x",	(double)BOARDWIDTH/2,
			   "y",	(double)BOARDHEIGHT - 35,
			   "width", (double) BOARDWIDTH - 20,
			   "width_set", TRUE,
			   "anchor", GTK_ANCHOR_CENTER,
			   NULL);
    gdk_pixbuf_unref(pixmap);
  }

  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_text_get_type (),
			 "text", _("Build the same tower in the empty area as the one you see on the right-hand side."),
			 "font", gc_skin_font_board_medium,
			 "x", (double) BOARDWIDTH/2 +1,
			 "y", (double) BOARDHEIGHT - 50 +1,
			 "anchor", GTK_ANCHOR_NORTH,
			 "fill_color", "black",
			 "justification", GTK_JUSTIFY_CENTER,
			 NULL);

  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_text_get_type (),
			 "text", _("Build the same tower in the empty area as the one you see on the right-hand side."),
			 "font", gc_skin_font_board_medium,
			 "x", (double) BOARDWIDTH/2,
			 "y", (double) BOARDHEIGHT - 50,
			 "anchor", GTK_ANCHOR_NORTH,
			 "fill_color_rgba", gc_skin_color_text_button,
			 "justification", GTK_JUSTIFY_CENTER,
			 NULL);


  /*----------------------------------------*/
  /* Empty the solution */
  for(i=0; i<(number_of_item_x+2); i++)
    {
      for(j=0; j<number_of_item_y; j++)
	{
	  position[i][j] = g_malloc(sizeof(PieceItem));
	  position[i][j]->color  = -1;
	  position[i][j]->i      = i;
	  position[i][j]->j      = j;
	  position[i][j]->on_top = FALSE;
	}
    }

  /* Clear the used colors list */
  for(i=0; i<NUMBER_OF_COLOR; i++)
    used_colors[i] = FALSE;

  /* Initialize a random goal and store the color index in position[number_of_item_x] */
  for(i=0; i<(number_of_item_y); i++)
    {
      guint color = (guint)g_random_int_range(0, NUMBER_OF_COLOR-1);
      position[number_of_item_x+1][i]->color = color;
      used_colors[color] = TRUE;

    }

  /* Randomly place the solution */
  for (color_to_place=0; color_to_place<number_of_item_y; color_to_place++)
    {
      gboolean done;

      do
	{
	  done = FALSE;

	  i = (guint)g_random_int_range(0, number_of_item_x);

	  /* Restrict the goal to lowest items */
	  j = (guint)g_random_int_range(0, 2);

	  if(position[i][j]->color == -1)
	    {
	      done = TRUE;
	      position[i][j]->color = position[number_of_item_x+1][color_to_place]->color;
	    }
	}
      while(!done);
    }

  /* Initialize the left open positions */
  for(i=0; i<(number_of_item_x); i++)
    {
      for(j=0; j<number_of_item_y-1; j++)
	{
	  if(position[i][j]->color == -1)
	    {
	      /* Take only a color that is not part of the goal */
	      guint color = (guint)g_random_int_range(0, NUMBER_OF_COLOR-1);
	      //printf(" i,j=%d,%d random color = %d used_colors[color]=%d\n", i,j,color, used_colors[color]);
	      while(used_colors[color])
		{
		  //printf("  used_colors[%d]=%d\n", color, used_colors[color]);
        color++;
		if(color >= NUMBER_OF_COLOR)
		  color = 0;
		}

	      position[i][j]->color = color;
	    }
	}
    }
  //dump_solution();

  /* Mark the top pieces */
  for(i=0; i<(number_of_item_x); i++)
    {
      position[i][number_of_item_y-2]->on_top = TRUE;
    }

  /*----------------------------------------*/
  /* Display it now */

  item_width  = BOARDWIDTH / (number_of_item_x + 2);
  item_height = 30;

  gap_x = item_width  * 0.1;
  gap_y = item_height * 0.25;

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
				 "fill_color_rgba", 0x036ED8FF,
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
				 "fill_color_rgba", 0x48AAF1FF,
				 "outline_color", "black",
				 "width_units", (double)1,
				 NULL);
	}

      /* Create the vertical line */
      w = 10;
      gnome_canvas_item_new (boardRootItem,
			     gnome_canvas_rect_get_type (),
			     "x1", (double) item_width * i + item_width/2 - w,
			     "y1", (double) baseline - item_height * number_of_item_y - gap_y,
			     "x2", (double) item_width * i + item_width/2 + w,
			     "y2", (double) baseline,
			     "fill_color_rgba", 0xFF1030FF,
			     "outline_color", "black",
			     "width_units", (double)1,
			     NULL);

      /* And the base line */
      w = 40;
      path = gnome_canvas_path_def_new();
      gnome_canvas_path_def_moveto (path, item_width * i + item_width/2 - w, baseline);
      gnome_canvas_path_def_lineto (path, item_width * i + item_width/2 + w, baseline);
      gnome_canvas_path_def_curveto (path,
				     item_width * i + item_width/2 + w , baseline,
				     item_width * i + item_width/2, baseline + w + 10,
				     item_width * i + item_width/2 - w, baseline);
      gnome_canvas_path_def_closepath_current (path);

      item = gnome_canvas_item_new (boardRootItem,
				    GNOME_TYPE_CANVAS_SHAPE,
				    "fill_color_rgba", 0x20FF30FF,
				    "outline_color", "black",
				    NULL);
      gnome_canvas_shape_set_path_def (GNOME_CANVAS_SHAPE (item), path);
      gnome_canvas_item_show (item);
      gnome_canvas_path_def_unref (path);

      for(j=0; j<number_of_item_y; j++)
	{

	  position[i][j]->x = item_width * i + gap_x;
	  position[i][j]->y = baseline - item_height * j - item_height + gap_y;

	  position[i][j]->xt = position[i][j]->x + 20;
	  position[i][j]->yt = position[i][j]->y + 2;


	  if(position[i][j]->color != -1)
	    {
	      char car[2];

	      item = gnome_canvas_item_new (boardRootItem,
					    gnome_canvas_rect_get_type (),
					    "x1", (double) position[i][j]->x,
					    "y1", (double) position[i][j]->y,
					    "x2", (double) item_width * i + item_width - gap_x,
					    "y2", (double) baseline - item_height * j,
					    "fill_color_rgba", colorlist[position[i][j]->color],
					    "outline_color", "black",
					    "width_units", (double)1,
					    NULL);

	      car[0] = 'a' + position[i][j]->color;
	      car[1] = '\0';

	       position[i][j]->item_text = \
		 gnome_canvas_item_new (boardRootItem,
					gnome_canvas_text_get_type (),
					"text", &car,
					"font", gc_skin_font_board_tiny,
					"x", (double) position[i][j]->xt,
					"y", (double) position[i][j]->yt,
					"anchor", GTK_ANCHOR_NORTH,
					"fill_color", "white",
					"justification", GTK_JUSTIFY_CENTER,
					NULL);

	      position[i][j]->item = item;

	      if(i!=number_of_item_x+1)
		gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event,  position[i][j]);

	    }

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
      gc_bonus_end_display(BOARD_FINISHED_RANDOM);
      return;
    }
    gc_sound_play_ogg ("sounds/bonus.ogg", NULL);
  }
  hanoi_next_level();
}

/*
 * Returns TRUE is the goal is reached
 */
static gboolean is_completed()
{
  gint j;
  gboolean done = TRUE;

  for(j=0; j<number_of_item_y; j++)
    {
      if(position[number_of_item_x+1][j]->color != position[number_of_item_x][j]->color)
	done = FALSE;
    }

  return done;
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

  if(!data->on_top)
    return FALSE;

  item_x = event->button.x;
  item_y = event->button.y;
  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
      gnome_canvas_item_set(item,
			    "outline_color", "white",
			    "width_units", (double)3,
			    NULL);
      break;
    case GDK_LEAVE_NOTIFY:
      gnome_canvas_item_set(item,
			    "outline_color", "black",
			    "width_units", (double)1,
			    NULL);
      break;
    case GDK_BUTTON_PRESS:
      switch(event->button.button)
	{
	case 1:

	  x = item_x;
	  y = item_y;

	  gnome_canvas_item_raise_to_top(data->item);
	  gnome_canvas_item_raise_to_top(data->item_text);

	  fleur = gdk_cursor_new(GDK_FLEUR);
	  gc_canvas_item_grab(data->item,
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

	  gnome_canvas_item_move(data->item     , new_x - x, new_y - y);
	  gnome_canvas_item_move(data->item_text, new_x - x, new_y - y);
	  x = new_x;
	  y = new_y;
	}
      break;

    case GDK_BUTTON_RELEASE:
      if(dragging)
	{
	  gint i;
	  gint tmpi, tmpj;
	  double tmpx, tmpy;
	  PieceItem *piece_src;
	  PieceItem *piece_dst;
	  gint col = 0, line;

	  gc_canvas_item_ungrab(data->item, event->button.time);
	  dragging = FALSE;

	  /* Search the column (x) where this item is ungrabbed */
	  for(i=0; i<=number_of_item_x; i++)
	    if(position[i][0]->x   < item_x &&
	       position[i+1][0]->x > item_x)
	      col = i;

	  /* Bad drop / Outside of column area */
	  /* Bad drop / On the same column */
	  if(col<0 || col > number_of_item_x || col == data->i)
	    {
	      /* Return to the original position */
	      gc_item_absolute_move (data->item     , data->x , data->y);
	      gc_item_absolute_move (data->item_text, data->xt, data->yt);

	      /* FIXME : Workaround for bugged canvas */
	      gnome_canvas_update_now(gcomprisBoard->canvas);

	      return FALSE;
	    }


	  /* Now search the free line (y) */
	  line = number_of_item_y;
	  for(i=number_of_item_y-1; i>=0; i--)
	    if(position[col][i]->color == -1)
	      line = i;

	  /* Bad drop / Too many pieces here */
	  if(line >= number_of_item_y)
	    {
	      /* Return to the original position */
	      gc_item_absolute_move (data->item     , data->x , data->y);
	      gc_item_absolute_move (data->item_text, data->xt, data->yt);

	      /* FIXME : Workaround for bugged canvas */
	      gnome_canvas_update_now(gcomprisBoard->canvas);

	      return FALSE;
	    }

	  /* Update ontop values for the piece under the grabbed one */
	  if(data->j>0)
	    position[data->i][data->j-1]->on_top = TRUE;

	  /* Update ontop values for the piece under the ungrabbed one */
	  if(line>0)
	    position[col][line-1]->on_top = FALSE;

	  /* Move the piece */
	  piece_dst = position[col][line];
	  piece_src = data;
	  gc_item_absolute_move (data->item     , piece_dst->x , piece_dst->y);
	  gc_item_absolute_move (data->item_text, piece_dst->xt, piece_dst->yt);

	  /* FIXME : Workaround for bugged canvas */
	  gnome_canvas_update_now(gcomprisBoard->canvas);

	  /* Swap values in the pieces */
	  tmpx    = data->x;
	  tmpy    = data->y;
	  piece_src->x = piece_dst->x;
	  piece_src->y = piece_dst->y;
	  piece_dst->x = tmpx;
	  piece_dst->y = tmpy;

	  tmpx    = data->xt;
	  tmpy    = data->yt;
	  piece_src->xt = piece_dst->xt;
	  piece_src->yt = piece_dst->yt;
	  piece_dst->xt = tmpx;
	  piece_dst->yt = tmpy;

	  tmpi    = data->i;
	  tmpj    = data->j;
	  position[tmpi][tmpj]->i = piece_dst->i;
	  position[tmpi][tmpj]->j = piece_dst->j;
	  piece_dst->i  = tmpi;
	  piece_dst->j  = tmpj;

	  position[piece_src->i][piece_src->j] = piece_src;
	  position[piece_dst->i][piece_dst->j] = piece_dst;

	  //	  dump_solution();
	  if(is_completed())
	    {
	      gamewon = TRUE;
	      hanoi_destroy_all_items();
	      gc_bonus_display(gamewon, BONUS_SMILEY);
	    }
	}
      break;

    default:
      break;
    }


  return FALSE;
}

