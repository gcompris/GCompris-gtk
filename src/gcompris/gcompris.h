/* gcompris - gcompris.h
 *
 * Time-stamp: <2002/02/23 02:19:03 bruno>
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
#include "score.h"

#define BOARDWIDTH  800
#define BOARDHEIGHT 520
#define BARHEIGHT   80


/*****************************************************************************/
/* Method usefull for boards provided by gcompris */

void		 gcompris_end_board();

/* Control Bar methods */
void		 gcompris_bar_start (GnomeCanvas *theCanvas);

GnomeCanvasItem *gcompris_set_background(GnomeCanvasGroup *parent, gchar *file);
void		 gcompris_bar_set_level (GcomprisBoard *gcomprisBoard);

/* Status bar control */
typedef enum
{
  GCOMPRIS_BAR_LEVEL	= 1 << 0,
  GCOMPRIS_BAR_OK	= 1 << 1,
  GCOMPRIS_BAR_REPEAT	= 1 << 2,
  GCOMPRIS_BAR_CONFIG	= 1 << 3,
  GCOMPRIS_BAR_ABOUT	= 1 << 4,
} GComprisBarFlags;

void		 gcompris_bar_set (const GComprisBarFlags flags);
void		 gcompris_bar_hide (gboolean hide);

/* Help window */
gboolean	 gcompris_board_has_help (GcomprisBoard *gcomprisBoard);
void		 gcompris_help_start (GcomprisBoard *gcomprisBoard);
void		 gcompris_help_stop ();

/* General */
GnomeCanvas     *gcompris_get_canvas();

gchar	        *gcompris_get_locale();
void		 gcompris_set_locale(gchar *locale);

void		 gcompris_set_cursor(guint gdk_cursor_type);

/*=========================================================*/
/* Some global definition to keep a constant look and feel */
/* Boards coders are invited to use them                   */
#define COLOR_TITLE		0x0F0FC000
#define COLOR_TEXT_BUTTON       0x0F0FC000
#define COLOR_CONTENT		0x0D0DFA00
#define COLOR_SUBTITLE		0xB0040000

#define FONT_TITLE		"-bitstream-charter-black-r-normal-*-*-320-*-*-p-*-iso8859-1"
#define FONT_TITLE_FALLBACK	"-adobe-times-medium-r-normal--*-240-*-*-*-*-*-*"
#define FONT_SUBTITLE		"-bitstream-charter-medium-r-normal-*-*-180-*-*-p-*-iso8859-1"
#define FONT_SUBTITLE_FALLBACK	"-adobe-times-medium-r-normal--*-180-*-*-*-*-*-*"
#define FONT_CONTENT		"-bitstream-charter-medium-r-normal-*-*-140-*-*-p-*-iso8859-1"
#define FONT_CONTENT_FALLBACK	"-adobe-times-medium-r-normal--*-140-*-*-*-*-*-*"

#define GCOMPRIS_DEFAULT_CURSOR	GDK_LEFT_PTR
#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
