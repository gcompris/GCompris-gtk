/* gcompris - sugar.c
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

#include <sys/stat.h>
#include <unistd.h>
#include <gdk/gdkx.h>
#include <dbus/dbus-glib.h>

#include <polyol/toolkit.h>

#include "gcompris.h"
#include "gc_core.h"
#include "bar.h"
#include "score.h"
#include "status.h"
#include "sugar.h"
#include "sugar_db.h"
#include "sugar_share.h"

#define JOURNAL_PREFIX "journal:"

static const gchar *jobject_mime_types[][2] = {
  {GC_MIME_TYPE "-wordprocessor", "/fun/wordprocessor"},
  {GC_MIME_TYPE "-anim", "/fun/anim"},
  {GC_MIME_TYPE "-draw", "/fun/draw"},
  {NULL, NULL}
};

static void notify_active_cb(GObject*);
static void chooser_response_cb(GObject*, const gchar*);

SugarActivity *activity;
SugarJobject *board_jobject;

static SugarJobject *journal_file_object;
static ImageSelectorCallBack choose_image_cb;
static void * choose_image_user_context;
static const gchar *jobject_mime_type;

extern Peer srv_peer;
extern Peer cli_peer;
static Peer empty_peer;
static Peer *peer = &empty_peer;
static gboolean initial_activate = TRUE;

extern Bar sugar_bar;
extern Score sugar_score;

void
sugar_setup(int *argc, char ***argv)
{
  if (!sugar_init(argv, argc))
    return;

  g_debug("Use sugar mode");

  if (strcmp("net.gcompris.administration",
              sugar_environ_get_bundle_id()) == 0)
  {
    /* There should be only one bundle_id for GCompris objects in
       non-MODE_STANDALONE_ACTIVITY mode to let teacher via Administration
       activity and students via GCompris activity share the same objects */
    (*argv)[(*argc)++] = "-b";
    (*argv)[(*argc)++] = "net.gcompris";
    (*argv)[*argc] = NULL;
    sugar_init(argv, argc);
  }

  if (!g_thread_supported())
    g_thread_init(NULL);
  dbus_g_thread_init ();

  sugar_environ_set_sync_dbus(TRUE);

  const gchar *resumed_jobject_id = sugar_environ_get_object_id();
  if (resumed_jobject_id)
  {
    SugarJobject *resumed_jobject = sugar_jobject_find(resumed_jobject_id);
    if (resumed_jobject)
    {
      const gchar *mime_type = sugar_jobject_get_mime_type(resumed_jobject);
      const gchar *(*i)[2];
      for (i = jobject_mime_types; (*i)[0]; ++i)
      {
        if (strcmp((*i)[0], mime_type) == 0)
        {
          jobject_mime_type = (*i)[1];
          g_debug("Start activity %s", jobject_mime_type);
          break;
        }
      }
    }
  }
}

gboolean
sugar_detected()
{
  return sugar_environ_get_initialized();
}

const gchar *
sugar_jobject_root_menu(void)
{
  return jobject_mime_type;
}

void
sugar_setup_profile(const gchar *root, gboolean administration)
{
  if (!sugar_detected())
    return;

  activity = sugar_activity_new (TRUE, !jobject_mime_type);

  g_debug("Setup sugar profile root=%s administration=%d ",
          root, administration);

  SugarJobject *jobject = sugar_activity_get_jobject(activity);

  if (!sugar_activity_get_resumed(activity))
  {
    if (!jobject_mime_type)
      sugar_jobject_set_mime_type(jobject, GC_MIME_TYPE);
    else
    {
      gchar *activity_name = strrchr(root, '/') + 1;
      gchar *mime = g_strdup_printf("%s-%s", GC_MIME_TYPE, activity_name);
      sugar_jobject_set_mime_type(jobject, mime);
      g_free(mime);
    }
  }

  if (!jobject_mime_type)
  {
    if (administration)
      peer = &srv_peer;
    else
      peer = &cli_peer;
  }

  g_signal_connect(sugar_activity_get_shell(activity), "notify::active",
          G_CALLBACK(notify_active_cb), NULL);
  g_signal_connect(sugar_activity_get_journal(activity), "chooser-response",
          G_CALLBACK(chooser_response_cb), NULL);

  gc_bar_register(&sugar_bar);
  gc_score_register(&sugar_score);

  if (peer->construct)
    peer->construct();
}

void
sugar_setup_x11()
{
  GtkWidget *window = gc_get_window();
  Window xwindow = GDK_WINDOW_XWINDOW(window->window);
  sugar_environ_set_window(GDK_DISPLAY(), xwindow);
}

void
sugar_cleanup()
{
  if (!sugar_detected())
    return;

  g_debug("Cleanup sugar mode");

  if (peer->finalize)
    peer->finalize();

  if (board_jobject != NULL)
    g_object_unref(G_OBJECT(board_jobject));
  board_jobject = NULL;

  if(journal_file_object != NULL)
    g_object_unref(G_OBJECT(journal_file_object));
  journal_file_object = NULL;

  g_object_unref(G_OBJECT(activity));
  activity = NULL;
}

const char *
sugar_load(void)
{
  if (!sugar_detected())
    return NULL;

  switch_board_jobject();

  const char *file_path = sugar_jobject_get_file_path(board_jobject);

  if (file_path != NULL)
    g_debug("Load file from %s", sugar_jobject_get_uid(board_jobject));

  return file_path;
}

void
sugar_save(const char *path)
{
  GError *error = NULL;
  GArray *preview = NULL;

  if (!sugar_detected())
    return;

  switch_board_jobject();

  preview = sugar_get_preview(&error);
  if (preview != NULL)
  {
    sugar_jobject_set_preview(board_jobject, preview);
    g_array_free(preview, FALSE);
  } else {
    g_warning("Cannot get preview: %s", error->message);
    g_error_free(error);
  }

  sugar_jobject_write_file(board_jobject, path, TRUE);

  if (peer->save_jobject)
    peer->save_jobject(board_jobject);

  g_warning("%p Saved %s file to journal entry %s", board_jobject, path,
          sugar_jobject_get_uid(board_jobject));
}

void
sugar_choose_image(ImageSelectorCallBack iscb, void *user_context)
{
  choose_image_cb = iscb;
  choose_image_user_context = user_context;
  sugar_journal_choose_object(sugar_activity_get_journal(activity),
          SUGAR_MIME_IMAGE);
}

gchar *
sugar_get_journal_file(gchar *file_id)
{
  if(!g_str_has_prefix(file_id, JOURNAL_PREFIX))
    return file_id;

  gchar *file_path = NULL;
  const gchar *object_id = file_id + strlen(JOURNAL_PREFIX);

  if(journal_file_object != NULL)
    g_object_unref(G_OBJECT(journal_file_object));
  journal_file_object = sugar_jobject_find(object_id);

  if(journal_file_object == NULL)
    g_warning("Cannot find jobject %s", object_id);
  else
  {
    file_path = g_strdup(sugar_jobject_get_file_path(journal_file_object));
    if(file_path == NULL)
      g_warning("Cannot get file from jobject %s", object_id);
  }

  g_free(file_id);
  return file_path;
}

static void
notify_active_cb(GObject *sender)
{
  SugarShell *shell = sugar_activity_get_shell(activity);
  gboolean active = sugar_shell_get_active(shell);

  g_debug("SugarShell.active=%d", active);

  if(!active)
    gc_sound_close();
  else if(initial_activate == FALSE)
    gc_sound_reopen();

  initial_activate = FALSE;
}

static void
chooser_response_cb(GObject *sender, const gchar *object_id)
{
  g_debug("SugarShell.chooser_response_cb=%s", object_id);

  if(choose_image_cb != NULL && object_id != NULL)
  {
    gchar file_id[256];
    snprintf(file_id, sizeof(file_id), "%s%s", JOURNAL_PREFIX, object_id);
    choose_image_cb(file_id, choose_image_user_context);
    choose_image_cb = NULL;
  }
}

gboolean
switch_board_jobject()
{
  static GcomprisBoard *board = NULL;

  if (board == gc_board_get_current())
    return FALSE;

  board = gc_board_get_current();
  SugarJobject *new_board_jobject = NULL;

  if (peer->get_jobject && is_activity_board())
    new_board_jobject = peer->get_jobject(board);
  else
  {
    new_board_jobject = sugar_activity_get_jobject(activity);
    g_object_ref(G_OBJECT(new_board_jobject));
  }

  gboolean result = (new_board_jobject != board_jobject);
  if (board_jobject)
    g_object_unref(board_jobject);
  board_jobject = new_board_jobject;

  return result;
}

void
save_profile(GKeyFile *profile)
{
  char tmp_file[] = "/tmp/GComprisXXXXXX";
  int fd = mkstemp(tmp_file);
  if (fd == -1)
  {
    g_warning("Cannot create temporary file to save");
    return;
  }

  gsize size = 0;
  gchar *profile_data = g_key_file_to_data(profile, &size, NULL);
  gboolean success = (write(fd, profile_data, size) == size);
  g_free(profile_data);
  fchmod(fd, 0644);
  close(fd);

  if (success)
    sugar_activity_write_file(activity, tmp_file, TRUE);
  else
  {
    unlink(tmp_file);
    g_warning("Cannot save profile to journal entry");
  }
}

gboolean
is_activity_board()
{
  GcomprisBoard *board = gc_board_get_current();
  return board->name[0] != '\0' && strcmp("login", board->name) != 0 &&
        strcmp("administration", board->name) != 0;
}
