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

#include "gcompris.h"
#include <signal.h>
#include <pthread.h>
#include <ao/ao.h>

static GList *pending_queue = NULL;
static int sound_policy;
static gboolean is_playing;

/* Forward function declarations */
pthread_t thread_scheduler, thread_play;
static void*	 thread_play_ogg (void*);
static char*	 get_next_sound_to_play( );
static void*	 scheduler ( );
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

  printf("...calling ao_initialize\n");
  ao_initialize();
  printf("...calling ao_initialize done\n");

  if ( pthread_create ( &thread_scheduler, NULL, scheduler, NULL ) != 0)
    perror("create failed for scheduler");
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

	  printf ("   in scheduler: before cond_wait \n");
	  err = pthread_cond_wait (&cond, &lock);
	  if (err)
	    printf ("cond_wait  : %s\n", strerror (err));

	  printf ("   in scheduler: after cond_wait \n");

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
  pthread_t pid_ogg = 0;

  fprintf (stderr, "+++thread_play_ogg:%s<-\n", s);

  strncpy( locale, gcompris_get_locale(), 2 );
  locale[2] = 0; // because strncpy does not put a '\0' at the end of the string

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
	  }
	else
	  {
	    g_free(file);
	    g_warning("Can't find sound %s", s);
	    return NULL;
	  }
      }

  if ( file )
    {
      printf("Calling decode_ogg_file(%s)\n");
      decode_ogg_file(file);
      g_free( file );
    }

  fprintf (stderr, "---thread_play_ogg\n");
  //  pthread_exit( NULL );
  return NULL;
}

/* =====================================================================
 * Returns the next sound play, or NULL if there is no
 ======================================================================*/
char* get_next_sound_to_play( )
{
  char* tmpSound = NULL;

  printf("+++get_next_sound_to_play\n");
  pthread_mutex_lock( &lock );

  if ( g_list_length(pending_queue) > 0 )
    {
      tmpSound = g_list_nth_data( pending_queue, 0 );
      pending_queue = g_list_remove( pending_queue, tmpSound );
      printf( "... get_next_sound_to_play : %s\n", tmpSound );
    }

  pthread_mutex_unlock( &lock );
  printf("---get_next_sound_to_play\n");

  return tmpSound;
}
/* =====================================================================
 * Play a list of OGG sound files. The list must be NULL terminated
 * The given ogg files will be first tested as a locale dependant sound file:
 * sounds/<current gcompris locale>/<sound>
 * If it doesn't exists, then the test is done with a music file:
 * music/<sound>
 ======================================================================*/
void gcompris_play_ogg(char *sound, ...)
{
  int err;
  va_list ap;
  char* tmp = NULL;
  char* tmpSound = NULL;

  printf("+++gcompris_play_ogg\n");

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

  if (g_list_length(pending_queue) < MAX_QUEUE_LENGTH)
    pending_queue = g_list_append(pending_queue, g_strdup( sound ) );

  va_start( ap, sound);
  while( (tmp = va_arg (ap, char *)))
    {
      if (g_list_length(pending_queue) < MAX_QUEUE_LENGTH)
	{
	  pending_queue = g_list_append(pending_queue, g_strdup( tmp ));
	}
    }

  va_end(ap);

  err = pthread_mutex_unlock ( &lock);
  if (err)
    printf ("mutex_unlock: %s\n", strerror (err));

  // Tell the scheduler to check for new sounds to play
  printf("Tell the scheduler to check for new sounds to play\n");
  err = pthread_cond_signal (&cond);
  if (err)
    printf ("cond_signal : %s\n", strerror (err));

  printf("---gcompris_play_ogg\n");
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
