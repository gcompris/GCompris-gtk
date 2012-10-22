/* gcompris - gameutil.h
 *
 * Copyright (C) 2000 Bruno Coudoin
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

#ifndef SOUNDUTIL_SDL_H
#define SOUNDUTIL_SDL_H

#include <stdio.h>
#include "gcompris.h"

#define MAX_QUEUE_LENGTH 5
#define MAX_SOUND_FILES 20

int	 decode_ogg_file(char *infile);
void	 display_ao_devices();

/*
 * SDL PLAYER
 */
int	 sdlplayer_init();
void	 sdlplayer_close();
void	 sdlplayer_reopen();

int	 sdlplayer_music(char *filename, int volume);
void	 sdlplayer_halt_music();
void	 sdlplayer_pause_music();
void	 sdlplayer_resume_music();

void	 sdlplayer_halt_fx();
void	 sdlplayer_resume_fx();
int	 sdlplayer_fx(char *filename, int volume);
void	 sdl_player_fx_cancel();

gchar *gc_sound_alphabet(gchar *chars);


/*************************************************************/
/* GObject to  control sound. code get from maman-file.h     */
/*************************************************************/
/*
 * Potentially, include other headers on which this header depends.
 */
#include <glib-object.h>

#define GCOMPRIS_SOUND_TYPE 	  (gc_sound_get_type ())
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
        void (*sound_played) (GcomprisSound *self, gchar *file, gpointer user_data);
};

/* used by MAMAN_TYPE_FILE */
GType gc_sound_get_type (void);

/* API. */

/* empty for now */


/* Declaration of GCompris Sound Controller */
extern GObject *gc_sound_controller;

#endif

#endif //
