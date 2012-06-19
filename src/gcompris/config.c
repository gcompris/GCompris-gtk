/* gcompris - config.c
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
 * Configuration of gcompris
 */

#include <string.h>

#include "gcompris.h"
#include "gc_core.h"
#include "gcompris_config.h"
#include "locale.h"

static GooCanvasItem	*rootitem		= NULL;
static GooCanvasItem	*item_locale_text	= NULL;
static GooCanvasItem	*item_bad_flag		= NULL;
static GooCanvasItem	*item_timer_text	= NULL;
static GooCanvasItem	*item_skin_text		= NULL;
static GooCanvasItem	*item_filter_text	= NULL;
static gchar		*pixmap_checked		= NULL;
static gchar		*pixmap_unchecked	= NULL;
static guint		 pixmap_width;

static gchar		*current_locale		= NULL;
static GList		*skinlist		= NULL;
static guint		skin_index;

static GooCanvasGroup	*stars_group		= NULL;
static double           stars_group_x;
static double           stars_group_y;

#define Y_GAP	45

static gboolean is_displayed			= FALSE;
static gint x_flag_start;
static gint y_flag_start;


#define SOUNDLISTFILE PACKAGE

static gchar *linguas[] = {
  "",			N_("Your system default"),
  "af_ZA.UTF-8",	N_("Afrikaans"),
  "sq_AL.UTF-8",	N_("Albanian"),
  "am_ET.UTF-8",	N_("Amharic"),
  "ar_AE.UTF-8",	N_("Arabic"),
  "ast_ES.UTF-8",	N_("Asturian"),
  "eu_ES.UTF-8",	N_("Basque"),
  "br_FR.UTF-8",	N_("Breton"),
  "bg_BG.UTF-8",	N_("Bulgarian"),
  "ca_ES.UTF-8",	N_("Catalan"),
  "zh_CN.UTF-8",	N_("Chinese (Simplified)"),
  "zh_TW.UTF-8",	N_("Chinese (Traditional)"),
  "hr_HR.UTF-8",	N_("Croatian"),
  "cs_CZ.UTF-8",	N_("Czech Republic"),
  "da_DK.UTF-8",	N_("Danish"),
  "nl_NL.UTF-8",	N_("Dutch"),
  "dz_BT.UTF-8",	N_("Dzongkha"),
  "en_CA.UTF-8",	N_("English (Canada)"),
  "en_GB.UTF-8",	N_("English (Great Britain)"),
  "en_US.UTF-8",	N_("English (United States)"),
  "fi_FI.UTF-8",	N_("Finnish"),
  "fr_FR.UTF-8",	N_("French"),
  "ka_GE.UTF-8",	N_("Georgian"),
  "de_DE.UTF-8",	N_("German"),
  "el_GR.UTF-8",	N_("Greek"),
  "gu_IN.UTF-8",	N_("Gujarati"),
  "he_IL.UTF-8",	N_("Hebrew"),
  "hi_IN.UTF-8",	N_("Hindi"),
  "hu_HU.UTF-8",	N_("Hungarian"),
  "id_ID.UTF-8",	N_("Indonesian"),
  "ga_IE.UTF-8",	N_("Irish (Gaelic)"),
  "it_IT.UTF-8",	N_("Italian"),
  "ja_JP.UTF-8",	N_("Japanese"),
  "rw_RW.UTF-8",	N_("Kinyarwanda"),
  "ko_KR.UTF-8",	N_("Korean"),
  "lv_LV.UTF-8",	N_("Latvian"),
  "lt_LT.UTF-8",	N_("Lithuanian"),
  "mk_MK.UTF-8",	N_("Macedonian"),
  "ms_MY.UTF-8",	N_("Malay"),
  "ml_IN.UTF-8",	N_("Malayalam"),
  "mr_IN.UTF-8",	N_("Marathi"),
  "sr_ME.UTF-8",	N_("Montenegrin"),
  "ne_NP.UTF-8",	N_("Nepal"),
  "nb_NO.UTF-8",	N_("Norwegian Bokmal"),
  "nn_NO.UTF-8",	N_("Norwegian Nynorsk"),
  "oc_FR.UTF-8",	N_("Occitan (languedocien)"),
  "fa_IR.UTF-8",	N_("Persian"),
  "pl_PL.UTF-8",	N_("Polish"),
  "pt_BR.UTF-8",	N_("Portuguese (Brazil)"),
  "pt_PT.UTF-8",	N_("Portuguese"),
  "pa_IN.UTF-8",	N_("Punjabi"),
  "ro_RO.UTF-8",	N_("Romanian"),
  "ru_RU.UTF-8",	N_("Russian"),
  "sr@latin_RS.UTF-8",	N_("Serbian (Latin)"),
  "sr_RS.UTF-8",	N_("Serbian"),
  "sk_SK.UTF-8",	N_("Slovak"),
  "sl_SI.UTF-8",	N_("Slovenian"),
  "so_SO.UTF-8",	N_("Somali"),
  "es_ES.UTF-8",	N_("Spanish"),
  "sv_FI.UTF-8",	N_("Swedish"),
  "ta_IN.UTF-8",	N_("Tamil"),
  "th_TH.UTF-8",	N_("Thai"),
  "az_AZ.UTF-8",	N_("Turkish (Azerbaijan)"),
  "tr_TR.UTF-8",	N_("Turkish"),
  "uk_UA.UTF8",		N_("Ukrainian"),
  "ur_PK.UTF-8",	N_("Urdu"),
  "vi_VN.UTF-8",	N_("Vietnamese"),
  "wa_BE.UTF-8",	N_("Walloon"),
  "zu_ZA.UTF-8",	N_("Zulu"),
  NULL, NULL
};

static gchar *timername[] = {
  N_("No time limit"),
  N_("Slow timer"),
  N_("Normal timer"),
  N_("Fast timer")
};

static gchar *filtername =
  N_("Use Gcompris administration module to filter boards");

static void set_locale_flag(gchar *locale);
static gchar *get_next_locale(gchar *locale);
static gchar *get_previous_locale(gchar *locale);
static gboolean item_event_ok (GooCanvasItem  *item,
			       GooCanvasItem  *target,
			       GdkEventButton *event,
			       gchar *data);
static void   display_previous_next(guint x_start, guint y_start,
				    gchar *eventname_previous, gchar *eventname_next);


/*
 * Do all the bar display and register the events
 */
void
gc_config_start ()
{
  GcomprisProperties	*properties = gc_prop_get();
  gint y_start = 0;
  gint x_start = 0;
  gint x_text_start = 0;
  gint y = 0;
  GooCanvasItem *item;

  /* Pause the board */
  gc_board_pause(TRUE);

  if(rootitem)
  {
    gc_config_stop();
    return;
  }

  gc_bar_hide(TRUE);

  rootitem = goo_canvas_group_new (goo_canvas_get_root_item(gc_get_canvas()),
				   NULL);

  item = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", "#DIALOG",
			     "pointer-events", GOO_CANVAS_EVENTS_NONE,
			     NULL);

  GooCanvasBounds bounds;
  goo_canvas_item_get_bounds(item, &bounds);
  x_start = bounds.x1;
  y_start = bounds.y1;

  y = bounds.y2 - 26;

  goo_canvas_text_new (rootitem,
		       _("GCompris Configuration"),
		       (gdouble) BOARDWIDTH/2,
		       (gdouble) y_start + 40,
		       -1,
		       GTK_ANCHOR_CENTER,
		       "font", gc_skin_font_title,
		       "fill-color-rgba", gc_skin_color_title,
		       NULL);

  pixmap_checked   = "#CHECKED";
  pixmap_unchecked = "#UNCHECKED";
  pixmap_width = 30;

  x_start += 150;
  x_flag_start = x_start + 50;
  x_text_start = x_start + 115;

  //--------------------------------------------------
  // Locale
  y_start += 105;

  display_previous_next(x_start, y_start, "locale_previous", "locale_next");

  y_flag_start = y_start - pixmap_width/2;

  /* Display a bad icon if this locale is not available */
  item_bad_flag = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", "#UNCHECKED",
			     "pointer-events", GOO_CANVAS_EVENTS_NONE,
			     NULL);
  SET_ITEM_LOCATION(item_bad_flag,
		    x_flag_start + 5,
		    y_start - pixmap_width/2);

  /* A repeat icon to reset the selection */
  item = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", "#REPEAT",
			     NULL);
    goo_canvas_item_get_bounds(item, &bounds);
    double zoom = 0.65;
    goo_canvas_item_scale(item, zoom, zoom);
    goo_canvas_item_translate(item,
			      (-1 * bounds.x1 + x_flag_start - 340) * zoom,
			      (-1 * bounds.y1 + y_start - 120) * zoom);
  g_signal_connect(item, "button_press_event",
		   (GCallback) item_event_ok,
		   "locale_reset");
  gc_item_focus_init(item, NULL);

  /*
   * The current locale is the one found in the config file
   */
  current_locale = properties->locale;
  set_locale_flag(current_locale);

  item_locale_text = goo_canvas_text_new (rootitem,
					  gc_locale_get_name(current_locale),
					  (gdouble) x_text_start,
					  (gdouble) y_start,
					  -1,
					  GTK_ANCHOR_WEST,
					  "font", gc_skin_font_subtitle,
					  "fill-color-rgba", gc_skin_color_content,
					  NULL);

  // Fullscreen / Window
  y_start += Y_GAP;

  item = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", (properties->fullscreen ? pixmap_checked : pixmap_unchecked),
			     NULL);
  SET_ITEM_LOCATION(item, x_start, y_start - pixmap_width/2);

  g_signal_connect(item, "button_press_event",
		   (GCallback) item_event_ok,
		   "fullscreen");
  gc_item_focus_init(item, NULL);


  goo_canvas_text_new (rootitem,
		       _("Fullscreen"),
		       (gdouble) x_text_start,
		       (gdouble) y_start,
		       -1,
		       GTK_ANCHOR_WEST,
		       "font", gc_skin_font_subtitle,
		       "fill-color-rgba", gc_skin_color_content,
		       NULL);

  // Music
  y_start += Y_GAP;

  item = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", (properties->music ? pixmap_checked : pixmap_unchecked),
			     NULL);
  SET_ITEM_LOCATION(item, x_start, y_start - pixmap_width/2);

  g_signal_connect(item, "button_press_event",
		   (GCallback) item_event_ok,
		   "music");
  gc_item_focus_init(item, NULL);


  goo_canvas_text_new (rootitem,
		       _("Music"),
		       (gdouble) x_text_start,
		       (gdouble) y_start,
		       -1,
		       GTK_ANCHOR_WEST,
		       "font", gc_skin_font_subtitle,
		       "fill-color-rgba", gc_skin_color_content,
		       NULL);

  // Effect
  y_start += Y_GAP;

  item = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", (properties->fx ? pixmap_checked : pixmap_unchecked),
			     NULL);
  SET_ITEM_LOCATION(item, x_start, y_start - pixmap_width/2);

  g_signal_connect(item, "button_press_event",
		   (GCallback) item_event_ok,
		   "effect");
  gc_item_focus_init(item, NULL);


  goo_canvas_text_new (rootitem,
		       _("Effect"),
		       (gdouble) x_text_start,
		       (gdouble) y_start,
		       -1,
		       GTK_ANCHOR_WEST,
		       "font", gc_skin_font_subtitle,
		       "fill-color-rgba", gc_skin_color_content,
		       NULL);

  // Zoom
  y_start += Y_GAP;

  item = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", (properties->zoom ? pixmap_checked : pixmap_unchecked),
			     NULL);
  SET_ITEM_LOCATION(item, x_start, y_start - pixmap_width/2);

  g_signal_connect(item, "button_press_event",
		   (GCallback) item_event_ok,
		   "zoom");
  gc_item_focus_init(item, NULL);

  goo_canvas_text_new (rootitem,
		       _("Zoom"),
		       (gdouble) x_text_start,
		       (gdouble) y_start,
		       -1,
		       GTK_ANCHOR_WEST,
		       "font", gc_skin_font_subtitle,
		       "fill-color-rgba", gc_skin_color_content,
		       NULL);

  // Timer
  y_start += Y_GAP;

  display_previous_next(x_start, y_start, "timer_previous", "timer_next");

  item_timer_text = goo_canvas_text_new (rootitem,
					 gettext(timername[properties->timer]),
					 (gdouble) x_text_start,
					 (gdouble) y_start,
					 -1,
					 GTK_ANCHOR_WEST,
					 "font", gc_skin_font_subtitle,
					 "fill-color-rgba", gc_skin_color_content,
					 NULL);

  // Skin
  {
    const gchar *one_dirent;
    guint  i;
    GDir  *dir;
    gchar *skin_dir;
    gchar *first_skin_name;

    /* Load the Pixpmaps directory file names */
    skin_dir = g_strconcat(properties->package_data_dir, "/skins", NULL);
    dir = g_dir_open(skin_dir, 0, NULL);

    if (!dir)
      g_warning (_("Couldn't open skin dir: %s"), skin_dir);

    /* Fill up the skin list */
    while((one_dirent = g_dir_read_name(dir)) != NULL) {

      if (one_dirent[0] != '.') {
	gchar *filename;
	/* Only directory here are skins */
	filename = g_strdup_printf("%s/%s", properties->package_skin_dir, one_dirent);

	if (g_file_test ((filename), G_FILE_TEST_IS_DIR)) {
	  gchar *skin_name = g_strdup_printf("%s", one_dirent);
	  skinlist = g_list_append (skinlist, skin_name);
	}
	g_free(filename);
      }
    }
    g_dir_close(dir);

    /* Find the current skin index */
    skin_index = 0;
    for(i=0; i<g_list_length(skinlist);  i++)
      if(!strcmp((char *)g_list_nth_data(skinlist, i), properties->skin))
	skin_index = i;

    y_start += Y_GAP;

    /* Should not happen. It the user found the config, there should be a skin */
    if(g_list_length(skinlist) > 0) {
      g_warning("No skin found in %s\n", skin_dir);
      display_previous_next(x_start, y_start, "skin_previous", "skin_next");
      first_skin_name = g_strdup_printf(_("Skin : %s"), (char *)g_list_nth_data(skinlist, skin_index));
    } else {
      first_skin_name = g_strdup(_("SKINS NOT FOUND"));
    }

    item_skin_text = goo_canvas_text_new (rootitem,
					  first_skin_name,
					  (gdouble) x_text_start,
					  (gdouble) y_start,
					  -1,
					  GTK_ANCHOR_WEST,
					  "font", gc_skin_font_subtitle,
					  "fill-color-rgba", gc_skin_color_content,
					  NULL);
    g_free(first_skin_name);
    g_free(skin_dir);

  }

  // Difficulty Filter
  y_start += Y_GAP;

  stars_group_x = x_start + 45;
  stars_group_y = y_start - 25;
  gchar *text = g_strdup_printf("<i>%s</i>", gettext(filtername));
  item_filter_text = goo_canvas_text_new (rootitem,
					  text,
					  x_text_start,
					  y_start,
					  400,
					  GTK_ANCHOR_WEST,
					  "use-markup", TRUE,
					  "font", gc_skin_font_subtitle,
					  "fill-color-rgba", gc_skin_color_content,
					  NULL);
  g_free(text);


  // OK
  gc_util_button_text_svg(rootitem,
			  BOARDWIDTH * 0.5,
			  y,
			  "#BUTTON_TEXT",
			  _("OK"),
			  (GCallback) item_event_ok,
			  "ok");

  is_displayed = TRUE;
}

void gc_config_stop ()
{
  // Destroy the help box
  if(rootitem!=NULL)
    {
      goo_canvas_item_remove(rootitem);
      gc_board_pause(FALSE);
    }
  rootitem = NULL;

  stars_group = NULL;

  /* UnPause the board */
  if(is_displayed)
    gc_board_pause(FALSE);

  gc_bar_hide(FALSE);

  is_displayed = FALSE;
}


/**
 * Given the locale name, return the full translated name
 * e.g In the French locale, gc_locale_get_name("fr_FR.UTF8") returns "Français"
 * If not found, return NULL
 * The result must not be freed.
 */
const gchar*
gc_locale_get_name(const gchar *locale_code)
{
  guint i = 0;
  const gchar *locale = locale_code;
  if ( !locale )
    return NULL;

  if( !g_ascii_strncasecmp(locale, "C", 1) )
      locale = GC_DEFAULT_LOCALE;

  /*
   * On some systems, locales are returned as fr_FR.utf8
   * but in GCompris we use fr_FR.UTF-8
   * Do our best to support both
   */
  const gchar *dot_char = g_strrstr ( locale, ".");

  while(linguas[i] != NULL)
    {

      if( !g_ascii_strncasecmp(locale, linguas[i],
			       (dot_char ? dot_char - locale : strlen(locale)) ) )
	return(gettext(linguas[i+1]));

      i=i+2;
    }
  // Oups this locale is not in the table.
  return( NULL );
}

/**
 * Given the locale code, return the short version
 * e.g In the French locale, gc_locale_short("fr_FR.UTF8") returns "fr"
 * The result must be freed.
 */
gchar*
gc_locale_short(const gchar *locale_code)
{
   const gchar *locale = locale_code;
   if ( ! locale )
    return NULL;

  if( !g_ascii_strncasecmp(locale, "C", 1) )
      locale = GC_DEFAULT_LOCALE;

  gchar **locale_short = g_strsplit_set(locale, "_", 2);
  if(g_strv_length(locale_short) >= 1)
    {
      gchar *result = g_strdup(locale_short[0]);
      g_strfreev(locale_short);
      return result;
    }
  g_strfreev(locale_short);

  // We found no '_', this is already a short version
  return ( g_strdup(locale) );
}

/**
 * Given the locale code, return the long version
 * e.g In the French locale, gc_locale_long("fr_FR.UTF8") returns "fr_FR"
 * The result must be freed.
 */
gchar*
gc_locale_long(const gchar *locale_code)
{
  const gchar *locale = locale_code;
  if ( ! locale )
    return NULL;

  if( !g_ascii_strncasecmp(locale, "C", 1) )
    locale = GC_DEFAULT_LOCALE;

  gchar **locale_long = g_strsplit_set(locale, ".", 2);
  if(g_strv_length(locale_long) >= 1)
    {
      gchar *result = g_strdup(locale_long[0]);
      g_strfreev(locale_long);
      return result;
    }
  g_strfreev(locale_long);

  // We found no '.', this is already a short version
  return ( g_strdup(locale) );
}

/**
 * Given the locale translated name, return the locale name
 * e.g In the French local, gc_locale_get_locale("Français") returns "fr_FR.UTF8"
 * If not found, return NULL
 * The result must not be freed.
 */
const gchar*
gc_locale_get_locale(const gchar *name)
{
  guint i = 0;

  if ( !name )
    return NULL;

  while(linguas[i] != NULL)
    {
      if( !strncmp(name, gettext(linguas[i+1]), strlen(name)) )
	 return( linguas[i] );

      i=i+2;
    }
  // Oups this locale is not in the table.
  return( NULL );
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
static void
display_previous_next(guint x_start, guint y_start,
		      gchar *eventname_previous, gchar *eventname_next)
{
  GooCanvasItem *item;
  GooCanvasBounds bounds;

  item = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", "#PREVIOUS",
			     NULL);
  goo_canvas_item_get_bounds(item, &bounds);
  SET_ITEM_LOCATION(item,
		    x_start - (bounds.x2 - bounds.x1) - 10,
		    y_start - pixmap_width/2);

  g_signal_connect(item, "button_press_event",
		   (GCallback) item_event_ok,
		   eventname_previous);
  gc_item_focus_init(item, NULL);


  item = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", "#NEXT",
			     NULL);
  SET_ITEM_LOCATION(item,
		    x_start,
		    y_start - pixmap_width/2);

  g_signal_connect(item, "button_press_event",
		   (GCallback) item_event_ok,
		   eventname_next);
  gc_item_focus_init(item, NULL);
}

static void
set_locale_flag(gchar *locale)
{

  if(locale == NULL)
    return;

  if(locale[0] == '\0') {
    /* Set the flag to the default user's locale */
    locale = gc_locale_get_user_default();
    g_message("gc_locale_get_user_default = %s\n", locale);
  }

  /* Check wether or not the locale is available */
#ifdef WIN32
  /* On win32, it's always available, do not try to check */
  g_object_set (item_bad_flag,
		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
		NULL);
#else
  if(setlocale(LC_MESSAGES, locale)==NULL)
    g_object_set (item_bad_flag,
		  "visibility", GOO_CANVAS_ITEM_VISIBLE,
		  NULL);

  else
    g_object_set (item_bad_flag,
		  "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		  NULL);
#endif

}


/**
 * Given the short locale name, return the next one in our linguas table
 */
static gchar *
get_next_locale(gchar *locale)
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
static gchar *
get_previous_locale(gchar *locale)
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

/* Callback for the bar operations */
static gint
item_event_ok(GooCanvasItem *item,
	      GooCanvasItem *target,
	      GdkEventButton *event,
	      gchar *data)
{
  GcomprisProperties *properties = gc_prop_get();

  if(data==NULL)
    return FALSE;

  if(!strcmp((char *)data, "ok"))
    {
      /* Set the new locale in the properties */
      if (properties->locale != current_locale)
	{
	  g_free(properties->locale);
	  properties->locale = strdup(current_locale);
	}

      if(current_locale[0] == '\0') {
	/* Set the locale to the default user's locale */
	gc_locale_set(gc_locale_get_user_default());
      } else {
	gc_locale_set(current_locale);
      }
      g_free(properties->skin);
      properties->skin = g_strdup((char *)g_list_nth_data(skinlist, skin_index));
      gc_skin_load(properties->skin);
      gc_config_stop();

      if(properties->music || properties->fx)
	gc_sound_init();

      if(!properties->music && !properties->fx)
	gc_sound_close();
      else
	{
	  if(!properties->music)
	    gc_sound_bg_close();

	  if(!properties->fx)
	    gc_sound_fx_close();
	}
      gc_prop_save(properties);
    }
  else if(!strcmp((char *)data, "fullscreen"))
    {
      properties->fullscreen = (properties->fullscreen ? 0 : 1);

      gc_fullscreen_set(properties->fullscreen);

      g_object_set (item,
		    "svg-id", (properties->fullscreen ? pixmap_checked : pixmap_unchecked),
		    NULL);

      gc_item_focus_init(item, NULL);
    }
  else if(!strcmp((char *)data, "music"))
    {
      properties->music = (properties->music ? 0 : 1);
      g_object_set (item,
		    "svg-id", (properties->music ? pixmap_checked : pixmap_unchecked),
		    NULL);
      if(!properties->music)
	{
	  gc_sound_bg_close();
	}
      else
	{
	  gc_sound_init();
	  gc_sound_bg_reopen();
	}
      gc_item_focus_init(item, NULL);
    }
  else if(!strcmp((char *)data, "effect"))
    {
      properties->fx = (properties->fx ? 0 : 1);
      if (properties->fx)
	gc_sound_init();
      g_object_set (item,
		    "svg-id", (properties->fx ? pixmap_checked : pixmap_unchecked),
		    NULL);
      gc_item_focus_init(item, NULL);
    }
  else if(!strcmp((char *)data, "zoom"))
    {
      properties->zoom = (properties->zoom ? 0 : 1);
      gc_update_canvas_zoom();
      g_object_set (item,
		    "svg-id", (properties->zoom ? pixmap_checked : pixmap_unchecked),
		    NULL);
      gc_item_focus_init(item, NULL);
    }
  else if(!strcmp((char *)data, "locale_previous"))
    {
      current_locale = get_previous_locale(current_locale);
      g_object_set (item_locale_text,
		    "text", gc_locale_get_name(current_locale),
		    NULL);
      set_locale_flag(current_locale);
    }
  else if(!strcmp((char *)data, "locale_next"))
    {
      current_locale = get_next_locale(current_locale);
      g_object_set (G_OBJECT(item_locale_text),
		    "text", gc_locale_get_name(current_locale),
		    NULL);

      set_locale_flag(current_locale);
    }
  else if(!strcmp((char *)data, "locale_reset"))
    {
      current_locale = linguas[0];
      g_object_set (G_OBJECT(item_locale_text),
		    "text", gc_locale_get_name(current_locale),
		    NULL);

      set_locale_flag(current_locale);
    }
  else if(!strcmp((char *)data, "timer_previous"))
    {
      if(properties->timer>0)
	properties->timer--;

      g_object_set (G_OBJECT(item_timer_text),
		    "text", gettext(timername[properties->timer]),
		    NULL);
    }
  else if(!strcmp((char *)data, "timer_next"))
    {
      if(properties->timer<MAX_TIMER_VALUE)
	properties->timer++;

      g_object_set (G_OBJECT(item_timer_text),
		    "text", gettext(timername[properties->timer]),
		    NULL);
    }
  else if(!strcmp((char *)data, "skin_previous"))
    {
      gchar *skin_str;
      if(skin_index-- < 1)
	skin_index = g_list_length(skinlist)-1;

      skin_str = g_strdup_printf(_("Skin : %s"),
				 (char *)g_list_nth_data(skinlist, skin_index));

      g_object_set (G_OBJECT(item_skin_text),
		    "text", skin_str,
		    NULL);
      g_free(skin_str);
    }
  else if(!strcmp((char *)data, "skin_next"))
    {
      gchar *skin_str;
      if(skin_index++ >= g_list_length(skinlist)-1)
	skin_index = 0;

      skin_str = g_strdup_printf(_("Skin : %s"),
				 (char *)g_list_nth_data(skinlist, skin_index));
      g_object_set (G_OBJECT(item_skin_text),
		    "text", skin_str,
		    NULL);
      g_free(skin_str);
    }

  return TRUE;
}
