/*  GCompris -- This files comes from XMMS
 *
 *  Copyright (C) 1998-2000  Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies
 *  Copyright (C) 2008 Bruno Coudoin
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
#ifndef BOARD_H
#define BOARD_H

/**
 * Handles plugins initialization
 *
 * STATIC_MODULE = Static module, else it's dynamic
 */
#if defined STATIC_MODULE
#define GET_BPLUGIN_INFO(pluginname) \
  BoardPlugin \
  *get_##pluginname##_bplugin_info(void) \
  { \
    return &menu_bp; \
  } \
  BoardPlugin * _##pluginname##_menu_bp = &menu_bp;
#else

#ifdef BOARD_DLL_EXPORT
// the dll exports
#  define EXPORT __declspec(dllexport)
#else
#    define EXPORT
#endif

#define GET_BPLUGIN_INFO(pluginname) \
EXPORT  BoardPlugin			     \
  *get_bplugin_info(void) \
  { \
    return &menu_bp; \
  }
#endif

struct BoardPluginData
{
  GcomprisBoard	*current_gcompris_board;
  gboolean	 playing;
};

void		 gc_board_init(void);
BoardPlugin	*gc_board_get_current_board_plugin(void);
GcomprisBoard	*gc_board_get_current(void);
void		 gc_board_set_current(GcomprisBoard * gcomprisBoard);
gboolean	 gc_board_check_file(GcomprisBoard *gcomprisBoard);
void		 gc_board_play(GcomprisBoard *gcomprisBoard);
void		 gc_board_stop(void);
void		 gc_board_pause(int pause);

void             gc_board_run_next(GcomprisBoard *board);
guint		 gc_board_get_number_of_activity();
gboolean	 gc_board_is_demo_only(GcomprisBoard *gcomprisBoard);

#endif
