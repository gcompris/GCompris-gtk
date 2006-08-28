/* gcompris - gcompris-files.h
 *
 * Time-stamp: <2005/08/25 23:31:06 yves>
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

#ifndef _GOMPRIS_FILES_H_
#define _GOMPRIS_FILES_H_

#include "gcompris.h"

gchar                   *gcompris_get_user_dirname(GcomprisUser *user);

gchar                   *gc_profile_get_current_user_dirname();

gchar                   *gcompris_get_board_dirname(GcomprisBoard *board);

gchar                   *gcompris_get_current_board_dirname();

#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
