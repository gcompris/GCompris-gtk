/* gcompris - sugar_srv.c
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <polyol/toolkit.h>

#include "gcompris.h"
#include "gc_core.h"
#include "status.h"
#include "sugar.h"
#include "sugar_db.h"
#include "sugar_share.h"

static void construct();
static void finalize();
static void scope_changed_cb(SugarConnection*, SugarShareScope, const gchar*);
static void channel_appeared_cb(SugarConnection*, SugarChannel*);
static char* get_profile_request_cb(void*);
static void report_cb (const char*, gint, const char*, const char*, gint, gint,
        gint, const char*, void*);
static void offer_failed_cb(SugarConnection*, const gchar *bus_name);

static GKeyFile *export_profile;
static gint imported_profile = 1;
static AdministrationServer *server;
static time_t last_db_mtime;

Peer srv_peer = {
  construct,
  finalize,
  NULL,
  NULL
};

gint
sugar_get_profile_id()
{
  return imported_profile;
}

static void
construct()
{
  export_profile = g_key_file_new();

  SugarConnection *conn = sugar_activity_get_connection(activity);

  g_signal_connect(conn, "scope-changed", G_CALLBACK(scope_changed_cb), NULL);
  g_signal_connect(conn, "channel-appeared", G_CALLBACK(channel_appeared_cb),
          NULL);
  g_signal_connect(conn, "offer-failed", G_CALLBACK(offer_failed_cb), NULL);

  sugar_db_open();

  GKeyFile *profile = g_key_file_new();
  SugarJobject *jobject = sugar_activity_get_jobject(activity);
  const char *file_path = sugar_jobject_get_file_path(jobject);

  if (file_path)
    g_key_file_load_from_file(profile, file_path, 0, NULL);
  imported_profile = sugar_db_write(profile);

  g_key_file_free(profile);
}

static void
finalize()
{
  GKeyFile *profile = g_key_file_new();
  sugar_db_read(profile, imported_profile);
  sugar_db_read_reports(profile);
  save_profile(profile);
  g_key_file_free(profile);

  if (server != NULL)
    administration_server_unref(server);
  server = NULL;

  g_key_file_free(export_profile);
  export_profile = NULL;

  sugar_db_close(imported_profile);
}

static void
scope_changed_cb(SugarConnection *conn, SugarShareScope prev_scope,
        const gchar *error)
{
  if (sugar_connection_get_scope(conn) == SUGAR_SHARE_SCOPE_PRIVATE)
  {
    if (server != NULL)
      administration_server_unref(server);
    server = NULL;
  }
  else
  {
    sugar_connection_offer_channel(conn, "org.gcompris.Administration.Channel");
    g_debug ("Sugar channel creation was initiated");
  }
}

static void
channel_appeared_cb(SugarConnection *conn, SugarChannel *channel)
{
  const gchar *address = sugar_channel_get_address(channel);
  server = administration_server_new(address,
          get_profile_request_cb, NULL, report_cb, NULL);

  g_debug ("Server sugar channel is online");
}

static char*
get_profile_request_cb(void* user_data)
{
  struct stat st = { };
  if ((stat(gc_prop_get()->database, &st) == 0 && st.st_mtime != last_db_mtime)
          || last_db_mtime == 0)
  {
    sugar_db_read(export_profile, imported_profile);
    last_db_mtime = st.st_mtime;
  }

  return g_key_file_to_data(export_profile, NULL, NULL);
}

static void
report_cb (const char *date, gint duration, const char *user, const char *board,
    gint level, gint sublevel, gint status, const char *comment,
    void *user_data)
{
  sugar_db_write_report(date, duration, user, board, level, sublevel, status,
          comment);
}

static void
offer_failed_cb(SugarConnection *conn, const gchar *bus_name)
{
  SugarAlert *alert = SUGAR_ALERT(sugar_notify_alert_new(D_(GETTEXT_ERRORS,"Sharing Error"),
            D_(GETTEXT_ERRORS,"Cannot initiate sharing session"), "emblem-warning", 7));
  sugar_alert_bin_push(alert);
  sugar_connection_set_scope(conn, SUGAR_SHARE_SCOPE_PRIVATE);
}
