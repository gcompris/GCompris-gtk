/* gcompris - sugar_gc.c
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

#include <polyol/toolkit.h>

#include "gcompris.h"
#include "gc_core.h"
#include "bar.h"
#include "score.h"
#include "status.h"
#include "sugar.h"

static void bar_start(GtkContainer*, GooCanvas*);
static void bar_set_level(GcomprisBoard*);
static void bar_set_flags(const GComprisBarFlags);
static void score_start(ScoreStyleList, guint, guint, guint);
static void score_end();
static void score_set(guint);
static void activity_clicked_cb(GtkToolButton*, gpointer);
static void about_clicked_cb(GtkToolButton*, gpointer);
static void level_clicked_cb(GtkToolButton*, gpointer);
static void refresh_clicked_cb(GtkToolButton*, gpointer);
static void zoom_clicked_cb(GtkToolButton*, gpointer);
static void config_clicked_cb(GtkToolButton*, gpointer);
static void back_clicked_cb(GtkToolButton*, gpointer);
static void stop_clicked_cb(GtkToolButton*, gpointer);
static GtkToolItem* level_widget_new();
static GtkToolItem* score_widget_new();
static GtkToolItem* expander_new();
static GtkToolItem* separator_new();
static GtkToolItem* activity_icon_new();
static GtkToolItem* journal_title_new();
static GtkToolItem* share_button_new();

/* export sugar bar */
Bar sugar_bar = {
  bar_start,
  bar_set_level,
  NULL,
  NULL,
  bar_set_flags,
  NULL
};

/* export sugar score */
Score sugar_score = {
  score_start,
  score_end,
  score_set
};

typedef struct {
  const gchar *icon;
  const gchar *label;
  gpointer cb;
  gpointer user_data;
} Button;

static Button buttons[] = {
  { NULL, NULL, activity_icon_new, NULL },
  { NULL, NULL, journal_title_new, NULL },
  { NULL, NULL, share_button_new, NULL },
  { "stock_home", N_("About"), about_clicked_cb, NULL },
  { "stock_refresh", N_("Refresh"), refresh_clicked_cb, NULL },
  { "preferences-system", N_("Settings"), config_clicked_cb, NULL },
  { "go-previous-paired", N_("Previous level"), level_clicked_cb,
      GINT_TO_POINTER(-1) },
  { NULL, NULL, level_widget_new, NULL },
  { "go-next-paired", N_("Next level"), level_clicked_cb,
      GINT_TO_POINTER(+1) },
  { NULL, NULL, expander_new, NULL },
  { NULL, NULL, score_widget_new, NULL },
  { NULL, NULL, expander_new, NULL },
  { "view-fullscreen", N_("Zoom"), zoom_clicked_cb, NULL },
  { NULL, NULL, separator_new, NULL },
  { "go-previous",
    /* TRANSLATORS: Back as in previous */
    N_("Back"), back_clicked_cb, NULL },
  { }
};

enum {
  BUTTON_ACTIVITY_ICON = 0,
  BUTTON_JOURNAL_TITLE,
  BUTTON_SHARE,
  BUTTON_ABOUT,
  BUTTON_REFRESH,
  BUTTON_CONFIG,
  BUTTON_PREV,
  BUTTON_LEVEL,
  BUTTON_NEXT,
  BUTTON_EXPANDER_1,
  BUTTON_SCORE,
  BUTTON_EXPANDER_2,
  BUTTON_ZOOM,
  BUTTON_SEPARATOR,
  BUTTON_BACK,
  BUTTON_STOP,
  BUTTONS_COUNT
};

static GtkToolItem *button_widgets[BUTTONS_COUNT];
static GtkToolbar *toolbar;
static SugarToolText *level_widget;
static SugarToolText *score_widget;
static gint current_level = -1;
static gint max_score;

static void
bar_start(GtkContainer *workspace, GooCanvas *theCanvas)
{
  GtkToolItem *item;

  SugarToolbarBox *toolbox = sugar_toolbar_box_new();
  gtk_widget_show(GTK_WIDGET(toolbox));
  gtk_box_pack_start(GTK_BOX(workspace), GTK_WIDGET(toolbox), FALSE, TRUE, 0);

  SugarAlertBin *alerts = sugar_alert_bin_new();
  gtk_widget_show(GTK_WIDGET(alerts));
  gtk_box_pack_start(GTK_BOX(workspace), GTK_WIDGET(alerts), FALSE, TRUE, 0);

  toolbar = GTK_TOOLBAR(sugar_toolbar_box_get_toolbar(toolbox));

  item = GTK_TOOL_ITEM(sugar_stop_button_new());
  g_signal_connect(item, "clicked", G_CALLBACK(stop_clicked_cb), NULL);
  g_object_ref_sink(item);
  button_widgets[BUTTON_STOP] = item;

  current_level = 1;
}

static void
set_button(gint number, gboolean visible)
{
  Button *button = &buttons[number];
  GtkToolItem *item = button_widgets[number];

  if (visible && item == NULL) {
    if (button->icon == NULL)
      item = ((GtkToolItem* (*)(void))button->cb)();
    else {
      SugarToolButton *tool_button = sugar_tool_button_new();
      sugar_tool_button_set_icon_name(tool_button, button->icon);
      sugar_tool_button_set_tooltip(tool_button, _(button->label));
      item = GTK_TOOL_ITEM(tool_button);
      g_signal_connect(item, "clicked", G_CALLBACK(button->cb),
                button->user_data);
    }

    g_object_ref_sink(item);
    button_widgets[number] = item;
  }

  if (visible) {
    if (gtk_widget_get_parent(GTK_WIDGET(item)) == NULL) {
      int i;
      int item_pos = 0;
      for (i = 0; i < number; ++i)
        if (button_widgets[i] != NULL &&
                gtk_widget_get_parent(GTK_WIDGET(button_widgets[i])) != NULL)
          ++item_pos;

      gtk_widget_show(GTK_WIDGET(item));
      gtk_toolbar_insert(toolbar, item, item_pos);
    }
  } else if (item) {
    if (gtk_widget_get_parent(GTK_WIDGET(item)))
      gtk_container_remove(GTK_CONTAINER(toolbar), GTK_WIDGET(item));
  }
}

static void
update_level()
{
  gchar level_string[256];
  snprintf(level_string, sizeof(level_string), "%d", current_level);
  g_object_set(level_widget, "text", level_string, NULL);
}

static void
update_score(guint value)
{
  gchar score_string[256];
  snprintf(score_string, sizeof(score_string), "%d/%d", value, max_score);
  g_object_set(score_widget, "text", score_string, NULL);
}

static GtkToolItem*
level_widget_new()
{
  g_assert(level_widget == NULL);
  level_widget = sugar_tool_text_new();
  update_level();
  return GTK_TOOL_ITEM(level_widget);
}

static GtkToolItem*
score_widget_new()
{
  g_assert(score_widget == NULL);
  score_widget = sugar_tool_text_new();
  return GTK_TOOL_ITEM(score_widget);
}

static GtkToolItem*
separator_new()
{
  GtkToolItem *separator = gtk_separator_tool_item_new();
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(separator), FALSE);
  return separator;
}

static GtkToolItem*
expander_new()
{
  GtkToolItem *expander = gtk_separator_tool_item_new();
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(expander), FALSE);
  gtk_tool_item_set_expand(GTK_TOOL_ITEM(expander), TRUE);
  return expander;
}

static void
beep()
{
  gc_sound_play_ogg("sounds/bleep.wav", NULL);
}

static void
activity_clicked_cb(GtkToolButton *button, gpointer user_data)
{
  beep();
  GcomprisBoard *board = gc_board_get_current();
  if (gc_help_has_board(board))
    gc_help_start(board);
}

static void
level_clicked_cb(GtkToolButton *button, gpointer user_data)
{
  beep();

  GcomprisBoard *board = gc_board_get_current();
  if (board == NULL)
      return;

  gint delta = GPOINTER_TO_INT(user_data);
  current_level += delta;

  if (current_level > board->maxlevel)
    current_level = 1;
  else if (current_level < 1)
    current_level = board->maxlevel;

  update_level();

  if (board->plugin->set_level != NULL)
    board->plugin->set_level(current_level);

  gc_bar_play_level_voice(current_level);
}

static void
refresh_clicked_cb(GtkToolButton *button, gpointer user_data)
{
  beep();
  GcomprisBoard *board = gc_board_get_current();
  if(board && board->plugin->repeat != NULL)
    board->plugin->repeat();
}

static void
zoom_clicked_cb(GtkToolButton *button, gpointer user_data)
{
  beep();
  GcomprisProperties *properties = gc_prop_get();
  properties->zoom = (properties->zoom ? 0 : 1);
  gc_update_canvas_zoom();
}

static void
config_clicked_cb(GtkToolButton *button, gpointer user_data)
{
  beep();
  GcomprisBoard *board = gc_board_get_current();
  if(board && board->plugin->config_start != NULL)
    board->plugin->config_start(board, gc_profile_get_current());
}

static void
back_clicked_cb(GtkToolButton *button, gpointer user_data)
{
  gc_board_stop();
}

static void
stop_clicked_cb(GtkToolButton *button, gpointer user_data)
{
  // save zoom setting
  GcomprisProperties *properties = gc_prop_get();
  gc_prop_save(properties);

  gc_exit();
}

static void
bar_set_flags(const GComprisBarFlags flags)
{
  GcomprisBoard *board = gc_board_get_current();
  gboolean sub_activity = board && board->previous_board;

  if (switch_board_jobject() && flags & GC_BAR_JOURNAL)
  {
    set_button(BUTTON_JOURNAL_TITLE, FALSE);
    if (button_widgets[BUTTON_JOURNAL_TITLE])
      g_object_unref(G_OBJECT(button_widgets[BUTTON_JOURNAL_TITLE]));
    button_widgets[BUTTON_JOURNAL_TITLE] = NULL;
  }

  set_button(BUTTON_ACTIVITY_ICON, FALSE);
  if (button_widgets[BUTTON_ACTIVITY_ICON])
    g_object_unref(G_OBJECT(button_widgets[BUTTON_ACTIVITY_ICON]));
  button_widgets[BUTTON_ACTIVITY_ICON] = NULL;

  set_button(BUTTON_ACTIVITY_ICON, TRUE);
  set_button(BUTTON_JOURNAL_TITLE, flags & GC_BAR_JOURNAL);
  set_button(BUTTON_SHARE, sugar_activity_get_connection(activity) != NULL &&
          flags & GC_BAR_SHARE);
  set_button(BUTTON_ABOUT, flags & GC_BAR_ABOUT);

  set_button(BUTTON_PREV, flags & GC_BAR_LEVEL);
  set_button(BUTTON_LEVEL, flags & GC_BAR_LEVEL);
  set_button(BUTTON_NEXT, flags & GC_BAR_LEVEL);

  set_button(BUTTON_REFRESH, flags & (GC_BAR_REPEAT | GC_BAR_REPEAT_ICON));
  set_button(BUTTON_CONFIG, flags & GC_BAR_CONFIG);

  set_button(BUTTON_EXPANDER_1, TRUE);
  set_button(BUTTON_SCORE, max_score);
  set_button(BUTTON_EXPANDER_2, TRUE);

  set_button(BUTTON_ZOOM, strcmp("/", gc_prop_get()->root_menu) != 0 &&
          strcmp("/login/login", gc_prop_get()->root_menu) != 0);
  set_button(BUTTON_SEPARATOR, TRUE);

  set_button(BUTTON_BACK, sub_activity);
  set_button(BUTTON_STOP, !sub_activity);
}

static void
score_start(ScoreStyleList style, guint x, guint y, guint max)
{
  max_score = max;
  set_button(BUTTON_SCORE, TRUE);
  update_score(0);
}

static void
score_end()
{
  max_score = 0;
  set_button(BUTTON_SCORE, FALSE);
}

static void
score_set(guint value)
{
  update_score(value);
}

static void
bar_set_level(GcomprisBoard *board)
{
  if (board == NULL)
      return;

  if (level_widget == NULL) {
    g_message("in bar_set_level, level_widget == NULL should not happen\n");
    return;
  }

  current_level = board->level;
  update_level();
}

static GtkToolItem*
activity_icon_new()
{
  SugarActivityButton *item = sugar_activity_button_new(board_jobject);
  if (is_activity_board())
  {
    gchar *icon_file = g_strdup_printf("%s/%s",
            gc_prop_get()->menu_dir, gc_board_get_current()->icon_name);
    sugar_tool_button_set_icon_name(SUGAR_TOOL_BUTTON(item), icon_file);
    g_free(icon_file);
  }
  g_signal_connect(item, "clicked", G_CALLBACK(activity_clicked_cb), NULL);
  return GTK_TOOL_ITEM(item);
}

static GtkToolItem*
journal_title_new()
{
  GtkToolItem *item;
  item = GTK_TOOL_ITEM(sugar_title_entry_new(board_jobject));
  return item;
}

static GtkToolItem*
share_button_new()
{
  GtkToolItem *item;
  SugarConnection *connection = sugar_activity_get_connection(activity);
  item = GTK_TOOL_ITEM(sugar_share_button_new(connection));
  return item;
}

static void
about_clicked_cb(GtkToolButton *button, gpointer user_data)
{
  beep();
  gc_about_start();
}
