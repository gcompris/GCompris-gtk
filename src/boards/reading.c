/* gcompris - reading.c
 *
 * Time-stamp: <2004/10/23 15:06:01 bruno>
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

static GcomprisBoard *gcomprisBoard = NULL;

static gint drop_items_id    = 0;
static gint next_level_timer = 0;

static char *textToFind = NULL;
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

static void		 player_win(void);
static void		 player_loose(void);
static char		*get_random_word();
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

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "reading/reading-bg.jpg");
      wait_for_ready = TRUE;
      gamewon = FALSE;

      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 9;
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

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

  current_y = BASE_Y1;

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
  textToFind    = NULL;
  previousFocus.rootItem = NULL;
  toDeleteFocus.rootItem = NULL;
}

static GnomeCanvasItem *display_what_to_do(GnomeCanvasGroup *parent)
{
  gint base_Y = 110;
  gint base_X = 580;
  gint i;

  /* Free the previous text to find */
  if(textToFind)
    g_free(textToFind);

  /* Load the text to find */
  textToFind = "*";
  textToFind = get_random_word();

  if(textToFind==NULL)
    return;

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

  /* Decide now if this time we will display the text to find */
  /* Use this formula to have a better random number see 'man 3 rand' */
  i=((int)(2.0*rand()/(RAND_MAX+1.0)));
  if(i==0)
      textToFindIndex = rand() % numberOfLine;
  else
    textToFindIndex = NOT_THERE;

  return NULL;
}

static gboolean reading_create_item(GnomeCanvasGroup *parent)
{
  gint   i;
  gint   anchor = GTK_ANCHOR_CENTER;
  gchar *word;
  gchar *overword;

  if(textToFind==NULL)
    return FALSE;

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
      word = get_random_word();
    }
  else
    {
      word = g_strdup(textToFind);
    }

  if(word==NULL)
    return FALSE;

  if(textToFindIndex>=0)
    textToFindIndex--;

  /* fill up the overword with X */
  overword = g_malloc(strlen(word));
  for(i=0; i<strlen(word); i++)
    overword[i] = 'x';

  overword[i]='\0';

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
			   "font", gcompris_skin_font_board_fixed,
			   "x", (double) 0,
			   "y", (double) 0,
			   "anchor", anchor,
			   "fill_color", "black",
			   NULL);
  g_free(word);

  previousFocus.overwriteItem = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(previousFocus.rootItem),
			   gnome_canvas_text_get_type (),
			   "text", overword,
			   "font", gcompris_skin_font_board_fixed,
			   "x", (double) 0,
			   "y", (double) 0,
			   "anchor", anchor,
			   "fill_color", "black",
			   NULL);
  gnome_canvas_item_hide(previousFocus.overwriteItem);
  g_free(overword);

  // Calculate the next spot
  if(currentMode==MODE_VERTICAL)
    {
      current_y += 20;
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
	  gnome_canvas_item_move(GNOME_CANVAS_ITEM(previousFocus.rootItem), BASE_X1-x1, 20);
	  current_y += 20;
	  current_x = BASE_X1;
	  numberOfLine--;
	}
      current_x += x2-x1 + 12;
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
      }
      break;

    default:
      break;
    }
  return FALSE;

}


static FILE *get_wordfile(const char *locale)
{
  char *filename;
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
      sprintf(filename, "%s%s%.2s",  
	      PACKAGE_DATA_DIR, "/wordsgame/wordslevelmax.", 
	      locale);
      //      g_message("Trying to open file %s ", filename);

      wordsfd = fopen (filename, "r");
    }

  g_free(filename);

  return wordsfd;
}
/*
 * Return a random word from a set of text file depending on 
 * the current level and language
 */
static char *get_random_word()
{
  FILE *wordsfd;
  long  size, i;
  char *str;
  int   try;

  str = malloc(MAXWORDSLENGTH);

  wordsfd = get_wordfile(gcompris_get_locale());

  if(wordsfd==NULL)
    {
      /* Try to open the english locale by default */
      wordsfd = get_wordfile("en");
      
      /* Too bad, even english is not there. Check your Install */
      if(wordsfd==NULL) {
	g_free(str);
	gcompris_dialog(_("Cannot open file of words for your locale"), gcompris_end_board);
	return NULL;
      }
    }

  fseek (wordsfd, 0L, SEEK_END);
  size = ftell (wordsfd);

  if(size==0) {
    g_free(str);
    gcompris_dialog(_("Cannot open file of words for your locale"), gcompris_end_board);
    return NULL;
  }

  i=rand()%size;
  fseek(wordsfd, i, SEEK_SET);

  try=-1;  /* We want to be sure to loop at least 2 times */
  do {

    /* Read 2 times so that we are sure to sync on an end of line */
    /* Warning, We are not allowed to return the text to find     */
    fgets(str, MAXWORDSLENGTH, wordsfd);

    if(ftell(wordsfd)==size) {
      rewind(wordsfd);
      str[0]='\0';
    }

    /* Chop the return */
    if(strlen(str)>0)
      str[strlen(str)-1]='\0';

    try++;

  } while((str[0]=='\0' || strncmp(textToFind, str, strlen(textToFind))==0 || try<=0) && try<10);

  /* We have a problem */
  if(str[0]=='\0') {
    gcompris_dialog(_("Error: The wordfile package used for your locale is corrupted."), gcompris_end_board);
  }

  fclose(wordsfd);

  if(strcmp(textToFind, str)==0) {
    g_free(str);
    str=get_random_word();
  }

  return (str);
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
