/* gcompris - soundutil.c
 *
 * Time-stamp: <2002/04/29 01:43:31 bruno>
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

static GList *pending_queue = NULL;
static GList *playing_queue = NULL;

static int sound_policy;
static int sound_channels_used;
static int max_sound_channels;

static gboolean ogg_avalaible;

void child_end(int  signum);

typedef void (*sighandler_t)(int);

/* =====================================================================
 *
 * =====================================================================*/
void initSound() {
	sound_policy = PLAY_AFTER_CURRENT;
	sound_channels_used = 0;
	max_sound_channels = 1;
	if (gnome_is_program_in_path("ogg123") == NULL) {
		ogg_avalaible = FALSE;
		} else {
			ogg_avalaible = TRUE;
			}
}

/* =====================================================================
 *
 * =====================================================================*/
void setSoundPolicy(int policy) {
	switch (policy) {
		case PLAY_ONLY_IF_IDLE : sound_policy = PLAY_ONLY_IF_IDLE; break;
		case PLAY_AFTER_CURRENT : sound_policy = PLAY_AFTER_CURRENT; break;
		case PLAY_OVERRIDE_ALL : sound_policy = PLAY_OVERRIDE_ALL; break;
		default : sound_policy = PLAY_AFTER_CURRENT;
	}
}
/* =====================================================================
 *
 * =====================================================================*/
int getSoundPolicy() {
	return sound_policy;
}
/* =====================================================================
 * Process the cleanup of the child (no zombies)
 * =====================================================================*/
void child_end(int  signum)
{
  pid_t pid;
	int i;
	tsSound * tmpSound;

	pid = waitpid(-1, NULL, WNOHANG);
printf("child_end pid=%d  pending = %d playing = %d\n", pid,g_list_length(pending_queue),g_list_length(playing_queue));

  if (pid == -1)
    g_error("Error waitpid");
		else {
			sound_channels_used--;
			for (i=0; i<g_list_length(playing_queue) ; i++) {
				tmpSound = g_list_nth_data(playing_queue, i);
				if ( tmpSound->pid == pid ) {
					printf("Found PID\n");
					g_free(tmpSound->string);
					playing_queue = g_list_remove(playing_queue, tmpSound);
					free(tmpSound);
					break;
				}
			} // FOR


			for (i=0; i<g_list_length(pending_queue); i++) {

				if (sound_channels_used >= max_sound_channels)
					break;

				tmpSound = g_list_nth_data(pending_queue, i);
				pid = exec_play(tmpSound->string);
				if (pid!=-1) {// play is OK
					pending_queue = g_list_remove(pending_queue, tmpSound);
					playing_queue = g_list_append(playing_queue, tmpSound);
					tmpSound->pid = pid;
				} else {
					/* could not play sound -> remove it from pending queues */
					pending_queue = g_list_remove(pending_queue, tmpSound);
					g_free(tmpSound->string);
					free(tmpSound);
				}
			} // FOR
		}
}

/* =====================================================================
 * returns -1 if sound can't be played, the pid of the sound process otherwise
 * =====================================================================*/
pid_t exec_play(char *s) {
  char * sound = NULL, *tmp = NULL;
  char locale[3];
  pid_t pid = 0;
	int argc;
	char *argv[MAX_SOUND_FILES];

	strncpy(locale,gcompris_get_locale(),2);
  locale[2] = 0; // because strncpy does not put a '\0' at the end of the string

  signal(SIGCHLD, child_end);

  pid = fork ();

  if (pid > 0) { // go back to gcompris
	printf("+++execplay %s child pid = %d pending = %d playing = %d\n",s, pid,g_list_length(pending_queue),g_list_length(playing_queue) );
		sound_channels_used++;
    return pid;
  } else if (pid == 0) { // child process
		argc = 0;
    argv[argc++] = "ogg123";
		argv[argc++] = "-q";

		tmp = s;
		while ( (sound = strtok(tmp, " "))) {
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
			tmp = NULL;
		} // WHILE

    argv[argc] = NULL;
		execvp( "ogg123", argv);
  } else {
    fprintf(stderr, "Unable to fork\n");
  }

	return -1;
}
/* =====================================================================
 * Play a list of OGG sound files. The list must be NULL terminated
 * The given ogg files will be first tested as a locale dependant sound file:
 * sounds/<current gcompris locale>/<sound>
 * If it doesn't exists, then the test is done with a music file:
 * music/<sound>
 ======================================================================*/
void gcompris_play_ogg(char *sound, ...) {
  va_list ap;
  gchar * s = NULL;
	char * tmp = NULL;
	pid_t pid;
	tsSound * tmpSound = NULL;

	if (!gcompris_get_properties()->fx || !ogg_avalaible)
    return;

	if (sound_policy == PLAY_ONLY_IF_IDLE && (g_list_length(playing_queue) > 0 || g_list_length(pending_queue) > 0))
		return;

	if (sound_policy == PLAY_OVERRIDE_ALL) {
		while ( g_list_length(playing_queue) > 0 ) {
			tmpSound = g_list_nth_data(playing_queue, 0);
		  // kill all playing sounds
      if (kill(tmpSound->pid, SIGKILL) != 0)
					perror("Kill failed:");
			g_free(tmpSound->string);
			playing_queue = g_list_remove(playing_queue, tmpSound);
			free(tmpSound);
		}

		// cancel all pending sounds
		while ( g_list_length(pending_queue) > 0 ) {
			tmpSound = g_list_nth_data(pending_queue, 0);
			g_free(tmpSound->string);
			pending_queue = g_list_remove(pending_queue, tmpSound);
			free(tmpSound);
		}

		sound_channels_used = 0;
	} // PLAY_OVERRIDE_ALL

	// PLAY_AFTER_CURRENT : the sound is played after current running ones, all pending sounds are cancelled
	if (sound_policy == PLAY_AFTER_CURRENT) {
		while ( g_list_length(pending_queue) > 0 ) {
			tmpSound = g_list_nth_data(pending_queue, 0);
			assert(tmpSound);
			g_free(tmpSound->string);
			pending_queue = g_list_remove(pending_queue, tmpSound);
			free(tmpSound);
		}
	} // PLAY_AFTER_CURRENT

	s = g_strconcat(sound, " ", NULL);
  va_start( ap, sound);
  while( (tmp = va_arg (ap, char *))) {
    s = g_strconcat(s, " ", tmp, " ", NULL);
  }
  va_end(ap);

	tmpSound = (tsSound*) malloc(sizeof(tsSound));
	tmpSound->string = s;

	if (sound_channels_used < max_sound_channels) {
		playing_queue = g_list_append(playing_queue, tmpSound);
 		pid = exec_play(s);
		tmpSound->pid = pid;
		if (pid == -1) { // we got a problem playing sound : cancel it
				g_free(tmpSound->string);
				pending_queue = g_list_remove(pending_queue, tmpSound);
				free(tmpSound);
				}
		} else {
			printf("No more channels free to play %s\n", s);
			if (g_list_length(pending_queue) < MAX_QUEUE_LENGTH)
				pending_queue = g_list_append(pending_queue, tmpSound);
			printf("pending = %d\n",g_list_length(pending_queue));
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
