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

int quit = 0;

//int decode_ogg_file(char *infile)
void *decode_ogg_file(void *infile)
{
  OggVorbis_File vf;
  ao_device *audio_device;
  ao_sample_format format;
  vorbis_comment *vc;
  vorbis_info *vi;
  FILE *input;    
  int bs = 0;
  char buf[4096];
  int buflen = 4096;
  int ret;  
  int driver_id;  
  int big_endian;
  int word_size;
  int signed_sample;
  int rate;
  int channels;
  
  input = fopen((char *)infile, "rb");
  if(!input) 
    {
      return 0;
    }

  if(ov_open(input, &vf, NULL, 0) < 0) 
    {
      fclose(input);
      return 0;
    }

  driver_id = ao_default_driver_id();      
  big_endian = ao_is_big_endian();
  signed_sample = 1;
  word_size = 2;  
  vc = ov_comment(&vf, -1);
  vi = ov_info(&vf, -1);	
  rate = vi->rate;
  channels = vi->channels;
  format.rate = rate;
  format.channels = channels;
  format.bits = word_size * 8;
  format.byte_format = big_endian ? AO_FMT_BIG : AO_FMT_LITTLE;	
  audio_device = ao_open_live(driver_id, &format, NULL);    
  if (audio_device == NULL)
    {
      fclose(input);
      return 0;
    }
      
  printf("%s : ", basename(infile));    
  printf("  %ldHz , ", rate);    
  printf(" %d channels  \n", channels);    
  //printf("number of logical streams : %ld \n", ov_streams(&vf));
  printf("duration = %f seconds\n", ov_time_total(&vf, -1));

  while (((ret = ov_read(&vf, buf, buflen, big_endian, word_size, signed_sample, &bs)) != 0) && !quit)
    {
      if(bs != 0) 
	{
	  printf("Only one logical bitstream currently supported\n");
	  break;
	}

      // write audio to device
      if (ret > 0)
	if (ao_play(audio_device, buf, ret) == 0)
	  printf("error writing audio data \n");	
    }

  ov_clear(&vf);
  ao_close(audio_device);
  return 0;
}

