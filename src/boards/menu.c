/* gcompris - menu.c
 *
 * Time-stamp: <2002/01/13 17:57:41 bruno>
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

static GList *item_list = NULL;

static GcomprisBoard *gcomprisBoard = NULL;

/* Hash table of all displayed images  */
static GHashTable *menu_table= NULL;

typedef struct {
  GcomprisBoard *board;
  GnomeCanvasItem *item;
  GdkPixbuf *image;
} MenuItem;

static void	 menu_start (GcomprisBoard *agcomprisBoard);
static void	 menu_pause (gboolean pause);
static void	 menu_end ();
static gboolean	 menu_is_our_board (GcomprisBoard *gcomprisBoard);
static void	 menu_config(void);

static GnomeCanvasItem *menu_create_item(GnomeCanvasGroup *parent, GcomprisBoard *board);
static gboolean next_spot();
static void create_info_area(GnomeCanvasGroup *parent);
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, MenuItem *menuitem);
static void display_board_icon(GcomprisBoard *board);
static gboolean read_xml_file(char *fname);

static double current_x = 0.0;
static double current_y = 0.0;

static GList		*boardlist;	/* List of Board */

static GnomeCanvasGroup *boardRootItem = NULL;

/* Information items */
static GnomeCanvasItem *boardname_item, *description_item, *author_item;

/* Description of this plugin */
BoardPlugin menu_bp =
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


BoardPlugin
*get_bplugin_info(void)
{
  return &menu_bp;
}

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void menu_pause (gboolean pause)
{

  if(gcomprisBoard==NULL)
    return;

}

/*
 */
static void menu_start (GcomprisBoard *agcomprisBoard)
{

  current_x = 0.0;
  current_y = 0.0;

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "gcompris/gcompris-init.jpg");

      read_xml_file(gcomprisBoard->filename);

      boardRootItem = GNOME_CANVAS_GROUP(
					 gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
								gnome_canvas_group_get_type (),
								"x", (double) 0,
								"y", (double) 0,
								NULL));

      g_list_foreach (boardlist, (GFunc) display_board_icon, NULL);

      create_info_area(boardRootItem);

      /* set initial values for this level */
      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel=1;
      gcompris_bar_set(GCOMPRIS_BAR_CONFIG|GCOMPRIS_BAR_ABOUT);

      menu_pause(FALSE);

    }

}

static void
menu_end ()
{
  GcomprisBoard *board;

  /* Erase the boardlist */
  while(g_list_length(boardlist)>0)
    {
      board = g_list_nth_data(boardlist, 0);
      boardlist = g_list_remove (boardlist, board);

      /* FIXME : We need a better cleanup */
      g_free(board->name);
      g_free(board->description);
      g_free(board->icon_name);
      g_free(board->author);
      g_free(board->boarddir);
      g_free(board->filename);
      g_free(board);
    }

  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem=NULL;
}

gboolean
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

static void display_board_icon(GcomprisBoard *board)
{
  if (board!=NULL)
    {
      menu_create_item(boardRootItem, board);
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
      current_y = gcomprisBoard->height/MENU_PER_LINE;
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

static GnomeCanvasItem *menu_create_item(GnomeCanvasGroup *parent, GcomprisBoard *board)
{
  GdkPixbuf *menu_pixmap = NULL, *pixmap = NULL;
  GnomeCanvasItem *item, *star;
  MenuItem *menuitem;

  menuitem = malloc(sizeof(MenuItem));
  menu_pixmap = gcompris_load_pixmap(board->icon_name);
  next_spot();

  item = gnome_canvas_item_new (parent,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", menu_pixmap,
				"x", (double)current_x - gdk_pixbuf_get_width(menu_pixmap)/2,
				"y", (double)current_y - gdk_pixbuf_get_height(menu_pixmap)/2,
				"width", (double) gdk_pixbuf_get_width(menu_pixmap),
				"height", (double) gdk_pixbuf_get_height(menu_pixmap),
				NULL);
  gdk_pixbuf_unref(menu_pixmap);

  item_list = g_list_append (item_list, item);

  menuitem->board=board;
  menuitem->item=item;
  menuitem->image=menu_pixmap;

  if (!menu_table)
    {
      menu_table= g_hash_table_new (g_direct_hash, g_direct_equal);
    }

  g_hash_table_insert (menu_table, item, menuitem);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event,
		     menuitem);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);



  // display difficulty stars ========================== BEGIN
  pixmap = gcompris_load_pixmap("gcompris/buttons/difficulty_star.png");
  if (board->difficulty != NULL) {
  	int i, diff = 0;
	diff = atoi(board->difficulty);
	for (i=0; i<diff; i++) {
		star =  gnome_canvas_item_new (parent,
					       gnome_canvas_pixbuf_get_type (),
					       "pixbuf", pixmap,
					       "x", (double)current_x - gdk_pixbuf_get_width(menu_pixmap)/2
					       - gdk_pixbuf_get_width(pixmap) + 5,
					       "y", (double)current_y - gdk_pixbuf_get_height(menu_pixmap)/2
					       + gdk_pixbuf_get_height(pixmap) * (i-1),
					       "width", (double) gdk_pixbuf_get_width(pixmap),
					       "height", (double) gdk_pixbuf_get_height(pixmap),
					       NULL);
		item_list = g_list_append (item_list, star);
	}
  }
  gdk_pixbuf_unref(pixmap);
  // display difficulty stars ========================== END
  
  // display menu icon ========================== BEGIN
  pixmap = gcompris_load_pixmap("gcompris/buttons/menuicon.png");
  if(g_strcasecmp(board->type, "menu")==0)
    {
      item =  gnome_canvas_item_new (parent,
				     gnome_canvas_pixbuf_get_type (),
				     "pixbuf", pixmap,
				     "x", (double)current_x + gdk_pixbuf_get_width(menu_pixmap)/2
				     - gdk_pixbuf_get_width(pixmap) - 5,
				     "y", (double)current_y + gdk_pixbuf_get_height(menu_pixmap)/2,
				     "width", (double) gdk_pixbuf_get_width(pixmap),
				     "height", (double) gdk_pixbuf_get_height(pixmap),
				     NULL);
      item_list = g_list_append (item_list, item);
    }
  gdk_pixbuf_unref(pixmap);
// display menu icon ========================== END

  return (item);
}

static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, MenuItem *menuitem)
{

  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
      /* HACK : If I don't set the color here, then the 3 text are not visible !!!
       *        just add again white here and it works again !!!! */
      gnome_canvas_item_set (boardname_item,
			     "text", menuitem->board->name,
			     "fill_color", "white",
			     NULL);
      gnome_canvas_item_set (description_item,
			     "text",  menuitem->board->description,
			     NULL);
      gnome_canvas_item_set (author_item,
			     "text",  menuitem->board->author,
			     NULL);

      break;
    case GDK_LEAVE_NOTIFY:
      gnome_canvas_item_set (boardname_item,
			     "text", " ",
			     NULL);

      gnome_canvas_item_set (description_item,
			     "text",  " ",
			     NULL);

      gnome_canvas_item_set (author_item,
			     "text",  " ",
			     NULL);

      break;
    case GDK_BUTTON_PRESS:
      gcompris_play_sound (SOUNDLISTFILE, "gobble");
	  
      if(!board_check_file(menuitem->board))
	{
	  g_warning("Can't find the board %s", menuitem->board->filename);
	  return FALSE;
	}

      /* Take care to not remove the next board */
      boardlist = g_list_remove (boardlist, menuitem->board);

      /* End this board */
      menu_end();

      /* Start the user's one */
      menuitem->board->previous_board = gcomprisBoard;
      board_play (menuitem->board);
      break;
      
    default:
      break;
    }

  return FALSE;
}

static void create_info_area(GnomeCanvasGroup *parent)
{
  GdkFont *gdk_font, *gdk_font_small, *gdk_font_big;
  gint x = (double)gcomprisBoard->width/2;
  gint y = 350;

  /* Load a gdk font */
  gdk_font = gdk_font_load (FONT_BOARD_MEDIUM);
  gdk_font_small = gdk_font_load (FONT_BOARD_TINY);
  gdk_font_big = gdk_font_load (FONT_BOARD_BIG);

  boardname_item = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_text_get_type (),
			   "text", "",
			   "font_gdk", gdk_font_big,
			   "x", (double) x,
			   "y", (double) y,
			   "anchor", GTK_ANCHOR_CENTER,
			   "fill_color", "white",
			   NULL);

  description_item = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_text_get_type (),
			   "text", "",
			   "font_gdk", gdk_font_small,
			   "x", (double) x,
			   "y", (double) y + 35,
			   "anchor", GTK_ANCHOR_NORTH,
			   "fill_color", "white",
			   "justification", GTK_JUSTIFY_CENTER,
			   NULL);

  author_item = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_text_get_type (),
			   "text", "",
			   "font_gdk", gdk_font_small,
			   "x", (double) x,
			   "y", (double) y + 80,
			   "anchor", GTK_ANCHOR_NORTH,
			   "fill_color", "white",
			   "justification", GTK_JUSTIFY_CENTER,
			   NULL);
}


/*
 * Thanks for George Lebl <jirka@5z.com> for his Genealogy example
 * for all the XML stuff there
 */

static void
add_menu(xmlNodePtr xmlnode, GNode * child)
{
  char *filename;

  if(/* if the node has no name */
     !xmlnode->name ||
     /* or if the name is not "Data" */
     (g_strcasecmp(xmlnode->name,"Data")!=0)
     )
    return;
  
  /* get the filename of this data */
  filename = xmlGetProp(xmlnode,"filename");

  boardlist = g_list_append (boardlist, gcompris_read_xml_file(filename));
  
}

/* parse the doc, add it to our internal structures and to the clist */
static void
parse_doc(xmlDocPtr doc)
{
  xmlNodePtr node;
  
  /* find <Shape> nodes and add them to the list, this just
     loops through all the children of the root of the document */
  for(node = doc->children->children; node != NULL; node = node->next) {
    /* add the shape to the list, there are no children so
       we pass NULL as the node of the child */
    add_menu(node,NULL);
  }
}



/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set */
static gboolean
read_xml_file(char *fname)
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

  /* parse the new file and put the result into newdoc */
  doc = xmlParseFile(fname);

  /* in case something went wrong */
  if(!doc)
    return FALSE;
  
  if(/* if there is no root element */
     !doc->children ||
     /* if it doesn't have a name */
     !doc->children->name ||
     /* if it isn't a GCompris node */
     g_strcasecmp(doc->children->name,"GCompris")!=0) {
    xmlFreeDoc(doc);
    return FALSE;
  }
  
  /* parse our document and replace old data */
  parse_doc(doc);
  
  xmlFreeDoc(doc);
  
  return TRUE;
}




/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
