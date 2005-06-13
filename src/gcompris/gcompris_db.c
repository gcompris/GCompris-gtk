/* gcompris - gameutil.c
 *
 * Time-stamp: <2005/06/12 23:19:11 yves>
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

#include <sqlite3.h>
#include "gcompris.h"

static sqlite3 *gcompris_db=NULL;
static sqlite3 *gcompris_db_log=NULL;

#define CREATE_TABLE_USERS \
        "CREATE TABLE users (user_id INT UNIQUE, name TEXT, firstname TEXT, birth TEXT, class_id INT ); "
#define CREATE_TABLE_GROUPS \
        "CREATE TABLE groups (group_id INT UNIQUE, group_name TEXT, class_id INT, description TEXT ); "
#define CREATE_TABLE_USERS_IN_GROUPS  \
        "CREATE TABLE list_users_in_groups (user_id INT, group_id INT ); "
#define CREATE_TABLE_GROUPS_IN_PROFILS  \
        "CREATE TABLE list_groups_in_profils (profil_id INT, group_id INT ); "
#define CREATE_TABLE_ACTIVITIES_OUT \
        "CREATE TABLE activities_out (board_id INT, type INT, out_id INT ); "
#define CREATE_TABLE_PROFILES \
        "CREATE TABLE profiles (profile_id INT UNIQUE, profile_name TEXT, profile_directory TEXT, description TEXT); "
#define CREATE_TABLE_BOARDS_PROFILES_CONF \
        "CREATE TABLE board_profile_conf (profile_id INT, board_id INT, key TEXT, value TEXT ); "
#define CREATE_TABLE_BOARDS \
        "CREATE TABLE boards (board_id INT UNIQUE, name TEXT, section_id INT, section TEXT, author TEXT, type TEXT, mode TEXT, difficulty INT, icon TEXT, boarddir TEXT, title TEXT, description TEXT, prerequisite TEXT, goal TEXT, manual TEXT, credit TEXT);"

#define CREATE_TABLE_INFO \
        "CREATE TABLE informations (gcompris_version TEXT, init_date TEXT, profile_id INT ); "

#define PRAGMA_INTEGRITY \
        "PRAGMA integrity_check; "

/* WARNING: template for g_strdup_printf */
#define SET_VERSION(v)\
        "INSERT INTO informations (gcompris_version) VALUES(\'%s\'); ", v

#define CHECK_VERSION \
        "SELECT gcompris_version FROM informations;"

void *gcompris_db_init()
{
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
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_GROUPS, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_USERS_IN_GROUPS, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      g_error("SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_GROUPS_IN_PROFILS, NULL,  0, &zErrMsg);
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

    g_date_set_time (gdate, time (NULL));
    g_date_strftime (date, 20, "%F", gdate);
    
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
  
}

gcompris_db_exit()
{
  sqlite3_close(gcompris_db);
  g_warning("Database closed");
}

#define BOARDS_SET_DATE(date) \
        "UPDATE informations SET init_date=\'%s\';",date
gboolean gcompris_db_set_date(gchar *date)
{

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

}

#define BOARDS_CHECK \
        "SELECT gcompris_version, init_date FROM informations;"
gboolean gcompris_db_check_boards()
{

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
}


#define BOARD_INSERT(board_id, name, section_id, section, author, type, mode, difficulty, icon, boarddir, title, description, prerequisite, goal, manual, credit) \
        "INSERT OR REPLACE INTO boards VALUES (%d, \"%s\", \"%d\", \"%s\", \"%s\", \"%s\", \"%s\", %d, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\");", board_id, name, section_id, section, author, type, mode, difficulty, icon, boarddir, title, description, prerequisite, goal, manual, credit

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
			      gchar *title, 
			      gchar *description, 
			      gchar *prerequisite, 
			      gchar *goal, 
			      gchar *manual,
			      gchar *credit
			      )
{
  char *zErrMsg;
  char **result;
  int rc;
  int nrow;
  int ncolumn;
  int i,j;
  gchar *request;

  if (gcompris_db == NULL)
    g_error("Database is closed !!!");

  printf("gcompris_db_board_update:  *board_id %d\n",  *board_id);

  if (*board_id==0){
    /* board not yet registered */
    
    /* assume name is unique */
    
    request = g_strdup_printf(CHECK_BOARD(name));

    printf("request %s\n", request);
    
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
      printf("get MAX board_id \n");
      //sqlite3_free_table(result);
      
      printf("%s\n",MAX_BOARD_ID);
      rc = sqlite3_get_table(gcompris_db, 
			     MAX_BOARD_ID,
			     &result,
			     &nrow,
			     &ncolumn,
			     &zErrMsg
			     );
      
      printf("nrow %d %s\n", nrow, result[1]);

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
  
  printf ("*board_id = %d \n", *board_id);
  /* get section_id */
 
  request = g_strdup_printf(SECTION_ID(section));

  printf("request %s\n",request);
 
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

  printf("*section_id %d \n", *section_id );

  request = g_strdup_printf(BOARD_INSERT( *board_id,  name, *section_id, section, author, type, mode, difficulty, icon, boarddir,  title, description, prerequisite, goal, manual, credit));

  printf("request %s\n",request);

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
 
}


GList *gcompris_load_menus_db()
{
}

GList *gcompris_db_read_board_from_section(gchar *section)
{
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
