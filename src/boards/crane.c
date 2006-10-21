/* gcompris - crane.c
 *
 * Copyright (C) 2005 Marc BRUN
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

// Define
#define SOUNDLISTFILE PACKAGE

#define CRANE_BUTTON_SPACE	60	// Space between a pair (left/right or up/down)
#define CRANE_BUTTON_LEFT_X	85
#define CRANE_BUTTON_LEFT_Y	447
#define CRANE_BUTTON_RIGHT_X (CRANE_BUTTON_LEFT_X + CRANE_BUTTON_SPACE)
#define CRANE_BUTTON_RIGHT_Y CRANE_BUTTON_LEFT_Y + 2
#define CRANE_BUTTON_UP_X	220
#define CRANE_BUTTON_UP_Y	CRANE_BUTTON_LEFT_Y - 5
#define CRANE_BUTTON_DOWN_X CRANE_BUTTON_UP_X + CRANE_BUTTON_SPACE
#define CRANE_BUTTON_DOWN_Y CRANE_BUTTON_UP_Y

#define CRANE_FRAME_X 38
#define CRANE_FRAME_Y 137
#define CRANE_ROPE_Y  CRANE_FRAME_Y - 32
#define CRANE_FRAME_COLUMN 6
#define CRANE_FRAME_LINE 5
#define CRANE_FRAME_CELL 52
#define CRANE_FRAME_BORDER 6
#define CRANE_FRAME_IMAGE_SIZE 40

#define CRANE_FRAME_MODEL_X 460
#define CRANE_FRAME_MODEL_Y 50


#define DOWN  0		/* Warning ordering is important */
#define UP    1
#define LEFT  2
#define RIGHT 3

#define MAX_LEVEL 6	/* Don't raise this number except if putting more values
			 * in pixmap[] array, in place_item function
			 */

// List of images to use in the game
static gchar *imageList[] =
  {
    "crane/water_spot1.png",
    "crane/water_spot2.png",
    "crane/water_drop1.png",
    "crane/water_drop2.png",
    "crane/tux.png",
    "crane/triangle1.png",
    "crane/triangle2.png",
    "crane/rectangle1.png",
    "crane/rectangle2.png",
    "crane/square1.png",
    "crane/square2.png",
    "crane/bulb.png",
    "crane/letter-a.png",
    "crane/letter-b.png"
  };

#define NB_ELEMENT G_N_ELEMENTS(imageList)

// Types

// Structure describes the objects of the frames
typedef struct {
  GdkPixbuf *pixmap;
  double x;
  double y;
  double h;
  double w;
} crane_object;

// Structure decribes a movement
typedef struct {
  int x;		// x movement
  int y;		// y movement
  int nb;		// how much of x/y movement (to give a smooth effect) ?
} move_object;

// Global variables
static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;
static GnomeCanvasGroup *boardRootItem = NULL;
static GnomeCanvasItem *selected_item = NULL;
static GnomeCanvasItem *red_hands = NULL;
static GnomeCanvasItem *crane_rope_item = NULL;
static gint timer_id = 0;
static gint nb_move = 0;
static gboolean moving = FALSE;
static move_object my_move;
static int list_answer[CRANE_FRAME_LINE * CRANE_FRAME_COLUMN];
static int list_game[CRANE_FRAME_LINE * CRANE_FRAME_COLUMN];
static GnomeCanvasPoints *crane_rope;

// gcompris functions
static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static int	 gamewon;
static void	 game_won(void);

static GnomeCanvasItem	*crane_create_item();
static void		 crane_destroy_all_items(void);
static void		 crane_next_level(void);

// crane functions
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint		 arrow_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static guint		 smooth_move(move_object *);
static int		 is_allowed_move (double, double, int);
static void		 shuffle_list(int list[], int);
static void		 select_item(GnomeCanvasItem *, int);
static int		 get_item_index (double, double);

static void draw_arrow(void);
static void draw_frame(int, int);
static void draw_redhands(void);
static void place_item(int, int, int);

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Duplicate the top right model.",
    "Select item in the bottom left frame and move them with the crane's arrows.",
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

GET_BPLUGIN_INFO(crane)

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
	gcomprisBoard->maxlevel = MAX_LEVEL;
	gcomprisBoard->sublevel = 1;
	gcomprisBoard->number_of_sublevel = 1; /* Go to next level after this number of 'play' */
	gc_bar_set(GC_BAR_LEVEL);

	img = gc_skin_image_get("gcompris-bg.jpg");
	gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
				img);
	g_free(img);


	crane_next_level();

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
	crane_destroy_all_items();
  }

  gcomprisBoard = NULL;
}

/* ======================================= */
static void set_level (guint level) {

  if (gcomprisBoard != NULL) {

      gcomprisBoard->level = level;
      gcomprisBoard->sublevel = 1;
      crane_next_level();
  }
}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard) {

  if (gcomprisBoard) {

      if (g_strcasecmp(gcomprisBoard->type, "crane") == 0) {

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
static void crane_next_level() {

  gc_bar_set_level(gcomprisBoard);

  crane_destroy_all_items();
  gamewon = FALSE;

  /* Try the next level */
  crane_create_item();
}

/* ==================================== */
/* Destroy all the items */
static void crane_destroy_all_items()
{
  if (timer_id) {
    gtk_timeout_remove (timer_id);
    timer_id = 0;
  }

  gnome_canvas_points_free(crane_rope);

  if(boardRootItem != NULL)
	gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}

/* ==================================== */
static GnomeCanvasItem *crane_create_item()
{
  int i;
  int nb_element;
  GdkPixbuf *pixmap;

  boardRootItem = GNOME_CANVAS_GROUP(gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
				     gnome_canvas_group_get_type (),
				     "x", (double) 0,
				     "y", (double) 0,
				     NULL));


  pixmap = gc_pixmap_load("crane/crane-bg.png");
  gnome_canvas_item_new (boardRootItem,
			 gnome_canvas_pixbuf_get_type(),
			 "pixbuf", pixmap,
			 "x", 0.0,
			 "y", 0.0,
			 "anchor", GTK_ANCHOR_NW,
			 NULL);
  gdk_pixbuf_unref(pixmap);

  // The four arrows on the crane
  draw_arrow();

  // The leading frames on both model (answer) and boardgame, only for first levels
  if (gcomprisBoard->level < 5) {
  	draw_frame(CRANE_FRAME_X, CRANE_FRAME_Y);		// Boardgame
  	draw_frame(CRANE_FRAME_MODEL_X, CRANE_FRAME_MODEL_Y);	// Model
  }

  // The red corners (hands) that show the selected item
  draw_redhands();

  nb_element = gcomprisBoard->level * 2 + 2;

  // Answer frame => the unmovable items of the model
  for (i = 0 ; i < nb_element ; i++) list_answer[i] = i;
  for ( ; i < CRANE_FRAME_LINE * CRANE_FRAME_COLUMN ; i ++) list_answer[i] = -1;
  shuffle_list(list_answer, CRANE_FRAME_LINE * CRANE_FRAME_COLUMN);
  place_item(CRANE_FRAME_MODEL_X, CRANE_FRAME_MODEL_Y, 0);

  // Game frame => the playable items
  for (i = 0 ; i < nb_element ; i++) list_game[i] = i;
  for ( ; i < CRANE_FRAME_LINE * CRANE_FRAME_COLUMN ; i ++) list_game[i] = -1;
  shuffle_list(list_game, CRANE_FRAME_LINE * CRANE_FRAME_COLUMN);
  place_item(CRANE_FRAME_X, CRANE_FRAME_Y, 1);

  return NULL;
}

// Display an happy face for end of level
static void bonus() {

  gc_bonus_display(gamewon, BONUS_SMILEY);
  gc_sound_play_ogg ("sounds/bonus.ogg", NULL);
  timer_id = 0;
}

// Display a 'end of game' animation
static void finished() {

  gc_bonus_end_display(BOARD_FINISHED_RANDOM);
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
  crane_next_level();
}

// Event on item
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {

  if(board_paused)
	return FALSE;

  if (event->type == GDK_MOTION_NOTIFY) // Mouse moved
	return FALSE;

  // Select item if left click on it
  if (event->type == GDK_BUTTON_PRESS && event->button.button == 1)
	select_item(item, TRUE);

  return FALSE;
}

// Event on arrow
static gint arrow_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {

  int success;
  double x,y;
  double dx1, dy1, dx2, dy2;
  int direction = GPOINTER_TO_INT(data);
  int i;
  int index, new_index;

  if (board_paused)
	return FALSE;

  if (moving) // An object is already moving
	return FALSE;

  if (gamewon)
	return FALSE;

  if (event->type == GDK_MOTION_NOTIFY) // Mouse moved
	return FALSE;

  if (selected_item == NULL)
	return FALSE;

  // Left click on an arrow move the selected item
  if (event->type == GDK_BUTTON_PRESS && event->button.button == 1) {
	x = event->button.x;
	y = event->button.y;

	gnome_canvas_item_get_bounds(selected_item, &dx1, &dy1, &dx2, &dy2);

	switch (direction) {
	case LEFT:
	  my_move.x = -1;
	  my_move.y = 0;
	  break;
	case RIGHT:
	  my_move.x = 1;
	  my_move.y = 0;
	  break;
	case UP:
	  my_move.x = 0;
	  my_move.y = -1;
	  break;
	case DOWN:
	  my_move.x = 0;
	  my_move.y = 1;
	  break;
	}

	// Check if the move doesn't go out of the frame
	if (is_allowed_move(dx1, dy1, direction)) {

		index = get_item_index(dx1, dy1);
	      	new_index = index + my_move.x + (my_move.y * CRANE_FRAME_COLUMN);

		// Check if no object is already here
		if (list_game[new_index] == -1) {

			// Do a smooth move
			my_move.nb = 52;
			timer_id = g_timeout_add(10,  (GtkFunction) smooth_move, &my_move);
			list_game[new_index] = list_game[index];
			list_game[index] = -1;
		}
	}
  }

  // Check if the level is won
  success = 1;
  for (i = 0 ; i < CRANE_FRAME_LINE * CRANE_FRAME_COLUMN ; i++) {
	  if (list_answer[i] != list_game[i]) success = 0;
  }

  if (success) {
      gamewon = TRUE;
      timer_id = g_timeout_add (1200, (GtkFunction) bonus, NULL);
  }

  return FALSE;
}

// Draw the four arrows on the crane
static void draw_arrow() {

  GnomeCanvasItem *item_arrow = NULL;

  int i;
  crane_object arrow[4];

  arrow[0].pixmap = gc_pixmap_load("crane/arrow_down.png");
  arrow[0].x = CRANE_BUTTON_DOWN_X;
  arrow[0].y = CRANE_BUTTON_DOWN_Y;

  arrow[1].pixmap = gc_pixmap_load("crane/arrow_up.png");
  arrow[1].x = CRANE_BUTTON_UP_X;
  arrow[1].y = CRANE_BUTTON_UP_Y;

  arrow[2].pixmap = gc_pixmap_load("crane/arrow_left.png");
  arrow[2].x = CRANE_BUTTON_LEFT_X;
  arrow[2].y = CRANE_BUTTON_LEFT_Y + 2;

  arrow[3].pixmap = gc_pixmap_load("crane/arrow_right.png");
  arrow[3].x = CRANE_BUTTON_RIGHT_X;
  arrow[3].y = CRANE_BUTTON_RIGHT_Y - 2;

  for (i = 0 ; i < 4 ; i++) {
  	item_arrow = gnome_canvas_item_new (boardRootItem,
				    gnome_canvas_pixbuf_get_type(),
				    "pixbuf", arrow[i].pixmap,
				    "x", arrow[i].x,
				    "y", arrow[i].y,
				    "anchor", GTK_ANCHOR_NW,
				    NULL);
  	gtk_signal_connect(GTK_OBJECT(item_arrow), "event",
			   (GtkSignalFunc) arrow_event, GINT_TO_POINTER(i));
	gtk_signal_connect(GTK_OBJECT(item_arrow), "event",
			 (GtkSignalFunc) gc_item_focus_event,
			 NULL);
	gdk_pixbuf_unref( arrow[i].pixmap);

  }

}

// Draw the red hands object which highlight the selected object
static void draw_redhands() {

  GdkPixbuf *pixmap;

  /* Initialize the rope (Warning, it's static and freed in crane_destroy_all_items) */
  crane_rope = gnome_canvas_points_new(2);

  crane_rope->coords[0] = 5;
  crane_rope->coords[1] = CRANE_BUTTON_LEFT_Y;
  crane_rope->coords[2] = 5;
  crane_rope->coords[3] = CRANE_BUTTON_LEFT_Y;

  crane_rope_item = gnome_canvas_item_new (boardRootItem,
					   gnome_canvas_line_get_type(),
					  "points", crane_rope,
					  "fill_color", "darkblue",
					  "width_units", (double) 1,
					  "width_pixels", (guint) 7,
					  NULL);

  pixmap = gc_pixmap_load("crane/selected.png");

  red_hands = gnome_canvas_item_new (boardRootItem,
	gnome_canvas_pixbuf_get_type(),
	"pixbuf", pixmap,
	"x", (double) 5,
	"y", (double) 5,
	"width", (double) (CRANE_FRAME_IMAGE_SIZE + 5),
	"height", (double) (CRANE_FRAME_IMAGE_SIZE + 5),
	"width_set", TRUE,
	"height_set", TRUE,
	"anchor", GTK_ANCHOR_NW,
	NULL);

  gdk_pixbuf_unref(pixmap);

  gnome_canvas_item_hide(red_hands);

}

// Draw the drak frame (horizontal and vertical lines) that helps positionning elements
static void draw_frame(int x, int y) {

  GnomeCanvasItem *item_frame = NULL;
  int i;
  GnomeCanvasPoints *track;

  track = gnome_canvas_points_new(2);

  for (i = 1 ; i < CRANE_FRAME_COLUMN ; i++) {

	track->coords[0] = x + (i * CRANE_FRAME_CELL);
	track->coords[1] = y + CRANE_FRAME_BORDER;
	track->coords[2] = x + i * CRANE_FRAME_CELL;
	track->coords[3] = y + (CRANE_FRAME_LINE * CRANE_FRAME_CELL) - CRANE_FRAME_BORDER;

	item_frame = gnome_canvas_item_new (boardRootItem,
			  		gnome_canvas_line_get_type (),
					"points", track,
					"width_pixels", 1,
					"fill_color", "black",
					NULL);
  }

  for (i = 1 ; i < CRANE_FRAME_LINE ; i++) {

	track->coords[0] = x + CRANE_FRAME_BORDER;
	track->coords[1] = y + (i * CRANE_FRAME_CELL);
	track->coords[2] = x + (CRANE_FRAME_COLUMN * CRANE_FRAME_CELL) - CRANE_FRAME_BORDER;
	track->coords[3] = y + (i * CRANE_FRAME_CELL);

	item_frame = gnome_canvas_item_new (boardRootItem,
			  		gnome_canvas_line_get_type (),
					"points", track,
					"width_pixels", 1,
					"fill_color", "black",
					NULL);
  }

  gnome_canvas_points_free(track);

}

// Place randomly nb_element (depending on the level) elements on the frame.
// if active is set, the elements are connected to their function, that's for the bottom left frame
// if active is off, the elements are unmovable, that's for the top right model
static void place_item(int x, int y, int active) {

  GdkPixbuf *pixmap;
  GnomeCanvasItem *item_image = NULL;
  int i;
  int valeur;

  for (i = 0 ; i < CRANE_FRAME_LINE * CRANE_FRAME_COLUMN ; i ++) {

	if (active) {
		valeur = list_game[i];
	} else {
		valeur = list_answer[i];
	}

	// Empty cell
	if (valeur == -1)
		continue;

	pixmap = gc_pixmap_load(imageList[valeur]);
	item_image = gnome_canvas_item_new (boardRootItem,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap,
				    "x", (double) (x + 5 + ((i % CRANE_FRAME_COLUMN) * CRANE_FRAME_CELL)),
				    "y", (double) (y + 5 + (floor(i / CRANE_FRAME_COLUMN) * CRANE_FRAME_CELL)),
				    NULL);
	gdk_pixbuf_unref( pixmap);

	if (active)
  		gtk_signal_connect(GTK_OBJECT(item_image), "event", (GtkSignalFunc) item_event, NULL);

  }

  // 'Focus' given to the last one
  if (active)
	select_item(item_image, FALSE);

}

static guint smooth_move(move_object *move) {
  double dx1, dy1, dx2, dy2;

  if (nb_move == 0) {
	moving = TRUE;
	nb_move = move->nb;
  }

  gnome_canvas_item_get_bounds(red_hands, &dx1, &dy1, &dx2, &dy2);
  crane_rope->coords[0] = (dx1 + dx2) / 2;
  crane_rope->coords[1] = CRANE_ROPE_Y;
  crane_rope->coords[2] = (dx1 + dx2) / 2;
  crane_rope->coords[3] = (dy1 + dy2) / 2;

  gnome_canvas_item_set (crane_rope_item,
			 "points", crane_rope,
			 NULL);

  gnome_canvas_item_move(selected_item, move->x, move->y);
  gnome_canvas_item_move(red_hands, move->x, move->y);
  nb_move--;

  if (nb_move == 0) {
	moving = FALSE;
	return FALSE;
  }

  return TRUE;
}

int is_allowed_move (double dx, double dy, int direction) {

  if ((direction == LEFT) && (dx > (CRANE_FRAME_X + CRANE_FRAME_CELL)))
	return 1;
  else if ((direction == RIGHT) && (dx < (CRANE_FRAME_X + ((CRANE_FRAME_COLUMN - 1) * CRANE_FRAME_CELL))))
  	return 1;
  else if ((direction == UP) && (dy > (CRANE_FRAME_Y + CRANE_FRAME_CELL)))
  	return 1;
  else if ((direction == DOWN) && (dy < (CRANE_FRAME_Y + ((CRANE_FRAME_LINE - 1) * CRANE_FRAME_CELL))))
	return 1;

  return 0;
}

int get_item_index (double dx, double dy) {

  return (((dx - CRANE_FRAME_X - 5) / CRANE_FRAME_CELL) + ((dy - CRANE_FRAME_Y - 5) / CRANE_FRAME_CELL * CRANE_FRAME_COLUMN));

}

void shuffle_list(int list[], int size) {

  int i;
  int rand1, rand2, buffer;

  for (i = 0 ; i < size ; i++) {

	rand1 = RAND(0, size-1);
	rand2 = RAND(0, size-1);

	buffer = list[rand2];
	list[rand2] = list[rand1];
	list[rand1] = buffer;
  }
}

static void select_item(GnomeCanvasItem *item, int sound) {

  double dx1, dy1, dx2, dy2;

  // Use of gnome_canvas_item_affine_absolute must be better

  gnome_canvas_item_hide(red_hands);

  // Place redhands in (O;O)
  gnome_canvas_item_get_bounds(red_hands, &dx1, &dy1, &dx2, &dy2);
  gnome_canvas_item_move(red_hands, -(dx1), -(dy1));

  // Place redhands 'around' the selected item
  gnome_canvas_item_get_bounds(item, &dx1, &dy1, &dx2, &dy2);
  gnome_canvas_item_move(red_hands, dx1 - 1 , dy1 - 1);

  gnome_canvas_item_show(red_hands);

  crane_rope->coords[0] = (dx1 + dx2) / 2;
  crane_rope->coords[1] = CRANE_ROPE_Y;
  crane_rope->coords[2] = (dx1 + dx2) / 2;
  crane_rope->coords[3] = (dy1 + dy2) / 2;

  gnome_canvas_item_set (crane_rope_item,
			 "points", crane_rope,
			 NULL);

  if (sound) gc_sound_play_ogg ("sounds/gobble.ogg", NULL);

  selected_item = item;
}

