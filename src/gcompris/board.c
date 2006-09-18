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

#include <string.h>

#include "gcompris.h"
#include "gc_core.h"


static struct BoardPluginData *bp_data;

static gboolean	 get_board_playing(void);

#if defined _WIN32 || defined __WIN32__
# undef WIN32   /* avoid warning on mingw32 */
# define WIN32
#endif


#ifdef WIN32
extern BoardPlugin * get_advanced_colors_bplugin_info();
extern BoardPlugin * get_algebra_bplugin_info();
extern BoardPlugin * get_algebra_guesscount_bplugin_info();
extern BoardPlugin * get_awele_bplugin_info();
extern BoardPlugin * get_canal_lock_bplugin_info();
extern BoardPlugin * get_chess_bplugin_info();
extern BoardPlugin * get_clickgame_bplugin_info();
extern BoardPlugin * get_click_on_letter_bplugin_info();
extern BoardPlugin * get_clockgame_bplugin_info();
extern BoardPlugin * get_colors_bplugin_info();
extern BoardPlugin * get_draw_bplugin_info();
extern BoardPlugin * get_enumerate_bplugin_info();
extern BoardPlugin * get_erase_bplugin_info();
extern BoardPlugin * get_fifteen_bplugin_info();
extern BoardPlugin * get_gletters_bplugin_info();
extern BoardPlugin * get_gtans_bplugin_info();
extern BoardPlugin * get_hanoi_bplugin_info();
extern BoardPlugin * get_hanoi_real_bplugin_info();
extern BoardPlugin * get_imageid_bplugin_info();
extern BoardPlugin * get_leftright_bplugin_info();
extern BoardPlugin * get_machpuzzle_bplugin_info();
extern BoardPlugin * get_magic_hat_bplugin_info();
extern BoardPlugin * get_maze_bplugin_info();
extern BoardPlugin * get_memory_bplugin_info();
extern BoardPlugin * get_menu_bplugin_info();
extern BoardPlugin * get_missingletter_bplugin_info();
extern BoardPlugin * get_money_bplugin_info();
extern BoardPlugin * get_paratrooper_bplugin_info();
extern BoardPlugin * get_planegame_bplugin_info();
extern BoardPlugin * get_railroad_bplugin_info();
extern BoardPlugin * get_read_colors_bplugin_info();
extern BoardPlugin * get_reading_bplugin_info();
extern BoardPlugin * get_reversecount_bplugin_info();
extern BoardPlugin * get_shapegame_bplugin_info();
extern BoardPlugin * get_smallnumbers_bplugin_info();
extern BoardPlugin * get_submarine_bplugin_info();
extern BoardPlugin * get_superbrain_bplugin_info();
extern BoardPlugin * get_target_bplugin_info();
extern BoardPlugin * get_traffic_bplugin_info();
extern BoardPlugin * get_wordsgame_bplugin_info();

#define MAX_NUMBER_OF_BOARDS 200
static BoardPlugin *static_boards_demo[MAX_NUMBER_OF_BOARDS];
static BoardPlugin *static_boards[MAX_NUMBER_OF_BOARDS];

#endif

#ifdef WIN32
void init_plugins(void)
{
  guint i=0;

  bp_data = g_malloc0(sizeof (struct BoardPluginData));

  static_boards_demo[i++] = get_awele_bplugin_info();
  static_boards_demo[i++] = get_click_on_letter_bplugin_info();
  static_boards_demo[i++] = get_clickgame_bplugin_info();
  static_boards_demo[i++] = get_colors_bplugin_info();
  static_boards_demo[i++] = get_draw_bplugin_info();
  static_boards_demo[i++] = get_enumerate_bplugin_info();
  static_boards_demo[i++] = get_erase_bplugin_info();
  static_boards_demo[i++] = get_gletters_bplugin_info();
  static_boards_demo[i++] = get_gtans_bplugin_info();
  static_boards_demo[i++] = get_hanoi_bplugin_info();
  static_boards_demo[i++] = get_magic_hat_bplugin_info();
  static_boards_demo[i++] = get_menu_bplugin_info();
  static_boards_demo[i++] = get_reading_bplugin_info();
  static_boards_demo[i++] = get_submarine_bplugin_info();
  static_boards_demo[i++] = get_superbrain_bplugin_info();
  static_boards_demo[i++] = get_target_bplugin_info();
  static_boards_demo[i++] = NULL;

  i=0;

  static_boards[i++] = get_awele_bplugin_info();
  static_boards[i++] = get_advanced_colors_bplugin_info();
  static_boards[i++] = get_algebra_bplugin_info();
  static_boards[i++] = get_algebra_bplugin_info();
  static_boards[i++] = get_algebra_guesscount_bplugin_info();
  static_boards[i++] = get_canal_lock_bplugin_info();
  static_boards[i++] = get_chess_bplugin_info();
  static_boards[i++] = get_click_on_letter_bplugin_info();
  static_boards[i++] = get_clickgame_bplugin_info();
  static_boards[i++] = get_clockgame_bplugin_info();
  static_boards[i++] = get_colors_bplugin_info();
  static_boards[i++] = get_draw_bplugin_info();
  static_boards[i++] = get_enumerate_bplugin_info();
  static_boards[i++] = get_erase_bplugin_info();
  static_boards[i++] = get_fifteen_bplugin_info();
  static_boards[i++] = get_gletters_bplugin_info();
  static_boards[i++] = get_gtans_bplugin_info();
  static_boards[i++] = get_hanoi_bplugin_info();
  static_boards[i++] = get_hanoi_real_bplugin_info();
  static_boards[i++] = get_imageid_bplugin_info();
  static_boards[i++] = get_leftright_bplugin_info();
  static_boards[i++] = get_machpuzzle_bplugin_info();
  static_boards[i++] = get_magic_hat_bplugin_info();
  static_boards[i++] = get_maze_bplugin_info();
  static_boards[i++] = get_memory_bplugin_info();
  static_boards[i++] = get_menu_bplugin_info();
  static_boards[i++] = get_missingletter_bplugin_info();
  static_boards[i++] = get_money_bplugin_info();
  static_boards[i++] = get_paratrooper_bplugin_info();
  static_boards[i++] = get_planegame_bplugin_info();
  static_boards[i++] = get_railroad_bplugin_info();
  static_boards[i++] = get_read_colors_bplugin_info();
  static_boards[i++] = get_reading_bplugin_info();
  static_boards[i++] = get_reversecount_bplugin_info();
  static_boards[i++] = get_shapegame_bplugin_info();
  static_boards[i++] = get_smallnumbers_bplugin_info();
  static_boards[i++] = get_submarine_bplugin_info();
  static_boards[i++] = get_superbrain_bplugin_info();
  static_boards[i++] = get_target_bplugin_info();
  static_boards[i++] = get_traffic_bplugin_info();
  static_boards[i++] = get_wordsgame_bplugin_info();
  static_boards[i++] = NULL;

  i=0;
  while(static_boards[i] != NULL) {
    /* If this plugin defines an initialisation entry point, call it */
    BoardPlugin *bp;

    /* Get the BoardPlugin Info */
    bp = (BoardPlugin *) static_boards[i++];

    if(bp->init != NULL) {
      bp->init(NULL);
    }
  }
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

BoardPlugin *gc_board_get_current_board_plugin(void)
{
  if(bp_data->current_gcompris_board)
    return bp_data->current_gcompris_board->plugin;

  return NULL;
}

GcomprisBoard *gc_board_get_current(void)
{
  return bp_data->current_gcompris_board;
}

void gc_board_set_current(GcomprisBoard * gcomprisBoard)
{
  bp_data->current_gcompris_board = gcomprisBoard;
}

#ifdef WIN32
gboolean
gc_board_check_file(GcomprisBoard *gcomprisBoard)
{
  GcomprisProperties	*properties = gc_prop_get();
  BoardPlugin *bp;
  guint        i=0;

  g_assert(gcomprisBoard!=NULL);
  g_assert(properties->key!=NULL);

  /* Check Already loaded */
  if(gcomprisBoard->plugin!=NULL) {
    return TRUE;
  }

  if(strncmp(properties->key, "thanks_for_your_help", 20)==0) {
    while(static_boards[i++] != NULL) {
      BoardPlugin *bp;

      /* Get the BoardPlugin Info */
      bp = (BoardPlugin *) static_boards[i-1];

      if(bp->is_our_board(gcomprisBoard)) {
	/* Great, we found our plugin */
	g_warning("We found the correct plugin for board %s (type=%s)\n",
		  gcomprisBoard->name, gcomprisBoard->type);

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
	g_warning("We found the correct plugin for board %s (type=%s)\n",
		  gcomprisBoard->name, gcomprisBoard->type);

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
gboolean
gc_board_check_file(GcomprisBoard *gcomprisBoard)
{
  GModule     *gmodule = NULL;
  gchar       *gmodule_file = NULL;
  BoardPlugin *(*plugin_get_bplugin_info) (void) = NULL;
  GcomprisProperties *properties = gc_prop_get();
  gchar *sep;
  gchar *type;

  g_assert(gcomprisBoard!=NULL);

  type = g_strdup(gcomprisBoard->type);

  /* Check Already loaded */
  if(gcomprisBoard->plugin!=NULL) {
    return TRUE;
  }

  /* Manage the python case where : is use to separate python plugin and boards */
  if((sep = strchr(type, ':')))
    *sep ='\0';

  gmodule_file = g_module_build_path (properties->package_plugin_dir, type);

  gmodule = g_module_open (gmodule_file, 0);
  if(gmodule) {
    g_warning("opened module %s with name %s\n", gmodule_file, type);
  } else {
    g_warning("Failed to open module %s with name %s (error=%s)\n", gmodule_file,
	      type, g_module_error());
  }
  g_free(type);

  if(gmodule != NULL) {

    g_module_symbol (gmodule, "get_bplugin_info", (gpointer) &plugin_get_bplugin_info);

    if(plugin_get_bplugin_info != NULL) {

      BoardPlugin *bp;

      /* Get the BoardPlugin Info */
      bp = (BoardPlugin *) plugin_get_bplugin_info();

      /* If this plugin defines an initialisation entry point, call it */
      if(bp->init != NULL) {
	bp->init(gcomprisBoard);
      }

      if(bp->is_our_board(gcomprisBoard)) {
	/* Great, we found our plugin */
	g_warning("We found the correct plugin for board %s (type=%s)\n", gcomprisBoard->name, gcomprisBoard->type);

	/* done in is_our_board gcomprisBoard->plugin = bp; */

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

void
gc_board_play(GcomprisBoard *gcomprisBoard)
{
  BoardPlugin *bp;

  g_assert(gcomprisBoard!=NULL);

  gc_board_check_file(gcomprisBoard);

  if(gcomprisBoard->plugin!=NULL)
    {
      /* Log the board start */
      gc_log_start(gcomprisBoard);

      bp = gcomprisBoard->plugin;
      gc_board_set_current(gcomprisBoard);

      bp->start_board(gcomprisBoard);
      bp_data->playing = TRUE;

      return;
    }

  /* We set the playing flag even if no boardplugin
     recognizes the board. This way we are sure it will be skipped. */
  bp_data->playing = TRUE;
}

void
gc_board_pause(int pause)
{
  if (get_board_playing() && gc_board_get_current_board_plugin())
    {
      if(gc_board_get_current_board_plugin()->pause_board)
	gc_board_get_current_board_plugin()->pause_board(pause);
    }
}

void
gc_board_stop(void)
{
  if (bp_data->playing && gc_board_get_current_board_plugin())
    {
      bp_data->playing = FALSE;

      if (gc_board_get_current_board_plugin()->end_board)
	gc_board_get_current_board_plugin()->end_board();

      gc_board_end();

      return;
    }
  bp_data->playing = FALSE;
}

static gboolean
get_board_playing(void)
{
  return bp_data->playing;
}

static GcomprisBoard *next_board = NULL;
static gint next_board_callback_id = 0;
#define NEXT_TIME_DELAY 10

void board_run_next_end()
{
  gtk_timeout_remove(next_board_callback_id);
  next_board_callback_id = 0;

  if (next_board &&
      next_board->previous_board &&
      next_board->previous_board->plugin->end_board)
    next_board->previous_board->plugin->end_board();

  /* reset the im context */
  gc_im_reset();

  /*run the board */
  gc_board_play(next_board);
}

void board_run_next(GcomprisBoard *board)
{
  board->previous_board = gc_board_get_current();

  next_board = board;

  next_board_callback_id = gtk_timeout_add (NEXT_TIME_DELAY,
					    (GtkFunction) board_run_next_end,
					    NULL);

}
