/* gcompris - images_selector.c
 *
 * Copyright (C) 2000, 2008 Bruno Coudoin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
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
#include "gc_core.h"

#define SOUNDLISTFILE PACKAGE

static gint		 item_event_images_selector(GooCanvasItem *item,
						    GooCanvasItem *target,
						    GdkEvent *event,
						    gchar *data);
static gint		 item_event_imageset_selector(GooCanvasItem *item,
						      GdkEvent *event,
						      gpointer data);
static void		 item_event_scroll(GtkAdjustment *adj,
					   GooCanvas *canvas);
static gboolean          read_xml_file(gchar *fname);
static gboolean		 read_dataset_directory(gchar *dataset_dir);
static void		 display_image(gchar *imagename, GooCanvasItem *rootitem);
static void		 free_stuff (GSList *data);

static gboolean		 images_selector_displayed = FALSE;

static GooCanvasItem	*rootitem = NULL;
static GooCanvasItem	*current_root_set = NULL;

static GtkWidget	*canvas_list_selector; /* The scrolled left part */
static GooCanvasItem	*list_bg_item;

static GtkWidget	*canvas_image_selector; /* The scrolled right part */
static GooCanvasItem    *image_bg_item;

static ImageSelectorCallBack imageSelectorCallBack = NULL;
static void *current_user_context = NULL;

static gboolean		 display_in_progress;

/* Represent the limits of the image area */
#define	DRAWING_AREA_X1	111.0
#define DRAWING_AREA_Y1	14.0
#define DRAWING_AREA_X2	774.0
#define DRAWING_AREA_Y2	460.0

/* Represent the limits of list area */
#define	LIST_AREA_X1	18.0
#define	LIST_AREA_Y1	16.0
#define	LIST_AREA_X2	80.0
#define	LIST_AREA_Y2	500.0
#define VERTICAL_NUMBER_OF_LIST_IMAGE	6

#define HORIZONTAL_NUMBER_OF_IMAGE	8
#define VERTICAL_NUMBER_OF_IMAGE	6
#define IMAGE_GAP			10

#define IMAGE_WIDTH  (DRAWING_AREA_X2-DRAWING_AREA_X1)/HORIZONTAL_NUMBER_OF_IMAGE-IMAGE_GAP
#define IMAGE_HEIGHT (DRAWING_AREA_Y2-DRAWING_AREA_Y1)/VERTICAL_NUMBER_OF_IMAGE-IMAGE_GAP

#define LIST_IMAGE_WIDTH  LIST_AREA_X2-LIST_AREA_X1-IMAGE_GAP
#define LIST_IMAGE_HEIGHT (LIST_AREA_Y2-LIST_AREA_Y1)/VERTICAL_NUMBER_OF_LIST_IMAGE-IMAGE_GAP

static double		 ix;
static double		 iy;
static double		 isy;

static GtkAdjustment    *list_adj;
static GtkAdjustment    *image_adj;

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
			  ImageSelectorCallBack iscb,
			  void *user_context)
{

  gint		 y_start = 0;
  gchar		*dataseturl = NULL;

  GtkWidget	*w;

  if(rootitem)
    return;

  gc_bar_hide(TRUE);

  gc_board_pause(TRUE);

  imageSelectorCallBack = iscb;
  current_user_context = user_context;

  rootitem = goo_canvas_group_new (goo_canvas_get_root_item(gc_get_canvas()),
				   NULL);

  images_selector_displayed = TRUE;

  goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", "#IMAGE_SELECTOR",
			     "pointer-events", GOO_CANVAS_EVENTS_NONE,
			     NULL);

  y_start += 110;

  /*
   * Create the list scrollbar
   * -------------------------
   */
  canvas_list_selector = goo_canvas_new ();

  goo_canvas_widget_new (rootitem,
			 canvas_list_selector,
			 LIST_AREA_X1,
			 LIST_AREA_Y1,
			 LIST_AREA_X2 - LIST_AREA_X1,
			 LIST_AREA_Y2 - LIST_AREA_Y1 - 35.0,
			 NULL);

  gtk_widget_show (canvas_list_selector);

  /* Set the new canvas to the background color or it's white */
  list_bg_item = goo_canvas_rect_new (goo_canvas_get_root_item(GOO_CANVAS(canvas_list_selector)),
				      0,
				      0,
				      (LIST_AREA_X2 - LIST_AREA_X1)
				      * gc_zoom_factor_get(),
				      (LIST_AREA_Y2 - LIST_AREA_Y1)
				      * gc_zoom_factor_get(),
				      "fill-color-rgba",
				      gc_skin_get_color("gcompris/imageselectbg_left"),
				      "line-width", 0.0,
				      NULL);

  list_adj = \
    GTK_ADJUSTMENT (gtk_adjustment_new (0.00, 0.00,
					0.00,
					10, IMAGE_HEIGHT,
					(LIST_AREA_Y2 - LIST_AREA_Y1)
					* gc_zoom_factor_get())
		    );

  w = gtk_vscrollbar_new (list_adj);

  goo_canvas_widget_new (rootitem,
			 w,
			 LIST_AREA_X2 - 5.0,
			 LIST_AREA_Y1,
			 30.0,
			 LIST_AREA_Y2 - LIST_AREA_Y1 - 20.0,
			 NULL);
  gtk_widget_show (w);

  /* Set the scrollwheel event */
  g_signal_connect (list_adj, "value_changed",
		    (GCallback) item_event_scroll,
		    canvas_list_selector);

  /*
   * Create the image scrollbar
   * --------------------------
   */
  canvas_image_selector = goo_canvas_new ();

  goo_canvas_widget_new (rootitem,
			 canvas_image_selector,
			 DRAWING_AREA_X1,
			 DRAWING_AREA_Y1,
			 DRAWING_AREA_X2 - DRAWING_AREA_X1,
			 DRAWING_AREA_Y2 - DRAWING_AREA_Y1,
			 NULL);

  gtk_widget_show (canvas_image_selector);

  /* Set the new canvas to the background color or it's white */
  image_bg_item = \
    goo_canvas_rect_new (goo_canvas_get_root_item(GOO_CANVAS(canvas_image_selector)),
			 0,
			 0,
			 (DRAWING_AREA_X2 - DRAWING_AREA_X1)
			 * gc_zoom_factor_get(),
			 (DRAWING_AREA_Y2 - DRAWING_AREA_Y1)
			 * gc_zoom_factor_get(),
			 "fill-color-rgba",
			 gc_skin_get_color("gcompris/imageselectbg_right"),
			 "line-width", 0.0,
			 NULL);

  image_adj = \
    GTK_ADJUSTMENT (gtk_adjustment_new (0.00, 0.00,
					0.00,
					10, IMAGE_HEIGHT,
					(DRAWING_AREA_Y2 - DRAWING_AREA_Y1)
					* gc_zoom_factor_get())
		    );

  w = gtk_vscrollbar_new (image_adj);

  goo_canvas_widget_new (rootitem,
			 w,
			 DRAWING_AREA_X2 - 5.0,
			 DRAWING_AREA_Y1,
			 30.0,
			 DRAWING_AREA_Y2 - DRAWING_AREA_Y1 - 20.0,
			 NULL);
  gtk_widget_show (w);

  /* Set the scrollwheel event */
  g_signal_connect (image_adj, "value_changed",
		    (GCallback) item_event_scroll,
		    canvas_image_selector);

  /*
   * DISPLAY IMAGES
   */

  /* Initial image position */
  isy = 0.0;

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
      /* TODO */
      filelist = NULL; //gc_net_dir_read_name(dataseturl, ".xml");

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
  gc_util_button_text_svg(rootitem,
			  (BOARDWIDTH*0.5),
			  BOARDHEIGHT - 32,
			  "#BUTTON_TEXT",
			  _("OK"),
			  (GCallback) item_event_images_selector,
			  "/ok/");
}

/*
 * Remove the displayed images_selector.
 * Do nothing if none is currently being dislayed
 */
void gc_selector_images_stop ()
{
  GcomprisBoard *gcomprisBoard = gc_board_get_current();

  if(gcomprisBoard != NULL && images_selector_displayed)
    {
      gc_board_pause(FALSE);
    }

  // Destroy the image_selector box
  if(rootitem!=NULL)
    goo_canvas_item_remove(rootitem);

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
display_image(gchar *imagename, GooCanvasItem *root_item)
{
  GdkPixbuf *pixmap = NULL;
  GooCanvasItem *item;
  double xratio, yratio;
  double iw, ih;

  if (imagename==NULL || !images_selector_displayed)
    return;

  pixmap = gc_pixmap_load_or_null(imagename);

  /* Sad, the image is not found */
  if(!pixmap)
    return;

  iw = IMAGE_WIDTH * gc_zoom_factor_get();
  ih = IMAGE_HEIGHT * gc_zoom_factor_get();

  /* Calc the max to resize width or height */
  xratio = (double) ((iw/(double)gdk_pixbuf_get_width(pixmap)));
  yratio = (double) ((ih/(double)gdk_pixbuf_get_height(pixmap)));
  xratio = MIN(yratio, xratio);

  item = goo_canvas_image_new (root_item,
			       pixmap,
			       0,
			       0,
			       NULL);
  goo_canvas_item_translate(item, ix, iy);
  goo_canvas_item_scale(item, xratio, xratio);
#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
  gdk_pixbuf_unref(pixmap);
#else
  g_object_unref(pixmap);
#endif

  g_signal_connect(item, "button_press_event",
		   (GCallback) item_event_images_selector,
		   imagename);
  gc_item_focus_init(item, NULL);

  ix += iw + IMAGE_GAP;
  if(ix >= (DRAWING_AREA_X2 - DRAWING_AREA_X1)
     * gc_zoom_factor_get() - IMAGE_GAP)
    {
      ix = 0;
      iy += ih + IMAGE_GAP;
    }

  /* Cannot use GINT_TO_POINTER with a constant calculation */
  guint iy_calc = iy + (ih + IMAGE_GAP)*2;
  g_object_set_data (G_OBJECT (root_item), "iy", GINT_TO_POINTER (iy_calc));
}

/*
 * Same as display_image but for the dataset
 * The imagelist contains the list of images to be displayed when this dataset is selected
 */
static void
display_image_set(gchar *imagename, GSList *imagelist)
{
  GdkPixbuf *pixmap = NULL;
  GooCanvasItem *item;
  GooCanvasItem *rootitem_set;
  double xratio, yratio;
  double iw, ih;

  if (imagename == NULL || !images_selector_displayed)
    return;

  pixmap = gc_pixmap_load_or_null(imagename);
  if (!pixmap)
    return;

  iw = LIST_IMAGE_WIDTH * gc_zoom_factor_get();
  ih = LIST_IMAGE_HEIGHT * gc_zoom_factor_get();

  /* Calc the max to resize width or height */
  xratio = (double) ((iw/(double)gdk_pixbuf_get_width(pixmap)));
  yratio = (double) ((ih/(double)gdk_pixbuf_get_height(pixmap)));
  xratio = MIN(yratio, xratio);

  item = goo_canvas_image_new (goo_canvas_get_root_item(GOO_CANVAS(canvas_list_selector)),
			       pixmap,
			       0,
			       0,
			       NULL);
  goo_canvas_item_translate(item, 5, isy);
  goo_canvas_item_scale(item, xratio, xratio);
#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
  gdk_pixbuf_unref(pixmap);
#else
  g_object_unref(pixmap);
#endif
  g_object_set_data (G_OBJECT (item), "imagelist", imagelist);

  g_signal_connect(item, "button_press_event",
		     (GCallback) item_event_imageset_selector,
		     imagename);
  gc_item_focus_init(item, NULL);

  isy += ih + IMAGE_GAP;

  gdouble upper = MAX(isy + ih + IMAGE_GAP,
		      (LIST_AREA_Y2 - LIST_AREA_Y1)
		      * gc_zoom_factor_get());

  goo_canvas_set_bounds (GOO_CANVAS(canvas_list_selector),
			 0, 0,
			 (LIST_AREA_X2 - LIST_AREA_X1)
			 * gc_zoom_factor_get(),
			 upper);

  g_object_set(list_bg_item,
	       "height", upper,
	       NULL);

  g_object_set(list_adj,
	       "upper", upper,
	       NULL);

  /* Create a root item to put the image list in it */
  rootitem_set = \
    goo_canvas_group_new (goo_canvas_get_root_item(GOO_CANVAS(canvas_image_selector)),
			  NULL);

  g_object_set_data (G_OBJECT (item), "rootitem", rootitem_set);
  g_object_set_data (G_OBJECT (item), "imageset_done", GINT_TO_POINTER (0));
  g_object_set_data_full (G_OBJECT (item), "imagelist",
			  imagelist, (GDestroyNotify)free_stuff );
}

static void
free_stuff (GSList *list)
{
  while (g_slist_length(list) > 0) {
    g_free(g_slist_nth_data(list,0));
    list = g_slist_remove(list, g_slist_nth_data(list,0));
  }
  g_slist_free(list);
}


/* Callback when an image set is selected */
static gint
item_event_imageset_selector(GooCanvasItem *item, GdkEvent *event,
			     gpointer data)
{
  GSList *image_list;
  GooCanvasItem *rootitem_set;
  guint imageset_done;
  gdouble last_iy;

  if(display_in_progress)
    return TRUE;

  display_in_progress = TRUE;

  /* We must display the list of images for this set */
  image_list = (GSList *)g_object_get_data (G_OBJECT (item), "imagelist");
  g_return_val_if_fail (image_list != NULL, FALSE);

  /* We must display the list of images for this set */
  rootitem_set = (GooCanvasItem *)g_object_get_data (G_OBJECT (item),
						     "rootitem");
  g_return_val_if_fail (rootitem_set != NULL, FALSE);

  imageset_done = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item),
						      "imageset_done"));

  /* Hide the previous image set if any */
  if (current_root_set != NULL) {
    g_object_set (current_root_set,
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);
  }
  /* Not yet displayed set */
  if(!imageset_done)
    {
      /* Initial image position */
      ix = 0;
      iy = 0;

      g_slist_foreach (image_list, (GFunc) display_image, rootitem_set);
      g_object_set_data (G_OBJECT (item), "imageset_done", GINT_TO_POINTER (1));
    }

  /* Set the image scrollbar back to its max position */
  last_iy = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (rootitem_set), "iy"));
  gdouble upper = MAX(last_iy,
		      (DRAWING_AREA_Y2 - DRAWING_AREA_Y1)
		      * gc_zoom_factor_get());

  goo_canvas_set_bounds  (GOO_CANVAS(canvas_image_selector),
			  0, 0,
			  (DRAWING_AREA_X2 - DRAWING_AREA_X1)
			  * gc_zoom_factor_get(),
			  upper);

  g_object_set(image_bg_item,
	       "height", upper,
	       NULL);

  g_object_set(image_adj,
  	       "upper", last_iy,
  	       NULL);
  gtk_adjustment_set_value(image_adj, 0);

  g_object_set(rootitem_set,
	       "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);

  current_root_set = rootitem_set;

  display_in_progress = FALSE;

  return FALSE;

}

/* Callback when an image is selected */
static gint
item_event_images_selector(GooCanvasItem *item,
			   GooCanvasItem *target,
			   GdkEvent *event,
			   gchar *data)
{

  if(display_in_progress)
    return TRUE;

  if(!strcmp(data, "/ok/"))
    {
      gc_selector_images_stop();
    }
  else
    {
      if(imageSelectorCallBack!=NULL)
	imageSelectorCallBack(data, current_user_context);

      gc_selector_images_stop();
    }

  return FALSE;

}

/* Callback when a scroll event happens */
static void
item_event_scroll(GtkAdjustment *adj, GooCanvas *canvas)
{
  if(!rootitem)
    return;

  goo_canvas_scroll_to (canvas, 0, adj->value);
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
  if (havePathName && pathname[0] == '~' && g_get_home_dir()) {
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
        g_warning("In ImageSet %s, directory %s is not found. Skipping all the ImageSet...",
		  absolutepath, pathname);
        return;
      }
    }
    imageset_directory = g_dir_open (pathname, 0, NULL);
    const gchar * onefile;
    while ((onefile = g_dir_read_name(imageset_directory))) {

      /* Skip README file */
      if(g_ascii_strcasecmp (onefile, "readme") == 0)
	continue;

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
    g_warning("empty document\n");
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

  doc = xmlParseFile(fname);

  /* in case something went wrong */
  if(!doc)
    return FALSE;

  if(/* if there is no root element */
     !doc->children ||
     /* if it doesn't have a name */
     !doc->children->name ||
     /* if it isn't the good node */
     g_ascii_strcasecmp((gchar *)doc->children->name, "ImageSetRoot")!=0) {
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
    {
      g_free(absolute_fname);
      continue;
    }

    /* parse the new file and put the result into newdoc */

    /* pointer to the new doc */
    xmlDocPtr doc;

    doc = xmlParseFile(absolute_fname);

    /* in case something went wrong */
    if(!doc)
    {
      g_free(absolute_fname);
      continue;
    }

    if(/* if there is no root element */
       !doc->children ||
       /* if it doesn't have a name */
       !doc->children->name ||
       /* if it isn't the good node */
       g_ascii_strcasecmp((gchar *)doc->children->name, "ImageSetRoot")!=0) {
      xmlFreeDoc(doc);
      g_free(absolute_fname);
      continue;
    }

    /* parse our document and replace old data */
    g_warning("Parsing dataset : %s \n", absolute_fname);
    g_free(absolute_fname);
    parse_doc(doc);

    xmlFreeDoc(doc);
  }

  g_dir_close(dataset_directory);

  return TRUE;
}
