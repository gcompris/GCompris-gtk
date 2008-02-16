/* gcompris - erase.c
 *
 * Copyright (C) 2001 Bruno Coudoin
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

#include <string.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE
#define MAX_LAYERS 3

typedef struct  {gint count; gint max;} counter;

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;
static SoundPolicy sound_policy;
static RsvgHandle *CoverPixmap[MAX_LAYERS];
static gulong event_handle_id;


static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem	*erase_create_item();
static void		 erase_destroy_all_items(void);
static void		 erase_next_level(void);
static void		 shuffle_image_list(char *list[], int size);
static gboolean		 item_event (GooCanvasItem  *item,
				     GooCanvasItem  *target,
				     GdkEventCrossing *event,
				     gpointer data);
static gboolean		 canvas_event (GooCanvasItem  *item,
				       GooCanvasItem  *target,
				       GdkEventButton *event,
				       gpointer data);

static int number_of_items = 0;
static int number_of_item_x = 0;
static int number_of_item_y = 0;
static int *items_per_cell = 0;
static guint normal_delay_id = 0;

static gint timer_id = 0;

#define NORMAL 0
#define CLIC   1

static gint board_mode =  NORMAL;

// List of images to use in the game
static gchar *imageList[] =
  {
    "erase/bear001.jpg",
    "erase/black-headed-gull.jpg",
    "erase/butterfly.jpg",
    "erase/cat1.jpg",
    "erase/cat2.jpg",
    "erase/donkey.jpg",
    "erase/elephanteauxgc.jpg",
    "erase/flamentrosegc.jpg",
    "erase/girafegc.jpg",
    "erase/hypogc.jpg",
    "erase/joybear001.jpg",
    "erase/joybear002.jpg",
    "erase/jumentmulassieregc.jpg",
    "erase/malaybear002.jpg",
    "erase/pigeon.jpg",
    "erase/polabear011.jpg",
    "erase/polarbear001.jpg",
    "erase/poolbears001.jpg",
    "erase/rhinogc.jpg",
    "erase/singegc.jpg",
    "erase/spectbear001.jpg",
    "erase/tetegorillegc.jpg",
    "erase/tiger1_by_Ralf_Schmode.jpg",
    "erase/tigercub003.jpg",
    "erase/tigerdrink001.jpg",
    "erase/tigerplay001.jpg",
    "erase/horses.jpg",
    "erase/horses2.jpg",
    "erase/squirrel.jpg",
    "erase/sheep_irish.jpg",
    "erase/sheep_irish2.jpg",
    "erase/cow.jpg",
    "erase/maki1.jpg",
    "erase/maki2.jpg",
    "erase/maki3.jpg",
    "erase/maki4.jpg",
    "erase/maki5.jpg",
    "erase/maki6.jpg",
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
      gcomprisBoard->number_of_sublevel=10;
      gc_bar_set(GC_BAR_LEVEL);

      /* CAUTION: CoverPixmap has MAX_LAYERS elements */
      CoverPixmap[0] = gc_rsvg_load("erase/transparent_square.svgz");
      CoverPixmap[1] = gc_rsvg_load("erase/transparent_square_green.svgz");
      CoverPixmap[2] = gc_rsvg_load("erase/transparent_square_yellow.svgz");

      event_handle_id =
	g_signal_connect(goo_canvas_get_root_item(gcomprisBoard->canvas),
			 "button_press_event",
			 (GtkSignalFunc) canvas_event, NULL);

      if (strcmp(gcomprisBoard->mode,"clic")==0)
	board_mode = CLIC;
      else {
	board_mode = NORMAL;
	gcomprisBoard->maxlevel=8;
      }

      current_image = 0;
      shuffle_image_list(imageList, NUMBER_OF_IMAGES);

      erase_next_level();

      gamewon = FALSE;
      pause_board(FALSE);

      /* initial state to restore */
      sound_policy = gc_sound_policy_get();
      gc_sound_policy_set(PLAY_AND_INTERRUPT);

      GcomprisProperties *properties = gc_prop_get ();
      if(properties->defaultcursor == GCOMPRIS_DEFAULT_CURSOR)
	{
	  GdkPixbuf *cursor_pixbuf = gc_pixmap_load("erase/sponge.png");
	  if(cursor_pixbuf)
	    {
	      GdkCursor *cursor = NULL;
	      cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(),
						  cursor_pixbuf,
						  gdk_pixbuf_get_width(cursor_pixbuf)/2,
						  gdk_pixbuf_get_height(cursor_pixbuf)/2);
	      gdk_window_set_cursor(gc_get_window()->window, cursor);
	      gdk_cursor_unref(cursor);
	      gdk_pixbuf_unref(cursor_pixbuf);
	    }
	}
    }
}
/* ======================================= */
static void end_board ()
{
  int i;
  for(i=0; i<MAX_LAYERS; i++)
    if(CoverPixmap[i]) {
      g_object_unref(CoverPixmap[i]);
      CoverPixmap[i]=NULL;
    }

  gc_cursor_set(GCOMPRIS_DEFAULT_CURSOR);
  if(gcomprisBoard!=NULL)
    {
      g_signal_handler_disconnect(gcomprisBoard->canvas,
				  event_handle_id);
      pause_board(TRUE);
      erase_destroy_all_items();
    }
  gcomprisBoard = NULL;
  gc_sound_policy_set(sound_policy);
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

static int get_num_layers()
{
  int layers;
  /* Select the number of layer depending on the level */

  if(gcomprisBoard->level>6)
    layers = 4;
  else if(gcomprisBoard->level>4)
    layers = 3;
  else if(gcomprisBoard->level>2)
    layers = 2;
  else
    layers = 1;

  return layers;
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* set initial values for the next level */
static void erase_next_level()
{
  gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			  imageList[current_image++]);

  if(current_image>=NUMBER_OF_IMAGES)
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
    assert(!items_per_cell);
    items_per_cell = g_new0(int, number_of_item_x * number_of_item_y);
  }

  /* Try the next level */
  erase_create_item();
}

/* ==================================== */
/* Destroy all the items */
static void erase_destroy_all_items()
{
  if (normal_delay_id) {
    g_source_remove (normal_delay_id);
    normal_delay_id = 0;
  }

  if (timer_id) {
    gtk_timeout_remove (timer_id);
    timer_id = 0;
  }

  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);
  boardRootItem = NULL;

  number_of_items = 0;

  if (items_per_cell) {
    g_free(items_per_cell);
    items_per_cell = NULL;
  }
}

static void add_one_item(int i, int j, int protect)
{
  int current_layer = get_num_layers();
  double w = (BOARDWIDTH/number_of_item_x) ;
  double h = (BOARDHEIGHT/number_of_item_y) ;
  int item_x = i / w;
  int item_y = j / h;
  i = item_x * w;
  j = item_y * h;

  if ((board_mode != NORMAL) && ((item_x+item_y) %2 == 0))
    return;

  if (current_layer == 4)
    current_layer = 1;

  while(current_layer--)
    {
      RsvgDimensionData dimension;

      assert(CoverPixmap[current_layer]);
      GooCanvasItem *item =
	goo_svg_item_new (boardRootItem,
			  CoverPixmap[current_layer],
			  NULL);
      rsvg_handle_get_dimensions(CoverPixmap[current_layer], &dimension);
      double scale = h/dimension.height;
      goo_canvas_item_set_simple_transform(item,
					   i,
					   j,
					   scale, 0.0);

      counter *c = g_new (counter, 1);
      c->count = 0 ;
      c->max = protect;
      protect = 0; /* only protect the top item */

      /* if item is not first, it must be keep first time mouse
       * pass over in normal mode or in layer 4 */
      if (current_layer > 0 || get_num_layers() == 4)
	c->max += 1;

      g_signal_connect_data (item, "enter_notify_event",
			     (GCallback) item_event,
			     (gpointer)c,
			     (GClosureNotify) g_free, 0);
      number_of_items++;
      if (items_per_cell)
	items_per_cell[item_x * number_of_item_x + item_y]++;
    }
}

/* ==================================== */
static GooCanvasItem *erase_create_item()
{
  int i,j;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);

  assert(number_of_items == 0);

  for(i=0; i<BOARDWIDTH; i+=BOARDWIDTH/number_of_item_x)
    for(j=0; j<BOARDHEIGHT; j+=BOARDHEIGHT/number_of_item_y)
      add_one_item(i, j, 0);

  return NULL;
}

static void bonus() {
  gc_bonus_display(gamewon, GC_BONUS_SMILEY);
  timer_id = 0;
}

static void finished() {
  gc_bonus_end_display(GC_BOARD_FINISHED_RANDOM);
  timer_id = 0;
}

/* ==================================== */
static void
game_won()
{
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    /* the current board is finished : bail out */
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) {
      timer_id = gtk_timeout_add (2000, (GtkFunction) finished, NULL);
      return;
    }
    gc_sound_play_ogg ("sounds/bonus.wav", NULL);
  }
  erase_next_level();
}

static gboolean
erase_one_item (GooCanvasItem *item)
{
  gdouble screen_x, screen_y;
  int x,y;
  goo_canvas_convert_from_item_space(goo_canvas_item_get_canvas(item),
				     item, &screen_x, &screen_y);
  x = screen_x / (BOARDWIDTH/number_of_item_x);
  y = screen_y / (BOARDHEIGHT/number_of_item_y);

  if (items_per_cell)
    items_per_cell[(int) (x * number_of_item_x + y)]--;

  goo_canvas_item_remove(item);

  if(number_of_items%2)
    gc_sound_play_ogg ("sounds/eraser1.wav", NULL);
  else
    gc_sound_play_ogg ("sounds/eraser2.wav", NULL);

  if(--number_of_items == 0)
    {
      gamewon = TRUE;
      erase_destroy_all_items();
      timer_id = gtk_timeout_add (4000, (GtkFunction) bonus, NULL);
    }
  normal_delay_id = 0;
  return FALSE;
}

/* ==================================== */
static gboolean
item_event (GooCanvasItem  *item,
	    GooCanvasItem  *target,
	    GdkEventCrossing *event,
	    gpointer data)
{
  counter *c = (counter *) data;
  if(board_paused)
    return FALSE;

  if (board_mode == NORMAL) {
    if (event->type == GDK_ENTER_NOTIFY) {
      if (c->count < c->max){
	c->count++ ;
	return FALSE ;
      }
      /* Are enter & leave always sent in pairs? Don't assume. */
      if (normal_delay_id)
	g_source_remove (normal_delay_id);
      normal_delay_id
	= g_timeout_add (50, (GSourceFunc) erase_one_item, item);
    } else if (event->type == GDK_LEAVE_NOTIFY) {
      if (normal_delay_id)
	g_source_remove (normal_delay_id);
      normal_delay_id = 0;
    }
    return FALSE;
  }
  if (board_mode == CLIC)
    if (event->type != GDK_BUTTON_PRESS)
      return FALSE;

  erase_one_item (item);

  return FALSE;
}

static gboolean
canvas_event (GooCanvasItem  *item,
	      GooCanvasItem  *target,
	      GdkEventButton *event,
	      gpointer data)
{
  if (!gcomprisBoard || board_paused || gamewon)
    return FALSE;

  if (board_mode == NORMAL)
    {
      int x = event->x;
      int y = event->y;
      int item_x = x / (BOARDWIDTH/number_of_item_x);
      int item_y = y / (BOARDHEIGHT/number_of_item_y);

      if (items_per_cell[item_x * number_of_item_x + item_y] == 0)
	add_one_item(x, y, 1);

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
  char *olditem;

  for(i=0; i < size - 1; i++)
    {
      int random1 = g_random_int_range(i, size-1);
      if (i == random1) continue;

      olditem = list[i];
      list[i] = list[random1];
      list[random1] = olditem;
    }
}
