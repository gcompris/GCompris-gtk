/* gcompris - click_on_letter.c
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

GcomprisBoard *gcomprisBoard = NULL;
gboolean board_paused = TRUE;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);

static int gamewon;

static void		 process_ok(void);
static void		 highlight_selected(GnomeCanvasItem *);
static void		 game_won(void);
static void		 repeat(void);

#define VERTICAL_SEPARATION 30
#define HORIZONTAL_SEPARATION 30

#define NUMBER_OF_SUBLEVELS 3
#define NUMBER_OF_LEVELS 5

#define TEXT_COLOR "white"

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem *phone_note_item = NULL;
static GnomeCanvasItem *l1_item = NULL, *l2_item = NULL, *l3_item = NULL, *l4_item = NULL;
static GdkPixbuf *l1_pixmap = NULL,*l2_pixmap = NULL,*l3_pixmap = NULL,*l4_pixmap = NULL;
static GnomeCanvasItem *button1 = NULL, *button2 = NULL, *button3 = NULL, *button4 = NULL, *selected_button = NULL;

static GnomeCanvasItem *click_on_letter_create_item(GnomeCanvasGroup *parent);
static void click_on_letter_destroy_all_items(void);
static void click_on_letter_next_level(void);
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint phone_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

static int right_position;
static char right_letter[2] = "";

/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Read a letter"),
    N_("Learn to recognize letters"),
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
    repeat
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
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), 
			      gcompris_image_to_skin("gcompris-bg.jpg"));
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=NUMBER_OF_LEVELS;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=NUMBER_OF_SUBLEVELS; /* Go to next level after this number of 'play' */
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL|GCOMPRIS_BAR_REPEAT);
      gcompris_score_start(SCORESTYLE_NOTE,
			   50,
			   50,
			   gcomprisBoard->number_of_sublevel);

      click_on_letter_next_level();

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
      gcompris_score_end();
      click_on_letter_destroy_all_items();
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
      click_on_letter_next_level();
    }
}
/* ======================================= */
gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "click_on_letter")==0)
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
  if(gcomprisBoard!=NULL)
    {
      gcompris_play_ogg(right_letter, NULL);
    }
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* set initial values for the next level */
static void click_on_letter_next_level()
{
  gcompris_bar_set_level(gcomprisBoard);

  click_on_letter_destroy_all_items();
  gamewon = FALSE;
  selected_button = NULL;
  gcompris_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  click_on_letter_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void click_on_letter_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
static GnomeCanvasItem *click_on_letter_create_item(GnomeCanvasGroup *parent)
{
  int xOffset,yOffset,i,j;
  GdkFont *gdk_font;
  GdkPixbuf *button_pixmap = NULL;
  GdkPixbuf *phone_note_pixmap = NULL;
  char *str[4];
  char l[4];
  int numberOfLetters = 4;

  gdk_font = gdk_font_load (gcompris_skin_font_board_big);

  /*  */
  right_position = ((int)(((float)numberOfLetters)*rand()/(RAND_MAX+1.0))) +1;
  assert(right_position >= 1  && right_position <= numberOfLetters);

  /* set up letters, the right one is at position right_position */
  /* level 1 : min letters, level 2 : cap letters, level 3 : mix of both */
  for (i=0; i<numberOfLetters; i++) {
	switch (gcomprisBoard->level) {
		case 1 : l[i] = 'a' + ((char)( 26.0 * rand() / (RAND_MAX+1.0)));
			break;
		case 2 : l[i] = 'A' + ((char)( 26.0 * rand() / (RAND_MAX+1.0)));
			break;
		default :
			if ( rand() > (RAND_MAX/2) )
				l[i] = 'a' + ((char)( 26.0 * rand() / (RAND_MAX+1.0)));
			else
				l[i] = 'A' + ((char)( 26.0 * rand() / (RAND_MAX+1.0)));
			break;
	}
	// check that the letter has not been taken yet (beware that 'a' == 'A' is to be avoided)
	for(j=0; j<i; j++) {
		if (l[j] == l[i] || abs(l[j] - l[i]) == 'a' - 'A' ) {
			i--;
			continue;
		}
	}
  }

  right_letter[0] = l[right_position-1];
  right_letter[1] = 0;
  g_strdown(right_letter);
  gcompris_play_ogg("click_on_letter",right_letter, NULL);

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));
  for (i=0; i<numberOfLetters; i++) {
	str[i] =g_strdup_printf("gcompris/letters/%c.png", l[i]);
  }

  l1_pixmap = gcompris_load_pixmap(str[0]);
  l2_pixmap = gcompris_load_pixmap(str[1]);
  l3_pixmap = gcompris_load_pixmap(str[2]);
  l4_pixmap = gcompris_load_pixmap(str[3]);

  for (i=0; i<numberOfLetters; i++) {
	g_free(str[i]);
  }

  phone_note_pixmap = gcompris_load_pixmap("gcompris/misc/phone_note.png");
  button_pixmap = gcompris_load_skin_pixmap("button.png");

  yOffset = (gcomprisBoard->height - gdk_pixbuf_get_height(phone_note_pixmap) - gdk_pixbuf_get_height(button_pixmap) - 2*VERTICAL_SEPARATION) / 3;
  phone_note_item = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf",  phone_note_pixmap,
				   "x",  (double) (gcomprisBoard->width - gdk_pixbuf_get_width(phone_note_pixmap))/2,
				   "y",  (double)  yOffset,
				   NULL);

  yOffset += gdk_pixbuf_get_height(phone_note_pixmap) + VERTICAL_SEPARATION;
  xOffset = (gcomprisBoard->width - (numberOfLetters-1) * HORIZONTAL_SEPARATION - numberOfLetters * gdk_pixbuf_get_width(button_pixmap))/2;

  button1 = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf",  button_pixmap,
				   "x",  (double) xOffset,
				   "y",  (double) yOffset,
				   NULL);

  l1_item = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf",  l1_pixmap,
				   "x",  (double) xOffset+(gdk_pixbuf_get_width(button_pixmap)-gdk_pixbuf_get_width(l1_pixmap))/2,
				   "y",  (double) yOffset+(gdk_pixbuf_get_height(button_pixmap)-gdk_pixbuf_get_height(l1_pixmap))/2,
				   NULL);

  xOffset +=HORIZONTAL_SEPARATION +gdk_pixbuf_get_width(button_pixmap);

  button2 = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf",  button_pixmap,
				   "x",  (double) xOffset,
				   "y",  (double) yOffset,
				   NULL);
  l2_item = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf",  l2_pixmap,
				   "x",  (double) xOffset+(gdk_pixbuf_get_width(button_pixmap)-gdk_pixbuf_get_width(l2_pixmap))/2,
				   "y",  (double) yOffset+(gdk_pixbuf_get_height(button_pixmap)-gdk_pixbuf_get_height(l2_pixmap))/2,
				   NULL);

  xOffset +=HORIZONTAL_SEPARATION +gdk_pixbuf_get_width(button_pixmap);

  button3 = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf",  button_pixmap,
				   "x",  (double) xOffset,
				   "y",  (double) yOffset,
				   NULL);
  l3_item = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf",  l3_pixmap,
				   "x",  (double) xOffset+(gdk_pixbuf_get_width(button_pixmap)-gdk_pixbuf_get_width(l3_pixmap))/2,
				   "y",  (double) yOffset+(gdk_pixbuf_get_height(button_pixmap)-gdk_pixbuf_get_height(l3_pixmap))/2,
				   NULL);

  xOffset +=HORIZONTAL_SEPARATION +gdk_pixbuf_get_width(button_pixmap);

  button4 = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf",  button_pixmap,
				   "x",  (double) xOffset,
				   "y",  (double) yOffset,
				   NULL);
  l4_item = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf",  l4_pixmap,
				   "x",  (double) xOffset+(gdk_pixbuf_get_width(button_pixmap)-gdk_pixbuf_get_width(l4_pixmap))/2,
				   "y",  (double) yOffset+(gdk_pixbuf_get_height(button_pixmap)-gdk_pixbuf_get_height(l4_pixmap))/2,
				   NULL);

  gdk_pixbuf_unref(button_pixmap);
  gdk_pixbuf_unref(phone_note_pixmap);
  gdk_pixbuf_unref(l1_pixmap);
  gdk_pixbuf_unref(l2_pixmap);
  gdk_pixbuf_unref(l3_pixmap);
  gdk_pixbuf_unref(l4_pixmap);

  gtk_signal_connect(GTK_OBJECT(l1_item), "event", (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(l2_item), "event", (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(l3_item), "event", (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(l4_item), "event", (GtkSignalFunc) item_event, NULL);

  gtk_signal_connect(GTK_OBJECT(button1), "event",  (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(button2), "event",  (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(button3), "event",  (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(button4), "event",  (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(phone_note_item), "event",  (GtkSignalFunc) phone_event, NULL);
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
      board_finished(BOARD_FINISHED_TUXPLANE);
      return;
    }
    gcompris_play_ogg ("bonus", NULL);
  }
  click_on_letter_next_level();
}

/* ==================================== */
static gboolean process_ok_timeout() {
  gcompris_display_bonus(gamewon, BONUS_FLOWER);
	return FALSE;
}

static void process_ok() {
	// leave time to display the right answer
  g_timeout_add(TIME_CLICK_TO_BONUS, process_ok_timeout, NULL);
}
/* ==================================== */
static gint phone_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
  	gcompris_play_ogg(right_letter, NULL);
	break;
    default:
      break;
    }
  return TRUE;
}
/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  double item_x, item_y;
  GnomeCanvasItem * temp = NULL;
  item_x = event->button.x;
  item_y = event->button.y;
  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      temp = item;
      if (item == l1_item)
	temp = button1;
      if (item == l2_item)
	temp = button2;
      if (item == l3_item )
	temp = button3;
      if (item == l4_item )
	temp = button4;

      assert( temp == button1 || temp == button2 || temp == button3 || temp == button4);
      if ( ( temp == button1 && right_position == 1) ||
	   ( temp == button2 && right_position == 2) ||
	   ( temp == button3 && right_position == 3) ||
	   ( temp == button4 && right_position == 4) ) {
	gamewon = TRUE;
      } else {
	gamewon = FALSE;
      }
      highlight_selected(temp);
      process_ok();
      break;

    case GDK_MOTION_NOTIFY:
      break;

    case GDK_BUTTON_RELEASE:
      break;

    default:
      break;
    }
  return FALSE;
}
/* ==================================== */
static void highlight_selected(GnomeCanvasItem * item) {
  GdkPixbuf *button_pixmap_selected = NULL, *button_pixmap = NULL;

  assert( item == button1 || item == button2 || item == button3 || item == button4);

  if (selected_button != NULL) {
  	button_pixmap = gcompris_load_skin_pixmap("button.png");
  	gnome_canvas_item_set(selected_button, "pixbuf", button_pixmap, NULL);
  	gdk_pixbuf_unref(button_pixmap);
  }

  if (selected_button != item) {
  	button_pixmap_selected = gcompris_load_skin_pixmap("button_selected.png");
  	gnome_canvas_item_set(item, "pixbuf", button_pixmap_selected, NULL);
  	selected_button = item;
  	gdk_pixbuf_unref(button_pixmap_selected);
  } else
  	selected_button = NULL;

}
