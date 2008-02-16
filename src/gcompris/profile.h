/* gcompris - profile.h
 *
 * Copyright (C) 2005 Bruno Coudoin
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

/*****************************************************************************/
/* this files describes structures and fucntions for profile management.     */
/* GcomprisClass is intended as school class, and defines a partition of     */
/* set of all users. Each user is in a class and only one.                   */
/* GcomprisGroup is a subset of class. It's not a partition of the class,    */
/* user can be in 1 or more groups.                                          */
/* There is at least one group in a class -- the whole class.                */
/* GcomprisUser is the structure defining a user.                            */
/* GcomprisProfile is the strucuture for the profile                         */
/*****************************************************************************/

#ifndef PROFILES_H
#define PROFILES_H

/*****************************************************************************/
/* The following structure dsecribes a class, partitionning the users set     */
struct _GcomprisClass {
  gint                class_id;

  /* name of class -- must be unique */
  gchar               *name;

  /* description */
  gchar               *description;

  /* Group id for whole class */
  gint                wholegroup_id;

  /* list of GComprisGroup id */
  GList               *group_ids;

  /* list of GComprisUser id */
  GList               *user_ids;
};

typedef struct _GcomprisClass  GcomprisClass;


/*****************************************************************************/
/* The following structure describes a group, subset of class  */

/* group data is saved when class is saved */

struct _GcomprisGroup {
  gint                group_id;

  /*name of group -- must be unique */
  gchar               *name;

  /* GcomprisClass containing the group */
  gint                class_id;

  /* list of GComprisUser user_id */
  GList               *user_ids;

  /* description */
  gchar               *description;
};

typedef struct _GcomprisGroup GcomprisGroup;

/* find a group */
GcomprisGroup         *gc_profile_group_load(gint *group_id);

/*****************************************************************************/
/* The following structure dsecribes a user */

/* users data is saved when class is saved */


struct _GcomprisUser {
  gint               user_id;

  /* The login name -- must be unique */
  gchar             *login;

  /* mandatory class */
  gint               class_id;

  /* Last Name */
  gchar             *lastname;

  /* First Name */
  gchar             *firstname;

  /* Birth day */
  gchar             *birthdate;

  /* Unique Session ID */
  guint             session_id;
};

typedef struct _GcomprisUser GcomprisUser;


/*****************************************************************************/
/* The following structure describes a profile object.  */

typedef struct {
  /* Profile Name */
  gint               profile_id;
  gchar             *name;

  /* The subdirectory into the user gcompris dir */
  gchar		    *directory;

  gchar             *description;

  /* list of GcomprisGroup. if empty user management is disabled */
  GList             *group_ids;

  /* list of activities to play -- gchar section/name */
  GList             *activities;

} GcomprisProfile;


/* Active profile */
GcomprisProfile     *gc_profile_get_current();
void gc_profile_destroy(GcomprisProfile *prof);
void gc_user_destroy(GcomprisUser *user);

/* List of Gcomprisusers */
void                 gc_profile_set_current_user(GcomprisUser *user);
GcomprisUser        *gc_profile_get_current_user();

#endif
