/* gcompris - gcompris_db.h
 *
 * Time-stamp: <2005/06/23 17:03:21 yves>
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

#ifdef USE_SQLITE
#include <sqlite3.h>
#endif

int gcompris_db_init();

void gcompris_db_exit();

gboolean gcompris_db_check_boards();

void gcompris_db_set_date(gchar *date);

void gcompris_db_board_update(gint *board_id, 
			      gint *section_id, 
			      gchar *name, 
			      gchar *section, 
			      gchar *author, 
			      gchar *type, 
			      gchar *mode, 
			      int difficulty, 
			      gchar *icon, 
			      gchar *boarddir,
			      gchar *mandatory_sound_file,
			      gchar *mandatory_sound_dataset,
			      gchar *filename,
			      gchar *title, 
			      gchar *description, 
			      gchar *prerequisite, 
			      gchar *goal, 
			      gchar *manual,
			      gchar *credit
			      );

void gcompris_db_save_user(int *user_id,
			   gchar *login,
			   gchar *name,
			   gchar *firstname,
			   gchar *birthday,
			   int class_id);

GList *gcompris_db_read_board_from_section(gchar *section);

GList *gcompris_load_menus_db(GList *boards);

GList *gcompris_db_get_board_id(GList *list);

void gcompris_db_remove_board(int board_id);

#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
