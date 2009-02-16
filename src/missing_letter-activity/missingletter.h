/* gcompris - missingletter.h
 *
 * Copyright (C) 2009 Bruno Coudoin
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

#ifndef _MISSINGLETTER_H_
#define _MISSINGLETTER_H_

extern GcomprisBoard *gcomprisBoard_missing;

gboolean              missing_read_xml_file(char *fname, GList**);
void                  missing_destroy_board_list(GList *);

#define MAX_PROPOSAL 6
typedef struct _Board {
  gchar *pixmapfile;
  gchar *question;
  gchar *answer;
  gchar *text[MAX_PROPOSAL + 1];
  guint solution;
} Board;

#endif
