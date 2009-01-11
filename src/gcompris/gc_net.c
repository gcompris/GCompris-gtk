/* gcompris - gc_net.c
 *
 * Copyright (C) 2006 Bruno Coudoin
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

#include "gc_net.h"
#include "gc_core.h"
#include <string.h>

static gboolean
path_represents_svg_image (const char *path)
{
	/* Synchronous mime sniffing is a really bad idea here
	 * since it's only useful for people adding custom icons,
	 * and if they're doing that, they can behave themselves
	 * and use a .svg extension.
	 */
	return path != NULL && (strstr (path, ".svg") != NULL || strstr (path, ".svgz") != NULL);
}

/** Load a pixmap localy
 *
 * \param pixmapfile : a full path to the file to load as an image
 * \return a GdkPixbuf or NULL
 */
GdkPixbuf *gc_net_load_pixmap(const char *file)
{
  if (path_represents_svg_image (file))
    return(rsvg_pixbuf_from_file (file, NULL));
  else
    return(gdk_pixbuf_new_from_file (file, NULL));
}

/** Load an xml file from the network
 *
 * \param xmlfile : a full URL to the xml file to load as an xmlDocPtr
 *                     in case a local file is given, it will be loaded.
 * \return a xmlDocPtr or NULL
 */
xmlDocPtr gc_net_load_xml(const char *url)
{
  return(xmlParseFile(url));
}

