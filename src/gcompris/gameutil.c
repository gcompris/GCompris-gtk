/* gcompris - gameutil.c
 *
 * Time-stamp: <2002/12/15 23:05:15 bruno>
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

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "gcompris.h"

#define IMAGEEXTENSION ".png"

#define MAX_DESCRIPTION_LENGTH 1000

/* default gnome pixmap directory in which this game tales the icon */
static char *lettersdir = "letters/";

static GnomeCanvasGroup *rootDialogItem = NULL;
static gint item_event_ok(GnomeCanvasItem *item, GdkEvent *event, DialogBoxCallBack dbcb);

extern GnomeCanvas *canvas;

typedef void (*sighandler_t)(int);

/*
 * Load the given pixmap in the gcompris number/letter directory
 * operations can also be loaded 
 */
GdkPixbuf *gcompris_load_number_pixmap(char number)
{
  gchar *filename;
  GdkPixbuf *pixmap;
  gchar     *file = NULL;

  switch(number)
    {
    case '+':
      file = "plus";
      break;
    case ':':
      file = "div";
      break;
    case '-':
      file = "minus";
      break;
    case '*':
    case 'x':
      file = "by";
      break;
    case '=':
      file = "equal";
      break;
    case '?':
      file = "question";
      break;
    default:
      break;
    }

  if(file == NULL)
    filename = g_strdup_printf("%s/%s/%s%c%s", PACKAGE_DATA_DIR, "gcompris",
			       lettersdir, number, IMAGEEXTENSION);
  else
    filename = g_strdup_printf("%s/%s/%s%s%s", PACKAGE_DATA_DIR, "gcompris",
			       lettersdir, file, IMAGEEXTENSION);

  if (!g_file_test ((filename), G_FILE_TEST_EXISTS)) {
    g_error (_("Couldn't find file %s !"), filename);
  }

  pixmap = gdk_pixbuf_new_from_file (filename, NULL);

  g_free (filename);

  return(pixmap);
}


GdkPixbuf *gcompris_load_pixmap(char *pixmapfile)
{
  gchar *filename;
  GdkPixbuf *smallnumbers_pixmap;

  filename = g_strdup_printf("%s/%s", PACKAGE_DATA_DIR, pixmapfile);

  if (!g_file_test ((filename), G_FILE_TEST_EXISTS)) {
    g_error (_("Couldn't find file %s !"), filename);
  }

  smallnumbers_pixmap = gdk_pixbuf_new_from_file (filename, NULL);

  if(!smallnumbers_pixmap)
    g_warning("Loading image returned a null pointer");

  g_free (filename);

  return(smallnumbers_pixmap);
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

  gtk_object_get (GTK_OBJECT (item), "pixbuf", &pixbuf, NULL);
  g_return_if_fail (pixbuf != NULL);

  switch (focus)
    {
    case TRUE:
      dest = make_hc_pixbuf(pixbuf, 30);
      gnome_canvas_item_set (item,
			     "pixbuf", dest,
			     NULL);

      break;
    case FALSE:
      dest = make_hc_pixbuf(pixbuf, -30);
      gnome_canvas_item_set (item,
			     "pixbuf", dest,
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
 printf("reactivate_newline %s\n", str); 
  if(str==NULL)
	return(NULL);
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
  gcomprisBoard->mode = xmlGetProp(xmlnode,"mode");

  gcomprisBoard->name = xmlGetProp(xmlnode,"name");

  gcomprisBoard->icon_name = xmlGetProp(xmlnode,"icon");

  gcomprisBoard->author = xmlGetProp(xmlnode,"author");

  gcomprisBoard->boarddir = xmlGetProp(xmlnode,"boarddir");

  gcomprisBoard->difficulty = xmlGetProp(xmlnode,"difficulty");

  gcomprisBoard->mandatory_sound_file = xmlGetProp(xmlnode,"mandatory_sound_file");

  gcomprisBoard->title = NULL;
  gcomprisBoard->description = NULL;
  gcomprisBoard->prerequisite = NULL;
  gcomprisBoard->goal = NULL;
  gcomprisBoard->manual = NULL;
  gcomprisBoard->credit = NULL;

  xmlnode = xmlnode->xmlChildrenNode;
  while (xmlnode != NULL) {
    gchar *lang = xmlGetProp(xmlnode,"lang");
    /* get the title of the board */
    if (!strcmp(xmlnode->name, "title")
	&& (lang==NULL
	    || !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	gcomprisBoard->title = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
      }

    /* get the description of the board */
    if (!strcmp(xmlnode->name, "description")
	&& (lang==NULL ||
	    !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	gcomprisBoard->description = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
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
GcomprisBoard *gcompris_read_xml_file(char *fname)
{
  char *filename;
  /* pointer to the new doc */
  xmlDocPtr doc;
  GcomprisBoard *gcomprisBoard = NULL;

  gcomprisBoard = g_malloc (sizeof (GcomprisBoard));

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
	  return NULL;
	}

    }

  /* parse the new file and put the result into newdoc */
  doc = xmlParseFile(filename);

  /* in case something went wrong */
  if(!doc) {
    g_warning("Oups, the parsing of %s failed", filename);
    return NULL;
  }
  
  if(/* if there is no root element */
     !doc->children ||
     /* if it doesn't have a name */
     !doc->children->name ||
     /* if it isn't a GCompris node */
     g_strcasecmp(doc->children->name,"GCompris")!=0) {
    xmlFreeDoc(doc);
    g_warning("Oups, the file %s is not for gcompris", filename);
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

  /* Fixed since I use the canvas own pixel_per_unit scheme */
  gcomprisBoard->width  = BOARDWIDTH;
  gcomprisBoard->height = BOARDHEIGHT;

  return gcomprisBoard;
}

/* ==================================== */
/* translates UTF8 charset to iso Latin1 */
gchar * convertUTF8Toisolat1(gchar * text) {
  gchar *retval;
  gint i;

  // this should never happen, it does often !!
  if (text == NULL)
    return NULL;

  //  retval = e_utf8_to_locale_string (text);

  if(retval != NULL)  {
    g_free(text);
    text = retval;

    // if we find \n on 2 char, recreate a real \n
    i=0;
    while(text[i]!='\0')
      {
	if(text[i]=='\\' && text[i+1]=='n')
	  {
	    text[i]=' ';
	    text[i+1]='\n';
	  }
	++i;
      }
  }

  return text;
}

/* ======================================= */
void item_absolute_move(GnomeCanvasItem *item, int x, int y) {
  double dx1, dy1, dx2, dy2;
  gnome_canvas_item_get_bounds(item, &dx1, &dy1, &dx2, &dy2);
  gnome_canvas_item_move(item, ((double)x)-dx1, ((double)y)-dy1);
}

/*
 * Display a dialog box and an OK button
 * When the box is closed, the given callback is called if any
 */
void gcompris_dialog(gchar *str, DialogBoxCallBack dbcb)
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();
  GnomeCanvasItem *item_dialog = NULL;
  GnomeCanvasItem *item_text   = NULL;
  GdkPixbuf *pixmap_dialog = NULL;
  GtkWidget *gtktext;
  GdkFont *gdk_font;

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
      
  pixmap_dialog = gcompris_load_pixmap("gcompris/dialogbox.png");

  item_dialog = gnome_canvas_item_new (rootDialogItem,
				       gnome_canvas_pixbuf_get_type (),
				       "pixbuf", pixmap_dialog,
				       "x", (double) (BOARDWIDTH - gdk_pixbuf_get_width(pixmap_dialog))/2,
				       "y", (double) (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap_dialog))/2,
				      NULL);

  gtk_signal_connect(GTK_OBJECT(item_dialog), "event",
		     (GtkSignalFunc) item_event_ok,
		     dbcb);

  gdk_font = gdk_font_load (FONT_SUBTITLE);
  if(!gdk_font)
    // Fallback to a more usual font
    gdk_font = gdk_font_load (FONT_SUBTITLE_FALLBACK);

  item_text = gnome_canvas_item_new (rootDialogItem,
			 gnome_canvas_text_get_type (),
			 "text", str,
			 "font_gdk", gdk_font,
			 "x", (double) (BOARDWIDTH/2),
			 "y", (double) (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap_dialog))/2 +
			 100,
			 "anchor", GTK_ANCHOR_NORTH,
			 "fill_color", "white",
			 "justification", GTK_JUSTIFY_CENTER,
			 NULL);

  gtk_signal_connect(GTK_OBJECT(item_text), "event",
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
