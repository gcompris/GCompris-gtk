/* this file is part of criawips, a gnome presentation application
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

#ifndef GLIB_HELPERS_H
#define GLIB_HELPERS_H

#include <glib/glist.h>

G_BEGIN_DECLS

#ifndef G_COMPARE_FUNC
#define G_COMPARE_FUNC(func) ((GCompareFunc)(func))
#endif /* !G_COMPARE_FUNC */

#ifndef G_DESTROY_NOTIFY
#define G_DESTROY_NOTIFY(func) ((GDestroyNotify)(func))
#endif /* !G_DESTROY_NOTIFY */

#ifndef G_FUNC
#define G_FUNC(func)	    ((GFunc)(func))
#endif /* !GFUNC */

#ifndef GH_FUNC
#define GH_FUNC(func) ((GHFunc)(func))
#endif /* !GH_FUNC */

#ifndef G_SOURCE_FUNC
#define G_SOURCE_FUNC(func) ((GSourceFunc)(func))
#endif /* !G_SOURCE_FUNC */

#ifndef G_COMPARE_FUNC
#define G_COMPARE_FUNC(func) ((GCompareFunc)(func))
#endif /* !G_COMPARE_FUNC */

#ifndef GHR_FUNC
#define GHR_FUNC(func) ((GHRFunc)(func))
#endif /* !GHR_FUNC */

#if GLIB_MAJOR_VERSION < 3 && GLIB_MINOR_VERSION < 9
# define g_value_get_gtype(value)       g_value_get_uint(value)
# define g_value_set_gtype(value, type) g_value_set_uint(value, type)
# define g_param_spec_gtype(nick, name, blurb, is_a_type, flags) \
					g_param_spec_uint(nick, name, blurb, 0, G_MAXUINT, 0, flags)
#endif

GList* g_list_copy_reversed  (GList* list);
void   g_list_foreach_reverse(GList* list, GFunc func, gpointer data);

gsize  g_utf8_length_next(gchar const* utf8);

G_END_DECLS

#endif /* GLIB_HELPERS_H */

