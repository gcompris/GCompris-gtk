/* gcompris - enumerate.c
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
#include <string.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void	 start_board (GcomprisBoard *agcomprisBoard);
static gint	 key_press(guint keyval, gchar *commit_str, gchar *preedit_str);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static void	 process_ok(void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem	*enumerate_create_item(GnomeCanvasGroup *parent);
static void		 enumerate_destroy_all_items(void);
static void		 enumerate_next_level(void);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint		 item_event_focus(GnomeCanvasItem *item, GdkEvent *event, guint index);

#define ANSWER_X	BOARDWIDTH - 150
#define ANSWER_WIDTH	40
#define ANSWER_HEIGHT	40

static int number_of_item_type	= 0;
static int number_of_item_max	= 0;

// List of images to use in the game
static gchar *imageList[] =
{
  "gcompris/food/banana.png",
  "gcompris/food/orange.png",
  "gcompris/food/milk_shake.png",
  "gcompris/food/pear.png",
  "gcompris/food/grapefruit.png",
  "gcompris/food/yahourt.png",
  "gcompris/food/milk_cup.png",
  "gcompris/food/suggar_box.png",
  "gcompris/food/butter.png",
  "gcompris/food/chocolate.png",
  "gcompris/food/cookie.png",
  "gcompris/food/french_croissant.png",
  "gcompris/food/chocolate_cake.png",
  "gcompris/food/marmelade.png",
  "gcompris/food/baby_bottle.png",
  "gcompris/food/bread_slice.png",
  "gcompris/food/round_cookie.png",
};
#define NUMBER_OF_IMAGES G_N_ELEMENTS(imageList)

static guint		 answer[NUMBER_OF_IMAGES];
static guint		 answer_to_find[NUMBER_OF_IMAGES];
static GnomeCanvasItem	*answer_item[NUMBER_OF_IMAGES];
static GnomeCanvasItem	*answer_item_focus[NUMBER_OF_IMAGES];
static guint	         current_focus	= 0;


/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Numeration training"),
    N_("Place the items in the best way to count them"),
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
    NULL,
    NULL,
    NULL
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(enumerate)

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

      /* disable im_context */
      gcomprisBoard->disable_im_context = TRUE;

      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=5;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL|GCOMPRIS_BAR_OK);

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      "images/enumerate_background.png");

      enumerate_next_level();

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
      enumerate_destroy_all_items();
    }
  gcomprisBoard = NULL;
}

/* Get the user keys to use to get the answer */
static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
{
  char str[2];
  GnomeCanvasItem	*item = NULL;

  if(!gcomprisBoard)
    return FALSE;

  /* Add some filter for control and shift key */
  switch (keyval)
    {
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
    case GDK_Mode_switch:
    case GDK_dead_circumflex:
    case GDK_Num_Lock:
      return FALSE; 
    case GDK_KP_Enter:
    case GDK_Return:
      process_ok();
      return TRUE;
    }

  sprintf(str, "%c", keyval);

  item = answer_item[current_focus];
      
  if(GNOME_IS_CANVAS_TEXT(item))
    {
      gchar *oldtext;
      gchar *newtext;

      gtk_object_get (GTK_OBJECT (item), "text", &oldtext, NULL);

      switch(keyval)
	{
	case GDK_BackSpace:
	case GDK_Delete:

	  if(oldtext[1] != '\0')
	    newtext = g_strndup(oldtext, strlen(oldtext)-1);
	  else
	    newtext = "?";

	  break;

	default:

	  if(keyval<'0' || keyval>'9')
	    str[0]='0';

	  if(oldtext[0] == '?' && strlen(oldtext)==1)
	    {
	      oldtext[0] = ' ';
	      g_strstrip(oldtext);
	    }

	  if(strlen(oldtext)<2)
	    newtext = g_strconcat(oldtext, &str, NULL);
	  else
	    newtext = g_strdup(oldtext);
	  break;

	}

      if(newtext[0] != '?')
	answer[current_focus] = atoi(newtext);

      gnome_canvas_item_set (item,
			     "text", newtext,
			     NULL);
	      
      g_free(oldtext);
    }

  return TRUE;
}

/* ======================================= */
static void set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      enumerate_next_level();
    }
}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "enumerate")==0)
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
static void enumerate_next_level()
{

  gcompris_bar_set_level(gcomprisBoard);

  enumerate_destroy_all_items();
  gamewon = FALSE;

  /* Select level difficulty */
  switch(gcomprisBoard->level)
    {
    case 1:
      number_of_item_type = 2;
      number_of_item_max  = 5;
      break;
    case 2:
      number_of_item_type = 3;
      number_of_item_max  = 6;
      break;
    case 3: 
      number_of_item_type = 4;
      number_of_item_max  = 7;
      break;
    case 4:
      number_of_item_type = 5;
      number_of_item_max  = 8;
      break;
    case 5:
      number_of_item_type = 6;
      number_of_item_max  = 10;
      break;
    default:
      number_of_item_type = 6;
      number_of_item_max = 10;
    }
      
  current_focus = 0;

  /* Try the next level */
  enumerate_create_item(gnome_canvas_root(gcomprisBoard->canvas));

}
/* ==================================== */
/* Destroy all the items */
static void enumerate_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
static GnomeCanvasItem *enumerate_create_item(GnomeCanvasGroup *parent)
{
  int i,j;
  int current_y;
  GnomeCanvasItem *item = NULL;
  GdkPixbuf *pixmap = NULL;
  GdkPixbuf *pixmap_answer = NULL;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,

							    NULL));

  current_y = BOARDHEIGHT;

  for(i=0; i<number_of_item_type; i++)
    {

      pixmap = gcompris_load_pixmap(imageList[i]);

      answer_to_find[i] = RAND(1, number_of_item_max);
      answer[i] = 0;

      for(j=0; j<answer_to_find[i]; j++)
	{
	  guint x, y;

	  x = RAND(0, ANSWER_X-gdk_pixbuf_get_width(pixmap)-ANSWER_WIDTH);
	  y = RAND(0, BOARDHEIGHT-gdk_pixbuf_get_height(pixmap));

	  item = gnome_canvas_item_new (boardRootItem,
					gnome_canvas_pixbuf_get_type (),
					"pixbuf", pixmap, 
					"x", (double) x,
					"y", (double) y,
					NULL);
      
	  gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event, NULL);
	}

      gdk_pixbuf_unref(pixmap);

      /* Display the answer area */
      current_y -= ANSWER_HEIGHT*2;

      pixmap_answer = gcompris_load_pixmap("images/enumerate_answer.png");

      item = \
	gnome_canvas_item_new (boardRootItem,
			       gnome_canvas_pixbuf_get_type (),
			       "pixbuf", pixmap_answer, 
			       "x", (double) ANSWER_X - ANSWER_WIDTH/2,
			       "y", (double) current_y - ANSWER_HEIGHT/2,
			       NULL);

      gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event_focus, (void *)i);

      gdk_pixbuf_unref(pixmap_answer);

      pixmap_answer = gcompris_load_pixmap("images/enumerate_answer_focus.png");

      answer_item_focus[i] = \
	gnome_canvas_item_new (boardRootItem,
			       gnome_canvas_pixbuf_get_type (),
			       "pixbuf", pixmap_answer, 
			       "x", (double) ANSWER_X - ANSWER_WIDTH/2,
			       "y", (double) current_y - ANSWER_HEIGHT/2,
			       NULL);

      gdk_pixbuf_unref(pixmap_answer);
      gnome_canvas_item_hide(answer_item_focus[i]);

      item = gnome_canvas_item_new (boardRootItem,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap, 
				    "x", (double) ANSWER_X,
				    "y", (double) current_y,
				    "width", (double) ANSWER_WIDTH,
				    "height", (double) ANSWER_HEIGHT,
				    "width_set", TRUE, 
				    "height_set", TRUE,
				    NULL);


      gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event_focus, (void *)i);

      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) gcompris_item_event_focus,
			 NULL);
      
      answer_item[i] = \
	gnome_canvas_item_new (boardRootItem,
			       gnome_canvas_text_get_type (),
			       "text", "?",
			       "font", gcompris_skin_font_board_big,
			       "x", (double) ANSWER_X + 2*ANSWER_WIDTH,
			       "y", (double) current_y + ANSWER_HEIGHT/2,
			       "anchor", GTK_ANCHOR_EAST,
			       "fill_color", "blue",
			       NULL);
      gtk_signal_connect(GTK_OBJECT(answer_item[i]), "event", (GtkSignalFunc) item_event_focus, (void *)i);

    }

  gnome_canvas_item_show(answer_item_focus[current_focus]);

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
  enumerate_next_level();
}

/* ==================================== */
static gint
item_event_focus(GnomeCanvasItem *item, GdkEvent *event, guint index)
{

   switch (event->type) 
     {
     case GDK_BUTTON_PRESS:
       gnome_canvas_item_hide(answer_item_focus[current_focus]);
       current_focus = index;
       gnome_canvas_item_show(answer_item_focus[current_focus]);
       return TRUE;
       break;
     default:
       break;
     }

   return FALSE;
}

/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  static double x, y;
  double item_x, item_y;
  GdkCursor *fleur;
  static int dragging;
  double new_x, new_y;

  item_x = event->button.x;
  item_y = event->button.y;
  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

  if(board_paused)
    return FALSE;


   switch (event->type) 
     {
     case GDK_BUTTON_PRESS:
       switch(event->button.button) 
         {
         case 1:
	   x = item_x;
	   y = item_y;
	   
	   gnome_canvas_item_raise_to_top(item);

	   fleur = gdk_cursor_new(GDK_FLEUR);
	   gcompris_canvas_item_grab(item,
				  GDK_POINTER_MOTION_MASK | 
				  GDK_BUTTON_RELEASE_MASK,
				  fleur,
				  event->button.time);
	   gdk_cursor_destroy(fleur);
	   dragging = TRUE;
           break;

         case 3:
         case 4:
	   /* fish up */
	   gnome_canvas_item_move(item, 0.0, -3.0);
	   break;

         case 2:
         case 5:
	   /* fish down */
	   gnome_canvas_item_move(item, 0.0, 3.0);
	   break;

         default:
           break;
         }
       break;

     case GDK_MOTION_NOTIFY:
       if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) 
         {
	   double x1, x2, y1, y2;

	   gnome_canvas_item_get_bounds(item,  &x1, &y1, &x2, &y2); 

           new_x = item_x;
           new_y = item_y;

	   /* Check board boundaries */
	   if((x1 < 0 && new_x<x)|| (x2 > BOARDWIDTH && new_x>x))
	     {
	       new_x = x;
	     }
	   if((y1 < 0 && new_y<y) || (y2 > BOARDHEIGHT && new_y>y))
	     {
	       new_y = y;
	     }

	   gnome_canvas_item_move(item, new_x - x, new_y - y);
	   x = new_x;
	   y = new_y;
         }
       break;
           
     case GDK_BUTTON_RELEASE:
       if(dragging) 
	 {
	   gcompris_canvas_item_ungrab(item, event->button.time);
	   dragging = FALSE;
	 }
       break;

     default:
       break;
     }
  
  return FALSE;
}

static void process_ok()
{
  guint i;
  gboolean win = TRUE;

  for(i=0; i<number_of_item_type; i++)
    {
      printf("answer[%d]=%d answer_to_find[%d]=%d\n",i,  answer[i], i, answer_to_find[i]);
      if(answer[i] != answer_to_find[i])
	win = FALSE;
    }

  if(win) {
    gamewon = TRUE;
    gcompris_display_bonus(gamewon, BONUS_SMILEY);
  } else {
    gamewon = FALSE;
    gcompris_display_bonus(gamewon, BONUS_SMILEY);
  }
}

