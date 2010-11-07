/* gcompris - gameutil_net.c
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
#ifdef USE_GNET
#include <gnet.h>
#endif
#include <glib/gstdio.h>

/* FIXME: Should not be needed, a bug in gnet header ? */
gboolean         gnet_http_get                     (const gchar      *url,
                                                    gchar           **buffer,
                                                    gsize            *length,
                                                    guint            *response);

#ifdef USE_GNET
static GHashTable *server_content = NULL;
static GHashTable *cache_content=NULL;
#define	SUPPORT_OR_RETURN(rv)	{if(!gc_prop_get()->server) return rv;}
#else
#define	SUPPORT_OR_RETURN(rv)	{ return rv; }
#endif

#ifdef USE_GNET
static void load_md5file(GHashTable *ht, gchar *content)
{
  gchar **lines, **keyval;
  int i;

  lines = g_strsplit(content, "\n", 0);
  if(lines && lines[0])
    {
      for(i=0; lines[i]; i++)
	{
	  keyval = g_strsplit(lines[i], "  ", 2);
	  if(keyval && keyval[0])
	    {
	      g_hash_table_insert(ht, g_strdup(keyval[1]), g_strdup(keyval[0]));
	    }
	  g_strfreev(keyval);
	}
    }
  g_strfreev(lines);
}
#endif

/** Init the network library, must be called once before using it
 *
 */
void gc_net_init()
{
  SUPPORT_OR_RETURN();

#ifdef USE_GNET
  GcomprisProperties *properties = gc_prop_get();
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
      server_content = g_hash_table_new(g_str_hash, g_str_equal);
      load_md5file(server_content, buf);
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

void gc_net_destroy(void)
{
  SUPPORT_OR_RETURN();

#ifdef USE_GNET
  if(server_content)
    g_hash_table_destroy(server_content);
  server_content = NULL;
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
  GcomprisProperties *properties = gc_prop_get();
  gchar *file, *cache=NULL, *value;
  va_list args;
  gboolean cache_ok=FALSE;

  va_start (args, format);
  file = g_strdup_vprintf (format, args);
  va_end (args);

  g_warning("gc_net_get_url_from_file '%s'", file);

  value = g_hash_table_lookup(server_content, (gpointer) file);
  if(value)
    {
      cache = g_strconcat(properties->cache_dir, "/", file, NULL);
      if(g_file_test(cache, G_FILE_TEST_IS_REGULAR))
	{
	  gchar * content;
	  gsize length;
	  GMD5 *md5cache, *md5serv;

	  /* calc md5 of cache file */
	  g_file_get_contents(cache, &content, &length, NULL);
	  md5cache = gnet_md5_new(content, length);
	  g_free(content);

	  md5serv = gnet_md5_new_string(value);

	  cache_ok = gnet_md5_equal(md5serv, md5cache);

	  gnet_md5_delete(md5serv);
	  gnet_md5_delete(md5cache);
	}
      if(cache_ok==0)
	{
	  gchar *url;
	  gchar *buf = NULL;
	  gsize  buflen;
	  guint  response;

	  url = g_strconcat(properties->server, "/", file, NULL);
	  if(gnet_http_get(url, &buf, &buflen, &response) && response == 200)
	    {
	      gchar *dirname;

	      dirname = g_path_get_dirname(cache);
	      g_mkdir_with_parents(dirname, 0755);
	      g_free(dirname);
	      g_file_set_contents(cache, buf, buflen, NULL);
	      g_free(buf);
	    }
	  else
	    { /* file is in content.txt but not in server */
	      g_free(cache);
	      cache = NULL;
	    }
	}
    }
  g_free(file);

  return cache;
#endif
}


#if 0
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
#endif


#define CONTENT_FILENAME "content.txt"

void gc_cache_init(void)
{
#ifdef USE_GNET
  gchar *filename;
  gchar *buf;
  gsize buflen;

  cache_content = g_hash_table_new(g_str_hash, g_str_equal);
  filename = gc_file_find_absolute_writeable(CONTENT_FILENAME);

  if(g_file_get_contents(filename, &buf, &buflen,NULL))
    {
      load_md5file(cache_content, buf);
      g_free(buf);
    }
  g_free(filename);
#endif
}

#ifdef USE_GNET
static gchar *gc_cache_get_relative(gchar *filename)
{
  gchar *filename_content, *dirname;

  filename_content = gc_file_find_absolute_writeable(CONTENT_FILENAME);
  dirname = g_path_get_dirname(filename_content);
  if(g_str_has_prefix(filename, dirname))
    filename = filename + strlen(dirname) + 1;
  g_free(filename_content);
  g_free(dirname);
  return filename;
}
#endif

void gc_cache_add(gchar *filename)
{
#ifdef USE_GNET
  if(cache_content==NULL)
    return;
  if(g_str_has_suffix(filename, CONTENT_FILENAME))
    return;

  filename = gc_cache_get_relative(filename);
  g_hash_table_insert(cache_content, g_strdup(filename), g_strdup("0"));
#endif
}

gchar* gc_cache_import_pixmap(gchar *filename, gchar *boarddir, gint width, gint height)
{
  GdkPixbuf *pixmap;
  gchar *basename, *file, *ext, *name, *abs;

  if(!g_path_is_absolute(filename))
    return g_strdup(filename);
  basename = g_path_get_basename(filename);
  name = g_build_filename(boarddir, basename,NULL);
  abs = gc_file_find_absolute(name);
  if(abs && strcmp(abs,filename)==0)
    {
      g_free(basename);
      g_free(abs);
      return name;
    }
  pixmap = gdk_pixbuf_new_from_file_at_size(filename, width, height,NULL);
  if(!pixmap)
    {
      g_free(abs);
      g_free(basename);
      g_free(name);
      return NULL;
    }

  file = gc_file_find_absolute_writeable(name);
  ext = strchr(basename, '.')+1;
  if(strcmp(ext, "jpg")==0)
    ext ="jpeg";

  gdk_pixbuf_save(pixmap, file, ext, NULL,NULL);

  g_free(abs);
  g_free(basename);
  g_free(file);
  return name;
}

void gc_cache_remove(gchar *filename)
{
#ifdef USE_GNET
  g_remove(filename);
  filename = gc_cache_get_relative(filename);
  g_hash_table_remove(cache_content, filename);
#endif
}

struct _table_data
  {
    FILE *pf;
    gchar *path;
  };

#ifdef USE_GNET
static void _table_foreach(gpointer key, gpointer value, gpointer user_data)
{
  struct _table_data *data = (struct _table_data*)user_data;
  gchar * content, *filename;
  gsize length;
  GMD5 *md5;

  if(strcmp(value, "0")==0)
    {
      filename = g_build_filename(data->path, (gchar*)key, NULL);
      if(g_file_get_contents(filename, &content, &length, NULL))
	{
	  md5 = gnet_md5_new(content, length);
	  value = gnet_md5_get_string(md5);
	  gnet_md5_delete(md5);
	  g_free(content);
	}
      g_free(filename);
    }
  if(strcmp(value, "0"))
    {
      fprintf(data->pf, "%s  %s\n", (gchar*)value, (gchar*)key);
    }
}
#endif

void gc_cache_save(void)
{
#ifdef USE_GNET
  struct _table_data data;
  FILE *pf;
  gchar *filename;

  filename = gc_file_find_absolute_writeable(CONTENT_FILENAME);
  pf = fopen(filename, "w");
  if(!pf)
    {
      g_warning("Couldn't save %s\n", filename);
      return;
    }

  data.pf = pf;
  data.path = g_path_get_dirname(filename);
  g_hash_table_foreach(cache_content, _table_foreach, &data);

  g_free(filename);
  g_free(data.path);
  fclose(pf);
#endif
}

void gc_cache_destroy(void)
{
#ifdef USE_GNET
  gc_cache_save();
  g_hash_table_destroy(cache_content);
  cache_content = NULL;
#endif
}

