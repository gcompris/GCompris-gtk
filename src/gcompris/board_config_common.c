/* gcompris - board_config_common.c
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

/* code get from gtk */
/* included here to not depend on gtk 2.6 */

#include "gcompris.h"
#include "board_config_common.h"
#include <string.h>

gchar *_get_active_text (GtkComboBox *combo_box)
{
  GtkTreeIter iter;
  gchar *text = NULL;

  g_return_val_if_fail (GTK_IS_LIST_STORE (gtk_combo_box_get_model (combo_box)), NULL);

  if (gtk_combo_box_get_active_iter (combo_box, &iter))
    gtk_tree_model_get (gtk_combo_box_get_model (combo_box), &iter,
			0, &text, -1);

  return text;
}

void check_key(gchar *key)
{
  if ((strcmp(key, "locale") == 0) ||
      (strcmp(key, "locale_sound") == 0) ||
      (strcmp(key, "wordlist") == 0))
    g_error(" Key %s forbiden ! Change !", key);
}

void _gc_destroy_boardconf_key(GtkWidget *widget, gpointer data)
{
  _gc_boardconf_key *u = (_gc_boardconf_key*)data;
  g_free(u->key);
  g_free(u);
}


