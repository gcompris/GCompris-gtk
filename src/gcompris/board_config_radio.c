/* gcompris - board_config_radio.c
 *
 * Copyright (C) 2009 Miguel De Izarra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "board_config.h"
#include "board_config_common.h"
#include <string.h>

typedef struct 
{
GcomprisBoardConf *config;
GHashTable *hash_radio;
gchar *key;
GSList *radio_group ;
GtkWidget *radio_box;
gchar *radio_key;
gchar *radio_init;
} Gconfig_radio;

static void radio_changed(GtkToggleButton *togglebutton,
			  gpointer data)
{
  gboolean state = gtk_toggle_button_get_active (togglebutton);
  Gconfig_radio *u = (Gconfig_radio*)data;
  gchar *h_key;
  gchar *h_value;

  if (state){
    h_key = g_strdup (u->radio_key);
    h_value = g_strdup((gchar *) u->key);
    g_hash_table_replace (u->config->hash_conf, h_key, h_value);
  }
}

static void radio_destroy(GtkWidget*w, gpointer data)
{
	Gconfig_radio *u = (Gconfig_radio*)data;

	/* simply free key and Gconfig_radio */
	g_free(u->key);
	g_free(u);
}

static void
create_radio_buttons(gpointer key,
		     gpointer value,
		     gpointer data)
{
  Gconfig_radio * u = (Gconfig_radio*)data;
  Gconfig_radio *u_copy;
  GtkWidget *radio_button;

  radio_button = gtk_radio_button_new_with_label (u->radio_group,
						  (const gchar *) g_strdup(value));

  gtk_box_pack_start (GTK_BOX (u->radio_box), radio_button, TRUE, TRUE, 2);

  gtk_widget_show (GTK_WIDGET (radio_button));

  u->radio_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_button));

  /* make a copy of Gconfig_radio, and set key */
  u_copy = g_memdup(u,sizeof(Gconfig_radio));
  u_copy->key = g_strdup ((gchar *)key);

  if (strcmp(u_copy->key, u->radio_init)==0)
    gtk_toggle_button_set_active    (GTK_TOGGLE_BUTTON(radio_button),TRUE);

  g_signal_connect(G_OBJECT(radio_button),
		   "toggled",
		   G_CALLBACK(radio_changed),
		   (gpointer) u_copy);
  g_signal_connect(G_OBJECT(radio_button),
  		"destroy",
		G_CALLBACK(radio_destroy),
		(gpointer) u_copy);
  g_hash_table_replace ( u->hash_radio, (gpointer) u_copy->key, (gpointer) radio_button);
}

static void
radio_box_destroy(GtkObject *object,
	      gpointer data)
{
  Gconfig_radio *u = (Gconfig_radio*) data;
  g_hash_table_destroy(u->hash_radio);
  u->radio_group = NULL;
  g_free(u->radio_key);
  g_free(u->radio_init);
  g_free(u);
}

GHashTable *
gc_board_config_radio_buttons(GcomprisBoardConf *conf, const gchar *label,
		       gchar *key,
		       GHashTable *buttons_label,
		       gchar *init)
{
  g_return_val_if_fail(conf, NULL);
  check_key( key);

  GtkWidget *radio_label;

  GHashTable *buttons = g_hash_table_new_full (g_str_hash,
					       g_str_equal,
					       g_free,
					       NULL);
  Gconfig_radio *u = g_malloc0(sizeof(Gconfig_radio));
  u->hash_radio = buttons;
  u->config = conf;

  u->radio_box = gtk_vbox_new (TRUE, 2);
  gtk_widget_show (GTK_WIDGET (u->radio_box));

  gtk_box_pack_start (GTK_BOX(conf->main_conf_box),
		      u->radio_box,
		      FALSE,
		      FALSE,
		      0);

  radio_label = gtk_label_new ((gchar *)NULL);
  gtk_widget_show(radio_label);

  gtk_box_pack_start (GTK_BOX(u->radio_box),
		      radio_label,
		      FALSE,
		      FALSE,
		      0);

  gtk_label_set_justify (GTK_LABEL(radio_label),
			 GTK_JUSTIFY_CENTER);

  gchar * radio_text = g_strdup(label);
  gtk_label_set_markup (GTK_LABEL(radio_label),
                        (const gchar *)radio_text);
  g_free(radio_text);

  u->radio_key = g_strdup(key);
  u->radio_init = g_strdup(init);

  g_hash_table_foreach( buttons_label,
			(GHFunc) create_radio_buttons,
			(gpointer) u);

  g_signal_connect (G_OBJECT(u->radio_box), "destroy", G_CALLBACK(radio_box_destroy), (gpointer) u);

  return buttons;
}

