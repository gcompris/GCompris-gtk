/* gcompris - reversecount.c
 *
 * Copyright (C) 2002 Bruno Coudoin
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

GcomprisBoard *gcomprisBoard = NULL;
gboolean board_paused = TRUE;

static void	 start_board (GcomprisBoard *agcomprisBoard);
gint		 key_press(guint keyval);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

#define TUX_IMG "gcompris/misc/tux.png"
#define TUX_TO_BORDER_GAP 10

static GnomeCanvasGroup *boardRootItem = NULL;

static void		 process_ok(void);
static GnomeCanvasItem	*reversecount_create_item(GnomeCanvasGroup *parent);
static void		 reversecount_destroy_all_items(void);
static void		 reversecount_next_level(void);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, gint *dice_index);
static GnomeCanvasItem	*display_item_at(gchar *imagename, int block);
static void		 display_random_fish();

static int number_of_item = 0;
static int number_of_item_x = 0;
static int number_of_item_y = 0;

static int number_of_dices = 0;
static int max_dice_number = 0;
static int number_of_fish = 0;

static int tux_index = 0;
static int fish_index = 0;

static int dicevalue_array[10];
static GnomeCanvasItem *fishItem;
static GnomeCanvasItem *tuxItem;

// List of images to use in the game
static gchar *imageList[] =
{
  "gcompris/animals/bear001.jpg",
  "gcompris/animals/malaybear002.jpg",
  "gcompris/animals/polabear011.jpg",
  "gcompris/animals/spectbear001.jpg",
  "gcompris/animals/joybear001.jpg",
  "gcompris/animals/polarbear001.jpg",
  "gcompris/animals/joybear002.jpg",
  "gcompris/animals/poolbears001.jpg"
};
#define NUMBER_OF_IMAGES 8

// List of fish to use in the game
static gchar *fishList[] =
{
  "fishes/blueking2_0.png",
  "fishes/butfish_0.png",
  "fishes/cichlid1_0.png",
  "fishes/cichlid4_0.png",
  "fishes/collaris_0.png",
  "fishes/discus2_0.png",
  "fishes/discus3_0.png",
  "fishes/eel_0.png",
  "fishes/f00_0.png",
  "fishes/f01_0.png",
  "fishes/f02_0.png",
  "fishes/f03_0.png",
  "fishes/f04_0.png",
  "fishes/f05_0.png",
  "fishes/f06_0.png",
  "fishes/f07_0.png",
  "fishes/f08_0.png",
  "fishes/f09_0.png",
  "fishes/f10_0.png",
  "fishes/f11_0.png",
  "fishes/f12_0.png",
  "fishes/f13_0.png",
  "fishes/manta_0.png",
  "fishes/newf1_0.png",
  "fishes/QueenAngel_0.png",
  "fishes/shark1_0.png",
  "fishes/six_barred_0.png",
  "fishes/teeth_0.png"
};
#define NUMBER_OF_FISHES 27

/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Reverse count"),
    N_("Practice the substraction with a funny game"),
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
      gcomprisBoard->maxlevel=7;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL|GCOMPRIS_BAR_OK);

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
gint key_press(guint keyval)
{

  if(!gcomprisBoard)
    return TRUE;

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


/* ======================================= */
static void process_ok()
{
  guint i;

  for(i=0; i<number_of_dices; i++)
      tux_index += dicevalue_array[i];

  // Wrapping
  if(tux_index >= number_of_item)
    tux_index = tux_index - (number_of_item);

  // Move tux
  if(tuxItem!=NULL)
    gtk_object_destroy(GTK_OBJECT(tuxItem));

  tuxItem = display_item_at(TUX_IMG, tux_index);

  if(tux_index == fish_index)
    {
      // Remove the fish
      if(fishItem!=NULL)
	gtk_object_destroy(GTK_OBJECT(fishItem));

      gcompris_play_sound (SOUNDLISTFILE, "gobble");
      
      if(--number_of_fish == 0)
	{
	  gamewon = TRUE;
	  reversecount_destroy_all_items();
	  gcompris_display_bonus(gamewon, BONUS_SMILEY);
	}
      else
	{
	  display_random_fish();
	}
    }
  else
    {
      gcompris_play_sound (SOUNDLISTFILE, "crash");
    }
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* set initial values for the next level */
static void reversecount_next_level()
{

  gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			  imageList[RAND(0, NUMBER_OF_IMAGES-1)]);

  gcompris_bar_set_level(gcomprisBoard);

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

  number_of_item = number_of_item_x * 2 + (number_of_item_y - 2) * 2;

  /* Try the next level */
  reversecount_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void reversecount_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
static GnomeCanvasItem *reversecount_create_item(GnomeCanvasGroup *parent)
{
  int i,j,d;
  GnomeCanvasItem *item = NULL;
  GdkPixbuf   *pixmap = NULL;
  double block_width, block_height;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,

							    NULL));

  block_width =  BOARDWIDTH/number_of_item_x;
  block_height = BOARDHEIGHT/number_of_item_y;

  pixmap = gcompris_load_pixmap("reversecount/iceblock.png");

  for(i=0; i<BOARDWIDTH; i+=block_width)
    {
      j=0;
      item = gnome_canvas_item_new (boardRootItem,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) i,
				    "y", (double) j,
				    "width", (double) block_width,
				    "height", (double)  block_height,
				    "width_set", TRUE, 
				    "height_set", TRUE,
					NULL);

      j=BOARDHEIGHT-block_height;
      item = gnome_canvas_item_new (boardRootItem,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) i,
				    "y", (double) j,
				    "width", (double) block_width,
				    "height", (double)  block_height,
				    "width_set", TRUE, 
				    "height_set", TRUE,
					NULL);
    }

  for(j=block_height; j<=BOARDHEIGHT-(block_height*2); j+=block_height)
    {
      i = 0;
      item = gnome_canvas_item_new (boardRootItem,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) i,
				    "y", (double) j,
				    "width", (double) block_width,
				    "height", (double)  block_height,
				    "width_set", TRUE, 
				    "height_set", TRUE,
					NULL);

      i = BOARDWIDTH - block_width;
      item = gnome_canvas_item_new (boardRootItem,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) i,
				    "y", (double) j,
				    "width", (double) block_width,
				    "height", (double)  block_height,
				    "width_set", TRUE, 
				    "height_set", TRUE,
					NULL);
    }

  gdk_pixbuf_unref(pixmap);

  //----------------------------------------
  // Create the dices
  pixmap = gcompris_load_pixmap("gcompris/dice/gnome-dice1.png");

  for(d=0; d<number_of_dices; d++)
    {
      int *val;

      i = BOARDWIDTH  * 0.4 + gdk_pixbuf_get_width(pixmap) * d + 20;
      j = BOARDHEIGHT * 0.6;

      item = gnome_canvas_item_new (boardRootItem,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) i,
				    "y", (double) j,
				    NULL);
      dicevalue_array[d] = 1;
      val = g_new(gint, 1);
      *val = d;
      gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event, 
			 val);
      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) gcompris_item_event_focus,
			 NULL);

    }
  gdk_pixbuf_unref(pixmap);

  tux_index = 0;
  tuxItem = display_item_at(TUX_IMG, tux_index);

  // Display the first fish
  display_random_fish();

  return NULL;
}

static void display_random_fish()
{

  fish_index = tux_index + 
    rand()%(max_dice_number*number_of_dices) + 
    number_of_dices;

  // Wrapping
  if(fish_index >= number_of_item)
    fish_index = fish_index - (number_of_item);

  fishItem = display_item_at(fishList[rand()%NUMBER_OF_FISHES],
			     fish_index);
}

/* ==================================== */
/**
 * Display given imagename on the given ice block.
 */
static GnomeCanvasItem *display_item_at(gchar *imagename, int block)
{
  double block_width, block_height;
  double xratio, yratio;
  GnomeCanvasItem *item = NULL;
  GdkPixbuf   *pixmap = NULL;
  int i,j;
  //  GdkFont *gdk_font;

  block_width  = BOARDWIDTH/number_of_item_x;
  block_height = BOARDHEIGHT/number_of_item_y;

  pixmap = gcompris_load_pixmap(imagename);
  //  gdk_font = gdk_font_load (FONT_BOARD_MEDIUM);

  if(block < number_of_item_x)
    {
      // Upper line
      printf("      // Upper line\n");
      i = block_width * block;
      j = 0;
    } 
  else if(block < number_of_item_x + number_of_item_y - 2)
    {
      // Right line
      printf("      // Right line\n");
      i = block_width * (number_of_item_x - 1);
      j = block_height * (block - (number_of_item_x-1));
    }
  else if(block < number_of_item_x*2 + number_of_item_y - 2)
    {
      // Bottom line
      printf("      // Bottom line\n");
      i = block_width * (number_of_item_x - (block-
					     (number_of_item_x+number_of_item_y-1))-2);
      j = block_height * (number_of_item_y-1);
    }
  else
    {
      // Left line
      printf("      // Left line\n");
      i = 0;
      j = block_height * (number_of_item_y - (block - (number_of_item_x*2 + 
						      number_of_item_y-4)));
    }

  printf("display_tux %d i=%d j=%d\n", block, i, j);

  /* Calculation to thrink the item while keeping the ratio */
  xratio =  block_width  / (gdk_pixbuf_get_width (pixmap) + TUX_TO_BORDER_GAP);
  yratio =  block_height / (gdk_pixbuf_get_height(pixmap) + TUX_TO_BORDER_GAP);
  xratio = yratio = MIN(xratio, yratio);

  item = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) i + (block_width -
						   (gdk_pixbuf_get_width (pixmap) * xratio)) / 2,
				"y", (double) j + (block_height -
						   (gdk_pixbuf_get_height (pixmap) * yratio)) / 2,
				"width", (double) gdk_pixbuf_get_width (pixmap) * xratio,
				"height", (double)  gdk_pixbuf_get_height (pixmap) * yratio,
				"width_set", TRUE, 
				"height_set", TRUE,
				NULL);

  /*
  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_text_get_type (),
			 "text", g_strdup_printf("%d", (guint)block),
			 "font_gdk", gdk_font,
			 "x", (double) i+block_width/2,
			 "y", (double) j+block_height/2,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color", "red",
			 NULL);
  */

  gdk_pixbuf_unref(pixmap);

  return(item);
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
  reversecount_next_level();
}

/* ==================================== */
/**
 * Increment the dices when they are clicked
 */

static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gint *dice_index)
{
  gchar *str;
  GdkPixbuf   *pixmap = NULL;
  gint i = *dice_index;

  if(board_paused)
    return FALSE;

  switch (event->type) 
    {
    case GDK_BUTTON_PRESS:
      switch(event->button.button) 
	{
	case 1:
	  if(dicevalue_array[i]++ >= max_dice_number)
	    dicevalue_array[i] = 1;
	  break;
	case 2:
	case 3:
	  if(dicevalue_array[i]-- == 1)
	    dicevalue_array[i] = max_dice_number;
	  break;
	  break;
	default:
	  break;
	}

      str = g_strdup_printf("gcompris/dice/gnome-dice%d.png", dicevalue_array[i]);
      
      pixmap = gcompris_load_pixmap(str);
      
      gnome_canvas_item_set(item,
			    "pixbuf",pixmap,
			    NULL);
      
      gdk_pixbuf_unref(pixmap);
      g_free(str);
      break;

    default:
      break;
    }

  return FALSE;
}
