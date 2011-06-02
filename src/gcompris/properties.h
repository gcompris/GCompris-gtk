/* gcompris - properties.h
 *
 * Copyright (C) 2000, 2008 Bruno Coudoin
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

#ifndef _PROPERTIES_H_
#define _PROPERTIES_H_

#include "profile.h"

/** Source config file for initialization */
typedef enum {
	GC_PROP_FROM_SYSTEM_CONF,
	GC_PROP_FROM_USER_CONF
} GCPropSourceConf;

typedef struct {
  gint		music;
  gint		fx;
  gint		fullscreen;
  gint		defaultcursor;
  gint		nocursor;
  gint		timer;
  gint		filter_style;
  gint		difficulty_filter;
  gint		difficulty_max;
  gint		disable_quit;
  gint		disable_config;
  gint		disable_level;
  gint		display_resource;
  gchar        *root_menu;
  gchar        *package_data_dir;
  gchar        *package_skin_dir;
  gchar        *package_locale_dir;
  gchar        *package_plugin_dir;
  gchar        *package_python_plugin_dir;
  gchar        *menu_dir;
  gchar        *system_icon_dir;
  gchar        *cache_dir;
  gchar	       *locale;
  gchar	       *skin;
  gchar	       *key;
  GcomprisProfile *profile;
  GcomprisUser    *logged_user;
  GcomprisBoard	  *menu_board;
  gchar	       *database;
  gint          administration;
  gint          reread_menu;
  gchar        *config_dir;
  gchar        *user_dir;
  /* this are set by gc_im_init() */
  GtkIMContext *context;
  gchar        *default_context;
  gint          experimental;
  gchar        *server;
  gint		drag_mode;
  gint		zoom;
  gboolean	bar_hidden;  /* Is the bar hiden */

} GcomprisProperties;

GcomprisProperties	*gc_prop_get (void);
GcomprisProperties	*gc_prop_new (void);
void			 gc_prop_destroy (GcomprisProperties *props);
void			 gc_prop_load (GcomprisProperties *props, GCPropSourceConf);
void			 gc_prop_save (GcomprisProperties *props);
void			 gc_prop_activate (GcomprisProperties *props);

gchar                   *gc_prop_default_database_name_get (gchar *config_dir);
int			 gc_setenv (const char * name, const char * value);

void			 gc_prop_old_config_migration(GcomprisProperties *props);
#endif
