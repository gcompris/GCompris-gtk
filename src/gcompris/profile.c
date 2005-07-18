/* gcompris - profile.c
 *
 * Time-stamp: <2005/07/11 13:18:11 yves>
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

/* Active profile */
static GcomprisProfile        *gcompris_profile = NULL;

/* logged user */
static GcomprisUser           *gcompris_logged_user = NULL;


GcomprisProfile       *gcompris_get_current_profile()
{

  if (gcompris_profile)
    return gcompris_profile;

  gcompris_profile = gcompris_db_get_profile();

  return gcompris_profile;

}


void                    *gcompris_set_current_user(GcomprisUser *user)
{
  gcompris_logged_user = user;
}


GcomprisUser            *gcompris_get_current_user()
{
  return gcompris_logged_user;
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */

