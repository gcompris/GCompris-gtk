/* gcompris - reporting.c
 *
 * Time-stamp: <2003/10/29 18:55:55 bcoudoin>
 *
 * Copyright (C) 2004 Bruno Coudoin
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


/* Trace formating
 * date,computer,user,board,level,sublevel,status, duration,comment
 *
 * status can be : PASSED or FAILED
 * comment is a free optional string that can describe what went wrong. For
 * example, in a reading activity, you could write here the word that make the
 * kid fail. It may be usefull for the teacher.
 *
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include <gcompris.h>
 
#define KEYLOG_MAX 256

static gchar hostname[256];

static gchar		*comment_set;
static gchar		 keylog[KEYLOG_MAX];
static GcomprisBoard	*gcomprisBoard_set;
static time_t		 start_time;
static time_t		 start_time_key;

/* By default, we use local time, not UTC */
#define USE_UTC 0

/** gcompris_log_start 
 * \param GcomprisBoard *gcomprisBoard: the board for which the event happen
 *
 */
void gcompris_log_start (GcomprisBoard *gcomprisBoard) {

  gcomprisBoard_set = gcomprisBoard;
  start_time     = time(NULL);
  start_time_key = time(NULL);
  gethostname(hostname, 256);
  comment_set = "";
  keylog[0]   = '\0';
}

/** gcompris_log_set_comment
 * \param GcomprisBoard *gcomprisBoard: the board for which the event happen
 * \param expected: string describing what was expected. This string is copied.
 * \param got: string describing what we got from the user. This string is copied.
 *
 * Note: Do not include the charater ';' in expected or got
 *
 */
void gcompris_log_set_comment (GcomprisBoard *gcomprisBoard, gchar *expected, gchar *got) {

  printf("gcompris_log_set_comment %s %s\n", expected, got);
  if(gcomprisBoard_set != gcomprisBoard) {
    return;
  }

  if(expected==NULL)
    expected="";

  if(got==NULL)
    got="";

  /* If We already had a comment, log the previous one */
  if(comment_set[0] != '\0') {
    gcompris_log_end(gcomprisBoard, GCOMPRIS_LOG_STATUS_FAILED);
  }

  comment_set = g_strdup_printf("%s;%s", expected, got);
}

/** gcompris_log_key
 * \param GcomprisBoard *gcomprisBoard: the board for which the event happen
 * \param key: a keyval as defined by gtk
 *
 */
void gcompris_log_set_key (GcomprisBoard *gcomprisBoard, guint keyval) {
  char utf8char[6];
  int i;
  /* get the current time from the Unix kernel */
  time_t end_time = time(NULL);
  double duration = difftime(end_time,start_time_key);

  if(!g_unichar_isalnum (gdk_keyval_to_unicode (keyval)))
    return;

  /* Reset the timer */
  start_time_key = end_time;

  /* Should be an easier way to get the UTF-8 code in our string */
  for(i=0; i<6; i++)
    utf8char[i] = '\0';

  sprintf(utf8char, "%c", gdk_keyval_to_unicode(keyval));

  g_unichar_to_utf8 (gdk_keyval_to_unicode(keyval),
  		     utf8char);

  if(strlen(keylog)<(KEYLOG_MAX-10)) {
    strcat(keylog, utf8char);
    printf(" 1 gcompris_log_set_key %s\n", keylog);
    sprintf(keylog+strlen(keylog), "/%d:", (guint)duration);
    printf(" 2 gcompris_log_set_key %s\n", keylog);
  }

}

/** gcompris_log_end
 * \param GcomprisBoard *gcomprisBoard: the board for which the event happen
 * \param status: a string representing the status like PASSED, FAILED.
 *
 */
void gcompris_log_end (GcomprisBoard *gcomprisBoard, gchar *status) {
  FILE *flog;
  gchar *file;

  /* Prepare our log */

  /* The default format for time represenation.  See strftime(3) */
  char *fmt = "%a %b %d %H:%M:%S %Z %Y";

  char buf[256];
   
  /* get the current time from the Unix kernel */
  time_t end_time = time(NULL);
  double duration = difftime(end_time,start_time);

  struct tm *tp;

  if(gcomprisBoard_set != gcomprisBoard)
    return;

  /* and convert it to UTC or local time representation */
  if (USE_UTC)
    tp = gmtime(&start_time);
  else
    tp = localtime(&start_time);

  /* convert the time to a string according to the format specification in fmt */
  strftime(buf, sizeof(buf), fmt, tp);

  /* Print it out */
  file = g_strconcat(g_get_home_dir(), "/.gcompris.log", NULL);

  flog = fopen(file,"a");

  /* date,computer,user,board,level,sublevel,status, duration,comment */
  fprintf(flog, "%s;%s;%s;gcompris;%s;%d;%d;%s;%d;%s;%s\n", buf, hostname, g_get_user_name(),
	  gcomprisBoard->name, 
	  gcomprisBoard->level, gcomprisBoard->sublevel,
	  status,
	  (guint)duration,
	  comment_set,
	  keylog);
  printf("%s;%s;%s;gcompris;%s;%d;%d;%s;%d;%s;%s\n", buf, hostname, g_get_user_name(), 
	 gcomprisBoard->name, 
	 gcomprisBoard->level, gcomprisBoard->sublevel,
	 status,
	 (guint)duration,
	 comment_set,
	 keylog);

  fclose(flog);

  g_free(file);
}

