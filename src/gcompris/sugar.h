/* gcompris - sugar.h
 *
 * Copyright (C) 2010, Aleksey Lim
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

#ifndef SUGAR_H
#define SUGAR_H

#define GC_MIME_TYPE "application/x-gcompris"

void save_profile(GKeyFile*);
gboolean switch_board_jobject();
gboolean is_activity_board();

extern SugarActivity *activity;
extern SugarJobject *board_jobject;

typedef void (*Callback)(void);
typedef SugarJobject *(*GetJobjectCallback)(GcomprisBoard*);
typedef void (*SaveJobjectCallback)(SugarJobject*);

typedef struct
{
  Callback construct;
  Callback finalize;
  GetJobjectCallback get_jobject;
  SaveJobjectCallback save_jobject;
} Peer;

#endif
