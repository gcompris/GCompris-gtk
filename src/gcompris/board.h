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
#ifndef BOARD_H
#define BOARD_H

/**
 * Handles plugins initialization
 *
 * WIN32 = Static module, else it's dynamic
 */
#if defined WIN32
#define GET_BPLUGIN_INFO(pluginname) \
  BoardPlugin \
  *get_##pluginname##_bplugin_info(void) \
  { \
    return &menu_bp; \
  } \
  BoardPlugin * _##pluginname##_menu_bp = &menu_bp;
#else
#define GET_BPLUGIN_INFO(pluginname) \
  BoardPlugin			     \
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

void		 init_plugins(void);
BoardPlugin	*gc_board_get_current_board_plugin(void);
void		 set_current_board_plugin(BoardPlugin * ip);
GcomprisBoard	*gc_board_get_current(void);
void		 gc_board_set_current(GcomprisBoard * gcomprisBoard);
gboolean	 gc_board_check_file(GcomprisBoard *gcomprisBoard);
void		 gc_board_play(GcomprisBoard *gcomprisBoard);
void		 gc_board_stop(void);
void		 gc_board_pause(int pause);
gchar		*board_get_info_text(void);

void		 board_set_info_text(gchar * text);
void		 board_file_info_box(gchar * filename);
void             board_run_next(GcomprisBoard *board);

#endif
