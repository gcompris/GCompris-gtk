/* gcompris - images_selector.c
 *
 * Time-stamp: <2002/05/01 23:05:46 bruno>
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
 * An image selector for gcompris
 *
 */

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "gcompris.h"

#define SOUNDLISTFILE PACKAGE

static gint		 item_event_images_selector(GnomeCanvasItem *item, 
						    GdkEvent *event, 
						    gpointer data);
static gboolean		 read_xml_file(gchar *fname);
static void		 display_image(gchar *imagename);

static gboolean		 images_selector_displayed = FALSE;

static GnomeCanvasItem	*rootitem = NULL;
static GnomeCanvasItem	*item_content = NULL;

static GnomeCanvasItem	*item_selected = NULL;

static ImageSelectorCallBack imageSelectorCallBack = NULL;

/* Represent the limits of the image area */
#define			 DRAWING_AREA_X1 111
#define			 DRAWING_AREA_Y1 14
#define			 DRAWING_AREA_X2 774
#define			 DRAWING_AREA_Y2 500

/* Represent the limits of list area */
#define			 LIST_AREA_X1 18
#define			 LIST_AREA_Y1 16
#define			 LIST_AREA_X2 80
#define			 LIST_AREA_Y2 500

#define HORIZONTAL_NUMBER_OF_IMAGE	10
#define VERTICAL_NUMBER_OF_IMAGE	5
#define IMAGE_GAP			10

#define IMAGE_WIDTH  (DRAWING_AREA_X2-DRAWING_AREA_X1)/HORIZONTAL_NUMBER_OF_IMAGE-IMAGE_GAP
#define IMAGE_HEIGHT (DRAWING_AREA_Y2-DRAWING_AREA_Y1)/VERTICAL_NUMBER_OF_IMAGE-IMAGE_GAP

static GList		*imagelist;	/* List of Images */

static guint		 ix = DRAWING_AREA_X1;
static guint		 iy = DRAWING_AREA_Y1;

/*
 * Main entry point 
 * ----------------
 *
 */

/*
 * Do all the images_selector display and register the events
 */

void gcompris_images_selector_start (GcomprisBoard *gcomprisBoard, gchar *dataset,
				     ImageSelectorCallBack iscb)
{

  GdkPixbuf *pixmap = NULL;
  GnomeCanvasItem *item;
  gint y = 0;
  gint y_start = 0;
  gint x_start = 0;
  GdkFont *gdk_font;
  gchar   *name = NULL;

  if(rootitem)
    return;

  gcompris_bar_hide(TRUE);

  if(gcomprisBoard!=NULL)
    {
      if(gcomprisBoard->plugin->pause_board != NULL)
	  gcomprisBoard->plugin->pause_board(TRUE);
    }

  name = gcomprisBoard->name;
  imageSelectorCallBack=iscb;

  rootitem = \
    gnome_canvas_item_new (gnome_canvas_root(gcompris_get_canvas()),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  pixmap = gcompris_load_pixmap("gcompris/images_selector_bg.png");
  y_start = (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  x_start = (BOARDWIDTH - gdk_pixbuf_get_width(pixmap))/2;
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) x_start,
				"y", (double) y_start,
				NULL);
  y = BOARDHEIGHT - (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  gdk_pixbuf_unref(pixmap);

  gdk_font = gdk_font_load (FONT_TITLE);
  if(!gdk_font)
    // Fallback to a more usual font
    gdk_font = gdk_font_load (FONT_TITLE_FALLBACK);

  y_start += 110;

  pixmap = gcompris_load_pixmap("gcompris/buttons/button_large_selected.png");


  // OK
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH*0.5) - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y - gdk_pixbuf_get_height(pixmap) - 25,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_images_selector,
		     "/ok/");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("OK"),
				"font_gdk", gdk_font,
				"x", (double)  BOARDWIDTH*0.5,
				"y", (double)  y - gdk_pixbuf_get_height(pixmap),
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", COLOR_TEXT_BUTTON,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_images_selector,
		     "ok");
  gdk_pixbuf_unref(pixmap);

  images_selector_displayed = TRUE;

  /* Initial image position */
  ix = DRAWING_AREA_X1;
  iy = DRAWING_AREA_Y1;

  /* Read the given data set */
  read_xml_file(dataset);

  /* Display the images */
  g_list_foreach (imagelist, (GFunc) display_image, NULL);

}

/*
 * Remove the displayed images_selector.
 * Do nothing if none is currently being dislayed
 */
void gcompris_images_selector_stop ()
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  if(gcomprisBoard!=NULL && images_selector_displayed)
    {
      if(gcomprisBoard->plugin->pause_board != NULL)
	{
	  gcomprisBoard->plugin->pause_board(FALSE);
	}
    }

  // Destroy the images_selector box
  if(rootitem!=NULL)
    {
      gtk_object_destroy(GTK_OBJECT(rootitem));
    }
  rootitem = NULL;	  

  g_list_free(imagelist);
  imagelist=NULL;

  gcompris_bar_hide(FALSE);
  images_selector_displayed = FALSE;
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

static void display_image(gchar *imagename)
{

  GdkPixbuf *pixmap = NULL;
  GnomeCanvasItem *item;
  double xratio, yratio;
  double iw, ih;

  if (imagename==NULL)
    return;


  pixmap = gcompris_load_pixmap(imagename);

  iw = IMAGE_WIDTH;
  ih = IMAGE_HEIGHT;

  /* Calc the max to resize width or height */
  xratio = (double) (((double)gdk_pixbuf_get_width(pixmap))/iw);
  yratio = (double) (((double)gdk_pixbuf_get_height(pixmap))/ih);
  xratio = MAX(yratio,xratio);

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap,
				"x", (double)ix,
				"y", (double)iy,
				"width", (double) gdk_pixbuf_get_width(pixmap)/xratio,
				"height", (double) gdk_pixbuf_get_height(pixmap)/xratio,
				"width_set", TRUE, 
				"height_set", TRUE,
				NULL);
  gdk_pixbuf_unref(pixmap);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_images_selector,
		     imagename);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  ix+=IMAGE_WIDTH + IMAGE_GAP;
  if(ix>=DRAWING_AREA_X2-IMAGE_GAP)
    {
      ix=DRAWING_AREA_X1;
      iy+=IMAGE_HEIGHT + IMAGE_GAP;
    }
}

static void select_item(GnomeCanvasItem *item)
{
  GdkPixbuf   *pixmap = NULL;
  
  if(item_selected)
    {
      pixmap = gcompris_load_pixmap("gcompris/buttons/button_large.png");
      gnome_canvas_item_set(item_selected, 
			    "pixbuf", pixmap,
			    NULL);
      gdk_pixbuf_unref(pixmap);
    }

  pixmap = gcompris_load_pixmap("gcompris/buttons/button_large_selected.png");
  gnome_canvas_item_set(item, 
			"pixbuf", pixmap,
			NULL);
  gdk_pixbuf_unref(pixmap);
  item_selected = item;
}

/* Callback when an image is selected */
static gint
item_event_images_selector(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
      if(!strcmp((char *)data, "/ok/"))
	{
	  gcompris_images_selector_stop();
	}
      else
	{
	  if(imageSelectorCallBack!=NULL)
	    imageSelectorCallBack(data);

	  gcompris_images_selector_stop();
	}
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
add_image(xmlNodePtr xmlnode, GNode * child)
{
  char *filename;

  if(/* if the node has no name */
     !xmlnode->name ||
     /* or if the name is not "Data" */
     (g_strcasecmp(xmlnode->name,"Image")!=0)
     )
    return;
  
  /* get the filename of this data */
  filename = xmlGetProp(xmlnode,"filename");

  imagelist = g_list_append (imagelist, filename);
  
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
    add_image(node,NULL);
  }
}



/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set */
static gboolean
read_xml_file(gchar *fname)
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
