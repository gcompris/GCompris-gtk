/* gcompris - gc_net.h
 *
 * Time-stamp: <2000/07/16 00:32:26 bruno>
 *
 * Copyright (C) 2006 Bruno Coudoin
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

/*! \file net.h
  \brief Function related to networking
*/

#ifndef GC_NET_H
#define GC_NET_H

#include <gdk-pixbuf/gdk-pixbuf.h>

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "gcompris.h"

#define GCOMPRIS_BASE_URL "http://gcompris.net/gcompris"

void gc_net_init();
GdkPixbuf *gc_net_load_pixmap(const char *url);
xmlDocPtr  gc_net_load_xml(const char *url);
gchar     *gc_net_get_url_from_file(const gchar *format, ...);
gboolean   gc_net_is_url(const char *url);
GSList    *gc_net_dir_read_name(const gchar* dir, const gchar *ext);

#endif
