/* gcompris - properties.c
 *
 * Time-stamp: <2006/08/21 23:26:56 bruno>
 *
 * Copyright (C) 2000,2003 Bruno Coudoin
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

#define G_STDIO_NO_WRAP_ON_UNIX
#include <glib/gstdio.h>
#include <fcntl.h>
#include <string.h>

#include "gcompris.h"

/* This should be detected in the configure for good portability */
#define HAVE_SETENV 1

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

/*
 * return 1 if parsing OK, 0 otherwise
 * the return value is returned in retval
 */
static guint
scan_get_int(GScanner *scanner, int *retval) {
  GTokenType token = g_scanner_get_next_token (scanner);
  token = g_scanner_get_next_token (scanner);
  if(token == G_TOKEN_INT) {
    /* we got it */
    GTokenValue value = g_scanner_cur_value(scanner);
    *retval = value.v_int;
    return 1;
  }
  return 0;
}

/*
 * return String if parsing OK, NULL otherwise
 */
static gchar *
scan_get_string(GScanner *scanner) {
  GTokenType token = g_scanner_get_next_token (scanner);
  token = g_scanner_get_next_token (scanner);
  if(token == G_TOKEN_STRING) {
    /* we got it */
    GTokenValue value = g_scanner_cur_value(scanner);
    return (g_strdup(value.v_string));
  }
  return NULL;
}


/* get the gcompris config directory name
 *
 * \return a newly allocated string
 *
 * \warning Architecture dependant: "gcompris" in Win9x,
 * "/.gcompris" in POSIX compliant systems
 */
static gchar *
gc_prop_default_config_directory_get ()
{

#ifdef WIN32
  if ( ! G_WIN32_IS_NT_BASED() ) /* Win9x */
    return g_strdup("../gcompris");
  else
#endif
    {
      const gchar *home_dir = g_get_home_dir();
      const gchar *xdg_config_home = g_getenv("XDG_CONFIG_HOME");

      if(xdg_config_home)
	return g_strconcat(xdg_config_home, "/.gcompris", NULL);

      gchar *xdg_config= g_strconcat(home_dir, "/.config", NULL);
      gc_util_create_rootdir(xdg_config);
      g_free(xdg_config);

      return g_strconcat(home_dir, "/.config/gcompris", NULL);
    }
}

/* get the gcompris data directory name
 *
 * \return a newly allocated string
 *
 * \warning Architecture dependant: "gcompris" in Win9x,
 * "/MyGCompris" in POSIX compliant systems
 */
static gchar *
gc_prop_default_user_directory_get ()
{
  char *name = "My GCompris";
#ifdef WIN32
  if ( ! G_WIN32_IS_NT_BASED() ) /* Win9x */
    return g_strconcat("../", name, NULL);
  else
#endif
    {
      const gchar *home_dir = g_get_home_dir();
      return g_strconcat(home_dir, "/", name, NULL);
    }
}

/** return the name of the configuration file used.
 * must not be freed.
 *
 */
static const gchar *
gc_prop_config_file_get()
{
  /* Was never called, must calculate it */
#ifdef WIN32
  if (! G_WIN32_IS_NT_BASED() ) {
    return("gcompris.cfg");
  } else
#endif
    return("gcompris.conf");
}

/** Create a new GcomprisProperties struct with it's default values
 *
 *
 * \return a newly allocated GcomprisProperties
 */
GcomprisProperties *
gc_prop_new ()
{
  GcomprisProperties *tmp;

  tmp = (GcomprisProperties *) malloc (sizeof (GcomprisProperties));
  tmp->music		 = 1;
  tmp->fx		 = 1;
  tmp->screensize	 = 1;
  tmp->fullscreen	 = 0;
  tmp->timer		 = 1;
  tmp->skin		 = g_strdup("babytoy");
  tmp->key		 = g_strdup("default");
  tmp->locale            = NULL;
  tmp->difficulty_max    = 0;
  tmp->filter_style      = GCOMPRIS_FILTER_NONE;	/* No difficulty filter by default */
  tmp->difficulty_filter = 1;				/* No difficulty filter by default */
  tmp->disable_quit      = 0;				/* Used to remove the quit button from the bar. Use it for kiosk mode */
  tmp->disable_config    = 0;				/* Used to remove the config button from the bar. Use it for kiosk mode */
  tmp->display_resource  = 0;
  tmp->root_menu         = g_strdup("/");
  tmp->local_directory   = NULL;
  tmp->profile           = NULL;
  tmp->logged_user       = NULL;

  tmp->administration    = FALSE;
  tmp->reread_menu       = FALSE;
  tmp->experimental      = FALSE;

  tmp->server            = NULL;

  tmp->package_data_dir           = NULL;
  tmp->package_locale_dir         = NULL;
  tmp->package_plugin_dir         = NULL;
  tmp->package_python_plugin_dir  = NULL;
  tmp->system_icon_dir            = NULL;
  tmp->cache_dir                  = NULL;
  tmp->drag_mode                  = GC_DRAG_MODE_GRAB;

  tmp->config_dir = gc_prop_default_config_directory_get();
  tmp->user_dir = gc_prop_default_user_directory_get();
  tmp->database = NULL;

  return (tmp);
}

void gc_prop_old_config_migration(GcomprisProperties *props)
{
  char *old;
  char *new;

  new = g_strconcat(props->config_dir, "/", gc_prop_config_file_get(), NULL);

  if (!g_file_test(new, G_FILE_TEST_IS_REGULAR))
    {
      old = g_strconcat(g_get_home_dir(), "/.gcompris/gcompris.conf", NULL);
      if (g_file_test(old, G_FILE_TEST_IS_REGULAR))
	{
	  printf("Config file migration '%s' -> '%s'\n",
		 old, new);
	  if (!g_file_test(props->config_dir, G_FILE_TEST_IS_DIR))
	    gc_util_create_rootdir(props->config_dir);

	  g_rename(old, new);
	}
      g_free(old);
    }
  g_free(new);

  /* DATA BASE FILE MIGRATION */
  new = g_strconcat(props->config_dir, "/gcompris_sqlite.db", NULL);

  if (!g_file_test(new, G_FILE_TEST_IS_REGULAR))
    {
      old = g_strconcat(g_get_home_dir(),
			"/.gcompris/shared/profiles/gcompris_sqlite.db",
			NULL);

      if (g_file_test(old, G_FILE_TEST_IS_REGULAR))
	{
	  printf("Database migration '%s' -> '%s'\n",
		 old, new);
	  g_rename(old, new);
	}
      g_free(old);
    }
  g_free(new);

  /* LOG FILE MIGRATION */
  new = g_strconcat(props->config_dir,
		    "/gcompris.log",
		    NULL);
  if (!g_file_test(new, G_FILE_TEST_IS_REGULAR))
    {
      old = g_strconcat(g_get_home_dir(),
			"/.gcompris/gcompris.log",
			NULL);
      if (g_file_test(old, G_FILE_TEST_IS_REGULAR))
	{
	  printf("Logs migration '%s' -> '%s'\n",
		 old, new);
	  g_rename(old, new);
	}
      g_free(old);
    }
  g_free(new);


  /* User Images Migration */
  new = g_strconcat(props->user_dir,
		    NULL);
  if (!g_file_test(new, G_FILE_TEST_IS_DIR))
    gc_util_create_rootdir(new);
  g_free(new);

  new = g_strconcat(props->user_dir,
		    "/Images",
		    NULL);
  if (!g_file_test(new, G_FILE_TEST_IS_DIR))
    {
      old = g_strconcat(g_get_home_dir(),
			"/.gcompris/user_data/images",
			NULL);
      if (g_file_test(old, G_FILE_TEST_IS_DIR))
	{
	  int retval = g_rename(old, new);
	  printf("Image directory migration (%d) '%s' -> '%s'\n",
		 retval, old, new);
	}
      g_free(old);
    }
  g_free(new);

  /* User Activity Data Migration */
  old = g_strconcat(g_get_home_dir(),
		    "/.gcompris/users",
		    NULL);
  if (g_file_test(old, G_FILE_TEST_IS_DIR))
    {
      GDir *diruser = g_dir_open(old, 0, NULL);
      const gchar *user;

      while((user = g_dir_read_name(diruser)))
	{
	  char *old2 = g_strconcat(old, "/", user, NULL);
	  GDir *diractivity = g_dir_open(old2, 0, NULL);

	  const gchar *activity;

	  while((activity = g_dir_read_name(diractivity)))
	    {
	      char *old3 = g_strconcat(old, "/", user, "/", activity, NULL);
	      GDir *dirfile = g_dir_open(old3, 0, NULL);

	      const gchar *file;
	      while((file = g_dir_read_name(dirfile)))
		{
		  gchar *oldfullfile = g_strconcat(old3, "/", file, NULL);
		  gchar *newfulldir = g_strconcat(props->user_dir, "/", activity, NULL);
		  gchar *newfullfile = g_strconcat(props->user_dir, "/", activity, "/", file, NULL);

		  gc_util_create_rootdir(newfulldir);

		  /* Oops, the file already exists, prepend the user name */
		  if(g_file_test(newfullfile, G_FILE_TEST_IS_REGULAR))
		    {
		      g_free(newfullfile);
		      newfullfile = g_strconcat(props->user_dir, "/", activity, "/", user, "_", file, NULL);
		      printf("Duplicate file, prepending user name %s\n", newfullfile);
		    }
		  int retval = g_rename(oldfullfile, newfullfile);
		  printf("Data file migration (%d) '%s' -> '%s'\n",
			 retval, oldfullfile, newfullfile);

		  g_free(oldfullfile);
		  g_free(newfulldir);
		  g_free(newfullfile);
		}

	      g_free(old3);
	      g_dir_close(dirfile);
	    }

	  g_dir_close(diractivity);
	  g_free(old2);
	}
      g_dir_close(diruser);
    }
  g_free(old);

}


void
gc_prop_load (GcomprisProperties *props, GCPropSourceConf source_conf)
{
  char          *config_file;
  GScanner      *scanner;
  gchar		*content;
  gsize		length;
#ifndef WIN32
  const gchar   *locale;
#endif

  switch(source_conf)
    {
    case GC_PROP_FROM_SYSTEM_CONF:
      config_file = g_strconcat(SYSTEM_CONFIG_DIR, "/", gc_prop_config_file_get(), NULL);
      break;
    case GC_PROP_FROM_USER_CONF:
      config_file = g_strconcat(props->config_dir, "/", gc_prop_config_file_get(), NULL);
      break;
    }

  if(g_file_get_contents(config_file,
			 &content,
			 &length,
			 NULL)) {

    g_warning("Loading config file '%s'", config_file);

    /* create a new scanner */
    scanner = g_scanner_new(NULL);

    /* set up the scanner to read from the file */
    g_scanner_input_text(scanner, content, length);

    /* while the next token is something else other than end of file */
    while(g_scanner_peek_next_token(scanner) != G_TOKEN_EOF) {

      /* get the next token */
      GTokenType tokent = g_scanner_get_next_token(scanner);
      switch(tokent) {
      case G_TOKEN_IDENTIFIER: {
	gchar *token;
	/* if we have a symbol, check it's ours */
	GTokenValue value = g_scanner_cur_value(scanner);
	token = g_strdup(value.v_identifier);

	if(!strcmp(value.v_identifier, "music")) {
	  if(!scan_get_int(scanner, &props->music))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "fx")) {
	  if(!scan_get_int(scanner, &props->fx))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "screensize")) {
	  if(!scan_get_int(scanner, &props->screensize))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "fullscreen")) {
	  if(!scan_get_int(scanner, &props->fullscreen))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "timer")) {
	  if(!scan_get_int(scanner, &props->timer))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "difficulty_filter")) {
	  if(!scan_get_int(scanner, &props->difficulty_filter))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "disable_quit")) {
	  if(!scan_get_int(scanner, &props->disable_quit))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "disable_config")) {
	  if(!scan_get_int(scanner, &props->disable_config))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "filter_style")) {
	  if(!scan_get_int(scanner, &props->filter_style))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "skin")) {
        g_free(props->skin);
	  props->skin = scan_get_string(scanner);
	  if(!props->skin)
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "locale")) {
	  props->locale = scan_get_string(scanner);
	  if(!props->locale)
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "key")) {
	  g_free(props->key);
	  props->key = scan_get_string(scanner);
	  if(!props->key)
	    g_warning("Config file parsing error on token %s", token);
	}
	g_free(token);
	break;
      }
      default:
	break;
      }
    }

    /* destroy the scanner */
    g_scanner_destroy(scanner);
    g_free(content);
  }

  /*
   * Warning, gcompris need a proper locale prefix to find suitable dataset
   * Some system use LOCALE 'C' for english. We have to set it explicitly
   */
  if(!props->locale) {

#if defined WIN32
    props->locale = g_win32_getlocale();
#else
    locale = g_getenv("LC_ALL");
    if(locale == NULL)
      locale = g_getenv("LC_CTYPE");
    if(locale == NULL)
      locale = g_getenv("LANG");

    if (locale != NULL && !strcmp(locale, "C"))
      {
	props->locale		= "en_US.UTF-8";
      }
#endif
  }

  if(!props->locale) {
    /* No user specified locale = '' */
    props->locale = strdup("");
  }

  g_free(config_file);

}

/** \brief One the properties have been loaded and overloaded by the user params
 * call this function to create the directory structure mandatory to make GCompris
 * works.
 *
 */
void gc_prop_activate(GcomprisProperties *props)
{
  char *tmp;

  if (!g_file_test(props->config_dir, G_FILE_TEST_IS_DIR))
    gc_util_create_rootdir(props->config_dir);

  if (!g_file_test(props->user_dir, G_FILE_TEST_IS_DIR))
    gc_util_create_rootdir(props->user_dir);

  tmp = g_strconcat(props->user_dir, "/Images", NULL);
  if (!g_file_test(tmp, G_FILE_TEST_IS_DIR))
    gc_util_create_rootdir(tmp);
  g_free(tmp);

  tmp = g_strconcat(props->user_dir, "/", _("readme"), ".txt", NULL);
  g_file_set_contents(tmp,
		      _("This directory contains the files you create with the GCompris educational suite\n"),
		      -1,
		      NULL);
  g_free(tmp);

  tmp = g_strconcat(props->user_dir, "/Images/", _("readme"), ".txt", NULL);
  g_file_set_contents(tmp,
		      _("Put any number of images in this directory.\n"
			"You can include these images in your drawings and animations.\n"
			"The image formats supported are jpeg, png and svg.\n"),
		      -1,
		      NULL);
  g_free(tmp);

}

void
gc_prop_destroy (GcomprisProperties *props)
{
  if(!props)
    return;
  g_free(props->package_data_dir);
  g_free(props->package_locale_dir);
  g_free(props->package_plugin_dir);
  g_free(props->package_python_plugin_dir);
  g_free(props->system_icon_dir);
  g_free(props->cache_dir);
  g_free(props->locale);
  g_free(props->skin);
  g_free(props->key);
  gc_profile_destroy(props->profile);
  gc_user_destroy(props->logged_user);
  g_free(props->database);
  g_free(props->config_dir);
  g_free(props->user_dir);
  g_free(props->server);
  g_free(props->root_menu);
  g_free(props);
  g_warning("properties free");
}

void
gc_prop_save (GcomprisProperties *props)
{
  char *config_file = g_strconcat(props->config_dir, "/", gc_prop_config_file_get(),
				  NULL);
  FILE *filefd;

  filefd = g_fopen(config_file, "w+");

  if(!filefd) {
      g_warning("cannot open '%s', configuration file not saved\n",(char *) config_file);
      return;
    }

  fprintf(filefd, "%s=%d\n", "music",			props->music);
  fprintf(filefd, "%s=%d\n", "fx",			props->fx);
  fprintf(filefd, "%s=%d\n", "screensize",		props->screensize);
  fprintf(filefd, "%s=%d\n", "fullscreen",		props->fullscreen);
  fprintf(filefd, "%s=%d\n", "timer",			props->timer);

  fprintf(filefd, "%s=\"%s\"\n", "skin",		props->skin);
  fprintf(filefd, "%s=\"%s\"\n", "locale",		props->locale);
  fprintf(filefd, "%s=\"%s\"\n", "key",			props->key);

  fclose(filefd);

  g_free(config_file);
}

int
gc_setenv (const char * name, const char * value) {
#if defined WIN32
  size_t namelen = strlen(name);
  size_t valuelen = (value==NULL ? 0 : strlen(value));
  /* On Woe32, each process has two copies of the environment variables,
     one managed by the OS and one managed by the C library. We set
     the value in both locations, so that other software that looks in
     one place or the other is guaranteed to see the value. Even if it's
     a bit slow. See also
     <http://article.gmane.org/gmane.comp.gnu.mingw.user/8272>
     <http://article.gmane.org/gmane.comp.gnu.mingw.user/8273>
     <http://www.cygwin.com/ml/cygwin/1999-04/msg00478.html> */
  if (!SetEnvironmentVariableA(name,value))
    return -1;
  //#endif
  //#if defined(HAVE_PUTENV)
  char* buffer = (char*)malloc(namelen+1+valuelen+1);
  if (!buffer)
    return -1; /* no need to set errno = ENOMEM */
  memcpy(buffer,name,namelen);
  if (value != NULL) {
    buffer[namelen] = '=';
    memcpy(buffer+namelen+1,value,valuelen);
    buffer[namelen+1+valuelen] = 0;
  } else
    buffer[namelen] = 0;
  return putenv(buffer);
#elif defined(HAVE_SETENV)
  return setenv(name,value,1);
#else
  /* Uh oh, neither putenv() nor setenv() ... */
  return -1;
#endif
}
