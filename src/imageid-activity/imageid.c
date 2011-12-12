/* gcompris - imageid.c
 *
 * Copyright (C) 2001, 2008 Pascal Georges
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
static void		 highlight_selected(GooCanvasItem *);
static void		 game_won(void);

static void		 config_start(GcomprisBoard *agcomprisBoard,
				      GcomprisProfile *aProfile);
static void		 config_stop(void);

#define MAX_PROPOSAL 6
typedef struct {
  gchar  *pixmapfile;
  gchar  *text[MAX_PROPOSAL + 1];
  guint solution; /* Index to solution in text[] */
} Board;

/* XML */
static void		 init_xml(guint level);
static void		 add_xml_data(xmlDocPtr doc,xmlNodePtr xmlnode, GNode * child);
static void		 parse_doc(xmlDocPtr doc);
static gboolean		 read_xml_file(char *fname);
static void		 destroy_board_list(void);
static void		 destroy_board(Board * board);

/* This is the list of boards */
static GList *board_list = NULL;

#define HORIZONTAL_SEPARATION 30
// the values are taken from the backgound image imageid-bg.jpg
#define IMAGE_AREA_X1 280
#define IMAGE_AREA_X2 750
#define IMAGE_AREA_Y1 90
#define IMAGE_AREA_Y2 430

#define TEXT_COLOR "white"

/* ================================================================ */
static int board_number; // between 0 and board_list.length-1
static int right_word; // indicates which choice is the right one

static GooCanvasItem *selected_button;
static GooCanvasItem *boardRootItem = NULL;

static void imageid_create_item(GooCanvasItem *parent);
static void imageid_destroy_all_items(void);
static void imageid_next_level(void);
static gboolean item_event (GooCanvasItem  *item,
			    GooCanvasItem  *target,
			    GdkEventButton *event,
			    gpointer data);

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
static void
pause_board (gboolean pause)
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
static void
start_board (GcomprisBoard *agcomprisBoard)
{
  gchar *filename = NULL;
  GHashTable *config = gc_db_get_board_conf();

  gc_locale_set(g_hash_table_lookup( config, "locale"));

  g_hash_table_destroy(config);

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			"imageid/imageid-bg.svgz");
      gcomprisBoard->level=1;
      gcomprisBoard->sublevel=1;

      /* Calculate the maxlevel based on the available data file for this board */
      gcomprisBoard->maxlevel=1;

      while( (filename = gc_file_find_absolute("%s/board%d.xml",
					       gcomprisBoard->boarddir,
					       gcomprisBoard->maxlevel++,
					       NULL)) )
	{
	  g_free(filename);

	}
      g_free(filename);

      gcomprisBoard->maxlevel -= 2;
      gc_bar_set(GC_BAR_CONFIG|GC_BAR_LEVEL);

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
      gc_score_end();
      imageid_destroy_all_items();
      destroy_board_list();
    }

  gc_locale_set( NULL );

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
      if(g_ascii_strcasecmp(gcomprisBoard->type, "imageid")==0)
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
static void
imageid_next_level()
{
  gc_bar_set_level(gcomprisBoard);

  imageid_destroy_all_items();
  selected_button = NULL;
  gamewon = FALSE;

  destroy_board_list();
  init_xml(gcomprisBoard->level);

  gcomprisBoard->number_of_sublevel = g_list_length(board_list);

  gc_score_end();
  gc_score_start(SCORESTYLE_NOTE,
		 BOARDWIDTH - 195,
		 BOARDHEIGHT - 30,
		 gcomprisBoard->number_of_sublevel);

  gc_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  imageid_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));
}

static void
imageid_next_sublevel()
{
  imageid_destroy_all_items();
  selected_button = NULL;
  gamewon = FALSE;

  gc_score_set(gcomprisBoard->sublevel);

  /* Try the next level */
  imageid_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));
}

/* ==================================== */
/* Destroy all the items */
static void imageid_destroy_all_items()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem = NULL;
}

/* ==================================== */
static void
imageid_create_item(GooCanvasItem *parent)
{
  GooCanvasItem *item;
  GooCanvasItem *group;
  GooCanvasItem *button;
  int x, y, xp, yp;
  guint vertical_separation;
  GdkPixbuf *pixmap = NULL;
  Board *board;
  guint i;

  board_number = gcomprisBoard->sublevel-1;

  g_assert(board_number >= 0  && board_number < g_list_length(board_list));

  boardRootItem = \
    goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
			  NULL);

  /* display the image */
  board = g_list_nth_data(board_list, board_number);
  g_assert(board != NULL);

  right_word = board->solution;

  pixmap = gc_pixmap_load(board->pixmapfile);

  x = IMAGE_AREA_X1 + ( IMAGE_AREA_X2 - IMAGE_AREA_X1 - gdk_pixbuf_get_width(pixmap))/2;
  y = IMAGE_AREA_Y1 + ( IMAGE_AREA_Y2 - IMAGE_AREA_Y1 - gdk_pixbuf_get_height(pixmap))/2;

  item = goo_canvas_image_new (boardRootItem,
			       pixmap,
			       x,
			       y,
			       NULL);

  gdk_pixbuf_unref(pixmap);

  xp = HORIZONTAL_SEPARATION;

  /* Calc the number of proposals */
  i = 0;
  while(board->text[i++]);

  vertical_separation = 10 + 60 / i;

  guint button_pixmap_height = 48;
  guint button_pixmap_width = 191;
  yp = (BOARDHEIGHT -
	i *  button_pixmap_height
	- 2 * vertical_separation)/2 - 20;

  i = 0;
  while(board->text[i])
    {
      group = goo_canvas_group_new (boardRootItem,
				    NULL);

      button = goo_canvas_svg_new (group,
				   gc_skin_rsvg_get(),
				   "svg-id", "#BUTTON_TEXT",
				   NULL);
      SET_ITEM_LOCATION(button,
			xp,
			yp);

      g_object_set_data(G_OBJECT(group),
		      "button", button);

      g_signal_connect(button, "button_press_event",
		       (GCallback) item_event,
		       GINT_TO_POINTER(i));

      item = goo_canvas_text_new (group,
				  board->text[i],
				  xp +  button_pixmap_width/2 + 1.0,
				  yp +  button_pixmap_height/2 + 1.0,
				  -1,
				  GTK_ANCHOR_CENTER,
				  "font", gc_skin_font_board_big,
				  "fill_color_rgba", gc_skin_color_shadow,
				  NULL);

      g_signal_connect(item, "button_press_event",
		       (GCallback) item_event,
		       GINT_TO_POINTER(i));

      item = goo_canvas_text_new (group,
				  board->text[i],
				  xp +  button_pixmap_width/2,
				  yp +  button_pixmap_height/2,
				  -1,
				  GTK_ANCHOR_CENTER,
				  "font", gc_skin_font_board_big,
				  "fill_color_rgba", gc_skin_color_text_button,
				  NULL);

      g_signal_connect(item, "button_press_event",
		       (GCallback) item_event,
		       GINT_TO_POINTER(i));

      yp +=  button_pixmap_height + vertical_separation;

      i++;
    }

}
/* ==================================== */
static void game_won()
{
  gcomprisBoard->sublevel++;
  if(gcomprisBoard->sublevel > gcomprisBoard->number_of_sublevel)
    {
      /* Try the next level */
      gcomprisBoard->sublevel=1;
      gcomprisBoard->level++;
      if(gcomprisBoard->level>gcomprisBoard->maxlevel)
	gcomprisBoard->level = gcomprisBoard->maxlevel;

      imageid_next_level();
    }
  else
    imageid_next_sublevel();
}

/* ==================================== */
static gboolean
process_ok_timeout()
{
  gc_bonus_display(gamewon, GC_BONUS_SMILEY);
  return FALSE;
}

static void process_ok() {
  // leave time to display the right answer
  g_timeout_add(TIME_CLICK_TO_BONUS, process_ok_timeout, NULL);
}
/* ==================================== */
static gboolean
item_event (GooCanvasItem  *item,
	    GooCanvasItem  *target,
	    GdkEventButton *event,
	    gpointer data)
{
  gint button_id = GPOINTER_TO_INT(data);
  GooCanvasItem *button;

  if(board_paused)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      board_paused = TRUE;

      if ( button_id == right_word )
	gamewon = TRUE;
      else
	gamewon = FALSE;

      button = (GooCanvasItem*)g_object_get_data(G_OBJECT(goo_canvas_item_get_parent(item)),
						   "button");
      highlight_selected(button);
      process_ok();
      break;

    default:
      break;
    }
  return FALSE;
}

/* ==================================== */
static void
highlight_selected(GooCanvasItem * button)
{
  if (selected_button != NULL && selected_button != button)
    {
      g_object_set(selected_button,
		   "svg-id", "#BUTTON_TEXT",
		   NULL);
    }

  if (selected_button != button)
    {
      g_object_set(button,
		   "svg-id", "#BUTTON_TEXT_SELECTED",
		   NULL);
      selected_button = button;
    }

}


/* ===================================
 *                XML stuff
 *                Ref : shapegame.c
 * ==================================== */

static void
init_xml(guint level)
{
  gchar *filename;

  filename = gc_file_find_absolute("%s/board%d.xml",
				   gcomprisBoard->boarddir,
				   level);

  g_assert(read_xml_file(filename)== TRUE);
  g_free(filename);
}

/* ==================================== */
static void
add_xml_data(xmlDocPtr doc, xmlNodePtr xmlnode, GNode *child)
{
  Board * board = g_new0(Board, 1);
  guint text_index = 0;

  xmlnode = xmlnode->xmlChildrenNode;

  while( (xmlnode = xmlnode->next) != NULL)
    {
      if (!strcmp((char *)xmlnode->name, "pixmapfile"))
	board->pixmapfile =						\
	  (gchar *)xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);

      if (!strcmp((char *)xmlnode->name, "good"))
	{
	  board->solution = text_index;
	  board->text[text_index++] =					\
	    g_strdup(gettext((gchar *)xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1)));
	}

      if (!strcmp((char *)xmlnode->name, "bad"))
	board->text[text_index++] =					\
	  g_strdup(gettext((gchar *)xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1)));

      if(text_index > MAX_PROPOSAL)
	{
	  gc_dialog(_("Data file for this level is not properly formatted. Too many choices are proposed."),
		    gc_board_stop);
	  g_free(board);
	  return;
	}
    }

  /* Check there is enough information to play it */
  if ( board->pixmapfile == NULL
       || board->text[0] == NULL
       || board->text[1] == NULL )
    {
      gc_dialog(_("Data file for this level is not properly formatted."),
		gc_board_stop);
      g_free(board);
      return;
    }

  /* Randomize the set */
  {
    guint c = text_index * 2;
    gchar *text;

    while(c--)
      {
	guint i1 = g_random_int_range(0, text_index);
	guint i2 = g_random_int_range(0, text_index);

	/* Swap entries */
	text = board->text[i1];
	board->text[i1] = board->text[i2];
	board->text[i2] = text;

	if(i1 == board->solution)
	  board->solution = i2;
	else if(i2 == board->solution)
	  board->solution = i1;
      }
  }

  /* Insert boards randomly in the list */
  if(g_random_int_range(0, 2))
    board_list = g_list_append (board_list, board);
  else
    board_list = g_list_prepend (board_list, board);
}

/* ==================================== */
static void
parse_doc(xmlDocPtr doc)
{
  xmlNodePtr node;

  for(node = doc->children->children; node != NULL; node = node->next) {
    if ( g_ascii_strcasecmp((gchar *)node->name, "Board") == 0 )
      add_xml_data(doc, node, NULL);
  }

}
/* ==================================== */
/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set */
static gboolean
read_xml_file(char *fname)
{
  /* pointer to the new doc */
  xmlDocPtr doc;

  g_return_val_if_fail(fname!=NULL, FALSE);

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
     g_ascii_strcasecmp((char *)doc->children->name,"ImageId")!=0) {
    xmlFreeDoc(doc);
    return FALSE;
  }

  parse_doc(doc);
  xmlFreeDoc(doc);
  return TRUE;
}

/* ======================================= */
static void
destroy_board_list()
{
  Board *board;

  while(g_list_length(board_list)>0)
    {
      board = g_list_nth_data(board_list, 0);
      board_list = g_list_remove (board_list, board);
      destroy_board(board);
    }
}

/* ======================================= */
static void
destroy_board(Board * board)
{
  guint i = 0;

  g_free(board->pixmapfile);
  while(board->text[i])
    g_free(board->text[i++]);

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

/* GHFunc */
static void save_table (gpointer key,
			gpointer value,
			gpointer user_data)
{
  gc_db_set_board_conf ( profile_conf,
			    board_conf,
			    (gchar *) key,
			    (gchar *) value);
}

static gboolean conf_ok(GHashTable *table)
{
  if (!table){
    if (gcomprisBoard)
      pause_board(FALSE);
    return TRUE;
  }

  g_hash_table_foreach(table, (GHFunc) save_table, NULL);

  if (gcomprisBoard) {
    GHashTable *config;

    if (profile_conf)
      config = gc_db_get_board_conf();
    else
      config = table;

    gc_locale_set(g_hash_table_lookup( config, "locale"));

    if (profile_conf)
      g_hash_table_destroy(config);

    destroy_board_list();
    init_xml(gcomprisBoard->level);

    imageid_next_level();

    pause_board(FALSE);
  }

  board_conf = NULL;
  profile_conf = NULL;

  return TRUE;
}

static void
config_start(GcomprisBoard *agcomprisBoard,
	     GcomprisProfile *aProfile)
{
  board_conf = agcomprisBoard;
  profile_conf = aProfile;

  if (gcomprisBoard)
    pause_board(TRUE);

  gchar *label = g_strdup_printf(_("<b>%s</b> configuration\n for profile <b>%s</b>"),
				 agcomprisBoard->name,
				 aProfile ? aProfile->name : "");
  GcomprisBoardConf *bconf;
  bconf = gc_board_config_window_display( label,
				 conf_ok);

  g_free(label);

  /* init the combo to previously saved value */
  GHashTable *config = gc_db_get_conf( profile_conf, board_conf);

  gchar *locale = g_hash_table_lookup( config, "locale");

  gc_board_config_combo_locales(bconf, locale);

}


/* ======================= */
/* = config_stop        = */
/* ======================= */
static void
config_stop()
{
}

