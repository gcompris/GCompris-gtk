/* gcompris - wordsgame.c
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
#include "gcompris/gameutil.h"


#define SOUNDLISTFILE PACKAGE
#define MAXWORDSLENGTH 50
static GcomprisWordlist *gc_wordlist = NULL;

GStaticRWLock items_lock = G_STATIC_RW_LOCK_INIT;
GStaticRWLock items2del_lock = G_STATIC_RW_LOCK_INIT;

/*
  word - word to type
  overword - part of word allready typed
  count - number of allready typed letters in word
  pos - pointer to current position in word
  letter - current expected letter to type
*/
typedef struct {
  GooCanvasItem *rootitem;
  GooCanvasItem *overwriteItem;
  gchar *word;
  gchar *overword;
  gint  count;
  gchar *pos;
  gchar *letter;
} LettersItem;

/*
  items - array of displayed items
  items2del - array of items where moved offscreen
  item_on_focus -  item on focus in array items. NULL - not set.
*/

static GPtrArray 	*items=NULL;
static GPtrArray 	*items2del=NULL;
static LettersItem 	*item_on_focus=NULL;


static GcomprisBoard *gcomprisBoard = NULL;

static gint dummy_id = 0;
static gint drop_items_id = 0;


static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void 		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);
static gint		 key_press(guint keyval, gchar *commit_str, gchar *preedit_str);

static GooCanvasItem	 *wordsgame_create_item(GooCanvasItem *parent);
static gint		 wordsgame_drop_items (GtkWidget *widget, gpointer data);
static gint		 wordsgame_move_items (GtkWidget *widget, gpointer data);
static void		 wordsgame_destroy_item(LettersItem *item);
static gboolean		 wordsgame_destroy_items(GPtrArray *items);
static void		 wordsgame_destroy_all_items(void);
static void		 wordsgame_next_level(void);
static void		 wordsgame_add_new_item(void);
static void		 wordsgame_config_start(GcomprisBoard *agcomprisBoard,
					     GcomprisProfile *aProfile);
static void		 wordsgame_config_stop(void);


static void		 player_win(LettersItem *item);
static void		 player_loose(void);


#define MAX_FALLSPEED  7000
#define MAX_SPEED  150
#define MIN_FALLSPEED  3000
#define MIN_SPEED  50
#define DEFAULT_FALLSPEED  7000
#define DEFAULT_SPEED  150

#define INCREMENT_FALLSPEED  1000
#define INCREMENT_SPEED  10


static  guint32              fallSpeed = 0;
static  double               speed = 0.0;

static GooCanvasItem *preedit_text = NULL;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Falling Words"),
    N_("Type the falling words before they reach the ground"),
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
    wordsgame_config_start,
    wordsgame_config_stop
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(wordsgame)

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
      if(!drop_items_id) {
	drop_items_id = g_timeout_add (0,
				       (GtkFunction) wordsgame_drop_items, NULL);
      }
      if(!dummy_id) {
	dummy_id = g_timeout_add (10, (GtkFunction) wordsgame_move_items, NULL);
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

      GHashTable *config = gc_db_get_board_conf();
      gc_locale_change(g_hash_table_lookup( config, "locale"));
      g_hash_table_destroy(config);

      /* disable im_context */
      //gcomprisBoard->disable_im_context = TRUE;

      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			"wordsgame/scenery_background.png");


      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 6;
      gcomprisBoard->sublevel = 0;
      gc_bar_set(GC_BAR_LEVEL|GC_BAR_CONFIG);

      /* Default speed */
      speed=DEFAULT_SPEED;
      fallSpeed=DEFAULT_FALLSPEED;

      gc_wordlist = gc_wordlist_get_from_file("wordsgame/default-$LOCALE.xml");

      if(!gc_wordlist)
	{
	  /* Fallback to english */
	  gc_wordlist = gc_wordlist_get_from_file("wordsgame/default-en.xml");

	  if(!gc_wordlist)
	    {
	      gcomprisBoard = NULL;
	      gc_dialog(_("Error: We can't find\na list of words to play this game.\n"), gc_board_end);
	      return;
	    }
	}

      wordsgame_next_level();
    }
}

static void
end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gc_score_end();
      wordsgame_destroy_all_items();
      if (preedit_text){
	goo_canvas_item_remove(preedit_text);
	preedit_text=NULL;
      }
      gc_im_reset();
      gcomprisBoard = NULL;

      if (gc_wordlist != NULL){
	gc_wordlist_free(gc_wordlist);
	gc_wordlist = NULL;
      }
    }

  gc_locale_reset();
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


static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
{
  gchar *letter;
  gint i;
  LettersItem *item;
  gchar *str;
  gunichar unichar_letter;

  if(!gcomprisBoard)
    return FALSE;

  if (keyval){
    g_warning("keyval %d", keyval);
    return TRUE;
  }


  if (preedit_str){
    g_warning("preedit_str %s", preedit_str);
    /* show the preedit string on bottom of the window */
    GcomprisProperties	*properties = gc_prop_get ();
    gchar *text;
    PangoAttrList *attrs;
    gint cursor_pos;
    gtk_im_context_get_preedit_string (properties->context,
				       &text,
				       &attrs,
				       &cursor_pos);

    if (!preedit_text)
      preedit_text = \
	goo_canvas_text_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
			     "",
			     BOARDWIDTH/2,
			     BOARDHEIGHT - 100,
			     -1,
			     GTK_ANCHOR_N,
			     "font", gc_skin_font_board_huge_bold,
			     //"fill_color_rgba", 0xba00ffff,
			     NULL);


    g_object_set (preedit_text,
		  "text", text,
		  "attributes", attrs,
		  NULL);

    return TRUE;

  }

  /* commit str */
  g_warning("commit_str %s", commit_str);

  str = commit_str;

  for (i=0; i < g_utf8_strlen(commit_str,-1); i++){
    unichar_letter = g_utf8_get_char(str);
    str = g_utf8_next_char(str);
    if(!g_unichar_isalnum (unichar_letter)){
      player_loose();
      return FALSE;
    }

    letter = g_new0(gchar,6);
    g_unichar_to_utf8 (unichar_letter, letter);

    if(item_on_focus==NULL)
      {
	g_static_rw_lock_reader_lock (&items_lock);
	gint count=items->len;
	g_static_rw_lock_reader_unlock (&items_lock);

	for (i=0;i<count;i++)
	  {
	    g_static_rw_lock_reader_lock (&items_lock);
	    item=g_ptr_array_index(items,i);
	    g_static_rw_lock_reader_unlock (&items_lock);
	    g_assert (item!=NULL);
	    if (strcmp(item->letter,letter)==0)
	      {
		item_on_focus=item;
		break;
	      }
	  }
      }


    if(item_on_focus!=NULL)
      {

	if(strcmp(item_on_focus->letter, letter)==0)
	  {
	    gchar *tmpstr;
	    item_on_focus->count++;
	    g_free(item_on_focus->overword);
	    tmpstr = g_utf8_strndup(item_on_focus->word,
				    item_on_focus->count);
	    /* Add the ZERO WIDTH JOINER to force joined char in Arabic and Hangul
	     *  http://en.wikipedia.org/wiki/Zero-width_joiner
	     */
	    item_on_focus->overword = g_strdup_printf("%s%lc", tmpstr, 0x200D);
	    g_free(tmpstr);
	    g_object_set (item_on_focus->overwriteItem,
			  "text", item_on_focus->overword,
			  NULL);


	    if (item_on_focus->count<g_utf8_strlen(item_on_focus->word,-1))
	      {
		g_free(item_on_focus->letter);
		item_on_focus->letter=g_utf8_strndup(item_on_focus->pos,1);
		item_on_focus->pos=g_utf8_find_next_char(item_on_focus->pos,NULL);
	      }
	    else
	      {
		player_win(item_on_focus);
		item_on_focus=NULL;
	      }
	  }
	else
	  {
	    /* It is a loose : unselect the word and defocus */
	    g_free(item_on_focus->overword);
	    item_on_focus->overword=g_strdup(" ");
	    item_on_focus->count=0;
	    g_free(item_on_focus->letter);
	    item_on_focus->letter=g_utf8_strndup(item_on_focus->word,1);

	    item_on_focus->pos=g_utf8_find_next_char(item_on_focus->word,NULL);

	    g_object_set (item_on_focus->overwriteItem,
			  "text", item_on_focus->overword,
			  NULL);
	    item_on_focus=NULL;
	    g_free(letter);
	    player_loose();
	    break;
	  }
      }
    else
      {
	/* Anyway kid you clicked on the wrong key */
	player_loose();
	g_free(letter);
	break;
      }

    g_free(letter);
  }
  return TRUE;
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
  gc_score_start(SCORESTYLE_NOTE,
		 BOARDWIDTH - 195,
		 BOARDHEIGHT - 30,
		 gcomprisBoard->number_of_sublevel);

  gc_bar_set_level(gcomprisBoard);
  gc_score_set(gcomprisBoard->sublevel);

  wordsgame_destroy_all_items();

  if (preedit_text){
    goo_canvas_item_remove(preedit_text);
    preedit_text=NULL;
  }
  gc_im_reset();

  items=g_ptr_array_new();
  items2del=g_ptr_array_new();


  /* Increase speed only after 5 levels */
  if(gcomprisBoard->level > 5)
    {
      gint temp = fallSpeed-gcomprisBoard->level*200;
      if (temp > MIN_FALLSPEED)	fallSpeed=temp;
    }

  pause_board(FALSE);
}


static void wordsgame_move_item(LettersItem *item)
{
  GooCanvasBounds bounds;


  goo_canvas_item_translate(item->rootitem, 0, 2.0);

  goo_canvas_item_get_bounds (item->rootitem,
			      &bounds);

  if(bounds.y1>BOARDHEIGHT) {

    if (item == item_on_focus)
      item_on_focus = NULL;

    g_static_rw_lock_writer_lock (&items_lock);
    g_ptr_array_remove (items, item);
    g_static_rw_lock_writer_unlock (&items_lock);

    g_static_rw_lock_writer_lock (&items2del_lock);
    g_ptr_array_add (items2del, item);
    g_static_rw_lock_writer_unlock (&items2del_lock);

    g_timeout_add (100,(GtkFunction) wordsgame_destroy_items, items2del);

    player_loose();
  }
}

/*
 * This does the moves of the game items on the play canvas
 *
 */
static gint wordsgame_move_items (GtkWidget *widget, gpointer data)
{
  g_assert (items!=NULL);
  gint i;
  LettersItem *item;

  for (i=items->len-1;i>=0;i--)
    {

      g_static_rw_lock_reader_lock (&items_lock);
      item=g_ptr_array_index(items,i);
      g_static_rw_lock_reader_unlock (&items_lock);
      wordsgame_move_item(item);
    }
  dummy_id = g_timeout_add (gc_timing (speed, items->len),
          (GtkFunction) wordsgame_move_items, NULL);
  return (FALSE);
}



static void wordsgame_destroy_item(LettersItem *item)
{

  /* The items are freed by player_win */
  goo_canvas_item_remove(item->rootitem);
  g_free(item->word);
  g_free(item->overword);
  g_free(item->letter);
  g_free(item);
}

/* Destroy items that falls out of the canvas */
static gboolean wordsgame_destroy_items(GPtrArray *item_list)
{
  LettersItem *item;

  g_assert(item_list!=NULL);



  if  (item_list==items) {
    g_static_rw_lock_writer_lock (&items_lock);
    while (item_list->len>0)
      {
	item = g_ptr_array_index(item_list,0);
	g_ptr_array_remove_index_fast(item_list,0);
	wordsgame_destroy_item(item);
      }
    g_static_rw_lock_writer_unlock (&items_lock);
  }

  if  (item_list==items2del) {
    g_static_rw_lock_writer_lock (&items2del_lock);
    while (item_list->len>0)
      {
	item = g_ptr_array_index(item_list,0);
	g_ptr_array_remove_index_fast(item_list,0);
	wordsgame_destroy_item(item);
      }
    g_static_rw_lock_writer_unlock (&items2del_lock);
  }

  return (FALSE);
}

/* Destroy all the items */
static void wordsgame_destroy_all_items()
{

  if (items!=NULL){
    if(items->len > 0) {
      wordsgame_destroy_items(items);
    }
    g_ptr_array_free (items, TRUE);
    items=NULL;
  }

  if (items2del!=NULL){
    if(items2del->len > 0) {
      wordsgame_destroy_items(items2del);
    }
    g_ptr_array_free (items2del, TRUE);
    items2del=NULL;
  }

}


static GooCanvasItem *wordsgame_create_item(GooCanvasItem *parent)
{

  GooCanvasItem *item2;
  LettersItem *item;
  gchar *word = gc_wordlist_random_word_get(gc_wordlist, gcomprisBoard->level);
  GtkAnchorType direction_anchor = GTK_ANCHOR_NW;

  if(!word)
    /* Should display the dialog box here */
    return NULL;

  // create and init item
  item = g_new(LettersItem,1);
  item->word = word;
  item->overword=g_strdup("");
  item->count=0;
  item->letter=g_utf8_strndup(item->word,1);
  item->pos=g_utf8_find_next_char(item->word,NULL);

  if (pango_unichar_direction(g_utf8_get_char(item->word)))
    direction_anchor = GTK_ANCHOR_NE;

  item->rootitem = goo_canvas_group_new (parent, NULL);
  goo_canvas_item_translate(item->rootitem, 0, -12);


  /* To 'erase' words, I create 2 times the text item. One is empty now */
  /* It will be filled each time the user enters the right key         */
  item2 = \
    goo_canvas_text_new (item->rootitem,
			 item->word,
			 (double) 0,
			 (double) 0,
			 -1,
			 direction_anchor,
			 "font", gc_skin_font_board_huge_bold,
			 "fill_color_rgba", 0xba00ffff,
			 NULL);

  item->overwriteItem = \
    goo_canvas_text_new (item->rootitem,
			 item->overword,
			 (double) 0,
			 (double) 0,
			 -1,
			 direction_anchor,
			 "font", gc_skin_font_board_huge_bold,
			 "fill-color", "blue",
			 NULL);

  /*set right x position */

  GooCanvasBounds bounds;

  goo_canvas_item_get_bounds    (item->rootitem,
				 &bounds);

 if(direction_anchor == GTK_ANCHOR_NW)
   goo_canvas_item_translate (item->rootitem,
			      (g_random_int()%(BOARDWIDTH-(gint)(bounds.x2))),
			      0);
  else
   {
      double new_x = (double)( g_random_int()%BOARDWIDTH);
      if ( new_x < -bounds.x1 )
	new_x -=  bounds.x1;
      goo_canvas_item_translate (item->rootitem,
				 new_x ,(double) 0);
   }


  g_static_rw_lock_writer_lock (&items_lock);
  g_ptr_array_add(items, item);
  g_static_rw_lock_writer_unlock (&items_lock);

  return (item->rootitem);
}

static void wordsgame_add_new_item()
{

  g_assert(gcomprisBoard->canvas!=NULL);
  wordsgame_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));

}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gint wordsgame_drop_items (GtkWidget *widget, gpointer data)
{
  gc_sound_play_ogg ("sounds/level.wav", NULL);
  wordsgame_add_new_item();
  g_source_remove(drop_items_id);
  drop_items_id = g_timeout_add (fallSpeed,(GtkFunction) wordsgame_drop_items, NULL);

  return (FALSE);
}

static void player_win(LettersItem *item)
{

  gc_sound_play_ogg ("sounds/flip.wav", NULL);

  g_assert(gcomprisBoard!=NULL);

  gcomprisBoard->sublevel++;
  gc_score_set(gcomprisBoard->sublevel);


  g_static_rw_lock_writer_lock (&items_lock);
  g_ptr_array_remove(items,item);
  g_static_rw_lock_writer_unlock (&items_lock);

  g_static_rw_lock_writer_lock (&items2del_lock);
  g_ptr_array_add(items2del,item);
  g_static_rw_lock_writer_unlock (&items2del_lock);

  g_object_set (item->rootitem, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
  g_timeout_add (500,(GtkFunction) wordsgame_destroy_items, items2del);


  if(gcomprisBoard->sublevel > gcomprisBoard->number_of_sublevel)
    {

      /* Try the next level */
      gcomprisBoard->level++;
      gcomprisBoard->sublevel = 0;
      if(gcomprisBoard->level>gcomprisBoard->maxlevel)
	gcomprisBoard->level = gcomprisBoard->maxlevel;

      wordsgame_next_level();
      gc_sound_play_ogg ("sounds/bonus.wav", NULL);
    }
  else
    {

      /* Drop a new item now to speed up the game */
      g_static_rw_lock_reader_lock (&items_lock);
      gint count=items->len;
      g_static_rw_lock_reader_unlock (&items_lock);

      if(count==0)
        {

	  if ((fallSpeed-=INCREMENT_FALLSPEED) < MIN_FALLSPEED) fallSpeed+=INCREMENT_FALLSPEED;

	  if ((speed-=INCREMENT_SPEED) < MIN_SPEED) speed+=INCREMENT_SPEED;

          if (drop_items_id) {
            /* Remove pending new item creation to sync the falls */
            g_source_remove (drop_items_id);
            drop_items_id = 0;
          }

          if(!drop_items_id) {
            drop_items_id = g_timeout_add (0,
					   (GtkFunction) wordsgame_drop_items,
					   NULL);
          }

        }
    }

}

static void player_loose()
{
  gc_sound_play_ogg ("sounds/crash.wav", NULL);
}

/* ************************************* */
/* *            Configuration          * */
/* ************************************* */


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

static void conf_ok(GHashTable *table)
{
  if (!table){
    if (gcomprisBoard)
      pause_board(FALSE);
    return;
  }

  g_hash_table_foreach(table, (GHFunc) save_table, NULL);

  if (gcomprisBoard){
    gc_locale_reset();

    GHashTable *config;

    if (profile_conf)
      config = gc_db_get_board_conf();
    else
      config = table;

    gc_locale_set(g_hash_table_lookup( config, "locale"));

    if (profile_conf)
      g_hash_table_destroy(config);

    wordsgame_next_level();

    pause_board(FALSE);
  }

  board_conf = NULL;
  profile_conf = NULL;
}

static void
wordsgame_config_start(GcomprisBoard *agcomprisBoard,
		       GcomprisProfile *aProfile)
{
  GcomprisBoardConf *conf;
  board_conf = agcomprisBoard;
  profile_conf = aProfile;

  if (gcomprisBoard)
    pause_board(TRUE);

  gchar *label = g_strdup_printf(_("<b>%s</b> configuration\n for profile <b>%s</b>"),
				 agcomprisBoard->name,
				 aProfile? aProfile->name: "");

  conf = gc_board_config_window_display( label,
				 (GcomprisConfCallback )conf_ok);

  g_free(label);

  /* init the combo to previously saved value */
  GHashTable *config = gc_db_get_conf( profile_conf, board_conf);

  gchar *locale = g_hash_table_lookup( config, "locale");

  gc_board_config_combo_locales(conf, locale);
  gc_board_config_wordlist(conf, "wordsgame/default-$LOCALE.xml");
}

static void wordsgame_config_stop(void)
{
}

