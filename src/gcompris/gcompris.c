/* gcompris - gcompris.c
 *
 * Time-stamp: <2002/01/02 23:46:19 bruno>
 *
 * Copyright (C) 2000,2001 Bruno Coudoin
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

#include "gcompris.h"
#include <popt-gnome.h>
#include <gconf/gconf.h>

GtkWidget *window;
GtkWidget *drawing_area;
GnomeCanvas *canvas;
GnomeCanvas *canvas_bar;
GnomeCanvas *canvas_bg;

//static gint pause_board_cb (GtkWidget *widget, gpointer data);
static void quit_cb (GtkWidget *widget, gpointer data);
static void about_cb (GtkWidget *widget, gpointer data);
static void help_cb (GtkWidget *widget, gpointer data);
static gint end_board_box (void);
static gint board_widget_key_press_callback (GtkWidget   *widget,
					    GdkEventKey *event,
					    gpointer     client_data);


GcomprisProperties	*properties = NULL;
GcomprisBoard		*gcomprisBoardMenu = NULL;

/****************************************************************************/
/* Some constants.  */

static GnomeCanvasItem *backgroundimg = NULL;
static gchar           *gcompris_locale = NULL;

/****************************************************************************/
/* Command line params */

#define P_FULLSCREEN	101
#define P_WINDOW	102
#define P_SOUND		103
#define P_MUTE		104
#define P_VERSION	105

struct poptOption command_line[] = {
  {"fullscreen", 'f', POPT_ARGFLAG_ONEDASH, NULL, P_FULLSCREEN,
   N_("run gcompris in fullscreen mode.")},
  {"window", 'w', POPT_ARGFLAG_ONEDASH, NULL, P_WINDOW,
   N_("run gcompris in window mode.")},
  {"sound", 's', POPT_ARGFLAG_ONEDASH, NULL, P_SOUND,
   N_("run gcompris with sound enabled.")},
  {"mute", 'm', POPT_ARGFLAG_ONEDASH, NULL, P_MUTE,
   N_("run gcompris without sound.")},
  {"version", 'v', POPT_ARGFLAG_ONEDASH, NULL, P_VERSION,
   N_("Prints the version of " PACKAGE)},
  POPT_AUTOHELP {NULL, 0, 0, NULL, 0}
};



/****************************************************************************/

static gint
board_widget_key_press_callback (GtkWidget   *widget,
				GdkEventKey *event,
				gpointer     client_data)
{

  switch (event->keyval)
    {
    case GDK_Escape:
      gcompris_help_stop();
      board_stop();
      //      quit_cb(NULL, NULL);
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
  double xratio, yratio;
  GtkWidget *vbox;

  yratio=gdk_screen_height()/(float)(BOARDHEIGHT+BARHEIGHT);
  xratio=gdk_screen_width()/(float)BOARDWIDTH;
    printf("The gdk_screen_width()=%f gdk_screen_height()=%f\n", 
  	 (double)gdk_screen_width(), (double)gdk_screen_height());
    printf("The xratio=%f yratio=%f\n", xratio, yratio);

  yratio=xratio=MIN(xratio, yratio);

  /* The canvas does not look pretty when resized above 1 ratio. Avoid that */
  xratio=MIN(1.0, xratio);

  printf("Calculated x ratio xratio=%f\n", xratio);
  
  /* Background area if ratio above 1 */
  if(xratio>=1.0 && properties->fullscreen)
    {
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

  if(xratio<1.0 || properties->fullscreen==FALSE)
    gnome_app_set_contents (GNOME_APP (window), GTK_WIDGET(vbox));

  gtk_widget_show (GTK_WIDGET(vbox));
  gtk_widget_show (GTK_WIDGET(canvas));
  gtk_widget_show (GTK_WIDGET(canvas_bar));

  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(canvas), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(canvas_bar), TRUE, TRUE, 0);

  if(xratio>=1.0 && properties->fullscreen)
    {
      gnome_canvas_item_new (gnome_canvas_root(canvas_bg),
			     gnome_canvas_widget_get_type (),
			     "widget", vbox, 
			     "x", (double) (gdk_screen_width()-BOARDWIDTH)/2,
			     "y", (double) (gdk_screen_height()-BOARDHEIGHT-BARHEIGHT)/2,
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

static void setup_window ()
{

  window = gnome_app_new (PACKAGE, _("GCompris I Have Understood"));

  gtk_window_set_policy (GTK_WINDOW (window), FALSE, FALSE, TRUE);

  gtk_widget_realize (window);

  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		      GTK_SIGNAL_FUNC (quit_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (window), "key_press_event",
		      GTK_SIGNAL_FUNC (board_widget_key_press_callback), 0);


  // Full screen
  if(properties->fullscreen)
    gnome_win_hints_set_layer (GTK_WIDGET (window),  WIN_LAYER_ABOVE_DOCK);
  //  gnome_win_hints_set_state(GTK_WIDGET (window),  WIN_STATE_FIXED_POSITION);


  /*  GdkCursor *hand_cursor; */
  /*  hand_cursor = gdk_cursor_new(GDK_HAND2);
      gdk_window_set_cursor	 (window->window, hand_cursor);
      gdk_cursor_destroy(hand_cursor);
  */

  /* -------------- */

  /* For non anti alias canvas */
  gtk_widget_push_visual (gdk_rgb_get_visual ());
  gtk_widget_push_colormap (gdk_rgb_get_cmap ());

  /* For anti alias canvas */
  /*
  gtk_widget_push_visual(gdk_rgb_get_visual());
  gtk_widget_push_colormap(gdk_rgb_get_cmap());
  */

  /* For non anti alias canvas */
  canvas     = GNOME_CANVAS(gnome_canvas_new ());
  canvas_bar = GNOME_CANVAS(gnome_canvas_new ());
  canvas_bg  = GNOME_CANVAS(gnome_canvas_new ());

  /* For anti alias canvas */
  /*
  canvas     = GNOME_CANVAS(gnome_canvas_new_aa ());
  canvas_bar = GNOME_CANVAS(gnome_canvas_new_aa ());
  */


  gnome_app_set_contents (GNOME_APP (window), GTK_WIDGET(canvas_bg));

  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();


  gtk_widget_show (GTK_WIDGET(canvas_bg));

  if(properties->fullscreen)
    {
      gdk_window_set_decorations (window->window, 0);
      gdk_window_set_functions (window->window, 0);
      gtk_widget_set_uposition (window, 0, 0);  
    }

  init_plugins();

  /* Load and Run the menu */
  gcomprisBoardMenu = gcompris_read_xml_file(PACKAGE_DATA_DIR "/menu.xml");
  if(!board_check_file(gcomprisBoardMenu))
    g_error("Cant't find the menu board or plugin execution error");

  /* Run the bar */
  gcompris_bar_start(canvas_bar);

  board_play (gcomprisBoardMenu);

  init_background();
}

/* It implements gcompris's own way to load help in order to be
 * Package relocatable.
 * Unfortunatly, this does not supports I18N
 */
static void help_cb (GtkWidget *widget, gpointer data)
{
  gnome_help_goto(NULL, PACKAGE_HELP_DIR "/C/gcompris.html");
}

static void about_cb (GtkWidget *widget, gpointer data)
{
  static GtkWidget *about;
  GtkWidget *l, *hbox;


  const gchar *authors[] = {
    "Bruno Coudoin <bruno.coudoin@free.fr>",
    NULL
  };

  if (about != NULL) {
    gdk_window_raise (about->window);
    gdk_window_show (about->window);
    return;
  }

  about = gnome_about_new ("GCompris", VERSION,
                        ("Copyright 2000,2001 Bruno Coudoin (Released under the GPL)"),
                        authors,
                        _("A simple educational board based game for children starting at 3. This software is a GNU Package"),
                        PACKAGE_DATA_DIR "/gcompris/gcompris-about.jpg");

  hbox = gtk_hbox_new (TRUE, 0);
  l = gnome_href_new ("http://www.ofset.org/gcompris",
		      _ ("GCompris Home Page"));
  gtk_box_pack_start (GTK_BOX (hbox), l, FALSE, FALSE, 0);
  l = gnome_href_new ("http://www.ofset.org",
		      _ ("An OFSET development"));
  gtk_box_pack_start (GTK_BOX (hbox), l, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (about)->vbox),
		      hbox, TRUE, FALSE, 0);
  gtk_widget_show_all (hbox);

  gtk_object_set_data (GTK_OBJECT (about), "about", about);
  gtk_window_set_modal (GTK_WINDOW (about), TRUE);

  gtk_signal_connect (GTK_OBJECT (about), "destroy", GTK_SIGNAL_FUNC
		      (gtk_widget_destroyed), &about);
  gnome_dialog_set_parent (GNOME_DIALOG (about), GTK_WINDOW (window));
  
  gtk_widget_show (about);
}

/*
 * Confirmation dialog
 */
static gint end_board_box ()
{
  static GtkWidget *box;
  gint status;

  if (box)
    return 0;

  board_pause();

  box = gnome_message_box_new (
			       _("Do you really want to quit GCompris?"),
			       GNOME_MESSAGE_BOX_QUESTION,
			       GNOME_STOCK_BUTTON_YES, GNOME_STOCK_BUTTON_NO,
			       NULL);
  gnome_dialog_set_parent (GNOME_DIALOG (box), GTK_WINDOW
			   (window));
  gnome_dialog_set_default (GNOME_DIALOG (box), 0);
  status = gnome_dialog_run (GNOME_DIALOG (box));
  box = NULL;
  
  board_pause();

  return (status);
}

/*
static gint pause_board_cb (GtkWidget *widget, gpointer data)
{
  board_pause();
  return(TRUE);
}
*/

void gcompris_end_board()
{
  if (get_current_gcompris_board()->previous_board == NULL)
    {
      /* We are in the upper menu: leave GCompris? */
      quit_cb(NULL, NULL);

      /* Oups, the user changed his mind : restart the menu */
      board_play (get_current_gcompris_board());
      return;
    }

  /* Run the previous board */
  board_play (get_current_gcompris_board()->previous_board);
}

static void quit_cb (GtkWidget *widget, gpointer data)
{
  // FIXME: Should be implemented as a canvas dialog window
  //  if (end_board_box ())
  //    return;

  /*      cleanup_plugins(); */
  gcompris_properties_save(properties);

  gtk_main_quit ();

}

static void load_properties ()
{
  properties = gcompris_properties_new ();
}

/*
 * This returns the locale for which text must be displayed
 *
 */
gchar *gcompris_get_locale()
{

  /* First check gcompris got overrided by the user */
  if(gcompris_locale !=NULL)
    return(gcompris_locale);

  if(!strcmp(setlocale(0, NULL), "C"))
    {
      /* NO Locale, Try to use the English Locale */
      return("en");
    } 
  else 
    {
      return(setlocale(0, NULL));
    }
}

/*
 * This set the locale for which text must be displayed
 *
 */
void gcompris_set_locale(gchar *locale)
{

  gcompris_locale = setlocale(LC_ALL, locale);
  printf("gcompris_set_locale requested %s got %s\n", locale, gcompris_locale);
  /* WARNING: This does not update gettext translation */
}

/*****************************************
 * Main
 *
 */

int
main (int argc, char *argv[])
{
  int c;
  poptContext optCon;
  GError *gconf_error = NULL;

  srand (time (NULL));

  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);

  gnome_init_with_popt_table (PACKAGE, VERSION, argc, argv, command_line, 0, &optCon);

  /* Init gconf for gtkhtml */
  gconf_init        (argc, argv, &gconf_error);
  if (gconf_error)
    g_error ("gconf error: %s\n", gconf_error->message);
  
  optCon = poptGetContext (NULL, argc, argv, command_line, 0);

  load_properties ();

  // Set the user's choice locale
  gcompris_set_locale(properties->locale);

  /*------------------------------------------------------------*/
  while ((c = poptGetNextOpt (optCon)) != -1)
    {
      switch (c)
	{
	case P_VERSION:
	  printf (_("GCompris\nVersion: %s\nLicence: GPL\n"
		    "More infos on http://ofset.sourceforge.net/gcompris\n"),
		  VERSION);
	  exit (0);
	  break;
	  
	case P_FULLSCREEN:
	  properties->fullscreen = TRUE;
	  break;

	case P_WINDOW:
	  properties->fullscreen = FALSE;
	  break;

	case P_MUTE:
	  g_warning("Sound disabled");
	  properties->music = FALSE;
	  properties->fx = FALSE;
	  break;

	case P_SOUND:
	  g_warning("Sound enabled");
	  properties->music = TRUE;
	  properties->fx = TRUE;
	  break;
	}
    }

  if (c < -1)
    {
      /* an error occurred during option processing */
      fprintf (stderr, "%s: %s\n",
	       poptBadOption (optCon, POPT_BADOPTION_NOALIAS),
	       poptStrerror (c));
      return (-1);
    }

  /*------------------------------------------------------------*/


  gnome_sound_init(NULL);

  /* Gdk-Pixbuf */
  gdk_rgb_init();

  setup_window ();

  gtk_widget_show (window);

  if (properties->music)
    {
      gcompris_play_ogg("../intro", "welcome", NULL);
    }
  else
    gcompris_play_ogg("welcome", NULL);

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
