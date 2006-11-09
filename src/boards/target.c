/* gcompris - target.c
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

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);
static int gamewon;
static void		 game_won(void);

#define VERTICAL_SEPARATION	30
#define HORIZONTAL_SEPARATION	30
#define TEXT_COLOR		"white"

static GnomeCanvasGroup *boardRootItem = NULL;
static GnomeCanvasGroup *speedRootItem = NULL;

static double wind_speed;
static double ang;

static GnomeCanvasItem  *answer_item = NULL;
static gchar		 answer_string[10];
static guint		 answer_string_index = 0;

static GnomeCanvasItem *animate_item = NULL;
static gint		animate_id = 0;
static gint		animate_item_distance = 0;
static gint		animate_item_size = 0;
static double		animate_item_x = 0;
static double		animate_item_y = 0;
#define MAX_DART_SIZE 20
#define MIN_DART_SIZE 3

static guint		user_points = 0;

/*
 * Functions Definition
 */
static void		process_ok(void);
static gint		key_press(guint keyval, gchar *commit_str, gchar *preedit_str);
static GnomeCanvasItem *target_create_item(GnomeCanvasGroup *parent);
static void		target_destroy_all_items(void);
static void		target_next_level(void);
static gint		item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static void		animate_items(void);
static void		launch_dart(double item_x, double item_y);

/*
 * distance to target, min wind speed, max wind speed, target1 width(center),
 * value1 (for target1), ... to target10, value10
 *
 * a target width of 0 means no such target
 */
#define MAX_NUMBER_OF_TARGET 10

typedef struct {
  guint number_of_arrow;
  guint target_distance;
  guint target_min_wind_speed;
  guint target_max_wind_speed;
  gint target_width_value[MAX_NUMBER_OF_TARGET*2];
} TargetDefinition;


/*
 * Definition of targets one line by level based on TargetDefinition
 */
static TargetDefinition targetDefinition[] =
{
  { 3, 100, 2, 5,  { 40, 5 , 80, 3, 150, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
  { 5, 150, 2, 7,  { 30, 10, 50, 5, 150, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
  { 7, 200, 4, 9,  { 20, 10, 40, 5, 60, 3, 150, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} },
  { 7, 200, 5, 10, { 15, 100, 35, 50, 55, 10, 75, 5, 150, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} }
};
#define NUMBER_OF_TARGET 4

#define TARGET_CENTER_X	235
#define TARGET_CENTER_Y	260

#define SPEED_CENTER_X	660
#define SPEED_CENTER_Y	125

static guint target_colors[] = {
  0xAA0000FF, 0x00AA00FF, 0x0000AAFF, 0xAAAA00FF, 0x00AAAAFF, 0xAA00AAFF, 0xAA0000FF, 0x00AA00FF, 0x0000AAFF, 0xAA0000FF
};

static guint number_of_arrow = 0;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Practice addition with a target game",
    "Hit the target and count your points",
    "Bruno Coudoin <bruno.coudoin@free.fr>",
    NULL,
    NULL,
    NULL,
    NULL,
    start_board,
    pause_board,
    end_board,
    is_our_board,
    key_press,
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

GET_BPLUGIN_INFO(target)

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

  if(pause)
    {
      if (animate_id) {
	gtk_timeout_remove (animate_id);
	animate_id = 0;
      }
    }
  else
    {
      if(animate_item) {
	animate_id = gtk_timeout_add (200, (GtkFunction) animate_items, NULL);
      }
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

      /* disable im_context */
      gcomprisBoard->disable_im_context = TRUE;

      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=NUMBER_OF_TARGET;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */

      gc_set_background(gnome_canvas_root(gcomprisBoard->canvas), "images/target_background.jpg");

      target_next_level();

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
      target_destroy_all_items();
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
      target_next_level();
    }
}


/* ======================================= */
static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
{
  guint c;

  if(!gcomprisBoard)
    return FALSE;

  /* Add some filter for control and shift key */
  switch (keyval)
    {
      /* Avoid all this keys to be interpreted by this game */
    case GDK_Shift_L:
    case GDK_Shift_R:
    case GDK_Control_L:
    case GDK_Control_R:
    case GDK_Caps_Lock:
    case GDK_Shift_Lock:
    case GDK_Meta_L:
    case GDK_Meta_R:
    case GDK_Alt_L:
    case GDK_Alt_R:
    case GDK_Super_L:
    case GDK_Super_R:
    case GDK_Hyper_L:
    case GDK_Hyper_R:
    case GDK_Num_Lock:
      return FALSE;
    case GDK_KP_Enter:
    case GDK_Return:
      process_ok();
      return TRUE;
    case GDK_Right:
    case GDK_Left:
      break;
    case GDK_BackSpace:
      if(answer_string_index>0)
	{
	  answer_string_index--;
	  answer_string[answer_string_index] = 0;
	}
      break;
    }

  c = tolower(keyval);

  /* Limit the user entry to 5 digits */
  if(c>='0' && c<='9' && answer_string_index<5)
    {
      answer_string[answer_string_index++] = c;
      answer_string[answer_string_index] = 0;
    }

  if(answer_item)
    {
      gchar *tmpstr = g_strdup_printf(_("Points = %s"), answer_string);
      gnome_canvas_item_set(answer_item,
			    "text", tmpstr,
			    NULL);
      g_free(tmpstr);
    }

  return TRUE;
}


/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "target")==0)
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
static void target_next_level()
{

  gc_bar_set(GC_BAR_LEVEL);
  gc_bar_set_level(gcomprisBoard);

  target_destroy_all_items();
  gamewon = FALSE;

  /* Try the next level */
  target_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void target_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;

  if(speedRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(speedRootItem));

  animate_item = NULL;
  answer_item = NULL;
  answer_string_index = 0;
  user_points = 0;

  speedRootItem = NULL;
}

/*
 * Display a random wind speed
 */
static void display_windspeed()
{
  guint second = 0;
  guint needle_zoom = 15;
  gchar *tmpstr;
  GnomeCanvasPoints *canvasPoints;
  canvasPoints = gnome_canvas_points_new (2);

  if(speedRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(speedRootItem));

  speedRootItem = GNOME_CANVAS_GROUP(
				    gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							   gnome_canvas_group_get_type (),
							   "x", (double) 0,
							   "y", (double) 0,
							   NULL));

  /* Speed orientation */
  second = g_random_int()%60;
  ang = second * M_PI / 30;

  /* Speed force */
  wind_speed = targetDefinition[gcomprisBoard->level-1].target_min_wind_speed \
    + g_random_int()%(targetDefinition[gcomprisBoard->level-1].target_max_wind_speed \
	      - targetDefinition[gcomprisBoard->level-1].target_min_wind_speed);

  canvasPoints->coords[0]=SPEED_CENTER_X;
  canvasPoints->coords[1]=SPEED_CENTER_Y;
  canvasPoints->coords[2]=SPEED_CENTER_X + wind_speed * sin(ang) * needle_zoom;
  canvasPoints->coords[3]=SPEED_CENTER_Y - wind_speed * cos(ang) * needle_zoom;
  gnome_canvas_item_new (speedRootItem,
			 gnome_canvas_line_get_type (),
			 "points", canvasPoints,
			 "fill_color_rgba", 0x6df438FF,
			 "width_units", (double)1,
			 "width_pixels", (guint) 4,
			 "last_arrowhead", TRUE,
			 "arrow_shape_a", (double) wind_speed,
			 "arrow_shape_b", (double) wind_speed-15,
			 "arrow_shape_c", (double) 5.0,
			 NULL);

  gnome_canvas_points_free(canvasPoints);

  /* Draw the center of the speedometer */
  gnome_canvas_item_new (speedRootItem,
			 gnome_canvas_ellipse_get_type(),
			 "x1", (double)SPEED_CENTER_X-5,
			 "y1", (double)SPEED_CENTER_Y-5,
			 "x2", (double)SPEED_CENTER_X+5,
			 "y2", (double)SPEED_CENTER_Y+5,
			 "fill_color_rgba", 0x6df438FF,
			 "outline_color", "red",
			 "width_units", (double)1,
			 NULL);

  tmpstr = g_strdup_printf(_("Wind speed = %d\nkilometers/hour"), (guint)wind_speed);
  gnome_canvas_item_new (speedRootItem,
			 gnome_canvas_text_get_type (),
			 "text", tmpstr,
			 "font", gc_skin_font_board_medium,
			 "x", (double) SPEED_CENTER_X,
			 "y", (double) SPEED_CENTER_Y + 110,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color", "white",
			 NULL);
  g_free(tmpstr);

}



/* ==================================== */
static GnomeCanvasItem *target_create_item(GnomeCanvasGroup *parent)
{
  int i;
  gchar *tmpstr;
  GnomeCanvasItem *item = NULL;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (parent,
							    gnome_canvas_group_get_type (),
							    "x", (double) TARGET_CENTER_X,
							    "y", (double) TARGET_CENTER_Y,
							    NULL));
  for(i=0; i<MAX_NUMBER_OF_TARGET; i++)
    {
      if(targetDefinition[gcomprisBoard->level-1].target_width_value[i*2]>0)
	{
	  item = gnome_canvas_item_new (boardRootItem,
					gnome_canvas_ellipse_get_type(),
					"x1", (double)-targetDefinition[gcomprisBoard->level-1].target_width_value[i*2],
					"y1", (double)-targetDefinition[gcomprisBoard->level-1].target_width_value[i*2],
					"x2", (double)targetDefinition[gcomprisBoard->level-1].target_width_value[i*2],
					"y2", (double)targetDefinition[gcomprisBoard->level-1].target_width_value[i*2],
					"fill_color_rgba", target_colors[i],
					"outline_color", "black",
					"width_units", (double)1,
					NULL);

	  gnome_canvas_item_lower_to_bottom(item);
	  gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event, NULL);

	  /* Display the value for this target */
	  tmpstr = g_strdup_printf("%d",
				   targetDefinition[gcomprisBoard->level-1].target_width_value[i*2+1]);
	  item = gnome_canvas_item_new (boardRootItem,
					gnome_canvas_text_get_type (),
					"text", tmpstr,
					"font", gc_skin_font_board_medium,
					"x", (double) 0,
					"y", (double) targetDefinition[gcomprisBoard->level-1].target_width_value[i*2] - 10,
					"anchor", GTK_ANCHOR_CENTER,
					"fill_color", "white",
					NULL);
	  g_free(tmpstr);

	  gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event, NULL);
	}
    }

  number_of_arrow = targetDefinition[gcomprisBoard->level-1].number_of_arrow;

  tmpstr = g_strdup_printf(_("Distance to target = %d meters"),
			   targetDefinition[gcomprisBoard->level-1].target_distance);
  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_text_get_type (),
			 "text", tmpstr,
			 "font", gc_skin_font_board_medium,
			 "x", (double) 0,
			 "y", (double) BOARDHEIGHT-TARGET_CENTER_Y -45,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color", "white",
			 NULL);
  g_free(tmpstr);

  display_windspeed();

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
  target_next_level();
}

static void process_ok()
{
  guint answer_points = atoi(answer_string);

  if(answer_item) {

    if(answer_points == user_points)
      {
	gamewon = TRUE;
	target_destroy_all_items();
	gc_bonus_display(gamewon, BONUS_SMILEY);
      }
    else
      {
	gamewon = FALSE;
	gc_bonus_display(gamewon, BONUS_SMILEY);
      }
  }

}

/*
 * Request score
 *
 */
static void request_score()
{
  GdkPixbuf *button_pixmap = NULL;
  double y_offset = 160;
  double x_offset = 245;
  gchar *tmpstr;

  gc_bar_set(GC_BAR_LEVEL|GC_BAR_OK);
  button_pixmap = gc_skin_pixmap_load("button_large2.png");
  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_pixbuf_get_type (),
			 "pixbuf",  button_pixmap,
			 "x", x_offset,
			 "y", y_offset,
			 NULL);

  tmpstr =  g_strdup_printf(_("Points = %s"), "");
  answer_item = gnome_canvas_item_new (boardRootItem,
				       gnome_canvas_text_get_type (),
				       "text", tmpstr,
				       "font", gc_skin_font_board_title_bold,
				       "x", (double) x_offset + gdk_pixbuf_get_width(button_pixmap)/2,
				       "y", (double) y_offset + gdk_pixbuf_get_height(button_pixmap)/2,
				       "anchor", GTK_ANCHOR_CENTER,
				       "fill_color", "white",
				       NULL);
  g_free(tmpstr);

  gdk_pixbuf_unref(button_pixmap);
}

static void add_points(double x, double y)
{
  guint i;
  double diametre;

  // Calculate the distance
  diametre = sqrt(x*x+y*y);

  for(i=0; i<MAX_NUMBER_OF_TARGET; i++)
    {
      if(diametre < targetDefinition[gcomprisBoard->level-1].target_width_value[i*2])
	{
	  user_points += targetDefinition[gcomprisBoard->level-1].target_width_value[i*2+1];
	  break;
	}
    }

}


/*
 * Dart animation
 *
 */
static void animate_items()
{
  if(board_paused)
    return;

  if(!animate_item)
    return;

  // Apply the wind move
  animate_item_x = animate_item_x + wind_speed * sin(ang);
  animate_item_y = animate_item_y - wind_speed * cos(ang);

  gnome_canvas_item_set (animate_item,
			 "x1", (double)animate_item_x - animate_item_size,
			 "y1", (double)animate_item_y - animate_item_size,
			 "x2", (double)animate_item_x + animate_item_size,
			 "y2", (double)animate_item_y + animate_item_size,
			 NULL);

  if(animate_item_size>MIN_DART_SIZE)
    animate_item_size--;

  if(animate_item_distance-- == 0)
    {
      gtk_timeout_remove (animate_id);
      animate_id = 0;
      animate_item = NULL;

      // Calc the point for this dart
      add_points(animate_item_x, animate_item_y);

      // Change the wind for the next target
      display_windspeed();
    }

}

/*
 *
 */
static void launch_dart(double item_x, double item_y)
{

  animate_item_x	= item_x;
  animate_item_y	= item_y;
  animate_item_size	= MAX_DART_SIZE;
  animate_item_distance	= targetDefinition[gcomprisBoard->level-1].target_distance/10;

  animate_item = gnome_canvas_item_new (boardRootItem,
					gnome_canvas_ellipse_get_type(),
					"x1", (double)item_x-MAX_DART_SIZE,
					"y1", (double)item_y-MAX_DART_SIZE,
					"x2", (double)item_x+MAX_DART_SIZE,
					"y2", (double)item_y+MAX_DART_SIZE,
					"fill_color_rgba", 0xFF80FFFF,
					"outline_color", "white",
					"width_units", (double)1,
					NULL);

  animate_id = gtk_timeout_add (200, (GtkFunction) animate_items, NULL);

  if(--number_of_arrow == 0)
    {
      request_score();
    }

}

/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  double item_x, item_y;

  if(board_paused)
    return FALSE;

  /* Is there already a dart on air */
  if(number_of_arrow == 0 || animate_item)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      switch(event->button.button)
	{
	case 1:
	case 2:
	case 3:
	  item_x = event->button.x;
	  item_y = event->button.y;
	  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

	  launch_dart(item_x, item_y);

	  break;
	default:
	  break;
	}
    default:
      break;
    }
  return FALSE;
}
