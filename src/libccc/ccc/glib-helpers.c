/* this file is part of gnome-gdb, a GUI for the gdb
 *
 * AUTHORS
 *       Sven Herzberg        <herzi@gnome-de.org>
 *
 * Copyright (C) 2005,2006 Sven Herzberg
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "glib-helpers.h"

#include <glib/gmessages.h>
#include <glib/gunicode.h>

GList*
g_list_copy_reversed(GList* list) {
	GList* retval = NULL;

	for(; list; list = list->next) {
		retval = g_list_prepend(retval, list->data);
	}

	return retval;
}

void
g_list_foreach_reverse(GList* list, GFunc func, gpointer data) {
	GList* copy = g_list_reverse(g_list_copy(list));
	g_list_foreach(copy, func, data);
	g_list_free(copy);
}

gsize
g_utf8_length_next(gchar const* utf8) {
	g_return_val_if_fail(utf8, 0);
	return g_utf8_find_next_char(utf8, NULL) - utf8;
}

