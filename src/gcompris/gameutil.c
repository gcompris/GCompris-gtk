/* gcompris - gameutil.c
 *
 * Time-stamp: <2001/11/05 20:45:23 bcoudoin>
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

extern GnomeCanvas *canvas;

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

/**
 * Set the focus of the given image (highlight or not)
 *
 */
void gcompris_set_image_focus(GdkPixbuf *pixbuf1, gboolean focus)
{
}

/**
 * Callback over a canvas item, this function will highlight the focussed item
 *
 */
gint gcompris_item_event_focus(GnomeCanvasItem *item, GdkEvent *event, GdkPixbuf *pixbuf)
{
  /* I Have not been able to reinplement this feature with gdk-pixbuf. Help wanted.
     It worked fine with the imlib */
  /*
  gint width;
  gint height;
  GdkPixbuf *dest;

  width = gdk_pixbuf_get_width(pixbuf);
  height = gdk_pixbuf_get_height(pixbuf);

  printf("creation temp pixbuf\n");
  dest = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, width, height);

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      printf("item_event_focus GDK_ENTER_NOTIFY:\n");
      gdk_pixbuf_composite (pixbuf, dest,
			    0, 0, width, height,
			    0, 0,
			    (double) 1,
			    (double) 1,
			    GDK_INTERP_BILINEAR, 255);
      

      gnome_canvas_item_set (item,
			     "pixbuf", dest,
			     NULL);
      break;
    case GDK_LEAVE_NOTIFY: 
      printf("item_event_focus GDK_LEAVE_NOTIFY:\n");
      gdk_pixbuf_composite (pixbuf, dest,
			    0, 0, width, height,
			    0, 0,
			    (double) 1,
			    (double) 1,
			    GDK_INTERP_BILINEAR, 127);
      
      gnome_canvas_item_set (item,
			     "pixbuf", dest,
			     NULL);
      break;
    default:
      break;
    }

  gdk_pixbuf_unref (dest);
  */
  return FALSE;
}

/* =====================================================================
 * Play a list of OGG sound files. The list must be NULL terminated
 * should have used threads instead of fork + exec calls
   ======================================================================*/
void gcompris_play_ogg(char *sound, ...) {
	va_list ap;
	char * s = NULL;
	char *argv[20];
	char locale[3];
	pid_t   pid;
	int argc = 0;

	if (!gcompris_get_properties()->fx)
		return;

	strncpy(locale,gcompris_get_locale(),2);
	locale[2] = 0; // because strncpy does not put a '\0' at the end of the string

	pid = fork ();
        if (pid > 0) { // go back to gcompris
                return;
        } else if (pid == 0) { // child process
		argv[0] = "ogg123";
		argv[1] = "-v";
		argv[2] = g_strdup_printf("%s/%s/%s.ogg", PACKAGE_DATA_DIR "/sounds", locale, sound);
		argc = 3;
		va_start( ap, sound);
		while( (s = va_arg (ap, char *))) {
			argv[argc] = g_strdup_printf("%s/%s/%s.ogg", PACKAGE_DATA_DIR "/sounds", locale, s);
			if (!g_file_exists (argv[argc])) {
				g_error (_("Couldn't find file %s !"), argv[argc]);
				g_free(argv[argc]);
				continue;
			}
			argc ++;
		}
		va_end(ap);
		argv[argc] = NULL;
		execvp( "ogg123", argv);
        } else {
                fprintf(stderr, "Unable to fork\n");
        }
}

/* Play a sound installed in the Gnome sound list */
void gcompris_play_sound (const char *soundlistfile, const char *which)
{
  gchar *filename;

  filename = g_strdup_printf("%s/%s.wav", PACKAGE_SOUNDS_DIR, which);

  if (!g_file_exists (filename)) {
    g_error (_("Couldn't find file %s !"), filename);
  }
  if (gcompris_get_properties()->fx) {
    gnome_sound_play (filename);
  }

  g_free (filename);
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

  xmlnode = xmlnode->xmlChildrenNode;
  while (xmlnode != NULL) {
    gchar *lang = xmlGetProp(xmlnode,"lang");
    /* get the name of the board */
    if (!strcmp(xmlnode->name, "name")
	&& (lang==NULL
	    || !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	const char *inptr;
	size_t inleft;
	char *outptr;
	size_t outleft;
	gint retval;

	gcomprisBoard->name = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);

	inptr   = (const char *) gcomprisBoard->name;
	outptr  = (char *) g_malloc(MAX_DESCRIPTION_LENGTH);
	inleft  = xmlUTF8Strsize(gcomprisBoard->name, MAX_DESCRIPTION_LENGTH);
	outleft = MAX_DESCRIPTION_LENGTH;
	// Conversion to ISO-8859-1
	retval = UTF8Toisolat1(outptr, &outleft, gcomprisBoard->name, &inleft);
	if(retval==0)
	  {
 	    g_free(gcomprisBoard->name);
	    gcomprisBoard->name=outptr;
	    gcomprisBoard->name[outleft]='\0';
	  }
	else
	  g_free(outptr);

      }

    /* get the description of the board */
    if (!strcmp(xmlnode->name, "description")
	&& (lang==NULL ||
	    !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2))) 
      {
	const char *inptr;
	size_t inleft;
	char *outptr;
	size_t outleft;
	gint retval;

	gcomprisBoard->description = xmlNodeListGetString(doc, xmlnode->xmlChildrenNode, 1);
	
	inptr   = (const char *) gcomprisBoard->description;
	outptr  = (char *) g_malloc(MAX_DESCRIPTION_LENGTH);
	inleft  = xmlUTF8Strsize(gcomprisBoard->description, MAX_DESCRIPTION_LENGTH);
	outleft = MAX_DESCRIPTION_LENGTH;
	// Conversion to ISO-8859-1
	retval = UTF8Toisolat1(outptr, &outleft, gcomprisBoard->description, &inleft);
	if(retval==0)
	  {
 	    g_free(gcomprisBoard->description);
	    gcomprisBoard->description=outptr;
	    gcomprisBoard->description[outleft]='\0';
	  }
	else
	  g_free(outptr);

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
#define MAX_LENGTH 512
  const char *inptr;
  size_t inleft;
  char *outptr;
  size_t outleft;
  gint retval;
 // this should never happen, it does often !!
  if (text == NULL)
  	return NULL;

  inptr   = (const char *) text;
  outptr  = (char *) g_malloc(MAX_LENGTH);
  inleft  = xmlUTF8Strsize(text, MAX_LENGTH);
  outleft = MAX_LENGTH;
  // Conversion to ISO-8859-1
  retval = UTF8Toisolat1(outptr, &outleft, text, &inleft);
  if(retval==0)  {
 	    g_free(text);
	    text = outptr;
	    text[outleft]='\0';
	  } else
	  	g_free(outptr);

  return text;
}

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
