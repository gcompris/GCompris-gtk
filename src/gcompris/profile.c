/* gcompris - profile.c
 *
 * Time-stamp: <2005/09/18 22:51:12 bruno>
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

GcomprisProfile
*gcompris_get_current_profile()
{
  GcomprisProperties	*properties = gcompris_get_properties();

  if (properties->profile)
    return properties->profile;

  properties->profile = gcompris_db_get_profile();

  return properties->profile;

}


void
*gcompris_set_current_user(GcomprisUser *user)
{
  GcomprisProperties	*properties = gcompris_get_properties();

  if (user)
    properties->logged_user = user;
  else {
    g_warning("No user, getting one from system.");
    GcomprisUser *sys_user = g_malloc0(sizeof(GcomprisUser));

    gchar *user_name = g_get_user_name ();
    if (user_name)
      sys_user->login = g_strdup(user_name);
    else
      sys_user->login = g_strdup("nobody");

    gchar *last_name = g_get_real_name ();
    if (last_name)
      sys_user->lastname = g_strdup(last_name);
    else
      sys_user->lastname = g_strdup("Nobody There ?");

    sys_user->firstname = g_strdup("Unknown");
    sys_user->birthdate = g_strdup("");

    properties->logged_user = sys_user;
  }

  GTimeVal now;
  g_get_current_time (&now);

  gchar *session_id_str = g_strdup_printf("%s%ld%ld", (properties->logged_user)->login, now.tv_sec, now.tv_usec);

  (properties->logged_user)->session_id = g_str_hash((gconstpointer) session_id_str);
   
}


GcomprisUser            *gcompris_get_current_user()
{
  GcomprisProperties	*properties = gcompris_get_properties();
  return properties->logged_user;
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */

