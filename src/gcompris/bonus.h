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

#include <ctype.h>
#include <math.h>
#include <assert.h>
#include "gcompris.h"

/* The time, in ms, between the click of the player, making a choice
		and the moment the bonus appears. Only used when there is no thumb 
    in the board */
#define TIME_CLICK_TO_BONUS 800

/* BONUS_LAST is equals to the last available bonus type */

typedef enum
{
  BONUS_RANDOM		= 0,
  BONUS_SMILEY		= 1,
  BONUS_FLOWER		= 2,
  BONUS_TUX		= 3,
  BONUS_GNU		= 4,
  BONUS_LAST		= 2
} BonusList;

typedef enum
{
  BOARD_FINISHED_RANDOM		= 0,
  BOARD_FINISHED_TUXPLANE	= 1,
  BOARD_FINISHED_TUXLOCO	= 2,
  BOARD_FINISHED_TOOMANYERRORS  = 3,
  BOARD_FINISHED_LAST		= 3
} BoardFinishedList;

void	 gcompris_display_bonus(int, int);
void	 board_finished(int);
void	 bonus_image(char *,int);
void	 end_bonus(void);

#endif
