/* gcompris - sugar_db.h
 *
 * Copyright (C) 2010, Aleksey Lim
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

#ifndef SUGAR_DB_H
#define SUGAR_DB_H

#if defined(USE_SUGAR) && defined(USE_SQLITE)
  void sugar_db_open(void);
  void sugar_db_close(int profile_id);

  /** Write profile data from key file to db */
  gint sugar_db_write(GKeyFile *profile);

  /** Read profile data from db and store it in key file */
  void sugar_db_read(GKeyFile *profile, gint profile_id);

  /** Write log entry to db */
  void sugar_db_write_report (const char *date, gint duration,
          const char *user, const char *board, gint level, gint sublevel,
          gint status, const char *comment);

  /** Read all log entries from db and store them in key file */
  void sugar_db_read_reports (GKeyFile *profile);
#else
# define sugar_db_open()
# define sugar_db_close(profile_id)
# define sugar_db_write(config) 1
# define sugar_db_read(config, profile_id)
# define sugar_db_write_report(date, duration, user, board, level, sublevel, \
        status, comment)
# define sugar_db_read_reports(profile)
#endif

#endif
