/* gcompris - read_colors.c
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
static int gamewon;

static void		 process_ok(void);
static void		 highlight_selected(int);
static void		 game_won(void);

static void 		 init_xml();
static gboolean 	 read_xml_file(char *fname);

/* ================================================================ */
static GnomeCanvasGroup *boardRootItem = NULL;
static GnomeCanvasItem *highlight_image_item = NULL;
static GnomeCanvasItem *color_item = NULL;
static GnomeCanvasItem *clock_image_item = NULL;
static GdkPixbuf *clock_pixmap = NULL;

static GnomeCanvasItem *read_colors_create_item(GnomeCanvasGroup *parent);
static void read_colors_destroy_all_items(void);
static void read_colors_next_level(void);
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static void update_clock();

static int highlight_width, highlight_height, errors;
static GList * listColors = NULL;

#define LAST_COLOR 11
#define MAX_ERRORS 10
#define CLOCK_X 40
#define CLOCK_Y 420

static char* colors[LAST_COLOR];

static int X[] = {100,300,500,700,200,400,600,100,300,500,700};
static int Y[] = {90,90,90,90,180,180,180,270,270,270,270};

/* (x1,y1) and (x2, y2) are the coordinates of the rectangle where to
   draw the color's name */
static int color_x1 = 240, color_x2 = 570;
static int color_y1 = 395, color_y2 = 490;

#define RADIUS 60

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Read Colors"),
    N_("Click on the corresponding color"),
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
    NULL,//set_level,
    NULL,
    NULL
  };

/* =====================================================================
 *
 * =====================================================================*/
BoardPlugin *get_bplugin_info(void) {
  return &menu_bp;
}

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
  GList * list = NULL;
  int * item;
  int i;

  if(agcomprisBoard!=NULL) {
    gcomprisBoard=agcomprisBoard;
    gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "read_colors/read_colors_background.png");
    gcomprisBoard->level=1;
    gcomprisBoard->maxlevel=1;
    gcompris_bar_set(0);

    gamewon = FALSE;
    errors = MAX_ERRORS;

    init_xml();

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
    g_list_free(list);

    gtk_signal_connect(GTK_OBJECT(gcomprisBoard->canvas), "event",  (GtkSignalFunc) item_event, NULL);
    read_colors_next_level();
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
    read_colors_destroy_all_items();
    // free list
    while (g_list_length(listColors) > 0)
      listColors = g_list_remove(listColors, g_list_nth_data(listColors,0));

    g_list_free(listColors);
    listColors = NULL;
  }
  gcomprisBoard = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
  if (gcomprisBoard) {
    if(g_strcasecmp(gcomprisBoard->type, "read_colors")==0) {
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
static void read_colors_next_level() {

  read_colors_destroy_all_items();
  gamewon = FALSE;

  /* Try the next level */
  read_colors_create_item(gnome_canvas_root(gcomprisBoard->canvas));

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
static void read_colors_destroy_all_items() {
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static GnomeCanvasItem *read_colors_create_item(GnomeCanvasGroup *parent) {
  GdkPixbuf *highlight_pixmap = NULL;
  char *str = NULL;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  str = g_strdup_printf("%s/%s", gcomprisBoard->boarddir, "read_colors_highlight.png");
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
static void game_won() {
  gcomprisBoard->sublevel++;

  listColors = g_list_remove(listColors, g_list_nth_data(listColors,0));

  if( g_list_length(listColors) <= 0 ) { // the current board is finished : bail out
    board_finished(BOARD_FINISHED_TUXLOCO);
    return;
  }

  read_colors_next_level();
}
/* =====================================================================
 *
 * =====================================================================*/
static gboolean process_ok_timeout() {
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

static void process_ok() {
  gcompris_bar_hide(TRUE);
  // leave time to display the right answer
  g_timeout_add(TIME_CLICK_TO_BONUS, process_ok_timeout, NULL);
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
      for (i=0; i<LAST_COLOR; i++) {
	if (hypot((double) (X[i]-x),(double)(Y[i]-y)) < RADIUS) {
	  clicked = i;
	  break;
	}
      }

      if (clicked >= 0) {
	board_paused = TRUE;
	highlight_selected(clicked);
	gamewon = (clicked == GPOINTER_TO_INT(g_list_nth_data(listColors,0)));
	process_ok();
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
static void highlight_selected(int c) {
  int x, y;

  assert(c>=0 && c<LAST_COLOR);

  x = X[c];
  y = Y[c];

  x -= highlight_width/2;
  y -= highlight_height/2;
  gnome_canvas_item_show(highlight_image_item);
  item_absolute_move(highlight_image_item, x, y);
}
/* ===================================
 *                XML stuff
 * ==================================== */
static void init_xml()
{
  char *filename;
  filename = g_strdup_printf("%s/%s/board1.xml", PACKAGE_DATA_DIR, gcomprisBoard->boarddir);
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

    lang = xmlGetProp(xmlnode,"lang");

    // try to match color[i]
    for (i=0; i<LAST_COLOR; i++) {
      sColor = g_strdup_printf("color%d", i+1);
      if (!strcmp(xmlnode->name, sColor)) {
	if (lang == NULL) { // get default value
	  text = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	  colors[i] = text;
	} else // get correct language
	  if ( !strcmp(lang, gcompris_get_locale())	|| !strncmp(lang, gcompris_get_locale(), 2) ) {
	    text = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	    g_warning("color prop::lang=%s locale=%s text=%s\n", lang, gcompris_get_locale(), text);
	    colors[i] = text;
	    color++;
	  }
	break;
      }
      g_free(sColor);
    } // end for
    xmlnode = xmlnode->next;
  }

  g_warning("colors found in XML = %d\n", color);
  for (color=0; color<LAST_COLOR; color++)
    g_warning("%d %s\n", color, colors[color]);

  // I really don't know why this test, but otherwise, the list is doubled
  // with 1 line on 2 filled with NULL elements
  if ( text == NULL)
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
     g_strcasecmp(doc->children->name,"ReadColors")!=0) {
    xmlFreeDoc(doc);
    return FALSE;
  }

  parse_doc(doc);
  xmlFreeDoc(doc);
  return TRUE;
}
