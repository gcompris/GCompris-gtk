/* gcompris - gcompris_db.h
 *
 * Copyright (C) 2005, 2008 Bruno Coudoin
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


#ifndef GCOMPRIS_DB_H
#define GCOMPRIS_DB_H

#ifdef USE_SQLITE
#include <sqlite3.h>
#endif

/* Database management */
int gc_db_init();
void gc_db_exit();

gboolean gc_db_check_boards();

void gc_db_set_date(gchar *date);

void gc_db_set_version(gchar *version);

void gc_db_board_update(guint *board_id,
			      guint *section_id,
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

void gc_db_save_user(int *user_id,
			   gchar *login,
			   gchar *name,
			   gchar *firstname,
			   gchar *birthday,
			   int class_id);

GList *gc_db_read_board_from_section(gchar *section);

GList *gc_menu_load_db(GList *boards);

GList *gc_db_get_board_id(GList *list);

void gc_db_remove_board(int board_id);


/* Profile management */

GcomprisProfile *gc_db_get_profile_from_id(gint profile_id);
GcomprisProfile *gc_db_profile_from_name_get(gchar *profile_name);
GList *gc_db_profiles_list_get();


/* Classes management */

GcomprisClass *gc_db_get_class_from_id(gint class_id);

GList *gc_db_get_classes_list();

GcomprisGroup *gc_db_get_group_from_id(gint group_id);

GList *gc_db_get_groups_list();

GcomprisUser *gc_db_get_user_from_id(gint user_id);

GList *gc_db_get_users_list();

GcomprisProfile *gc_db_get_profile();

GList *gc_db_users_from_group_get(gint group_id);

void gc_db_set_board_conf(GcomprisProfile *profile,
			     GcomprisBoard  *board,
			     gchar *key,
			     gchar *value);


/* These two hash tables needs to be destroyed when not used more */

/* get conf for currents profile and board */
GHashTable *gc_db_get_board_conf();

/* get conf for specific profile and board */
GHashTable *gc_db_get_conf(GcomprisProfile *profile, GcomprisBoard  *board);

/*internal initialisation use only, do not use it for boards*/
GHashTable *gc_db_conf_with_table_get(int profile_id, int board_id, GHashTable *table );

GcomprisBoard *gc_db_get_board_from_id(int board_id);

int gc_db_is_activity_in_profile(GcomprisProfile *profile, char *activity_name);

void gc_db_log(gchar *date, int duration,
	       int user_id, int board_id,
	       int level, int sublevel,
	       int status, gchar *comment);

#endif
