/* gcompris - board_config.h
 *
 * Time-stamp: <2006/08/29 01:12:58 bruno>
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

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

void	 gc_board_config_start(GcomprisBoard *aBoard,
			       GcomprisProfile *aProfile);
void	 gc_board_config_stop();


/* Usefull fonctions for configuration of boards */
/* This fonction returns the main GtkVBox of the configuration window. */
/* You can add your own widget if you need */
/* the callback is called wish the hash key, value when the apply button is clicked */

/* the callback type */
typedef void (*GcomprisConfCallback) (GHashTable *table);

GtkVBox *gc_board_config_window_display(gchar *label, GcomprisConfCallback callback);

GtkCheckButton *gc_board_config_boolean_box(const gchar *label, gchar *key, gboolean initial_value);

GtkComboBox *gc_board_config_combo_box(const gchar *label, GList *strings, gchar *key, gchar *init);

/* Params: */
/*   - Label */
/*   - key (for hashtable  return) */
/*   - g_hash_table (gchar *values, gchar *label) */
/* Returns */
/*   - g_hash_table (gchar *values, GtkWidget *pointer) */

GHashTable *gc_board_config_radio_buttons(const gchar *label,
				   gchar *key,
				   GHashTable *buttons_label,
				   gchar *init);

GtkSpinButton *gc_board_config_spin_int(const gchar *label, 
				 gchar *key, 
				 gint min, 
				 gint max, 
				 gint step, 
				 gint init);

GtkHSeparator *gc_board_conf_separator();

GList *gc_locale_gets_list();

GtkComboBox *gc_board_config_combo_locales( gchar *init);

GtkComboBox *gc_board_config_combo_drag( gint init);

void gc_locale_change(gchar *locale);

void gc_locale_reset();

GList *gc_locale_gets_asset_list(const gchar *file);

GtkComboBox *gc_board_config_combo_locales_asset(const gchar *label, gchar *init,
					  const gchar *file);
#endif

/***********************************/
/* TextView  facility              */
/* Callback is text vaidating function */
/* key is used to pass last validated text on "Apply" event */
/* and returned to callback */

typedef gboolean (*GcomprisTextCallback) (gchar *key, gchar *text, GtkLabel *label);

GtkTextView *gc_board_config_textview(const gchar *label, 
			       gchar *key,
			       const gchar*description, 
			       gchar *init_text, 
			       GcomprisTextCallback validate);
