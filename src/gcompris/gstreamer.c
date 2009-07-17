/* gcompris - gstreamer.c
 *
 * Copyright (C) 2009 Bruno Coudoin
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

#ifdef USE_GSTREAMER

#include "string.h"

#include "gcompris.h"
#include <signal.h>
#include <glib.h>
#include <gst/gst.h>

static GstElement *bg_pipeline = NULL;
static GstElement *fx_pipeline = NULL;

static gboolean	 fx_paused = FALSE;
static gboolean	 bg_paused = FALSE;

/* Singleton */
static guint	 sound_init = 0;

/* =====================================================================
 *
 * =====================================================================*/
void
gc_sound_init()
{

  /* Check to run the init only once */
  if(sound_init == 1)
    return;

  sound_init = 1;

  /* gstreamer init */
  gst_init(NULL, NULL);

  gc_sound_policy_set(PLAY_AFTER_CURRENT);

}

static gboolean
fx_bus(GstBus* bus, GstMessage* msg, gpointer data)
{
  switch( GST_MESSAGE_TYPE( msg ) )
    {
    case GST_MESSAGE_EOS:
      g_warning("fx_bus: EOS START");
      gc_sound_fx_close();
      gc_sound_callback((gchar *)data);
      fx_play();
      g_warning("fx_bus: EOS END");
      break;
    default:
      return TRUE;
    }

  return FALSE;
}

static gboolean
bg_bus(GstBus* bus, GstMessage* msg, gpointer data)
{
  switch( GST_MESSAGE_TYPE( msg ) ) {
    case GST_MESSAGE_EOS:
        g_warning("bg_bus: EOS");
	gc_sound_bg_close();
	bg_play(NULL);
	break;
    default:
      return TRUE;
  }
  return FALSE;
}

void
gc_sound_bg_close()
{
  if (bg_pipeline)
    {
      gst_element_set_state(bg_pipeline, GST_STATE_NULL);
      gst_object_unref(GST_OBJECT(bg_pipeline));
      bg_pipeline = NULL;
    }
}

void
gc_sound_fx_close()
{
  g_warning("gc_sound_fx_close");
  if (fx_pipeline)
    {
      gst_element_set_state(fx_pipeline, GST_STATE_NULL);
      gst_object_unref(GST_OBJECT(fx_pipeline));
      fx_pipeline = NULL;
    }
  g_warning("gc_sound_fx_close done");
}

void
gc_sound_bg_reopen()
{
  if(gc_prop_get()->music)
    bg_play(NULL);
}

void
gc_sound_fx_reopen()
{
}

void
gc_sound_bg_pause()
{
  if (bg_pipeline)
  {
    gst_element_set_state(bg_pipeline, GST_STATE_PAUSED);
  }
  bg_paused = TRUE;
}

void
gc_sound_bg_resume()
{
  if(bg_pipeline)
    gst_element_set_state(bg_pipeline, GST_STATE_PLAYING);
  else
    {
      bg_paused = FALSE;
      gc_sound_bg_reopen();
    }

  bg_paused = FALSE;
}

void
gc_sound_fx_pause()
{
  if (fx_pipeline)
  {
    gst_element_set_state(fx_pipeline, GST_STATE_PAUSED);
  }
  fx_paused = TRUE;
}

void
gc_sound_fx_resume()
{
  if(fx_pipeline)
  {
    gst_element_set_state(fx_pipeline, GST_STATE_PLAYING);
  }
  fx_paused = FALSE;
}

/* background play
 *
 */
gpointer
bg_play(gpointer dummy)
{
  gchar *absolute_file = gc_sound_get_next_music();

  if (!absolute_file)
    return NULL;

  bg_pipeline = gst_element_factory_make ("playbin", "play");

  if(!bg_pipeline)
    {
      g_warning("Failed to build the gstreamer pipeline (for background music)");
      gc_prop_get()->music = 0;
      return NULL;
    }

  gst_bus_add_watch (gst_pipeline_get_bus (GST_PIPELINE (bg_pipeline)),
		     bg_bus, bg_pipeline);


  gchar *uri = g_strconcat("file://", absolute_file, NULL);
  g_free(absolute_file);
  g_warning("  bg_play %s", uri);

  g_object_set (G_OBJECT (bg_pipeline), "uri", uri, NULL);

  gst_element_set_state (bg_pipeline, GST_STATE_PLAYING);

  g_free(uri);

  return(NULL);
}

/* playing a single file
 *
 */
void
fx_play()
{
  gchar *file;
  gchar *absolute_file;
  GcomprisProperties *properties = gc_prop_get();

  if(fx_pipeline)
    return;

  file = get_next_sound_to_play();

  if(!file)
    return;

  g_warning("  fx_play %s", file);

  absolute_file = gc_file_find_absolute(file);

  if (!absolute_file ||
      !properties->fx)
    return;

  fx_pipeline = gst_element_factory_make ("playbin", "play");

  if (!fx_pipeline)
    {
      g_warning("Failed to build the gstreamer pipeline");
      gc_prop_get()->fx = 0;
      return;
    }

  gchar *uri = g_strconcat("file://", absolute_file, NULL);
  g_free(absolute_file);
  g_warning("   uri '%s'", uri);

  g_object_set (G_OBJECT (fx_pipeline), "uri", uri, NULL);
  gst_bus_add_watch (gst_pipeline_get_bus (GST_PIPELINE (fx_pipeline)),
		     fx_bus, file);

  gst_element_set_state (fx_pipeline, GST_STATE_PLAYING);

  g_free(uri);

  return;
}

#endif // USE_GSTREAMER
