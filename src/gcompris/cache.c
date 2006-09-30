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

#include "gcompris.h"
#include "gc_core.h"
#include <string.h>

/** \file Implementation of the cache functionality
 *
 */

/* The max size of the cache in bytes,
 * 0  = NO LIMITS
 * -1 = NO CACHE
 */
#ifdef USE_GNET
static int cache_max_size = 1000;

static GHashTable *hash_cache = NULL;
#endif

/** For debug only
 */
#ifdef USE_GNET
static void
_dump_cache(gchar *key,
	    gchar *value,
	    gpointer dummy)
{
  printf("cache %s = %s\n", key, value);
}

static void dump_cache(void)
{
  g_hash_table_foreach(hash_cache,
		       (GHFunc) _dump_cache,
		       NULL);
}
#endif

/**
 * recursively parse the cache and fill up the hash with files there
 */
void _cache_init(const gchar *basedir, const gchar *currentdir)
{
#ifdef USE_GNET
  GcomprisProperties *properties = gc_prop_get();
  GDir *dir;
  const gchar *file;

  if(!currentdir)
    currentdir = basedir;

  dir = g_dir_open(currentdir, 0, NULL);

  printf("  _cache_init %s\n", currentdir);

  while((file = g_dir_read_name(dir)))
    {
      gchar *fullfile = g_strconcat(currentdir, "/", file, NULL);
      printf("  processing file = %s\n", fullfile);

      if(g_file_test(fullfile, G_FILE_TEST_IS_DIR))
	{
	  _cache_init(basedir, fullfile);
	}
      else if(g_file_test(fullfile, G_FILE_TEST_IS_REGULAR))
	{
	  /* Strip file to get the KEY part out of it */
	  printf("  Adding '%s' in the cache\n", fullfile);
 	  g_hash_table_insert(hash_cache,
			      g_strconcat(properties->server, "/",
					  fullfile + strlen(basedir) + 1,
					  NULL),
			      g_strdup(fullfile));

	}
      g_free(fullfile);
    }

  g_dir_close(dir);
#endif
}

/** Initialize the cache system
 *
 * \param max_size: the max size in byte of the cache
 *
 */
void gc_cache_init(int max_size)
{
#ifdef USE_GNET
  cache_max_size = max_size;

  /* No server defined, the cache is useless */
  if(!gc_prop_get()->server ||
     !gc_prop_get()->cache_dir)
    return;

  hash_cache = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

  printf("gc_cache_init\n");

  /* try to create it */
  printf("Creating cache dir %s\n", gc_prop_get()->cache_dir);
  if(g_mkdir_with_parents(gc_prop_get()->cache_dir, 0755))
    {
      g_error("Failed to create the cache directory");
    }
  printf("  opened top directory\n");
  /* Load the previous cache directory if any */
  _cache_init(gc_prop_get()->cache_dir, NULL);

  dump_cache();
#endif
}

/** End the cache system
 *
 */
void gc_cache_end()
{
#ifdef USE_GNET

  if(!hash_cache)
    return;

  g_hash_table_destroy (hash_cache);
  hash_cache = NULL;
#endif
}


#ifdef USE_GNET
static void
_clear_cache(gchar *key,
	     gchar *value,
	     gpointer dummy)
{
  printf("NOT IMPLEMENTED: Clearing chache %s:%s\n", key, value);
}
#endif

/** Clear the cache. All files in the cache are removed
 *
 */
void gc_cache_clear()
{
#ifdef USE_GNET
  g_hash_table_foreach(hash_cache,
		       (GHFunc) _clear_cache,
		       NULL);

#endif
}

/** Get a file from the cache based on it's URL
 *
 */
gchar *gc_cache_get(gchar *url)
{
#ifdef USE_GNET
  return((char *)g_hash_table_lookup(hash_cache, url));
#else
  return NULL;
#endif
}

/** Put and Get a file from the cache. The data in 'buffer' are saved in the
 *  cache under the name 'url'. A file is created and is returned.
 *
 * \param url: the url of the data in buffer
 * \param buffer: the data for the url
 * \param length: length of contents, or -1 if contents is a nul-terminated string
 *
 * \return a full path to the file in the cache
 */
gchar *gc_cache_insert(const gchar *url, const char *buffer, gssize length)
{
#ifdef USE_GNET
  /* Save the buffer in the cache */
  if(g_file_set_contents("TBD", buffer, length, NULL))
    g_hash_table_replace(hash_cache, (gpointer) url, (gpointer) "TBD");
#endif
  return("TBD");
}
