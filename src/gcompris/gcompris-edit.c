/* gcompris - gcompris-edit.c
 *
 * Time-stamp: <2002/06/04 21:40:44 bruno>
 *
 * Copyright (C) 2000,2001 Bruno Coudoin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "interface.h"
#include "support.h"

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "gcompris.h"
#include "gcompris-edit.h"

#include "on.xpm"
#include "off.xpm"

static GtkWidget	*gcompris_edit	= NULL;
static GtkWidget	*ctree1		= NULL;

static GdkPixmap	*pixmap_on;
static GdkPixmap	*pixmap_on_mask;
static GdkPixmap	*pixmap_on_transp;
static GdkPixmap	*pixmap_off;
static GdkPixmap	*pixmap_off_mask;
static GdkPixmap	*pixmap_off_transp;

static gboolean		 read_xml_file(GtkCTreeNode *parentNode, char *fname);

static GcomprisBoard	*selectedBoard = NULL;
static GtkCTreeNode	*selectedNode = NULL;

void gcompris_edit_display_iconlist()
{
  GtkWidget	*windowWidget = NULL;
  GtkWidget	*tmpWidget = NULL;

  windowWidget = create_window_iconlist();

  gtk_widget_show (GTK_WINDOW(windowWidget));

  tmpWidget = gtk_object_get_data (GTK_OBJECT (windowWidget),
				   "iconselection");
  gnome_icon_selection_add_directory (GNOME_ICON_SELECTION(tmpWidget), 
				      PACKAGE_DATA_DIR "/boardicons");
  gnome_icon_selection_show_icons (GNOME_ICON_SELECTION(tmpWidget));
}

/*
 * Redisplay the icon representing the status of the board
 */
void gcompris_ctree_update_status(GcomprisBoard *gcomprisBoard, 
				  GtkCTreeNode *node)
{
  if(gcompris_properties_get_board_status(gcomprisBoard->name))
    gcompris_ctree_set_board_status(gcomprisBoard, node, TRUE);
  else
    gcompris_ctree_set_board_status(gcomprisBoard, node, FALSE);
}

/*
 * Set the icon in the tree depending on the given status
 */
void gcompris_ctree_set_board_status(GcomprisBoard *gcomprisBoard,
				     GtkCTreeNode *node,
				     gboolean status)
{
  if(status)
    gtk_ctree_node_set_pixtext(GTK_CTREE(ctree1), node, 0, 
			       gcomprisBoard->name, 8,
			       pixmap_on, pixmap_on_mask);
  else
    gtk_ctree_node_set_pixtext(GTK_CTREE(ctree1), node, 0, 
			       gcomprisBoard->name, 8,
			       pixmap_off, pixmap_off_mask);

}

/*
 * Update the description based on the given gcomprisBoard
 */
void gcompris_edit_display_description(GcomprisBoard *gcomprisBoard) 
{
  GtkWidget	*tmpWidget = NULL;
  gchar		*strTmp = NULL;

  printf("gcompris-edit-display-description for %s\n", gcomprisBoard->name);

  /* BoardIcon */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "pixmapBoardIcon");

  strTmp =g_strdup_printf("%s/%s", PACKAGE_DATA_DIR, gcomprisBoard->icon_name);
  gnome_pixmap_load_file(GNOME_PIXMAP(tmpWidget), strTmp);
  g_free(strTmp);

  /* Name */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "entryName");
      
  if(gcomprisBoard->name)
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), gcomprisBoard->name);
  else
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), "");


  /* Title */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "entryTitle");
  if(gcomprisBoard->title)
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), gcomprisBoard->title);
  else
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), "");


  /* Description */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "textDescription");
  
  gtk_text_backward_delete(GTK_TEXT(tmpWidget), gtk_text_get_length(GTK_TEXT(tmpWidget)));

  if(gcomprisBoard->description)
    gtk_text_insert(GTK_TEXT(tmpWidget), NULL, NULL, NULL, gcomprisBoard->description,
		    strlen(gcomprisBoard->description));

  /* Difficulty */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "spinbuttonDifficulty");
  if(gcomprisBoard->difficulty)
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(tmpWidget), atof(gcomprisBoard->difficulty));
  else
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(tmpWidget), (double)0);

  /* Author */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "entryAuthor");
  if(gcomprisBoard->author)
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), gcomprisBoard->author);
  else
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), "");

  /* BoardDir */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "entryBoardDir");
      
  if(gcomprisBoard->boarddir)
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), gcomprisBoard->boarddir);
  else
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), "");

  /* Prerequisite */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "textPrerequisite");
      
  gtk_text_backward_delete(GTK_TEXT(tmpWidget), gtk_text_get_length(GTK_TEXT(tmpWidget)));

  if(gcomprisBoard->prerequisite != NULL)
    {
      gtk_text_insert(GTK_TEXT(tmpWidget), NULL, NULL, NULL, gcomprisBoard->prerequisite,
		      strlen(gcomprisBoard->prerequisite));
    }

  /* Goal */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "textGoal");
  
  gtk_text_backward_delete(GTK_TEXT(tmpWidget), gtk_text_get_length(GTK_TEXT(tmpWidget)));
  
  if(gcomprisBoard->goal != NULL)
    {
      gtk_text_insert(GTK_TEXT(tmpWidget), NULL, NULL, NULL, gcomprisBoard->goal,
		      strlen(gcomprisBoard->goal));
    }

  /* Manual */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "textManual");
  
  gtk_text_backward_delete(GTK_TEXT(tmpWidget), gtk_text_get_length(GTK_TEXT(tmpWidget)));
  
  if(gcomprisBoard->manual != NULL)
    {
      gtk_text_insert(GTK_TEXT(tmpWidget), NULL, NULL, NULL, gcomprisBoard->manual,
		      strlen(gcomprisBoard->manual));
    }

  /* Credit */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "textCredit");
  
  gtk_text_backward_delete(GTK_TEXT(tmpWidget), gtk_text_get_length(GTK_TEXT(tmpWidget)));
  
  if(gcomprisBoard->credit != NULL)
    {
      gtk_text_insert(GTK_TEXT(tmpWidget), NULL, NULL, NULL, gcomprisBoard->credit,
		      strlen(gcomprisBoard->credit));
    }

}


GtkCTreeNode *add_node_to_ctree ( GtkWidget *ctree1,
				  GtkCTreeNode * gn, GcomprisBoard *gcomprisBoardMenu)
{
  GtkCTreeNode	       *newNode = NULL;
  gchar		       *text[3];

  printf("add_node_to_ctree %s\n", gcomprisBoardMenu->name);

  /* Create the text */
  text[0] = gcomprisBoardMenu->name;

  text[1] = gcomprisBoardMenu->title;

  if(gcomprisBoardMenu->difficulty)
    text[2] = gcomprisBoardMenu->difficulty;
  else
    text[2] = "";

  newNode = gtk_ctree_insert_node(GTK_CTREE(ctree1),
				  gn,
				  NULL,
				  text,
				  2,
				  NULL,
				  NULL,
				  NULL,
				  NULL,
				  FALSE,
				  TRUE);

  /* Associaste our data to the node */
  gtk_ctree_node_set_row_data(GTK_CTREE(ctree1), newNode, gcomprisBoardMenu);
  
  gcompris_ctree_update_status(gcomprisBoardMenu, newNode);  
  
  return(newNode);
}

void init_tree()
{
  /* Get our ctree */
  ctree1 = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				"ctree1");

  /* Init pixmaps */
  pixmap_on    = gdk_pixmap_create_from_xpm_d(gcompris_edit->window,
					      &pixmap_on_mask,
					      &pixmap_on_transp,
					      BOARD_ON_XPM);
  pixmap_off   = gdk_pixmap_create_from_xpm_d(gcompris_edit->window,
					      &pixmap_off_mask,
					      &pixmap_off_transp,
					      BOARD_OFF_XPM);

  read_xml_file(NULL, PACKAGE_DATA_DIR INITIAL_MENU);
}


/*
 * Thanks for George Lebl <jirka@5z.com> for his Genealogy example
 * for all the XML stuff there
 */

static void
add_menu(GtkCTreeNode *parentNode, xmlNodePtr xmlnode, GNode * child)
{
  char *filename;
  GcomprisBoard		*gcomprisBoardMenu = NULL;
  GtkCTreeNode		*newNode = NULL;

  if(/* if the node has no name */
     !xmlnode->name ||
     /* or if the name is not "Data" */
     (g_strcasecmp(xmlnode->name,"Data")!=0)
     )
    return;
  
  /* get the filename of this data */
  filename = xmlGetProp(xmlnode,"filename");

  gcomprisBoardMenu = gcompris_read_xml_file(filename);
  if(!board_check_file(gcomprisBoardMenu))
    g_error("Cant't find the menu board or plugin execution error");

  newNode = add_node_to_ctree(ctree1, parentNode, gcomprisBoardMenu);
  
  /* Recursively add sub menu */
  if(g_strcasecmp(gcomprisBoardMenu->type, "menu")==0)
    {
      read_xml_file(newNode, filename);
    }
}

/* parse the doc, add it to our internal structures and to the clist */
static void
parse_doc(GtkCTreeNode *parentNode, xmlDocPtr doc)
{
  xmlNodePtr node;
  
  /* find <Shape> nodes and add them to the list, this just
     loops through all the children of the root of the document */
  for(node = doc->children->children; node != NULL; node = node->next) {
    /* add the shape to the list, there are no children so
       we pass NULL as the node of the child */
    add_menu(parentNode, node, NULL);
  }
}



/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set */
static gboolean
read_xml_file(GtkCTreeNode *parentNode, char *fname)
{
  char *filename;

  /* pointer to the new doc */
  xmlDocPtr doc;

  g_return_val_if_fail(fname!=NULL,FALSE);

  filename = g_strdup(fname);

  /* if the file doesn't exist */
  if(!g_file_exists(filename))
    {
      g_free(filename);

      /* if the file doesn't exist, try with our default prefix */
      filename = g_strdup_printf("%s/%s",
				 PACKAGE_DATA_DIR, fname);

      if(!g_file_exists(filename))
	{
	  g_warning(_("Couldn't find file %s !"), fname);
	  g_warning(_("Couldn't find file %s !"), filename);
	  g_free(filename);
	  return FALSE;
	}

    }


  /* parse the new file and put the result into newdoc */
  doc = xmlParseFile(filename);

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
  parse_doc(parentNode, doc);
  
  xmlFreeDoc(doc);
  
}


/*
 * Manage the tree selection : add the selection
 */
void gcompris_ctree_selection_add(GcomprisBoard *gcomprisBoard, GtkCTreeNode *node)
{
  selectedBoard = gcomprisBoard;
  selectedNode  = node;
}

/*
 * Manage the tree selection : del the selection
 */
void gcompris_ctree_selection_del(GcomprisBoard *gcomprisBoard, GtkCTreeNode *node)
{
  selectedBoard = NULL;
  selectedNode  = NULL;
}

/*
 * Manage the tree selection : return it
 */
GcomprisBoard *gcompris_ctree_get_selected_board()
{
  return (selectedBoard);
}

/*
 * Manage the tree selection : return it
 */
GtkCTreeNode *gcompris_ctree_get_selected_node()
{
  return (selectedNode);
}



int
main (int argc, char *argv[])
{

#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif

  gnome_init (PACKAGE, VERSION, argc, argv);

  /*
   * The following code was added by Glade to create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */
  gcompris_edit = create_gcompris_edit ();
  gtk_widget_show (gcompris_edit);

  /* connect exit code */
  gtk_signal_connect (GTK_OBJECT (gcompris_edit), "delete_event",
		      GTK_SIGNAL_FUNC (gtk_exit), NULL);
  gtk_signal_connect (GTK_OBJECT (gcompris_edit), "destroy",
		      GTK_SIGNAL_FUNC (gtk_exit), NULL);

  init_plugins();

  init_tree();

  gtk_main ();

  return 0;
}

