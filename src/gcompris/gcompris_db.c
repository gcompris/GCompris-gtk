/* gcompris - gcompris_db.c
 *
 * Time-stamp: <2005/07/18 15:19:39 yves>
 *
 * Copyright (C) 2000 Bruno Coudoin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gcompris.h"

#ifdef USE_SQLITE
static sqlite3 *gcompris_db=NULL;
static sqlite3 *gcompris_db_log=NULL;
#endif

extern GnomeCanvas *canvas;

#define CREATE_TABLE_USERS \
        "CREATE TABLE users (user_id INT UNIQUE, login TEXT, lastname TEXT, firstname TEXT, birthdate TEXT, class_id INT ); "
#define CREATE_TABLE_CLASS \
        "CREATE TABLE class (class_id INT UNIQUE, name TEXT, teacher TEXT, wholegroup_id INT ); "
#define CREATE_TABLE_GROUPS \
        "CREATE TABLE groups (group_id INT UNIQUE, name TEXT, class_id INT, description TEXT ); "
#define CREATE_TABLE_USERS_IN_GROUPS  \
        "CREATE TABLE list_users_in_groups (user_id INT, group_id INT ); "
#define CREATE_TABLE_GROUPS_IN_PROFILES  \
        "CREATE TABLE list_groups_in_profiles (profile_id INT, group_id INT ); "
#define CREATE_TABLE_ACTIVITIES_OUT \
        "CREATE TABLE activities_out (board_id INT, type INT, out_id INT ); "
#define CREATE_TABLE_PROFILES \
        "CREATE TABLE profiles (profile_id INT UNIQUE, profile_name TEXT, profile_directory TEXT, description TEXT); "
#define CREATE_TABLE_BOARDS_PROFILES_CONF \
        "CREATE TABLE board_profile_conf (profile_id INT, board_id INT, key TEXT, value TEXT ); "
#define CREATE_TABLE_BOARDS \
        "CREATE TABLE boards (board_id INT UNIQUE, name TEXT, section_id INT, section TEXT, author TEXT, type TEXT, mode TEXT, difficulty INT, icon TEXT, boarddir TEXT, mandatory_sound_file TEXT, mandatory_sound_dataset TEXT, filename TEXT, title TEXT, description TEXT, prerequisite TEXT, goal TEXT, manual TEXT, credit TEXT);"

#define CREATE_TABLE_INFO \
        "CREATE TABLE informations (gcompris_version TEXT UNIQUE, init_date TEXTUNIQUE, profile_id INT UNIQUE ); "

#define PRAGMA_INTEGRITY \
        "PRAGMA integrity_check; "

/* WARNING: template for g_strdup_printf */
#define SET_VERSION(v)\
        "INSERT INTO informations (gcompris_version) VALUES(\'%s\'); ", v

#define CHECK_VERSION \
        "SELECT gcompris_version FROM informations;"

#define SET_DEFAULT_PROFILE \
        "INSERT INTO profiles (profile_id, profile_name, profile_directory, description) VALUES ( 1, \'Default\', \'Default\', \'Default profil for gcompris\');"
#define ACTIVATE_DEFAULT_PROFILE \
        "UPDATE informations SET profile_id=1;"

int gcompris_db_init()
{
#ifdef USE_SQLITE
  gboolean creation = FALSE;
  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  int i,j;
  gchar *request;
  GDate *gdate;
  gchar date[10];


  GcomprisProperties	*properties = gcompris_get_properties();
  
  if (!g_file_test(properties->database, G_FILE_TEST_EXISTS))
    creation = TRUE;

  rc = sqlite3_open(properties->database, &gcompris_db);
  if( rc ){
    g_error("Can't open database: %s\n", sqlite3_errmsg(gcompris_db));
    sqlite3_close(gcompris_db);
    exit(1);
  }

  g_warning(_("Database %s opened"),properties->database);

  if (creation){
    /* create all tables needed */
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_USERS, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_CLASS, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_GROUPS, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_USERS_IN_GROUPS, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_GROUPS_IN_PROFILES, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_ACTIVITIES_OUT, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_PROFILES, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_BOARDS_PROFILES_CONF, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_BOARDS, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_INFO, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }

    g_warning(_("Database tables created"));

    request = g_strdup_printf(SET_VERSION(VERSION));

    rc = sqlite3_get_table(gcompris_db, 
			   request,  
			   &result,
			   &nrow,
			   &ncolumn,
			   &zErrMsg
			   );
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }

    rc = sqlite3_exec(gcompris_db,SET_DEFAULT_PROFILE, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }

    rc = sqlite3_exec(gcompris_db,ACTIVATE_DEFAULT_PROFILE, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }


    sqlite3_free_table(result);

    g_free(request);

  } else {
    /* Check the db integrity */
    rc = sqlite3_get_table(gcompris_db, 
			   PRAGMA_INTEGRITY,  
			   &result,
			   &nrow,
			   &ncolumn,
			   &zErrMsg
			   );
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    if (!(strcmp(result[1],"ok")==0))
      g_error("DATABASE integrity check returns %s \n", result[1]);
    g_warning("Database Integrity ok");
    sqlite3_free_table(result);

    rc = sqlite3_get_table(gcompris_db, 
			   CHECK_VERSION,  
			   &result,
			   &nrow,
			   &ncolumn,
			   &zErrMsg
			   );
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }

    if (strcmp(result[1],VERSION)!=0)
      g_warning("Running GCompris is %s, but databse vrsion is %s", VERSION, result[1]);
    sqlite3_free_table(result);
  }

  return TRUE;
#else
  return FALSE;
#endif
}

void gcompris_db_exit()
{
#ifdef USE_SQLITE
  sqlite3_close(gcompris_db);
  g_warning("Database closed");
#endif
}

#define BOARDS_SET_DATE(date) \
        "UPDATE informations SET init_date=\'%s\';",date

void gcompris_db_set_date(gchar *date)
{
#ifdef USE_SQLITE

  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  gboolean ret_value;
  gchar *request;

  request = g_strdup_printf(BOARDS_SET_DATE(date));
  rc = sqlite3_get_table(gcompris_db, 
			 request,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    g_free(request);

    sqlite3_free_table(result);

#endif
}

#define BOARDS_CHECK \
        "SELECT gcompris_version, init_date FROM informations;"
gboolean gcompris_db_check_boards()
{
#ifdef USE_SQLITE

  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  gboolean ret_value;

  rc = sqlite3_get_table(gcompris_db, 
			 BOARDS_CHECK,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }

    ret_value = (strcmp(result[2],VERSION)==0) && (result[3] != NULL);

    sqlite3_free_table(result);

    return ret_value;
#else
    return FALSE;
#endif
}


#define Q(a) a==NULL ? "" : "\"", a==NULL ? "NULL" : a, a==NULL ? "" : "\""
#define BOARD_INSERT(board_id, name, section_id, section, author, type, mode, difficulty, icon, boarddir, mandatory_sound_file, mandatory_sound_dataset, filename, title, description, prerequisite, goal, manual, credit) \
        "INSERT OR REPLACE INTO boards VALUES (%d, %s%s%s, %d, %s%s%s, %s%s%s, %s%s%s, %s%s%s, %d, %s%s%s, %s%s%s, %s%s%s, %s%s%s, %s%s%s, %s%s%s, %s%s%s, %s%s%s, %s%s%s, %s%s%s, %s%s%s);", board_id, Q(name), section_id, Q(section), Q(author), Q(type), Q(mode), difficulty, Q(icon), Q(boarddir), Q(mandatory_sound_file), Q(mandatory_sound_dataset), Q(filename), Q(title), Q(description), Q(prerequisite), Q(goal), Q(manual), Q(credit)

#define MAX_BOARD_ID \
        "SELECT MAX(board_id) FROM boards;"

#define SECTION_ID(s) \
        "SELECT section_id FROM boards WHERE section=\'%s\';",s

#define MAX_SECTION_ID \
        "SELECT MAX(section_id) FROM boards;"

#define CHECK_BOARD(n) \
        "SELECT board_id FROM boards WHERE name=\'%s\';",n


void gcompris_db_board_update(gint *board_id,
			      gint *section_id,
			      gchar *name,
			      gchar *section,
			      gchar *author,
			      gchar *type,
			      gchar *mode,
			      int difficulty,
			      gchar *icon,
			      gchar *boarddir,
			      gchar *mandatory_sound_file,
			      gchar *mandatory_sound_dataset,
			      gchar *filename,
			      gchar *title, 
			      gchar *description, 
			      gchar *prerequisite, 
			      gchar *goal, 
			      gchar *manual,
			      gchar *credit
			      )
{
#ifdef USE_SQLITE

  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  int i,j;
  gchar *request;

  if (gcompris_db == NULL)
    g_error("Database is closed !!!");

  if (*board_id==0){
    /* board not yet registered */
    
    /* assume name is unique */
    
    request = g_strdup_printf(CHECK_BOARD(name));

    rc = sqlite3_get_table(gcompris_db, 
			   request,  
			   &result,
			   &nrow,
			   &ncolumn,
			   &zErrMsg
			   );
    
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }

    g_free(request);

    if (nrow != 0){
      *board_id = atoi(result[1]);
      sqlite3_free_table(result);
    } else {

      /* get last board_id written */
      rc = sqlite3_get_table(gcompris_db, 
			     MAX_BOARD_ID,
			     &result,
			     &nrow,
			     &ncolumn,
			     &zErrMsg
			     );
      
      if( rc!=SQLITE_OK ){
	g_error("SQL error: %s\n", zErrMsg);
      }
      
      if (result[1] == NULL)
	*board_id = 1;
      else
	*board_id = atoi(result[1]) + 1;

      sqlite3_free_table(result);
      
    }      
  }
  
  /* get section_id */
  request = g_strdup_printf(SECTION_ID(section));
 
  rc = sqlite3_get_table(gcompris_db, 
			 request,
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
    
  g_free(request);
 
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }
    
  if (nrow == 0){
    
    /* get max section_id */
    
    rc = sqlite3_get_table(gcompris_db, 
			   MAX_SECTION_ID,
			   &result,
			   &nrow,
			   &ncolumn,
			   &zErrMsg
			   );
    
    
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
      
    if (result[1] == NULL){
      *section_id = 1;
    } else {
      *section_id = atoi(result[1]) + 1;
    }
    sqlite3_free_table(result);
  } else { 
    *section_id = atoi(result[1]);
    sqlite3_free_table(result);
  }

  request = g_strdup_printf(BOARD_INSERT( *board_id,  name, *section_id, section, 
					  author, type, mode, difficulty, icon, boarddir,
					  mandatory_sound_file, mandatory_sound_dataset, 
					  filename, title, description, prerequisite, goal, 
					  manual, credit));

  rc = sqlite3_get_table(gcompris_db, 
			 request,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }
  
  sqlite3_free_table(result);
  
  g_free(request);
 
#endif
}


#define BOARDS_READ \
        "SELECT board_id ,name, section_id, section, author, type, mode, difficulty, icon, boarddir, mandatory_sound_file, mandatory_sound_dataset, filename, title, description, prerequisite, goal, manual, credit FROM boards;"

GList *gcompris_load_menus_db(GList *boards_list)
{
#ifdef USE_SQLITE

  GcomprisProperties	*properties = gcompris_get_properties();
  
  GList *boards = boards_list;

  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  int i;

  rc = sqlite3_get_table(gcompris_db, 
			 BOARDS_READ,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }

  /* first ncolumns are columns labels. */
  i = ncolumn;
  
  while (i < (nrow +1)*ncolumn) {
    GcomprisBoard *gcomprisBoard = NULL;

    gcomprisBoard = g_malloc0 (sizeof (GcomprisBoard));
    
    gcomprisBoard->plugin=NULL;
    gcomprisBoard->previous_board=NULL;
    gcomprisBoard->board_ready=FALSE;
    gcomprisBoard->canvas=canvas;
    
    gcomprisBoard->gmodule      = NULL;
    gcomprisBoard->gmodule_file = NULL;
    
    /* From DB we have only package_data_dir. */
    gcomprisBoard->board_dir = properties->package_data_dir;
    
    /* Fixed since I use the canvas own pixel_per_unit scheme */
    gcomprisBoard->width  = BOARDWIDTH;
    gcomprisBoard->height = BOARDHEIGHT;
    
    
    gcomprisBoard->board_id = atoi(result[i++]);
    gcomprisBoard->name = g_strdup(result[i++]);
    gcomprisBoard->section_id = atoi(result[i++]);
    gcomprisBoard->section = g_strdup(result[i++]);
    gcomprisBoard->author = g_strdup(result[i++]);
    gcomprisBoard->type = g_strdup(result[i++]);
    gcomprisBoard->mode = g_strdup(result[i++]);
    gcomprisBoard->difficulty = g_strdup(result[i++]);
    gcomprisBoard->icon_name = g_strdup(result[i++]);
    gcomprisBoard->boarddir = g_strdup(result[i++]);
    gcomprisBoard->mandatory_sound_file = g_strdup(result[i++]);
    gcomprisBoard->mandatory_sound_dataset = g_strdup(result[i++]);
    gcomprisBoard->filename = g_strdup(result[i++]);
    gcomprisBoard->title =  reactivate_newline(gettext(result[i++]));
    gcomprisBoard->description  = reactivate_newline(gettext(result[i++]));
    gcomprisBoard->prerequisite = reactivate_newline(gettext(result[i++]));
    gcomprisBoard->goal = reactivate_newline(gettext(result[i++]));
    gcomprisBoard->manual = reactivate_newline(gettext(result[i++]));
    gcomprisBoard->credit = reactivate_newline(gettext(result[i++]));
    
    boards = g_list_append(boards, gcomprisBoard);
  } 

  sqlite3_free_table(result);
  
  return boards;

#else
  return NULL;
#endif
}

GList *gcompris_db_read_board_from_section(gchar *section)
{
}


#define BOARD_ID_READ \
        "SELECT board_id FROM boards;"

GList *gcompris_db_get_board_id(GList *list)
{
#ifdef USE_SQLITE
  int *board_id = g_malloc0(sizeof(int));
  
  GList *board_id_list = list;

  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  int i;
  
  rc = sqlite3_get_table(gcompris_db, 
			 BOARD_ID_READ,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }

  /* first ncolumns are columns labels. */
  i = ncolumn;
  
  while (i < (nrow +1)*ncolumn) {
    int *board_id = g_malloc(sizeof(int));
  
    *board_id = atoi(result[i++]);
    board_id_list = g_list_append(board_id_list, board_id);
  }

  return  board_id_list;

#else
  return NULL;
#endif
}

#define DELETE_BOARD(table, board_id) \
        "DELETE FROM %s WHERE board_id=%d;", table, board_id

void gcompris_db_remove_board(int board_id)
{
#ifdef USE_SQLITE
  g_warning("Supress board %d from db.", board_id);

  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  int i;
  gchar *request;

  /* get section_id */
  request = g_strdup_printf(DELETE_BOARD("boards",board_id));

  rc = sqlite3_get_table(gcompris_db, 
			 request,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }

  g_free(request);


  /* get section_id */
  request = g_strdup_printf(DELETE_BOARD("board_profile_conf",board_id));

  rc = sqlite3_get_table(gcompris_db, 
			 request,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }

  g_free(request);


  /* get section_id */
  request = g_strdup_printf(DELETE_BOARD("activities_out",board_id));

  rc = sqlite3_get_table(gcompris_db, 
			 request,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }

  g_free(request);
#endif
}


#define GET_PROFILE(n) \
        "SELECT profile_name, profile_directory, description FROM profiles WHERE profile_id=%d;",n

#define GET_GROUPS_IN_PROFILE(n) \
        "SELECT group_id FROM list_groups_in_profiles WHERE profile_id=%d;",n

#define GET_ACTIVITIES_OUT_OF_PROFILE(n) \
        "SELECT board_id FROM activities_out WHERE out_id=%d;",n

GcomprisProfile *gcompris_get_profile_from_id(gint profile_id)
{
#ifdef USE_SQLITE
  GcomprisProfile *profile = NULL;

  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  gchar *request;

  int i;
  GList *ids;
  /* get section_id */
  request = g_strdup_printf(GET_PROFILE(profile_id));

  
  rc = sqlite3_get_table(gcompris_db, 
			 request,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }
  
  if (nrow != 0){
    profile = g_malloc0(sizeof(GcomprisProfile));

    profile->profile_id = profile_id; 

    
    profile->name = g_strdup(result[3]);
    profile->directory = g_strdup(result[4]);
    profile->description = g_strdup(result[5]);
    
    g_free(request);
    
    request = g_strdup_printf(GET_GROUPS_IN_PROFILE(profile->profile_id));

    rc = sqlite3_get_table(gcompris_db, 
			   request,  
			   &result,
			   &nrow,
			   &ncolumn,
			   &zErrMsg
			 );
    
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }

    g_free(request);
    
    if (nrow == 0){
      g_warning(_("No users groups for profile %s"), profile->name);
      profile->group_ids = NULL;
    } else {
      ids = NULL;
      
      i = ncolumn;
      while (i < (nrow +1)*ncolumn) {
	int *group_id = g_malloc(sizeof(int));
	
	*group_id = atoi(result[i++]);
	ids = g_list_append(ids, group_id);
      }
      profile->group_ids = ids;
    }

    request = g_strdup_printf(GET_ACTIVITIES_OUT_OF_PROFILE(profile->profile_id));
    rc = sqlite3_get_table(gcompris_db, 
			   request,  
			   &result,
			   &nrow,
			   &ncolumn,
			   &zErrMsg
			   );
    
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    
    g_free(request);
    
    if (nrow == 0){
      g_warning(_("No activities out for profile %s"), profile->name);
      profile->activities = NULL;
    } else {
      ids = NULL;
      
      i = ncolumn;
      while (i < (nrow +1)*ncolumn) {
	int *board_id = g_malloc(sizeof(int));
	
	*board_id = atoi(result[i++]);
	ids = g_list_append(ids, board_id);
      }
      profile->activities = ids;
    }
  }

  return profile;
#else
  return NULL;
#endif
}

#define GET_ACTIVE_PROFILE_ID \
        "SELECT profile_id FROM informations;"

GcomprisProfile *gcompris_db_get_profile()
{
#ifdef USE_SQLITE
  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  gchar *request;

  int profile_id;

  rc = sqlite3_get_table(gcompris_db, 
			 GET_ACTIVE_PROFILE_ID,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }

  profile_id = atoi(result[1]); 

  return gcompris_get_profile_from_id(profile_id);

#else
  return NULL;
#endif
}

#define USERS_FROM_GROUP(n) \
        "SELECT users.user_id, users.login, users.lastname, users.firstname, users.birthdate, users.class_id  FROM users, list_users_in_groups WHERE users.user_id = list_users_in_groups.user_id AND list_users_in_groups.group_id = %d;",n

GList *gcompris_get_users_from_group(gint group_id)
{
#ifdef USE_SQLITE
  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  gchar *request;

  int i;
  GList *users = NULL;

  request = g_strdup_printf(USERS_FROM_GROUP(group_id));
  rc = sqlite3_get_table(gcompris_db, 
			 request,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }

  g_free(request);

  if (nrow == 0){
    g_warning(_("No users in group with id  %d"), group_id);
  } else {
    i = ncolumn;
    while (i < (nrow +1)*ncolumn) {
      GcomprisUser *user = g_malloc0(sizeof(GcomprisUser));
      
      user->user_id = atoi(result[i++]);
      user->login = g_strdup(result[i++]);
      user->lastname = g_strdup(result[i++]);
      user->firstname = g_strdup(result[i++]);
      user->birthdate = g_strdup(result[i++]);
      user->class_id = atoi(result[i++]);
      
      users = g_list_append(users, user);
    }
  }

  return users;
#else
  return NULL;
#endif
}

#define USER_FROM_ID(n) \
        "SELECT users.login, lastname, firstname, birthdate, class_id  FROM users WHERE user_id = %d;",n

GcomprisUser *gcompris_get_user_from_id(gint user_id)
{
#ifdef USE_SQLITE
  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  gchar *request;

  int i;
  GList *users = NULL;

  GcomprisUser *user = NULL;

  request = g_strdup_printf(USER_FROM_ID(user_id));
  rc = sqlite3_get_table(gcompris_db, 
			 request,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }

  g_free(request);

  if (nrow == 0){
    g_warning(_("No user with id  %d"), user_id);
    return NULL;
  } else {
    i = ncolumn;
    user = g_malloc0(sizeof(GcomprisUser));
    
    user->user_id = user_id;
    user->login = g_strdup(result[i++]);
    user->lastname = g_strdup(result[i++]);
    user->firstname = g_strdup(result[i++]);
    user->birthdate = g_strdup(result[i++]);
    user->class_id = atoi(result[i++]);
  }
  

  return user ;
#else
  return NULL;
#endif
}

#define CLASS_FROM_ID(n) \
        "SELECT name, teacher, wholegroup_id  FROM class WHERE class_id = %d;",n

#define GROUPS_IN_CLASS(n) \
        "SELECT group_id  FROM groups WHERE class_id = %d;",n

GcomprisClass *gcompris_get_class_from_id(gint class_id)
{
#ifdef USE_SQLITE
  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  gchar *request;

  int i;
  GList *users = NULL;
  GcomprisClass *class = NULL;

  request = g_strdup_printf(CLASS_FROM_ID(class_id));
  rc = sqlite3_get_table(gcompris_db, 
			 request,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }

  g_free(request);

  if (nrow == 0){
    g_warning(_("No class with id  %d"), class_id);
    return NULL;
    return NULL;
  } else {
    i = ncolumn;

    class = g_malloc0(sizeof(GcomprisClass));

    class->class_id = class_id;
    class->name = g_strdup(result[i++]);
    class->description = g_strdup(result[i++]);
    class->wholegroup_id = atoi(result[i++]);
  }

  /* Group _ids */

  GList *group_ids = NULL;

  request = g_strdup_printf(GROUPS_IN_CLASS(class_id));

  rc = sqlite3_get_table(gcompris_db, 
			 request,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }

  g_free(request);

  if (nrow == 0){
    g_error(_("No groups in for class %s, there must be at least one for whole class (%d)"), class_id, class->wholegroup_id);
    g_free(class);
    class = NULL;
  } else {

    i = ncolumn;
    while (i < (nrow +1)*ncolumn) {
      int *group_id = g_malloc(sizeof(int));

      *group_id = atoi(result[i++]);
      group_ids = g_list_append(group_ids, group_id);
    }
    class->group_ids = group_ids;
  }

  return class ;
#else
  return NULL;
#endif
}


#define CHECK_CONF(p, b, k) \
        "SELECT * FROM board_profile_conf WHERE profile_id=%d AND board_id=%d AND key=%s%s%s;", p, b, Q(k)

#define INSERT_KEY(p, b, k, v) \
        "INSERT INTO board_profile_conf (profile_id, board_id, key, value) VALUES (%d, %d, %s%s%s, %s%s%s);", p, b, Q(k), Q(v)

#define UPDATE_KEY(p, b, k, v) \
        "UPDATE board_profile_conf SET value=%s%s%s WHERE profile_id=%d AND board_id=%d AND key=%s%s%s;", Q(v), p, b, Q(k)

void gcompris_set_board_conf(GcomprisProfile *profile, 
			     GcomprisBoard  *board, 
			     gchar *key, 
			     gchar *value)
{
  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  gchar *request;

  request = g_strdup_printf(CHECK_CONF(profile->profile_id, 
				       board->board_id, 
				       key));
  
  rc = sqlite3_get_table(gcompris_db, 
			 request,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }
  
  g_free(request);
  
  if (nrow == 0){
    request = g_strdup_printf(INSERT_KEY(profile->profile_id, 
					 board->board_id, 
					 key,
					 value));
    
    rc = sqlite3_get_table(gcompris_db, 
			   request,  
			   &result,
			   &nrow,
			   &ncolumn,
			   &zErrMsg
			   );
    
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    
    g_free(request);
  } else {
    request = g_strdup_printf(UPDATE_KEY(profile->profile_id, 
					 board->board_id, 
					 key,
					 value));
    
    rc = sqlite3_get_table(gcompris_db, 
			   request,  
			   &result,
			   &nrow,
			   &ncolumn,
			   &zErrMsg
			   );
    
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    
    g_free(request);
  }
} 

#define GET_CONF(p, b) \
        "SELECT key, value FROM board_profile_conf WHERE profile_id=%d AND board_id=%d;", p, b

GList *gcompris_get_board_conf()
{
  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  gchar *request;
  int i;

  GList *list_conf = NULL;

  request = g_strdup_printf(GET_CONF(gcompris_get_current_profile()->profile_id, 
				     get_current_gcompris_board()->board_id));
  
  rc = sqlite3_get_table(gcompris_db, 
			 request,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }

  g_free(request);
  
  i = ncolumn;

  if (nrow != 0){
    while (i < (nrow +1)*ncolumn){
      GcomprisConfPair *pair = g_malloc0(sizeof(GcomprisConfPair));
      
      pair->key = g_strdup(result[i++]);
      pair->value = g_strdup(result[i++]);
      
      list_conf = g_list_append(list_conf, pair);
    }
  } else {
    /* Get default profile if nothing in gcompris_get_current_profile() */
    request = g_strdup_printf(GET_CONF( 1, 
				       get_current_gcompris_board()->board_id));
  
    rc = sqlite3_get_table(gcompris_db, 
			   request,  
			   &result,
			   &nrow,
			   &ncolumn,
			   &zErrMsg
			   );
  
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }

    g_free(request);
  
    i = ncolumn;

    if (nrow != 0){
      while (i < (nrow +1)*ncolumn){
	GcomprisConfPair *pair = g_malloc0(sizeof(GcomprisConfPair));
      
	pair->key = g_strdup(result[i++]);
	pair->value = g_strdup(result[i++]);
	
	list_conf = g_list_append(list_conf, pair);
      }
      
    }
  } 

  return list_conf;

}

#define GET_ALL_PROFILES \
        "SELECT profile_id, profile_name, profile_directory, description FROM profiles;"


GList *gcompris_get_profiles_list()
{
#ifdef USE_SQLITE


  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  gchar *request;

  int i;
  GList *profiles_list;

  char **result_;
  int nrow_;
  int ncolumn_;

  int i_;
  GList *ids_;

  
  rc = sqlite3_get_table(gcompris_db, 
			 GET_ALL_PROFILES,  
			 &result,
			 &nrow,
			 &ncolumn,
			 &zErrMsg
			 );
  
  if( rc!=SQLITE_OK ){
    g_error("SQL error: %s\n", zErrMsg);
  }

  if (nrow == 0)
    return NULL;
  
  i = ncolumn;
  while (i < (nrow +1)*ncolumn) {
    GcomprisProfile *profile = g_malloc0(sizeof(GcomprisProfile));

    profile->profile_id = atoi(result[i++]); 
    
    profile->name = g_strdup(result[i++]);
    profile->directory = g_strdup(result[i++]);
    profile->description = g_strdup(result[i++]);
    
    request = g_strdup_printf(GET_GROUPS_IN_PROFILE(profile->profile_id));

    rc = sqlite3_get_table(gcompris_db, 
			   request,  
			   &result_,
			   &nrow_,
			   &ncolumn_,
			   &zErrMsg
			 );
    
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }

    g_free(request);
    
    if (nrow_ == 0){
      g_warning(_("No users groups for profile %s"), profile->name);
      profile->group_ids = NULL;
    } else {
      ids_ = NULL;
      
      i_ = ncolumn_;
      while (i_ < (nrow_ +1)*ncolumn_) {
	int *group_id = g_malloc(sizeof(int));
	
	*group_id = atoi(result_[i_++]);
	ids_ = g_list_append(ids_, group_id);
      }
      profile->group_ids = ids_;
    }

    sqlite3_free_table(result_);

    request = g_strdup_printf(GET_ACTIVITIES_OUT_OF_PROFILE(profile->profile_id));
    rc = sqlite3_get_table(gcompris_db, 
			   request,  
			   &result_,
			   &nrow_,
			   &ncolumn_,
			   &zErrMsg
			   );
    
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    
    g_free(request);
    
    if (nrow_ == 0){
      g_warning(_("No activities out for profile %s"), profile->name);
      profile->activities = NULL;
    } else {
      ids_ = NULL;
      
      i_ = ncolumn_;
      while (i_ < (nrow_ +1)*ncolumn_) {
	int *board_id = g_malloc(sizeof(int));
	
	*board_id = atoi(result_[i_++]);
	ids_ = g_list_append(ids_, board_id);
      }
      profile->activities = ids_;
    }

  sqlite3_free_table(result_);  
  profiles_list = g_list_append( profiles_list, profile);
  }  

  sqlite3_free_table(result);  
  
  return profiles_list;
#else
  return NULL;
#endif
}

GcomprisGroup *gcompris_get_group_from_id(int group_id)
{
  return NULL;
}

GList *gcompris_get_groups_list()
{
  return NULL;
}

GcomprisBoard *gcompris_get_board_from_id(int board_id)
{
  return NULL;
}

GList *gcompris_get_users_list()
{
  return NULL;
}

GList *gcompris_get_classes_list()
{
  return NULL;
}

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
