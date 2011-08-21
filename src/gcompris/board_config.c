/* gcompris - board_config.c
 *
 * Copyright (C) 2001, 2008 Pascal Georges
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
#include "gcompris.h"
#include "board_config_common.h"

static GcomprisBoard *config_board;

void
gc_board_config_start(GcomprisBoard *aBoard, GcomprisProfile *aProfile)
{

  if (aBoard->plugin == NULL)
    gc_board_check_file(aBoard);

  if (aBoard->plugin == NULL){
    g_warning("gc_board_config_start: board %s/%s is not initialised ? Hummmm...",
	      aBoard->section, aBoard->name);
    return;
  }

  if (aBoard->plugin->config_start == NULL) {
    g_warning("Trying to configure board %s/%s without config_start",
	      aBoard->section, aBoard->name);
    return;
  }

  config_board = aBoard;

  aBoard->plugin->config_start(aBoard, aProfile);
  return;
}



void
gc_board_config_stop()
{
  if (!config_board)
    return;

  config_board->plugin->config_stop();

  config_board = NULL;
  return;
}

void
gc_board_conf_close (GtkDialog *dialog,
		     gpointer   user_data)
{
  GcomprisBoardConf *u= (GcomprisBoardConf*)user_data;

  gtk_object_destroy(GTK_OBJECT(dialog));
  g_hash_table_destroy (u->hash_conf);
  u->hash_conf = NULL;

  /* in case we close without response */
  if (u->Confcallback){
    u->Confcallback(NULL);
    u->Confcallback = NULL;
  }
  gc_bar_hide(FALSE);
  g_free(u);
}

void
_response_board_conf (GtkButton *button,
		      gint arg1,
		      gpointer user_data)
{
  GcomprisBoardConf *u= (GcomprisBoardConf*)user_data;

  if (u->Confcallback){

    switch (arg1){
    case GTK_RESPONSE_APPLY:
      /* Do not close the config dialog if the user returns FALSE */
      if ( ! u->Confcallback(u->hash_conf) )
	return;
      break;
    case GTK_RESPONSE_CANCEL:
      u->Confcallback(NULL);
      break;
    case GTK_RESPONSE_NONE:
      u->Confcallback(NULL);
      break;
    default:
      u->Confcallback(NULL);
      break;
    }

    u->Confcallback = NULL;
  }

  gc_board_conf_close (GTK_DIALOG(u->conf_window), u);

}

#ifdef XF86_VIDMODE
static GdkEventConfigure last_configure_event;

static gint
_conf_window_configured(GtkWindow *window,
			GdkEventConfigure *event, gpointer param)
{
  GcomprisBoardConf *u= (GcomprisBoardConf*)param;
  gint new_x, new_y;
  double screen_width, screen_height;
  /* Because we call gtk_window_move, we cause a configure event. Filter out
     identical events to avoid looping. */
  if (memcmp(&last_configure_event, event, sizeof(GdkEventConfigure)))
  {
    gnome_canvas_get_scroll_region( GNOME_CANVAS( gtk_bin_get_child( GTK_BIN(
      gc_get_window()))), NULL, NULL, &screen_width, &screen_height);
    /* strange but gcompris.c sets the scrollheight to screen_height + 30 */
    screen_height -= 30;
    new_x = ((gint)screen_width - event->width) / 2;
    new_y = ((gint)screen_height - event->height) / 2;
    /* printf("screen %dx%d, window %dx%d, place %dx%d\n", (int)screen_width, (int)screen_height, event->width, event->height, new_x, new_y); */
    gtk_window_move (u->conf_window, new_x, new_y);
    memcpy(&last_configure_event, event, sizeof(GdkEventConfigure));
  }

  /* Act as if we aren't there / aren't hooked up */
  return FALSE;
}
#endif

GcomprisBoardConf *
gc_board_config_window_display(gchar *label, GcomprisConfCallback callback)
{
  GtkWidget *header;
  GcomprisBoardConf *config;

  config = g_malloc0(sizeof(GcomprisBoardConf));

  /* init static values or callbacks */
  config->Confcallback = callback;
  config->hash_conf = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

  gc_bar_hide(TRUE);

  /* Creating a config window will cause our main window to loose focus,
     this tells the main window to ignore the next focus out event (and thus
     stay in fullscreen mode if we're fullscreen). */

  /* main configuration window */
  config->conf_window = \
    GTK_WINDOW(gtk_dialog_new_with_buttons ("GCompris",
					    GTK_WINDOW(gtk_widget_get_toplevel (GTK_WIDGET(gc_board_get_current()->canvas))),
					    GTK_DIALOG_DESTROY_WITH_PARENT,
					    GTK_STOCK_CANCEL,
					    GTK_RESPONSE_CANCEL,
					    GTK_STOCK_APPLY,
					    GTK_RESPONSE_APPLY,
					    NULL));


  /* parameters */
  gtk_window_set_position (config->conf_window,
			   GTK_WIN_POS_CENTER_ALWAYS);

  gtk_widget_show(GTK_WIDGET(config->conf_window));

  /* main vbox in window */
  config->main_conf_box = GTK_VBOX(GTK_DIALOG(config->conf_window)->vbox);

  g_signal_connect(G_OBJECT(config->conf_window),
		   "response",
		   G_CALLBACK(_response_board_conf),
		   config);

  g_signal_connect (G_OBJECT(config->conf_window),
		    "close",
		    G_CALLBACK(gc_board_conf_close),
		    config);



  /* Label header */
  header = gtk_label_new ((gchar *)NULL);
  gtk_widget_show(header);
  gtk_box_pack_start (GTK_BOX(config->main_conf_box),
		      header,
		      FALSE,
		      FALSE,
		      0);

  gtk_label_set_justify (GTK_LABEL(header),
			 GTK_JUSTIFY_CENTER);

  gchar *label_markup = NULL;
  label_markup = g_strdup_printf("<span size='large'>%s</span>",label);
  gtk_label_set_markup (GTK_LABEL(header),
                        (const gchar *)label_markup);
  g_free(label_markup);

  gc_board_conf_separator(config);

  return config;
}

void
gc_board_conf_boolean_box_toggled (GtkToggleButton *togglebutton,
			      gpointer key)
{
  _gc_boardconf_key *u = (_gc_boardconf_key*)key;
  gchar *the_key = g_strdup(u->key);
  gchar *value;

  if (gtk_toggle_button_get_active (togglebutton))
    value = g_strdup("True");
  else
    value = g_strdup("False");

  g_hash_table_replace(u->config->hash_conf, (gpointer) the_key, (gpointer) value);
}

GtkCheckButton *
gc_board_config_boolean_box(GcomprisBoardConf *config, const gchar *label, gchar *key, gboolean initial_value)
{
  _gc_boardconf_key *user_data;

  g_return_val_if_fail(config, NULL);
  check_key( key);

  GtkWidget *CheckBox = gtk_check_button_new_with_label (label);

  gtk_widget_show(CheckBox);

  gtk_box_pack_start (GTK_BOX(config->main_conf_box),
		      CheckBox,
		      FALSE,
		      FALSE,
		      0);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(CheckBox),
				initial_value);

  user_data = g_malloc0(sizeof(_gc_boardconf_key));
  user_data -> key =g_strdup(key);
  user_data -> config = config;

  g_signal_connect   (G_OBJECT(CheckBox),
		      "toggled",
		      G_CALLBACK(gc_board_conf_boolean_box_toggled),
		      user_data);
  g_signal_connect  (G_OBJECT(CheckBox),
  			"destroy",
			G_CALLBACK(_gc_destroy_boardconf_key),
			user_data);
  return GTK_CHECK_BUTTON(CheckBox);
}

static void
spin_changed (GtkSpinButton *spinbutton,
	      gpointer data)
{
  _gc_boardconf_key *u = (_gc_boardconf_key*)data;

  gchar *h_key = g_strdup(u->key);
  gchar *h_value = g_strdup_printf("%d",gtk_spin_button_get_value_as_int (spinbutton));

  g_hash_table_replace (u->config->hash_conf, h_key, h_value);
}

GtkSpinButton *
gc_board_config_spin_int(GcomprisBoardConf *config, const gchar *label, gchar *key, gint min, gint max, gint step, gint init)
{
  g_return_val_if_fail(config, NULL);
  check_key( key);

  GtkWidget *spin;
  GtkWidget *hbox = gtk_hbox_new (FALSE, 8);
  GtkWidget *label_spin;

  gtk_widget_show(hbox);

  gtk_box_pack_start (GTK_BOX(config->main_conf_box),
		      hbox,
		      FALSE,
		      FALSE,
		      0);

  /* Label */
  label_spin = gtk_label_new ((gchar *)NULL);
  gtk_widget_show(label_spin);
  gtk_box_pack_start (GTK_BOX(hbox),
		      label_spin,
		      FALSE,
		      FALSE,
		      0);

  gtk_label_set_justify (GTK_LABEL(label_spin),
			 GTK_JUSTIFY_RIGHT);

  gtk_label_set_markup (GTK_LABEL(label_spin),
                        (const gchar *)label);

  spin = gtk_spin_button_new_with_range  ((gdouble )min,
					  (gdouble )max,
                                          (gdouble )step);
  gtk_widget_show(spin);
  gtk_box_pack_start (GTK_BOX(hbox),
		      spin,
		      FALSE,
		      FALSE,
		      0);

  gtk_spin_button_set_wrap ( GTK_SPIN_BUTTON(spin), TRUE);
  gtk_spin_button_set_numeric ( GTK_SPIN_BUTTON(spin), TRUE);
  gtk_spin_button_set_digits ( GTK_SPIN_BUTTON(spin), 0);

  gtk_spin_button_set_value ( GTK_SPIN_BUTTON(spin), (gdouble) init);

  _gc_boardconf_key *u = g_malloc0(sizeof(_gc_boardconf_key));
  u->key = g_strdup(key);
  u->config = config;

  g_signal_connect (G_OBJECT(spin),
		    "value-changed",
		    G_CALLBACK(spin_changed),
		    u);
  g_signal_connect( G_OBJECT(spin),
	  	"destroy",
		G_CALLBACK(_gc_destroy_boardconf_key),
		u);

  return GTK_SPIN_BUTTON(spin);

}

GtkHSeparator *
gc_board_conf_separator(GcomprisBoardConf *config)
{
  g_return_val_if_fail(config, NULL);
  GtkWidget *separator = gtk_hseparator_new ();

  gtk_widget_show(separator);

  gtk_box_pack_start (GTK_BOX(config->main_conf_box),
		      separator,
		      FALSE,
		      FALSE,
		      8);

  return GTK_HSEPARATOR(separator);

}

