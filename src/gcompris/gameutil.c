/* gcompris - gameutil.c
 *
 * Time-stamp: <2002/02/17 19:48:40 bruno>
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
#include "e-unicode.h"

#define IMAGEEXTENSION ".png"

#define MAX_DESCRIPTION_LENGTH 1000

/* default gnome pixmap directory in which this game tales the icon */
static char *lettersdir = "letters/";

extern GnomeCanvas *canvas;

typedef void (*sighandler_t)(int);

GdkPixbuf *gcompris_load_operation_pixmap(char operation)
{
  gchar *filename;
  GdkPixbuf *pixmap;

  filename = g_strdup_printf("%s/%s/%s%c%s", PACKAGE_DATA_DIR, "gcompris", lettersdir, operation, IMAGEEXTENSION);

  if (!g_file_exists (filename)) {
    g_error (_("Couldn't find file %s !"), filename);
  }

  pixmap = gdk_pixbuf_new_from_file (filename);

  g_free (filename);

  return(pixmap);
}

GdkPixbuf *gcompris_load_number_pixmap(char number)
{
  gchar *filename;
  GdkPixbuf *pixmap;

  filename = g_strdup_printf("%s/%s/%s%c%s", PACKAGE_DATA_DIR, "gcompris", lettersdir, number, IMAGEEXTENSION);

  if (!g_file_exists (filename)) {
    g_error (_("Couldn't find file %s !"), filename);
  }

  pixmap = gdk_pixbuf_new_from_file (filename);

  g_free (filename);

  return(pixmap);
}


GdkPixbuf *gcompris_load_pixmap(char *pixmapfile)
{
  gchar *filename;
  GdkPixbuf *smallnumbers_pixmap;

  filename = g_strdup_printf("%s/%s", PACKAGE_DATA_DIR, pixmapfile);

  if (!g_file_exists (filename)) {
    g_error (_("Couldn't find file %s !"), filename);
  }

  smallnumbers_pixmap = gdk_pixbuf_new_from_file (filename);

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
 *
 */
gint gcompris_item_event_focus(GnomeCanvasItem *item, GdkEvent *event, void *unused)
{

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

  gcomprisBoard->icon_name = xmlGetProp(xmlnode,"icon");

  gcomprisBoard->author = xmlGetProp(xmlnode,"author");

  gcomprisBoard->boarddir = xmlGetProp(xmlnode,"boarddir");

  gcomprisBoard->difficulty = xmlGetProp(xmlnode,"difficulty");

  gcomprisBoard->mandatory_sound_file = xmlGetProp(xmlnode,"mandatory_sound_file");

  gcomprisBoard->name = NULL;
  gcomprisBoard->description = NULL;
  gcomprisBoard->prerequisite = NULL;
  gcomprisBoard->goal = NULL;
  gcomprisBoard->manual = NULL;

  xmlnode = xmlnode->xmlChildrenNode;
  while (xmlnode != NULL) {
    gchar *lang = xmlGetProp(xmlnode,"lang");
    /* get the name of the board */
    if (!strcmp(xmlnode->name, "name")
	&& (lang==NULL
	    || !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	gcomprisBoard->name = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	gcomprisBoard->name = convertUTF8Toisolat1(gcomprisBoard->name);
      }

    /* get the description of the board */
    if (!strcmp(xmlnode->name, "description")
	&& (lang==NULL ||
	    !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	gcomprisBoard->description = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	gcomprisBoard->description = convertUTF8Toisolat1(gcomprisBoard->description);
      }

    /* get the help prerequisite help of the board */
    if (!strcmp(xmlnode->name, "prerequisite")
	&& (lang==NULL ||
	    !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	gcomprisBoard->prerequisite = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	gcomprisBoard->prerequisite = convertUTF8Toisolat1(gcomprisBoard->prerequisite);
      }

    /* get the help goal of the board */
    if (!strcmp(xmlnode->name, "goal")
	&& (lang==NULL ||
	    !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	gcomprisBoard->goal = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	gcomprisBoard->goal = convertUTF8Toisolat1(gcomprisBoard->goal);
      }

    /* get the help user manual of the board */
    if (!strcmp(xmlnode->name, "manual")
	&& (lang==NULL ||
	    !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	gcomprisBoard->manual = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	gcomprisBoard->manual = convertUTF8Toisolat1(gcomprisBoard->manual);
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

  retval = e_utf8_to_locale_string (text);

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

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
