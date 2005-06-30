/* gcompris - properties.h
 *
 * Time-stamp: <2005/06/29 10:55:12 yves>
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

#ifndef _PROPERTIES_H_
#define _PROPERTIES_H_

typedef struct {
  gint		music;
  gint		fx;
  gint		audio_works;
  gint		fullscreen;
  gint		noxrandr;
  gint		screensize;
  gint		defaultcursor;
  gint		timer;
  gint		filter_style;
  gint		difficulty_filter;
  gint		difficulty_max;
  gchar        *root_menu;
  gchar        *local_directory;
  gchar        *package_data_dir;
  gchar	       *locale;
  gchar	       *skin;
  gchar	       *key;
  gchar        *profil;
  gchar        *database;
  gint          administration;
  gint          reread_xml;
} GcomprisProperties;

GcomprisProperties	*gcompris_get_properties (void);
GcomprisProperties	*gcompris_properties_new (void);
void			 gcompris_properties_destroy (GcomprisProperties *props);
void			 gcompris_properties_save (GcomprisProperties *props);

#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
