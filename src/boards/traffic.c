/* gcompris - traffic.c
 *
 * Copyright (C) 2002 Bruno Coudoin
 *
 * Based on the original code from Geoff Reedy <vader21@imsa.edu>
 * Copyright (C) 2000 Geoff Reedy 
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

#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

#include "traffic.h"

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

#define DATAFILE "TrafficData"


static GcomprisBoard  *gcomprisBoard = NULL;
static gboolean	board_paused  = TRUE;

static GnomeCanvasGroup *allcars         = NULL;

#define OFSET_X 250
#define OFSET_Y 128

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);
static void	 repeat(void);

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem	*traffic_create_item(GnomeCanvasGroup *parent);
static void		 traffic_destroy_all_items(void);
static void		 traffic_next_level(void);

typedef struct _car car;
typedef struct _jam jam;

struct _car {
  guint x : 3;
  guint y : 3;
#define CAR_ORIENT_NS	0
#define CAR_ORIENT_EW	1
  guint orient : 1;
  guint goal : 1;
  guint size;
  guint color;
  gchar color_string[50];
  GnomeCanvasGroup *canvasgroup;
};

struct _jam {
  guint num_cars;
  guint card;
  guint level;
#define MAX_NUMBER_OF_CARS 20
  car *cars[MAX_NUMBER_OF_CARS];
};

static int	 car_cb(GnomeCanvasItem *item, GdkEvent *event, car *thiscar);
gboolean	 load_level(guint level, guint card);

static jam	 current_card  ={0,0,0,{NULL}};

static void	 draw_grid  (GnomeCanvasGroup *rootBorder);
static gint	 cars_from_strv(char *strv);

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "A sliding block puzzle game",
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
    repeat,
    NULL,
    NULL
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(traffic)

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
  GdkPixbuf *pixmap = NULL;
  char *str;

  if(agcomprisBoard!=NULL)
    {
      gchar *img;

      gcomprisBoard=agcomprisBoard;
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=8;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=5; /* Go to next level after this number of 'play' */
      gcompris_score_start(SCORESTYLE_NOTE, 
			   gcomprisBoard->width - 220, 
			   gcomprisBoard->height - 50, 
			   gcomprisBoard->number_of_sublevel);

      str = gcompris_image_to_skin("button_reload.png");
      pixmap = gcompris_load_pixmap(str);
      g_free(str);
      if(pixmap) {
	gcompris_bar_set_repeat_icon(pixmap);
	gdk_pixbuf_unref(pixmap);
	gcompris_bar_set(GCOMPRIS_BAR_LEVEL|GCOMPRIS_BAR_REPEAT_ICON);
      } else {
	gcompris_bar_set(GCOMPRIS_BAR_LEVEL|GCOMPRIS_BAR_REPEAT);
      }

      img = gcompris_image_to_skin("traffic-bg.jpg");
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      img);
      g_free(img);

      traffic_next_level();

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
      gcompris_score_end();
      traffic_destroy_all_items();
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
      traffic_next_level();
    }
}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "traffic")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

/*
 * Repeat let the user restart the current level
 *
 */
static void repeat (){

  traffic_destroy_all_items();

  /* Try the next level */
  traffic_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* set initial values for the next level */
static void traffic_next_level()
{

  gcompris_bar_set_level(gcomprisBoard);

  traffic_destroy_all_items();
  gamewon = FALSE;

  gcompris_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  traffic_create_item(gnome_canvas_root(gcomprisBoard->canvas));

}
/* ==================================== */
/* Destroy all the items */
static void traffic_destroy_all_items()
{
  guint i;

  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));
  boardRootItem = NULL;

  for (i=0; i<current_card.num_cars; i++)
    {
      g_free(current_card.cars[i]);
    }
  current_card.num_cars = 0;
}

/* ==================================== */
static GnomeCanvasItem *traffic_create_item(GnomeCanvasGroup *parent)
{
  GnomeCanvasGroup *borderItem = NULL;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));
  borderItem = GNOME_CANVAS_GROUP(
				  gnome_canvas_item_new (boardRootItem,
							 gnome_canvas_group_get_type (),
							 "x", (double) OFSET_X,
							 "y", (double) OFSET_Y,
							 NULL));
  draw_grid(borderItem);

  allcars= GNOME_CANVAS_GROUP(
			      gnome_canvas_item_new(GNOME_CANVAS_GROUP(borderItem),
						    gnome_canvas_group_get_type(),
						    "x", 11.0,
						    "y", 11.0,
						    NULL));
  gtk_object_set_data(GTK_OBJECT(allcars),"whatami",(gpointer)"allcars");

  /* Ready now, let's go */
  load_level(gcomprisBoard->level, gcomprisBoard->sublevel);

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
    gcompris_play_ogg ("bonus", NULL);
  }
  traffic_next_level();
}

/* from canvas.c */

static void draw_grid(GnomeCanvasGroup *rootBorder)
{
  GnomeCanvasGroup *grid_group;
  int xlooper, ylooper;

  grid_group = GNOME_CANVAS_GROUP(
				  gnome_canvas_item_new(rootBorder,
							gnome_canvas_group_get_type(),
							"x", 10.0,
							"y", 10.0,
							NULL));

  gtk_object_set_data(GTK_OBJECT(grid_group),"whatami",(gpointer)"grid_group");

  gnome_canvas_item_lower_to_bottom(GNOME_CANVAS_ITEM(grid_group));

  for (xlooper=0;xlooper<=6;xlooper++)
    for (ylooper=0;ylooper<=6;ylooper++)
      gtk_object_set_data(GTK_OBJECT(
				     gnome_canvas_item_new(grid_group,
							   gnome_canvas_rect_get_type(),
							   "x1", 0.0*xlooper,
							   "x2", 40.0*xlooper,
							   "y1", 0.0*ylooper,
							   "y2", 40.0*ylooper,
							   "fill_color", NULL,
							   "outline_color", "white",
							   "width_units", 2.0,
							   NULL)),
			  "whatami",(gpointer)"grid square");;
}


void draw_car(car *thiscar)
{
  GnomeCanvasItem *car_group;
  GnomeCanvasItem *car_rect;

  gtk_object_set_data(GTK_OBJECT(allcars),"whatami",(gpointer)"allcars");

  car_group=gnome_canvas_item_new(GNOME_CANVAS_GROUP(allcars),
				  gnome_canvas_group_get_type(),
				  "x",40.0*thiscar->x,
				  "y",40.0*thiscar->y,
				  NULL);

  car_rect=gnome_canvas_item_new(GNOME_CANVAS_GROUP(car_group),
				 gnome_canvas_rect_get_type(),
				 "x1",0.0,
				 "y1",0.0,
				 "x2",(thiscar->orient?40.0*thiscar->size:40.0)-2.25,
				 "y2",(thiscar->orient?40.0:40.0*thiscar->size)-2.25,
				 "fill_color_rgba", thiscar->color,
				 "outline_color", NULL,
				 NULL);

  gtk_signal_connect(GTK_OBJECT(car_group),"event",
		     GTK_SIGNAL_FUNC(car_cb),(gpointer)thiscar);
  gtk_object_set_data(GTK_OBJECT(car_group),"car",(gpointer)thiscar);
  gtk_object_set_data(GTK_OBJECT(car_group),"whatami",(gpointer)"car_group");
  gtk_object_set_data(GTK_OBJECT(car_rect),"whatami",(gpointer)"car_rect");
}

void draw_jam(jam *myjam)
{
  int whichcar;
  for (whichcar=0;whichcar<myjam->num_cars;whichcar++)
    draw_car(myjam->cars[whichcar]);
}

static int car_cb(GnomeCanvasItem *item, GdkEvent *event, car *thiscar)
{
  static gboolean moving;
  static int button;
  static double start_x,start_y;
  static double hit=0;
  double item_x, item_y;
  double world_x, world_y;
  double dx,dy;
  double small_x, big_x, small_y, big_y;
  GdkCursor *cursor;
  GnomeCanvasItem *atdest=NULL;
  car *othercar=NULL;

  item_x=world_x=event->button.x;
  item_y=world_y=event->button.y;
  gnome_canvas_item_w2i(item->parent,&item_x,&item_y);

  switch (event->type) {
  case GDK_BUTTON_PRESS:
    start_x=item_x;
    start_y=item_y;
    button=event->button.button;

    if (thiscar->orient==CAR_ORIENT_NS)
      cursor=gdk_cursor_new(GDK_SB_V_DOUBLE_ARROW);
    else 
      cursor=gdk_cursor_new(GDK_SB_H_DOUBLE_ARROW);
    gcompris_canvas_item_grab(item,
			   GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
			   cursor,
			   event->button.time);
    gdk_cursor_destroy(cursor);

    moving=TRUE;
    break;
  case GDK_MOTION_NOTIFY:
    if (moving)
      switch (thiscar->orient) {
      case CAR_ORIENT_NS:
	small_x=0;
	big_x=40-1;
	small_y=0;
	big_y=40*thiscar->size-1;

	gnome_canvas_item_i2w(item,&small_x,&small_y);
	gnome_canvas_item_i2w(item,&big_x,&big_y);

	dy=CLAMP(item_y-start_y,-39,39);
	dx=CLAMP(item_x-start_x,-39,39);

	if (small_y+dy<11+OFSET_Y) {
	  dy=11-small_y+OFSET_Y;
	} else if (big_y+dy>250+OFSET_Y) {
	  dy=250-big_y+OFSET_Y;
	}

	if ((hit<0)!=(dy<0)) { hit=0; }

	if (hit==0) {
	  if (dy>0) {
	    do {
	      atdest=gnome_canvas_get_item_at(gcomprisBoard->canvas,
					      small_x+20,
					      big_y+dy);
	      if (atdest)
		othercar=(car*)gtk_object_get_data(GTK_OBJECT(atdest->parent),"car");
	      if (othercar) {
		hit=1;
		dy--;
	      }
	    } while (othercar);
	  } else if (dy<0) {
	    do {
	      atdest=gnome_canvas_get_item_at(gcomprisBoard->canvas,
					      small_x+20,small_y+dy-1);
	      if (atdest)
		othercar=(car*)gtk_object_get_data(GTK_OBJECT(atdest->parent),"car");
	      if (othercar) {
		hit=-1;
		dy++;
	      }
	    } while (othercar);
	  }
	} else { dy=0; }

	start_x+=dx;
	start_y+=dy;

	gnome_canvas_item_move(item,0,dy);
	gnome_canvas_item_request_update(item);

	break;
      case CAR_ORIENT_EW:
	small_x=0;
	big_x=40*thiscar->size-1;
	small_y=0;
	big_y=40-1;

	gnome_canvas_item_i2w(item,&small_x,&small_y);
	gnome_canvas_item_i2w(item,&big_x,&big_y);

	dy=CLAMP(item_y-start_y,-39,39);
	dx=CLAMP(item_x-start_x,-39,39);

	if (thiscar->goal && big_x==250+OFSET_X) { 
	  gcompris_canvas_item_ungrab(item,event->button.time);
	  gnome_canvas_item_hide(item);
	  moving=FALSE;

	  gamewon = TRUE;
	  gcompris_display_bonus(gamewon, BONUS_SMILEY);

	}

	if (small_x+dx<11+OFSET_X) {
	  dx=11-small_x+OFSET_X;
	} else if (big_x+dx>250+OFSET_X) {
	  dx=250-big_x+OFSET_X;
	}

	if ((hit<0)!=(dx<0)) { hit=0;}

	if (hit==0) {
	  if (dx>0) {
	    do {
	      atdest=gnome_canvas_get_item_at(gcomprisBoard->canvas,
					      big_x+dx,small_y+20);
	      if (atdest)
		othercar=(car*)gtk_object_get_data(GTK_OBJECT(atdest->parent),"car");
	      if (othercar) {
		hit=1;
		dx--;
	      }
	    } while (othercar);
	  } else if (dx<0) {
	    do {
	      atdest=gnome_canvas_get_item_at(gcomprisBoard->canvas,
					      small_x+dx-1,small_y+20);
	      if (atdest)
		othercar=(car*)gtk_object_get_data(GTK_OBJECT(atdest->parent),"car");
	      if (othercar) {
		hit=-1;
		dx++;
	      }
	    } while (othercar);
	  }
	} else { dx=0; }

	start_x+=dx;
	start_y+=dy;

	gnome_canvas_item_move(item,dx,0);
	gnome_canvas_item_request_update(item);

	break;
      default:
	break;
      }
  case GDK_BUTTON_RELEASE:
    if (moving && (event->button.button==button))
      {
	double even_vals_x[]={11+OFSET_X,51+OFSET_X,91+OFSET_X,131+
			      OFSET_X,171+OFSET_X,211+OFSET_X,HUGE_VAL};
	double even_vals_y[]={11+OFSET_Y,51+OFSET_Y,91+OFSET_Y,131+
			      OFSET_Y,171+OFSET_Y,211+OFSET_Y,HUGE_VAL};
	double *ptr;
	double x=0,y=0;

	gnome_canvas_item_i2w(item,&x,&y);

	for (ptr=even_vals_x;*ptr<x;ptr++);
	if (*ptr-x>20)
	  dx=*(ptr-1)-x;
	else
	  dx=*ptr-x;

	for (ptr=even_vals_y;*ptr<y;ptr++);
	if (*ptr-y>20)
	  dy=*(ptr-1)-y;
	else
	  dy=*ptr-y;

	gnome_canvas_item_move(item,dx,dy);
	gcompris_canvas_item_ungrab(item,event->button.time);
	hit=0;
	moving=FALSE;
      }
    break;
  default:
    break;
  }
  return TRUE;
}

/* From jam.c */

gboolean load_level(guint level, guint sublevel)
{
  char *car_strv=NULL;

  current_card.level = level;
  current_card.card  = sublevel;

  car_strv = DataList[(level-1) * gcomprisBoard->number_of_sublevel + (sublevel-1)];

  current_card.num_cars = cars_from_strv(car_strv);

  if(current_card.num_cars == -1)
    g_error("In loading dataset for traffic activity");

  draw_jam(&current_card);

  return TRUE;
}

/* Returns the number of cars
 * I took the formatting from
 *  http://www.javascript-games.org/puzzle/rushhour/
 */
gint cars_from_strv(char *strv)
{
  car *ccar;
  char x,y,id;
  int number_of_cars = 0;
  gboolean more_car = TRUE;

  while (more_car) {

    current_card.cars[number_of_cars] = (car *)g_new(car, 1);
    ccar = current_card.cars[number_of_cars];

    /* By default, not a goal car */
    ccar->goal   = 0;

    number_of_cars++;

    if (sscanf(strv,"%c%c%c",
	       &id,&x,&y)!=3) {
      return -1;
    }

    /* Point to the next car */
    strv += 3;

    if(strv[0] != ',')
      more_car = FALSE;

    strv += 1;

    if (id == 'O' || id == 'P' || id == 'Q' || id == 'R') ccar->size = 3;
    else ccar->size = 2;

    ccar->orient = 1;
    ccar->x = 0;
    ccar->y = y-'1';

    if (x == 'A') ccar->x = 0;
    else if (x == 'B') ccar->x = 1;
    else if (x == 'C') ccar->x = 2;
    else if (x == 'D') ccar->x = 3;
    else if (x == 'E') ccar->x = 4;
    else if (x == 'F') ccar->x = 5;
    else {
      ccar->y = x-'1';
      ccar->orient = 0;

      if (y == 'A') ccar->x = 0;
      else if (y == 'B') ccar->x = 1;
      else if (y == 'C') ccar->x = 2;
      else if (y == 'D') ccar->x = 3;
      else if (y == 'E') ccar->x = 4;
      else if (y == 'F') ccar->x = 5;
    }

    if (id == 'X') 
      {
	ccar->color  = 0xFF0000FF;
	ccar->goal   = 1;
      }
    else if (id == 'A') ccar->color = 0x80FF80FF;
    else if (id == 'B') ccar->color = 0xC0C000FF;
    else if (id == 'C') ccar->color = 0x8080FFFF;
    else if (id == 'D') ccar->color = 0xFF80FFFF;
    else if (id == 'E') ccar->color = 0xC00000FF;
    else if (id == 'F') ccar->color = 0x008000FF;
    else if (id == 'G') ccar->color = 0xC0C0C0FF;
    else if (id == 'H') ccar->color = 0x6000efFF;
    else if (id == 'I') ccar->color = 0xFFFF00FF;
    else if (id == 'J') ccar->color = 0xFFA801FF;
    else if (id == 'K') ccar->color = 0x00FF00FF;
    else if (id == 'O') ccar->color = 0xFFFF00FF;
    else if (id == 'P') ccar->color = 0xFF80FFFF;
    else if (id == 'Q') ccar->color = 0x0000FFFF;
    else if (id == 'R') ccar->color = 0x00FFFFFF;
    
  }
  return number_of_cars;
}

