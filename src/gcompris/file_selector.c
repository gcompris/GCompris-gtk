/* gcompris - file_selector.c
 *
 * Time-stamp: <2003/12/19 22:00:48 bcoudoin>
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
 * An file selector for gcompris
 *
 */

#include <dirent.h>
#include "gcompris.h"

#define SOUNDLISTFILE PACKAGE

static gint		 item_event_file_selector(GnomeCanvasItem *item, 
						  GdkEvent *event, 
						  gpointer data);
static gint		 item_event_fileset_selector(GnomeCanvasItem *item, 
						     GdkEvent *event, 
						     gpointer data);
static void		 display_files(GnomeCanvasItem *rootitem, gchar *rootdir);
static void		 free_stuff (GtkObject *obj, GList *data);

static gboolean		 file_selector_displayed = FALSE;

static GnomeCanvasItem	*rootitem = NULL;
static GnomeCanvasItem	*current_root_set = NULL;
static GnomeCanvasItem	*item_content = NULL;

static FileSelectorCallBack fileSelectorCallBack = NULL;

/* Represent the limits of control area */
#define	CONTROL_AREA_X1	40
#define	CONTROL_AREA_Y1	20
#define	CONTROL_AREA_X2	760
#define	CONTROL_AREA_Y2	80

/* Represent the limits of the file area */
#define	DRAWING_AREA_X1	40
#define DRAWING_AREA_Y1	120
#define DRAWING_AREA_X2	760
#define DRAWING_AREA_Y2	500

#define HORIZONTAL_NUMBER_OF_IMAGE	10
#define VERTICAL_NUMBER_OF_IMAGE	5
#define IMAGE_GAP			10

#define IMAGE_WIDTH  (DRAWING_AREA_X2-DRAWING_AREA_X1)/HORIZONTAL_NUMBER_OF_IMAGE-IMAGE_GAP
#define IMAGE_HEIGHT (DRAWING_AREA_Y2-DRAWING_AREA_Y1)/VERTICAL_NUMBER_OF_IMAGE-IMAGE_GAP

static guint		 ix  = DRAWING_AREA_X1;
static guint		 iy  = DRAWING_AREA_Y1;
static guint		 isy = DRAWING_AREA_Y1;

/*
 * Main entry point 
 * ----------------
 *
 */

/*
 * Do all the file_selector display and register the events
 */

void gcompris_file_selector_save (GcomprisBoard *gcomprisBoard, gchar *rootdir,
				  FileSelectorCallBack iscb)
{
  printf("gcompris_file_selector_save\n");
}

void gcompris_file_selector_load (GcomprisBoard *gcomprisBoard, gchar *rootdir,
				  FileSelectorCallBack iscb)
{

  GnomeCanvasItem *item, *item2;
  GdkPixbuf	*pixmap = NULL;
  gint		 y = 0;
  gint		 y_start = 0;
  gint		 x_start = 0;
  gchar		*name = NULL;

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

  y_start += 110;

  pixmap = gcompris_load_skin_pixmap("button_large.png");


  // OK
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH*0.5) - gdk_pixbuf_get_width(pixmap)/2,
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
				 "text", _("OK"),
				 "font", gcompris_skin_font_title,
				 "x", (double)  BOARDWIDTH*0.5,
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

  /* Initial image position */
  ix  = DRAWING_AREA_X1;
  iy  = DRAWING_AREA_Y1;
  isy = DRAWING_AREA_Y1;

  display_files(rootitem, rootdir);

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
  if(rootitem!=NULL)
    {
      gtk_object_destroy(GTK_OBJECT(rootitem));
    }
  rootitem = NULL;	  
  current_root_set = NULL;

  gcompris_bar_hide(FALSE);
  file_selector_displayed = FALSE;
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

static void display_files(GnomeCanvasItem *root_item, gchar *rootdir)
{

  GdkPixbuf *pixmap_dir  = NULL;
  GdkPixbuf *pixmap_file = NULL;
  GnomeCanvasItem *item;
  double iw, ih;
  gchar *gc_rootdir;
  struct dirent *one_dirent;
  DIR *dir;

  if(g_get_home_dir()) {
    gc_rootdir = g_strconcat(g_get_home_dir(), "/gcompris", NULL);
  } else {
    /* On WIN98, No home dir */
    gc_rootdir = g_strdup("gcompris");
  }

  /* Display the directory content */
  dir = opendir(gc_rootdir);

  if (!dir) {
    g_warning("gcompris_file_selector : no root directory found in %s", gc_rootdir);
    g_free(gc_rootdir);
    return;
  }

  pixmap_dir  = gcompris_load_pixmap(gcompris_image_to_skin("directory.png"));
  pixmap_file = gcompris_load_pixmap(gcompris_image_to_skin("file.png"));

  iw = IMAGE_WIDTH;
  ih = IMAGE_HEIGHT;

  while((one_dirent = readdir(dir)) != NULL) {
    /* add the file to the display */
    gchar *filename;
    GdkPixbuf *pixmap_current  = pixmap_file;
      
    if((strcmp(one_dirent->d_name, "..")==0) ||
       (strcmp(one_dirent->d_name, ".")==0))
      continue;

    filename = g_strdup_printf("%s/%s",
			       gc_rootdir, one_dirent->d_name);
    
    if(g_file_test(filename, G_FILE_TEST_IS_DIR)) {
      pixmap_current  = pixmap_dir;
    }

    item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(root_item),
				  gnome_canvas_pixbuf_get_type (),
				  "pixbuf", pixmap_current,
				  "x", (double)ix,
				  "y", (double)iy,
				  NULL);

    gtk_signal_connect(GTK_OBJECT(item), "event",
		       (GtkSignalFunc) item_event_file_selector,
		       filename);
    gtk_signal_connect(GTK_OBJECT(item), "event",
		       (GtkSignalFunc) gcompris_item_event_focus,
		       NULL);

    item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(root_item),
				  gnome_canvas_text_get_type (),
				  "text", g_path_get_basename(filename),
				  "x", (double)ix + (IMAGE_WIDTH + IMAGE_GAP)/2,
				  "y", (double)iy + IMAGE_HEIGHT - 15,
				  "anchor", GTK_ANCHOR_CENTER,
				  "fill_color_rgba", 0x0000FFFF,
				  NULL);

    gtk_signal_connect(GTK_OBJECT(item), "event",
		       (GtkSignalFunc) item_event_file_selector,
		       filename);

    ix+=IMAGE_WIDTH + IMAGE_GAP;
    if(ix>=DRAWING_AREA_X2-IMAGE_GAP)
      {
	ix=DRAWING_AREA_X1;
	iy+=IMAGE_HEIGHT + IMAGE_GAP;
      }

    g_free(filename);
  }

  closedir(dir);

  gdk_pixbuf_unref(pixmap_dir);
  gdk_pixbuf_unref(pixmap_file);

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


/* Callback when an image is selected */
static gint
item_event_file_selector(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
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
	  gcompris_file_selector_stop();
	}
      else
	{
	  if(fileSelectorCallBack!=NULL)
	    fileSelectorCallBack(data);

	  gcompris_file_selector_stop();
	}
    default:
      break;
    }
  return FALSE;

}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
