/* gcompris - hanoi_real.c
 *
 * Copyright (C) 2005 Bruno Coudoin
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

static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem	*hanoi_create_item(GooCanvasItem *parent);
static void		 hanoi_destroy_all_items(void);
static void		 hanoi_next_level(void);

/*
 * Contains the piece information
 */
typedef struct {
  GooCanvasItem *item;
  gint i;
  gint j;
  double x;
  double y;
  gboolean on_top;
  gint width;
} PieceItem;

static gboolean item_event (GooCanvasItem  *item,
			    GooCanvasItem  *target,
			    GdkEvent *event,
			    PieceItem *data);

/* This contains the layout of the pieces */
#define MAX_NUMBER_X 3
#define MAX_NUMBER_Y 10
static PieceItem *position[MAX_NUMBER_X][MAX_NUMBER_Y];

static int number_of_item = 0;
static int number_of_item_x = MAX_NUMBER_X;
static int number_of_item_y = 0;
static int item_width;
static int item_height;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Tower of Hanoi"),
    "",
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

GET_BPLUGIN_INFO(hanoi_real)

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
      gchar *img;

      gcomprisBoard=agcomprisBoard;
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=2;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gc_bar_set(GC_BAR_LEVEL);

      img = gc_skin_image_get("gcompris-bg.jpg");
      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			img);
      g_free(img);

      boardRootItem = NULL;

      gc_drag_start(goo_canvas_get_root_item(gcomprisBoard->canvas),
		    (GcDragFunc)item_event, GC_DRAG_MODE_DEFAULT);

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
      if(g_strcasecmp(gcomprisBoard->type, "hanoi_real")==0)
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
  number_of_item_y = gcomprisBoard->level + 2;

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
      for(i=0; i<number_of_item_x; i++)
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

  printf("Dumping solution\n");
  for(i=0; i<number_of_item_x; i++)
    {
      for(j=0; j<number_of_item_y; j++)
	{
	  printf("(%d,%d= width=%d top=%d) ",  position[i][j]->i, position[i][j]->j,
		    position[i][j]->width,
		    position[i][j]->on_top);
	}
      printf("\n");
    }

}
#endif

/* ==================================== */
static GooCanvasItem *
hanoi_create_item(GooCanvasItem *parent)
{
  int i,j;
  double gap_x, gap_y;
  double baseline;
  GooCanvasItem *item;
  GdkPixbuf *pixmap;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);

  pixmap = gc_skin_pixmap_load("gcompris-shapelabel.png");
  if(pixmap) {
    item = goo_canvas_image_new (boardRootItem,
				 pixmap,
				 10,
				 BOARDHEIGHT - 60,
				 NULL);
    goo_canvas_item_scale(item,
			  (double)(BOARDWIDTH - 20)/gdk_pixbuf_get_width(pixmap),
			  1);
    gdk_pixbuf_unref(pixmap);
  }

  goo_canvas_text_new (boardRootItem,
		       _("Move the entire stack to the right peg, one disc at a time"),
		       BOARDWIDTH/2 +1,
		       BOARDHEIGHT - 50 +1,
		       -1,
		       GTK_ANCHOR_NORTH,
		       "font", gc_skin_font_board_medium,
		       "fill_color_rgba", gc_skin_color_shadow,
		       "alignment", PANGO_ALIGN_CENTER,
		       NULL);

  goo_canvas_text_new (boardRootItem,
		       _("Move the entire stack to the right peg, one disc at a time"),
		       BOARDWIDTH/2,
		       BOARDHEIGHT - 50,
		       -1,
		       GTK_ANCHOR_NORTH,
		       "font", gc_skin_font_board_medium,
		       "fill_color_rgba", gc_skin_color_text_button,
		       "alignment", PANGO_ALIGN_CENTER,
		       NULL);


  /*----------------------------------------*/
  /* Empty the solution */
  for(i=0; i<number_of_item_x; i++)
    {
      for(j=0; j<number_of_item_y; j++)
	{
	  position[i][j] = g_malloc(sizeof(PieceItem));
	  position[i][j]->width  = -1;
	  position[i][j]->i      = i;
	  position[i][j]->j      = j;
	  position[i][j]->on_top = FALSE;
	}
    }

  /* Initialize the left positions */
  for(j=0; j<number_of_item_y; j++)
    {
      position[0][j]->width = number_of_item_y - j;
    }

  /* Mark the top piece */
  position[0][number_of_item_y-1]->on_top = TRUE;

  /*dump_solution();*/

  /*----------------------------------------*/
  /* Display it now */

  item_width  = BOARDWIDTH / (number_of_item_x);
  item_height = 30;

  gap_x = item_width  * 0.1;
  gap_y = item_height * 0.25;

  baseline = BOARDHEIGHT/2 + item_height * number_of_item_y/2;

  number_of_item = 0;

  for(i=0; i<number_of_item_x; i++)
    {
      if(i == number_of_item_x-1)
	{
	  /* Create the backgound for the target */
	  goo_canvas_rect_new (boardRootItem,
			       item_width * i + gap_x/2,
			       baseline - item_height * number_of_item_y - gap_y - 50,
			       item_width - gap_x,
			       baseline - item_height * number_of_item_y - gap_y,
			       "fill_color_rgba", 0x036ED8FF,
			       "stroke-color", "black",
			       "line-width", (double)1,
			       NULL);
	}

      /* The disc support */
      pixmap = gc_pixmap_load ("hanoi_real/disc_support.png");

      item = goo_canvas_image_new (boardRootItem,
				   pixmap,
				   item_width * i
				   + (item_width - gdk_pixbuf_get_width(pixmap))/2,
				   baseline - gdk_pixbuf_get_height(pixmap) + item_height,
				   NULL);

      gdk_pixbuf_unref(pixmap);


      for(j=0; j<number_of_item_y; j++)
	{

	  position[i][j]->x = item_width * i + item_width/2;
	  position[i][j]->y = baseline - item_height * j - item_height;

	  if(position[i][j]->width != -1)
	    {
	      pixmap = gc_pixmap_load ("%s%d.png", "hanoi_real/disc", j+1);
	      int w = gdk_pixbuf_get_width(pixmap);

	      item = goo_canvas_image_new (boardRootItem,
					   pixmap,
					   0, 0,
					   NULL);
	      gdk_pixbuf_unref(pixmap);
	      goo_canvas_item_translate(item,
					position[i][j]->x - w/2,
					position[i][j]->y);


	      position[i][j]->item = item;

	      g_signal_connect(item,
			       "button_press_event",
			       (GtkSignalFunc) gc_drag_event,  position[i][j]);
	      g_signal_connect(item,
			       "button_release_event",
			       (GtkSignalFunc) gc_drag_event,  position[i][j]);
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
      gc_bonus_end_display(GC_BOARD_FINISHED_RANDOM);
      return;
    }
    gc_sound_play_ogg ("sounds/bonus.wav", NULL);
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
      if(position[number_of_item_x-1][j]->width != number_of_item_y - j)
	done = FALSE;
    }

  return done;
}

/* ==================================== */
static gboolean
item_event (GooCanvasItem  *item,
	    GooCanvasItem  *target,
	    GdkEvent *event,
	    PieceItem *data)
{
   double item_x, item_y;

   if(!gcomprisBoard)
     return FALSE;

  if(board_paused)
    return FALSE;

  if(!data->on_top)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      switch(event->button.button)
	{
	case 1:

	  gc_drag_offset_save(event);
	  goo_canvas_item_raise(data->item, NULL);
	  break;
	}
      break;

    case GDK_MOTION_NOTIFY:
      gc_drag_item_move(event, NULL);
      break;

    case GDK_BUTTON_RELEASE:
	{
	  gint i;
	  gint tmpi, tmpj;
	  double tmpx, tmpy;
	  PieceItem *piece_src;
	  PieceItem *piece_dst;
	  gint line;
	  gint col=-1;
	  double disc_w, disc_h;
	  GooCanvasBounds bounds;

	  item_x = event->button.x;
	  item_y = event->button.y;

	  goo_canvas_item_get_bounds(data->item, &bounds);
	  disc_w = (bounds.x2 - bounds.x1)/2;
	  disc_h = (bounds.y2 - bounds.y1)/2;

	  gc_canvas_item_ungrab(data->item, event->button.time);

	  goo_canvas_convert_from_item_space(goo_canvas_item_get_canvas(data->item),
					   data->item,
					   &item_x, &item_y);

	  /* Search the column (x) where this item is ungrabbed */
	  if(item_x > (position[number_of_item_x-1][0]->x
		       - (position[number_of_item_x-1][0]->x - position[number_of_item_x-2][0]->x) / 2))
	     col = number_of_item_x-1;
	  else if(item_x < position[0][0]->x)
	    col = 0;
	  else {
	    for(i=0; i<number_of_item_x-1; i++) {
	      int distance = (position[i+1][0]->x - position[i][0]->x) / 2;
	      if(position[i][0]->x -  distance < item_x &&
		 position[i+1][0]->x - distance > item_x)
		col = i;
	    }
	  }

	  /* Bad drop / Outside of column area */
	  /* Bad drop / On the same column */
	  if(col<0 || col > number_of_item_x || col == data->i)
	    {
	      /* Return to the original position */
	      gc_item_absolute_move (data->item ,
				     data->x - disc_w,
				     data->y);

	      return FALSE;
	    }


	  /* Now search the free line (y) */
	  line = number_of_item_y;
	  for(i=number_of_item_y-1; i>=0; i--)
	    if(position[col][i]->width == -1)
	      line = i;

	  /* Bad drop / Too many pieces here or larger disc is above */
	  if(line > number_of_item_y ||
	     (line > 0 && position[col][line-1]->width != -1 && position[col][line-1]->width < data->width))
	    {
	      /* Return to the original position */
	      gc_item_absolute_move (data->item ,
				     data->x - disc_w,
				     data->y - disc_h);
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

	  gc_item_absolute_move (data->item,
				 piece_dst->x - disc_w,
				 piece_dst->y);

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

	  /*dump_solution();*/
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

