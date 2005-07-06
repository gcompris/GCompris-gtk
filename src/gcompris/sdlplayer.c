/* sdlplayer - sdlplayer.c
 *
 * Time-stamp: <2004/05/04 01:33:46 bcoudoin>
 *
 * Copyright (C) 2004 Bruno Coudoin
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
#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_mixer.h"
#include "SDL_audio.h"

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <glib.h>

Sint16 stream[2][4096];
int len=4096, bits=0, which=0;

// set this to any of 512,1024,2048,4096
// the higher it is, the more FPS shown and CPU needed
int audio_buffers=2048;

static gboolean sound_closed = FALSE;

/******************************************************************************/
/* some simple exit and error routines                                        */

void errorv(char *str, va_list ap)
{
  vfprintf(stderr,str,ap);

  if(strcmp(Mix_GetError(), "Unrecognized file type (not VOC)")==0) {
    fprintf(stderr,"*** You need a version of \"SDL_mixer\" with OGG Vorbis supported ***\n");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr,": %s.\n", Mix_GetError());
  fprintf(stderr,": %s.\n", SDL_GetError());

}

int cleanExit(char *str,...)
{
  va_list ap;
  va_start(ap, str);
  errorv(str,ap);
  va_end(ap);
  Mix_CloseAudio();
  SDL_Quit();
  return(1);
}

/******************************************************************************/

int sdlplayer_init()
{
  int audio_rate,audio_channels;
  Uint16 audio_format;

  // initialize SDL for audio
  if(SDL_Init(SDL_INIT_AUDIO)<0)
    return(cleanExit("SDL_Init"));
	
  // initialize sdl mixer, open up the audio device
  if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,audio_buffers)<0)
    return(cleanExit("Mix_OpenAudio"));

  // print out some info on the audio device and stream
  Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
  bits=audio_format&0xFF;
  printf("Opened audio at %d Hz %d bit %s, %d bytes audio buffer\n", audio_rate,
	 bits, audio_channels>1?"stereo":"mono", audio_buffers );

  return(0);
}

int sdlplayer_quit(Mix_Music *music)
{
  // free & close
  Mix_FreeMusic(music); 
  Mix_CloseAudio(); 
  printf("SDL PLAYER SDL_Quit\n"); 
  SDL_Quit(); 
}

int sdlplayer_bg(char *filename, int volume)
{
  Mix_Music *music;

  printf("sdlplayer_bg %s\n", filename);

  // load the song
  if(!(music=Mix_LoadMUS(filename)))
    return(cleanExit("Mix_LoadMUS(\"%s\")",filename));
	
  if(Mix_PlayMusic(music, 1)==-1) {
    return(cleanExit("Mix_LoadMUS(0x%p,1)",music));
  }

  Mix_VolumeMusic(volume);

  // wait for the music to complete
  while((Mix_PlayingMusic() || Mix_PausedMusic()))
    {
      SDL_Delay(50);
    }
	
  return(0);
}

int sdlplayer(char *filename, int volume)
{
  Mix_Chunk *sample;
  static int channel;

  g_warning("sdlplayer %s\n", filename);

  Mix_PauseMusic();

  sample=Mix_LoadWAV_RW(SDL_RWFromFile(filename, "rb"), 1);
  if(!sample) {
    return(cleanExit("Mix_LoadWAV_RW"));
    // handle error
  }

  Mix_VolumeChunk(sample, MIX_MAX_VOLUME);

  if(channel=Mix_PlayChannel(-1, sample, 0)==-1) {
    return(cleanExit("Mix_LoadChannel(0x%p,1)",channel));
  }

  while(Mix_Playing(channel))
    {
      SDL_Delay(50);
    }
	
  // fade in music. Removed, eats too much CPU on low end PCs
  /*   for(i=32; i<=128; i+=10) { */
  /*     Mix_VolumeMusic(i); */
  /*     SDL_Delay(20); */
  /*   } */
  
  // resume music playback
  if (!sound_closed)
    Mix_ResumeMusic();

  // free the sample
  // Mix_Chunk *sample;
  Mix_FreeChunk(sample);

  g_warning("sdlplayer complete playing of %s\n", filename);

  return(0);
}
void sdlplayer_close()
{
  Mix_HaltMusic();
  SDL_PauseAudio(1);
  Mix_CloseAudio();
  sound_closed = TRUE;
}
void sdlplayer_reopen()
{
  Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,audio_buffers);
  SDL_PauseAudio(0);
  sound_closed = FALSE;
}

