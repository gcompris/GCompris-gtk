/* gcompris - config.c
 *
 * Time-stamp: <2005/03/11 21:46:17 bruno>
 *
 * Copyright (C) 2000-2003 Bruno Coudoin
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
 * Configuration of gcompris
 */

#include "gcompris.h"
#include "gcompris_config.h"
#include "locale.h"

#include <dirent.h>

#if defined _WIN32 || defined __WIN32__
# undef WIN32   /* avoid warning on mingw32 */
# define WIN32
#endif

static GnomeCanvasItem	*rootitem		= NULL;
static GnomeCanvasItem	*item_locale_text	= NULL;
static GnomeCanvasItem	*item_locale_flag	= NULL;
static GnomeCanvasItem	*item_bad_flag		= NULL;
static GnomeCanvasItem	*item_screen_text	= NULL;
static GnomeCanvasItem	*item_timer_text	= NULL;
static GnomeCanvasItem	*item_skin_text		= NULL;
static GnomeCanvasItem	*item_filter_text	= NULL;
static GdkPixbuf	*pixmap_checked		= NULL;
static GdkPixbuf	*pixmap_unchecked	= NULL;

static gchar		*current_locale		= NULL;
static GList		*skinlist		= NULL;
static guint		skin_index;

static GnomeCanvasGroup	*stars_group		= NULL;
static double           stars_group_x;
static double           stars_group_y;

#define Y_GAP	45

static gboolean is_displayed			= FALSE;

#define SOUNDLISTFILE PACKAGE

static gchar *linguas[] = {
  "",			N_("Your system default"),
  "am_ET.UTF-8", 	N_("Amharic"),
  "ar_AE.UTF-8", 	N_("Arabic"),
  "az_AZ.UTF-8", 	N_("Azerbaijani Turkic"),
  "bg_BG.UTF-8",	N_("Bulgarian"),
  "ca_ES.UTF-8",	N_("Catalan"),
  "cs_CZ.UTF-8",	N_("Czech"),
  "da_DK.UTF-8", 	N_("Danish"),
  "de_DE.UTF-8", 	N_("German"),
  "el_GR.UTF-8", 	N_("Greek"),
  "en_CA.UTF-8",	N_("English (Canadian)"),
  "en_GB.UTF-8",	N_("English (Great Britain)"),
  "en_US.UTF-8",	N_("English (USA)"),
  "es_ES.UTF-8", 	N_("Spanish"),
  "fi_FI.UTF-8", 	N_("Finnish"),
  "fr_FR.UTF-8", 	N_("French"),
  "ga_IE.UTF-8", 	N_("Irish (Gaelic)"),
  "he_IL.UTF-8",	N_("Hebrew"),
  "hi_IN.UTF-8",	N_("Indian (Hindi)"),
  "gu_IN.UTF-8",	N_("Indian (Gujarati)"),
  "pa_IN.UTF-8",	N_("Indian (Punjabi)"),
  "hu_HU.UTF-8",	N_("Hungarian"),
  "hr_HR.UTF-8",	N_("Croatian"),
  "it_IT.UTF-8", 	N_("Italian"),
  "lt_LT.UTF-8",	N_("Lithuanian"),
  "mk_MK.UTF-8",	N_("Macedonian"),
  "ml_IN.UTF-8",	N_("Malayalam"),
  "ms_MY.UTF-8",	N_("Malay"),
  "nl_NL.UTF-8",	N_("Dutch"),
  "nb_NO.UTF-8",	N_("Norwegian Bokmal"),
  "nn_NO.UTF-8",	N_("Norwegian Nynorsk"),
  "pl_PL.UTF-8",	N_("Polish"),
  "pt_PT.UTF-8",	N_("Portuguese"),
  "pt_BR.UTF-8",	N_("Brazil Portuguese"),
  "ro_RO.UTF-8",	N_("Romanian"),
  "ru_RU.UTF-8",	N_("Russian"),
  "sk_SK.UTF-8",	N_("Slovak"),
  "sl_SI.UTF-8",	N_("Slovenian"),
  "sq_AL.UTF-8",	N_("Albanian"),
  "sr@Latn_YU.ISO-8859-2",	N_("Serbian (Latin)"),
  "sr_YU.UTF-8",	N_("Serbian"),
  "sv_FI.UTF-8",	N_("Swedish"),
  "tr_TR.UTF-8",	N_("Turkish"),
  "wa_BE.UTF-8",	N_("Walloon"),
  "zh_CN.UTF-8",	N_("Chinese (Simplified)"),
  NULL, NULL
};

static gchar *timername[] = {
  N_("No time limit"),
  N_("Slow timer"),
  N_("Normal timer"),
  N_("Fast timer")
};

static gchar *screenname[] = {
  "640x480",
  N_("800x600 (Default for gcompris)"),
  "1024x768",
};

static gchar *filtername[] = {
  N_("No filter"),
  N_("Only this level"),
  N_("Up to this level"),
  N_("This level and above")
};

static void set_locale_flag(gchar *locale);
static gchar *get_locale_name(gchar *locale);
static gchar *get_next_locale(gchar *locale);
static gchar *get_previous_locale(gchar *locale);
static void   display_difficulty_level();
static gint item_event_ok(GnomeCanvasItem *item, GdkEvent *event, gpointer data);


/*
 * Do all the bar display and register the events
 */
void gcompris_config_start ()
{
  GcomprisBoard		*gcomprisBoard = get_current_gcompris_board();
  GcomprisProperties	*properties = gcompris_get_properties();
  GdkPixbuf   *pixmap = NULL;
  gint y_start = 0;
  gint x_start = 0;
  gint x_text_start = 0;
  gint x_flag_start = 0;
  gint y = 0;
  GnomeCanvasItem *item, *item2;

  /* Pause the board */
  if(gcomprisBoard->plugin->pause_board != NULL)
    gcomprisBoard->plugin->pause_board(TRUE);

  if(rootitem)
    return;

  gcompris_bar_hide(TRUE);

  rootitem = \
    gnome_canvas_item_new (gnome_canvas_root(gcompris_get_canvas()),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  pixmap = gcompris_load_skin_pixmap("help_bg.png");
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

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("GCompris Configuration"),
				"font", gcompris_skin_font_title,
				"x", (double) BOARDWIDTH/2,
				"y", (double) y_start + 40,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_title,
				NULL);

  // OK
  pixmap = gcompris_load_skin_pixmap("button_large.png");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH*0.5) - gdk_pixbuf_get_width(pixmap)/2,
				"y", (double) y - gdk_pixbuf_get_height(pixmap) - 5,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "ok");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  item2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("OK"),
				"font", gcompris_skin_font_title,
				"x", (double)  BOARDWIDTH*0.5,
				"y", (double)  y - gdk_pixbuf_get_height(pixmap) + 20,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", gcompris_skin_color_text_button,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) item_event_ok,
		     "ok");
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     item);
  gdk_pixbuf_unref(pixmap);


  pixmap_checked   = gcompris_load_skin_pixmap("button_checked.png");
  pixmap_unchecked = gcompris_load_skin_pixmap("button_unchecked.png");


  x_start += 150;
  x_flag_start = x_start + 50;
  x_text_start = x_start + 115;

  //--------------------------------------------------
  // Locale
  y_start += 105;

  display_previous_next(x_start, y_start, "locale_previous", "locale_next");

  item_locale_flag = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", NULL, 
					    "x", (double) x_flag_start,
					    "y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
					    NULL);

  /* Display a bad icon if this locale is not available */
  pixmap   = gcompris_load_skin_pixmap("mini_bad.png");
  item_bad_flag = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", pixmap,
					    "x", (double) x_flag_start - 20,
					    "y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
					    NULL);
  gdk_pixbuf_unref(pixmap);

  /*
   * The current locale is the one found in the config file
   */
  current_locale = properties->locale;
  set_locale_flag(current_locale);
 
  item_locale_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					    gnome_canvas_text_get_type (),
					    "text", get_locale_name(current_locale), 
					    "font", gcompris_skin_font_subtitle,
					    "x", (double) x_text_start,
					    "y", (double) y_start,
					    "anchor", GTK_ANCHOR_WEST,
					    "fill_color_rgba", gcompris_skin_color_content,
					    NULL);

  // Fullscreen / Window
  y_start += Y_GAP;

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", (properties->fullscreen ? pixmap_checked : pixmap_unchecked), 
				"x", (double) x_start,
				"y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "fullscreen");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);


  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", _("Fullscreen"), 
			 "font", gcompris_skin_font_subtitle,
			 "x", (double) x_text_start,
			 "y", (double) y_start,
			 "anchor", GTK_ANCHOR_WEST,
			 "fill_color_rgba", gcompris_skin_color_content,
			 NULL);

  // Screen size
  y_start += Y_GAP;

  display_previous_next(x_start, y_start, "screen_previous", "screen_next");

  item_screen_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					    gnome_canvas_text_get_type (),
					    "text", gettext(screenname[properties->screensize]), 
					    "font", gcompris_skin_font_subtitle,
					    "x", (double) x_text_start,
					    "y", (double) y_start,
					    "anchor", GTK_ANCHOR_WEST,
					    "fill_color_rgba", gcompris_skin_color_content,
					    NULL);

  // Music
  y_start += Y_GAP;

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", (properties->music ? pixmap_checked : pixmap_unchecked), 
				"x", (double) x_start,
				"y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "music");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);


  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", _("Music"), 
			 "font", gcompris_skin_font_subtitle,
			 "x", (double) x_text_start,
			 "y", (double) y_start,
			 "anchor", GTK_ANCHOR_WEST,
			 "fill_color_rgba", gcompris_skin_color_content,
			 NULL);

  // Effect
  y_start += Y_GAP;

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", (properties->fx ? pixmap_checked : pixmap_unchecked), 
				"x", (double) x_start,
				"y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     "effect");
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);


  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", _("Effect"), 
			 "font", gcompris_skin_font_subtitle,
			 "x", (double) x_text_start,
			 "y", (double) y_start,
			 "anchor", GTK_ANCHOR_WEST,
			 "fill_color_rgba", gcompris_skin_color_content,
			 NULL);

  // Timer
  y_start += Y_GAP;

  display_previous_next(x_start, y_start, "timer_previous", "timer_next");

  item_timer_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					   gnome_canvas_text_get_type (),
					   "text", gettext(timername[properties->timer]),
					   "font", gcompris_skin_font_subtitle,
					   "x", (double) x_text_start,
					   "y", (double) y_start,
					   "anchor", GTK_ANCHOR_WEST,
					   "fill_color_rgba", gcompris_skin_color_content,
					   NULL);

  // Skin
  {
    struct dirent *one_dirent;
    guint i;
    char *str;
    DIR *dir;
    
    /* Load the Pixpmaps directory file names */
    dir = opendir(PACKAGE_DATA_DIR"/skins");

    if (!dir)
      g_error (_("Couldn't open skin dir: %s"), PACKAGE_DATA_DIR"/skins");
    
    /* Fill up the skin list */
    while((one_dirent = readdir(dir)) != NULL) {

      if (one_dirent->d_name[0] != '.') {
	gchar *filename;
	/* Only directory here are skins */
	filename = g_strdup_printf("%s/skins/%s", PACKAGE_DATA_DIR, one_dirent->d_name);

	if (g_file_test ((filename), G_FILE_TEST_IS_DIR)) {
	  gchar *skin_name = g_strdup_printf("%s", one_dirent->d_name);
	  skinlist = g_list_append (skinlist, skin_name);
	}
	g_free(filename);
      }
    }
    closedir(dir);

    /* Should not happen. It the user found the config, there should be a skin */
    if(g_list_length(skinlist) == 0) {
      g_warning( _("No skin found in %s\n"), PACKAGE_DATA_DIR"/skins");
    }

    /* Find the current skin index */
    skin_index = 0;
    for(i=0; i<g_list_length(skinlist);  i++)
      if(!strcmp((char *)g_list_nth_data(skinlist, i), properties->skin))
	skin_index = i;

    y_start += Y_GAP;

    display_previous_next(x_start, y_start, "skin_previous", "skin_next");

    item_skin_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					    gnome_canvas_text_get_type (),
					    "text", g_strdup_printf(_("Skin : %s"), 
								    (char *)g_list_nth_data(skinlist, skin_index)),
					    "font", gcompris_skin_font_subtitle,
					    "x", (double) x_text_start,
					    "y", (double) y_start,
					    "anchor", GTK_ANCHOR_WEST,
					    "fill_color_rgba", gcompris_skin_color_content,
					    NULL);
  }

  // Difficulty Filter
  y_start += Y_GAP;

  display_previous_next(x_start, y_start, "filter_style_previous", "filter_style_next");

  stars_group_x = x_start + 45;
  stars_group_y = y_start - 25;

  item_filter_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
						gnome_canvas_text_get_type (),
						"text", gettext(filtername[properties->filter_style]), 
						"font", gcompris_skin_font_subtitle,
						"x", (double) x_text_start,
						"y", (double) y_start,
						"anchor", GTK_ANCHOR_WEST,
						"fill_color_rgba", gcompris_skin_color_content,
						NULL);


  // Difficulty star action button
  display_difficulty_level();

  is_displayed = TRUE;
}

void gcompris_config_stop ()
{
  GcomprisBoard *gcomprisBoard = get_current_gcompris_board();

  // Destroy the help box
  if(rootitem!=NULL)
    {
      gtk_object_destroy(GTK_OBJECT(rootitem));
      gcomprisBoard->plugin->pause_board(FALSE);
    }
  rootitem = NULL;	  

  stars_group = NULL;

  if(pixmap_unchecked)
    gdk_pixbuf_unref(pixmap_unchecked);
  pixmap_unchecked = NULL;

  if(pixmap_checked)
    gdk_pixbuf_unref(pixmap_checked);
  pixmap_checked = NULL;

  /* UnPause the board */
  if(gcomprisBoard->plugin->pause_board != NULL && is_displayed)
    {
      gcomprisBoard->plugin->pause_board(FALSE);
    }

  gcompris_bar_hide(FALSE);

  is_displayed = FALSE;
}


/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

display_previous_next(guint x_start, guint y_start, 
		      gchar *eventname_previous, gchar *eventname_next)
{
  GdkPixbuf   *pixmap = NULL;
  GnomeCanvasItem *item;

  pixmap = gcompris_load_skin_pixmap("button_backward.png");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) x_start - gdk_pixbuf_get_width(pixmap) - 10,
				"y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     eventname_previous);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
  gdk_pixbuf_unref(pixmap);


  pixmap = gcompris_load_skin_pixmap("button_forward.png");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) x_start,
				"y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
				NULL);

  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event_ok,
		     eventname_next);
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
  gdk_pixbuf_unref(pixmap);
}
  
static void set_locale_flag(gchar *locale)
{
  char *str = NULL;
  GdkPixbuf *pixmap = NULL;

  if(locale == NULL)
    return;

  if(locale[0] == '\0') {
    /* Set the flag to the default user's locale */
    locale = gcompris_get_user_default_locale();
    printf("gcompris_get_user_default_locale = %s\n", locale);
  }

  /* First try to find a flag for the long locale name */
  str = g_strdup_printf("%.5s.png", locale);
  pixmap = gcompris_load_pixmap_asset("gcompris flags", "flags", "image/png", str);

  /* Not found, Try now with the short locale name */
  if(!pixmap) {
    g_free(str);
    str = g_strdup_printf("%.2s.png", locale);
    pixmap = gcompris_load_pixmap_asset("gcompris flags", "flags", "image/png", str);
  }

  gnome_canvas_item_set (item_locale_flag,
			 "pixbuf", pixmap,
			 NULL);

  /* Check wether or not the locale is available */
#ifdef WIN32
  /* On win32, it's always available, do not try to check */
  gnome_canvas_item_hide (item_bad_flag);
#else
  if(setlocale(LC_MESSAGES, locale)==NULL)
    gnome_canvas_item_show (item_bad_flag);
  else
    gnome_canvas_item_hide (item_bad_flag);
#endif

  g_free(str);

  if(pixmap)
    gdk_pixbuf_unref(pixmap);
}


/**
 * Given the locale name, return the full translated name
 * If not found, simply return the name
 */
static gchar *get_locale_name(gchar *locale)
{
  guint i = 0;

  while(linguas[i] != NULL)
    {

      if(!strncmp(locale, linguas[i], strlen(locale)))
	return(gettext(linguas[i+1]));

      i=i+2;
    }
  // Oups this locale is not in the table. Return the first one (system default)
  return(linguas[1]);
}

/**
 * Given the short locale name, return the next one in our linguas table
 */
static gchar *get_next_locale(gchar *locale)
{
  guint i = 0;

  while(linguas[i] != NULL)
    {
      if(!strcmp(locale, linguas[i]))
	{
	  // Found it
	  if(linguas[i+2]!=NULL)
	    return(linguas[i+2]);
	  else
	    return(linguas[0]);
	  
	}
      i=i+2;
    }
  // Oups this locale is not in the table. Return the first one
  return(linguas[0]);
}

/**
 * Given the short locale name, return the previous one in our linguas table
 */
static gchar *get_previous_locale(gchar *locale)
{
  guint i = 0;

  while(linguas[i] != NULL)
    {
      if(!strcmp(locale, linguas[i]))
	{
	  // Found it
	  if(i!=0)
	    return(linguas[i-2]);
	  else
	    {
	      // Go to the end of the list
	      while(linguas[i]!=NULL) { i=i+2; };
	      return(linguas[i-2]);
	    }	  
	}
      i=i+2;
    }
  return(locale);
}

static void display_difficulty_level() 
{
  GcomprisProperties	*properties = gcompris_get_properties();

  if(properties->filter_style != GCOMPRIS_FILTER_NONE) {
    if(properties->difficulty_filter == 0)	/* If it was the devel mode, exit it */
      properties->difficulty_filter = 1;

    if(stars_group)
      gtk_object_destroy(GTK_OBJECT(stars_group));
	    
    stars_group = gcompris_display_difficulty_stars(GNOME_CANVAS_GROUP(rootitem),
						    (double) stars_group_x,
						    (double) stars_group_y,
						    (double) 1,
						    properties->difficulty_filter);
    gtk_signal_connect(GTK_OBJECT(stars_group), "event",
		       (GtkSignalFunc) item_event_ok,
		       "difficulty_next");
  } else {
    if(stars_group)
      gnome_canvas_item_hide(stars_group);
  }
}

/* Callback for the bar operations */
static gint
item_event_ok(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  GcomprisProperties	*properties = gcompris_get_properties();

  if(data==NULL)
    return;

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
      if(!strcmp((char *)data, "ok"))
	{
	  if(current_locale[0] == '\0') {
	    /* Set the locale to the default user's locale */
	    gcompris_set_locale(gcompris_get_user_default_locale());
	  } else {
	    gcompris_set_locale(current_locale);
	  }
	  properties->skin = g_strdup((char *)g_list_nth_data(skinlist, skin_index));
	  gcompris_skin_load(properties->skin);
	  gcompris_config_stop();
	  gcompris_properties_save(properties);
	  gcompris_load_menus();

	  if(properties->music || properties->fx) {
	    initSound();
	  }
	}
      else if(!strcmp((char *)data, "fullscreen"))
	{
	  properties->fullscreen = (properties->fullscreen ? 0 : 1);
	  /* Warning changing the image needs to update pixbuf_ref for the focus usage */
	  g_object_set_data (G_OBJECT (item), "pixbuf_ref",  
			     (properties->fullscreen ? pixmap_checked : pixmap_unchecked));
	  gnome_canvas_item_set (item,
				 "pixbuf", (properties->fullscreen ? pixmap_checked : pixmap_unchecked),
				 NULL);

	}
      else if(!strcmp((char *)data, "music"))
	{
	  properties->music = (properties->music ? 0 : 1);
	  /* Warning changing the image needs to update pixbuf_ref for the focus usage */
	  g_object_set_data (G_OBJECT (item), "pixbuf_ref",  
			     (properties->music ? pixmap_checked : pixmap_unchecked));
	  gnome_canvas_item_set (item,
				 "pixbuf", (properties->music ? pixmap_checked : pixmap_unchecked),
				 NULL);

	}
      else if(!strcmp((char *)data, "effect"))
	{
	  properties->fx = (properties->fx ? 0 : 1);
	  /* Warning changing the image needs to update pixbuf_ref for the focus usage */
	  g_object_set_data (G_OBJECT (item), "pixbuf_ref",  
			     (properties->fx ? pixmap_checked : pixmap_unchecked));
	  gnome_canvas_item_set (item,
				 "pixbuf", (properties->fx ? pixmap_checked : pixmap_unchecked),
				 NULL);


	}
      else if(!strcmp((char *)data, "locale_previous"))
	{
	  current_locale = get_previous_locale(current_locale);
	  gnome_canvas_item_set (item_locale_text,
				 "text", get_locale_name(current_locale),
				 NULL);
	  set_locale_flag(current_locale);
	  properties->locale = current_locale;
	}
      else if(!strcmp((char *)data, "locale_next"))
	{
	  current_locale = get_next_locale(current_locale);
	  gnome_canvas_item_set (item_locale_text,
				 "text", get_locale_name(current_locale),
				 NULL);

	  set_locale_flag(current_locale);
	  properties->locale = current_locale;
	}
      else if(!strcmp((char *)data, "screen_previous"))
	{
	  if(properties->screensize>0)
	    properties->screensize--;

	  gnome_canvas_item_set (item_screen_text,
				 "text", gettext(screenname[properties->screensize]), 
				 NULL);
	}
      else if(!strcmp((char *)data, "screen_next"))
	{
	  if(properties->screensize<MAX_SCREEN_VALUE)
	    properties->screensize++;

	  gnome_canvas_item_set (item_screen_text,
				 "text", gettext(screenname[properties->screensize]), 
				 NULL);
	}
      else if(!strcmp((char *)data, "timer_previous"))
	{
	  if(properties->timer>0)
	    properties->timer--;

	  gnome_canvas_item_set (item_timer_text,
				 "text", gettext(timername[properties->timer]), 
				 NULL);
	}
      else if(!strcmp((char *)data, "timer_next"))
	{
	  if(properties->timer<MAX_TIMER_VALUE)
	    properties->timer++;

	  gnome_canvas_item_set (item_timer_text,
				 "text", gettext(timername[properties->timer]), 
				 NULL);
	}
      else if(!strcmp((char *)data, "skin_previous"))
	{
	  if(skin_index-- < 1)
	    skin_index = g_list_length(skinlist)-1;

	  gnome_canvas_item_set (item_skin_text,
				 "text", g_strdup_printf(_("Skin : %s"), 
							 (char *)g_list_nth_data(skinlist, skin_index)),
				 NULL);

	}
      else if(!strcmp((char *)data, "skin_next"))
	{
	  if(skin_index++ >= g_list_length(skinlist)-1)
	    skin_index = 0;

	  gnome_canvas_item_set (item_skin_text,
				 "text", g_strdup_printf(_("Skin : %s"), 
							 (char *)g_list_nth_data(skinlist, skin_index)),
				 NULL);
	}
      else if(!strcmp((char *)data, "difficulty_next"))
	{
	  if(properties->difficulty_filter++ >= properties->difficulty_max)
	    properties->difficulty_filter = 1;

	  display_difficulty_level();
	}
      else if(!strcmp((char *)data, "filter_style_previous"))
	{
	  if(properties->filter_style-- < 1)
	    properties->filter_style = GCOMPRIS_FILTER_ABOVE;

	  gnome_canvas_item_set (item_filter_text,
				 "text", gettext(filtername[properties->filter_style]), 
				 NULL);
	  display_difficulty_level();
	}
      else if(!strcmp((char *)data, "filter_style_next"))
	{
	  if(properties->filter_style++ >= GCOMPRIS_FILTER_ABOVE)
	    properties->filter_style = 0;

	  gnome_canvas_item_set (item_filter_text,
				 "text", gettext(filtername[properties->filter_style]), 
				 NULL);
	  display_difficulty_level();
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
