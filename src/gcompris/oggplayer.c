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

#include "gcompris.h"

#include <stdio.h>
#include <libgen.h>
#include <pthread.h>
#include <vorbis/vorbisfile.h>
#include <ao/ao.h>

#ifdef __WIN32
#include <io.h>
#include <fcntl.h>
#endif

static GnomeCanvasItem	*rootitem		= NULL;

static gint	 item_event_ogginfo(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

int quit = 0;

//void *decode_ogg_file(void *infile)
int decode_ogg_file(char *infile)
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
  
  GcomprisProperties	*properties = gcompris_get_properties();

  input = fopen((char *)infile, "rb");
  if(!input) {
      fprintf(stderr, "cannot open %s\n",(char *) infile);
      return -1;
    }

  if((ov_status = ov_open(input, &vf, NULL, 0)) < 0) 
    {
      fclose(input);
      fprintf(stderr, "ov_open failed for %s (%d)\n",(char *) infile, ov_status);
      return -1;
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

  /* Get the audio output from the gcompris properties */
  if(!strcmp(properties->audio_output, ""))
    driver_id = ao_default_driver_id();
  else
    driver_id = ao_driver_id(properties->audio_output);

  if ( driver_id < 0 ){
     fprintf(stderr, "Error unable to find a usable audio output device (%d)\nTry gcompris -A to select an alternative audio output", driver_id);
     /* Disable sounds in gcompris */
     properties->music = FALSE;
     properties->fx = FALSE;
     properties->audio_works = FALSE;
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
      return -1;
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

/*
 * This does the erase of the credits
 *
 */
static gint erase_credits (GtkWidget *widget, gpointer data)
{
  if(rootitem) {
    gtk_object_destroy (GTK_OBJECT(rootitem));
    rootitem = NULL;
  }
  return(FALSE);
}

void *display_ogg_file_credits(void *infile)
{
  OggVorbis_File vf;
  vorbis_comment *vc;
  long ov_status;
  guint i = 0;
  guint y = BOARDHEIGHT - 120;
  FILE* input;
  GdkPixbuf   *pixmap = NULL;
  GnomeCanvasItem *item;
  
  if(rootitem)
    return;

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

  rootitem = \
    gnome_canvas_item_new (gnome_canvas_root(gcompris_get_canvas()),
			   gnome_canvas_group_get_type (),
			   "x", (double)0,
			   "y", (double)0,
			   NULL);

  pixmap = gcompris_load_skin_pixmap("display_area_small.png");
  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap, 
				"x", (double) (BOARDWIDTH - gdk_pixbuf_get_width(pixmap))/2,
				"y", (double) y - 60,
				NULL);
  gtk_signal_connect(GTK_OBJECT(rootitem), "event",
		     (GtkSignalFunc) item_event_ogginfo,
		     NULL);

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", _("Now Playing Music"),
			 "font", gcompris_skin_font_subtitle,
			 "x", (double) BOARDWIDTH/2 +1,
			 "y", (double) y - 10 + i*20 +1,
			 "anchor", GTK_ANCHOR_NORTH,
			 "fill_color", "black",
			 "justification", GTK_JUSTIFY_CENTER,
			 NULL);

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			 gnome_canvas_text_get_type (),
			 "text", _("Now Playing Music"),
			 "font", gcompris_skin_font_subtitle,
			 "x", (double) BOARDWIDTH/2,
			 "y", (double) y - 10 + i++*20,
			 "anchor", GTK_ANCHOR_NORTH,
			 "fill_color", "white",
			 "justification", GTK_JUSTIFY_CENTER,
			 NULL);

  {
    char **ptr=vc->user_comments;
    guint line = 0;

    /* Take only the first lines */
    while(*ptr && line < 2){
      gchar **stra;
      gchar *str;

      fprintf(stderr,"%s\n",*ptr);
      line++;


      stra = g_strsplit (*ptr, "=", 2);

      if(!strcmp(stra[0], "TITLE")) {
	g_free(stra[0]);
	stra[0] = _("Title");
      }  else {
	if (!strcmp(stra[0], "ARTIST")) {
	  g_free(stra[0]);
	  stra[0] = _("Artist");
	}
      }

      str = g_strdup_printf("%s: %s", stra[0], stra[1]);
		
		
      gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			     gnome_canvas_text_get_type (),
			     "text", str,
			     "font", gcompris_skin_font_board_tiny,
			     "x", (double) BOARDWIDTH/2 +1,
			     "y", (double) y + i*20 +1,
			     "anchor", GTK_ANCHOR_NORTH,
			     "fill_color", "black",
			     "justification", GTK_JUSTIFY_CENTER,
			     NULL);

      gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
			     gnome_canvas_text_get_type (),
			     "text", str,
			     "font", gcompris_skin_font_board_tiny,
			     "x", (double) BOARDWIDTH/2,
			     "y", (double) y + i++*20,
			     "anchor", GTK_ANCHOR_NORTH,
			     "fill_color", "white",
			     "justification", GTK_JUSTIFY_CENTER,
			     NULL);

      ++ptr;
    }

    gtk_timeout_add (20000,
		     (GtkFunction) erase_credits, NULL);

  }

  ov_clear(&vf);
}

/* Callback for the ogginfo operations */
static gint
item_event_ogginfo(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:
      break;
    case GDK_BUTTON_PRESS:
      if(rootitem) {
	gtk_object_destroy (GTK_OBJECT(rootitem));
	rootitem = NULL;
      }
    default:
      break;
    }
  return FALSE;

}
