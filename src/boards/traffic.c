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

#include <ctype.h>
#include <math.h>
#include <assert.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

#define DATAFILE "TrafficData"


GcomprisBoard  *gcomprisBoard = NULL;
gboolean	board_paused  = TRUE;

static GnomeCanvasGroup *allcars         = NULL;

static GnomeCanvasItem  *removedCarsItem = NULL;
static GnomeCanvasItem  *removeLeftItem  = NULL;
static GnomeCanvasItem  *moveTakenItem   = NULL;
static GnomeCanvasItem  *bestMoveItem    = NULL;

#define MESSAGE_CENTER_X	575
#define MESSAGE_CENTER_Y	190
#define MESSAGE_COLOR		"blue"

#define OFSET_X 50
#define OFSET_Y 130

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

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
  gchar color_string[50];
  GnomeCanvasGroup *canvasgroup;
};

struct _jam {
  car *cars;
  guint num_cars;
  guint cars_for_goal;
  guint minmoves;
  guint card;
  guint level;
};

static int	 car_cb(GnomeCanvasItem *item, GdkEvent *event, car *thiscar);
gboolean	 load_level(guint level, guint card);

static jam	 current_card  ={NULL,0,0,0,0,0};
static jam	 old_jam       ={NULL,0,0,0,0,0}; /* for disaster recovery */

guint		 cars_for_goal;
guint		 cars_removed;
gboolean	 must_free;
guint		 moves_taken;

static void	 draw_border(GnomeCanvasGroup *rootBorder);
static void	 draw_grid  (GnomeCanvasGroup *rootBorder);
static gboolean	 cars_from_strv(char **strv);
static void	 load_error(void);
static void	 load_not_found(void);

static void	 label_update_moves(void);
static void	 label_update_goal(void);

/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("A sliding block puzzle game"),
    N_(""),
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
    NULL
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

BoardPlugin
*get_bplugin_info(void)
{
  return &menu_bp;
}

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
      gcomprisBoard->maxlevel=8;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      "traffic/traffic-bg.jpg");

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
gboolean is_our_board (GcomprisBoard *gcomprisBoard)
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

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* set initial values for the next level */
static void traffic_next_level()
{

  gcompris_bar_set_level(gcomprisBoard);

  traffic_destroy_all_items();
  gamewon = FALSE;

  /* Select level difficulty */
  switch(gcomprisBoard->level)
    {
    case 1:
      break;
    case 2:
      break;
    default:
      break;
    }
      
  /* Try the next level */
  traffic_create_item(gnome_canvas_root(gcomprisBoard->canvas));

}
/* ==================================== */
/* Destroy all the items */
static void traffic_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));
  boardRootItem = NULL;
}

/* ==================================== */
static GnomeCanvasItem *traffic_create_item(GnomeCanvasGroup *parent)
{
  int i,j;
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
  //  draw_border(borderItem);
  draw_grid(borderItem);

  allcars= GNOME_CANVAS_GROUP(
			      gnome_canvas_item_new(GNOME_CANVAS_GROUP(borderItem),
						    gnome_canvas_group_get_type(),
						    "x", 11.0,
						    "y", 11.0,
						    NULL));
  gtk_object_set_data(GTK_OBJECT(allcars),"whatami",(gpointer)"allcars");

  /* ------ */
  removedCarsItem = gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_text_get_type (),
			 "text", "",
			 "font", FONT_BOARD_MEDIUM,
			 "x", (double) MESSAGE_CENTER_X,
			 "y", (double) MESSAGE_CENTER_Y,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color", MESSAGE_COLOR,
			 NULL);

  removeLeftItem = gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_text_get_type (),
			 "text", "",
			 "font", FONT_BOARD_MEDIUM,
			 "x", (double) MESSAGE_CENTER_X,
			 "y", (double) MESSAGE_CENTER_Y+20,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color", MESSAGE_COLOR,
			 NULL);

  moveTakenItem = gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_text_get_type (),
			 "text", "TOTO",
			 "font", FONT_BOARD_MEDIUM,
			 "x", (double) MESSAGE_CENTER_X,
			 "y", (double) MESSAGE_CENTER_Y+120,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color", MESSAGE_COLOR,
			 NULL);

  bestMoveItem = gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_text_get_type (),
			 "text", "",
			 "font", FONT_BOARD_MEDIUM,
			 "x", (double) MESSAGE_CENTER_X,
			 "y", (double) MESSAGE_CENTER_Y+140,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color", MESSAGE_COLOR,
			 NULL);

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
    gcompris_play_sound (SOUNDLISTFILE, "bonus");
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
				 "fill_color", g_strdup(thiscar->color_string),
				 "outline_color", NULL,
				 NULL);

  gtk_signal_connect(GTK_OBJECT(car_group),"event",
		     GTK_SIGNAL_FUNC(car_cb),(gpointer)thiscar);
  gtk_object_set_data(GTK_OBJECT(car_group),"car",(gpointer)thiscar);
  gtk_object_set_data(GTK_OBJECT(car_group),"whatami",(gpointer)"car_group");
  gtk_object_set_data(GTK_OBJECT(car_rect),"whatami",(gpointer)"car_rect");
}

static void draw_border(GnomeCanvasGroup *rootBorder)
{
  GnomeCanvasItem *border;
  GnomeCanvasPoints *points;
  double mypoints[]={
    0,0,
    261,0,
    261,90,
    251,90,
    251,9,
    9,9,
    9,251,
    251,251,
    251,130,
    261,130,
    261,261,
    0,261,
    0,0
  };

  points=gnome_canvas_points_new(13);
  memcpy(points->coords,mypoints,sizeof(mypoints));

  border=gnome_canvas_item_new(rootBorder,
			       gnome_canvas_polygon_get_type(),
			       "points", points,
			       "fill_color", "black",
			       "outline_color", NULL,
			       "width_units", 0.0,
			       NULL);
}

void draw_jam(jam *myjam)
{
  int whichcar;
  for (whichcar=0;whichcar<myjam->num_cars;whichcar++)
    draw_car(&myjam->cars[whichcar]);
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
    gnome_canvas_item_grab(item,
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
	  cars_removed++;
	  gnome_canvas_item_ungrab(item,event->button.time);
	  gnome_canvas_item_hide(item);
	  moves_taken++;
	  moving=FALSE;
	  label_update_goal();
	  label_update_moves();

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
	double even_vals_x[]={11+OFSET_X,51+OFSET_X,91+OFSET_X,131+OFSET_X,171+OFSET_X,211+OFSET_X,HUGE_VAL};
	double even_vals_y[]={11+OFSET_Y,51+OFSET_Y,91+OFSET_Y,131+OFSET_Y,171+OFSET_Y,211+OFSET_Y,HUGE_VAL};
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
	gnome_canvas_item_ungrab(item,event->button.time);
	hit=0;
	moves_taken++;
	label_update_moves();
	moving=FALSE;
      }
    break;
  default:
    break;
  }
  return TRUE;
}

/* From jam.c */

#define strip_spaces(string) g_strjoinv("",g_strsplit((string)," ",0))

gboolean load_level(guint level, guint card)
{
  char **car_strv=NULL;
  char *data_file;
  int num_cars=0;

  current_card.level = level;
  current_card.card  = card;

  cars_removed=0;

  data_file = g_strdup_printf("%s/traffic/%s",PACKAGE_DATA_DIR, DATAFILE);

  if (!g_file_test ((data_file), G_FILE_TEST_EXISTS)) {
    g_error (_("Couldn't find file %s !"), data_file);
  }

  gnome_config_push_prefix(g_strconcat("=", data_file, "=",
				       g_strdup_printf("/Level%d",level), "/",
				       NULL));
  gnome_config_get_vector(g_strdup_printf("Card%d",card),
			  &num_cars, &car_strv);
  gnome_config_pop_prefix();
  if (!car_strv) { load_not_found(); return FALSE; }

  current_card.num_cars=num_cars-1;

  if (sscanf(car_strv[0],"%d,%d",
	     &current_card.cars_for_goal,
	     &current_card.minmoves)!=2) 
    { 
      load_error(); 
      return FALSE; 
    }


  cars_for_goal=current_card.cars_for_goal;


  g_free(current_card.cars);
  current_card.cars=g_new(car,num_cars);
  must_free=1;
  if (!cars_from_strv(&car_strv[1]))
    load_error();
  
  old_jam=current_card;

  moves_taken=0;

  label_update_moves();
  label_update_goal();
  draw_jam(&current_card);
  return TRUE;
}

gboolean cars_from_strv(char **strv)
{
  car *ccar;
  int counter=0;
  int x,y,orient,goal;
  while (*strv) {
    if (++counter>current_card.num_cars)  return FALSE;
    ccar=&current_card.cars[counter-1];
    if (sscanf(*strv,"%u,%u,%u,%u,%u,%49s",
	       &x,&y,
	       &orient,&goal,
	       &ccar->size,ccar->color_string)!=6) {
      return FALSE;
    }
    ccar->x=x; ccar->y=y;
    ccar->orient=orient;
    ccar->goal=goal;
    strv++;
  }
  return TRUE;
}

void load_error(void)
{
  board_finished(BOARD_FINISHED_RANDOM);
}

void load_not_found(void)
{
  board_finished(BOARD_FINISHED_RANDOM);
}

static void label_update_moves(void) 
{
  if(removedCarsItem)
    gnome_canvas_item_set(removedCarsItem,
			  "text", g_strdup_printf(_("You've taken %d moves."),
						  moves_taken),
			  NULL);

  if(removeLeftItem)
    gnome_canvas_item_set(removeLeftItem,
			  "text", g_strdup_printf(_("The best you can do is %d."),
						  current_card.minmoves),
			  NULL);

}

static void label_update_goal(void)
{
  if(moveTakenItem)
    gnome_canvas_item_set(moveTakenItem,
			  "text", g_strdup_printf(_("You've removed %d cars."),
						  cars_removed),
			  NULL);

  if(bestMoveItem)
    gnome_canvas_item_set(bestMoveItem,
			  "text", g_strdup_printf(_("You need to remove %d more."),
						  cars_for_goal-cars_removed),
			  NULL);
}
