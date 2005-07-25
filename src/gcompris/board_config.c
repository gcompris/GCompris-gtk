/* gcompris - board_config.c
 *
 * Time-stamp: <2005/07/25 16:23:19 yves>
 *
 * Copyright (C) 2001 Pascal Georges
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

#include "gcompris.h"


static GcomprisBoard *config_board;

void	 board_config_start(GcomprisBoard *aBoard, GcomprisProfile *aProfile)
{

/*   if (config_board){ */
/*     g_warning("board_config_start Cannot configure two boards at same time !\n Call config_end on previous board before !"); */
/*     return; */
/*   } */

  if (aBoard->plugin == NULL){
    g_warning("board_config_start: board %s/%s is not initialised ? Hummmm...", aBoard->section,aBoard->name);
    return;
  }

  if (aBoard->plugin->config_start == NULL) {
    g_warning("Trying to configure board %s/%s without config_start", aBoard->section,aBoard->name);
    return;
  }

  config_board = aBoard;

  aBoard->plugin->config_start(aBoard, aProfile);
  return;
}


  
void	 board_config_stop()
{
  if (!config_board)
    return;

  config_board->plugin->config_stop();

  config_board = NULL;
  return;
}

static GtkWindow *conf_window = NULL;
static GtkVBox *main_conf_box = NULL;
static GHashTable *hash_conf = NULL;
static GcomprisConfCallback Confcallback = NULL;


void gcompris_close_board_conf (GtkButton *button,
				gpointer user_data)
{
  gtk_object_destroy              ((GtkObject *)conf_window);
  g_hash_table_destroy (hash_conf);
  hash_conf = NULL;
 
}

void gcompris_apply_board_conf (GtkButton *button,
				gpointer user_data)
{
  if (Confcallback != NULL)
    Confcallback(hash_conf);
}

GtkVBox *gcompris_configuration_window(gchar *label, GcomprisConfCallback callback)
{
  GtkButton *button;
  GtkHBox   *footer;
  GtkLabel  *header;

  /* init static values or callbacks */
  Confcallback = callback;
  hash_conf = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

  /* main configuration window */
  conf_window = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));

  /* parameters */
  gtk_window_set_default_size     (conf_window,
				   320,
				   300); 
  gtk_window_set_transient_for(conf_window,
			       GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(get_current_gcompris_board()->canvas))));
  gtk_window_set_modal(conf_window, TRUE);
  gtk_widget_show(GTK_WIDGET(conf_window));

  /* main vbox in window */
  main_conf_box = GTK_VBOX(gtk_vbox_new ( FALSE, 0));
  gtk_widget_show(GTK_WIDGET(main_conf_box));
  gtk_container_add(GTK_CONTAINER(conf_window), GTK_WIDGET(main_conf_box));

  /* hbox for apply and close buttons */
  footer = GTK_HBOX(gtk_hbox_new (FALSE, 0));
  gtk_widget_show(GTK_WIDGET(footer));
  gtk_box_pack_end (GTK_BOX(main_conf_box),
		    GTK_WIDGET(footer),
		    FALSE,
                    FALSE,
		    0);

  /* Close button */
  button = GTK_BUTTON(gtk_button_new_from_stock(GTK_STOCK_CLOSE));
  gtk_widget_show(GTK_WIDGET(button));
  gtk_box_pack_end (GTK_BOX(footer),
		    GTK_WIDGET(button),
		    FALSE,
		    FALSE,
		    0);
  g_signal_connect(G_OBJECT(button), 
		   "clicked",
		   G_CALLBACK(gcompris_close_board_conf),
		   NULL);

  /* Apply button */
  button = GTK_BUTTON(gtk_button_new_from_stock(GTK_STOCK_APPLY));
  gtk_widget_show(GTK_WIDGET(button));
  gtk_box_pack_start (GTK_BOX(footer),
		      GTK_WIDGET(button),
		      FALSE,
		      FALSE,
		      0);
  g_signal_connect                (G_OBJECT(button),
				   "clicked",
                                   G_CALLBACK(gcompris_apply_board_conf),
				   NULL);


  /* Label header */
  header = gtk_label_new (NULL);
  gtk_widget_show(header);
  gtk_box_pack_start (GTK_BOX(main_conf_box),
		      header,
		      FALSE,
		      FALSE,
		      0);

  gtk_label_set_justify (GTK_LABEL(header),
			 GTK_JUSTIFY_CENTER);

  gtk_label_set_markup (GTK_LABEL(header),
                        (const gchar *)label);

  return main_conf_box;
}

void gcompris_boolean_box_toggled (GtkToggleButton *togglebutton,
				   gpointer key)
{
  gchar *the_key = g_strdup((gchar *)key);
  gchar *value;

  if (gtk_toggle_button_get_active (togglebutton))
    value = g_strdup("True");
  else
    value = g_strdup("False");
  
  g_hash_table_replace(hash_conf, (gpointer) the_key, (gpointer) value);
}

GtkCheckButton *gcompris_boolean_box(const gchar *label, gchar *key, gboolean initial_value)
{
  GtkWidget *CheckBox = gtk_check_button_new_with_label (label);

  gtk_widget_show(CheckBox);

  gtk_box_pack_start (GTK_BOX(main_conf_box),
		      CheckBox,
		      FALSE,
		      FALSE,
		      0);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(CheckBox),
				initial_value);

  g_signal_connect   (G_OBJECT(CheckBox),
		      "toggled",
		      G_CALLBACK(gcompris_boolean_box_toggled),
		      key);

  return GTK_CHECK_BUTTON(CheckBox);
}

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
