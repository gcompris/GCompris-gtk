/* gcompris - animtest.c
 *
 * Copyright (C) 2005 Joe Neeman
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

#include "gcompris/gcompris.h"

#define CENTER_LEFT_X 185
#define CENTER_LEFT_Y 430

static GcomprisBoard *board;
static gboolean paused = TRUE;

static void start_board(GcomprisBoard*);
static void end_board();
static gboolean is_our_board(GcomprisBoard*);

/*=============================================*/
static GcomprisAnimation *animation;
static GnomeCanvasItem *anim_item;

static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Blah",
    "Blah",
    "Me",
    NULL,
    NULL,
    NULL,
    NULL,
    start_board,
    NULL,
    end_board,
    is_our_board,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
  };

BoardPlugin *get_bplugin_info()
{
    return &menu_bp;
}

static void start_board(GcomprisBoard *b)
{
  if(b != NULL)
    {
      board = b;
      gcompris_set_background(gnome_canvas_root(board->canvas), "leftright/leftright-bg.jpg");
      board->level = 1;
      board->maxlevel=1;
      board->sublevel=1;
      board->number_of_sublevel=1;
      gcompris_score_start(SCORESTYLE_NOTE, 10, 50, board->number_of_sublevel);
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);
      gcompris_bar_set_level(board);
      gcompris_score_set(board->sublevel);

      animation = gcompris_load_animation( "animtest/test.txt" );
      anim_item = (GnomeCanvasItem*)
                  gcompris_activate_animation( gnome_canvas_root(board->canvas),
                                               animation );
      gnome_canvas_item_set(anim_item,
            "x", (double) CENTER_LEFT_X,
            "y", (double) CENTER_LEFT_Y,
            "anchor", GTK_ANCHOR_CENTER,
            "width", (double)128,
            "height", (double)128,
            "width_set", TRUE,
            "height_set", TRUE,
            NULL);
    }
}

static void end_board()
{
  if(board != NULL)
    {
      gcompris_deactivate_animation(anim_item);
      gcompris_free_animation(animation);
    }
}

static gboolean is_our_board(GcomprisBoard *b)
{
  if(b)
    {
      if(g_strcasecmp(b->type, "animtest") == 0)
        {
          b->plugin = &menu_bp;
          return TRUE;
        }
    }
  return FALSE;
}
