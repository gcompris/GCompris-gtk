/* gcompris - board_config.c
 *
 * Time-stamp: <2005/08/14 14:29:23 yves>
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
  GtkWidget  *header;

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
  g_signal_connect (G_OBJECT(button),
		    "clicked",
		    G_CALLBACK(gcompris_apply_board_conf),
		    NULL);

  /* Label header */
  header = gtk_label_new ((gchar *)NULL);
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

void gcompris_combo_box_changed(GtkComboBox *combobox,
				gpointer key)
{
  gchar *the_key = g_strdup((gchar *)key);

  gchar *value = g_strdup_printf("%d", gtk_combo_box_get_active (combobox));
  
  g_hash_table_replace(hash_conf, (gpointer) the_key, (gpointer) value);
}

GtkComboBox *gcompris_combo_box(const gchar *label, GList *strings, gchar *key, gint index)
{
  GtkWidget *combobox;
  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
  GList *list;
  GtkWidget *label_combo;

  gtk_widget_show(hbox);

  gtk_box_pack_start (GTK_BOX(main_conf_box),
		      hbox,
		      FALSE,
		      FALSE,
		      0);

  /* Label */
  label_combo = gtk_label_new ((gchar *)NULL);
  gtk_widget_show(label_combo);
  gtk_box_pack_start (GTK_BOX(hbox),
		      label_combo,
		      FALSE,
		      FALSE,
		      0);

  gtk_label_set_justify (GTK_LABEL(label_combo),
			 GTK_JUSTIFY_RIGHT);

  gtk_label_set_markup (GTK_LABEL(label_combo),
                        (const gchar *)label);


  combobox = gtk_combo_box_new_text();

  gtk_widget_show(combobox);

  gtk_box_pack_start (GTK_BOX(hbox),
		      combobox,
		      FALSE,
		      FALSE,
		      0);


  for (list = strings; list != NULL; list = list->next)
    gtk_combo_box_append_text       (GTK_COMBO_BOX(combobox),
				     list->data);

  
  gtk_combo_box_set_active (GTK_COMBO_BOX(combobox),
			    index);
  
  g_signal_connect(G_OBJECT(combobox),
		   "changed",
		   G_CALLBACK(gcompris_combo_box_changed),
		   key);

  return GTK_COMBO_BOX(combobox);
}

static gchar *radio_value = NULL;
static GSList *radio_group = NULL;
static GtkWidget *radio_box;
static gchar *radio_key = NULL;
static gchar *radio_text = NULL;
static radio_init = NULL;

void radio_changed(GtkToggleButton *togglebutton,
		   gpointer key)
{
  gboolean state = gtk_toggle_button_get_active (togglebutton);
  gchar *h_key;
  gchar *h_value;

  if (state){
    h_key = g_strdup (radio_key);
    h_value = g_strdup((gchar *) key);
    g_hash_table_replace (hash_conf, h_key, h_value);
  }
}

void create_radio_buttons(gpointer key,
			  gpointer value,
			  gpointer hash_radio)
{
  GtkWidget *radio_button;
  gchar *key_copy;

  radio_button = gtk_radio_button_new_with_label (radio_group,
						  (const gchar *) g_strdup(value));

  gtk_box_pack_start (GTK_BOX (radio_box), radio_button, TRUE, TRUE, 2);
  
  gtk_widget_show (GTK_WIDGET (radio_button));
  
  radio_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_button));
  
  key_copy = g_strdup ((gchar *)key);

  if (strcmp( key_copy, radio_init)==0)
    gtk_toggle_button_set_active    (GTK_TOGGLE_BUTTON(radio_button),TRUE);

  g_signal_connect(G_OBJECT(radio_button), 
		   "toggled", 
		   G_CALLBACK(radio_changed), 
		   (gpointer) key_copy);
  
  g_hash_table_replace ( hash_radio, (gpointer) key_copy, (gpointer) radio_button);
}

void destroy_hash (GtkObject *object,
                   gpointer hash_table)
{
  g_hash_table_destroy((GHashTable *)hash_table);
  radio_group = NULL;
  g_free(radio_text);
  g_free(radio_key);
  g_free(radio_init);
}

GHashTable *gcompris_radio_buttons(const gchar *label,
				   gchar *key,
				   GHashTable *buttons_label,
				   gchar *init)
{
  GtkWidget *radio_label;

  GHashTable *buttons = g_hash_table_new_full (g_str_hash,
					       g_str_equal,
					       g_free,
					       NULL);

  radio_box = gtk_vbox_new (TRUE, 2);
  gtk_widget_show (GTK_WIDGET (radio_box));

  gtk_box_pack_start (GTK_BOX(main_conf_box),
		      radio_box,
		      FALSE,
		      FALSE,
		      0);

  radio_label = gtk_label_new ((gchar *)NULL);
  gtk_widget_show(radio_label);

  gtk_box_pack_start (GTK_BOX(radio_box),
		      radio_label,
		      FALSE,
		      FALSE,
		      0);

  gtk_label_set_justify (GTK_LABEL(radio_label),
			 GTK_JUSTIFY_CENTER);
  
  radio_text = g_strdup(label);
  gtk_label_set_markup (GTK_LABEL(radio_label),
                        (const gchar *)radio_text);

  radio_key = g_strdup(key);
  radio_init = g_strdup(init);

  g_hash_table_foreach( buttons_label, 
			(GHFunc) create_radio_buttons,
			(gpointer) buttons);

  g_signal_connect (G_OBJECT(radio_box), "destroy", G_CALLBACK(destroy_hash), (gpointer) buttons);

  return buttons;
}

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
