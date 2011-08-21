/* gcompris - gletters.c
 *
 * Copyright (C) 2000, 2008 Bruno Coudoin
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

#include <string.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE
#define MAX_RAND_ATTEMPTS 5

static GList *item_list = NULL;
static GList *item2del_list = NULL;
static guint actors_count = 0;

static GcomprisBoard *gcomprisBoard = NULL;

static gint dummy_id = 0;
static gint drop_items_id = 0;

/* Sublevels are now allocated dynamically
 * based on the number of chars at that level
 * Set DEFAULT_SUBLEVEL to the minimum
 * number of sublevels you want
 */

#define DEFAULT_SUBLEVEL 8

/* these constants control how fast letters fall
 * the base rate is fixed
 * the increment governs increase per level
 * the smaller the numbers, the faster the letters fall
 */

#define FALL_RATE_BASE 40
static float fallRateBase = FALL_RATE_BASE;
#define FALL_RATE_MULT 100
static float fallRateMult = FALL_RATE_MULT;

/* these constants control how often letters are dropped
 * the base rate is fixed
 * the increment governs increase per level
 */

#define DROP_RATE_BASE 1000
static float dropRateBase = DROP_RATE_BASE;
#define DROP_RATE_MULT 8000
static float dropRateMult = DROP_RATE_MULT;

/* both letters_array and keymap are read in
 * dynamically at run-time from files based on
 * user locale
 */

/* letters_array contains letters you want shown
 * on each play level
 * there can be an arbitrary number of levels,
 * but there are only graphics to level 9
 * so that's where we stop
 */

#define MAXLEVEL 10
static int maxLevel;
static char *letters_array[MAXLEVEL];

/* keymap contains pairs of chars. The first char is
 * on the keyboard map, the second is the unichar that
 * is also represented by that key.  That way, if there is more
 * than one character represented by a key, the user doesn't
 * have to use alternate input methods.
 * It turns out that some keyboards generate long unichars,
 * so keymap has to be big enough for 2 unichars
 * both chars are packed into the same string; this makes it
 * easier to deal with.
 */

static int keyMapSize;
static char **keyMap;

/* Hash table of all displayed letters  */
static GHashTable *letters_table= NULL;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str);
static void gletter_config_start(GcomprisBoard *agcomprisBoard,
					     GcomprisProfile *aProfile);
static void gletter_config_stop(void);

static GooCanvasItem *gletters_create_item(GooCanvasItem *parent);
static gboolean gletters_drop_items (gpointer data);
static gboolean gletters_move_items (gpointer data);
static void gletters_destroy_item(GooCanvasItem *item);
static void gletters_destroy_items(void);
static void gletters_destroy_all_items(void);
static void gletters_next_level(void);
static void gletters_add_new_item(void);

static void player_win(GooCanvasItem *item);
static void player_loose(void);
static GooCanvasItem *item_find_by_title (const gunichar *title);
static gunichar *key_find_by_item (const GooCanvasItem *item);

static  guint32              fallSpeed = 0;
static  double               speed = 0.0;
static  int		     gamewon;

static gboolean with_sound = FALSE;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Simple Letters"),
    N_("Type the falling letters before they reach the ground"),
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
    NULL,
    set_level,
    NULL,
    NULL,
    gletter_config_start,
    gletter_config_stop
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(gletters)

/*
 * in : boolean TRUE = PAUSE : FALSE = UNPAUSE
 *
 */

static void level_set_score() {
  int l;

  g_message("letters_array length for level %d is %ld\n",
	    gcomprisBoard->level,
	    g_utf8_strlen(letters_array[gcomprisBoard->level-1],-1));
  l = g_utf8_strlen(letters_array[gcomprisBoard->level-1],-1)/3;
  gcomprisBoard->number_of_sublevel = (DEFAULT_SUBLEVEL>l?DEFAULT_SUBLEVEL:l);

  gc_score_start(SCORESTYLE_NOTE,
		       BOARDWIDTH - 195,
		       BOARDHEIGHT - 30,
		       gcomprisBoard->number_of_sublevel);
  gc_bar_set(GC_BAR_CONFIG|GC_BAR_LEVEL);
}

static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if(pause)
    {
      if (dummy_id) {
	g_source_remove (dummy_id);
	dummy_id = 0;
      }
      if (drop_items_id) {
	g_source_remove (drop_items_id);
	drop_items_id = 0;
      }
    }
  else
    {
      if(gamewon == TRUE) /* the game is won */
	{
	  level_set_score();
	  gletters_next_level();
	}

      if(!drop_items_id) {
	drop_items_id = g_timeout_add (1000,
				       gletters_drop_items, NULL);
      }
      if(!dummy_id) {
	dummy_id = g_timeout_add (1000, gletters_move_items, NULL);
      }
    }
}

static gboolean uppercase_only;

int load_default_charset() {
  g_message("in load_default_charset\n");

  gchar *numbers;
  gchar *alphabet_lowercase;
  gchar *alphabet_uppercase;

  /* TRANSLATORS: Put here the numbers in your language */
  numbers=_("0123456789");
  g_assert(g_utf8_validate(numbers,-1,NULL)); // require by all utf8-functions

  /* TRANSLATORS: Put here the alphabet lowercase in your language */
  alphabet_lowercase=_("abcdefghijklmnopqrstuvwxyz");
  g_assert(g_utf8_validate(alphabet_lowercase,-1,NULL)); // require by all utf8-functions

  g_warning("Using lowercase %s", alphabet_lowercase);

  /* TRANSLATORS: Put here the alphabet uppercase in your language */
  alphabet_uppercase=_("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  g_assert(g_utf8_validate(alphabet_uppercase,-1,NULL)); // require by all utf8-functions
  g_warning("Using uppercase %s", alphabet_uppercase);

  letters_array[0] = g_strdup(alphabet_uppercase);
  letters_array[1] = g_strdup_printf("%s%s",
				     alphabet_uppercase,
				     numbers);
  if (!uppercase_only){
  letters_array[2] = g_strdup(alphabet_lowercase);
  letters_array[3] = g_strdup_printf("%s%s",
				     alphabet_lowercase,
				     numbers);
  letters_array[4] = g_strdup_printf("%s%s",
				     alphabet_lowercase,
				     alphabet_uppercase);
  letters_array[5] = g_strdup_printf("%s%s%s",
				     alphabet_lowercase,
				     alphabet_uppercase,
				     numbers);
  } else{
    g_warning("Uppercase only is set");
      letters_array[2] = g_strdup(alphabet_uppercase);
    letters_array[3] = g_strdup_printf("%s%s",
				       alphabet_uppercase,
				       numbers);
    letters_array[4] = g_strdup_printf("%s%s",
				       alphabet_uppercase,
				       numbers);
    letters_array[5] = g_strdup_printf("%s%s",
				       alphabet_uppercase,
				       numbers);
  }


  keyMapSize = 0;
  maxLevel = 6;
  return TRUE;
}

int whitespace(char *buffer) {
  int i;
  i = 0;
  while(buffer[i] != '\0') {
    if(buffer[i] == ' ' || buffer[i] == '\t' || buffer[i++] == '\n')
      continue;
    else return FALSE;
  }
  return TRUE;
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{
  GHashTable *config = gc_db_get_board_conf();

  gc_locale_change(g_hash_table_lookup( config, "locale"));

  gchar *up_init_str = g_hash_table_lookup( config, "uppercase_only");

  if (up_init_str && (strcmp(up_init_str, "True")==0))
    uppercase_only = TRUE;
  else
    uppercase_only = FALSE;

  gchar *control_sound = g_hash_table_lookup( config, "with_sound");

  if (control_sound && strcmp(g_hash_table_lookup( config, "with_sound"),"True")==0)
    with_sound = TRUE;
  else
    with_sound = FALSE;

  g_hash_table_destroy(config);

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      load_default_charset();
      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			"gletters/scenery_background.png");
      gcomprisBoard->maxlevel=maxLevel;
      gcomprisBoard->level = 1;
      level_set_score();
      gletters_next_level();
      gamewon = FALSE;
      pause_board(FALSE);
    }
}



static void
end_board ()
{
  int i;
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gc_score_end();
      gletters_destroy_all_items();
      g_message("freeing memory");
      for (i = 0; i < maxLevel; i++)
	g_free(letters_array[i]);

      for (i = 0; i < keyMapSize; i++)
	g_free(keyMap[i]);

      g_free(keyMap);
    }

  gc_locale_reset();

  gcomprisBoard = NULL;
}

static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      level_set_score();
      gletters_next_level();
    }
}

/* Append in char_list one of the falling letter */
static void add_char(char *key, char *value, char *char_list)
{
  strcat(char_list, key);
}

gboolean unichar_comp(gpointer key,
		      gpointer value,
		      gpointer user_data)
{
  gunichar *target = (gunichar *) user_data;
  if (*((gunichar *)key) == *target)
    return TRUE;

  return FALSE;
}

gint is_falling_letter(gunichar  unichar)
{
  GooCanvasItem *item;

  if ((item = g_hash_table_find(letters_table,
			       unichar_comp,
				&unichar)))
    {
      player_win(item);
      return TRUE;
    }
  return FALSE;
}

static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str) {
  gint length_passed, i;
  gunichar c;
  gchar  list_of_letters[255];
  gchar *str;

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

  str = g_strdup(string_passed);

  length_passed = g_utf8_strlen(string_passed, -1);

  for (i=0; i < length_passed; i++){
    c = g_utf8_get_char (string_passed);
    if (is_falling_letter(c)){
      gc_im_reset();
      return TRUE;
    }

    /* if uppercase_only is set we do not care about upper or lower case at all */
    gint level_uppercase;
    if (uppercase_only)
      level_uppercase = 10;
    else
      level_uppercase = 3;

    /* for 2 (or all) first level don't care abour uppercase/lowercase */
    if ((gcomprisBoard->level < level_uppercase) &&
	(is_falling_letter(g_unichar_toupper(c)))){
      gc_im_reset();
      return TRUE;
    }

    string_passed = g_utf8_next_char (string_passed);
  }

  list_of_letters[0] = '\0';

  /* We have to loop to concat the letters */
  g_hash_table_foreach (letters_table,
			(GHFunc) add_char,
			list_of_letters);

  /* Log what happened, what was expected, what we got */

  gc_log_set_comment(gcomprisBoard, list_of_letters, str);
  g_free(str);

  return TRUE;
}

static gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "gletters")==0)
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
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

/* set initial values for the next level */
static void gletters_next_level()
{

  gamewon = FALSE;
  gc_bar_set_level(gcomprisBoard);

  gletters_destroy_all_items();

  /* Try the next level */
  speed=fallRateBase+(fallRateMult/gcomprisBoard->level);
  fallSpeed=dropRateBase+(dropRateMult/gcomprisBoard->level);

  gcomprisBoard->sublevel=1;
  gc_score_set(gcomprisBoard->sublevel);
}


static void gletters_move_item(GooCanvasItem *item)
{
  GooCanvasBounds bounds;

  goo_canvas_item_translate(item, 0, 2.0);

  goo_canvas_item_get_bounds (item,
			      &bounds);

  if(bounds.y1>BOARDHEIGHT) {
    item2del_list = g_list_append (item2del_list, item);
    player_loose();
  }
}

static void gletters_destroy_item(GooCanvasItem *item)
{
  gunichar *key;

  key = key_find_by_item(item);

  item_list = g_list_remove (item_list, item);
  --actors_count;

  item2del_list = g_list_remove (item2del_list, item);

  /* Remove old letter; this destroy the canvas item  */
  g_hash_table_remove (letters_table, key);

}

/* Destroy items that falls out of the canvas */
static void gletters_destroy_items()
{
  GooCanvasItem *item;

  while(g_list_length(item2del_list)>0)
    {
      item = g_list_nth_data(item2del_list, 0);
      gletters_destroy_item(item);
    }
}

/* Destroy all the items */
static void gletters_destroy_all_items()
{
  GooCanvasItem *item;

  if(item_list)
    while(g_list_length(item_list)>0)
      {
	item = g_list_nth_data(item_list, 0);
	gletters_destroy_item(item);
      }

   actors_count= 0;

  /* Delete the letters_table */
  if(letters_table) {
    g_hash_table_destroy (letters_table);
    letters_table=NULL;
  }
}

/*
 * This does the moves of the game items on the play canvas
 *
 */
static gboolean gletters_move_items (gpointer data)
{
  g_list_foreach (item_list, (GFunc) gletters_move_item, NULL);

  /* Destroy items that falls out of the canvas */
  gletters_destroy_items();

  dummy_id = g_timeout_add (gc_timing (speed, actors_count),
			    gletters_move_items, NULL);

  return(FALSE);
}


void destroy_canvas_item(gpointer item)
{
  //g_free(g_object_get_data (G_OBJECT(item),"unichar_key"));
  //g_free(g_object_get_data (G_OBJECT(item),"utf8_key"));
  goo_canvas_item_remove(item);
}

static GooCanvasItem *gletters_create_item(GooCanvasItem *parent)
{
  GooCanvasItem *item;
  gint i,j,k;
  guint x;
  gunichar *lettersItem;
  gchar *str_p, *letter;

  if (!letters_table)
    {
      letters_table = g_hash_table_new_full (g_int_hash, g_int_equal, g_free, destroy_canvas_item);
    }

  /* Beware, since we put the letters in a hash table, we do not allow the same
   * letter to be displayed two times
   */

  g_warning("dump: %d, %s\n",gcomprisBoard->level,letters_array[gcomprisBoard->level-1]);

  k = g_utf8_strlen(letters_array[gcomprisBoard->level-1],-1);

  lettersItem = g_new(gunichar,1);
  gint attempt=0;
  do
    {
      attempt++;
      str_p = letters_array[gcomprisBoard->level-1];
      i = g_random_int_range(0,k);

      for(j = 0; j < i; j++)
	{
	  str_p=g_utf8_next_char(str_p);
	}

      *lettersItem = g_utf8_get_char (str_p);

    } while((attempt<MAX_RAND_ATTEMPTS) && (item_find_by_title(lettersItem)!=NULL));

  if (item_find_by_title(lettersItem)!=NULL)
    {
      g_free(lettersItem);
      return NULL;
    }

  letter = g_new0(gchar, 6);
  g_unichar_to_utf8 ( *lettersItem, letter);

  if (with_sound)
    {
      gchar *str2 = NULL;
      gchar *letter_unichar_name = gc_sound_alphabet(letter);

      str2 = g_strdup_printf("voices/$LOCALE/alphabet/%s", letter_unichar_name);

      gc_sound_play_ogg(str2, NULL);

      g_free(letter_unichar_name);
      g_free(str2);
    }

  item = \
    goo_canvas_group_new (parent,
			  NULL);
  goo_canvas_item_translate(item, 0, -12);

  x = g_random_int_range( 80, BOARDWIDTH-160);
  goo_canvas_text_new (item,
		       letter,
		       x,
		       -20,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_board_huge_bold,
		       "fill_color_rgba", 0x8c8cFFFF,
		       NULL);
  x -= 2;
  goo_canvas_text_new (item,
		       letter,
		       x,
		       -22,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_board_huge_bold,
		       "fill_color_rgba", 0x254c87FF,
		       NULL);

  g_object_set_data (G_OBJECT(item), "unichar_key", lettersItem);
  g_object_set_data (G_OBJECT(item), "utf8_key", letter);

  item_list = g_list_append (item_list, item);
  ++actors_count;

  /* Add letter to hash table of all falling letters. */
  g_hash_table_insert (letters_table, lettersItem, item);

  g_free(letter);

  return (item);
}

static void gletters_add_new_item()
{
  gletters_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));
}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gboolean gletters_drop_items (gpointer data)
{
  gc_sound_play_ogg ("sounds/level.wav", NULL);
  gletters_add_new_item();

  drop_items_id = g_timeout_add (fallSpeed,
				 gletters_drop_items, NULL);
  return (FALSE);
}

static void player_win(GooCanvasItem *item)
{
  gletters_destroy_item(item);
  gc_sound_play_ogg ("sounds/flip.wav", NULL);

  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel > gcomprisBoard->number_of_sublevel)
    {
      /* Try the next level */
      gcomprisBoard->level++;
      if(gcomprisBoard->level>gcomprisBoard->maxlevel)
	gcomprisBoard->level = gcomprisBoard->maxlevel;

      gamewon = TRUE;
      gletters_destroy_all_items();
      gc_bonus_display(gamewon, GC_BONUS_SMILEY);
    }
  else
    {
      gc_score_set(gcomprisBoard->sublevel);

      /* Drop a new item now to speed up the game */
      if(g_list_length(item_list)==0)
	{
	  if (drop_items_id) {
	    /* Remove pending new item creation to sync the falls */
	    g_source_remove (drop_items_id);
	    drop_items_id = 0;
	  }
	  if(!drop_items_id) {
	    drop_items_id = g_timeout_add (0,
					   gletters_drop_items, NULL);
	  }
	}
    }
}

static void player_loose()
{
  gc_sound_play_ogg ("sounds/crash.wav", NULL);
}

static gunichar *
key_find_by_item (const GooCanvasItem *item)
{
  return g_object_get_data (G_OBJECT(item), "unichar_key");
}

static GooCanvasItem *
item_find_by_title (const gunichar *title)
{
  if (!letters_table)
    return NULL;

  return g_hash_table_lookup (letters_table, title);
}


/***********************************
************************************
    Config
************************************/

/* ======================= */
/* = config_start        = */
/* ======================= */

static GcomprisProfile *profile_conf;
static GcomprisBoard   *board_conf;

static void save_table (gpointer key,
			gpointer value,
			gpointer user_data)
{
  gc_db_set_board_conf ( profile_conf,
			    board_conf,
			    (gchar *) key,
			    (gchar *) value);
}

/* a GcomprisConfCallback
 */
static gboolean conf_ok(GHashTable *table)
{
  if (!table){
    if (gcomprisBoard)
      pause_board(FALSE);
    return TRUE;
  }

  g_hash_table_foreach(table, save_table, NULL);

  if (gcomprisBoard){
    GHashTable *config;

    if (profile_conf)
      config = gc_db_get_board_conf();
    else
      config = table;

    gc_locale_set(g_hash_table_lookup( config, "locale"));

    gchar *up_init_str = g_hash_table_lookup( config, "uppercase_only");

    if (up_init_str && (strcmp(up_init_str, "True")==0))
      uppercase_only = TRUE;
    else
      uppercase_only = FALSE;

    gchar *control_sound = g_hash_table_lookup( config, "with_sound");

    if (control_sound && strcmp(g_hash_table_lookup( config, "with_sound"),"True")==0)
      with_sound = TRUE;
    else
      with_sound = FALSE;

    if (profile_conf)
      g_hash_table_destroy(config);

    load_default_charset();

    level_set_score();
    gletters_next_level();

    pause_board(FALSE);

  }

  board_conf = NULL;
  profile_conf = NULL;
  return TRUE;
}

static void
gletter_config_start(GcomprisBoard *agcomprisBoard,
		    GcomprisProfile *aProfile)
{
  board_conf = agcomprisBoard;
  profile_conf = aProfile;

  gchar *label;

  if (gcomprisBoard)
    pause_board(TRUE);

  label = g_strdup_printf(_("<b>%s</b> configuration\n for profile <b>%s</b>"),
			  agcomprisBoard->name, aProfile ? aProfile->name : "");

  GcomprisBoardConf *bconf = gc_board_config_window_display(label, (GcomprisConfCallback )conf_ok);

  g_free(label);

  /* init the combo to previously saved value */
  GHashTable *config = gc_db_get_conf( profile_conf, board_conf);

  gchar *locale = g_hash_table_lookup( config, "locale");

  gc_board_config_combo_locales( bconf, locale);

  gboolean up_init = FALSE;

  gchar *up_init_str = g_hash_table_lookup( config, "uppercase_only");

  if (up_init_str && (strcmp(up_init_str, "True")==0))
    up_init = TRUE;

  gc_board_conf_separator(bconf);

  gchar *control_sound = g_hash_table_lookup( config, "with_sound");
  if (control_sound && strcmp(g_hash_table_lookup( config, "with_sound"),"True")==0)
    with_sound = TRUE;
  else
    with_sound = FALSE;

  gc_board_config_boolean_box(bconf, _("Enable sounds"), "with_sound", with_sound);

  gc_board_conf_separator(bconf);

  gc_board_config_boolean_box(bconf, _("Uppercase only text"),
		       "uppercase_only",
		       up_init);

}


/* ======================= */
/* = config_stop        = */
/* ======================= */
static void
gletter_config_stop()
{
}
