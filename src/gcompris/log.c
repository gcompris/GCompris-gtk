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
 

static gchar hostname[256];

static gchar *comment_set;
static GcomprisBoard *gcomprisBoard_set;
static time_t start_time;

/* By default, we use local time, not UTC */
#define USE_UTC 0

/** gcompris_log_start 
 * \param GcomprisBoard *gcomprisBoard: the board for which the event happen
 *
 */
void gcompris_log_start (GcomprisBoard *gcomprisBoard) {

  gcomprisBoard_set = gcomprisBoard;
  start_time = time(NULL);
  gethostname(hostname, 256);
  comment_set = "";
}

/** gcompris_log_set_comment
 * \param GcomprisBoard *gcomprisBoard: the board for which the event happen
 * \param comment: string describing what failed. this string is copied.
 *
 */
void gcompris_log_set_comment (GcomprisBoard *gcomprisBoard, gchar *comment) {

  if(comment != NULL && gcomprisBoard_set == gcomprisBoard)
    comment_set = g_strdup(comment);
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

  if(gcomprisBoard_set != gcomprisBoard)
    return;

  /* and convert it to UTC or local time representation */
  struct tm *tp;
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
  fprintf(flog, "%s;%s;%s;gcompris;%s;%d;%d;%s;%d;%s\n", buf, hostname, g_get_user_name(),
	  gcomprisBoard->name, 
	  gcomprisBoard->level, gcomprisBoard->sublevel,
	  status,
	  (guint)duration,
	  comment_set);
  printf("%s;%s;%s;gcompris;%s;%d;%d;%s;%d;%s\n", buf, hostname, g_get_user_name(), 
	 gcomprisBoard->name, 
	 gcomprisBoard->level, gcomprisBoard->sublevel,
	 status,
	 (guint)duration,
	 comment_set);

  fclose(flog);

  g_free(file);
}

