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

#include <glib/gstdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "gcompris.h"

/* This should be detected in the configure for good portability */
#define HAVE_SETENV 1

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

static gchar *config_file = NULL;

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


/* get the gcompris user directory name */
/* Architecture dependant: "gcompris" in Win9x, */
/* "/.gcompris" in POSIX compliant systems */

gchar *
gc_prop_user_root_directory_get ()
{
  G_CONST_RETURN gchar *home_dir = g_get_home_dir();

  if (home_dir == NULL) /* Win9x */
    return g_strdup("gcompris");
  else
    return g_strconcat(home_dir, "/.gcompris", NULL);
}

/** return the name of the configuration file used.
 *  the name has the full path and is platform dependant
 *  must not be freed by the caller.
 *
 */
gchar *
gc_prop_config_file_get()
{
  gchar *dir;
  if(config_file)
    return(config_file);
  dir = gc_prop_user_root_directory_get();
  /* Was never called, must calculate it */
  if (g_get_home_dir()==NULL) {
    config_file = g_strconcat(dir, "/gcompris.cfg", NULL);
  } else {
    config_file = g_strconcat(dir, "/gcompris.conf", NULL);
  }
  g_free(dir);
   return(config_file);
}

/* get the default database name */
#define DEFAULT_DATABASE "gcompris_sqlite.db"
#define PROFILES_ROOT "profiles"

gchar *
gc_prop_default_database_name_get (gchar *shared_dir)
{
  gchar *dir_base = g_strconcat( shared_dir, "/",  PROFILES_ROOT, NULL);
  gc_util_create_rootdir(dir_base);
  g_free(dir_base);
  return g_strconcat( shared_dir, "/",  PROFILES_ROOT, "/",  DEFAULT_DATABASE, NULL);

}

GcomprisProperties *
gc_prop_new ()
{
  GcomprisProperties *tmp;
  char          *config_file = gc_prop_config_file_get();
  GScanner      *scanner;
  int		 filefd;
  gchar         *full_rootdir;
  const gchar   *locale;
  gchar         *user_dir;

  tmp = (GcomprisProperties *) malloc (sizeof (GcomprisProperties));
  tmp->music		 = 1;
  tmp->fx		 = 1;
  tmp->screensize	 = 1;
  tmp->fullscreen	 = 1;
  tmp->noxf86vm		 = FALSE;
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
  tmp->root_menu         = "/";
  tmp->local_directory   = NULL;
  tmp->profile           = NULL;
  tmp->logged_user       = NULL;

  tmp->administration    = FALSE;
  tmp->reread_menu       = FALSE;
  tmp->experimental      = FALSE;
  tmp->menu_position     = NULL;

  tmp->server            = NULL;

  tmp->package_data_dir           = NULL;
  tmp->package_locale_dir         = NULL;
  tmp->package_plugin_dir         = NULL;
  tmp->package_python_plugin_dir  = NULL;
  tmp->system_icon_dir            = NULL;
  tmp->cache_dir                  = NULL;

  user_dir = gc_prop_user_root_directory_get() ;
  gc_util_create_rootdir( user_dir );

  tmp->shared_dir        = g_strconcat(user_dir, "/shared", NULL);
  gc_util_create_rootdir( tmp->shared_dir );

  tmp->users_dir        = g_strconcat(user_dir, "/users", NULL);
  gc_util_create_rootdir( tmp->users_dir );

  tmp->user_data_dir	= g_strconcat(user_dir, "/Plugins/boards", NULL);
  gc_util_create_rootdir( tmp->user_data_dir );

  /* Needs to be set after command line parsing */
  tmp->database          = NULL;

  full_rootdir = g_strconcat(user_dir, "/user_data", NULL);
  gc_util_create_rootdir(full_rootdir);
  g_free(full_rootdir);

  full_rootdir = g_strconcat(user_dir, "/user_data/images", NULL);
  gc_util_create_rootdir(full_rootdir);
  g_free(full_rootdir);

  full_rootdir = g_strconcat(user_dir, "/", PROFILES_ROOT, NULL);
  gc_util_create_rootdir(full_rootdir);
  g_free(full_rootdir);

  g_free(user_dir);

  g_warning("config_file %s", config_file);

  filefd = open(config_file, O_RDONLY);

  if(filefd > 0) {

    /* create a new scanner */
    scanner = g_scanner_new(NULL);

    /* set up the scanner to read from the file */
    g_scanner_input_file(scanner, filefd);

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
	  if(!scan_get_int(scanner, &tmp->music))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "fx")) {
	  if(!scan_get_int(scanner, &tmp->fx))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "screensize")) {
	  if(!scan_get_int(scanner, &tmp->screensize))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "fullscreen")) {
	  if(!scan_get_int(scanner, &tmp->fullscreen))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "noxf86vm")) {
	  if(!scan_get_int(scanner, &tmp->noxf86vm))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "timer")) {
	  if(!scan_get_int(scanner, &tmp->timer))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "difficulty_filter")) {
	  if(!scan_get_int(scanner, &tmp->difficulty_filter))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "disable_quit")) {
	  if(!scan_get_int(scanner, &tmp->disable_quit))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "disable_config")) {
	  if(!scan_get_int(scanner, &tmp->disable_config))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "filter_style")) {
	  if(!scan_get_int(scanner, &tmp->filter_style))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "skin")) {
        g_free(tmp->skin);
	  tmp->skin = scan_get_string(scanner);
	  if(!tmp->skin)
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "locale")) {
	  tmp->locale = scan_get_string(scanner);
	  if(!tmp->locale)
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "key")) {
      g_free(tmp->key);
	  tmp->key = scan_get_string(scanner);
	  if(!tmp->key)
	    g_warning("Config file parsing error on token %s", token);
	}
	else if(!strcmp(value.v_identifier, "database")) {
	  tmp->database = scan_get_string(scanner);
	  if(!tmp->database)
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

    close(filefd);

  }

  /*
   * Warning, gcompris need a proper locale prefix to find suitable dataset
   * Some system use LOCALE 'C' for english. We have to set it explicitly
   */
  if(!tmp->locale) {

#if defined WIN32
    tmp->locale = g_win32_getlocale();
#else
    locale = g_getenv("LC_ALL");
    if(locale == NULL)
      locale = g_getenv("LC_MESSAGES");
    if(locale == NULL)
      locale = g_getenv("LANG");

    if (locale != NULL && !strcmp(locale, "C"))
      {
	tmp->locale		= "en_US.UTF-8";
      }
#endif
  }

  if(!tmp->locale) {
    /* No user specified locale = '' */
    tmp->locale		= strdup("");
  }

  return (tmp);
}

void
gc_prop_destroy (GcomprisProperties *props)
{
  if(!props)
    return;
  g_free(props->user_data_dir);
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
  g_free(props->shared_dir);
  g_free(props->users_dir);
  g_free(props->menu_position);
  g_free(props->server);
  g_free (props);
  g_warning("properties free");
}

void
gc_prop_save (GcomprisProperties *props)
{
  char *config_file = gc_prop_config_file_get();
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

  fprintf(filefd, "%s=\"%s\"\n", "database",		props->database);

  fclose(filefd);
}

gchar*
gc_prop_user_dirname_get(GcomprisUser *user)
{
  GcomprisProperties	*properties = gc_prop_get ();

  gchar *user_dirname = g_strconcat (properties->users_dir,
				     "/",
				     user->login,
				     NULL);

  gc_util_create_rootdir(user_dirname);

  return user_dirname;
}

gchar*
gc_prop_current_user_dirname_get()
{
  return gc_prop_user_dirname_get(gc_profile_get_current_user());
}

gchar*
gc_prop_board_dirname_get(GcomprisBoard *board)
{
  GcomprisProperties	*properties = gc_prop_get ();

  gchar *board_main = g_strconcat (properties->shared_dir, "/boards", NULL);
  gc_util_create_rootdir(board_main);

  gchar *board_dirname = g_strconcat (board_main, "/", board->name, NULL);
  gc_util_create_rootdir(board_dirname);

  g_free(board_main);
  return board_dirname;
}

gchar*
gc_prop_current_board_dirname_get()
{
  return gc_prop_board_dirname_get(gc_board_get_current());
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
