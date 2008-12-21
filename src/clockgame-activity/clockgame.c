/* gcompris - clockgame.c
 *
 * Copyright (C) 2000, 2008 Bruno Coudoin
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

#include <ctype.h>
#include <math.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem *second_item;
static GooCanvasItem *hour_item;
static GooCanvasItem *minute_item;
static GooCanvasItem *digital_time_item;
static GooCanvasItem *time_to_find_item;

static gboolean dragging = FALSE;
static double drag_x, drag_y;

/* Center of the clock and it's size */
static double cx;
static double cy;
static double clock_size;

typedef struct {
  guint hour;
  guint minute;
  guint second;
} GcomprisTime;
static GcomprisTime timeToFind, currentTime;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);

static int gamewon;
static void		 game_won(void);

static void		 clockgame_create_item(GooCanvasItem *parent);
static void		 destroy_all_items(void);
static void		 get_random_hour(GcomprisTime *time);
static void		 clockgame_next_level(void);

static gboolean		 on_button_release (GooCanvasItem *item,
					    GooCanvasItem *target,
					    GdkEventButton *event,
					    gpointer data);

static gboolean		 on_button_press (GooCanvasItem *item,
					  GooCanvasItem *target,
					  GdkEventButton *event,
					  gpointer data);

static gboolean		 on_motion_notify (GooCanvasItem *item,
					   GooCanvasItem *target,
					   GdkEventMotion *event,
					   gpointer data);

static void		 display_hour(guint hour);
static void		 display_minute(guint minute);
static void		 display_second(guint second);
static gboolean		 time_equal(GcomprisTime *time1, GcomprisTime *time2);

/* Description of this plugin */
static BoardPlugin menu_bp =
{
   NULL,
   NULL,
   "Learning Clock",
   "Learn how to tell the time",
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

GET_BPLUGIN_INFO(clockgame)

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

      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			      "clockgame/clockgame-bg.svgz");

      /* set initial values for this level adjusted to
	 fit the watch background */
      cx =  BOARDWIDTH/2 + 16;
      cy =  BOARDHEIGHT*0.4 + 42;
      clock_size = BOARDHEIGHT*0.3;

      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=6;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=3; /* Go to next level after this number of 'play' */
      gc_score_start(SCORESTYLE_NOTE,
		     BOARDWIDTH - 195,
		     30,
			   gcomprisBoard->number_of_sublevel);
      gc_bar_set(GC_BAR_LEVEL);
      gc_bar_location(BOARDWIDTH - 200, -1, 0.8);

      clockgame_next_level();

      gamewon = FALSE;
      pause_board(FALSE);

    }

}

static void
end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gc_score_end();
      destroy_all_items();
    }
  gcomprisBoard = NULL;
}

static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      clockgame_next_level();
    }
}

static gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "clockgame")==0)
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
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

/* set initial values for the next level */
static void clockgame_next_level()
{

  gc_bar_set_level(gcomprisBoard);
  gc_score_set(gcomprisBoard->sublevel);

  destroy_all_items();

  /* Try the next level */
  get_random_hour(&timeToFind);

  /* Avoid to show up the solution directly */
  do {
    get_random_hour(&currentTime);
  } while(time_equal(&timeToFind, &currentTime));

  clockgame_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));

}

/* ==================================== */
/* Destroy all the items */
static void
destroy_all_items()
{

  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}


static void display_digital_time(GooCanvasItem *item, GcomprisTime *time)
{
  gchar *text = NULL;
  int temps;

  temps = (time->hour+12)*3600 + time->minute*60 + time->second;
  time->hour = (temps / 3600) % 12;
  time->minute = (temps / 60) % 60;
  time->second = temps % 60;

  if(item==NULL)
    return;

  if(gcomprisBoard->level<=2)
    text = g_strdup_printf("%.2d:%.2d", time->hour, time->minute);
  else
    text = g_strdup_printf("%.2d:%.2d:%.2d", time->hour, time->minute, time->second);

  g_object_set (item,
		"text", text,
		NULL);
  g_free(text);

}

static void display_hour(guint hour)
{
  double needle_size = clock_size*0.70;
  double ang;
  GooCanvasPoints *canvasPoints;

  if(hour_item==NULL)
    return;

  /* Calc the needle angle */
  ang = ((hour > 12) ? hour-12 : hour) * M_PI / 6;
  ang += currentTime.minute * M_PI / 360;
  ang += currentTime.second * M_PI / 21600;

  canvasPoints = goo_canvas_points_new (2);
  canvasPoints->coords[0]=cx;
  canvasPoints->coords[1]=cy;
  canvasPoints->coords[2]=cx + needle_size * sin(ang);
  canvasPoints->coords[3]=cy - needle_size * cos(ang);
  double w = 4.0;
  g_object_set (hour_item,
		"points", canvasPoints,
		"fill-color", "darkblue",
		"stroke-color", "red",
		"line-width", w,
		"end-arrow", TRUE,
		"arrow-tip-length", needle_size/w,
		"arrow-length", 4.0,
		"arrow-width", 4.0,
		NULL);
  goo_canvas_points_unref(canvasPoints);

  currentTime.hour=hour;
  display_digital_time(digital_time_item, &currentTime);
}

static void display_minute(guint minute)
{
  double needle_size = clock_size;
  double ang;
  GooCanvasPoints *canvasPoints;

  if(minute_item==NULL)
    return;

  ang = minute * M_PI / 30;
  ang += currentTime.second * M_PI / 1800;

  canvasPoints = goo_canvas_points_new (2);
  canvasPoints->coords[0]=cx;
  canvasPoints->coords[1]=cy;
  canvasPoints->coords[2]=cx + needle_size * sin(ang);
  canvasPoints->coords[3]=cy - needle_size * cos(ang);
  double w = 4.0;
  g_object_set (minute_item,
		"points", canvasPoints,
		"stroke-color", "red",
		"fill-color", "red",
		"line-width", w,
		"end-arrow", TRUE,
		"arrow-tip-length", needle_size/w,
		"arrow-length", (double) 4.0,
		"arrow-width", (double) 3.0,
		NULL);
  goo_canvas_points_unref(canvasPoints);

  currentTime.minute=minute;
  display_digital_time(digital_time_item, &currentTime);
}

static void display_second(guint second)
{
  double needle_size = clock_size;
  double ang;
  GooCanvasPoints *canvasPoints;

  /* No seconds at first levels */
  if(second_item==NULL || gcomprisBoard->level<=2)
    return;

  ang = second * M_PI / 30;

  canvasPoints = goo_canvas_points_new (2);
  canvasPoints->coords[0]=cx;
  canvasPoints->coords[1]=cy;
  canvasPoints->coords[2]=cx + needle_size * sin(ang);
  canvasPoints->coords[3]=cy - needle_size * cos(ang);
  g_object_set (second_item,
		"points", canvasPoints,
		"stroke-color", "red",
		"fill_color_rgba", 0x68c46fFF,
		"line-width", 4.0,
		NULL);
  goo_canvas_points_unref(canvasPoints);

  currentTime.second=second;
  display_digital_time(digital_time_item, &currentTime);
}


static void
clockgame_create_item(GooCanvasItem *parent)
{
  GooCanvasItem *item;
  double needle_size = clock_size;
  double min_point_size = clock_size*0.05;
  double hour_point_size = clock_size*0.1;
  double ang;
  guint min;
  GooCanvasPoints *canvasPoints;
  char *color;
  char *color_text;
  gchar *mtext = NULL;
  gchar *font = NULL;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  canvasPoints = goo_canvas_points_new (2);

  for(min = 1 ; min <= 60 ; min += 1)
    {
      ang = min * M_PI / 30;
      if(min%5==0)
	{
	  /* Hour point */
	  canvasPoints->coords[0]=cx + (needle_size-hour_point_size) * sin(ang);
	  canvasPoints->coords[1]=cy - (needle_size-hour_point_size) * cos(ang);
	  color="darkblue";
	  color_text="red";
	  font = gc_skin_font_board_small;
	}
      else
	{
	  /* Minute point */
	  canvasPoints->coords[0]=cx + (needle_size-min_point_size) * sin(ang);
	  canvasPoints->coords[1]=cy - (needle_size-min_point_size) * cos(ang);
	  color="red";
	  color_text="red";
	  font = gc_skin_font_board_tiny;
	}

      canvasPoints->coords[2]=cx + needle_size * sin(ang);
      canvasPoints->coords[3]=cy - needle_size * cos(ang);
      item = goo_canvas_polyline_new (boardRootItem, FALSE, 2,
				      canvasPoints->coords[0],
				      canvasPoints->coords[1],
				      canvasPoints->coords[2],
				      canvasPoints->coords[3],
				      "stroke-color", color,
				      "line-width", (double)1,
				      NULL);

      /* Display minute number */
      if(gcomprisBoard->level<5)
	{
	  mtext = g_strdup_printf("%d", min);
	  item = goo_canvas_text_new (boardRootItem,
				      mtext,
				      (double) cx + (needle_size+10) * sin(ang),
				      (double) cy - (needle_size+10) * cos(ang),
				      -1,
				      GTK_ANCHOR_CENTER,
				      "font", font,
				      "fill-color", color_text,
				      NULL);
	  g_free(mtext);
	}

      /* Display hour numbers */
      if(gcomprisBoard->level<6)
	if(min%5==0)
	  {
	    mtext = g_strdup_printf( "%d", min/5);
	    item = goo_canvas_text_new (boardRootItem,
					mtext,
					(double) cx + (needle_size-30) * sin(ang),
					(double) cy - (needle_size-30) * cos(ang),
					-1,
					GTK_ANCHOR_CENTER,
					"font", font,
					"fill-color", "blue",
					NULL);
	    g_free(mtext);
	  }
    }

  /* Create the text area for the digital time display */
  if(gcomprisBoard->level<4)
    {
      digital_time_item =
	goo_canvas_text_new (boardRootItem,
			     "",
			     (double) cx,
			     (double) cy +  needle_size/2,
			     -1,
			     GTK_ANCHOR_CENTER,
			     "font", gc_skin_font_board_medium,
			     "fill-color", "blue",
			     NULL);
      display_digital_time(digital_time_item, &currentTime);
    }
  else
    {
      digital_time_item = NULL;
    }

  /* Create the Hour needle */

  hour_item = goo_canvas_polyline_new (boardRootItem, FALSE, 0,
				       NULL);
  g_signal_connect (hour_item, "motion_notify_event",
		    (GtkSignalFunc) on_motion_notify, NULL);
  g_signal_connect (hour_item, "button_press_event",
		    (GtkSignalFunc) on_button_press, NULL);
  g_signal_connect (hour_item, "button_release_event",
		    (GtkSignalFunc) on_button_release, NULL);
  display_hour(currentTime.hour);

  /* Create the minute needle */

  minute_item = goo_canvas_polyline_new (boardRootItem, FALSE, 0,
					 NULL);
  g_signal_connect (minute_item, "motion_notify_event",
		    (GtkSignalFunc) on_motion_notify, NULL);
  g_signal_connect (minute_item, "button_press_event",
		    (GtkSignalFunc) on_button_press, NULL);
  g_signal_connect (minute_item, "button_release_event",
		    (GtkSignalFunc) on_button_release, NULL);
  display_minute(currentTime.minute);

  /* Create the second needle */

  second_item = goo_canvas_polyline_new (boardRootItem, FALSE, 0,
					 NULL);
  g_signal_connect (second_item, "motion_notify_event",
		    (GtkSignalFunc) on_motion_notify, NULL);
  g_signal_connect (second_item, "button_press_event",
		    (GtkSignalFunc) on_button_press, NULL);
  g_signal_connect (second_item, "button_release_event",
		    (GtkSignalFunc) on_button_release, NULL);
  display_second(currentTime.second);

  /* Create the text area for the time to find display */
  goo_canvas_text_new (boardRootItem,
		       _("Set the watch to:"),
		       (double) BOARDWIDTH*0.17,
		       (double) cy + needle_size +  needle_size / 3 - 30,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_board_small,
		       "fill_color_rgba", gc_skin_get_color("clockgame/text"),
		       NULL);

  time_to_find_item = \
    goo_canvas_text_new (boardRootItem,
			 "",
			 (double) BOARDWIDTH*0.17,
			 (double) cy + needle_size +  needle_size / 3,
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_board_big_bold,
			 "fill_color_rgba", gc_skin_get_color("clockgame/text"),
			 NULL);
  display_digital_time(time_to_find_item, &timeToFind);

  goo_canvas_points_unref(canvasPoints);

}

/*
 * Returns true is given times are equal
 */
static gboolean
time_equal(GcomprisTime *time1, GcomprisTime *time2)
{
  /* No seconds at first levels */
  if(second_item==NULL || gcomprisBoard->level<=2)
    return(time1->hour==time2->hour
	   &&time1->minute==time2->minute);
  else
    return(time1->hour==time2->hour
	   &&time1->minute==time2->minute
	   &&time1->second==time2->second);

}

/* ==================================== */
static void game_won()
{
  gamewon = FALSE;
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel)
      gcomprisBoard->level = gcomprisBoard->maxlevel;

    gc_sound_play_ogg ("sounds/bonus.wav", NULL);
  }
  clockgame_next_level();
}

static gboolean
on_button_press (GooCanvasItem *item,
		 GooCanvasItem *target,
		 GdkEventButton *event,
		 gpointer data)
{
  GooCanvas *canvas;
  GdkCursor *fleur;

  canvas = goo_canvas_item_get_canvas (item);

  switch (event->button)
    {
    case 1:
      {
	gc_sound_play_ogg ("sounds/bleep.wav", NULL);

	goo_canvas_item_raise(item, NULL);

	drag_x = event->x;
	drag_y = event->y;

	fleur = gdk_cursor_new (GDK_FLEUR);
	goo_canvas_pointer_grab (canvas, item,
				 GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
				 fleur,
				 event->time);
	gdk_cursor_unref (fleur);
	dragging = TRUE;
      }
      break;

    default:
      break;
    }

  return TRUE;
}


static gboolean
on_button_release (GooCanvasItem *item,
		   GooCanvasItem *target,
		   GdkEventButton *event,
		   gpointer data)
{
  GooCanvas *canvas;

  canvas = goo_canvas_item_get_canvas (item);
  goo_canvas_pointer_ungrab (canvas, item, event->time);
  dragging = FALSE;

  if(time_equal(&timeToFind, &currentTime))
    {
      gamewon = TRUE;
      gc_bonus_display(gamewon, GC_BONUS_FLOWER);
    }
  return TRUE;
}


static gboolean
on_motion_notify (GooCanvasItem *item,
		  GooCanvasItem *target,
		  GdkEventMotion *event,
		  gpointer data)
{
  if (dragging && (event->state & GDK_BUTTON1_MASK))
    {
      double angle;
      double item_x, item_y;
      double new_x, new_y;

      item_x = event->x;
      item_y = event->y;

      goo_canvas_convert_from_item_space(goo_canvas_item_get_canvas(item),
					 item, &item_x, &item_y);


      /* only use coords relative to the center, with standard
	 direction for axis (y's need to be negated for this) */
      new_x = item_x - cx;
      new_y = - item_y + cy;

      /* angle as mesured relatively to noon position */
      /* Thanks to Martin Herweg for this code        */
      angle =  atan2(new_x,new_y);
      if (angle<0) {angle= angle + 2.0*M_PI;}

      if(item==hour_item)
	display_hour(angle * 6 / M_PI);
      else if(item==minute_item)
	{
	  if(currentTime.minute > 45 && angle * 30 / M_PI < 15)
	    {
	      currentTime.hour++;
	      gc_sound_play_ogg ("sounds/paint1.wav", NULL);
	    }

	  if(currentTime.minute < 15 && angle * 30 / M_PI > 45)
	    {
	      currentTime.hour--;
	      gc_sound_play_ogg ("sounds/paint1.wav", NULL);
	    }

	  display_minute(angle * 30 / M_PI);
	  display_hour(currentTime.hour);
	}
      else if(item==second_item)
	{
	  if(currentTime.second > 45 && angle * 30 / M_PI < 15)
	    {
	      currentTime.minute++;
	      gc_sound_play_ogg ("sounds/paint1.wav", NULL);
	    }

	  if(currentTime.second < 15 && angle * 30 / M_PI > 45)
	    {
	      currentTime.minute--;
	      gc_sound_play_ogg ("sounds/paint1.wav", NULL);
	    }

	  display_second(angle * 30 / M_PI);
	  display_minute(currentTime.minute);
	  display_hour(currentTime.hour);
	}
    }

  return TRUE;
}


/* Returns a random time based on the current level */
static void get_random_hour(GcomprisTime *time)
{

  time->hour=g_random_int()%12;

  if(gcomprisBoard->level>3)
    time->second=g_random_int()%60;
  else time->second=0;

  time->minute=g_random_int()%60;

  switch(gcomprisBoard->level)
    {
    case 1:
      time->minute=g_random_int()%4*15;
      break;
    case 2:
      time->minute=g_random_int()%12*5;
      break;
    default:
      break;
    }

}
