/* gcompris - money.c
 *
 * Copyright (C) 2001, 2010 Bruno Coudoin
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
#include "money_widget.h"

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static GooCanvasItem *boardRootItem = NULL;

static void	 money_destroy_all_items(void);
static void	 money_next_level(void);
void		 moneyactivity_process_ok(void);
static void	 display_paying_tux(guint note);

Money_Widget    *tux_money    = NULL;
Money_Widget    *seller_money = NULL;

static float	 price_target = 0;

typedef struct {
  char *image;
  guint status;
  float value;
  GooCanvasItem *item;
} MoneyItem;

// We create 3 prices categories to make the game more realistic.
// List of images to use in the game (cheap items)
static gchar *imageList1[] =
{
  "money/apple.svgz",
  "money/orange.svgz",
  "money/banane.svgz",
  "money/pamplemousse.svgz",
  "money/carot.svgz",
  "money/cerise.svgz",
  "money/cake.svgz",
};
#define NUMBER_OF_IMAGES1 G_N_ELEMENTS(imageList1)

// List of images to use in the game (middle price items)
static gchar *imageList2[] =
{
  "money/umbrella.svgz",
  "money/pencil.svgz",
  "money/bottle.svgz",
  "money/light.svgz",
  "money/eggpot.svgz",
};
#define NUMBER_OF_IMAGES2 G_N_ELEMENTS(imageList2)

// List of images to use in the game (expensive items)
static gchar *imageList3[] =
{
  "money/lamp.svgz",
  "money/football.svgz",
  "money/bicycle.svgz",
  "money/crown.svgz",
};
#define NUMBER_OF_IMAGES3 G_N_ELEMENTS(imageList3)

typedef enum
{
  WITHOUT_CENTS,
  WITH_CENTS,
  BACK_WITHOUT_CENTS,
  BACK_WITH_CENTS
} type;

static char currentMode = WITHOUT_CENTS;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Money",
    "Practise money usage",
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

GET_BPLUGIN_INFO(money)

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
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=10; /* Go to next level after this number of 'play' */
      gc_bar_set(GC_BAR_LEVEL);
      gc_bar_location(0, -1, 0.6);

      /* Default mode */
      if(!gcomprisBoard->mode)
	{
	  gcomprisBoard->maxlevel=9;
	  currentMode=WITHOUT_CENTS;
	}
      else if(g_strcmp0(gcomprisBoard->mode, "WITH_CENTS")==0)
	{
	  gcomprisBoard->maxlevel=5;
	  currentMode=WITH_CENTS;
	}
      else if(g_strcmp0(gcomprisBoard->mode, "BACK_WITHOUT_CENTS")==0)
	{
	  gcomprisBoard->maxlevel=9;
	  currentMode=BACK_WITHOUT_CENTS;
	}
      else if(g_strcmp0(gcomprisBoard->mode, "BACK_WITH_CENTS")==0)
	{
	  gcomprisBoard->maxlevel=5;
	  currentMode=BACK_WITH_CENTS;
	}
      money_next_level();

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
      money_destroy_all_items();
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
      money_next_level();
    }
}
/* ======================================= */
gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "money")==0)
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
static void money_next_level()
{
  guint		   min_price = 0, max_price = 0;
  guint		   paid = 0;
  guint		   number_of_item = 0;
  guint		   i;
  gchar		  *display_format;

  gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
		    "money/money-bg.png");

  gc_bar_set_level(gcomprisBoard);

  money_destroy_all_items();
  gamewon = FALSE;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  /* The OK Button */
  GooCanvasItem *item = goo_canvas_svg_new( boardRootItem,
					    gc_skin_rsvg_get(),
					    "svg-id", "#OK",
					    NULL);
  SET_ITEM_LOCATION(item, 725, 230);
  g_signal_connect(item, "button_press_event",
		   (GtkSignalFunc) moneyactivity_process_ok, NULL);
  gc_item_focus_init(item, NULL);

  tux_money = MONEY_WIDGET(money_widget_new());
  money_widget_set_position(tux_money,
			    boardRootItem,
			    100.0, 380.0,
			    700.0, 490.0,
			    5, 2,
			    FALSE);

  /* Select level difficulty */
  switch(currentMode)
    {
    case WITHOUT_CENTS:
      switch(gcomprisBoard->level)
	{
	case 1:
	  number_of_item = 1;
	  min_price      = 3;
	  max_price      = 10;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 2:
	  number_of_item = 1;
	  min_price      = 10;
	  max_price      = 20;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 3:
	  number_of_item = 2;
	  min_price      = 20;
	  max_price      = 30;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 4:
	  number_of_item = 2;
	  min_price      = 30;
	  max_price      = 40;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 5:
	  number_of_item = 3;
	  min_price      = 40;
	  max_price      = 50;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_20E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 6:
	  number_of_item = 3;
	  min_price      = 50;
	  max_price      = 60;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_20E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_20E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 7:
	  number_of_item = 4;
	  min_price      = 60;
	  max_price      = 70;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_50E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_20E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 8:
	  number_of_item = 4;
	  min_price      = 70;
	  max_price      = 80;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_50E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_20E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 9:
	  number_of_item = 4;
	  min_price      = 50;
	  max_price      = 100;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_50E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_20E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  break;
	}
      break;
    case WITH_CENTS:
      switch(gcomprisBoard->level)
	{
	case 1:
	  number_of_item = 1;
	  min_price      = 1;
	  max_price      = 3;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_5C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_50C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_10C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  break;
	case 2:
	  number_of_item = 1;
	  min_price      = 1;
	  max_price      = 3;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_5C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_50C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_10C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  break;
	case 3:
	  number_of_item = 2;
	  min_price      = 1;
	  max_price      = 3;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_5C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_50C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_10C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  break;
	case 4:
	  number_of_item = 3;
	  min_price      = 1;
	  max_price      = 3;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_5C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_50C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_10C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  break;
	case 5:
	  number_of_item = 4;
	  min_price      = 0;
	  max_price      = 4;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_5C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_50C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_10C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  break;
	}
      break;
    case BACK_WITHOUT_CENTS:
      switch(gcomprisBoard->level)
	{
	case 1:
	  number_of_item = 1;
	  min_price      = 3;
	  max_price      = 9;
	  paid		 = 10;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 2:
	  number_of_item = 1;
	  min_price      = 11;
	  max_price      = 19;
	  paid		 = 20;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 3:
	  number_of_item = 2;
	  min_price      = 21;
	  max_price      = 29;
	  paid		 = 30;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 4:
	  number_of_item = 2;
	  min_price      = 30;
	  max_price      = 39;
	  paid		 = 40;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 5:
	  number_of_item = 3;
	  min_price      = 40;
	  max_price      = 49;
	  paid		 = 50;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_20E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 6:
	  number_of_item = 3;
	  min_price      = 50;
	  max_price      = 60;
	  paid		 = 100;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_20E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_20E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 7:
	  number_of_item = 4;
	  min_price      = 60;
	  max_price      = 70;
	  paid		 = 100;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_50E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_20E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 8:
	  number_of_item = 4;
	  min_price      = 70;
	  max_price      = 80;
	  paid		 = 100;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_50E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_20E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  break;
	case 9:
	  number_of_item = 4;
	  min_price      = 50;
	  max_price      = 99;
	  paid		 = 100;
	  money_widget_add(tux_money, MONEY_EURO_PAPER_10E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_50E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_20E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  money_widget_add(tux_money, MONEY_EURO_PAPER_5E);
	  break;
	}
      break;
    case BACK_WITH_CENTS:
      switch(gcomprisBoard->level)
	{
	case 1:
	  number_of_item = 1;
	  min_price      = 1;
	  max_price      = 3;
	  paid		 = 5;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_5C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_50C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_10C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  break;
	case 2:
	  number_of_item = 1;
	  min_price      = 1;
	  max_price      = 3;
	  paid		 = 5;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_5C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_50C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_10C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  break;
	case 3:
	  number_of_item = 2;
	  min_price      = 1;
	  max_price      = 3;
	  paid		 = 5;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_5C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_50C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_10C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  break;
	case 4:
	  number_of_item = 3;
	  min_price      = 1;
	  max_price      = 3;
	  paid		 = 5;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_5C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_50C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_10C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  break;
	case 5:
	  number_of_item = 4;
	  min_price      = 0;
	  max_price      = 4;
	  paid		 = 5;
	  money_widget_add(tux_money, MONEY_EURO_COIN_2E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1E);
	  money_widget_add(tux_money, MONEY_EURO_COIN_5C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_2C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_50C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_20C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_10C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  money_widget_add(tux_money, MONEY_EURO_COIN_1C);
	  break;
	}
      break;
    }

  seller_money = MONEY_WIDGET(money_widget_new());
  money_widget_set_position(seller_money,
			    boardRootItem,
			    100.0,  20.0,
			    700.0, 130.0,
			    5, 2,
			    FALSE);

  money_widget_set_target(seller_money, tux_money);
  money_widget_set_target(tux_money, seller_money);

  /* Display what to buy */
  price_target = 0;
  for(i=1; i<=number_of_item; i++)
  {
    gdouble object_price;
    gchar *text;
    GooCanvasItem *item;
    RsvgHandle *svg_handle;
    RsvgDimensionData dimension;
    double xratio, yratio;
    guint offset_x = 50;
    guint boardwidth = BOARDWIDTH - offset_x * 2;

    /* Display the price */
    object_price  = (double) g_random_int_range(min_price/number_of_item,
						max_price/number_of_item);

    {
      /* Select an image list depending on the price */
      gchar **imageList;
      guint number_of_images;
      if ( object_price < 5 )
	{
	  imageList = imageList1;
	  number_of_images = NUMBER_OF_IMAGES1;
	}
      else if ( object_price < 10 )
	{
	  imageList = imageList2;
	  number_of_images = NUMBER_OF_IMAGES2;
	}
      else
	{
	  imageList = imageList3;
	  number_of_images = NUMBER_OF_IMAGES3;
	}

      svg_handle =							\
	gc_rsvg_load(imageList[g_random_int_range(0, number_of_images - 1)]);

      rsvg_handle_get_dimensions(svg_handle, &dimension);

      item = goo_canvas_svg_new ( boardRootItem,
				  svg_handle,
				  NULL);
    }
    xratio =  (gdouble)(boardwidth/(number_of_item+1)) / dimension.width;
    yratio =  100.0 / dimension.height;

    xratio = yratio = MIN(xratio, yratio);
    goo_canvas_item_translate(item,
			      offset_x +
			      (i*boardwidth)/(number_of_item+1)
    			      - dimension.width*xratio/2,
    			      200);

    goo_canvas_item_scale(item, xratio, xratio);

    if( (currentMode == WITH_CENTS) ||
	(currentMode == BACK_WITH_CENTS) )
      {
	/* Set here the way to display money. Change only the money sign, and it's place, always keep %.2f, it will be replaced by 0,34 if decimal is ',' in your locale */
	display_format = _("$ %.2f");
	/* Add random cents */
	if(gcomprisBoard->level == 1)
	  {
	    object_price += (double)((double) g_random_int_range(1, 9))/10.0;
	  }
	else
	  {
	    object_price += (double)((double) g_random_int_range(1, 99))/100.0;
	  }
      }
    else
      {
	/* Set here the way to display money. Change only the money sign, and it's place, always keep %.2f, it will be replaced by 0,34 if decimal is ',' in your locale */
	display_format = _("$ %.0f");
      }

    price_target += object_price;
    text = g_strdup_printf(display_format, object_price);
    goo_canvas_text_new(boardRootItem,
			text,
			offset_x + (i*boardwidth)/(number_of_item+1),
			185,
			-1,
			GTK_ANCHOR_CENTER,
			"font", gc_skin_font_board_big,
			"fill-color", "white",
			NULL);
    g_free(text);
    g_object_unref(svg_handle);
  }

  if (paid)
    {
      // Calc the money back instead of the objects price
      price_target = paid - price_target;
      // Display Tux and his money
      display_paying_tux(paid);
    }
}
/* ==================================== */
/* Destroy all the items */
static void money_destroy_all_items()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  if(tux_money!=NULL)
    gtk_object_destroy (GTK_OBJECT (tux_money));
  tux_money = NULL;

  if(seller_money!=NULL)
    gtk_object_destroy (GTK_OBJECT (seller_money));
  seller_money = NULL;

  boardRootItem = NULL;
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
  money_next_level();
}

/* ==================================== */
void moneyactivity_process_ok()
{

  if(board_paused)
    /*return FALSE*/;

  if(price_target >= money_widget_get_total(seller_money) - 0.001 &&
     price_target <= money_widget_get_total(seller_money) + 0.001 )
    {
      gamewon = TRUE;
      gc_bonus_display(gamewon, GC_BONUS_SMILEY);
    }
}

static void display_paying_note(guint note, guint x, guint y)
{
  gchar *note_str = g_strdup_printf("money/n%de.svgz", note);
  RsvgHandle *svg_handle;
  svg_handle = gc_rsvg_load(note_str);
  GooCanvasItem *item = goo_canvas_svg_new(boardRootItem,
					   svg_handle,
					   NULL);
  goo_canvas_item_translate(item, x, y);
  goo_canvas_item_scale(item, 0.25, 0.25);
  g_object_unref(svg_handle);
  g_free(note_str);
}

static void display_paying_tux(guint note)
{
  GdkPixbuf *pixmap = NULL;

  g_assert(boardRootItem);

  pixmap = gc_pixmap_load("money/tux_graduate.png");
  goo_canvas_image_new (boardRootItem,
			pixmap,
			20,
			130,
			NULL);
  gdk_pixbuf_unref(pixmap);

  if (note == 30)
    {
      display_paying_note(20, 20, 200);
      display_paying_note(10, 30, 250);
    }
  else if (note == 40)
    {
      display_paying_note(20, 20, 200);
      display_paying_note(20, 30, 250);
    }
  else if (note == 100)
    {
      display_paying_note(50, 20, 200);
      display_paying_note(50, 30, 250);
    }
  else
    display_paying_note(note, 20, 200);

  /* Set here the way to display money. Change only the money sign, and it's place, always keep %d */
  gchar *text = g_strdup_printf(_("Tux just bought some items in your shop.\n"
				  "He gives you $ %d, please give back his change."),
				note);
  goo_canvas_text_new(boardRootItem,
		      text,
		      BOARDWIDTH / 2,
		      320,
		      -1,
		      GTK_ANCHOR_CENTER,
		      "font", gc_skin_font_board_medium,
		      "fill-color", "white",
		      "alignment", PANGO_ALIGN_CENTER,
		      NULL);
  g_free(text);

}
