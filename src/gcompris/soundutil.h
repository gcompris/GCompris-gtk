/* gcompris - gameutil.h
 *
 * Time-stamp: <2000/07/16 00:32:26 bruno>
 *
 * Copyright (C) 2000 Bruno Coudoin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef SOUNDUTIL_H
#define SOUNDUTIL_H

#include <stdio.h>
#include "gcompris.h"

#define MAX_QUEUE_LENGTH 5
#define MAX_SOUND_FILES 20

typedef enum
{
  PLAY_ONLY_IF_IDLE		= 0,
  PLAY_AFTER_CURRENT		= 1,
} SoundPolicy;

typedef void (*GcomprisSoundCallback) (gchar *);

void	 gcompris_play_ogg(const gchar *, ...);
void	 gcompris_play_ogg_cb(const gchar *, GcomprisSoundCallback);
void	 gcompris_play_ogg_list( GList* files );
void	 setSoundPolicy(int);
int	 getSoundPolicy(void);
void	 initSound(void);
void	 gcompris_reopen_sound(void);
void	 gcompris_close_sound(void);
void	 gcompris_pause_sound(void);
void	 gcompris_resume_sound(void);

int	 decode_ogg_file(char *infile);
void	 display_ao_devices();

/*
 * SDL PLAYER
 */
int	 sdlplayer_init();
int	 sdlplayer_bg();
void	 sdlplayer_close();
void	 sdlplayer_reopen();
void	 sdlplayer_pause();
void	 sdlplayer_resume();
int	 sdlplayer(char *filename, int volume);

gchar *gcompris_alphabet_sound(gchar *chars);


/*************************************************************/
/* GObject to  control sound. code get from maman-file.h     */
/*************************************************************/
/*
 * Potentially, include other headers on which this header depends.
 */
#include <glib-object.h>

#define GCOMPRIS_SOUND_TYPE 	  (gcompris_sound_get_type ())
#define GCOMPRIS_SOUND(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCOMPRIS_SOUND_TYPE, GcomprisSound))
#define GCOMPRIS_SOUND_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GCOMPRIS_SOUND_TYPE, GcomprisSoundClass))
#define GCOMPRIS_SOUND_IS(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),  GCOMPRIS_SOUND_TYPE))
#define GCOMPRIS_SOUND_IS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GCOMPRIS_SOUND_TYPE))
#define GCOMPRIS_SOUND_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GCOMPRIS_SOUND_TYPE, GcomprisSoundClass))

typedef struct _GcomprisSound GcomprisSound;
typedef struct _GcomprisSoundClass GcomprisSoundClass;
typedef struct _GcomprisSoundPrivate GcomprisSoundPrivate;

struct _GcomprisSound {
	GObject parent;
	/* instance members */
        GcomprisSoundPrivate *private;
};

struct _GcomprisSoundClass {
	GObjectClass parent;
        
        guint sound_played_signal_id;
        void (*sound_played) (GcomprisSound *self, gchar *file);
};

/* used by MAMAN_TYPE_FILE */
GType gcompris_sound_get_type (void);

/* API. */

/* empty for now */


/* Declaration of GCompris Sound Controller */
extern GObject *gcompris_sound_controller;

#endif
