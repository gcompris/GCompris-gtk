/* gcompris - board_config_common.h
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


#ifndef BOARD_CONFIG_COMMON_H
#define BOARD_CONFIG_COMMON_H

gchar *_get_active_text (GtkComboBox *combo_box);

void check_key(gchar *key);

typedef struct
{
	gchar *key;
	GcomprisBoardConf *config;
  } _gc_boardconf_key;

void _gc_destroy_boardconf_key(GtkWidget*w, gpointer data);


#define COMBOBOX_COL_MAX 15

#endif

