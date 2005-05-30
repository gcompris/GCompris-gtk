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


