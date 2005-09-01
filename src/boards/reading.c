/* gcompris - reading.c
 *
 * Time-stamp: <2005/09/01 22:38:40 yves>
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

#include <errno.h>
#include <stdio.h>

#include "gcompris/gcompris.h"


#define SOUNDLISTFILE PACKAGE
#define MAXWORDSLENGTH 50
#define MAX_WORDS 100


static GPtrArray *words=NULL;

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
static gchar		*get_random_word(gboolean);
static gboolean	 read_wordfile();
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
   N_("Reading"),
   N_("Read a list of words and then tell if the given word is in it"),
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
  GHashTable *config = gcompris_get_board_conf();

  gcompris_change_locale(g_hash_table_lookup( config, "locale"));

  g_hash_table_destroy(config);

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "reading/reading-bg.jpg");
      wait_for_ready = TRUE;
      gamewon = FALSE;

      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 9;
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

      font_size = PANGO_PIXELS(pango_font_description_get_size (pango_font_description_from_string (gcompris_skin_font_board_medium)));
      interline = (int) (1.5*font_size);

      PangoContext *pango_context = gtk_widget_get_pango_context  (GTK_WIDGET(agcomprisBoard->canvas));

      PangoFontMetrics* pango_metrics =  pango_context_get_metrics (pango_context,
								    pango_font_description_from_string (gcompris_skin_font_board_medium),
								    pango_language_from_string   (gcompris_get_locale()));

      int ascent = PANGO_PIXELS(pango_font_metrics_get_ascent (pango_metrics));
      int descent = PANGO_PIXELS(pango_font_metrics_get_descent (pango_metrics));
      interline = ascent + descent;

      g_warning ("Font to display words have size %d  ascent : %d, descent : %d.\n Set inerline to %d", font_size, ascent, descent, interline);

      /* Default mode */
      currentMode=MODE_VERTICAL;
      if(gcomprisBoard->mode && g_strcasecmp(gcomprisBoard->mode, "horizontal")==0)
	currentMode=MODE_HORIZONTAL;

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

  gcompris_reset_locale();

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

  gcompris_bar_set_level(gcomprisBoard);

  gamewon = FALSE;

  g_warning("destroying_all...");
  reading_destroy_all_items();
  g_warning("destroying_all... done ");

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

  g_warning("reading wordfile...");
  read_wordfile();
  g_warning("reading wordfile...done");
  display_what_to_do(boardRootItem);
  g_warning("display_what_to_do...done");
  ask_ready(TRUE);
  g_warning("ask_ready...done");
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

  g_warning("destroying boardRootItem ...");
  if(boardRootItem!=NULL)
      gtk_object_destroy (GTK_OBJECT(boardRootItem));

  g_warning("destroying boardRootItem ... done");

  boardRootItem = NULL;
  previousFocus.rootItem = NULL;
  toDeleteFocus.rootItem = NULL;

  if (textToFind!=NULL) 
    {
    g_free(textToFind);
    textToFind=NULL;
    }

  g_warning("destroying words ...");  

  if (words!=NULL) 
    {
    g_ptr_array_free (words, TRUE);
    words=NULL;
    }

  g_warning("destroying words ... done");  
  
  

}

static GnomeCanvasItem *display_what_to_do(GnomeCanvasGroup *parent)
{

  gint base_Y = 110;
  gint base_X = 580;
  gint i;

  /* Load the text to find */
  
  textToFind = get_random_word(TRUE);

  assert(textToFind != NULL);

  /* Decide now if this time we will display the text to find */
  /* Use this formula to have a better random number see 'man 3 rand' */
  i=((int)(2.0*rand()/(RAND_MAX+1.0)));
  if(i==0)
      textToFindIndex = rand() % numberOfLine;
  else
    textToFindIndex = NOT_THERE;

  gnome_canvas_item_new (parent,
			 gnome_canvas_text_get_type (),
			 "text", _("Please, check if the word"),
			 "font", gcompris_skin_font_board_big,
			 "x", (double) base_X,
			 "y", (double) base_Y,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color", "white",
			 NULL);

  gnome_canvas_item_new (parent,
			 gnome_canvas_text_get_type (),
			 "text", textToFind,
			 "font", gcompris_skin_font_board_big,
			 "x", (double) base_X,
			 "y", (double) base_Y + 30,
			 "anchor", GTK_ANCHOR_CENTER,
			 "fill_color", "green",
			 NULL);

  gnome_canvas_item_new (parent,
			 gnome_canvas_text_get_type (),
			 "text", _("is being displayed"),
			 "font", gcompris_skin_font_board_big,
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
  gchar *overword;

  assert(textToFind!=NULL);

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
      word = get_random_word(FALSE);
    }
  else
    {
      word = textToFind;
    }

  assert(word!=NULL);

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
			   "font", gcompris_skin_font_board_medium,
			   "x", (double) 0,
			   "y", (double) 0,
			   "anchor", anchor,
			   "fill_color", "black",
			   NULL);

  gchar *oldword = g_strdup_printf("<span foreground=\"black\" background=\"black\">%s</span>", word);

  previousFocus.overwriteItem = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(previousFocus.rootItem),
			   gnome_canvas_text_get_type (),
			   "markup", oldword,
			   "font", gcompris_skin_font_board_medium,
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
  button_pixmap = gcompris_load_skin_pixmap("button_large2.png");
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
				"font", gcompris_skin_font_board_big,
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
  button_pixmap = gcompris_load_skin_pixmap("button_large2.png");
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
				"text", _("Yes I saw it"),
				"font", gcompris_skin_font_board_big,
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
  button_pixmap = gcompris_load_skin_pixmap("button_large2.png");
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
				"font", gcompris_skin_font_board_big,
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
  gcompris_display_bonus(gamewon, BONUS_FLOWER);
  /* Try the next level */
  gcomprisBoard->level++;
  if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
    board_finished(BOARD_FINISHED_RANDOM);
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

  gcompris_log_set_comment (gcomprisBoard, expected, got);

  g_free(expected);
  g_free(got);

  gcompris_display_bonus(gamewon, BONUS_FLOWER);

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




static FILE *get_wordfile(const char *locale)
{
  gchar *filename;
  FILE *wordsfd = NULL;
                                                                                                                              
  /* First Try to find a file matching the level and the locale */
  filename = g_strdup_printf("%s%s%d.%.2s",
                             PACKAGE_DATA_DIR, "/wordsgame/wordslevel",
                             gcomprisBoard->level, locale);
  //  g_message("Trying to open file %s ", filename);
  wordsfd = fopen (filename, "r");
                                                                                                                              
  if(wordsfd==NULL)
    {
      /* Second Try to find a file matching the 'max' and the locale */
      g_sprintf(filename, "%s%s%.2s",
              PACKAGE_DATA_DIR, "/wordsgame/wordslevelmax.",
              locale);
      //      g_message("Trying to open file %s ", filename);
                                                                                                                              
      wordsfd = fopen (filename, "r");
    }
                                                                                                                              
  g_free(filename);
                                                                                                                              
  return wordsfd;
}




static gboolean  read_wordfile()
{

  FILE *wordsfd;
  gchar *buf;
  int len;                                                                                                                                

  wordsfd = get_wordfile(gcompris_get_locale());
                                                                                                                              
  if(wordsfd==NULL)
    {
      /* Try to open the english locale by default */
      wordsfd = get_wordfile("en");
                                                                                                                              
      /* Too bad, even english is not there. Check your Install */
      if(wordsfd==NULL) {
        gcompris_dialog(_("Cannot open file of words for your locale"), gcompris_end_board);
        return FALSE;
      }
    }
                                                                                                                              
   words=g_ptr_array_new ();
   while (buf=fgets(g_new(gchar,MAXWORDSLENGTH), MAXWORDSLENGTH, wordsfd)) {
 	assert(g_utf8_validate(buf,-1,NULL));

	//remove \n from end of line
        len = strlen(buf);
        if((0 < len)&&('\n'==buf[len-1]))
            buf[len-1] = '\0';
	    
	g_ptr_array_add(words,buf);
	}
   fclose(wordsfd);

}
/*
 * Return a random word from a set of text file depending on 
 * the current level and language
 */


static gchar *get_random_word(gboolean remove)
{
  int i=rand()%words->len;
  if (remove) 
    return g_ptr_array_remove_index(words,i);
  else
    return g_ptr_array_index(words,i);
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
  gcompris_set_board_conf ( profile_conf,
			    board_conf,
			    (gchar *) key, 
			    (gchar *) value);
}

static GcomprisConfCallback conf_ok(GHashTable *table)
{
  g_hash_table_foreach(table, (GHFunc) save_table, NULL);
  
  board_conf = NULL;
  profile_conf = NULL;
}

static void
reading_config_start(GcomprisBoard *agcomprisBoard,
		    GcomprisProfile *aProfile)
{
  board_conf = agcomprisBoard;
  profile_conf = aProfile;

  gcompris_configuration_window( g_strdup_printf("<b>%s</b> configuration\n for profile <b>%s</b>",
						 agcomprisBoard->name, 
						 aProfile->name), 
				 (GcomprisConfCallback )conf_ok);

  /* init the combo to previously saved value */
  GHashTable *config = gcompris_get_conf( profile_conf, board_conf);

  gchar *locale = g_hash_table_lookup( config, "locale");
  
  gcompris_combo_locales( locale);

}

  
/* ======================= */
/* = config_stop        = */
/* ======================= */
static void 
reading_config_stop()
{
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
