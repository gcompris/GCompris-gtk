/* gcompris - imageid.c
 *
 * Copyright (C) 2000 Pascal Georges
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

GcomprisBoard *gcomprisBoard = NULL;
gboolean board_paused = TRUE;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
static int gamewon;

static void process_ok(void);
static void highlight_selected(GnomeCanvasItem *);
static void game_won();

#ifdef DEBUG
static void dump_xml();
#endif


typedef struct _Board Board;
struct _Board {
  char  *pixmapfile;
  char  *text1;
  char  *text2;
  char  *text3;
};

/* XML */
static gboolean read_xml_file(char *fname);
static void init_xml();
static void add_xml_data(xmlDocPtr doc,xmlNodePtr xmlnode, GNode * child);
static void parse_doc(xmlDocPtr doc);
static gboolean read_xml_file(char *fname);
static void destroy_board_list();
static void destroy_board(Board * board);

/* This is the list of boards */
static GList *board_list = NULL;

#define VERTICAL_SEPARATION 30
#define HORIZONTAL_SEPARATION 30
#define TEXT_COLOR "white"

/* ================================================================ */
static int board_number; // between 0 and board_list.length-1
static int right_word; // between 1 and 3, indicates which choice is the right one (the player clicks on it

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem *image_item = NULL;
static GnomeCanvasItem *text1_item = NULL;
static GnomeCanvasItem *text2_item = NULL;
static GnomeCanvasItem *text3_item = NULL;
static GnomeCanvasItem *button1 = NULL, *button2 = NULL, *button3 = NULL;

static GnomeCanvasItem *imageid_create_item(GnomeCanvasGroup *parent);
static void imageid_destroy_all_items(void);
static void imageid_next_level(void);
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

/* Description of this plugin */
BoardPlugin menu_bp =
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
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if(gamewon == TRUE) /* the game is won */
    {
      game_won();
    }

  board_paused = pause;
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{
  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "imageid/imageid-bg.jpg");
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=4;
      gcomprisBoard->sublevel=0;
      gcomprisBoard->number_of_sublevel=2; /* Go to next level after this number of 'play' */
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL|GCOMPRIS_BAR_OK);
      gcompris_bar_set_timer(0);
      init_xml();
      gcompris_bar_set_maxtimer(g_list_length(board_list));

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
      imageid_destroy_all_items();
      destroy_board_list();
    }
}

static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=0;
      imageid_next_level();
    }
}

gboolean
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
  gamewon = FALSE;

  gcompris_bar_set_timer(board_number);

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
  int x, y, xp, yp, dx, place;
  GdkFont *gdk_font;
  GdkPixbuf *pixmap = NULL;
  GdkPixbuf *button_pixmap = NULL;
  char *str = NULL;
  Board * board;

  board_number = (gcomprisBoard->level-1) *2 + gcomprisBoard->sublevel;
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

  button_pixmap = gcompris_load_pixmap("gcompris/buttons/button_large.png");
  /* display the image */
  board = g_list_nth_data(board_list, board_number);
  assert(board != NULL);

  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, board->pixmapfile);
  pixmap = gcompris_load_pixmap(str);
  dx = (gcomprisBoard->width -
	HORIZONTAL_SEPARATION -
	gdk_pixbuf_get_width(button_pixmap) -
	gdk_pixbuf_get_width(pixmap))/2;
  x = HORIZONTAL_SEPARATION + gdk_pixbuf_get_width(button_pixmap) + dx;

  y = (gcomprisBoard->height - gdk_pixbuf_get_height(pixmap))/2;

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

  gdk_font = gdk_font_load ("-adobe-times-medium-r-normal--*-240-*-*-*-*-*-*");
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
  text1_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", _(buf[0]),
				      "font_gdk", gdk_font,
				      "x", (double) xp + gdk_pixbuf_get_width(button_pixmap)/2,
				      "y", (double) yp + gdk_pixbuf_get_height(button_pixmap)/2,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color", "white",
				      NULL);

  yp += gdk_pixbuf_get_height(button_pixmap) + VERTICAL_SEPARATION;
  button2 = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf",  button_pixmap,
				"x",  (double) xp,
				"y",  (double) yp,
				NULL);
  text2_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", _(buf[1]),
				      "font_gdk", gdk_font,
				      "x", (double) xp + gdk_pixbuf_get_width(button_pixmap)/2,
				      "y", (double) yp + gdk_pixbuf_get_height(button_pixmap)/2,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color", "white",
				      NULL);

  yp += gdk_pixbuf_get_height(button_pixmap) + VERTICAL_SEPARATION;
  button3 = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf",  button_pixmap,
				"x",  (double) xp,
				"y",  (double) yp,
				NULL);

  text3_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", _(buf[2]),
				      "font_gdk", gdk_font,
				      "x", (double) xp + gdk_pixbuf_get_width(button_pixmap)/2,
				      "y", (double) yp + gdk_pixbuf_get_height(button_pixmap)/2,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color", "white",
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

  if(gcomprisBoard->sublevel>=gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=0;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel)
      gcomprisBoard->level=gcomprisBoard->maxlevel;
    gcompris_play_sound (SOUNDLISTFILE, "bonus");
  }
  imageid_next_level();
}

/* ==================================== */
static void process_ok()
{
  gcompris_display_bonus(gamewon, BONUS_SMILEY);
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
    	temp = item;
	if (item == button1)
		temp = text1_item;
	if (item == button2)
		temp = text2_item;
	if (item == button3)
		temp = text3_item;

	assert(temp == button1 || temp == text1_item || temp == button2 || temp == text2_item || temp == button3 || temp == text3_item);
      if ( ( temp == text1_item && right_word == 1) ||
	    ( temp == text2_item && right_word == 2) ||
	    ( temp == text3_item && right_word == 3) ) {
	gamewon = TRUE;
      } else {
	gamewon = FALSE;
      }
      highlight_selected(temp);
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
  assert ( (item == text1_item) || (item == text2_item) || (item == text3_item));
  gnome_canvas_item_set(text1_item, "fill_color", TEXT_COLOR, NULL);
  gnome_canvas_item_set(text2_item, "fill_color", TEXT_COLOR, NULL);
  gnome_canvas_item_set(text3_item, "fill_color", TEXT_COLOR, NULL);
  gnome_canvas_item_set(item, "fill_color", "green", NULL);
}

/* ===================================
 *                XML stuff
 *                Ref : shapegame.c
 * ==================================== */
static void init_xml()
{
  char *filename;
  filename = g_strdup_printf("%s/%s/board1.xml", PACKAGE_DATA_DIR, gcomprisBoard->boarddir);
  printf("filename = %s %s %s\n", filename,PACKAGE_DATA_DIR,gcomprisBoard->boarddir);

  assert(g_file_exists(filename));
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
  printf("XML lentgh = %d\n", g_list_length(board_list));

  for(list = board_list; list != NULL; list = list->next) {
    Board * board = list->data;
    printf("xml = %s %s %s %s\n", board->pixmapfile, board->text1, board->text2, board->text3);
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
		pixmapfile = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	if (!strcmp(xmlnode->name, "text1") && (lang==NULL
	    || !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
		text1 = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
		text1 = convertUTF8Toisolat1(text1);
	if (!strcmp(xmlnode->name, "text2") && (lang==NULL
	    || !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
		text2 = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
		text2 = convertUTF8Toisolat1(text2);
	if (!strcmp(xmlnode->name, "text3") && (lang==NULL
	    || !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
		text3 = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
		text3 = convertUTF8Toisolat1(text3);
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
  if(!g_file_exists(fname))
    {
      g_warning(_("Couldn't find file %s !"), fname);
      return FALSE;
    }
  g_warning(_("find file %s !"), fname);

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
