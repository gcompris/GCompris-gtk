/* gcompris - log.c
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


/* Trace formating are saved in our sqlite database with the fields
 * date,user,activity,level,sublevel,status,duration,comment
 *
 * status can be : PASSED or FAILED or COMPLETED
 * comment is a free optional string that can describe what went wrong. For
 * example, in a reading activity, you could write here the word that make the
 * kid fail. It may be usefull for the teacher.
 *
 *
 */

#include <time.h>

#include <gcompris.h>
#include "gcompris_db.h"
#include "profile.h"

#define KEYLOG_MAX 256

static gchar		*comment_set;
static gchar		 keylog[KEYLOG_MAX];
static GcomprisBoard	*gcomprisBoard_set;
static time_t		 start_time;
static time_t		 start_time_key;

/* By default, we use local time, not UTC */
#define USE_UTC 0

/** gc_log_start
 * \param GcomprisBoard *gcomprisBoard: the board for which the event happen
 *
 */
void gc_log_start (GcomprisBoard *gcomprisBoard) {

  gcomprisBoard_set = gcomprisBoard;
  start_time     = time(NULL);
  start_time_key = time(NULL);

  comment_set = g_strdup("");
  keylog[0]   = '\0';
}

/** gc_log_set_comment
 * \param GcomprisBoard *gcomprisBoard: the board for which the event happen
 * \param expected: string describing what was expected. This string is copied.
 * \param got: string describing what we got from the user. This string is copied.
 *
 */
void
gc_log_set_comment(GcomprisBoard *gcomprisBoard, gchar *expected, gchar *got)
{
  if(gcomprisBoard_set != gcomprisBoard) {
    return;
  }

  if(expected==NULL)
    expected="";

  if(got==NULL)
    got="";

  g_free(comment_set);
  comment_set = g_strdup_printf("%s;%s.", expected, got);
}

/** gc_log_end
 * \param GcomprisBoard *gcomprisBoard: the board for which the event happen
 * \param status
 *
 */
void gc_log_end (GcomprisBoard *gcomprisBoard, GCBonusStatusList status) {

  /* Prepare our log */

  /* The default format for time represenation.  See strftime(3) */
  char *fmt = "%F %T";

  char buf[256];

  /* get the current time from the Unix kernel */
  time_t end_time = time(NULL);
  double duration = difftime(end_time,start_time);

  struct tm *tp;

  GcomprisUser *gcomprisUser = gc_profile_get_current_user();

  /* A board change in between doesn't make sense */
  if(gcomprisBoard_set != gcomprisBoard)
    return;

  /* and convert it to UTC or local time representation */
  if (USE_UTC)
    tp = gmtime(&start_time);
  else
    tp = localtime(&start_time);

  /* convert the time to a string according to the format specification in fmt */
  strftime(buf, sizeof(buf), fmt, tp);


  gc_db_log(buf, (guint)duration,
	    gcomprisUser->user_id, gcomprisBoard->board_id,
	    gcomprisBoard->level, gcomprisBoard->sublevel,
	    status, comment_set);

  g_free(comment_set);
}

