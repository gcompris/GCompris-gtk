/* gcompris - soundutil.c
 *
 * Copyright (C) 2002, 2008 Pascal Georges
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

#include "string.h"

#include "gcompris.h"
#include <signal.h>
#include <glib.h>

#include <soundutil.h>

static GList	*pending_queue = NULL;
static int	 sound_policy;

static GSList *music_list = NULL;

GHashTable *sound_callbacks = NULL;

/* =====================================================================
 *
 * =====================================================================*/
void
gc_sound_policy_set(int policy)
{
  switch (policy)
    {
    case PLAY_ONLY_IF_IDLE : sound_policy = PLAY_ONLY_IF_IDLE; break;
    case PLAY_AFTER_CURRENT : sound_policy = PLAY_AFTER_CURRENT; break;
    case PLAY_AND_INTERRUPT : sound_policy = PLAY_AND_INTERRUPT; break;
    default : sound_policy = PLAY_AFTER_CURRENT;
    }
}
/* =====================================================================
 *
 * =====================================================================*/
int
gc_sound_policy_get()
{
  return sound_policy;
}

void
gc_sound_close()
{
  gc_sound_bg_close();
  gc_sound_fx_close();
}


void
gc_sound_reopen()
{
  gc_sound_bg_reopen();
  gc_sound_fx_reopen();
}

GSList *
gc_sound_get_music_list()
{
  return music_list;
}

char *gc_sound_get_next_music()
{
  static guint bg_music_index = 0;

  /* Music wrapping */
  if(bg_music_index++ >= g_slist_length(gc_sound_get_music_list()))
    bg_music_index = 0;

  return gc_file_find_absolute(g_slist_nth_data(gc_sound_get_music_list(),
							 bg_music_index));
}

void
gc_sound_build_music_list()
{
  GcomprisProperties *properties = gc_prop_get();
  gchar *str;
  gchar *music_dir;
  GDir *dir;
  const gchar *one_dirent;

  /* Load the Music directory file names */
  music_dir = g_strconcat(properties->package_data_dir, "/music/background",
			  NULL);

  dir = g_dir_open(music_dir, 0, NULL);

  if (!dir) {
    g_message ("Couldn't open music dir: %s", music_dir);
    g_free(music_dir);
    return;
  }

  /* Fill up the music list */
  while((one_dirent = g_dir_read_name(dir)) != NULL)
    {
      if (g_str_has_suffix(one_dirent, ".ogg"))
	{
	  str = g_strdup_printf("%s/%s", music_dir, one_dirent);
	  music_list = g_slist_insert (music_list, str,
				      RAND(0, g_slist_length(music_list)));
	}
    }
  g_dir_close(dir);

  /* No music no play */
  if(g_slist_length(music_list)==0)
    {
      g_free(music_dir);
      return;
    }

}


/* =====================================================================
 * Returns the next sound play, or NULL if there is no
 ======================================================================*/
char*
get_next_sound_to_play( )
{
  char* tmpSound = NULL;

  if ( g_list_length(pending_queue) > 0 )
    {
      tmpSound = g_list_nth_data( pending_queue, 0 );
      pending_queue = g_list_remove( pending_queue, tmpSound );
      g_debug( "... get_next_sound_to_play : %s\n", tmpSound );
    }

  return tmpSound;
}

/* =====================================================================
 * Play a OGG sound file.
 * gc_sound_play_ogg function to process the sounds.
 ======================================================================*/
void
gc_sound_play_ogg_cb(const gchar *file, GcomprisSoundCallback cb)
{

  g_assert ( cb != NULL);

  /* g_intern_string is in 2.10 */
  const gchar *intern_file = g_quark_to_string( g_quark_from_string(file));

  if (!sound_callbacks)
    sound_callbacks = g_hash_table_new_full (g_str_hash,
					     g_str_equal,
					     NULL,
					     NULL);

  /* i suppose there will not be two call of that function with same sound
   * file before sound is played
   */
  g_hash_table_replace (sound_callbacks,
			(gpointer)intern_file,
			cb);
  gc_sound_play_ogg(intern_file, NULL);
}

/* =====================================================================
 * Play a list of OGG sound files. The list must be NULL terminated
 * This function wraps the var args into a GList and call the
 * gc_sound_play_ogg_list function to process the sounds.
 ======================================================================*/
void
gc_sound_play_ogg(const gchar *sound, ...)
{
  va_list ap;
  char* tmp = NULL;
  GList* list = NULL;

  if(!sound || !gc_prop_get()->fx)
    return;

  list = g_list_append(list, (gpointer)sound);

  g_debug("Adding %s in the play list queue\n", sound);

  va_start( ap, sound);
  while( (tmp = va_arg (ap, char *)))
    {
      list = g_list_append(list, (gpointer)tmp);
    }
  va_end(ap);

  gc_sound_play_ogg_list( list );

  g_list_free(list);
}

/* =====================================================================
 * Play a list of OGG sound files.
 * The given ogg files will be first tested as a locale dependant sound file:
 * sounds/<current gcompris locale>/<sound>
 * If it doesn't exists, then the test is done with a music file:
 * music/<sound>
 =====================================================================*/
void
gc_sound_play_ogg_list( GList* files )
{
  GList* list;
  gchar* tmpSound = NULL;

  if ( !gc_prop_get()->fx )
    return;

  if (sound_policy == PLAY_ONLY_IF_IDLE &&
      g_list_length( pending_queue ) > 0 )
    return;

  if (sound_policy == PLAY_AND_INTERRUPT ) {
    gc_sound_fx_close();
    while ( g_list_length(pending_queue) > 0 )
    {
      tmpSound = g_list_nth_data( pending_queue, 0 );
      g_debug("removing queue file (%s)", tmpSound);
      pending_queue = g_list_remove( pending_queue, tmpSound );
      gc_sound_callback(tmpSound);
    }
  }

  list = g_list_first( files );
  while( list!=NULL )
    {
      if (g_list_length(pending_queue) < MAX_QUEUE_LENGTH)
	{
	  pending_queue = g_list_append(pending_queue,
					g_strdup( (gchar*)(list->data) ));
	  g_debug("adding queue file (%s)", (gchar*)(list->data));
	}
      list = g_list_next(list);
    }

  fx_play();
}

/** return a string representing a letter or number audio file
 *  get alphabet sound file name from gunichar
 *
 * the returned sound has the suffix .ogg
 *
 * \return a newly allocated string of the form U0033.ogg
 */

gchar *
gc_sound_alphabet(gchar *chars)
{
  gchar *next, *str, *prev, *result;
  gint i;
  gint length;
  gunichar next_unichar;

  length = g_utf8_strlen(chars, -1);

  next = chars;
  result = NULL;

  for (i=0; i < length; i++) {
    next_unichar = g_utf8_get_char(next);
    str = g_strdup_printf("U%.4X",(gint32) g_unichar_tolower(next_unichar));
    prev = result;
    if (prev)
      result = g_strconcat( prev, str, NULL);
    else
      result = g_strdup(str);

    g_free(str);
    g_free(prev);
    next = g_utf8_next_char(next);
  }

  gchar *result2 = g_strdup_printf("%s.ogg",result);
  g_free(result);

  return result2;
}


void gc_sound_callback(gchar *file)
{
  GcomprisSoundCallback cb;

  if (!sound_callbacks)
    return;

  cb = g_hash_table_lookup (sound_callbacks, file);

  if (cb)
    {
      g_debug("calling callback for %s", file);
      cb(file);
    }
  else
    g_debug("%s has no callback", file);

  g_hash_table_remove(sound_callbacks, file);

  g_free(file);
}

