/* gcompris - gcompris-board.h
 *
 * Copyright (C) 2000, 2008 Bruno Coudoin
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
#ifndef TYPES_H
#define TYPES_H



/*****************************************************************************/
/* The following structure describes a board object.  */

struct _GcomprisBoard
{
  /* Board Type */
  gchar               *type;
  gboolean	       board_ready;

  /* Board specific mode */
  gchar		      *mode;

  /* Name of this board */
  gchar               *name;
  gchar               *title;
  gchar               *description;
  gchar               *icon_name;
  gchar		      *author;
  gchar		      *boarddir;
  gchar		      *filename;
  gchar		      *difficulty;
  gchar		      *mandatory_sound_file;
  gchar		      *mandatory_sound_dataset;

  /* Menu positionning */
  gchar		      *section;
  gchar		      *menuposition;

  /* Help information */
  gchar		      *prerequisite;
  gchar		      *goal;
  gchar		      *manual;
  gchar		      *credit;

  /* Size of the window in which we have to run */
  GooCanvas         *canvas;

  /* Pointors to boards's own functions */
  BoardPlugin         *plugin;

  /* The gmodule pointers */
  GModule	      *gmodule;
  gchar		      *gmodule_file;


  /* Pointors to the previous board to start when this one ends */
  GcomprisBoard       *previous_board;

  /* Dynamic board information (status) */
  guint                level;
  guint                maxlevel;
  guint                sublevel;
  guint                number_of_sublevel;

  /* database value */
  guint                board_id;
  guint                section_id;

  /* IM_context control */
  gboolean             disable_im_context;

  /* This activity is a demo one */
  gboolean	       demo;
};




#endif
