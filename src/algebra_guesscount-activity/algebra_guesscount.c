/* gcompris - algebra_guesscount.c
 *
 * Copyright (C) 2001, 2008 Pascal Georges
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

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);

static int gamewon;
static gint process_time_id = 0;

static void		 process_time(void);
static void		 game_won(void);
static void		 destroy_board(void);

/* 4 levels :
 * 1evel 1 : 2 numbers and 1 operation
 * 1evel 2 : 3 numbers and 2 operations
 * 1evel 3 : 4 numbers and 3 operations
 * 1evel 4 : 5 numbers and 4 operations
 */
#define NUMBER_OF_SUBLEVELS 3 // 3
#define NUMBER_OF_LEVELS 4 // 4
#define MAX_NUMBER 5

#define TEXT_COLOR_FRONT "yellow"
#define TEXT_COLOR_BACK "black"

#define TEXT_RESULT_COLOR_FRONT "red"
#define TEXT_RESULT_COLOR_BACK "orange"

#define BLANK "___"

#define NO_RESULT -1

#define BUTTON_WIDTH 81
#define BUTTON_HEIGHT 64
#define HORIZONTAL_SEPARATION 20
#define VERTICAL_SEPARATION 20

static char* background_images[] = {"algebra_guesscount/tiger1_RS",
				    "algebra_guesscount/tigerdrink001.jpg",
				    "algebra_guesscount/tigercub003.jpg",
				    "algebra_guesscount/tigerplay001.jpg"};
static const char  oper_values[] = {'+', '-', 'x', ':', '='};
static const char *oper_images[] = {"plus", "minus", "by", "div", "equal"};
static const int   num_values[] = {1,2,3,4,5,6,7,8,9,10,25,50,100};
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
  int xOffset_original;
  int signal_id;
  GooCanvasItem * item;
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
static GooCanvasItem *boardRootItem = NULL;

static GdkPixbuf * num_pixmap[NUM_VALUES];
static GdkPixbuf * oper_pixmap[5];
static GdkPixbuf *button_pixmap = NULL;

static GooCanvasItem *oper_item[4];
static GooCanvasItem *num_item[MAX_NUMBER];
static GooCanvasItem *equal_item[NUMBER_OF_LEVELS];
static GooCanvasItem *calcul_line_item[NUMBER_OF_LEVELS*2];
static GooCanvasItem *calcul_line_item_back[NUMBER_OF_LEVELS*2];
static GooCanvasItem *result_item_front, *result_item_back;

static GooCanvasItem *algebra_guesscount_create_item(GooCanvasItem *parent);
static void algebra_guesscount_destroy_all_items(void);
static void algebra_guesscount_next_level(void);
static gboolean item_event_num (GooCanvasItem  *item,
				GooCanvasItem  *target,
				GdkEventButton *event,
				gpointer data);
static gboolean item_event_oper (GooCanvasItem  *item,
				 GooCanvasItem  *target,
				 GdkEventButton *event,
				 gpointer data);
static gboolean item_event_oper_moved (GooCanvasItem  *item,
				       GooCanvasItem  *target,
				       GdkEventButton *event,
				       gpointer data);

static int generate_numbers();
static int token_result();

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Guess operations",
    "Guess operations",
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
    NULL,
    set_level,
    NULL,
    NULL,
    NULL,
    NULL
  };

/* ==================================== */
/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 */
GET_BPLUGIN_INFO(algebra_guesscount)

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
    g_warning("loading pixmaps in start_board\n");
    for (i=0; i<NUM_VALUES; i++) {
      str = g_strdup_printf("%s/%d.png", gcomprisBoard->boarddir,num_values[i]);
      num_pixmap[i] = gc_pixmap_load(str);
      g_free(str);
    }
    for (i=0; i<5; i++) {
      str = g_strdup_printf("%s/%s.png", gcomprisBoard->boarddir,oper_images[i]);
      oper_pixmap[i] = gc_pixmap_load(str);
      g_free(str);
    }

    str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir,"button.png");
    button_pixmap = gc_pixmap_load(str);
    g_free(str);

    gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),"algebra_guesscount/tiger1_RS.jpg");
    gcomprisBoard->level=1;
    gcomprisBoard->maxlevel=NUMBER_OF_LEVELS;
    gcomprisBoard->sublevel=1;
    gcomprisBoard->number_of_sublevel=NUMBER_OF_SUBLEVELS; /* Go to next level after this number of 'play' */
    gc_score_start(SCORESTYLE_NOTE,
			 50,
			 BOARDHEIGHT - 50,
			 gcomprisBoard->number_of_sublevel);
    gc_bar_set(GC_BAR_LEVEL);

    algebra_guesscount_next_level();

    gamewon = FALSE;

    pause_board(FALSE);
  }
}

/* ==================================== */
static void end_board () {
  if(gcomprisBoard!=NULL) {
    pause_board(TRUE);
    gc_score_end();
    destroy_board();
    algebra_guesscount_destroy_all_items();
  }
  gcomprisBoard = NULL;
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
static gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
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
  gc_bar_set_level(gcomprisBoard);

  algebra_guesscount_destroy_all_items();
  gamewon = FALSE;
  token_count = 0;

  gc_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  algebra_guesscount_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void algebra_guesscount_destroy_all_items() {
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}
/* ==================================== */
static int token_result() {
  int result, i;

  if (token_count < 2)
    return NO_RESULT;

  g_assert(ptr_token_selected[0]->isNumber);
  result = num_values[ptr_token_selected[0]->num];

  for (i=2; i<token_count; i+=2) {
    g_assert(!ptr_token_selected[i-1]->isNumber);
    switch (ptr_token_selected[i-1]->oper) {
    case '+' : 	result += num_values[ptr_token_selected[i]->num];
      break;
    case '-' :		if (result - num_values[ptr_token_selected[i]->num] < 0)
      return NO_RESULT;
      result -= num_values[ptr_token_selected[i]->num];
      break;
    case 'x' : 	result *= num_values[ptr_token_selected[i]->num];
      break;
    case ':' :		if (result%num_values[ptr_token_selected[i]->num] != 0)
      return NO_RESULT;
      result /= num_values[ptr_token_selected[i]->num];
      break;
    default : g_warning("bug in token_result()\n"); break;
    }
  }
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

  if(line==-1)
    return;

  sprintf(str, "%d",token_result());
  g_object_set(calcul_line_item[line*2],      "text", BLANK, NULL);
  g_object_set(calcul_line_item_back[line*2], "text", BLANK, NULL);

  if(line < gcomprisBoard->level-1) {			/* No next line to update on last line */
    g_object_set(calcul_line_item[line*2+1],      "text", BLANK, NULL);
    g_object_set(calcul_line_item_back[line*2+1], "text", BLANK, NULL);
  }

}
/* ==================================== */
static int generate_numbers() {
  int i, r, j, result;
  gboolean minus, divide;

  for (i=0; i<gcomprisBoard->level+1; i++) {
    j = g_random_int_range(0,NUM_VALUES-1);
    answer_num_index[i] = j;
  }

  result = num_values[answer_num_index[0]];
  for (i=0; i<gcomprisBoard->level; i++) {
    // + and x can always be chosen, but we must ensure - and / are valid
    minus = (result - num_values[answer_num_index[i+1]] >= 0);

    if(gcomprisBoard->level > 2 && num_values[answer_num_index[i+1]] <= 5) {       /* Avoid div operator at lower level */
      divide = (result % num_values[answer_num_index[i+1]] == 0);
    } else {
      divide = 0;
    }
    r = 2 + minus + divide;

    switch (g_random_int_range(1,r)) {
    case 1 :
      answer_oper[i] = '+';
      result += num_values[answer_num_index[i+1]];
      break;
    case 2 :		// prevent result from getting too big and accept only < 10 bor the by operator
      if ( (result*num_values[answer_num_index[i+1]] < 1000 ) &&
	   ( num_values[answer_num_index[i+1]] < 10) ) {
	answer_oper[i] = 'x';
	result *= num_values[answer_num_index[i+1]];
      } else {
	answer_oper[i] = '+';
	result += num_values[answer_num_index[i+1]];
      }
      break;
    case 3 :
      if (minus) {
	answer_oper[i] = '-';
	result -= num_values[answer_num_index[i+1]];
	g_assert(result >= 0);
      } else {
	answer_oper[i] = ':';
	g_assert(result%num_values[answer_num_index[i+1]] == 0);
	result /= num_values[answer_num_index[i+1]];
      }
      break;
    case 4 :
      if ( g_random_int_range(0,1) == 0) {
	answer_oper[i] = '-';
	result -= num_values[answer_num_index[i+1]];
	g_assert(result >= 0);
      } else {
	answer_oper[i] = ':';
	g_assert(result%num_values[answer_num_index[i+1]] == 0);
	result /= num_values[answer_num_index[i+1]];
      }
      break;
    default : g_warning("Bug in guesscount\n"); break;
    }
  }
  return result;
}
/* ==================================== */
static GooCanvasItem *algebra_guesscount_create_item(GooCanvasItem *parent) {
  int i, xOffset, sid;
  char str[10];

  result_to_find = generate_numbers();

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  // the intermediate result, line by line, when empty is BLANK
  for (i=0; i<gcomprisBoard->level; i++) {
    calcul_line_item_back[i*2] = goo_canvas_text_new (boardRootItem,
						      BLANK,
						      (double) X_EQUAL+BUTTON_WIDTH*1.5 + 1,
						      (double) y_equal_offset[i]+BUTTON_HEIGHT/2 + 1,
						      -1,
						      GTK_ANCHOR_CENTER,
						      "font", gc_skin_font_board_title_bold,
						      "fill-color", TEXT_COLOR_BACK,
						      NULL);
    calcul_line_item[i*2] = goo_canvas_text_new (boardRootItem,
						 BLANK,
						 (double) X_EQUAL+BUTTON_WIDTH*1.5,
						 (double) y_equal_offset[i]+BUTTON_HEIGHT/2,
						 -1,
						 GTK_ANCHOR_CENTER,
						 "font", gc_skin_font_board_title_bold,
						 "fill-color", TEXT_COLOR_FRONT,
						 NULL);
  }

  for (i=0; i<gcomprisBoard->level-1; i++) {
    calcul_line_item_back[i*2+1] = goo_canvas_text_new (boardRootItem,
							BLANK,
							(double) X_NUM1+BUTTON_WIDTH/2 + 1,
							(double) y_equal_offset[i+1]+BUTTON_HEIGHT/2 + 1,
							-1,
							GTK_ANCHOR_CENTER,
							"font", gc_skin_font_board_title_bold,
							"fill-color", TEXT_COLOR_BACK,
							NULL);
    calcul_line_item[i*2+1] = goo_canvas_text_new (boardRootItem,
						   BLANK,
						   (double) X_NUM1+BUTTON_WIDTH/2,
						   (double) y_equal_offset[i+1]+BUTTON_HEIGHT/2,
						   -1,
						   GTK_ANCHOR_CENTER,
						   "font", gc_skin_font_board_title_bold,
						   "fill-color", TEXT_COLOR_FRONT,
						   NULL);
  }

  xOffset = (BOARDWIDTH - 4 * BUTTON_WIDTH - 3 * HORIZONTAL_SEPARATION)/2;
  for (i=0; i<4; i++) {
    oper_item[i] = goo_canvas_image_new (boardRootItem,
					 oper_pixmap[i],
					 xOffset ,
					 Y_OPE,
					  NULL);
    xOffset += BUTTON_WIDTH+HORIZONTAL_SEPARATION;
    g_signal_connect(oper_item[i], "button_press_event",
		     (GtkSignalFunc) item_event_oper,
		     GINT_TO_POINTER(&(token_value[i*2+1])) );
    token_value[i*2+1].isNumber = FALSE;
    token_value[i*2+1].isMoved = FALSE;
    token_value[i*2+1].oper = oper_values[i];
  }

  // displays the target result
  sprintf(str,"%d",result_to_find);
  result_item_back = goo_canvas_text_new (boardRootItem,
					  str,
					  (double) xOffset+BUTTON_WIDTH +1,
					  (double) Y_OPE+BUTTON_HEIGHT/2 +1,
					  -1,
					  GTK_ANCHOR_CENTER,
					  "font", gc_skin_font_board_title_bold,
					  "fill-color", TEXT_RESULT_COLOR_BACK,
					  NULL);
  result_item_front = goo_canvas_text_new (boardRootItem,
					   str,
					   (double) xOffset+BUTTON_WIDTH,
					   (double) Y_OPE+BUTTON_HEIGHT/2,
					   -1,
					   GTK_ANCHOR_CENTER,
					   "font", gc_skin_font_board_title_bold,
					   "fill-color", TEXT_RESULT_COLOR_FRONT,
					   NULL);

  xOffset = (BOARDWIDTH - (gcomprisBoard->level+1) * BUTTON_WIDTH - gcomprisBoard->level * HORIZONTAL_SEPARATION)/2;
  for (i=0; i<gcomprisBoard->level+1; i++) {
    num_item[i] = goo_canvas_image_new (boardRootItem,
					num_pixmap[answer_num_index[i]],
					xOffset ,
					Y_NUM,
					NULL);
    sid = g_signal_connect(num_item[i], "button_press_event",
			   (GtkSignalFunc) item_event_num,
			   (void *)&(token_value[i*2]));
    token_value[i*2].isNumber = TRUE;
    token_value[i*2].num = answer_num_index[i];
    token_value[i*2].signal_id = sid;
    token_value[i*2].item = num_item[i];
    token_value[i*2].isMoved = FALSE;
    token_value[i*2].xOffset_original = xOffset;
    xOffset += BUTTON_WIDTH+HORIZONTAL_SEPARATION;
  }

  // items "="
  for (i=0; i<gcomprisBoard->level; i++) {
    equal_item[i] = goo_canvas_image_new (boardRootItem,
					  oper_pixmap[4],
					  X_EQUAL ,
					  y_equal_offset[i],
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
      gc_bonus_end_display(GC_BOARD_FINISHED_TUXPLANE);
      return;
    }
    gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),background_images[gcomprisBoard->level-1]);
  }
  algebra_guesscount_next_level();
}

/* ==================================== */
static void process_time(){
  if (process_time_id) {
    gtk_timeout_remove (process_time_id);
    process_time_id = 0;
  }
  gc_bonus_display(gamewon, GC_BONUS_RANDOM);
}
/* ==================================== */
static int oper_char_to_pixmap_index(char oper) {
  int i;

  g_assert(oper == '+' || oper == '-' || oper == 'x' || oper == ':' || oper == '=');

  for (i=0; i<5; i++)
    if (oper_values[i] == oper)
      return i;

  return -1;
}
/* ==================================== */
static gboolean item_event_oper (GooCanvasItem  *item,
				 GooCanvasItem  *target,
				 GdkEventButton *event,
				 gpointer data)
{
  token *t = (	token *)data;

  GooCanvasItem * tmp_item;

  if(board_paused)
    return FALSE;
  // first verify it is oper turn
  if (token_count % 2 == 0 || token_count >= 2*gcomprisBoard->level+1)
    return FALSE;

  switch (event->type) {
  case GDK_BUTTON_PRESS:
    gc_sound_play_ogg ("sounds/flip.wav", NULL);
    ptr_token_selected[token_count] = t;
    tmp_item = goo_canvas_image_new (boardRootItem,
				     oper_pixmap[oper_char_to_pixmap_index(t->oper)],
				     x_token_offset[token_count],
				     y_token_offset[token_count],
				     NULL);
    token_count++;
    g_signal_connect(tmp_item, "button_press_event",
		     (GtkSignalFunc) item_event_oper_moved,
		     GINT_TO_POINTER(token_count));
    break;
  default : break;
  }
  return FALSE;
}
/* ==================================== */
static gboolean item_event_oper_moved (GooCanvasItem  *item,
				       GooCanvasItem  *target,
				       GdkEventButton *event,
				       gpointer data)
{
  int count = GPOINTER_TO_INT(data);

  if(board_paused)
    return FALSE;
  // we only allow the undo of an operation if it is the last element displayed
  switch (event->type) {
  case GDK_BUTTON_PRESS:
    gc_sound_play_ogg ("sounds/flip.wav", NULL);
    if (count == token_count) {
      goo_canvas_item_remove(item);
      token_count--;
      update_line_calcul();
    }
    break;
  default : break;
  }
  return FALSE;
}
/* ==================================== */
static gboolean item_event_num (GooCanvasItem  *item,
				GooCanvasItem  *target,
				GdkEventButton *event,
				gpointer data)
{
  token *t = (token *)data;
  char str[12];

  if(board_paused)
    return FALSE;

  switch (event->type){
  case GDK_BUTTON_PRESS:
    gc_sound_play_ogg ("sounds/bleep.wav", NULL);
    if (t->isMoved) {
      if (item != ptr_token_selected[token_count-1]->item)
	return FALSE;
      // we put back in its original place a number item
      gc_item_absolute_move(item, t->xOffset_original, Y_NUM);
      token_count--;
      update_line_calcul();
      t->isMoved = FALSE;
    } else { // the item is at its original place
      if (token_count % 2 == 1 || token_count > 2*gcomprisBoard->level+1)
	return FALSE;
      token_count++;
      ptr_token_selected[token_count-1] = t;
      // some operations are illegal A - B must be > 0 and A/B an integer
      if (token_result() == NO_RESULT && token_count != 1) {
	token_count--;
	return FALSE;
      }

      gc_item_absolute_move(item, x_token_offset[token_count-1], y_token_offset[token_count-1]);
      t->isMoved = TRUE;

      // update result text items
      if (token_count != 1 && token_count % 2 == 1) {
	sprintf(str,"%d",token_result());

	g_object_set(calcul_line_item[token_count-3], "text", str, NULL);
	g_object_set(calcul_line_item_back[token_count-3], "text", str, NULL);

	if(token_count < 2*gcomprisBoard->level+1) {			/* No next line to update on last line */
	  g_object_set(calcul_line_item[token_count-2], "text", str, NULL);
	  g_object_set(calcul_line_item_back[token_count-2], "text", str, NULL);
	}

	gamewon = (result_to_find == token_result());
	if(gamewon)
	  process_time_id = gtk_timeout_add (2000, (GtkFunction) process_time, NULL);


      }
    }
    break;
  default : break;
  }
  return FALSE;
}
/* ======================================= */
static void destroy_board() {
  int i;

  for (i=0; i<NUM_VALUES; i++)
    gdk_pixbuf_unref(num_pixmap[i]);
  for (i=0; i<5; i++)
    gdk_pixbuf_unref(oper_pixmap[i]);
}
