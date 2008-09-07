/* gcompris - canal_lock.c
 *
 * Copyright (C) 2001, 2008 Bruno Coudoin
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
#define LOCK_HEIGHT_MIN		60
#define LOCK_RHEIGHT_MIN	160

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
static int	 from;

static gint timer_id;
static gboolean animation;

static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem	*lock_left_item		= NULL;
static GooCanvasItem	*lock_right_item	= NULL;

static GooCanvasItem	*canallock_left_item	= NULL;
static GooCanvasItem	*canallock_right_item	= NULL;

static GooCanvasItem	*canal_left_item	= NULL;
static GooCanvasItem	*canal_middle_item	= NULL;
static GooCanvasItem	*canal_right_item	= NULL;

static GooCanvasItem	*tuxboat_item		= NULL;
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

static double timer_item_limit_y, timer_item_limit_x;
static GooCanvasItem *timer_item;
static gint timer_step_y1, timer_step_x1;

static GooCanvasItem	*canal_lock_create_item(GooCanvasItem *parent);
static void		 canal_lock_destroy_all_items(void);
static void		 canal_lock_next_level(void);
static gboolean		 item_event (GooCanvasItem  *item,
				     GooCanvasItem  *target,
				     GdkEventButton *event,
				     gpointer data);
static gboolean          hightlight(GooCanvasItem *item,
                                    GooCanvasItem  *target,
                                    GdkEventButton *event,
                                    gpointer status);
static gboolean		 animate_step();
static void		 update_water();
static void		 toggle_lock(GooCanvasItem *item);

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
      gc_bar_location(0, -1, 0.7);

      animation = FALSE;

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

  gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			  "canal_lock/canal_lock_bg.png");

  gc_bar_set_level(gcomprisBoard);

  canal_lock_destroy_all_items();
  gamewon = FALSE;
  from = 0;

  /* Original state of the lock */
  boat_position = BOAT_POS_LEFT;
  right_way = TRUE;
  lock_left_up = TRUE;
  lock_right_up = TRUE;
  lock_water_low = TRUE;
  canallock_left_up = TRUE;
  canallock_right_up = TRUE;

  /* Try the next level */
  canal_lock_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void canal_lock_destroy_all_items()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}
/* ==================================== */
static GooCanvasItem *canal_lock_create_item(GooCanvasItem *parent)
{
  GdkPixbuf *pixmap = NULL;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  /* The boat */
  pixmap = gc_pixmap_load("canal_lock/tuxboat.png");

  tuxboat_item = \
    goo_canvas_image_new (boardRootItem,
			  pixmap,
			  (LEFT_CANAL_WIDTH - gdk_pixbuf_get_width(pixmap)) / 2,
			  BASE_LINE - LEFT_CANAL_HEIGHT - gdk_pixbuf_get_height(pixmap)*0.9,
			  NULL);
  g_signal_connect(tuxboat_item, "button-press-event",
                   (GtkSignalFunc) item_event,
                   NULL);
  gc_item_focus_init(tuxboat_item, NULL);

  tuxboat_width = gdk_pixbuf_get_width(pixmap);
  gdk_pixbuf_unref(pixmap);

  /* This is the ground canal */
  goo_canvas_rect_new (boardRootItem,
		       0,
		       BASE_LINE,
		       BOARDWIDTH,
		       BOARDHEIGHT - BASE_LINE,
		       "fill_color_rgba", GROUND_COLOR,
		       "line-width", (double) 0,
		       NULL);

  /* This is the left canal */
  canal_left_item = goo_canvas_rect_new (boardRootItem,
					 0,
					 BASE_LINE - LEFT_CANAL_HEIGHT,
					 LEFT_CANAL_WIDTH,
					 LEFT_CANAL_HEIGHT,
					 "fill_color_rgba", CANAL_COLOR,
					 "line-width", (double) 0,
					 NULL);

  /* This is the middle canal */
  canal_middle_item = goo_canvas_rect_new (boardRootItem,
					   LEFT_CANAL_WIDTH,
					   BASE_LINE - LEFT_CANAL_HEIGHT,
					   MIDDLE_CANAL_WIDTH,
					   LEFT_CANAL_HEIGHT,
					   "fill_color_rgba", CANAL_COLOR,
					   "line-width", (double) 0,
					   NULL);

  /* This is the right canal */
  canal_right_item = goo_canvas_rect_new (boardRootItem,
					  LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH,
					  BASE_LINE - RIGHT_CANAL_HEIGHT,
					  RIGHT_CANAL_WIDTH,
					  RIGHT_CANAL_HEIGHT,
					  "fill_color_rgba", CANAL_COLOR,
					  "line-width", (double) 0,
					  NULL);

  /* This is the left lock */
  lock_left_item = goo_canvas_rect_new (boardRootItem,
					LEFT_CANAL_WIDTH - LOCK_WIDTH / 2,
					BASE_LINE - LOCK_HEIGHT_MAX,
					LOCK_WIDTH,
					LOCK_HEIGHT_MAX,
					"fill_color_rgba", LOCK_COLOR,
					"line-width", (double) 0,
					NULL);
  g_signal_connect(lock_left_item, "button-press-event",
		   (GtkSignalFunc) item_event,
		   NULL);
  g_signal_connect(lock_left_item, "enter_notify_event",
		   (GtkSignalFunc) hightlight,
		   GINT_TO_POINTER(TRUE));
  g_signal_connect(lock_left_item, "leave_notify_event",
		   (GtkSignalFunc) hightlight,
		   GINT_TO_POINTER(FALSE));

  /* This is the right lock */
  lock_right_item = goo_canvas_rect_new (boardRootItem,
					 LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH - LOCK_WIDTH / 2,
					 BASE_LINE - LOCK_HEIGHT_MAX,
					 LOCK_WIDTH,
					 LOCK_HEIGHT_MAX,
					 "fill_color_rgba", LOCK_COLOR,
					 "line-width", (double) 0,
					 NULL);
  g_signal_connect(lock_right_item, "button-press-event",
		   (GtkSignalFunc) item_event,
		   NULL);
  g_signal_connect(lock_right_item, "enter_notify_event",
		   (GtkSignalFunc) hightlight,
		   GINT_TO_POINTER(TRUE));
  g_signal_connect(lock_right_item, "leave_notify_event",
		   (GtkSignalFunc) hightlight,
		   GINT_TO_POINTER(FALSE));

  /* This is the water conduit under the canal */
  goo_canvas_rect_new (boardRootItem,
		       LEFT_CANAL_WIDTH/2,
		       SUBCANAL_BASE_LINE - SUBCANAL_HEIGHT,
		       LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH + RIGHT_CANAL_WIDTH/2 - LEFT_CANAL_WIDTH/2,
		       SUBCANAL_HEIGHT,
		       "fill_color_rgba", CANAL_COLOR,
		       "line-width", (double) 0,
		       NULL);

  /* Left conduit */
  goo_canvas_rect_new (boardRootItem,
		       LEFT_CANAL_WIDTH/2,
		       BASE_LINE,
		       SUBCANAL_HEIGHT,
		       SUBCANAL_BASE_LINE - BASE_LINE,
		       "fill_color_rgba", CANAL_COLOR,
		       "line-width", (double) 0,
		       NULL);

  /* Middle conduit */
  goo_canvas_rect_new (boardRootItem,
		       LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH/2 - SUBCANAL_HEIGHT/2,
		       BASE_LINE,
		       SUBCANAL_HEIGHT/2,
		       SUBCANAL_BASE_LINE - BASE_LINE,
		       "fill_color_rgba", CANAL_COLOR,
		       "line-width", (double) 0,
		       NULL);

  /* Right conduit */
  goo_canvas_rect_new (boardRootItem,
		       LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH + RIGHT_CANAL_WIDTH/2,
		       BASE_LINE,
		       SUBCANAL_HEIGHT,
		       SUBCANAL_BASE_LINE - BASE_LINE,
		       "fill_color_rgba", CANAL_COLOR,
		       "line-width", (double) 0,
		       NULL);

  /* And to finish, the 2 canal locks */
  canallock_left_item =
    goo_canvas_rect_new (boardRootItem,
			 LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH * 0.1,
			 SUBCANAL_BASE_LINE - SUBCANAL_HEIGHT,
			 LOCK_WIDTH / 2,
			 SUBCANAL_HEIGHT,
			 "fill_color_rgba", CANALLOCK_COLOR,
			 "line-width", (double) 0,
			 NULL);
  g_signal_connect(canallock_left_item, "button-press-event",
		     (GtkSignalFunc) item_event,
		     NULL);
  g_signal_connect(canallock_left_item, "enter_notify_event",
		   (GtkSignalFunc) hightlight,
		   GINT_TO_POINTER(TRUE));
  g_signal_connect(canallock_left_item, "leave_notify_event",
		   (GtkSignalFunc) hightlight,
		   GINT_TO_POINTER(FALSE));

  canallock_right_item =
    goo_canvas_rect_new (boardRootItem,
			 LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH * 0.9,
			 SUBCANAL_BASE_LINE - SUBCANAL_HEIGHT,
			 LOCK_WIDTH / 2,
			 SUBCANAL_HEIGHT,
			 "fill_color_rgba", CANALLOCK_COLOR,
			 "line-width", (double) 0,
			 NULL);
  g_signal_connect(canallock_right_item, "button-press-event",
                   (GtkSignalFunc) item_event,
                   NULL);
  g_signal_connect(canallock_right_item, "enter_notify_event",
		   (GtkSignalFunc) hightlight,
		   GINT_TO_POINTER(TRUE));
  g_signal_connect(canallock_right_item, "leave_notify_event",
		   (GtkSignalFunc) hightlight,
		   GINT_TO_POINTER(FALSE));



  return NULL;
}
/* ==================================== */
/* Move the boat to the next possible position */
static void
move_boat()
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
      if (from == 1)
        {
          gamewon = TRUE;
          from = 0;
        }
    }
  else if(boat_position == BOAT_POS_MIDDLE && !lock_right_up)
    {
      boat_position = BOAT_POS_RIGHT;
      timer_item_limit_x = LEFT_CANAL_WIDTH + MIDDLE_CANAL_WIDTH + (RIGHT_CANAL_WIDTH - tuxboat_width)/2;
      timer_step_x1 = 2;
      if (from == 0)
        {
          gamewon = TRUE;
          from = 1;
        }
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

  gc_item_focus_remove(tuxboat_item, NULL);

  gc_sound_play_ogg ("sounds/eraser2.wav", NULL);

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
  gint min = BASE_LINE - LEFT_CANAL_HEIGHT;

  /* If there is already an animation do nothing else set
     animation to avoid deadlock */
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

  timer_item = canal_middle_item;
  timer_item_limit_y = (status ? min :
			y1);
  timer_step_y1 = (status ? 2 : -2);
  timer_step_x1 = 0;

  timer_id = gtk_timeout_add (ANIMATE_SPEED, (GtkFunction) animate_step, NULL);
}

/* ==================================== */
/* Toggle the given lock */
static void
toggle_lock(GooCanvasItem *item)
{
  gboolean status = TRUE;
  double y1 = 0;
  gint min = 0;
  guint animate_speed = 0;

  /* If there is already an animation do nothing else set animation to avoid deadlock */
  if(animation)
    return;
  animation = TRUE;

  gc_sound_play_ogg ("sounds/bleep.wav", NULL);

  if(item == lock_left_item)
    {
      status = lock_left_up;
      lock_left_up = !lock_left_up;
      y1 = BASE_LINE - LOCK_HEIGHT_MAX;
      min = BASE_LINE - LOCK_HEIGHT_MIN;
      animate_speed = ANIMATE_SPEED;
    }
  else if(item == lock_right_item)
    {
      status = lock_right_up;
      lock_right_up = !lock_right_up;
      y1 = BASE_LINE - LOCK_HEIGHT_MAX;
      min = BASE_LINE - LOCK_RHEIGHT_MIN;
      animate_speed = ANIMATE_SPEED;
    }
  else if(item == canallock_left_item)
    {
      status = canallock_left_up;
      canallock_left_up = !canallock_left_up;
      y1 = SUBCANAL_BASE_LINE - SUBCANAL_HEIGHT;
      min = SUBCANAL_BASE_LINE - CANALLOCK_HEIGHT_MIN;
      animate_speed = ANIMATE_SPEED;
    }
  else if(item == canallock_right_item)
    {
      status = canallock_right_up;
      canallock_right_up = !canallock_right_up;
      y1 = SUBCANAL_BASE_LINE - SUBCANAL_HEIGHT;
      min = SUBCANAL_BASE_LINE - CANALLOCK_HEIGHT_MIN;
      animate_speed = ANIMATE_SPEED;
    }

  timer_item = item;
  timer_item_limit_y = (status ? min :
			y1);
  timer_step_y1 = (status ? 2 : -2);
  timer_step_x1 = 0;

  timer_id = gtk_timeout_add (animate_speed, (GtkFunction) animate_step,
			      NULL);

}


/* ==================================== */
static gboolean
animate_step()
{

  if(!gcomprisBoard)
    return FALSE;

  GooCanvasBounds bounds;
  goo_canvas_item_get_bounds(timer_item, &bounds);

  if(GOO_IS_CANVAS_IMAGE(timer_item))
    g_object_set(timer_item,
		 "x", bounds.x1 + timer_step_x1,
		 "y", bounds.y1 + timer_step_y1,
		 NULL);
  else if(GOO_IS_CANVAS_RECT(timer_item))
    g_object_set(timer_item,
		 "x", bounds.x1 + timer_step_x1,
		 "y", bounds.y1 + timer_step_y1,
		 "height", bounds.y2 - bounds.y1 - timer_step_y1,
		 NULL);

  /* Special case for raising/lowering the boat */
  if(boat_position==BOAT_POS_MIDDLE && timer_item==canal_middle_item)
    {
      GooCanvasBounds bounds;

      goo_canvas_item_get_bounds(tuxboat_item, &bounds);

      g_object_set(tuxboat_item,
			    "y", bounds.y1 + timer_step_y1,
			    NULL);
    }

  if((bounds.y1 >= timer_item_limit_y && timer_step_y1 > 0) ||
     (bounds.y1 <= timer_item_limit_y && timer_step_y1 < 0))
    {
      gtk_timeout_remove (timer_id);
      timer_id = 0;
      animation = FALSE;
      update_water();
      if (timer_item == tuxboat_item)
        gc_item_focus_init(tuxboat_item, NULL);
    }
  else if((bounds.x1 >= timer_item_limit_x && timer_step_x1 > 0) ||
     (bounds.x1 <= timer_item_limit_x && timer_step_x1 < 0))
    {
      gtk_timeout_remove (timer_id);
      timer_id = 0;
      animation = FALSE;
      update_water();
      if (gamewon)
        {
          gc_bonus_display(TRUE, GC_BONUS_FLOWER);
          gamewon = FALSE;
        }
      if (timer_item == tuxboat_item)
        gc_item_focus_init(tuxboat_item, NULL);
    }

  return TRUE;
}

/* ==================================== */
/* Highlight the given item */
static gboolean
hightlight(GooCanvasItem *item,
           GooCanvasItem  *target,
           GdkEventButton *event,
           gpointer status_)
{
  guint color = 0;
  gboolean status = GPOINTER_TO_INT(status_);

  /* This is an image, not a rectangle */
  if(item == tuxboat_item)
    return FALSE;

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


  g_object_set(item,
               "fill_color_rgba", color,
               NULL);

  return TRUE;
}

/* ==================================== */
static gboolean
item_event (GooCanvasItem  *item,
	    GooCanvasItem  *target,
	    GdkEventButton *event,
	    gpointer data)
{
  if(board_paused)
    return FALSE;

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
  return FALSE;
}

