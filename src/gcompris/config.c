/* gcompris - config.c
 *
 * Time-stamp: <2002/06/30 23:38:33 bruno>
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
 * Configuration of gcompris
 */

#include "gcompris.h"
#include "gcompris_config.h"

static GnomeCanvasItem	*rootitem		= NULL;
static GnomeCanvasItem	*item_locale_text	= NULL;
static GnomeCanvasItem	*item_locale_flag	= NULL;
static GnomeCanvasItem	*item_bad_flag		= NULL;
static GnomeCanvasItem	*item_screen_text	= NULL;
static GnomeCanvasItem	*item_timer_text	= NULL;
static GdkPixbuf	*pixmap_checked		= NULL;
static GdkPixbuf	*pixmap_unchecked	= NULL;

static gchar		*current_locale		= NULL;

#define SOUNDLISTFILE PACKAGE

static gchar *linguas[] = {
  "az", 	N_("Azerbaijani Turkic"),
  "de", 	N_("German"),
  "el", 	N_("Greek"),
  "en",		N_("English"),
  "es", 	N_("Spanish"),
  "fi", 	N_("Finnish"),
  "fr", 	N_("French"),
  "hu",		N_("Hungarian"),
  "it",	 	N_("Italian"),
  "ms",		N_("Malay"),
  "nl",		N_("Dutch"),
  "pt_BR",	N_("Brazil Portuguese"),
  "ru",		N_("Romanian"),
  "sv",		N_("Swedish"),
  "sk",		N_("Slovak"),
  "lt",		N_("Lithuanian"),
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

static void set_locale_flag(gchar *locale);
static gchar *get_locale_name(gchar *locale);
static gchar *get_next_locale(gchar *locale);
static gchar *get_previous_locale(gchar *locale);
static gint item_event_ok(GnomeCanvasItem *item, GdkEvent *event, gpointer data);


/*
 * Do all the bar display and register the events
 */
void gcompris_config_start ()
{
  GcomprisBoard		*gcomprisBoard = get_current_gcompris_board();
  GcomprisProperties	*properties = gcompris_get_properties();
  GdkFont *gdk_font;
  GdkFont *gdk_font_small;
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

  pixmap = gcompris_load_pixmap("gcompris/help_bg.png");
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

  // TITLE
  gdk_font = gdk_font_load (FONT_TITLE);
  if(!gdk_font)
    // Fallback to a more usual font
    gdk_font = gdk_font_load (FONT_TITLE_FALLBACK);

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_text_get_type (),
				"text", _("GCompris Configuration"), 
				"font_gdk", gdk_font,
				"x", (double) BOARDWIDTH/2,
				"y", (double) y_start + 40,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", COLOR_TITLE,
				NULL);

  // OK
  pixmap = gcompris_load_pixmap("gcompris/buttons/button_large.png");
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
				"font_gdk", gdk_font,
				"x", (double)  BOARDWIDTH*0.5,
				"y", (double)  y - gdk_pixbuf_get_height(pixmap) + 20,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", COLOR_TITLE,
				NULL);
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) item_event_ok,
		     "ok");
  gtk_signal_connect(GTK_OBJECT(item2), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     item);
  gdk_pixbuf_unref(pixmap);


  // Configuration content
  gdk_font_small = gdk_font_load (FONT_SUBTITLE);
  if(!gdk_font_small)
    // Fallback to a more usual font
    gdk_font_small = gdk_font_load (FONT_SUBTITLE_FALLBACK);

  pixmap_checked   = gcompris_load_pixmap("gcompris/buttons/button_checked.png");
  pixmap_unchecked = gcompris_load_pixmap("gcompris/buttons/button_unchecked.png");


  x_start += 150;
  x_flag_start = x_start + 50;
  x_text_start = x_start + 120;

  //--------------------------------------------------
  // Locale
  y_start += 140;

  display_previous_next(x_start, y_start, "locale_previous", "locale_next");

  item_locale_flag = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", NULL, 
					    "x", (double) x_flag_start,
					    "y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
					    NULL);

  /* Display a bad icon if this locale is not available */
  pixmap   = gcompris_load_pixmap("gcompris/buttons/mini_bad.png");
  item_bad_flag = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", pixmap,
					    "x", (double) x_flag_start - 20,
					    "y", (double) y_start - gdk_pixbuf_get_width(pixmap_checked)/2,
					    NULL);
  gdk_pixbuf_unref(pixmap);

  current_locale = gcompris_get_locale();
  set_locale_flag(current_locale);

  item_locale_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					    gnome_canvas_text_get_type (),
					    "text", get_locale_name(current_locale), 
					    "font_gdk", gdk_font_small,
					    "x", (double) x_text_start,
					    "y", (double) y_start,
					    "anchor", GTK_ANCHOR_WEST,
					    "fill_color_rgba", COLOR_CONTENT,
					    NULL);

  // Fullscreen / Window
  y_start += 50;

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
			 "font_gdk", gdk_font_small,
			 "x", (double) x_text_start,
			 "y", (double) y_start,
			 "anchor", GTK_ANCHOR_WEST,
			 "fill_color_rgba", COLOR_CONTENT,
			 NULL);

  // Screen size
  y_start += 50;

  display_previous_next(x_start, y_start, "screen_previous", "screen_next");

  item_screen_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					    gnome_canvas_text_get_type (),
					    "text", screenname[properties->screensize], 
					    "font_gdk", gdk_font_small,
					    "x", (double) x_text_start,
					    "y", (double) y_start,
					    "anchor", GTK_ANCHOR_WEST,
					    "fill_color_rgba", COLOR_CONTENT,
					    NULL);

  // Music
  y_start += 50;

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
			 "font_gdk", gdk_font_small,
			 "x", (double) x_text_start,
			 "y", (double) y_start,
			 "anchor", GTK_ANCHOR_WEST,
			 "fill_color_rgba", COLOR_CONTENT,
			 NULL);

  // Effect
  y_start += 50;

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
			 "font_gdk", gdk_font_small,
			 "x", (double) x_text_start,
			 "y", (double) y_start,
			 "anchor", GTK_ANCHOR_WEST,
			 "fill_color_rgba", COLOR_CONTENT,
			 NULL);

  // Timer
  y_start += 50;

  display_previous_next(x_start, y_start, "timer_previous", "timer_next");

  item_timer_text = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
					   gnome_canvas_text_get_type (),
					   "text", timername[properties->timer], 
					   "font_gdk", gdk_font_small,
					   "x", (double) x_text_start,
					   "y", (double) y_start,
					   "anchor", GTK_ANCHOR_WEST,
					   "fill_color_rgba", COLOR_CONTENT,
					   NULL);

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

  gdk_pixbuf_unref(pixmap_unchecked);
  gdk_pixbuf_unref(pixmap_checked);

  /* UnPause the board */
  if(gcomprisBoard->plugin->pause_board != NULL)
    gcomprisBoard->plugin->pause_board(FALSE);

  gcompris_bar_hide(FALSE);
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

  pixmap = gcompris_load_pixmap("gcompris/buttons/button_backward.png");
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


  pixmap = gcompris_load_pixmap("gcompris/buttons/button_forward.png");
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
  char *filename = NULL;
  GdkPixbuf *pixmap = NULL;

  str = g_strdup_printf("flags/%.2s.png", locale);
  filename = g_strdup_printf("%s/%s", PACKAGE_DATA_DIR, str);

  g_warning("Trying to load flag %s", filename);

  if (g_file_exists (filename)) 
    {
      pixmap = gcompris_load_pixmap(str);
    } 

  gnome_canvas_item_set (item_locale_flag,
			 "pixbuf", pixmap,
			 NULL);

  /* Check wether or not the locale is available */
  if(setlocale(LC_ALL, locale)==NULL)
    gnome_canvas_item_show (item_bad_flag);
  else
    gnome_canvas_item_hide (item_bad_flag);

  g_free(str);
  g_free(filename);

  if(pixmap)
    gdk_pixbuf_unref(pixmap);
}


/**
 * Given the short locale name, return the full translated name
 * If not found, simply return the short given locale
 */
static gchar *get_locale_name(gchar *locale)
{
  guint i = 0;

  while(linguas[i] != NULL)
    {
      if(!strncmp(locale, linguas[i], 2))
	return(gettext(linguas[i+1]));

      i=i+2;
    }
  // Oups this locale is not in the table. Return the first one
  return(linguas[0]);
}

/**
 * Given the short locale name, return the next one in our linguas table
 */
static gchar *get_next_locale(gchar *locale)
{
  guint i = 0;

  while(linguas[i] != NULL)
    {
      if(!strncmp(locale, linguas[i], 2))
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
      if(!strncmp(locale, linguas[i], 2))
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

/* Callback for the bar operations */
static gint
item_event_ok(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  GcomprisProperties	*properties = gcompris_get_properties();

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
      if(!strcmp((char *)data, "ok"))
	{
	  gcompris_set_locale(current_locale);
	  gcompris_config_stop();
	}
      else if(!strcmp((char *)data, "fullscreen"))
	{
	  properties->fullscreen = (properties->fullscreen ? 0 : 1);
	  gnome_canvas_item_set (item,
				 "pixbuf", (properties->fullscreen ? pixmap_checked : pixmap_unchecked),
				 NULL);

	}
      else if(!strcmp((char *)data, "music"))
	{
	  properties->music = (properties->music ? 0 : 1);
	  gnome_canvas_item_set (item,
				 "pixbuf", (properties->music ? pixmap_checked : pixmap_unchecked),
				 NULL);

	}
      else if(!strcmp((char *)data, "effect"))
	{
	  properties->fx = (properties->fx ? 0 : 1);
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
				 "text", screenname[properties->screensize], 
				 NULL);
	}
      else if(!strcmp((char *)data, "screen_next"))
	{
	  if(properties->screensize<MAX_SCREEN_VALUE)
	    properties->screensize++;

	  gnome_canvas_item_set (item_screen_text,
				 "text", screenname[properties->screensize], 
				 NULL);
	}
      else if(!strcmp((char *)data, "timer_previous"))
	{
	  if(properties->timer>0)
	    properties->timer--;

	  gnome_canvas_item_set (item_timer_text,
				 "text", timername[properties->timer], 
				 NULL);
	}
      else if(!strcmp((char *)data, "timer_next"))
	{
	  if(properties->timer<MAX_TIMER_VALUE)
	    properties->timer++;

	  gnome_canvas_item_set (item_timer_text,
				 "text", timername[properties->timer], 
				 NULL);
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
