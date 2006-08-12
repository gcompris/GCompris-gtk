/* gcompris - money.c
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

#include <ctype.h>
#include <math.h>
#include <assert.h>

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
static void	 process_ok(void);
static int	 gamewon;
static void	 game_won(void);

static GnomeCanvasGroup *boardRootItem = NULL;

static void		 money_destroy_all_items(void);
static void		 money_next_level(void);

Money_Widget    *tux_money    = NULL;
Money_Widget    *seller_money = NULL;

static double	 price_target = 0;

typedef struct {
  char *image;
  guint status;
  float value;
  GnomeCanvasItem *item;
} MoneyItem;

// List of images to use in the game
static gchar *imageList[] =
{
  "gcompris/misc/apple.png",
  "gcompris/misc/bicycle.png",
  "gcompris/misc/bottle.png",
  "gcompris/misc/carot.png",
  "gcompris/misc/cerise.png",
  "gcompris/misc/crown.png",
  "gcompris/misc/eggpot.png",
  "gcompris/misc/flower.png",
  "gcompris/misc/football.png",
  "gcompris/misc/lamp.png",
  "gcompris/misc/light.png",
  "gcompris/misc/peer.png",
  "gcompris/misc/pencil.png",
  "gcompris/misc/strawberry.png"
};
#define NUMBER_OF_IMAGES 14

#define WITHOUT_CENTS	1
#define WITH_CENTS	2
static char currentMode = WITHOUT_CENTS;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Money",
    "Pratise money usage",
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
    process_ok,
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
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL|GCOMPRIS_BAR_OK);

      /* Default mode */
      if(!gcomprisBoard->mode)
	{
	  gcomprisBoard->maxlevel=9;
	  currentMode=WITHOUT_CENTS;
	}
      else if(g_strncasecmp(gcomprisBoard->mode, "WITH_CENTS", 1)==0)
	{
	  gcomprisBoard->maxlevel=5;
	  currentMode=WITH_CENTS;
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
  GdkPixbuf       *pixmap = NULL;
  guint		   min_price = 0, max_price = 0;
  guint		   number_of_item = 0;
  guint		   i;
  gchar		  *display_format;
  gchar		  *img;

  img = gcompris_image_to_skin("money-bg.png");
  gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			  img);
  g_free(img);

  gcompris_bar_set_level(gcomprisBoard);

  money_destroy_all_items();
  gamewon = FALSE;

  boardRootItem = GNOME_CANVAS_GROUP(gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  tux_money = MONEY_WIDGET(money_widget_new());
  money_widget_set_position(tux_money,
			    boardRootItem,
			    100.0, 400.0,
			    700.0, 500.0,
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
	}
      break;
    }

  seller_money = MONEY_WIDGET(money_widget_new());
  money_widget_set_position(seller_money,
			    boardRootItem,
			    100.0,  30.0,
			    700.0, 130.0,
			    5, 2,
			    FALSE);

  money_widget_set_target(seller_money, tux_money);
  money_widget_set_target(tux_money, seller_money);

  /* Display what to buy */
  price_target = 0;
  for(i=1; i<=number_of_item; i++)
  {
    double object_price;
    gchar *text;

    pixmap = gcompris_load_pixmap(imageList[RAND(0, NUMBER_OF_IMAGES-1)]);

    gnome_canvas_item_new ( boardRootItem,
			    gnome_canvas_pixbuf_get_type (),
			    "pixbuf", pixmap,
			    "x", (double) (i*BOARDWIDTH)/(number_of_item+1) 
			    - gdk_pixbuf_get_width(pixmap)/2,
			    "y", (double) 200,
			    NULL);
    
    /* Diplay the price */
    object_price  = (double) RAND(min_price/number_of_item, max_price/number_of_item);

    if(currentMode==WITH_CENTS)
      {
	/* Set here the way to display money. Change only the money sign, and it's place, always keep %.2f, it will be replaced by 0,34 if decimal is ',' in your locale */
	display_format = _("$ %.2f");
	/* Add random cents */
	if(gcomprisBoard->level == 1)
	  {
	    object_price += (double)((double) RAND(1, 9))/10.0;
	  }
	else
	  {
	    object_price += (double)((double) RAND(1, 99))/100.0;
	  }
      }
    else
      {
	display_format = _("$ %.0f");
      }

    price_target += object_price;
    text = g_strdup_printf(display_format, object_price);
    gnome_canvas_item_new(boardRootItem,
			  gnome_canvas_text_get_type (),
			  "text", text,
			  "font", gcompris_skin_font_board_big,
			  "x", (double) (i*BOARDWIDTH)/(number_of_item+1),
			  "y", (double) 180,
			  "anchor", GTK_ANCHOR_CENTER,
			  "fill_color", "white",
			  NULL);
    g_free(text);
    gdk_pixbuf_unref(pixmap);
  }

}
/* ==================================== */
/* Destroy all the items */
static void money_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

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
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
      board_finished(BOARD_FINISHED_RANDOM);
      return;
    }
    gcompris_play_ogg ("sounds/bonus.ogg", NULL);
  }
  money_next_level();
}

/* ==================================== */
static void process_ok()
{

  if(board_paused)
    /*return FALSE*/;

  /* FIXME: Why do I need this trick !! */
  if(price_target >= money_widget_get_total(seller_money) - 0.001 &&
     price_target <= money_widget_get_total(seller_money) + 0.001 )
    {
      gamewon = TRUE;
      money_destroy_all_items();
      gcompris_display_bonus(gamewon, BONUS_SMILEY);
    }
  else
    gcompris_display_bonus(gamewon, BONUS_SMILEY);
}
