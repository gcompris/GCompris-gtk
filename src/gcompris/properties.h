/* gcompris - properties.h
 *
 * Time-stamp: <2006/08/13 16:55:16 bruno>
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

#include "profile.h"

typedef struct {
  gint		music;
  gint		fx;
  gint		fullscreen;
  gint		noxf86vm;
  gint		screensize;
  gint		defaultcursor;
  gint		timer;
  gint		filter_style;
  gint		difficulty_filter;
  gint		difficulty_max;
  gint		disable_quit;
  gint		disable_config;
  gint		display_resource;
  gchar        *root_menu;
  gchar        *local_directory;
  gchar        *user_data_dir;
  gchar        *package_data_dir;
  gchar        *package_locale_dir;
  gchar        *package_plugin_dir;
  gchar        *package_python_plugin_dir;
  gchar        *system_icon_dir;
  gchar	       *locale;
  gchar	       *skin;
  gchar	       *key;
  GcomprisProfile *profile;
  GcomprisUser    *logged_user;
  GcomprisBoard	  *menu_board;
  gchar	       *database;
  gint          administration;
  gint          reread_menu;
  gchar        *shared_dir;
  gchar        *users_dir;
  /* this are set by gcompris_im_init() */
  GtkIMContext *context;
  gchar        *default_context;
  gint         experimental;
  gchar        *menu_position;
  gchar        *server;
} GcomprisProperties;

GcomprisProperties	*gc_prop_get (void);
GcomprisProperties	*gcompris_properties_new (void);
void			 gcompris_properties_destroy (GcomprisProperties *props);
void			 gcompris_properties_save (GcomprisProperties *props);

gchar                   *get_default_database_name (gchar *shared_dir);
int                      create_rootdir (gchar *rootdir);
int			 my_setenv (const char * name, const char * value);

#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
