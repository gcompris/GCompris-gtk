/* gcompris - bonus.h
 *
 * Copyright (C) 2001, 2008 Pascal Georges
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

#ifndef BONUS_H
#define BONUS_H

/* The time, in ms, between the click of the player, making a choice
 * and the moment the bonus appears.
 */
#define TIME_CLICK_TO_BONUS 800

/* BONUS_LAST is equals to the last available bonus type */

typedef enum
{
  GC_BONUS_RANDOM,
  GC_BONUS_SMILEY,
  GC_BONUS_FLOWER,
  GC_BONUS_TUX,
  GC_BONUS_GNU,
  GC_BONUS_LION,
  GC_BONUS_NOTE,
  GC_BONUS_LAST
} GCBonusList;

typedef enum
{
  GC_BOARD_LOOSE,
  GC_BOARD_WIN,
  GC_BOARD_DRAW,
  GC_BOARD_COMPLETED,
} GCBonusStatusList;

/*
 * Public BONUS API Entry
 * ----------------------
 */
void	 gc_bonus_display(GCBonusStatusList, GCBonusList);

#endif
