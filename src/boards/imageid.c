/* gcompris - imageid.c
 *
 * Copyright (C) 2001 Pascal Georges
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

#define DEBUG 0

#include <ctype.h>
#include <math.h>
#include <assert.h>

#include "gcompris/gcompris.h"

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);
static int gamewon;

static void		 process_ok(void);
static void		 highlight_selected(GnomeCanvasItem *);
static void		 game_won(void);

static void		 config_start(GcomprisBoard *agcomprisBoard,
					     GcomprisProfile *aProfile);
static void		 config_stop(void);

#ifdef DEBUG
static void		 dump_xml(void);
#endif


typedef struct {
  char  *pixmapfile;
  char  *text1;
  char  *text2;
  char  *text3;
} Board;

/* XML */
static gboolean		 read_xml_file(char *fname);
static void		 init_xml(void);
static void		 add_xml_data(xmlDocPtr doc,xmlNodePtr xmlnode, GNode * child);
static void		 parse_doc(xmlDocPtr doc);
static gboolean		 read_xml_file(char *fname);
static void		 destroy_board_list(void);
static void		 destroy_board(Board * board);

/* This is the list of boards */
static GList *board_list = NULL;

#define VERTICAL_SEPARATION 30
#define HORIZONTAL_SEPARATION 30
// the values are taken from the backgound image imageid-bg.jpg
#define IMAGE_AREA_X1 280
#define IMAGE_AREA_X2 750
#define IMAGE_AREA_Y1 90
#define IMAGE_AREA_Y2 430

//NUMBER_OF_SUBLEVELS*NUMBER_OF_LEVELS must equal the number of boards in XML file
#define NUMBER_OF_SUBLEVELS 3
#define NUMBER_OF_LEVELS 4

#define TEXT_COLOR "white"

/* ================================================================ */
static int board_number; // between 0 and board_list.length-1
static int right_word; // between 1 and 3, indicates which choice is the right one (the player clicks on it

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem *image_item = NULL;
static GnomeCanvasItem *text1_item = NULL;
static GnomeCanvasItem *text2_item = NULL;
static GnomeCanvasItem *text3_item = NULL;
static GnomeCanvasItem *button1 = NULL, *button2 = NULL, *button3 = NULL, *selected_button = NULL;

static GnomeCanvasItem *imageid_create_item(GnomeCanvasGroup *parent);
static void imageid_destroy_all_items(void);
static void imageid_next_level(void);
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Reading"),
    N_("Learn how to read"),
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
    process_ok,
    set_level,
    NULL,
    NULL,
    config_start,
    config_stop
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(imageid)

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
  GHashTable *config = gcompris_get_board_conf();

  gcompris_change_locale(g_hash_table_lookup( config, "locale"));

  g_hash_table_destroy(config);

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "imageid/imageid-bg.jpg");
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=NUMBER_OF_LEVELS;
      gcomprisBoard->sublevel=1;

      init_xml();

      gcomprisBoard->number_of_sublevel = NUMBER_OF_SUBLEVELS;
      assert(NUMBER_OF_LEVELS*NUMBER_OF_SUBLEVELS == g_list_length(board_list));
      gcompris_score_start(SCORESTYLE_NOTE,
			   50,
			   gcomprisBoard->height - 50,
			   gcomprisBoard->number_of_sublevel);
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

      imageid_next_level();

      gamewon = FALSE;
      pause_board(FALSE);
    }
}

static void
end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gcompris_score_end();
      imageid_destroy_all_items();
      destroy_board_list();
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
      gcomprisBoard->sublevel=1;
      imageid_next_level();
    }
}

static gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "imageid")==0)
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
/* set initial values for the next level */
static void imageid_next_level()
{
  gcompris_bar_set_level(gcomprisBoard);

  imageid_destroy_all_items();
  selected_button = NULL;
  gamewon = FALSE;

  gcompris_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  imageid_create_item(gnome_canvas_root(gcomprisBoard->canvas));

}
/* ==================================== */
/* Destroy all the items */
static void imageid_destroy_all_items()
{
  if(boardRootItem!=NULL)
      gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
static GnomeCanvasItem *imageid_create_item(GnomeCanvasGroup *parent)
{
  char *buf[3];
  int x, y, xp, yp, place;
  GdkPixbuf *pixmap = NULL;
  GdkPixbuf *button_pixmap = NULL;
  char *str = NULL;
  Board * board;

  board_number = (gcomprisBoard->level-1) * NUMBER_OF_SUBLEVELS + gcomprisBoard->sublevel-1;
/*  if (board_number >= g_list_length(board_list))
	board_number = g_list_length(board_list)-1;
*/
  assert(board_number >= 0  && board_number < g_list_length(board_list));
  place = ((int)(3.0*rand()/(RAND_MAX+1.0)));
  assert(place >= 0  && place < 3);

  right_word = place+1;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  button_pixmap = gcompris_load_skin_pixmap("button_large.png");
  /* display the image */
  board = g_list_nth_data(board_list, board_number);
  assert(board != NULL);

  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, board->pixmapfile);
  pixmap = gcompris_load_pixmap(str);

  x = IMAGE_AREA_X1 + ( IMAGE_AREA_X2 - IMAGE_AREA_X1 - gdk_pixbuf_get_width(pixmap))/2;
  y = IMAGE_AREA_Y1 + ( IMAGE_AREA_Y2 - IMAGE_AREA_Y1 - gdk_pixbuf_get_height(pixmap))/2;

  image_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", pixmap,
				      "x", (double) x,
				      "y", (double) y,
				      "width", (double) gdk_pixbuf_get_width(pixmap),
				      "height", (double) gdk_pixbuf_get_height(pixmap),
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);

  gdk_pixbuf_unref(pixmap);
  g_free(str);

  /* display the 3 words */
  /* the right word is at position 0 : it is swapped with any position depending of place value */

  switch (place) {
  case 1 :
    buf[0] = board->text2;
    buf[1] = board->text1;
    buf[2] = board->text3;
    break;
  case 2 :
    buf[0] = board->text3;
    buf[1] = board->text2;
    buf[2] = board->text1;
    break;
  default :
    buf[0] = board->text1;
    buf[1] = board->text2;
    buf[2] = board->text3;
    break;
  }

  xp = HORIZONTAL_SEPARATION;
  yp = (gcomprisBoard->height - 3*gdk_pixbuf_get_height(button_pixmap) - 2*VERTICAL_SEPARATION)/2;

  button1 = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf",  button_pixmap,
				"x",  (double) xp,
				"y",  (double) yp,
				NULL);
  gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", buf[0],
				      "font", gcompris_skin_font_board_big,
				      "x", (double) xp + gdk_pixbuf_get_width(button_pixmap)/2 + 1.0,
				      "y", (double) yp + gdk_pixbuf_get_height(button_pixmap)/2 + 1.0,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color_rgba", gcompris_skin_color_shadow,
				      NULL);
  text1_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", buf[0],
				      "font", gcompris_skin_font_board_big,
				      "x", (double) xp + gdk_pixbuf_get_width(button_pixmap)/2,
				      "y", (double) yp + gdk_pixbuf_get_height(button_pixmap)/2,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color_rgba", gcompris_skin_color_text_button,
				      NULL);

  yp += gdk_pixbuf_get_height(button_pixmap) + VERTICAL_SEPARATION;
  button2 = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf",  button_pixmap,
				"x",  (double) xp,
				"y",  (double) yp,
				NULL);
  gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", buf[1],
				      "font", gcompris_skin_font_board_big,
				      "x", (double) xp + gdk_pixbuf_get_width(button_pixmap)/2 + 1.0,
				      "y", (double) yp + gdk_pixbuf_get_height(button_pixmap)/2 + 1.0,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color_rgba", gcompris_skin_color_shadow,
				      NULL);
  text2_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", buf[1],
				      "font", gcompris_skin_font_board_big,
				      "x", (double) xp + gdk_pixbuf_get_width(button_pixmap)/2,
				      "y", (double) yp + gdk_pixbuf_get_height(button_pixmap)/2,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color_rgba", gcompris_skin_color_text_button,
				      NULL);

  yp += gdk_pixbuf_get_height(button_pixmap) + VERTICAL_SEPARATION;
  button3 = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf",  button_pixmap,
				"x",  (double) xp,
				"y",  (double) yp,
				NULL);

  gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", buf[2],
				      "font", gcompris_skin_font_board_big,
				      "x", (double) xp + gdk_pixbuf_get_width(button_pixmap)/2 + 1.0,
				      "y", (double) yp + gdk_pixbuf_get_height(button_pixmap)/2 + 1.0,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color_rgba", gcompris_skin_color_shadow,
				      NULL);
  text3_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", buf[2],
				      "font", gcompris_skin_font_board_big,
				      "x", (double) xp + gdk_pixbuf_get_width(button_pixmap)/2,
				      "y", (double) yp + gdk_pixbuf_get_height(button_pixmap)/2,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color_rgba", gcompris_skin_color_text_button,
				      NULL);

  gdk_pixbuf_unref(button_pixmap);

  gtk_signal_connect(GTK_OBJECT(button1), "event",  (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(button2), "event",  (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(button3), "event",  (GtkSignalFunc) item_event, NULL);

  gtk_signal_connect(GTK_OBJECT(text1_item), "event", (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(text2_item), "event", (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(text3_item), "event", (GtkSignalFunc) item_event, NULL);

  return NULL;
}
/* ==================================== */
static void game_won()
{
  gcomprisBoard->sublevel++;
  if(gcomprisBoard->sublevel > gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
	board_finished(BOARD_FINISHED_TUXLOCO);
	return;
      }
  }
  imageid_next_level();
}

/* ==================================== */
static gboolean process_ok_timeout() {
  gcompris_display_bonus(gamewon, BONUS_SMILEY);
	return FALSE;
}

static void process_ok() {
	// leave time to display the right answer
  g_timeout_add(TIME_CLICK_TO_BONUS, process_ok_timeout, NULL);
}
/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  double item_x, item_y;
  GnomeCanvasItem * temp = NULL;
  item_x = event->button.x;
  item_y = event->button.y;
  gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      board_paused = TRUE;
      temp = item;
      if (item == text1_item)
	temp = button1;
      if (item == text2_item)
	temp = button2;
      if (item == text3_item)
	temp = button3;

      assert(temp == button1 || temp == button2 || temp == button3);

      if ( ( temp == button1 && right_word == 1) ||
	   ( temp == button2 && right_word == 2) ||
	   ( temp == button3 && right_word == 3) ) {
	gamewon = TRUE;
      } else {
	gamewon = FALSE;
      }
      highlight_selected(temp);
      process_ok();
      break;

    case GDK_MOTION_NOTIFY:
      break;

    case GDK_BUTTON_RELEASE:
      break;

    default:
      break;
    }
  return FALSE;
}

/* ==================================== */
static void highlight_selected(GnomeCanvasItem * item) {
  GdkPixbuf *button_pixmap_selected = NULL, *button_pixmap = NULL;
  GnomeCanvasItem *button;

  /* Replace text item by button item */
  button = item;
  if ( button == text1_item ) {
    button = button1;
  } else if ( item == text2_item ) {
    button = button2;
  } else if ( item == text3_item ) {
    button = button3;
  }

  if (selected_button != NULL && selected_button != button) {
  	button_pixmap = gcompris_load_skin_pixmap("button_large.png");
  	gnome_canvas_item_set(selected_button, "pixbuf", button_pixmap, NULL);
  	gdk_pixbuf_unref(button_pixmap);
  }

  if (selected_button != button) {
  	button_pixmap_selected = gcompris_load_skin_pixmap("button_large_selected.png");
  	gnome_canvas_item_set(button, "pixbuf", button_pixmap_selected, NULL);
  	selected_button = button;
  	gdk_pixbuf_unref(button_pixmap_selected);
  }

}


/* ===================================
 *                XML stuff
 *                Ref : shapegame.c
 * ==================================== */
static void init_xml()
{
  char *filename;
  filename = g_strdup_printf("%s/%s/board1.xml", PACKAGE_DATA_DIR, gcomprisBoard->boarddir);
  g_warning("filename = %s %s %s\n", filename,PACKAGE_DATA_DIR,gcomprisBoard->boarddir);

  assert(g_file_test(filename, G_FILE_TEST_EXISTS));
  assert(read_xml_file(filename)== TRUE);
  g_free(filename);
#ifdef DEBUG
  dump_xml();
#endif
}
/* ======  for DEBUG ========  */
#ifdef DEBUG
static void dump_xml() {
  GList *list;
  g_warning("XML lentgh = %d\n", g_list_length(board_list));

  for(list = board_list; list != NULL; list = list->next) {
    Board * board = list->data;
    g_warning("xml = %s %s %s %s\n", board->pixmapfile, board->text1, board->text2, board->text3);
    }
}
#endif

/* ==================================== */
static void add_xml_data(xmlDocPtr doc, xmlNodePtr xmlnode, GNode * child)
{
  char *pixmapfile = NULL;
  char *text1 = NULL, *text2 = NULL, *text3 = NULL;
  Board * board = g_new(Board,1);

  xmlnode = xmlnode->xmlChildrenNode;

  xmlnode = xmlnode->next;

  while (xmlnode != NULL) {
  	gchar *lang = xmlGetProp(xmlnode,"lang");
	if (!strcmp(xmlnode->name, "pixmapfile") && (lang==NULL
	    || !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
	  {
	    pixmapfile = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	  }
	if (!strcmp(xmlnode->name, "text1") && (lang==NULL
	    || !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
	  {
		text1 = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	  }
	if (!strcmp(xmlnode->name, "text2") && (lang==NULL
	    || !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
	  {
	    text2 = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	  }
	if (!strcmp(xmlnode->name, "text3") && (lang==NULL
	    || !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
	  {
	    text3 = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	  }
	xmlnode = xmlnode->next;
	}
	// I really don't know why this test, but otherwise, the list is doubled
	// with 1 line on 2 filled with NULL elements
	if ( (pixmapfile == NULL || text1 == NULL || text2 == NULL || text3 == NULL))
		return;

	board->pixmapfile = g_strdup(pixmapfile);
	board->text1 = g_strdup(text1);
	board->text2 = g_strdup(text2);
	board->text3 = g_strdup(text3);

	board_list = g_list_append (board_list, board);
}

/* ==================================== */
static void parse_doc(xmlDocPtr doc)
{
  xmlNodePtr node;

  for(node = doc->children->children; node != NULL; node = node->next) {
    if ( g_strcasecmp(node->name, "Board") == 0 )
    	add_xml_data(doc, node,NULL);
  }

}
/* ==================================== */
/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set */
static gboolean read_xml_file(char *fname)
{
  /* pointer to the new doc */
  xmlDocPtr doc;

  g_return_val_if_fail(fname!=NULL,FALSE);

  /* if the file doesn't exist */
  if(!g_file_test(fname, G_FILE_TEST_EXISTS))
    {
      g_warning("Couldn't find file %s !", fname);
      return FALSE;
    }
  g_warning("found file %s !", fname);

  /* parse the new file and put the result into newdoc */
  doc = xmlParseFile(fname);
  /* in case something went wrong */
  if(!doc)
    return FALSE;

  if(/* if there is no root element */
     !doc->children ||
     /* if it doesn't have a name */
     !doc->children->name ||
     /* if it isn't a ImageId node */
     g_strcasecmp(doc->children->name,"ImageId")!=0) {
    xmlFreeDoc(doc);
    return FALSE;
  }

  parse_doc(doc);
  xmlFreeDoc(doc);
  return TRUE;
}
/* ======================================= */
static void destroy_board_list() {
Board *board;
 while(g_list_length(board_list)>0)
    {
      board = g_list_nth_data(board_list, 0);
      board_list = g_list_remove (board_list, board);
      destroy_board(board);
    }
}

/* ======================================= */
static void destroy_board(Board * board) {
	g_free(board->pixmapfile);
	g_free(board->text1);
	g_free(board->text2);
	g_free(board->text3);
	g_free(board);
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
config_start(GcomprisBoard *agcomprisBoard,
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
config_stop()
{
}

