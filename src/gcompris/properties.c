/* gcompris - properties.c
 *
 * Time-stamp: <2001/12/27 01:14:26 bruno>
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

#include "gcompris.h"

GcomprisProperties *properties;

GcomprisProperties *gcompris_properties_new (void);

GcomprisProperties *gcompris_get_properties ()
{
  return (properties);
}

GcomprisProperties *gcompris_properties_new ()
{
  GcomprisProperties *tmp;

  tmp = (GcomprisProperties *) malloc (sizeof (GcomprisProperties));

  tmp->music		= gnome_config_get_int ("/gcompris/Preferences/music=1");
  tmp->fx		= gnome_config_get_int ("/gcompris/Preferences/fx=1");
  tmp->fullscreen	= gnome_config_get_int ("/gcompris/Preferences/fullscreen=1");
  tmp->locale		= gnome_config_get_string ("/gcompris/Preferences/locale=C");

  return (tmp);
}

void gcompris_properties_destroy (GcomprisProperties *props)
{
  if(props->locale!=NULL)
    g_free(props->locale);

  free (props);
}

GcomprisProperties *gcompris_properties_copy (GcomprisProperties *props)
{
  GcomprisProperties *tmp;

  tmp = (GcomprisProperties *) malloc (sizeof (GcomprisProperties));

  tmp->music = props->music;
  tmp->fx = props->fx;
  tmp->fullscreen = props->fullscreen;
  tmp->locale = g_strdup(props->locale);
	
  return (tmp);
}

void gcompris_properties_save (GcomprisProperties *props)
{
  gnome_config_set_int ("/gcompris/Preferences/music",
			props->music);
  gnome_config_set_int ("/gcompris/Preferences/fx",
			props->fx);
  gnome_config_set_int ("/gcompris/Preferences/fullscreen",
			props->fullscreen);
  gnome_config_set_string ("/gcompris/Preferences/locale",
			props->locale);

  gnome_config_sync ();
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
