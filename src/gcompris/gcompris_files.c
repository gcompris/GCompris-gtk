/* gcompris - gcompris-files.c
 *
 * Time-stamp: <2005/08/25 23:33:24 yves>
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

#include "gcompris_files.h"

gchar                   *gcompris_get_user_dirname(GcomprisUser *user)
{
  GcomprisProperties	*properties = gcompris_get_properties ();

  gchar *user_dirname = g_strconcat (properties->users_dir, 
				     "/", 
				     user->login, 
				     NULL);

  create_rootdir(user_dirname);

  return user_dirname;
}

gchar                   *gcompris_get_current_user_dirname()
{
  return gcompris_get_user_dirname(gcompris_get_current_user());
}

gchar                   *gcompris_get_board_dirname(GcomprisBoard *board)
{
  GcomprisProperties	*properties = gcompris_get_properties ();

  gchar *board_main = g_strconcat (properties->shared_dir, "/boards", NULL);
  create_rootdir(board_main);

  gchar *board_dirname = g_strconcat (board_main, "/", board->name, NULL);
  create_rootdir(board_dirname);

  g_free(board_main);
  return board_dirname;
}

gchar                   *gcompris_get_current_board_dirname()
{
  return gcompris_get_board_dirname(get_current_gcompris_board());
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
