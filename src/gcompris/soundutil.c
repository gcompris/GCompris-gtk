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

#ifdef __APPLE__
#   include <sys/types.h>
#endif
#include <dirent.h>

#include "gcompris.h"
#include <signal.h>
#include <pthread.h>
#include <ao/ao.h>

static GList *pending_queue = NULL;
static int sound_policy;
static gboolean is_playing;

/* Forward function declarations */
pthread_t thread_scheduler, thread_play;
pthread_t thread_scheduler_bgnd, thread_play_bgnd;
static void*	 thread_play_ogg (void*);
static char*	 get_next_sound_to_play( );
static void*	 scheduler ( );
static void*	 scheduler_bgnd ( );
extern int	 ogg123(char * sound);

/* mutex */
pthread_mutex_t lock;
pthread_cond_t cond;

/* =====================================================================
 *
 * =====================================================================*/
void initSound()
{
  pthread_mutexattr_t mutattr;
  pthread_mutexattr_init (&mutattr);
  pthread_mutex_init( &lock, &mutattr );
  pthread_cond_init( &cond, NULL );
  sound_policy = PLAY_AFTER_CURRENT;
  is_playing = FALSE;

  ao_initialize();

  if ( pthread_create ( &thread_scheduler, NULL, scheduler, NULL ) != 0)
    perror("create failed for scheduler");

  if ( pthread_create ( &thread_scheduler_bgnd, NULL, scheduler_bgnd, NULL ) != 0)
    perror("create failed for scheduler background");

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
    case PLAY_OVERRIDE_ALL : sound_policy = PLAY_OVERRIDE_ALL; break;
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
static void* scheduler_bgnd ()
{
  gint i;
  gchar *str;
  gchar *filename;
  struct dirent **namelist = NULL;
  int namelistlength = 0;
  GList *musiclist = NULL;

  if ( !gcompris_get_properties()->music )
    return;

  /* Sleep to let gcompris intialisation and intro music to complete */
  sleep(20);

  /* Load the Music directory file names */
  filename = g_strdup_printf("%s", PACKAGE_DATA_DIR "/music/background");
  namelistlength = scandir(filename,
			   &namelist, 0, NULL);
  
  if (namelistlength < 0)
    g_warning (_("Couldn't open music dir: %s"), filename);
  
  g_free(filename);
  
  /* Fill up the music list */
  for(i=2; i<namelistlength; i++)
    {
      str = g_strdup_printf("%s/%s", PACKAGE_DATA_DIR "/music/background", namelist[i]->d_name);

      g_free(namelist[i]);

      musiclist = g_list_append (musiclist, str);
    }

  g_free(namelist);

  /* No music no play */
  if(g_list_length(musiclist)==0)
    return NULL;

  /* Now loop over all our music files */
  while (TRUE)
    {
      for(i=0; i<g_list_length(musiclist); i++)
	{
	  /* WARNING Displaying stuff in a thread seems to make gcompris unstable */
	  /*	  display_ogg_file_credits((char *)g_list_nth_data(musiclist, i)); */
	  if(decode_ogg_file((char *)g_list_nth_data(musiclist, i))!=0)
	    goto exit;
	}
    }


 exit:
  g_list_free(musiclist);
  g_warning("The background thread music is stopped now. The files in %s are not ogg vorbis OR the sound output failed", PACKAGE_DATA_DIR "/music/background");
  return NULL;
}
/* =====================================================================
 * Thread scheduler :
 *	- launches a single thread for playing a file
 *	- joins the previous thread at its end
 *	-	then launches another thread if some sounds are pending
 *	-	the thread never ends
 ======================================================================*/
static void* scheduler ()
{
  int retcode;
  char *sound = NULL;

  while (TRUE)
    {
      if ( ( sound = get_next_sound_to_play( ) ) != NULL )
	{
	  thread_play_ogg(sound);
	  is_playing = FALSE;
	  g_free(sound);
	}
      else
	{
	  int err;

	  err = pthread_cond_wait (&cond, &lock);
	  if (err)
	    printf ("cond_wait  : %s\n", strerror (err));

	  err = pthread_mutex_unlock ( &lock);
	  if (err)
	    printf ("mutex_unlock: %s\n", strerror (err));
	}
    }
  return NULL;
}
/* =====================================================================
 * Thread function for playing a single file
 ======================================================================*/
static void* thread_play_ogg (void *s)
{
  char* file = NULL;
  char locale[3];

  strncpy( locale, gcompris_get_locale(), 2 );
  locale[2] = 0; // because strncpy does not put a '\0' at the end of the string

  if(((char *)s)[0]=='/')
    {
      /* If the given file starts with a / then we don't need to search it */
      file = g_strdup(s);
    }
  else
    {
      file = g_strdup_printf("%s/%s/%s.ogg", PACKAGE_DATA_DIR "/sounds", locale, s);
      
      if (g_file_test ((file), G_FILE_TEST_EXISTS))
	{
	  printf("trying to play %s\n", file);
	} else
	  {
	    g_free(file);
	    file = g_strdup_printf("%s/%s.ogg", PACKAGE_DATA_DIR "/music", s);
	    if (g_file_test ((file), G_FILE_TEST_EXISTS))
	      {
		printf("trying to play %s\n", file);
	      } else
		/* Try to find a sound file that does not need to be localized 
		   (ie directly in root /sounds directory) */
		{
		  g_free(file);
		  file = g_strdup_printf("%s/%s.ogg", PACKAGE_DATA_DIR "/sounds", s);
		  if (g_file_test ((file), G_FILE_TEST_EXISTS))
		    {
		      printf("trying to play %s\n", file);
		    } else
		      {
			g_free(file);
			g_warning("Can't find sound %s", s);
			return NULL;
		      }
		}
	  }
    }

  if ( file )
    {
      printf("Calling decode_ogg_file(%s)\n", file);
      decode_ogg_file(file);
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

  pthread_mutex_lock( &lock );

  if ( g_list_length(pending_queue) > 0 )
    {
      tmpSound = g_list_nth_data( pending_queue, 0 );
      pending_queue = g_list_remove( pending_queue, tmpSound );
      printf( "... get_next_sound_to_play : %s\n", tmpSound );
    }

  pthread_mutex_unlock( &lock );

  return tmpSound;
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

  list = g_list_append(list, sound);

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
  int err;
  char* tmp = NULL;
  char* tmpSound = NULL;

  if ( !gcompris_get_properties()->fx )
    return;

  if ( 	sound_policy == PLAY_ONLY_IF_IDLE &&
	( is_playing == TRUE || g_list_length( pending_queue ) > 0 ) )
    return;

  if (sound_policy == PLAY_OVERRIDE_ALL)
    {
      // cancel playing thread
      if ( pthread_cancel(thread_play) != 0)
	perror("thread cancel failed:");

      // cancel all pending sounds
      pthread_mutex_lock( &lock );
      while ( g_list_length(pending_queue) > 0 )
	{
	  tmpSound = g_list_nth_data(pending_queue, 0);
	  pending_queue = g_list_remove(pending_queue, tmpSound);
	  g_free(tmpSound);
	}
      pthread_mutex_unlock( &lock );

    } // PLAY_OVERRIDE_ALL

  pthread_mutex_lock( &lock );

  list = g_list_first( files );
  while( list!=NULL )
    {
      if (g_list_length(pending_queue) < MAX_QUEUE_LENGTH)
	{
	  pending_queue = g_list_append(pending_queue, g_strdup( (gchar*)(list->data) ));
	}
      list = g_list_next(list);
    }
  
  err = pthread_mutex_unlock ( &lock);
  if (err)
    printf ("mutex_unlock: %s\n", strerror (err));

  // Tell the scheduler to check for new sounds to play
  printf("Tell the scheduler to check for new sounds to play\n");
  err = pthread_cond_signal (&cond);
  if (err)
    printf ("cond_signal : %s\n", strerror (err));

}
/* =====================================================================
 *     Play a sound installed in the Gnome sound list
 * ======================================================================*/
void gcompris_play_sound (const char *soundlistfile, const char *which)
{
  gchar *filename;

  if (!gcompris_get_properties()->fx)
    return;

  filename = g_strdup_printf("%s/%s.wav", PACKAGE_SOUNDS_DIR, which);
  // DEBUG
  printf("gcompris_play_sound %s\n", filename);
  if (!g_file_test ((filename), G_FILE_TEST_EXISTS))
    g_error (_("Couldn't find file %s !"), filename);

  gnome_sound_play (filename);

  g_free (filename);
}

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
