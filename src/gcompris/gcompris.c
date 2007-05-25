/* gcompris - gcompris.c
 *
 * Copyright (C) 2000-2003 Bruno Coudoin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <glib/gstdio.h>

#include "gcompris.h"
#include "gc_core.h"
#include "gcompris_config.h"
#include "about.h"
#include <locale.h>

#include "binreloc.h"

/* For XF86_VIDMODE Support */
#ifdef XF86_VIDMODE
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>
#endif

/* for NSBUNDLE */
#ifdef NSBUNDLE
#include "gcompris-nsbundle.h"
#endif

#include <ccc/ccc.h>

/* get the default database name */
#define DEFAULT_DATABASE "gcompris_sqlite.db"

/* Multiple instance check */
static gchar *lock_file;
#define GC_LOCK_FILE "gcompris.lock"
#define GC_LOCK_LIMIT 30 /* seconds */

static GtkWidget *window;
static GnomeCanvas *canvas;
static GnomeCanvas *canvas_bar;
static GnomeCanvas *canvas_bg;

gchar * exec_prefix = NULL;

//static gint pause_board_cb (GtkWidget *widget, gpointer data);
static void quit_cb (GtkWidget *widget, gpointer data);
static void map_cb  (GtkWidget *widget, gpointer data);
static gint board_widget_key_press_callback (GtkWidget   *widget,
					    GdkEventKey *event,
					    gpointer     client_data);
void gc_terminate(int signum);

/*
 * For the Activation dialog
 */
#ifdef STATIC_MODULE
int gc_activation_check(char *code);
static void activation_enter_callback(GtkWidget *widget,
				      GtkWidget *entry );
static void activation_done();
static void display_activation_dialog();
static GnomeCanvasItem *activation_item;
static GtkEntry *widget_activation_entry;
#else
#define display_activation_dialog()
#endif


static GcomprisProperties *properties = NULL;
static gboolean		   antialiased = FALSE;
static gboolean		   is_mapped = FALSE;

/****************************************************************************/
/* Some constants.  */

static GnomeCanvasItem *backgroundimg = NULL;
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
static gint popt_version	   = FALSE;
static gint popt_aalias		   = FALSE;
static gint popt_difficulty_filter = FALSE;
static gint popt_debug		   = FALSE;
static gint popt_noxf86vm	   = FALSE;
static gint popt_nobackimg	   = FALSE;
static gint popt_nolockcheck	   = FALSE;
static gchar *popt_root_menu       = NULL;
static gchar *popt_local_activity  = NULL;
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
static gint  popt_display_resource = FALSE;
static gchar *popt_server          = NULL;
static gint  *popt_web_only        = NULL;
static gchar *popt_cache_dir       = NULL;
static gchar *popt_drag_mode       = NULL;

static GOptionEntry options[] = {
  {"fullscreen", 'f', 0, G_OPTION_ARG_NONE, &popt_fullscreen,
   N_("run gcompris in fullscreen mode."), NULL},

  {"window", 'w', 0, G_OPTION_ARG_NONE, &popt_window,
   N_("run gcompris in window mode."), NULL},

  {"sound", 's', 0, G_OPTION_ARG_NONE, &popt_sound,
   N_("run gcompris with sound enabled."), NULL},

  {"mute", 'm', 0, G_OPTION_ARG_NONE, &popt_mute,
   N_("run gcompris without sound."), NULL},

  {"cursor", 'c', 0, G_OPTION_ARG_NONE, &popt_cursor,
   N_("run gcompris with the default gnome cursor."), NULL},

  {"difficulty", 'd', 0, G_OPTION_ARG_INT, &popt_difficulty_filter,
   N_("display only activities with this difficulty level."), NULL},

  {"debug", 'D', 0, G_OPTION_ARG_NONE, &popt_debug,
   N_("display debug informations on the console."), NULL},

  {"version", 'v', 0, G_OPTION_ARG_NONE, &popt_version,
   N_("Print the version of " PACKAGE), NULL},

  {"antialiased", '\0', 0, G_OPTION_ARG_NONE, &popt_aalias,
   N_("Use the antialiased canvas (slower)."), NULL},

  {"noxf86vm", 'x', 0, G_OPTION_ARG_NONE, &popt_noxf86vm,
   N_("Disable XF86VidMode (No screen resolution change)."), NULL},

  {"root-menu", 'l', 0, G_OPTION_ARG_STRING, &popt_root_menu,
   N_("Run gcompris with local menu (e.g -l /reading will let you play only activities in the reading directory, -l /strategy/connect4 only the connect4 activity)"), NULL},

  {"local-activity", 'L', 0, G_OPTION_ARG_STRING, &popt_local_activity,
   N_("Run GCompris with local activity directory added to menu"), NULL},

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

#if 0 /* Never completed, this option means we should document resources
	    of each activities which is boring to do */
  {"display-resource",'\0', 0, G_OPTION_ARG_NONE, &popt_display_resource,
   N_("Display the resources on stdout based on the selected activities"), NULL},
#endif

  {"server", '\0', 0, G_OPTION_ARG_STRING, &popt_server,
   N_("GCompris will get images, sounds and activity data from this server if not found locally."), NULL},

  {"web-only", '\0', 0, G_OPTION_ARG_NONE, &popt_web_only,
   N_("Only when --server is provided, disable check for local resource first."
      " Data are always taken from the web server."), NULL},

  {"cache-dir", '\0', 0, G_OPTION_ARG_STRING, &popt_cache_dir,
   N_("In server mode, specify the cache directory used to avoid useless downloads."), NULL},

  {"drag-mode", 'g', 0, G_OPTION_ARG_STRING, &popt_drag_mode,
   N_("Global drag and drop mode: normal, 2clicks, both. Default mode is normal."), NULL},

  {"nobackimg", '\0', 0, G_OPTION_ARG_NONE, &popt_nobackimg,
   N_("Do not display the background images of activities."), NULL},

  {"nolockcheck", '\0', 0, G_OPTION_ARG_NONE, &popt_nolockcheck,
   N_("Do not avoid the execution of multiple instances of GCompris."), NULL},

  { NULL }
};

/* Fullscreen Stuff */
#ifdef XF86_VIDMODE
static struct
{
  XF86VidModeModeInfo fs_mode;
  XF86VidModeModeInfo orig_mode;
  int orig_viewport_x;
  int orig_viewport_y;
  int window_x;
  int window_y;
  gboolean fullscreen_active;
} XF86VidModeData = { { 0 }, { 0 }, 0, 0, 0, 0, FALSE };

static void xf86_vidmode_init( void );
static void xf86_vidmode_set_fullscreen( int state );
static gint xf86_window_configured(GtkWindow *window,
  GdkEventConfigure *event, gpointer param);
static gint xf86_focus_changed(GtkWindow *window,
  GdkEventFocus *event, gpointer param);
#endif

/****************************************************************************/
#define N_POINTS  12
#define RADIUS 100.0
#define STEP 0.02
#define X(radius, angle)  ((radius)*RADIUS*cos(M_PI*((double) angle)/N_POINTS))
#define Y(radius, angle)  ((radius)*RADIUS*sin(M_PI*((double) angle)/N_POINTS))

static void
update_bounds(CcItem* moving_line, CcView* view, CcDRect* bounds, CcRectangle* rect) {
	if(!bounds) {
		return;
	}

	cc_rectangle_set_position(rect,
				  bounds->x1, bounds->y1,
				  bounds->x2 - bounds->x1,
				  bounds->y2 - bounds->y1);
}

static gboolean
enter_callback(CcShape* shape, CcView* view, GdkEventCrossing* ev) {
	CcColor* color = cc_color_new_rgb(1.0, 0.0, 0.0);
	cc_shape_set_brush_border(shape, CC_BRUSH(color));
	return TRUE;
}

static gboolean
leave_callback(CcShape* shape, CcView* view, GdkEventCrossing* ev) {
	CcColor* color = cc_color_new_rgb(0.0, 0.0, 1.0);
	cc_shape_set_brush_border(shape, CC_BRUSH(color));
	return FALSE;
}

static int ccindex = 1;
static CcItem *rounded_line = NULL;

static gboolean
timer_callback(gpointer data)
{
  int i;
  float  angle = ccindex*STEP;

  for (i=0; i <  N_POINTS; i++)
    cc_line_data_set_position(CC_LINE(rounded_line), 2*i+1, X(1.5 + cos(M_PI*((double) angle)/N_POINTS),  2*i + 1 + angle),  Y(1.5 + cos(M_PI*((double) angle)/N_POINTS),  2*i + 1 + angle));

  ccindex = (ccindex+1) % ((int) (2*N_POINTS*1/STEP));

  return TRUE;
}

GtkWidget*
test_ccc()
{
  GtkWidget* canvas;
  CcItem* item   = cc_item_new();
  rounded_line   = cc_line_new();
  CcItem* rect   = cc_rectangle_new();

  g_signal_connect(G_OBJECT(rounded_line), "all-bounds-changed",
		   G_CALLBACK(update_bounds), rect);

  printf("TEST CCC\n");
  cc_line_move(CC_LINE(rounded_line), X(1, 0), Y(1, 0));

  int i;
  for (i=0; i <  N_POINTS; i++) {
    cc_line_line(CC_LINE(rounded_line), X(0.5, 2*i + 1), Y(0.5, 2*i + 1));
    cc_line_line(CC_LINE(rounded_line), X(1, 2*i+2), Y(1, 2*i+2));
  }

  cc_shape_set_brush_border(CC_SHAPE(rect), CC_BRUSH(cc_color_new_rgb(1.0, 0.0, 0.0)));
  cc_shape_set_brush_border(CC_SHAPE(rounded_line), CC_BRUSH(cc_color_new_rgb(0.0, 0.0, 1.0)));
  cc_item_append(item, rect);
  cc_item_append(item, rounded_line);

  g_signal_connect(rounded_line, "enter-notify-event",
		   G_CALLBACK(enter_callback), NULL);
  g_signal_connect(rounded_line, "leave-notify-event",
		   G_CALLBACK(leave_callback), NULL);

  g_timeout_add  (10, (GSourceFunc) timer_callback, NULL);

  canvas = cc_view_widget_new_root(item);

  return canvas;
}

/* Remove any dialog box */
static void gc_close_all_dialog() {
  gc_dialog_close();
  gc_help_stop();
  gc_config_stop();
  gc_about_stop();
  gc_selector_file_stop();
  gc_selector_images_stop();
}

static gint
board_widget_key_press_callback (GtkWidget   *widget,
				 GdkEventKey *event,
				 gpointer     client_data)
{
  int kv = event->keyval;

  if(event->state & GDK_CONTROL_MASK && ((event->keyval == GDK_r)
					 || (event->keyval == GDK_R))) {
    g_message("Refreshing the canvas\n");
    gnome_canvas_update_now(canvas);
    return TRUE;
  }

  if(event->state & GDK_CONTROL_MASK && ((event->keyval == GDK_q)
					 || (event->keyval == GDK_Q))) {
    gc_exit();
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
      gnome_canvas_update_now(canvas);
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
	  g_warning("%d key is handled by context", kv);
	  return TRUE;
	}
    }

  g_warning("%d key is NOT handled by context", kv);
  /* If the board needs to receive key pressed */
  /* NOTE: If a board receives key press, it must bind the ENTER Keys to OK
   *       whenever possible
   */
  if (gc_board_get_current_board_plugin()!=NULL && gc_board_get_current_board_plugin()->key_press)
    {
      return(gc_board_get_current_board_plugin()->key_press (event->keyval, NULL, NULL));
    }
  else if (gc_board_get_current_board_plugin()!=NULL && gc_board_get_current_board_plugin()->ok &&
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

/**
 * Return the main canvas we run in
 */
GnomeCanvas *gc_get_canvas()
{
  return canvas;
}

GtkWidget *gc_get_window()
{
  return window;
}

GnomeCanvasItem *gc_set_background(GnomeCanvasGroup *parent, gchar *file)
{
  GdkPixbuf *background_pixmap = NULL;
  gchar *img = NULL;

  if ( popt_nobackimg && (strncmp(file, "opt/", 4) == 0) )
    {
      img = gc_skin_image_get("gcompris-bg.jpg");
      background_pixmap = gc_pixmap_load (img);
      g_free(img);
    }
  else
    background_pixmap = gc_pixmap_load (file);

  if(backgroundimg)
      gnome_canvas_item_set (backgroundimg,
			     "pixbuf", background_pixmap,
			     NULL);
  else
    backgroundimg=gnome_canvas_item_new (parent,
					 gnome_canvas_pixbuf_get_type (),
					 "pixbuf", background_pixmap,
					 "x", 0.0,
					 "y", 0.0,
					 "width", (double) BOARDWIDTH,
					 "height", (double) BOARDHEIGHT,
					 NULL);
  gnome_canvas_item_lower_to_bottom(backgroundimg);

  gdk_pixbuf_unref(background_pixmap);

  GtkWidget* ccwidget = test_ccc();
  gnome_canvas_item_new (parent,
			 gnome_canvas_widget_get_type (),
			 "widget", GTK_WIDGET(ccwidget),
			 "x", (double) 160.0,
			 "y", (double) 10.0,
			 "width", 500.0,
			 "height", 500.0,
			 "anchor", GTK_ANCHOR_NW,
			 "size_pixels", FALSE,
			 NULL);
  gtk_widget_show(GTK_WIDGET(ccwidget));

  return (backgroundimg);
}

static void init_background()
{
  double xratio, yratio, max;
  gint screen_height, screen_width;
  GtkWidget *vbox;

#ifdef XF86_VIDMODE
  xf86_vidmode_init();

  if(properties->fullscreen && !properties->noxf86vm) {
    screen_height = XF86VidModeData.fs_mode.vdisplay;
    screen_width  = XF86VidModeData.fs_mode.hdisplay;
  }
  else
#endif
  {
    screen_height = gdk_screen_height();
    screen_width  = gdk_screen_width();
  }

  yratio=screen_height/(float)(BOARDHEIGHT+BARHEIGHT);
  xratio=screen_width/(float)BOARDWIDTH;
  g_message("The screen_width=%f screen_height=%f\n",
	    (double)screen_width, (double)screen_height);
  g_message("The xratio=%f yratio=%f\n", xratio, yratio);

  yratio=xratio=MIN(xratio, yratio);

  /* Depending on user preference, set the max ratio */
  switch(properties->screensize)
    {
    case 0: max = 0.8;
      break;
    case 1: max = 1;
      break;
    case 2: max = 1.28;
      break;
    default: max = 1;
      break;
    }
  xratio=MIN(max, xratio);

  g_message("Calculated x ratio xratio=%f\n", xratio);


  /* Background area if ratio above 1 */
  if(properties->fullscreen)
    {

      /* WARNING : I add 30 here for windows. don't know why it's needed. Doesn't hurt the Linux version */
      gnome_canvas_set_scroll_region (canvas_bg,
				      0, 0,
				      screen_width,
				      screen_height + 30);

      gtk_widget_set_usize (GTK_WIDGET(canvas_bg), screen_width, screen_height);

      /* Create a black box for the background */
      gnome_canvas_item_new (gnome_canvas_root(canvas_bg),
			     gnome_canvas_rect_get_type (),
			     "x1", (double) 0,
			     "y1", (double) 0,
			     "x2", (double) screen_width,
			     "y2", (double) screen_height + 30,
			     "fill_color", "black",
			     "outline_color", "black",
			     "width_units", (double)0,
			     NULL);

    }

  /* Create a vertical box in which I put first the play board area, then the button bar */
  vbox = gtk_vbox_new (FALSE, 0);
  if(!properties->fullscreen)
    gtk_container_add (GTK_CONTAINER(window), GTK_WIDGET(vbox));

  gtk_widget_show (GTK_WIDGET(vbox));
  gtk_widget_show (GTK_WIDGET(canvas));
  gtk_widget_show (GTK_WIDGET(canvas_bar));

  gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET(canvas), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET(canvas_bar), TRUE, TRUE, 0);

  if(properties->fullscreen)
    {
      gnome_canvas_item_new (gnome_canvas_root(canvas_bg),
			     gnome_canvas_widget_get_type (),
			     "widget", vbox,
			     "x", (double) (screen_width-
					    BOARDWIDTH*xratio)/2,
			     "y", (double) (screen_height-
					    BOARDHEIGHT*xratio-BARHEIGHT*xratio)/2,
			     "width",  (double)BOARDWIDTH*xratio,
			     "height", (double)BOARDHEIGHT*xratio+BARHEIGHT*xratio,
			     "size_pixels", TRUE,
			     NULL);
    }

  /* Create the drawing area */
  gnome_canvas_set_pixels_per_unit (canvas, xratio);

  gnome_canvas_set_scroll_region (canvas,
				  0, 0,
				  BOARDWIDTH,
				  BOARDHEIGHT);

  gtk_widget_set_usize (GTK_WIDGET(canvas), BOARDWIDTH*xratio, BOARDHEIGHT*xratio);

  /* Create the spot for the bar */
  gnome_canvas_set_pixels_per_unit (canvas_bar, xratio);
  gnome_canvas_set_scroll_region (canvas_bar,
				  0, 0,
				  BOARDWIDTH,
				  BARHEIGHT);
  gtk_widget_set_usize (GTK_WIDGET(canvas_bar),  BOARDWIDTH*xratio,  BARHEIGHT*xratio);

}

void gc_cursor_set(guint gdk_cursor_type)
{
  GdkCursor *cursor = NULL;

  // Little hack to force gcompris to use the default cursor
  if(gdk_cursor_type==GCOMPRIS_DEFAULT_CURSOR)
    gdk_cursor_type=properties->defaultcursor;

  // I suppose there is less than GCOMPRIS_FIRST_CUSTOM_CURSOR cursors defined in gdkcursors.h !
  if (gdk_cursor_type < GCOMPRIS_FIRST_CUSTOM_CURSOR) {
    cursor = gdk_cursor_new(gdk_cursor_type);
    gdk_window_set_cursor (window->window, cursor);
    gdk_cursor_destroy(cursor);
  } else { // we use a custom cursor
    GdkPixbuf *cursor_pixbuf = NULL;

    switch (gdk_cursor_type) {
    case GCOMPRIS_DEFAULT_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load("cursor_default.png");
      break;
    case GCOMPRIS_LINE_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load("cursor_line.png");
      break;
    case GCOMPRIS_RECT_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load("cursor_rect.png");
      break;
    case GCOMPRIS_FILLRECT_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load("cursor_fillrect.png");
      break;
    case GCOMPRIS_CIRCLE_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load("cursor_circle.png");
      break;
    case GCOMPRIS_FILLCIRCLE_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load("cursor_fillcircle.png");
      break;
    case GCOMPRIS_FILL_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load("cursor_fill.png");
      break;
    case GCOMPRIS_DEL_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load("cursor_del.png");
      break;
    case GCOMPRIS_SELECT_CURSOR :
      cursor_pixbuf = gc_skin_pixmap_load("cursor_select.png");
      break;
    default :
      return;
      break;
    }

    if(cursor_pixbuf)
      {
	cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), cursor_pixbuf, 0, 0);
	gdk_window_set_cursor(window->window, cursor);
	gdk_cursor_unref(cursor);
	gdk_pixbuf_unref(cursor_pixbuf);
      }
  }
}

static void setup_window ()
{
  GcomprisBoard *board_to_start;
  GdkPixbuf     *icon_pixbuf;
  gchar         *icon_file;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  /*
   * Set an icon for gcompris
   * ------------------------
   */
  icon_file = g_strconcat(properties->system_icon_dir, "/gcompris.png", NULL);
  if (!g_file_test (icon_file, G_FILE_TEST_EXISTS)) {
      /* Now check if this file is on the net */
      icon_file = gc_net_get_url_from_file("gcompris.png", NULL);
  }

  if(!icon_file)
      g_warning ("Couldn't find file %s !", icon_file);

  icon_pixbuf = gc_net_load_pixmap(icon_file);
  if (!icon_pixbuf)
    {
      g_warning ("Failed to load pixbuf file: %s\n",
               icon_file);
    }
  g_free(icon_file);

  if (icon_pixbuf)
    {
      gtk_window_set_icon (GTK_WINDOW (window), icon_pixbuf);
      gdk_pixbuf_unref (icon_pixbuf);
    }
  gtk_window_set_title(GTK_WINDOW (window), "GCompris");

  /*
   * Set the main window
   * -------------------
   */

  gtk_window_set_policy (GTK_WINDOW (window), FALSE, FALSE, TRUE);
  gtk_window_set_default_size(GTK_WINDOW(window), 250, 350);
  gtk_window_set_wmclass(GTK_WINDOW(window), "gcompris", "GCompris");

  gtk_widget_realize (window);

  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		      GTK_SIGNAL_FUNC (quit_cb), NULL);

  gtk_signal_connect (GTK_OBJECT (window), "map_event",
		      GTK_SIGNAL_FUNC (map_cb), NULL);

#ifdef XF86_VIDMODE
  /* The Xf86VidMode code needs to accuratly now the window position,
     this is the only way to get it, and it needs to track the focus to
     enable/disable fullscreen on alt-tab */
  gtk_widget_add_events(GTK_WIDGET(window),
    GDK_STRUCTURE_MASK|GDK_FOCUS_CHANGE_MASK);
  gtk_signal_connect (GTK_OBJECT (window), "configure_event",
    GTK_SIGNAL_FUNC (xf86_window_configured), 0);
  gtk_signal_connect (GTK_OBJECT (window), "focus_in_event",
    GTK_SIGNAL_FUNC (xf86_focus_changed), 0);
  gtk_signal_connect (GTK_OBJECT (window), "focus_out_event",
    GTK_SIGNAL_FUNC (xf86_focus_changed), 0);
#endif

  /* For non anti alias canvas */
  gtk_widget_push_visual (gdk_rgb_get_visual ());
  gtk_widget_push_colormap (gdk_rgb_get_cmap ());

  // Set the cursor
  gc_cursor_set(GCOMPRIS_DEFAULT_CURSOR);

  /* For anti alias canvas */
  /*
  gtk_widget_push_visual(gdk_rgb_get_visual());
  gtk_widget_push_colormap(gdk_rgb_get_cmap());
  */

  if(antialiased)
    {
      /* For anti alias canvas */
	canvas     = GNOME_CANVAS(gnome_canvas_new_aa ());
	canvas_bar = GNOME_CANVAS(gnome_canvas_new_aa ());
	canvas_bg = GNOME_CANVAS(gnome_canvas_new_aa ());
    }
  else
    {
      /* For non anti alias canvas */
      canvas     = GNOME_CANVAS(gnome_canvas_new ());
      canvas_bar = GNOME_CANVAS(gnome_canvas_new ());
      canvas_bg  = GNOME_CANVAS(gnome_canvas_new ());
    }

  gtk_signal_connect_after (GTK_OBJECT (window), "key_press_event",
			    GTK_SIGNAL_FUNC (board_widget_key_press_callback), 0);
  gtk_signal_connect_after (GTK_OBJECT (canvas), "key_press_event",
			    GTK_SIGNAL_FUNC (board_widget_key_press_callback), 0);
  gtk_signal_connect_after (GTK_OBJECT (canvas_bar), "key_press_event",
			    GTK_SIGNAL_FUNC (board_widget_key_press_callback), 0);
  gtk_signal_connect_after (GTK_OBJECT (canvas_bg), "key_press_event",
			    GTK_SIGNAL_FUNC (board_widget_key_press_callback), 0);

  gc_im_init(window);


  if(properties->fullscreen)
    gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET(canvas_bg));

  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();


  gtk_widget_show (GTK_WIDGET(canvas_bg));

  gc_board_init();


  /* Load all the menu once */
  gc_menu_load();

  /* Load the mime type */
  gc_mime_type_load();

  /* Save the root_menu */
  properties->menu_board = gc_menu_section_get(properties->root_menu);

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

    /* Run the bar */
  gc_bar_start(canvas_bar);

  init_background();

  if(!board_to_start) {
    gchar *tmpstr= g_strdup_printf("Couldn't find the board menu %s, or plugin execution error", properties->root_menu);
    gc_dialog(tmpstr, NULL);
    g_free(tmpstr);
  } else if(!gc_board_check_file(board_to_start)) {
    gchar *tmpstr= g_strdup_printf("Couldn't find the board menu, or plugin execution error");
    gc_dialog(tmpstr, NULL);
    g_free(tmpstr);
  } else {
    g_warning("Fine, we got the gcomprisBoardMenu, xml boards parsing went fine");
    gc_board_play(board_to_start);
  }

  display_activation_dialog();

}

#ifdef STATIC_MODULE
extern int gc_board_number_in_demo;
/** Display the activation dialog for the windows version
 *
 */
void
display_activation_dialog()
{
  int board_count = 0;
  GList *list;
  guint  key_is_valid = 0;

  key_is_valid = gc_activation_check(properties->key);

  if(key_is_valid == 1)
    return;

  /* Count non menu boards */
  for (list = gc_menu_get_boards(); list != NULL; list = list->next)
    {
      GcomprisBoard *board = list->data;
      if (strcmp(board->type, "menu") != 0 &&
	  strcmp(board->section, "/experimental") != 0 &&
	  strcmp(board->section, "/administration") != 0)
	board_count++;
    }

  /* Entry area */
  widget_activation_entry = (GtkEntry *)gtk_entry_new();
  gtk_entry_set_max_length(widget_activation_entry, 6);
  activation_item = \
    gnome_canvas_item_new (gnome_canvas_root(canvas),
			   gnome_canvas_widget_get_type (),
			   "widget", GTK_WIDGET(widget_activation_entry),
			   "x", (double) BOARDWIDTH / 2 - 50,
			   "y", (double) BOARDHEIGHT - 60,
			   "width", 100.0,
			   "height", 30.0,
			   "anchor", GTK_ANCHOR_NW,
			   "size_pixels", FALSE,
			   NULL);
  gtk_signal_connect(GTK_OBJECT(widget_activation_entry), "activate",
		     GTK_SIGNAL_FUNC(activation_enter_callback),
		     NULL);

  gtk_widget_show(GTK_WIDGET(widget_activation_entry));
  gtk_entry_set_text(GTK_ENTRY(widget_activation_entry), "CODE");

  gc_board_stop();

  char *msg = g_strdup_printf(_("GCompris is free software released under the GPL License. In order to support its development, the Windows version provides only %d of the %d activities. You can get the full version for a small fee at\n<http://gcompris.net>\nThe GNU/Linux version does not have this restriction. Note that GCompris is being developed to free schools from monopolistic software vendors. If you also believe that we should teach freedom to children, please consider using GNU/Linux. Get more information at FSF:\n<http://www.fsf.org/philosophy>"),
			      gc_board_number_in_demo, board_count);
  gc_dialog(msg, activation_done);
  g_free(msg);
}

/**
 * Return -1 if the code is not valid
 *        0  if the code is valid but out of date
 *        1  if the code is valid and under 2 years
 */
int gc_activation_check(char *code)
{
  int value = 0;
  int i;
  char crc1 = 0;
  char crc2 = 0;
  char codeddate[4];

  if(strlen(code) != 6)
    return -1;

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
}

/* Check the activation code
 *
 */
static void
activation_enter_callback( GtkWidget *entry,
			   GtkWidget *notused )
{
  switch(gc_activation_check((char *)gtk_entry_get_text(GTK_ENTRY(entry))))
    {
    case 1:
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

  gc_board_play(properties->menu_board);
  gtk_object_destroy (GTK_OBJECT(activation_item));
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

/** \brief toggle full screen mode
 *
 *
 */
void gc_fullscreen_set(gboolean state)
{
#ifdef XF86_VIDMODE
  xf86_vidmode_set_fullscreen(state);
#endif
  if(state)
    {
      gdk_window_set_decorations (window->window, 0);
      gdk_window_set_functions (window->window, 0);
#ifdef XF86_VIDMODE
      if(properties->noxf86vm)
#endif
	gtk_window_fullscreen (GTK_WINDOW(window));
      gtk_widget_set_uposition (window, 0, 0);
    }
  else
    {
      /* The hide must be done at least for KDE */
      if (is_mapped)
        gtk_widget_hide (window);
      gdk_window_set_decorations (window->window, GDK_DECOR_ALL);
      if (is_mapped)
        gtk_widget_show (window);
      gdk_window_set_functions (window->window, GDK_FUNC_ALL);
#ifdef XF86_VIDMODE
      if(properties->noxf86vm)
#endif
	gtk_window_unfullscreen (GTK_WINDOW(window));
      gtk_widget_set_uposition (window, 0, 0);
    }

}

/* Use these instead of the gnome_canvas ones for proper fullscreen mousegrab
   handling. */
int gc_canvas_item_grab (GnomeCanvasItem *item, unsigned int event_mask,
			    GdkCursor *cursor, guint32 etime)
{
  int retval;

  retval = gnome_canvas_item_grab(item, event_mask, cursor, etime);
  if (retval != GDK_GRAB_SUCCESS)
    return retval;

#ifdef XF86_VIDMODE
  /* When fullscreen override mouse grab with our own which
     confines the cursor to our fullscreen window */
  if (XF86VidModeData.fullscreen_active)
    if (gdk_pointer_grab(item->canvas->layout.bin_window, FALSE, event_mask,
          window->window, cursor, etime+1) != GDK_GRAB_SUCCESS)
      g_warning("Pointer grab failed");
#endif

  return retval;
}

void gc_canvas_item_ungrab (GnomeCanvasItem *item, guint32 etime)
{
  gnome_canvas_item_ungrab(item, etime);
#ifdef XF86_VIDMODE
  /* When fullscreen restore the normal mouse grab which avoids
     scrolling the virtual desktop */
  if (XF86VidModeData.fullscreen_active)
    if (gdk_pointer_grab(window->window, TRUE, 0, window->window, NULL,
          etime+1) != GDK_GRAB_SUCCESS)
      g_warning("Pointer grab failed");
#endif
}

static void cleanup()
{
  /* Do not loopback in exit */
  signal(SIGINT,  NULL);
  signal(SIGSEGV, NULL);

  gc_board_stop();
  gc_db_exit();
  gc_fullscreen_set(FALSE);
  gc_menu_destroy();
  gc_prop_destroy(gc_prop_get());
  g_unlink(lock_file);
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
      gc_fullscreen_set(properties->fullscreen);
      is_mapped = TRUE;
    }
  g_warning("gcompris window is now mapped");
}

/*
 * Process the cleanup of the child (no zombies)
 * ---------------------------------------------
 */
void gc_terminate(int signum)
{

  g_warning("gcompris got the %d signal, starting exit procedure", signum);

  gc_exit();

}

static void load_properties ()
{
  gchar *prefix_dir;
  gchar *tmpstr;

  properties = gc_prop_new ();

  /* Initialize the binary relocation API
   *  http://autopackage.org/docs/binreloc/
   */
  if(gbr_init (NULL))
    g_warning("Binary relocation enabled");
  else
    g_warning("Binary relocation disabled");

  prefix_dir = gbr_find_prefix(NULL);
  g_warning("prefix_dir=%s\n", prefix_dir);

  /* Check if we are in the source code (developper usage) */
  tmpstr = g_strconcat(prefix_dir, "/gcompris/gcompris.c", NULL);
  if(g_file_test(tmpstr, G_FILE_TEST_EXISTS))
    {
      /* Set all directory to get data from the source code we are run in */
      properties->package_data_dir = g_strconcat(prefix_dir, "/../boards", NULL);

      /* In source code, locale mo files are not generated, use the installed one */
      properties->package_locale_dir = g_strdup(PACKAGE_LOCALE_DIR);

      properties->package_plugin_dir = g_strconcat(prefix_dir, "/boards/.libs", NULL);
      properties->package_python_plugin_dir = g_strconcat(prefix_dir, "/boards/python", NULL);
      properties->system_icon_dir = g_strconcat(prefix_dir, "/..", NULL);
    }
  else
    {
      gchar *pkg_data_dir = gbr_find_data_dir(PACKAGE_DATA_DIR);
      gchar *pkg_clib_dir = gbr_find_lib_dir(PACKAGE_CLIB_DIR);

      properties->package_data_dir = g_strconcat(pkg_data_dir, "/gcompris/boards", NULL);
      properties->package_locale_dir = gbr_find_locale_dir(PACKAGE_LOCALE_DIR);
      properties->package_plugin_dir = g_strconcat(pkg_clib_dir, "/gcompris", NULL);
      properties->package_python_plugin_dir = g_strconcat(pkg_data_dir, "/gcompris/python", NULL);
      properties->system_icon_dir = g_strconcat(pkg_data_dir, "/pixmaps", NULL);
      g_free(pkg_data_dir);
      g_free(pkg_clib_dir);
    }
  g_free(tmpstr);
  g_free(prefix_dir);


  /* Display the directory value we have */
  printf("package_data_dir         = %s\n", properties->package_data_dir);
  printf("package_locale_dir       = %s\n", properties->package_locale_dir);
  printf("package_plugin_dir       = %s\n", properties->package_plugin_dir);
  printf("package_python_plugin_dir= %s\n", properties->package_python_plugin_dir);
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
 *
 */
void
gc_locale_set(gchar *locale)
{

  if(!locale)
    return;

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
    g_warning("Requested locale '%s' got '%s'", locale, gc_locale);

  if(gc_locale==NULL)
    g_warning("Failed to set requested locale %s got %s", locale, gc_locale);

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

#ifdef XF86_VIDMODE
/*
 * XF86VidMode STUFF
 * -----------------
 */
static void
xf86_vidmode_init ( void )
{
  int i,j, mode_count;
  XF86VidModeModeInfo **modes;
  XF86VidModeModeLine *l = (XF86VidModeModeLine *)((char *)
    &XF86VidModeData.orig_mode + sizeof XF86VidModeData.orig_mode.dotclock);

  if (properties->noxf86vm)
    return;

  if (!XF86VidModeQueryVersion(GDK_DISPLAY(), &i, &j))
    properties->noxf86vm = TRUE;
  else if (!XF86VidModeQueryExtension(GDK_DISPLAY(), &i, &j))
    properties->noxf86vm = TRUE;
  else if (!XF86VidModeGetModeLine(GDK_DISPLAY(), GDK_SCREEN_XNUMBER(
            gdk_screen_get_default()), (int*)&XF86VidModeData.orig_mode.dotclock, l))
    properties->noxf86vm = TRUE;
  else if (!XF86VidModeGetViewPort(GDK_DISPLAY(), GDK_SCREEN_XNUMBER(
            gdk_screen_get_default()), &XF86VidModeData.orig_viewport_x,
            &XF86VidModeData.orig_viewport_y))
    properties->noxf86vm = TRUE;
  else if (!XF86VidModeGetAllModeLines(GDK_DISPLAY(), GDK_SCREEN_XNUMBER(
        gdk_screen_get_default()), &mode_count, &modes))
    properties->noxf86vm = TRUE;
  else {
    unsigned short hdisplay;
    unsigned short vdisplay;

    /* Follow the use choice in the screen size resolution */
    switch(properties->screensize)
      {
      case 0:
	hdisplay = BOARDWIDTH * 0.8;
	vdisplay = (BOARDHEIGHT+BARHEIGHT) * 0.8;
	break;
      case 2:
	hdisplay = BOARDWIDTH * 1.28;
	vdisplay = (BOARDHEIGHT+BARHEIGHT) * 1.28;
	break;
      default:
	hdisplay = BOARDWIDTH;
	vdisplay = BOARDHEIGHT+BARHEIGHT;
	break;
      }
    for (i = 0; i < mode_count; i++)
      {
        if ((modes[i]->hdisplay == hdisplay) &&
            (modes[i]->vdisplay == vdisplay))
          {
            XF86VidModeData.fs_mode = *modes[i];
            break;
          }
      }
    if (i == mode_count)
      properties->noxf86vm = TRUE;
    XFree(modes);
  }

  if (properties->noxf86vm)
      g_warning("XF86VidMode (or 800x600 resolution) not available");
  else
      g_warning("XF86VidMode support enabled");
}


static void
xf86_vidmode_set_fullscreen ( int state )
{
  if (properties->noxf86vm || XF86VidModeData.fullscreen_active == state)
    return;

  if (state)
    {
      if (!XF86VidModeSwitchToMode(GDK_DISPLAY(), GDK_SCREEN_XNUMBER(
            gdk_screen_get_default()), &XF86VidModeData.fs_mode))
        g_warning("XF86VidMode could not switch resolution");
      /* Notice the pointer must be grabbed before setting the viewport
         otherwise setviewport may get "canceled" by the pointer being outside
         the current viewport. */
      if (gdk_pointer_grab(window->window, TRUE, 0, window->window, NULL,
            GDK_CURRENT_TIME) != GDK_GRAB_SUCCESS)
        g_warning("Pointer grab failed");
      if (!XF86VidModeSetViewPort(GDK_DISPLAY(),
            GDK_SCREEN_XNUMBER(gdk_screen_get_default()),
              XF86VidModeData.window_x, XF86VidModeData.window_y))
        g_warning("XF86VidMode could not change viewport");
    }
  else
    {
      if (!XF86VidModeSwitchToMode(GDK_DISPLAY(), GDK_SCREEN_XNUMBER(
            gdk_screen_get_default()), &XF86VidModeData.orig_mode))
        g_warning("XF86VidMode could not restore original resolution");

      gdk_pointer_ungrab(GDK_CURRENT_TIME);
      if (XF86VidModeData.orig_viewport_x || XF86VidModeData.orig_viewport_y)
        if (!XF86VidModeSetViewPort(GDK_DISPLAY(), GDK_SCREEN_XNUMBER(
              gdk_screen_get_default()), XF86VidModeData.orig_viewport_x,
              XF86VidModeData.orig_viewport_y))
          g_warning("XF86VidMode could not restore original viewport");
    }
  XF86VidModeData.fullscreen_active = state;
}

/* We need to accuratly now the window position, this is the only way to get
   it. We also grab the pointer to be sure it is really grabbed. Gtk seems
   to be playing tricks with the window (destroying and recreating?) when
   switching fullscreen <-> window which sometimes (race condition) causes
   the pointer to not be properly grabbed.

   This has the added advantage that this way we know for sure the pointer is
   always grabbed before setting the viewport otherwise setviewport may get
   "canceled" by the pointer being outside the current viewport. */
static gint xf86_window_configured(GtkWindow *window,
  GdkEventConfigure *event, gpointer param)
{
  XF86VidModeData.window_x = event->x;
  XF86VidModeData.window_y = event->y;

  if(XF86VidModeData.fullscreen_active) {
    if (gdk_pointer_grab(event->window, TRUE, 0, event->window, NULL,
          GDK_CURRENT_TIME) != GDK_GRAB_SUCCESS)
      g_warning("Pointer grab failed");
    if (!XF86VidModeSetViewPort(GDK_DISPLAY(),
          GDK_SCREEN_XNUMBER(gdk_screen_get_default()), event->x, event->y))
      g_warning("XF86VidMode could not change viewport");
  }
  /* Act as if we aren't there / aren't hooked up */
  return FALSE;
}

static gint xf86_focus_changed(GtkWindow *window,
  GdkEventFocus *event, gpointer param)
{
  if(properties->fullscreen)
    gdk_pointer_grab(event->window, TRUE, 0, event->window, NULL,
		     GDK_CURRENT_TIME);
  /* Act as if we aren't there / aren't hooked up */
  return FALSE;
}

#endif

static void
start_bg_music (gchar *file)
{
  g_warning ("start_bg_music %s", file);
  gc_sound_bg_reopen();
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

  /* usefull for OSX bundle app */
#ifdef NSBUNDLE
  exec_prefix = gcompris_nsbundle_resource ();
  printf("exec_prefix %s\n", exec_prefix);
#else
  exec_prefix = NULL;
  printf("exec_prefix NULL\n");
#endif

  /* First, Remove the gnome crash dialog because it locks the user when in full screen */
  signal(SIGSEGV, gc_terminate);
  signal(SIGINT, gc_terminate);

#ifdef XF86_VIDMODE
  printf("XF86VidMode: Compiled with XF86VidMode.\nIf you have problems starting GCompris in fullscreen, try the -x option to disable XF86VidMode.\n");
#else
  printf("XF86VidMode: Not compiled with XVIDMODE\n");
#endif

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

  // Set the default gcompris cursor
  properties->defaultcursor = GCOMPRIS_DEFAULT_CURSOR;

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
      printf("popt_config_dir\n");
      if ((!g_file_test(popt_config_dir, G_FILE_TEST_IS_DIR)) ||
	  (g_access(popt_config_dir, popt_administration? W_OK : R_OK ) == -1))
	{
	  printf("%s does not exists or is not %s", popt_config_dir,
		 popt_administration? "writable" : "readable"	);
	  exit(0);
	}
      else
	{
	  g_warning("Using %s as config directory.", popt_config_dir);
	  g_free(properties->config_dir);
	  properties->config_dir = g_strdup(popt_config_dir);
	}
    }

  /* Now we know where our config file is, load the saved config */
  gc_prop_load(properties);

  /* Single instance Check */
  lock_file = g_strdup_printf("%s/%s", properties->config_dir, GC_LOCK_FILE);
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
	    exit(0);
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

  if (popt_noxf86vm)
    {
      properties->noxf86vm = TRUE;
    }

  if (popt_window)
    {
      properties->fullscreen = FALSE;
    }

  if (popt_mute)
    {
      g_warning("Sound disabled");
      properties->music = FALSE;
      properties->fx = FALSE;
    }

  if (popt_sound)
    {
      g_warning("Sound enabled");
      properties->music = TRUE;
      properties->fx = TRUE;
    }

  if (popt_cursor)
    {
      g_warning("Default gnome cursor enabled");
      properties->defaultcursor = GDK_LEFT_PTR;
    }
#ifdef WIN32
  properties->defaultcursor = GDK_LEFT_PTR;
#endif

  if (popt_aalias)
    {
      g_warning("Slower Antialiased canvas used");
      antialiased = TRUE;
    }

  if (popt_experimental)
    {
      g_warning("Experimental boards allowed");
      properties->experimental  = TRUE;
    }

  if (popt_no_quit)
    {
      g_warning("Disable quit button");
      properties->disable_quit = TRUE;
    }

  if (popt_no_config)
    {
      g_warning("Disable config button");
      properties->disable_config = TRUE;
    }

  if (popt_difficulty_filter>=0)
    {
      /* This option provide less capacity than the GUI since we cannot set the filter_style */
      g_warning("Display only activities of level %d", popt_difficulty_filter);
      properties->difficulty_filter = popt_difficulty_filter;
      properties->filter_style      = GCOMPRIS_FILTER_EQUAL;
    }

  if (popt_local_activity){
    g_warning("Adding local activies from %s.", popt_local_activity);
    properties->local_directory = g_strdup(popt_local_activity);
  }

  if (popt_root_menu){
    if (strcmp(popt_root_menu,"list")==0){
      /* check the list of possible values for -l, then exit */
      printf(_("Use -l to access an activity directly.\n"));
      printf(_("The list of available activities is :\n"));

      gc_db_init();

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
      /* FIXME: Need to translate */
      printf("Number of activities: %d\n", board_count);

      exit(0);
    }
    else {
      g_warning("Using menu %s as root.", popt_root_menu);
      g_free(properties->root_menu);
      properties->root_menu = g_strdup(popt_root_menu);
    }
  }

  if (popt_user_dir)
    {
      if ((!g_file_test(popt_user_dir, G_FILE_TEST_IS_DIR)) ||
	  (g_access(popt_user_dir, popt_administration? R_OK : W_OK ) == -1))
	{
	  g_warning("%s does not exists or is not %s ", popt_user_dir,
		    popt_administration? "readable" : "writable");
	  exit(0);
	}
      else
	{
	  g_warning("Using %s as user directory.", popt_user_dir);
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
	      printf("%s exists but is not readable or writable", properties->database);
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
  printf("Infos:\n   Config dir '%s'\n   Users dir '%s'\n   Database '%s'\n",
	 properties->config_dir,
	 properties->user_dir,
	 properties->database);

  if (popt_create_db)
    {
      gchar *dirname = g_path_get_dirname (properties->database);
      if (g_access(dirname, W_OK)==-1)
	{
	  g_warning("Cannot create %s : %s is not writable !", properties->database, dirname);
	  exit (0);
	}
      /* We really want to recreate it, erase the old one */
      g_warning("Removing %s database.", properties->database);
      unlink(properties->database);
    }

  if (popt_administration){
    if (popt_database){
      if (g_access(popt_database,R_OK|W_OK)==-1){
	g_warning("%s exists but is not writable", popt_database);
	exit(0);
      }
    }
    g_warning("Running in administration mode");
    properties->administration = TRUE;
    g_warning("Music disabled");
    properties->music = FALSE;
    properties->fx = FALSE;
    g_warning("Fullscreen and cursor is disabled");
    properties->fullscreen = FALSE;
    properties->defaultcursor = GDK_LEFT_PTR;
  }

  if (popt_reread_menu){
    g_warning("Rebuild db from xml files");
    if (g_access(properties->database, W_OK)==-1)
      g_warning("Cannot reread menu when database is read-only !");
    else
      properties->reread_menu = TRUE;
  }

  if (popt_server){
#ifdef USE_GNET
      properties->server = g_strdup(popt_server);
#else
      printf("The --server option cannot be used because GCompris has been compiled without network support!");
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
      properties->cache_dir = g_strdup(popt_cache_dir);
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
	else g_warning("Unknown drag mode ! Valids modes are \"normal\", \"2clicks\" and \"both\"");
      }
    }
  }

  gc_prop_activate(properties);

  /*
   * Database init MUST BE after properties
   * And after a possible alternate database as been provided
   *
   */
  gc_db_init();

  /* An alternate profile is requested, check it does exists */
  if (popt_profile){
    properties->profile = gc_db_profile_from_name_get(popt_profile);

    if(properties->profile == NULL)
      {
	printf("ERROR: Profile '%s' is not found. Run 'gcompris --profile-list' to list available ones\n",
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

  /* An alternate profile is requested, check it does exists */
  if (popt_display_resource){
    properties->display_resource = TRUE;
    properties->reread_menu = TRUE;
    printf("Resources for selected activities (as selected by gcompris --administration):\n");
    gc_menu_load();
    exit(0);
  }

  /*------------------------------------------------------------*/

  gc_skin_load(properties->skin);

  if(properties->music || properties->fx)
    gc_sound_init();

  /* Gdk-Pixbuf */
  gdk_rgb_init();

  /* Cache init */
  gc_cache_init(-1);

  /* networking init */
  gc_net_init();

  setup_window ();

  gtk_widget_show_all (window);

  /* If a specific activity is selected, skeep the intro music */
  if(!popt_root_menu)
    {
      if (properties->music)
	{
	  gc_sound_play_ogg("music/intro.ogg",
			    NULL);
	  gc_sound_play_ogg_cb("sounds/$LOCALE/misc/welcome.ogg",
			       start_bg_music);
	}
      else
	gc_sound_play_ogg("sounds/$LOCALE/misc/welcome.ogg", NULL);
    }

  gtk_main ();

  /* FIXME: HACK Needed or we have unresolved symbols at python plugin dlopen
   *        Is there a better way to fix these?
   */
  GType dummy = gnome_canvas_polygon_get_type();
  dummy = gnome_canvas_clipgroup_get_type();
  dummy = gnome_canvas_bpath_get_type();

  return(0);
}
