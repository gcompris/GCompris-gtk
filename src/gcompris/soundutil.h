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

void	 gcompris_play_ogg(char *, ...);
void	 gcompris_play_ogg_list( GList* files );
void	 setSoundPolicy(int);
int	 getSoundPolicy(void);
void	 initSound(void);
void	 gcompris_reopen_sound(void);
void	 gcompris_close_sound(void);

int	 decode_ogg_file(char *infile);
void	 display_ao_devices();

/*
 * SDL PLAYER
 */
int	 sdlplayer_init();
int	 sdlplayer_bg();
void	 sdlplayer_close();
void	 sdlplayer_reopen();
int	 sdlplayer(char *filename, int volume);

gchar *gcompris_alphabet_sound(gchar *chars);

#endif
