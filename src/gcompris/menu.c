/* gcompris - menu.c
 *
 * Time-stamp: <2006/08/20 10:45:56 bruno>
 *
 * Copyright (C) 2000-2006 Bruno Coudoin
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

#include <math.h>
#include <string.h>
#include <time.h>

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>

#include "gcompris.h"

GcomprisBoard	*_read_xml_file(GcomprisBoard *gcomprisBoard, char *fname, gboolean db);

/* List of all available boards  */
static GList *boards_list = NULL;

void gc_menu_board_free(GcomprisBoard *board);

GList *gc_menu_get_boards()
{
  return boards_list;
}

/*
 * Thanks for George Lebl <jirka@5z.com> for his Genealogy example
 * for all the XML stuff there
 */

static void
_add_xml_to_data(xmlDocPtr doc, xmlNodePtr xmlnode, GNode * child,
			 GcomprisBoard *gcomprisBoard, gboolean db)
{
  GcomprisProperties *properties = gc_prop_get();
  gchar *title=NULL;
  gchar *description=NULL;
  gchar *prerequisite=NULL;
  gchar *goal=NULL;
  gchar *manual=NULL;
  gchar *credit=NULL;

  if(/* if the node has no name */
     !xmlnode->name ||
     /* or if the name is not "Board" */
     (g_strcasecmp((char *)xmlnode->name,"Board")!=0)
     )
    return;

  /* get the type of the board */
  gcomprisBoard->type = (char *)xmlGetProp(xmlnode, BAD_CAST "type");

  /* get the specific mode for this board */
  gcomprisBoard->mode			 = (char *)xmlGetProp(xmlnode, BAD_CAST "mode");
  gcomprisBoard->name			 = (char *)xmlGetProp(xmlnode, BAD_CAST "name");
  gcomprisBoard->icon_name		 = (char *)xmlGetProp(xmlnode, BAD_CAST "icon");
  gcomprisBoard->author			 = (char *)xmlGetProp(xmlnode, BAD_CAST "author");
  gcomprisBoard->boarddir		 = (char *)xmlGetProp(xmlnode, BAD_CAST "boarddir");
  gcomprisBoard->mandatory_sound_file	 = (char *)xmlGetProp(xmlnode, BAD_CAST "mandatory_sound_file");
  gcomprisBoard->mandatory_sound_dataset = (char *)xmlGetProp(xmlnode, BAD_CAST "mandatory_sound_dataset");

  gchar *path                            = (char *)xmlGetProp(xmlnode, BAD_CAST "section");
  if (strlen(path)==1){
    g_free(path);
    path = g_strdup("");
    if (strcmp(gcomprisBoard->name,"root")==0)
    {
      g_free(gcomprisBoard->name);
      gcomprisBoard->name = g_strdup("");
    }
  }

  gcomprisBoard->section		 = path;

  gcomprisBoard->title = NULL;
  gcomprisBoard->description = NULL;
  gcomprisBoard->prerequisite = NULL;
  gcomprisBoard->goal = NULL;
  gcomprisBoard->manual = NULL;
  gcomprisBoard->credit = NULL;

  gcomprisBoard->difficulty		= (char *)xmlGetProp(xmlnode, BAD_CAST "difficulty");
  if(gcomprisBoard->difficulty == NULL)
    gcomprisBoard->difficulty		= g_strdup("0");

  /* Update the difficulty max */
  if(properties->difficulty_max < atoi(gcomprisBoard->difficulty))
    properties->difficulty_max = atoi(gcomprisBoard->difficulty);

  for (xmlnode = xmlnode->xmlChildrenNode; xmlnode != NULL; xmlnode = xmlnode->next) {
    if (xmlHasProp(xmlnode, BAD_CAST "lang"))
      continue;

    /* get the title of the board */
    if (!strcmp((char *)xmlnode->name, "title"))
      {
	title = (char *)xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 0);
	gcomprisBoard->title = reactivate_newline(gettext(title));
	g_free(title);
      }

    /* get the description of the board */
    if (!strcmp((char *)xmlnode->name, "description"))
      {
	description = (char *)xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 0);
	gcomprisBoard->description = reactivate_newline(gettext(description));
	g_free(description);
      }

    /* get the help prerequisite help of the board */
    if (!strcmp((char *)xmlnode->name, "prerequisite"))
      {
	if(gcomprisBoard->prerequisite)
	  g_free(gcomprisBoard->prerequisite);

	prerequisite = (char *)xmlNodeListGetString(doc,  xmlnode->xmlChildrenNode, 0);
	gcomprisBoard->prerequisite = reactivate_newline(gettext(prerequisite));
    g_free(prerequisite);
      }

    /* get the help goal of the board */
    if (!strcmp((char *)xmlnode->name, "goal"))
      {
	if(gcomprisBoard->goal)
	  g_free(gcomprisBoard->goal);

	goal = (char *)xmlNodeListGetString(doc,  xmlnode->xmlChildrenNode, 0);
	gcomprisBoard->goal = reactivate_newline(gettext(goal));
    g_free(goal);
      }

    /* get the help user manual of the board */
    if (!strcmp((char *)xmlnode->name, "manual"))
      {
	if(gcomprisBoard->manual)
	  g_free(gcomprisBoard->manual);

	manual = (char *)xmlNodeListGetString(doc,  xmlnode->xmlChildrenNode, 0);
	gcomprisBoard->manual = reactivate_newline(gettext(manual));
    g_free(manual);
      }

    /* get the help user credit of the board */
    if (!strcmp((char *)xmlnode->name, "credit"))
      {
	if(gcomprisBoard->credit)
	  g_free(gcomprisBoard->credit);

	credit =(char *) xmlNodeListGetString(doc,  xmlnode->xmlChildrenNode, 0);
	gcomprisBoard->credit = reactivate_newline(gettext(credit));
    g_free(credit);
      }

    /* Display the resource on stdout */
    if (properties->display_resource
	&& !strcmp((char *)xmlnode->name, "resource")
	&& gc_profile_get_current())
      {
	if(gc_db_is_activity_in_profile(gc_profile_get_current(), gcomprisBoard->name))
	  {
	    char *resource = (char *)xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 0);
	    printf("%s\n", resource);
	  }
      }

  }


  /* Check all mandatory field are specified */
  if(gcomprisBoard->name        == NULL ||
     gcomprisBoard->type        == NULL ||
     gcomprisBoard->icon_name   == NULL ||
     gcomprisBoard->section     == NULL ||
     gcomprisBoard->title       == NULL ||
     gcomprisBoard->description == NULL) {
    g_error("failed to read a mandatory field for this board (mandatory fields are name type icon_name difficulty section title description). check the board xml file is complete, perhaps xml-i18n-tools did not generate the file properly");
  }

  if (db){
    gc_db_board_update( &gcomprisBoard->board_id,
			&gcomprisBoard->section_id,
			gcomprisBoard->name,
			gcomprisBoard->section,
			gcomprisBoard->author,
			gcomprisBoard->type,
			gcomprisBoard->mode,
			atoi(gcomprisBoard->difficulty),
			gcomprisBoard->icon_name,
			gcomprisBoard->boarddir,
			gcomprisBoard->mandatory_sound_file,
			gcomprisBoard->mandatory_sound_dataset,
			gcomprisBoard->filename,
			gcomprisBoard->title,
			gcomprisBoard->description,
			gcomprisBoard->prerequisite,
			gcomprisBoard->goal,
			gcomprisBoard->manual,
			gcomprisBoard->credit
			);

    g_warning("db board written %d in %d  %s/%s",
	      gcomprisBoard->board_id, gcomprisBoard->section_id,
	      gcomprisBoard->section, gcomprisBoard->name);
  }

}

/* parse the doc, add it to our internal structures and to the clist */
static void
parse_doc(xmlDocPtr doc, GcomprisBoard *gcomprisBoard, gboolean db)
{
  xmlNodePtr node;

  /* find <Board> nodes and add them to the list, this just
     loops through all the children of the root of the document */
  for(node = doc->children->children; node != NULL; node = node->next) {
    /* add the board to the list, there are no children so
       we pass NULL as the node of the child */
    _add_xml_to_data(doc, node, NULL, gcomprisBoard, db);
  }
}



/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set
   Return a newly allocated GcomprisBoard or NULL if the parsing failed
*/
GcomprisBoard *
_read_xml_file(GcomprisBoard *gcomprisBoard,
	       char *fname,
	       gboolean db)
{
  GcomprisProperties *properties = gc_prop_get();
  gchar *filename;
  /* pointer to the new doc */
  xmlDocPtr doc;

  g_return_val_if_fail(fname!=NULL, NULL);

  filename = g_strdup(fname);

  /* if the file doesn't exist */
  if(!g_file_test ((filename), G_FILE_TEST_EXISTS))
    {
      g_free(filename);

      /* if the file doesn't exist, try with our default prefix */
      filename = g_strdup_printf("%s/%s", properties->package_data_dir, fname);

      if(!g_file_test ((filename), G_FILE_TEST_EXISTS))
	{
	  g_warning("Couldn't find file %s !", fname);
	  g_warning("Couldn't find file %s !", filename);
	  g_free(filename);
	  g_free(gcomprisBoard);
	  return NULL;
	}

    }

  /* parse the new file and put the result into newdoc */
  doc = xmlParseFile(filename);

  /* in case something went wrong */
  if(!doc) {
    g_warning("Oops, the parsing of %s failed", filename);
    return NULL;
  }

  if(/* if there is no root element */
     !doc->children ||
     /* if it doesn't have a name */
     !doc->children->name ||
     /* if it isn't a GCompris node */
     g_strcasecmp((char *)doc->children->name,"GCompris")!=0) {
    xmlFreeDoc(doc);
    g_free(gcomprisBoard);
    g_warning("Oops, the file %s is not for gcompris", filename);
    return NULL;
  }

  /* Store the file that belong to this board for trace and further need */
  gcomprisBoard->filename=filename;

  /* parse our document and replace old data */
  parse_doc(doc, gcomprisBoard, db);

  xmlFreeDoc(doc);

  gcomprisBoard->board_ready=FALSE;
  gcomprisBoard->canvas=gc_get_canvas();

  gcomprisBoard->gmodule      = NULL;
  gcomprisBoard->gmodule_file = NULL;

  /* Fixed since I use the canvas own pixel_per_unit scheme */
  gcomprisBoard->width  = BOARDWIDTH;
  gcomprisBoard->height = BOARDHEIGHT;

  return gcomprisBoard;
}

/* Return the first board with the given section
 */
GcomprisBoard *
gc_menu_section_get(gchar *section)
{
  GList *list = NULL;

  for(list = boards_list; list != NULL; list = list->next) {
    GcomprisBoard *board = list->data;

    gchar *fullname = NULL;

    fullname = g_strdup_printf("%s/%s",
			       board->section, board->name);

    if (strcmp (fullname, section) == 0){
      g_free(fullname);
      return board;
    }
    g_free(fullname);

  }
  g_warning("gc_menu_section_get searching '%s' but NOT FOUND\n", section);
  return NULL;
}

static int
boardlist_compare_func(const void *a, const void *b)
{
  return strcasecmp(((GcomprisBoard *) a)->difficulty, ((GcomprisBoard *) b)->difficulty);
}

/** Return true is there are at least one activity in the given section
 *
 * \param section: the section to check
 *
 * \return 1 if there is at least one activity, 0 instead
 *
 */
int
gc_menu_has_activity(gchar *section, gchar *name)
{
  GList *list = NULL;
  GcomprisProperties	*properties = gc_prop_get();
  gchar *section_name = g_strdup_printf("%s/%s", section, name);

  if (strlen(section)==1)
    return 1;

  for(list = boards_list; list != NULL; list = list->next) {
    GcomprisBoard *board = list->data;

    if ( (!properties->experimental) &&
	 (strcmp (board->name, "experimental") == 0))
      continue;

    if ((strcmp (section_name, board->section) == 0) &&
	(strlen(board->name) != 0) &&
	gc_board_check_file(board))
	{
	  if((strcmp(board->type, "menu") == 0) &&
	     strcmp(board->section, section) != 0)
	    {
	      /* We must check this menu is not empty recursively */
	      if(gc_menu_has_activity(board->section, board->name))
		{
		  g_free(section_name);
		  return(1);
		}
	    }
	  else
	    {
	      g_free(section_name);
	      return 1;
	    }
	}
    }

  g_free(section_name);
  return 0;
}

/* Return the list of boards in the given section
 * Boards are sorted depending on their difficulty value
 */
GList *gc_menu_getlist(gchar *section)
{
  GList *list = NULL;
  GList *result_list = NULL;

  GcomprisProperties	*properties = gc_prop_get();

  if(!section){
    g_error("gc_menu_getlist called with section == NULL !");
    return NULL;
  }

  if (strlen(section)==1)
    section = "";

  for(list = boards_list; list != NULL; list = list->next) {
    GcomprisBoard *board = list->data;

    if ( (!properties->experimental) &&
	 (strcmp (board->name, "experimental") == 0))
      continue;

    if (strcmp (section, board->section) == 0) {
      if (strlen(board->name) != 0)
	{
	  if(strcmp(board->type, "menu") == 0)
	    {
	      /* We must check first this menu is not empty */
	      if(gc_menu_has_activity(board->section, board->name))
		result_list = g_list_append(result_list, board);
	    }
	  else
	    {
	      result_list = g_list_append(result_list, board);
	    }
	}
    }
  }

  /* Sort the list now */
  result_list = g_list_sort(result_list, boardlist_compare_func);

  return result_list;
}

/** Select only files with .xml extention
 *
 * Return 1 if the given file end in .xml 0 else
 */
int
file_end_with_xml(const gchar *file)
{

  if(strlen(file)<4)
    return 0;

  return (strncmp (&file[strlen(file)-4], ".xml", 4) == 0);
}

static void cleanup_menus() {
  GList *list = NULL;

  for(list = boards_list; list != NULL; list = list->next) {
    GcomprisBoard *gcomprisBoard = list->data;

    _read_xml_file(gcomprisBoard, gcomprisBoard->filename, FALSE);
  }
}


/*
 * suppress_int_from_list
 */

static GList *suppress_int_from_list(GList *list, int value)
{

  GList *cell = list;
  int *data;

  while (cell != NULL){
    data = cell->data;
    if (*data==value){
      g_free(data);
      return g_list_remove(list, data);
    }
    cell = cell->next;
  }
  g_warning("suppress_int_from_list value %d not found", value);
  return list;
}

static gboolean compare_id(gconstpointer data1, gconstpointer data2)
{
  int *i = (int *) data1;
  int *j = (int *) data2;

  if (*i == *j)
    return 0;
  else
    return -1;
}

/*
 * gc_menu_load
 *
 * Load all the menu it can from the given dirname
 *
 */
void gc_menu_load_dir(char *dirname, gboolean db){
  const gchar   *one_dirent;
  GDir          *dir;
  GcomprisProperties	*properties = gc_prop_get();
  GList *list_old_boards_id = NULL;

  if (!g_file_test(dirname, G_FILE_TEST_IS_DIR)) {
    g_warning("Failed to parse board in '%s' because it's not a directory\n", dirname);
    return;
  }

  dir = g_dir_open(dirname, 0, NULL);

  if (!dir) {
    g_warning("gc_menu_load : no menu found in %s", dirname);
    return;
  } else {
    if (db)
      list_old_boards_id = gc_db_get_board_id(list_old_boards_id);

    while((one_dirent = g_dir_read_name(dir)) != NULL) {
      /* add the board to the list */
      GcomprisBoard *gcomprisBoard = NULL;
      gchar *filename;

      filename = g_strdup_printf("%s/%s",
				 dirname, one_dirent);

      if(!g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
	g_free(filename);
	continue;
      }

      if(file_end_with_xml(one_dirent)) {
	gcomprisBoard = g_malloc0 (sizeof (GcomprisBoard));
	gcomprisBoard->board_dir = g_strdup(dirname);

	/* Need to be initialized here because _read_xml_file is used also to reread 	*/
	/* the locale data									*/
	/* And we don't want in this case to loose the current plugin				*/
	gcomprisBoard->plugin=NULL;
	gcomprisBoard->previous_board=NULL;

	GcomprisBoard *board_read = _read_xml_file(gcomprisBoard, filename, db);
	if (board_read){
	  list_old_boards_id = suppress_int_from_list(list_old_boards_id, board_read->board_id);
	  if (properties->administration)
	    boards_list = g_list_append(boards_list, board_read);
	  else {
	    if ((strncmp(board_read->section,
			 "/administration",
			 strlen("/administration"))!=0)) {
		boards_list = g_list_append(boards_list, board_read);
	      }
      else
          gc_menu_board_free(board_read);
	    }
	  }
    else
        gc_menu_board_free(gcomprisBoard);
	}
      g_free(filename);
    }
  }

  if (db){
    /* remove suppressed boards from db */
    while (list_old_boards_id != NULL){
      int *data=list_old_boards_id->data;
      gc_db_remove_board(*data);
      list_old_boards_id=g_list_remove(list_old_boards_id, data);
      g_free(data);
    }

  }

  g_dir_close(dir);
}


/* load all the menus xml files in the gcompris path
 * into our memory structures.
 */
void gc_menu_load()
{
  GcomprisProperties	*properties = gc_prop_get();

  if(boards_list)
    {
      cleanup_menus();
      return;
    }

  if ((!properties->reread_menu) && gc_db_check_boards())
    {
      boards_list = gc_menu_load_db(boards_list);

      if (!properties->administration)
	{
	  GList *out_boards = NULL;
	  GList *list = NULL;
	  GcomprisBoard *board;

	  for (list = boards_list; list != NULL; list = list->next)
	    {
	      board = (GcomprisBoard *)list->data;
	      if (g_list_find_custom(gc_profile_get_current()->activities,
				     &(board->board_id), compare_id))
		out_boards = g_list_append(out_boards, board);
	    }
	  for (list = out_boards; list != NULL; list = list->next)
	    boards_list = g_list_remove(boards_list, list->data);
	}
    }
  else
    {
      int db = (gc_profile_get_current() ? TRUE: FALSE);
      properties->reread_menu = TRUE;
      gc_menu_load_dir(properties->package_data_dir, db);
      GDate *today = g_date_new();
      g_date_set_time (today, time (NULL));

      gchar date[11];
      g_date_strftime (date, 11, "%F", today);
      gc_db_set_date(date);
      gc_db_set_version(VERSION);
      g_date_free(today);
    }


  if (properties->local_directory){
    gchar *board_dir = g_strdup_printf("%s/boards/", properties->local_directory);
    gc_menu_load_dir(board_dir, FALSE);
    g_free(board_dir);
  }
}


void gc_menu_board_free(GcomprisBoard *board)
{
#ifndef WIN32 /* Win32 plugins are static */
    /* Need to reference cound plugins and free them once it's no more used */
    // FIXME g_free(board->plugin);
#endif
    g_free(board->type);
    g_free(board->board_dir);
    g_free(board->mode);

    g_free(board->name);
    g_free(board->title);
    g_free(board->description);
    g_free(board->icon_name);
    g_free(board->author);
    g_free(board->boarddir);
    g_free(board->filename);
    g_free(board->difficulty);
    g_free(board->mandatory_sound_file);
    g_free(board->mandatory_sound_dataset);

    g_free(board->section);
    g_free(board->menuposition);

    g_free(board->prerequisite);
    g_free(board->goal);
    g_free(board->manual);
    g_free(board->credit);

    if (board->gmodule)
        g_module_close(board->gmodule);
    g_free(board->gmodule_file);

    g_free(board);
}

void gc_menu_destroy(void)
{
    GList * list;
    for(list = boards_list ; list ; list = list -> next)
    {
        gc_menu_board_free((GcomprisBoard *) list->data);
    }
}
