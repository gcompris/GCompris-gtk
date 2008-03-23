/* gcompris - plugin.h
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
#ifndef PLUGIN_H
#define PLUGIN_H

#include "profile.h"

/* Forward declaration of GcomprisBoard */
typedef struct _GcomprisBoard         GcomprisBoard;

/* Return 1 if the board plugin can handle this board */
typedef	gboolean      (*GcomprisIsOurBoard)   (GcomprisBoard *gcomprisBoard);

typedef void          (*GcomprisInitBoard)    (GcomprisBoard *gcomprisBoard);
typedef void          (*GcomprisStartBoard)   (GcomprisBoard *gcomprisBoard);
typedef void          (*GcomprisPauseBoard)   (gboolean pause);
typedef void          (*GcomprisEndBoard)     (void);
typedef gint          (*GcomprisKeyPress)     (guint keyval,
					       gchar *commited_str,
					       gchar *preedit_str);
typedef void          (*GcomprisOK)           (void);
typedef void          (*GcomprisSetLevel)     (guint level);
typedef void          (*GcomprisConfig)       ();
typedef void          (*GcomprisRepeat)       ();
typedef void          (*GcomprisConfigStart)  (GcomprisBoard *gcomprisBoard,
					       GcomprisProfile *profile);
typedef void          (*GcomprisConfigStop)    (void);

typedef struct
{
  void *handle;			/* Filled in by gcompris */
  char *filename;		/* Filled in by gcompris */
  char *name;			/* The name that describes this board */
  char *description;		/* The description that describes this board */
  char *author;			/* The author of this board */

  GcomprisInitBoard    init;	/* Called when the plugin is loaded */

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

  GcomprisConfigStart  config_start;
  GcomprisConfigStop   config_stop;

}
BoardPlugin;

/* Board plugin entry point */
BoardPlugin *get_bplugin_info(void);

#endif
