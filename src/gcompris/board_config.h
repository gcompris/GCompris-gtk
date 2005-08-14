/* gcompris - board_config.h
 *
 * Time-stamp: <2005/08/14 02:35:18 yves>
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

#include "gcompris.h"
gboolean board_config_check(GcomprisBoard *aBoard);
void	 board_config_start(GcomprisBoard *aBoard,
			    GcomprisProfile *aProfile);
void	 board_config_stop();


/* Usefull fonctions for configuration of boards */
/* This fonction returns the main GtkVBox of the configuration window. */
/* You can add your own widget if you need */
/* the callback is called wish the hash key, value when the apply button is clicked */

/* the callback type */
typedef void (*GcomprisConfCallback) (GHashTable *table);

GtkVBox *gcompris_configuration_window(gchar *label, GcomprisConfCallback callback);

GtkCheckButton *gcompris_boolean_box(const gchar *label, gchar *key, gboolean initial_value);

GtkComboBox *gcompris_combo_box(const gchar *label, GList *strings, gchar *key, gint index);

/* Params: */
/*   - Label */
/*   - key (for hashtable  return) */
/*   - g_hash_table (gchar *values, gchar *label) */
/* Returns */
/*   - g_hash_table (gchar *values, GtkWidget *pointer) */

GHashTable *gcompris_radio_buttons(const gchar *label,
				   gchar *key,
				   GHashTable *buttons_label,
				   gchar *init);

#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
