/* gcompris - click_on_letter.c
 *
 * Copyright (C) 2001, 2010 Pascal Georges
 * Copyright (C) 2011 Bruno Coudoin (Mostly full rewrite)
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

#include <glib.h>
#include <string.h>

#include "gcompris/gcompris.h"



#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);
static gint		 key_press(guint keyval, gchar *commit_str,
				   gchar *preedit_str);

static int gamewon;

static void		 process_ok(void);
static void		 highlight_selected(GooCanvasItem *);
static void		 game_won(void);
static void		 repeat(void);
static void		 config_start(GcomprisBoard *agcomprisBoard,
				      GcomprisProfile *aProfile);
static void		 config_stop(void);

/* The data structure of a level */
typedef struct
{
  guint  level;
  gchar *questions;
  gchar *answers;
} Level;
static GArray *levels = NULL;
static gchar **questions;
static gchar **answers;

static void		 load_datafile();
static void		 clear_levels();

/* The data model of the configuration */
static GtkListStore *model;

enum
  {
    LEVEL_COLUMN,
    ANSWER_COLUMN,
    QUESTION_COLUMN,
    N_COLUMNS
  };

#define VERTICAL_SEPARATION 505
#define HORIZONTAL_SEPARATION -1

#define N_LETTER_PER_LINE 6
#define MAX_N_LETTER_LINE 3
#define MAX_N_ANSWER      (N_LETTER_PER_LINE * MAX_N_LETTER_LINE)

#define NOT_OK		0
#define OK		1
#define OK_NO_INIT	2

#define TEXT_COLOR "white"

static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem *selected_button = NULL;

static RsvgHandle *carriage_svg_handle;
static RsvgDimensionData carriage_svg_dimension;
static RsvgHandle *cloud_svg_handle;
static RsvgDimensionData cloud_svg_dimension;

static GooCanvasItem *click_on_letter_create_item(GooCanvasItem *parent);

static void click_on_letter_destroy_all_items(void);
static void click_on_letter_next_level(void);
static gint item_event(GooCanvasItem *item, GooCanvasItem *target,
		       GdkEvent *event, gpointer data);
static guint sounds_are_fine();

static int n_answer;
static gchar *right_letter = NULL;

static gchar *alphabet;

static void sound_played(gchar *file);

static gboolean uppercase_only;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Read a letter",
    "Learn to recognize letters",
    "Pascal Georges pascal.georges1@free.fr>",
    NULL,
    NULL,
    NULL,
    NULL,
    start_board,
    pause_board,
    end_board,
    is_our_board,
    key_press,
    NULL,
    set_level,
    NULL,
    repeat,
    config_start,
    config_stop
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(click_on_letter)

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
  GHashTable *config = gc_db_get_board_conf();
  guint ready;

  board_paused = TRUE;

  gc_locale_set(g_hash_table_lookup( config, "locale_sound"));

  g_hash_table_destroy(config);

  ready = sounds_are_fine();

  gc_sound_bg_pause();

  if (agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      if ( gcomprisBoard->mode && g_strcasecmp(gcomprisBoard->mode, "uppercase")==0 )
	uppercase_only = TRUE;
      else
	uppercase_only = FALSE;

      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			      "click_on_letter/background.svgz");

      gc_score_start(SCORESTYLE_NOTE,
		     50,
		     50,
		     0);

      load_datafile();

      gcomprisBoard->level=1;
      gcomprisBoard->sublevel=1;

      carriage_svg_handle = gc_rsvg_load("click_on_letter/carriage.svgz");
      rsvg_handle_get_dimensions (carriage_svg_handle, &carriage_svg_dimension);

      cloud_svg_handle = gc_rsvg_load("click_on_letter/cloud.svgz");
      rsvg_handle_get_dimensions (cloud_svg_handle, &cloud_svg_dimension);

      if(ready)
	{
	  if(ready == OK)
	    click_on_letter_next_level();

	  gamewon = FALSE;
	  pause_board(FALSE);
	}

      RsvgHandle *svg_handle;
      svg_handle = gc_rsvg_load("click_on_letter/repeat.svg");
      if(svg_handle) {
	gc_bar_set_repeat_icon(svg_handle);
	g_object_unref(svg_handle);
	gc_bar_set(GC_BAR_CONFIG|GC_BAR_LEVEL|GC_BAR_REPEAT_ICON);
      } else {
	gc_bar_set(GC_BAR_CONFIG|GC_BAR_LEVEL|GC_BAR_REPEAT);
      }
      gc_bar_location(BOARDWIDTH-300, 5, 0.7);

    }

}

static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str) {
  gint length_passed, i;

  if(!gcomprisBoard)
    return FALSE;

  /* i suppose even numbers are passed through IM_context */
  if ((commit_str == NULL) && (preedit_str == NULL))
    return FALSE;

  gchar *string_passed;
  if (commit_str)
    string_passed = commit_str;
  else
    string_passed = preedit_str;

  length_passed = g_utf8_strlen(string_passed, -1);

  for (i=0; i < length_passed; i++){
    gunichar ckey = \
      g_unichar_tolower( g_utf8_get_char (string_passed) );
    gunichar cright = \
      g_unichar_tolower( g_utf8_get_char (right_letter) );

    if (ckey == cright){
      gamewon = TRUE;
      process_ok();
      gc_im_reset();
      return TRUE;
    }

    string_passed = g_utf8_next_char (string_passed);
  }

  return TRUE;
}

/* ======================================= */
static void end_board ()
{
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gc_score_end();
      click_on_letter_destroy_all_items();
      g_strfreev(answers);
      g_strfreev(questions);

      g_object_unref(carriage_svg_handle);
      g_object_unref(cloud_svg_handle);
      clear_levels();
    }
  gc_locale_set( NULL );
  gcomprisBoard = NULL;
  gc_sound_bg_resume();
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
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
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
/*
 * return TRUE if the sound is found and the audio is activated
 *        FALSE if the sound was not played
 */
static gboolean _repeat ()
{
  gboolean retval = FALSE;

  gchar *str1 = NULL;
  gchar *right_letter_ogg = NULL;


  GcomprisProperties *properties = gc_prop_get();
  if( !properties->fx )
    return FALSE;

  str1 = gc_sound_alphabet(right_letter);

  right_letter_ogg = g_strdup_printf("voices/$LOCALE/alphabet/%s", str1);
  g_free(str1);

  if(right_letter_ogg) {

    /* Let's check the file exist to be abble to return FALSE */
    gchar *absolute_file = gc_file_find_absolute(right_letter_ogg, NULL);
    if (absolute_file)
      {
	gc_sound_play_ogg_cb(right_letter_ogg, sound_played);
	g_free(absolute_file);
	retval = TRUE;
      }
  }

  g_free(right_letter_ogg);

  return retval;
}

static void repeat ()
{
  if(gcomprisBoard!=NULL)
    {
      _repeat();
    }
}

static gchar *
get_alphabet()
{
  gchar *alphabet = gettext( _("abcdefghijklmnopqrstuvwxyz") );
  /* TRANSLATORS: Put here the alphabet in your language */
  g_assert(g_utf8_validate(alphabet, -1, NULL)); // require by all utf8-functions
  return alphabet;
}

static guint sounds_are_fine()
{
  char *letter_str;
  char *str2;
  GcomprisProperties *properties = gc_prop_get();
  gchar *text_mode_str = _("This activity will be played with questions displayed as text"
			   " instead of being spoken");

  if(!properties->fx)
    {
      gchar *msg = g_strconcat( _("Error: this activity cannot be played with the\n"
				  "sound effects disabled.\nGo to the configuration"
				  " dialog to\nenable the sound"),
			       "\n", text_mode_str, NULL);
      gc_dialog(msg, click_on_letter_next_level);
      g_free(msg);
      return(OK_NO_INIT);
    }

  alphabet = get_alphabet();

  gchar *letter = g_new0(gchar, 8);
  g_unichar_to_utf8(g_utf8_get_char(alphabet), letter);
  letter_str = gc_sound_alphabet(letter);
  g_free(letter);

  str2 = gc_file_find_absolute("voices/$LOCALE/alphabet/%s", letter_str);
  g_free(letter_str);

  if (!str2)
    {
      gchar *msg2 = g_strdup_printf( _("Error: this activity requires that you first install\nthe packages with GCompris voices for the %s locale."),
				     gc_locale_get_name( gc_locale_get() ) );
      gchar *msg = g_strconcat(msg2, "\n", text_mode_str, NULL);
      g_free(msg2);
      gc_dialog(msg, click_on_letter_next_level);
      g_free(msg);
      return (OK_NO_INIT);
    }

  return(OK);
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* set initial values for the next level */
static void
click_on_letter_next_level()
{
  gc_bar_set_level(gcomprisBoard);

  click_on_letter_destroy_all_items();
  gamewon = FALSE;
  selected_button = NULL;
  /* Try the next level */
  gc_sound_play_ogg("voices/$LOCALE/misc/click_on_letter.ogg", NULL);
  click_on_letter_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));
  gc_score_set_max(gcomprisBoard->number_of_sublevel);
  gc_score_set(gcomprisBoard->sublevel);
}
/* ==================================== */
/* Destroy all the items */
static void click_on_letter_destroy_all_items()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;

  g_free (right_letter);
  right_letter = NULL;
}

/*
 * given an utf8 string, return an array of pointers to string
 * where each string is a char from the original string.
 * All the char in the returned array are shuffled
 * free the result with g_strfreev()
 */
static gchar
**shuffle_utf8(gchar *string)
{
  guint n_letters = g_utf8_strlen (string, -1);
  gchar **result = (gchar **) g_new( gpointer, n_letters + 1);
  /* Randomize the list, create a random index first */
  int random[n_letters];
  int i;
  for ( i = 0 ; i < n_letters ; i++) {
    random[i] = i;
  }

  for ( i = 0 ; i < n_letters ; i++) {
    int swap_index = g_random_int_range(0, n_letters);
    int save = random[i];
    random[i] = random[swap_index];
    random[swap_index] = save;
  }

  /* Now use the index to swap letter */
  for ( i = 0 ; i < n_letters ; i++) {
    gchar *copy_from = g_utf8_offset_to_pointer(string, random[i]);
    gchar *copy_to = g_utf8_offset_to_pointer(string, random[i] + 1);
    result[i] = g_strndup(copy_from, copy_to - copy_from);
  }
  result[i] = NULL;
  return result;
}

/* ==================================== */
static GooCanvasItem *click_on_letter_create_item(GooCanvasItem *parent)
{

  int xOffset, yOffset, i;

  if (gcomprisBoard->sublevel == 1)
    {
      Level *level = &g_array_index (levels, Level, gcomprisBoard->level - 1);
      n_answer = g_utf8_strlen (level->answers, -1);
      g_assert( n_answer <= MAX_N_ANSWER );

      if ( uppercase_only )
	{
	  gchar *answers_up = g_utf8_strup( level->answers, -1 );
	  gchar *questions_up = g_utf8_strup( level->questions, -1 );
	  answers = shuffle_utf8(answers_up);
	  questions = shuffle_utf8(questions_up);
	  g_free(answers_up);
	  g_free(questions_up);
	}
      else
	{
	  answers = shuffle_utf8(level->answers);
	  questions = shuffle_utf8(level->questions);
	}

      /* Go to next level after this number of 'play' */
      gcomprisBoard->number_of_sublevel = g_utf8_strlen (level->questions, -1);
    }
  right_letter = g_utf8_strdown(questions[gcomprisBoard->sublevel - 1], -1);


  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);

  if ( ! _repeat() )
  {
    /* Sound was not played, display the letter to find instead */
    guint x = 300;
    guint y = 30;
    guint width = 100;
    guint height = 100;
    goo_canvas_rect_new (boardRootItem,
			 x,
			 y,
			 width,
			 height,
			 "stroke_color_rgba", 0xFFFFFFFFL,
			 "fill_color_rgba", 0x00005550L,
			 "line-width", (double) 2,
			 "radius-x", (double) 10,
			 "radius-y", (double) 10,
			 NULL);
    goo_canvas_text_new (boardRootItem,
			 questions[gcomprisBoard->sublevel - 1],
			 (double) x + width / 2,
			 (double) y + height / 2,
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_board_huge_bold,
			 "fill_color_rgba", 0xffffffff,
			 NULL);
  }

  /* Display the answers */
  yOffset = VERTICAL_SEPARATION - carriage_svg_dimension.height;
  xOffset = 144;
  gint text_gap_x = -5;
  gint text_gap_y = -35;

  RsvgHandle *svg_handle= carriage_svg_handle;
  RsvgDimensionData svg_dimension = carriage_svg_dimension;

  for (i = 0; i< n_answer; i++) {

    if ( i > 0 && i % N_LETTER_PER_LINE == 0 )
      {
	/* Line wrapping */
	svg_handle= cloud_svg_handle;
	svg_dimension = cloud_svg_dimension;
	xOffset = 144;
	yOffset -= svg_dimension.height;
	text_gap_x = 0;
	text_gap_y = 0;
      }

    GooCanvasItem *button_item =		\
      goo_canvas_svg_new (boardRootItem,
			  svg_handle,
			  "svg-id", "#OFF",
			  NULL);
    goo_canvas_item_translate( button_item,
			       xOffset,
			       yOffset);


    GooCanvasItem *text_item = \
      goo_canvas_text_new (boardRootItem,
			   answers[i],
			   (double) xOffset + svg_dimension.width / 2 + text_gap_x,
			   (double) yOffset + svg_dimension.height / 2 + text_gap_y,
			   -1,
			   GTK_ANCHOR_CENTER,
			   "font", gc_skin_font_board_huge_bold,
			   "fill_color_rgba", 0x000000ff,
			   NULL);

    xOffset += HORIZONTAL_SEPARATION + svg_dimension.width;

    g_signal_connect(text_item, "button_press_event",
		     (GCallback) item_event, answers[i]);
    g_signal_connect(button_item, "button_press_event",
		     (GCallback) item_event, answers[i]);
    gc_item_focus_init(text_item, button_item);
    gc_item_focus_init(button_item, NULL);
    g_object_set_data(G_OBJECT(button_item), "button_item", button_item);
    g_object_set_data(G_OBJECT(text_item), "button_item", button_item);
  }

  return NULL;
}
/* ==================================== */
static void game_won()
{
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel > gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level > gcomprisBoard->maxlevel)
      gcomprisBoard->level = gcomprisBoard->maxlevel;
  }
  click_on_letter_next_level();
}

/* ==================================== */
static gboolean process_ok_timeout() {
  gc_bonus_display(gamewon, GC_BONUS_FLOWER);
  return FALSE;
}

static void process_ok() {
  // leave time to display the right answer
  g_timeout_add(TIME_CLICK_TO_BONUS, process_ok_timeout, NULL);
}
/* ==================================== */
static gint
item_event(GooCanvasItem *item, GooCanvasItem *target,
	   GdkEvent *event, gpointer data)
{
  gchar *answer = (gchar*)data;

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      /* We really don't want the user to change his/her mind */
      board_paused = TRUE;

      if ( strcmp(answer, right_letter) == 0 ) {
	gamewon = TRUE;
      } else {
	gamewon = FALSE;
      }
      highlight_selected(item);
      process_ok();
      break;

    default:
      break;
    }
  return FALSE;
}
/* ==================================== */
static void highlight_selected(GooCanvasItem * item) {
  GooCanvasItem *button;

  button = (GooCanvasItem*)g_object_get_data(G_OBJECT(item), "button_item");

  if (selected_button != NULL && selected_button != button) {
    g_object_set(selected_button, "svg-id", "#OFF",  NULL);
  }

  if (selected_button != button) {
    g_object_set(button, "svg-id", "#ON",  NULL);
    selected_button = button;
  }

}

/*
 * Management of Data File (Desktop style)
 */

static void load_desktop_datafile(gchar *filename)
{
  printf("load_desktop_datafile %s\n", filename);
  GKeyFile *keyfile = g_key_file_new ();
  GError *error = NULL;
  if ( ! g_key_file_load_from_file (keyfile,
				    filename,
				    G_KEY_FILE_NONE,
				    &error)  ) {
    if (error)
      g_error ("%s", error->message);
    return;
  }

  gsize n_level;
  gchar **groups = g_key_file_get_groups (keyfile, &n_level);
  int i;
  for (i=0; i<n_level; i++)
    {
      Level level;
      level.level = i + 1;
      level.questions = g_key_file_get_string (keyfile, groups[i],
					      "Questions", NULL);
      level.answers = g_key_file_get_string (keyfile, groups[i],
					    "Answers", NULL);
      g_array_append_vals (levels, &level, 1);
    }

  g_strfreev(groups);
  gcomprisBoard->maxlevel = n_level;
}

static void create_level_from_alphabet(gchar *alphabet)
{
  guint alphabet_len = g_utf8_strlen (alphabet, -1);

  guint level_i = 0;
  guint i = 0;
  while ( TRUE )
    {
      Level level;
      level.level = ++level_i;

      guint n_questions = level_i + 3;

      if ( i + n_questions > alphabet_len)
	{
	  n_questions = MAX_N_ANSWER;
	  i = alphabet_len - n_questions;
	}
      gchar *copy_from = g_utf8_offset_to_pointer(alphabet, i);
      gchar *copy_to = g_utf8_offset_to_pointer(alphabet, i + n_questions);

      i += n_questions;

      level.questions = g_strndup(copy_from, copy_to - copy_from);
      level.answers = g_strdup(level.questions);
      g_array_append_vals (levels, &level, 1);

      if ( i >= alphabet_len)
	break;
    }

  gcomprisBoard->maxlevel = level_i;

}

/*
 * Load a desktop style data file and create the levels array
 */
static void load_datafile() {
  gchar *filename = gc_file_find_absolute("click_on_letter/default-$LOCALE.desktop");

  clear_levels();

  // Reset the alphabet to match the current locale
  alphabet = get_alphabet();

  /* create level array */
  levels = g_array_sized_new (FALSE, FALSE, sizeof (Level), 10);

  if ( filename )
    {
      load_desktop_datafile(filename);
    }
  else if ( ! filename && alphabet[0] == 'a')
    {
      /* This is a LATIN based language, let's fallback to english */
      filename = gc_file_find_absolute("click_on_letter/default-en.desktop");

      if( filename )
	load_desktop_datafile(filename);
      else
	// Should not happens but in case let's create the level for LATIN
	create_level_from_alphabet(alphabet);
    }
  else
    {
      // No data file and no latin character set
      create_level_from_alphabet(alphabet);
    }

  g_free(filename);
}

/**
 * Based on the levels array, returns a desktop file
 * free the returned value with g_free()
 */
static gchar *levels_to_desktop() {
  GKeyFile *keyfile = g_key_file_new ();

  int i;
  for (i = 0; i < levels->len; i++)
    {
      Level level = g_array_index (levels, Level, i);
      gchar *group = g_strdup_printf("%d", level.level);
      g_key_file_set_string(keyfile, group, "Questions", level.questions);
      g_key_file_set_string(keyfile, group, "Answers", level.answers);
      g_free(group);
    }

  gchar *buffer = g_key_file_to_data(keyfile, NULL, NULL);
  return buffer;
}

/* ************************************* */
/* *            Configuration          * */
/* ************************************* */


/* ======================= */
/* = config_start        = */
/* ======================= */

static GcomprisProfile *profile_conf;
static GcomprisBoard   *board_conf;

static GHFunc save_table (gpointer key,
			  gpointer value,
			  gpointer user_data)
{
  gc_db_set_board_conf ( profile_conf,
			    board_conf,
			    (gchar *) key,
			    (gchar *) value);

  return NULL;
}

static gboolean _save_level_from_model(GtkTreeModel *model, GtkTreePath *path,
				       GtkTreeIter *iter, gpointer data)
{
  Level level;

  gtk_tree_model_get (GTK_TREE_MODEL (model), iter,
                      LEVEL_COLUMN, &level.level,
                      ANSWER_COLUMN, &level.answers,
                      QUESTION_COLUMN, &level.questions,
                      -1);
  g_array_append_vals (levels, &level, 1);
  gcomprisBoard->maxlevel = level.level;

  return FALSE;
}


static void create_levels_from_config_model()
{
  GtkTreeIter iter;
  gtk_tree_model_get_iter_first (GTK_TREE_MODEL(model), &iter );

  clear_levels();
  levels = g_array_sized_new (FALSE, FALSE, sizeof (Level), 10);
  gtk_tree_model_foreach(GTK_TREE_MODEL(model), _save_level_from_model, NULL);
}

void
load_model_from_levels(GtkListStore *model)
{
  GtkTreeIter iter;

  gtk_list_store_clear(model);
  guint i;
  for ( i = 0; i < levels->len; i++)
    {
      Level level = g_array_index (levels, Level, i);
      gtk_list_store_append(model, &iter);
      gtk_list_store_set(model, &iter,
			 LEVEL_COLUMN, level.level,
			 ANSWER_COLUMN, level.answers,
			 QUESTION_COLUMN, level.questions,
			 -1);
    }
}

void
clear_levels()
{
  if ( ! levels )
    return;

  gint i = 0;
  for (i = 0; i < levels->len; i++)
    {
      Level level = g_array_index (levels, Level, i);
      g_free(level.answers);
      g_free(level.questions);
    }

  g_array_free(levels, TRUE);
  levels = NULL;
}

static gboolean
valid_entry(Level *level)
{
  gboolean result=FALSE;
  gchar *error;
  GtkWidget *dialog;

  g_assert(level->questions);
  g_assert(level->answers);

  if ( strlen(level->questions) == 0 )
    {
      error = g_strdup (_("Questions cannot be empty.") );
      goto error;
    }

  /* Now check all chars in questions are in answers */
  guint n_answers = g_utf8_strlen (level->answers, -1);
  guint n_questions = g_utf8_strlen (level->questions, -1);

  if ( n_answers == 0 )
    {
      error = g_strdup( _("Answers cannot be empty.") );
      goto error;
    }

  if ( n_answers > MAX_N_ANSWER )
    {
      error = g_strdup_printf( _("Too many characters in the Answer (maximum is %d)."),
				 MAX_N_ANSWER );
      goto error;
    }

  /* The shuffle is not important, we are using it to get an array of chars */
  gchar **answers = shuffle_utf8(level->answers);
  gchar **questions = shuffle_utf8(level->questions);

  guint a;
  guint q;
  for ( q = 0; q < n_questions; q++)
    {
      gboolean found = FALSE;
      for ( a = 0; a < n_answers; a++)
	{
	  if ( strcmp( answers[a], questions[q] ) == 0 )
	    {
	      found = TRUE;
	      break;
	    }
	}
      if ( ! found )
	{
	  error = g_strdup ( _("All the characters in Questions must also be in the Answers.") );
	  g_strfreev(questions);
	  g_strfreev(answers);
	  goto error;
	}
    }

  g_strfreev(questions);
  g_strfreev(answers);
  return TRUE;

 error:
  dialog = \
    gtk_message_dialog_new (NULL,
			    GTK_DIALOG_DESTROY_WITH_PARENT,
			    GTK_MESSAGE_ERROR,
			    GTK_BUTTONS_CLOSE,
			    _("Invalid entry:\n"
			      "At level %d, Questions '%s' / Answers '%s'\n%s"),
			    level->level, level->questions, level->answers,
			    error);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
  g_free(error);

  return result;
}

static gboolean
_check_errors(GtkTreeModel *model, GtkTreePath *path,
	      GtkTreeIter *iter, gpointer data)
{
  Level level;
  gboolean *has_error = (gboolean*)data;

  gtk_tree_model_get (model, iter,
                      LEVEL_COLUMN, &level.level,
                      QUESTION_COLUMN, &level.questions,
                      ANSWER_COLUMN, &level.answers,
                      -1);

  if( ! valid_entry( &level ) )
    {
      *has_error = TRUE;
      /* Don't check more errors */
      return TRUE;
    }

  return FALSE;
}


/**
 * Based on the current locale, return the user desktop filename
 * to use.
 * The returned value must be freed
 */
gchar *get_user_desktop_file()
{
  gchar **locale = g_strsplit_set(gc_locale_get(), ".", 2);
  gchar *filename =
    gc_file_find_absolute_writeable("%s/default-%s.desktop",
				    gcomprisBoard->boarddir,
				    locale[0]);
  g_strfreev(locale);
  return filename;
}

static gboolean
conf_ok(GHashTable *table)
{
  if (!table){
    if (gcomprisBoard)
      pause_board(FALSE);

    return TRUE;
  }

  g_hash_table_foreach(table, (GHFunc) save_table, NULL);

  board_conf = NULL;
  profile_conf = NULL;

  if (gcomprisBoard){
    GHashTable *config;
    if (profile_conf)
      config = gc_db_get_board_conf();
    else
      config = table;

    gboolean has_error = FALSE;
    gtk_tree_model_foreach(GTK_TREE_MODEL(model), _check_errors, &has_error);

    /* Tell the config not to close the dialog to let the user fix the issues */
    if (has_error)
      return FALSE;


    printf("conf_ok =%s\n", (char*)g_hash_table_lookup(config, "locale_sound"));
    gc_locale_set(g_hash_table_lookup(config, "locale_sound"));

    if (profile_conf)
      g_hash_table_destroy(config);

    guint ready = sounds_are_fine();

    /*
     * Save the new editied value if needed
     */
    /* Keep a cache of the old model */
    gchar *old_levels = levels_to_desktop();
    create_levels_from_config_model();
    gchar *new_levels = levels_to_desktop();

    if ( gcomprisBoard->level > gcomprisBoard->maxlevel )
      gcomprisBoard->level = 1;

    if (strcmp(old_levels, new_levels) != 0)
      {
	/* The level has changed, save the new desktop file in the user's dir */
	gchar *filename = get_user_desktop_file();
	g_file_set_contents(filename, new_levels, -1, NULL);
	g_free(filename);
      }
    g_free(old_levels);
    g_free(new_levels);

    if(ready)
      {
	if(ready == OK)
	  click_on_letter_next_level();

	gamewon = FALSE;
	pause_board(FALSE);
      }

  }

  board_conf = NULL;
  profile_conf = NULL;
  return TRUE;
}

static gboolean
resequence_level_in_model(GtkTreeModel *model, GtkTreePath *path,
			  GtkTreeIter *iter, gpointer data)
{
  guint *level = (guint*)data;

  gtk_list_store_set (GTK_LIST_STORE(model), iter,
                      LEVEL_COLUMN, (*level)++,
                      -1);

  return FALSE;
}

static gboolean next_level_in_model(GtkTreeModel *model, GtkTreePath *path,
				    GtkTreeIter *iter, gpointer data)
{
  guint *level = (guint*)data;
  guint level_in_tree;

  gtk_tree_model_get (model, iter,
                      LEVEL_COLUMN, &level_in_tree,
                      -1);

  if ( level_in_tree >= *level)
    *level = level_in_tree + 1;

  return FALSE;
}

static void
add_item (GtkWidget *button, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeModel *model = (GtkTreeModel *)data;

  guint next_level = 0;
  gtk_tree_model_foreach(model, next_level_in_model, &next_level);

  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      LEVEL_COLUMN, next_level,
                      ANSWER_COLUMN, "",
                      QUESTION_COLUMN, "",
                      -1);
}

static void
remove_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeView *treeview = (GtkTreeView *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (treeview);
  GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview);

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
    }

  guint next_level = 1;
  gtk_tree_model_foreach(model, resequence_level_in_model, &next_level);

}

static void
move_item (GtkWidget *widget, gpointer data, gboolean up)
{
  GtkTreeIter iter;
  GtkTreeView *treeview = (GtkTreeView *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (treeview);
  GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview);

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gint i;
      GtkTreePath *path;

      path = gtk_tree_model_get_path (model, &iter);
      i = gtk_tree_path_get_indices (path)[0];
      if (up) {
	if ( i > 0)
	  i--;
      } else {
	if ( i < gtk_tree_model_iter_n_children(model, NULL) - 1 )
	  i++;
      }

      GtkTreePath *dst_path = gtk_tree_path_new_from_indices (i, -1);

      GtkTreeIter dst_iter;
      gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &dst_iter, dst_path);


      if (up)
	gtk_list_store_move_before (GTK_LIST_STORE(model), &iter, &dst_iter);
      else
	gtk_list_store_move_after (GTK_LIST_STORE(model), &iter, &dst_iter);

      gtk_tree_path_free (path);
      gtk_tree_path_free (dst_path);
    }

  guint next_level = 1;
  gtk_tree_model_foreach(model, resequence_level_in_model, &next_level);

}

static void
up_item (GtkWidget *widget, gpointer data)
{
  move_item(widget, data, TRUE);
}

static void
down_item (GtkWidget *widget, gpointer data)
{
  move_item(widget, data, FALSE);
}

static void
return_to_default(GtkWidget *widget, gpointer data)
{
  GtkListStore *model = (GtkListStore *)data;
  gchar *filename = get_user_desktop_file();
  /* Erase the user desktop file */
  gc_cache_remove(filename);
  g_free(filename);

  load_datafile();
  load_model_from_levels(model);

}


static void cell_edited_callback (GtkCellRendererText *cell,
			   gchar               *path,
			   gchar               *new_text,
			   GtkTreeView         *tree_view) {
  guint column_number = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(cell), "my_column_num"));

  GtkTreeIter iter;
  GtkListStore *model = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view)));
  gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(model),&iter,path);
  if (column_number == 0)
    gtk_list_store_set(model, &iter, column_number, atoi(new_text), -1);
  else
    gtk_list_store_set(model, &iter, column_number, new_text, -1);
}

static void configure_colummns(GtkTreeView *treeview)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  /* Level column */
  renderer = gtk_cell_renderer_text_new();
  g_object_set_data(G_OBJECT(renderer), "my_column_num",  GUINT_TO_POINTER(LEVEL_COLUMN) );

  column = gtk_tree_view_column_new_with_attributes(_("Level"),
                                                    renderer,
						    "text", LEVEL_COLUMN,
						    NULL);
  gtk_tree_view_append_column(treeview, column);

  /* Answer column */
  renderer = gtk_cell_renderer_text_new();
  g_object_set(renderer, "editable", TRUE, NULL);
  g_object_set_data(G_OBJECT(renderer), "my_column_num",  GUINT_TO_POINTER(ANSWER_COLUMN) );
  g_signal_connect(renderer, "edited", (GCallback) cell_edited_callback, treeview);
  column = gtk_tree_view_column_new_with_attributes(_("Answer"),
                                                    renderer,
						    "text", ANSWER_COLUMN,
						    NULL);
  gtk_tree_view_append_column(treeview, column);

  /* Question column */
  renderer = gtk_cell_renderer_text_new();
  g_object_set(renderer, "editable", TRUE, NULL);
  g_object_set_data(G_OBJECT(renderer), "my_column_num",  GUINT_TO_POINTER(QUESTION_COLUMN) );
  g_signal_connect(renderer, "edited", (GCallback) cell_edited_callback, treeview);
  column = gtk_tree_view_column_new_with_attributes(_("Question"),
                                                    renderer,
						    "text", QUESTION_COLUMN,
						    NULL);
  gtk_tree_view_append_column(treeview, column);

}


static void
locale_changed (GtkComboBox *combobox, gpointer data)
{
  const gchar *locale;
  GtkTreeIter iter;
  gchar *text = NULL;

  if (gtk_combo_box_get_active_iter (combobox, &iter))
    gtk_tree_model_get (gtk_combo_box_get_model (combobox), &iter,
			0, &text, -1);

  // Get back the locale from the locale name (French becomes fr_FR.UTF8)
  locale = gc_locale_get_locale( text );
  printf("LOCALE = %s\n", locale);

  gc_locale_set(locale);

  load_datafile();
  load_model_from_levels(model);

  // Our job is done, set back the default locale
  gc_locale_set( NULL );
}


static void
config_start(GcomprisBoard *agcomprisBoard,
	     GcomprisProfile *aProfile)
{
  board_conf = agcomprisBoard;
  profile_conf = aProfile;

  if (gcomprisBoard)
    pause_board(TRUE);

  gc_locale_set( NULL );

  gchar *label = g_strdup_printf(_("<b>%1$s</b> configuration\n for profile <b>%2$s</b>"),
				 agcomprisBoard->name,
				 aProfile ? aProfile->name : "");
  GcomprisBoardConf *bconf;
  bconf = gc_board_config_window_display(label, conf_ok);

  g_free(label);

  /* init the combo to previously saved value */
  GHashTable *config = gc_db_get_conf( profile_conf, board_conf);

  gchar *saved_locale_sound = g_hash_table_lookup( config, "locale_sound");

  gc_board_config_combo_locales_asset(bconf, "Select sound locale", saved_locale_sound,
				      "voices/$LOCALE/colors/purple.ogg",
				      G_CALLBACK (locale_changed));

  /* frame */
  GtkWidget *frame = gtk_frame_new("");
  gtk_widget_show(frame);
  gtk_box_pack_start(GTK_BOX(bconf->main_conf_box),
		     frame, TRUE, TRUE, 8);

  GtkWidget *vbox = gtk_vbox_new(FALSE, 8);
  gtk_widget_show(vbox);
  gtk_container_add(GTK_CONTAINER(frame), vbox);

  /* list view */
  GtkListStore *list = gtk_list_store_new(N_COLUMNS,
					  G_TYPE_UINT,    /* Level */
					  G_TYPE_STRING,  /* Answers */
					  G_TYPE_STRING   /* Questions */
					  );

  GtkWidget *treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list));
  configure_colummns(GTK_TREE_VIEW(treeview));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), LEVEL_COLUMN);
  gtk_widget_set_size_request(treeview, -1, 200);
  gtk_widget_show(treeview);

  GtkScrolledWindow *scroll = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL,NULL));
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_show(GTK_WIDGET(scroll));
  gtk_container_add(GTK_CONTAINER(scroll), treeview);

  gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(scroll), TRUE, TRUE, 10);

  GtkTreeSelection *selection;
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

  model = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));

  load_model_from_levels(model);

  /* some buttons */
  GtkWidget *hbox = gtk_hbox_new (TRUE, 4);
  gtk_widget_show(hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  GtkWidget *button = gtk_button_new_from_stock(GTK_STOCK_NEW);
  gtk_widget_show(button);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (add_item), model);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock(GTK_STOCK_DELETE);
  gtk_widget_show(button);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (remove_item), treeview);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock(GTK_STOCK_GO_UP);
  gtk_widget_show(button);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (up_item), treeview);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);
  gtk_widget_show(button);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (down_item), treeview);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  hbox = gtk_hbox_new (TRUE, 4);
  gtk_widget_show(hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label(_("Back to default"));
  gtk_widget_show(button);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (return_to_default), model);
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);


  g_hash_table_destroy(config);
}


/* ======================= */
/* = config_stop        = */
/* ======================= */
static void
config_stop()
{
}

static void
sound_played (gchar *file)
{
  g_warning ("Sound_played %s\n", file);
}
