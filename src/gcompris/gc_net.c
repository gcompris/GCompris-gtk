/* gcompris - gameutil_net.c
 *
 * Time-stamp: <2006/07/10 01:24:04 bruno>
 *
 * Copyright (C) 2006 Bruno Coudoin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gc_net.h"

#ifdef USE_GNET
#include <gnet.h>
#endif

/* FIXME: Should not be needed, a bug in gnet header ? */
gboolean         gnet_http_get                     (const gchar      *url,
                                                    gchar           **buffer,
                                                    gsize            *length,
                                                    guint            *response);

#include <string.h>

#ifdef USE_GNET
static GSList *server_content_list = NULL;
#define	SUPPORT_OR_RETURN(rv)	{if(!gcompris_get_properties()->server) return rv;}
#else
#define	SUPPORT_OR_RETURN(rv)	{ return rv; }
#endif


static inline int my_strcmp(gchar *a, gchar *b) { return strcmp( a, b); }

/** Init the network library, must be called once before using it
 *
 */
void gc_net_init()
{
  SUPPORT_OR_RETURN();

#ifdef USE_GNET
#endif
}

/** Load a pixmap localy or from the network
 *
 * \param pixmapfile : a full URL to the file to load as an image
 *                     in case a local file is given, it will be loaded.
 * \return a GdkPixbuf or NULL
 */
GdkPixbuf *gc_net_load_pixmap(const char *url)
{
  if(!gc_net_is_url(url))
    return(gdk_pixbuf_new_from_file (url, NULL));

  SUPPORT_OR_RETURN(NULL);

#ifdef USE_GNET
#endif
}

/** Load an xml file from the network
 *
 * \param xmlfile : a full URL to the xml file to load as an xmlDocPtr
 *                     in case a local file is given, it will be loaded.
 * \return a xmlDocPtr or NULL
 */
xmlDocPtr gc_net_load_xml(const char *url)
{
  if(!gc_net_is_url(url))
    return(xmlParseFile(url));

  SUPPORT_OR_RETURN(NULL);

#ifdef USE_GNET
#endif
}

/** return an absolute URL if the given file is part of the file available on our server
 *
 * \param file: the file to check
 * \return: a newly allocated URL or NULL
 */
gchar *
gc_net_get_url_from_file(const gchar *format, ...)
{
  SUPPORT_OR_RETURN(NULL);

#ifdef USE_GNET
#endif
}

/** return TRUE if the url starts with http://
 *
 * \param url: an url to check
 * \return TRUE is the url starts with 'http://'
 */
gboolean
gc_net_is_url(const gchar *url)
{
  if( !url || strncmp(url, "http://", 7) )
    return FALSE;

  return TRUE;
}

/** return a glist with the content of the files in the given directory
 *
 * \param dir: the directory to scan
 * \param ext: optional extention filter. e.g. ".xml" to get only *.xml files.
 *
 * \return: a new allocated glist that points to internal elements. Do not free the list
 *          data itself.
 */
GSList *gc_net_dir_read_name(const gchar* dir, const gchar *ext)
{
  SUPPORT_OR_RETURN(NULL);

#ifdef USE_GNET
#endif
}
