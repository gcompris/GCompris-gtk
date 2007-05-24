/* This file is part of gundo, a multilevel undo/redo facility for GTK+
 *
 * AUTHORS
 *	Sven Herzberg		<herzi@gnome-de.org>
 *
 * Copyright (C) 2005		Sven Herzberg
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

#ifndef GOBJECT_HELPERS_H
#define GOBJECT_HELPERS_H

#include <glib-object.h>

#ifndef G_BOXED_COPY_FUNC
#define G_BOXED_COPY_FUNC(f) (GBoxedCopyFunc)(f)
#endif /* !G_BOXED_COPY_FUNC */

#ifndef G_BOXED_FREE_FUNC
#define G_BOXED_FREE_FUNC(f) (GBoxedFreeFunc)(f)
#endif /* !G_BOXED_FREE_FUNC */

#ifndef g_signal_connect_swapped_after
#define g_signal_connect_swapped_after(instance, detailed_signal, callback, data) \
	g_signal_connect_data((instance), (detailed_signal), (callback), (data), NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED)
#endif /* !g_signal_connect_swapped_after */

#if (!defined G_DEFINE_IFACE_EXTENDED) && (!defined G_DEFINE_IFACE) && (!defined G_DEFINE_IFACE_FULL)
#define G_DEFINE_IFACE_EXTENDED(TypeName, type_name, parent, type_init, CODE) \
GType \
type_name##_get_type(void) { \
	static GType type = 0; \
	\
	if(G_UNLIKELY(!type)) { \
		static const GTypeInfo info = { \
			sizeof(TypeName##Iface), \
			NULL, \
			NULL, \
			(GClassInitFunc)type_init \
		}; \
		\
		type = g_type_register_static(parent, #TypeName, &info, 0); \
		g_type_interface_add_prerequisite(type, G_TYPE_OBJECT); \
		{ CODE ; } \
	} \
	\
	return type; \
}

#define G_DEFINE_IFACE(TypeName, type_name, parent)  G_DEFINE_IFACE_EXTENDED(TypeName, type_name, parent, NULL, {})

#define G_DEFINE_IFACE_FULL(TypeName, type_name, parent) \
static void type_name##_iface_init(gpointer iface); \
\
G_DEFINE_IFACE_EXTENDED(TypeName, type_name, parent, type_name##_iface_init, {})

#endif /* not defined any of the G_DEFINE_IFACE macros */

#endif /* GOBJECT_HELPERS_H */

