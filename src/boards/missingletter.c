/* gcompris - missingletter.c
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

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);
static int gamewon;
static void		 process_ok(void);
static void		 highlight_selected(GnomeCanvasItem *);
static void		 game_won(void);

typedef struct _Board Board;
struct _Board {
  char  *pixmapfile;
  char *question;
  char *answer;
  char  *l1;
  char  *l2;
  char  *l3;
};

static Board * board;

/* XML */
static gboolean		 read_xml_file(char *fname);
static void		 init_xml(void);
static void		 add_xml_data(xmlDocPtr, xmlNodePtr, GNode *);
static void		 parse_doc(xmlDocPtr doc);
static gboolean		 read_xml_file(char *fname);
static void		 destroy_board_list();
static void		 destroy_board(Board * board);

/* This is the list of boards */
static GList *board_list = NULL;

#define VERTICAL_SEPARATION 30
#define HORIZONTAL_SEPARATION 30

//NUMBER_OF_SUBLEVELS*NUMBER_OF_LEVELS must equal the number of boards in XML file
#define NUMBER_OF_SUBLEVELS 9
#define NUMBER_OF_LEVELS 4

#define TEXT_COLOR "white"

/* ================================================================ */
static int board_number; // between 0 and board_list.length-1
static int right_word; // between 1 and 3, indicates which choice is the right one (the player clicks on it

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem *image_item = NULL;
static GnomeCanvasItem *l1_item = NULL;
static GnomeCanvasItem *l2_item = NULL;
static GnomeCanvasItem *l3_item = NULL;
static GnomeCanvasItem *text = NULL;
static GnomeCanvasItem *button1 = NULL, *button2 = NULL, *button3 = NULL;

static GnomeCanvasItem *missing_letter_create_item(GnomeCanvasGroup *parent);
static void missing_letter_destroy_all_items(void);
static void missing_letter_next_level(void);
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

  gcompris_bar_hide(FALSE);

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
  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      "missing_letter/missingletter-bg.jpg");
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=NUMBER_OF_LEVELS;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=NUMBER_OF_SUBLEVELS; /* Go to next level after this number of 'play' */
      init_xml();
      assert(NUMBER_OF_LEVELS*NUMBER_OF_SUBLEVELS == g_list_length(board_list));
      gcompris_score_start(SCORESTYLE_NOTE,
			   50,
			   gcomprisBoard->height - 50,
			   gcomprisBoard->number_of_sublevel);
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

      missing_letter_next_level();

      gamewon = FALSE;
      pause_board(FALSE);
    }
}

static void end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gcompris_score_end();
      missing_letter_destroy_all_items();
      destroy_board_list();
    }
  gcomprisBoard = NULL;
}

static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      missing_letter_next_level();
    }
}

gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "missing_letter")==0)
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
static void missing_letter_next_level()
{
  gcompris_bar_set_level(gcomprisBoard);

  missing_letter_destroy_all_items();
  gamewon = FALSE;

  gcompris_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  missing_letter_create_item(gnome_canvas_root(gcomprisBoard->canvas));

}
/* ==================================== */
/* Destroy all the items */
static void missing_letter_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
static GnomeCanvasItem *missing_letter_create_item(GnomeCanvasGroup *parent)
{
  char *buf[3];
  int xOffset,yOffset,place;
  double dx1, dy1, dx2, dy2;
  GdkPixbuf *button_pixmap = NULL;
  GdkPixbuf *pixmap = NULL;
  char *str = NULL;

  /* This are the values of the area in which we must display the image */
  gint img_area_x = 290;
  gint img_area_y = 80;
  gint img_area_w = 440;
  gint img_area_h = 310;

  /* this is the coordinate of the text to find */
  gint txt_area_x = 515;
  gint txt_area_y = 430;

  board_number = (gcomprisBoard->level-1) * NUMBER_OF_SUBLEVELS + gcomprisBoard->sublevel-1;

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
  button_pixmap = gcompris_load_skin_pixmap("button.png");
  /* display the image */
  board = g_list_nth_data(board_list, board_number);
  assert(board != NULL);
  str = g_strdup_printf("%s/%s", "imageid"/*gcomprisBoard->boarddir*/, board->pixmapfile);
  pixmap = gcompris_load_pixmap(str);

  yOffset = (gcomprisBoard->height - gdk_pixbuf_get_height(button_pixmap) - gdk_pixbuf_get_height(pixmap) - 2*VERTICAL_SEPARATION)/2;

  text = gnome_canvas_item_new (boardRootItem,
				gnome_canvas_text_get_type (),
				"text", _(board->question),
				"font", gcompris_skin_font_board_big,
				"x", (double) txt_area_x,
				"y", (double) txt_area_y,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color", TEXT_COLOR,
				NULL);

  gnome_canvas_item_get_bounds(text, &dx1, &dy1, &dx2, &dy2);
  yOffset += VERTICAL_SEPARATION + dy2-dy1;

  image_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_pixbuf_get_type (),
				      "pixbuf", pixmap,
				      "x", (double) img_area_x+(img_area_w - gdk_pixbuf_get_width(pixmap))/2,
				      "y", (double) img_area_y+(img_area_h - gdk_pixbuf_get_height(pixmap))/2,
				      "width", (double) gdk_pixbuf_get_width(pixmap),
				      "height", (double) gdk_pixbuf_get_height(pixmap),
				      "width_set", TRUE,
				      "height_set", TRUE,
				      NULL);
  gtk_signal_connect(GTK_OBJECT(image_item), "event", (GtkSignalFunc) item_event, NULL);
  gdk_pixbuf_unref(pixmap);
  g_free(str);
  /* display the 3 words */
  /* the right word is at position 0 : it is swapped with any position depending of place value */

  switch (place) {
  case 1 :
    buf[0] = board->l2;
    buf[1] = board->l1;
    buf[2] = board->l3;
    break;
  case 2 :
    buf[0] = board->l3;
    buf[1] = board->l2;
    buf[2] = board->l1;
    break;
  default :
    buf[0] = board->l1;
    buf[1] = board->l2;
    buf[2] = board->l3;
    break;
  }

  yOffset = ( gcomprisBoard->height - 3*gdk_pixbuf_get_height(button_pixmap) - 2*VERTICAL_SEPARATION) / 2;
  xOffset = (img_area_x-gdk_pixbuf_get_width(button_pixmap))/2;
  button1 = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf",  button_pixmap,
				   "x",  (double) xOffset,
				   "y",  (double) yOffset,
				   NULL);
  l1_item = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_text_get_type (),
				   "text", buf[0],
				   "font", gcompris_skin_font_board_big,
				   "x", (double) xOffset + gdk_pixbuf_get_width(button_pixmap)/2,
				   "y", (double) yOffset + gdk_pixbuf_get_height(button_pixmap)/2,
				   "anchor", GTK_ANCHOR_CENTER,
				   "fill_color", TEXT_COLOR,
				   NULL);

  yOffset += HORIZONTAL_SEPARATION + gdk_pixbuf_get_height(button_pixmap);
  button2 = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf",  button_pixmap,
				   "x",  (double) xOffset,
				   "y",  (double) yOffset,
				   NULL);
  l2_item = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_text_get_type (),
				   "text", buf[1],
				   "font", gcompris_skin_font_board_big,
				   "x", (double) xOffset + gdk_pixbuf_get_width(button_pixmap)/2,
				   "y", (double) yOffset + gdk_pixbuf_get_height(button_pixmap)/2,
				   "anchor", GTK_ANCHOR_CENTER,
				   "fill_color", TEXT_COLOR,
				   NULL);

  yOffset += HORIZONTAL_SEPARATION + gdk_pixbuf_get_height(button_pixmap);

  button3 = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf",  button_pixmap,
				   "x",  (double) xOffset,
				   "y",  (double) yOffset,
				   NULL);
  l3_item = gnome_canvas_item_new (boardRootItem,
				   gnome_canvas_text_get_type (),
				   "text", buf[2],
				   "font", gcompris_skin_font_board_big,
				   "x", (double) xOffset + gdk_pixbuf_get_width(button_pixmap)/2,
				   "y", (double) yOffset + gdk_pixbuf_get_height(button_pixmap)/2,
				   "anchor", GTK_ANCHOR_CENTER,
				   "fill_color", TEXT_COLOR,
				   NULL);

  gdk_pixbuf_unref(button_pixmap);

  gtk_signal_connect(GTK_OBJECT(l1_item), "event", (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(l2_item), "event", (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(l3_item), "event", (GtkSignalFunc) item_event, NULL);

  gtk_signal_connect(GTK_OBJECT(button1), "event",  (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(button2), "event",  (GtkSignalFunc) item_event, NULL);
  gtk_signal_connect(GTK_OBJECT(button3), "event",  (GtkSignalFunc) item_event, NULL);
  return NULL;
}
/* ==================================== */
static void game_won() {
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) {
	board_finished(BOARD_FINISHED_TUXPLANE);
	return;
    }
  }
  missing_letter_next_level();
}

/* ==================================== */
static gboolean process_ok_timeout() {
  gcompris_display_bonus(gamewon, BONUS_FLOWER);
  return FALSE;
}

static void process_ok() {
  if (gamewon) {
    gnome_canvas_item_set(text, "text", board->answer, NULL);
  }
  // leave time to display the right answer
  gcompris_bar_hide(TRUE);
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
      temp = item;
      if (item == button1)
	temp = l1_item;
      if (item == button2)
	temp = l2_item;
      if (item == button3)
	temp = l3_item;

      assert(temp == button1 || temp == l1_item || temp == button2 || temp == l2_item || temp == button3 || temp == l3_item);
      if ( ( temp == l1_item && right_word == 1) ||
	   ( temp == l2_item && right_word == 2) ||
	   ( temp == l3_item && right_word == 3) ) {
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
  assert ( (item == l1_item) || (item == l2_item) || (item == l3_item));
  gnome_canvas_item_set(l1_item, "fill_color", TEXT_COLOR, NULL);
  gnome_canvas_item_set(l2_item, "fill_color", TEXT_COLOR, NULL);
  gnome_canvas_item_set(l3_item, "fill_color", TEXT_COLOR, NULL);
  gnome_canvas_item_set(item, "fill_color", "green", NULL);
}

/* ===================================
 *                XML stuff
 *                Ref : shapegame.c
 * ==================================== */
/* ======  for DEBUG ========  */
static void dump_xml() {
  GList *list;
  printf("XML lentgh = %d\n", g_list_length(board_list));

  for(list = board_list; list != NULL; list = list->next) {
    Board * board = list->data;
    printf("xml = %s %s %s %s %s %s\n", board->pixmapfile, board->answer, board->question, board->l1, board->l2, board->l3);
  }
}
/* ==================================== */
static void init_xml()
{
  char *filename;
  filename = g_strdup_printf("%s/%s/board1.xml", PACKAGE_DATA_DIR, gcomprisBoard->boarddir);
  assert(g_file_test(filename, G_FILE_TEST_EXISTS));
  assert(read_xml_file(filename)== TRUE);
  g_free(filename);
  dump_xml();
}

/* ==================================== */
static void add_xml_data(xmlDocPtr doc, xmlNodePtr xmlnode, GNode * child)
{
	//int i;
  char *pixmapfile = NULL;
  char *question = NULL, *answer = NULL;
  char *l1 = NULL, *l2 = NULL, *l3 = NULL;
  char *data = NULL;
  Board * board = g_new(Board,1);

  xmlnode = xmlnode->xmlChildrenNode;

  xmlnode = xmlnode->next;

  while (xmlnode != NULL) {
    gchar *lang = xmlGetProp(xmlnode,"lang");
    if (!strcmp(xmlnode->name, "pixmapfile") && (lang==NULL
						 || !strcmp(lang, gcompris_get_locale())
						 || !strncmp(lang, gcompris_get_locale(), 2)))
      pixmapfile = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
    if (!strcmp(xmlnode->name, "data") && (lang==NULL
					   || !strcmp(lang, gcompris_get_locale())
					   || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	data = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
      }
    xmlnode = xmlnode->next;
  }

  // I really don't know why this test, but otherwise, the list is doubled
  // with 1 line on 2 filled with NULL elements
  if ( pixmapfile == NULL || data == NULL)
    return;

/*  if ((i=sscanf(data, "%s / %s / %s / %s / %s", answer, question, l1, l2, l3)) != 5)
		printf("Error sscanf result != 5 = %i\n",i);
*/
	answer = strtok(data,"/");
	question = strtok(NULL,"/");
	l1 = strtok(NULL,"/");
	l2 = strtok(NULL,"/");
	l3 = strtok(NULL,"/");

	assert(l1 != NULL  && l2 != NULL && l3 != NULL && answer != NULL && question != NULL);

  board->pixmapfile = g_strdup(pixmapfile);
  board->question = g_strdup(question);
  board->answer = g_strdup(answer);
  board->l1 = g_strdup(l1);
  board->l2 = g_strdup(l2);
  board->l3 = g_strdup(l3);

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
     g_strcasecmp(doc->children->name,"missing_letter")!=0) {
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
  g_free(board->answer);
  g_free(board->question);
  g_free(board->l1);
  g_free(board->l2);
  g_free(board->l3);
  g_free(board);
}
