/* gcompris - reversecount.c
 *
 * Copyright (C) 2002, 2008 Bruno Coudoin
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

static GcomprisBoard	*gcomprisBoard = NULL;
static gboolean	 board_paused = TRUE;
static gint	 animate_id = 0;

static void	 start_board (GcomprisBoard *agcomprisBoard);
static gint	 key_press(guint keyval, gchar *commit_str, gchar *preedit_str);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

#define NORTH 1
#define WEST 2
#define SOUTH 4
#define EAST 8

#define TUX_TO_BORDER_GAP 10

static GooCanvasItem *boardRootItem = NULL;

static void		 process_ok(void);
static void		 process_error(void);
static GooCanvasItem	*reversecount_create_item(GooCanvasItem *parent);
static void		 reversecount_destroy_all_items(void);
static void		 reversecount_next_level(void);
static gboolean		 item_event (GooCanvasItem  *item,
				     GooCanvasItem  *target,
				     GdkEventButton *event,
				     gint *dice_index);
static GooCanvasItem	*display_item_at(gchar *imagename, int block);
static void		 display_random_fish();
static void		 create_clock(double x, double y, int value);
static void		 update_clock(int value);
static gboolean		 animate_tux(gpointer data);
static void		 rotate_tux(GooCanvasItem *tuxitem, gint direction,
				    gdouble scale);
static void		 move_item_at(GooCanvasItem *item,
				      int block, double ratio);

static int number_of_item   = 0;
static int number_of_item_x = 0;
static int number_of_item_y = 0;

static int errors           = 0;
static int number_of_dices  = 0;
static int max_dice_number  = 0;
static int number_of_fish   = 0;

static int tux_index	    = 0;
static int tux_destination  = 0;
static int fish_index	    = 0;
static int animate_speed    = 0;

#define ANIMATE_SPEED	800

static gdouble tux_ratio = 0;

static int dicevalue_array[10];
static GooCanvasItem *fishItem;
static GooCanvasItem *tuxItem;
static GooCanvasItem *tuxRootItem;
static GooCanvasItem *clock_image_item;

// List of images to use in the game
static gchar *imageList[] =
{
  "reversecount/baleine.svgz",
  "reversecount/phoque.svgz",
  "reversecount/ourspolaire.svgz",
  "reversecount/morse.svgz",
  "reversecount/elephant_mer.svgz",
  "reversecount/epaulard.svgz",
  "reversecount/narval.svgz",
};
#define NUMBER_OF_IMAGES 10

// List of fish to use in the game
static gchar *fishList[] =
{
  "reversecount/blueking2_0.png",
  "reversecount/butfish_0.png",
  "reversecount/cichlid1_0.png",
  "reversecount/cichlid4_0.png",
  "reversecount/collaris_0.png",
  "reversecount/discus2_0.png",
  "reversecount/discus3_0.png",
  "reversecount/eel_0.png",
  "reversecount/f00_0.png",
  "reversecount/f01_0.png",
  "reversecount/f02_0.png",
  "reversecount/f03_0.png",
  "reversecount/f04_0.png",
  "reversecount/f05_0.png",
  "reversecount/f06_0.png",
  "reversecount/f07_0.png",
  "reversecount/f08_0.png",
  "reversecount/f09_0.png",
  "reversecount/f10_0.png",
  "reversecount/f11_0.png",
  "reversecount/f12_0.png",
  "reversecount/f13_0.png",
  "reversecount/manta_0.png",
  "reversecount/newf1_0.png",
  "reversecount/QueenAngel_0.png",
  "reversecount/shark1_0.png",
  "reversecount/six_barred_0.png",
  "reversecount/teeth_0.png"
};
#define NUMBER_OF_FISHES 27

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Reverse count",
    "Practice substraction with a funny game",
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

GET_BPLUGIN_INFO(reversecount)

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
  else if(gamewon == FALSE && pause == FALSE) /* the game is lost */
    reversecount_next_level();

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
      gcomprisBoard->maxlevel=7;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gc_bar_set(GC_BAR_LEVEL);
      gc_bar_location(10, -1, 0.7);

      reversecount_next_level();

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
      reversecount_destroy_all_items();
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
      reversecount_next_level();
    }
}

/* ======================================= */
gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "reversecount")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

/* ======================================= */
gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
{

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
    }

  return TRUE;
}


static void process_error()
{
  gc_sound_play_ogg ("sounds/crash.wav", NULL);
  errors--;
  if(errors==0)
    {
      gamewon = FALSE;
      reversecount_destroy_all_items();
      gc_bonus_display(gamewon, GC_BONUS_SMILEY);
    }
  else
    {
      update_clock(errors);
    }
}

/* ======================================= */
static void process_ok()
{
  guint i;

  tux_destination = tux_index;

  for(i=0; i<number_of_dices; i++)
      tux_destination += dicevalue_array[i];

  // Wrapping
  if(tux_destination >= number_of_item)
    tux_destination = tux_destination - (number_of_item);

  // Do not allow going at a position after the fish
  if((tux_destination > fish_index)
     || (tux_destination == tux_index))
    {
      process_error();
      return;
    }

  if(!animate_id) {
    animate_id = g_timeout_add (animate_speed,
				animate_tux, NULL);
  }

}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

/* set initial values for the next level */
static void reversecount_next_level()
{

  gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
		    imageList[gcomprisBoard->level-1]);

  reversecount_destroy_all_items();
  gamewon = FALSE;

  /* Select level difficulty */
  switch(gcomprisBoard->level)
    {
    case 1:
      number_of_item_x = 5;
      number_of_item_y = 5;
      number_of_dices = 1;
      max_dice_number = 3;
      number_of_fish = 3;
      break;
    case 2:
      number_of_item_x = 5;
      number_of_item_y = 5;
      number_of_dices = 1;
      max_dice_number = 6;
      number_of_fish = 6;
      break;
    case 3:
      number_of_item_x = 6;
      number_of_item_y = 6;
      number_of_dices = 1;
      max_dice_number = 9;
      number_of_fish = 6;
      break;
    case 4:
      number_of_item_x = 8;
      number_of_item_y = 6;
      number_of_dices = 1;
      max_dice_number = 3;
      number_of_fish = 6;
      break;
    case 5:
      number_of_item_x = 8;
      number_of_item_y = 6;
      number_of_dices = 2;
      max_dice_number = 6;
      number_of_fish = 10;
      break;
    case 6:
      number_of_item_x = 8;
      number_of_item_y = 8;
      number_of_dices = 2;
      max_dice_number = 9;
      number_of_fish = 10;
      break;
    default:
      number_of_item_x = 10;
      number_of_item_y = 10;
      number_of_dices = 3;
      max_dice_number = 9;
      number_of_fish = 10;
      break;
    }

  animate_speed = ANIMATE_SPEED - gcomprisBoard->level * 60;

  number_of_item = number_of_item_x * 2 + (number_of_item_y - 2) * 2;

  /* Try the next level */
  reversecount_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));

  gc_bar_set_level(gcomprisBoard);

}
/* ==================================== */
/* Destroy all the items */
static void reversecount_destroy_all_items()
{

  gc_timer_end();

  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}
/* ==================================== */
static GooCanvasItem *reversecount_create_item(GooCanvasItem *parent)
{
  gint i, j, d;
  GooCanvasItem *item = NULL;
  gdouble block_width, block_height;
  gdouble dice_area_x;
  gdouble xratio, yratio;
  GcomprisProperties	*properties = gc_prop_get();
  RsvgHandle *svg_handle;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  block_width =  BOARDWIDTH/number_of_item_x;
  block_height = (BOARDHEIGHT-BARHEIGHT)/number_of_item_y;

  /* Timer is not requested */
  if(properties->timer > 0)
    {
      errors = number_of_dices + 4 - (MIN(properties->timer, 4));
      create_clock(BOARDWIDTH - block_width - 100,
		   BOARDHEIGHT - block_height - 100 - BARHEIGHT,
		   errors) ;
    }
  else
    {
      errors = -1;
    }

  // Ice blocks
  svg_handle = gc_rsvg_load("reversecount/iceblock.svgz");
  RsvgDimensionData rsvg_dimension;
  rsvg_handle_get_dimensions (svg_handle, &rsvg_dimension);

  xratio = block_width / rsvg_dimension.width;
  yratio =  block_height / rsvg_dimension.height;

  for(i=0; i<BOARDWIDTH; i+=block_width)
    {
      j=0;
      item = goo_canvas_svg_new (boardRootItem, svg_handle, NULL);
      goo_canvas_item_translate(item, i, j);
      goo_canvas_item_scale(item, xratio, yratio);

      j=BOARDHEIGHT-BARHEIGHT-block_height;
      item = goo_canvas_svg_new (boardRootItem, svg_handle, NULL);
      goo_canvas_item_translate(item, i, j);
      goo_canvas_item_scale(item, xratio, yratio);
    }

  for(j = block_height; j<=BOARDHEIGHT - (block_height*2) - BARHEIGHT;
      j += block_height)
    {
      i = 0;
      item = goo_canvas_svg_new (boardRootItem, svg_handle, NULL);
      goo_canvas_item_translate(item, i, j);
      goo_canvas_item_scale(item, xratio, yratio);


      i = BOARDWIDTH - block_width;
      item = goo_canvas_svg_new (boardRootItem, svg_handle, NULL);
      goo_canvas_item_translate(item, i, j);
      goo_canvas_item_scale(item, xratio, yratio);
    }

  g_object_unref(svg_handle);


  //----------------------------------------
  // Create the dice area
  svg_handle = gc_rsvg_load("reversecount/dice_area.svgz");
  rsvg_handle_get_dimensions (svg_handle, &rsvg_dimension);

  dice_area_x = BOARDWIDTH - block_width - rsvg_dimension.width - 20;

  item = goo_canvas_svg_new (boardRootItem, svg_handle, NULL);
  goo_canvas_item_translate(item,
			    dice_area_x, block_height + 20);

  g_object_unref(svg_handle);

  //----------------------------------------
  // Create the dices
  svg_handle = gc_rsvg_load("reversecount/dice1.svgz");
  rsvg_handle_get_dimensions (svg_handle, &rsvg_dimension);
  guint dice_width = 78;

  for(d=0; d<number_of_dices; d++)
    {
      int *val;

      i = dice_area_x + dice_width * d + 20;
      j = block_height + 24 + d*7;

      item = goo_canvas_svg_new (boardRootItem, svg_handle, NULL);
      goo_canvas_item_translate(item,
				i, j);

      xratio = (gdouble)dice_width / rsvg_dimension.width;
      goo_canvas_item_scale(item, xratio, xratio);

      dicevalue_array[d] = 1;
      val = g_new(gint, 1);
      *val = d;

      g_signal_connect(item, "button_press_event",
		       (GCallback) item_event,
		       val);
      gc_item_focus_init(item, NULL);
    }
  g_object_unref(svg_handle);

  // OK Button (Validation)
  item = goo_canvas_svg_new (boardRootItem,
			     gc_skin_rsvg_get(),
			     "svg-id", "#OK",
			     NULL);
  SET_ITEM_LOCATION(item,
		    dice_area_x - 60,
		    block_height + 20);
  g_signal_connect(item, "button_press_event",
		   (GCallback) process_ok, NULL);
  gc_item_focus_init(item, NULL);

  // Tux
  tuxRootItem = goo_canvas_group_new (boardRootItem, NULL);

  svg_handle = gc_rsvg_load("reversecount/tux_top_south.svgz");
  tuxItem = goo_canvas_svg_new (tuxRootItem, svg_handle, NULL);

  RsvgDimensionData dimension;
  rsvg_handle_get_dimensions(svg_handle, &dimension);

  /* Calc the tux best ratio to display it */
  xratio =  block_width  / (dimension.width + TUX_TO_BORDER_GAP);
  yratio =  block_height / (dimension.height + TUX_TO_BORDER_GAP);
  tux_ratio = yratio = MIN(xratio, yratio);

  goo_canvas_item_scale(tuxItem, tux_ratio, tux_ratio);

  goo_canvas_item_translate(tuxItem, (BOARDWIDTH - dimension.width)/2,
			    (BOARDHEIGHT - dimension.height)/2);
  rotate_tux(tuxItem, EAST, tux_ratio);
  g_object_unref (svg_handle);

  tux_index = 0;

  // Display the first fish
  display_random_fish();

  return NULL;
}

static void display_random_fish()
{

  fish_index = tux_index +
    g_random_int()%(max_dice_number*number_of_dices) + 1;

  // Wrapping
  if(fish_index >= number_of_item)
    fish_index = fish_index - (number_of_item);

  fishItem = display_item_at(fishList[g_random_int()%NUMBER_OF_FISHES],
			     fish_index);
}

/* ==================================== */
/**
 * Display given imagename on the given ice block.
 */
static GooCanvasItem*
display_item_at(gchar *imagename, int block)
{
  double block_width, block_height;
  double xratio, yratio;
  GooCanvasItem *item;
  GdkPixbuf *pixmap;
  GdkPixbuf *pixmap2;
  int i,j;

  block_width  = BOARDWIDTH/number_of_item_x;
  block_height = (BOARDHEIGHT-BARHEIGHT)/number_of_item_y;

  if(block < number_of_item_x)
    {
      // Upper line
      g_warning("      // Upper line\n");
      i = block_width * block;
      j = 0;
    }
  else if(block < number_of_item_x + number_of_item_y - 2)
    {
      // Right line
      g_warning("      // Right line\n");
      i = block_width * (number_of_item_x - 1);
      j = block_height * (block - (number_of_item_x-1));
    }
  else if(block < number_of_item_x*2 + number_of_item_y - 2)
    {
      // Bottom line
      g_warning("      // Bottom line\n");
      i = block_width * (number_of_item_x - (block-
					     (number_of_item_x+number_of_item_y-1))-2);
      j = block_height * (number_of_item_y-1);
    }
  else
    {
      // Left line
      g_warning("      // Left line\n");
      i = 0;
      j = block_height * (number_of_item_y - (block - (number_of_item_x*2 +
						      number_of_item_y-4)));
    }

  g_warning("display_tux %d i=%d j=%d\n", block, i, j);

  /* Calculation to thrink the item while keeping the ratio */
  pixmap = gc_pixmap_load(imagename);
  xratio =  block_width  / (gdk_pixbuf_get_width (pixmap) + TUX_TO_BORDER_GAP);
  yratio =  block_height / (gdk_pixbuf_get_height(pixmap) + TUX_TO_BORDER_GAP);
  xratio = yratio = MIN(xratio, yratio);

  pixmap2 = gdk_pixbuf_scale_simple(pixmap,
				    gdk_pixbuf_get_width (pixmap) * xratio,
				    gdk_pixbuf_get_height(pixmap) * xratio,
				    GDK_INTERP_BILINEAR);
  gdk_pixbuf_unref(pixmap);

  item = goo_canvas_image_new (boardRootItem,
			       pixmap2,
			       i + (block_width -
				    (gdk_pixbuf_get_width (pixmap2))) / 2,
			       j + (block_height -
				    (gdk_pixbuf_get_height (pixmap2))) / 2,
			       NULL);

  gdk_pixbuf_unref(pixmap2);

  return(item);
}

/**
 * Move given GooCanvasItem on the given ice block.
 */
static void
move_item_at(GooCanvasItem *item, int block, double ratio)
{
  double block_width, block_height;
  int i,j;
  GooCanvasBounds bounds;

  block_width  = BOARDWIDTH/number_of_item_x;
  block_height = (BOARDHEIGHT-BARHEIGHT)/number_of_item_y;

  if(block < number_of_item_x)
    {
      // Upper line
      g_warning("      // Upper line\n");
      i = block_width * block;
      j = 0;
    }
  else if(block < number_of_item_x + number_of_item_y - 2)
    {
      // Right line
      g_warning("      // Right line\n");
      i = block_width * (number_of_item_x - 1);
      j = block_height * (block - (number_of_item_x-1));
    }
  else if(block < number_of_item_x*2 + number_of_item_y - 2)
    {
      // Bottom line
      g_warning("      // Bottom line\n");
      i = block_width * (number_of_item_x - (block-
					     (number_of_item_x+number_of_item_y-1))-2);
      j = block_height * (number_of_item_y-1);
    }
  else
    {
      // Left line
      g_warning("      // Left line\n");
      i = 0;
      j = block_height * (number_of_item_y - (block - (number_of_item_x*2 +
						      number_of_item_y-4)));
    }

  g_warning("move_item_at %d i=%d j=%d\n", block, i, j);

  goo_canvas_item_get_bounds(item, &bounds);

  goo_canvas_item_animate(item,
			  i,
			  j,
			  1.0,
			  0,
			  TRUE,
			  animate_speed,
			  40,
			  GOO_CANVAS_ANIMATE_FREEZE);

}

/* ==================================== */
static void game_won()
{
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level> gcomprisBoard->maxlevel) { // the current board is finished : bail out
      gcomprisBoard->level = gcomprisBoard->maxlevel;
      return;
    }
    gc_sound_play_ogg ("sounds/bonus.wav", NULL);
  }
  reversecount_next_level();
}

/* ==================================== */
/**
 * Increment the dices when they are clicked
 */

static gboolean
item_event (GooCanvasItem  *item,
	    GooCanvasItem  *target,
	    GdkEventButton *event,
	    gint *dice_index)
{
  gchar *str;
  RsvgHandle *rsvg_handle;
  gint i = *dice_index;

  if(board_paused)
    return FALSE;

  switch(event->button)
    {
    case 1:
      if(dicevalue_array[i]++ >= max_dice_number)
	dicevalue_array[i] = (number_of_dices==1 ? 1 : 0);
      break;
    case 2:
    case 3:
      if(dicevalue_array[i]-- == (number_of_dices==1 ? 1 : 0))
	dicevalue_array[i] = max_dice_number;
      break;
    default:
      break;
    }

  str = g_strdup_printf("reversecount/dice%d.svgz", dicevalue_array[i]);
  rsvg_handle = gc_rsvg_load(str);

  g_object_set (item,
		"svg-handle", rsvg_handle,
		NULL);
  gc_item_focus_init(item, NULL);
  g_object_unref(rsvg_handle);

  g_free(str);

  return FALSE;
}

/*
 * Clock management
 */
static void create_clock(double x, double y, int value)
{
  GdkPixbuf   *pixmap = NULL;
  char	      *str = NULL;

  if(value<0)
    return;

  str = g_strdup_printf("%s%d.png", "timers/clock",value);

  pixmap = gc_skin_pixmap_load(str);

  clock_image_item = goo_canvas_image_new (boardRootItem,
					   pixmap,
					   x,
					   y,
					   NULL);

  gdk_pixbuf_unref(pixmap);
  g_free(str);
}

static void update_clock(int value)
{
  GdkPixbuf   *pixmap = NULL;
  char        *str = NULL;

  if(value<0)
    return;

  str = g_strdup_printf("%s%d.png", "timers/clock",value);

  pixmap = gc_skin_pixmap_load(str);

  g_object_set (clock_image_item,
		"pixbuf", pixmap,
		NULL);

  gdk_pixbuf_unref(pixmap);
  g_free(str);
}

static gboolean animate_tux(gpointer data)
{
  // Move tux
  tux_index++;

  move_item_at(tuxRootItem, tux_index, tux_ratio);

  g_warning("=========== tux_index=%d tux_destination=%d fish_index=%d\n",
	    tux_index, tux_destination, fish_index);

  // Wrapping
  if(tux_index >= number_of_item)
    tux_index = tux_index - (number_of_item);

  /* Calculate which tux should be displayed */
  if(tux_index<number_of_item_x-1)
    rotate_tux(tuxItem, EAST, tux_ratio);
  else if(tux_index<number_of_item_x+number_of_item_y-2)
    rotate_tux(tuxItem, SOUTH, tux_ratio);
  else if(tux_index<2*number_of_item_x+number_of_item_y-3)
    rotate_tux(tuxItem, WEST, tux_ratio);
  else
    rotate_tux(tuxItem, NORTH, tux_ratio);

  /* Rearm the timer to go to the next spot */
  if(tux_index != tux_destination)
    {
      animate_id = g_timeout_add (animate_speed,
				  animate_tux, NULL);
    }
  else
    {
      animate_id = 0;

      if(tux_destination != fish_index)
	{
	  process_error();
	}
      else
	{
	  // Remove the fish
	  if(fishItem!=NULL)
	    goo_canvas_item_remove(fishItem);

	  gc_sound_play_ogg ("sounds/gobble.wav", NULL);

	  if(--number_of_fish == 0)
	    {
	      gamewon = TRUE;
	      reversecount_destroy_all_items();
	      gc_bonus_display(gamewon, GC_BONUS_SMILEY);
	    }
	  else
	    {
	      display_random_fish();
	    }
	}
    }

  return(FALSE);
}

static void
rotate_tux(GooCanvasItem *tuxitem, gint direction,
	   gdouble scale)
{
  gint rotation = 0;
  GooCanvasBounds bounds;

  /* Our svg image of tux is faced south */
  switch(direction)
    {
    case EAST:
      rotation = -90;
      break;
    case WEST:
      rotation = 90;
      break;
    case NORTH:
      rotation = 180;
      break;
    case SOUTH:
      rotation = 0;
      break;
    }

  goo_canvas_item_set_transform(tuxitem, NULL);

  goo_canvas_item_get_bounds(tuxitem, &bounds);

  goo_canvas_item_scale(tuxitem, scale, scale);

  goo_canvas_item_rotate(tuxitem, rotation,
			 (bounds.x2-bounds.x1)/2,
			 (bounds.y2-bounds.y1)/2);

}
