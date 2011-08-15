/* gcompris - target.c
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

static GooCanvasItem *boardRootItem = NULL;
static GooCanvasItem *valueRootItem = NULL;
static GooCanvasItem *speedRootItem = NULL;
static GooCanvasItem *tooltipItem = NULL;

static double wind_speed;
static double ang;

static GooCanvasItem  *answer_item = NULL;
static gchar		 answer_string[10];
static guint		 answer_string_index = 0;

static GooCanvasItem *animate_item = NULL;
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
static GooCanvasItem *target_create_item(GooCanvasItem *parent);
static void		target_destroy_all_items(void);
static void		target_next_level(void);
static gboolean		item_event (GooCanvasItem  *item,
				    GooCanvasItem  *target,
				    GdkEventButton *event,
				    gpointer data);
static gboolean		tooltip_event (GooCanvasItem  *item,
				       GooCanvasItem  *target,
				       GdkEventButton *event,
				       gpointer gpoint);
static gboolean		animate_items(gpointer data);
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

#define TARGET_CENTER_X	235.0
#define TARGET_CENTER_Y	260.0

#define SPEED_CENTER_X	660.0
#define SPEED_CENTER_Y	125.0

static guint target_colors[] = {
  0xAA0000FF, 0x00AA00FF, 0x0000AAFF,
  0xAAAA00FF, 0x00AAAAFF, 0xAA00AAFF,
  0xAA0000FF, 0x00AA00FF, 0x0000AAFF,
  0xAA0000AF
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
	g_source_remove (animate_id);
	animate_id = 0;
      }
    }
  else
    {
      if(animate_item) {
	animate_id = g_timeout_add (200, animate_items, NULL);
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

      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			"target/target_background.svgz");

      gc_bar_set(GC_BAR_LEVEL);
      gc_bar_location(BOARDWIDTH-200, -1, 0.8);

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

  if(!gcomprisBoard || gamewon == TRUE)
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
      g_object_set(answer_item,
		   "text", tmpstr,
		   NULL);
      g_free(tmpstr);
      process_ok();
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

  gc_bar_set_level(gcomprisBoard);

  target_destroy_all_items();
  gamewon = FALSE;

  /* Try the next level */
  target_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void target_destroy_all_items()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;

  if(speedRootItem!=NULL)
    goo_canvas_item_remove(speedRootItem);

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
  GooCanvasPoints *canvasPoints;
  canvasPoints = goo_canvas_points_new (2);

  if(speedRootItem!=NULL)
    goo_canvas_item_remove(speedRootItem);

  speedRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


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
  double w = 4.0;
  goo_canvas_polyline_new (speedRootItem, FALSE, 0,
			   "points", canvasPoints,
			   "stroke-color-rgba", 0Xa05a2cffL,
			   "fill-color-rgba", 0Xa05a2cffL,
			   "line-width", w,
			   "end-arrow", TRUE,
			   "arrow-tip-length", 7.0,
			   "arrow-length", 5.0,
			   "arrow-width", 4.0,
			   NULL);

  goo_canvas_points_unref(canvasPoints);

  /* Draw the center of the speedometer */
  goo_canvas_ellipse_new (speedRootItem,
			 SPEED_CENTER_X,
			 SPEED_CENTER_Y,
			 10.0,
			 10.0,
			 "fill_color_rgba", 0Xa05a2cffL,
			 "stroke-color", "black",
			 "line-width", (double)2,
			 NULL);

  tmpstr = g_strdup_printf(_("Wind speed = %d\nkilometers/hour"), (guint)wind_speed);
  goo_canvas_text_new (speedRootItem,
		       tmpstr,
		       (double) SPEED_CENTER_X,
		       (double) SPEED_CENTER_Y + 110,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_board_medium,
		       "fill-color", "white",
		       NULL);
  g_free(tmpstr);

}



/* ==================================== */
static GooCanvasItem *target_create_item(GooCanvasItem *parent)
{
  int i;
  gchar *tmpstr;
  GooCanvasItem *item = NULL;

  boardRootItem = goo_canvas_group_new (parent, NULL);
  valueRootItem = goo_canvas_group_new (boardRootItem, NULL);
  goo_canvas_item_translate(boardRootItem,
			    TARGET_CENTER_X,
			    TARGET_CENTER_Y);

  for(i=0; i<MAX_NUMBER_OF_TARGET; i++)
    {
      if(targetDefinition[gcomprisBoard->level-1].target_width_value[i*2]>0)
	{
	  item = \
	    goo_canvas_ellipse_new (boardRootItem,
				    0.0,
				    0.0,
				    (double)targetDefinition[gcomprisBoard->level-1].target_width_value[i*2],
				    (double)targetDefinition[gcomprisBoard->level-1].target_width_value[i*2],
				    "fill_color_rgba", target_colors[i],
				    "stroke-color", "black",
				    "line-width", 1.0,
				    NULL);

	  goo_canvas_item_lower(item, NULL);
	  g_signal_connect(item, "button-press-event", (GCallback) item_event, NULL);

	  /* Display the value for this target */
	  tmpstr = g_strdup_printf("%d",
				   targetDefinition[gcomprisBoard->level-1].target_width_value[i*2+1]);
	  item = goo_canvas_text_new (valueRootItem,
				      tmpstr,
				      (double) 0,
				      (double) targetDefinition[gcomprisBoard->level-1].target_width_value[i*2] - 10,
				      -1,
				      GTK_ANCHOR_CENTER,
				      "font", gc_skin_font_board_medium,
				      "fill-color", "white",
				      NULL);
	  g_free(tmpstr);

	  g_signal_connect(item, "button-press-event", (GCallback) item_event, NULL);
	}
    }

  number_of_arrow = targetDefinition[gcomprisBoard->level-1].number_of_arrow;

  tmpstr = g_strdup_printf(_("Distance to target = %d meters"),
			   targetDefinition[gcomprisBoard->level-1].target_distance);
  goo_canvas_text_new (boardRootItem,
		       tmpstr,
		       (double) 0,
		       (double) BOARDHEIGHT-TARGET_CENTER_Y -45,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_board_medium,
		       "fill-color", "black",
		       NULL);
  g_free(tmpstr);

  display_windspeed();

  /* Tooltip */
  tooltipItem =
    goo_canvas_text_new (boardRootItem,
			 "",
			 150,
			 -140,
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_board_title_bold,
			 "fill-color", "white",
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
    if(gcomprisBoard->level>gcomprisBoard->maxlevel)
	gcomprisBoard->level = gcomprisBoard->maxlevel;

    gc_sound_play_ogg ("sounds/bonus.wav", NULL);
  }
  target_next_level();
}

static gboolean bonus(gpointer data)
{
  gc_bonus_display(gamewon, GC_BONUS_SMILEY);
  animate_id = 0;
  return(FALSE);
}

static void process_ok()
{
  guint answer_points = atoi(answer_string);

  if(answer_item) {

    if(answer_points == user_points)
      {
	gamewon = TRUE;
	animate_id = g_timeout_add (200, bonus, NULL);
      }
  }

}

/*
 * Request score
 *
 */
static void request_score()
{
  double x_offset = 390;
  double y_offset = 150;
  gchar *tmpstr;

  /* Set the maximum text to calc the background */
  tmpstr =  g_strdup_printf(_("Points = %s"), "00000");
  answer_item = goo_canvas_text_new (boardRootItem,
				     tmpstr,
				     (double) x_offset,
				     (double) y_offset,
				     -1,
				     GTK_ANCHOR_CENTER,
				     "font", gc_skin_font_board_title_bold,
				     "fill-color", "white",
				     NULL);
  g_free(tmpstr);

  GooCanvasBounds bounds;
  goo_canvas_item_get_bounds (answer_item, &bounds);
  goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(answer_item),
				   answer_item,
				   &bounds.x1, &bounds.y1);
  goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(answer_item),
				   answer_item,
				   &bounds.x2, &bounds.y2);

  int gap = 15;
  GooCanvasItem *item =				\
    goo_canvas_rect_new (boardRootItem,
			 bounds.x1 - gap,
			 bounds.y1 - gap,
			 (bounds.x2 - bounds.x1) + gap*2,
			 (bounds.y2 - bounds.y1) + gap*2,
			 "stroke_color_rgba", 0xFFFFFFFFL,
			 "fill_color_rgba", 0X5599FFCCL,
			 "line-width", (double) 2,
			 "radius-x", (double) 10,
			 "radius-y", (double) 10,
			 NULL);
  goo_canvas_item_raise(answer_item, item);

  /* Set the correct initial text */
  tmpstr = g_strdup_printf(_("Points = %s"), answer_string);
  g_object_set(answer_item,
	       "text", tmpstr,
	       NULL);
  g_free(tmpstr);
}

static guint add_points(double x, double y)
{
  guint i;
  double diametre;
  guint points = 0;

  // Calculate the distance
  diametre = sqrt(x*x+y*y);

  for(i=0; i<MAX_NUMBER_OF_TARGET; i++)
    {
      if(diametre < targetDefinition[gcomprisBoard->level-1].target_width_value[i*2])
	{
	  points = targetDefinition[gcomprisBoard->level-1].target_width_value[i*2+1];
	  user_points += points;
	  break;
	}
    }

  return points;
}


/*
 * Dart animation
 *
 */
static gboolean animate_items(gpointer data)
{
  if(board_paused)
    return(FALSE);

  if(!animate_item)
    return(FALSE);

  // Apply the wind move
  animate_item_x = animate_item_x + wind_speed * sin(ang);
  animate_item_y = animate_item_y - wind_speed * cos(ang);

  g_object_set (animate_item,
		"center-x", (double)animate_item_x,
		"center-y", (double)animate_item_y,
		"radius-x", (double)animate_item_size,
		"radius-y", (double)animate_item_size,
		NULL);

  if(animate_item_size>MIN_DART_SIZE)
    animate_item_size--;

  if(--animate_item_distance == 0)
    {
      gc_sound_play_ogg ("sounds/brick.wav", NULL);

      // Calc the point for this dart
      guint points = add_points(animate_item_x, animate_item_y);

      // Add a tooltip on this dart to let the children
      // see how we count it
      g_signal_connect(animate_item,
		       "enter_notify_event", (GCallback) tooltip_event,
		       GINT_TO_POINTER(points));
      g_signal_connect(animate_item,
		       "leave_notify_event", (GCallback) tooltip_event,
		       GINT_TO_POINTER(-1));

      g_source_remove (animate_id);
      animate_id = 0;
      animate_item = NULL;

      // Change the wind for the next target
      display_windspeed();
    }
  return(TRUE);
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

  gc_sound_play_ogg ("sounds/line_end.wav", NULL);

  animate_item = goo_canvas_ellipse_new (boardRootItem,
					(double)item_x,
					(double)item_y,
					(double)MAX_DART_SIZE,
					(double)MAX_DART_SIZE,
					"fill_color_rgba", 0xFF80FFFF,
					"stroke-color", "white",
					"line-width", (double)1,
					NULL);
  /* Make sure the target values stay on top */
  goo_canvas_item_lower(animate_item, valueRootItem);
  animate_id = g_timeout_add (200, animate_items, NULL);

  if(--number_of_arrow == 0)
    {
      request_score();
    }

}

/* ==================================== */
static gboolean
item_event (GooCanvasItem  *item,
	    GooCanvasItem  *target,
	    GdkEventButton *event,
	    gpointer data)
{
  double item_x, item_y;

  if(board_paused)
    return FALSE;

  /* Is there already a dart on air */
  if(number_of_arrow == 0 || animate_item)
    return FALSE;

  switch(event->button)
    {
    case 1:
    case 2:
    case 3:
      item_x = event->x;
      item_y = event->y;
      //goo_canvas_convert_to_item_space(item->parent, &item_x, &item_y);

      launch_dart(item_x, item_y);

      break;
    default:
      break;
    }

  return FALSE;
}

static gboolean
tooltip_event (GooCanvasItem  *item,
	       GooCanvasItem  *target,
	       GdkEventButton *event,
	       gpointer gpoint)
{
  gint point = GPOINTER_TO_INT(gpoint);

  if (point >= 0) {
    gchar *tmpstr = g_strdup_printf("%d", point);
    g_object_set(tooltipItem,
		 "text", tmpstr,
		 NULL);
    g_free(tmpstr);
  } else {
    g_object_set(tooltipItem,
		 "text", "",
		 NULL);
  }

  return TRUE;
}
