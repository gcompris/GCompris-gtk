/* this file is part of criawips, a gnome presentation application
 *
 * AUTHORS
 *       Sven Herzberg        <herzi@gnome-de.org>
 *
 * Copyright (C) 2004,2005 Sven Herzberg
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

#ifndef CDEBUG_H
#define CDEBUG_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#ifndef CDEBUG_DISABLED
void cdebugv (const gchar* func, const gchar* format, va_list args);
void cdebugc (gconstpointer klass, const gchar* func, const gchar* format, ...);
void cdebugo (gconstpointer inst, const gchar* func, const gchar* format, ...);
void cdebugt (GType type, const gchar* func, const gchar* format, ...);

void cdebug_shutdown(void);

#define CDEBUG(code)	G_STMT_START{ code }G_STMT_END

#if (defined(CDEBUG_TYPE) && (CDEBUG_TYPE != -1))
# ifdef G_HAVE_ISO_VARARGS
#  define cdebug(nspace, ...) cdebugt(CDEBUG_TYPE(), nspace, __VA_ARGS__)
# elif defined(G_HAVE_GNUC_VARARGS)
#  define cdebug(nspace, message...) cdebugt(CDEBUG_TYPE(), nspace, message)
# endif
#else
# if !defined(CDEBUG_TYPE)
#  warning "You want to set CDEBUG_TYPE to the GType you want to debug; if you want to turn this warning off, add '#define CDEBUG_TYPE -1' to your code"
# endif
void cdebug(const gchar* context, const gchar* format, ...);
#endif
#else
# define cdebug (nspace, message, ...)		G_STMT_START{ (void)0; }G_STMT_END
# define cdebugc(klass, func, message, ...)	G_STMT_START{ (void)0; }G_STMT_END
# define cdebugo(inst, func, message, ...)	G_STMT_START{ (void)0; }G_STMT_END
# define cdebugt(type, func, message, ...)	G_STMT_START{ (void)0; }G_STMT_END
# define cdebug_shutdown()                      G_STMT_START{ (void)0; }G_STMT_END
# define CDEBUG(code)                           G_STMT_START{ (void)0; }G_STMT_END
#endif

G_END_DECLS

#endif /* CDEBUG_H */
