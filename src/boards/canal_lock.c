/* gcompris - canal_lock.c
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

#define ANIMATE_SPEED 30

#define CANAL_COLOR		0x0010FFFF
#define LOCK_COLOR		0x8cc679FF
#define LOCK_COLOR_H		0x71A65FFF
#define CANALLOCK_COLOR		0xd1cd0cFF
#define CANALLOCK_COLOR_H	0xf1ed1cFF
#define GROUND_COLOR		0x9b5f5aFF
#define BASE_LINE		400
#define LEFT_CANAL_HEIGHT	100
#define LEFT_CANAL_WIDTH	325
#define RIGHT_CANAL_HEIGHT	200
#define RIGHT_CANAL_WIDTH	325
#define MIDDLE_CANAL_WIDTH	(BOARDWIDTH - RIGHT_CANAL_WIDTH -  LEFT_CANAL_WIDTH)

#define LOCK_WIDTH		20
#define LOCK_HEIGHT_MAX		(RIGHT_CANAL_HEIGHT + 40)
#define LOCK_HEIGHT_MIN		20

#define SUBCANAL_BASE_LINE	(BASE_LINE + 80)
#define SUBCANAL_HEIGHT		40

#define CANALLOCK_WIDTH		30
#define CANALLOCK_HEIGHT_MAX	SUBCANAL_HEIGHT
#define CANALLOCK_HEIGHT_MIN	15

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static gint timer_id;
static gboolean animation;

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem	*lock_left_item		= NULL;
static GnomeCanvasItem	*lock_right_item	= NULL;

static GnomeCanvasItem	*canallock_left_item	= NULL;
static GnomeCanvasItem	*canallock_right_item	= NULL;

static GnomeCanvasItem	*canal_left_item	= NULL;
static GnomeCanvasItem	*canal_middle_item	= NULL;
static GnomeCanvasItem	*canal_right_item	= NULL;

static GnomeCanvasItem	*tuxboat_item		= NULL;
static double		 tuxboat_width;

#define BOAT_POS_LEFT		1
#define BOAT_POS_MIDDLE		2
#define BOAT_POS_RIGHT		3

static guint boat_position;
static gboolean right_way;
static gboolean lock_left_up;
static gboolean lock_right_up;
static gboolean lock_water_low;
static gboolean canallock_left_up;
static gboolean canallock_right_up;

static double timer_item_x1, timer_item_y1, timer_item_x2, timer_item_y2;
static double timer_item_limit_y, timer_item_limit_x;
static GnomeCanvasItem *timer_item;
static gint timer_step_y1, timer_step_x1;

static GnomeCanvasItem	*canal_lock_create_item(GnomeCanvasGroup *parent);
static void		 canal_lock_destroy_all_items(void);
static void		 canal_lock_next_level(void);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gboolean		 animate_step();
static void		 update_water();
static void		 toggle_lock(GnomeCanvasItem *item);

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Operate a canal lock",
    "Tux is in trouble in his ship. He needs to take it through a lock",
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

GET_BPLUGIN_INFO(canal_lock)

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
      gcomprisBoard->maxlevel=2;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */

      canal_lock_next_level();

      gc_bar_set(0);

      animation = FALSE;

      gamewon = FALSE;
      pause_board(FALSE);
    }
}
/* ======================================= */
static void end_board ()
{
  // If we don't stop animation, there may be a segfault if leaving while the animation is running
  if (timer_id) {
    gtk_timeout_remove (timer_id);
    timer_id = 0;
  }
  animation = FALSE;

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      canal_lock_destroy_all_items();
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
      canal_lock_next_level();
    }
}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "canal_lock")==0)
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
static void canal_lock_next_level()
{

  gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			  "canal_lock/canal_lock_bg.png");

  gc_bar_set_level(gcomprisBoard);

  canal_lock_destroy_all_items();
  gamewon = FALSE;

  /* Original state of the lock */
  boat_position = BOAT_POS_LEFT;
  right_way = TRUE;
  lock_left_up = TRUE;
  lock_right_up = TRUE;
  lock_water_low = TRUE;
  canallock_left_up = TRUE;
  canallock_right_up = TRUE;

  /* Try the next level */
  canal_lock_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void canal_lock_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
static GnomeCanvasItem *canal_lock_create_item(GnomeCanvasGroup *parent)
{
  GdkPixbuf *pixmap = NULL;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,

							    NULL));

  /* The boat */
  pixmap = gc_pixmap_load("gcompris/misc/tuxboat.png");

  tuxboat_item = gnome_canvas_item_new (boardRootItem,
					gnome_canvas_pixbuf_get_type (),
					"pixbuf",  pixmap, 
					"x", (double) (LEFT_CANAL_WIDTH - gdk_pixbuf_get_width(pixmap)) / 2,
					"y", (double) BASE_LINE - LEFT_CANAL_HEIGHT - gdk_pixbuf_get_height(pixmap)*0.9,
					NULL);
  gtk_signal_connect(GTK_OBJECT(tuxboat_item), "event",
		     (GtkSignalFunc) item_event,
		     NULL);
  gtk_signal_connect(GTK_OBJECT(tuxboat_item), "event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);
  tuxboat_width = gdk_pixbuf_get_width(pixmap);

  /* This is the ground canal */
  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_rect_get_type (),
			 "x1", (double) 0,
			 "y1", (double) BASE_LINE,
			 "x2", (double) BOARDWIDTH,
			 "y2", (double) BOARDHEIGHT,
			 "fill_color_rgba", GROUND_COLOR,
			 "width_units", (double) 0,
			 NULL);

  /* This is the left canal */
  canal_left_item = gnome_canvas_item_new (boardRootItem,
					    gnome_canvas_rect_get_type (),
					    "x1", (double) 0,
					    "y1", (double) BASE_LINE - LEFT_CANAL_HEIGHT,
					    "x2", (double) LEFT_CANAL_WIDTH,
					    "y2", (double) BASE_LINE,
					    "fill_color_rgba", CANAL_COLOR,
					    "width_units", (double) 0,
					    NULL);

  /* This is the middle canal */
  canal_middle_item = gnome_canvas_item_new (boardRootItem,
					     gnome_canvas_rect_get_type (),
					     "x1", (double) LEFT_CANAL_WIDTH,
					     "y1", (double) BASE_LINE - LEFT_CANAL_HEIGHT,
					     "x2", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH,
					     "y2", (double) BASE_LINE,
					     "fill_color_rgba", CANAL_COLOR,
					     "width_units", (double) 0,
					     NULL);

  /* This is the right canal */
  canal_right_item = gnome_canvas_item_new (boardRootItem,
					    gnome_canvas_rect_get_type (),
					    "x1", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH,
					    "y1", (double) BASE_LINE - RIGHT_CANAL_HEIGHT,
					    "x2", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH + RIGHT_CANAL_WIDTH,
					    "y2", (double) BASE_LINE,
					    "fill_color_rgba", CANAL_COLOR,
					    "width_units", (double) 0,
					    NULL);

  /* This is the left lock */
  lock_left_item = gnome_canvas_item_new (boardRootItem,
					  gnome_canvas_rect_get_type (),
					  "x1", (double) LEFT_CANAL_WIDTH - LOCK_WIDTH / 2,
					  "y1", (double) BASE_LINE - LOCK_HEIGHT_MAX,
					  "x2", (double) LEFT_CANAL_WIDTH + LOCK_WIDTH / 2,
					  "y2", (double) BASE_LINE,
					  "fill_color_rgba", LOCK_COLOR,
					  "width_units", (double) 0,
					  NULL);
  gtk_signal_connect(GTK_OBJECT(lock_left_item), "event",
		     (GtkSignalFunc) item_event,
		     NULL);

  /* This is the right lock */
  lock_right_item = gnome_canvas_item_new (boardRootItem,
					   gnome_canvas_rect_get_type (),
					   "x1", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH - LOCK_WIDTH / 2,
					   "y1", (double) BASE_LINE - LOCK_HEIGHT_MAX,
					   "x2", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH + LOCK_WIDTH / 2,
					   "y2", (double) BASE_LINE,
					   "fill_color_rgba", LOCK_COLOR,
					   "width_units", (double) 0,
					   NULL);
  gtk_signal_connect(GTK_OBJECT(lock_right_item), "event",
		     (GtkSignalFunc) item_event,
		     NULL);

  /* This is the water conduit under the canal */
  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_rect_get_type (),
			 "x1", (double) LEFT_CANAL_WIDTH/2,
			 "y1", (double) SUBCANAL_BASE_LINE - SUBCANAL_HEIGHT,
			 "x2", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH + RIGHT_CANAL_WIDTH / 2 + SUBCANAL_HEIGHT,
			 "y2", (double) SUBCANAL_BASE_LINE,
			 "fill_color_rgba", CANAL_COLOR,
			 "width_units", (double) 0,
			 NULL);

  /* Left conduit */
  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_rect_get_type (),
			 "x1", (double) LEFT_CANAL_WIDTH/2,
			 "y1", (double) BASE_LINE,
			 "x2", (double) LEFT_CANAL_WIDTH/2 + SUBCANAL_HEIGHT,
			 "y2", (double) SUBCANAL_BASE_LINE,
			 "fill_color_rgba", CANAL_COLOR,
			 "width_units", (double) 0,
			 NULL);

  /* Middle conduit */
  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_rect_get_type (),
			 "x1", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH/2 - SUBCANAL_HEIGHT/2,
			 "y1", (double) BASE_LINE,
			 "x2", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH/2 + SUBCANAL_HEIGHT/2,
			 "y2", (double) SUBCANAL_BASE_LINE,
			 "fill_color_rgba", CANAL_COLOR,
			 "width_units", (double) 0,
			 NULL);

  /* Right conduit */
  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_rect_get_type (),
			 "x1", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH + RIGHT_CANAL_WIDTH/2,
			 "y1", (double) BASE_LINE,
			 "x2", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH + RIGHT_CANAL_WIDTH/2 + SUBCANAL_HEIGHT,
			 "y2", (double) SUBCANAL_BASE_LINE,
			 "fill_color_rgba", CANAL_COLOR,
			 "width_units", (double) 0,
			 NULL);

  /* And to finish, the 2 canal locks */
  canallock_left_item = 
    gnome_canvas_item_new (boardRootItem,
			   gnome_canvas_rect_get_type (),
			   "x1", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH * 0.1,
			   "y1", (double) SUBCANAL_BASE_LINE - SUBCANAL_HEIGHT,
			   "x2", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH * 0.1 + LOCK_WIDTH / 2,
			   "y2", (double) SUBCANAL_BASE_LINE,
			   "fill_color_rgba", CANALLOCK_COLOR,
			   "width_units", (double) 0,
			   NULL);
  gtk_signal_connect(GTK_OBJECT(canallock_left_item), "event",
		     (GtkSignalFunc) item_event,
		     NULL);

  canallock_right_item = 
    gnome_canvas_item_new (boardRootItem,
			   gnome_canvas_rect_get_type (),
			   "x1", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH * 0.9,
			   "y1", (double) SUBCANAL_BASE_LINE - SUBCANAL_HEIGHT,
			   "x2", (double) LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH * 0.9 + LOCK_WIDTH / 2,
			   "y2", (double) SUBCANAL_BASE_LINE,
			   "fill_color_rgba", CANALLOCK_COLOR,
			   "width_units", (double) 0,
			   NULL);
    gtk_signal_connect(GTK_OBJECT(canallock_right_item), "event",
		     (GtkSignalFunc) item_event,
		     NULL);



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
  canal_lock_next_level();
}

/* ==================================== */
/* Move the boat to the next possible position */
static void move_boat()
{

  /* If there is already an animation do nothing else set animation to avoid deadlock */
  if(animation)
    return;
  animation = TRUE;

  if(boat_position == BOAT_POS_LEFT && !lock_left_up)
    {
      boat_position = BOAT_POS_MIDDLE;
      timer_item_limit_x = LEFT_CANAL_WIDTH + (MIDDLE_CANAL_WIDTH - tuxboat_width)/2;
      timer_step_x1 = 2;
    }
  else if(boat_position == BOAT_POS_MIDDLE && !lock_left_up)
    {
      boat_position = BOAT_POS_LEFT;
      timer_item_limit_x = (LEFT_CANAL_WIDTH - tuxboat_width)/2;
      timer_step_x1 = -2;
    }
  else if(boat_position == BOAT_POS_MIDDLE && !lock_right_up)
    {
      boat_position = BOAT_POS_RIGHT;
      timer_item_limit_x = LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH + (RIGHT_CANAL_WIDTH - tuxboat_width)/2;
      timer_step_x1 = 2;
    }
  else if(boat_position == BOAT_POS_RIGHT && !lock_right_up)
    {
      boat_position = BOAT_POS_MIDDLE;
      timer_item_limit_x = LEFT_CANAL_WIDTH + (MIDDLE_CANAL_WIDTH - tuxboat_width)/2;
      timer_step_x1 = -2;
    }
  else
    {
      /* No possible move */
      gc_sound_play_ogg ("sounds/crash.ogg", NULL);
      animation = FALSE;
      return;
    }

  gnome_canvas_item_get_bounds(tuxboat_item, &timer_item_x1, &timer_item_y1, 
			       &timer_item_x2, &timer_item_y2);

  timer_item = tuxboat_item;
  timer_step_y1 = 0;

  timer_id = gtk_timeout_add (ANIMATE_SPEED, (GtkFunction) animate_step, NULL);
}

/* ==================================== */
/* Update the water level if necessary */
static void update_water()
{
  gboolean status = TRUE;
  double y1 = 0;
  gint min = LEFT_CANAL_HEIGHT;

  /* If there is already an animation do nothing else set animation to avoid deadlock */
  if(animation)
    return;
  animation = TRUE;

  if((!canallock_left_up && !lock_water_low) ||
     (!canallock_right_up && lock_water_low))
    {
      status = !lock_water_low;
      lock_water_low = !lock_water_low;
      y1 = BASE_LINE - RIGHT_CANAL_HEIGHT;
    }
  else 
    {
      /* The water level is correct */
      animation = FALSE;
      return;
    }

  gnome_canvas_item_get_bounds(canal_middle_item, &timer_item_x1, &timer_item_y1, 
			       &timer_item_x2, &timer_item_y2);

  timer_item = canal_middle_item;
  timer_item_limit_y = (status ? timer_item_y2 - min :
			y1);
  timer_step_y1 = (status ? 2 : -2);
  timer_step_x1 = 0;

  timer_id = gtk_timeout_add (ANIMATE_SPEED, (GtkFunction) animate_step, NULL);
}

/* ==================================== */
/* Toggle the given lock */
static void toggle_lock(GnomeCanvasItem *item)
{
  gboolean status = TRUE;
  double y1 = 0;
  gint min = 0;
  guint animate_speed = 0;

  /* If there is already an animation do nothing else set animation to avoid deadlock */
  if(animation)
    return;
  animation = TRUE;

  gnome_canvas_item_get_bounds(item, &timer_item_x1, &timer_item_y1, 
			       &timer_item_x2, &timer_item_y2);

  if(item == lock_left_item)
    {
      status = lock_left_up;
      lock_left_up = !lock_left_up;
      y1 = BASE_LINE - LOCK_HEIGHT_MAX;
      min = LOCK_HEIGHT_MIN;
      animate_speed = ANIMATE_SPEED;
    }
  else if(item == lock_right_item)
    {
      status = lock_right_up;
      lock_right_up = !lock_right_up;
      y1 = BASE_LINE - LOCK_HEIGHT_MAX;
      min = LOCK_HEIGHT_MIN;
      animate_speed = ANIMATE_SPEED;
    }
  else if(item == canallock_left_item)
    {
      status = canallock_left_up;
      canallock_left_up = !canallock_left_up;
      y1 = SUBCANAL_BASE_LINE - SUBCANAL_HEIGHT;
      min = CANALLOCK_HEIGHT_MIN;
      animate_speed = ANIMATE_SPEED;
    }
  else if(item == canallock_right_item)
    {
      status = canallock_right_up;
      canallock_right_up = !canallock_right_up;
      y1 = SUBCANAL_BASE_LINE - SUBCANAL_HEIGHT;
      min = CANALLOCK_HEIGHT_MIN;
      animate_speed = ANIMATE_SPEED;
    }

  timer_item = item;
  timer_item_limit_y = (status ? timer_item_y2 - min :
			y1);
  timer_step_y1 = (status ? 2 : -2);
  timer_step_x1 = 0;

  timer_id = gtk_timeout_add (animate_speed, (GtkFunction) animate_step, NULL);

}


/* ==================================== */
static gboolean animate_step() 
{

  if(!gcomprisBoard)
    return FALSE;

  timer_item_x1 += timer_step_x1;
  timer_item_y1 += timer_step_y1;

  if(GNOME_IS_CANVAS_PIXBUF(timer_item))
    gnome_canvas_item_set(timer_item, 
			  "x", timer_item_x1,
			  "y", timer_item_y1,
			  NULL);
  else if(GNOME_IS_CANVAS_RECT(timer_item))
    gnome_canvas_item_set(timer_item, 
			  "x1", timer_item_x1,
			  "y1", timer_item_y1,
			  NULL);

  /* Special case for raising/lowering the boat */
  if(boat_position==BOAT_POS_MIDDLE && timer_item==canal_middle_item)
    {
      double item_x1, item_y1, item_x2, item_y2;

      gnome_canvas_item_get_bounds(tuxboat_item, &item_x1, &item_y1, 
				   &item_x2, &item_y2);

      gnome_canvas_item_set(tuxboat_item, 
			    "y", item_y1 + timer_step_y1,
			    NULL);
    }

  if((timer_item_y1 >= timer_item_limit_y && timer_step_y1 > 0) ||
     (timer_item_y1 <= timer_item_limit_y && timer_step_y1 < 0))
    {
      gtk_timeout_remove (timer_id);
      timer_id = 0;
      animation = FALSE;
      update_water();
    }
  else if((timer_item_x1 >= timer_item_limit_x && timer_step_x1 > 0) ||
     (timer_item_x1 <= timer_item_limit_x && timer_step_x1 < 0))
    {
      gtk_timeout_remove (timer_id);
      timer_id = 0;
      animation = FALSE;
      update_water();
    }

  gnome_canvas_update_now(gcomprisBoard->canvas);

  return TRUE;
}

/* ==================================== */
/* Highlight the given item */
static void hightlight(GnomeCanvasItem *item, gboolean status)
{
  guint color = 0;
  
  /* This is an image, not a rectangle */
  if(item == tuxboat_item)
    return;

  if(item == lock_left_item ||
     item == lock_right_item)
    {
      color   = (status ? LOCK_COLOR_H : LOCK_COLOR);
    }
  else if (item == canallock_left_item ||
	   item == canallock_right_item)
    {
      color   = (status ? CANALLOCK_COLOR_H : CANALLOCK_COLOR);
    }


    gnome_canvas_item_set(item,
			  "fill_color_rgba", color,
			  NULL);

}

/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  double item_x, item_y;
  item_x = event->button.x;
  item_y = event->button.y;
  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

  if(board_paused)
    return FALSE;

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      hightlight(item, TRUE);
      break;
    case GDK_LEAVE_NOTIFY:
      hightlight(item, FALSE);
      break;
    case GDK_BUTTON_PRESS:

      /* If there is already an animation do nothing */
      if(animation)
	return FALSE;
	   
      if(item == lock_left_item)
	{
	  if(lock_water_low && canallock_right_up)
	      toggle_lock(item);
	  else
	    gc_sound_play_ogg ("sounds/crash.ogg", NULL);

	}
      else if(item == lock_right_item)
	{
	  if(!lock_water_low && canallock_left_up)
	      toggle_lock(item);
	  else
	    gc_sound_play_ogg ("sounds/crash.ogg", NULL);
	}
      else if(item == canallock_left_item && canallock_right_up)
	{
	  if(lock_right_up)
	      toggle_lock(item);
	  else
	    gc_sound_play_ogg ("sounds/crash.ogg", NULL);
	}
      else if(item == canallock_right_item && canallock_left_up)
	{
	  if(lock_left_up)
	      toggle_lock(item);
	  else
	    gc_sound_play_ogg ("sounds/crash.ogg", NULL);
	}
      else if(item == tuxboat_item)
	{
	  move_boat();
	}
      else
	{
	  gc_sound_play_ogg ("sounds/crash.ogg", NULL);
	}
      break;
    default:
      break;
    }
  return FALSE;


  /*
    gamewon = TRUE;
    canal_lock_destroy_all_items();
    gc_bonus_display(gamewon, BONUS_SMILEY);
  */
  return FALSE;
}

