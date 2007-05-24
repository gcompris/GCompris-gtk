/* this file is part of criawips, a gnome presentation application
 *
 * AUTHORS
 *       Sven Herzberg        <herzi@gnome-de.org>
 *
 * Copyright (C) 2004 Sven Herzberg
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

#define CDEBUG_TYPE -1
#include <cdebug.h>

#include <stdlib.h>

/* configuration */
#define CDEBUG_ENVIRONMENT_VARIABLE "CDEBUG"
#define CDEBUG_DELIMITERS ";,"

static GPatternSpec** pspecs = NULL;

void
cdebug(const gchar* context, const gchar* format, ...) {
	va_list  args;

	va_start(args, format);
	cdebugv(context, format, args);
	va_end(args);
}

static void
cdebug_init(void) {
	gchar* contexts = getenv(CDEBUG_ENVIRONMENT_VARIABLE);
	gchar**contextv = NULL;
	guint  i;

	if(G_LIKELY(contexts)) {
		contextv = g_strsplit_set(contexts, CDEBUG_DELIMITERS, -1);
		pspecs = g_new0(GPatternSpec*, g_strv_length(contextv) + 1);
	} else {
		contextv = g_new0(gchar*,1);
		pspecs = g_new0(GPatternSpec*, 1);
	}

	for(i = 0; contextv && contextv[i]; i++) {
		gchar* glob = g_strdup_printf("%s*", contextv[i]);
		pspecs[i] = g_pattern_spec_new(glob);
		g_free(glob);
	}

	/* don't free contexts here because it's a pointer into the environment */
	g_strfreev(contextv);
}

/**
 * cdebug_shutdown:
 *
 * Shut down the cdebug messaging system. This function cleans up all the
 * memory that was internally used for cdebug.
 */
void
cdebug_shutdown(void) {
	GPatternSpec** pspec;
	for(pspec = pspecs; *pspec; pspec++) {
		g_pattern_spec_free(*pspec);
	}

	g_free(pspecs);
	pspecs = NULL;
}

/**
 * cdebugv:
 * @context: a context for this debugging call
 * @format: the debugging message
 * @args: variable argument list
 *
 * Print a debugging message for a namespace. A namespace is usually
 * something like "ClassName::functionName()".
 */
void
cdebugv(const gchar* context, const gchar* format, va_list args) {
	gchar       * message;
	GPatternSpec**it;

	if(!pspecs) {
		cdebug_init();
	}


	for(it = pspecs; it && *it; it++) {
		if(g_pattern_match_string(*it, context)) {
			message = g_strdup_vprintf(format, args);
			g_debug("%s: %s", context, message);
			g_free(message);
			break;
		}
	}

}

static void
cdebugv2(const gchar* context1, const gchar* context2, const gchar* format, va_list args) {
	gchar * context = g_strdup_printf("%s::%s", context1, context2);
	cdebugv(context, format, args);
	g_free(context);
}

/**
 * cdebugc:
 * @klass: a #GObjectClass
 * @func: a string representing the debugging context
 * @format: the debugging message
 *
 * Print a debugging message for a #GObjectClass. Provide @func in
 * a form like "functionName()" to get some nice output.
 */
void
cdebugc(gconstpointer klass, const gchar* func, const gchar* format, ...) {
	va_list args;
	gchar * classname;
	g_return_if_fail(G_IS_OBJECT_CLASS(klass));

	va_start(args, format);
	classname = g_strdup_printf("%sClass", G_OBJECT_CLASS_NAME(klass));
	cdebugv2(classname, func, format, args);
	g_free(classname);
	va_end(args);
}

/**
 * cdebugo:
 * @inst: a #GObject
 * @func: a string representing the debugging context
 * @format: the debugging message
 *
 * Print a debugging message for an instance of a #GObject class. Provide
 * @func in a form like "functionName()" to get some nice output.
 */
void
cdebugo(gconstpointer inst, const gchar* func, const gchar* format, ...) {
	va_list args;
	g_return_if_fail(G_IS_OBJECT(inst));
	
	va_start(args, format);
	cdebugv2(G_OBJECT_TYPE_NAME(inst), func, format, args);
	va_end(args);
}

/**
 * cdebugt:
 * @type: a #GType
 * @func: a string representation of the debugging context
 * @format: the debugging message
 *
 * Print a debugging message for a #GType. Provide func in a form like
 * "functionName()" to get some nice output.
 */
void
cdebugt(GType type, const gchar* func, const gchar* format, ...) {
	va_list args;

	va_start(args, format);
	cdebugv2(g_type_name(type), func, format, args);
	va_end(args);
}

