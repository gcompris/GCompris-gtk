/* gcompris - file_selector.c
 *
 * Time-stamp: <2005/03/01 00:26:43 bruno>
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
 * A file selector for gcompris
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "gcompris.h"

#define SOUNDLISTFILE PACKAGE

#define MODE_LOAD 1
#define MODE_SAVE 2
static gint mode;

static gint		 item_event_file_selector(GnomeCanvasItem *item, 
						  GdkEvent *event, 
						  gpointer data);
static gint		 item_event_scroll(GnomeCanvasItem *item,
					   GdkEvent *event,
					   GnomeCanvas *canvas);
static gint		 item_event_fileset_selector(GnomeCanvasItem *item, 
						     GdkEvent *event, 
						     gpointer data);
static gint		 item_event_directory(GnomeCanvasItem *item, 
					      GdkEvent *event, 
					      char *dir);
static void		 display_files(GnomeCanvasItem *rootitem, gchar *rootdir);
static int		 display_file_selector(int mode, 
					       GcomprisBoard *gcomprisBoard,
					       gchar *rootdir,
					       gchar *file_types,
					       FileSelectorCallBack iscb);
static void		 entry_enter_callback( GtkWidget *widget,
					       GtkWidget *entry );
static int		 create_rootdir (gchar *rootdir);
static void		 free_stuff (GtkObject *obj, gchar* data);

static gboolean		 file_selector_displayed = FALSE;

static GnomeCanvasItem	*rootitem = NULL;
static GnomeCanvasItem	*file_root_item = NULL;

static FileSelectorCallBack  fileSelectorCallBack = NULL;
static GtkWidget            *gtk_combo_filetypes = NULL;

static gchar *current_rootdir = NULL;
static GtkEntry *widget_entry = NULL;

static GList  *file_list = NULL;

/* Represent the limits of control area */
#define	CONTROL_AREA_X1	40.0
#define	CONTROL_AREA_Y1	20.0
#define	CONTROL_AREA_X2	760.0
#define	CONTROL_AREA_Y2	80.0

/* Represent the limits of the file area */
#define	DRAWING_AREA_X1	40.0
#define DRAWING_AREA_Y1	120.0
#define DRAWING_AREA_X2	760.0
#define DRAWING_AREA_Y2	500.0

#define HORIZONTAL_NUMBER_OF_IMAGE	6
#define VERTICAL_NUMBER_OF_IMAGE	5
#define IMAGE_GAP			18

#define IMAGE_WIDTH  (DRAWING_AREA_X2-DRAWING_AREA_X1)/HORIZONTAL_NUMBER_OF_IMAGE-IMAGE_GAP
#define IMAGE_HEIGHT (DRAWING_AREA_Y2-DRAWING_AREA_Y1)/VERTICAL_NUMBER_OF_IMAGE-IMAGE_GAP

/*
 * Mime type management
 * --------------------
 */
typedef struct {
  gchar	       *mimetype;
  gchar	       *description;
  gchar	       *extension;
  gchar	       *icon;
} GcomprisMimeType;

static GHashTable* mimetypes_hash = NULL;
static GHashTable* mimetypes_ext_hash = NULL; /* Mime type with the key being the extension */

/*
 * Main entry point 
 * ----------------
 *
 */

/*
 * Do all the file_selector display and register the events
 * file_types is A Comma separated text explaining the different file types
 */

void gcompris_file_selector_save (GcomprisBoard *gcomprisBoard, gchar *rootdir,
				  gchar *file_types,
				  FileSelectorCallBack iscb)
{
  display_file_selector(MODE_SAVE, gcomprisBoard, rootdir, file_types,
			iscb);
}

void gcompris_file_selector_load (GcomprisBoard *gcomprisBoard, gchar *rootdir,
				  gchar *file_types,
				  FileSelectorCallBack iscb)
{
  display_file_selector(MODE_LOAD, gcomprisBoard, rootdir, file_types,
			iscb);
}

/*
 * Remove the displayed file_selector.
 * Do nothing if none is currently being dislayed
 */
void gcompris_file_selector_stop ()
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  if(gcomprisBoard!=NULL && file_selector_displayed)
    {
      if(gcomprisBoard->plugin->pause_board != NULL)
	{
	  gcomprisBoard->plugin->pause_board(FALSE);
	}
    }

  // Destroy the file_selector box
  /* FIXME: Crashes randomly */
  if(rootitem!=NULL)
    gtk_object_destroy(GTK_OBJECT(rootitem));

  rootitem = NULL;	  

  /* No need to destroy it since it's in rootitem but just clear it */
  file_root_item = NULL;

  /* Cleanup the file list */
  if(file_list) {
    //    g_list_foreach(file_list, (GFunc)g_free, NULL);
    g_list_free(file_list);
  }
  file_list = NULL;

  gcompris_bar_hide(FALSE);
  file_selector_displayed = FALSE;
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/


static int 
display_file_selector(int the_mode, 
		      GcomprisBoard *gcomprisBoard,
		      gchar *rootdir,
		      gchar *file_types,
		      FileSelectorCallBack iscb) {


  GnomeCanvasItem  *item, *item2;
  GdkPixbuf	   *pixmap = NULL;
  gint		    y = 0;
  gint		    y_start = 0;
  gint		    x_start = 0;
  gchar		   *name = NULL;
  gchar            *full_rootdir;
  gchar            *sub_string;
  gchar            *file_types_string = NULL;

  mode = the_mode;

  if(file_types) {
    file_types_string = g_strdup(file_types);
  }

  if(rootitem)
    return;

  gcompris_bar_hide(TRUE);

  if(gcomprisBoard!=NULL)
    {
      if(gcomprisBoard->plugin->pause_board != NULL)
	gcomprisBoard->plugin->pause_board(TRUE);
    }

  name = gcomprisBoard->name;
  fileSelectorCallBack=iscb;

  rootitem = \
    gnome_canvas_item_new (gnome_canvas_root(gcompris_get_canvas()),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  pixmap = gcompris_load_skin_pixmap("file_selector_bg.png");
  y_start = (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;
  x_start = (BOARDWIDTH - gdk_pixbuf_get_width(pixmap))/2;
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) x_start,
				"y", (double) y_start,
				NULL);
  y = BOARDHEIGHT - (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2 + 20;
  gdk_pixbuf_unref(pixmap);

  /* Entry area */
  widget_entry = (GtkEntry *)gtk_entry_new_with_max_length (50);
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_widget_get_type (),
				"widget", GTK_WIDGET(widget_entry),
				"x", (double) CONTROL_AREA_X1 + 10,
				"y", (double) y_start + 30,
				"width", 250.0,
				"height", 30.0,
				"anchor", GTK_ANCHOR_NW,
				"size_pixels", FALSE,
				NULL);
  gtk_signal_connect(GTK_OBJECT(widget_entry), "activate",
		     GTK_SIGNAL_FUNC(entry_enter_callback),
		     widget_entry);

  gtk_widget_show(GTK_WIDGET(widget_entry));

  /*
   * Create the combo with the file types 
   * ------------------------------------
   */
  if(file_types_string && *file_types_string!='\0') {
    gtk_combo_filetypes = gtk_combo_box_new_text();

    /* Extract first string	*/
    gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_combo_filetypes), (char *)strtok(file_types_string, " "));

    while ( (sub_string=(char *)strtok(NULL, " ")) != NULL)
      {
	gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_combo_filetypes), sub_string);
      }

    gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			   gnome_canvas_widget_get_type (),
			   "widget", GTK_WIDGET(gtk_combo_filetypes),
			   "x", (double) CONTROL_AREA_X1 + 300,
			   "y", (double) y_start + 30,
			   "width", 250.0,
			   "height", 30.0,
			   "anchor", GTK_ANCHOR_NW,
			   "size_pixels", FALSE,
			   NULL);

    gtk_widget_show(GTK_WIDGET(gtk_combo_filetypes));
    gtk_combo_box_set_active(GTK_COMBO_BOX(gtk_combo_filetypes), 0);
    g_free(file_types_string);
  }


  y_start += 110;


  /*
   * Buttons
   * -------
   */

  pixmap = gcompris_load_skin_pixmap("button_large.png");

  // CANCEL
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH*0.33) - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y - gdk_pixbuf_get_height(pixmap) - 25,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_file_selector,
		     "/cancel/");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  item2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				 gnome_canvas_text_get_type (),
				 "text", _("CANCEL"),
				 "font", gcompris_skin_font_title,
				 "x", (double)  BOARDWIDTH*0.33,
				 "y", (double)  y - gdk_pixbuf_get_height(pixmap),
				 "anchor", GTK_ANCHOR_CENTER,
				 "fill_color_rgba", gcompris_skin_color_text_button,
				 NULL);
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) item_event_file_selector,
		     "/cancel/");
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     item);

  // OK
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH*0.66) - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y - gdk_pixbuf_get_height(pixmap) - 25,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_file_selector,
		     "/ok/");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  item2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				 gnome_canvas_text_get_type (),
				 "text", (mode==MODE_LOAD ? _("LOAD") : _("SAVE")),
				 "font", gcompris_skin_font_title,
				 "x", (double)  BOARDWIDTH*0.66,
				 "y", (double)  y - gdk_pixbuf_get_height(pixmap),
				 "anchor", GTK_ANCHOR_CENTER,
				 "fill_color_rgba", gcompris_skin_color_text_button,
				 NULL);
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) item_event_file_selector,
		     "/ok/");
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     item);
  gdk_pixbuf_unref(pixmap);



  file_selector_displayed = TRUE;

  if(g_get_home_dir()) {
    full_rootdir = g_strconcat(g_get_home_dir(), "/.gcompris/user_data", NULL);
    create_rootdir(full_rootdir);
    g_free(full_rootdir);

    full_rootdir = g_strconcat(g_get_home_dir(), "/.gcompris/user_data/", rootdir, NULL);
  } else {
    /* On WIN98, No home dir */
    full_rootdir = g_strdup_printf("%s", "gcompris/user_data");
    create_rootdir(full_rootdir);
    g_free(full_rootdir);
  
    full_rootdir = g_strdup_printf("%s/%s", "gcompris/user_data", rootdir);
  }

  create_rootdir(full_rootdir);

  current_rootdir = full_rootdir;
  display_files(rootitem, full_rootdir);

}


/*
 * create the root dir if needed
 *
 * return 0 if OK, -1 if ERROR
 */
static int
create_rootdir (gchar *rootdir)
{

  if(g_file_test(rootdir, G_FILE_TEST_IS_DIR)) {
    return 0;
  }

  return(mkdir(rootdir, 0755));
}

static void
free_stuff (GtkObject *obj, gchar *data)
{
  g_free(data);
}

static void display_files(GnomeCanvasItem *root_item, gchar *rootdir)
{

  GdkPixbuf *pixmap_dir  = NULL;
  GdkPixbuf *pixmap_file = NULL;
  GnomeCanvasItem *item;
  double iw, ih;
  struct dirent *one_dirent;
  DIR *dir;

  /* Initial image position */
  guint ix  = 0.0;
  guint iy  = 0.0;

  GtkWidget	  *w;
  GnomeCanvasItem *bg_item;

  GnomeCanvas	  *canvas; /* The scrolled part */

  GList  *dir_list  = NULL;
  GList  *listrunner;

  if(!rootitem)
    return;

  /* Display the directory content */
  dir = opendir(rootdir);

  if (!dir) {
    g_warning("gcompris_file_selector : no root directory found in %s", rootdir);
    g_free(rootdir);
    return;
  }

  /* Delete the previous file root if any */
  if(file_root_item!=NULL) {
    gtk_object_destroy(GTK_OBJECT(file_root_item));
  }

  /* Cleanup the file list */
  if(file_list) {
    //g_list_foreach(file_list, (GFunc)g_free, NULL);
    g_list_free(file_list);
  }
  file_list = NULL;
  
  /* Create a root item to put the image list in it */
  file_root_item = \
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(root_item),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  /*
   * Create the scrollbar
   * --------------------
   */
  canvas     = GNOME_CANVAS(gnome_canvas_new ());

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(file_root_item),
			 gnome_canvas_widget_get_type (),
			 "widget", GTK_WIDGET(canvas),
			 "x", (double) DRAWING_AREA_X1,
			 "y", (double) DRAWING_AREA_Y1,
			 "width",  DRAWING_AREA_X2- DRAWING_AREA_X1 - 20.0,
			 "height", DRAWING_AREA_Y2-DRAWING_AREA_Y1 - 35.0,
			 NULL);

  gtk_widget_show (GTK_WIDGET(canvas));

  /* Set the new canvas to the background color or it's white */
  bg_item = gnome_canvas_item_new (gnome_canvas_root(canvas),
			 gnome_canvas_rect_get_type (),
			 "x1", (double) 0,
			 "y1", (double) 0,
			 "x2", (double) DRAWING_AREA_X2- DRAWING_AREA_X1,
			 "y2", (double) DRAWING_AREA_Y2-DRAWING_AREA_Y1,
			 "fill_color_rgba", gcompris_skin_get_color("gcompris/fileselectbg"),
			 NULL);


  w = gtk_vscrollbar_new (GTK_LAYOUT(canvas)->vadjustment);

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(file_root_item),
			 gnome_canvas_widget_get_type (),
			 "widget", GTK_WIDGET(w),
			 "x", (double) DRAWING_AREA_X2 - 15.0,
			 "y", (double) DRAWING_AREA_Y1,
			 "width", 30.0,
			 "height", DRAWING_AREA_Y2-DRAWING_AREA_Y1 - 20.0,
			 NULL);
  gtk_widget_show (w);
  gnome_canvas_set_center_scroll_region (GNOME_CANVAS (canvas), FALSE);

  /* Set the scrollwheel event */
  gtk_signal_connect(GTK_OBJECT(canvas), "event",
		     (GtkSignalFunc) item_event_scroll,
		     GNOME_CANVAS(canvas));

  /* Display the directory name
   * --------------------------
   */

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(file_root_item),
				gnome_canvas_text_get_type (),
				"text", rootdir,
				"x", (double)CONTROL_AREA_X1 + 10,
				"y", (double)CONTROL_AREA_Y1 + 50,
				"fill_color_rgba", 0x0000FFFF,
				"anchor", GTK_ANCHOR_NW,
				NULL);


  pixmap_dir  = gcompris_load_pixmap(gcompris_image_to_skin("directory.png"));
  pixmap_file = gcompris_load_pixmap(gcompris_image_to_skin("file.png"));

  iw = IMAGE_WIDTH;
  ih = IMAGE_HEIGHT;

  /* Insert all files in a sorted list */

  while((one_dirent = readdir(dir)) != NULL) {
    gchar *filename;

    if((strcmp(one_dirent->d_name, "..")==0) &&
       strcmp(current_rootdir, rootdir)==0)
      continue;

    if(strcmp(one_dirent->d_name, ".")==0)
      continue;

    filename = g_strdup_printf("%s/%s",
			       rootdir, (gchar*)(one_dirent->d_name));

    if(g_file_test(filename, G_FILE_TEST_IS_DIR)) {
      dir_list = g_list_insert_sorted(dir_list, filename,
				       (GCompareFunc)strcmp);
    } else {
      file_list = g_list_insert_sorted(file_list, filename,
				       (GCompareFunc)strcmp);
    }
  }

  /* Concat the directory list and file list */
  file_list = g_list_concat(dir_list, file_list);

  /* We have the list sorted, now display it */
  listrunner = g_list_first(file_list);
  while(listrunner) {
    gchar *allfilename = listrunner->data;
    gchar *filename    = g_path_get_basename(allfilename);
    gchar *ext         = rindex(filename, '.');

    /* add the file to the display */
    GdkPixbuf *pixmap_current  = pixmap_file;

    if(ext)
      printf("filename=%s extension=%s\n", filename, ext);

    if(g_file_test(allfilename, G_FILE_TEST_IS_DIR)) {
      pixmap_current  = pixmap_dir;
    }

    item = gnome_canvas_item_new (gnome_canvas_root(canvas),
				  gnome_canvas_pixbuf_get_type (),
				  "pixbuf", pixmap_current,
				  "x", (double)ix + (IMAGE_WIDTH + IMAGE_GAP
						     - gdk_pixbuf_get_width(pixmap_current))/2,
				  "y", (double)iy,
				  NULL);

    if(g_file_test(allfilename, G_FILE_TEST_IS_DIR)) {
      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) item_event_directory,
			 allfilename);
    } else {
      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) item_event_file_selector,
			 allfilename);
    }
    gtk_signal_connect(GTK_OBJECT(item), "event",
		       (GtkSignalFunc) gcompris_item_event_focus,
		       NULL);

    g_object_set_data (G_OBJECT (item), "path", filename);
    g_signal_connect (item, "destroy",
		      G_CALLBACK (free_stuff),
		      allfilename);

    item = gnome_canvas_item_new (gnome_canvas_root(canvas),
				  gnome_canvas_text_get_type (),
				  "text", filename,
				  "x", (double)ix + (IMAGE_WIDTH + IMAGE_GAP)/2,
				  "y", (double)iy + IMAGE_HEIGHT - 5,
				  "anchor", GTK_ANCHOR_CENTER,
				  "fill_color_rgba", 0x0000FFFF,
				  NULL);

    if(g_file_test(allfilename, G_FILE_TEST_IS_DIR)) {
      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) item_event_directory,
			 allfilename);
    } else {
      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) item_event_file_selector,
			 allfilename);
    }
      
    ix+=IMAGE_WIDTH + IMAGE_GAP;
    if(ix>=DRAWING_AREA_X2 - DRAWING_AREA_X1 -IMAGE_GAP)
      {
	ix=0;

	iy+=IMAGE_HEIGHT + IMAGE_GAP;

	gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas), 0, 0, 
					DRAWING_AREA_X2- DRAWING_AREA_X1, 
					iy + IMAGE_HEIGHT + IMAGE_GAP);

	if(iy>=DRAWING_AREA_Y2-DRAWING_AREA_Y1) {
	  gnome_canvas_item_set(bg_item,
				"y2", (double)iy + IMAGE_HEIGHT + IMAGE_GAP,
				NULL);
	}
      }
    listrunner = g_list_next(listrunner);
  }

  closedir(dir);

  gdk_pixbuf_unref(pixmap_dir);
  gdk_pixbuf_unref(pixmap_file);

}

/* Callback when a directory is selected */
static gint
item_event_directory(GnomeCanvasItem *item, GdkEvent *event, gchar *dir)
{

  if(!rootitem)
    return;

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
      printf("directory selected=%s\n", dir);
      if(strcmp(g_path_get_basename(dir), "..")==0) {
	/* Up one level. Remove .. and one directory on the right of the path */
	dir[strlen(dir)-3] = '\0';
	dir=g_path_get_dirname(dir);
      }
      display_files(rootitem, g_strdup(dir));
      gtk_entry_set_text(widget_entry, "");
      break;
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
  return FALSE;
}

/* Callback when a file is selected */
static gint
item_event_file_selector(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{

  if(!rootitem)
    return;

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
      if(!strcmp((char *)data, "/ok/"))	{

	/* Nothing selected, please cancel instead */
	if(strcmp(gtk_entry_get_text(widget_entry),"")==0) {
	  return FALSE;
	}

	if(fileSelectorCallBack!=NULL) {
	  gchar *result=NULL;
	  gchar *file_type=NULL;

	  GtkTreeModel *model;
	  GtkTreeIter iter;

	  model = gtk_combo_box_get_model ((GtkComboBox *)gtk_combo_filetypes);
	  if (gtk_combo_box_get_active_iter ((GtkComboBox *)gtk_combo_filetypes, &iter)) {
	    gtk_tree_model_get (model, &iter, 0, &file_type, -1);
	  }

	  result = g_strdup_printf("%s/%s", current_rootdir, gtk_entry_get_text(widget_entry));
	  
	  if(mode==MODE_SAVE) {
	    GcomprisMimeType *mimeType = NULL;

	    /* Extract the mime type */
	    mimeType = (GcomprisMimeType *)(g_hash_table_lookup(mimetypes_hash, file_type));
	    printf("display_file_selector mimetype=%s\n", mimeType->extension);
	    if(!g_str_has_suffix(result,mimeType->extension)) {
	      gchar *old_result = result;
	      result = g_strconcat(result, mimeType->extension, NULL);
	      g_free(old_result);
	    }
	  }

	  /* Callback with the proper params */
	  fileSelectorCallBack(result, file_type);

	  if(file_type) {
	    g_free(file_type);
	  }

	  /* DO NOT FREE RESULT OR PYTHON SIDE WILL BE IN TROUBLE */
	  //	  if(result) {
	  //	    g_free(result);
	  //	  }
	}
	gcompris_file_selector_stop();
      } else if(!strcmp((char *)data, "/cancel/")) {
	gcompris_file_selector_stop();
      } else {
	gtk_entry_set_text(widget_entry, g_path_get_basename((gchar *)data));
      }
      break;
    default:
      break;
    }
  return FALSE;

}

static void entry_enter_callback( GtkWidget *widget,
				  GtkWidget *entry )
{
  gchar *entry_text;

  if(!rootitem)
    return;

  entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
}

/*
 * MimeType PARSING
 * ----------------
 */

void parseMime (xmlDocPtr doc, xmlNodePtr xmlnode) {
  gchar *mimeSetName = NULL;
  gchar *filename;
  GList	*mimeList = NULL;	/* List of Mimes */

  GcomprisMimeType *gcomprisMime = g_malloc0 (sizeof (GcomprisMimeType));

  gcomprisMime->mimetype    = xmlGetProp(xmlnode,"mimetype");
  gcomprisMime->extension   = xmlGetProp(xmlnode,"extension");
  gcomprisMime->icon        = xmlGetProp(xmlnode,"icon");
  gcomprisMime->description = NULL;

  /* get the help user credit of the board */
  xmlnode = xmlnode->xmlChildrenNode;
  while (xmlnode != NULL) {
    gchar *lang = xmlGetProp(xmlnode,"lang");
    if (!strcmp(xmlnode->name, "description")
	&& (lang==NULL ||
	    !strcmp(lang, gcompris_get_locale())
	    || !strncmp(lang, gcompris_get_locale(), 2)))
      {
	if(gcomprisMime->description)
	  g_free(gcomprisMime->description);
	
	gcomprisMime->description = xmlNodeListGetString(doc, 
							 xmlnode->xmlChildrenNode, 1);
      }
    xmlnode = xmlnode->next;
  }

  if(!gcomprisMime->mimetype || !gcomprisMime->extension || !gcomprisMime->description ) {
    g_warning("Incomplete mimetype description\n");
    g_free(gcomprisMime);
    return;
  }

  g_message("Mime type mimetype=%s description=%s extension=%s icon=%s\n",
	    gcomprisMime->mimetype,
	    gcomprisMime->description,
	    gcomprisMime->extension,
	    gcomprisMime->icon);

  g_hash_table_insert(mimetypes_hash, gcomprisMime->mimetype, gcomprisMime);
  g_hash_table_insert(mimetypes_ext_hash, gcomprisMime->extension, gcomprisMime);

  return;
}

static void parse_doc(xmlDocPtr doc) {
  xmlNodePtr cur;

  cur = xmlDocGetRootElement(doc);	
  if (cur == NULL) {
    fprintf(stderr,"empty document\n");
    xmlFreeDoc(doc);
    return;
  }
  cur = cur->xmlChildrenNode;

  while (cur != NULL) {
    if ((!xmlStrcmp(cur->name, (const xmlChar *)"MimeType"))){
      parseMime (doc, cur);
    }
    
    cur = cur->next;
  }
	
  return;
}


/* read an xml file into our memory structures and update our view,
   dump any old data we have in memory if we can load a new set */
gboolean load_mime_type_from_file(gchar *fname)
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
     g_strcasecmp(doc->children->name,"MimeTypeRoot")!=0) {
    xmlFreeDoc(doc);
    return FALSE;
  }
  
  /* parse our document and replace old data */
  parse_doc(doc);
  
  xmlFreeDoc(doc);
  
  return TRUE;
}

/**
 * gcompris_load_mime_types
 * Load all the mime type in PACKAGE_DATA_DIR"/gcompris/mimetypes/*.xml"
 *
 * Must be called once at GCompris startup.
 *
 */
void gcompris_load_mime_types() 
{
  struct dirent *one_dirent;
  DIR *dir;
  int n;

  if(mimetypes_hash) {
    return;
  }

  mimetypes_hash = g_hash_table_new (g_str_hash, g_str_equal);
  printf("ICI\n");
  /* Load the Pixpmaps directory file names */
  dir = opendir(PACKAGE_DATA_DIR"/gcompris/mimetypes/");

  if (!dir) {
    g_warning("gcompris_load_mime_types : no mime types found in %s", PACKAGE_DATA_DIR"/gcompris/mimetypes/");
  } else {

    while((one_dirent = readdir(dir)) != NULL) {
      /* add the board to the list */
      gchar *filename;
      
      filename = g_strdup_printf("%s/%s",
				 PACKAGE_DATA_DIR"/gcompris/mimetypes/", one_dirent->d_name);

      printf("ICI file =%s\n", filename);
      if(!g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
	g_free(filename);
	continue;
      }

      printf("ICI file is regular %s\n", one_dirent->d_name);
      if(selectMenuXML(one_dirent->d_name)) {
	printf("  calling load_mime_type_from_file\n");
	load_mime_type_from_file(filename);
      }
      g_free(filename);
    }
  }
  closedir(dir);
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
