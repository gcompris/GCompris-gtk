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
        "CREATE TABLE informations (gcompris_version INT, update TEXT, profile_id INT ); "

void *gcompris_db_init()
{
  int rc;
  int creation = FALSE;
  char *zErrMsg;

  GcomprisProperties	*properties = gcompris_get_properties();
  
  if (!g_file_test(properties->database, G_FILE_TEST_EXISTS))
    creation = TRUE;

  rc = sqlite3_open(properties->database, &gcompris_db);
  if( rc ){
    g_error(stderr, "Can't open database: %s\n", sqlite3_errmsg(gcompris_db));
    sqlite3_close(gcompris_db);
    exit(1);
  }
  else g_warning("Database %s opened",properties->database);

  if (creation){
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_USERS, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_GROUPS, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_USERS_IN_GROUPS, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_GROUPS_IN_PROFILS, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_ACTIVITIES_OUT, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_PROFILES, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_BOARDS_PROFILES_CONF, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_BOARDS, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_BOARDS_LOCALES, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
    }
    rc = sqlite3_exec(gcompris_db,CREATE_TABLE_INFO, NULL,  0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
    }
  } /*else {
    
    }*/
  
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



