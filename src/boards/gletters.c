/* gcompris - gletters.c
 *
 * Time-stamp: <2005/03/30 21:41:45 bruno>
 *
 * Copyright (C) 2000 Bruno Coudoin
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

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE
#define MAX_RAND_ATTEMPTS 5

static GList *item_list = NULL;
static GList *item2del_list = NULL;

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
static gint key_press(guint keyval);

static GnomeCanvasItem *gletters_create_item(GnomeCanvasGroup *parent);
static gint gletters_drop_items (GtkWidget *widget, gpointer data);
static gint gletters_move_items (GtkWidget *widget, gpointer data);
static void gletters_destroy_item(GnomeCanvasItem *item);
static void gletters_destroy_items(void);
static void gletters_destroy_all_items(void);
static void gletters_next_level(void);
static void gletters_add_new_item(void);

static void player_win(GnomeCanvasItem *item);
static void player_loose(void);
static GnomeCanvasItem *item_find_by_title (const gchar *title);
static char *key_find_by_item (const GnomeCanvasItem *item);

static  guint32              fallSpeed = 0;
static  double               speed = 0.0;
static  int		     gamewon;

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
 * in : boolean TRUE = PAUSE : FALSE = UNPAUSE
 *
 */

static void level_set_score() {
  int l;

  g_message("letters_array length for level %d is %d\n",gcomprisBoard->level,
	    g_utf8_strlen(letters_array[gcomprisBoard->level-1],-1));
  l = g_utf8_strlen(letters_array[gcomprisBoard->level-1],-1)/3;
  gcomprisBoard->number_of_sublevel=(DEFAULT_SUBLEVEL>l?DEFAULT_SUBLEVEL:l);

  gcompris_score_start(SCORESTYLE_NOTE, 
		       gcomprisBoard->width - 220, 
		       gcomprisBoard->height - 50, 
		       gcomprisBoard->number_of_sublevel);
  gcompris_bar_set(GCOMPRIS_BAR_LEVEL);
}

static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if(pause)
    {
      if (dummy_id) {
	gtk_timeout_remove (dummy_id);
	dummy_id = 0;
      }
      if (drop_items_id) {
	gtk_timeout_remove (drop_items_id);
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
	drop_items_id = gtk_timeout_add (1000,
					 (GtkFunction) gletters_drop_items, NULL);
      }
      if(!dummy_id) {
	dummy_id = gtk_timeout_add (1000, (GtkFunction) gletters_move_items, NULL);
      }
    }
}

int fill_letters(char **letterString,char *buffer) {
  g_message("in fill_letters\n");
  *letterString = g_malloc(strlen(buffer)+1);
  sprintf(*letterString,"%s",buffer);
}

int load_default_charset() {
  g_message("in load_default_charset\n");
  fill_letters(&letters_array[0],"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  fill_letters(&letters_array[1],"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
  fill_letters(&letters_array[2],"abcdefghijklmnopqrstuvwxyz");
  fill_letters(&letters_array[3],"abcdefghijklmnopqrstuvwxyz0123456789");
  fill_letters(&letters_array[4],"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
  fill_letters(&letters_array[5],"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
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

int load_charset_from_file(FILE *fp) {
  int level, fp_ret_code, currKeyMap;
  char lineBuffer[4096], strBuffer[4096], charBuffer[12];

  /* keymap size is dynamically allocated */
  g_message("scanning line\n");
  keyMapSize = 64;
  currKeyMap = 0;
  keyMap = (char **)g_malloc((sizeof (char *))*keyMapSize);
  g_warning("in load_charset_from_file\n");
  /* first read each line into a buffer */
  while( fgets(lineBuffer,4095,fp) ) {
    /* if it's a comment or blank (empty), skip line */
    if ( lineBuffer[0] == '#' || whitespace(lineBuffer) ) continue;
    /* if it's not a comment, it can be one of 4 things
     * a level string, where it starts with "level "
     * and the format is "level %d %s" or
     * a keymap which starts with "key" and the format is "key %lc%lc"
     * fall rate, which starts with "fallrate" and format is "fallrate %f %f"
     * drop rate, which starts with "droprate" and format is "droprate %f %f"
     */
    g_message("scanning line\n");
    if( sscanf(lineBuffer,"level %d %s",&level, strBuffer) == 2 ) {
      /* we have a level charset */
      if (level > MAXLEVEL || level < 1) {
	g_message("level %d outside range of 1 to %d in line %s ",level,MAXLEVEL,lineBuffer);
	return FALSE;
      }
      fill_letters(&letters_array[level-1],strBuffer);
      if(maxLevel < level) maxLevel = level;
      g_message("maxLevel: %d\n",maxLevel);
    }
    else if( sscanf(lineBuffer,"key %11s",charBuffer) == 1) {
      /* we potentially have a keymap */
      if( !g_utf8_validate(charBuffer,-1,(void *)NULL) ) {
	/* it's not a valid UTF-8 string */
	g_message("malformed UTF-8 character string >%s< ",charBuffer);
	return FALSE;
      }
      keyMap[currKeyMap] = (char *)g_malloc((sizeof (char))*strlen(charBuffer));
      sprintf(keyMap[currKeyMap],"%s",charBuffer);
      currKeyMap++;
      if(currKeyMap ==  keyMapSize) {
	keyMapSize *= 2;
	keyMap = (char **)realloc(keyMap,(sizeof(char *))*keyMapSize);
      }
    }
    else if (sscanf(lineBuffer,"fallrate %f %f",&fallRateBase,&fallRateMult) == 2) {
      if(fallRateBase < 5 || fallRateBase > 500 || fallRateMult < 5 || fallRateMult > 500) 
	g_message("WARNING: fallrate outside reasonable parameters");
    }
    else if (sscanf(lineBuffer,"droprate %f %f",&dropRateBase,&dropRateMult) == 2) {
      if(dropRateBase < 100 || dropRateBase > 20000 || dropRateMult < 100 || dropRateMult > 20000)
	g_message("WARNING: droprate outside reasonable parameters");
    }
    else
      g_message("unknown or bad command in file: >%s<",lineBuffer);
  }
  /* now check to see all levels have been filled
   */
  keyMapSize = currKeyMap;
  return TRUE;
}   

int get_charset(const char *locale) {
  char *filename;
  FILE *charsfd = NULL;
  int i;
  
  /* zero out letters_array so we know which ones got assigned */
  for(i = 0; i < MAXLEVEL; i++)
    letters_array[i] = (char *)0;
  maxLevel=0;

  /* First Try to find a file matching the level and the locale */
  filename = g_strdup_printf("%s%s.%.2s",
                             PACKAGE_DATA_DIR, "/gletters/gletters",
                             locale);
  g_message("Trying to open file %s ", filename);
  charsfd = fopen (filename, "r");
  g_free(filename);

                                                                                                         
  if (charsfd == (FILE *)NULL) {
    g_message("failed to open file.\n");
    load_default_charset();
  }
  else if (load_charset_from_file(charsfd))
    g_message("loaded charset from file.\n");
  else {
    g_message("failed to load charset from file - using defaults.\n");
    load_default_charset();
  }

  /*now make sure all levels have been filled */
  for(i = 0; i < maxLevel; i++)
    if(letters_array[i] == (char *)0) {
      g_message("WARNING: level %d uninitialized in config file, setting defaults",i);
      fill_letters(&letters_array[i],"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    }

}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{
  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "images/scenery_background.png");
      get_charset(gcompris_get_locale());
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
      gcompris_score_end();
      gletters_destroy_all_items();
      g_message("freeing memory");
      for (i = 0; i < maxLevel; i++) 
	g_free(letters_array[i]);

      for (i = 0; i < keyMapSize; i++) 
	g_free(keyMap[i]);

      g_free(keyMap);
    }
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

gint is_falling_letter(char *utfchar) {
  gchar *old_value;
  gchar *old_name;

  if (g_hash_table_lookup_extended (letters_table,
                                    utfchar,
                                    (gpointer) &old_name,
                                    (gpointer) &old_value))
    {
      player_win(item_find_by_title(utfchar));
      return TRUE;
    }
  return FALSE;
}

static gint key_press(guint keyval) {
  char c;
  char lcStr[6],ucStr[6];
  gchar *str;
  char utf8char[6], keyChar[6], mapChar[6];
  int i;
  gchar list_of_letters[255];

  g_message("in key_press: %d, %c, %lc",keyval,keyval,keyval);

  if(!gcomprisBoard)
    return FALSE;

  /* Add some filter for control and shift key */
  switch (keyval)
    {
      /* Avoid all this keys to be interpreted by this game */
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
    case GDK_Num_Lock:
      return FALSE; 
    case GDK_KP_0:
    case GDK_KP_Insert:
      keyval=GDK_0;
      break;
    case GDK_KP_1:
    case GDK_KP_End:
      keyval=GDK_1;
      break;
    case GDK_KP_2:
    case GDK_KP_Down:
      keyval=GDK_2;
      break;
    case GDK_KP_3:
    case GDK_KP_Page_Down:
      keyval=GDK_3;
      break;
    case GDK_KP_4:
    case GDK_KP_Left:
      keyval=GDK_4;
      break;
    case GDK_KP_5:
    case GDK_KP_Begin:
      keyval=GDK_5;
      break;
    case GDK_KP_6:
    case GDK_KP_Right:
      keyval=GDK_6;
      break;
    case GDK_KP_7:
    case GDK_KP_Home:
      keyval=GDK_7;
      break;
    case GDK_KP_8:
    case GDK_KP_Up:
      keyval=GDK_8;
      break;
    case GDK_KP_9:
    case GDK_KP_Page_Up:
      keyval=GDK_9;
      break;
    }

  /* first check the obvious - simple keystrokes 
     check for keymap 
     convert to unichar/utf8
     set to lower case
     check direct match
     check upper case match
  */
  g_message("checking keymap: %d\n",keyMapSize);

  i = g_unichar_to_utf8 (gdk_keyval_to_unicode(keyval),utf8char);
  utf8char[i]='\0';


  for (i = 0; i < keyMapSize; i++) {
    g_message("keymap: %d: %s\n",i,keyMap[i]);

    sprintf(keyChar, "%lc", g_utf8_get_char(keyMap[i]));
    sprintf(mapChar, "%lc", g_utf8_get_char(g_utf8_find_next_char(keyMap[i], NULL)));

    g_message("char1: %s, char2: %s",keyChar, mapChar);

    if (strcmp(utf8char, keyChar) == 0) {

      /* match in keymap */
      sprintf(utf8char, "%s", mapChar);
      if( is_falling_letter(utf8char) ) {
	str = utf8char;
	break;
      }
    }
  }

  g_message("no match-moving on\n");

  /* no match in keymap */
  if (i == keyMapSize) {

    g_message("i == keyMapSize\n");

    sprintf(lcStr,"%s",g_utf8_strdown(utf8char,-1));
    sprintf(ucStr,"%s",g_utf8_strup(utf8char,-1));
    g_message("lcStr = %s\n", lcStr);
    g_message("ucStr = %s\n", ucStr);
    if ( is_falling_letter(lcStr) ) {
      str = lcStr;
    }
    else if( is_falling_letter(ucStr) ) {
      str = ucStr;
    }
    else {
      str = utf8char;
      player_loose();
    }
  }

  list_of_letters[0] = '\0';

  /* We have to loop to concat the letters */
  g_hash_table_foreach (letters_table,
			(GHFunc) add_char,
			list_of_letters);

  /* Log what happened, what was expected, what we got */

  gcompris_log_set_comment(gcomprisBoard, list_of_letters, str);

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
  gcompris_bar_set_level(gcomprisBoard);

  gletters_destroy_all_items();

  /* Try the next level */
  speed=fallRateBase+(fallRateMult/gcomprisBoard->level);
  fallSpeed=dropRateBase+(dropRateMult/gcomprisBoard->level);

  gcomprisBoard->sublevel=1;
  gcompris_score_set(gcomprisBoard->sublevel);
}


static void gletters_move_item(GnomeCanvasItem *item)
{
  double x1, y1, x2, y2;

  gnome_canvas_item_move(item, 0, 2.0);

  gnome_canvas_item_get_bounds    (item,
				   &x1,
				   &y1,
				   &x2,
				   &y2);
  
  if(y1>gcomprisBoard->height) {
    item2del_list = g_list_append (item2del_list, item);
    player_loose();
  }
}

static void gletters_destroy_item(GnomeCanvasItem *item)
{
  char *key;


  key = key_find_by_item(item);
  /* Remove old letter  */
  g_hash_table_remove (letters_table, key);
  g_free (key);

  item_list = g_list_remove (item_list, item);
  item2del_list = g_list_remove (item2del_list, item);
  gtk_object_destroy (GTK_OBJECT(item));
}

/* Destroy items that falls out of the canvas */
static void gletters_destroy_items()
{
  GnomeCanvasItem *item;

  while(g_list_length(item2del_list)>0) 
    {
      item = g_list_nth_data(item2del_list, 0);
      gletters_destroy_item(item);
    }
}

/* Destroy all the items */
static void gletters_destroy_all_items()
{
  GnomeCanvasItem *item;

  if(item_list)
    while(g_list_length(item_list)>0) 
      {
	item = g_list_nth_data(item_list, 0);
	gletters_destroy_item(item);
      }

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
static gint gletters_move_items (GtkWidget *widget, gpointer data)
{
  g_list_foreach (item_list, (GFunc) gletters_move_item, NULL);

  /* Destroy items that falls out of the canvas */
  gletters_destroy_items();

  dummy_id = gtk_timeout_add (speed, 
			      (GtkFunction) gletters_move_items, NULL);

  return(FALSE);
}

static GnomeCanvasItem *gletters_create_item(GnomeCanvasGroup *parent)
{
  GdkPixbuf *gletters_pixmap = NULL;
  GnomeCanvasItem *item;
  gchar *str  = NULL;
  gchar *str2 = NULL;
  gint i,j,k;
  guint c, x;
  gchar *lettersItem, *str_p;

  if (!letters_table)
    {
      letters_table= g_hash_table_new (g_str_hash, g_str_equal);
    }


  /* Beware, since we put the letters in a hash table, we do not allow the same
     letter to be displayed two times 
  */

  g_warning("dump: %d, %s\n",gcomprisBoard->level,letters_array[gcomprisBoard->level-1]);

  k=g_utf8_strlen(letters_array[gcomprisBoard->level-1],-1);

  lettersItem = g_new(gchar,6);
  gint attempt=0;
  do {

    attempt++;
    str_p=letters_array[gcomprisBoard->level-1];
    i=(int)((float)k*rand()/(RAND_MAX+1.0));

    for( j = 0; j < i; j++) {
      str_p=g_utf8_find_next_char(str_p,NULL);
    }

    g_utf8_strncpy (lettersItem, str_p,1);

  } while((attempt<MAX_RAND_ATTEMPTS) && (item_find_by_title(lettersItem)!=NULL));

  if (item_find_by_title(lettersItem)!=NULL)  {g_free(lettersItem); return NULL;}

  gchar *letter_no_caps=g_utf8_strdown(lettersItem,-1);
  str = g_strdup_printf("%s%s", letter_no_caps, ".ogg");
  str2 = gcompris_get_asset_file("gcompris alphabet", NULL, "audio/x-ogg", str);
  gcompris_play_ogg(str2, NULL);

  g_free(letter_no_caps);
  g_free(str);
  g_free(str2);

  item =					\
    gnome_canvas_item_new (parent,
			   gnome_canvas_group_get_type (),
			   "x", (double) 0,
			   "y", (double) -12,
			   NULL);

  x = 80 + (int)((float)(gcomprisBoard->width-160)*rand()/(RAND_MAX+1.0));
  gnome_canvas_item_new (GNOME_CANVAS_GROUP(item),
			 gnome_canvas_text_get_type (),
			 "text", lettersItem,
			 "font", gcompris_skin_font_board_huge_bold,
			 "x", (double) x,
			 "y", (double) -20,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color_rgba", 0x8c8cFFFF,
			 NULL);
  x -= 2;
  gnome_canvas_item_new (GNOME_CANVAS_GROUP(item),
			 gnome_canvas_text_get_type (),
			 "text", lettersItem,
			 "font", gcompris_skin_font_board_huge_bold,
			 "x", (double) x,
			 "y", (double) -22,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color_rgba", 0x254c87FF,
			 NULL);

  item_list = g_list_append (item_list, item);

  /* Add letter to hash table of all falling letters. */
  g_hash_table_insert (letters_table, lettersItem, item);

  g_warning("done\n");

  return (item);
}

static void gletters_add_new_item() 
{
  gletters_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gint gletters_drop_items (GtkWidget *widget, gpointer data)
{
  gletters_add_new_item();

  drop_items_id = gtk_timeout_add (fallSpeed,
				   (GtkFunction) gletters_drop_items, NULL);
  return (FALSE);
}

static void player_win(GnomeCanvasItem *item)
{
  g_message("in player_win\n");

  gletters_destroy_item(item);
  gcompris_play_ogg ("gobble", NULL);

  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel > gcomprisBoard->number_of_sublevel)
    {
      /* Try the next level */
      gcomprisBoard->level++;
      if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
        gcompris_score_end();
	board_finished(BOARD_FINISHED_RANDOM);
	return;
      }

      gamewon = TRUE;
      gletters_destroy_all_items();
      gcompris_display_bonus(gamewon, BONUS_SMILEY);
    }
  else
    {
      gcompris_score_set(gcomprisBoard->sublevel);

      /* Drop a new item now to speed up the game */
      if(g_list_length(item_list)==0)
	{
	  if (drop_items_id) {
	    /* Remove pending new item creation to sync the falls */
	    gtk_timeout_remove (drop_items_id);
	    drop_items_id = 0;
	  }
	  if(!drop_items_id) {
	    drop_items_id = gtk_timeout_add (0,
					     (GtkFunction) gletters_drop_items, NULL);
	  }
	}
    }
  g_warning("leaving player_win\n");
}

static void player_loose()
{
  g_warning("entering player_loose\n");

  gcompris_play_ogg ("crash", NULL);
  g_warning("leaving player_loose\n");
}

/* Return in item the key if the value equals the item */
static void get_item(char *key, char *value, char **item)
{
  if(value==*item) {
    *item=key;
  }
}

static char *
key_find_by_item (const GnomeCanvasItem *item)
{
  static char *key_found;

  key_found = (char *)item;

  if (!letters_table)
    return NULL;
  
  /* We have to loop to find this item's key */
  g_hash_table_foreach (letters_table,
			(GHFunc) get_item,
			&key_found);

  return key_found;
}

static GnomeCanvasItem *
item_find_by_title (const gchar *title)
{
  if (!letters_table)
    return NULL;
  
  return g_hash_table_lookup (letters_table, title);
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
