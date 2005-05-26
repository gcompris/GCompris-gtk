/* gcompris - properties.h
 *
 * Time-stamp: <2005/05/26 23:07:30 yves>
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

/*****************************************************************************/
/* this files describes structures and fucntions for profile management.     */
/* GcomprisClass is intended as school class, and defines a partition of     */
/* set uf all users. Each user is in a class and only one.                   */
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
  /* name of class -- must be unique */
  gchar               *name;

  /* description */
  gchar               *description;

  /* Group for whole class */
  struct _GcomprisGroup *all;

  /* list of GComprisGroup */
  GList               *groups;
};

typedef struct _GcomprisClass  GcomprisClass;

//GcomprisClass         *gcompris_class_new();
void                   gcompris_class_destroy();

/* save class, groups and users in file */
void                   gcompris_class_save(GcomprisClass *class);
GcomprisClass         *gcompris_class_load(gchar *name);

/*****************************************************************************/
/* The following structure dsecribes a group, subset of class  */

/* group data is saved when class is saved */

struct _GcomprisGroup {
  /*name of group -- must be unique */
  gchar               *name;
  
  /* GcomprisClass containing the group */
  gchar               *classname;

  /* list of GComprisUser */
  GList               *users;

  /* description */
  gchar               *description;
};

typedef struct _GcomprisGroup GcomprisGroup;

GcomprisGroup         *gcompris_group_new();
void                   gcompris_group_destroy();

/* find a group into a class */
GcomprisGroup         *gcompris_group_load(gchar *classname, gchar *name);

/*****************************************************************************/
/* The following structure dsecribes a user */

/* users data is saved when class is saved */


struct _GcomprisUser {
  /* The login name -- must be unique */
  gchar               *login;

  /* mandatory class */
  gchar               *classname;

  /* Real Name */
  gchar               *surname;
  
  /* First Name */
  gchar               *firstname;

  /* Birth day */
  GDate               *birth;

  /* desactived activities for this user */
  GList               *desactived_activities;
};

typedef struct _GcomprisUser GcomprisUser;

GcomprisUser          *gcompris_user_new();
void                   gcompris_user_destroy();

/* find a user into a group */
GcomprisUser          *gcompris_user_find_by_name(GcomprisGroup *group, gchar *name);

/*****************************************************************************/
/* The following structure describes a profile object.  */

typedef struct {
  /* Profile Namee */
  gchar               *name;

  /* The subdirectory into the user gcompris dir */
  gchar		      *profile_dir;

  gchar               *description;
  gchar		      *filename;

  /* Help information */
  gchar		      *information;

  /* list of GcomprisGroup. if empty user management is disabled */
  GList               *groups; 

  /* User logged in */
  GcomprisUser        *user;

  /* list of activities to play -- gchar section/name */
  GList               *activities;

  /* Profile directoy */
  gchar               *directory;

} GcomprisProfile;

GcomprisProfile       *gcompris_get_profile();
GcomprisProfile       *gcompris_profile_load(gchar *name);
void                   gcompris_profile_destroy();
void                   gcompris_profile_save(GcomprisProfile *profile);


/* Usefull for management */
GList                 *list_of_classname();
GList                 *list_of_username();


#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
