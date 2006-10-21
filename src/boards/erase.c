/* gcompris - erase.c
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

#include <string.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE
#define MAX_LAYERS 3

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem	*erase_create_item(int layer);
static void		 erase_destroy_all_items(void);
static void		 erase_next_level(void);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static void		 shuffle_image_list(char *list[], int size);

static int number_of_item = 0;
static int number_of_item_x = 0;
static int number_of_item_y = 0;

static gint timer_id = 0;

// Default Double click distance to restore on exit.
static gint DefaultDoubleClicDistance;

static gint DoubleClicLevel[6]= {  1000, 750, 600, 500, 400, 250};

#define NORMAL 0
#define CLIC   1
#define DOUBLECLIC 2

static gint board_mode =  NORMAL;

// List of images to use in the game
static gchar *imageList[] =
  {
    "gcompris/animals/bear001.jpg",
    "gcompris/animals/black-headed-gull.jpg",
    "gcompris/animals/butterfly.jpg",
    "gcompris/animals/donkey.jpg",
    "gcompris/animals/elephanteauxgc.jpg",
    "gcompris/animals/flamentrosegc.jpg",
    "gcompris/animals/girafegc.jpg",
    "gcompris/animals/hypogc.jpg",
    "gcompris/animals/joybear001.jpg",
    "gcompris/animals/joybear002.jpg",
    "gcompris/animals/jumentmulassieregc.jpg",
    "gcompris/animals/malaybear002.jpg",
    "gcompris/animals/pigeon.jpg",
    "gcompris/animals/polabear011.jpg",
    "gcompris/animals/polarbear001.jpg",
    "gcompris/animals/poolbears001.jpg",
    "gcompris/animals/rhinogc.jpg",
    "gcompris/animals/singegc.jpg",
    "gcompris/animals/spectbear001.jpg",
    "gcompris/animals/tetegorillegc.jpg",
    "gcompris/animals/tiger1_by_Ralf_Schmode.jpg",
    "gcompris/animals/tigercub003.jpg",
    "gcompris/animals/tigerdrink001.jpg",
    "gcompris/animals/tigerplay001.jpg",
    "gcompris/animals/horses.jpg",
    "gcompris/animals/horses2.jpg",
    "gcompris/animals/squirrel.jpg",
    "gcompris/animals/sheep_irish.jpg",
    "gcompris/animals/sheep_irish2.jpg",
    "gcompris/animals/cow.jpg",
    "gcompris/animals/maki1.jpg",
    "gcompris/animals/maki2.jpg",
    "gcompris/animals/maki3.jpg",
    "gcompris/animals/maki4.jpg",
    "gcompris/animals/maki5.jpg",
    "gcompris/animals/maki6.jpg",
  };
#define NUMBER_OF_IMAGES G_N_ELEMENTS(imageList)

/* Store the image index to use */
static int current_image;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Move the mouse"),
    N_("Move the mouse to erase the area and discover the background"),
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

GET_BPLUGIN_INFO(erase)

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if (timer_id) {
    gtk_timeout_remove (timer_id);
    timer_id = 0;
  }

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
      gcomprisBoard->maxlevel=6;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=10; /* Go to next level after this number of 'play' */
      gc_score_start(SCORESTYLE_NOTE,
			   gcomprisBoard->width - 220,
			   gcomprisBoard->height - 50,
			   gcomprisBoard->number_of_sublevel);
      gc_bar_set(GC_BAR_LEVEL);

      if (strcmp(gcomprisBoard->mode,"double_clic")==0)
	board_mode = DOUBLECLIC;
      else if (strcmp(gcomprisBoard->mode,"clic")==0)
	board_mode = CLIC;
      else
	board_mode = NORMAL;

      if (board_mode == DOUBLECLIC){
	GtkSettings *DefaultsGtkSettings = gtk_settings_get_default ();

	if (DefaultsGtkSettings == NULL) {
	  g_warning("Couldn't get GTK settings");
	} else {
	  g_object_get(G_OBJECT(DefaultsGtkSettings),
		       "gtk-double-click-time", &DefaultDoubleClicDistance, NULL);

	  g_warning("Double-click default value %d.",DefaultDoubleClicDistance);
	}

	gdk_display_set_double_click_time( gdk_display_get_default(),
					   DoubleClicLevel[gcomprisBoard->level-1]);
	g_warning("Double-click value is now %d.",DoubleClicLevel[gcomprisBoard->level-1]);
      }

      current_image = 0;
      shuffle_image_list(imageList, NUMBER_OF_IMAGES);

      erase_next_level();

      gamewon = FALSE;
      pause_board(FALSE);
    }
}
/* ======================================= */
static void end_board ()
{
  if (board_mode == DOUBLECLIC){
    gdk_display_set_double_click_time( gdk_display_get_default(),
					   DefaultDoubleClicDistance);
    g_warning("Double click value is now %d.",DefaultDoubleClicDistance);
  }
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gc_score_end();
      erase_destroy_all_items();
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
      erase_next_level();
    }
  if (board_mode == DOUBLECLIC){
    gdk_display_set_double_click_time( gdk_display_get_default(),
				       DoubleClicLevel[gcomprisBoard->level-1]);
    g_warning("Double click value is now %d.",DoubleClicLevel[gcomprisBoard->level-1]);
  }

}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "erase")==0)
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
static void erase_next_level()
{
  int layers = 1;

  gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			  imageList[current_image++]);

  if(current_image>NUMBER_OF_IMAGES)
    current_image=0;

  gc_bar_set_level(gcomprisBoard);

  erase_destroy_all_items();
  gamewon = FALSE;

  /* Select level difficulty */

  if (board_mode != NORMAL) {
    number_of_item_x = 5;
    number_of_item_y = 5;
  } else {
    number_of_item_x = ((gcomprisBoard->level+1)%2+1)*5;
    number_of_item_y = ((gcomprisBoard->level+1)%2+1)*5;
  }

  /* Select the number of layer depending on the level */

  if (board_mode != DOUBLECLIC){
    if(gcomprisBoard->level>4)
      layers = 3;
    else if(gcomprisBoard->level>2)
      layers = 2;
  }

  /* Try the next level */
  erase_create_item(layers);

  gc_score_set(gcomprisBoard->sublevel);

}

/* ==================================== */
/* Destroy all the items */
static void erase_destroy_all_items()
{
  if (timer_id) {
    gtk_timeout_remove (timer_id);
    timer_id = 0;
  }

  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
static GnomeCanvasItem *erase_create_item(int layer)
{
  int i,j;
  int ix, jy;
  GnomeCanvasItem *item = NULL;
  GdkPixbuf *pixmap[MAX_LAYERS];

  g_assert(layer<=MAX_LAYERS);

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  number_of_item = 0;

  for(i=0; i<MAX_LAYERS; i++)
    pixmap[i] = NULL;

  if(layer>0)
    pixmap[0] = gc_pixmap_load("images/transparent_square.png");

  if(layer>1)
    pixmap[1] = gc_pixmap_load("images/water_spot.png");

  if(layer>2)
    pixmap[2] = gc_pixmap_load("images/water_drop.png");

  for(i=0,ix=0; i<BOARDWIDTH; i+=BOARDWIDTH/number_of_item_x, ix++)
    {
      for(j=0, jy=0; j<BOARDHEIGHT; j+=BOARDHEIGHT/number_of_item_y, jy++)
	{
	  int current_layer = layer;

	  if ((board_mode != NORMAL) && ((ix+jy) %2 == 0))
	    continue;

	  while(current_layer--)
	    {
	      double w = (BOARDWIDTH/number_of_item_x) *  (1.0 - (0.3 * current_layer));
	      double h = (BOARDHEIGHT/number_of_item_y) * (1.0 - (0.3 * current_layer));
	      double x = i + ((BOARDWIDTH/number_of_item_x)  - w) / 2;
	      double y = j + ((BOARDHEIGHT/number_of_item_y) - h) / 2;

	      item = gnome_canvas_item_new (boardRootItem,
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", pixmap[current_layer],
					    "x", x,
					    "y", y,
					    "width", w,
					    "height", h,
					    "width_set", TRUE,
					    "height_set", TRUE,
					    "anchor", GTK_ANCHOR_NW,
					    NULL);

	      gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event, NULL);
	      number_of_item++;
	    }
	}
    }

  for(i=0; i<MAX_LAYERS; i++)
    if(pixmap[i])
      gdk_pixbuf_unref(pixmap[i]);

  return NULL;
}

static void bonus() {
  gc_bonus_display(gamewon, BONUS_SMILEY);
  timer_id = 0;
}

static void finished() {
  gc_bonus_end_display(BOARD_FINISHED_RANDOM);
  timer_id = 0;
}

/* ==================================== */
static void game_won()
{
  gcomprisBoard->sublevel++;
  gc_score_set(gcomprisBoard->sublevel);

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
      timer_id = gtk_timeout_add (2000, (GtkFunction) finished, NULL);
      return;
    }
    if (board_mode == DOUBLECLIC){
      gdk_display_set_double_click_time( gdk_display_get_default(),
					 DoubleClicLevel[gcomprisBoard->level-1]);
      g_warning("Double click value is now %d.", DoubleClicLevel[gcomprisBoard->level-1]);
    }

    gc_sound_play_ogg ("sounds/bonus.ogg", NULL);
  }
  erase_next_level();
}

/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{

  if(board_paused)
    return FALSE;

  if (event->type == GDK_MOTION_NOTIFY)
    return FALSE;

  if (board_mode == NORMAL)
    if (event->type != GDK_ENTER_NOTIFY)
      return FALSE;

  if (board_mode == CLIC)
    if (event->type != GDK_BUTTON_PRESS)
      return FALSE;

  if (board_mode == DOUBLECLIC)
    if ((event->type != GDK_BUTTON_PRESS) &
	(event->type != GDK_2BUTTON_PRESS) &
	(event->type != GDK_BUTTON_RELEASE))
      return FALSE;


  if (board_mode == DOUBLECLIC){
    if (event->type == GDK_BUTTON_PRESS){
      return FALSE;
    }
    if (event->type == GDK_BUTTON_RELEASE){
      return FALSE;
    }
  }

  gtk_object_destroy(GTK_OBJECT(item));

  if(--number_of_item == 0)
    {
      gamewon = TRUE;
      erase_destroy_all_items();
      timer_id = gtk_timeout_add (4000, (GtkFunction) bonus, NULL);
    }

  return FALSE;
}

/** \brief shuffle_image_list, takes a char* array and it's length.
 *         it swaps a random number of items in it in order to provide
 *         the same list but in a random order.
 *
 * \param list: the array to shuffle
 * \param size: the size of the array
 *
 */
void shuffle_image_list(char *list[], int size)
{
  int i;

  for(i=0; i<size; i++)
    {
      int random1 = RAND(0, size-1);
      int random2 = RAND(0, size-1);
      char *olditem;

      olditem = list[random2];
      list[random2] = list[random1];
      list[random1] = olditem;
    }
}
