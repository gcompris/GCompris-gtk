/* gcompris - sugar_db.c
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

#include <string.h>
#include <libgen.h>
#include <glib.h>
#include <sqlite3.h>

#include "gcompris.h"

#define SQL_EXEC(result, query, ...) \
  { \
    int _ncolumn; \
    char *_query = sqlite3_mprintf((query), __VA_ARGS__); \
    db_error = NULL; \
    result = NULL; \
    if (sqlite3_get_table(db, _query, &result, &db_count, &_ncolumn, \
            &db_error) != SQLITE_OK) \
      g_warning("SQL error in '%s': %s", _query, db_error); \
    else if (db_count == 0 || (db_count == 1 && !result[_ncolumn])) \
    { \
      sqlite3_free_table(result); \
      result = NULL; \
    } \
    sqlite3_free(_query); \
  } \
  for (; result; sqlite3_free_table(result), result = NULL)

static void import_classes(GKeyFile*, gchar**);
static void import_users(GKeyFile*, gchar**);
static void import_groups(GKeyFile*, gint);
static void import_config(GKeyFile*, gchar*, gint, gint);
static void import_logs(GKeyFile*, gchar**);

static sqlite3 *db;
static char *db_error;
static int db_count;

void
sugar_db_open()
{
  if (sqlite3_open(gc_prop_get()->database, &db) != SQLITE_OK)
  {
    g_warning("Cannot open SQL db");
    db = NULL;
  }
}

void
sugar_db_close(int profile_id)
{
  if (db == NULL)
    return;

  if (profile_id != 1)
  {
    char **nop;

    SQL_EXEC(nop, "DELETE FROM board_profile_conf WHERE profile_id=%d;",
            profile_id);
    SQL_EXEC(nop, "DELETE FROM activities_out WHERE out_id=%d;", profile_id);
    SQL_EXEC(nop, "DELETE FROM profiles WHERE profile_id=%d;", profile_id);
    SQL_EXEC(nop, "DELETE FROM logs;", NULL);

    g_debug("Cleaned up sugar profile with id %d", profile_id);
  }

  sqlite3_close(db);
}

gint
sugar_db_write(GKeyFile *profile)
{
  char **result, **nop;
  int profile_id = 1;
  int i;

  if (db == NULL)
    return 1;

  SQL_EXEC(result, "SELECT MAX(profile_id) FROM profiles;", NULL)
     profile_id = atoi(result[1]) + 1;

  SQL_EXEC(nop, "INSERT INTO profiles (profile_id, name, profile_directory,"
          "description) VALUES (%d, %Q, %Q, %Q);",
          profile_id, "sugar", ".", "sugar");
  if (db_error)
    return 1;

  gchar **sections = g_key_file_get_groups(profile, NULL);
  if (!sections)
    return profile_id;

  g_debug("Import sugar profile with id %d", profile_id);

  SQL_EXEC(nop, "BEGIN TRANSACTION", NULL);

  SQL_EXEC(result, "SELECT board_id, filename FROM boards;", NULL)
  {
    gchar **include_boards = g_key_file_get_string_list(profile,
            "profile", "boards", NULL, NULL);

    for (i = db_count + 1; i-- > 1;)
    {
      gboolean exclude = include_boards != NULL;
      gchar *board_file = basename(result[i * 2 + 1]);
      gchar **include_file;

      if (include_boards)
        for (include_file = include_boards; *include_file; include_file++)
          if (strcmp(board_file, *include_file) == 0)
          {
            exclude = FALSE;
            break;
          }

      import_config(profile, board_file, profile_id, atoi(result[i * 2]));

      if (exclude)
        SQL_EXEC(nop, "INSERT INTO activities_out (board_id, out_id) "
                "VALUES (%s, %d);", result[i * 2], profile_id);
    }
    g_strfreev(include_boards);
  }

  import_config(profile, "common", profile_id, -1);
  import_classes(profile, sections);
  import_users(profile, sections);
  import_groups(profile, profile_id);
  import_logs(profile, sections);

  g_strfreev(sections);

  SQL_EXEC(nop, "END TRANSACTION", NULL);

  return profile_id;
}

void
sugar_db_read(GKeyFile *profile, gint profile_id)
{
  if (db == NULL)
    return;

  gchar **result;
  gint i, j;

  SQL_EXEC(result, "SELECT filename FROM boards "
          "WHERE board_id NOT IN "
          "(SELECT board_id FROM activities_out WHERE out_id=%d);",
          profile_id)
  {
    gchar *files[db_count];
    for (i = db_count + 1; i-- > 1;)
      files[i - 1] = basename(result[i]);
    g_key_file_set_string_list(profile, "profile", "boards",
            (const gchar* const*)files, db_count);
  }

  SQL_EXEC(result, "SELECT boards.filename, conf_key, conf_value "
          "FROM board_profile_conf "
          "LEFT JOIN boards ON boards.board_id = board_profile_conf.board_id "
          "WHERE profile_id=%d", profile_id)
  {
    for (i = db_count + 1; i-- > 1;)
    {
      gchar *board_file;
      if (result[i * 3])
        board_file = basename(result[i * 3]);
      else
        board_file = "common";
      gchar *board_section = g_strdup_printf("board/%s", board_file);
      g_key_file_set_string(profile, board_section,
              result[i * 3 + 1], result[i * 3 + 2]);
      g_free(board_section);
    }
  }

  SQL_EXEC(result, "SELECT class_id, name, teacher FROM class "
          "WHERE class_id IN (SELECT class_id FROM groups "
          "WHERE group_id IN (SELECT group_id FROM list_groups_in_profiles "
          "WHERE profile_id=%d));", profile_id)
  {
    for (i = db_count + 1; i-- > 1;)
    {
      gchar *section = g_strdup_printf("class/%s", result[i * 3 + 1]);
      g_key_file_set_string(profile, section, "class_id", result[i * 3]);
      g_key_file_set_string(profile, section, "name", result[i * 3 + 1]);
      g_key_file_set_string(profile, section, "teacher", result[i * 3 + 2]);
      g_free(section);
    }
  }

  SQL_EXEC(result, "SELECT user_id, login, lastname, firstname, birthdate, "
          "'class/'||class.name FROM users "
          "INNER JOIN class ON class.class_id = users.class_id "
          "WHERE user_id IN (SELECT user_id FROM list_users_in_groups "
          "WHERE group_id IN (SELECT group_id FROM list_groups_in_profiles "
          "WHERE profile_id=%d));", profile_id)
  {
    for (i = db_count + 1; i-- > 1;)
    {
      gchar *section = g_strdup_printf("user/%s", result[i * 6 + 1]);
      g_key_file_set_string(profile, section, "user_id", result[i * 6]);
      g_key_file_set_string(profile, section, "login", result[i * 6 + 1]);
      g_key_file_set_string(profile, section, "lastname", result[i * 6 + 2]);
      g_key_file_set_string(profile, section, "firstname", result[i * 6 + 3]);
      g_key_file_set_string(profile, section, "birthdate", result[i * 6 + 4]);
      g_key_file_set_string(profile, section, "class", result[i * 6 + 5]);
      g_free(section);
    }
  }

  SQL_EXEC(result, "SELECT group_id, groups.name, 'class/'||class.name, "
          "description, 'group/'||groups.name FROM groups "
          "INNER JOIN class ON class.class_id = groups.class_id "
          "WHERE group_id IN (SELECT group_id FROM list_groups_in_profiles "
          "WHERE profile_id=%d);", profile_id)
  {
    int group_count = db_count;
    gchar *group_sections[group_count];

    for (i = db_count + 1; i-- > 1;)
    {
      group_sections[i - 1] = result[i * 5 + 4];

      gchar *section = g_strdup_printf("group/%s", result[i * 5 + 1]);
      g_key_file_set_string(profile, section, "group_id", result[i * 5]);
      g_key_file_set_string(profile, section, "name", result[i * 5 + 1]);
      g_key_file_set_string(profile, section, "class", result[i * 5 + 2]);
      g_key_file_set_string(profile, section, "description", result[i * 5 + 3]);
      gchar **users_result;
      SQL_EXEC(users_result, "SELECT 'user/'||login FROM users "
              "WHERE user_id IN (SELECT user_id FROM list_users_in_groups "
              "WHERE group_id=%s);", result[i * 5])
      {
        gchar *users[db_count];
        for (j = db_count + 1; j-- > 1;)
          users[j - 1] = users_result[j];
        g_key_file_set_string_list(profile, section, "users",
                (const gchar* const*)users, db_count);
      }
      g_free(section);
    }

    g_key_file_set_string_list(profile, "profile", "groups",
            (const gchar* const*)group_sections, group_count);
  }
}

void
sugar_db_read_reports(GKeyFile *profile)
{
  if (db == NULL)
    return;

  gchar **result;
  gint i;

  SQL_EXEC(result, "SELECT date, duration, 'user/'||login, boards.name, "
          "level, sublevel, status, comment FROM logs "
          "LEFT JOIN users ON users.user_id = logs.user_id "
          "INNER JOIN boards ON boards.board_id = logs.board_id;", NULL)
  {
    for (i = db_count + 1; i-- > 1;)
    {
      gchar *section = g_strdup_printf("report/%s/%s",
              result[i * 8], result[i * 8 + 3]);
      g_key_file_set_string(profile, section, "date", result[i * 8]);
      g_key_file_set_string(profile, section, "duration", result[i * 8 + 1]);
      g_key_file_set_string(profile, section, "user", result[i * 8 + 2]);
      g_key_file_set_string(profile, section, "board", result[i * 8 + 3]);
      g_key_file_set_string(profile, section, "level", result[i * 8 + 4]);
      g_key_file_set_string(profile, section, "sublevel", result[i * 8 + 5]);
      g_key_file_set_string(profile, section, "status", result[i * 8 + 6]);
      g_key_file_set_string(profile, section, "comment", result[i * 8 + 7]);
      g_free(section);
    }
  }
}

void
sugar_db_write_report(const char *date, gint duration, const char *user,
        const char *board, gint level, gint sublevel, gint status,
        const char *comment)
{
  if (db == NULL)
    return;

  gchar **nop;

  SQL_EXEC(nop, "INSERT INTO logs (date, duration, user_id, board_id, level, "
          "sublevel, status, comment) VALUES (%Q, %d, "
          "(select user_id from users where login=%Q), "
          "(select board_id from boards where name=%Q), %d, %d, %d, %Q);",
          date, duration, user, board, level, sublevel, status, comment);
}

static void
import_classes(GKeyFile *profile, gchar **sections)
{
  gchar **result, **nop, **klass;

  for (klass = sections; *klass; klass++)
  {
    if (!g_str_has_prefix(*klass, "class/"))
      continue;
    int klass_id = -1;
    SQL_EXEC(result, "SELECT class_id FROM class WHERE name=%Q;",
            g_key_file_get_string(profile, *klass, "name", NULL))
      klass_id = atoi(result[1]);
    if (klass_id == -1)
    {
      klass_id = 1;
      SQL_EXEC(result, "SELECT MAX(class_id) FROM class;", NULL)
        klass_id = atoi(result[1]) + 1;
      SQL_EXEC(nop, "INSERT INTO class (class_id, name, teacher, wholegroup_id)"
              "VALUES (%d, %Q, %Q, %d);",
              klass_id,
              g_key_file_get_string(profile, *klass, "name", NULL),
              g_key_file_get_string(profile, *klass, "teacher", NULL),
              1);
      if (!db_error)
        g_debug("Imported class with id %d", klass_id);
    }
    g_key_file_set_integer(profile, *klass, "class_id", klass_id);
  }
}

static void
import_users(GKeyFile *profile, gchar **sections)
{
  gchar **result, **nop, **user;

  for (user = sections; *user; user++)
  {
    if (!g_str_has_prefix(*user, "user/"))
      continue;
    int user_id = -1;
    SQL_EXEC(result, "SELECT user_id FROM users WHERE login=%Q;",
            g_key_file_get_string(profile, *user, "login", NULL))
      user_id = atoi(result[1]);
    if (user_id == -1)
    {
      user_id = 1;
      SQL_EXEC(result, "SELECT MAX(user_id) FROM users;", NULL)
        user_id = atoi(result[1]) + 1;
      SQL_EXEC(nop, "INSERT INTO users (user_id, login, lastname, firstname, "
              "birthdate, class_id) VALUES (%d, %Q, %Q, %Q, %Q, %d);",
              user_id,
              g_key_file_get_string(profile, *user, "login", NULL),
              g_key_file_get_string(profile, *user, "lastname", NULL),
              g_key_file_get_string(profile, *user, "firstname", NULL),
              g_key_file_get_string(profile, *user, "birthdate", NULL),
              g_key_file_get_integer(profile,
                  g_key_file_get_string(profile, *user, "class", NULL),
                  "class_id", NULL));
      if (!db_error)
        g_debug("Imported user with id %d", user_id);
    }
    g_key_file_set_integer(profile, *user, "user_id", user_id);
  }
}

static void
import_groups(GKeyFile *profile, gint profile_id)
{
  gchar **result, **nop, **group, **groups;

  groups = g_key_file_get_string_list(profile, "profile", "groups", NULL, NULL);
  if (groups == NULL)
    return;

  for (group = groups; *group; group++)
  {
    int group_id = -1;
    SQL_EXEC(result, "SELECT group_id FROM groups WHERE name=%Q;",
            g_key_file_get_string(profile, *group, "name", NULL))
      group_id = atoi(result[1]);
    if (group_id == -1)
    {
      group_id = 1;
      SQL_EXEC(result, "SELECT MAX(group_id) FROM groups;", NULL)
        group_id = atoi(result[1]) + 1;
      SQL_EXEC(nop, "INSERT INTO groups (group_id, name, class_id, "
              "description) VALUES (%d, %Q, %d, %Q);",
              group_id,
              g_key_file_get_string(profile, *group, "name", NULL),
              g_key_file_get_integer(profile,
                  g_key_file_get_string(profile, *group, "class", NULL),
                  "class_id", NULL),
              g_key_file_get_string(profile, *group, "description", NULL));
      if (!db_error)
        g_debug("Imported group with id %d", group_id);
    }
    gchar **users = g_key_file_get_string_list(profile, *group, "users",
            NULL, NULL);
    if (users)
    {
      gchar **user;
      for (user = users; *user; user++)
        SQL_EXEC(nop, "INSERT INTO list_users_in_groups (user_id, "
                "group_id) VALUES (%d, %d);",
                g_key_file_get_integer(profile, *user, "user_id", NULL),
                group_id);
      g_strfreev(users);
    }
    SQL_EXEC(nop, "INSERT INTO list_groups_in_profiles (profile_id, "
            "group_id) VALUES (%d, %d);", profile_id, group_id);
    g_key_file_set_integer(profile, *group, "group_id", group_id);
  }

  g_strfreev(groups);
}

static void
import_config(GKeyFile *profile, gchar *board_file, gint profile_id,
        gint board_id)
{
  gchar *board_section = g_strdup_printf("board/%s", board_file);
  gchar **keys = g_key_file_get_keys(profile, board_section, NULL, NULL);
  char **nop;

  if (keys)
  {
    gchar **key;
    for (key = keys; *key; key++)
      SQL_EXEC(nop, "INSERT INTO board_profile_conf (profile_id, board_id, "
              "conf_key, conf_value) VALUES (%d, %d, %Q, %Q);",
              profile_id, board_id, *key,
              g_key_file_get_string(profile, board_section, *key, NULL));
    g_strfreev(keys);
  }

  g_free(board_section);
}

static void
import_logs(GKeyFile *profile, gchar **sections)
{
  gchar **nop, **log;

  SQL_EXEC(nop, "DELETE FROM logs;", NULL);

  for (log = sections; *log; log++)
  {
    if (!g_str_has_prefix(*log, "report/"))
      continue;

    SQL_EXEC(nop, "INSERT INTO logs (date, duration, user_id, board_id, level, "
            "sublevel, status, comment) VALUES (%Q, %Q, %d, "
            "(select board_id from boards where name=%Q), %Q, %Q, %Q, %Q);",
            g_key_file_get_string(profile, *log, "date", NULL),
            g_key_file_get_string(profile, *log, "duration", NULL),
            g_key_file_get_integer(profile,
                g_key_file_get_string(profile, *log, "user", NULL),
                "user_id", NULL),
            g_key_file_get_string(profile, *log, "board", NULL),
            g_key_file_get_string(profile, *log, "level", NULL),
            g_key_file_get_string(profile, *log, "sublevel", NULL),
            g_key_file_get_string(profile, *log, "status", NULL),
            g_key_file_get_string(profile, *log, "comment", NULL));
  }
}
