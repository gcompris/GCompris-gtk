/* gcompris - click_on_letter.c
 *
 * Copyright (C) 2001 Pascal Georges
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

static int gamewon;

static void		 process_ok(void);
static void		 highlight_selected(GooCanvasItem *);
static void		 game_won(void);
static void		 repeat(void);
static void		 config_start(GcomprisBoard *agcomprisBoard,
				      GcomprisProfile *aProfile);
static void		 config_stop(void);


#define VERTICAL_SEPARATION 408
#define HORIZONTAL_SEPARATION 0

#define NUMBER_OF_SUBLEVELS 3
#define NUMBER_OF_LEVELS 5
#define MAX_NUMBER_OF_LETTERS 4

#define NOT_OK		0
#define OK		1
#define OK_NO_INIT	2

#define TEXT_COLOR "white"

static GooCanvasItem *boardRootItem = NULL;

static GooCanvasItem *l_items[MAX_NUMBER_OF_LETTERS];
static GooCanvasItem *buttons[MAX_NUMBER_OF_LETTERS];
static GooCanvasItem *selected_button = NULL;

static GooCanvasItem *click_on_letter_create_item(GooCanvasItem *parent);

static void click_on_letter_destroy_all_items(void);
static void click_on_letter_next_level(void);
static gint item_event(GooCanvasItem *item, GooCanvasItem *target,
		       GdkEvent *event, gpointer data);
static gboolean sounds_are_fine();

static int right_position;
static int number_of_letters=MAX_NUMBER_OF_LETTERS;
static gchar *right_letter;

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
    NULL,
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
  int ready;

  board_paused = TRUE;

  gc_locale_set(g_hash_table_lookup( config, "locale_sound"));

  gchar *up_init_str = g_hash_table_lookup( config, "uppercase_only");

  if (up_init_str && (strcmp(up_init_str, "True")==0))
    uppercase_only = TRUE;
  else
    uppercase_only = FALSE;

  g_hash_table_destroy(config);

  ready = sounds_are_fine();

  gc_sound_bg_pause();

  if (agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			      "click_on_letter/scenery4_background.png");
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=NUMBER_OF_LEVELS;
      gcomprisBoard->sublevel=1;

      /* Go to next level after this number of 'play' */
      gcomprisBoard->number_of_sublevel=NUMBER_OF_SUBLEVELS;

      if(ready)
	{
	  /* Warning, bar buttons are set in click_on_letter_next_level()
	     to avoid them to appear in the case a dialog is displayed */
	  gc_score_start(SCORESTYLE_NOTE,
			       50,
			       50,
			       gcomprisBoard->number_of_sublevel);

	  if(ready == OK)
	    click_on_letter_next_level();

	  gamewon = FALSE;
	  pause_board(FALSE);
	}
    }

}
/* ======================================= */
static void end_board ()
{
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gc_score_end();
      click_on_letter_destroy_all_items();
    }
  gc_locale_reset();
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
static void repeat ()
{
  if(gcomprisBoard!=NULL)
    {
      gchar *str1 = NULL;
      gchar *right_letter_ogg = NULL;

      str1 = gc_sound_alphabet(right_letter);

      right_letter_ogg = g_strdup_printf("voices/$LOCALE/alphabet/%s", str1);
      g_free(str1);

      if(right_letter_ogg) {
	gc_sound_play_ogg_cb(right_letter_ogg, sound_played);
      }

      g_free(right_letter_ogg);
    }
}

static gboolean sounds_are_fine()
{
  char *letter_str;
  char *str2;
  GcomprisProperties	*properties = gc_prop_get();

  if(!properties->fx)
    {
      gc_dialog(_("Error: this activity cannot be played with the\nsound effects disabled.\nGo to the configuration dialog to\nenable the sound"), gc_board_stop);
      return(NOT_OK);
    }

  /* TRANSLATORS: Put here the alphabet in your language */
  alphabet=_("abcdefghijklmnopqrstuvwxyz");
  g_assert(g_utf8_validate(alphabet, -1, NULL)); // require by all utf8-functions

  gchar *letter = g_new0(gchar, 8);
  g_unichar_to_utf8(g_utf8_get_char(alphabet), letter);
  letter_str = gc_sound_alphabet(letter);
  g_free(letter);

  str2 = gc_file_find_absolute("voices/$LOCALE/alphabet/%s", letter_str);

  if (!str2)
    {
      gchar *locale = NULL;

      locale = g_strndup(gc_locale_get(), 2);
      gc_locale_reset();
      gc_locale_set("en_US");

      str2 = gc_file_find_absolute("voices/en/alphabet/%s", letter_str);

      if (!str2)
	{
	  gchar *msg = g_strdup_printf( _("Error: this activity requires that you first install\nthe packages with GCompris voices for the locale '%s' or '%s'"),
					locale, "en");
	  gc_dialog(msg, gc_board_stop);
	  g_free(msg);
	  return (NOT_OK);
	}
      else
	{
	  gchar *msg = g_strdup_printf( _("Error: this activity requires that you first install\nthe packages with GCompris voices for the locale '%s' ! Fallback to english, sorry!"), locale);
	  gc_dialog(msg, click_on_letter_next_level);
	  g_free(msg);
	  g_free(str2);
	  return(OK_NO_INIT);
	}
    }
  else
    {
      g_free(str2);
    }

  return(OK);
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* set initial values for the next level */
static void
click_on_letter_next_level()
{
  /* It must be set it for the warning dialogs */
  gc_bar_set(GC_BAR_CONFIG|GC_BAR_LEVEL|GC_BAR_REPEAT);

  gc_bar_set_level(gcomprisBoard);

  click_on_letter_destroy_all_items();
  gamewon = FALSE;
  selected_button = NULL;
  gc_score_set(gcomprisBoard->sublevel);
  g_free (right_letter);
  /* Try the next level */
  gc_sound_play_ogg("voices/$LOCALE/misc/click_on_letter.ogg", NULL);
  click_on_letter_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));
}
/* ==================================== */
/* Destroy all the items */
static void click_on_letter_destroy_all_items()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}
/* ==================================== */
static GooCanvasItem *click_on_letter_create_item(GooCanvasItem *parent)
{

  int xOffset,yOffset,i,j;
  GdkPixbuf *button_pixmap = NULL;

  int length_of_aphabet=g_utf8_strlen (alphabet,-1);

  number_of_letters=gcomprisBoard->level+1;
  if (number_of_letters>MAX_NUMBER_OF_LETTERS) number_of_letters=MAX_NUMBER_OF_LETTERS;

  int numbers[number_of_letters];

  gchar *letters[number_of_letters];
  g_assert(number_of_letters<=length_of_aphabet); // because we must set unique letter on every "vagon"

  for (i=0;i<number_of_letters;i++){
    numbers[i]= g_random_int_range(0, length_of_aphabet);

    // check that the letter has not been taken yet
    for(j=0;j<i;j++){
      if (numbers[i]==numbers[j]) {
	i--;
	continue;
      }
    }

  }

  for (i=0;i<number_of_letters;i++){
    gchar *copy_from=g_utf8_offset_to_pointer(alphabet, numbers[i]);
    gchar *copy_to=g_utf8_offset_to_pointer(alphabet, numbers[i]+1);
    letters[i]=g_strndup(copy_from,copy_to-copy_from);

    if (uppercase_only)
      letters[i]=g_utf8_strup(copy_from,copy_to-copy_from);
    else {
      switch (gcomprisBoard->level) {
      case 1	:
      case 2  : letters[i]=g_strndup(copy_from,copy_to-copy_from); break;
      case 3  : letters[i]=g_utf8_strup(copy_from,copy_to-copy_from); break;
      default :
	if ( g_random_boolean() )
	  letters[i]=g_strndup(copy_from,copy_to-copy_from);
	else
	  letters[i]=g_utf8_strup(copy_from,copy_to-copy_from);
      }
    }
  }

  /*  */
  right_position = g_random_int_range(0,number_of_letters);
  g_assert(right_position >= 0  && right_position < number_of_letters);
  right_letter = g_utf8_strdown(letters[right_position],-1);

  repeat();


  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);



  button_pixmap = gc_pixmap_load("click_on_letter/wagon-yellow.png");

  yOffset = VERTICAL_SEPARATION;
  xOffset = 5;


  for (i=0; i< number_of_letters; i++) {
    buttons[i] = goo_canvas_image_new (boardRootItem,
				       button_pixmap,
				       xOffset,
				       yOffset,
				       NULL);


    l_items[i] = goo_canvas_text_new (boardRootItem,
				      letters[i],
				      (double) xOffset + gdk_pixbuf_get_width(button_pixmap)/2,
				      (double) yOffset + gdk_pixbuf_get_height(button_pixmap)/2 - 5,
				      -1,
				      GTK_ANCHOR_CENTER,
				      "font", gc_skin_font_board_huge_bold,
				      "fill_color_rgba", 0x0000ffff,
				      NULL);

    g_free(letters[i]);
    xOffset +=HORIZONTAL_SEPARATION +gdk_pixbuf_get_width(button_pixmap);

    g_signal_connect(l_items[i], "button_press_event",
		     (GtkSignalFunc) item_event, GINT_TO_POINTER(i));
    g_signal_connect(buttons[i], "button_press_event",
		     (GtkSignalFunc) item_event, GINT_TO_POINTER(i));
    //  g_signal_connect(GTK_OBJECT(buttons[i]), "enter_notify_event", (GtkSignalFunc) gc_item_focus_event, NULL);
  }


  gdk_pixbuf_unref(button_pixmap);

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
      gc_bonus_end_display(GC_BOARD_FINISHED_TUXPLANE);
      return;
    }
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
  int pos = GPOINTER_TO_INT(data);
  double item_x, item_y;
  item_x = event->button.x;
  item_y = event->button.y;
  //goo_canvas_convert_to_item_space(item->parent, &item_x, &item_y);

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      /* We really don't want the user to change his/her mind */
      board_paused = TRUE;

      if ( pos == right_position ) {
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
  GdkPixbuf *button_pixmap_selected = NULL, *button_pixmap = NULL;
  GooCanvasItem *button;
  int i;


  /* Replace text item by button item */
  button = item;
  for (i=0; i<number_of_letters;i++) {
    if ( l_items[i] == item ) {
      button = buttons[i];
    }
  }

  if (selected_button != NULL && selected_button != button) {
    button_pixmap = gc_pixmap_load("click_on_letter/wagon-yellow.png");
    /* Warning changing the image needs to update pixbuf_ref for the focus usage */
    gc_item_focus_free(selected_button, NULL);
    g_object_set(selected_button, "pixbuf", button_pixmap, NULL);
    gdk_pixbuf_unref(button_pixmap);
  }

  if (selected_button != button) {
    button_pixmap_selected = gc_pixmap_load("click_on_letter/wagon-green.png");
    /* Warning changing the image needs to update pixbuf_ref for the focus usage */
      gc_item_focus_free(button, NULL);
    g_object_set(button, "pixbuf", button_pixmap_selected, NULL);
    selected_button = button;
    gdk_pixbuf_unref(button_pixmap_selected);
  }

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

static void
conf_ok(GHashTable *table)
{
  if (!table){
    if (gcomprisBoard)
      pause_board(FALSE);

    return;
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

    gc_locale_reset();
    gc_locale_set(g_hash_table_lookup(config, "locale_sound"));

    gchar *up_init_str = g_hash_table_lookup( config, "uppercase_only");
    if (up_init_str)
      {
	if(strcmp(up_init_str, "True")==0)
	  uppercase_only = TRUE;
	else
	  uppercase_only = FALSE;
      }

    if (profile_conf)
      g_hash_table_destroy(config);

    sounds_are_fine();

    click_on_letter_next_level();

    gamewon = FALSE;
    pause_board(FALSE);

  }

  board_conf = NULL;
  profile_conf = NULL;
}

static void
config_start(GcomprisBoard *agcomprisBoard,
	     GcomprisProfile *aProfile)
{
  board_conf = agcomprisBoard;
  profile_conf = aProfile;

  if (gcomprisBoard)
    pause_board(TRUE);

  gchar *label = g_strdup_printf(_("<b>%s</b> configuration\n for profile <b>%s</b>"),
				 agcomprisBoard->name,
				 aProfile ? aProfile->name : "");

  gc_board_config_window_display(label, conf_ok);

  g_free(label);

  /* init the combo to previously saved value */
  GHashTable *config = gc_db_get_conf( profile_conf, board_conf);

  gchar *saved_locale_sound = g_hash_table_lookup( config, "locale_sound");

  gc_board_config_combo_locales_asset( "Select sound locale", saved_locale_sound,
				"voices/$LOCALE/colors/purple.ogg");

  gboolean up_init = FALSE;

  gchar *up_init_str = g_hash_table_lookup( config, "uppercase_only");

  if (up_init_str && (strcmp(up_init_str, "True")==0))
    up_init = TRUE;

  gc_board_config_boolean_box(_("Uppercase only text"),
		       "uppercase_only",
		       up_init);

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
