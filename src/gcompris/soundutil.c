/* gcompris - soundutil.c
 *
 * Copyright (C) 2002 Pascal Georges
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

#ifdef __APPLE__
#   include <sys/types.h>
#endif
#include "gcompris.h"
#include <signal.h>
#include <glib.h>
#include <gst/gst.h>

static GList	*pending_queue = NULL;
static int	 sound_policy;
static gboolean	 fx_paused = FALSE;
static gboolean	 bg_paused = FALSE;

static GstElement *bg_pipeline = NULL;
static GstElement *fx_pipeline = NULL;

static guint bg_music_index;

GSList *music_list;

/* Singleton */
static guint	 sound_init = 0;

/* Forward function declarations */
static void	 fx_play ();
static char	*get_next_sound_to_play( );

static gpointer  bg_play (gpointer dummy);
static GSList   *bg_build_music_list();

/* sound control */
void gc_sound_callback(gchar *file);
GHashTable *sound_callbacks = NULL;

/* =====================================================================
 *
 * =====================================================================*/
void
gc_sound_init()
{

  /* Check to run the init only once */
  if(sound_init == 1)
    return;

  sound_init = 1;

  /* gstreamer init */
  gst_init(NULL, NULL);

  sound_policy = PLAY_AFTER_CURRENT;

  music_list = bg_build_music_list();
}

static gboolean
fx_bus(GstBus* bus, GstMessage* msg, gpointer data)
{
  switch( GST_MESSAGE_TYPE( msg ) )
    {
    case GST_MESSAGE_EOS:
      g_warning("fx_bus: EOS START");
      gc_sound_fx_close();
      gc_sound_callback((gchar *)data);
      fx_play();
      g_warning("fx_bus: EOS END");
      break;
    default:
      break;
    }

  return TRUE;
}

static gboolean
bg_bus(GstBus* bus, GstMessage* msg, gpointer data)
{
  switch( GST_MESSAGE_TYPE( msg ) ) {
    case GST_MESSAGE_EOS:
        g_warning("bg_bus: EOS");
	gc_sound_bg_close();
	bg_play(NULL);
	break;
    default:
	break;
  }
  return TRUE;
}

void
gc_sound_close()
{
  gc_sound_bg_close();
  gc_sound_fx_close();
}

void
gc_sound_bg_close()
{
  if (bg_pipeline)
    {
      gst_element_set_state(bg_pipeline, GST_STATE_NULL);
      gst_element_get_state(bg_pipeline, NULL, NULL, 1000*GST_MSECOND);
      gst_object_unref(GST_OBJECT(bg_pipeline));
      bg_pipeline = NULL;
    }
}

void
gc_sound_fx_close()
{
  g_warning("gc_sound_fx_close");
  if (fx_pipeline)
    {
      gst_element_set_state(fx_pipeline, GST_STATE_NULL);
      gst_element_get_state(fx_pipeline, NULL, NULL, 1000*GST_MSECOND);
      gst_object_unref(GST_OBJECT(fx_pipeline));
      fx_pipeline = NULL;
    }
}

void
gc_sound_bg_reopen()
{
  if(gc_prop_get()->music)
    bg_play(NULL);
}

void
gc_sound_fx_reopen()
{
}

void
gc_sound_reopen()
{
  gc_sound_bg_reopen();
  gc_sound_fx_reopen();
}

void
gc_sound_bg_pause()
{
  if (bg_pipeline)
  {
    gst_element_set_state(bg_pipeline, GST_STATE_PAUSED);
  }
  bg_paused = TRUE;
}

void
gc_sound_bg_resume()
{
  if(bg_pipeline)
  {
    gst_element_set_state(bg_pipeline, GST_STATE_PLAYING);
    gst_element_get_state(bg_pipeline, NULL, NULL, 1000*GST_MSECOND);
  }
  bg_paused = FALSE;
}

void
gc_sound_fx_pause()
{
  if (fx_pipeline)
  {
    gst_element_set_state(fx_pipeline, GST_STATE_PAUSED);
  }
  fx_paused = TRUE;
}

void
gc_sound_fx_resume()
{
  if(fx_pipeline)
  {
    gst_element_set_state(fx_pipeline, GST_STATE_PLAYING);
    gst_element_get_state(fx_pipeline, NULL, NULL, 1000*GST_MSECOND);
  }
  fx_paused = FALSE;
}

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

static GSList *
bg_build_music_list()
{
  GcomprisProperties *properties = gc_prop_get();
  gchar *str;
  gchar *music_dir;
  GSList *musiclist = NULL;
  GDir *dir;
  const gchar *one_dirent;

  bg_music_index = 0;

  /* Load the Music directory file names */
  music_dir = g_strconcat(properties->package_data_dir, "/music/background",
			  NULL);

  dir = g_dir_open(music_dir, 0, NULL);

  if (!dir) {
    g_warning ("Couldn't open music dir: %s", music_dir);
    g_free(music_dir);
    return NULL;
  }

  /* Fill up the music list */
  while((one_dirent = g_dir_read_name(dir)) != NULL)
    {
      if (g_str_has_suffix(one_dirent, ".ogg"))
	{
	  str = g_strdup_printf("%s/%s", music_dir, one_dirent);
	  musiclist = g_slist_insert (musiclist, str,
				      RAND(0, g_slist_length(musiclist)));
	}
    }
  g_dir_close(dir);

  /* No music no play */
  if(g_slist_length(musiclist)==0)
    {
      g_free(music_dir);
      return NULL;
    }

  return(musiclist);
}

/* =====================================================================
 * Thread scheduler background :
 *	- launches a single thread for playing and play any file found
 *        in the gcompris music directory
 ======================================================================*/
static gpointer
bg_play(gpointer dummy)
{
  gchar *absolute_file;

  /* Music wrapping */
  if(bg_music_index >= g_slist_length(music_list))
    bg_music_index = 0;

  absolute_file = gc_file_find_absolute(g_slist_nth_data(music_list,
							 bg_music_index));

  if (!absolute_file)
    return NULL;

  bg_pipeline = gst_element_factory_make ("playbin", "play");

  if(!bg_pipeline)
    {
      g_warning("Failed to build the gstreamer pipeline (for background music)");
      gc_prop_get()->music = 0;
      return NULL;
    }

  gst_bus_add_watch (gst_pipeline_get_bus (GST_PIPELINE (bg_pipeline)),
		     bg_bus, bg_pipeline);


  gchar *uri = g_strconcat("file://", absolute_file, NULL);
  g_free(absolute_file);
  g_warning("  bg_play %s", uri);

  g_object_set (G_OBJECT (bg_pipeline), "uri", uri, NULL);

  GstStateChangeReturn statechanged = gst_element_set_state (bg_pipeline,
							     GST_STATE_PLAYING);
  gst_element_get_state(bg_pipeline, NULL, NULL, 1000*GST_MSECOND);
  if( statechanged == GST_STATE_CHANGE_SUCCESS) {
    g_warning("%s : bg_playing\n",__FUNCTION__);
  }

  g_free(uri);

  return(NULL);
}

/* =====================================================================
 * Thread function for playing a single file
 ======================================================================*/
static void
fx_play()
{
  gchar *file;
  gchar *absolute_file;
  GcomprisProperties *properties = gc_prop_get();

  if(fx_pipeline)
    return;

  file = get_next_sound_to_play();

  if(!file)
    return;

  g_warning("  fx_play %s", file);

  absolute_file = gc_file_find_absolute(file);

  if (!absolute_file ||
      !properties->fx)
    return;

  fx_pipeline = gst_element_factory_make ("playbin", "play");

  if (!fx_pipeline)
    {
      g_warning("Failed to build the gstreamer pipeline");
      gc_prop_get()->fx = 0;
      return;
    }

  gchar *uri = g_strconcat("file://", absolute_file, NULL);
  g_free(absolute_file);
  g_warning("   uri '%s'", uri);

  g_object_set (G_OBJECT (fx_pipeline), "uri", uri, NULL);
  gst_bus_add_watch (gst_pipeline_get_bus (GST_PIPELINE (fx_pipeline)),
		     fx_bus, file);

  GstStateChangeReturn statechanged = gst_element_set_state (fx_pipeline,
							     GST_STATE_PLAYING);
  gst_element_get_state(fx_pipeline, NULL, NULL, 1000*GST_MSECOND);
  if( statechanged == GST_STATE_CHANGE_SUCCESS) {
    g_warning("%s : fx_playing\n",__FUNCTION__);
  }

  g_free(uri);

  return;
}

/* =====================================================================
 * Returns the next sound play, or NULL if there is no
 ======================================================================*/
static char*
get_next_sound_to_play( )
{
  char* tmpSound = NULL;

  if ( g_list_length(pending_queue) > 0 )
    {
      tmpSound = g_list_nth_data( pending_queue, 0 );
      pending_queue = g_list_remove( pending_queue, tmpSound );
      g_warning( "... get_next_sound_to_play : %s\n", tmpSound );
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

  g_warning("Adding %s in the play list queue\n", sound);

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
  char* tmpSound = NULL;

  if ( !gc_prop_get()->fx )
    return;

  if (sound_policy == PLAY_ONLY_IF_IDLE &&
        g_list_length( pending_queue ) > 0 )
    return;

  if (sound_policy == PLAY_AND_INTERRUPT ) {
    g_warning("halt music");
    while ( g_list_length(pending_queue) > 0 )
    {
      tmpSound = g_list_nth_data( pending_queue, 0 );
      pending_queue = g_list_remove( pending_queue, tmpSound );
      g_free(tmpSound);
    }
    //    sdlplayer_halt_fx();
  }

  list = g_list_first( files );
  while( list!=NULL )
    {
      if (g_list_length(pending_queue) < MAX_QUEUE_LENGTH)
	{
	  pending_queue = g_list_append(pending_queue,
					g_strdup( (gchar*)(list->data) ));
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
      g_warning("calling callback for %s", file);
      cb(file);
    }
  else
    g_warning("%s has no callback", file);

  g_hash_table_remove(sound_callbacks, file);

  g_free(file);
}

