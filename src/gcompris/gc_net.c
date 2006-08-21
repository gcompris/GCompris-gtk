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
  GcomprisProperties *properties = gcompris_get_properties();
  gchar *url;
  gchar *buf = NULL;
  gsize  buflen;
  guint  response;

  gnet_init();

  /*
   * Get the content.txt file at the root of the http server and store it in a glist
   * we then now exactly which files we have there
   * warning, do not use gc_net_get_url_from_file() since we are in fact buildind the list of file
   * for it.
   */
  url = g_strdup_printf("%s/%s", properties->server, "/content.txt");

  if(gnet_http_get(url, &buf, &buflen, &response) && response == 200)
    {
      char line[200];
      int i = 0;
      /* Parse each line of the buffer and save it in 'server_content_list' */
      while( i < buflen)
	{
	  sscanf(buf+i, "%s", (char *)&line);
	  server_content_list = g_slist_prepend(server_content_list, g_strdup(line));
	  i+=strlen(line)+1;
	}
    }
  else
    {
      /* We did not get the content list, disable network now */
      g_free(properties->server);
      properties->server = NULL;
      g_warning("Failed to initialize networked GCompris because '%s' is not found", url);
    }

  g_free(buf);
  g_free(url);
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
  gchar *buf = NULL;
  gsize  buflen;
  guint  response;

  g_warning("Loading image from url '%s'", url);

  if(gnet_http_get(url, &buf, &buflen, &response) && response == 200)
    {
      GdkPixbuf *pixmap=NULL;
      GdkPixbufLoader* loader;
      loader = gdk_pixbuf_loader_new();
      gdk_pixbuf_loader_write(loader, (guchar *)buf, buflen, NULL);
      g_free(buf);
      gdk_pixbuf_loader_close(loader, NULL);
      pixmap = gdk_pixbuf_loader_get_pixbuf(loader);
      if(!pixmap)
	g_warning("Loading image from url '%s' returned a null pointer", url);

      return(pixmap);
    }

  g_free(buf);
  return(NULL);

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
  gchar *buf = NULL;
  gsize  buflen;
  guint  response;

  g_warning("Loading xml file from url '%s'", url);

  if(gnet_http_get(url, &buf, &buflen, &response) && response == 200)
    {
      xmlDocPtr	doc = xmlParseMemory((const char *)buf, buflen);
      g_free(buf);
      if(!buf)
	g_warning("Loading xml file from url '%s' returned a null pointer", url);

      return(doc);
    }

  g_free(buf);
  return(NULL);

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
  GcomprisProperties *properties = gcompris_get_properties();
  gchar *file, *url;
  va_list args;

  va_start (args, format);
  file = g_strdup_vprintf (format, args);
  va_end (args);

  /* FIXME: In case the file does not starts with boards/, preprend it */
  {
    if(strncmp(file, "boards/", 7))
      {
	gchar *file2 = g_strconcat("boards/", file, NULL);
	g_free(file);
	file = file2;
      }
  }

  g_warning("gc_net_get_url_from_file '%s'", file);
  if(!g_slist_find_custom(server_content_list,(gconstpointer) file, (GCompareFunc) my_strcmp))
    {
      g_free(file);
      return NULL;
    }
  url = g_strconcat(properties->server, "/", file, NULL);
  g_free(file);

  g_warning("gc_net_get_url_from_file returns url '%s'", url);
  return url;
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
  GSList *filelist = NULL;
  GSList *i = NULL;

  g_return_val_if_fail(dir!=NULL, NULL);

  for (i = server_content_list; i != NULL; i = g_slist_next (i))
    {
      if(strncmp(dir, (gchar *)i->data, strlen(dir)) == 0)
	if(ext == NULL ||
	   g_str_has_suffix ((gchar *)i->data, ext))
	  filelist = g_slist_prepend(filelist, i->data);
    }

  return(filelist);
#endif
}
