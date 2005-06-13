/* gcompris - gcompris-board.h
 *
 * Time-stamp: <2005/06/13 21:39:18 yves>
 *
 * Copyright (C) 2000 Bruno Coudoin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

  /* The directory from which it was started */
  gchar		      *board_dir;

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
  gint16               width;
  gint16               height;
  GnomeCanvas         *canvas;

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

#ifdef USE_PROFILS
  /* database value */
  guint                board_id;
  guint                section_id;
#endif
};




#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
