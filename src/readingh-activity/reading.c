/* gcompris - reading.c
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
#define MAXWORDSLENGTH 50
#define MAX_WORDS 100


static GcomprisWordlist *gc_wordlist = NULL;

static GcomprisBoard *gcomprisBoard = NULL;

static gint drop_items_id    = 0;
static gint next_level_timer = 0;

static gchar *textToFind = NULL;
static gint textToFindIndex = 0;
#define NOT_THERE -1000

static GooCanvasItem *boardRootItem = NULL;

static gboolean uppercase_only;

typedef enum
{
  MODE_HORIZONTAL		= 0,
  MODE_VERTICAL			= 1,
  MODE_HORIZONTAL_RTL		= 2
} Mode;
static Mode currentMode = MODE_VERTICAL;

/* Store the moving words */
typedef struct {
  GooCanvasItem *rootItem;
  GooCanvasItem *overwriteItem;
  GooCanvasItem *item;
} LettersItem;

static LettersItem previousFocus;
static LettersItem toDeleteFocus;


/* Define the page area where text can be displayed */
#define BASE_X1 70
#define BASE_Y1 120
#define BASE_X2 350
#define BASE_Y2 520
#define BASE_CX  BASE_X1+(BASE_X2-BASE_X1)/2

static gint current_x;
static gint current_y;
static gint numberOfLine;
static gint font_size;
static gint interline;


static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);
static int		 wait_for_ready;
static int		 gamewon;

static gboolean		 reading_create_item(GooCanvasItem *parent);
static gboolean		 reading_drop_items (gpointer data);
static void		 reading_destroy_all_items(void);
static gint		 reading_next_level(void);
static void		 reading_config_start(GcomprisBoard *agcomprisBoard,
					     GcomprisProfile *aProfile);
static void		 reading_config_stop(void);

static void		 player_win(void);
static void		 player_loose(void);
static gchar		*get_random_word(const gchar *except);
static GooCanvasItem	*display_what_to_do(GooCanvasItem *parent);
static void		 ask_ready(gboolean status);
static void		 ask_yes_no(void);
static gboolean		 item_event_valid (GooCanvasItem  *item,
					   GooCanvasItem  *target,
					   GdkEventButton *event,
					   gpointer data);

static  guint32          fallSpeed = 0;

/* Description of this plugin */
static BoardPlugin menu_bp =
{
   NULL,
   NULL,
   "Reading",
   "Read a list of words and then work out if the given word is in it",
   "Bruno Coudoin <bruno.coudoin@free.fr>",
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
   reading_config_start,
   reading_config_stop
};

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(reading)

/*
 * in : boolean TRUE = PAUSE : FALSE = UNPAUSE
 *
 */
static void pause_board (gboolean pause)
{
  // after the bonus is ended, the board is unpaused, but we must wait for
  // the player to be ready (this board does not use the same framework as others)
  if (wait_for_ready)
    return;

  if(gcomprisBoard==NULL)
    return;

  if(pause)
    {
      if (drop_items_id) {
	g_source_remove (drop_items_id);
	drop_items_id = 0;
      }
    }
  else
    {
      if(!drop_items_id) {
	reading_drop_items(NULL);
      }
    }
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

  g_hash_table_destroy(config);

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			"readingh/reading-bg.svgz");
      wait_for_ready = TRUE;
      gamewon = FALSE;

      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 9;
      gc_bar_set(GC_BAR_CONFIG|GC_BAR_LEVEL);
      gc_bar_location(BOARDWIDTH-240, -1, 0.7);
      PangoFontDescription *font_medium = pango_font_description_from_string(gc_skin_font_board_medium);
      font_size = PANGO_PIXELS(pango_font_description_get_size (font_medium));
      interline = (int) (1.5*font_size);

      PangoContext *pango_context = gtk_widget_get_pango_context  (GTK_WIDGET(agcomprisBoard->canvas));

      PangoFontMetrics* pango_metrics =  pango_context_get_metrics (pango_context,
								    font_medium,
								    pango_language_from_string   (gc_locale_get()));
      pango_font_description_free(font_medium);

      int ascent = PANGO_PIXELS(pango_font_metrics_get_ascent (pango_metrics));
      int descent = PANGO_PIXELS(pango_font_metrics_get_descent (pango_metrics));

      pango_font_metrics_unref(pango_metrics);

      interline = ascent + descent;

      g_warning ("Font to display words have size %d  ascent : %d, descent : %d.\n Set inerline to %d",
		 font_size, ascent, descent, interline);

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


      currentMode=MODE_VERTICAL; // Default mode
      if(gcomprisBoard->mode && g_strcasecmp(gcomprisBoard->mode, "horizontal")==0)
        {
          if (pango_unichar_direction(g_utf8_get_char(gc_wordlist_random_word_get(gc_wordlist, gcomprisBoard->level))) == PANGO_DIRECTION_RTL)
              currentMode=MODE_HORIZONTAL_RTL;
          else
               currentMode=MODE_HORIZONTAL;
        }

       reading_next_level();
    }
}

static void
end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      reading_destroy_all_items();
    }

  if (gc_wordlist != NULL){
    gc_wordlist_free(gc_wordlist);
    gc_wordlist = NULL;
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
      reading_next_level();
    }
}

gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "reading")==0)
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
static gint reading_next_level()
{

  gc_bar_set_level(gcomprisBoard);

  gamewon = FALSE;

  reading_destroy_all_items();

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  /* Default speed */
  fallSpeed = 1400-gcomprisBoard->level*120;

  if(currentMode==MODE_VERTICAL)
    {
      current_x = BASE_CX;
      numberOfLine = 7 + gcomprisBoard->level;
    }
  else
    {
      current_x = BASE_X2;
      numberOfLine = 2 + gcomprisBoard->level;
    }

  current_y = BASE_Y1 - 2 * interline;

  gcomprisBoard->number_of_sublevel = 1;
  gcomprisBoard->sublevel = 1;

  display_what_to_do(boardRootItem);
  ask_ready(TRUE);
  return (FALSE);
}

/* Destroy all the items */
static void
reading_destroy_all_items()
{

  if (drop_items_id) {
    g_source_remove (drop_items_id);
    drop_items_id = 0;
  }

  if (next_level_timer) {
    g_source_remove (next_level_timer);
    drop_items_id = 0;
  }

  if(boardRootItem!=NULL)
      goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
  previousFocus.rootItem = NULL;
  toDeleteFocus.rootItem = NULL;

  if (textToFind!=NULL)
    {
    g_free(textToFind);
    textToFind=NULL;
    }
}

static GooCanvasItem *
display_what_to_do(GooCanvasItem *parent)
{

  gint base_Y = 90;
  gint base_X = 570;

  /* Load the text to find */

  textToFind = get_random_word(NULL);

  g_assert(textToFind != NULL);

  /* Decide now if this time we will display the text to find */
  /* Use this formula to have a better random number see 'man 3 rand' */
  if(g_random_boolean())
      textToFindIndex = g_random_int_range(0, numberOfLine);
  else
    textToFindIndex = NOT_THERE;

  goo_canvas_text_new (parent,
		       _("Please, check if the word"),
		       (double) base_X,
		       (double) base_Y,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_board_medium,
		       "fill-color", "black",
		       NULL);

  goo_canvas_text_new (parent,
		       textToFind,
		       (double) base_X,
		       (double) base_Y + 30,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_board_big,
		       "fill-color", "blue",
		       NULL);

  goo_canvas_text_new (parent,
		       _("is being displayed"),
		       (double) base_X,
		       (double) base_Y + 60,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_board_medium,
		       "fill-color", "black",
		       NULL);


  return NULL;
}

static gboolean
reading_create_item(GooCanvasItem *parent)
{
  gint   anchor = GTK_ANCHOR_CENTER;
  gchar *word;

  g_assert(textToFind!=NULL);

  if(toDeleteFocus.rootItem)
    {
      goo_canvas_item_remove(toDeleteFocus.rootItem);
      toDeleteFocus.rootItem = NULL;
    }

  if(previousFocus.rootItem)
    {
      g_object_set (previousFocus.overwriteItem,
		    "visibility", GOO_CANVAS_ITEM_VISIBLE,
		    NULL);
      toDeleteFocus.rootItem = previousFocus.rootItem;
    }

  if(numberOfLine<=0)
    {
      goo_canvas_item_remove(toDeleteFocus.rootItem);
      toDeleteFocus.rootItem = NULL;

      ask_yes_no();
      return FALSE;
    }


  if(textToFindIndex!=0)
    {
      word = get_random_word(textToFind);
    }
  else
    {
      word = g_strdup(textToFind);
    }

  if(word==NULL)
  {
  	gc_dialog(_("We skip this level because there are not enough words in the list!"),
		(DialogBoxCallBack)reading_next_level);
	gcomprisBoard->level++;
	if(gcomprisBoard->level>gcomprisBoard->maxlevel)
	  gcomprisBoard->level = gcomprisBoard->maxlevel;
	return FALSE;
  }

  if(textToFindIndex>=0)
    textToFindIndex--;

  previousFocus.rootItem = \
    goo_canvas_group_new (parent,
			  NULL);

  goo_canvas_item_translate(previousFocus.rootItem,
			    current_x,
			    current_y);

  if(currentMode==MODE_HORIZONTAL)
    anchor=GTK_ANCHOR_WEST;
  else if (currentMode==MODE_HORIZONTAL_RTL)
    anchor=GTK_ANCHOR_EAST;

  previousFocus.item = \
    goo_canvas_text_new (previousFocus.rootItem,
			 word,
			 (double) 0,
			 (double) 0,
			 -1,
			 anchor,
			 "font", gc_skin_font_board_medium,
			 "fill-color", "black",
			 NULL);


  gchar *oldword = g_strdup_printf("<span foreground=\"black\" background=\"black\">%s</span>", word);

  g_free(word);

  previousFocus.overwriteItem = \
    goo_canvas_text_new (previousFocus.rootItem,
			 oldword,
			 0,
			 0,
			 -1,
			 anchor,
			 "font", gc_skin_font_board_medium,
			 "use-markup", TRUE,
			 NULL);

  g_free(oldword);
  g_object_set (previousFocus.overwriteItem,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

  // Calculate the next spot
  if(currentMode==MODE_VERTICAL)
    {
      current_y += interline;
      numberOfLine--;
    }
  else if (currentMode==MODE_HORIZONTAL_RTL)
    {
      GooCanvasBounds bounds;
      goo_canvas_item_get_bounds(previousFocus.rootItem, &bounds);

      // Are we out of bound
      if(bounds.x1<BASE_X1)
	{
	  // Do the line Wrapping
	  goo_canvas_item_translate(previousFocus.rootItem, BASE_X2-bounds.x2, interline);
	  current_y += interline;
	  current_x = BASE_X2;
	  numberOfLine--;
	}
      current_x -= bounds.x2-bounds.x1 + font_size;
     }

  else
    {
      GooCanvasBounds bounds;

      goo_canvas_item_get_bounds(previousFocus.rootItem, &bounds);

      // Are we out of bound
      if(bounds.x2 > BASE_X2)
	{
	  // Do the line Wrapping
	  goo_canvas_item_translate(previousFocus.rootItem,
				    BASE_X1-bounds.x1, interline);
	  current_y += interline;
	  current_x = BASE_X1;
	  numberOfLine--;
	}
      current_x += bounds.x2 - bounds.x1 + font_size;
    }

  return (TRUE);
}

/*
 * This is called on a low frequency and is used to display new items
 *
 */
static gboolean
reading_drop_items (gpointer data)
{

  if(reading_create_item(boardRootItem))
    drop_items_id = g_timeout_add (fallSpeed,
				   reading_drop_items, NULL);
  return (FALSE);
}

static GooCanvasItem *
addBackground(GooCanvasItem *parent,
	      GooCanvasItem *item)
{
  GooCanvasBounds bounds;
  int gap = 8;

  goo_canvas_item_get_bounds (item, &bounds);

  return(goo_canvas_rect_new (parent,
			      bounds.x1 - gap,
			      bounds.y1 - gap,
			      bounds.x2 - bounds.x1 + gap*2,
			      bounds.y2 - bounds.y1 + gap*2,
			      "stroke_color_rgba", 0xFFFFFFFFL,
			      "fill_color_rgba", 0XD5C393FFL,
			      "line-width", (double) 2,
			      "radius-x", (double) 10,
			      "radius-y", (double) 10,
			      NULL) );
}

static void
ask_ready(gboolean status)
{
  static GooCanvasItem *item1 = NULL;
  static GooCanvasItem *item2 = NULL;
  double x_offset = 560;
  double y_offset = 260;

  if(textToFind==NULL)
    return;

  if(status==FALSE)
    {
      gc_item_focus_remove(item1, NULL);
      gc_item_focus_remove(item2, item1);

      if(item1!=NULL)
	goo_canvas_item_remove(item1);

      if(item2!=NULL)
	goo_canvas_item_remove(item2);

      item1 = NULL;
      item2 = NULL;
      return;
    }

  /*----- READY -----*/
  item2 = goo_canvas_text_new (boardRootItem,
			       _("I am Ready"),
			       x_offset,
			       y_offset,
			       -1,
			       GTK_ANCHOR_CENTER,
			       "font", gc_skin_font_board_big,
			       "fill-color", "white",
				NULL);

  g_signal_connect(item2, "button-press-event",
		   (GCallback) item_event_valid,
		   "R");

  item1 = addBackground(boardRootItem, item2);

  g_signal_connect(item1, "button-press-event",
		   (GCallback) item_event_valid,
		   "R");
  gc_item_focus_init(item1, NULL);
  gc_item_focus_init(item2, item1);
  goo_canvas_item_raise(item2, NULL);
}

static void
ask_yes_no()
{
  GooCanvasItem *item1;
  GooCanvasItem *item2;
  double x_offset = 560;
  double y_offset = 260;

  if(textToFind==NULL)
    return;

  /*----- YES -----*/

  item2 =
    goo_canvas_text_new (boardRootItem,
			 _("Yes, I saw it"),
			 x_offset,
			 y_offset,
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_board_big,
			 "fill-color", "white",
			 NULL);

  item1 = addBackground(boardRootItem, item2);

  g_signal_connect(item2, "button-press-event",
		   (GCallback) item_event_valid,
		   "Y");
  g_signal_connect(item1, "button-press-event",
		   (GCallback) item_event_valid,
		   "Y");

  gc_item_focus_init(item1, NULL);
  gc_item_focus_init(item2, item1);
  goo_canvas_item_raise(item2, NULL);

  /*----- NO -----*/
  y_offset += 100;

  item2 =
    goo_canvas_text_new (boardRootItem,
			 _("No, it was not there"),
			 x_offset,
			 y_offset,
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_board_big,
			 "fill-color", "white",
			 NULL);

  item1 = addBackground(boardRootItem, item2);

  g_signal_connect(item2, "button-press-event",
		   (GCallback) item_event_valid,
		   "N");
  g_signal_connect(item1, "button-press-event",
		   (GCallback) item_event_valid,
		   "N");

  gc_item_focus_init(item1, NULL);
  gc_item_focus_init(item2, item1);
  goo_canvas_item_raise(item2, NULL);
}


static void
player_win()
{
  gamewon = TRUE;
  wait_for_ready = TRUE;
  gc_bonus_display(gamewon, GC_BONUS_FLOWER);
  /* Try the next level */
  gcomprisBoard->level++;
  if(gcomprisBoard->level>gcomprisBoard->maxlevel)
    gcomprisBoard->level = gcomprisBoard->maxlevel;

  next_level_timer = g_timeout_add(3000, (GSourceFunc)reading_next_level, NULL);
}

static void
player_loose()
{
  gchar *expected;
  gchar *got;
  gamewon = FALSE;
  wait_for_ready = TRUE;

  /* Report what was wrong in the log */
  expected = g_strdup_printf(_("The word to find was '%s'"), textToFind);

  if(textToFindIndex == NOT_THERE)
    got    = g_strdup_printf(_("But it was not displayed"));
  else
    got    = g_strdup_printf(_("And it was displayed"));

  gc_log_set_comment (gcomprisBoard, expected, got);

  g_free(expected);
  g_free(got);

  gc_bonus_display(gamewon, GC_BONUS_FLOWER);

  next_level_timer = g_timeout_add(3000, (GSourceFunc)reading_next_level, NULL);
}

/* Callback for the yes and no buttons */
static gboolean
item_event_valid (GooCanvasItem  *item,
		  GooCanvasItem  *target,
		  GdkEventButton *event,
		  gpointer data)
{

  if (((char *)data)[0]=='R')
    {
      // The user is Ready
      wait_for_ready = FALSE;
      ask_ready(FALSE);
      pause_board(FALSE);
    }
  else if(!wait_for_ready) {
    if ((((char *)data)[0]=='Y' && textToFindIndex == -1)
	|| (((char *)data)[0]=='N' && textToFindIndex == NOT_THERE))
      {
	player_win();
      }
    else
      {
	player_loose();
      }
  }
  return TRUE;
}


/** Return a random word from a set of text file depending on
 *  the current level and language
 *
 * \param except: if non NULL, never return this value
 *
 * \return a random word from. must be freed by the caller
 */
static gchar *
get_random_word(const gchar* except)
{
  gchar *word;
  int count=0;

  word = gc_wordlist_random_word_get(gc_wordlist, gcomprisBoard->level);

  if(except)
    while(strcmp(except, word)==0)
      {
	g_free(word);

	if(count++>100)
	{
		word = NULL;
		break;
	}
	word = gc_wordlist_random_word_get(gc_wordlist, gcomprisBoard->level);
      }

  if (word && uppercase_only)
    {
      gchar *old = word;
      word = g_utf8_strup(old, -1);
      g_free(old);
    }


  return(word);
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

static gboolean
conf_ok(GHashTable *table)
{
  if (!table){
    if (gcomprisBoard)
      pause_board(FALSE);
    return TRUE;
  }

  g_hash_table_foreach(table, (GHFunc) save_table, NULL);

  if (gcomprisBoard){
    GHashTable *config;

    if (profile_conf)
      config = gc_db_get_board_conf();
    else
      config = table;

    gc_locale_set(g_hash_table_lookup( config, "locale"));

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

    reading_next_level();

    pause_board(FALSE);
  }

  board_conf = NULL;
  profile_conf = NULL;
  return TRUE;
}

static void
reading_config_start(GcomprisBoard *agcomprisBoard,
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

  /* upper case */
  gboolean up_init = FALSE;
  gchar *up_init_str = g_hash_table_lookup( config, "uppercase_only");

  if (up_init_str && (strcmp(up_init_str, "True")==0))
    up_init = TRUE;

  gc_board_config_boolean_box(conf, _("Uppercase only text"),
			      "uppercase_only",
			      up_init);

}


/* ======================= */
/* = config_stop        = */
/* ======================= */
static void
reading_config_stop()
{
}
