/* gcompris - profile.c
 *
 * Time-stamp: <2005/06/14 22:17:23 yves>
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

static GcomprisProfile *gcompris_profile;


/* This is main fonction used to get list of users autorised on gcompris */
GcomprisGroup         *gcompris_group_load(gchar *classname, char * groupname)
{

}

void                   gcompris_class_destroy()
{}

/* save class, groups and users in file */
void                   gcompris_class_save(GcomprisClass *class)
{}

GcomprisClass         *gcompris_class_load(gchar *name)
{}



GcomprisGroup         *gcompris_group_new()
{}

void                   gcompris_group_destroy()
{}

/* find a group into a class */
GcomprisGroup         *gcompris_group_find_by_name(GcomprisClass *class, gchar *name)
{}


GcomprisUser          *gcompris_user_new()
{}

void                   gcompris_user_destroy()
{}

/* find a user into a group */
GcomprisUser          *gcompris_user_find_by_name(GcomprisGroup *group, gchar *name)
{}

GcomprisProfile       *gcompris_get_profile()
{}

GcomprisProfile       *gcompris_profile_load(gchar *name)
{}

void                   gcompris_profile_destroy()
{}

void                   gcompris_profile_save(GcomprisProfile *profile)
{}


/* Usefull for management */
GList                 *list_of_classname()
{}

GList                 *list_of_username()
{}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */

