/* gcompris - gameutil.c
 *
 * Time-stamp: <2005/06/22 22:46:45 yves>
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
        "CREATE TABLE boards (board_id INT UNIQUE, name TEXT, section_id INT, section TEXT, author TEXT, type TEXT, mode TEXT, difficulty INT, icon TEXT, boarddir TEXT, mandatory_sound_file TEXT, mandatory_sound_dataset TEXT, filename TEXT, title TEXT, description TEXT, prerequisite TEXT, goal TEXT, manual TEXT, credit TEXT);"

#define CREATE_TABLE_INFO \
        "CREATE TABLE informations (gcompris_version TEXT, init_date TEXT, profile_id INT ); "

#define PRAGMA_INTEGRITY \
        "PRAGMA integrity_check; "

/* WARNING: template for g_strdup_printf */
#define SET_VERSION(v)\
        "INSERT INTO informations (gcompris_version) VALUES(\'%s\'); ", v

#define CHECK_VERSION \
        "SELECT gcompris_version FROM informations;"

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

#define SAVE_USER(user_id, login, name, firstname, birthday, class_id) \
        "INSERT OR REPLACE INTO users ( %d, \'%s\', \'%s\', \'%s\', %s)", user_id, login, name, firstname, birthday,class_id

#define CHECK_USER(n) \
        "SELECT user_id FROM users WHERE login=\'%s\';",n


#define MAX_USER_ID \
        "SELECT MAX(user_id) FROM users;"

void gcompris_db_save_user(int *user_id, gchar *login, gchar *name, gchar *firstname, gchar *birthday, int class_id)
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

  if (*user_id==0){
    /* user not yet registered */
    
    /* assume login is unique */
    
    request = g_strdup_printf(CHECK_USER(login));

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
      *user_id = atoi(result[1]);
      sqlite3_free_table(result);
    } else {

      /* get last board_id written */
      rc = sqlite3_get_table(gcompris_db, 
			     MAX_USER_ID,
			     &result,
			     &nrow,
			     &ncolumn,
			     &zErrMsg
			     );
      
      if( rc!=SQLITE_OK ){
	g_error("SQL error: %s\n", zErrMsg);
      }
      
      if (result[1] == NULL)
	*user_id = 1;
      else
	*user_id = atoi(result[1]) + 1;

      sqlite3_free_table(result);
      
    }      
  }
  
  request = g_strdup_printf(SAVE_USER( *user_id,
				       login,
				       name,
				       firstname,
				       birthday,
				       class_id));

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


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
