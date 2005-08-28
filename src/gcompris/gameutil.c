/* gcompris - gameutil.c
 *
 * Time-stamp: <2005/08/29 00:26:24 yves>
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

#include <math.h>

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "gcompris.h"

#include "assetml.h"

#define IMAGEEXTENSION ".png"

#define MAX_DESCRIPTION_LENGTH 1000

/* List of all available boards  */
static GList *boards_list = NULL;

static GnomeCanvasGroup *rootDialogItem = NULL;
static GnomeCanvasItem *itemDialogText = NULL;
static gint item_event_ok(GnomeCanvasItem *item, GdkEvent *event, DialogBoxCallBack dbcb);

extern GnomeCanvas *canvas;

typedef void (*sighandler_t)(int);


GList *gcompris_get_boards_list()
{
  return boards_list;
}

/*
 * Returns a filename path found from the assetml base. 
 */
gchar *gcompris_get_asset_file(gchar *dataset, gchar* categories, 
			       gchar* mimetype, gchar* file)
{
  GList *gl_result;
  AssetML *assetml;
  gchar* resultfile = NULL;

  gl_result = assetml_get_asset(dataset, categories, mimetype, gcompris_get_locale(), file);

  if(gl_result && g_list_length(gl_result)>0)
    {

      /* Always get the first item */
      assetml = (AssetML *)g_list_nth_data(gl_result, 0);

      if(assetml->file)
	{
	  resultfile = g_strdup(assetml->file);
	}

      assetml_free_assetlist(gl_result);
    }
  else
    {
      
      g_warning("Asset not found:\n");
      g_warning("   locale='%s'\n", gcompris_get_locale());
      if (dataset)
	g_warning("   dataset='%s'\n", dataset);
      if (categories)
	g_warning("   category='%s'\n", categories);
      if(mimetype)
	g_warning("   mimetype='%s'\n", mimetype);
      if(file)
	g_warning("   file='%s'\n", file);
    }

  return (resultfile);
}

/*
 * Returns a filename path found from the assetml base. 
 */
gchar *gcompris_get_asset_file_locale(gchar *dataset, gchar* categories, 
			       gchar* mimetype, gchar* file, gchar *locale)
{
  GList *gl_result;
  AssetML *assetml;
  gchar* resultfile = NULL;
  gchar *mylocale = locale;

  if (!mylocale || (strcmp( mylocale, "NULL") == 0))
    mylocale = gcompris_get_locale();

  gl_result = assetml_get_asset(dataset, categories, mimetype, mylocale, file);

  if(gl_result && g_list_length(gl_result)>0)
    {

      /* Always get the first item */
      assetml = (AssetML *)g_list_nth_data(gl_result, 0);

      if(assetml->file)
	{
	  resultfile = g_strdup(assetml->file);
	}

      assetml_free_assetlist(gl_result);
    }
  else
    {
      
      g_warning("Asset not found:\n");
      g_warning("   locale='%s'\n", mylocale);
      if (dataset)
	g_warning("   dataset='%s'\n", dataset);
      if (categories)
	g_warning("   category='%s'\n", categories);
      if(mimetype)
	g_warning("   mimetype='%s'\n", mimetype);
      if(file)
	g_warning("   file='%s'\n", file);
    }

  return (resultfile);
}


/*
 * Load a pixmap from the assetml database
 */
GdkPixbuf *gcompris_load_pixmap_asset(gchar *dataset, gchar* categories, 
				      gchar* mimetype, gchar* name)
{
  GdkPixbuf *pixmap;
  gchar* file = NULL;
  
  file = gcompris_get_asset_file(dataset, categories, mimetype, name);
  
  if(file)
    {
      
      pixmap = gdk_pixbuf_new_from_file (file, NULL);
      
      if(!pixmap)
	g_warning("Loading image returned a null pointer");
      
      return pixmap;
    }

  return NULL;
}

/*
 * load a pixmap from the filesystem 
 * pixmapfile is given relative to PACKAGE_DATA_DIR
 */
GdkPixbuf *gcompris_load_pixmap(char *pixmapfile)
{
  gchar *filename;
  GdkPixbuf *pixmap=NULL;

  /* Search */

  filename = gcompris_find_absolute_filename(pixmapfile);

  if (!filename) {
    char *str;
    g_warning (_("Couldn't find file %s !"), pixmapfile);

    str = g_strdup_printf("%s\n%s\n%s\n%s", 
			  _("Couldn't find file"), 
			  pixmapfile,
			  _("This activity is incomplete."),
			  _("Exit it and report\nthe problem to the authors."));
    gcompris_dialog (str, NULL);
    g_free(str);
    return NULL;
  }

  pixmap = gdk_pixbuf_new_from_file (filename, NULL);

  g_free(filename);

  if(!pixmap)
    g_warning("Loading image returned a null pointer");

  return(pixmap);
}

/*************************************************************
 * colorshift a pixbuf
 * code taken from the gnome-panel of gnome-core
 */
static void
do_colorshift (GdkPixbuf *dest, GdkPixbuf *src, int shift)
{
  gint i, j;
  gint width, height, has_alpha, srcrowstride, destrowstride;
  guchar *target_pixels;
  guchar *original_pixels;
  guchar *pixsrc;
  guchar *pixdest;
  int val;
  guchar r,g,b;

  has_alpha = gdk_pixbuf_get_has_alpha (src);
  width = gdk_pixbuf_get_width (src);
  height = gdk_pixbuf_get_height (src);
  srcrowstride = gdk_pixbuf_get_rowstride (src);
  destrowstride = gdk_pixbuf_get_rowstride (dest);
  target_pixels = gdk_pixbuf_get_pixels (dest);
  original_pixels = gdk_pixbuf_get_pixels (src);

  for (i = 0; i < height; i++) {
    pixdest = target_pixels + i*destrowstride;
    pixsrc = original_pixels + i*srcrowstride;
    for (j = 0; j < width; j++) {
      r = *(pixsrc++);
      g = *(pixsrc++);
      b = *(pixsrc++);
      val = r + shift;
      *(pixdest++) = CLAMP(val, 0, 255);
      val = g + shift;
      *(pixdest++) = CLAMP(val, 0, 255);
      val = b + shift;
      *(pixdest++) = CLAMP(val, 0, 255);
      if (has_alpha)
	*(pixdest++) = *(pixsrc++);
    }
  }
}



static GdkPixbuf *
make_hc_pixbuf(GdkPixbuf *pb, gint val)
{
  GdkPixbuf *new;
  if(!pb)
    return NULL;

  new = gdk_pixbuf_new(gdk_pixbuf_get_colorspace(pb),
		       gdk_pixbuf_get_has_alpha(pb),
		       gdk_pixbuf_get_bits_per_sample(pb),
		       gdk_pixbuf_get_width(pb),
		       gdk_pixbuf_get_height(pb));
  do_colorshift(new, pb, val);
  /*do_saturate_darken (new, pb, (int)(1.00*255), (int)(1.15*255));*/

  return new;
}

/**
 * Free the highlight image from our image_focus system
 */
static void
free_image_focus (GnomeCanvasItem *item, void *none)
{
  GdkPixbuf *pixbuf;

  pixbuf = (GdkPixbuf *)g_object_get_data (G_OBJECT (item), "pixbuf_ref");
  gdk_pixbuf_unref(pixbuf);
}

/**
 * Set the focus of the given image (highlight or not)
 *
 */
void gcompris_set_image_focus(GnomeCanvasItem *item, gboolean focus)
{
  GdkPixbuf *dest = NULL;
  GdkPixbuf *pixbuf;
  GdkPixbuf *pixbuf_ref;

  gtk_object_get (GTK_OBJECT (item), "pixbuf", &pixbuf, NULL);
  g_return_if_fail (pixbuf != NULL);

  /* Store the first pixbuf */
  pixbuf_ref = (GdkPixbuf *)g_object_get_data (G_OBJECT (item), "pixbuf_ref");
  if(!pixbuf_ref)
    {
      g_object_set_data (G_OBJECT (item), "pixbuf_ref", pixbuf);
      pixbuf_ref = pixbuf;
      g_signal_connect (item, "destroy",
 			G_CALLBACK (free_image_focus),
 			NULL);

    }
  

  switch (focus)
    {
    case TRUE:
      dest = make_hc_pixbuf(pixbuf, 30);
      gnome_canvas_item_set (item,
			     "pixbuf", dest,
			     NULL);

      break;
    case FALSE:
      gnome_canvas_item_set (item,
			     "pixbuf", pixbuf_ref,
			     NULL);
      break;
    default:
      break;
    }

  if(dest!=NULL)
    gdk_pixbuf_unref (dest);

}

/**
 * Callback over a canvas item, this function will highlight the focussed item
 * or the given one
 *
 */
gint gcompris_item_event_focus(GnomeCanvasItem *item, GdkEvent *event,
			       GnomeCanvasItem *dest_item)
{

  if(dest_item!=NULL)
    item = dest_item;

  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
      gcompris_set_image_focus(item, TRUE);
      break;
    case GDK_LEAVE_NOTIFY:
      gcompris_set_image_focus(item, FALSE);
      break;
    default:
      break;
    }

  return FALSE;
}

/*
 * Return a copy of the given string in which it has
 * changes '\''n' to '\n'.
 * The recognized sequences are \b
 * \f \n \r \t \\ \" and the octal format.
 * 
 */
gchar *reactivate_newline(char *str)
{
  gchar *newstr;

  if(str==NULL)
    return NULL;

  newstr = g_strcompress(str);
  
  //g_free(str);

  return newstr;
}

/*
 * Thanks for George Lebl <jirka@5z.com> for his Genealogy example
 * for all the XML stuff there
 */

static void
gcompris_add_xml_to_data(xmlDocPtr doc, xmlNodePtr xmlnode, GNode * child, GcomprisBoard *gcomprisBoard, gboolean db)
{
  GcomprisProperties	*properties = gcompris_get_properties();
  gchar *title=NULL;
  gchar *description=NULL;
  gchar *prerequisite=NULL;
  gchar *goal=NULL;
  gchar *manual=NULL;
  gchar *credit=NULL;

  if(/* if the node has no name */
     !xmlnode->name ||
     /* or if the name is not "Board" */
     (g_strcasecmp(xmlnode->name,"Board")!=0)
     )
    return;

  /* get the type of the board */
  gcomprisBoard->type = xmlGetProp(xmlnode,"type");

  /* get the specific mode for this board */
  gcomprisBoard->mode			 = xmlGetProp(xmlnode,"mode");
  gcomprisBoard->name			 = xmlGetProp(xmlnode,"name");
  gcomprisBoard->icon_name		 = xmlGetProp(xmlnode,"icon");
  gcomprisBoard->author			 = xmlGetProp(xmlnode,"author");
  gcomprisBoard->boarddir		 = xmlGetProp(xmlnode,"boarddir");
  gcomprisBoard->mandatory_sound_file	 = xmlGetProp(xmlnode,"mandatory_sound_file");
  gcomprisBoard->mandatory_sound_dataset = xmlGetProp(xmlnode,"mandatory_sound_dataset");

  gchar *path                            = xmlGetProp(xmlnode,"section");
  if (strlen(path)==1){
    g_free(path);
    path = g_strdup("");
    if (strcmp(gcomprisBoard->name,"root")==0)
      gcomprisBoard->name = "";
  }

  gcomprisBoard->section		 = path;

  gcomprisBoard->title = NULL;
  gcomprisBoard->description = NULL;
  gcomprisBoard->prerequisite = NULL;
  gcomprisBoard->goal = NULL;
  gcomprisBoard->manual = NULL;
  gcomprisBoard->credit = NULL;

  gcomprisBoard->difficulty		= xmlGetProp(xmlnode,"difficulty");
  if(gcomprisBoard->difficulty == NULL)
    gcomprisBoard->difficulty		= "0";

  /* Update the difficulty max */
  if(properties->difficulty_max < atoi(gcomprisBoard->difficulty))
    properties->difficulty_max = atoi(gcomprisBoard->difficulty);

  for (xmlnode = xmlnode->xmlChildrenNode; xmlnode != NULL; xmlnode = xmlnode->next) {
    if (xmlHasProp(xmlnode, "lang"))
      continue;

    /* get the title of the board */
    if (!strcmp(xmlnode->name, "title"))
      {
	title = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	gcomprisBoard->title = reactivate_newline(gettext(title));
      }

    /* get the description of the board */
    if (!strcmp(xmlnode->name, "description"))
      {
	int i;
	description = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	gcomprisBoard->description = reactivate_newline(gettext(description));

	/* WARNING: I used to use the richtext item that handle the clipping but it is not
	 * stable enough. Here I remove any \n unterered by the translator and insert new newlines
	 */

	/* Remove newline in description if needed */
	for(i=0; i<strlen(gcomprisBoard->description); i++) {
	  if(gcomprisBoard->description[i]=='\n')
	    gcomprisBoard->description[i]=' ';
	}

	/* Insert newline in description if needed */
	i=60;
	while(i<strlen(gcomprisBoard->description)) {
	  char *index;
	  index = strchr(gcomprisBoard->description+i, ' ');
	  if(index)
	    *index='\n';
	  i+=70;
	}
      }

    /* get the help prerequisite help of the board */
    if (!strcmp(xmlnode->name, "prerequisite"))
      {
	if(gcomprisBoard->prerequisite)
	  g_free(gcomprisBoard->prerequisite);
	
	prerequisite = xmlNodeListGetString(doc,  xmlnode->xmlChildrenNode, 1);
	gcomprisBoard->prerequisite = reactivate_newline(gettext(prerequisite));
      }

    /* get the help goal of the board */
    if (!strcmp(xmlnode->name, "goal"))
      {
	if(gcomprisBoard->goal)
	  g_free(gcomprisBoard->goal);
  
	goal = xmlNodeListGetString(doc,  xmlnode->xmlChildrenNode, 1);
	gcomprisBoard->goal = reactivate_newline(gettext(goal));
      }

    /* get the help user manual of the board */
    if (!strcmp(xmlnode->name, "manual"))
      {
	if(gcomprisBoard->manual)
	  g_free(gcomprisBoard->manual);

	manual = xmlNodeListGetString(doc,  xmlnode->xmlChildrenNode, 1);
	gcomprisBoard->manual = reactivate_newline(gettext(manual));
      }

    /* get the help user credit of the board */
    if (!strcmp(xmlnode->name, "credit"))
      {
	if(gcomprisBoard->credit)
	  g_free(gcomprisBoard->credit);
  
	credit = xmlNodeListGetString(doc,  xmlnode->xmlChildrenNode, 1);
	gcomprisBoard->credit = reactivate_newline(gettext(credit));
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
    gcompris_db_board_update( &gcomprisBoard->board_id, 
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
			      title,
			      description,
			      prerequisite,
			      goal,
			      manual,
			      credit
			      );
  
    g_warning("db board written %d in %d  %s/%s", gcomprisBoard->board_id, gcomprisBoard->section_id, gcomprisBoard->section, gcomprisBoard->name);
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
    gcompris_add_xml_to_data(doc, node, NULL, gcomprisBoard, db);
  }
}



/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set
   Return a newly allocated GcomprisBoard or NULL if the parsing failed
*/
GcomprisBoard *gcompris_read_xml_file(GcomprisBoard *gcomprisBoard, 
				      char *fname,
				      gboolean db)
{
  gchar *filename;
  /* pointer to the new doc */
  xmlDocPtr doc;

  g_return_val_if_fail(fname!=NULL,FALSE);

  filename = g_strdup(fname);

  /* if the file doesn't exist */
  if(!g_file_test ((filename), G_FILE_TEST_EXISTS))
    {
      g_free(filename);

      /* if the file doesn't exist, try with our default prefix */
      filename = g_strdup_printf("%s/%s",
				 PACKAGE_DATA_DIR, fname);

      if(!g_file_test ((filename), G_FILE_TEST_EXISTS))
	{
	  g_warning(_("Couldn't find file %s !"), fname);
	  g_warning(_("Couldn't find file %s !"), filename);
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
     g_strcasecmp(doc->children->name,"GCompris")!=0) {
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
  gcomprisBoard->canvas=canvas;

  gcomprisBoard->gmodule      = NULL;
  gcomprisBoard->gmodule_file = NULL;

  /* Fixed since I use the canvas own pixel_per_unit scheme */
  gcomprisBoard->width  = BOARDWIDTH;
  gcomprisBoard->height = BOARDHEIGHT;

  return gcomprisBoard;
}

/* Return the first board with the given section
 */
GcomprisBoard *gcompris_get_board_from_section(gchar *section)
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
  g_warning("gcompris_get_board_from_section searching '%s' but NOT FOUND\n", section);
  return NULL;
}

static int boardlist_compare_func(const void *a, const void *b)
{
  return strcasecmp(((GcomprisBoard *) a)->difficulty, ((GcomprisBoard *) b)->difficulty);
}


/* Return the list of boards in the given section
 * Boards are sorted depending on their difficulty value
 */
GList *gcompris_get_menulist(gchar *section)
{
  GList *list = NULL;
  GList *result_list = NULL;
  gchar *model;

  if (strlen(section)==1)
    model = g_strdup("");
  else
    model = g_strdup(section);

  if(!section){
    g_warning("gcompris_get_menulist called with section == NULL !");
    return NULL;
  }

  for(list = boards_list; list != NULL; list = list->next) {
    GcomprisBoard *board = list->data;

    if (strcmp (model, board->section) == 0) {	
      if (strlen(board->name) != 0)
	result_list = g_list_append(result_list, board);
    }
  }

  g_free(model);

  /* Sort the list now */
  result_list = g_list_sort(result_list, boardlist_compare_func);

  return result_list;
}

/*
 * Select only files with .xml extention
 * Return 1 if the given file end in .xml 0 else
 */
int selectMenuXML(const gchar *file)
{

  if(strlen(file)<4)
    return 0;

  return (strncmp (&file[strlen(file)-4], ".xml", 4) == 0);
}

void cleanup_menus() {
  GList *list = NULL;

  for(list = boards_list; list != NULL; list = list->next) {
    GcomprisBoard *gcomprisBoard = list->data;

    gcompris_read_xml_file(gcomprisBoard, gcomprisBoard->filename, FALSE);
  }
}


/*
 * suppress_int_from_list
 */

GList *suppress_int_from_list(GList *list, int value)
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

gboolean compare_id(gconstpointer data1, gconstpointer data2)
{
  int *i = (int *) data1;
  int *j = (int *) data2;

  if (*i == *j)
    return 0;
  else
    return -1;
}

/*
 * gcompris_load_menus
 *
 * Load all the menu it can from the given dirname
 *
 */
void gcompris_load_menus_dir(char *dirname, gboolean db){
  const gchar   *one_dirent;
  GDir          *dir;
  GcomprisProperties	*properties = gcompris_get_properties();
  GList *list_old_boards_id = NULL;

  if (!g_file_test(dirname, G_FILE_TEST_IS_DIR)) {
    g_warning("Failed to parse board in '%s' because it's not a directory\n", dirname);
    return;
  }

  dir = g_dir_open(dirname, 0, NULL);

  if (!dir) {
    g_warning("gcompris_load_menus : no menu found in %s", dirname);
    return;
  } else {
    if (db)
      list_old_boards_id = gcompris_db_get_board_id(list_old_boards_id);

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

      if(selectMenuXML(one_dirent)) {
	gcomprisBoard = g_malloc0 (sizeof (GcomprisBoard));
	gcomprisBoard->board_dir = dirname;

	/* Need to be initialized here because gcompris_read_xml_file is used also to reread 	*/
	/* the locale data									*/
	/* And we don't want in this case to loose the current plugin				*/
	gcomprisBoard->plugin=NULL;
	gcomprisBoard->previous_board=NULL;

	GcomprisBoard *board_read = gcompris_read_xml_file(gcomprisBoard, filename, db);
	if (board_read){
	  list_old_boards_id = suppress_int_from_list(list_old_boards_id, board_read->board_id);
	  if (properties->administration)
	    boards_list = g_list_append(boards_list, board_read);
	  else 
	    if ((strncmp(board_read->section,
			 "/administration",
			 strlen("/administration"))!=0) &&
		(!(g_list_find_custom(gcompris_get_current_profile()->activities, &(board_read->board_id), compare_id))))
	      boards_list = g_list_append(boards_list, board_read);
	}
      }
      g_free(filename);
    }
  }

  if (db){
    /* remove suppressed boards from db */
    while (list_old_boards_id != NULL){
      int *data=list_old_boards_id->data;
      gcompris_db_remove_board(*data);
      list_old_boards_id=g_list_remove(list_old_boards_id, data);
      g_free(data);
    }

  }
  
  g_dir_close(dir);
}
  

/* load all the menus xml files in the gcompris path
 * into our memory structures.
 */
void gcompris_load_menus()
{
  GcomprisProperties	*properties = gcompris_get_properties();

  if(boards_list) {
    cleanup_menus();
    return;
  }

  if ((!properties->reread_menu) && gcompris_db_check_boards()){
    boards_list = gcompris_load_menus_db(boards_list);

    if (!properties->administration){
      GList *out_boards = NULL;
      GList *list = NULL;
      GcomprisBoard *board;

      for (list = boards_list; list != NULL; list = list->next){
	board = (GcomprisBoard *)list->data;
	if (g_list_find_custom(gcompris_get_current_profile()->activities,
			       &(board->board_id), compare_id))
	  out_boards = g_list_append(out_boards, board);
      }
      for (list = out_boards; list != NULL; list = list->next)
	boards_list = g_list_remove(boards_list, list->data);
    }
  }
  else {
    properties->reread_menu = TRUE;
    gcompris_load_menus_dir(properties->package_data_dir, TRUE);
    GDate *today = g_date_new();
    g_date_set_time (today, time (NULL));

    gchar date[11];
    g_date_strftime (date, 11, "%F", today);
    gcompris_db_set_date(date);
    g_date_free(today);
  }
  

  if (properties->local_directory)
    gcompris_load_menus_dir(properties->local_directory, FALSE);

}

/* ======================================= */
void item_absolute_move(GnomeCanvasItem *item, int x, int y) {
  double dx1, dy1, dx2, dy2;
  gnome_canvas_item_get_bounds(item, &dx1, &dy1, &dx2, &dy2);
  gnome_canvas_item_move(item, ((double)x)-dx1, ((double)y)-dy1);
}

/* ======================================= */
/* As gnome does not implement its own API : gnome_canvas_item_rotate
   we have to do it ourselves ....
   IMPORTANT NOTE : This is designed for an item with "anchor" =  GTK_ANCHOR_CENTER
   rotation is clockwise if angle > 0 */
void item_rotate(GnomeCanvasItem *item, double angle) {
  double r[6],t[6], x1, x2, y1, y2;

  gnome_canvas_item_get_bounds( item, &x1, &y1, &x2, &y2 );
  art_affine_translate( t , -(x2+x1)/2, -(y2+y1)/2 );
  art_affine_rotate( r, angle );
  art_affine_multiply( r, t, r);
  art_affine_translate( t , (x2+x1)/2, (y2+y1)/2 );
  art_affine_multiply( r, r, t);

  gnome_canvas_item_affine_absolute(item, r );
}

/* As gnome does not implement its own API : gnome_canvas_item_rotate
   we have to do it ourselves ....
   IMPORTANT NOTE : This is designed for an item with "anchor" =  GTK_ANCHOR_CENTER
   rotation is clockwise if angle > 0 */
void item_rotate_relative(GnomeCanvasItem *item, double angle) {
  double x1, x2, y1, y2;
  double tx1, tx2, ty1, ty2;
  double cx, cy;
  double t;
  double r[6];

  //  gnome_canvas_item_get_bounds( item, &x1, &y1, &x2, &y2 );
  /* WARNING: Do not use gnome_canvas_item_get_bounds which gives unpredictable results */
  if(GNOME_IS_CANVAS_LINE(item)) {
    GnomeCanvasPoints	*points;
    gtk_object_get (GTK_OBJECT (item), "points", &points, NULL);
    x1 = points->coords[0];
    y1 = points->coords[1];
    x2 = points->coords[2];
    y2 = points->coords[3];
  } else if(GNOME_IS_CANVAS_PIXBUF(item)){
    gtk_object_get (GTK_OBJECT (item), "x", &x1, NULL);
    gtk_object_get (GTK_OBJECT (item), "y", &y1, NULL);
    gtk_object_get (GTK_OBJECT (item), "width",  &x2, NULL);
    gtk_object_get (GTK_OBJECT (item), "height", &y2, NULL);
    x2 += x1;
    y2 += y1;
  } else if(GNOME_IS_CANVAS_GROUP(item)){
    gtk_object_get (GTK_OBJECT (item), "x", &x1, NULL);
    gtk_object_get (GTK_OBJECT (item), "y", &y1, NULL);
    x2 = x1;
    y2 = y1;
  } else {
    gtk_object_get (GTK_OBJECT (item), "x1", &x1, NULL);
    gtk_object_get (GTK_OBJECT (item), "y1", &y1, NULL);
    gtk_object_get (GTK_OBJECT (item), "x2", &x2, NULL);
    gtk_object_get (GTK_OBJECT (item), "y2", &y2, NULL);
  }

  tx1 = x1;
  ty1 = y1;
  tx2 = x2;
  ty2 = y2;

  x1 = MIN(tx1,tx2);
  y1 = MIN(ty1,ty2);
  x2 = MAX(tx1,tx2);
  y2 = MAX(ty1,ty2);
    

  cx = (x2+x1)/2;
  cy = (y2+y1)/2;

  /* Taken from anim by Yves Combe
   * This matrix rotate around ( cx, cy )
   * This is the result of the product:
   *            T_{-c}             Rot (t)                 T_c
   *
   *       1    0   cx       cos(t) -sin(t)    0        1    0  -cx
   *       0    1   cy  by   sin(t)  cos(t)    0   by   0    1  -cy
   *       0    0    1         0       0       1        0    0   1
   */

  t = M_PI*angle/180.0;

  r[0] = cos(t);
  r[1] = sin(t);
  r[2] = -sin(t);
  r[3] = cos(t);
  r[4] = (1-cos(t))*cx + sin(t)*cy;
  r[5] = -sin(t)*cx + (1 - cos(t))*cy;

  gnome_canvas_item_affine_relative(item, r );
}

/* rotates an item around the center (x,y), relative to the widget's coordinates */
void	item_rotate_with_center(GnomeCanvasItem *item, double angle, int x, int y) {
  double r[6],t[6], x1, x2, y1, y2, tx, ty;

  gnome_canvas_item_get_bounds( item, &x1, &y1, &x2, &y2 );
  tx = x1 + x;
  ty = y1 + y;
  art_affine_translate( t , -tx, -ty );
  art_affine_rotate( r, angle );
  art_affine_multiply( r, t, r);
  art_affine_translate( t , tx, ty );
  art_affine_multiply( r, r, t);

  gnome_canvas_item_affine_absolute(item, r );
}

/* rotates an item around the center (x,y), relative to the widget's coordinates */
/* The rotation is relative to the previous rotation */
void	item_rotate_relative_with_center(GnomeCanvasItem *item, double angle, int x, int y) {
  double r[6],t[6], x1, x2, y1, y2, tx, ty;

  gnome_canvas_item_get_bounds( item, &x1, &y1, &x2, &y2 );
  tx = x1 + x;
  ty = y1 + y;
  art_affine_translate( t , -tx, -ty );
  art_affine_rotate( r, angle );
  art_affine_multiply( r, t, r);
  art_affine_translate( t , tx, ty );
  art_affine_multiply( r, r, t);

  gnome_canvas_item_affine_relative(item, r );
}

/*
 * Close the dialog box if it was open. It not, do nothing.
 */
void gcompris_dialog_close() {

  /* If we already running delete the previous one */
  if(rootDialogItem) {
    /* WORKAROUND: There is a bug in the richtex item and we need to remove it first */
    while (g_idle_remove_by_data (itemDialogText));
    gtk_object_destroy (GTK_OBJECT(itemDialogText));
    itemDialogText = NULL;

    gtk_object_destroy(GTK_OBJECT(rootDialogItem));
  }
  
  rootDialogItem = NULL;

}

/*
 * Display a dialog box and an OK button
 * When the box is closed, the given callback is called if any
 */
void gcompris_dialog(gchar *str, DialogBoxCallBack dbcb)
{
  GcomprisBoard   *gcomprisBoard = get_current_gcompris_board();
  GnomeCanvasItem *item_text   = NULL;
  GnomeCanvasItem *item_text_ok   = NULL;
  GdkPixbuf       *pixmap_dialog = NULL;
  GtkTextIter      iter_start, iter_end;
  GtkTextBuffer   *buffer;
  GtkTextTag      *txt_tag;

  printf("Dialog=%s\n", str);

  if(!gcomprisBoard)
    return;

  /* If we are already running do nothing */
  if(rootDialogItem) {
    g_warning("Cannot run a dialog box, one is already running. Message = %s\n", str);
    return;
  }

  /* First pause the board */
  if(gcomprisBoard->plugin && gcomprisBoard->plugin->pause_board != NULL)
      gcomprisBoard->plugin->pause_board(TRUE);

  gcompris_bar_hide(TRUE);

  rootDialogItem = GNOME_CANVAS_GROUP(
				      gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							     gnome_canvas_group_get_type (),
							     "x", (double) 0,
							     "y", (double) 0,
							     NULL));
      
  pixmap_dialog = gcompris_load_skin_pixmap("dialogbox.png");

  itemDialogText = gnome_canvas_item_new (rootDialogItem,
				       gnome_canvas_pixbuf_get_type (),
				       "pixbuf", pixmap_dialog,
				       "x", (double) (BOARDWIDTH - gdk_pixbuf_get_width(pixmap_dialog))/2,
				       "y", (double) (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap_dialog))/2,
				      NULL);

  /* OK Text */
  item_text_ok = gnome_canvas_item_new (rootDialogItem,
				gnome_canvas_text_get_type (),
				"text", _("OK"),
				"font", gcompris_skin_font_title,
				"x", (double)  BOARDWIDTH*0.5,
				"y", (double)  (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap_dialog))/2 +
					gdk_pixbuf_get_height(pixmap_dialog) - 35,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_text_button,
				NULL);

  gdk_pixbuf_unref(pixmap_dialog);

  gtk_signal_connect(GTK_OBJECT(itemDialogText), "event",
		     (GtkSignalFunc) item_event_ok,
		     dbcb);

  item_text = gnome_canvas_item_new (rootDialogItem,
				     gnome_canvas_rich_text_get_type (),
				     "x", (double) BOARDWIDTH/2,
				     "y", (double) 100.0,
				     "width", (double)BOARDWIDTH-260.0,
				     "height", 400.0,
				     "anchor", GTK_ANCHOR_NORTH,
				     "justification", GTK_JUSTIFY_CENTER,
				     "grow_height", FALSE,
				     "cursor_visible", FALSE,
				     "cursor_blink", FALSE,
				     "editable", FALSE,
				     NULL);

  gnome_canvas_item_set (item_text,
			 "text", str,
			 NULL);

  buffer  = gnome_canvas_rich_text_get_buffer(GNOME_CANVAS_RICH_TEXT(item_text));
  txt_tag = gtk_text_buffer_create_tag(buffer, NULL, 
				       "font",       gcompris_skin_font_board_medium,
				       "foreground", "blue",
				       "family-set", TRUE,
				       NULL);
  gtk_text_buffer_get_end_iter(buffer, &iter_end);
  gtk_text_buffer_get_start_iter(buffer, &iter_start);
  gtk_text_buffer_apply_tag(buffer, txt_tag, &iter_start, &iter_end);

  gtk_signal_connect(GTK_OBJECT(item_text), "event",
		     (GtkSignalFunc) item_event_ok,
		     dbcb);
  gtk_signal_connect(GTK_OBJECT(item_text_ok), "event",
		     (GtkSignalFunc) item_event_ok,
		     dbcb);

}

/* Callback for the bar operations */
static gint
item_event_ok(GnomeCanvasItem *item, GdkEvent *event, DialogBoxCallBack dbcb)
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
      if(rootDialogItem)
	gcompris_dialog_close();

      /* restart the board */
      if(gcomprisBoard->plugin->pause_board != NULL)
	gcomprisBoard->plugin->pause_board(FALSE);
      
      gcompris_bar_hide(FALSE);

      if(dbcb != NULL)
	dbcb();
	  
    default:
      break;
    }
}

/**
 * Display the number of stars representing the difficulty level at the x,y location
 * The stars are created in a group 'parent'
 * The new group in which the stars are created is returned.
 * This is only usefull for the menu plugin and the configuration dialog box.
 */
GnomeCanvasGroup *gcompris_display_difficulty_stars(GnomeCanvasGroup *parent, 
						    double x, double y, 
						    double ratio,
						    gint difficulty)
{
  GdkPixbuf *pixmap = NULL;
  GnomeCanvasGroup *stars_group = NULL;
  GnomeCanvasPixbuf *item = NULL;
  gchar *filename = NULL;

  if(difficulty==0 || difficulty>6)
    return NULL;

  filename = g_strdup_printf("difficulty_star%d.png", difficulty);
  pixmap   = gcompris_load_skin_pixmap(filename);
  g_free(filename);

  if(!pixmap)
    return NULL;

  stars_group = GNOME_CANVAS_GROUP(
				  gnome_canvas_item_new (parent,
							 gnome_canvas_group_get_type (),
							 "x", (double) 0,
							 "y", (double) 0,
							 NULL));

  item = gnome_canvas_item_new (stars_group,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap,
				"x", x,
				"y", y,
				"width", (double) gdk_pixbuf_get_width(pixmap) * ratio,
				"height", (double) gdk_pixbuf_get_height(pixmap) * ratio,
				"width_set", TRUE, 
				"height_set", TRUE,
				NULL);
  
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  gdk_pixbuf_unref(pixmap);

  return(stars_group);
}

gchar *g_utf8_strndup(gchar* utf8text, gint n)
{
 gchar* result;

 gint len = g_utf8_strlen(utf8text, -1);

 if( n < len && n > 0 )
   len = n;

 result = g_strndup(utf8text, g_utf8_offset_to_pointer(utf8text, len) - utf8text);

 return result;
} 

/*
 * Return NULL or a new gchar* with the absolute_filename of the given filename
 *
 */
gchar *gcompris_find_absolute_filename(gchar *filename)
{
  gchar *absolute_filename;

  /* Search it on the file system */

  if (!g_path_is_absolute (filename)) {
    GcomprisProperties	*properties = gcompris_get_properties();

    absolute_filename = g_strdup_printf("%s/%s", properties->package_data_dir, filename);

    if (!g_file_test (absolute_filename, G_FILE_TEST_EXISTS)) {
      GcomprisBoard   *gcomprisBoard = get_current_gcompris_board();
      g_free(absolute_filename);
      if(gcomprisBoard) {
	absolute_filename = g_strdup_printf("%s/%s", gcomprisBoard->board_dir, filename);
      } else {
	return NULL;
      }
    }
  } else {
    absolute_filename = strdup(filename);
  }
    
  if (!g_file_test (absolute_filename, G_FILE_TEST_EXISTS)){
    g_free(absolute_filename);
    return NULL;
  }
  
  return absolute_filename;
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
