/*  GCompris -- This files comes from XMMS
 * 
 *  XMMS - Cross-platform multimedia player
 *  Copyright (C) 1998-2000  Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "gcompris.h"

static struct BoardPluginData *bp_data;

#if defined _WIN32 || defined __WIN32__
# undef WIN32   /* avoid warning on mingw32 */
# define WIN32
#endif


#ifdef WIN32
extern gboolean    grace_period;
extern BoardPlugin advanced_colors_menu_bp;
extern BoardPlugin algebra_menu_bp;
extern BoardPlugin algebra_guesscount_menu_bp;
extern BoardPlugin canal_lock_menu_bp;
extern BoardPlugin clickgame_menu_bp;
extern BoardPlugin click_on_letter_menu_bp;
extern BoardPlugin clockgame_menu_bp;
extern BoardPlugin colors_menu_bp;
extern BoardPlugin draw_menu_bp;
extern BoardPlugin enumerate_menu_bp;
extern BoardPlugin erase_menu_bp;
extern BoardPlugin fifteen_menu_bp;
extern BoardPlugin gletters_menu_bp;
extern BoardPlugin hanoi_menu_bp;
extern BoardPlugin imageid_menu_bp;
extern BoardPlugin leftright_menu_bp;
extern BoardPlugin machpuzzle_menu_bp;
extern BoardPlugin maze_menu_bp;
extern BoardPlugin memory_menu_bp;
extern BoardPlugin menu_menu_bp;
extern BoardPlugin missingletter_menu_bp;
extern BoardPlugin money_menu_bp;
extern BoardPlugin paratrooper_menu_bp;
extern BoardPlugin planegame_menu_bp;
extern BoardPlugin railroad_menu_bp;
extern BoardPlugin read_colors_menu_bp;
extern BoardPlugin reading_menu_bp;
extern BoardPlugin reversecount_menu_bp;
extern BoardPlugin shapegame_menu_bp;
extern BoardPlugin smallnumbers_menu_bp;
extern BoardPlugin submarine_menu_bp;
extern BoardPlugin superbrain_menu_bp;
extern BoardPlugin target_menu_bp;
extern BoardPlugin traffic_menu_bp;
extern BoardPlugin wordsgame_menu_bp;

static BoardPlugin *static_boards_demo[] = {
  &algebra_menu_bp,
  &click_on_letter_menu_bp,
  &colors_menu_bp,
  &clickgame_menu_bp,
  &draw_menu_bp,
  &enumerate_menu_bp,
  &erase_menu_bp,
  &gletters_menu_bp,
  &hanoi_menu_bp,
  &menu_menu_bp,
  &reading_menu_bp,
  &submarine_menu_bp,
  &superbrain_menu_bp,
  &target_menu_bp,
};

static BoardPlugin *static_boards[] = {
  &advanced_colors_menu_bp,
  &algebra_guesscount_menu_bp,
  &imageid_menu_bp,
  &leftright_menu_bp,
  &machpuzzle_menu_bp,
  &maze_menu_bp,
  &memory_menu_bp,
  &missingletter_menu_bp,
  &money_menu_bp,
  &paratrooper_menu_bp,
  &planegame_menu_bp,
  &read_colors_menu_bp,
  &reversecount_menu_bp,
  &shapegame_menu_bp,
  &smallnumbers_menu_bp,
  &traffic_menu_bp,
  &wordsgame_menu_bp,
  &fifteen_menu_bp,
  &algebra_menu_bp,
  &click_on_letter_menu_bp,
  &colors_menu_bp,
  &canal_lock_menu_bp,
  &clickgame_menu_bp,
  &clockgame_menu_bp,
  &draw_menu_bp,
  &enumerate_menu_bp,
  &erase_menu_bp,
  &gletters_menu_bp,
  &hanoi_menu_bp,
  &menu_menu_bp,
  &railroad_menu_bp,
  &reading_menu_bp,
  &submarine_menu_bp,
  &superbrain_menu_bp,
  &target_menu_bp,
  NULL
};
#endif

/*
 * The directory in which we will search for plugins
 * (In that order)
 */
static gchar *plugin_paths[] = {
  "../boards/.libs/",		/* Usefull path to test plugins without re-installing them */
  "./boards/.libs/",
  PLUGIN_DIR,
  NULL
};

#ifdef WIN32
void init_plugins(void)
{
  guint i=0;
  BoardPlugin **boards_list = NULL;
  printf(">init_plugins\n");
  /* First make sure the module loading is supported on this platform */
  if (!g_module_supported())
    g_error("Dynamic module loading is not supported. gcompris cannot work.\n");

  bp_data = g_malloc0(sizeof (struct BoardPluginData));

  while(static_boards[i++] != NULL) {
    /* If this plugin defines an initialisation entry point, call it */
    BoardPlugin *bp;
      
    /* Get the BoardPlugin Info */
    bp = (BoardPlugin *) static_boards[i-1];
      
    printf("Initializing plugin %s\n", bp->name);
    if(bp->init != NULL) {
      bp->init();
    }
  }

  printf("<init_plugins\n");

}
#else
void init_plugins(void)
{

  /* Fist make sure the module loading is supported on this platform */
  if (!g_module_supported())
    g_error( _("Dynamic module loading is not supported. gcompris cannot load.\n") );

  bp_data = g_malloc0(sizeof (struct BoardPluginData));

}
#endif

BoardPlugin *get_current_board_plugin(void)
{
  return bp_data->current_gcompris_board->plugin;
}

GcomprisBoard *get_current_gcompris_board(void)
{
  return bp_data->current_gcompris_board;
}

void set_current_gcompris_board(GcomprisBoard * gcomprisBoard)
{
  bp_data->current_gcompris_board = gcomprisBoard;
}

#ifdef WIN32
gboolean board_check_file(GcomprisBoard *gcomprisBoard)
{
  GcomprisProperties	*properties = gcompris_get_properties();
  BoardPlugin *bp;
  guint        i=0;

  g_assert(gcomprisBoard!=NULL);
  g_assert(properties->key!=NULL);

  /* Check Already loaded */  
  if(gcomprisBoard->plugin!=NULL) {
    return TRUE;
  }

  if(strncmp(properties->key, "thanks_for_your_help", 20)==0 || grace_period==TRUE) {
    while(static_boards[i++] != NULL) {

      BoardPlugin *bp;

      /* Get the BoardPlugin Info */
      bp = (BoardPlugin *) static_boards[i-1];

      if(bp->is_our_board(gcomprisBoard)) {
	/* Great, we found our plugin */
	g_warning("We found the correct plugin for board %s (type=%s)\n", gcomprisBoard->name, gcomprisBoard->type);

	gcomprisBoard->plugin = bp;

	return TRUE;
      }
    }
  } else {
    while(static_boards_demo[i++] != NULL) {

      BoardPlugin *bp;

      /* Get the BoardPlugin Info */
      bp = (BoardPlugin *) static_boards_demo[i-1];

      if(bp->is_our_board(gcomprisBoard)) {
	/* Great, we found our plugin */
	g_warning("We found the correct plugin for board %s (type=%s)\n", gcomprisBoard->name, gcomprisBoard->type);

	gcomprisBoard->plugin = bp;

	return TRUE;
      }
    }
  }


  g_warning("No plugin library found for board type '%s', requested by '%s'", 
	    gcomprisBoard->type,  gcomprisBoard->filename);

  return FALSE;
}
#else
gboolean board_check_file(GcomprisBoard *gcomprisBoard)
{
  BoardPlugin *bp;
  GModule     *gmodule = NULL;
  gchar       *gmodule_file = NULL;
  BoardPlugin *(*plugin_get_bplugin_info) (void) = NULL;
  guint        i=0;

  g_assert(gcomprisBoard!=NULL);
  
  /* Check Already loaded */  
  if(gcomprisBoard->plugin!=NULL) {
    return TRUE;
  }

  while(plugin_paths[i] != NULL && gmodule == NULL) {
    gchar *sep;
    gchar *type = g_strdup(gcomprisBoard->type);

    /* Manage the python case where : is use to separate python plugin and boards */
    if(sep = strchr(type, ':'))
      *sep ='\0';

    gmodule_file = g_module_build_path (plugin_paths[i++], type);

    gmodule = g_module_open (gmodule_file, 0);
    if(gmodule) {
      g_warning("opened module %s with name %s\n",gmodule_file , type);
    } else {
      g_warning("Failed to open module %s with name %s (error=%s)\n",gmodule_file , type, g_module_error());
    }
    g_free(type);
  }

  if(gmodule != NULL) {

    g_module_symbol (gmodule, "get_bplugin_info", (gpointer) &plugin_get_bplugin_info);

    if(plugin_get_bplugin_info != NULL) {

      BoardPlugin *bp;

      /* Get the BoardPlugin Info */
      bp = (BoardPlugin *) plugin_get_bplugin_info();

      /* If this plugin defines an initialisation entry point, call it */
      if(bp->init != NULL) {
	bp->init();
      }

      if(bp->is_our_board(gcomprisBoard)) {
	/* Great, we found our plugin */
	g_warning("We found the correct plugin for board %s (type=%s)\n", gcomprisBoard->name, gcomprisBoard->type);

	gcomprisBoard->plugin = bp;

	/* Save this for caching and cleanup */
	gcomprisBoard->gmodule_file = gmodule_file;
	gcomprisBoard->gmodule      = gmodule;

	return TRUE;
      } else {
	g_warning("We found a plugin with the name %s but is_our_board() returned FALSE (type=%s)\n", 
		  gcomprisBoard->name,
		  gcomprisBoard->type);
      }
    } else {
      g_warning("plugin_get_bplugin_info entry point not found for %s\n", 
		gcomprisBoard->filename);
    }
  }
  g_warning("No plugin library found for board type '%s', requested by '%s'", 
	    gcomprisBoard->type,  gcomprisBoard->filename);

  return FALSE;
}
#endif

void board_play(GcomprisBoard *gcomprisBoard)
{
  BoardPlugin *bp;
  GModule     *gmodule = NULL;

  g_assert(gcomprisBoard!=NULL);

  board_check_file(gcomprisBoard);

  if(gcomprisBoard->plugin!=NULL)
    {
      /* Log the board start */
      gcompris_log_start(gcomprisBoard);

      bp = gcomprisBoard->plugin;
      set_current_gcompris_board(gcomprisBoard);
      bp->start_board(gcomprisBoard);
      bp_data->playing = TRUE;
      return;
    }

  /* We set the playing flag even if no boardplugin
     recognizes the board. This way we are sure it will be skipped. */
  bp_data->playing = TRUE;
}

void board_pause(void)
{
  if (get_board_playing() && get_current_board_plugin())
    {
      bp_data->paused = !bp_data->paused;
      get_current_board_plugin()->pause_board(bp_data->paused);
    }
}

void board_stop(void)
{
  /* If we are in the upper menu, no need to stop and restart it */
  if (get_current_gcompris_board()->previous_board == NULL)
    return;

  if (bp_data->playing && get_current_board_plugin())
    {
      bp_data->playing = FALSE;

      if (bp_data->paused)
	board_pause();
      if (get_current_board_plugin()->end_board)
	get_current_board_plugin()->end_board();

      bp_data->paused = FALSE;

      gcompris_end_board();
      return;
    }
  bp_data->playing = FALSE;
}

gboolean get_board_playing(void)
{
  return bp_data->playing;
}

gboolean get_board_paused(void)
{
  return bp_data->paused;
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */




