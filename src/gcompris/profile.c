/* gcompris - profile.c
 *
 * Time-stamp: <2006/08/21 23:26:49 bruno>
 *
 * Copyright (C) 2005 Bruno Coudoin
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

#include <stdio.h>
#include "gcompris.h"
#include "properties.h"

/** Return the current profile or NULL if sqlite not activated
 *
 */
GcomprisProfile*
gc_profile_get_current()
{
  GcomprisProperties	*properties = gc_prop_get();

  if (properties->profile)
    return properties->profile;

  properties->profile = gc_db_get_profile();

  return properties->profile;

}


void
gc_profile_set_current_user(GcomprisUser *user)
{
  GcomprisProperties	*properties = gc_prop_get();

  if (user)
    properties->logged_user = user;
  else {
    g_warning("No user, getting one from system.");
    GcomprisUser *sys_user = g_malloc0(sizeof(GcomprisUser));

    const gchar *user_name = g_get_user_name ();
    if (user_name)
      sys_user->login = g_strdup(user_name);
    else
      sys_user->login = g_strdup("nobody");

    const gchar *last_name = g_get_real_name ();
    if (last_name)
      sys_user->lastname = g_strdup(last_name);
    else
      sys_user->lastname = g_strdup("Nobody There ?");

    sys_user->firstname = g_strdup("Unknown");
    sys_user->birthdate = g_strdup("");
    sys_user->user_id = -1;

    properties->logged_user = sys_user;
  }

  GTimeVal now;
  g_get_current_time (&now);

  gchar *session_id_str = g_strdup_printf("%s%ld%ld", (properties->logged_user)->login, now.tv_sec, now.tv_usec);

  (properties->logged_user)->session_id = g_str_hash((gconstpointer) session_id_str);
  g_free(session_id_str);

}

void gc_profile_destroy(GcomprisProfile*prof)
{
  if(!prof)
    return;

  g_free(prof->name);
  g_free(prof->directory);
  g_free(prof->description);
  g_free(prof);
}

void gc_user_destroy(GcomprisUser*user)
{
  if(!user)
    return;

  g_free(user->login);
  g_free(user->lastname);
  g_free(user->firstname);
  g_free(user->birthdate);
  g_free(user);
}

GcomprisUser*
gc_profile_get_current_user()
{
  GcomprisProperties	*properties = gc_prop_get();
  return properties->logged_user;
}

