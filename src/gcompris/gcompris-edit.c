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

static GtkWidget	*gcompris_edit	= NULL;
static GtkWidget	*treeview	= NULL;
static GtkTreeModel	*model		= NULL;

static void quit_cb (GtkWidget *widget, gpointer data);
static gboolean		 read_xml_file(GtkTreeIter *parentNode, char *fname);

/* Prototype for selection handler callback */
static void		tree_selection_changed_cb (GtkTreeSelection *selection, 
						   gpointer data);


/* columns */
enum
{
  VISIBLE_COLUMN = 0,
  NAME_COLUMN,
  DESCRIPTION_COLUMN,

  BOARD_POINTER_COLUMN,
  NUM_COLUMNS
};

static void quit_cb (GtkWidget *widget, gpointer data)
{
  gcompris_write_boards_status();
  gtk_main_quit ();
}

/*
 * Update the description based on the given gcomprisBoard
 */
void gcompris_edit_display_description(GcomprisBoard *gcomprisBoard) 
{
  GtkWidget	*tmpWidget = NULL;
  gchar		*strTmp = NULL;
  GtkTextBuffer *buffer;

  printf("gcompris-edit-display-description for %s\n", gcomprisBoard->name);

  /* BoardIcon */

  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "iconentry");

  strTmp =g_strdup_printf("%s/%s", PACKAGE_DATA_DIR, gcomprisBoard->icon_name);
  gnome_icon_entry_set_filename(GNOME_ICON_ENTRY(tmpWidget), strTmp);
  g_free(strTmp);


  /* Name */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "entry_name");
      
  if(gcomprisBoard->name)
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), gcomprisBoard->name);
  else
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), "");


  /* Title */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "entry_title");
  if(gcomprisBoard->title)
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), gcomprisBoard->title);
  else
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), "");


  /* Description */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "textview_description");
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tmpWidget));
  gtk_text_buffer_set_text (buffer, gcomprisBoard->description, -1);


  /* Difficulty */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "spinbutton_difficulty");
  if(gcomprisBoard->difficulty)
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(tmpWidget), atof(gcomprisBoard->difficulty));
  else
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(tmpWidget), (double)0);

  /* Author */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "entry_author");
  if(gcomprisBoard->author)
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), gcomprisBoard->author);
  else
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), "");

  /* Type */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "entry_type");
  if(gcomprisBoard->type)
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), gcomprisBoard->type);
  else
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), "");

  /* BoardDir */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "entry_directory");
      
  if(gcomprisBoard->boarddir)
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), gcomprisBoard->boarddir);
  else
    gtk_entry_set_text(GTK_ENTRY(tmpWidget), "");

  /* Prerequisite */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "textview_prerequisite");
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tmpWidget));
  gtk_text_buffer_set_text (buffer, gcomprisBoard->prerequisite, -1);


  /* Goal */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "textview_goal");
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tmpWidget));
  gtk_text_buffer_set_text (buffer, gcomprisBoard->goal, -1);


  /* Manual */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "textview_manual");
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tmpWidget));
  gtk_text_buffer_set_text (buffer, gcomprisBoard->manual, -1);
  

  /* Credit */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "textview_credit");
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tmpWidget));
  gtk_text_buffer_set_text (buffer, gcomprisBoard->credit, -1);
  
}

static void
tree_selection_changed_cb (GtkTreeSelection *selection, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  GcomprisBoard *board;
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, BOARD_POINTER_COLUMN, &board, -1);
      
      gcompris_edit_display_description(board);

    }
}


static void
item_toggled (GtkCellRendererToggle *cell,
	      gchar                 *path_str,
	      gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  GtkTreeIter iter;
  gboolean toggle_item;
  GcomprisBoard *board;

  gint *column;

  column       = g_object_get_data (G_OBJECT (cell), "column");

  /* get toggled item */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, column, &toggle_item, -1);

  /* Get the actual board */
  gtk_tree_model_get (model, &iter, BOARD_POINTER_COLUMN, &board, -1);

  if(toggle_item)
    gcompris_properties_disable_board(board->name);
  else
    gcompris_properties_enable_board(board->name);

  /* set new value */
  toggle_item ^= 1;
  gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,
		      toggle_item, -1);

  /* clean up */
  gtk_tree_path_free (path);
}

static void
add_columns (GtkTreeView *treeview)
{
  gint col_offset;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeModel *model = gtk_tree_view_get_model (treeview);

  /* active column */
  renderer = gtk_cell_renderer_toggle_new ();
  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)VISIBLE_COLUMN);

  g_signal_connect (G_OBJECT (renderer), "toggled", G_CALLBACK (item_toggled),
		    model);
  col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
							    -1, "active",
							    renderer,
							    "active",
							    VISIBLE_COLUMN,
							    NULL);

  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
				   GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

  /* column for names */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
							    -1, "Name",
							    renderer, "text",
							    NAME_COLUMN,
							    NULL);
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

  /* column for description */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
							    -1, "Description",
							    renderer, "text",
							    DESCRIPTION_COLUMN,
							    NULL);
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

}

static GtkTreeModel *
create_model (void)
{
  GtkTreeStore *model;
  GtkTreeIter iter;

  /* create tree store */
  model = gtk_tree_store_new (NUM_COLUMNS,
			      G_TYPE_BOOLEAN,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_POINTER);


  return GTK_TREE_MODEL (model);
}


void init_tree()
{
  /* Setup the selection handler */
  GtkTreeSelection *select;

  /* Get our ctree */
  treeview = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				"treeview");

  model = create_model();

  /* create tree view */
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), model);
  g_object_unref (G_OBJECT (model));

  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
			       GTK_SELECTION_SINGLE);

  /* Setup the selection handler */
  select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  g_signal_connect (G_OBJECT (select), "changed",
		    G_CALLBACK (tree_selection_changed_cb),
		    NULL);

  add_columns (GTK_TREE_VIEW (treeview));

  /* expand all rows after the treeview widget has been realized */
  g_signal_connect (G_OBJECT (treeview), "realize",
		    G_CALLBACK (gtk_tree_view_expand_all), NULL);


  read_xml_file(NULL, "/");

}

static void
add_menu(GtkTreeIter *parentNode, GcomprisBoard *gcomprisBoardMenu, GNode * child)
{
  char *filename;
  GtkCTreeNode		*newNode = NULL;
  GtkTreeIter		 iter;

  printf("add_menu %s\n", gcomprisBoardMenu->name);
  gtk_tree_store_append ( GTK_TREE_STORE (model), &iter, parentNode);
  gtk_tree_store_set ( GTK_TREE_STORE (model), &iter,
		      VISIBLE_COLUMN, 
		      (gcompris_properties_get_board_status(gcomprisBoardMenu->name)? TRUE : FALSE),
		      NAME_COLUMN, gcomprisBoardMenu->name,
		      DESCRIPTION_COLUMN, gcomprisBoardMenu->description,
		      BOARD_POINTER_COLUMN, gcomprisBoardMenu,
		      -1);


  /* Recursively add sub menu */
  if(g_strcasecmp(gcomprisBoardMenu->type, "menu")==0)
    {
      printf("add_sub_menu %s\n", gcomprisBoardMenu->name);
      read_xml_file(&iter, gcomprisBoardMenu->section);
    }
}


/* parse the doc, add it to our internal structures and to the clist */
static void
parse_doc(GtkCTreeNode *parentNode, GList *boards_list)
{
  GList *list = NULL;

  printf("parse_doc\n");

  for(list = boards_list; list != NULL; list = list->next) {
    GcomprisBoard *board = list->data;

    printf("   parse_doc board=%s\n", board->name);
    add_menu( (GtkTreeIter *)parentNode, board, NULL);
  }
}



/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set */
static gboolean
read_xml_file(GtkTreeIter *parentNode, char *fname)
{

  printf("read_xml_file section=%s\n", fname);
  /* parse our document and replace old data */
  parse_doc( (GtkCTreeNode *)parentNode, gcompris_get_menulist(fname));

}

int
main (int argc, char *argv[])
{
  GtkWidget	*tmpWidget = NULL;

  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
                      argc, argv,
                      GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR,
                      NULL);

  /* Init the gcompris properties */
  gcompris_properties_new();

  /*
   * The following code was added by Glade to create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */
  gcompris_edit = create_gcompris_edit ();
  gtk_widget_show (gcompris_edit);

  /* connect exit code */
  gtk_signal_connect (GTK_OBJECT (gcompris_edit), "delete_event",
		      GTK_SIGNAL_FUNC (quit_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (gcompris_edit), "destroy",
		      GTK_SIGNAL_FUNC (quit_cb), NULL);

  init_plugins();

  /* Set the directory for the pixmaps in the description */
  tmpWidget = gtk_object_get_data (GTK_OBJECT (gcompris_edit),
				   "iconentry");
  gnome_icon_entry_set_pixmap_subdir( (GnomeIconEntry *)tmpWidget, PACKAGE_DATA_DIR"/boardicons");

  /* Load all the menu once */
  gcompris_load_menus();

  init_tree();

  gtk_main ();
  return 0;
}

