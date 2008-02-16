/* gcompris - magic_hat.c
 *
 * Copyright (C) 2006 Marc BRUN
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
#include <string.h>

#define MH_FRAME1_X 420		// Coordonates for the first frame (first operand of the operation)
#define MH_FRAME1_Y 60
#define MH_FRAME2_X 420		// Coordonates for the second frame (second operand of the operation)
#define MH_FRAME2_Y 200
#define MH_FRAME_PLAYER_X 420	// Coordonates for the 'player' frame
#define MH_FRAME_PLAYER_Y 380

#define MH_HAT_X	190	// Coordonates for the hat
#define MH_HAT_Y	90
#define MH_HAT_HEIGHT	250
#define MH_HAT_WIDTH	130
#define POINT		0	// Which hat to draw ? The one with an interrogation point or
#define STARS		1	// the one with stars ?

#define NORMAL		0
#define EMPTY		1
#define UNDERHAT	2
#define DYNAMIC		3

#define MAX_ITEM 10		// Max number for an item in a list
#define MAX_LIST 3		// Max number of list of items
#define ITEM_SIZE 30		// Items are squares (or square-included)
#define SPACE_BETWEEN_ITEMS 5

#define MODE_MINUS	0
#define MODE_PLUS	1
#define DEFAULT_MODE	MODE_MINUS

// Types

// This structure describes a frame (there are 3 of them on the board)
typedef struct {
  int		id;
  double	coord_x;
  double	coord_y;
  int		nb_stars[MAX_LIST];
  int		array_star_type[MAX_LIST][MAX_ITEM];
  GooCanvasItem *array_item[MAX_LIST][MAX_ITEM];
} frame;

// This structure decribes a movement
typedef struct {
  int i;                // i index in array_item
  int j;                // j index in array_item
  double dx;
  double dy;
  int nb;               // how much of x/y movement (to give a smooth effect) ?
  int frame;		// number of the concerned frame (1 or 2)
} move_object;

// Global variables
static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;
static GooCanvasItem *boardRootItem = NULL;
static gint timer_id = 0;
static gint board_mode = DEFAULT_MODE;
static gint hat_event_id;	// value returned by g_signal_connect. Used by gtk_signal_disconnect

static GooCanvasItem *hat;
static frame frame1;
static frame frame2;
static frame frame_player;

// gcompris functions
static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 process_ok ();
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static GooCanvasItem	*magic_hat_create_item();
static void		 magic_hat_destroy_all_items(void);
static void		 magic_hat_next_level(void);

// magic_hat functions
static void draw_frame(frame *);
static void draw_table(void);
static void draw_hat(GooCanvasItem *hat, int);
static void place_item(frame *, int);
static gboolean hat_event (GooCanvasItem  *item,
			   GooCanvasItem  *target,
			   GdkEventButton *event,
			   gpointer data);
static gboolean item_event (GooCanvasItem  *item,
			    GooCanvasItem  *target,
			    GdkEventButton *event,
			    gpointer data);

static int  nb_list();
static gint smooth_move(move_object *);
static gint move_stars(frame *);
static gint close_hat();

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Give the result of an operation.",
    "Click on the bottom left stars to give the result of the operation.",
    "Marc BRUN",
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

GET_BPLUGIN_INFO(magic_hat)

static void pause_board (gboolean pause)
{
  if (gcomprisBoard == NULL)
    return;

  if (timer_id) {
    gtk_timeout_remove (timer_id);
    timer_id = 0;
  }

  /* the game is won */
  if (gamewon == TRUE && pause == FALSE)
	game_won();

  board_paused = pause;
}

static void start_board (GcomprisBoard *agcomprisBoard)
{

  if (agcomprisBoard != NULL) {
        gchar *img;

	gcomprisBoard = agcomprisBoard;
	gcomprisBoard->level = 1;
	gcomprisBoard->maxlevel = 9;
	gcomprisBoard->sublevel = 1;
	gcomprisBoard->number_of_sublevel = 1;	// Go to next level after this number of 'play'
	gc_bar_set(GC_BAR_LEVEL|GC_BAR_OK);

	if (strcmp(gcomprisBoard->mode, "minus") == 0)
		board_mode = MODE_MINUS;
	else if (strcmp(gcomprisBoard->mode, "plus") == 0)
		board_mode = MODE_PLUS;
	else
		board_mode = DEFAULT_MODE;

	img = gc_skin_image_get("gcompris-bg.jpg");
	gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas), img);
	g_free(img);

	magic_hat_next_level();

	gamewon = FALSE;
	pause_board(FALSE);
  }
}

static void end_board () {

  if (timer_id) {
	gtk_timeout_remove (timer_id);
	timer_id = 0;
  }

  if (gcomprisBoard != NULL) {
	pause_board(TRUE);
	magic_hat_destroy_all_items();
  }

  gcomprisBoard = NULL;
}

// Check if player has won
static void process_ok() {

  int i;
  int ok = TRUE;

  if (board_mode == MODE_MINUS) {
	for (i = 0 ; i < nb_list() ; i++) {
		if (frame1.nb_stars[i] != (frame2.nb_stars[i] + frame_player.nb_stars[i]))
			ok = FALSE;
	}
  } else {
	for (i = 0 ; i < nb_list() ; i++) {
		if (frame_player.nb_stars[i] != (frame1.nb_stars[i] + frame2.nb_stars[i]))
			ok = FALSE;
	}
  }

  if (ok) {
	gamewon = TRUE;
	gc_sound_play_ogg ("sounds/bonus.wav", NULL);
  }

  gc_bonus_display(gamewon, GC_BONUS_FLOWER);

}

/* ======================================= */
static void set_level (guint level) {

  if (gcomprisBoard != NULL) {

      gcomprisBoard->level = level;
      gcomprisBoard->sublevel = 1;
      magic_hat_next_level();
  }
}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard) {

  if (gcomprisBoard) {

      if (g_strcasecmp(gcomprisBoard->type, "magic_hat") == 0) {

	  /* Set the plugin entry */
	  gcomprisBoard->plugin = &menu_bp;
	  return TRUE;
	}
  }

  return FALSE;
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* set initial values for the next level */
static void magic_hat_next_level() {

  gc_bar_set_level(gcomprisBoard);

  magic_hat_destroy_all_items();
  gamewon = FALSE;

  /* Try the next level */
  magic_hat_create_item();
}

/* ==================================== */
/* Destroy all the items */
static void magic_hat_destroy_all_items()
{
  if (timer_id) {
    gtk_timeout_remove (timer_id);
    timer_id = 0;
  }

  if(boardRootItem != NULL)
	goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}

/* ==================================== */
static GooCanvasItem *magic_hat_create_item()
{
  int i, j;
  GdkPixbuf *pixmap;
  int step;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  if (board_mode == MODE_MINUS)
    pixmap = gc_pixmap_load("magic_hat/magic_hat_minus_bg.png");
  else
    pixmap = gc_pixmap_load("magic_hat/magic_hat_plus_bg.png");

  goo_canvas_image_new (boardRootItem,
			pixmap,
			0.0,
			0.0,
			NULL);
  gdk_pixbuf_unref(pixmap);

  // Initialisation for frame1
  frame1.id = 1;
  frame1.coord_x = MH_FRAME1_X;
  frame1.coord_y = MH_FRAME1_Y;

  // Initialisation for frame1
  frame2.id = 2;
  frame2.coord_x = MH_FRAME2_X;
  frame2.coord_y = MH_FRAME2_Y;

  // Initialisation for frame1
  frame_player.id = 3;
  frame_player.coord_x = MH_FRAME_PLAYER_X;
  frame_player.coord_y = MH_FRAME_PLAYER_Y;

  // The three frames of this activity : one for the sentence of the magician (top left), one for
  // the items out of the hat (top right), one for the answer of the payer (bottom right)
  draw_frame(&frame1);
  draw_frame(&frame2);
  draw_frame(&frame_player);

  // Description of the 9 levels for substraction
  // Level 1 : one list (yellow stars), from 2 to 4 stars in frame 1
  // Level 2 : one list (yellow stars), from 2 to 7 stars in frame 1
  // Level 3 : one list (yellow stars), from 2 to 10 stars in frame 1
  // Level 4 : two lists (yellow and green stars), from 2 to 4 stars in frame 1
  // Level 5 : two lists (yellow and green stars), from 2 to 7 stars in frame 1
  // Level 6 : two lists (yellow and green stars), from 2 to 10 stars in frame 1
  // Level 7 : three lists (yellow, green and blue stars), from 2 to 4 stars in frame 1
  // Level 8 : three lists (yellow, green and blue stars), from 2 to 7 stars in frame 1
  // Level 9 : three lists (yellow, green and blue stars), from 2 to 10 stars in frame 1
  //
  // Description of the 9 levels for addition
  // Level 1 : one list (yellow stars), from 2 to 4 for the total
  // Level 2 : one list (yellow stars), from 2 to 7 for the total
  // Level 3 : one list (yellow stars), from 2 to 10 for the total
  // Level 4 : two lists (yellow and green stars), from 2 to 4 for the total
  // Level 5 : two lists (yellow and green stars), from 2 to 7 for the total
  // Level 6 : two lists (yellow and green stars), from 2 to 10 for the total
  // Level 7 : three lists (yellow, green and blue stars), from 2 to 4 for the total
  // Level 8 : three lists (yellow, green and blue stars), from 2 to 7 for the total
  // Level 9 : three lists (yellow, green and blue stars), from 2 to 10 for the total

  step = 3;

  for (i = 0 ; i < nb_list() ; i++) {

	// Frame 1
	if (board_mode == MODE_MINUS)
		frame1.nb_stars[i] = g_random_int_range(2, (1 + (step * nb_list()))); // Minimum 2 to avoid '0' value (which is not easy to understand for kids)
	else
		frame1.nb_stars[i] = g_random_int_range(1, (step * nb_list()));

	for (j = 0 ; j < frame1.nb_stars[i] ; j++) frame1.array_star_type[i][j] = i;
	for ( ; j < MAX_ITEM ; j++) frame1.array_star_type[i][j] = -1;

	// Frame 2
	if (board_mode == MODE_MINUS)
		frame2.nb_stars[i] = g_random_int_range(1, frame1.nb_stars[i]);	// Minimum 1 to avoid '0'
	else
		frame2.nb_stars[i] = g_random_int_range(1, ((step * nb_list()) - frame1.nb_stars[i] + 1));

	for (j = 0 ; j < frame2.nb_stars[i] ; j++) frame2.array_star_type[i][j] = i;
	for ( ; j < MAX_ITEM ; j++) frame2.array_star_type[i][j] = -1;
	for (j = 0 ; j < MAX_ITEM ; j++)
	  frame2.array_item[i][j] = goo_canvas_image_new (boardRootItem,
							  NULL,
							  0,
							  0,
							  NULL);

	// Player frame
	frame_player.nb_stars[i] = 0;
	for (j = 0 ; j < MAX_ITEM ; j++) frame_player.array_star_type[i][j] = -1;
  }

  if (board_mode == MODE_MINUS) {
    place_item(&frame1, NORMAL);	// design the 'total' stars, with all the items
    place_item(&frame2, UNDERHAT);	// design 'out' stars, all the items are hidden under the hat
  } else {
    place_item(&frame1, NORMAL);	// design the first frame stars, with all the items
    place_item(&frame2, NORMAL);	// design the second frame stars, with all the items
  }

  // The magic hat !! And its table
  // The hat is designed after the 'minus' items so that it hides them

  /* Place the lower left corner as the (0,0) coord for the animation */
  hat = goo_canvas_image_new (boardRootItem,
			      NULL,
			      0,
			      -MH_HAT_HEIGHT,
			      NULL);
  goo_canvas_item_translate(hat,
			    MH_HAT_X,
			    MH_HAT_Y + MH_HAT_HEIGHT);

  draw_hat(hat, STARS);
  draw_table();

  return NULL;
}

// Display a 'end of game' animation
static void finished() {

  gc_bonus_end_display(GC_BOARD_FINISHED_RANDOM);
  timer_id = 0;
}

// One more level completed
static void game_won() {

  gcomprisBoard->sublevel++;

  if (gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
	/* Try the next level */
	gcomprisBoard->sublevel = 1;
	gcomprisBoard->level++;

	if (gcomprisBoard->level>gcomprisBoard->maxlevel) { // all levels completed : the current board is finished
		timer_id = g_timeout_add (2000, (GtkFunction) finished, NULL);
		return;
	}
  }
  magic_hat_next_level();
}

// Draw a frame with empty small squares
static void draw_frame(frame *my_frame) {

  GooCanvasItem *item_frame = NULL;
  int i, j;
  double x = my_frame->coord_x;
  double y = my_frame->coord_y;
  GooCanvasPoints *track;

  track = goo_canvas_points_new(5);

  for (i = 0 ; i < nb_list() ; i++) {

	  for (j = 0 ; j < MAX_ITEM ; j++) {

		track->coords[0] = x + (j * (ITEM_SIZE + SPACE_BETWEEN_ITEMS));
		track->coords[1] = y + (i * (ITEM_SIZE + SPACE_BETWEEN_ITEMS));
		track->coords[2] = x + (j * (ITEM_SIZE + SPACE_BETWEEN_ITEMS)) + ITEM_SIZE;
		track->coords[3] = y + (i * (ITEM_SIZE + SPACE_BETWEEN_ITEMS));
		track->coords[4] = x + (j * (ITEM_SIZE + SPACE_BETWEEN_ITEMS)) + ITEM_SIZE;
		track->coords[5] = y + (i * (ITEM_SIZE + SPACE_BETWEEN_ITEMS)) + ITEM_SIZE;
		track->coords[6] = x + (j * (ITEM_SIZE + SPACE_BETWEEN_ITEMS));
		track->coords[7] = y + (i * (ITEM_SIZE + SPACE_BETWEEN_ITEMS)) + ITEM_SIZE;
		track->coords[8] = x + (j * (ITEM_SIZE + SPACE_BETWEEN_ITEMS));
		track->coords[9] = y + (i * (ITEM_SIZE + SPACE_BETWEEN_ITEMS));

		item_frame = goo_canvas_polyline_new (boardRootItem, FALSE, 0,
						      "points", track,
						      "line-width", 1.0,
						      "fill-color", "#948d85",
						      NULL);
	}
  }
  goo_canvas_points_unref(track);

  place_item(my_frame, EMPTY);

}

// Draw the table (line)
static void draw_table() {

  GooCanvasItem *item_frame = NULL;
  GooCanvasPoints *track;

  track = goo_canvas_points_new(2);

  track->coords[0] = MH_HAT_X;
  track->coords[1] = MH_HAT_Y + MH_HAT_HEIGHT + 5;
  track->coords[2] = MH_HAT_X + MH_HAT_WIDTH;
  track->coords[3] = MH_HAT_Y + MH_HAT_HEIGHT + 5;

  item_frame = goo_canvas_polyline_new (boardRootItem, FALSE, 0,
					"points", track,
					"line-width", 1.0,
					"fill-color", "black",
					NULL);

  goo_canvas_points_unref(track);
}

// Draw the hat
static void draw_hat(GooCanvasItem *item,
		     int type) {

  GdkPixbuf *image;

  if (type == STARS)
    image = gc_pixmap_load("magic_hat/hat.png");
  else
    image = gc_pixmap_load("magic_hat/hat-point.png");

  g_object_set(item,
	       "pixbuf", image,
	       NULL);
  gdk_pixbuf_unref(image);

  if (type == STARS) {
	 hat_event_id = g_signal_connect(item,
					 "button_press_event",
					 (GtkSignalFunc) hat_event,
					 NULL);
	 gc_item_focus_init(item, NULL);
  }
}

// Place items on the board
// 	frame * my_frame : which frame to create the items on ?
// 	int type         : four possible values
// 				EMPTY => only grey stars, which symbolise an empty item
// 				NORMAL => a coloured star
// 				UNDERHAT => objects are not visible, they are localised under the hat
// 				DYNAMIC => the items are made clicable (for the player frame)
static void place_item(frame * my_frame, int type)
{
  GooCanvasItem *item = NULL;
  int i, j;
  int k, nb_item;
  RsvgHandle *image;
  double item_x, item_y;
  double x, y;


  RsvgHandle *image_name[MAX_LIST];
  RsvgHandle *image_star_clear = gc_rsvg_load("magic_hat/star-clear.svgz");

  image_name[0] = gc_rsvg_load("magic_hat/star1.svgz");
  image_name[1] = gc_rsvg_load("magic_hat/star2.svgz");
  image_name[2] = gc_rsvg_load("magic_hat/star3.svgz");

  x = my_frame->coord_x;
  y = my_frame->coord_y;

  for (i = 0 ; i < nb_list() ; i++) {

    for (j = 0 ; j < MAX_ITEM ; j++) {

      if ((j < my_frame->nb_stars[i]) && (type != EMPTY))
	image = image_name[i];
      else
	image = image_star_clear;

      if (type == UNDERHAT) {
	item_x = (MH_HAT_X + ((MH_HAT_WIDTH - ITEM_SIZE) / 2));
	item_y = (MH_HAT_Y + MH_HAT_HEIGHT -  2 * ITEM_SIZE);
      } else {
	item_x = x + (j * (ITEM_SIZE + SPACE_BETWEEN_ITEMS));
	item_y = y + (i * (ITEM_SIZE + SPACE_BETWEEN_ITEMS));
      }

      // If NORMAL, we have to create two items : the first one stays on the frame, the
      // other one moves to the hat
      if (type == NORMAL)
	nb_item = 2;
      else
	nb_item = 1;

      for (k = 0 ; k < nb_item ; k++)
	{
	  item = goo_svg_item_new (boardRootItem,
				   image, NULL);
	  goo_canvas_item_translate(item,
				    item_x,
				    item_y);
	}

      if (type == DYNAMIC)
	g_signal_connect(item, "button_press_event",
			 (GtkSignalFunc) item_event,
			 GINT_TO_POINTER(MAX_ITEM * i + j));

      if (type == UNDERHAT || type == NORMAL)
	my_frame->array_item[i][j] = item;
    }
  }

  g_object_unref(image_star_clear);
  g_object_unref(image_name[0]);
  g_object_unref(image_name[1]);
  g_object_unref(image_name[2]);

}

/* When clicked, an star from the player frame changes
 * its appearance (grey or coloured) and the counter is re-evaluated
 */
static gboolean item_event (GooCanvasItem  *item,
			    GooCanvasItem  *target,
			    GdkEventButton *event,
			    gpointer data)
{

  int index = GPOINTER_TO_INT(data);

  if (board_paused)
    return FALSE;

  if ((event->type == GDK_BUTTON_PRESS) && (event->button == 1)) {

    if (frame_player.array_star_type[index / MAX_ITEM][index % MAX_ITEM] >= 0)
      {
	RsvgHandle *pixmap;

	// Desactivate the star
	frame_player.nb_stars[index / MAX_ITEM]--;
	frame_player.array_star_type[index / MAX_ITEM][index % MAX_ITEM] = -1;

	pixmap = gc_rsvg_load("magic_hat/star-clear.svgz");

	g_object_set(item, "rsvg-handle", pixmap, NULL);

	g_object_unref(pixmap);

      }
    else
      {
	RsvgHandle *pixmap = NULL;

	// Activate the star
	frame_player.nb_stars[index / MAX_ITEM]++;
	frame_player.array_star_type[index / MAX_ITEM][index % MAX_ITEM] = index / MAX_ITEM;

	switch(index / MAX_ITEM)
	  {
	  case 0:
	    pixmap = gc_rsvg_load("magic_hat/star1.svgz");
	    break;
	  case 1:
	    pixmap = gc_rsvg_load("magic_hat/star2.svgz");
	    break;
	  case 2:
	    pixmap = gc_rsvg_load("magic_hat/star3.svgz");
	    break;
	  }
      g_object_set(item, "rsvg-handle", pixmap, NULL);

      g_object_unref(pixmap);
    }
    gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  }

  return FALSE;
}

// When clicked, the hat rotates and a few items can go out of or into it
// Then the hat go back in its previous position, and the game can start
static gboolean hat_event (GooCanvasItem  *item,
			   GooCanvasItem  *target,
			   GdkEventButton *event,
			   gpointer data)
{
  if (board_paused)
    return FALSE;

  if ((event->type == GDK_BUTTON_PRESS) && (event->button == 1))
    {
      // disconnect hat and hat_event, so that hat can not be clicked any more
      g_signal_handler_disconnect(hat, hat_event_id);

      // 'open' the hat
      goo_canvas_item_animate (item,
			       0,
			       0,
			       1,
			       -30,
			       FALSE,
			       40 * 20, 40,
			       GOO_CANVAS_ANIMATE_FREEZE);

    // Make the items move from/out the hat, depending on the mode
    // Wait a few seconds between the two frames
    move_stars(&frame1);
    timer_id = g_timeout_add(1200, (GtkFunction) move_stars, &frame2);

    // Wait again a few seconds before closing the hat. Then the game is ready to start
    timer_id = g_timeout_add(2600, (GtkFunction) close_hat, NULL);
  }

  return FALSE;
}

// Return nb_list to be displayed, depending of the game level :
// levels 1, 2 and 3 : one list
// levels 4, 5 and 6 : two lists
// levels 7, 8 and 9 : three lists
static int nb_list() {

  if(gcomprisBoard == NULL)
    return 0;

  return (1 + (gcomprisBoard->level - 1) / MAX_LIST) ;

}

// Move all the stars of one frame to or from the hat
static gint move_stars(frame *my_frame) {

  int i, j;
  move_object *my_move = NULL;

  gc_sound_play_ogg ("sounds/level.wav", NULL);

  for (i = 0 ; i < nb_list() ; i++) {
	for (j = 0 ; j < my_frame->nb_stars[i] ; j++) {
		if ((my_move = g_malloc(sizeof(move_object))) == NULL) {        // Freed in function smooth_move
			g_error ("Malloc error in hat_event");
		}
		my_move->i = i;
		my_move->j = j;
		my_move->nb = 20;
		my_move->dx = - ((my_frame->coord_x + (my_move->j * (ITEM_SIZE + SPACE_BETWEEN_ITEMS)) -
				(MH_HAT_X + ((MH_HAT_WIDTH - ITEM_SIZE) / 2))) / my_move->nb);
		my_move->dy = - ((my_frame->coord_y + (my_move->i * (ITEM_SIZE + SPACE_BETWEEN_ITEMS)) -
				(MH_HAT_Y + MH_HAT_HEIGHT -  2 * ITEM_SIZE)) / my_move->nb);

		if (board_mode == MODE_MINUS && my_frame->id == 2) {
			my_move->dx = -my_move->dx;
			my_move->dy = -my_move->dy;
		}

		my_move->frame = my_frame->id;
		timer_id = g_timeout_add(50, (GtkFunction) smooth_move, my_move);
	}
  }
  return FALSE;
}

// Close the hat, then the game can start
static gint close_hat() {

  // 'close' the hat
  goo_canvas_item_animate (hat,
			   0,
			   0,
			   1,
			   30,
			   FALSE,
			   40 * 20, 40,
			   GOO_CANVAS_ANIMATE_FREEZE);

  draw_hat(hat, POINT);

  // draw an empty dynamic frame, each item is activable by left click
  // before this, the player_frame is not clicable
  place_item(&frame_player, DYNAMIC);

  return FALSE;
}

// Move a star smoothly from under the hat to its final location, on the minus frame
static gint smooth_move(move_object *my_move) {

  if (!my_move->nb-- || boardRootItem == NULL) {
  	g_free(my_move);
	return FALSE;
  }

  if (my_move->frame == 1)
	goo_canvas_item_translate(frame1.array_item[my_move->i][my_move->j],
				  my_move->dx, my_move->dy);
  else
	goo_canvas_item_translate(frame2.array_item[my_move->i][my_move->j],
				  my_move->dx, my_move->dy);

  return TRUE;

}
