/* gcompris - board_config.h
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

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H
#include "gcompris.h"

/**
 * the callback type
 * returns TRUE if the data is valid and the save succeeded
 */

typedef gboolean (*GcomprisConfCallback) (GHashTable *table);

typedef struct
{
	GtkWindow *conf_window;
	GtkVBox *main_conf_box;
	GHashTable *hash_conf;
	GcomprisConfCallback Confcallback;
} GcomprisBoardConf;

void	 gc_board_config_start(GcomprisBoard *aBoard,
			       GcomprisProfile *aProfile);
void	 gc_board_config_stop();


/* Usefull fonctions for configuration of boards */
/* This fonction returns the main GtkVBox of the configuration window. */
/* You can add your own widget if you need */
/* the callback is called wish the hash key, value when the apply button is clicked */

GcomprisBoardConf *gc_board_config_window_display(gchar *label, GcomprisConfCallback callback);

GtkCheckButton *gc_board_config_boolean_box(GcomprisBoardConf *config, const gchar *label, gchar *key, gboolean initial_value);

GtkComboBox *gc_board_config_combo_box(GcomprisBoardConf *config, const gchar *label, GList *strings, gchar *key, gchar *init);

/* Params: */
/*   - Label */
/*   - key (for hashtable  return) */
/*   - g_hash_table (gchar *values, gchar *label) */
/* Returns */
/*   - g_hash_table (gchar *values, GtkWidget *pointer) */

GHashTable *gc_board_config_radio_buttons(GcomprisBoardConf *config, const gchar *label,
				   gchar *key,
				   GHashTable *buttons_label,
				   gchar *init);

GtkSpinButton *gc_board_config_spin_int(GcomprisBoardConf *config, const gchar *label,
				 gchar *key,
				 gint min,
				 gint max,
				 gint step,
				 gint init);

GtkHSeparator *gc_board_conf_separator(GcomprisBoardConf *config);

GList *gc_locale_gets_list();

GtkComboBox *gc_board_config_combo_locales(GcomprisBoardConf *config, gchar *init);

GtkComboBox *gc_board_config_combo_drag( GcomprisBoardConf *config, gint init);

GList *gc_locale_gets_asset_list(const gchar *file);

GtkComboBox *gc_board_config_combo_locales_asset(GcomprisBoardConf *config,
						 const gchar *label, gchar *init,
						 const gchar *file, GCallback callback);
/***********************************/
/* TextView  facility              */
/* Callback is text vaidating function */
/* key is used to pass last validated text on "Apply" event */
/* and returned to callback */

typedef gboolean (*GcomprisTextCallback) (gchar *key, gchar *text, GtkLabel *label);

GtkTextView *gc_board_config_textview(GcomprisBoardConf *config, const gchar *label,
			       gchar *key,
			       const gchar*description,
			       gchar *init_text,
			       GcomprisTextCallback validate);

GtkWidget *gc_board_config_wordlist(GcomprisBoardConf *config, const gchar *files);

#endif

