/* gcompris - soundutil.c
 *
 * Copyright (C) 2002 Pascal Georges
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

#include "string.h"
#include <unistd.h>		/* sleep */

#ifdef __APPLE__
#   include <sys/types.h>
#endif
#include "gcompris.h"
#include <signal.h>
#include <glib.h>

static GList	*pending_queue = NULL;
static int	 sound_policy;
static gboolean	 is_playing;
static gboolean	 sound_closed = FALSE;

#if defined _WIN32 || defined __WIN32__
# undef WIN32   /* avoid warning on mingw32 */
# define WIN32
#endif

/* mutex */
GMutex		*lock = NULL;
GMutex		*lock_bg = NULL;
GCond		*cond = NULL;

/* Singleton */
static guint	 sound_init = 0;

/* Forward function declarations */
GThread		*thread_scheduler, *thread_scheduler_bgnd;

static void	*thread_play_ogg (char *file);
static char	*get_next_sound_to_play( );

static gpointer  scheduler (gpointer user_data);
static gpointer  scheduler_bgnd (gpointer user_data);

extern int	 ogg123(char * sound);

/* sound control */
GObject *gcompris_sound_controller = NULL;
void gcompris_sound_callback(GcomprisSound *ctl, gchar *file, gpointer user_data);
GHashTable *sound_callbacks = NULL;

/* =====================================================================
 *
 * =====================================================================*/
void initSound()
{

  gcompris_sound_controller = g_object_new (GCOMPRIS_SOUND_TYPE, NULL);

  g_signal_connect( gcompris_sound_controller, 
		    "sound-played",
		    (GCallback) gcompris_sound_callback,
		    NULL);

  g_assert( gcompris_sound_controller != NULL );

  /* Check to run the init once only */
  if(sound_init == 1) {
    return;
  }
  sound_init = 1;

  /* Initialize the thread system */
  if (!g_thread_supported ()) g_thread_init (NULL);

  lock = g_mutex_new ();
  lock_bg = g_mutex_new ();
  cond = g_cond_new ();

  sound_policy = PLAY_AFTER_CURRENT;
  is_playing = FALSE;

  if(sdlplayer_init()!=0) {
    /* Sound init failed. Desactivate the sound */
    gcompris_get_properties()->music = 0;
    gcompris_get_properties()->fx    = 0;
    return;
  }

  thread_scheduler = g_thread_create((GThreadFunc)scheduler, NULL, FALSE, NULL);
  if (thread_scheduler == NULL)
    perror("create failed for scheduler");

  thread_scheduler_bgnd = g_thread_create((GThreadFunc)scheduler_bgnd, NULL, FALSE, NULL);
  if (thread_scheduler_bgnd == NULL)
    perror("create failed for scheduler background");

}
void gcompris_close_sound()
{
  if ( ! ( sound_closed || ( ( gcompris_get_properties()->music ==  0 )
			     && ( gcompris_get_properties()->fx   == 0) ) ) ){
    g_mutex_lock(lock);
    g_mutex_lock(lock_bg);
    sdlplayer_close();
    sound_closed = TRUE;
  }
}
void gcompris_reopen_sound()
{
  if (sound_closed){
    sdlplayer_reopen();
    g_mutex_unlock(lock);
    g_mutex_unlock(lock_bg);
    sound_closed = FALSE;
  }
}

void gcompris_pause_sound(){
  sdlplayer_pause();
}

void gcompris_resume_sound(){
  sdlplayer_resume();
}

/* =====================================================================
 *
 * =====================================================================*/
void setSoundPolicy(int policy)
{
  switch (policy)
    {
    case PLAY_ONLY_IF_IDLE : sound_policy = PLAY_ONLY_IF_IDLE; break;
    case PLAY_AFTER_CURRENT : sound_policy = PLAY_AFTER_CURRENT; break;
    default : sound_policy = PLAY_AFTER_CURRENT;
    }
}
/* =====================================================================
 *
 * =====================================================================*/
int getSoundPolicy()
{
  return sound_policy;
}

/* =====================================================================
 * Thread scheduler background :
 *	- launches a single thread for playing and play any file found
 *        in the gcompris music directory
 ======================================================================*/
static gpointer scheduler_bgnd (gpointer user_data)
{
  gint i;
  gchar *str;
  gchar *filename;
  GList *musiclist = NULL;
  GDir *dir;
  const gchar *one_dirent;

  /* Sleep to let gcompris intialisation and intro music to complete */
#if defined WIN32
  sleep(20000);
#else
  sleep(20);
#endif

  /* Load the Music directory file names */
  filename = g_strdup_printf("%s", PACKAGE_DATA_DIR "/music/background");

  dir = g_dir_open(filename, 0, NULL);
      
  if (!dir) {
    g_warning ("Couldn't open music dir: %s", filename);
    g_free(filename);
    return NULL;
  }
  
  g_free(filename);
  
  /* Fill up the music list */
  while((one_dirent = g_dir_read_name(dir)) != NULL) {

    if (strcmp(one_dirent, "COPYRIGHT")) {
      str = g_strdup_printf("%s/%s", PACKAGE_DATA_DIR "/music/background", one_dirent);

      musiclist = g_list_append (musiclist, str);
    }
  }
  g_dir_close(dir);

  /* No music no play */
  if(g_list_length(musiclist)==0)
    return NULL;

  /* Now loop over all our music files */
  while (TRUE)
    {
      /* Music can be disabled at any time */
      if ( !gcompris_get_properties()->music )
	return NULL;

      for(i=0; i<g_list_length(musiclist); i++)
	{
	  /* WARNING Displaying stuff in a thread seems to make gcompris unstable */
	  /*	  display_ogg_file_credits((char *)g_list_nth_data(musiclist, i)); */
	  //	  if(decode_ogg_file((char *)g_list_nth_data(musiclist, i))!=0)
	  if(sdlplayer_bg((char *)g_list_nth_data(musiclist, i), 128)!=0){
	    printf("vire de sound_ng\n");
	    goto exit;
	  }
	  g_mutex_lock( lock_bg );
	  g_mutex_unlock( lock_bg );
	}
    }


 exit:
  g_list_free(musiclist);
  g_warning( "The background thread music is stopped now. The files in %s are not ogg vorbis OR the sound output failed", PACKAGE_DATA_DIR "/music/background");
  return NULL;
}
/* =====================================================================
 * Thread scheduler :
 *	- launches a single thread for playing a file
 *	- joins the previous thread at its end
 *	-	then launches another thread if some sounds are pending
 *	-	the thread never ends
 ======================================================================*/
static gpointer scheduler (gpointer user_data)
{
  char *sound = NULL;

  while (TRUE)
    {
      if ( ( sound = get_next_sound_to_play( ) ) != NULL )
	{
	  thread_play_ogg(sound);
	  is_playing = FALSE;
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
static void* thread_play_ogg (char *file)
{
  gchar *tmpstr;
  gchar *file_arg = g_strdup(file);

  g_warning(" thread_play_ogg %s \n", file);

  if (!g_file_test (file, G_FILE_TEST_IS_REGULAR)) {
    gchar *relative_filename;
    char   locale[3];
    strncpy( locale, gcompris_get_locale(), 2 );
    locale[2] = 0; // because strncpy does not put a '\0' at the end of the string
    relative_filename = g_strdup_printf("sounds/%s/%s.ogg", locale, file);
    tmpstr = gcompris_find_absolute_filename(relative_filename);
    g_free(relative_filename);
    if (!tmpstr){
      relative_filename = g_strdup_printf("music/%s.ogg", file);
      tmpstr = gcompris_find_absolute_filename(relative_filename);
      g_free(relative_filename);
      if (!tmpstr){
	/* Try to find a sound file that does not need to be localized 
	   (ie directly in root /sounds directory) */
	relative_filename = g_strdup_printf("sounds/%s.ogg", file);
	tmpstr = gcompris_find_absolute_filename(relative_filename);
	g_free(relative_filename);
	if (!tmpstr){
	  tmpstr = gcompris_find_absolute_filename(file);
	  if (!tmpstr){
	  g_warning("Can't find sound %s", file);
	  if (sound_callbacks)
	    g_hash_table_remove (sound_callbacks, file);
	  return NULL;
	  }
	}
      }
    }
    g_free( file );
    file = tmpstr;
  }

  if ( file )
    {
      g_warning("Calling gcompris internal sdlplayer_file(%s)\n", file);
      sdlplayer(file, 128);
      g_signal_emit (gcompris_sound_controller,
		     GCOMPRIS_SOUND_GET_CLASS (gcompris_sound_controller)->sound_played_signal_id,
		     0 /* details */,
		     file_arg);
      g_warning("sdlplayer_file(%s) ended.\n", file);
      g_free( file );
    }
  
  return NULL;
}

/* =====================================================================
 * Returns the next sound play, or NULL if there is no
 ======================================================================*/
char* get_next_sound_to_play( )
{
  char* tmpSound = NULL;

  g_mutex_lock (lock);

  if ( g_list_length(pending_queue) > 0 )
    {
      tmpSound = g_list_nth_data( pending_queue, 0 );
      pending_queue = g_list_remove( pending_queue, tmpSound );
      g_warning( "... get_next_sound_to_play : %s\n", tmpSound );
    }

  g_mutex_unlock (lock);

  return tmpSound;
}

/* =====================================================================
 * Play a OGG sound file.
 * gcompris_play_ogg function to process the sounds.
 ======================================================================*/

void	 gcompris_play_ogg_cb(gchar *file, GcomprisSoundCallback cb)
{

  g_assert ( cb != NULL);

  /* g_intern_string is in 2.10 */
  const gchar *intern_file = g_quark_to_string( g_quark_from_string(file));

  if (!sound_callbacks)
    sound_callbacks = g_hash_table_new_full (g_str_hash,
					    g_str_equal,
					    NULL,
					    NULL);

  /* i suppose there will not be two call of that function with same sound file before sound is played */
  g_hash_table_replace (sound_callbacks,
			intern_file,
			cb);
  gcompris_play_ogg(intern_file, NULL);
}

/* =====================================================================
 * Play a list of OGG sound files. The list must be NULL terminated
 * This function wraps the var args into a GList and call the 
 * gcompris_play_ogg_list function to process the sounds.
 ======================================================================*/
void gcompris_play_ogg(char *sound, ...)
{
  va_list ap;
  char* tmp = NULL;
  GList* list = NULL;

  if(!sound)
    return;

  list = g_list_append(list, sound);

  g_warning("Adding %s in the play list queue\n", sound);

  va_start( ap, sound);
  while( (tmp = va_arg (ap, char *)))
    {
      list = g_list_append(list, tmp);
    }
  va_end(ap);

  gcompris_play_ogg_list( list );
  
  g_list_free(list);  
}

/* =====================================================================
 * Play a list of OGG sound files. 
 * The given ogg files will be first tested as a locale dependant sound file:
 * sounds/<current gcompris locale>/<sound>
 * If it doesn't exists, then the test is done with a music file:
 * music/<sound>
 =====================================================================*/
void gcompris_play_ogg_list( GList* files )
{
  GList* list;

  if ( !gcompris_get_properties()->fx )
    return;

  if ( 	sound_policy == PLAY_ONLY_IF_IDLE &&
	( is_playing == TRUE || g_list_length( pending_queue ) > 0 ) )
    return;

  g_mutex_lock (lock);

  list = g_list_first( files );
  while( list!=NULL )
    {
      if (g_list_length(pending_queue) < MAX_QUEUE_LENGTH)
	{
	  pending_queue = g_list_append(pending_queue, g_strdup( (gchar*)(list->data) ));
	}
      list = g_list_next(list);
    }
  
  g_mutex_unlock (lock);

  // Tell the scheduler to check for new sounds to play
  g_warning("Tell the scheduler to check for new sounds to play\n");
  g_cond_signal (cond);

}

/* get alphabet sound file name from gunichar */

gchar *gcompris_alphabet_sound(gchar *chars)
{
  gchar *next, *str, *prev, *result;
  gint i;
  gint length;
  gunichar next_unichar;

  length = g_utf8_strlen(chars, -1);

  next = chars;
  result = NULL;
  str = g_new0(gchar, 6);

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


void gcompris_sound_callback(GcomprisSound *ctl, gchar *file, gpointer user_data)
{
  GcomprisSoundCallback cb;

  if (!sound_callbacks)
    return;
  
  cb = g_hash_table_lookup (sound_callbacks, file);

  if (cb){
    g_warning("calling callback for %s", file);
    cb(file);
  }
  else
    g_warning("%s has no callback", file);
  g_hash_table_remove(sound_callbacks, file);

}

/*************************************/
/* GObject control sound             */
struct _GcomprisSoundPrivate
{
};

#include "gcompris-marshal.h"

static void
gcompris_sound_instance_init (GTypeInstance   *instance,
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
gcompris_sound_class_init (gpointer g_class,
                      gpointer g_class_data)
{
        GObjectClass *gobject_class = G_OBJECT_CLASS (g_class);
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

GType gcompris_sound_get_type (void)
{
        static GType type = 0;
        if (type == 0) {
                static const GTypeInfo info = {
                        sizeof (GcomprisSoundClass),
                        NULL,   /* base_init */
                        NULL,   /* base_finalize */
                        gcompris_sound_class_init,   /* class_init */
                        NULL,   /* class_finalize */
                        NULL,   /* class_data */
                        sizeof (GcomprisSound),
                        0,      /* n_preallocs */
                        gcompris_sound_instance_init    /* instance_init */
                };
                type = g_type_register_static (G_TYPE_OBJECT,
                                               "GcomprisSoundType",
                                               &info, 0);
        }
        return type;
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
