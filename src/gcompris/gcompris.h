/* gcompris - gcompris.h
 *
 * Time-stamp: <2006/08/20 10:03:53 bruno>
 *
 * Copyright (C) 2000,2001,2002 Bruno Coudoin
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

#include <goocanvas.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <glib.h>
#include <libintl.h>

#include <gmodule.h>

#include "plugin.h"
#include "gcompris-board.h"
#include "board.h"
#include "board_config.h"
#include "properties.h"
#include "gameutil.h"
#include "gc_net.h"
#include "bonus.h"
#include "timer.h"
#include "score.h"
#include "skin.h"
#include "anim.h"

#include "profile.h"
#include "gcompris_db.h"
#include "wordlist.h"
#include "gcompris_im.h"
#include "gcompris_alphabeta.h"

#include "drag.h"

#ifdef USE_CAIRO
#include "gcompris-cairo.h"
#endif

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#define BOARDWIDTH  800
#define BOARDHEIGHT 520
#define BARHEIGHT   80

#define DEFAULT_SKIN "gartoon"

#define _(String) gettext (String)
#define gettext_noop(String) String

#ifndef N_
#define N_(String) gettext_noop (String)
#endif

/*****************************************************************************/
/* Method usefull for boards provided by gcompris */

void		 gc_board_end(void);

/* Control Bar methods */
void		 gc_bar_start (GooCanvas *theCanvas);

void		 gc_set_background(GooCanvasItem *parent, gchar *file);
void		 gc_bar_set_level (GcomprisBoard *gcomprisBoard);
void		 gc_bar_set_repeat_icon (GdkPixbuf *pixmap);

/* Status bar control */
typedef enum
{
  GC_BAR_LEVEL	   = 1 << 0,
  GC_BAR_OK	   = 1 << 1,
  GC_BAR_REPEAT	   = 1 << 2,
  GC_BAR_CONFIG	   = 1 << 3,
  GC_BAR_ABOUT	   = 1 << 4,
  GC_BAR_REPEAT_ICON = 1 << 5,
  GC_BAR_QUIT	   = 1 << 6,
} GComprisBarFlags;

/* Difficulty filtering */
typedef enum {
  GCOMPRIS_FILTER_NONE,
  GCOMPRIS_FILTER_EQUAL,
  GCOMPRIS_FILTER_UNDER,
  GCOMPRIS_FILTER_ABOVE,
} GcomprisFilterType;

void		 gc_bar_set (const GComprisBarFlags flags);
void		 gc_bar_hide (gboolean hide);

/* General */
GooCanvas     *gc_get_canvas(void);
GtkWidget	*gc_get_window(void);

const gchar	*gc_locale_get(void);
void		 gc_locale_set(gchar *locale);
char		*gc_locale_get_user_default(void);
gchar		*gc_locale_get_name(gchar *locale);

void		 gc_cursor_set(guint gdk_cursor_type);

typedef void     (*ImageSelectorCallBack)     (gchar* image);
void		 gc_selector_images_start (GcomprisBoard *gcomprisBoard,
						 gchar *dataset,
						 ImageSelectorCallBack imscb);
void		 gc_selector_images_stop (void);

typedef void     (*FileSelectorCallBack)     (gchar *file, gchar *file_type); /* file_type is one string from file_types in the save */
void		 gc_selector_file_load (GcomprisBoard *gcomprisBoard,
					      gchar *rootdir,
					      gchar *file_types, /* A Comma separated text explaining the different file types */
					      FileSelectorCallBack fscb);
void		 gc_selector_file_save (GcomprisBoard *gcomprisBoard,
					      gchar *rootdir,
					      gchar *file_types, /* A Comma separated text explaining the different file types */
					      FileSelectorCallBack fscb);
void		 gc_selector_file_stop (void);

gchar		*gc_db_get_filename();

/* Dialog box */
typedef void     (*DialogBoxCallBack)     ();
void		 gc_dialog(gchar *str, DialogBoxCallBack dbcb);
void		 gc_dialog_close();

/* Confirm box */
typedef void     (*ConfirmCallBack)     (gboolean answer);

void		 gc_confirm_box (gchar *title,
				 gchar *question_text,
				 gchar *yes_text,
				 gchar *no_text,
				 ConfirmCallBack iscb);

void		 gc_confirm_box_stop (void);

/* Use these instead of the goo_canvas ones for proper fullscreen mousegrab
   handling. */
int		 gc_canvas_item_grab (GooCanvasItem *item, unsigned int event_mask,
				      GdkCursor *cursor, guint32 etime);
void		 gc_canvas_item_ungrab (GooCanvasItem *item, guint32 etime);

/* Use it to tell the teacher where the kid failed */
void		 gc_log_set_comment (GcomprisBoard *gcomprisBoard, gchar *expected, gchar* got);

/* Do not use it if you use the bonus API in your board */
void		 gc_log_end (GcomprisBoard *gcomprisBoard, GCBonusStatusList status);

/* For menu type activity */
GList		*gc_menu_getlist(gchar *section);
GcomprisBoard   *gc_menu_section_get(gchar *section);
GList           *gc_menu_get_boards();

/*=========================================================*/
/* Some global definition to keep a constant look and feel */
/* Boards coders are invited to use them                   */
#define COLOR_TITLE		0x0F0FC0FF
#define COLOR_TEXT_BUTTON       0x0F0FC0FF
#define COLOR_CONTENT		0x0D0DFAFF
#define COLOR_SUBTITLE		0xB00400FF
#define COLOR_SHADOW		0x000000FF

#define FONT_TITLE		"Sans 20"
#define FONT_TITLE_FALLBACK	"Sans 12"
#define FONT_SUBTITLE		"Sans 16"
#define FONT_SUBTITLE_FALLBACK	"Sans 12"
#define FONT_CONTENT		"Sans 12"
#define FONT_CONTENT_FALLBACK	"Sans 12"

#define FONT_BOARD_TINY		"Sans 10"
#define FONT_BOARD_SMALL	"Sans 12"
#define FONT_BOARD_MEDIUM	"Sans 14"
#define FONT_BOARD_BIG		"Sans 16"
#define FONT_BOARD_BIG_BOLD	"Sans bold 16"
#define FONT_BOARD_FIXED	"Fixed 12"
#define FONT_BOARD_TITLE	"Sans 20"
#define FONT_BOARD_TITLE_BOLD	"Sans bold 20"
#define FONT_BOARD_HUGE		"Sans 28"
#define FONT_BOARD_HUGE_BOLD	"Sans bold 28"

/*=========================================================*/
// These are gcompris defined cursors
// cursor defines must be over the last gnome cursor defined in gdkcursors.h
#define GCOMPRIS_FIRST_CUSTOM_CURSOR	1000
#define GCOMPRIS_DEFAULT_CURSOR		1001
#define GCOMPRIS_LINE_CURSOR		1003
#define GCOMPRIS_FILLRECT_CURSOR	1004
#define GCOMPRIS_RECT_CURSOR		1005
#define GCOMPRIS_FILLCIRCLE_CURSOR	1006
#define GCOMPRIS_CIRCLE_CURSOR		1007
#define GCOMPRIS_DEL_CURSOR		1008
#define GCOMPRIS_FILL_CURSOR		1009
#define GCOMPRIS_SELECT_CURSOR		1010

#endif
