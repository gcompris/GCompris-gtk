/* gcompris - images_selector.c
 *
 * Time-stamp: <2005/02/09 00:56:51 bruno>
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
static gint		 item_event_imageset_selector(GnomeCanvasItem *item, 
						      GdkEvent *event, 
						      gpointer data);
static gint		 item_event_scroll(GnomeCanvasItem *item,
					   GdkEvent *event,
					   GnomeCanvas *canvas);
static gboolean		 read_xml_file(gchar *fname);
static void		 display_image(gchar *imagename, GnomeCanvasItem *rootitem);
static void		 free_stuff (GtkObject *obj, GList *data);

static gboolean		 images_selector_displayed = FALSE;

static GnomeCanvasItem	*rootitem = NULL;
static GnomeCanvasItem	*current_root_set = NULL;
static GnomeCanvasItem	*item_content = NULL;

static GnomeCanvas	*canvas_list_selector; /* The scrolled left part */
static GnomeCanvasItem  *list_bg_item;

static GnomeCanvas	*canvas_image_selector; /* The scrolled right part */
static GnomeCanvasItem  *image_bg_item;

static ImageSelectorCallBack imageSelectorCallBack = NULL;

/* Represent the limits of the image area */
#define	DRAWING_AREA_X1	111.0
#define DRAWING_AREA_Y1	14.0
#define DRAWING_AREA_X2	774.0
#define DRAWING_AREA_Y2	500.0

/* Represent the limits of list area */
#define	LIST_AREA_X1	18.0
#define	LIST_AREA_Y1	16.0
#define	LIST_AREA_X2	80.0
#define	LIST_AREA_Y2	500.0
#define VERTICAL_NUMBER_OF_LIST_IMAGE	6

#define HORIZONTAL_NUMBER_OF_IMAGE	8
#define VERTICAL_NUMBER_OF_IMAGE	4
#define IMAGE_GAP			10

#define IMAGE_WIDTH  (DRAWING_AREA_X2-DRAWING_AREA_X1)/HORIZONTAL_NUMBER_OF_IMAGE-IMAGE_GAP
#define IMAGE_HEIGHT (DRAWING_AREA_Y2-DRAWING_AREA_Y1)/VERTICAL_NUMBER_OF_IMAGE-IMAGE_GAP

#define LIST_IMAGE_WIDTH  LIST_AREA_X2-LIST_AREA_X1-IMAGE_GAP
#define LIST_IMAGE_HEIGHT (LIST_AREA_Y2-LIST_AREA_Y1)/VERTICAL_NUMBER_OF_LIST_IMAGE-IMAGE_GAP

static guint		 ix;
static guint		 iy;
static guint		 isy;

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

  GnomeCanvasItem *item, *item2;
  GdkPixbuf	*pixmap = NULL;
  gint		 y = 0;
  gint		 y_start = 0;
  gint		 x_start = 0;
  gchar		*name = NULL;

  GtkWidget	  *w;

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

  pixmap = gcompris_load_skin_pixmap("images_selector_bg.png");
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

  y_start += 110;

  pixmap = gcompris_load_skin_pixmap("button_large.png");

  /*
   * Create the list scrollbar
   * -------------------------
   */
  canvas_list_selector     = GNOME_CANVAS(gnome_canvas_new ());

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_widget_get_type (),
			 "widget", GTK_WIDGET(canvas_list_selector),
			 "x", (double) LIST_AREA_X1,
			 "y", (double) LIST_AREA_Y1,
			 "width",  LIST_AREA_X2 - LIST_AREA_X1,
			 "height", LIST_AREA_Y2 - LIST_AREA_Y1 - 35.0,
			 NULL);

  gtk_widget_show (GTK_WIDGET(canvas_list_selector));

  /* Set the new canvas to the background color or it's white */
  list_bg_item = gnome_canvas_item_new (gnome_canvas_root(canvas_list_selector),
					gnome_canvas_rect_get_type (),
					"x1", (double) 0,
					"y1", (double) 0,
					"x2", (double) LIST_AREA_X2 - LIST_AREA_X1,
					"y2", (double) LIST_AREA_Y2 - LIST_AREA_Y1,
					"fill_color_rgba", gcompris_skin_get_color("gcompris/imageselectbg"),
					NULL);


  w = gtk_vscrollbar_new (GTK_LAYOUT(canvas_list_selector)->vadjustment);

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_widget_get_type (),
			 "widget", GTK_WIDGET(w),
			 "x", (double) LIST_AREA_X2 - 5.0,
			 "y", (double) LIST_AREA_Y1,
			 "width", 30.0,
			 "height", LIST_AREA_Y2 - LIST_AREA_Y1 - 20.0,
			 NULL);
  gtk_widget_show (w);
  gnome_canvas_set_center_scroll_region (GNOME_CANVAS (canvas_list_selector), FALSE);

  /*
   * Create the image scrollbar
   * --------------------------
   */
  canvas_image_selector     = GNOME_CANVAS(gnome_canvas_new ());

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_widget_get_type (),
			 "widget", GTK_WIDGET(canvas_image_selector),
			 "x", (double) DRAWING_AREA_X1,
			 "y", (double) DRAWING_AREA_Y1,
			 "width",  DRAWING_AREA_X2 - DRAWING_AREA_X1,
			 "height", DRAWING_AREA_Y2 - DRAWING_AREA_Y1 - 35.0,
			 NULL);

  gtk_widget_show (GTK_WIDGET(canvas_image_selector));

  /* Set the new canvas to the background color or it's white */
  image_bg_item = gnome_canvas_item_new (gnome_canvas_root(canvas_image_selector),
					gnome_canvas_rect_get_type (),
					"x1", (double) 0,
					"y1", (double) 0,
					"x2", (double) DRAWING_AREA_X2 - DRAWING_AREA_X1,
					"y2", (double) DRAWING_AREA_Y2 - DRAWING_AREA_Y1,
					 "fill_color_rgba", gcompris_skin_get_color("gcompris/imageselectbg"),
					NULL);


  w = gtk_vscrollbar_new (GTK_LAYOUT(canvas_image_selector)->vadjustment);

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_widget_get_type (),
			 "widget", GTK_WIDGET(w),
			 "x", (double) DRAWING_AREA_X2 - 5.0,
			 "y", (double) DRAWING_AREA_Y1,
			 "width", 30.0,
			 "height", DRAWING_AREA_Y2 - DRAWING_AREA_Y1 - 20.0,
			 NULL);
  gtk_widget_show (w);
  gnome_canvas_set_center_scroll_region (GNOME_CANVAS (canvas_image_selector), FALSE);


  /*
   * OK Button
   * ---------
   */
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH*0.5) - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y - gdk_pixbuf_get_height(pixmap) - 10,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_images_selector,
		     "/ok/");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  item2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("OK"),
				"font", gcompris_skin_font_title,
				"x", (double)  BOARDWIDTH*0.5,
				"y", (double)  y - gdk_pixbuf_get_height(pixmap) + 15,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_text_button,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) item_event_images_selector,
		     "/ok/");
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     item);
  gdk_pixbuf_unref(pixmap);

  images_selector_displayed = TRUE;

  /* Initial image position */
  ix  = 0;
  iy  = 0;
  isy = 0;

  /* Read the given data set */
  read_xml_file(dataset);

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
  current_root_set = NULL;

  gcompris_bar_hide(FALSE);
  images_selector_displayed = FALSE;
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

static void display_image(gchar *imagename, GnomeCanvasItem *root_item)
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

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(root_item),
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
  if(ix>=DRAWING_AREA_X2-DRAWING_AREA_X1-IMAGE_GAP)
    {
      guint iy_calc;
      ix=0;
      iy+=IMAGE_HEIGHT + IMAGE_GAP;
      gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas_image_selector), 0, 0, 
				      DRAWING_AREA_X2- DRAWING_AREA_X1, 
				      iy + IMAGE_HEIGHT + IMAGE_GAP);

      /* Cannot use GINT_TO_POINTER with a constant calculation */
      iy_calc = iy + IMAGE_HEIGHT + IMAGE_GAP;
      g_object_set_data (G_OBJECT (root_item), "iy", GINT_TO_POINTER (iy_calc));

      if(iy>=DRAWING_AREA_Y2-DRAWING_AREA_Y1) {
	gnome_canvas_item_set(image_bg_item,
			      "y2", (double) iy + IMAGE_HEIGHT + IMAGE_GAP,
			      NULL);
      }
    }
}

/*
 * Same as display_image but for the dataset
 * The imagelist contains the list of images to be displayed when this dataset is selected
 */
static void display_image_set(gchar *imagename, GList *imagelist)
{

  GdkPixbuf *pixmap = NULL;
  GnomeCanvasItem *item;
  GnomeCanvasItem *rootitem_set;
  double xratio, yratio;
  double iw, ih;

  if (imagename==NULL)
    return;

  pixmap = gcompris_load_pixmap(imagename);

  iw = LIST_IMAGE_WIDTH;
  ih = LIST_IMAGE_HEIGHT;

  /* Calc the max to resize width or height */
  xratio = (double) (((double)gdk_pixbuf_get_width(pixmap))/iw);
  yratio = (double) (((double)gdk_pixbuf_get_height(pixmap))/ih);
  xratio = MAX(yratio,xratio);

  item = gnome_canvas_item_new (gnome_canvas_root(canvas_list_selector),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap,
				"x", (double)5,
				"y", (double)isy,
				"width", (double) gdk_pixbuf_get_width(pixmap)/xratio,
				"height", (double) gdk_pixbuf_get_height(pixmap)/xratio,
				"width_set", TRUE, 
				"height_set", TRUE,
				NULL);
  gdk_pixbuf_unref(pixmap);
  g_object_set_data (G_OBJECT (item), "imagelist", imagelist);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_imageset_selector,
		     imagename);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  isy+=LIST_IMAGE_HEIGHT + IMAGE_GAP;

  gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas_list_selector), 0, 0, 
				  LIST_AREA_X2- LIST_AREA_X1, 
				  isy - IMAGE_GAP);

  if(isy>=LIST_AREA_Y2-LIST_AREA_Y1) {
    gnome_canvas_item_set(list_bg_item,
			  "y2", (double)isy + LIST_IMAGE_HEIGHT + IMAGE_GAP,
			  NULL);
  }

  /* Create a root item to put the image list in it */
  rootitem_set = \
    gnome_canvas_item_new (gnome_canvas_root(canvas_image_selector),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  g_object_set_data (G_OBJECT (item), "rootitem", rootitem_set);
  g_object_set_data (G_OBJECT (item), "imageset_done", GINT_TO_POINTER (0));
  g_signal_connect (item, "destroy",
		    G_CALLBACK (free_stuff),
		    imagelist);

}

static void
free_stuff (GtkObject *obj, GList *list)
{
  while (g_list_length(list) > 0) {
    g_free(g_list_nth_data(list,0));
    list = g_list_remove(list, g_list_nth_data(list,0));
  }
  g_list_free(list);
}


/* Callback when an image set is selected */
static gint
item_event_imageset_selector(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  GList *image_list;
  GnomeCanvasItem *rootitem_set;
  guint imageset_done;

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
	{
	  guint last_iy;

	  /* We must display the list of images for this set */
	  image_list = (GList *)g_object_get_data (G_OBJECT (item), "imagelist");
	  g_return_if_fail (image_list != NULL);

	  /* We must display the list of images for this set */
	  rootitem_set = (GnomeCanvasItem *)g_object_get_data (G_OBJECT (item), "rootitem");
	  g_return_if_fail (rootitem_set != NULL);

	  imageset_done = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item), "imageset_done"));

	  /* Hide the previous image set if any */
	  if (current_root_set != NULL) {
	    gnome_canvas_item_hide(current_root_set);
	  }
	  /* Not yet displayed this set */
	  if(!imageset_done) {
	    g_list_foreach (image_list, (GFunc) display_image, rootitem_set);
	    g_object_set_data (G_OBJECT (item), "imageset_done", GINT_TO_POINTER (1));
	  }

	  /* Set the image scrollbar back to its max position */
	  last_iy = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (rootitem_set), "iy"));
	  gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas_image_selector), 0, 0, 
					  DRAWING_AREA_X2- DRAWING_AREA_X1, 
					  last_iy - IMAGE_GAP);
	  
	  if(last_iy>=DRAWING_AREA_Y2-DRAWING_AREA_Y1) {
	    gnome_canvas_item_set(image_bg_item,
				  "y2", (double) last_iy + IMAGE_HEIGHT + IMAGE_GAP,
				  NULL);
	  }


	  gnome_canvas_item_show(rootitem_set);
	  current_root_set = rootitem_set;

	  /* Back to the initial image position */
	  ix  = 0;
	  iy  = 0;
	}
    default:
      break;
    }
  return FALSE;

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

/* Callback when a scroll event happens */
/* FIXME This doesn't work. */
static gint
item_event_scroll(GnomeCanvasItem *item, GdkEvent *event, GnomeCanvas *canvas)
{
  int x, y;
  if(!rootitem)
    return;

  switch (event->type) 
    {
    case GDK_SCROLL:
      gnome_canvas_get_scroll_offsets (canvas, &x, &y);
      if ( event->scroll.direction == GDK_SCROLL_UP )
	gnome_canvas_scroll_to (canvas, x, y - 20);
      else if ( event->scroll.direction == GDK_SCROLL_DOWN )
	gnome_canvas_scroll_to (canvas, x, y + 20);

      break;
    }
}

/*
 * Images PARSING
 * --------------
 */

void
parseImage (xmlDocPtr doc, xmlNodePtr cur) {
  gchar *imageSetName = NULL;
  gchar *filename;
  GList	*imageList = NULL;	/* List of Images */

  /* get the filename of this ImageSet */
  imageSetName = xmlGetProp(cur,"filename");

  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    if ((!xmlStrcmp(cur->name, (const xmlChar *)"Image"))) {
      /* get the filename of this ImageSet */
      filename = xmlGetProp(cur,"filename");
      imageList = g_list_append (imageList, filename);
    }
    cur = cur->next;
  }

  display_image_set(imageSetName, imageList);

  return;
}

static void
parse_doc(xmlDocPtr doc) {
  xmlNodePtr cur;

  cur = xmlDocGetRootElement(doc);	
  if (cur == NULL) {
    fprintf(stderr,"empty document\n");
    xmlFreeDoc(doc);
    return;
  }
  cur = cur->xmlChildrenNode;

  while (cur != NULL) {
    if ((!xmlStrcmp(cur->name, (const xmlChar *)"ImageSet"))){
      parseImage (doc, cur);
    }
    
    cur = cur->next;
  }
	
  return;
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
  if(!g_file_test ((fname), G_FILE_TEST_EXISTS)) 
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
     /* if it isn't the good node */
     g_strcasecmp(doc->children->name,"ImageSetRoot")!=0) {
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
