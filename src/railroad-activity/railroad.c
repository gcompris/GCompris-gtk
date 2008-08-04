/* gcompris - railroad.c
 *
 * Copyright (C) 2001, 2008 Pascal Georges
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
static void		 process_ok(void);
static void		 game_won(void);
static void		 repeat(void);
static void		 animate_model(void);
static gboolean		 animate_step(void);
static void		 stop_animation();

#define ENGINES 9
#define WAGONS 13
#define NUMBER_OF_SUBLEVELS 3
#define NUMBER_OF_LEVELS 3
#define MODEL_MAX_SIZE NUMBER_OF_LEVELS+1

static const int line[] = { 100,180,260,340, 420, 500};
static gboolean animation_pending;
static gint animation_count = 0;

static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem *allwagonsRootItem = NULL;
static GooCanvasItem *modelRootItem = NULL;
static GooCanvasItem *answerRootItem = NULL;

static GList * listPixmapEngines = NULL;
static GList * listPixmapWagons = NULL;

// ==========================================
// In all the lists below, 0 is the LEFTmost vehicle|
// ==========================================
// contains the list of vehicles to be found.
static GooCanvasItem *item_model[MODEL_MAX_SIZE];
// contains the list of vehicles proposed by child.
static GList *item_answer_list = NULL;
// contains the list of vehicles proposed by child.
static GList *int_answer_list = NULL;
// contains the list of vehicles to be found
static GList *int_model_list = NULL;

static int model_size = 0;
static gint timer_id;

static GooCanvasItem *railroad_create_item(GooCanvasItem *parent);
static void railroad_destroy_all_items(void);
static void railroad_next_level(void);
static gint item_event(GooCanvasItem *item,
		       GooCanvasItem *target,
		       GdkEventButton *event,
		       gpointer data);
static gint answer_event(GooCanvasItem *item,
			 GooCanvasItem *target,
			 GdkEventButton *event,
			 gpointer data);

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
      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			      "railroad/railroad-bg.svg");

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
      gc_score_start(SCORESTYLE_NOTE,
		     BOARDWIDTH - 220,
		     BOARDHEIGHT - 50,
		     gcomprisBoard->number_of_sublevel);


      str = gc_skin_image_get("button_reload.png");
      pixmap = gc_pixmap_load(str);
      g_free(str);
      if(pixmap) {
	gc_bar_set_repeat_icon(pixmap);
	gdk_pixbuf_unref(pixmap);
	gc_bar_set(GC_BAR_LEVEL|GC_BAR_REPEAT_ICON);
      } else {
	gc_bar_set(GC_BAR_LEVEL|GC_BAR_REPEAT);
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
      gc_score_end();
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
    g_object_set (answerRootItem, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
    g_object_set (modelRootItem, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
    g_object_set (allwagonsRootItem, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
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
  g_assert(g_list_length(item_answer_list) == 0 && g_list_length(int_answer_list) == 0 && g_list_length(int_model_list) == 0);

  model_size = 0;

  railroad_destroy_all_items();
  gamewon = FALSE;
  gc_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);

  railroad_create_item(boardRootItem);
}
/* ==================================== */
/* Destroy all the items */
static void railroad_destroy_all_items()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}
/* ==================================== */
static GooCanvasItem *railroad_create_item(GooCanvasItem *boardRootItem)
{
  int xOffset = 0, yOffset = 0;
  int i, r, l = 1;
  GdkPixbuf * pixmap = NULL;
  GooCanvasItem *item;

  /* Create a root group for the answer */
  answerRootItem = goo_canvas_group_new (boardRootItem, NULL);

  // Create the vehicules
  allwagonsRootItem = goo_canvas_group_new (boardRootItem, NULL);

  for (i=0; i<ENGINES+WAGONS; i++) {
    if (i<ENGINES)
      pixmap = g_list_nth_data(listPixmapEngines, i);
    else
      pixmap = g_list_nth_data(listPixmapWagons, i-ENGINES);

    if ( (xOffset + gdk_pixbuf_get_width(pixmap)) >= BOARDWIDTH) {
      xOffset = 0;
      l++;
    }
    yOffset = line[l] - gdk_pixbuf_get_height(pixmap);

    item = goo_canvas_image_new (allwagonsRootItem,
				 pixmap,
				 xOffset,
				 yOffset,
				 NULL);
    xOffset += gdk_pixbuf_get_width(pixmap);

    g_signal_connect(item,
		     "button_press_event", (GtkSignalFunc) item_event,
		     GINT_TO_POINTER(i));

  }
  // hide them
  g_object_set (allwagonsRootItem, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

  // construct the model to be recognized
  modelRootItem = goo_canvas_group_new (boardRootItem, NULL);

  yOffset = line[0];
  xOffset = 0;
  model_size = gcomprisBoard->level +1; // engine + cars
  // First the cars, depending of the level
  for (i=0; i<model_size-1; i++) {
    r = g_random_int_range( 0, WAGONS);
    g_assert( r >=0 && r < WAGONS);
    // keep track of the answer
    int_model_list = g_list_append(int_model_list, GINT_TO_POINTER(r+ENGINES));
    pixmap = g_list_nth_data(listPixmapWagons, r);
    g_assert(i >= 0 && i<MODEL_MAX_SIZE);
    item_model[i] = goo_canvas_image_new (modelRootItem,
					  pixmap,
					  xOffset,
					  yOffset - gdk_pixbuf_get_height(pixmap),
					  NULL);
    g_signal_connect(item_model[i],
		     "button_press_event", (GtkSignalFunc) stop_animation,
		     NULL);
    xOffset  += gdk_pixbuf_get_width(pixmap);
  }

  // Then the engine
  r = g_random_int_range( 0, ENGINES);
  g_assert( r >=0 && r < ENGINES);
  // keep track of the answer
  int_model_list = g_list_append(int_model_list, GINT_TO_POINTER(r));
  pixmap = g_list_nth_data(listPixmapEngines, r);
  item_model[model_size-1] =goo_canvas_image_new (modelRootItem,
						  pixmap,
						  xOffset,
						  yOffset  - gdk_pixbuf_get_height(pixmap),
						  NULL);

  g_signal_connect(item_model[model_size-1],
		   "button_press_event", (GtkSignalFunc) stop_animation,
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
      gc_bonus_end_display(GC_BOARD_FINISHED_RANDOM);
      return;
    }
    gc_sound_play_ogg ("sounds/bonus.wav", NULL);
  }
  railroad_next_level();
}

/* ==================================== */
static void process_ok()
{
  int i;

  gamewon = TRUE;

  if (g_list_length(int_answer_list) != g_list_length(int_model_list))
    gamewon = FALSE;
  else
    for (i=0; i<g_list_length(int_answer_list); i++) {
      if ( GPOINTER_TO_INT(g_list_nth_data(int_answer_list,i)) != GPOINTER_TO_INT(g_list_nth_data(int_model_list,i))) {
	//printf("pour i= %d --> différent\n", i);
	gamewon = FALSE;
	break;
      }
    }

  if(gamewon)
    gc_bonus_display(gamewon, GC_BONUS_FLOWER);
}
/* ==================================== */
static gint item_event(GooCanvasItem *item,
		       GooCanvasItem *target,
		       GdkEventButton *event,
		       gpointer data)
{
  double item_x, item_y;
  int item_number;
  GdkPixbuf * pixmap = NULL;
  int i, xOffset = 0;
  GooCanvasItem * local_item;
  item_number = GPOINTER_TO_INT(data);

  // we don't allow any input until train is gone
  if (animation_pending)
    return FALSE;

  item_x = event->x;
  item_y = event->y;
  goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(item),
				   item, &item_x, &item_y);

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      gc_sound_play_ogg ("sounds/bleep.wav", NULL);
      xOffset = 0;
      for (i=0; i<g_list_length(item_answer_list); i++) {
	GooCanvasBounds bounds;
	goo_canvas_item_get_bounds(g_list_nth_data(item_answer_list,i), &bounds);
	xOffset += bounds.x2 - bounds.x1;
      }
      if (item_number < ENGINES)
	pixmap = g_list_nth_data(listPixmapEngines, item_number);
      else
	pixmap = g_list_nth_data(listPixmapWagons, item_number-ENGINES);

      local_item =goo_canvas_image_new (answerRootItem,
					pixmap,
					xOffset,
					line[0] - gdk_pixbuf_get_height(pixmap),
					NULL);
      item_answer_list = g_list_append(item_answer_list, local_item);
      int_answer_list = g_list_append(int_answer_list,GINT_TO_POINTER(item_number));
      //	printf("added %d to int_answer_list\n", item_number);
      g_signal_connect(local_item,
		       "button_press_event", (GtkSignalFunc) answer_event,
		       GINT_TO_POINTER( g_list_length(item_answer_list)-1 ));
      process_ok();
      break;

    default:
      break;
    }
  return FALSE;
}
/* ==================================== */
/* Used to delete a vehicule at the top (the proposed answer) */
static gint answer_event(GooCanvasItem *item,
			 GooCanvasItem *target,
			 GdkEventButton *event,
			 gpointer data)
{
  double item_x, item_y;
  int item_number, i;
  GooCanvasItem *local_item;
  item_number = GPOINTER_TO_INT(data);
  // we don't allow any input until train is gone
  if (animation_pending)
    return FALSE;

  item_x = event->x;
  item_y = event->y;
  goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(item),
				   item, &item_x, &item_y);

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      gc_sound_play_ogg ("sounds/smudge.wav", NULL);
      local_item = g_list_nth_data(item_answer_list,item_number);
      item_answer_list = g_list_remove( item_answer_list, local_item );
      goo_canvas_item_remove(local_item);
      int_answer_list = g_list_remove(int_answer_list, g_list_nth_data(int_answer_list, item_number) );
      reposition_answer();

      // setup the signals for the cars at the right side of the deleted object
      for (i=item_number; i<g_list_length(item_answer_list); i++)
	{
	  local_item = g_list_nth_data(item_answer_list, i);
	  g_signal_handlers_disconnect_by_func(G_OBJECT(local_item),
				      (GtkSignalFunc) answer_event,
				      GINT_TO_POINTER( i+1 ));
	  g_signal_connect(local_item,
			   "button_press_event",
			   (GtkSignalFunc) answer_event,
			   GINT_TO_POINTER( i ));
	}
      break;

    default:
      break;
    }
  return FALSE;
}
/* ==================================== */
static void reposition_answer() {
  int i;
  int xOffset = 0;
  GooCanvasItem * item = NULL;

  if(!gcomprisBoard)
    return;

  for (i=0; i<g_list_length(item_answer_list); i++) {
    GooCanvasBounds bounds;
    item = g_list_nth_data(item_answer_list,i);
    goo_canvas_item_get_bounds(item, &bounds);
    goo_canvas_item_translate(item, xOffset-bounds.x1, line[0]-bounds.y2);
    xOffset += bounds.x2 - bounds.x1;
  }
}
/* ==================================== */
static void reposition_model() {
  int i;
  int xOffset = 0;
  GooCanvasItem * item = NULL;

  if(!gcomprisBoard)
    return;

  goo_canvas_item_translate(modelRootItem, 0, 0);
  for (i=0; i<model_size; i++) {
    GooCanvasBounds bounds;
    item = item_model[i];
    goo_canvas_item_get_bounds(item, &bounds);
    goo_canvas_item_translate(item, xOffset-bounds.x1, line[0]-bounds.y2);
    xOffset += bounds.x2 - bounds.x1;
  }
}

static void stop_animation()
{
  if (timer_id) {
    gtk_timeout_remove (timer_id);
    timer_id = 0;
  }
  animation_pending = FALSE;
  g_object_set (modelRootItem,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);

  g_object_set (allwagonsRootItem, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
  g_object_set (answerRootItem, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
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
    stop_animation();
    return FALSE;
  }

  step = (double) (animation_count-MODEL_PAUSE) / 50.0;
  step *= step;

  goo_canvas_item_translate(modelRootItem, step, 0.0);

  return TRUE;
}
/* ==================================== */
static void animate_model() {
  animation_pending = TRUE;
  animation_count = 0;

  gc_sound_play_ogg( "sounds/train.wav", NULL );

  // warning : if timeout is too low, the model will not be displayed
  timer_id = gtk_timeout_add (100, (GtkFunction) animate_step, NULL);
}
/* ==================================== */
static void reset_all_lists(void) {
  GooCanvasItem *item;

  int_model_list = reset_list(int_model_list);
  int_answer_list = reset_list(int_answer_list);

  while(g_list_length(item_answer_list)>0) {
    item = g_list_nth_data(item_answer_list, 0);
    item_answer_list = g_list_remove (item_answer_list, item);
    // causes segfaults
    //  	goo_canvas_item_remove(item);
  }

}
/* ==================================== */
static GList * reset_list(GList * list) {
  while (g_list_length(list) > 0)
    list = g_list_remove(list, g_list_nth_data(list,0));

  return list;
}
