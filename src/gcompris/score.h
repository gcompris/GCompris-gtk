/* gcompris - bonus.h
 *
 * Copyright (C) 2001, 2008 Bruno Coudoin
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

#ifndef SCORE_H
#define SCORE_H

#include "gcompris.h"

typedef enum
{
  SCORESTYLE_NOTE,
  SCORESTYLE_LIFE,
} ScoreStyleList;

void gc_score_start (ScoreStyleList style, guint x, guint y, guint max);
void gc_score_end();
void gc_score_set_max(guint max);
void gc_score_set(guint value);

typedef struct
{
  void (*start) (ScoreStyleList style, guint x, guint y, guint max);
  void (*end) ();
  void (*set) (guint value);
  void (*set_max) (guint max);
} Score;

void gc_score_register (Score *score);

#endif
