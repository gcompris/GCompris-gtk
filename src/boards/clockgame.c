/* gcompris - clockgame.c
 *
 * Time-stamp: <2006/08/21 23:33:07 bruno>
 *
 * Copyright (C) 2000 Bruno Coudoin
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

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem *second_item;
static GnomeCanvasItem *hour_item;
static GnomeCanvasItem *minute_item;
static GnomeCanvasItem *digital_time_item;
static GnomeCanvasItem *digital_time_item_s;
static GnomeCanvasItem *time_to_find_item;
static GnomeCanvasItem *time_to_find_item_s;

/* Center of the clock and it's size */
double cx;
double cy;
double clock_size;

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
static void		 process_ok(void);

static int gamewon;
static void		 game_won(void);

static void		 clockgame_create_item(GnomeCanvasGroup *parent);
static void		 destroy_all_items(void);
static void		 get_random_hour(GcomprisTime *time);
static void		 clockgame_next_level(void);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
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
   process_ok,
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
      gchar *img;
      gcomprisBoard=agcomprisBoard;

      img = gc_skin_image_get("clockgame-bg.jpg");
      gc_set_background(gnome_canvas_root(gcomprisBoard->canvas), 
			      img);
      g_free(img);

      /* set initial values for this level adjusted to fit the watch background */
      cx =  gcomprisBoard->width/2;
      cy =  gcomprisBoard->height*0.4 + 42;
      clock_size = gcomprisBoard->height*0.3;

      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=6;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=3; /* Go to next level after this number of 'play' */
      gc_score_start(SCORESTYLE_NOTE,
			   gcomprisBoard->width - 220,
			   gcomprisBoard->height - 50,
			   gcomprisBoard->number_of_sublevel);
      gc_bar_set(GC_BAR_LEVEL|GC_BAR_OK);

      clockgame_next_level();

      gamewon = FALSE;
      pause_board(FALSE);

      gc_cursor_set(GCOMPRIS_LINE_CURSOR);
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
  gc_cursor_set(GCOMPRIS_DEFAULT_CURSOR);
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

  clockgame_create_item(gnome_canvas_root(gcomprisBoard->canvas));

}

/* ==================================== */
/* Destroy all the items */
static void
destroy_all_items()
{

  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}


static void display_digital_time(GnomeCanvasItem *item, GcomprisTime *time)
{
  gchar *text = NULL;

  if(item==NULL)
    return;

  if(gcomprisBoard->level<=2)
    text = g_strdup_printf("%.2d:%.2d", time->hour, time->minute);
  else
    text = g_strdup_printf("%.2d:%.2d:%.2d", time->hour, time->minute, time->second);

  gnome_canvas_item_set (item,
			 "text", text,
			 NULL);
  g_free(text);

}

static void display_hour(guint hour)
{
  double needle_size = clock_size*0.70;
  double ang;
  GnomeCanvasPoints *canvasPoints;
  canvasPoints = gnome_canvas_points_new (2);

  if(hour_item==NULL)
    return;

  /* Calc the needle angle */
  ang = ((hour > 12) ? hour-12 : hour) * M_PI / 6;
  ang += currentTime.minute * M_PI / 360;
  ang += currentTime.second * M_PI / 21600;

  canvasPoints->coords[0]=cx;
  canvasPoints->coords[1]=cy;
  canvasPoints->coords[2]=cx + needle_size * sin(ang);
  canvasPoints->coords[3]=cy - needle_size * cos(ang);
  gnome_canvas_item_set (hour_item,
			 "points", canvasPoints,
			 "fill_color", "darkblue",
			 "width_units", (double)1,
			 "width_pixels", (guint) 4,
			 "last_arrowhead", TRUE,
			 "arrow_shape_a", (double) needle_size,
			 "arrow_shape_b", (double) needle_size-20,
			 "arrow_shape_c", (double) 8.0,
			 NULL);
  gnome_canvas_points_free(canvasPoints);

  currentTime.hour=hour;
  display_digital_time(digital_time_item, &currentTime);
  display_digital_time(digital_time_item_s, &currentTime);
}


static void display_minute(guint minute)
{
  double needle_size = clock_size;
  double ang;
  GnomeCanvasPoints *canvasPoints;
  canvasPoints = gnome_canvas_points_new (2);

  if(minute_item==NULL)
    return;

  ang = minute * M_PI / 30;
  ang += currentTime.second * M_PI / 1800;

  canvasPoints->coords[0]=cx;
  canvasPoints->coords[1]=cy;
  canvasPoints->coords[2]=cx + needle_size * sin(ang);
  canvasPoints->coords[3]=cy - needle_size * cos(ang);
  gnome_canvas_item_set (minute_item,
			 "points", canvasPoints,
			 "fill_color", "red",
			 "width_units", (double)1,
			 "width_pixels", (guint) 4,
			 "last_arrowhead", TRUE,
			 "arrow_shape_a", (double) needle_size,
			 "arrow_shape_b", (double) needle_size-10,
			 "arrow_shape_c", (double) 3.0,
			 NULL);
  gnome_canvas_points_free(canvasPoints);

  currentTime.minute=minute;
  display_digital_time(digital_time_item, &currentTime);
}

static void display_second(guint second)
{
  double needle_size = clock_size;
  double ang;
  GnomeCanvasPoints *canvasPoints;
  canvasPoints = gnome_canvas_points_new (2);

  /* No seconds at first levels */
  if(second_item==NULL || gcomprisBoard->level<=2)
    return;

  ang = second * M_PI / 30;

  canvasPoints->coords[0]=cx;
  canvasPoints->coords[1]=cy;
  canvasPoints->coords[2]=cx + needle_size * sin(ang);
  canvasPoints->coords[3]=cy - needle_size * cos(ang);
  gnome_canvas_item_set (second_item,
			 "points", canvasPoints,
			 "fill_color_rgba", 0x68c46fFF,
			 "width_units", (double)1,
			 "width_pixels", (guint) 4,
			 "last_arrowhead", TRUE,
			 "arrow_shape_a", (double) 0,
			 "arrow_shape_b", (double) 0,
			 "arrow_shape_c", (double) 0,
			 NULL);
  gnome_canvas_points_free(canvasPoints);

  currentTime.second=second;
  display_digital_time(digital_time_item, &currentTime);
}


static void
clockgame_create_item(GnomeCanvasGroup *parent)
{
  GnomeCanvasItem *item;
  double needle_size = clock_size;
  double min_point_size = clock_size*0.05;
  double hour_point_size = clock_size*0.1;
  double ang;
  guint min;
  GnomeCanvasPoints *canvasPoints;
  char *color;
  char *color_text;
  gchar *mtext = NULL;
  gchar *font = NULL;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  canvasPoints = gnome_canvas_points_new (2);

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
      item = gnome_canvas_item_new (boardRootItem,
				    gnome_canvas_line_get_type (),
				    "points", canvasPoints,
				    "fill_color", color,
				    "width_units", (double)1,
				    "width_pixels", (guint) 2,
				    NULL);

      /* Display minute number */
      if(gcomprisBoard->level<5)
	{
	  mtext = g_strdup_printf("%d", min);
	  item = gnome_canvas_item_new (boardRootItem,
					gnome_canvas_text_get_type (),
					"text", mtext,
					"font", font,
					"x", (double) cx + (needle_size+10) * sin(ang),
					"y", (double) cy - (needle_size+10) * cos(ang),
					"anchor", GTK_ANCHOR_CENTER,
					"fill_color", color_text,
					"justification", GTK_JUSTIFY_CENTER,
					NULL);
	  g_free(mtext);
	}

      /* Display hour numbers */
      if(gcomprisBoard->level<6)
	if(min%5==0)
	  {
	    mtext = g_strdup_printf( "%d", min/5);
	    item = gnome_canvas_item_new (boardRootItem,
					  gnome_canvas_text_get_type (),
					  "text", mtext,
					  "font", font,
					  "x", (double) cx + (needle_size-30) * sin(ang),
					  "y", (double) cy - (needle_size-30) * cos(ang),
					  "anchor", GTK_ANCHOR_CENTER,
					  "fill_color", "blue",
					  "justification", GTK_JUSTIFY_CENTER,
					  NULL);
	    g_free(mtext);
	  }
    }

  /* Create the text area for the digital time display */
  if(gcomprisBoard->level<4)
    {
      digital_time_item_s =
	gnome_canvas_item_new (boardRootItem,
			       gnome_canvas_text_get_type (),
			       "text", "",
			       "font", gc_skin_font_board_medium,
			       "x", (double) cx + 1.0,
			       "y", (double) cy +  needle_size/2 + 1.0,
			       "anchor", GTK_ANCHOR_CENTER,
			       "fill_color_rgba", 0xc4c4c4ff,
			       NULL);
      display_digital_time(digital_time_item_s, &currentTime);

      digital_time_item =
	gnome_canvas_item_new (boardRootItem,
			       gnome_canvas_text_get_type (),
			       "text", "",
			       "font", gc_skin_font_board_medium,
			       "x", (double) cx,
			       "y", (double) cy +  needle_size/2,
			       "anchor", GTK_ANCHOR_CENTER,
			       "fill_color", "blue",
			       NULL);
      display_digital_time(digital_time_item, &currentTime);
    }
  else
    {
      digital_time_item_s = NULL;
      digital_time_item = NULL;
    }

  /* Create the Hour needle */

  canvasPoints->coords[0]=0;
  canvasPoints->coords[1]=0;
  canvasPoints->coords[2]=0;
  canvasPoints->coords[3]=0;
  hour_item = gnome_canvas_item_new (boardRootItem,
				     gnome_canvas_line_get_type (),
				     "points", canvasPoints,
				     "fill_color", "darkblue",
				     "width_units", (double)1,
				     "width_pixels", (guint) 0,
				     NULL);
  gtk_signal_connect(GTK_OBJECT(hour_item), "event",
		     (GtkSignalFunc) item_event,
		     NULL);
  display_hour(currentTime.hour);

  /* Create the minute needle */

  minute_item = gnome_canvas_item_new (boardRootItem,
				       gnome_canvas_line_get_type (),
				       "points", canvasPoints,
				       "fill_color", "darkblue",
				       "width_units", (double)1,
				       "width_pixels", (guint) 0,
				       NULL);
  gtk_signal_connect(GTK_OBJECT(minute_item), "event",
		     (GtkSignalFunc) item_event,
		     NULL);
  display_minute(currentTime.minute);

  /* Create the second needle */

  second_item = gnome_canvas_item_new (boardRootItem,
				       gnome_canvas_line_get_type (),
				       "points", canvasPoints,
				       "fill_color", "darkblue",
				       "width_units", (double)1,
				       "width_pixels", (guint) 0,
				       NULL);
  gtk_signal_connect(GTK_OBJECT(second_item), "event",
		     (GtkSignalFunc) item_event,
		     NULL);
  display_second(currentTime.second);

  /* Create the text area for the time to find display */
  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_text_get_type (),
			 "text", _("Set the watch to:"),
			 "font", gc_skin_font_board_small,
			 "x", (double) gcomprisBoard->width*0.17 + 1.0,
			 "y", (double) cy + needle_size +  needle_size / 3 - 30 + 1.0,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color_rgba", gc_skin_color_shadow,
			 NULL);

  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_text_get_type (),
			 "text", _("Set the watch to:"),
			 "font", gc_skin_font_board_small,
			 "x", (double) gcomprisBoard->width*0.17,
			 "y", (double) cy + needle_size +  needle_size / 3 - 30,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color_rgba", gc_skin_get_color("clockgame/text"),
			 NULL);

  time_to_find_item_s =
    gnome_canvas_item_new (boardRootItem,
			   gnome_canvas_text_get_type (),
			   "text", "",
			   "font", gc_skin_font_board_big_bold,
			   "x", (double) gcomprisBoard->width*0.17 + 1.0,
			   "y", (double) cy + needle_size +  needle_size / 3 + 1.0,
			   "anchor", GTK_ANCHOR_CENTER,
			   "fill_color_rgba", gc_skin_color_shadow,
			   NULL);
  display_digital_time(time_to_find_item_s, &timeToFind);

  time_to_find_item =
    gnome_canvas_item_new (boardRootItem,
			   gnome_canvas_text_get_type (),
			   "text", "",
			   "font", gc_skin_font_board_big_bold,
			   "x", (double) gcomprisBoard->width*0.17,
			   "y", (double) cy + needle_size +  needle_size / 3,
			   "anchor", GTK_ANCHOR_CENTER,
			   "fill_color_rgba", gc_skin_get_color("clockgame/text"),
			   NULL);
  display_digital_time(time_to_find_item, &timeToFind);

  gnome_canvas_points_free(canvasPoints);

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
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
      gc_bonus_end_display(BOARD_FINISHED_RANDOM);
      return;
    }
    gc_sound_play_ogg ("sounds/bonus.ogg", NULL);
  }
  clockgame_next_level();
}

/* ==================================== */
static void process_ok()
{
  if(time_equal(&timeToFind, &currentTime))
    {
      gamewon = TRUE;
    }
  else
    {
      /* Oups, you're wrong */
      gamewon = FALSE;
    }
  gc_bonus_display(gamewon, BONUS_FLOWER);
}


/* Callback for the 'toBeFoundItem' */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  static double x, y;
  double item_x, item_y;
  static int dragging;
  GdkCursor *fleur;
  double new_x, new_y;

  if(board_paused)
    return FALSE;

  item_x = event->button.x;
  item_y = event->button.y;
  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      x = item_x;
      y = item_y;

      fleur = gdk_cursor_new(GDK_FLEUR);
      gc_canvas_item_grab(item,
			     GDK_POINTER_MOTION_MASK |
			     GDK_BUTTON_RELEASE_MASK,
			     fleur,
			     event->button.time);
      gdk_cursor_destroy(fleur);
      dragging = TRUE;
      break;

    case GDK_MOTION_NOTIFY:
      if (dragging && (event->motion.state & GDK_BUTTON1_MASK))
	{

          double angle;

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
	      currentTime.hour++;

	    if(currentTime.minute < 15 && angle * 30 / M_PI > 45)
	      currentTime.hour--;

	    display_minute(angle * 30 / M_PI);
	    display_hour(currentTime.hour);
	  }
	  else if(item==second_item)
	  {
	    if(currentTime.second > 45 && angle * 30 / M_PI < 15)
	      currentTime.minute++;

	    if(currentTime.second < 15 && angle * 30 / M_PI > 45)
	      currentTime.minute--;

	    display_second(angle * 30 / M_PI);
	    display_minute(currentTime.minute);
	    display_hour(currentTime.hour);
	  }

          x = new_x + cx;
          y = new_y + cy;
	}
      break;

    case GDK_BUTTON_RELEASE:
      if(dragging)
	{
	  gc_canvas_item_ungrab(item, event->button.time);
	  dragging = FALSE;
	}
      break;

    default:
      break;
    }
  return FALSE;
}


/* Returns a random time based on the current level */
static void get_random_hour(GcomprisTime *time)
{

  time->hour=rand()%12;
  
  if(gcomprisBoard->level>3)
    time->second=rand()%60;
  else time->second=0;
  
  time->minute=rand()%60;

  switch(gcomprisBoard->level)
    {
    case 1:
      time->minute=rand()%4*15;
      break;
    case 2:
      time->minute=rand()%12*5;
      break;
    default:
      break;
    }

}
