/* gcompris - oggplayer.c
 *
 * Copyright (C) 2002 Nabil Ben Khalifa
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

#include <stdio.h>
#include <libgen.h>
#include <pthread.h>
#include <vorbis/vorbisfile.h>
#include <ao/ao.h>

#ifdef __WIN32
#include <io.h>
#include <fcntl.h>
#endif

int quit = 0;

//int decode_ogg_file(char *infile)
void *decode_ogg_file(void *infile)
{
  char buf[4096];
  OggVorbis_File vf;
  vorbis_comment *vc;
  vorbis_info *vi;
  long ov_status;

  ao_device *audio_device;
  ao_sample_format format;

  int driver_id;  
  int big_endian;
  int word_size;
  int signed_sample;
  long rate;
  int channels;

  FILE* input;
  int eof=0;
  int bs=0;
  
  input = fopen((char *)infile, "rb");
  if(!input) {
      fprintf(stderr, "cannot open %s\n",(char *) infile);
      return 0;
    }

  if((ov_status = ov_open(input, &vf, NULL, 0)) < 0) 
    {
      fclose(input);
      fprintf(stderr, "ov_open failed for %s (%d)\n",(char *) infile, ov_status);
      return 0;
    }
  vc = ov_comment(&vf, -1);
  vi=ov_info(&vf,-1);

  {
    char **ptr=vc->user_comments;
    while(*ptr){
      fprintf(stderr,"%s\n",*ptr);
      ++ptr;
    }
    printf("%s : ", basename(infile));    
    printf("\nBitstream is %d channel(s), %ldHz\n",vi->channels,vi->rate);
    printf("Encoded by: %s\n",vc->vendor);
    printf("number of logical streams : %ld \n", ov_streams(&vf));
    printf("duration = %f seconds\n\n", ov_time_total(&vf, -1));
  }

  #ifdef __APPLE__
  driver_id = ao_driver_id("esd");
  printf("\n!!! Audio output device set to esd by default on Mac Os 10.2 !!!!\n\n");
  #else
  driver_id = ao_default_driver_id();      
  #endif
  
  if ( driver_id < 0 ){
     fprintf(stderr, "Error unable to find a usable audio output device (%d)\n", driver_id);
     return 0;
  }
  
  big_endian = ao_is_big_endian();
  signed_sample = 1;
  word_size = 2;  
  
  format.rate = (int ) vi->rate;
  format.channels = vi->channels;
  format.bits = word_size * 8;
  format.byte_format = big_endian ? AO_FMT_BIG : AO_FMT_LITTLE;	
  audio_device = ao_open_live(driver_id, &format, NULL);    
  if (audio_device == NULL)
    {
      fclose(input);
      fprintf(stderr, "Error opening audio device\n");
      return 0;
    }

  
  while(!eof){
    long ret=ov_read(&vf,buf,sizeof(buf),big_endian, word_size, signed_sample, &bs);
    if(bs != 0){
	fprintf(stderr, "Only one logical bitstream currently supported\n");
	break;
    }
    if (ret == 0) {
      /* EOF */
      eof=1;
    } else if (ret < 0) {
      /* error in the stream.  Not a problem, just reporting it in
	 case we (the app) cares.  In this case, we don't. */
    } else {
      /* we don't bother dealing with sample rate changes, etc, but
	 you'll have to*/
      if (ao_play(audio_device, buf, (unsigned int) ret) == 0)
	  fprintf(stderr, "error writing audio data \n");	
    }
  }

  ov_clear(&vf);
  ao_close(audio_device);
  return 0;
}

