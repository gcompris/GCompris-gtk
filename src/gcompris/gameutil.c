/* gcompris - gameutil.c
 *
 * Time-stamp: <2004/05/29 02:07:14 bcoudoin>
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

#include <dirent.h>
#include <math.h>

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "gcompris.h"

#include "assetml.h"

#define IMAGEEXTENSION ".png"

#define MAX_DESCRIPTION_LENGTH 1000

/* default gnome pixmap directory in which this game tales the icon */
static char *lettersdir = "letters/";

/* List of all available boards  */
static GList *boards_list = NULL;

static GnomeCanvasGroup *rootDialogItem = NULL;
static gint item_event_ok(GnomeCanvasItem *item, GdkEvent *event, DialogBoxCallBack dbcb);

extern GnomeCanvas *canvas;

typedef void (*sighandler_t)(int);

/*
 * Returns a filename path found from the assetml base. 
 */
gchar *gcompris_get_asset_file(gchar *dataset, gchar* categories, 
			       gchar* mimetype, gchar* file)
{
  GList *gl_result;
  AssetML *assetml;
  GdkPixbuf *pixmap;
  gchar* resultfile = NULL;

  gl_result = assetml_get_asset(dataset, categories, mimetype, gcompris_get_locale(), file);

  if(g_list_length(gl_result)>0)
    {

      /* Always get the first item */
      assetml = (AssetML *)g_list_nth_data(gl_result, 0);

      if(assetml->file)
	{
	  resultfile = g_strdup(assetml->file);
	}

    }
  else
    {
      g_warning("Asset not found (dataset=%s category=%s mimetype=%s locale=%s file=%s)\n", 
		dataset, categories, mimetype, gcompris_get_locale(), file);
    }
  assetml_free_assetlist(gl_result);

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
  GdkPixbuf *pixmap;

  /* Search it on the file system */
  filename = g_strdup_printf("%s/%s", PACKAGE_DATA_DIR, pixmapfile);

  if (!g_file_test ((filename), G_FILE_TEST_EXISTS)) {
    char *str;
    g_warning (_("Couldn't find file %s !"), filename);

    str = g_strdup_printf("%s\n%s\n%s\n%s", 
			  _("Couldn't find file"), 
			  pixmapfile,
			  _("This activity is incomplete."),
			  _("Exit it and report\nthe problem to the authors."));
    gcompris_dialog (str, NULL);
    g_free(str);
  }

  pixmap = gdk_pixbuf_new_from_file (filename, NULL);

  if(!pixmap)
    g_warning("Loading image returned a null pointer");

  g_free (filename);

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
gchar *reactivate_newline(gchar *str)
{
  gchar *newstr;

  if(str==NULL)
    return NULL;

  newstr = g_strcompress(str);
  
  g_free(str);

  return newstr;
}

/*
 * Thanks for George Lebl <jirka@5z.com> for his Genealogy example
 * for all the XML stuff there
 */

static void
gcompris_add_xml_to_data(xmlDocPtr doc, xmlNodePtr xmlnode, GNode * child, GcomprisBoard *gcomprisBoard)
{

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
  gcomprisBoard->section		 = xmlGetProp(xmlnode,"section");

  gcomprisBoard->title = NULL;
  gcomprisBoard->description = NULL;
  gcomprisBoard->prerequisite = NULL;
  gcomprisBoard->goal = NULL;
  gcomprisBoard->manual = NULL;
  gcomprisBoard->credit = NULL;

  gcomprisBoard->difficulty		= xmlGetProp(xmlnode,"difficulty");
  if(gcomprisBoard->difficulty == NULL)
    gcomprisBoard->difficulty		= "0";

  xmlnode = xmlnode->xmlChildrenNode;
  while (xmlnode != NULL) {
    gchar *lang = xmlGetProp(xmlnode,"lang");
    /* get the title of the board */
    if (!strcmp(xmlnode->name, "title")
	&& (lang==NULL
	    || !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	gcomprisBoard->title = reactivate_newline(xmlNodeListGetString(doc, 
								       xmlnode->xmlChildrenNode, 1));
      }

    /* get the description of the board */
    if (!strcmp(xmlnode->name, "description")
	&& (lang==NULL ||
	    !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	gcomprisBoard->description = reactivate_newline(xmlNodeListGetString(doc, 
									     xmlnode->xmlChildrenNode, 1));
      }

    /* get the help prerequisite help of the board */
    if (!strcmp(xmlnode->name, "prerequisite")
	&& (lang==NULL ||
	    !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	if(gcomprisBoard->prerequisite)
	  g_free(gcomprisBoard->prerequisite);

	gcomprisBoard->prerequisite = reactivate_newline(xmlNodeListGetString(doc, 
									      xmlnode->xmlChildrenNode, 1));
      }

    /* get the help goal of the board */
    if (!strcmp(xmlnode->name, "goal")
	&& (lang==NULL ||
	    !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	if(gcomprisBoard->goal)
	  g_free(gcomprisBoard->goal);
  
	gcomprisBoard->goal = reactivate_newline(xmlNodeListGetString(doc, 
								      xmlnode->xmlChildrenNode, 1));
      }

    /* get the help user manual of the board */
    if (!strcmp(xmlnode->name, "manual")
	&& (lang==NULL ||
	    !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	if(gcomprisBoard->manual)
	  g_free(gcomprisBoard->manual);
  
	gcomprisBoard->manual = reactivate_newline(xmlNodeListGetString(doc, 
									xmlnode->xmlChildrenNode, 1));
      }

    /* get the help user credit of the board */
    if (!strcmp(xmlnode->name, "credit")
	&& (lang==NULL ||
	    !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	if(gcomprisBoard->credit)
	  g_free(gcomprisBoard->credit);
  
	gcomprisBoard->credit = reactivate_newline(xmlNodeListGetString(doc, 
									xmlnode->xmlChildrenNode, 1));
      }

    xmlnode = xmlnode->next;
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
}

/* parse the doc, add it to our internal structures and to the clist */
static void
parse_doc(xmlDocPtr doc, GcomprisBoard *gcomprisBoard)
{
  xmlNodePtr node;

  /* find <Board> nodes and add them to the list, this just
     loops through all the children of the root of the document */
  for(node = doc->children->children; node != NULL; node = node->next) {
    /* add the board to the list, there are no children so
       we pass NULL as the node of the child */
    gcompris_add_xml_to_data(doc, node, NULL, gcomprisBoard);
  }
}



/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set
   Return a newly allocated GcomprisBoard or NULL if the parsing failed
*/
GcomprisBoard *gcompris_read_xml_file(GcomprisBoard *gcomprisBoard, char *fname)
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
  
  /* parse our document and replace old data */
  parse_doc(doc, gcomprisBoard);

  xmlFreeDoc(doc);

  /* Store the file that belong to this board for trace and further need */
  gcomprisBoard->filename=filename;
  gcomprisBoard->board_ready=FALSE;
  gcomprisBoard->canvas=canvas;
  gcomprisBoard->previous_board=NULL;

  gcomprisBoard->gmodule      = NULL;
  gcomprisBoard->gmodule_file = NULL;

  /* Fixed since I use the canvas own pixel_per_unit scheme */
  gcomprisBoard->width  = BOARDWIDTH;
  gcomprisBoard->height = BOARDHEIGHT;

  return gcomprisBoard;
}

/* Return the boards with the given section
 */
GcomprisBoard *gcompris_get_board_from_section(gchar *section)
{  
  GList *list = NULL;

  for(list = boards_list; list != NULL; list = list->next) {
    GcomprisBoard *board = list->data;

    if( board->section && (strcmp (board->section, section) == 0))
      {
	return board;
      }
  }
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
  gchar *path;

  for(list = boards_list; list != NULL; list = list->next) {
    GcomprisBoard *board = list->data;

    if(board->section 
       && strlen(section)<=strlen(board->section)
       && strcmp (section, board->section) != 0) {

      path = g_path_get_dirname(board->section);

      if(strcmp (section, path) == 0) {	
	result_list = g_list_append(result_list, board);
      }
      g_free(path);
    }
  }

  /* Sort the list now */
  result_list = g_list_sort(result_list, boardlist_compare_func);

  return result_list;
}

/*
 * Select only files with .xml extention
 * Return 1 if the given file end in .xml 0 else
 */
int selectMenuXML(gchar *file)
{

  if(strlen(file)<4)
    return 0;

  return (strncmp (&file[strlen(file)-4], ".xml", 4) == 0);
}

void cleanup_menus() {
  GList *list = NULL;

  for(list = boards_list; list != NULL; list = list->next) {
    GcomprisBoard *gcomprisBoard = list->data;

    gcompris_read_xml_file(gcomprisBoard, gcomprisBoard->filename);
  }
}

/* load all the menus xml files in the gcompris path
 * into our memory structures.
 */
void gcompris_load_menus()
{
  struct dirent *one_dirent;
  DIR *dir;
  int n;

  if(boards_list) {
    cleanup_menus();
    return;
  }

  /* Load the Pixpmaps directory file names */
  dir = opendir(PACKAGE_DATA_DIR);

  if (!dir) {
    g_warning("gcompris_load_menus : no menu found in %s", PACKAGE_DATA_DIR);
  } else {

    while((one_dirent = readdir(dir)) != NULL) {
      /* add the board to the list */
      GcomprisBoard *gcomprisBoard = NULL;
      gchar *filename;
      
      filename = g_strdup_printf("%s/%s",
				 PACKAGE_DATA_DIR, one_dirent->d_name);

      if(!g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
	g_free(filename);
	continue;
      }

      if(selectMenuXML(one_dirent->d_name)) {
	gcomprisBoard = g_malloc (sizeof (GcomprisBoard));

	/* Need to be initialized here because gcompris_read_xml_file is used also to reread 	*/
	/* the locale data									*/
	/* And we don't want in this case to loose the current plugin				*/
	gcomprisBoard->plugin=NULL;

	boards_list = g_list_append(boards_list, gcompris_read_xml_file(gcomprisBoard, 
									filename));

      }
      g_free(filename);
    }
  }
  closedir(dir);

  gcompris_get_menulist("/");
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
  double r[6],t[6], x1, x2, y1, y2;

  gnome_canvas_item_get_bounds( item, &x1, &y1, &x2, &y2 );
  art_affine_translate( t , -(x2+x1)/2, -(y2+y1)/2 );
  art_affine_rotate( r, angle );
  art_affine_multiply( r, t, r);
  art_affine_translate( t , (x2+x1)/2, (y2+y1)/2 );
  art_affine_multiply( r, r, t);

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
 * Display a dialog box and an OK button
 * When the box is closed, the given callback is called if any
 */
void gcompris_dialog(gchar *str, DialogBoxCallBack dbcb)
{
  GcomprisBoard   *gcomprisBoard = get_current_gcompris_board();
  GnomeCanvasItem *item_dialog = NULL;
  GnomeCanvasItem *item_text   = NULL;
  GnomeCanvasItem *item_text_ok   = NULL;
  GdkPixbuf       *pixmap_dialog = NULL;
  GtkTextIter      iter_start, iter_end;
  GtkTextBuffer   *buffer;
  GtkTextTag      *txt_tag;

  /* If we already running delete the previous one */
  if(rootDialogItem)
    gtk_object_destroy(GTK_OBJECT(rootDialogItem));
  rootDialogItem = NULL;


  /* First pause the board */
  if(gcomprisBoard->plugin->pause_board != NULL)
      gcomprisBoard->plugin->pause_board(TRUE);

  gcompris_bar_hide(TRUE);

  rootDialogItem = GNOME_CANVAS_GROUP(
				      gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							     gnome_canvas_group_get_type (),
							     "x", (double) 0,
							     "y", (double) 0,
							     NULL));
      
  pixmap_dialog = gcompris_load_skin_pixmap("dialogbox.png");

  item_dialog = gnome_canvas_item_new (rootDialogItem,
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

  gtk_signal_connect(GTK_OBJECT(item_dialog), "event",
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
	gtk_object_destroy(GTK_OBJECT(rootDialogItem));
      rootDialogItem = NULL;
      
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

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
