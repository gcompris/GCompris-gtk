/* gcompris - wordsgame.c
 *
 * Time-stamp: <2004/10/21 22:59:17 bruno>
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

#include <errno.h>
#include <stdio.h>

#include "gcompris/gcompris.h"


#define SOUNDLISTFILE PACKAGE
#define MAXWORDSLENGTH 50

static GList *item_list = NULL;
static GList *item2del_list = NULL;

static GcomprisBoard *gcomprisBoard = NULL;

static gint dummy_id = 0;
static gint drop_items_id = 0;

/* Hash table of all displayed letters  */
static GHashTable *words_table= NULL;

typedef struct {
  gchar *word;
  gchar *overword;
  gint  charcounter;
  GnomeCanvasItem *rootitem;
  GnomeCanvasItem *overwriteItem;
} LettersItem;

static LettersItem *currentFocus = NULL;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void 		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);
static gint		 key_press(guint keyval);

static GnomeCanvasItem	 *wordsgame_create_item(GnomeCanvasGroup *parent);
static gint		 wordsgame_drop_items (GtkWidget *widget, gpointer data);
static gint		 wordsgame_move_items (GtkWidget *widget, gpointer data);
static void		 wordsgame_destroy_item(LettersItem *item);
static void		 wordsgame_destroy_items(void);
static void		 wordsgame_destroy_all_items(void);
static void		 wordsgame_next_level(void);
static void		 wordsgame_add_new_item(void);

static void		 player_win(LettersItem *item);
static void		 player_loose(void);
static LettersItem 	*item_find_by_title (const gchar *title);
static char		 *get_random_word(void);
static void		 wordsgame_check_focus (gchar	*key,
				   LettersItem *value,
				   LettersItem **user_data);
static gboolean words_table_foreach_remove (char *key,
					    LettersItem *value,
					    LettersItem *user_data);

static  guint32              fallSpeed = 0;
static  double               speed = 0.0;

/* Description of this plugin */
static BoardPlugin menu_bp =
{
   NULL,
   NULL,
   N_("Falling Words"),
   N_("Fully type the falling words before they reach the ground"),
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
      if(!drop_items_id) {
	drop_items_id = gtk_timeout_add (100,
					 (GtkFunction) wordsgame_drop_items, NULL);
      }
      if(!dummy_id) {
	dummy_id = gtk_timeout_add (100, (GtkFunction) wordsgame_move_items, NULL);
      }
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


      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 6;
      gcomprisBoard->sublevel = 1;
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

      wordsgame_next_level();
    }
}

static gboolean words_table_foreach_remove (char *key,
					    LettersItem *value,
					    LettersItem *user_data)
{
  g_free(value->word);
  g_free(value->overword);
  return TRUE;
}

static void
end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gcompris_score_end();
      wordsgame_destroy_all_items();
      if (words_table)
	{
	  g_hash_table_foreach_remove (words_table,
				       (GHRFunc)words_table_foreach_remove,
				       NULL);
	  g_hash_table_destroy (words_table);
	  words_table=NULL;
	}
    }
  gcomprisBoard = NULL;
}

static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      wordsgame_next_level();
    }
}

static void wordsgame_check_focus (gchar       *key,
				   LettersItem *value,
				   LettersItem **user_data)
{
  LettersItem *usrdata = *user_data;
  gchar *nextCurrentChar;

  if(usrdata->rootitem!=NULL)
    return;

  nextCurrentChar = g_utf8_next_char(key);

  if(strncmp(key, usrdata->word, nextCurrentChar-key)==0)
    {
      free(*user_data);
      *user_data = value;
    }

}

static gint key_press(guint keyval)
{
  char utf8char[6];
  guint utf8_length;

  if(!gcomprisBoard)
    return TRUE;

  if(!g_unichar_isalnum (gdk_keyval_to_unicode (keyval)))
    return TRUE;

  /* Add some filter for control and shift key */
  switch (keyval)
    {
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
    case GDK_Mode_switch:
    case GDK_dead_circumflex:
    case GDK_Num_Lock:
      return FALSE; 
    case GDK_KP_0:
      keyval=GDK_0;
      break;
    case GDK_KP_1:
      keyval=GDK_1;
      break;
    case GDK_KP_2:
      keyval=GDK_2;
      break;
    case GDK_KP_3:
      keyval=GDK_3;
      break;
    case GDK_KP_4:
      keyval=GDK_4;
      break;
    case GDK_KP_5:
      keyval=GDK_5;
      break;
    case GDK_KP_6:
      keyval=GDK_6;
      break;
    case GDK_KP_7:
      keyval=GDK_7;
      break;
    case GDK_KP_8:
      keyval=GDK_8;
      break;
    case GDK_KP_9:
      keyval=GDK_9;
      break;
    }

  utf8_length = g_unichar_to_utf8 (gdk_keyval_to_unicode(keyval),
				   utf8char);

  if(currentFocus==NULL) 
    {
      LettersItem *searchitem;

      searchitem = malloc(sizeof(LettersItem));

      /* Try to see if this letter matches the first one of any words */
      searchitem->word        = utf8char;
      searchitem->rootitem    = NULL;
      searchitem->overword    = "";
      searchitem->charcounter = 0;

      g_hash_table_foreach (words_table, (GHFunc) wordsgame_check_focus, &searchitem);

      
      if(searchitem->rootitem!=NULL) 
	{
	  currentFocus=searchitem;
	}
      else
	{
	  free(searchitem);
	}
    }

  if(currentFocus!=NULL) 
    {
      if(currentFocus->rootitem!=NULL) 
	{
	  gchar *currentChar;
	  gchar *nextCurrentChar;
	  gint   i;

	  /* Find the next Char */
	  currentChar = currentFocus->word + currentFocus->charcounter;

	  if(strncmp(currentChar, utf8char, utf8_length)==0)
	    {
	      currentFocus->charcounter += utf8_length;

	      /* Increment the overword */
	      strncpy(currentFocus->overword, currentFocus->word, currentFocus->charcounter);
	      currentFocus->overword[currentFocus->charcounter] = '\0';

	      gnome_canvas_item_set (currentFocus->overwriteItem,
				     "text", currentFocus->overword,
				     NULL);

	      if(g_utf8_strlen(currentFocus->overword, MAXWORDSLENGTH) ==
		 g_utf8_strlen(currentFocus->word, MAXWORDSLENGTH))
		{
		  LettersItem *item = item_find_by_title(currentFocus->word);

		  /* You won Guy */
		  player_win(item);

		  currentFocus=NULL;
		}
	    }
	  else
	    {
	      /* It is a loose : unselect the word and defocus */
	      currentFocus->overword[0]='\0';
	      currentFocus->charcounter=0;
	      gnome_canvas_item_set (currentFocus->overwriteItem,
				     "text", currentFocus->overword,
				     NULL);
	      
	      currentFocus=NULL;
	      player_loose();
	    }
	}
    }
  else
    {
      /* Anyway kid you clicked on the wrong key */
      player_loose();
    }

  return FALSE;
}

static gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "wordsgame")==0)
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
static void wordsgame_next_level() 
{

  gcomprisBoard->number_of_sublevel = 10 + 
    ((gcomprisBoard->level-1) * 5);
  gcompris_score_start(SCORESTYLE_NOTE, 
		       gcomprisBoard->width - 220, 
		       gcomprisBoard->height - 50, 
		       gcomprisBoard->number_of_sublevel);
  
  gcompris_bar_set_level(gcomprisBoard);
  gcompris_score_set(gcomprisBoard->sublevel);

  wordsgame_destroy_all_items();

  /* Default speed */
  speed=150;
  fallSpeed=7000;
  
  /* Increase speed only after 5 levels */
  if(gcomprisBoard->level>5)
    {
      fallSpeed=7000-gcomprisBoard->level*200;
    }

  pause_board(FALSE);
}

static void remove_old_word(LettersItem *item)
{
  /* Remove old word */
  g_hash_table_remove (words_table, (item->word));
  /* The items are freed by player_win */
  g_free(item->word);
  g_free(item->overword);		  
  g_free(item);
}

static void wordsgame_move_item(LettersItem *item)
{
  double x1, y1, x2, y2;

  gnome_canvas_item_move(item->rootitem, 0, 2.0);

  gnome_canvas_item_get_bounds    (item->rootitem,
				   &x1,
				   &y1,
				   &x2,
				   &y2);
  
  if(y1>gcomprisBoard->height) {
    item2del_list = g_list_append (item2del_list, item);
    player_loose();
  }
}

static void wordsgame_destroy_item(LettersItem *item)
{

  /* Andrew Stribblehill <ads@debian.org> -- clear current focus if
       it goes off the canvas */
  if (currentFocus == item) {
    currentFocus = NULL;
  }

  item_list = g_list_remove (item_list, item);
  item2del_list = g_list_remove (item2del_list, item);
  gtk_object_destroy (GTK_OBJECT(item->rootitem));
  remove_old_word(item);
}

/* Destroy items that falls out of the canvas */
static void wordsgame_destroy_items()
{
  LettersItem *item;

  while(g_list_length(item2del_list)>0) 
    {
      item = g_list_nth_data(item2del_list, 0);
      wordsgame_destroy_item(item);
    }
}

/* Destroy all the items */
static void wordsgame_destroy_all_items()
{
  LettersItem *item;

  if(item_list == NULL)
    return;

  while(g_list_length(item_list)>0) 
    {
      item = g_list_nth_data(item_list, 0);
      wordsgame_destroy_item(item);
    }
}

/*
 * This does the moves of the game items on the play canvas
 *
 */
static gint wordsgame_move_items (GtkWidget *widget, gpointer data)
{
  g_list_foreach (item_list, (GFunc) wordsgame_move_item, NULL);

  /* Destroy items that falls out of the canvas */
  wordsgame_destroy_items();

  dummy_id = gtk_timeout_add (speed, 
			      (GtkFunction) wordsgame_move_items, NULL);

  return(FALSE);
}

static GnomeCanvasItem *wordsgame_create_item(GnomeCanvasGroup *parent)
{
  GnomeCanvasItem *item2;
  LettersItem *lettersItem;
  guint maxtry = 10;

  lettersItem = malloc(sizeof(LettersItem));

  if (!words_table)
    {
      words_table= g_hash_table_new (g_str_hash, g_str_equal);
    }

  /* Beware, since we put the words in a hash table, we do not allow the same
     letter to be displayed two times */

  do {
    lettersItem->word = get_random_word();
  } while(item_find_by_title(lettersItem->word)!=NULL && maxtry--);

  if(maxtry==0) {
    return NULL;
  }

  /* fill up the overword with zeros */
  lettersItem->overword=calloc(strlen(lettersItem->word), 1);
  lettersItem->charcounter=0;

  lettersItem->rootitem = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_group_get_type (),
			   "x", (double)(rand()%(gcomprisBoard->width-170)),
			   "y", (double) -12,
			   NULL);

  /* To 'erase' words, I create 2 times the text item. One is empty now */
  /* It will be filled each time the user enters the right key         */  
  item2 = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(lettersItem->rootitem),
			   gnome_canvas_text_get_type (),
			   "text", lettersItem->word,
			   "font", gcompris_skin_font_board_huge_bold,
			   "x", (double) 0,
			   "y", (double) 0,
			   "anchor", GTK_ANCHOR_NW,
			   "fill_color", "black",
			   NULL);

  lettersItem->overwriteItem = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(lettersItem->rootitem),
			   gnome_canvas_text_get_type (),
			   "text", "",
			   "font", gcompris_skin_font_board_huge_bold,
			   "x", (double) 0,
			   "y", (double) 0,
			   "anchor", GTK_ANCHOR_NW,
			   "fill_color", "blue",
			   NULL);

  item_list = g_list_append (item_list, lettersItem);

  /* Add word to hash table of all falling words. */
  g_hash_table_insert (words_table, lettersItem->word, lettersItem);

  return (lettersItem->rootitem);
}

static void wordsgame_add_new_item() 
{
  wordsgame_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gint wordsgame_drop_items (GtkWidget *widget, gpointer data)
{
  wordsgame_add_new_item();

  drop_items_id = gtk_timeout_add (fallSpeed,
				   (GtkFunction) wordsgame_drop_items, NULL);
  return (FALSE);
}

static void player_win(LettersItem *item)
{

  gcompris_play_ogg ("gobble", NULL);

  gcomprisBoard->sublevel++;
  gcompris_score_set(gcomprisBoard->sublevel);

  wordsgame_destroy_item(item);

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) 
    {
      /* Try the next level */
      gcomprisBoard->level++;
      gcomprisBoard->sublevel = 1;
      if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
	board_finished(BOARD_FINISHED_RANDOM);
	return;
      }
      wordsgame_next_level();
      gcompris_play_ogg ("bonus", NULL);
    }
  else
    {
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
					     (GtkFunction) wordsgame_drop_items, NULL);
	  }
	}
    }
}

static void player_loose()
{
  gcompris_play_ogg ("crash", NULL);
}

static LettersItem *
item_find_by_title (const gchar *title)
{
  if (!words_table)
    return NULL;
  
  return g_hash_table_lookup (words_table, title);
}

static FILE *get_wordfile(char *locale)
{
  char *filename;
  FILE *wordsfd = NULL;

  /* First Try to find a file matching the level and the locale */
  filename = g_strdup_printf("%s%s%d.%.2s",  
			     PACKAGE_DATA_DIR, "/wordsgame/wordslevel", 
			     gcomprisBoard->level, locale);
  g_message("Trying to open file %s ", filename);
  wordsfd = fopen (filename, "r");

  if(wordsfd==NULL)
    {
      /* Second Try to find a file matching the 'max' and the locale */
      sprintf(filename, "%s%s%.2s",  
	      PACKAGE_DATA_DIR, "/wordsgame/wordslevelmax.", 
	      locale);
      g_message("Trying to open file %s ", filename);

      wordsfd = fopen (filename, "r");
    }

  g_free(filename);

  return wordsfd;
}
/*
 * Return a random word from a set of text file depending on 
 * the current level and language
 */
static gchar *get_random_word()
{
  FILE *wordsfd;
  long size, i;
  gchar *str;

  str = g_malloc(MAXWORDSLENGTH);

  wordsfd = get_wordfile(gcompris_get_locale());

  if(wordsfd==NULL)
    {
      /* Try to open the english locale by default */
      wordsfd = get_wordfile("en");
      
      /* Too bad, even english is not there. Check your Install */
      if(wordsfd==NULL)
	g_error("Cannot open file %s : Check your GCompris install", strerror(errno));
    }

  fseek (wordsfd, 0L, SEEK_END);
  size = ftell (wordsfd);

  i=rand()%size;
  fseek(wordsfd, i, SEEK_SET);

  /* Read 2 times so that we are sure to sync on an end of line */
  fgets(str, MAXWORDSLENGTH, wordsfd);
  if(ftell(wordsfd)==size)
    rewind(wordsfd);
  fgets(str, MAXWORDSLENGTH, wordsfd);

  /* Chop the return */
  str[strlen(str)-1]='\0';

  fclose(wordsfd);

  return (str);
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
