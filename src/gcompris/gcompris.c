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
#include <popt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "gcompris.h"
#include "gcompris_config.h"
#include <locale.h>

#include "cursor.h"

/* For XRANDR Support */
#ifdef XRANDR
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xrender.h>
#endif

#if defined _WIN32 || defined __WIN32__
# undef WIN32   /* avoid warning on mingw32 */
# define WIN32
#endif

GtkWidget *window;
GtkWidget *drawing_area;
GnomeCanvas *canvas;
GnomeCanvas *canvas_bar;
GnomeCanvas *canvas_bg;

//static gint pause_board_cb (GtkWidget *widget, gpointer data);
static void quit_cb (GtkWidget *widget, gpointer data);
static gint board_widget_key_press_callback (GtkWidget   *widget,
					    GdkEventKey *event,
					    gpointer     client_data);

GcomprisProperties	*properties = NULL;
static gboolean		 antialiased = FALSE;

/****************************************************************************/
/* Some constants.  */

static GnomeCanvasItem *backgroundimg = NULL;
static gchar           *gcompris_locale = NULL;
static gchar           *gcompris_user_default_locale = NULL;
static gboolean		gcompris_debug = FALSE;

/****************************************************************************/
/* Command line params */

/*** gcompris-popttable */
static int popt_fullscreen	   = FALSE;
static int popt_window		   = FALSE;
static int popt_sound		   = FALSE;
static int popt_mute		   = FALSE;
static int popt_cursor		   = FALSE;
static int popt_version		   = FALSE;
static int popt_aalias		   = FALSE;
static int popt_difficulty_filter  = FALSE;
static int popt_debug		   = FALSE;
static int popt_noxrandr	   = FALSE;
static char *popt_root_menu        = NULL;
static char *popt_local_activity   = NULL;
static int popt_administration	   = FALSE;
static char *popt_database         = NULL;
static char *popt_logs_database    = NULL;
static int popt_create_db   	   = FALSE;
static int popt_reread_menu   	   = FALSE;
static char *popt_profile	   = NULL;
static int *popt_profile_list	   = FALSE;
static char *popt_shared_dir	   = NULL;
static char *popt_users_dir	   = NULL;

GTimer *chronometer;

static struct poptOption options[] = {
  {"fullscreen", 'f', POPT_ARG_NONE, &popt_fullscreen, 0,
   N_("run gcompris in fullscreen mode."), NULL},
  {"window", 'w', POPT_ARG_NONE, &popt_window, 0,
   N_("run gcompris in window mode."), NULL},
  {"sound", 's', POPT_ARG_NONE, &popt_sound, 0,
   N_("run gcompris with sound enabled."), NULL},
  {"mute", 'm', POPT_ARG_NONE, &popt_mute, 0,
   N_("run gcompris without sound."), NULL},
  {"cursor", 'c', POPT_ARG_NONE, &popt_cursor, 0,
   N_("run gcompris with the default gnome cursor."), NULL},
  {"difficulty", 'd', POPT_ARG_INT, &popt_difficulty_filter, 0,
   N_("display only activities with this difficulty level."), NULL},
  {"debug", 'D', POPT_ARG_NONE, &popt_debug, 0,
   N_("display debug informations on the console."), NULL},
  {"version", 'v', POPT_ARG_NONE, &popt_version, 0,
   N_("Print the version of " PACKAGE), NULL},
  {"antialiased", '\0', POPT_ARG_NONE, &popt_aalias, 0,
   N_("Use the antialiased canvas (slower)."), NULL},
  {"noxrandr", 'x', POPT_ARG_NONE, &popt_noxrandr, 0,
   N_("Disable XRANDR (No screen resolution change)."), NULL},
  {"root-menu", 'l', POPT_ARG_STRING, &popt_root_menu, 0,
   N_("Run gcompris with local menu (e.g -l /reading will let you play only reading activity directory, -l /boards/connect4 only the connect4 activity), -l list show the list of menus and activity"), NULL},
  {"local-activity", 'L', POPT_ARG_STRING, &popt_local_activity, 0,
   N_("Run gcompris with local activity directory added to menu"), NULL},
  {"administration", 'a', POPT_ARG_NONE, &popt_administration, 0,
   N_("Run gcompris with administration and users management mode"), NULL},
  {"database", 'b', POPT_ARG_STRING, &popt_database, 0,
   N_("Use alternate database for profiles"), NULL},
  {"logs", 'j', POPT_ARG_STRING, &popt_logs_database, 0,
   N_("Use alternate database for logs"), NULL},
  {"create-db",'\0', POPT_ARG_NONE, &popt_create_db, 0,
   N_("Create the alternate database for profiles"), NULL},
  {"reread-menu",'\0', POPT_ARG_NONE, &popt_reread_menu, 0,
   N_("Re-read XML Menus and store them in the database"), NULL},
  {"profile",'p', POPT_ARG_STRING, &popt_profile, 0,
   N_("Set the profile to use. Use 'gcompris -a' to create profiles"), NULL},
  {"profile-list",'\0', POPT_ARG_NONE, &popt_profile_list, 0,
   N_("List all availaible profiles. Use 'gcompris -a' to create profiles"), NULL},
  {"shared-dir",'\0', POPT_ARG_STRING, &popt_shared_dir, 0,
   N_("SHDIR Shared directory emplacement. For profiles and boards configuration data. [$HOME/.gcompris/shared]"), NULL},
  {"users-dir",'\0', POPT_ARG_STRING, &popt_users_dir, 0,
   N_("USERSDIR Users directory emplacement. [$HOME/.gcompris/users] "), NULL},
#ifndef WIN32	/* Not supported on windows */
  POPT_AUTOHELP
#endif
  {
    NULL,
    '\0',
    0,
    NULL,
    0,
    NULL,
    NULL
  }
};

/* XRandr Stuff */
#ifdef XRANDR
typedef struct 
{
  gboolean                xr_lock_updates;

  XRRScreenConfiguration *xr_screen_conf;

  XRRScreenSize          *xr_sizes;
  int                     xr_nsize;
  SizeID	          xr_current_size;

  Rotation                xr_rotations;
  Rotation                xr_current_rotation;

} XRANDRData; 

static gboolean		 changed_xrandr = FALSE;
static SizeID		 xr_previous_size = -1;
static XRANDRData	*xrandr;

static void xrandr_get_config ( XRANDRData *data );
gboolean xrandr_set_config( XRANDRData  *grandr );
#endif

/****************************************************************************/

/* Remove any dialog box */
static void gcompris_close_all_dialog() {
  gcompris_dialog_close();
  gcompris_help_stop();
  gcompris_config_stop();
  gcompris_about_stop();
  gcompris_file_selector_stop();
  gcompris_images_selector_stop();
}

static gint
board_widget_key_press_callback (GtkWidget   *widget,
				GdkEventKey *event,
				gpointer     client_data)
{

  if(event->state & GDK_CONTROL_MASK && ((event->keyval == GDK_r)
					 || (event->keyval == GDK_R))) {
    g_message("Refreshing the canvas\n");
    gnome_canvas_update_now(canvas);
    return TRUE;
  }

  if(event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK) && ((event->keyval == GDK_l)
							  || (event->keyval == GDK_L))) {
    properties->key="thanks_for_your_help";
    gcompris_properties_save(properties);
    gcompris_load_menus();

    gcompris_close_all_dialog();

    board_stop();
    return TRUE;
  }

  if(event->state & GDK_CONTROL_MASK && ((event->keyval == GDK_p)
					 || (event->keyval == GDK_P))) {
    properties->key="thanks_for_your_help";
    gcompris_properties_save(properties);
    gcompris_load_menus();

    gcompris_close_all_dialog();

    board_stop();
    return TRUE;
  }

  if(event->state & GDK_CONTROL_MASK && ((event->keyval == GDK_q)
					 || (event->keyval == GDK_Q))) {
    gcompris_exit();
    return TRUE;
  }

  switch (event->keyval)
    {
    case GDK_Escape:

      gcompris_close_all_dialog();

      board_stop();
      return TRUE;
    case GDK_F5:
      g_message("Refreshing the canvas\n");
      gnome_canvas_update_now(canvas);
      return TRUE;
    case GDK_KP_Enter:
    case GDK_Return:
      /* If the board needs to receive key pressed */
      /* NOTE: If a board receives key press, it must bind the ENTER Keys to OK
       *       whenever possible
       */
      if (get_current_board_plugin()!=NULL && get_current_board_plugin()->key_press)
	{
	  return(get_current_board_plugin()->key_press (event->keyval));
	} 
      else if (get_current_board_plugin()!=NULL && get_current_board_plugin()->ok)
	{
	  /* Else we send the OK signal. */
	  get_current_board_plugin()->ok ();
	}
      return TRUE;
    default:
      /* If the board needs to receive key pressed */
      if (get_current_board_plugin()!=NULL && get_current_board_plugin()->key_press)
	{
	  // Removed, Too hard to analyse.
	  //	  gcompris_log_set_key(get_current_board_plugin(), event->keyval);

	  return(get_current_board_plugin()->key_press (event->keyval));
	}
    }

  /* Event not handled; try parent item */
  return FALSE;
};

/**
 * Return the main canvas we run in
 */
GnomeCanvas *gcompris_get_canvas()
{
  return canvas;
}

GtkWidget *gcompris_get_window()
{
  return window;
}


GnomeCanvasItem *gcompris_set_background(GnomeCanvasGroup *parent, gchar *file)
{
  GdkPixbuf *background_pixmap = NULL;

  background_pixmap = gcompris_load_pixmap (file);

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

  return (backgroundimg);
}

static void init_background()
{
  double xratio, yratio, max;
  gint screen_height, screen_width;
  GtkWidget *vbox;
  int i;

#ifdef XRANDR
  /* Search the 800x600 Resolution */
  if(properties->fullscreen && !properties->noxrandr) {
    g_warning("XRANDR Is compiled in. Searching a good resolution");

    xrandr = g_new0 (XRANDRData, 1);
    xrandr_get_config ( xrandr );
    xr_previous_size = (SizeID)xrandr->xr_current_size;

    for (i = 0; i < xrandr->xr_nsize; i++) {
      if(xrandr->xr_sizes[i].width == BOARDWIDTH, xrandr->xr_sizes[i].height == BOARDHEIGHT+BARHEIGHT) {
	xrandr->xr_current_size = (SizeID)i;
	xrandr_set_config( xrandr );
	changed_xrandr = TRUE;
	break;
      }
    }
    screen_height = xrandr->xr_sizes[xrandr->xr_current_size].height;
    screen_width  = xrandr->xr_sizes[xrandr->xr_current_size].width;
  } else {
  screen_height = gdk_screen_height();
  screen_width  = gdk_screen_width();
  }
#else
  screen_height = gdk_screen_height();
  screen_width  = gdk_screen_width();
#endif

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
      /* First, Remove the gnome crash dialog because it locks the user when in full screen */
      signal(SIGSEGV, SIG_DFL);

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

void gcompris_set_cursor(guint gdk_cursor_type)
{
  GdkCursor *cursor;

  // Little hack to force gcompris to use the default cursor
  if(gdk_cursor_type==GCOMPRIS_DEFAULT_CURSOR)
    gdk_cursor_type=properties->defaultcursor;

  // I suppose there is less than GCOMPRIS_FIRST_CUSTOM_CURSOR cursors defined in gdkcursors.h !
  if (gdk_cursor_type < GCOMPRIS_FIRST_CUSTOM_CURSOR) {
    cursor = gdk_cursor_new(gdk_cursor_type);
    gdk_window_set_cursor (window->window, cursor);
    gdk_cursor_destroy(cursor);
  } else { // we use a custom cursor
    GdkColor fg, bg;
    //    static const gchar * cursor;
    static const gchar ** bits;

    gdk_color_parse("rgb:FFFF/FFFF/FFFF",&fg);
    gdk_color_parse("rgb:FFFF/3FFF/0000",&bg);

    gdk_color_parse("black",&fg);
    gdk_color_parse("red",&bg);

    switch (gdk_cursor_type) {
    case GCOMPRIS_BIG_RED_ARROW_CURSOR : 
      bits = big_red_arrow_cursor_bits;
      break;
    case GCOMPRIS_BIRD_CURSOR : 
      bits = bird_cursor_bits; 
      break;
    case GCOMPRIS_LINE_CURSOR : 
      bits = big_red_line_cursor_bits; 
      break;
    case GCOMPRIS_RECT_CURSOR : 
      bits = big_red_rectangle_cursor_bits; 
      break;
    case GCOMPRIS_FILLRECT_CURSOR : 
      bits = big_red_filledrectangle_cursor_bits; 
      break;
    case GCOMPRIS_CIRCLE_CURSOR : 
      bits = big_red_circle_cursor_bits; 
      break;
    case GCOMPRIS_FILLCIRCLE_CURSOR : 
      bits = big_red_filledcircle_cursor_bits; 
      break;
    case GCOMPRIS_FILL_CURSOR : 
      bits = big_red_fill_cursor_bits; 
      break;
    case GCOMPRIS_DEL_CURSOR : 
      bits = big_red_del_cursor_bits; 
      break;
    case GCOMPRIS_SELECT_CURSOR : 
      bits = big_red_select_cursor_bits; 
      break;
    default : bits = big_red_arrow_cursor_bits;
    }
    
    cursor = gdk_cursor_new_from_data(bits, 40 , 40, &fg, &bg, 0, 0);
    gdk_window_set_cursor(window->window, cursor);
    gdk_cursor_unref(cursor);
  }
}

static void setup_window ()
{
  GcomprisBoard *board_to_start;
  GdkPixbuf     *gcompris_icon_pixbuf;
  GError        *error = NULL;

  gdouble        elapsed;
  gulong         musec;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  /*
   * Set an icon for gcompris
   * ------------------------
   */
  if (!g_file_test ((GNOME_ICONDIR"/gcompris.png"), G_FILE_TEST_EXISTS)) {
      g_warning (_("Couldn't find file %s !"), GNOME_ICONDIR"/gcompris.png" );
  }
  gcompris_icon_pixbuf = gdk_pixbuf_new_from_file (GNOME_ICONDIR"/gcompris.png", &error);
  if (!gcompris_icon_pixbuf)
    {
      g_warning ("Failed to load pixbuf file: %s: %s\n",
               GNOME_ICONDIR"/gcompris.png", error->message);
      g_error_free (error);
    }
  if (gcompris_icon_pixbuf)
    {
      gtk_window_set_icon (GTK_WINDOW (window), gcompris_icon_pixbuf);
      gdk_pixbuf_unref (gcompris_icon_pixbuf);
    }

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

  /* For non anti alias canvas */
  gtk_widget_push_visual (gdk_rgb_get_visual ());
  gtk_widget_push_colormap (gdk_rgb_get_cmap ());

  // Set the cursor
  gcompris_set_cursor(GCOMPRIS_DEFAULT_CURSOR);

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

  if(properties->fullscreen)
    gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET(canvas_bg));

  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();


  gtk_widget_show (GTK_WIDGET(canvas_bg));

  if(properties->fullscreen)
    {
      gdk_window_set_decorations (window->window, 0);
      gdk_window_set_functions (window->window, 0);
      gtk_widget_set_uposition (window, 0, 0);
      gtk_window_fullscreen (GTK_WINDOW(window));
    }

  init_plugins();
  
  elapsed = g_timer_elapsed( chronometer, &musec);
  printf("init_plugins %f sec.\n", elapsed);

  /* Load all the menu once */
  gcompris_load_menus();

  elapsed = g_timer_elapsed( chronometer, &musec);
  printf("gcompris_load_menus %f sec.\n", elapsed);

  /* Load the mime type */
  gcompris_load_mime_types();

  elapsed = g_timer_elapsed( chronometer, &musec);
  printf("gcompris_load_mime_types %f sec.\n", elapsed);
  
  /* Save the root_menu */
  properties->menu_board = gcompris_get_board_from_section(properties->root_menu);

  /* By default, the menu will be started */
  board_to_start = properties->menu_board;

  /* Get and Run the root menu */
  if(properties->administration)
    {
      board_to_start = gcompris_get_board_from_section("/administration/administration");
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
	    if (g_list_length(gcompris_get_users_from_group( *((int *) group_id->data))) > 0){
	      found = TRUE;
	      break;
	    }
	    
	  /* No profile start normally */
	  if (found)
	    board_to_start = gcompris_get_board_from_section("/login/login");	
	  else {
	    board_to_start = gcompris_get_board_from_section(properties->root_menu);
	    /* this will set user information to system one */
	    gcompris_set_current_user(NULL);
	  }
	}
      else
	/* this will set user information to system one */
	gcompris_set_current_user(NULL);
    }

  if(!board_to_start) {
    g_warning("Couldn't find the board menu %s, or plugin execution error", properties->root_menu);
    exit(1);
  } else if(!board_check_file(board_to_start)) {
    g_error("Couldn't find the board menu, or plugin execution error");
  } else {
    g_warning("Fine, we got the gcomprisBoardMenu, xml boards parsing went fine");
  }
  /* Run the bar */
  gcompris_bar_start(canvas_bar);

  init_background();

  board_play (board_to_start);

#ifdef WIN32
  {
    if(strncmp(properties->key, "thanks_for_your_help", 20)!=0) {
      board_pause();
      gcompris_dialog(_("GCompris is free software released under the GPL License. In order to support the development, the Windows version provides only 12 of the 45 activities. You can get the full version for a small fee at\nhttp://gcompris.net\nThe Linux version does not have this restriction. Note that gcompris is being developped to free schools from monopolistic software vendors. If you also believe that we should teach freedom to kids, please consider using GNU/Linux. Get more information on\nhttp://www.fsf.org/philosophy"), NULL);
    }
  }
#endif
}

void gcompris_end_board()
{
  if (get_current_gcompris_board()->previous_board == NULL)
    /* We are in the upper menu */
    /* board_play (properties->menu_board); */
    gcompris_exit();
  else
    /* Run the previous board */
    board_play (get_current_gcompris_board()->previous_board);
}

void gcompris_exit()
{
  board_stop();

  gcompris_db_exit();

  g_timer_destroy (chronometer);

#ifdef XRANDR
  /* Set back the original screen size */
  if((properties->fullscreen || changed_xrandr) && properties->noxrandr == 0) 
    {
      /* Need to refresh our config or xrandr api will reject us */
      xrandr_get_config ( xrandr );
      xrandr->xr_current_size = (SizeID)xr_previous_size;
      xrandr_set_config( xrandr );
    }
#endif

  gtk_main_quit ();


}

static void quit_cb (GtkWidget *widget, gpointer data)
{

#ifdef DMALLOC
  dmalloc_shutdown();
#endif
  gcompris_exit();

}

/* 
 * Process the cleanup of the child (no zombies)
 * ---------------------------------------------
 */
void gcompris_terminate(int  signum)
{

  g_warning("gcompris got the SIGINT signal, starting exit procedure");
  gcompris_exit();
  
}

static void load_properties ()
{
  properties = gcompris_properties_new ();
  gcompris_skin_load(properties->skin);
}

GcomprisProperties *gcompris_get_properties ()
{
  return (properties);
}

/* Return the database file name
 * Must be called after properties is initialised
 */
gchar *gcompris_get_database ()
{
  g_assert(properties!=NULL);

  return (properties->database);
}

/*
 * This returns the locale for which text must be displayed
 *
 */
const gchar *gcompris_get_locale()
{
  const gchar *locale;

  /* First check locale got overrided by the user */
  if(gcompris_locale != NULL)
    return(gcompris_locale);

  locale = g_getenv("LC_ALL");
  if(locale == NULL)
    locale = g_getenv("LC_MESSAGES");
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
char *gcompris_get_user_default_locale()
{
  return gcompris_user_default_locale;
}

/*
 * This set the locale for which text must be displayed
 *
 */
void gcompris_set_locale(gchar *locale)
{

  g_message("gcompris_set_locale '%s'\n", locale);
  if(gcompris_locale != NULL)
    g_free(gcompris_locale);

#if defined WIN32
  /* On windows, it always works */
  gcompris_locale = g_strdup(locale);
  setlocale(LC_MESSAGES, locale);
  setlocale(LC_ALL, locale);
#else
  gcompris_locale = g_strdup(setlocale(LC_MESSAGES, locale));
  if (!gcompris_locale)
    gcompris_locale = g_strdup(locale);
#endif

  if(gcompris_locale!=NULL && strcmp(locale, gcompris_locale))
    g_warning("Requested locale '%s' got '%s'", locale, gcompris_locale);

  if(gcompris_locale==NULL)
    g_warning("Failed to set requested locale %s got %s", locale, gcompris_locale);

  /* Override the env locale to what the user requested */
  /* This makes gettext to give us the new locale text  */
  my_setenv ("LC_ALL", gcompris_get_locale());
  my_setenv ("LC_MESSAGES", gcompris_get_locale());
  my_setenv ("LANGUAGE", gcompris_get_locale());
  my_setenv ("LANG", gcompris_get_locale());

  /* This does update gettext translation uppon next gettext call */
  /* Call for localization startup */
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* Make change known.  */
  {
    extern int  _nl_msg_cat_cntr;
    ++_nl_msg_cat_cntr;
  }

}

void gcompris_log_handler (const gchar *log_domain,
			   GLogLevelFlags log_level,
			   const gchar *message,
			   gpointer user_data) {
  if(gcompris_debug)
    g_printerr ("%s: %s\n\n", "gcompris", message);
}

#ifdef XRANDR
/*
 * XRANDR STUFF
 * ------------
 */
static void
xrandr_get_config ( XRANDRData *data )
{
  data->xr_screen_conf = XRRGetScreenInfo (GDK_DISPLAY(), GDK_ROOT_WINDOW());
  
  data->xr_current_size = XRRConfigCurrentConfiguration (data->xr_screen_conf, 
							 &data->xr_current_rotation);

  data->xr_sizes = XRRConfigSizes(data->xr_screen_conf, &data->xr_nsize);
  
  data->xr_rotations = XRRConfigRotations(data->xr_screen_conf,
					  &data->xr_current_rotation);
}

gboolean
xrandr_set_config( XRANDRData  *data )
{
  Status  status = RRSetConfigFailed;

  if (data->xr_lock_updates) return FALSE;
  
  status = XRRSetScreenConfig (GDK_DISPLAY(), 
			       data->xr_screen_conf, 
			       GDK_ROOT_WINDOW(), 
			       data->xr_current_size, 
			       data->xr_current_rotation, 
			       CurrentTime);

  if(status) {
    printf("ERROR: Failed to set back the original resolution XRRSetScreenConfig returned status = %d\n", (int)status);
  }
  return TRUE;

}
#endif

/*****************************************
 * Main
 *
 */

int
gcompris_init (int argc, char *argv[])
{
  poptContext pctx; 
  int popt_option;
  gchar *str;

  chronometer = g_timer_new ();

  g_timer_start (chronometer);

  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* To have some real random behaviour */
  srand (time (NULL));

  signal(SIGINT, gcompris_terminate);

  /* Default difficulty filter: non specified */
  popt_difficulty_filter = -1;

  gtk_init (&argc, &argv);

  pctx = poptGetContext (PACKAGE, argc, (const char **)argv, options, 0);

  /* Argument parsing */
  popt_option = poptGetNextOpt (pctx);

  load_properties ();

  // Set the default gcompris cursor
  properties->defaultcursor = GCOMPRIS_DEFAULT_CURSOR;

  /* Save the default locale */
#if defined WIN32
  gcompris_user_default_locale = g_win32_getlocale();
  // Set the user's choice locale
  if(properties->locale[0]=='\0') {
    gcompris_set_locale(gcompris_user_default_locale);
  } else {
    gcompris_set_locale(properties->locale);
  }
#else
  gcompris_user_default_locale = g_strdup(setlocale(LC_MESSAGES, NULL));
  // Set the user's choice locale
  gcompris_set_locale(properties->locale);
#endif

  /* Set the default message handler, it avoids message with option -D */
  g_log_set_handler (NULL, G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING | G_LOG_FLAG_FATAL
		     | G_LOG_FLAG_RECURSION, gcompris_log_handler, NULL);

  /*------------------------------------------------------------*/
  if (popt_version)
    {
      printf (_("GCompris\nVersion: %s\nLicence: GPL\n"
		"More info at http://gcompris.net\n"),
	      VERSION);
      exit (0);
    }

  if (popt_debug)
    {
      gcompris_debug = TRUE;
    }

  if (popt_fullscreen)
    {
      properties->fullscreen = TRUE;
    }

  if (popt_noxrandr)
    {
      properties->noxrandr = TRUE;
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

  if (popt_aalias)
    {
      g_warning("Slower Antialiased canvas used");
      antialiased = TRUE;
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
      printf(_("Use -l activity to access directly to activity.\n"));
      printf(_("List of available activity is :\n"));
      properties->root_menu = "/";

      gcompris_db_init();

      gcompris_load_menus();

      GList *list = NULL;
      GList *menulist = NULL;
      GList *menu_todo = NULL;

      menu_todo = g_list_append(menu_todo,g_strdup("/"));

      while ( menu_todo != NULL) {
	menulist = gcompris_get_menulist(menu_todo->data);
	g_free(menu_todo->data);
	menu_todo = menu_todo->next;

	for(list = menulist; list != NULL; list = list->next) {
	  GcomprisBoard *board = list->data;
	
	  if (board){
	    if (strcmp(board->type,"menu")==0)
	      menu_todo = g_list_prepend(menu_todo, g_strdup_printf("%s/%s",board->section, board->name));

	    printf("%s/%s : %s (%s) \n", board->section, board->name, board->title, board->description );
	  }
	}
      }
      
      exit(0);
    }
    else {
    g_warning("Using menu %s as root.", popt_root_menu);
    properties->root_menu = g_strdup(popt_root_menu);
    }
  }

  if (popt_users_dir){
    if ((!g_file_test(popt_users_dir, G_FILE_TEST_IS_DIR)) || 
	(access(popt_users_dir, popt_administration? R_OK : W_OK ) == -1)){
	g_warning("%s does not exists or is not %s ", popt_users_dir,
		  popt_administration? "readable" : "writable");
	exit(0);
    } else {
      g_warning("Using %s as users directory.", popt_users_dir);
      properties->users_dir = g_strdup(popt_users_dir);
    }
  }

  if (popt_shared_dir){
    if ((!g_file_test(popt_shared_dir, G_FILE_TEST_IS_DIR)) ||
	(access(popt_shared_dir, popt_administration? W_OK : R_OK ) == -1)){
      g_warning("%s does not exists or is not %s", popt_shared_dir,
		popt_administration? "writable" : "readable"	);
      exit(0);
    }
    else {
      g_warning("Using %s as shared directory.", popt_shared_dir);
      properties->shared_dir = g_strdup(popt_shared_dir);
    }
  }

  /* shared_dir initialised, now we can set the default */
  properties->database = get_default_database_name ( properties->shared_dir );
  printf( " Infos: %s %s %s \n",  
	  properties->shared_dir, 
	  properties->users_dir,
	  properties->database);

  if (popt_database){
    if (g_file_test(popt_database, G_FILE_TEST_EXISTS)) {
      if (access(popt_database,R_OK)==-1){
	g_warning("%s exists but is not readable or writable", popt_database);
	exit(0);
      } else {
	g_warning("Using %s as database", popt_database);
	properties->database = g_strdup(popt_database);
      }
    } else if (popt_create_db) {
      gchar *dirname = g_path_get_dirname (popt_database);
      if (access(dirname, W_OK)==-1){
	g_warning("Cannot create %s : %s is not writable !", popt_database, dirname);
	exit (0);
      }
      g_warning("Using %s as database.", popt_database);
      properties->database = g_strdup(popt_database);
    } else {
      g_warning("Alternate database %s does not exists.\n Use --create-db to force creation !", popt_database); 
      exit(0);
    }
  }

  if (popt_administration){
    if (popt_database){
      if (access(popt_database,R_OK|W_OK)==-1){
	g_warning("%s exists but is not writable", popt_database);
	exit(0);
      }
    }
    g_warning("Running in administration mode");
    properties->administration = TRUE;
    g_warning("Background music disabled");
    properties->music = FALSE;
    g_warning("Fullscreen is disabled");
    properties->fullscreen = FALSE;
  }

  if (popt_reread_menu){
    g_warning("Rebuild db from xml files");
    properties->reread_menu = TRUE;
  }


  /*
   * Database nit MUST BE after properties
   * And after a possible alternate database as been provided
   *
   */
  gcompris_db_init();
  
  /* An alternate profile is requested, check it does exists */
  if (popt_profile){
    GList * profile_list;

    properties->profile = gcompris_get_profile_from_name(popt_profile);

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

    profile_list = gcompris_get_profiles_list();

    printf(_("List of available profiles are:\n"));
    for(i=0; i< g_list_length(profile_list); i++)
      {
	GcomprisProfile *profile = g_list_nth_data(profile_list, i);
	printf("   %s\n", profile->name);
      }

    g_list_free(profile_list);

    exit(0);
  }

  poptFreeContext(pctx); 
  /*------------------------------------------------------------*/

  if(properties->music || properties->fx)
    initSound();

  /* Gdk-Pixbuf */
  gdk_rgb_init();

  setup_window ();

  gtk_widget_show_all (window);

  str = gcompris_get_asset_file("gcompris misc", NULL, "audio/x-ogg", "welcome.ogg");

  if (properties->music)
    {
      gcompris_play_ogg("intro", str, NULL);
    }
  else
    gcompris_play_ogg(str, NULL);

  g_free(str);

  gtk_main ();
  return 0;
}
    


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
