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

#include <config.h>

#ifdef USE_SDLMIXER

#include "string.h"

#ifdef __APPLE__
#   include <sys/types.h>
#endif
#include "gcompris.h"
#include "soundutil_sdl.h"
#include <signal.h>
#include <glib.h>

static gboolean	 music_paused = FALSE;
static gboolean	 sound_closed = FALSE;

/* mutex */
GMutex		*lock = NULL;
GMutex		*lock_music = NULL;
GMutex		*lock_fx = NULL;
GCond		*cond = NULL;

/* Singleton */
static guint	 sound_init = 0;

/* Forward function declarations */
GThread		*thread_scheduler_fx, *thread_scheduler_music;

static void	*thread_play_ogg (gchar *file);

static gpointer  scheduler_fx (gpointer user_data);
static gpointer  scheduler_music (gpointer user_data);

/* sound control */
GObject *gc_sound_controller = NULL;
void gc_sound_callback_sdl(GcomprisSound *ctl,
			   gchar *file, gpointer user_data);

/* =====================================================================
 *
 * =====================================================================*/
void
gc_sound_init()
{

  /* Check to run the init once only */
  if(sound_init == 1)
    {
      if(sound_closed == TRUE)
	gc_sound_reopen();

	  return;
    }
  sound_init = 1;

  gc_sound_controller = g_object_new (GCOMPRIS_SOUND_TYPE, NULL);

  g_signal_connect( gc_sound_controller,
		    "sound-played",
		    (GCallback) gc_sound_callback_sdl,
		    NULL);

  g_assert( gc_sound_controller != NULL );

  /* Initialize the thread system */
  if (!g_thread_supported ()) g_thread_init (NULL);

  lock = g_mutex_new ();
  lock_music = g_mutex_new ();
  lock_fx = g_mutex_new ();
  cond = g_cond_new ();

  gc_sound_policy_set(PLAY_AFTER_CURRENT);


  if(sdlplayer_init()!=0) {
    /* Sound init failed. Desactivate the sound */
    gc_prop_get()->music = 0;
    gc_prop_get()->fx    = 0;
    return;
  }

  thread_scheduler_fx = g_thread_create((GThreadFunc)scheduler_fx, NULL, FALSE, NULL);
  if (thread_scheduler_fx == NULL)
    perror("create failed for fx scheduler");

  thread_scheduler_music = g_thread_create((GThreadFunc)scheduler_music, NULL, FALSE, NULL);
  if (thread_scheduler_music == NULL)
    perror("create failed for music scheduler");

}

void
gc_sound_bg_close()
{
  if ( !sound_closed )
    {
      sound_closed = TRUE;
      if ( music_paused ) {
         music_paused = FALSE;
	gc_sound_bg_resume();
         }
      g_mutex_lock(lock_fx);
      sdlplayer_halt_music();
      sdlplayer_halt_fx();
      g_mutex_lock(lock_music);
      sdlplayer_close();
    }
}

void
gc_sound_fx_close()
{
}

void
gc_sound_fx_reopen()
{
  if (gc_prop_get()->fx || gc_prop_get()->music) {
    if (sound_closed)
      {
	sdlplayer_reopen();
	g_mutex_unlock(lock_fx);
	g_mutex_unlock(lock_music);
	sound_closed = FALSE;
	music_paused = FALSE;
      }
  }
}

void
gc_sound_bg_reopen()
{
}

void
gc_sound_bg_pause()
{
  sdlplayer_pause_music();
  music_paused = TRUE;
}

void
gc_sound_bg_resume()
{
  sdlplayer_resume_music();
  music_paused = FALSE;
}

void
gc_sound_fx_pause()
{
}

void
gc_sound_fx_resume()
{
}

/* =====================================================================
 * Thread scheduler background :
 *	- launches a single thread for playing and play any file found
 *        in the gcompris music directory
 ======================================================================*/
static gpointer
scheduler_music (gpointer user_data)
{
  GcomprisProperties *properties = gc_prop_get();
  gint i;
  gchar *str;
  gchar *music_dir;
  GSList *musiclist = NULL;
  GDir *dir;
  const gchar *one_dirent;

  /* Sleep to let gcompris intialisation and intro music to complete */
  g_usleep(25000000);

  /* Load the Music directory file names */
  music_dir = g_strconcat(properties->package_data_dir, "/music/background", NULL);

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
	  musiclist = g_slist_insert (musiclist, str, RAND(0, g_slist_length(musiclist)));
	}
    }
  g_dir_close(dir);

  /* No music no play */
  if(g_slist_length(musiclist)==0)
    {
      g_free(music_dir);
      return NULL;
    }

  /* Now loop over all our music files */
  while (TRUE)
    {
      for(i=0; i<g_slist_length(musiclist); i++)
	{
	  /* Music can be disabled at any time */
	  while(!gc_prop_get()->music || music_paused || sound_closed)
	    g_usleep(1000000);

	  /* WARNING Displaying stuff in a thread seems to make gcompris unstable */
	  /*	  display_ogg_file_credits((char *)g_list_nth_data(musiclist, i)); */
	  //	  if(decode_ogg_file((char *)g_list_nth_data(musiclist, i))!=0)
	  g_mutex_lock(lock_music);
	  if(sdlplayer_music((char *)g_slist_nth_data(musiclist, i), 128)!=0){
	    g_warning("sdlplayer_music failed, try again in 5 seconds");
	    g_usleep(5000000);
	  }
	  g_mutex_unlock(lock_music);

	}
    }

  /* Never happen */
  g_slist_free(musiclist);
  g_warning( "The background thread music is stopped now. "\
	     "The files in %s are not ogg vorbis OR the sound output failed",
	     music_dir);
  g_free(music_dir);
  return NULL;
}

/* =====================================================================
 * Thread scheduler :
 *	- launches a single thread for playing a file
 *	- joins the previous thread at its end
 *	-	then launches another thread if some sounds are pending
 *	-	the thread never ends
 ======================================================================*/
static gpointer
scheduler_fx (gpointer user_data)
{
  char *sound = NULL;

  while (TRUE)
    {
      if ( ( sound = get_next_sound_to_play( ) ) != NULL )
	{
	  thread_play_ogg(sound);
	  g_free(sound);
	}
      else
	{
	  g_mutex_lock (lock);
	  g_cond_wait (cond, lock);
	  g_mutex_unlock (lock);
	}
    }
  return NULL;
}


/* =====================================================================
 * Thread function for playing a single file
 ======================================================================*/
static void*
thread_play_ogg (gchar *file)
{
  gchar *absolute_file;

  g_warning("  Thread_play_ogg %s", file);

  absolute_file = gc_file_find_absolute(file);

  if (!absolute_file)
    return NULL;

  g_warning("   Calling gcompris internal sdlplayer_file (%s)", absolute_file);
  g_mutex_lock(lock_fx);
  sdlplayer_fx(absolute_file, 128);
  g_mutex_unlock(lock_fx);
  g_signal_emit (gc_sound_controller,
		 GCOMPRIS_SOUND_GET_CLASS (gc_sound_controller)->sound_played_signal_id,
		 0 /* details */,
		 g_strdup(file));
  g_warning("  sdlplayer_fx(%s) ended.", absolute_file);

  g_free(absolute_file);

  return NULL;
}

void
fx_play()
{
  // Tell the scheduler to check for new sounds to play
  g_warning("Tell the scheduler to check for new sounds to play\n");
  g_cond_signal (cond);
}

void gc_sound_callback_sdl(GcomprisSound *ctl,
			   gchar *file, gpointer user_data)
{
  gc_sound_callback(file);
}

/*************************************/
/* GObject control sound             */
struct _GcomprisSoundPrivate
{
};

/* "gcompris-marshal.h" */

#include	<glib-object.h>

/* VOID:POINTER (gcompris-marshal.list:3) */
#define gnome_canvas_marshal_VOID__POINTER	g_cclosure_marshal_VOID__POINTER

static void
gc_sound_instance_init (GTypeInstance   *instance,
			      gpointer         g_class)
{
        GcomprisSound *self = (GcomprisSound *)instance;
        self->private = g_new (GcomprisSoundPrivate, 1);
}

static void
default_sound_played_signal_handler (GcomprisSound *obj, gchar *file, gpointer user_data)
{
        /* Here, we trigger the real file write. */
        g_warning ("sound_played: %s\n", file);
}

static void
gc_sound_class_init (gpointer g_class,
			   gpointer g_class_data)
{
        GcomprisSoundClass *klass = GCOMPRIS_SOUND_CLASS (g_class);

	klass->sound_played = default_sound_played_signal_handler;

        klass->sound_played_signal_id =
                g_signal_new ("sound-played",
			      G_TYPE_FROM_CLASS (g_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			      G_STRUCT_OFFSET (GcomprisSoundClass, sound_played),
			      NULL /* accumulator */,
			      NULL /* accu_data */,
			      gnome_canvas_marshal_VOID__POINTER,
			      G_TYPE_NONE /* return_type */,
			      1     /* n_params */,
			      G_TYPE_POINTER  /* param_types */);

}

GType gc_sound_get_type (void)
{
        static GType type = 0;
        if (type == 0) {
                static const GTypeInfo info = {
                        sizeof (GcomprisSoundClass),
                        NULL,   /* base_init */
                        NULL,   /* base_finalize */
                        gc_sound_class_init,   /* class_init */
                        NULL,   /* class_finalize */
                        NULL,   /* class_data */
                        sizeof (GcomprisSound),
                        0,      /* n_preallocs */
                        gc_sound_instance_init    /* instance_init */
                };
                type = g_type_register_static (G_TYPE_OBJECT,
                                               "GcomprisSoundType",
                                               &info, 0);
        }
        return type;
}

#endif

