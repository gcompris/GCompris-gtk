/* gcompris - algebra_guesscount.c
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

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
static int gamewon;
static gint process_time_id = 0;
static void process_ok(void);
static void process_time(void);
static void game_won();

void dump_token(); // FIXME DEBUG ONLY

static void destroy_board();

/* 4 levels :
 * 1evel 1 : 2 numbers and 1 operation
 * 1evel 2 : 3 numbers and 2 operations
 * 1evel 3 : 4 numbers and 3 operations
 * 1evel 4 : 5 numbers and 4 operations
 */
#define NUMBER_OF_SUBLEVELS 2
#define NUMBER_OF_LEVELS 4
#define MAX_NUMBER 5

#define TEXT_COLOR "yellow"
#define TEXT_RESULT_COLOR "red"
#define BLANK "___"

#define NO_RESULT -1

#define BUTTON_WIDTH 81
#define BUTTON_HEIGHT 64
#define HORIZONTAL_SEPARATION 20
#define VERTICAL_SEPARATION 20

static const char oper_values[] = {'+', '-', 'x', ':', '='};
static const int num_values[] = {1,2,3,4,5,6,7,8,9,10,25,50,100};
#define NUM_VALUES 13
#define Y_OPE 20
#define Y_NUM 100
#define Y_ANS 200

#define X_NUM1 300
#define X_OPE 400
#define X_NUM2 500
#define X_EQUAL 600
#define X_RESULT 700

typedef struct _token token;
struct _token {
	gboolean isNumber;
	gboolean isMoved;
	char oper;
	int num;
	int original_num_item_place;
	int xOffset_original;
	int signal_id;
	GnomeCanvasItem * item;
};

// contains the values, NUM OPER NUM OPER NUM etc.
token token_value[MAX_NUMBER*2-1];
token * ptr_token_selected[MAX_NUMBER*2-1];

static const int y_equal_offset[] = {Y_ANS,Y_ANS+BUTTON_HEIGHT+VERTICAL_SEPARATION,
Y_ANS+2*BUTTON_HEIGHT+2*VERTICAL_SEPARATION,Y_ANS+3*BUTTON_HEIGHT+3*VERTICAL_SEPARATION};

static const int x_token_offset[] = {X_NUM1,X_OPE,X_NUM2,X_OPE,X_NUM2,X_OPE,X_NUM2,X_OPE,X_NUM2};
static const int y_token_offset[] = {Y_ANS, Y_ANS,Y_ANS,
	Y_ANS+BUTTON_HEIGHT+VERTICAL_SEPARATION, Y_ANS+BUTTON_HEIGHT+VERTICAL_SEPARATION,
	Y_ANS+2*BUTTON_HEIGHT+2*VERTICAL_SEPARATION,Y_ANS+2*BUTTON_HEIGHT+2*VERTICAL_SEPARATION,
	Y_ANS+3*BUTTON_HEIGHT+3*VERTICAL_SEPARATION, Y_ANS+3*BUTTON_HEIGHT+3*VERTICAL_SEPARATION};

static char answer_oper[MAX_NUMBER-1];
static int answer_num_index[MAX_NUMBER];
static int token_count;
static int result_to_find;

/* ================================================================ */
static GnomeCanvasGroup *boardRootItem = NULL;

static GdkPixbuf * num_pixmap[NUM_VALUES];
static GdkPixbuf * oper_pixmap[5];
static GdkPixbuf *button_pixmap = NULL;

static GnomeCanvasItem *oper_item[4];
static GnomeCanvasItem *num_item[MAX_NUMBER];
static GnomeCanvasItem *equal_item[NUMBER_OF_LEVELS];
static GnomeCanvasItem *calcul_line_item[NUMBER_OF_LEVELS*2];
static GnomeCanvasItem *result_item;

static GnomeCanvasItem *algebra_guesscount_create_item(GnomeCanvasGroup *parent);
static void algebra_guesscount_destroy_all_items(void);
static void algebra_guesscount_next_level(void);
static gint item_event_num(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint item_event_oper(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static gint item_event_oper_moved(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

void item_absolute_move(GnomeCanvasItem *item, int x, int y);
static int generate_numbers();
static int token_result();

/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Guess operations"),
    N_("Guess operations"),
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
    NULL
  };

/* ==================================== */
/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 */
BoardPlugin * get_bplugin_info(void){
  return &menu_bp;
}

/* ==================================== */
// in : boolean TRUE = PAUSE : FALSE = CONTINUE
static void pause_board (gboolean pause){
  if(gcomprisBoard==NULL)
    return;

  if(gamewon == TRUE && pause == FALSE) {
      game_won();
    }

  board_paused = pause;
}

/* ==================================== */
static void start_board (GcomprisBoard *agcomprisBoard) {
	int i;
  gchar *str;

  if(agcomprisBoard!=NULL){
      gcomprisBoard=agcomprisBoard;

			// load pixmap files
			for (i=0; i<NUM_VALUES; i++) {
  			str = g_strdup_printf("%s/%d.png", gcomprisBoard->boarddir,num_values[i]);
				num_pixmap[i] = gcompris_load_pixmap(str);
			  g_free(str);
				}
			for (i=0; i<5; i++) {
  			str = g_strdup_printf("%s/%c.png", gcomprisBoard->boarddir,oper_values[i]);
				oper_pixmap[i] = gcompris_load_pixmap(str);
			  g_free(str);
				}

			str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir,"button.png");
   		button_pixmap = gcompris_load_pixmap(str);
			g_free(str);

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),"gcompris/animals/tiger1_by_Ralf_Schmode.jpg");
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=NUMBER_OF_LEVELS;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=NUMBER_OF_SUBLEVELS; /* Go to next level after this number of 'play' */
      gcompris_score_start(SCORESTYLE_NOTE,
			   50,
			   gcomprisBoard->height - 50,
			   gcomprisBoard->number_of_sublevel);
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL|GCOMPRIS_BAR_OK);

      algebra_guesscount_next_level();

      gamewon = FALSE;
      pause_board(FALSE);
    }
}

/* ==================================== */
static void end_board () {
  if(gcomprisBoard!=NULL) {
      pause_board(TRUE);
      gcompris_score_end();
			destroy_board();
			algebra_guesscount_destroy_all_items();
    }
}

/* ==================================== */
static void set_level (guint level) {
  if(gcomprisBoard!=NULL) {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      algebra_guesscount_next_level();
    }
}

/* ==================================== */
gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
  if (gcomprisBoard) {
      if(g_strcasecmp(gcomprisBoard->type, "algebra_guesscount")==0) {
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;
	  return TRUE;
	}
    }
  return FALSE;
}

/* ==================================== */
/* set initial values for the next level */
static void algebra_guesscount_next_level() {
  gcompris_bar_set_level(gcomprisBoard);

  algebra_guesscount_destroy_all_items();
  gamewon = FALSE;
	token_count = 0;

  gcompris_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  algebra_guesscount_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void algebra_guesscount_destroy_all_items() {
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
static int token_result() {
	int result, i;

	if (token_count == 0)
		return NO_RESULT;

	assert(ptr_token_selected[0]->isNumber);
	result = num_values[ptr_token_selected[0]->num];
  printf("result = %d ", result);

	for (i=2; i<token_count; i+=2) {
		assert(!ptr_token_selected[i-1]->isNumber);
		switch (ptr_token_selected[i-1]->oper) {
			case '+' : 	result += num_values[ptr_token_selected[i]->num];
									break;
			case '-' : 	result -= num_values[ptr_token_selected[i]->num];// printf(" - %d\n",num_values[token_value[i].num]);
									break;
			case 'x' : 	result *= num_values[ptr_token_selected[i]->num];
									break;
			case ':' : 	result /= num_values[ptr_token_selected[i]->num];
									break;
			default : printf("bug in token_result()\n"); break;
		}
		printf("result = %d ", result);
	}
	printf("\n");
	return result;
}
/* ==================================== */
static void update_line_calcul() {
	int line;
	char str[12];

	// finds which line has to be zeroed.
	if (token_count%2 == 0)
		line = (int)(token_count/2-1);
		else
			line = (int)(token_count/2);

	sprintf(str, "%d",token_result());
	gnome_canvas_item_set(calcul_line_item[line*2], "text", BLANK, NULL);
	gnome_canvas_item_set(calcul_line_item[line*2+1], "text", BLANK, NULL);
}
/* ==================================== */
static int generate_numbers() {
	int i, r, j, result;
  gboolean minus, divide;

  for (i=0; i<gcomprisBoard->level+1; i++) {
    j = RAND(0,NUM_VALUES-1);
    answer_num_index[i] = j;
  }

  result = num_values[answer_num_index[0]];
  for (i=0; i<gcomprisBoard->level; i++) {
    // + and x can always be chosen, but we must ensure - and / are valid
		minus = (result - num_values[answer_num_index[i+1]] >= 0);
    divide = (result % num_values[answer_num_index[i+1]] == 0);

    r = 2 + (minus ? 1 : 0) + (divide ? 1 : 0);
		printf("r = %d\n", r);

    switch (RAND(1,r)) {
						case 1 : 	answer_oper[i] = '+';
											result += num_values[answer_num_index[i+1]];
											break;
						case 2 : 	answer_oper[i] = 'x';
											result *= num_values[answer_num_index[i+1]];
											break;
						case 3 : 	if (minus) {
							answer_oper[i] = '-';
							result -= num_values[answer_num_index[i+1]];
							assert(result >= 0);
						} else {
							answer_oper[i] = ':';
							assert(result%num_values[answer_num_index[i+1]] == 0);
							result /= num_values[answer_num_index[i+1]];
							}
								break;
						case 4 : if ( RAND(0,1) == 0) {
											answer_oper[i] = '-';
											result -= num_values[answer_num_index[i+1]];
											assert(result >= 0);
											} else {
												answer_oper[i] = ':';
												assert(result%num_values[answer_num_index[i+1]] == 0);
												result /= num_values[answer_num_index[i+1]];
												}
								break;
						default : printf("Bug in guesscount\n"); break;
    }
  }
// dump for DEBUG
printf("-----------------------------------\n");
  printf("num:%d ",num_values[answer_num_index[0]]);
  for (i=0; i<gcomprisBoard->level; i++) {
		printf("oper:%c ",answer_oper[i]);
		printf("num:%d ",num_values[answer_num_index[i+1]]);
	}
	printf("\n");
// end DEBUG
	return result;
}
/* ==================================== */
static GnomeCanvasItem *algebra_guesscount_create_item(GnomeCanvasGroup *parent) {
  GdkFont *gdk_font;
  int i, xOffset, sid;
	char str[10];

	gdk_font = gdk_font_load ("-adobe-times-medium-r-normal--*-240-100-*-*-*-*-*");

	result_to_find = generate_numbers();

	boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

	// the intermediate result, line by line, when empty is "_"
	for (i=0; i<gcomprisBoard->level; i++) {
		calcul_line_item[i*2] = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_text_get_type (),
				"text", BLANK,
				"font_gdk", gdk_font,
				"x", (double) X_EQUAL+BUTTON_WIDTH*1.5,
				"y", (double) y_equal_offset[i]+BUTTON_HEIGHT/2,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color", TEXT_COLOR,
				NULL);
	}

	for (i=0; i<gcomprisBoard->level-1; i++) {
		calcul_line_item[i*2+1] = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_text_get_type (),
				"text", BLANK,
				"font_gdk", gdk_font,
				"x", (double) X_NUM1+BUTTON_WIDTH/2,
				"y", (double) y_equal_offset[i+1]+BUTTON_HEIGHT/2,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color", TEXT_COLOR,
				NULL);
	}

  xOffset = (gcomprisBoard->width - 4 * BUTTON_WIDTH - 3 * HORIZONTAL_SEPARATION)/2;
	for (i=0; i<4; i++) {
		oper_item[i] = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", oper_pixmap[i],
				      "x", (double) xOffset ,
				      "y", (double) Y_OPE,
				      "width", (double) BUTTON_WIDTH,
				      "height", (double) BUTTON_HEIGHT,
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);
		xOffset += BUTTON_WIDTH+HORIZONTAL_SEPARATION;
  	gtk_signal_connect(GTK_OBJECT(oper_item[i]), "event", (GtkSignalFunc) item_event_oper, (void *) &(token_value[i*2+1]));
		token_value[i*2+1].isNumber = FALSE;
		token_value[i*2+1].isMoved = FALSE;
		token_value[i*2+1].oper = oper_values[i];
		}

	// displays the target result
	sprintf(str,"%d",result_to_find);
	result_item = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_text_get_type (),
				"text", str,
				"font_gdk", gdk_font,
				"x", (double) xOffset+BUTTON_WIDTH,
				"y", (double) Y_OPE+BUTTON_HEIGHT/2,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color", TEXT_RESULT_COLOR,
				NULL);

	xOffset = (gcomprisBoard->width - (gcomprisBoard->level+1) * BUTTON_WIDTH - gcomprisBoard->level * HORIZONTAL_SEPARATION)/2;
	for (i=0; i<gcomprisBoard->level+1; i++) {
		num_item[i] = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", num_pixmap[answer_num_index[i]],
				      "x", (double) xOffset ,
				      "y", (double) Y_NUM,
				      "width", (double) BUTTON_WIDTH,
				      "height", (double) BUTTON_HEIGHT,
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);
  	sid = gtk_signal_connect(GTK_OBJECT(num_item[i]), "event", (GtkSignalFunc) item_event_num,
			(void *)&(token_value[i*2]));
		token_value[i*2].isNumber = TRUE;
		token_value[i*2].num = answer_num_index[i];
		token_value[i*2].signal_id = sid;
		token_value[i*2].item = num_item[i];
		token_value[i*2].isMoved = FALSE;
		token_value[i*2].original_num_item_place = i;
		token_value[i*2].xOffset_original = xOffset;
		xOffset += BUTTON_WIDTH+HORIZONTAL_SEPARATION;
		}

	// items "="
	for (i=0; i<gcomprisBoard->level; i++) {
		equal_item[i] = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", oper_pixmap[4],
				      "x", (double) X_EQUAL ,
				      "y", (double) y_equal_offset[i],
				      "width", (double) BUTTON_WIDTH,
				      "height", (double) BUTTON_HEIGHT,
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);
	}

  return NULL;
}
/* ==================================== */
static void game_won() {
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) {
	board_finished(BOARD_FINISHED_TUXPLANE);
	return;
    }
  }
  algebra_guesscount_next_level();
}

/* ==================================== */
static void process_ok(){
	gamewon = (result_to_find == token_result());
  process_time_id = gtk_timeout_add (50, (GtkFunction) process_time, NULL);
}
/* ==================================== */
static void process_time(){
  if (process_time_id) {
    gtk_timeout_remove (process_time_id);
    process_time_id = 0;
  }
  gcompris_display_bonus(gamewon, BONUS_FLOWER);
}
/* ==================================== */
static int oper_char_to_pixmap_index(char oper) {
	int i;
	printf("oper = %c\n", oper);
	assert(oper == '+' || oper == '-' || oper == 'x' || oper == ':' || oper == '=');

	for (i=0; i<5; i++)
		if (oper_values[i] == oper)
			return i;

	return -1;
}
/* ==================================== */
static gint item_event_oper(GnomeCanvasItem *item, GdkEvent *event, gpointer data){
	token *t = (	token *)data;

	GnomeCanvasItem * tmp_item;

	if(board_paused)
    return FALSE;
	// first verify it is oper turn
	if (token_count % 2 == 0 || token_count >= 2*gcomprisBoard->level+1)
		return FALSE;

  switch (event->type) {
    case GDK_BUTTON_PRESS:
printf("token_count = %d clicked on %c\n",token_count,t->oper);
			ptr_token_selected[token_count] = t;
			tmp_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", oper_pixmap[oper_char_to_pixmap_index(t->oper)],
				      "x", (double) x_token_offset[token_count],
				      "y", (double) y_token_offset[token_count],
				      "width", (double) BUTTON_WIDTH,
				      "height", (double) BUTTON_HEIGHT,
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);
			token_count++;
  		gtk_signal_connect(GTK_OBJECT(tmp_item), "event", (GtkSignalFunc) item_event_oper_moved, GINT_TO_POINTER(token_count));
      break;
    }
  return FALSE;
}
/* ==================================== */
static gint item_event_oper_moved(GnomeCanvasItem *item, GdkEvent *event, gpointer data){
	int count = GPOINTER_TO_INT(data);

	if(board_paused)
    return FALSE;
  // we only allow the undo of an operation if it is the last element displayed
  switch (event->type) {
    case GDK_BUTTON_PRESS:
		printf("count=%d token_count=%d\n", count,token_count);
			if (count == token_count) {
				gtk_object_destroy (GTK_OBJECT(item));
				token_count--;
				update_line_calcul();
				}
			break;
    }
  return FALSE;
}
/* ==================================== */
static gint item_event_num(GnomeCanvasItem *item, GdkEvent *event, gpointer data){
	token *t = (token *)data;
  char str[12];

	if(board_paused)
    return FALSE;

  switch (event->type){
    case GDK_BUTTON_PRESS:
			printf("clicked token_count = %d for value = %d ismoved=%d\n", token_count,num_values[t->num], t->isMoved);
			if (t->isMoved) {
				if (item != ptr_token_selected[token_count-1]->item)
					return FALSE;
			// we put back in its original place a number item
				item_absolute_move(item, t->xOffset_original, Y_NUM);
				token_count--;
				update_line_calcul();
				t->isMoved = FALSE;
			} else { // the item is at its original place
				if (token_count % 2 == 1 || token_count > 2*gcomprisBoard->level+1)
					return FALSE;
				item_absolute_move(item, x_token_offset[token_count], y_token_offset[token_count]);
				t->isMoved = TRUE;
				ptr_token_selected[token_count] = t;
				token_count++;
				// update result text items
				if (token_count != 1 && token_count % 2 == 1) {
					sprintf(str,"%d",token_result());
					printf("token_result = %s\n", str);
					gnome_canvas_item_set(calcul_line_item[token_count-3], "text", str, NULL);
					gnome_canvas_item_set(calcul_line_item[token_count-2], "text", str, NULL);
					}
				}
			break;
		default : break;
    }
  return FALSE;
}
/* ======================================= */
// causes a segfault, why ?
// FIXME : potential memory leak ?
static void destroy_board() {
	return;
	int i;
	for (i=0; i<NUM_VALUES; i++)
  	gdk_pixbuf_unref(num_pixmap[i]);
	for (i=0; i<5; i++)
		gdk_pixbuf_unref(oper_pixmap[i]);
}
/* ======================================= */
void item_absolute_move(GnomeCanvasItem *item, int x, int y) {
	double dx1, dy1, dx2, dy2;
	gnome_canvas_item_get_bounds(item, &dx1, &dy1, &dx2, &dy2);
	gnome_canvas_item_move(item, ((double)x)-dx1, ((double)y)-dy1);
}
/* ======================================= */
// FIXME : here for debug only
void dump_token() {
	int i;
	token *t;
	printf("+++++ dump_token +++++\n");
	for (i=0; i<MAX_NUMBER*2-1; i++) {
		t = &token_value[i];
		if (t->isNumber) {
			printf("i = %d num = %d value = %d\n", i, t->num, num_values[t->num]);
		}
	}
	printf("----- dump_token -----\n");
}
