/* gcompris - gcompris.h
 *
 * Time-stamp: <2001/11/11 23:13:57 bruno>
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

#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include <glib.h>

#include <config.h>
#include <gmodule.h>

#include "plugin.h"
#include "gcompris-board.h"
#include "board.h"
#include "properties.h"
#include "gameutil.h"
#include "pluginenum.h"
#include "bonus.h"

#define BOARDWIDTH  800
#define BOARDHEIGHT 600
#define BARHEIGHT   80


/*****************************************************************************/
/* Method usefull for boards provided by gcompris */

void		 gcompris_end_board();

/* Control Bar methods */
void		 gcompris_bar_start (GnomeCanvas *theCanvas, GtkWidget *theStatusbar);

GnomeCanvasItem *gcompris_set_background(GnomeCanvasGroup *parent, gchar *file);
void		 gcompris_bar_set_level (GcomprisBoard *gcomprisBoard);
void		 gcompris_bar_set_timer (guint time);
void		 gcompris_bar_set_maxtimer (guint time);
void		 gcompris_bar_set_message (const gchar *message);

/* Status bar control */
typedef enum
{
  GCOMPRIS_BAR_LEVEL	= 1 << 0,
  GCOMPRIS_BAR_OK	= 1 << 1,
  GCOMPRIS_BAR_HELP	= 1 << 2,
  GCOMPRIS_BAR_REPEAT	= 1 << 3,
} GComprisBarFlags;

void		 gcompris_bar_set (const GComprisBarFlags flags);

/* Help window */
void		 gcompris_help_start (gchar *title, gchar *content);
void		 gcompris_help_stop ();

/* General */
GnomeCanvas     *gcompris_get_canvas();

gchar	        *gcompris_get_locale();
void		 gcompris_set_locale(gchar *locale);

#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
