/* gcompris - menu.c
 *
 * Time-stamp: <2005/06/20 22:44:45 yves>
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

/**
 * The menu display icons of each boards and let the user select one
 *
 * Seen from gcompris, the menu is a board like another.
 */

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "gcompris/gcompris.h"
#include "gcompris/gcompris_config.h"

#define SOUNDLISTFILE PACKAGE
#define MENU_PER_LINE 5

typedef struct {
  /* Information items */
  GnomeCanvasItem *boardname_item;
  GnomeCanvasItem *description_item;
  GnomeCanvasItem *author_item;
} MenuItems;

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void		 menu_start (GcomprisBoard *agcomprisBoard);
static void		 menu_pause (gboolean pause);
static void		 menu_end (void);
static gboolean		 menu_is_our_board (GcomprisBoard *gcomprisBoard);
static void		 menu_config(void);

static void		 menu_create_item(GnomeCanvasGroup *parent, MenuItems *menuitems, GcomprisBoard *board);
static gboolean		 next_spot();
static void		 create_info_area(GnomeCanvasGroup *parent, MenuItems *menuitems);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, MenuItems *menuitems);
static void		 display_board_icon(GcomprisBoard *board, MenuItems *menuitems);
static gboolean		 read_xml_file(char *fname);
static void		 free_stuff (GtkObject *obj, gpointer data);

static double current_x = 0.0;
static double current_y = 0.0;

static GnomeCanvasGroup *boardRootItem = NULL;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Main Menu"),
    N_("Select a Board"),
    "Bruno Coudoin <bruno.coudoin@free.fr>",
    NULL,
    NULL,
    NULL,
    NULL,
    menu_start,
    menu_pause,
    menu_end,
    menu_is_our_board,
    NULL,
    NULL,
    NULL,
    menu_config,
    NULL
  };

/*
 * Main entry point mandatory for each Gcompris's board
 * ---------------------------------------------------
 *
 */


GET_BPLUGIN_INFO(menu)

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void menu_pause (gboolean pause)
{

  if(gcomprisBoard==NULL)
    return;

  board_paused = pause;
}

/*
 */
static void menu_start (GcomprisBoard *agcomprisBoard)
{

  current_x = 0.0;
  current_y = 0.0;

  if(agcomprisBoard!=NULL)
    {
      MenuItems		*menuitems;
      GList		*boardlist;	/* List of Board */

      gcomprisBoard=agcomprisBoard;
      menuitems = g_new(MenuItems, 1);

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), 
			      gcompris_image_to_skin("gcompris-init.jpg"));


      boardRootItem = GNOME_CANVAS_GROUP(
					 gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
								gnome_canvas_group_get_type (),
								"x", (double) 0,
								"y", (double) 0,
								NULL));

      g_object_set_data (G_OBJECT (boardRootItem), "menuitems", menuitems);
      g_signal_connect (boardRootItem, "destroy",
			G_CALLBACK (free_stuff),
			menuitems);

      gchar *path = g_strdup_printf("%s/%s",gcomprisBoard->section, gcomprisBoard->name);

      boardlist = gcompris_get_menulist(path);

      g_free(path);

      create_info_area(boardRootItem, menuitems);

      g_list_foreach (boardlist, (GFunc) display_board_icon, menuitems);

      g_list_free(boardlist);

      /* set initial values for this level */
      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel=1;
      gcompris_bar_set(GCOMPRIS_BAR_CONFIG|GCOMPRIS_BAR_ABOUT);

      /* FIXME : Workaround for bugged canvas */
      gnome_canvas_update_now(gcomprisBoard->canvas);

      menu_pause(FALSE);

    }

}

static void
menu_end ()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem=NULL;
}

static gboolean
menu_is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "menu")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

static void
menu_config ()
{
  if(gcomprisBoard!=NULL)
    {
      menu_pause(TRUE);
      gcompris_config_start();
    }
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

static void display_board_icon(GcomprisBoard *board, MenuItems *menuitems)
{
  gint difficulty = 0;
  GcomprisProperties	*properties = gcompris_get_properties();

  difficulty = atoi(board->difficulty);

  /* FIXME: Check board is available for the current profile */
  if (board!=NULL 
      && board_check_file(board))
    {
      /* Always display menu items or we risk to have unaccessible boards */
      if(g_strcasecmp(board->type, "menu")==0) {
	menu_create_item(boardRootItem, menuitems, board);
      } else {
	/* Implements the level filtering system */
	switch (properties->filter_style) {
	case GCOMPRIS_FILTER_NONE:
	  if(difficulty>0)	/* Skip in development boards */
	    menu_create_item(boardRootItem, menuitems, board);
	  break;
	case GCOMPRIS_FILTER_EQUAL:
	  if(properties->difficulty_filter==difficulty)
	    menu_create_item(boardRootItem, menuitems, board);
	  break;
	case GCOMPRIS_FILTER_UNDER:
	  if(difficulty<=properties->difficulty_filter && difficulty>0)
	    menu_create_item(boardRootItem, menuitems, board);
	  break;
	case GCOMPRIS_FILTER_ABOVE:
	  if(difficulty>=properties->difficulty_filter)
	    menu_create_item(boardRootItem, menuitems, board);
	  break;
	default:
	  /* Hum, should not happen, let's display the board anyway */
	  break;
	}
      }
    }
}

/*
 * Calculate the next stop where to place an item
 * return false if there is no more space left
 */
static gboolean next_spot() 
{
  if(current_x==0.0)
    {
      /* Initialisation case */
      current_x = gcomprisBoard->width/MENU_PER_LINE;
      current_y = gcomprisBoard->height/MENU_PER_LINE - 40;
      return(TRUE);
    }

  current_x += gcomprisBoard->width/MENU_PER_LINE;
  if(current_x>=gcomprisBoard->width-100)
    {
      current_x = gcomprisBoard->width/MENU_PER_LINE;
      current_y += 120;
    }

  return(TRUE);
}

static void menu_create_item(GnomeCanvasGroup *parent, MenuItems *menuitems, GcomprisBoard *board)
{
  GdkPixbuf *menu_pixmap = NULL;
  GdkPixbuf *pixmap = NULL;
  GnomeCanvasItem *item, *menu_button;
  int difficulty;
  gchar *tmp_board_dir;

  /*
   * Take care to load the board->icon_name from the dir specified
   * in the board->board_dir
   * To make it simple and reuse the gcompris_load_pixmap, we overwrite temporarily
   * the gcomprisBoard->board_dir and board->board_dir
   */
  tmp_board_dir = gcomprisBoard->board_dir;
  gcomprisBoard->board_dir = board->board_dir;
  menu_pixmap = gcompris_load_pixmap(board->icon_name);
  gcomprisBoard->board_dir = tmp_board_dir;

  next_spot();

  menu_button = gnome_canvas_item_new (parent,
				       gnome_canvas_pixbuf_get_type (),
				       "pixbuf", menu_pixmap,
				       "x", (double)current_x - gdk_pixbuf_get_width(menu_pixmap)/2,
				       "y", (double)current_y - gdk_pixbuf_get_height(menu_pixmap)/2,
				       NULL);

  // display difficulty stars
  if (board->difficulty != NULL) {
    difficulty = atoi(board->difficulty);
    gcompris_display_difficulty_stars(parent,
				      (double)current_x - gdk_pixbuf_get_width(menu_pixmap)/2 - 25,
				      (double)current_y - gdk_pixbuf_get_height(menu_pixmap)/2,
				      (double) 0.6,
				      difficulty);
  }

  // display board availability due to sound voice not present
  if(board->mandatory_sound_file)
    {
      gchar *soundfile = NULL;
      
      if(board->mandatory_sound_dataset) {
	/* We have to search for an assetml sound */
	soundfile = gcompris_get_asset_file(board->mandatory_sound_dataset, NULL, NULL, 
					    board->mandatory_sound_file);
      } else {
	/* We search a fixed path sound file */
	soundfile = g_strdup_printf("%s/%s", PACKAGE_DATA_DIR "/sounds", 
				    board->mandatory_sound_file);
	g_warning("Checking mandatory_sound_file %s\n", soundfile);
      }

      if (!g_file_test (soundfile, G_FILE_TEST_EXISTS) || !gcompris_get_properties()->audio_works) 
	{
	  pixmap = gcompris_load_skin_pixmap("voice_bad.png");
	}
      else
	{
	  pixmap = gcompris_load_skin_pixmap("voice.png");
	}

      gnome_canvas_item_new (parent,
			     gnome_canvas_pixbuf_get_type (),
			     "pixbuf", pixmap,
			     "x", (double)current_x - gdk_pixbuf_get_width(menu_pixmap)/2 - 25,
			     "y", (double)current_y - gdk_pixbuf_get_height(menu_pixmap)/2 + 28,
			     NULL);
      gdk_pixbuf_unref(pixmap);
      g_free(soundfile);
    }

  // display menu icon ========================== BEGIN
  if(g_strcasecmp(board->type, "menu")==0)
    {
      pixmap = gcompris_load_skin_pixmap("menuicon.png");
      item =  gnome_canvas_item_new (parent,
				     gnome_canvas_pixbuf_get_type (),
				     "pixbuf", pixmap,
				     "x", (double)current_x - gdk_pixbuf_get_width(menu_pixmap)/2 - 25,
				     "y", (double)current_y - gdk_pixbuf_get_height(menu_pixmap)/2,
				     NULL);
      gdk_pixbuf_unref(pixmap);
    }

  gdk_pixbuf_unref(menu_pixmap);

  // display menu icon ========================== END

  /*
   * Now everything ready, map the events
   * -------------------------------------
   */
  g_object_set_data (G_OBJECT (menu_button), "board", board);
  gtk_signal_connect(GTK_OBJECT(menu_button), "event",
		     (GtkSignalFunc) item_event,
		     menuitems);

  gtk_signal_connect(GTK_OBJECT(menu_button), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);


}

static gint
item_event(GnomeCanvasItem *item, GdkEvent *event,  MenuItems *menuitems)
{
  GtkTextIter    iter_start, iter_end;
  GtkTextBuffer *buffer;
  GtkTextTag    *txt_tag;
  GcomprisBoard *board;

  if(board_paused)
    return FALSE;

  if(boardRootItem    == NULL)
    return;

  board = g_object_get_data (G_OBJECT (item), "board");

  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
      if(board->title && G_IS_OBJECT(menuitems->boardname_item))
	gnome_canvas_item_set (menuitems->boardname_item,
			       "text", board->title,
			       NULL);

      if(board->description && G_IS_OBJECT(menuitems->description_item))
	gnome_canvas_item_set (menuitems->description_item,
			       "text",  board->description,
			       NULL);

      if(board->author && G_IS_OBJECT(menuitems->author_item))
	gnome_canvas_item_set (menuitems->author_item,
			       "text",  board->author,
			       NULL);

      break;
    case GDK_LEAVE_NOTIFY:
      gnome_canvas_item_set (menuitems->boardname_item,
			     "text", " ",
			     NULL);

      gnome_canvas_item_set (menuitems->description_item,
			     "text",  " ",
			     NULL);

      gnome_canvas_item_set (menuitems->author_item,
			     "text",  " ",
			     NULL);

      break;
    case GDK_BUTTON_PRESS:
      {
	/* Back Reference the current board in the next one */
	board->previous_board = gcomprisBoard;

	gcompris_play_ogg ("gobble", NULL);

	/* End this board */
	menu_end();

	/* Start the user's one */
	board_play (board);
      };
      break;
      
    default:
      break;
    }

  return FALSE;
}

static void create_info_area(GnomeCanvasGroup *parent, MenuItems *menuitems)
{
  gint x = (double)gcomprisBoard->width/2;
  gint y = 383;

  if(parent    == NULL)
    return;

  menuitems->boardname_item = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_text_get_type (),
			   "text", " ",
			   "font", gcompris_skin_font_board_big,
			   "x", (double) x,
			   "y", (double) y,
			   "anchor", GTK_ANCHOR_NORTH,
			   "fill_color", "white",
			   NULL);

  menuitems->description_item = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_text_get_type (),
			   "text", "",
			   "font",       gcompris_skin_font_board_medium,
			   "x", (double) x,
			   "y", (double) y + 28,
			   "anchor", GTK_ANCHOR_NORTH,
			   "fill_color", "white",
			   NULL);

  menuitems->author_item = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_text_get_type (),
			   "text", " ",
			   "font", gcompris_skin_font_board_tiny,
			   "x", (double) x,
			   "y", (double) y + 90,
  			   "anchor", GTK_ANCHOR_NORTH,
  			   "fill_color", "white",
  			   "justification", GTK_JUSTIFY_CENTER,
			   NULL);

}

static void
free_stuff (GtkObject *obj, gpointer data)
{
  g_free (data);
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
