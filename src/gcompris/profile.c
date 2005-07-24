/* gcompris - profile.c
 *
 * Time-stamp: <2005/07/25 00:10:07 bruno>
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

/* logged user */
static GcomprisUser           *gcompris_logged_user = NULL;


GcomprisProfile       *gcompris_get_current_profile()
{
  GcomprisProperties	*properties = gcompris_get_properties();

  if (properties->profile)
    return properties->profile;

  properties->profile = gcompris_db_get_profile();

  return properties->profile;

}


void                    *gcompris_set_current_user(GcomprisUser *user)
{
  GcomprisProperties	*properties = gcompris_get_properties();
  properties->logged_user = user;
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

