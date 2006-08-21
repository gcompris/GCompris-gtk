/* gcompris - railroad.c
 *
 * Copyright (C) 2001 Pascal Georges
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

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);
static int gamewon;
static void		 process_ok(void);
static void		 game_won(void);
static void		 repeat(void);
static void		 animate_model(void);
static gboolean		 animate_step(void);

#define ENGINES 9
#define WAGONS 13
#define NUMBER_OF_SUBLEVELS 3
#define NUMBER_OF_LEVELS 3
#define MODEL_MAX_SIZE NUMBER_OF_LEVELS+1

static const int line[] = { 100,180,260,340, 420, 500};
static gboolean animation_pending;
static gint animation_count = 0;

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasGroup *allwagonsRootItem = NULL;
static GnomeCanvasGroup *modelRootItem = NULL;
static GnomeCanvasGroup *answerRootItem = NULL;

static GList * listPixmapEngines = NULL;
static GList * listPixmapWagons = NULL;

// ==========================================
// In all the lists below, 0 is the LEFTmost vehicle|
// ==========================================
// contains the list of vehicles to be found.
static GnomeCanvasItem *item_model[MODEL_MAX_SIZE];
// contains the list of vehicles proposed by child.
static GList *item_answer_list = NULL;
// contains the list of vehicles proposed by child.
static GList *int_answer_list = NULL;
// contains the list of vehicles to be found
static GList *int_model_list = NULL;

static int model_size = 0;
static gint timer_id;

static GnomeCanvasItem *railroad_create_item(GnomeCanvasGroup *parent);
static void railroad_destroy_all_items(void);
static void railroad_next_level(void);
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint answer_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

static void reposition_model(void);
static void reposition_answer(void);

// helper function because g_list_free does not actually reset a list
static GList * reset_list(GList * list);
static void reset_all_lists(void);

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Memory game"),
    N_("Build a train according to the model"),
    "Pascal Georges pascal.georges1@free.fr>",
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
    repeat,
    NULL,
    NULL
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(railroad)

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

/* ======================================= */
static void start_board (GcomprisBoard *agcomprisBoard)
{
  int i;
  char *str;
  GdkPixbuf *pixmap = NULL;

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), 
			      "railroad/railroad-bg.png");

      for (i=0; i<ENGINES; i++) {
      	str = g_strdup_printf("railroad/loco%d.png", i+1);
	pixmap = gc_pixmap_load(str);
	listPixmapEngines = g_list_append(listPixmapEngines, pixmap);
	g_free(str);
      }

      for (i=0; i<WAGONS; i++) {
      	str = g_strdup_printf("railroad/wagon%d.png", i+1);
	pixmap = gc_pixmap_load(str);
	listPixmapWagons = g_list_append(listPixmapWagons, pixmap);
	g_free(str);
      }

      animation_pending = FALSE;
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=NUMBER_OF_LEVELS;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=NUMBER_OF_SUBLEVELS; /* Go to next level after this number of 'play' */
      gcompris_score_start(SCORESTYLE_NOTE,
			   gcomprisBoard->width - 220,
			   gcomprisBoard->height - 50,
			   gcomprisBoard->number_of_sublevel);


      str = gcompris_image_to_skin("button_reload.png");
      pixmap = gc_pixmap_load(str);
      g_free(str);
      if(pixmap) {
	gc_bar_set_repeat_icon(pixmap);
	gdk_pixbuf_unref(pixmap);
	gc_bar_set(GC_BAR_LEVEL|GC_BAR_OK|GC_BAR_REPEAT_ICON);
      } else {
	gc_bar_set(GC_BAR_LEVEL|GC_BAR_OK|GC_BAR_REPEAT);
      }

      railroad_next_level();

      gamewon = FALSE;
      pause_board(FALSE);
    }
}
/* ======================================= */
static void end_board ()
{
  GdkPixbuf * pixmap = NULL;
  // If we don't end animation, there may be a segfault if leaving while the animation is pending
  if (timer_id) {
    gtk_timeout_remove (timer_id);
    timer_id = 0;
  }

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gcompris_score_end();
      railroad_destroy_all_items();

      while(g_list_length(listPixmapEngines)>0) {
	pixmap = g_list_nth_data(listPixmapEngines, 0);
	listPixmapEngines = g_list_remove (listPixmapEngines, pixmap);
	gdk_pixbuf_unref(pixmap);
      }

      while(g_list_length(listPixmapWagons)>0) {
	pixmap = g_list_nth_data(listPixmapWagons, 0);
	listPixmapWagons = g_list_remove (listPixmapWagons, pixmap);
	gdk_pixbuf_unref(pixmap);
      }

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
      railroad_next_level();
    }
}
/* ======================================= */
gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "railroad")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

/* ======================================= */
static void repeat ()
{
  if(gcomprisBoard!=NULL && !animation_pending) {
    gnome_canvas_item_hide(GNOME_CANVAS_ITEM(answerRootItem));
    gnome_canvas_item_show(GNOME_CANVAS_ITEM(modelRootItem));
    gnome_canvas_item_hide(GNOME_CANVAS_ITEM(allwagonsRootItem));
    reposition_model();
    animate_model();
  }
}

/* ==================================== */
/* set initial values for the next level */
static void railroad_next_level()
{
  gc_bar_set_level(gcomprisBoard);

  reset_all_lists();
  // I have big troubles with the GList API : the worst I have ever seen !
  assert(g_list_length(item_answer_list) == 0 && g_list_length(int_answer_list) == 0 && g_list_length(int_model_list) == 0);

  model_size = 0;

  railroad_destroy_all_items();
  gamewon = FALSE;
  gcompris_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  railroad_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void railroad_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
static GnomeCanvasItem *railroad_create_item(GnomeCanvasGroup *parent)
{
  int xOffset = 0, yOffset = 0;
  int i, r, l = 1;
  GdkPixbuf * pixmap = NULL;
  GnomeCanvasItem *item;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (parent,
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));
  /* Create a root group for the answer */
  answerRootItem = GNOME_CANVAS_GROUP(
				      gnome_canvas_item_new (boardRootItem,
							     gnome_canvas_group_get_type (),
							     "x", (double) 0,
							     "y", (double) 0,
							     NULL));

  // Create the vehicules
  allwagonsRootItem = GNOME_CANVAS_GROUP(
					 gnome_canvas_item_new (boardRootItem,
								gnome_canvas_group_get_type (),
								"x", (double) 0,
								"y", (double) 0,
								NULL));

  for (i=0; i<ENGINES+WAGONS; i++) {
    if (i<ENGINES)
      pixmap = g_list_nth_data(listPixmapEngines, i);
    else
      pixmap = g_list_nth_data(listPixmapWagons, i-ENGINES);

    if ( (xOffset + gdk_pixbuf_get_width(pixmap)) >= gcomprisBoard->width) {
      xOffset = 0;
      l++;
    }
    yOffset = line[l] - gdk_pixbuf_get_height(pixmap);

    item = gnome_canvas_item_new (allwagonsRootItem,
				  gnome_canvas_pixbuf_get_type (),
				  "pixbuf",  pixmap,
				  "x",  (double) xOffset,
				  "y",  (double) yOffset,
				  NULL);
    xOffset += gdk_pixbuf_get_width(pixmap);

    gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event,
		       GINT_TO_POINTER(i));

  }
  // hide them
  gnome_canvas_item_hide(GNOME_CANVAS_ITEM(allwagonsRootItem));

  // construct the model to be recognized
  modelRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (boardRootItem,
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  yOffset = line[0];
  xOffset = 0;
  model_size = gcomprisBoard->level +1; // engine + cars
  // First the cars, depending of the level
  for (i=0; i<model_size-1; i++) {
    r = (int)(((float) WAGONS)*rand()/(RAND_MAX+1.0));
    assert( r >=0 && r < WAGONS);
    // keep track of the answer
    int_model_list = g_list_append(int_model_list, GINT_TO_POINTER(r+ENGINES));
    pixmap = g_list_nth_data(listPixmapWagons, r);
    assert(i >= 0 && i<MODEL_MAX_SIZE);
    item_model[i] =gnome_canvas_item_new (modelRootItem,
					  gnome_canvas_pixbuf_get_type (),
					  "pixbuf",  pixmap,
					  "x",  (double) xOffset,
					  "y",  (double) yOffset - gdk_pixbuf_get_height(pixmap),
					  NULL);
    xOffset  += gdk_pixbuf_get_width(pixmap);
  }

  // Then the engine
  r = (int)(((float) ENGINES)*rand()/(RAND_MAX+1.0));
  assert( r >=0 && r < ENGINES);
  // keep track of the answer
  int_model_list = g_list_append(int_model_list, GINT_TO_POINTER(r));
  pixmap = g_list_nth_data(listPixmapEngines, r);
  item_model[model_size-1] =gnome_canvas_item_new (modelRootItem,
						   gnome_canvas_pixbuf_get_type (),
						   "pixbuf",  pixmap,
						   "x",  (double) xOffset,
						   "y",  (double) yOffset  - gdk_pixbuf_get_height(pixmap),
						   NULL);

  animate_model();

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
    gcompris_play_ogg ("sounds/bonus.ogg", NULL);
  }
  railroad_next_level();
}

/* ==================================== */
static void process_ok()
{
  int i;

  gamewon = TRUE;

  // DEBUG
  g_warning("l answer = %d\tl model = %d\n", g_list_length(int_answer_list), g_list_length(int_model_list));
  if (g_list_length(int_answer_list) != g_list_length(int_model_list))
    gamewon = FALSE;
  else
    for (i=0; i<g_list_length(int_answer_list); i++) {
      if ( GPOINTER_TO_INT(g_list_nth_data(int_answer_list,i)) != GPOINTER_TO_INT(g_list_nth_data(int_model_list,i))) {
	printf("pour i= %d --> différent\n", i);
	gamewon = FALSE;
	break;
      }
    }
  // DUMP lists
  g_warning("answer:\n");
  for (i=0; i<g_list_length(int_answer_list); i++)
    g_warning(" i = \t%d val = \t%d\n", i, GPOINTER_TO_INT(g_list_nth_data(int_answer_list,i)) );
  g_warning("model:\n");
  for (i=0; i<g_list_length(int_model_list); i++)
    g_warning(" i = \t%d val = \t%d\n", i, GPOINTER_TO_INT(g_list_nth_data(int_model_list,i)) );

  gcompris_display_bonus(gamewon, BONUS_FLOWER);
}
/* ==================================== */
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
  double item_x, item_y;
  int item_number;
  GdkPixbuf * pixmap = NULL;
  int i, xOffset = 0;
  GnomeCanvasItem * local_item;
  double dx1, dy1, dx2, dy2;
  item_number = GPOINTER_TO_INT(data);

  // we don't allow any input until train is gone
  if (animation_pending)
    return FALSE;

  item_x = event->button.x;
  item_y = event->button.y;
  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      g_warning("GDK_BUTTON_PRESS item %d\tlength answer = %d\n",item_number,g_list_length(item_answer_list));
      xOffset = 0;
      for (i=0; i<g_list_length(item_answer_list); i++) {
	gnome_canvas_item_get_bounds(g_list_nth_data(item_answer_list,i), &dx1, &dy1, &dx2, &dy2);
	xOffset += dx2-dx1;
      }
      if (item_number < ENGINES)
	pixmap = g_list_nth_data(listPixmapEngines, item_number);
      else
	pixmap = g_list_nth_data(listPixmapWagons, item_number-ENGINES);

      local_item =gnome_canvas_item_new (answerRootItem,
					 gnome_canvas_pixbuf_get_type (),
					 "pixbuf",  pixmap,
					 "x",  (double) xOffset,
					 "y",  (double) line[0] - gdk_pixbuf_get_height(pixmap),
					 NULL);
      item_answer_list = g_list_append(item_answer_list, local_item);
      int_answer_list = g_list_append(int_answer_list,GINT_TO_POINTER(item_number));
      //	printf("added %d to int_answer_list\n", item_number);
      gtk_signal_connect(GTK_OBJECT(local_item), "event", (GtkSignalFunc) answer_event, GINT_TO_POINTER( g_list_length(item_answer_list)-1 ));
      break;

    default:
      break;
    }
  return FALSE;
}
/* ==================================== */
/* Used to delete a vehicule at the top (the proposed answer) */
static gint answer_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
  double item_x, item_y;
  int item_number, i;
  GnomeCanvasItem *local_item;
  item_number = GPOINTER_TO_INT(data);
  // we don't allow any input until train is gone
  if (animation_pending)
    return FALSE;

  item_x = event->button.x;
  item_y = event->button.y;
  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      g_warning("Deleting %d\n",item_number);
      local_item = g_list_nth_data(item_answer_list,item_number);
      item_answer_list = g_list_remove( item_answer_list, local_item );
      //	gtk_signal_disconnect(GTK_OBJECT(local_item), (GtkSignalFunc) answer_event, NULL);
      gtk_object_destroy (GTK_OBJECT(local_item));
      int_answer_list = g_list_remove(int_answer_list, g_list_nth_data(int_answer_list, item_number) );
      reposition_answer();
      // setup the signals for the cars at the right side of the deleted object
      for (i=item_number; i<g_list_length(item_answer_list); i++) {
	local_item = g_list_nth_data(item_answer_list, i);
	gtk_signal_disconnect_by_func(GTK_OBJECT(local_item), (GtkSignalFunc) answer_event, GINT_TO_POINTER( i+1 ));
        gtk_signal_connect(GTK_OBJECT(local_item),"event", (GtkSignalFunc) answer_event, GINT_TO_POINTER( i ));
      }
      break;

    default:
      break;
    }
  return FALSE;
}
/* ==================================== */
static void reposition_answer() {
  double dx1, dy1, dx2, dy2;
  int i;
  int xOffset = 0;
  GnomeCanvasItem * item = NULL;

  if(!gcomprisBoard)
    return;

  g_warning("+++ reposition_answer\n");
  for (i=0; i<g_list_length(item_answer_list); i++) {
    item = g_list_nth_data(item_answer_list,i);
    gnome_canvas_item_get_bounds(item, &dx1, &dy1, &dx2, &dy2);
    gnome_canvas_item_move(item, xOffset-dx1, line[0]-dy2);
    xOffset += dx2-dx1;
  }
}
/* ==================================== */
static void reposition_model() {
  double dx1, dy1, dx2, dy2;
  int i;
  int xOffset = 0;
  GnomeCanvasItem * item = NULL;

  if(!gcomprisBoard)
    return;

  g_warning("+++ reposition_model\n");
  gnome_canvas_item_move(GNOME_CANVAS_ITEM(modelRootItem), 0, 0);
  for (i=0; i<model_size; i++) {
    item = item_model[i];
    gnome_canvas_item_get_bounds(item, &dx1, &dy1, &dx2, &dy2);
    gnome_canvas_item_move(item, xOffset-dx1, line[0]-dy2);
    xOffset += dx2-dx1;
  }
}

/* ==================================== */
static gboolean animate_step() {
  double step = 0;

  // this defines how the train waits before start
#define MODEL_PAUSE 30
  //	printf("+++animate_step %d \n",animation_count);

  if(board_paused)
    return TRUE;

  if(!gcomprisBoard)
    return FALSE;

  animation_count++;

  if (animation_count < MODEL_PAUSE)
    return TRUE;

  if (animation_count >= 160+MODEL_PAUSE) {
    if (timer_id) {
      gtk_timeout_remove (timer_id);
      timer_id = 0;
    }
    animation_pending = FALSE;
    gnome_canvas_item_hide(GNOME_CANVAS_ITEM(modelRootItem));
    /* Move back the model to its 0 position */
    gnome_canvas_item_set(GNOME_CANVAS_ITEM(modelRootItem),
			  "x", 0.0,
			  NULL);

    gnome_canvas_item_show(GNOME_CANVAS_ITEM(allwagonsRootItem));
    gnome_canvas_item_show(GNOME_CANVAS_ITEM(answerRootItem));
    return FALSE;
  }

  step = (double) (animation_count-MODEL_PAUSE) / 50.0;
  step *= step;

  gnome_canvas_item_move(GNOME_CANVAS_ITEM(modelRootItem), step, 0.0);

  return TRUE;
}
/* ==================================== */
static void animate_model() {
  animation_pending = TRUE;
  animation_count = 0;

  gcompris_play_ogg( "sounds/train.ogg", NULL );

  // warning : if timeout is too low, the model will not be displayed
  timer_id = gtk_timeout_add (100, (GtkFunction) animate_step, NULL);
}
/* ==================================== */
static void reset_all_lists(void) {
  GnomeCanvasItem *item;

  int_model_list = reset_list(int_model_list);
  int_answer_list = reset_list(int_answer_list);

  while(g_list_length(item_answer_list)>0) {
    item = g_list_nth_data(item_answer_list, 0);
    item_answer_list = g_list_remove (item_answer_list, item);
    // causes segfaults
    //  	gtk_object_destroy (GTK_OBJECT(item));
  }

}
/* ==================================== */
static GList * reset_list(GList * list) {
  while (g_list_length(list) > 0)
    list = g_list_remove(list, g_list_nth_data(list,0));

  return list;
}
