/* gcompris - gameutil.c
 *
 * Time-stamp: <2002/02/12 19:47:08 bcoudoin>
 *
 * Copyright (C) 2000 Bruno Coudoin
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

static gboolean sound_playing_1 = FALSE;
static gboolean sound_playing_2 = FALSE;
static pid_t sound_pid_1 = 0;
static pid_t sound_pid_2 = 0;

typedef void (*sighandler_t)(int);

/* =====================================================================
 * Generic code to remove zombie processes
 ======================================================================*/
void zombie_cleanup(void)
{
  int pid;

  while((pid = waitpid(-1, NULL, WNOHANG)))
    {
		  if (pid == sound_pid_1)
				sound_playing_1 = FALSE;

		  if (pid == sound_pid_2)
				sound_playing_2 = FALSE;

      if(pid == -1)
	  		g_error("Error waitpid");

    }
}

/* =====================================================================
 * Process the cleanup of the child (no zombies)
 * And update our status as not playing ogg
 ======================================================================*/
void child_end(int  signum)
{
  int pid;

	pid = waitpid(-1, NULL, WNOHANG);
  if (pid == sound_pid_1)
		sound_playing_1 = FALSE;

  if (pid == sound_pid_2)
		sound_playing_2 = FALSE;

	if (pid == -1)
      g_error("Error waitpid");
}

/* =====================================================================
 * Play a list of OGG sound files. The list must be NULL terminated
 * should have used threads instead of fork + exec calls
 * The given ogg files will be first tested as a locale dependant sound file:
 * sounds/<current gcompris locale>/<sound>
 * If it doesn't exists, then the test is done with a music file:
 * music/<sound>
 ======================================================================*/
void gcompris_play_ogg(char *sound, ...) {
  va_list ap;
  char * s = NULL;
  char *argv[20];
  char locale[3];
  int argc = 0;
  pid_t pid = 0;
	pid_t *ppid = NULL;

	if (!gcompris_get_properties()->fx)
    return;

	/* Only 2 sounds can be played : the current one and a pending.
	 * The pending sound is the last coming in.
 	 */
	assert( (!sound_playing_1) || (sound_playing_1 && !sound_playing_2) || (sound_playing_1 && sound_playing_2));

  if (!sound_playing_1) {
		ppid = &sound_pid_1;
		sound_playing_1 = TRUE;
	} else {
			ppid = &sound_pid_2;
			if (sound_playing_2) { //sound_playing_1 && sound_playing_2
				// kill the last pending sound
				if (kill(sound_pid_2, SIGKILL) != 0) {
					perror("Kill failed:");
				}
				sound_playing_2 = TRUE;
			} else { //sound_playing_1 && !sound_playing_2
					sound_playing_2 = TRUE;
				}
		}

	assert(ppid != NULL);

  strncpy(locale,gcompris_get_locale(),2);
  locale[2] = 0; // because strncpy does not put a '\0' at the end of the string

  signal(SIGCHLD, child_end);

  pid = fork ();
  *ppid = pid;

  if (pid > 0) { // go back to gcompris
    return;
  } else if (pid == 0) { // child process
    argc = 0;
    argv[argc++] = "ogg123";
    //    argv[argc++] = "-v";
    argv[argc] = g_strdup_printf("%s/%s/%s.ogg", PACKAGE_DATA_DIR "/sounds", locale, sound);
    if (g_file_exists (argv[argc])) {
      printf("trying to play %s\n", argv[argc]);
      argc++;
    } else {
      g_free(argv[argc]);
      argv[argc] = g_strdup_printf("%s/%s.ogg", PACKAGE_DATA_DIR "/music", sound);
      if (g_file_exists (argv[argc])) {
	printf("trying to play %s\n", argv[argc]);
	argc++;
      } else
	g_free(argv[argc]);
    }

    va_start( ap, sound);
    while( (s = va_arg (ap, char *))) {
      argv[argc] = g_strdup_printf("%s/%s/%s.ogg", PACKAGE_DATA_DIR "/sounds", locale, s);
      printf("trying to play %s\n", argv[argc]);
      if (!g_file_exists (argv[argc]))
	argv[argc] = g_strdup_printf("%s/%s.ogg", PACKAGE_DATA_DIR "/music", s);

      if (!g_file_exists (argv[argc])) {
	g_warning (_("Couldn't find file %s !"), argv[argc]);
	g_free(argv[argc]);
	//				continue;
      }
      else
	argc ++;
    }
    va_end(ap);
    argv[argc] = NULL;
    execvp( "ogg123", argv);
  } else {
    fprintf(stderr, "Unable to fork\n");
  }

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

  if (!g_file_exists (filename)) {
    g_error (_("Couldn't find file %s !"), filename);
  }
  if (gcompris_get_properties()->fx) {
    gnome_sound_play (filename);
  }

  g_free (filename);
}

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
