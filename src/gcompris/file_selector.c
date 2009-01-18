/* gcompris - file_selector.c
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
 * A file selector for gcompris
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "gcompris.h"
#include "gc_core.h"

#define SOUNDLISTFILE PACKAGE

#define MODE_LOAD 1
#define MODE_SAVE 2
static gint mode;

static gboolean item_event_file_selector (GooCanvasItem  *item,
					  GooCanvasItem  *target,
					  GdkEventButton *event,
					  char *data);
static void		 item_event_scroll(GtkAdjustment *adj,
					   GooCanvas *canvas);
static gint		 item_event_directory(GooCanvasItem *item,
					      GdkEvent *event,
					      char *dir);
static void		 display_files(GooCanvasItem *rootitem, gchar *rootdir);
static int		 display_file_selector(int mode,
					       GcomprisBoard *gcomprisBoard,
					       gchar *rootdir,
					       gchar *file_types,
					       FileSelectorCallBack iscb);
static void		 entry_enter_callback( GtkWidget *widget,
					       GtkWidget *entry );
static gboolean		 file_selector_displayed = FALSE;

static GooCanvasItem	*rootitem = NULL;
static GooCanvasItem	*file_root_item = NULL;

static FileSelectorCallBack  fileSelectorCallBack = NULL;
static GtkWidget            *gtk_combo_filetypes = NULL;

static gchar *current_rootdir = NULL;
static GtkWidget *widget_entry = NULL;

/* Represent the limits of control area */
static guint32 control_area_x1;
static guint32 control_area_y1;
static guint32 directory_label_y;

/* Represent the limits of the file area */
#define	DRAWING_AREA_X1	40.0
#define DRAWING_AREA_Y1	130.0
#define DRAWING_AREA_X2	760.0
#define DRAWING_AREA_Y2	490.0

#define HORIZONTAL_NUMBER_OF_IMAGE	5
#define VERTICAL_NUMBER_OF_IMAGE	3
#define IMAGE_GAP			18

#define IMAGE_WIDTH  (DRAWING_AREA_X2-DRAWING_AREA_X1)/HORIZONTAL_NUMBER_OF_IMAGE-IMAGE_GAP
#define IMAGE_HEIGHT (DRAWING_AREA_Y2-DRAWING_AREA_Y1)/VERTICAL_NUMBER_OF_IMAGE-IMAGE_GAP

/*
 * Main entry point
 * ----------------
 *
 */

/*
 * Do all the file_selector display and register the events
 * file_types is A Space separated possible file extensions
 */

void gc_selector_file_save (GcomprisBoard *gcomprisBoard, gchar *rootdir,
				  gchar *file_exts,
				  FileSelectorCallBack iscb)
{
  display_file_selector(MODE_SAVE, gcomprisBoard, rootdir, file_exts,
			iscb);
}

void gc_selector_file_load (GcomprisBoard *gcomprisBoard, gchar *rootdir,
				  gchar *file_exts,
				  FileSelectorCallBack iscb)
{
  display_file_selector(MODE_LOAD, gcomprisBoard, rootdir, file_exts,
			iscb);
}

/*
 * Remove the displayed file_selector.
 * Do nothing if none is currently being dislayed
 */
void
gc_selector_file_stop ()
{
  GcomprisBoard *gcomprisBoard = gc_board_get_current();

  if(gcomprisBoard!=NULL && file_selector_displayed)
    gc_board_pause(FALSE);

  // Destroy the file_selector box
  if(rootitem!=NULL)
    goo_canvas_item_remove(rootitem);

  rootitem = NULL;

  /* No need to destroy it since it's in rootitem but just clear it */
  file_root_item = NULL;

  gc_bar_hide(FALSE);
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
		      gchar *file_exts,
		      FileSelectorCallBack iscb)
{
  GooCanvasItem    *item;
  gchar		   *name = NULL;
  gchar            *full_rootdir;

  mode = the_mode;

  /* Get the coordinate x y of the control area from the skin */
  control_area_x1   = gc_skin_get_number_default("gcompris/fileselectx", 85);
  control_area_y1   = gc_skin_get_number_default("gcompris/fileselecty", 80);
  directory_label_y = gc_skin_get_number_default("gcompris/fileselectdiry", 180);

  if(rootitem)
    return 0;

  gc_bar_hide(TRUE);

  gc_board_pause(TRUE);

  name = gcomprisBoard->name;
  fileSelectorCallBack=iscb;

  rootitem = goo_canvas_group_new (goo_canvas_get_root_item(gc_get_canvas()),
				   NULL);

  item = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", "#FILE_SELECTOR",
			     "pointer-events", GOO_CANVAS_EVENTS_NONE,
			     NULL);

  /* Entry area */
  widget_entry = gtk_entry_new ();

  if(mode==MODE_SAVE)
    gtk_entry_set_max_length(GTK_ENTRY(widget_entry), 30);

  item = goo_canvas_widget_new (rootitem,
				GTK_WIDGET(widget_entry),
				control_area_x1,
				control_area_y1,
				230.0,
				30.0,
				NULL);

  gtk_signal_connect(GTK_OBJECT(widget_entry), "activate",
		     GTK_SIGNAL_FUNC(entry_enter_callback),
		     widget_entry);

  gtk_widget_show(widget_entry);

  /*
   * Create the combo with the file types
   * ------------------------------------
   */
  if(mode==MODE_SAVE && file_exts && *file_exts != '\0')
  {
    gchar **all_type = g_strsplit(file_exts, " ", 0);
    guint i = 0;

    gtk_combo_filetypes = gtk_combo_box_new_text();

    /* Extract first string */
    while (all_type[i])
      gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_combo_filetypes),
				g_strdup(all_type[i++]) );

    g_strfreev(all_type);

    goo_canvas_widget_new (rootitem,
			   gtk_combo_filetypes,
			   control_area_x1 + 400,
			   control_area_y1,
			   250.0,
			   35.0,
			   NULL);

    gtk_widget_show(gtk_combo_filetypes);
    gtk_combo_box_set_active(GTK_COMBO_BOX(gtk_combo_filetypes), 0);
  }

  /*
   * Buttons
   * -------
   */

  // CANCEL
  gc_util_button_text_svg(rootitem,
			  BOARDWIDTH * 1/3,
			  BOARDHEIGHT - 32,
			  "#BUTTON_TEXT",
			  _("CANCEL"),
			  (GtkSignalFunc) item_event_file_selector,
			  "/cancel/");
  // OK
  gc_util_button_text_svg(rootitem,
			  BOARDWIDTH * 2/3,
			  BOARDHEIGHT - 32,
			  "#BUTTON_TEXT",
			  (mode==MODE_LOAD ? _("LOAD") : _("SAVE")),
			  (GtkSignalFunc) item_event_file_selector,
			  "/cancel/");

  file_selector_displayed = TRUE;

  full_rootdir = g_strconcat(gc_prop_get()->user_dir, "/", rootdir, NULL);
  gc_util_create_rootdir(full_rootdir);

  current_rootdir = full_rootdir;

  display_files(rootitem, full_rootdir);

  return 0;

}

static void
display_files(GooCanvasItem *root_item, gchar *rootdir)
{
  GooCanvasItem *item;
  double iw, ih;
  const gchar *one_dirent;
  GDir  *dir;

  /* Initial image position */
  guint ix  = 0.0;
  guint iy  = 30.0;

  GtkWidget	  *w;
  GooCanvasItem *bg_item;

  GtkWidget *canvas; /* The scrolled part */

  GList  *dir_list  = NULL;
  GList  *file_list = NULL;
  GList  *listrunner;

  GtkAdjustment *adj;

  if(!rootitem)
    return;

  /* Display the directory content */
  dir = g_dir_open(rootdir, 0, NULL);

  if (!dir) {
    g_warning("gcompris_file_selector : no root directory found in %s", rootdir);
    g_free(rootdir);
    return;
  }

  /* Delete the previous file root if any */
  if(file_root_item)
    goo_canvas_item_remove(file_root_item);

  /* Create a root item to put the image list in it */
  file_root_item = goo_canvas_group_new (root_item, NULL);

  /*
   * Create the scrollbar
   * --------------------
   */
  canvas = goo_canvas_new();

  goo_canvas_widget_new (file_root_item,
			 canvas,
			 DRAWING_AREA_X1,
			 DRAWING_AREA_Y1,
			 DRAWING_AREA_X2 - DRAWING_AREA_X1 - 20.0,
			 DRAWING_AREA_Y2 - DRAWING_AREA_Y1 - 35.0,
			 NULL);

  gtk_widget_show (canvas);

  /* Set the new canvas to the background color or it's white */
  bg_item = goo_canvas_rect_new (goo_canvas_get_root_item(GOO_CANVAS(canvas)),
				 0,
				 0,
				 DRAWING_AREA_X2 - DRAWING_AREA_X1 + 200,
				 DRAWING_AREA_Y2 - DRAWING_AREA_Y1,
				 "fill-color-rgba", gc_skin_get_color("gcompris/fileselectbg"),
				 "line-width", 0.0,
				 NULL);


  adj = \
    GTK_ADJUSTMENT (gtk_adjustment_new (0.00, 0.00,
					IMAGE_HEIGHT,
					10, IMAGE_HEIGHT,
					(DRAWING_AREA_Y2 - DRAWING_AREA_Y1)/3)
		    );
  w = gtk_vscrollbar_new (adj);

  goo_canvas_widget_new (file_root_item,
			 w,
			 DRAWING_AREA_X2 - 15.0,
			 DRAWING_AREA_Y1,
			 30.0,
			 DRAWING_AREA_Y2 - DRAWING_AREA_Y1 - 20.0,
			 NULL);
  gtk_widget_show (w);

  /* Set the scrollwheel event */
  g_signal_connect (adj, "value_changed",
		    (GtkSignalFunc) item_event_scroll,
		    canvas);

  /* Display the directory name
   * --------------------------
   */

  item = goo_canvas_text_new (file_root_item,
			      rootdir,
			      (gdouble)control_area_x1,
			      (gdouble)directory_label_y,
			      -1,
			      GTK_ANCHOR_NW,
			      "font", "Sans 7",
			      "fill-color-rgba",
			      gc_skin_get_color("gcompris/fileselectcol"),
			      NULL);


  iw = IMAGE_WIDTH;
  ih = IMAGE_HEIGHT;

  /* Insert all files in a sorted list */

  while((one_dirent = g_dir_read_name(dir)) != NULL)
    {
      gchar *filename;

      filename = g_strdup_printf("%s/%s",
				 rootdir, (gchar*)(one_dirent));

      if(g_file_test(filename, G_FILE_TEST_IS_DIR))
	{
	  dir_list = g_list_insert_sorted(dir_list, filename,
					  (GCompareFunc)strcmp);
	}
      else
	{
	  file_list = g_list_insert_sorted(file_list, filename,
					   (GCompareFunc)strcmp);
	}
    }

  /* Concat the directory list and file list */
  file_list = g_list_concat(dir_list, file_list);

  g_list_free(dir_list);
  dir_list = NULL;

  /* We have the list sorted, now display it */
  listrunner = g_list_first(file_list);
  while(listrunner)
    {
      /* add the file to the display */
      gchar *svg_id;

      gchar *allfilename = listrunner->data;
      gchar *filename    = g_path_get_basename(allfilename);
      gchar *ext = g_strrstr(filename, ".");
      gchar *file_wo_ext = g_strdup(filename);

      if(ext)
	{
	  gchar *ext2 = g_strrstr(file_wo_ext, ".");
	  *ext2 = '\0';
	}

      if(g_file_test(allfilename, G_FILE_TEST_IS_DIR))
	svg_id = "#FOLDER";
      else
	svg_id = "#FILE";

      item = goo_canvas_svg_new (goo_canvas_get_root_item(GOO_CANVAS(canvas)),
				 gc_skin_rsvg_get(),
				 "svg-id", svg_id,
				 NULL);

      SET_ITEM_LOCATION_CENTER(item,
			       ix + (IMAGE_WIDTH + IMAGE_GAP)/2,
			       iy);

      if(g_file_test(allfilename, G_FILE_TEST_IS_DIR))
	{
	  g_signal_connect(item, "button_press_event",
			   (GtkSignalFunc) item_event_directory,
			   allfilename);
	}
      else
	{
	  g_signal_connect(item, "button_press_event",
			   (GtkSignalFunc) item_event_file_selector,
			   allfilename);
	}
      gc_item_focus_init(item, NULL);

      g_object_set_data_full (G_OBJECT (item),
			      "allfilename", allfilename, g_free);
      /* The type */
      if(ext)
	goo_canvas_text_new (goo_canvas_get_root_item(GOO_CANVAS(canvas)),
			     ext,
			     ix + (IMAGE_WIDTH + IMAGE_GAP)/2,
			     iy + 10,
			     -1,
			     GTK_ANCHOR_CENTER,
			     "font", "Sans 6",
			     "fill-color-rgba",
			     gc_skin_get_color("gcompris/fileselectcol"),
			     NULL);

      /* The filename */
      item = goo_canvas_text_new (goo_canvas_get_root_item(GOO_CANVAS(canvas)),
				  file_wo_ext,
				  ix + (IMAGE_WIDTH + IMAGE_GAP)/2,
				  iy + IMAGE_HEIGHT - 30,
				  -1,
				  GTK_ANCHOR_CENTER,
				  "font", "Sans 7",
				  "fill-color-rgba", gc_skin_get_color("gcompris/fileselectcol"),
				  NULL);
      g_free(file_wo_ext);
      g_free(filename);

      if(g_file_test(allfilename, G_FILE_TEST_IS_DIR))
	{
	  g_signal_connect(item, "button_press_event",
			   (GtkSignalFunc) item_event_directory,
			   allfilename);
	}
      else
	{
	  g_signal_connect(item, "button_press_event",
			   (GtkSignalFunc) item_event_file_selector,
			   allfilename);
	}

      ix += IMAGE_WIDTH + IMAGE_GAP;

      if(ix >= DRAWING_AREA_X2 - DRAWING_AREA_X1 -
	 (IMAGE_WIDTH + IMAGE_GAP) )
	{
	  ix=0;

	  iy+=IMAGE_HEIGHT + IMAGE_GAP;

	  goo_canvas_set_bounds (GOO_CANVAS(canvas),
				 0, 0,
				 DRAWING_AREA_X2- DRAWING_AREA_X1,
				 iy + IMAGE_HEIGHT + IMAGE_GAP);

	  if(iy >= DRAWING_AREA_Y2-DRAWING_AREA_Y1)
	    {
	      g_object_set(bg_item,
			   "height", (double)iy + IMAGE_HEIGHT + IMAGE_GAP,
			   NULL);
	      g_object_set(adj,
			   "upper", (double)iy - IMAGE_HEIGHT + IMAGE_GAP - 1,
			   NULL);
	    }
	}
      listrunner = g_list_next(listrunner);
    }

  g_dir_close(dir);
  g_list_free(file_list);

}

/* Callback when a directory is selected */
static gint
item_event_directory(GooCanvasItem *item, GdkEvent *event, gchar *dir)
{

  if(!rootitem)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      if(strcmp(g_path_get_basename(dir), "..")==0) {
	/* Up one level. Remove .. and one directory on the right of the path */
	dir[strlen(dir)-3] = '\0';
	dir=g_path_get_dirname(dir);
      }
      display_files(rootitem, g_strdup(dir));
      gtk_entry_set_text(GTK_ENTRY(widget_entry), "");
      break;
    default:
      break;
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

/* Callback when a file is selected */
static gboolean
item_event_file_selector (GooCanvasItem  *item,
			  GooCanvasItem  *target,
			  GdkEventButton *event,
			  gchar *data)
{
  if(!rootitem)
    return FALSE;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      if(!strcmp(data, "/ok/"))	{

	/* Nothing selected, please cancel instead */
	if(strcmp(gtk_entry_get_text(GTK_ENTRY(widget_entry)), "")==0)
	  return FALSE;

	if(fileSelectorCallBack!=NULL)
	  {
	    gchar *result = NULL;
	    gchar *file_type = NULL;

	    if(mode==MODE_SAVE)
	      {
		GtkTreeModel *model;
		GtkTreeIter iter;

		model = gtk_combo_box_get_model ((GtkComboBox *)gtk_combo_filetypes);
		if (gtk_combo_box_get_active_iter ((GtkComboBox *)gtk_combo_filetypes,
						   &iter))
		  gtk_tree_model_get (model, &iter, 0,
				      &file_type, -1);
	      }

	    result = g_strdup_printf("%s/%s%s",
				     current_rootdir,
				     gtk_entry_get_text(GTK_ENTRY(widget_entry)),
				     (file_type ? file_type :  "") );

	  /* Callback with the proper params */
	  fileSelectorCallBack(result, file_type);

	  if(file_type)
	    g_free(file_type);

	  /* DO NOT FREE RESULT OR PYTHON SIDE WILL BE IN TROUBLE */
	  /* ADDENDUM: DOES NOT HURT ANYMORE, WHY ? */
	  if(result)
	    g_free(result);
	}
	gc_selector_file_stop();
      }
      else if(!strcmp(data, "/cancel/"))
	{
	  gc_selector_file_stop();
	}
      else
	{
	  gchar *ext = g_strrstr(data, ".");
	  gchar *file_wo_ext = g_strdup(data);

	  if(ext)
	    {
	      gchar *ext2 = g_strrstr(file_wo_ext, ".");
	      *ext2 = '\0';
	    }

	  gtk_entry_set_text(GTK_ENTRY(widget_entry),
			     g_path_get_basename(file_wo_ext));
	  g_free(file_wo_ext);
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

  entry_text = (char *)gtk_entry_get_text(GTK_ENTRY(entry));
}

