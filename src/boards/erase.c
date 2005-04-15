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

#include <ctype.h>
#include <math.h>
#include <assert.h>

#include "gcompris/gcompris.h"

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

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem	*erase_create_item();
static void		 erase_destroy_all_items(void);
static void		 erase_next_level(void);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

static int number_of_item = 0;
static int number_of_item_x = 0;
static int number_of_item_y = 0;

static gint timer_id = 0;

// Default Double clic distance to restore on exit.
gint DefaultDoubleClicDistance;

gint DoubleClicLevel[6]= {  1000, 750, 600, 500, 400, 250};
 1000, 750, 600, 500, 400, 25
#define NORMAL 0
#define CLIC   1
#define DOUBLECLIC 2

gint board_mode =  NORMAL;

// List of images to use in the game
static gchar *imageList[] =
  {
    "gcompris/animals/flamentrosegc.jpg",
    "gcompris/animals/girafegc.jpg",
    "gcompris/animals/rhinogc.jpg",
    "gcompris/animals/singegc.jpg",
    "gcompris/animals/joybear002.jpg",
    "gcompris/animals/elephanteauxgc.jpg",
    "gcompris/animals/hypogc.jpg",
    "gcompris/animals/jumentmulassieregc.jpg",
    "gcompris/animals/tetegorillegc.jpg"
  };
#define NUMBER_OF_IMAGES 9

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
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

      if (strcmp(gcomprisBoard->mode,"double_clic")==0)
	board_mode = DOUBLECLIC;
      else if (strcmp(gcomprisBoard->mode,"clic")==0)
	board_mode = CLIC;
      else 
	board_mode = NORMAL;

      if (board_mode == DOUBLECLIC){
	GtkSettings *DefaultsGtkSettings = gtk_settings_get_default ();

	if (DefaultsGtkSettings == NULL) {
	  g_warning(_("Couldn't get GTK settings"));
	} else {
	  g_object_get(G_OBJECT(DefaultsGtkSettings), 
		       "gtk-double-click-time", &DefaultDoubleClicDistance, NULL);
	  
	  g_warning(_("Double clic default value %d."),DefaultDoubleClicDistance);
	}

	gdk_display_set_double_click_time( gdk_display_get_default(),
					   DoubleClicLevel[gcomprisBoard->level-1]);
	g_warning(_("Double clic value is now %d."),DoubleClicLevel[gcomprisBoard->level-1]);
      }

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
    g_warning(_("Double clic value is now %d."),DefaultDoubleClicDistance);
  }
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
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
    g_warning(_("Double clic value is now %d."),DoubleClicLevel[gcomprisBoard->level-1]);
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

  gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			  imageList[RAND(0, NUMBER_OF_IMAGES-1)]);

  gcompris_bar_set_level(gcomprisBoard);

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
      
  /* Try the next level */
  erase_create_item(gnome_canvas_root(gcomprisBoard->canvas));}
  
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
static GnomeCanvasItem *erase_create_item()
{
  int i,j;
  int ix, jy;
  GnomeCanvasItem *item = NULL;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  number_of_item = 0;

  for(i=0,ix=0; i<BOARDWIDTH; i+=BOARDWIDTH/number_of_item_x, ix++)
    {
      for(j=0, jy=0; j<BOARDHEIGHT; j+=BOARDHEIGHT/number_of_item_y, jy++)
	{

	  if ((board_mode != NORMAL) && ((ix+jy) %2 == 0))
	    continue;

	  item = gnome_canvas_item_new (boardRootItem,
					gnome_canvas_rect_get_type (),
					"x1", (double) i,
					"y1", (double) j,
					"x2", (double) i+BOARDWIDTH/number_of_item_x,
					"y2", (double)  j+BOARDHEIGHT/number_of_item_y,
					"fill_color_rgba", gcompris_skin_get_color("erase/rectangle in"),
					"outline_color_rgba", gcompris_skin_get_color("erase/rectangle out"),
					"width_units", (double)1,
					NULL);
	  gtk_object_set_data(GTK_OBJECT(item),"state", GINT_TO_POINTER(0));
	  gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event, NULL);
	  number_of_item++;
	}
    }

  return NULL;
}

static void bonus() {
  gcompris_display_bonus(gamewon, BONUS_SMILEY);
  timer_id = 0;
}

static void finished() {
  board_finished(BOARD_FINISHED_RANDOM);
  timer_id = 0;
}

/* ==================================== */
static void game_won()
{
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    int i;
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
      timer_id = gtk_timeout_add (2000, (GtkFunction) finished, NULL);
      return;
    }
    if (board_mode == DOUBLECLIC){
      gdk_display_set_double_click_time( gdk_display_get_default(),
					 DoubleClicLevel[gcomprisBoard->level-1]);
      g_warning(_("Double clic value is now %d."),DoubleClicLevel[gcomprisBoard->level-1]);
    }

    gcompris_play_ogg ("bonus", NULL);
  }
  erase_next_level();
}

/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  guint state;

  if(board_paused)
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


  if (board_mode != DOUBLECLIC){
    state = (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "state")));
    if(gcomprisBoard->level>2)
      {
	if(!state)
	  {
	    gnome_canvas_item_set(item,
				  "fill_color_rgba", gcompris_skin_get_color("erase/rectangle in2"),
				  "outline_color_rgba", gcompris_skin_get_color("erase/rectangle out2"),
				  NULL);
	    state++;
	    gtk_object_set_data(GTK_OBJECT(item),"state", GINT_TO_POINTER(state));
	    return FALSE;
	  }
	
	if(gcomprisBoard->level>4) {
	  if(state==1)
	    {     
	      state++;
	      gtk_object_set_data(GTK_OBJECT(item),"state", GINT_TO_POINTER(state));
	      gnome_canvas_item_set(item, 
				    "fill_color_rgba", gcompris_skin_get_color("erase/rectangle in3"),
				    "outline_color_rgba", NULL,
				    NULL);
	      return FALSE;
	    }
	}
      }
  }
  else {
    if (event->type == GDK_BUTTON_PRESS){
      gnome_canvas_item_set(item,
			    "fill_color_rgba", gcompris_skin_get_color("erase/rectangle in2"),
			    "outline_color_rgba", gcompris_skin_get_color("erase/rectangle out2"),
			    NULL);
      return FALSE;
    }
    if (event->type == GDK_BUTTON_RELEASE){
      gnome_canvas_item_set(item,
			    "fill_color_rgba", gcompris_skin_get_color("erase/rectangle in"),
			    "outline_color_rgba", gcompris_skin_get_color("erase/rectangle out"),
			    NULL);
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
