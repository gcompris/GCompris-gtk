/* gcompris - plugin.h
 *
 * Time-stamp: <2002/01/11 00:29:06 bruno>
 *
 * Copyright (C) 2000 Bruno Coudoin
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
#ifndef PLUGIN_H
#define PLUGIN_H

/* Forward declaration of GcomprisBoard */
typedef struct _GcomprisBoard         GcomprisBoard;

/* Return 1 if the board plugin can handle this board */
typedef	gboolean      (*GcomprisIsOurBoard)   (GcomprisBoard *gcomprisBoard);

typedef void          (*GcomprisStartBoard)   (GcomprisBoard *gcomprisBoard);
typedef void          (*GcomprisPauseBoard)   (gboolean pause);
typedef void          (*GcomprisEndBoard)     (void);
typedef gint          (*GcomprisKeyPress)     (guint keyval);
typedef void          (*GcomprisOK)           (void);
typedef void          (*GcomprisSetLevel)     (guint level);
typedef void          (*GcomprisConfig)       ();
typedef void          (*GcomprisRepeat)       ();

typedef struct
{
  void *handle;			/* Filled in by gcompris */
  char *filename;		/* Filled in by gcompris */
  char *name;			/* The name that describes this board */
  char *description;		/* The description that describes this board */
  char *author;			/* The author of this board */
  char *prerequisite;		/* Help information */
  char *goal;
  char *manual;

  void (*init) (void);		/* Called when the plugin is loaded */
  void (*cleanup) (void);      	/* Called when gcompris exit */
  void (*about) (void);		/* Show the about box */
  void (*configure) (void);	/* Show the configuration dialog */
  
  GcomprisStartBoard   start_board;
  GcomprisPauseBoard   pause_board;
  GcomprisEndBoard     end_board;

  GcomprisIsOurBoard   is_our_board; /* Return 1 if the plugin can handle the board file */

  GcomprisKeyPress     key_press;
  GcomprisOK           ok;
  GcomprisSetLevel     set_level;
  GcomprisConfig       config;
  GcomprisRepeat       repeat;

}
BoardPlugin;

/* Board plugin entry point */
BoardPlugin *get_bplugin_info(void);

#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
