/* gcompris - hanoi.c
 *
 * Copyright (C) 2003, 2008 Bruno Coudoin
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

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem	*hanoi_create_item(GooCanvasItem *parent);
static void		 hanoi_destroy_all_items(void);
static void		 hanoi_next_level(void);

/*
 * Contains the piece information
 */
typedef struct {
  GooCanvasItem *group;
  gint i;
  gint j;
  double x;
  double y;
  gboolean on_top;
  gint color;
} PieceItem;

static gint item_event(GooCanvasItem *item,
		       GooCanvasItem *target,
		       GdkEvent *event, PieceItem *data);

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
    0x00C0C0FF,
    0xA00000FF,
    0xF00000FF,
    0x00A000FF,
    0x00F000FF,
    0x0000AAFF,
    0x0000FFFF,
    0x505000FF,
    0xA0A000FF,
    0xC0C080FF,
    0x005050FF,
    0x00A0A0FF,
    0x500050FF,
    0xA000A0FF,
    0xF000F0FF
  };
#define NUMBER_OF_COLOR G_N_ELEMENTS(colorlist)

static char symbollist [NUMBER_OF_COLOR] =
  {
    '!',
    '/',
    '<',
    '>',
    '&',
    '~',
    '#',
    '{',
    '%',
    '|',
    '?',
    '}',
    '=',
    '+',
    '*'
  };

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
static void
pause_board (gboolean pause)
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
static void
start_board (GcomprisBoard *agcomprisBoard)
{

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=6;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gc_bar_set(GC_BAR_LEVEL);

      gc_set_default_background(goo_canvas_get_root_item(gcomprisBoard->canvas));

      gc_drag_start(goo_canvas_get_root_item(gcomprisBoard->canvas),
		    (GcDragFunc)item_event,
		    GC_DRAG_MODE_DEFAULT);

      hanoi_next_level();

      gamewon = FALSE;
      pause_board(FALSE);
    }
}
/* ======================================= */
static void
end_board ()
{
  if(gcomprisBoard!=NULL)
    {
      gc_drag_stop(goo_canvas_get_root_item(gcomprisBoard->canvas));
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
      if(g_ascii_strcasecmp(gcomprisBoard->type, "hanoi")==0)
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
  hanoi_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void hanoi_destroy_all_items()
{
  guint i,j;

  if(boardRootItem!=NULL)
    {
      goo_canvas_item_remove(boardRootItem);

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
  g_warning("Piece: (%d,%d=%2d/%d)\n",
	    piece->i,  piece->j,
	    piece->color, piece->on_top);
}
#endif

/* ==================================== */
static GooCanvasItem *
hanoi_create_item(GooCanvasItem *parent)
{
  int i,j;
  double gap_x, gap_y;
  double baseline;
  GooCanvasItem *item = NULL;
  guint color_to_place;
  guint used_colors[NUMBER_OF_COLOR];
  guint w;

  boardRootItem = \
    goo_canvas_group_new (parent,
			  NULL);


  if (gcomprisBoard->level == 1)
    {
      item = goo_canvas_svg_new (boardRootItem,
				 gc_skin_rsvg_get(),
				 "svg-id", "#BAR_BG",
				 NULL);
      SET_ITEM_LOCATION_CENTER(item,
			       BOARDWIDTH/2,
			       50);

      goo_canvas_text_new (boardRootItem,
                           _("Build the same tower in the empty area as the one you see on the right-hand side."),
                           BOARDWIDTH/2,
                           50,
                           -1,
                           GTK_ANCHOR_CENTER,
                           "font", gc_skin_font_board_medium,
                           "fill_color_rgba", gc_skin_color_text_button,
                           NULL);
    }

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

  /* Initialize a random goal and store the color index
     in position[number_of_item_x] */
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
	      while(used_colors[color])
		{
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
      if(i == number_of_item_x+1)
	{
	  /* Create the backgound for the target */
	  goo_canvas_rect_new (boardRootItem,
			       item_width * i + gap_x/2,
			       baseline - item_height * number_of_item_y - gap_y - 50,
			       item_width - gap_x,
			       item_height * number_of_item_y + gap_y*2 + 100,
			       "fill_color_rgba", 0x036ED8FF,
			       "stroke-color", "black",
			       "line-width", (double)1,
			       NULL);
	}
      else if (i == number_of_item_x)
	{
	  /* Create the backgound for the empty area */
	  goo_canvas_rect_new (boardRootItem,
			       item_width * i + gap_x/2,
			       baseline - item_height * number_of_item_y - gap_y - 50,
			       item_width - gap_x,
			       item_height * number_of_item_y + gap_y*2 + 100,
			       "fill_color_rgba", 0x48AAF1FF,
			       "stroke-color", "black",
			       "line-width", (double)1,
			       NULL);
	}

      /* Create the vertical line */
      w = 10;
      goo_canvas_rect_new (boardRootItem,
			   item_width * i + item_width/2 - w,
			   baseline - item_height * number_of_item_y - gap_y,
			   w*2,
			   (item_height + gap_y/2 - 2) * number_of_item_y,
			   "fill_color_rgba", 0xFF1030FF,
			   "stroke-color", "black",
			   "line-width", (double)1,
			   NULL);

      /* And the base line */
      item = goo_canvas_path_new (boardRootItem,
				  "M 43,19 A 22,20 0 1 1 -1,19 L 20,19 z",
				  "fill_color_rgba", 0xFF1030FF,
				  "stroke-color", "black",
				  "line-width", 1.0,
				  NULL);
      goo_canvas_item_translate(item,
				item_width * i + item_width/2 - 20,
				baseline - 25);

      for(j=0; j<number_of_item_y; j++)
	{

	  position[i][j]->x = item_width * i + gap_x;
	  position[i][j]->y = baseline - item_height * j - item_height + gap_y;

	  if(position[i][j]->color != -1)
	    {
	      char car[2];

	      GooCanvasItem *group = goo_canvas_group_new(boardRootItem,
							  NULL);
	      goo_canvas_item_translate(group,
					position[i][j]->x,
					position[i][j]->y);

	      position[i][j]->group = group;

	      item = goo_canvas_rect_new (group,
					  0,
					  0,
					  item_width - gap_x * 2,
					  item_height - gap_y,
					  "fill_color_rgba",
					  colorlist[position[i][j]->color],
					  "stroke-color", "black",
					  "line-width", (double)1,
					  NULL);

	      car[0] = symbollist[position[i][j]->color];
	      car[1] = '\0';

	      goo_canvas_text_new (group,
				   (char *)car,
				   20,
				   0,
				   -1,
				   GTK_ANCHOR_NORTH,
				   "font", "sans bold 14",
				   "fill-color", "white",
				   NULL);

	      if(i != number_of_item_x+1)
		{
		  g_signal_connect(item, "button_press_event",
				   (GCallback)gc_drag_event,
				   position[i][j]);
		  g_signal_connect(item, "button_release_event",
				   (GCallback)gc_drag_event,
				   position[i][j]);

		  g_signal_connect(item, "enter_notify_event",
				   (GCallback) item_event,
				   position[i][j]);
		  g_signal_connect(item, "leave_notify_event",
				   (GCallback) item_event,
				   position[i][j]);
		}
	    }

	}
    }

  return NULL;
}
/* ==================================== */
static void
game_won()
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
  hanoi_next_level();
}

/*
 * Returns TRUE is the goal is reached
 */
static gboolean
is_completed()
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
item_event(GooCanvasItem *item,
	   GooCanvasItem *target,
	   GdkEvent *event,
	   PieceItem *data)
{
  double item_x, item_y;

  if(!gcomprisBoard)
    return FALSE;

  if(board_paused)
    return FALSE;

  if(data && !data->on_top)
    return FALSE;

  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
      g_object_set(item,
		   "stroke-color", "white",
		   "line-width", (double)3,
		   NULL);
      break;
    case GDK_LEAVE_NOTIFY:
      g_object_set(item,
		   "stroke-color", "black",
		   "line-width", (double)1,
		   NULL);
      break;
    case GDK_BUTTON_PRESS:
      switch(event->button.button)
	{
	case 1:
	  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
	  gc_drag_offset_save(event);
	  goo_canvas_item_raise(data->group, NULL);
	  break;
	}
      break;

    case GDK_MOTION_NOTIFY:
      gc_drag_item_move(event, data->group);
      break;

    case GDK_BUTTON_RELEASE:
	{
	  gint i;
	  gint tmpi, tmpj;
	  double tmpx, tmpy;
	  PieceItem *piece_src;
	  PieceItem *piece_dst;
	  gint col = 0, line;

	  item_x = event->button.x;
	  item_y = event->button.y;
	  goo_canvas_convert_from_item_space(goo_canvas_item_get_canvas(item),
					     item,
					     &item_x, &item_y);

	  /* Search the column (x) where this item is ungrabbed */
	  for(i=0; i<=number_of_item_x; i++)
	    if(position[i][0]->x   < item_x &&
	       position[i+1][0]->x > item_x)
	      col = i;

	  /* Bad drop / Outside of column area */
	  /* Bad drop / On the same column */
	  if(col<0 || col > number_of_item_x || col == data->i)
	    {
	      gc_sound_play_ogg ("sounds/eraser2.wav", NULL);

	      /* Return to the original position */
	      gc_item_absolute_move (data->group, data->x , data->y);
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
	      gc_sound_play_ogg ("sounds/eraser2.wav", NULL);

	      /* Return to the original position */
	      gc_item_absolute_move (data->group, data->x , data->y);

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
	  gc_item_absolute_move (data->group, piece_dst->x , piece_dst->y);

	  gc_sound_play_ogg ("sounds/scroll.wav", NULL);

	  /* Swap values in the pieces */
	  tmpx    = data->x;
	  tmpy    = data->y;
	  piece_src->x = piece_dst->x;
	  piece_src->y = piece_dst->y;
	  piece_dst->x = tmpx;
	  piece_dst->y = tmpy;

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
	      gc_bonus_display(gamewon, GC_BONUS_SMILEY);
	    }
	}
      break;

    default:
      break;
    }


  return FALSE;
}

