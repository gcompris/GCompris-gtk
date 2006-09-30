/* gcompris - images_selector.c
 *
 * Time-stamp: <2006/08/21 23:26:43 bruno>
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

#include "string.h"

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
static gboolean		 read_dataset_directory(gchar *dataset_dir);
static void		 display_image(gchar *imagename, GnomeCanvasItem *rootitem);
static void		 free_stuff (GtkObject *obj, GSList *data);

static gboolean		 images_selector_displayed = FALSE;

static GnomeCanvasItem	*rootitem = NULL;
static GnomeCanvasItem	*current_root_set = NULL;

static GnomeCanvas	*canvas_list_selector; /* The scrolled left part */
static GnomeCanvasItem  *list_bg_item;

static GnomeCanvas	*canvas_image_selector; /* The scrolled right part */
static GnomeCanvasItem  *image_bg_item;

static ImageSelectorCallBack imageSelectorCallBack = NULL;

static gboolean		 display_in_progress;

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

void
gc_selector_images_start (GcomprisBoard *gcomprisBoard, gchar *dataset,
				ImageSelectorCallBack iscb)
{

  GnomeCanvasItem *item, *item2;
  GdkPixbuf	*pixmap = NULL;
  gint		 y = 0;
  gint		 y_start = 0;
  gint		 x_start = 0;
  gchar		*dataseturl = NULL;

  GtkWidget	  *w;

  if(rootitem)
    return;

  gc_bar_hide(TRUE);

  gc_board_pause(TRUE);

  imageSelectorCallBack=iscb;

  rootitem = \
    gnome_canvas_item_new (gnome_canvas_root(gc_get_canvas()),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  images_selector_displayed = TRUE;

  pixmap = gc_skin_pixmap_load("images_selector_bg.png");
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

  pixmap = gc_skin_pixmap_load("button_large.png");

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
					"fill_color_rgba", gc_skin_get_color("gcompris/imageselectbg"),
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

  /* Set the scrollwheel event */
  gtk_signal_connect(GTK_OBJECT(canvas_list_selector), "event",
		     (GtkSignalFunc) item_event_scroll,
		     GNOME_CANVAS(canvas_list_selector));

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
					 "fill_color_rgba", gc_skin_get_color("gcompris/imageselectbg"),
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

  /* Set the scrollwheel event */
  gtk_signal_connect(GTK_OBJECT(canvas_image_selector), "event",
		     (GtkSignalFunc) item_event_scroll,
		     GNOME_CANVAS(canvas_image_selector));

  /*
   * DISPLAY IMAGES
   */

  /* Initial image position */
  ix  = 0;
  iy  = 0;
  isy = 0;

  /* I need  the following :
     -> if dataset is a file read it.
     -> if dataset is a directory, read all xml file in it.
  */
  dataseturl = \
    gc_file_find_absolute(dataset,
				    NULL);

  /* if the file doesn't exist */
  if(g_file_test ((dataseturl), G_FILE_TEST_IS_DIR) )
    {
      g_warning("dataset %s is a directory. Trying to read xml", dataseturl);

      read_dataset_directory(dataseturl);
    }
  else if(dataseturl)
    {
      /* Read the given data set file, local or net */
      read_xml_file(dataseturl);
    }
  else
    {
      /* Network code for dataset directory */
      GSList *filelist = NULL;
      GSList *i = NULL;

      g_free(dataseturl);
      dataseturl = g_strconcat("boards/", dataset, NULL);

      filelist = gc_net_dir_read_name(dataseturl, ".xml");

      for (i = filelist; i != NULL; i = g_slist_next (i))
	{
	  gchar *url = gc_file_find_absolute(i->data,
					     NULL);
	  g_warning("processing dataset=%s\n", (char *)i->data);
	  read_xml_file(url);
	  g_free(url);
	}

      g_slist_free(filelist);

    }
  g_free(dataseturl);

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
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

  item2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("OK"),
				"font", gc_skin_font_title,
				"x", (double)  BOARDWIDTH*0.5,
				"y", (double)  y - gdk_pixbuf_get_height(pixmap) + 15,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gc_skin_color_text_button,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) item_event_images_selector,
		     "/ok/");
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) gc_item_focus_event,
		     item);
  gdk_pixbuf_unref(pixmap);

}

/*
 * Remove the displayed images_selector.
 * Do nothing if none is currently being dislayed
 */
void gc_selector_images_stop ()
{
  GcomprisBoard *gcomprisBoard = gc_board_get_current();

  if(gcomprisBoard!=NULL && images_selector_displayed)
    {
      gc_board_pause(FALSE);
    }

  // Destroy the images_selector box
  if(rootitem!=NULL)
    {
      gtk_object_destroy(GTK_OBJECT(rootitem));
    }
  rootitem = NULL;
  current_root_set = NULL;

  gc_bar_hide(FALSE);
  images_selector_displayed = FALSE;
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

static void
display_image(gchar *imagename, GnomeCanvasItem *root_item)
{

  GdkPixbuf *pixmap = NULL;
  GnomeCanvasItem *item;
  double xratio, yratio;
  double iw, ih;

  if (imagename==NULL || !images_selector_displayed)
    return;

  pixmap = gc_pixmap_load(imagename);

  /* Sad, the image is not found */
  if(!pixmap)
    return;

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
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

  ix+=IMAGE_WIDTH + IMAGE_GAP;
  if(ix>=DRAWING_AREA_X2-DRAWING_AREA_X1-IMAGE_GAP)
    {
      guint iy_calc;
      ix=0;
      iy+=IMAGE_HEIGHT + IMAGE_GAP;

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
static void
display_image_set(gchar *imagename, GSList *imagelist)
{
  GdkPixbuf *pixmap = NULL;
  GnomeCanvasItem *item;
  GnomeCanvasItem *rootitem_set;
  double xratio, yratio;
  double iw, ih;

  if (imagename==NULL || !images_selector_displayed)
    return;

  pixmap = gc_pixmap_load(imagename);

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
		     (GtkSignalFunc) gc_item_focus_event,
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
free_stuff (GtkObject *obj, GSList *list)
{
  while (g_slist_length(list) > 0) {
    g_free(g_slist_nth_data(list,0));
    list = g_slist_remove(list, g_slist_nth_data(list,0));
  }
  g_slist_free(list);
}


/* Callback when an image set is selected */
static gint
item_event_imageset_selector(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  GSList *image_list;
  GnomeCanvasItem *rootitem_set;
  guint imageset_done;

  if(display_in_progress)
    return TRUE;

  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
	{
	  guint last_iy;

	  display_in_progress = TRUE;

	  /* We must display the list of images for this set */
	  image_list = (GSList *)g_object_get_data (G_OBJECT (item), "imagelist");
	  g_return_val_if_fail (image_list != NULL, FALSE);

	  /* We must display the list of images for this set */
	  rootitem_set = (GnomeCanvasItem *)g_object_get_data (G_OBJECT (item), "rootitem");
	  g_return_val_if_fail (rootitem_set != NULL, FALSE);

	  imageset_done = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item), "imageset_done"));

	  /* Hide the previous image set if any */
	  if (current_root_set != NULL) {
	    gnome_canvas_item_hide(current_root_set);
	  }
	  /* Not yet displayed this set */
	  if(!imageset_done) {
	    guint iy_calc;
	    /* Cannot use GINT_TO_POINTER with a constant calculation */
	    /* Set the initial iy value                               */
	    iy_calc = IMAGE_HEIGHT + IMAGE_GAP;
	    g_object_set_data (G_OBJECT (rootitem_set), "iy", GINT_TO_POINTER (iy_calc));

	    g_slist_foreach (image_list, (GFunc) display_image, rootitem_set);
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

	  display_in_progress = FALSE;
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

  if(display_in_progress)
    return TRUE;

  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
      if(!strcmp((char *)data, "/ok/"))
	{
	  gc_selector_images_stop();
	}
      else
	{
	  if(imageSelectorCallBack!=NULL)
	    imageSelectorCallBack(data);

	  gc_selector_images_stop();
	}
    default:
      break;
    }
  return FALSE;

}

/* Callback when a scroll event happens */
static gint
item_event_scroll(GnomeCanvasItem *item, GdkEvent *event, GnomeCanvas *canvas)
{
  int x, y;
  if(!rootitem)
    return FALSE;

  switch (event->type)
    {
    case GDK_SCROLL:
      gnome_canvas_get_scroll_offsets (canvas, &x, &y);
      if ( event->scroll.direction == GDK_SCROLL_UP )
	gnome_canvas_scroll_to (canvas, x, y - 20);
      else if ( event->scroll.direction == GDK_SCROLL_DOWN )
	gnome_canvas_scroll_to (canvas, x, y + 20);

      break;
    default:
      break;
    }
  return FALSE;
}

/*
 * Images PARSING
 * --------------
 */

static void
parseImage (xmlDocPtr doc, xmlNodePtr cur) {
  GcomprisProperties *properties = gc_prop_get();
  gchar  *imageSetName = NULL;
  gchar  *filename;
  gchar	 *pathname = NULL;
  gchar  *absolutepath;
  GSList *imageList = NULL;	/* List of Images */
  gboolean havePathName = FALSE;
  gchar  *type = NULL;
  GDir   *imageset_directory;

  /* get the filename of this ImageSet */
  imageSetName = (gchar *)xmlGetProp(cur, BAD_CAST "filename");
  if (xmlHasProp(cur, BAD_CAST "PathName")){
    pathname = (gchar *)xmlGetProp(cur, BAD_CAST "PathName");
    havePathName = TRUE;
  }
  if (havePathName && pathname[0] == '~'){
    /* replace '~' by home dir */
    pathname = g_strdup_printf("%s%s",g_get_home_dir(), pathname+1);
    if (!g_file_test ((pathname), G_FILE_TEST_IS_DIR)){
       g_warning("In ImageSet %s, the pathname for the home directory '%s' is not found. Skipping the whole ImageSet.",
		 imageSetName, pathname);
      return;
    }
  }

  if (xmlHasProp(cur, BAD_CAST "type")){
    /* lsdir means try all file of directory */
    /* list means just keep old behaviour */
    /* others are extensions to look for */
    type = (gchar *)xmlGetProp(cur, BAD_CAST "type");
  }

  /* Looking for imageSetName */
  /* absolute path -> we check it's here */
  /* relative path -> we check for pathname/imagesetname */
  /*               -> and else for PACKAGE_DATA_DIR/imagesetname */
  if (havePathName) {
    if (!g_path_is_absolute (imageSetName)){
      absolutepath = gc_file_find_absolute(imageSetName, NULL);
    }
    else
      absolutepath = g_strdup(imageSetName);
  }
  else
    absolutepath = gc_file_find_absolute(imageSetName, NULL);

  if(!absolutepath)
    {
      g_warning("In ImageSet %s, an image is not found. Skipping ImageSet...", imageSetName);
      return;
    }

  if ((type == NULL) || (g_ascii_strcasecmp (type,"list")==0)) {
    /* old behaviour : we read the filenames from xml files */

    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
      if ((!xmlStrcmp(cur->name, (const xmlChar *)"Image"))) {
	/* get the filename of this ImageSet */
	filename = (gchar *)xmlGetProp(cur, BAD_CAST "filename");
	if (havePathName){
	  filename = g_strdup_printf("%s/%s",pathname,filename);
	}
	imageList = g_slist_prepend (imageList, filename);
      }
      cur = cur->next;
    }

  }
  else {
    /* new behaviour : we read all file of a directory */
    /* or all files with a given suffix */

    if (!g_file_test ((pathname), G_FILE_TEST_IS_DIR)){
      char *tmpdir;
      tmpdir = g_strdup_printf("%s/%s", properties->package_data_dir, pathname);
      g_free(pathname);
      pathname = tmpdir;
      if (!g_file_test ((pathname), G_FILE_TEST_IS_DIR)){
        g_warning("In ImageSet %s, directory %s is not found. Skipping all the ImageSet...", absolutepath, pathname);
        return;
      }
    }
    imageset_directory = g_dir_open (pathname, 0, NULL);
    const gchar * onefile;
    while ((onefile = g_dir_read_name(imageset_directory))){
      if ((g_ascii_strcasecmp (type,"lsdir") != 0) &&
	  (!g_str_has_suffix (onefile, type))){
	continue;
      }
      filename = g_strdup_printf("%s/%s", pathname, onefile);
      if (!g_file_test ((filename), G_FILE_TEST_IS_REGULAR)){
	continue;
      }

      {
	/* Add a % before a % to avoid it being used as a format arg */
	gchar **tmp;
	gchar *filename2;
	if((tmp = g_strsplit(filename, "%", -1)))
	  {
	    filename2 = g_strjoinv("%%", tmp);
	    g_strfreev(tmp);
	    g_free(filename);
	    filename = filename2;
	  }
      }
      imageList = g_slist_prepend (imageList, filename);
    }
    g_dir_close(imageset_directory);
  }

  /* do not display if there is nothing to display */
  if (imageList != NULL) /* g_slist is not empty */
    display_image_set(imageSetName, imageList);

  g_free(absolutepath);
  if(pathname)
    g_free(pathname);
  if(type)
    g_free(type);

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
 * dump any old data we have in memory if we can load a new set
 *
 * \param fname is an absolute file name
 *
 */
static gboolean
read_xml_file(gchar *fname)
{
  /* pointer to the new doc */
  xmlDocPtr doc;

  g_return_val_if_fail(fname!=NULL, FALSE);

  doc = gc_net_load_xml(fname);

  /* in case something went wrong */
  if(!doc)
    return FALSE;

  if(/* if there is no root element */
     !doc->children ||
     /* if it doesn't have a name */
     !doc->children->name ||
     /* if it isn't the good node */
     g_strcasecmp((gchar *)doc->children->name, "ImageSetRoot")!=0) {
    xmlFreeDoc(doc);
    return FALSE;
  }

  /* parse our document and replace old data */
  parse_doc(doc);

  xmlFreeDoc(doc);

  return TRUE;
}


/** read an xml file into our memory structures and update our view,
 *   dump any old data we have in memory if we can load a new set
 *
 * \return TRUE is the parsing occurs, FALSE instead
 *
 */
static gboolean
read_dataset_directory(gchar *dataset_dir)
{
  GDir *dataset_directory = g_dir_open (dataset_dir, 0, NULL);
  const gchar *fname;
  gchar *absolute_fname;

  if(!dataset_directory)
    return FALSE;

  while ((fname = g_dir_read_name(dataset_directory))) {
    /* skip files without ".xml" */
    if (!g_str_has_suffix (fname,".xml")){
      g_warning("skipping file not in .xml : %s", fname);
      continue;
    }

    absolute_fname = g_strdup_printf("%s/%s", dataset_dir, fname);
    g_warning("Reading dataset file %s", absolute_fname);

    if (!g_file_test ((absolute_fname), G_FILE_TEST_EXISTS))
      continue;

    /* parse the new file and put the result into newdoc */

    /* pointer to the new doc */
    xmlDocPtr doc;

    doc = xmlParseFile(absolute_fname);

    /* in case something went wrong */
    if(!doc)
      continue;

    if(/* if there is no root element */
       !doc->children ||
       /* if it doesn't have a name */
       !doc->children->name ||
       /* if it isn't the good node */
       g_strcasecmp((gchar *)doc->children->name, "ImageSetRoot")!=0) {
      xmlFreeDoc(doc);
      continue;
    }

    /* parse our document and replace old data */
    g_warning("Parsing dataset : %s \n", absolute_fname);
    parse_doc(doc);

    xmlFreeDoc(doc);
  }

  g_dir_close(dataset_directory);

  return TRUE;
}
