/* gcompris - bonus.h
 *
 * Time-stamp: <2001/10/15 01:10:21 bruno>
 *
 * Copyright (C) 2001 Pascal Georges
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

#ifndef BONUS_H
#define BONUS_H

/* The time, in ms, between the click of the player, making a choice
 * and the moment the bonus appears. Only used when there is no thumb
 * in the board */
#define TIME_CLICK_TO_BONUS 800

/* BONUS_LAST is equals to the last available bonus type */

typedef enum
{
  GC_BONUS_RANDOM,
  GC_BONUS_SMILEY,
  GC_BONUS_FLOWER,
  GC_BONUS_TUX,
  GC_BONUS_GNU,
  GC_BONUS_LAST
} GCBonusList;

typedef enum
{
  GC_BOARD_FINISHED_RANDOM,
  GC_BOARD_FINISHED_TUXPLANE,
  GC_BOARD_FINISHED_TUXLOCO,
  GC_BOARD_FINISHED_TOOMANYERRORS,
  GC_BOARD_FINISHED_LAST
} GCBoardFinishedList;

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
void	 gc_bonus_end_display(GCBoardFinishedList);

#endif
