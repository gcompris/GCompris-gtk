/* gcompris - reading.c
 *
 * Time-stamp: <2006/08/21 23:34:48 bruno>
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

// FIXME: Cleanup of LettersItem created struct is not done

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

static GnomeCanvasGroup *boardRootItem = NULL;

typedef enum
{
  MODE_HORIZONTAL		= 0,
  MODE_VERTICAL			= 1
} Mode;
static Mode currentMode = MODE_VERTICAL;

/* Store the moving words */
typedef struct {
  GnomeCanvasGroup *rootItem;
  GnomeCanvasItem  *overwriteItem;
  GnomeCanvasItem  *item;
} LettersItem;

static LettersItem previousFocus;
static LettersItem toDeleteFocus;


/* Define the page area where text can be displayed */
#define BASE_X1 70
#define BASE_Y1 120
#define BASE_X2 350
#define BASE_Y2 520
#define BASE_CX  BASE_X1+(BASE_X2-BASE_X1)/2

gint current_x;
gint current_y;
gint numberOfLine;
gint font_size;
gint interline;


static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);
static int		 wait_for_ready;
static int		 gamewon;

static gboolean		 reading_create_item(GnomeCanvasGroup *parent);
static gint		 reading_drop_items (void);
//static void reading_destroy_item(LettersItem *item);
static void		 reading_destroy_all_items(void);
static gint		 reading_next_level(void);
static void		 reading_config_start(GcomprisBoard *agcomprisBoard,
					     GcomprisProfile *aProfile);
static void		 reading_config_stop(void);

static void		 player_win(void);
static void		 player_loose(void);
static gchar		*get_random_word(const gchar *except);
static GnomeCanvasItem	*display_what_to_do(GnomeCanvasGroup *parent);
static void		 ask_ready(gboolean status);
static void		 ask_yes_no(void);
static gint		 item_event_valid(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

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
	gtk_timeout_remove (drop_items_id);
	drop_items_id = 0;
      }
    }
  else
    {
      if(!drop_items_id) {
	reading_drop_items();
      }
    }
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{
  GHashTable *config = gc_db_get_board_conf();

  gc_locale_set(g_hash_table_lookup( config, "locale"));

  g_hash_table_destroy(config);

  if(agcomprisBoard!=NULL)
    {
      gchar *img;

      gcomprisBoard=agcomprisBoard;

      img = gc_skin_image_get("reading-bg.jpg");
      gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      img);
      g_free(img);
      wait_for_ready = TRUE;
      gamewon = FALSE;

      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 9;
      gc_bar_set(GC_BAR_CONFIG|GC_BAR_LEVEL);
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

      g_warning ("Font to display words have size %d  ascent : %d, descent : %d.\n Set inerline to %d", font_size, ascent, descent, interline);

      /* Default mode */
      currentMode=MODE_VERTICAL;
      if(gcomprisBoard->mode && g_strcasecmp(gcomprisBoard->mode, "horizontal")==0)
	currentMode=MODE_HORIZONTAL;

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

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  /* Default speed */
  fallSpeed=1400-gcomprisBoard->level*120;

  if(currentMode==MODE_VERTICAL)
    {
      current_x = BASE_CX;
      numberOfLine=7+gcomprisBoard->level;
    }
  else
    {
      current_x = BASE_X1;
      numberOfLine=2+gcomprisBoard->level;
    }

  current_y = BASE_Y1 - 2 * interline;

  gcomprisBoard->number_of_sublevel=1;
  gcomprisBoard->sublevel=1;

  display_what_to_do(boardRootItem);
  ask_ready(TRUE);
  return (FALSE);
}

/* Destroy all the items */
static void reading_destroy_all_items()
{

  if (drop_items_id) {
    gtk_timeout_remove (drop_items_id);
    drop_items_id = 0;
  }

  if (next_level_timer) {
    gtk_timeout_remove (next_level_timer);
    drop_items_id = 0;
  }

  if(boardRootItem!=NULL)
      gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
  previousFocus.rootItem = NULL;
  toDeleteFocus.rootItem = NULL;

  if (textToFind!=NULL)
    {
    g_free(textToFind);
    textToFind=NULL;
    }
}

static GnomeCanvasItem *display_what_to_do(GnomeCanvasGroup *parent)
{

  gint base_Y = 110;
  gint base_X = 580;

  /* Load the text to find */

  textToFind = get_random_word(NULL);

  g_assert(textToFind != NULL);

  /* Decide now if this time we will display the text to find */
  /* Use this formula to have a better random number see 'man 3 rand' */
  if(g_random_boolean())
      textToFindIndex = g_random_int_range(0, numberOfLine);
  else
    textToFindIndex = NOT_THERE;

  gnome_canvas_item_new (parent,
			 gnome_canvas_text_get_type (),
			 "text", _("Please, check if the word"),
			 "font", gc_skin_font_board_big,
			 "x", (double) base_X,
			 "y", (double) base_Y,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color", "white",
			 NULL);

  gnome_canvas_item_new (parent,
			 gnome_canvas_text_get_type (),
			 "text", textToFind,
			 "font", gc_skin_font_board_big,
			 "x", (double) base_X,
			 "y", (double) base_Y + 30,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color", "green",
			 NULL);

  gnome_canvas_item_new (parent,
			 gnome_canvas_text_get_type (),
			 "text", _("is being displayed"),
			 "font", gc_skin_font_board_big,
			 "x", (double) base_X,
			 "y", (double) base_Y + 60,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color", "white",
			 NULL);


  return NULL;
}

static gboolean reading_create_item(GnomeCanvasGroup *parent)
{
  gint   anchor = GTK_ANCHOR_CENTER;
  gchar *word;

  g_assert(textToFind!=NULL);

  if(toDeleteFocus.rootItem)
    {
      gtk_object_destroy (GTK_OBJECT(toDeleteFocus.rootItem));
      toDeleteFocus.rootItem = NULL;
    }

  if(previousFocus.rootItem)
    {
      gnome_canvas_item_show (previousFocus.overwriteItem);
      toDeleteFocus.rootItem = previousFocus.rootItem;
    }

  if(numberOfLine<=0)
    {
      gtk_object_destroy (GTK_OBJECT(toDeleteFocus.rootItem));
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

  g_assert(word!=NULL);

  if(textToFindIndex>=0)
    textToFindIndex--;

  previousFocus.rootItem = \
    GNOME_CANVAS_GROUP( gnome_canvas_item_new (parent,
					       gnome_canvas_group_get_type (),
					       "x", (double) current_x,
					       "y", (double) current_y,
					       NULL));

  if(currentMode==MODE_HORIZONTAL)
    anchor=GTK_ANCHOR_WEST;

  previousFocus.item = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(previousFocus.rootItem),
			   gnome_canvas_text_get_type (),
			   "text", word,
			   "font", gc_skin_font_board_medium,
			   "x", (double) 0,
			   "y", (double) 0,
			   "anchor", anchor,
			   "fill_color", "black",
			   NULL);


  gchar *oldword = g_strdup_printf("<span foreground=\"black\" background=\"black\">%s</span>", word);

  g_free(word);

  previousFocus.overwriteItem =						\
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(previousFocus.rootItem),
			   gnome_canvas_text_get_type (),
			   "markup", oldword,
			   "font", gc_skin_font_board_medium,
			   "x", (double) 0,
			   "y", (double) 0,
			   "anchor", anchor,
			   NULL);

  g_free(oldword);
  gnome_canvas_item_hide(previousFocus.overwriteItem);

  // Calculate the next spot
  if(currentMode==MODE_VERTICAL)
    {
      current_y += interline;
      numberOfLine--;
    }
  else
    {
      double x1, y1, x2, y2;

      gnome_canvas_item_get_bounds(GNOME_CANVAS_ITEM(previousFocus.rootItem), &x1, &y1, &x2, &y2);

      // Are we out of bound
      if(x2>BASE_X2)
	{
	  // Do the line Wrapping
	  gnome_canvas_item_move(GNOME_CANVAS_ITEM(previousFocus.rootItem), BASE_X1-x1, interline);
	  current_y += interline;
	  current_x = BASE_X1;
	  numberOfLine--;
	}
      current_x += x2-x1 + font_size;
    }

  return (TRUE);
}

/*
 * This is called on a low frequency and is used to display new items
 *
 */
static gint reading_drop_items ()
{

  if(reading_create_item(boardRootItem))
    drop_items_id = gtk_timeout_add (fallSpeed,
				     (GtkFunction) reading_drop_items, NULL);
  return (FALSE);
}

static void ask_ready(gboolean status)
{
  static GnomeCanvasItem *item1 = NULL;
  static GnomeCanvasItem *item2 = NULL;
  GdkPixbuf *button_pixmap = NULL;
  double y_offset = 310;
  double x_offset = 430;

  if(textToFind==NULL)
    return;

  if(status==FALSE)
    {
      if(item1!=NULL)
	gtk_object_destroy(GTK_OBJECT(item1));

      if(item2!=NULL)
	gtk_object_destroy(GTK_OBJECT(item2));

      item1 = NULL;
      item2 = NULL;
      return;
    }

  /*----- READY -----*/
  button_pixmap = gc_skin_pixmap_load("button_large2.png");
  item1 = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf",  button_pixmap,
				"x", x_offset,
				"y", y_offset,
				NULL);

  gdk_pixbuf_unref(button_pixmap);
  gtk_signal_connect(GTK_OBJECT(item1), "event",
		     (GtkSignalFunc) item_event_valid,
		     "R");

  item2 = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_text_get_type (),
				"text", _("I am Ready"),
				"font", gc_skin_font_board_big,
				"x", (double) x_offset +
				gdk_pixbuf_get_width(button_pixmap)/2,
				"y", (double) y_offset + 40,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color", "white",
				NULL);

  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) item_event_valid,
		     "R");
}

static void ask_yes_no()
{
  GnomeCanvasItem *item;
  GdkPixbuf *button_pixmap = NULL;
  double y_offset = 310;
  double x_offset = 430;

  if(textToFind==NULL)
    return;

  /*----- YES -----*/
  button_pixmap = gc_skin_pixmap_load("button_large2.png");
  item = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf",  button_pixmap,
				"x", x_offset,
				"y", y_offset,
				NULL);

  gdk_pixbuf_unref(button_pixmap);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_valid,
		     "Y");

  item = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_text_get_type (),
				"text", _("Yes, I saw it"),
				"font", gc_skin_font_board_big,
				"x", (double) x_offset +
				gdk_pixbuf_get_width(button_pixmap)/2,
				"y", (double) y_offset + 40,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color", "white",
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_valid,
		     "Y");

  /*----- NO -----*/
  y_offset += 100;
  button_pixmap = gc_skin_pixmap_load("button_large2.png");
  item = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf",  button_pixmap,
				"x", x_offset,
				"y", y_offset,
				NULL);

  gdk_pixbuf_unref(button_pixmap);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_valid,
		     "N");

  item = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_text_get_type (),
				"text", _("No, it was not there"),
				"font", gc_skin_font_board_big,
				"x", (double) x_offset +
				gdk_pixbuf_get_width(button_pixmap)/2,
				"y", (double) y_offset + 40,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color", "white",
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_valid,
		     "N");
}


static void player_win()
{
  gamewon = TRUE;
  wait_for_ready = TRUE;
  gc_bonus_display(gamewon, GC_BONUS_FLOWER);
  /* Try the next level */
  gcomprisBoard->level++;
  if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
    gc_bonus_end_display(GC_BOARD_FINISHED_RANDOM);
    return;
  }

  next_level_timer = g_timeout_add(3000, (GtkFunction)reading_next_level, NULL);
}

static void player_loose()
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

  next_level_timer = g_timeout_add(3000, (GtkFunction)reading_next_level, NULL);
}

/* Callback for the yes and no buttons */
static gint
item_event_valid(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{

  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
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
	return TRUE;
      }
      break;

    default:
      break;
    }
  return FALSE;

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

  word = gc_wordlist_random_word_get(gc_wordlist, gcomprisBoard->level);

  if(except)
    while(strcmp(except, word)==0)
      {
	g_free(word);
	word = gc_wordlist_random_word_get(gc_wordlist, gcomprisBoard->level);
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

    reading_next_level();

    pause_board(FALSE);
  }

  board_conf = NULL;
  profile_conf = NULL;

}

static void
reading_config_start(GcomprisBoard *agcomprisBoard,
		    GcomprisProfile *aProfile)
{
  board_conf = agcomprisBoard;
  profile_conf = aProfile;

  if (gcomprisBoard)
    pause_board(TRUE);

  gchar *label = g_strdup_printf("<b>%s</b> configuration\n for profile <b>%s</b>",
				 agcomprisBoard->name,
				 aProfile? aProfile->name: "");

  gc_board_config_window_display( label,
				 (GcomprisConfCallback )conf_ok);

  g_free(label);

  /* init the combo to previously saved value */
  GHashTable *config = gc_db_get_conf( profile_conf, board_conf);

  gchar *locale = g_hash_table_lookup( config, "locale");

  gc_board_config_combo_locales( locale);

}


/* ======================= */
/* = config_stop        = */
/* ======================= */
static void
reading_config_stop()
{
}
