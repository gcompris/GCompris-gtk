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

struct BoardPluginData *bp_data;


BoardPlugin *get_current_board_plugin(void)
{
  return bp_data->current_board_plugin;
}

void set_current_board_plugin(BoardPlugin * bp)
{
  bp_data->current_board_plugin = bp;
}

GcomprisBoard *get_current_gcompris_board(void)
{
  return bp_data->current_gcompris_board;
}

void set_current_gcompris_board(GcomprisBoard * gcomprisBoard)
{
  bp_data->current_gcompris_board = gcomprisBoard;
}

GList *get_board_list(void)
{
  return bp_data->board_list;
}
	
gboolean board_check_file(GcomprisBoard *gcomprisBoard)
{
  GList *node;
  BoardPlugin *bp;

  node = get_board_list();
  while (node)
    {
      bp = (BoardPlugin *) node->data;
      if (bp && bp->is_our_board(gcomprisBoard))
	return TRUE;
      node = node->next;
    }
  return FALSE;
}

void board_play(GcomprisBoard *gcomprisBoard)
{
  GList *node;
  BoardPlugin *bp;

  node = get_board_list();
  while (node)
    {
      bp = (BoardPlugin *) node->data;
      if (bp && bp->is_our_board(gcomprisBoard))
	{
	  set_current_board_plugin(bp);
	  set_current_gcompris_board(gcomprisBoard);
	  bp->start_board(gcomprisBoard);
	  bp_data->playing = TRUE;
	  return;

	}
      node = node->next;
    }
  /* We set the playing flag even if no boardplugin
     recognizes the board. This way we are sure it will be skipped. */
  bp_data->playing = TRUE;
  set_current_board_plugin(NULL);
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

void board_about(gint index)
{
  BoardPlugin *bp;

  bp = g_list_nth(bp_data->board_list, index)->data;
  if (bp && bp->about)
    bp->about();
}

void board_configure(gint index)
{
  BoardPlugin *bp;

  bp = g_list_nth(bp_data->board_list, index)->data;
  if (bp && bp->configure)
    bp->configure();
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */




