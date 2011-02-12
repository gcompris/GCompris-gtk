/* gcompris - advanced_colors.c
 *
 * Copyright (C) 2002, 2008 Pascal Georges
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

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static int gamewon, errors;

static void		 ok(void);
static void		 highlight_selected(int);
static void		 game_won(void);
static void		 init_xml(void);
static gboolean		 read_xml_file(char *fname);

/* ================================================================ */
static GooCanvasItem *boardRootItem = NULL;
static GooCanvasItem *highlight_image_item = NULL;
static GooCanvasItem *clock_image_item = NULL;
static GooCanvasItem *color_item = NULL;

static GooCanvasItem *colors_create_item(GooCanvasItem *parent);
static void colors_destroy_all_items(void);
static void colors_next_level(void);
static void set_level (guint);
static void update_clock();
static gboolean item_event (GooCanvasItem  *item,
			    GooCanvasItem  *target,
			    GdkEventButton *event,
			    gpointer data);
static int highlight_width, highlight_height;
static GList * listColors = NULL;
static gint timer_id = 0;

#define LAST_COLOR 8
#define LAST_BOARD 9
#define MAX_ERRORS 4
#define CLOCK_X 40
#define CLOCK_Y 420


static char* colors[LAST_COLOR];
static char *backgroundFile = NULL;

static int X[] = {57,229,236,389,413,567,573,744,
		  7,207,212,388,415,589,594,794};
static int Y[] = {158,255,268,380};

/* (x1,y1) and (x2, y2) are the coordinates of the rectangle where to
   draw the color's name */
static int color_x1 = 199, color_x2 = 582;
static int color_y1 = 47, color_y2 = 133;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Advanced colors",
    "Click on the right color",
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
    NULL,
    set_level,
    NULL,
    NULL,
    NULL,
    NULL
  };

/* =====================================================================
 *
 * =====================================================================*/
GET_BPLUGIN_INFO(advanced_colors)

/* =====================================================================
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 * =====================================================================*/
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if (timer_id) {
    gtk_timeout_remove (timer_id);
    timer_id = 0;
  }

  if(gamewon == TRUE && pause == FALSE) /* the game is won */
    game_won();

  if(gamewon == FALSE && pause == FALSE &&
     errors < 1) {
    gcomprisBoard->sublevel = 1;
    colors_next_level();
  }

  board_paused = pause;
}

/* =====================================================================
 *
 * =====================================================================*/
static void start_board (GcomprisBoard *agcomprisBoard) {
  if(agcomprisBoard!=NULL) {
    gcomprisBoard=agcomprisBoard;
    gcomprisBoard->level=1;
    gcomprisBoard->maxlevel=LAST_BOARD;
    gcomprisBoard->sublevel = 1;
    gcomprisBoard->number_of_sublevel = 8;

    gc_bar_set(GC_BAR_LEVEL);
    gc_score_start(SCORESTYLE_NOTE,
		   BOARDWIDTH - 195,
		   BOARDHEIGHT - 30,
		   gcomprisBoard->number_of_sublevel);

    gamewon = FALSE;
    init_xml();

    g_signal_connect(goo_canvas_get_root_item(gcomprisBoard->canvas),
		     "button_press_event",
    		     (GtkSignalFunc) item_event, NULL);

    colors_next_level();
    pause_board(FALSE);
  }
}

/* =====================================================================
 *
 * =====================================================================*/
static void end_board () {

  if(gcomprisBoard!=NULL){
    pause_board(TRUE);
    gc_score_end();
    colors_destroy_all_items();
    // free list
    while (g_list_length(listColors) > 0)
      listColors = g_list_remove(listColors, g_list_nth_data(listColors,0));
  }
  gcomprisBoard = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static void set_level (guint level) {
  if(gcomprisBoard!=NULL) {
    gcomprisBoard->level=level;
    gcomprisBoard->sublevel=1;
    init_xml();
    colors_next_level();
  }
}

/* =====================================================================
 *
 * =====================================================================*/
static gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
  if (gcomprisBoard) {
    if(g_strcasecmp(gcomprisBoard->type, "advanced_colors")==0) {
      /* Set the plugin entry */
      gcomprisBoard->plugin=&menu_bp;
      return TRUE;
    }
  }
  return FALSE;
}
/* =====================================================================
 * set initial values for the next level
 * =====================================================================*/
static void colors_next_level() {
  gchar *str = NULL;
  int i, list_length;
  GList * list = NULL;
  int * item;

  colors_destroy_all_items();
  gamewon = FALSE;

  gc_score_set(gcomprisBoard->sublevel);
  gc_bar_set_level(gcomprisBoard);

  /* initialize board only once*/
  if (gcomprisBoard->sublevel == 1) {
    errors = MAX_ERRORS;
    // we generate a list of color indexes in a random order
    while (g_list_length(listColors) > 0)
      listColors = g_list_remove(listColors, g_list_nth_data(listColors,0));

    for (i=0; i<LAST_COLOR; i++)
      list = g_list_append(list, GINT_TO_POINTER(i));

    while((list_length = g_list_length(list))) {
      i = list_length == 1 ? 0 : g_random_int_range(0,g_list_length(list)-1);
      item = g_list_nth_data(list, i);
      listColors = g_list_append(listColors, item);
      list = g_list_remove(list, item);
    }

    /* set background */
    str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, backgroundFile);
    g_warning("background = %s\n", str);
    gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas), str);
    g_free(str);
  }

  colors_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));

  /* show text of color to find */
  color_item = goo_canvas_text_new (boardRootItem,
				    colors[GPOINTER_TO_INT(g_list_nth_data(listColors,0))],
				    (color_x1+color_x2)/2,
				    (color_y1+color_y2)/2,
				    -1,
				    GTK_ANCHOR_CENTER,
				    "font", gc_skin_font_board_title_bold,
				    "fill-color", "darkblue",
				    NULL);

}
/* =====================================================================
 * Destroy all the items
 * =====================================================================*/
static void colors_destroy_all_items()
{
  if (timer_id) {
    gtk_timeout_remove (timer_id);
    timer_id = 0;
  }

  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static GooCanvasItem *colors_create_item(GooCanvasItem *parent)
{
  GdkPixbuf *pixmap;
  char *str = NULL;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);


  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir,
			"highlight.png");
  pixmap = gc_pixmap_load(str);

  highlight_image_item = goo_canvas_image_new (boardRootItem,
					       pixmap,
					       0,
					       0,
					       NULL);

  highlight_width = gdk_pixbuf_get_width(pixmap);
  highlight_height = gdk_pixbuf_get_height(pixmap);

  g_free(str);
  g_object_set (highlight_image_item,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);

  gdk_pixbuf_unref(pixmap);

  /* setup the clock */
  str = g_strdup_printf("%s%d.png", "timers/clock",errors);
  pixmap = gc_skin_pixmap_load(str);

  clock_image_item = goo_canvas_image_new (boardRootItem,
					   pixmap,
					   CLOCK_X,
					   CLOCK_Y,
					   NULL);

  gdk_pixbuf_unref(pixmap);
  g_free(str);

  return NULL;
}
/* =====================================================================
 *
 * =====================================================================*/
static void game_won() {

  listColors = g_list_remove(listColors, g_list_nth_data(listColors,0));

  if( g_list_length(listColors) <= 0 ) { // the current board is finished
    gcomprisBoard->level++;
    gcomprisBoard->sublevel = 1;
    if (gcomprisBoard->level > gcomprisBoard->maxlevel) {
      gcomprisBoard->level = gcomprisBoard->maxlevel;
      return;
    }

    init_xml();
  } else { // the current board is not finished
    gcomprisBoard->sublevel++;
  }

  colors_next_level();
}
/* =====================================================================
 *
 * =====================================================================*/
static gboolean ok_timeout()
{
  timer_id = 0;
  gc_bonus_display(gamewon, GC_BONUS_SMILEY);
  if (!gamewon)
    errors--;
  if (errors >= 1)
    update_clock();

  g_object_set (highlight_image_item,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);
  return FALSE;
}

static void ok() {
  // leave time to display the right answer
  timer_id = g_timeout_add(TIME_CLICK_TO_BONUS, ok_timeout, NULL);
}

/* =====================================================================
 *
 * =====================================================================*/
static gboolean item_event (GooCanvasItem  *item,
			    GooCanvasItem  *target,
			    GdkEventButton *event,
			    gpointer data)
{
  double x, y;
  int i, j, clicked;

  x = event->x;
  y = event->y;

  if (!gcomprisBoard || board_paused)
    return FALSE;

  //goo_canvas_c2w (gcomprisBoard->canvas, x, y, &x, &y);
  clicked = -1;
  for (i=0; i<4; i++) {
    for (j=0; j<2; j++) {
      if (x>X[i*2] && x<X[i*2+1] && y>Y[j*2] && y<Y[j*2+1]) {
	clicked = j*4 + i;
      }
    }
  }
  if (clicked >= 0) {
    gc_sound_play_ogg ("sounds/bleep.wav", NULL);
    board_paused = TRUE;
    highlight_selected(clicked);
    gamewon = (clicked == GPOINTER_TO_INT(g_list_nth_data(listColors,0)));
    ok();
  }

  return FALSE;
}

/* =====================================================================
 *
 * =====================================================================*/
static void update_clock()
{
  GdkPixbuf *pixmap;
  char *str = g_strdup_printf("%s%d.png", "timers/clock", errors);

  pixmap = gc_skin_pixmap_load(str);

  g_object_set(clock_image_item,
	       "pixbuf", pixmap,
	       NULL);

  gdk_pixbuf_unref(pixmap);
  g_free(str);
}
/* =====================================================================
 *
 * =====================================================================*/
static void highlight_selected(int c)
{
  int x, y;

  g_assert(c>=0 && c<8);

  x = (X[c*2] + X[c*2+1]) /2;
  y = (Y[(int)(c/4)*2] + Y[(int)(c/4)*2+1]) /2;

  x -= highlight_width/2;
  y -= highlight_height;
  g_object_set (highlight_image_item,
		"visibility", GOO_CANVAS_ITEM_VISIBLE,
		NULL);
  gc_item_absolute_move(highlight_image_item, x, y);
}

/* ===================================
 *                XML stuff
 * ==================================== */
static void init_xml()
{
  char *filename;

  filename = gc_file_find_absolute("%s/board%d.xml",
			     gcomprisBoard->boarddir,
			     gcomprisBoard->level);

  g_assert(read_xml_file(filename)== TRUE);

  g_free(filename);
}

/* ==================================== */
static void add_xml_data(xmlDocPtr doc, xmlNodePtr xmlnode, GNode * child)
{
  char *text = NULL;
  char *sColor = NULL;
  int i;

  xmlnode = xmlnode->xmlChildrenNode;

  xmlnode = xmlnode->next;

  while (xmlnode != NULL)
    {
      if (!strcmp((char *)xmlnode->name, "pixmapfile"))
	backgroundFile = (char *)xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);

      // try to match color[i]
      for (i=0; i<8; i++)
	{
	  sColor = g_strdup_printf("color%d", i+1);
	  if (!strcmp((char *)xmlnode->name, sColor))
	    {
	      text = (char*)xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	      if(text)
		{
		  colors[i] = gettext((char *)text);
		  // We got a translation, free the original value
		  if ( text != colors[i] )
		    g_free(text);
		}
	      text = NULL;
	      g_free(sColor);
	      break;
	    }
	  g_free(sColor);
	} // end for
      xmlnode = xmlnode->next;
    }

  // I really don't know why this test, but otherwise, the list is doubled
  // with 1 line on 2 filled with NULL elements
  if ( backgroundFile == NULL || text == NULL)
    return;

}

/* ==================================== */
static void parse_doc(xmlDocPtr doc)
{
  xmlNodePtr node;

  for(node = doc->children->children; node != NULL; node = node->next) {
    if ( g_strcasecmp((char *)node->name, "Board") == 0 )
      add_xml_data(doc, node, NULL);
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
     g_strcasecmp((char *)doc->children->name,"AdvancedColors")!=0) {
    xmlFreeDoc(doc);
    return FALSE;
  }

  parse_doc(doc);
  xmlFreeDoc(doc);
  return TRUE;
}

