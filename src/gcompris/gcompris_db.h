/* gcompris - gcompris_db.h
 *
 * Time-stamp: <2005/06/02 23:51:56 yves>
 *
 * Copyright (C) 2005 Bruno Coudoin
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


#ifndef GCOMPRIS_DB_H
#define GCOMPRIS_DB_H

#include <sqlite3.h>

sqlite3 *gcompris_db;

void *gcompris_db_init();

void *gcompris_db_exit();

void gcompris_db_board_update(gchar *name, gchar *section, gchar *author, gchar *type, gchar *mode, int difficulty, gchar *icon);

void gcompris_db_board_locale_update(int board_id, gchar *language, gchar *title, gchar *description, gchar *prerequisite, gchar *goal, gchar *manual);

GList *gcompris_db_read_board_from_section(gchar *section);


#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
