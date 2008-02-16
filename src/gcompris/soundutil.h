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
  PLAY_AND_INTERRUPT		= 2,
} SoundPolicy;

typedef void (*GcomprisSoundCallback) (gchar *);

void	 gc_sound_init(void);

void	 gc_sound_play_ogg(const gchar *, ...);
void	 gc_sound_play_ogg_cb(const gchar *, GcomprisSoundCallback);
void	 gc_sound_play_ogg_list( GList* files );

void	 gc_sound_policy_set(int);
int	 gc_sound_policy_get(void);

void	 gc_sound_reopen(void);
void	 gc_sound_bg_reopen(void);
void	 gc_sound_fx_reopen(void);

void	 gc_sound_close(void);
void	 gc_sound_bg_close(void);
void	 gc_sound_fx_close(void);

void	 gc_sound_bg_pause(void);
void	 gc_sound_fx_pause(void);

void	 gc_sound_bg_resume(void);
void	 gc_sound_fx_resume(void);

gchar   *gc_sound_alphabet(gchar *chars);


#endif
