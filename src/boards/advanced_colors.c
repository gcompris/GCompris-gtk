/* gcompris - advanced_colors.c
 *
 * Copyright (C) 2002 Pascal Georges
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
#include <assert.h>

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
static GnomeCanvasGroup *boardRootItem = NULL;
static GnomeCanvasItem *highlight_image_item = NULL;
static GnomeCanvasItem *clock_image_item = NULL;
static GdkPixbuf *clock_pixmap = NULL;
static GnomeCanvasItem *color_item = NULL;

static GnomeCanvasItem *colors_create_item(GnomeCanvasGroup *parent);
static void colors_destroy_all_items(void);
static void colors_next_level(void);
static void set_level (guint);
static void update_clock();
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static int highlight_width, highlight_height;
static GList * listColors = NULL;

#define LAST_COLOR 8
#define LAST_BOARD 9
#define MAX_ERRORS 10
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

  gcompris_bar_hide(FALSE);
  if(gamewon == TRUE && pause == FALSE) /* the game is won */
    game_won();

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

    gcompris_bar_set(GCOMPRIS_BAR_LEVEL);
    gcompris_score_start(SCORESTYLE_NOTE,
			 gcomprisBoard->width - 220,
			 gcomprisBoard->height - 50,
			 gcomprisBoard->number_of_sublevel);

    gamewon = FALSE;
    errors = MAX_ERRORS;
    init_xml();

    gtk_signal_connect(GTK_OBJECT(gcomprisBoard->canvas), "event",  (GtkSignalFunc) item_event, NULL);

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
    gcompris_score_end();
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
  int i;
  GList * list = NULL;
  int * item;

  colors_destroy_all_items();
  gamewon = FALSE;

  gcompris_score_set(gcomprisBoard->sublevel);
  gcompris_bar_set_level(gcomprisBoard);

  /* initialize board only once*/
  if (gcomprisBoard->sublevel == 1) {
    // we generate a list of color indexes in a random order
    while (g_list_length(listColors) > 0)
      listColors = g_list_remove(listColors, g_list_nth_data(listColors,0));

    for (i=0; i<LAST_COLOR; i++)
      list = g_list_append(list, GINT_TO_POINTER(i));

    while ((g_list_length(list) > 0)) {
      i = RAND(0,g_list_length(list)-1);
      item = g_list_nth_data(list, i);
      listColors = g_list_append(listColors, item);
      list = g_list_remove(list, item);
    }

    /* set background */
    str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, backgroundFile);
    g_warning("background = %s\n", str);
    gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), str);
    g_free(str);
  }

  colors_create_item(gnome_canvas_root(gcomprisBoard->canvas));

  /* show text of color to find */
  color_item = gnome_canvas_item_new (boardRootItem,
				      gnome_canvas_text_get_type (),
				      "text", colors[GPOINTER_TO_INT(g_list_nth_data(listColors,0))],
				      "font", gcompris_skin_font_board_title_bold,
				      "x", (double) (color_x1+color_x2)/2,
				      "y", (double) (color_y1+color_y2)/2,
				      "anchor", GTK_ANCHOR_CENTER,
				      "fill_color", "darkblue",
				      NULL);

}
/* =====================================================================
 * Destroy all the items
 * =====================================================================*/
static void colors_destroy_all_items() {
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static GnomeCanvasItem *colors_create_item(GnomeCanvasGroup *parent) {
  GdkPixbuf *highlight_pixmap = NULL;

  char *str = NULL;
  int i;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, "advanced_colors_highlight.png");
  highlight_pixmap = gcompris_load_pixmap(str);

  highlight_image_item = gnome_canvas_item_new (boardRootItem,
						gnome_canvas_pixbuf_get_type (),
						"pixbuf", highlight_pixmap,
						"x", (double) 0,
						"y", (double) 0,
						"width", (double) gdk_pixbuf_get_width(highlight_pixmap),
						"height", (double) gdk_pixbuf_get_height(highlight_pixmap),
						"width_set", TRUE,
						"height_set", TRUE,
						NULL);

  highlight_width = gdk_pixbuf_get_width(highlight_pixmap);
  highlight_height = gdk_pixbuf_get_height(highlight_pixmap);

  g_free(str);
  gnome_canvas_item_hide(highlight_image_item);
  i = RAND(0,LAST_COLOR);

  gdk_pixbuf_unref(highlight_pixmap);

  /* setup the clock */
  str = g_strdup_printf("%s%d.png", "gcompris/timers/clock",errors);
  clock_pixmap = gcompris_load_pixmap(str);

  clock_image_item = gnome_canvas_item_new (boardRootItem,
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", clock_pixmap,
					    "x", (double) CLOCK_X,
					    "y", (double) CLOCK_Y,
					    "width", (double) gdk_pixbuf_get_width(clock_pixmap),
					    "height", (double) gdk_pixbuf_get_height(clock_pixmap),
					    "width_set", TRUE,
					    "height_set", TRUE,
					    NULL);

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
      board_finished(BOARD_FINISHED_TUXLOCO);
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
static gboolean ok_timeout() {
  g_warning("+++ ok_timeout errors = %d\n", errors);
  gcompris_display_bonus(gamewon, BONUS_SMILEY);
  if (!gamewon)
    errors--;
  if (errors <1)
    errors = 1;
  update_clock();

  if (errors <= 1) {
    board_finished(BOARD_FINISHED_TOOMANYERRORS);
  }

  return FALSE;
}

static void ok() {
  gcompris_bar_hide(TRUE);
  // leave time to display the right answer
  g_timeout_add(TIME_CLICK_TO_BONUS, ok_timeout, NULL);
}

/* =====================================================================
 *
 * =====================================================================*/
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data) {
  double x, y;
  int i, j, clicked;

  x = event->button.x;
  y = event->button.y;

  if (!gcomprisBoard || board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      gnome_canvas_c2w (gcomprisBoard->canvas, x, y, &x, &y);
      clicked = -1;
      for (i=0; i<4; i++) {
	for (j=0; j<2; j++) {
	  if (x>X[i*2] && x<X[i*2+1] && y>Y[j*2] && y<Y[j*2+1]) {
	    clicked = j*4 + i;
	  }
	}
      }
      if (clicked >= 0) {
	board_paused = TRUE;
	highlight_selected(clicked);
	gamewon = (clicked == GPOINTER_TO_INT(g_list_nth_data(listColors,0)));
	ok();
      }
      break;

    default:
      break;
    }
  return FALSE;
}

/* =====================================================================
 *
 * =====================================================================*/
static void update_clock() {
  char *str = g_strdup_printf("%s%d.png", "gcompris/timers/clock",errors);

  gtk_object_destroy (GTK_OBJECT(clock_image_item));

  clock_pixmap = gcompris_load_pixmap(str);

  clock_image_item = gnome_canvas_item_new (boardRootItem,
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", clock_pixmap,
					    "x", (double) CLOCK_X,
					    "y", (double) CLOCK_Y,
					    "width", (double) gdk_pixbuf_get_width(clock_pixmap),
					    "height", (double) gdk_pixbuf_get_height(clock_pixmap),
					    "width_set", TRUE,
					    "height_set", TRUE,
					    NULL);

  gdk_pixbuf_unref(clock_pixmap);
  g_free(str);
}
/* =====================================================================
 *
 * =====================================================================*/
static void highlight_selected(int c) {
  int x, y;

  assert(c>=0 && c<8);

  x = (X[c*2] + X[c*2+1]) /2;
  y = (Y[(int)(c/4)*2] + Y[(int)(c/4)*2+1]) /2;

  x -= highlight_width/2;
  y -= highlight_height;
  gnome_canvas_item_show(highlight_image_item);
  item_absolute_move(highlight_image_item, x, y);
}

/* ===================================
 *                XML stuff
 * ==================================== */
static void init_xml()
{
  char *filename;
  filename = g_strdup_printf("%s/%s/board%d.xml", PACKAGE_DATA_DIR, gcomprisBoard->boarddir,
			     gcomprisBoard->level);
  g_warning("filename = %s %s %s\n", filename,PACKAGE_DATA_DIR,gcomprisBoard->boarddir);

  assert(g_file_test(filename, G_FILE_TEST_EXISTS));
  assert(read_xml_file(filename)== TRUE);
  g_free(filename);
}

/* ==================================== */
static void add_xml_data(xmlDocPtr doc, xmlNodePtr xmlnode, GNode * child)
{
  char *text = NULL;
  char * tmp = NULL;
  char *sColor = NULL;
  int color = 0;
  int i;
  gchar *lang;

  xmlnode = xmlnode->xmlChildrenNode;

  xmlnode = xmlnode->next;

  while (xmlnode != NULL) {
    if (!strcmp(xmlnode->name, "pixmapfile"))
      backgroundFile = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);

    lang = xmlGetProp(xmlnode,"lang");

    // try to match color[i]
    for (i=0; i<8; i++) {
      sColor = g_strdup_printf("color%d", i+1);
      if (!strcmp(xmlnode->name, sColor)) {
	if (lang == NULL) { // get default value
	  text = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	  colors[i] = text;
	} else { // get correct language
	  if ( !strncmp(lang, gcompris_get_locale(), strlen(lang)) ) {
	    text = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	    g_warning("color prop::lang=%s locale=%s text=%s\n", lang, gcompris_get_locale(), text);
	    colors[i] = text;
	  }
	}
	g_free(sColor);
	break;
      }
      g_free(sColor);
    } // end for
    xmlnode = xmlnode->next;
  }

  g_warning("colors found in XML:\n");
  for (color=0; color<8; color++)
    g_warning("%d %s\n", color, colors[color]);

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
     g_strcasecmp(doc->children->name,"AdvancedColors")!=0) {
    xmlFreeDoc(doc);
    return FALSE;
  }

  parse_doc(doc);
  xmlFreeDoc(doc);
  return TRUE;
}

