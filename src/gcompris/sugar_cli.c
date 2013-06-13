/* gcompris - sugar_cli.c
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

#include <polyol/toolkit.h>

#include "gcompris.h"
#include "gc_core.h"
#include "status.h"
#include "sugar.h"
#include "sugar_share.h"

static void construct();
static void finalize();
static void load_profile();
static void setup_gc_profile();
static void scope_changed_cb(SugarConnection*, SugarShareScope, const gchar*);
static void channel_appeared_cb(SugarConnection*, SugarChannel*);
static void buddy_appeared_cb(SugarChannel*, guint, const gchar*);
static void buddy_disappeared_cb(SugarChannel*, guint, const gchar*);
static void get_profile_from_server_cb(const char*, void*);
static void load_board(gchar*, gint);
static void finalize_delayed_start();
static SugarJobject *get_jobject(GcomprisBoard*);
static void save_jobject(SugarJobject*);

static GKeyFile *profile;
static AdministrationClient *client;
static gboolean delayed_start;

Peer cli_peer = {
  construct,
  finalize,
  get_jobject,
  save_jobject
};

gboolean
sugar_delayed_start(void)
{
  return delayed_start;
}

void
sugar_report(const gchar *date, guint duration, const gchar *user,
        GcomprisBoard *board, GCBonusStatusList status, const gchar *comment)
{
  gchar *user_section = g_strdup_printf("user/%s", user);
  gchar *report_section = g_strdup_printf("report/%s/%s", date, board->name);

  if (client != NULL)
    administration_client_report(client, date, duration, user,
            board->name, board->level, board->sublevel, (int) status, comment);

  g_key_file_set_string(profile, report_section, "date", date);
  g_key_file_set_integer(profile, report_section, "duration", duration);
  g_key_file_set_string(profile, report_section, "user", user_section);
  g_key_file_set_string(profile, report_section, "board", board->name);
  g_key_file_set_integer(profile, report_section, "level", board->level);
  g_key_file_set_integer(profile, report_section, "sublevel", board->sublevel);
  g_key_file_set_integer(profile, report_section, "status", (int)status);
  g_key_file_set_string(profile, report_section, "comment", comment);

  g_free(report_section);
  g_free(user_section);
}

static void
construct()
{
  profile = g_key_file_new();

  SugarConnection *conn = sugar_activity_get_connection(activity);

  g_signal_connect(conn, "scope-changed", G_CALLBACK(scope_changed_cb), NULL);
  g_signal_connect(conn, "channel-appeared", G_CALLBACK(channel_appeared_cb),
          NULL);

  if (sugar_connection_get_scope(conn) == SUGAR_SHARE_SCOPE_PRIVATE ||
          !sugar_connection_get_shared(conn) ||
          sugar_connection_get_initiator(conn))
    load_profile();
  else
    delayed_start = TRUE;
}

static void
finalize()
{
  if (client != NULL)
    administration_client_unref(client);
  client = NULL;

  if (!delayed_start)
    save_profile(profile);

  g_key_file_free(profile);
  profile = NULL;
}

static SugarJobject *
get_jobject(GcomprisBoard *board)
{
  SugarJobject *jobject = NULL;

  gchar *object_id = g_key_file_get_string(profile, "jobjects",
          board->name, NULL);
  if (object_id != NULL)
  {
    jobject = sugar_jobject_find(object_id);
    g_free(object_id);
  }

  if (jobject == NULL)
  {
    jobject = sugar_jobject_create();
    gchar *bundle_id = g_strdup_printf("net.gcompris.%s", board->name);
    sugar_jobject_set_activity(jobject, bundle_id);
    gchar *mime = g_strdup_printf("%s-%s", GC_MIME_TYPE, board->name);
    sugar_jobject_set_mime_type(jobject, mime);
    g_free(bundle_id);
    g_free(mime);
  }

  return jobject;
}

static void
save_jobject(SugarJobject *jobject)
{
  g_key_file_set_string(profile, "jobjects", gc_board_get_current()->name,
          sugar_jobject_get_uid(jobject));
}

static void
scope_changed_cb(SugarConnection *conn, SugarShareScope prev_scope,
        const gchar *error)
{
  if (sugar_connection_get_scope(conn) == SUGAR_SHARE_SCOPE_PRIVATE)
  {
    if (client != NULL)
      administration_client_unref(client);
    client = NULL;

    if (delayed_start)
    {
      load_profile();
      finalize_delayed_start();
    }
  }
}

static void
channel_appeared_cb(SugarConnection *conn, SugarChannel *channel)
{
  g_signal_connect(channel, "buddy-appeared",
          G_CALLBACK(buddy_appeared_cb), NULL);
  g_signal_connect(channel, "buddy-disappeared",
          G_CALLBACK(buddy_disappeared_cb), NULL);

  g_debug ("Wait for server");
}

static void
buddy_appeared_cb(SugarChannel *channel, guint buddy, const gchar *bus_name)
{
  if (client == NULL && sugar_channel_get_owner(channel) == buddy)
  {
    const gchar *address = sugar_channel_get_address(channel);
    client = administration_client_new(address, bus_name,
            get_profile_from_server_cb, NULL);

    g_debug ("Connected to server %s", bus_name);

    if (delayed_start)
      administration_client_import_profile(client);
  }
}

static void
buddy_disappeared_cb(SugarChannel *channel, guint buddy, const gchar *bus_name)
{
  if (client != NULL && sugar_channel_get_owner(channel) == buddy)
  {
    g_debug ("Disconnected from server %s", bus_name);
    administration_client_unref(client);
    client = NULL;
  }
}

static void
get_profile_from_server_cb(const char* profile_data, void* user_data)
{
  if (profile_data != NULL)
  {
    GError *error;
    if (!g_key_file_load_from_data(profile, profile_data, strlen(profile_data),
                G_KEY_FILE_NONE, &error))
    {
      profile_data = NULL;
      g_warning("Cannot import server profile: %s", error->message);
      g_error_free(error);
    }
  }

  if (profile_data == NULL)
  {
    SugarAlert *alert = SUGAR_ALERT(sugar_notify_alert_new(_("Sharing Error"),
              _("Cannot retrieve remote data"), "emblem-warning", 7));
    sugar_alert_bin_push(alert);

    SugarConnection *conn = sugar_activity_get_connection(activity);
    sugar_connection_set_scope(conn, SUGAR_SHARE_SCOPE_PRIVATE);
  }

  if (delayed_start)
  {
    if (profile_data == NULL)
      load_profile();
    else
      setup_gc_profile();
    finalize_delayed_start();
  }
}

static void
finalize_delayed_start()
{
  GcomprisProperties *properties = gc_prop_get();
  GcomprisBoard *board;

  g_debug("Delayed start");

  delayed_start = FALSE;

  gc_menu_load();
  gc_status_close();

  properties->menu_board = gc_menu_section_get("/");
  if (properties->profile == NULL || properties->profile->group_ids == NULL)
    board = gc_menu_section_get("/");
  else
    board = gc_menu_section_get("/login/login");
  gc_board_play(board);
}

static void
setup_gc_profile()
{
  int i, j;
  gchar **board_files = g_key_file_get_string_list(profile,
          "profile", "boards", NULL, NULL);

  if (!board_files)
      return;

  GcomprisProfile *gc_profile = g_malloc0(sizeof(GcomprisProfile));
  gc_prop_get()->profile = gc_profile;

  gc_profile->profile_id = 0;
  gc_profile->name = g_strdup("Sugar Profile");
  gc_profile->directory = g_strdup(sugar_environ_get_activity_root());
  gc_profile->description = g_strdup("Sugar Journal Object");
  gc_profile->boards = g_hash_table_new_full(g_direct_hash, g_direct_equal,
          NULL, g_free);
  gc_profile->groups = g_hash_table_new_full(g_direct_hash, g_direct_equal,
          NULL, (GDestroyNotify) gc_group_destroy);
  gc_profile->config = g_hash_table_new_full(g_direct_hash, g_direct_equal,
          NULL, (GDestroyNotify) g_hash_table_unref);

  load_board("common", -1);

  gint board_id = 0;
  g_hash_table_replace(gc_profile->boards,
          GINT_TO_POINTER(++board_id), g_strdup("menu.xml"));
  g_hash_table_replace(gc_profile->boards,
          GINT_TO_POINTER(++board_id), g_strdup("login.xml"));
  load_board("login.xml", board_id);

  for (i = 0; board_files[i]; i++)
  {
    g_hash_table_replace(gc_profile->boards,
            GINT_TO_POINTER(++board_id), board_files[i]);
    load_board(board_files[i], board_id);
  }

  g_free(board_files);

  gchar **sections = g_key_file_get_groups(profile, NULL);
  if (!sections)
    return;

  for (i = 0; sections[i]; i++)
  {
    if (!g_str_has_prefix(sections[i], "group/"))
      continue;

    GcomprisGroup *group = g_malloc0(sizeof(GcomprisGroup));
    group->group_id = g_key_file_get_integer(profile,
            sections[i], "group_id", NULL);
    group->name = g_key_file_get_string(profile,
            sections[i], "name", NULL);
    group->class_id = g_key_file_get_integer(profile,
            sections[i], "class_id", NULL);
    group->description = g_key_file_get_string(profile,
            sections[i], "description", NULL);

    int *group_id = g_malloc(sizeof(int));
    *group_id = group->group_id;
    gc_profile->group_ids = g_list_append(gc_profile->group_ids, group_id);

    g_hash_table_insert(gc_profile->groups,
            GINT_TO_POINTER(group->group_id), group);

    gchar **users = g_key_file_get_string_list(profile,
            sections[i], "users", NULL, NULL);
    if (users)
    {
      for (j = 0; users[j]; j++)
      {
        GcomprisUser *user = g_malloc0(sizeof(GcomprisUser));

        user->user_id = g_key_file_get_integer(profile,
                users[j], "user_id", NULL);
        user->login = g_key_file_get_string(profile,
                users[j], "login", NULL);
        user->lastname = g_key_file_get_string(profile,
                users[j], "lastname", NULL);
        user->firstname = g_key_file_get_string(profile,
                users[j], "firstname", NULL);
        user->birthdate = g_key_file_get_string(profile,
                users[j], "birthdate", NULL);
        user->class_id = g_key_file_get_integer(profile,
                users[j], "class_id", NULL);

        group->user_ids = g_list_append(group->user_ids, user);
      }
      g_strfreev(users);
    }
  }
  g_strfreev(sections);
}

static void
load_board(gchar *board_file, gint board_id)
{
  GHashTable *config = g_hash_table_new_full(g_direct_hash, g_direct_equal,
              g_free, g_free);

  gchar *board_section = g_strdup_printf("board/%s", board_file);
  gchar **keys = g_key_file_get_keys(profile, board_section, NULL, NULL);
  if (keys)
  {
    gchar **key;
    for (key = keys; *key; key++)
      g_hash_table_replace(config, *key, g_strdup(g_key_file_get_string(
                      profile, board_section, *key, NULL)));
    g_free(keys);
  }
  g_free(board_section);

  g_hash_table_replace(gc_prop_get()->profile->config,
          GINT_TO_POINTER(board_id), config);
}

static void
load_profile()
{
  SugarJobject *jobject = sugar_activity_get_jobject(activity);
  const char *file_path = sugar_jobject_get_file_path(jobject);

  if (file_path != NULL &&
          g_key_file_load_from_file(profile, file_path, 0, NULL))
      setup_gc_profile();
}
