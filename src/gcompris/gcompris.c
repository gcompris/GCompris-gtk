/* gcompris - gcompris.c
 *
 * Time-stamp: <2004/04/14 01:08:56 bcoudoin>
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

#include "gcompris.h"
#include <locale.h>

#include "cursor.h"

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
GcomprisBoard		*gcomprisBoardMenu = NULL;
static gboolean		 antialiased = FALSE;

/****************************************************************************/
/* Some constants.  */

static GnomeCanvasItem *backgroundimg = NULL;
static gchar           *gcompris_locale = NULL;
static gchar           *gcompris_user_default_locale = NULL;

/****************************************************************************/
/* Command line params */

/*** gcompris-popttable */
static int popt_fullscreen	  = FALSE;
static int popt_window		  = FALSE;
static int popt_sound		  = FALSE;
static int popt_mute		  = FALSE;
static int popt_cursor		  = FALSE;
static int popt_version		  = FALSE;
static int popt_aalias		  = FALSE;
static int popt_difficulty_filter = FALSE;
static gchar *popt_audio_output   = NULL;

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
  {"audio", 'A', POPT_ARG_STRING, &popt_audio_output, 0,
   N_("select the audio output. Use '-A list' to get the list of available audio outputs"), NULL},
  {"version", 'v', POPT_ARG_NONE, &popt_version, 0,
   N_("Print the version of " PACKAGE), NULL},
  {"antialiased", 'a', POPT_ARG_NONE, &popt_aalias, 0,
   N_("Use the antialiased canvas (slower)."), NULL},
  POPT_AUTOHELP
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


/****************************************************************************/

static gint
board_widget_key_press_callback (GtkWidget   *widget,
				GdkEventKey *event,
				gpointer     client_data)
{

  if(event->state & GDK_CONTROL_MASK && ((event->keyval == GDK_r)
					 || (event->keyval == GDK_R))) {
    printf("Refreshing the canvas\n");
    gnome_canvas_update_now(canvas);
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
      /* Remove any dialog box */
      gcompris_help_stop();
      gcompris_config_stop();
      gcompris_about_stop();

      board_stop();
      return TRUE;
    case GDK_F5:
      printf("Refreshing the canvas\n");
      gnome_canvas_update_now(canvas);
      return TRUE;
    case GDK_KP_Enter:
    case GDK_Return:
      if (get_current_board_plugin()!=NULL && get_current_board_plugin()->ok)
	{
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
    gtk_object_destroy (GTK_OBJECT(backgroundimg));

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
  GtkWidget *vbox;

  yratio=gdk_screen_height()/(float)(BOARDHEIGHT+BARHEIGHT);
  xratio=gdk_screen_width()/(float)BOARDWIDTH;
    printf("The gdk_screen_width()=%f gdk_screen_height()=%f\n",
  	 (double)gdk_screen_width(), (double)gdk_screen_height());
    printf("The xratio=%f yratio=%f\n", xratio, yratio);

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

  printf("Calculated x ratio xratio=%f\n", xratio);
  
  /* Background area if ratio above 1 */
  if(properties->fullscreen)
    {
      /* First, Remove the gnome crash dialog because it locks the user when in full screen */
      signal(SIGSEGV, SIG_DFL);

      gnome_canvas_set_scroll_region (canvas_bg,
				      0, 0,
				      gdk_screen_width(),
				      gdk_screen_height());
      
      gtk_widget_set_usize (GTK_WIDGET(canvas_bg), gdk_screen_width(), gdk_screen_height());
      
      /* Create a black box for the background */
      gnome_canvas_item_new (gnome_canvas_root(canvas_bg),
			     gnome_canvas_rect_get_type (),
			     "x1", (double) 0,
			     "y1", (double) 0,
			     "x2", (double) gdk_screen_width(),
			     "y2", (double) gdk_screen_height(),
			     "fill_color", "black",
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
			     "x", (double) (gdk_screen_width()-
					    BOARDWIDTH*xratio)/2,
			     "y", (double) (gdk_screen_height()-
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
  GtkWidget* frame;

  GdkPixbuf *gcompris_icon_pixbuf;
  GError *error = NULL;

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
      fprintf (stderr, "Failed to load pixbuf file: %s: %s\n",
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
  frame = gtk_frame_new(NULL);

  gtk_window_set_policy (GTK_WINDOW (window), FALSE, FALSE, TRUE);
  gtk_window_set_default_size(GTK_WINDOW(window), 250, 350);
  gtk_window_set_wmclass(GTK_WINDOW(window), "gcompris", "GCompris");

  gtk_widget_realize (window);

  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		      GTK_SIGNAL_FUNC (quit_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (window), "key_press_event",
		      GTK_SIGNAL_FUNC (board_widget_key_press_callback), 0);


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

  /* Load all the menu once */
  gcompris_load_menus();

  /* Get and Run the root menu */
  gcomprisBoardMenu = gcompris_get_board_from_section("/");
  if(!board_check_file(gcomprisBoardMenu)) {
    g_error("Couldn't find the board menu, or plugin execution error");
  } else {
    g_warning("Fine, we got the gcomprisBoardMenu, xml boards parsing went fine");
  }
  /* Run the bar */
  gcompris_bar_start(canvas_bar);

  board_play (gcomprisBoardMenu);

  init_background();
}

void gcompris_end_board()
{
  if (get_current_gcompris_board()->previous_board == NULL)
    /* We are in the upper menu */
    board_play (gcomprisBoardMenu);
  else
    /* Run the previous board */
    board_play (get_current_gcompris_board()->previous_board);
}

void gcompris_exit()
{
  board_stop();
  gtk_main_quit ();
}

static void quit_cb (GtkWidget *widget, gpointer data)
{
  gcompris_exit();
}

/* =====================================================================
 * Process the cleanup of the child (no zombies)
 * =====================================================================*/
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

  printf("gcompris_set_locale %s\n", locale);
  if(gcompris_locale != NULL)
    g_free(gcompris_locale);

  gcompris_locale = g_strdup(setlocale(LC_ALL, locale));

  if(gcompris_locale!=NULL && strcmp(locale, gcompris_locale))
    g_warning("Requested locale '%s' got '%s'", locale, gcompris_locale);

  if(gcompris_locale==NULL)
    g_warning("Failed to set requested locale %s got %s", locale, gcompris_locale);

  /* Override the env locale to what the user requested */
  /* This makes gettext to give us the new locale text  */
  setenv ("LC_ALL", gcompris_get_locale(), TRUE);
  setenv ("LC_MESSAGES", gcompris_get_locale(), TRUE);
  setenv ("LANGUAGE", gcompris_get_locale(), TRUE);
  setenv ("LANG", gcompris_get_locale(), TRUE);

  /* This does update gettext translation uppon next gettext call */
  /* Call for localization startup */
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

}

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

  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* To have some real random behaviour */
  srand (time (NULL));

  signal(SIGINT, gcompris_terminate);

  /* Default difficulty filter value = NONE */
  popt_difficulty_filter = -1;

  gtk_init (&argc, &argv);

  pctx = poptGetContext (PACKAGE, argc, (const char **)argv, options, 0);

  /* Argument parsing */
  popt_option = poptGetNextOpt (pctx);

  load_properties ();

  // Set the default gcompris cursor
  properties->defaultcursor = GCOMPRIS_DEFAULT_CURSOR;

  /* Save the default locale */
  gcompris_user_default_locale = g_strdup(setlocale(LC_ALL, NULL));

  // Set the user's choice locale
  gcompris_set_locale(properties->locale);

  /*------------------------------------------------------------*/
  if (popt_version)
    {
      printf (_("GCompris\nVersion: %d\nLicence: GPL\n"
		"More info at http://ofset.sourceforge.net/gcompris\n"),
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
      g_warning("Display only activities of level %d", popt_difficulty_filter);
      properties->difficulty_filter = popt_difficulty_filter;
    }

  if (popt_audio_output)
    {
      if(!strcmp(popt_audio_output, "list")) {
	
	display_ao_devices();
	exit(0);

      } else {
	g_warning("Selected audio output %s", popt_audio_output);
	if(!strcmp(popt_audio_output, "default"))
	  properties->audio_output = "";
	else
	  properties->audio_output = popt_audio_output;
      }
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
