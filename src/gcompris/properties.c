/* gcompris - properties.c
 *
 * Time-stamp: <2004/02/04 01:36:34 bcoudoin>
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gcompris.h"

GcomprisProperties *properties;

GHashTable* boards_hash = NULL;

void read_boards_status();
void write_boards_status();

GcomprisProperties *gcompris_get_properties ()
{
  return (properties);
}

/*
 * return 1 if parsing OK, 0 otherwise
 * the return value is returned in retval
 */
guint scan_get_int(GScanner *scanner, int *retval) {
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
gchar *scan_get_string(GScanner *scanner) {
  GTokenType token = g_scanner_get_next_token (scanner);
  token = g_scanner_get_next_token (scanner);
  if(token == G_TOKEN_STRING) {
    /* we got it */
    GTokenValue value = g_scanner_cur_value(scanner);
    return (g_strdup(value.v_string));
  }
  return NULL;
}

GcomprisProperties *gcompris_properties_new ()
{
  GcomprisProperties *tmp;
  char *config_file;
  char *locale;
  int i;
  GScanner *scanner;
  int filefd;

  boards_hash = g_hash_table_new (g_str_hash, g_str_equal);

  tmp = (GcomprisProperties *) malloc (sizeof (GcomprisProperties));
  tmp->music		= 1;
  tmp->fx		= 1;
  tmp->screensize	= 1;
  tmp->fullscreen	= 1;
  tmp->timer		= 1;
  tmp->skin		= "default";
  tmp->audio_output	= "";
  tmp->locale           = NULL;

  config_file = g_strdup_printf("%s/.gcompris",g_get_home_dir());

  filefd = open(config_file, O_RDONLY);

  g_free(config_file);

  if(filefd > 0) {

    /* create a new scanner */
    scanner = g_scanner_new(NULL);

    /* set up the scanner to read from the file */
    g_scanner_input_file(scanner, filefd);

    /* while the next token is something else other than end of file */
    while(g_scanner_peek_next_token(scanner) != G_TOKEN_EOF) {

      /* get the next token */
      GTokenType token = g_scanner_get_next_token(scanner);
      switch(token) {
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
	} else if(!strcmp(value.v_identifier, "timer")) {
	  if(!scan_get_int(scanner, &tmp->timer))
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "skin")) {
	  tmp->skin = scan_get_string(scanner);
	  if(!tmp->skin)
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "audio_output")) {
	  tmp->audio_output = scan_get_string(scanner);
	  if(!tmp->audio_output)
	    g_warning("Config file parsing error on token %s", token);
	} else if(!strcmp(value.v_identifier, "locale")) {
	  tmp->locale = scan_get_string(scanner);
	  if(!tmp->locale)
	    g_warning("Config file parsing error on token %s", token);
	}
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

  /* By default audio is said to work until libao fails to load it */
  tmp->audio_works	= TRUE;

  /* Non persistant value */
  tmp->difficulty_filter = -1;		/* No difficulty filter by default */

  /*
   * Warning, gcompris need a proper locale prefix to find suitable dataset
   * Some system use LOCALE 'C' for english. We have to set it explicitly
   */
  if(!tmp->locale) {
    locale = getenv("LC_ALL");
    if(locale == NULL)
      locale = getenv("LC_MESSAGES");
    if(locale == NULL)
      locale = getenv("LANG");

    if (locale != NULL && !strcmp(locale, "C"))
      {
	tmp->locale		= "en_US.UTF-8";
      } 
    else 
      {
	/* No user specified locale = '' */
	tmp->locale		= "";
      }
  }

  /*
   * Read the board status
   * ---------------------
   */
  read_boards_status();

  return (tmp);
}

void gcompris_properties_destroy (GcomprisProperties *props)
{
  if(props->locale!=NULL)
    g_free(props->locale);

  free (props);
}

void gcompris_properties_save (GcomprisProperties *props)
{
  char *config_file;
  FILE *filefd;

  config_file = g_strdup_printf("%s/.gcompris",g_get_home_dir());

  filefd = fopen(config_file, "w+");

  if(!filefd) {
      g_warning("cannot open '%s', configuration file not saved\n",(char *) config_file);
      return;
    }

  g_free(config_file);

  fprintf(filefd, "%s=%d\n", "music",		props->music);
  fprintf(filefd, "%s=%d\n", "fx",		props->fx);
  fprintf(filefd, "%s=%d\n", "screensize",	props->screensize);
  fprintf(filefd, "%s=%d\n", "fullscreen",	props->fullscreen);
  fprintf(filefd, "%s=%d\n", "timer",		props->timer);
  
  fprintf(filefd, "%s=\"%s\"\n", "skin",		props->skin);
  fprintf(filefd, "%s=\"%s\"\n", "audio_output",	props->audio_output);
  fprintf(filefd, "%s=\"%s\"\n", "locale",		props->locale);
  
  fclose(filefd);
}



static void boards_write (gchar       *key,
			  gpointer     value,
			  FILE        *filefd)
{
  printf("  %s=%d\n", key, GPOINTER_TO_UINT(value));
  fprintf(filefd, "%s=%d\n", key, GPOINTER_TO_UINT(value));
}


/*
 * Save the board status (enable/disable)
 */
void gcompris_write_boards_status()
{
  char *config_file;
  int i;
  GScanner *scanner;
  FILE *filefd;

  printf("gcompris_write_boards_status\n");
  config_file = g_strdup_printf("%s/.gcompris_boards",g_get_home_dir());

  filefd = fopen(config_file, "w+");

  if(!filefd) {
    g_warning("cannot open '%s', configuration file not saved\n",(char *) config_file);
    return;
  }

  g_free(config_file);

  g_hash_table_foreach (boards_hash, (GHFunc) boards_write, filefd);

}

void read_boards_status()
{
  char *config_file;
  int i;
  GScanner *scanner;
  int filefd;

  config_file = g_strdup_printf("%s/.gcompris_boards",g_get_home_dir());

  filefd = open(config_file, O_RDONLY);

  g_free(config_file);

  if(filefd > 0) {

    /* create a new scanner */
    scanner = g_scanner_new(NULL);

    /* set up the scanner to read from the file */
    g_scanner_input_file(scanner, filefd);

    /* while the next token is something else other than end of file */
    while(g_scanner_peek_next_token(scanner) != G_TOKEN_EOF) {

      /* get the next token */
      GTokenType token = g_scanner_get_next_token(scanner);
      switch(token) {
      case G_TOKEN_IDENTIFIER: {
	guint data;
	gchar *token;
	/* if we have a symbol, check it's ours */
	GTokenValue value = g_scanner_cur_value(scanner);
	token = g_strdup(value.v_identifier);

	if(!scan_get_int(scanner, &data))
	  g_warning("Config file parsing error on token %s", token);
	else {
	  g_hash_table_insert(boards_hash, token, GUINT_TO_POINTER(data));
	}
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
}

/*
 * Management of the status of the boards
 */
void gcompris_properties_enable_board(gchar *boardName)
{
  g_hash_table_remove(boards_hash, boardName);
}

void gcompris_properties_disable_board(gchar *boardName)
{
  g_hash_table_insert(boards_hash, boardName, GUINT_TO_POINTER(1));
}

/*
 * Return TRUE if boardName is available, FALSE otherwise
 *
 */
gboolean gcompris_properties_get_board_status(gchar *boardName)
{
  guint result;

  result = GPOINTER_TO_UINT(g_hash_table_lookup(boards_hash, boardName));

  if(result==1)
    return FALSE;
  else
    return TRUE;
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
