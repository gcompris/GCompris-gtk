/* gcompris - gcompris.c
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

#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#ifdef WIN32
// WIN32
#elif MAC_INTEGRATION
// MACOSX
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#endif

// Include Mac OS X menu synchronization on native OSX build
#ifdef  MAC_INTEGRATION
#include "ige-mac-menu.h"
#endif

#include <glib/gstdio.h>

#include "gcompris.h"
#include "gc_core.h"
#include "gcompris_config.h"
#include "about.h"
#include "bar.h"
#include "status.h"
#include <locale.h>

#include "binreloc.h"

/* get the default database name */
#define DEFAULT_DATABASE "gcompris_sqlite.db"

static double zoom_factor = 1.0;

/* Multiple instance check */
#define GC_LOCK_FILE "gcompris.lock"
#define GC_LOCK_LIMIT 30 /* seconds */

static GtkWidget *window;
static GtkWidget *workspace;
static GtkWidget *alignment;
static GtkWidget *canvas;
gchar * exec_prefix = NULL;

//static gint pause_board_cb (GtkWidget *widget, gpointer data);
static void quit_cb (GtkWidget *widget, gpointer data);
static void map_cb  (GtkWidget *widget, gpointer data);
static gint board_widget_key_press_callback (GtkWidget   *widget,
					    GdkEventKey *event,
					    gpointer     client_data);
void gc_terminate(int signum);
static void single_instance_release();

/*
 * For the Activation dialog
 */
#ifdef ACTIVATION_CODE
int gc_activation_check(const char *code);
static void activation_enter_callback(GtkWidget *widget,
				      GtkWidget *entry );
static void activation_done();
static int display_activation_dialog();
static GooCanvasItem *activation_item;
static GtkWidget *widget_activation_entry;
#else
#define display_activation_dialog() FALSE
#endif


static GcomprisProperties *properties = NULL;
static gboolean		   is_mapped = FALSE;
static gboolean		   fullscreen;
static gboolean		   mute;
static guint		   gc_cursor_current;

/****************************************************************************/
/* Some constants.  */

static GooCanvasItem *backgroundimg = NULL;
static GooCanvasItem *backgroundsvgimg = NULL;
static gchar           *gc_locale = NULL;
static gchar           *gc_user_default_locale = NULL;
static gboolean		gc_debug = FALSE;

/****************************************************************************/
/* Command line params */

/*** gcompris-popttable */
static gint popt_fullscreen	   = FALSE;
static gint popt_window		   = FALSE;
static gint popt_sound		   = FALSE;
static gint popt_mute		   = FALSE;
static gint popt_cursor		   = FALSE;
static gint popt_nocursor	   = FALSE;
static gint popt_version	   = FALSE;
static gint popt_difficulty_filter = FALSE;
static gint popt_debug		   = FALSE;
static gint popt_nolockcheck	   = FALSE;
static gchar *popt_root_menu       = NULL;
static gchar *popt_package_data_dir = NULL;
static gchar *popt_package_skin_dir = NULL;
static gchar *popt_plugin_dir      = NULL;
static gchar *popt_python_plugin_dir = NULL;
static gchar *popt_locale_dir      = NULL;
static gchar *popt_menu_dir        = NULL;
static gint popt_administration	   = FALSE;
static gchar *popt_database        = NULL;
static gint popt_create_db   	   = FALSE;
static gint popt_reread_menu   	   = FALSE;
static gchar *popt_profile	   = NULL;
static gint *popt_profile_list	   = FALSE;
static gchar *popt_config_dir	   = NULL;
static gchar *popt_user_dir	   = NULL;
static gint  popt_experimental     = FALSE;
static gint  popt_no_quit	   = FALSE;
static gint  popt_no_config        = FALSE;
static gint  popt_no_level         = FALSE;
static gchar *popt_server          = NULL;
static gint  *popt_web_only        = NULL;
static gchar *popt_cache_dir       = NULL;
static gchar *popt_drag_mode       = NULL;
static gint popt_sugar_look        = FALSE;
static gint popt_no_zoom           = FALSE;
static gint popt_test              = FALSE;
static gdouble popt_timing_base    = 1.0;
static gdouble popt_timing_mult    = 1.0;

static GOptionEntry options[] = {
  {"fullscreen", 'f', 0, G_OPTION_ARG_NONE, &popt_fullscreen,
   N_("run GCompris in fullscreen mode."), NULL},

  {"window", 'w', 0, G_OPTION_ARG_NONE, &popt_window,
   N_("run GCompris in window mode."), NULL},

  {"sound", 's', 0, G_OPTION_ARG_NONE, &popt_sound,
   N_("run GCompris with sound enabled."), NULL},

  {"mute", 'm', 0, G_OPTION_ARG_NONE, &popt_mute,
   N_("run GCompris without sound."), NULL},

  {"cursor", 'c', 0, G_OPTION_ARG_NONE, &popt_cursor,
   N_("run GCompris with the default system cursor."), NULL},

  {"nocursor", 'C', 0, G_OPTION_ARG_NONE, &popt_nocursor,
   N_("run GCompris without cursor (touch screen mode)."), NULL},

  {"difficulty", 'd', 0, G_OPTION_ARG_INT, &popt_difficulty_filter,
   N_("display only activities with this difficulty level."), NULL},

  {"debug", 'D', 0, G_OPTION_ARG_NONE, &popt_debug,
   N_("display debug informations on the console."), NULL},

  {"version", 'v', 0, G_OPTION_ARG_NONE, &popt_version,
   N_("Print the version of " PACKAGE), NULL},

  {"root-menu", 'l', 0, G_OPTION_ARG_STRING, &popt_root_menu,
   N_("Run GCompris with local menu"
      " (e.g -l /reading will let you play only activities in the reading directory, -l /strategy/connect4 only the connect4 activity)."
      " Use '-l list' to list all the availaible activities and their descriptions."), NULL},

  {"package_data_dir", 'A', 0, G_OPTION_ARG_STRING, &popt_package_data_dir,
   N_("GCompris will find the data dir in this directory"), NULL},

  {"package_skin_dir", 'S', 0, G_OPTION_ARG_STRING, &popt_package_skin_dir,
   N_("GCompris will find the skins in this directory"), NULL},

  {"plugin_dir", 'L', 0, G_OPTION_ARG_STRING, &popt_plugin_dir,
   N_("GCompris will find the activity plugins in this directory"), NULL},

  {"python_plugin_dir", 'P', 0, G_OPTION_ARG_STRING, &popt_python_plugin_dir,
   N_("GCompris will find the python activity in this directory"), NULL},

  {"locale_dir", '\0', 0, G_OPTION_ARG_STRING, &popt_locale_dir,
   N_("GCompris will find the locale file (.mo translation) in this directory"), NULL},

  {"menu_dir", 'M', 0, G_OPTION_ARG_STRING, &popt_menu_dir,
   N_("GCompris will find the activities menu in this directory"), NULL},

  {"administration", 'a', 0, G_OPTION_ARG_NONE, &popt_administration,
   N_("Run GCompris in administration and user-management mode"), NULL},

  {"database", 'b', 0, G_OPTION_ARG_STRING, &popt_database,
   N_("Use alternate database for profiles [$HOME/.config/gcompris/gcompris_sqlite.db]"), NULL},

  {"create-db",'\0', 0, G_OPTION_ARG_NONE, &popt_create_db,
   N_("Create the alternate database for profiles"), NULL},

  {"reread-menu",'\0', 0, G_OPTION_ARG_NONE, &popt_reread_menu,
   N_("Re-read XML Menus and store them in the database"), NULL},

  {"profile",'p', 0, G_OPTION_ARG_STRING, &popt_profile,
   N_("Set the profile to use. Use 'gcompris -a' to create profiles"), NULL},

  {"profile-list",'\0', 0, G_OPTION_ARG_NONE, &popt_profile_list,
   N_("List all available profiles. Use 'gcompris -a' to create profiles"), NULL},

  {"config-dir",'\0', 0, G_OPTION_ARG_STRING, &popt_config_dir,
   N_("Config directory location: [$HOME/.config/gcompris]. Alternate is to set $XDG_CONFIG_HOME."), NULL},

  {"user-dir",'\0', 0, G_OPTION_ARG_STRING, &popt_user_dir,
   N_("The location of user directories: [$HOME/My GCompris]"), NULL},

  {"experimental",'\0', 0, G_OPTION_ARG_NONE, &popt_experimental,
   N_("Run the experimental activities"), NULL},

  {"disable-quit",'\0', 0, G_OPTION_ARG_NONE, &popt_no_quit,
   N_("Disable the quit button"), NULL},

  {"disable-config",'\0', 0, G_OPTION_ARG_NONE, &popt_no_config,
   N_("Disable the config button"), NULL},

  {"disable-level",'\0', 0, G_OPTION_ARG_NONE, &popt_no_level,
   N_("Disable the level button"), NULL},

  {"server", '\0', 0, G_OPTION_ARG_STRING, &popt_server,
   N_("GCompris will get images, sounds and activity data from this server if not found locally."), NULL},

  {"web-only", '\0', 0, G_OPTION_ARG_NONE, &popt_web_only,
   N_("Only when --server is provided, disable check for local resource first."
      " Data are always taken from the web server."), NULL},

  {"cache-dir", '\0', 0, G_OPTION_ARG_STRING, &popt_cache_dir,
   N_("In server mode, specify the cache directory used to avoid useless downloads."), NULL},

  {"drag-mode", 'g', 0, G_OPTION_ARG_STRING, &popt_drag_mode,
   N_("Global drag and drop mode: normal, 2clicks, both. Default mode is normal."), NULL},

  {"nolockcheck", '\0', 0, G_OPTION_ARG_NONE, &popt_nolockcheck,
   N_("Do not avoid the execution of multiple instances of GCompris."), NULL},

  {"sugar",'\0', 0, G_OPTION_ARG_NONE, &popt_sugar_look,
   ("Use Sugar DE look&feel"), NULL},

  {"no-zoom",'\0', 0, G_OPTION_ARG_NONE, &popt_no_zoom,
   N_("Disable maximization zoom"), NULL},

  {"timing-base",'\0', 0, G_OPTION_ARG_DOUBLE, &popt_timing_base,
   N_("Increase activities' timeout delays; useful values > 1.0; 1.0 to not change hardcoded value"), NULL},

  {"timing-mult",'\0', 0, G_OPTION_ARG_DOUBLE, &popt_timing_mult,
   N_("How activities' timeout delays are growing for several actors; useful values < 1.0; 1.0 to not change hardcoded value"), NULL},

  {"test",'\0', 0, G_OPTION_ARG_NONE, &popt_test,
   N_("For test purpose, run in a loop all the activities"), NULL},

  { NULL }
};

/* Remove any dialog box */
static void gc_close_all_dialog() {
  gc_dialog_close();
  gc_help_stop();
  gc_config_stop();
  gc_about_stop();
  gc_selector_file_stop();
  gc_selector_images_stop();
}

/* Return the zoom factor we are currently using for our window
 *
 */
double gc_zoom_factor_get()
{
  return zoom_factor;
}

static gint
_gc_size_allocate_event_callback (GtkWidget   *widget,
			      GtkAllocation *allocation,
			      gpointer     client_data)
{
  double xratio, yratio;
  double canvas_width, canvas_height;

  yratio=allocation->height/(float)(BOARDHEIGHT);
  xratio=allocation->width/(float)BOARDWIDTH;
  zoom_factor = MIN(xratio, yratio);
  g_message("The screen_width=%f screen_height=%f ratio=%f\n",
	    (double)allocation->width, (double)allocation->height, zoom_factor);

  if (!properties->zoom && zoom_factor > 1.)
      zoom_factor = 1.;

  canvas_width = BOARDWIDTH * zoom_factor;
  canvas_height = BOARDHEIGHT * zoom_factor;

  gtk_widget_set_size_request(canvas, canvas_width, canvas_height);
  goo_canvas_set_scale (GOO_CANVAS(canvas), zoom_factor);

  return FALSE;
}

static gint
board_widget_key_press_callback (GtkWidget   *widget,
				 GdkEventKey *event,
				 gpointer     client_data)
{
  int kv = event->keyval;

  if(event->state & GDK_CONTROL_MASK && ((event->keyval == GDK_b)
					 || (event->keyval == GDK_B))) {
    if(properties->bar_hidden){
      g_message("Restoring the control bar\n");
      properties->bar_hidden=FALSE;
      gc_bar_hide(properties->bar_hidden);
    }
    else{
      g_message("Hidding the control bar\n");
      properties->bar_hidden=TRUE;
      gc_bar_hide(properties->bar_hidden);
    };
    goo_canvas_update(GOO_CANVAS(canvas));
    return TRUE;
  }
  if(event->state & GDK_CONTROL_MASK && ((event->keyval == GDK_r)
					 || (event->keyval == GDK_R))) {
    g_message("Refreshing the canvas\n");
    goo_canvas_update(GOO_CANVAS(canvas));
    return TRUE;
  }
  else if(event->state & GDK_CONTROL_MASK && ((event->keyval == GDK_q)
					 || (event->keyval == GDK_Q))) {
    gc_exit();
    return TRUE;
  }
  else if(event->state & GDK_CONTROL_MASK && ((event->keyval == GDK_w)
					 || (event->keyval == GDK_W))) {
    gc_close_all_dialog();

    if (gc_board_get_current()->previous_board != NULL)
      gc_board_stop();
    return TRUE;
  }
  else if(event->state & GDK_CONTROL_MASK && ((event->keyval == GDK_f)
					 || (event->keyval == GDK_F))) {
    /* Toggle fullscreen */
    if (fullscreen)
      gc_fullscreen_set(FALSE);
    else
      gc_fullscreen_set(TRUE);
    return TRUE;
  }
  else if(event->state & GDK_CONTROL_MASK && ((event->keyval == GDK_m)
					 || (event->keyval == GDK_M))) {
    /* Toggle Mute */
    if (mute)
	gc_sound_bg_resume();
    else
	gc_sound_bg_pause();
    mute = ! mute;
    return TRUE;
  }

  switch (event->keyval)
    {
    case GDK_Escape:
      gc_close_all_dialog();

      if (gc_board_get_current()->previous_board != NULL)
	gc_board_stop();
      return TRUE;
    case GDK_F5:
      g_message("Refreshing the canvas\n");
      goo_canvas_update(GOO_CANVAS(canvas));
      return TRUE;

    case GDK_KP_Multiply:
      break;
    case GDK_KP_0:
    case GDK_KP_Insert:
      event->keyval=GDK_0;
      break;
    case GDK_KP_1:
    case GDK_KP_End:
      event->keyval=GDK_1;
      break;
    case GDK_KP_2:
    case GDK_KP_Down:
      event->keyval=GDK_2;
      break;
    case GDK_KP_3:
    case GDK_KP_Page_Down:
      event->keyval=GDK_3;
      break;
    case GDK_KP_4:
    case GDK_KP_Left:
      event->keyval=GDK_4;
      break;
    case GDK_KP_5:
    case GDK_KP_Begin:
      event->keyval=GDK_5;
      break;
    case GDK_KP_6:
    case GDK_KP_Right:
      event->keyval=GDK_6;
      break;
    case GDK_KP_7:
    case GDK_KP_Home:
      event->keyval=GDK_7;
      break;
    case GDK_KP_8:
    case GDK_KP_Up:
      event->keyval=GDK_8;
      break;
    case GDK_KP_9:
    case GDK_KP_Page_Up:
      event->keyval=GDK_9;
      break;
    default:
      break;
    }

  /* pass through the IM context */
  if (gc_board_get_current() && (!gc_board_get_current()->disable_im_context))
    {
      if (gtk_im_context_filter_keypress (properties->context, event))
	{
	  g_message("%d key is handled by context", kv);
	  return TRUE;
	}
    }

  g_message("%d key is NOT handled by context", kv);
  /* If the board needs to receive key pressed */
  /* NOTE: If a board receives key press, it must bind the ENTER Keys to OK
   *       whenever possible
   */
  if (gc_board_get_current_board_plugin()!=NULL
      && gc_board_get_current_board_plugin()->key_press)
    {
      return(gc_board_get_current_board_plugin()->key_press (event->keyval, NULL, NULL));
    }
  else if (gc_board_get_current_board_plugin()!=NULL
	   && gc_board_get_current_board_plugin()->ok &&
	   (event->keyval == GDK_KP_Enter ||
	    event->keyval == GDK_Return   ||
	    event->keyval == GDK_KP_Space))
    {
      /* Else we send the OK signal. */
      gc_board_get_current_board_plugin()->ok ();
      return TRUE;
    }

  /* Event not handled; try parent item */
  return FALSE;
};

guint gc_cursor_get()
{
  return gc_cursor_current;
}

/* Set the cursor to be invisible
 * (Useful for touch screens)
 */
void gc_cursor_hide()
{
  char in_cursor[] = {0x0};
  GdkCursor *cursor;
  GdkBitmap *bp;
  GdkColor color = {0, 0, 0, 0};

  bp = gdk_bitmap_create_from_data(NULL , in_cursor , 1, 1);
  cursor = gdk_cursor_new_from_pixmap(bp , bp ,
                                      &color , &color ,
                                      1 , 1);
  gdk_window_set_cursor(window->window , cursor);
  gdk_cursor_unref(cursor);
}


#if 0
  // READY FOR GTK 3
void gc_cursor_hide()
{

  GdkCursor *cursor;
  cairo_surface_t *s;
  GdkPixbuf *pixbuf;
  cairo_t *cr;

  s = cairo_image_surface_create (CAIRO_FORMAT_A1, 1, 1);
  cr = cairo_create (s);
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  cairo_rectangle(cr, 0, 0, 1, 1);
  cairo_fill(cr);
  cairo_destroy(cr);

  pixbuf = gdk_pixbuf_get_from_surface (s,
					0, 0,
					1, 1);
  cairo_surface_destroy (s);

  cursor = gdk_cursor_new_from_pixbuf (NULL, pixbuf, 0, 0);

  g_object_unref (pixbuf);

  gdk_window_set_cursor(window->window , cursor);
  gdk_cursor_unref(cursor);
}
#endif

void gc_cursor_set(guint gdk_cursor_type)
{
  GdkCursor *cursor = NULL;

  if (properties->nocursor)
    return;

  // Little hack to force gcompris to use the default cursor
  if(gdk_cursor_type==GCOMPRIS_DEFAULT_CURSOR)
    gdk_cursor_type=properties->defaultcursor;

  // I suppose there is less than GCOMPRIS_FIRST_CUSTOM_CURSOR cursors defined in gdkcursors.h !
  if (gdk_cursor_type < GCOMPRIS_FIRST_CUSTOM_CURSOR) {
    cursor = gdk_cursor_new(gdk_cursor_type);
    gdk_window_set_cursor (window->window, cursor);
    gdk_cursor_unref(cursor);
  } else { // we use a custom cursor
    GdkPixbuf *cursor_pixbuf = NULL;

    switch (gdk_cursor_type) {
    case GCOMPRIS_DEFAULT_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load_or_null("cursors/default.png");
      break;
    case GCOMPRIS_LINE_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load_or_null("cursors/line.png");
      break;
    case GCOMPRIS_RECT_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load_or_null("cursors/rect.png");
      break;
    case GCOMPRIS_FILLRECT_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load_or_null("cursors/fillrect.png");
      break;
    case GCOMPRIS_CIRCLE_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load_or_null("cursors/circle.png");
      break;
    case GCOMPRIS_FILLCIRCLE_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load_or_null("cursors/fillcircle.png");
      break;
    case GCOMPRIS_FILL_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load_or_null("cursors/fill.png");
      break;
    case GCOMPRIS_DEL_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load_or_null("cursors/del.png");
      break;
    case GCOMPRIS_SELECT_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load_or_null("cursors/select.png");
      break;
    default :
      return;
      break;
    }

    if(cursor_pixbuf)
      {
	cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(),
					    cursor_pixbuf, 0, 0);
	gdk_window_set_cursor(window->window, cursor);
	gdk_cursor_unref(cursor);
	gdk_pixbuf_unref(cursor_pixbuf);
      }
    else
      {
	/* The cursor image was not found, falback to default one */
	properties->defaultcursor = GDK_LEFT_PTR;
	gc_cursor_set(GCOMPRIS_DEFAULT_CURSOR);
	return;
      }
  }
  gc_cursor_current = gdk_cursor_type;
}

/**
 * Return the main canvas we run in
 */
GooCanvas *gc_get_canvas()
{
  return GOO_CANVAS (canvas);
}

GtkWidget *gc_get_window()
{
  return window;
}

static void
_set_svg_background(GooCanvasItem *parent, gchar *file, gchar *id)
{
  RsvgHandle *rsvg_handle;

  rsvg_handle = gc_rsvg_load (file);

  if(backgroundsvgimg)
    g_object_set(backgroundsvgimg,
		 "svg-handle", rsvg_handle,
		 "svg-id", id,
		 NULL);
  else
    backgroundsvgimg = goo_canvas_svg_new (parent,
					 rsvg_handle,
					 NULL);
  goo_canvas_item_lower(backgroundsvgimg, NULL);

  g_object_unref(rsvg_handle);
}

static void
_set_pixmap_background(GooCanvasItem *parent, gchar *file)
{
  GdkPixbuf *background_pixmap;

  background_pixmap = gc_pixmap_load (file);

  if(backgroundimg)
    g_object_set(backgroundimg,
		 "pixbuf", background_pixmap,
		 NULL);
  else
    backgroundimg = goo_canvas_image_new (parent,
					  background_pixmap,
					  0.0,
					  0.0,
					  "width", (gdouble) BOARDWIDTH,
					  "height", (gdouble) BOARDHEIGHT,
					  NULL);
  goo_canvas_item_lower(backgroundimg, NULL);

  gdk_pixbuf_unref(background_pixmap);

}

void _clear_svg_background()
{
  if(backgroundsvgimg)
    goo_canvas_item_remove(backgroundsvgimg);

  backgroundsvgimg = NULL;
}

void _clear_pixmap_background()
{
  if(backgroundimg)
    goo_canvas_item_remove(backgroundimg);

  backgroundimg = NULL;
}

void
gc_set_background(GooCanvasItem *parent, gchar *file)
{
  g_assert(parent);
  g_assert(file);

  if(g_str_has_suffix(file, ".svg") ||
     g_str_has_suffix(file, ".svgz"))
    {
      if(backgroundimg)
	goo_canvas_item_remove(backgroundimg);

      backgroundimg = NULL;
      _set_svg_background(parent, file, NULL);
    }
  else
    {
      _clear_svg_background();
      _set_pixmap_background(parent, file);
    }
}

void
gc_set_background_by_id(GooCanvasItem *parent, RsvgHandle *rsvg_handle,
			gchar *id)
{
  g_assert(parent);
  g_assert(rsvg_handle);

  _clear_pixmap_background();

  if(backgroundsvgimg)
    g_object_set(backgroundsvgimg,
		 "svg-handle", rsvg_handle,
		 "svg-id", id,
		 NULL);
  else
    backgroundsvgimg = goo_canvas_svg_new (parent,
					   rsvg_handle,
					   "svg-id", id,
					   NULL);

  goo_canvas_item_lower(backgroundsvgimg, NULL);
}

void
gc_set_default_background(GooCanvasItem *parent)
{
  gc_set_background_by_id(parent,
			  gc_skin_rsvg_get(),
			  "#BACKGROUND");
}

static void
init_workspace()
{
  workspace = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window), workspace);

  GtkWidget *background = gtk_event_box_new();
  GdkColor black = {0, 0, 0, 0};
  gtk_widget_modify_bg(background, GTK_STATE_NORMAL, &black);
  gtk_box_pack_end(GTK_BOX(workspace), background, TRUE, TRUE, 0);

  alignment = gtk_alignment_new(.5, .5, 0., 0.);
  g_signal_connect (GTK_OBJECT (alignment), "size-allocate",
		      G_CALLBACK (_gc_size_allocate_event_callback), NULL);
  gtk_container_add(GTK_CONTAINER(background), alignment);

  gtk_container_add(GTK_CONTAINER(alignment), canvas);
  gtk_widget_set_size_request (GTK_WIDGET(canvas), BOARDWIDTH, BOARDHEIGHT);
  goo_canvas_set_bounds (GOO_CANVAS(canvas),
			 0, 0,
			 BOARDWIDTH,
			 BOARDHEIGHT);
  g_object_set (G_OBJECT(canvas), "background-color", "#000", NULL);

#ifdef MAC_INTEGRATION
  GtkWidget *quit_item;
  quit_item = gtk_menu_item_new();
  ige_mac_menu_set_quit_menu_item(GTK_MENU_ITEM (quit_item));
  g_signal_connect(GTK_OBJECT (quit_item),
		   "activate", G_CALLBACK (quit_cb), NULL);
  gtk_widget_show (quit_item);

#endif

}

static GcomprisBoard *get_board_to_start()
{
  GcomprisBoard *board_to_start;

  /* By default, the menu will be started */
  board_to_start = properties->menu_board;

  /* Get and Run the root menu */
  if(properties->administration)
    {
      board_to_start = gc_menu_section_get("/administration/administration");
    }
  else
    {
      /* If we have a profile defined, run the login screen
       * (the login screen is a board that uppon login completion
       * starts the menu)
       */
      if(properties->profile && properties->profile->group_ids)
	{
	  gboolean found = FALSE;

	  GList *group_id;

	  for (group_id = properties->profile->group_ids; group_id != NULL; group_id = group_id->next)
	    if (g_list_length(gc_db_users_from_group_get( *((int *) group_id->data))) > 0){
	      found = TRUE;
	      break;
	    }

	  /* No profile start normally */
	  if (found)
	    board_to_start = gc_menu_section_get("/login/login");
	  else {
	    board_to_start = gc_menu_section_get(properties->root_menu);
	    /* this will set user information to system one */
	    gc_profile_set_current_user(NULL);
	  }
	}
      else
	/* this will set user information to system one */
	gc_profile_set_current_user(NULL);
    }

  return board_to_start;
}

static void setup_window ()
{
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  /*
   * Set an icon for gcompris
   * ------------------------
   */
  {
    GdkPixbuf *icon_pixbuf = NULL;
    gchar *iconfile = gc_file_find_absolute("%s/%s",
					    properties->system_icon_dir, "gcompris.png",
					    NULL);
    if(iconfile)
      {
	icon_pixbuf = gdk_pixbuf_new_from_file(iconfile, NULL);
	g_free(iconfile);

	if (icon_pixbuf)
	  {
	    gtk_window_set_icon (GTK_WINDOW (window), icon_pixbuf);
	    gdk_pixbuf_unref (icon_pixbuf);
	  }
      }
    else
      g_message ("Failed to find icon file: 'gcompris.png'");

  }

  gtk_window_set_title(GTK_WINDOW (window), "GCompris");

  /*
   * Set the main window
   * -------------------
   */

  gtk_window_set_default_size(GTK_WINDOW(window), BOARDWIDTH, BOARDHEIGHT);
  gtk_window_set_wmclass(GTK_WINDOW(window), "gcompris", "GCompris");
  gtk_widget_realize (window);

  g_signal_connect (GTK_OBJECT (window), "delete_event",
		    G_CALLBACK (quit_cb), NULL);

  g_signal_connect (GTK_OBJECT (window), "map_event",
		    G_CALLBACK (map_cb), NULL);

  // Set the cursor
  if (properties->nocursor)
    gc_cursor_hide();
  else
    gc_cursor_set(GCOMPRIS_DEFAULT_CURSOR);

  canvas     = goo_canvas_new();

  g_object_set (canvas,
		"has-tooltip", TRUE,
		NULL);

  g_object_set (G_OBJECT(goo_canvas_get_root_item(GOO_CANVAS(canvas))),
		"can-focus", TRUE,
		NULL);


  g_signal_connect_after (GTK_OBJECT (window), "key_press_event",
			  (GCallback) board_widget_key_press_callback, 0);
  g_signal_connect_after (canvas,
			  "key_press_event",
			  (GCallback) board_widget_key_press_callback, 0);

  gc_im_init(window);

  init_workspace();

  gtk_widget_set_can_focus(canvas, TRUE);
  gtk_widget_grab_focus (canvas);

}

#ifdef ACTIVATION_CODE
/** Display the activation dialog for the windows version
 *
 * return TRUE is the dialog is display, FALSE instead.
 */
int
display_activation_dialog()
{
  int board_count = 0;
  int gc_board_number_in_demo = 0;
  GList *list;
  gint  key_is_valid = 0;

  key_is_valid = gc_activation_check(properties->key);

  if(key_is_valid == 1)
    return FALSE;
  else if(key_is_valid == 2)
    {
      g_free ( gc_prop_get()->key );
      gc_prop_get()->key = strdup("");
      gc_prop_save(properties);
    }
  /* Count non menu boards */
  for (list = gc_menu_get_boards(); list != NULL; list = list->next)
    {
      GcomprisBoard *board = list->data;
      if (strcmp(board->type, "menu") != 0 &&
	  strncmp(board->section, "/experimental", 13) != 0)
	{
	  board_count++;
	  if(board->demo)
	      gc_board_number_in_demo++;
	}
    }

  /* Entry area */
  widget_activation_entry = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(widget_activation_entry), 6);
  activation_item = \
    goo_canvas_widget_new (goo_canvas_get_root_item(GOO_CANVAS(canvas)),
			   GTK_WIDGET(widget_activation_entry),
			   BOARDWIDTH / 2 - 50,
			   BOARDHEIGHT - 90,
			   100.0,
			   30.0,
			   NULL);
  g_signal_connect(GTK_OBJECT(widget_activation_entry), "activate",
		   GTK_SIGNAL_FUNC(activation_enter_callback),
		   NULL);

  char *msg = g_strdup_printf( \
      _("GCompris is free software released under the GPL License. "
	"In order to support its development, this version "
	"provides only %d of the %d activities. You can get the "
	"full version for a small fee at\n<http://gcompris.net>\n"
	"The GNU/Linux version does not have this restriction. "
	"If you also believe that we should teach freedom to children, "
	"please consider using GNU/Linux. Get more information at "
	"FSF:\n<http://www.fsf.org/philosophy>"),
      gc_board_number_in_demo, board_count);
  gc_dialog(msg, activation_done);
  g_free(msg);

  gtk_widget_show(GTK_WIDGET(widget_activation_entry));
  gtk_entry_set_text(GTK_ENTRY(widget_activation_entry), "CODE");
  gtk_widget_grab_focus(GTK_WIDGET(widget_activation_entry));
  return TRUE;
}

/**
 * Return -1 if the code is not valid
 *        0  if the code is valid but out of date
 *        1  if the code is valid and under 2 years
 *        2  for the demo code
 */
gint gc_activation_check(const char *code)
{
#ifdef  DISABLE_ACTIVATION_CODE
  return 1;
#else
  int value = 0;
  int i;
  char crc1 = 0;
  char crc2 = 0;
  char codeddate[5];

  if (properties->administration)
    return 1;

  if(strlen(code) != 6)
    return -1;

  // A special code to test the full version without
  // saving the activation
  if (strncmp(code, "123321", 6) == 0)
    {
      return 2;
    }

  for(i=3; i>=0; i--)
    {
      value |= code[i] & 0x07;
      value = value << 3;
      crc1 = (crc1 ^ code[i]) & 0x07;
    }
  value = value >> 3;
  crc1 = 0x30 | crc1;
  crc2 = 0x30 | (code[2] ^ code[3]);

  if(crc1 != code[4])
    return(-1);

  if(crc2 != code[5])
    return(-1);

  codeddate[3] = 0x30 | (value & 0x000F);
  value = value >> 4;

  codeddate[2] = 0x30 | (value & 0x0001);
  value = value >> 1;

  codeddate[1] = 0x30 | (value & 0x000F);
  value = value >> 4;

  codeddate[0] = 0x30 | (value & 0x0003);
  codeddate[4] = '\0';

  if(atoi(codeddate) + 200 >= atoi(BUILD_DATE))
    return(1);
  else
    return(0);
#endif
}

/* Check the activation code
 *
 */
static void
activation_enter_callback( GtkWidget *entry,
			   GtkWidget *notused )
{
  const char *code = gtk_entry_get_text(GTK_ENTRY(entry));

  switch(gc_activation_check(code))
    {
    case 1:
    case 2:
      gc_prop_get()->key = strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
      gc_prop_save(properties);
      gtk_entry_set_text(GTK_ENTRY(entry), "GOOD");
      break;
    case 0:
      gtk_entry_set_text(GTK_ENTRY(entry), "EXPIRE");
      break;
    case -1:
      gtk_entry_set_text(GTK_ENTRY(entry), "WRONG");
      break;
    }
}

/* Callback for the activation dialog
 *
 */
static void
activation_done()
{
  if ((strcmp((char *)gtk_entry_get_text(GTK_ENTRY(widget_activation_entry)), "CODE") != 0) &&
      (strcmp((char *)gtk_entry_get_text(GTK_ENTRY(widget_activation_entry)), "GOOD") != 0) &&
      (strcmp((char *)gtk_entry_get_text(GTK_ENTRY(widget_activation_entry)), "WRONG") != 0))
    {
      activation_enter_callback(GTK_WIDGET(widget_activation_entry), NULL);
    }

  gc_board_play( get_board_to_start());
  goo_canvas_item_remove (activation_item);
}
#endif

/** Call me to end an activity
 *
 */
void gc_board_end()
{
  if (gc_board_get_current()->previous_board) {
    /* Run the previous board */
    gc_board_play(gc_board_get_current()->previous_board);
  }
}

void _set_geometry_hints(gboolean state)
{
  GdkGeometry hints;
  hints.base_width = 615;
  hints.base_height = 400;
  hints.min_width = hints.base_width;
  hints.min_height = hints.base_height;
  hints.width_inc = 1;
  hints.height_inc = 1;
  hints.min_aspect = (float)BOARDWIDTH/BOARDHEIGHT;
  if (state)
    /* Warning: this is a workaround for GTK-OSX */
    hints.max_aspect = (float)10; /* Fullscreen case */
  else
    hints.max_aspect = (float)BOARDWIDTH/BOARDHEIGHT;
  gint geom_mask = GDK_HINT_RESIZE_INC |
                   GDK_HINT_MIN_SIZE |
                   GDK_HINT_BASE_SIZE;
  if (!popt_sugar_look)
      geom_mask |= GDK_HINT_ASPECT;
  gtk_window_set_geometry_hints (GTK_WINDOW (window), NULL, &hints, geom_mask);
}

/** \brief toggle full screen mode
 *
 *
 */
void gc_fullscreen_set(gboolean state)
{
  static gint window_x = 0;
  static gint window_y = 0;
  static gint window_w = BOARDWIDTH;
  static gint window_h = BOARDHEIGHT;

  fullscreen = state;
  _set_geometry_hints(state);
  if(state)
    {
      gtk_window_get_position ( (GtkWindow*)( window ), &window_x, &window_y );
      gtk_window_get_size ( GTK_WINDOW ( window ), &window_w, &window_h );
#ifdef WIN32
      // WARNING: Doing this is required on Windows
      //          but keep the window hidden on GNU/Linux
      gtk_widget_hide ( window );
#elif MAC_INTEGRATION
      // WARNING: Doing this is required on MacOSX
      //          but keep the window hidden on GNU/Linux
      gtk_widget_hide ( window );
#else
#endif
      gtk_window_set_decorated ( GTK_WINDOW ( window ), FALSE );
      gtk_window_set_type_hint ( GTK_WINDOW ( window ),
				 GDK_WINDOW_TYPE_HINT_DESKTOP );
      if (popt_sugar_look)
	gtk_window_maximize ( GTK_WINDOW( window ) );
      else
        gtk_window_fullscreen ( GTK_WINDOW ( window ) );

      gtk_window_move ( GTK_WINDOW ( window ), 0, 0 );

      GdkScreen *screen = gtk_window_get_screen ( GTK_WINDOW ( window ) );
      gtk_window_resize ( GTK_WINDOW ( window ),
			  gdk_screen_get_width (screen),
			  gdk_screen_get_height (screen) );
    }
  else
    {
      gtk_widget_hide ( window );
      gtk_window_set_type_hint ( GTK_WINDOW ( window ),
				 GDK_WINDOW_TYPE_HINT_NORMAL );
      gtk_window_unfullscreen ( GTK_WINDOW ( window ) );
      gtk_window_set_decorated ( GTK_WINDOW ( window ), TRUE );
      gtk_window_move ( GTK_WINDOW ( window ), window_x, window_y );
      gtk_window_resize ( GTK_WINDOW ( window ), window_w, window_h );
    }
  gtk_window_present ( GTK_WINDOW ( window ) );

}

/* Use these instead of the goo_canvas ones for proper fullscreen mousegrab
   handling. */
int gc_canvas_item_grab (GooCanvasItem *item, unsigned int event_mask,
			 GdkCursor *cursor, guint32 etime)
{
  int retval;

  retval = goo_canvas_pointer_grab(goo_canvas_item_get_canvas(item), item,
				   event_mask, cursor, etime);
  if (retval != GDK_GRAB_SUCCESS)
    return retval;

  return retval;
}

void gc_canvas_item_ungrab (GooCanvasItem *item, guint32 etime)
{
  goo_canvas_pointer_ungrab(goo_canvas_item_get_canvas(item), item, etime);
}

static void cleanup()
{
  /* Do not loopback in exit */
  signal(SIGINT,  NULL);
  signal(SIGSEGV, NULL);

  single_instance_release(); /* Must be done before property destroy */
  gc_board_stop();
  gc_db_exit();
  gc_fullscreen_set(FALSE);
  gc_menu_destroy();
  gc_net_destroy();
  gc_cache_destroy();
  gc_prop_destroy(gc_prop_get());
}

void gc_exit()
{
  g_signal_emit_by_name(G_OBJECT(window), "delete_event");
}

static void quit_cb (GtkWidget *widget, gpointer data)
{

#ifdef DMALLOC
  dmalloc_shutdown();
#endif
  cleanup();
  gtk_main_quit();
  gc_sound_close();

  /*
   * Very important or GCompris crashes on exit when closed from the dialog
   * It's like if code in the dialog callback continue after the gtk_main_quit is done
   */
  exit(0);
}

/*
 * We want GCompris to be set as fullscreen the later possible
 *
 */
static void map_cb (GtkWidget *widget, gpointer data)
{
  if(is_mapped == FALSE)
    {
      is_mapped = TRUE;
      GcomprisBoard *board_to_start;

      if (! gc_skin_load(properties->skin) )
	{
	  gc_status_init("");
	  gchar *filename = \
	    g_strdup_printf("%s/%s/skin.xml",
			    properties->package_skin_dir,
			    properties->skin);

	  gc_status_set_msg(_("Failed to load the skin '%s'"
			      " (Check the file exists and is readable)"),
			    filename);
	  g_free(filename);
	  return;
	}

      gc_set_default_background (goo_canvas_get_root_item (GOO_CANVAS(canvas)));

      gc_fullscreen_set(properties->fullscreen);

      gc_status_init("");

      gc_board_init();
      /* Load all the menu once */
      gc_menu_load();
      /* Save the root_menu */
      properties->menu_board = gc_menu_section_get(properties->root_menu);

      gc_bar_start(GTK_CONTAINER(workspace), GOO_CANVAS(canvas));

      gc_status_close();

      board_to_start = get_board_to_start();

      if(!board_to_start) {
	gchar *tmpstr= g_strdup_printf("Couldn't find the board menu %s, or plugin execution error", properties->root_menu);
	gc_dialog(tmpstr, NULL);
	g_free(tmpstr);
      } else if(!gc_board_check_file(board_to_start) || gc_board_is_demo_only(board_to_start) ) {
	gchar *tmpstr= g_strdup_printf("Couldn't find the board menu, or plugin execution error");
	gc_dialog(tmpstr, NULL);
	g_free(tmpstr);
      } else {
	g_message("Fine, we got the gcomprisBoardMenu, xml boards parsing went fine");
	if(!display_activation_dialog())
	  gc_board_play(board_to_start);
      }

    }
  g_message("gcompris window is now mapped");
}

/*
 * Process the cleanup of the child (no zombies)
 * ---------------------------------------------
 */
void gc_terminate(int signum)
{

  g_message("GCompris got the %d signal, starting exit procedure", signum);

  gc_exit();

}

static void load_properties ()
{
  GError *error = NULL;
  properties = gc_prop_new ();

  /* Initialize the binary relocation API
   *  http://autopackage.org/docs/binreloc/
   */
  if(gbr_init (&error))
    g_message("Binary relocation enabled");
  else
    {
      if (error->code == GBR_INIT_ERROR_DISABLED)
	g_message("Binary relocation disabled");
      else
	g_message("Binary relocation start failed with error %d", error->code);
      g_error_free (error);
    }

  exec_prefix = gbr_find_exe_dir(NULL);
  g_warning("exec_prefix %s\n", (exec_prefix==NULL ? "NONE" : exec_prefix));

  {
    gchar *pkg_data_dir = gbr_find_data_dir(PACKAGE_DATA_DIR);
    gchar *pkg_clib_dir = gbr_find_lib_dir(PACKAGE_CLIB_DIR);

    properties->package_data_dir = g_strconcat(pkg_data_dir, "/gcompris/boards", NULL);
    properties->package_skin_dir = g_strconcat(pkg_data_dir, "/gcompris/boards/skins", NULL);
    properties->package_locale_dir = gbr_find_locale_dir(PACKAGE_LOCALE_DIR);
    properties->package_plugin_dir = g_strconcat(pkg_clib_dir, "/gcompris", NULL);
    properties->package_python_plugin_dir = g_strconcat(pkg_data_dir, "/gcompris/python",
							NULL);
    properties->system_icon_dir = g_strconcat(pkg_data_dir, "/pixmaps", NULL);
    properties->menu_dir = g_strdup(properties->package_data_dir);
    g_free(pkg_data_dir);
    g_free(pkg_clib_dir);
  }

  /* Display the directory value we have */
  printf("package_data_dir         = %s\n", properties->package_data_dir);
  printf("package_skin_dir         = %s\n", properties->package_skin_dir);
  printf("package_menu_dir         = %s\n", properties->menu_dir);
  printf("package_locale_dir       = %s\n", properties->package_locale_dir);
  printf("package_plugin_dir       = %s\n", properties->package_plugin_dir);
  printf("package_python_plugin_dir= %s\n", properties->package_python_plugin_dir);
  printf("user_dir                 = %s\n", properties->user_dir);
}

GcomprisProperties *gc_prop_get ()
{
  return (properties);
}

/* Return the database file name
 * Must be called after properties is initialised
 */
gchar *gc_db_get_filename ()
{
  g_assert(properties!=NULL);

  return (properties->database);
}

/*
 * This returns the locale for which text must be displayed
 *
 */
const gchar *gc_locale_get()
{
  const gchar *locale;

  /* First check locale got overrided by the user */
  if(gc_locale != NULL)
    return(gc_locale);

  locale = g_getenv("LC_ALL");
  if(locale == NULL)
    locale = g_getenv("LC_CTYPE");
  if(locale == NULL)
    locale = g_getenv("LANG");

  if(locale!=NULL)
    return(locale);

  return("en_US.UTF-8");
}

/*
 * This return the user default locale like it was at program
 * startup before we started changing the locale
 *
 */
char *gc_locale_get_user_default()
{
  return gc_user_default_locale;
}

/*
 * This set the locale for which text must be displayed
 * If locale is NULL, "" or "NULL" then locale is set to the user's default locale
 * if any or the user's system locale instead.
 *
 */
void
gc_locale_set(const gchar *locale)
{

  if(!locale || locale[0] == '\0' || (g_strcmp0(locale, "NULL") == 0) )
    {
      if ( properties->locale && properties->locale[0] != '\0')
	locale = properties->locale;
      else
	locale = gc_locale_get_user_default();
    }

  g_message("gc_locale_set '%s'\n", locale);
  if(gc_locale != NULL)
    g_free(gc_locale);

#if defined WIN32
  /* On windows, it always works */
  gc_locale = g_strdup(locale);
  setlocale(LC_MESSAGES, locale);
  setlocale(LC_ALL, locale);
#else
  gc_locale = g_strdup(setlocale(LC_CTYPE, locale));
  if (!gc_locale)
    gc_locale = g_strdup(locale);
#endif

  if(gc_locale!=NULL && strcmp(locale, gc_locale))
    g_message("Requested locale '%s' got '%s'", locale, gc_locale);

  if(gc_locale==NULL)
    g_message("Failed to set requested locale %s got %s", locale, gc_locale);

  /* Override the env locale to what the user requested */
  /* This makes gettext to give us the new locale text  */
  gc_setenv ("LC_ALL", gc_locale_get());
  gc_setenv ("LC_CTYPE", gc_locale_get());
  gc_setenv ("LANGUAGE", gc_locale_get());
  gc_setenv ("LANG", gc_locale_get());

  /* This does update gettext translation uppon next gettext call */
  /* Call for localization startup */
  bindtextdomain (GETTEXT_PACKAGE, properties->package_locale_dir);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

#ifndef WIN32
  /* Make change known.  */
  {
    extern int  _nl_msg_cat_cntr;
    ++_nl_msg_cat_cntr;
  }
#endif

}

void gc_log_handler (const gchar *log_domain,
			   GLogLevelFlags log_level,
			   const gchar *message,
			   gpointer user_data) {
  if(gc_debug)
    g_printerr ("%s: %s\n\n", "gcompris", message);
}

/* Refresh canvas zoom e.g. after setting zoom setting */
void
gc_update_canvas_zoom()
{
  _gc_size_allocate_event_callback(NULL, &alignment->allocation, NULL);
}

static void
start_bg_music (gchar *file)
{
  g_message ("start_bg_music %s", file);
  gc_sound_bg_reopen();
}

/* Single instance Check */
static void
single_instance_release()
{
  gchar *lock_file = g_strdup_printf("%s/%s", properties->config_dir, GC_LOCK_FILE);

  g_unlink(lock_file);
  g_free(lock_file);
}

static void
single_instance_check()
{
  gchar *lock_file = g_strdup_printf("%s/%s", properties->config_dir, GC_LOCK_FILE);
  if(!popt_nolockcheck)
    {
      GTimeVal current_time;
      g_get_current_time(&current_time);

      if (g_file_test (lock_file, G_FILE_TEST_EXISTS))
	{
	  char *result;
	  gsize length;
	  glong seconds;
	  /* Read it's content */
	  g_file_get_contents(lock_file,
			      &result,
			      &length,
			      NULL);
	  sscanf(result, "%ld", &seconds);

	  if(current_time.tv_sec - seconds < GC_LOCK_LIMIT)
	    {
	      printf(ngettext("GCompris won't start because the lock file is less than %d second old.\n",
			      "GCompris won't start because the lock file is less than %d seconds old.\n",
			      GC_LOCK_LIMIT),
		     GC_LOCK_LIMIT);
	      printf(_("The lock file is: %s\n"),
		     lock_file);
	      exit(0);
	    }
	}

      {
	/* Update the date in it (in seconds) */
	char date_str[64];
	sprintf(date_str, "%ld", current_time.tv_sec);
	g_file_set_contents(lock_file,
			    date_str,
			    strlen(date_str),
			    NULL);
      }
    }

  // Do not free lock_file, the unlink needs it
}

gboolean test_board_end(gpointer data);

gboolean test_board_start(gpointer data) {
  GList *full_list = (GList *) data;
  GcomprisBoard *board = (GcomprisBoard*)full_list->data;
  printf("Testing %s/%s : %s (%s) \n", board->section, board->name, board->title, board->description );
  gc_board_run_next(board);

  g_timeout_add(600, test_board_end, full_list);
  return FALSE;
}
gboolean test_board_end(gpointer data) {
   GList *full_list = (GList *) data;
   gc_close_all_dialog();

  gc_board_stop();
  if (full_list->next) {
    full_list = full_list->next;
    g_timeout_add(600, test_board_start, full_list);
  } else {
    printf("TEST DONE\n");
  }
  return FALSE;
}

void gc_test() {
  gc_db_init(FALSE /* ENABLE DATABASE */);
  gc_board_init();
  gc_menu_load();

  // Create the list of all the activity in full_list
  GList *full_list = NULL;

  GList *list = NULL;
  GList *menulist = NULL;
  GList *menu_todo = NULL;

  menu_todo = g_list_append(menu_todo,g_strdup("/"));

  while ( menu_todo != NULL) {
    menulist = gc_menu_getlist(menu_todo->data);
    g_free(menu_todo->data);
    menu_todo = menu_todo->next;

    for(list = menulist; list != NULL; list = list->next) {
      GcomprisBoard *board = list->data;

      if (board){
	if (strcmp(board->type,"menu")==0)
	  menu_todo = g_list_prepend(menu_todo, g_strdup_printf("%s/%s",board->section, board->name));
	else {
	  full_list = g_list_append(full_list, board);
	}
      }
    }
  }

  // Start the test in a few seconds to be sure everything is loaded
  g_timeout_add(5000, test_board_start, full_list);

}

/*****************************************
 * Main
 *
 */

int
main (int argc, char *argv[])
{
  GError *error = NULL;
  GOptionContext *context;

  /* First, Remove the gnome crash dialog because it locks the user when in full screen */
  signal(SIGSEGV, gc_terminate);
  signal(SIGINT, gc_terminate);

  load_properties();

  bindtextdomain (GETTEXT_PACKAGE, properties->package_locale_dir);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* To have some real random behaviour */
  g_random_set_seed     (time (NULL));

  /* Default difficulty filter: non specified */
  popt_difficulty_filter = -1;

  gtk_init (&argc, &argv);

  /* Argument parsing */
  context = g_option_context_new("GCompris");
  g_option_context_add_main_entries (context, options, GETTEXT_PACKAGE);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free(context);

  /* Set the default message handler, it avoids message with option -D */
  g_log_set_handler (NULL, G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_DEBUG | G_LOG_FLAG_FATAL
		     | G_LOG_FLAG_RECURSION, gc_log_handler, NULL);

  /*------------------------------------------------------------*/
  if (popt_debug)
    {
      gc_debug = TRUE;
    }

  if (popt_config_dir)
    {
      if ((!g_file_test(popt_config_dir, G_FILE_TEST_IS_DIR)) ||
	  (g_access(popt_config_dir, popt_administration? W_OK : R_OK ) == -1))
	{
	  printf("%s does not exists or is not %s\n", popt_config_dir,
		 popt_administration? "writable" : "readable"	);
	  exit(0);
	}
      else
	{
	  g_message("Using %s as config directory.", popt_config_dir);
	  g_free(properties->config_dir);
	  properties->config_dir = g_strdup(popt_config_dir);
	}
      printf("package_config_dir       = %s\n", properties->config_dir);
    }

  /* Now we know where our config file is, load the saved config */
  gc_prop_old_config_migration(properties);
  gc_prop_load(properties, GC_PROP_FROM_USER_CONF);

  /* We overwrite the user config with the administrator system config
     It let the sysadmin to specify a certain set of value that are restaured
     whatever the user saved
  */
  gc_prop_load(properties, GC_PROP_FROM_SYSTEM_CONF);

  /* Set the locale */
#if defined WIN32
  gc_user_default_locale = g_win32_getlocale();
  // Set the user's choice locale
  if(properties->locale[0]=='\0') {
    gc_locale_set(gc_user_default_locale);
  } else {
    gc_locale_set(properties->locale);
  }
#else
  gc_user_default_locale = g_strdup(setlocale(LC_CTYPE, NULL));
  // Set the user's choice locale
  gc_locale_set(properties->locale);
#endif

  if (popt_version)
    {
      printf (_("GCompris\nVersion: %s\nLicence: GPL\n"
		"More info at http://gcompris.net\n"),
	      VERSION);
      exit (0);
    }

  if (popt_fullscreen)
    {
      properties->fullscreen = TRUE;
    }

  if (popt_window)
    {
      properties->fullscreen = FALSE;
    }

  if (popt_mute)
    {
      g_message("Sound disabled");
      properties->music = FALSE;
      properties->fx = FALSE;
    }

  if (popt_sound)
    {
      g_message("Sound enabled");
      properties->music = TRUE;
      properties->fx = TRUE;
    }

  if (popt_cursor)
    {
      g_message("Default system cursor enabled");
      properties->defaultcursor = GDK_LEFT_PTR;
    }

  if (popt_nocursor)
    {
      g_message("No cursors");
      properties->nocursor = TRUE;
    }

  if (popt_experimental)
    {
      g_message("Experimental boards allowed");
      properties->experimental  = TRUE;
    }

  if (popt_no_quit)
    {
      g_message("Disable quit button");
      properties->disable_quit = TRUE;
    }

  if (popt_no_config)
    {
      g_message("Disable config button");
      properties->disable_config = TRUE;
    }

  if (popt_no_level)
    {
      g_message("Disable level button");
      properties->disable_level = TRUE;
    }

  if (popt_difficulty_filter>=0)
    {
      /* This option provide less capacity than the GUI since we cannot set the filter_style */
      g_message("Display only activities of level %d", popt_difficulty_filter);
      properties->difficulty_filter = popt_difficulty_filter;
      properties->filter_style      = GCOMPRIS_FILTER_EQUAL;
    }

  if (popt_package_data_dir) {
    printf("Overloaded package_data_dir          = %s\n", popt_package_data_dir);
    g_free(properties->package_data_dir);
    properties->package_data_dir = g_strdup(popt_package_data_dir);
  }

  if (popt_package_skin_dir) {
    printf("Overloaded package_skin_dir          = %s\n", popt_package_skin_dir);
    g_free(properties->package_skin_dir);
    properties->package_skin_dir = g_strdup(popt_package_skin_dir);
  }

  if (popt_menu_dir) {
    printf("Overloaded menu_dir                  = %s\n", popt_menu_dir);
    g_free(properties->menu_dir);
    properties->menu_dir = g_strdup(popt_menu_dir);
  }

  if (popt_plugin_dir) {
    printf("Overloaded package_plugin_dir        = %s\n", popt_plugin_dir);
    g_free(properties->package_plugin_dir);
    properties->package_plugin_dir = g_strdup(popt_plugin_dir);
  }

  if (popt_python_plugin_dir) {
    printf("Overloaded package_python_plugin_dir = %s\n", popt_python_plugin_dir);
    g_free(properties->package_python_plugin_dir);
    properties->package_python_plugin_dir = g_strdup(popt_python_plugin_dir);
  }

  if (popt_locale_dir) {
    printf("Overloaded locale_dir                = %s\n", popt_locale_dir);
    g_free(properties->package_locale_dir);
    properties->package_locale_dir = g_strdup(popt_locale_dir);

    bindtextdomain (GETTEXT_PACKAGE, properties->package_locale_dir);
  }

  if (popt_root_menu){
    if (strcmp(popt_root_menu,"list")==0){
      /* check the list of possible values for -l, then exit */
      printf(_("Use -l to access an activity directly.\n"));
      printf(_("The list of available activities is :\n"));

      gc_db_init(FALSE /* ENABLE DATABASE */);
      gc_board_init();
      gc_menu_load();

      GList *list = NULL;
      GList *menulist = NULL;
      GList *menu_todo = NULL;
      int board_count = 0;

      menu_todo = g_list_append(menu_todo,g_strdup("/"));

      while ( menu_todo != NULL) {
	menulist = gc_menu_getlist(menu_todo->data);
	g_free(menu_todo->data);
	menu_todo = menu_todo->next;

	for(list = menulist; list != NULL; list = list->next) {
	  GcomprisBoard *board = list->data;

	  if (board){
	    if (strcmp(board->type,"menu")==0)
	      menu_todo = g_list_prepend(menu_todo, g_strdup_printf("%s/%s",board->section, board->name));
	    else
	      board_count++;

	    printf("%s/%s : %s (%s) \n", board->section, board->name, board->title, board->description );
	  }
	}
      }
      printf(_("Number of activities: %d\n"), board_count);

      exit(0);
    }
    else {
      g_message("Using menu %s as root.", popt_root_menu);
      g_free(properties->root_menu);
      properties->root_menu = g_strdup(popt_root_menu);
    }
  }

  if (popt_user_dir)
    {
      if ((!g_file_test(popt_user_dir, G_FILE_TEST_IS_DIR)) ||
	  (g_access(popt_user_dir, popt_administration? R_OK : W_OK ) == -1))
	{
	  g_message("%s does not exists or is not %s ", popt_user_dir,
		    popt_administration? "readable" : "writable");
	  exit(0);
	}
      else
	{
	  g_message("Using %s as user directory.", popt_user_dir);
	  g_free(properties->user_dir);
	  properties->user_dir = g_strdup(popt_user_dir);
	}
    }

  if (popt_database)
    {
      properties->database = g_strdup(popt_database);

      if (g_file_test(properties->database, G_FILE_TEST_EXISTS))
	{
	  if (g_access(properties->database, R_OK)==-1)
	    {
	      printf(_("%s exists but is not readable or writable"), properties->database);
	      exit(0);
	    }
	}
    }
  else
    {
      /* Init the default database location */
      properties->database = g_strconcat(properties->config_dir,
					 "/", DEFAULT_DATABASE, NULL);
    }

  /* config_dir initialised, now we can set the default */
  printf("Infos:\n   Config dir '%s'\n   Users dir '%s'\n",
	 properties->config_dir,
	 properties->user_dir);
#ifdef USE_SQLITE
  printf("   Database '%s'\n", properties->database);
#endif

  if (popt_create_db)
    {
      gchar *dirname = g_path_get_dirname (properties->database);
      if (g_access(dirname, W_OK)==-1)
	{
	  g_message("Cannot create %s : %s is not writable !", properties->database, dirname);
	  exit (0);
	}
      /* We really want to recreate it, erase the old one */
      g_message("Removing %s database.", properties->database);
      unlink(properties->database);
    }

  if (popt_administration){
    if (popt_database){
      if (g_access(popt_database,R_OK|W_OK)==-1){
	g_message("%s exists but is not writable", popt_database);
	exit(0);
      }
    }
    g_message("Running in administration mode");
    properties->administration = TRUE;
    g_message("Music disabled");
    properties->music = FALSE;
    properties->fx = FALSE;
    g_message("Fullscreen and cursor is disabled");
    properties->fullscreen = FALSE;
    properties->defaultcursor = GDK_LEFT_PTR;
  }

  if (popt_reread_menu){
    g_message("Rebuild db from xml files");
    if (g_access(properties->database, W_OK)==-1)
      g_message("Cannot reread menu when database is read-only !");
    else
      properties->reread_menu = TRUE;
  }

  if (popt_server){
#ifdef USE_GNET
      properties->server = g_strdup(popt_server);
      printf("   Server '%s'\n", properties->server);
#else
      printf(_("The --server option cannot be used because"
	       " GCompris has been compiled without network support!"));
      exit(1);
#endif
  }

  if(popt_web_only) {
    g_free(properties->package_data_dir);
    properties->package_data_dir = g_strdup("");

    g_free(properties->system_icon_dir);
    properties->system_icon_dir = g_strdup("");
  }

  if (popt_server){
    if(popt_cache_dir)
      properties->cache_dir = g_strdup(popt_cache_dir);
    else
      properties->cache_dir = g_build_filename(g_get_user_cache_dir(), "gcompris", NULL);
    printf("   Cache dir '%s'\n",properties->cache_dir);
  }

  if (popt_drag_mode){
    if (strcmp(popt_drag_mode, "default") == 0)
      properties->drag_mode = GC_DRAG_MODE_GRAB;
    else {
      if (strcmp(popt_drag_mode, "2clicks") == 0)
	properties->drag_mode = GC_DRAG_MODE_2CLICKS;
      else {
	if (strcmp(popt_drag_mode, "both") == 0)
	  properties->drag_mode = GC_DRAG_MODE_BOTH;
	else g_message("Unknown drag mode ! Valids modes are \"normal\", \"2clicks\" and \"both\"");
      }
    }
  }

  gc_prop_activate(properties);

  /*
   * Database init MUST BE after properties
   * And after a possible alternate database as been provided
   *
   */
  if (popt_root_menu)
      gc_db_init(TRUE /* DISABLE DATABASE */);
    else
      gc_db_init(FALSE /* ENABLE DATABASE */);

  /* An alternate profile is requested, check it does exists */
  if (popt_profile){
    properties->profile = gc_db_profile_from_name_get(popt_profile);

    if(properties->profile == NULL)
      {
	printf(_("ERROR: Profile '%s' is not found."
		 " Run 'gcompris --profile-list' to list available ones\n"),
	       popt_profile);
	exit(1);
      }
  }

  /* List all available profiles */
  if (popt_profile_list){
    GList * profile_list;
    int i;

    profile_list = gc_db_profiles_list_get();

    printf(_("The list of available profiles is:\n"));
    for(i=0; i< g_list_length(profile_list); i++)
      {
	GcomprisProfile *profile = g_list_nth_data(profile_list, i);
	printf("   %s\n", profile->name);
      }

    g_list_free(profile_list);
    exit(0);
  }

  if (popt_sugar_look){
    #ifdef USE_SUGAR
      extern Bar sugar_bar;
      gc_bar_register(&sugar_bar);
      extern Score sugar_score;
      gc_score_register(&sugar_score);
    #else
	  printf("GCompris was not built with Sugar DE support.\n");
      popt_sugar_look = FALSE;
    #endif
  }

  if (popt_no_zoom){
    g_message("Zoom disabled");
    properties->zoom = FALSE;
  }

  /*------------------------------------------------------------*/

  single_instance_check();

  /* networking init */
  gc_net_init();
  gc_cache_init();

  gc_sound_build_music_list();

  if(properties->music || properties->fx)
    gc_sound_init();

  /* Gdk-Pixbuf */
  gdk_rgb_init();

  rsvg_init();

  setup_window ();

  gtk_widget_show_all (window);


  if(properties->music || properties->fx)
    mute = FALSE;
  else
    mute = TRUE;

  /* If a specific activity is selected, skeep the intro music */
  if(!popt_root_menu)
    {
      if (properties->music)
	{
	  gc_sound_play_ogg("music/intro.ogg",
			    NULL);
	  gc_sound_play_ogg_cb("voices/$LOCALE/misc/welcome.ogg",
			       start_bg_music);
	}
      else
	gc_sound_play_ogg("voices/$LOCALE/misc/welcome.ogg", NULL);
    }

  if (popt_test) {
    gc_test();
  }

  gtk_main ();

  return(0);
}

gint
gc_timing (gint timeout, gint actors_number)
{
  if (popt_timing_base > 1.0)
    timeout = (int) (timeout * popt_timing_base);
  if (popt_timing_mult < 1.0 && actors_number > 1)
    timeout += (int) (timeout * actors_number * popt_timing_mult);
  return timeout;
}
