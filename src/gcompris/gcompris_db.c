#include <sqlite3.h>
#include "gcompris.h"

static sqlite3 *gcompris_db;
static sqlite3 *gcompris_db_log;

#define CREATE_TABLE_USERS \
        "CREATE TABLE users (user_id INT, name TEXT, firstname TEXT, birth TEXT, class_id INT ); "
#define CREATE_TABLE_GROUPS \
        "CREATE TABLE groups (group_id INT, group_name TEXT, class_id INT, description TEXT ); "
#define CREATE_TABLE_USERS_IN_GROUPS  \
        "CREATE TABLE list_users_in_groups (user_id INT, group_id INT ); "
#define CREATE_TABLE_GROUPS_IN_PROFILS  \
        "CREATE TABLE list_groups_in_profils (profil_id INT, group_id INT ); "
#define CREATE_TABLE_ACTIVITIES_OUT \
        "CREATE TABLE activities_out (board_id INT, type INT, out_id INT ); "
#define CREATE_TABLE_PROFILES \
        "CREATE TABLE profiles (profile_id INT, profile_name TEXT, profile_directory TEXT, description TEXT); "
#define CREATE_TABLE_BOARDS_PROFILES_CONF \
        "CREATE TABLE board_profile_conf (profile_id INT, board_id INT, key TEXT, value TEXT ); "
#define CREATE_TABLE_BOARDS \
        "CREATE TABLE boards (board_id INT, name TEXT, section_id INT, section TEXT, author TEXT, type TEXT, mode TEXT, difficulty INT, icon TEXT );"
#define CREATE_TABLE_BOARDS_LOCALES \
        "CREATE TABLE boards_locales (board_id INT, language TEXT, title TEXT, description TEXT, prerequisite TEXT, goal TEXT, manual TEXT);"

#define CREATE_TABLE_INFO \
        "CREATE TABLE informations (gcompris_version TEXT, init_date TEXT, profile_id INT ); "

#define PRAGMA_INTEGRITY \
        "PRAGMA integrity_check; "

/* WARNING: template for g_strdup_printf */
#define SET_VERSION(v,d)\
        "INSERT INTO informations (gcompris_version, init_date) VALUES(\'%s\', \'%s\'); ", v,d

#define CHECK_VERSION \
        "SELECT gcompris_version FROM informations;"

void *gcompris_db_init()
{
  int creation = FALSE;
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
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_BOARDS_LOCALES, NULL,  0, &zErrMsg);
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
    
    request = g_strdup_printf(SET_VERSION(VERSION, date));

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
   
  }
  
}

gcompris_db_exit()
{
  sqlite3_close(gcompris_db);
  g_warning("Database closed");
}

void gcompris_db_board_update(gchar *name, gchar *section, gchar *author, gchar *type, gchar *mode, int difficulty, gchar *icon)
{
}

void gcompris_db_board_locale_update(int board_id, gchar *language, gchar *title, gchar *description, gchar *prerequisite, gchar *goal, gchar *manual)
{
}

GList *gcompris_db_read_board_from_section(gchar *section)
{
}

