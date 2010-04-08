/* GNU Chess 5.0 - ponder.c - Pondering
   Copyright (c) 2002 Free Software Foundation, Inc.

   GNU Chess is based on the two research programs 
   Cobalt by Chua Kong-Sian and Gazebo by Stuart Cracraft.

   GNU Chess is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GNU Chess is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Chess; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Contact Info: 
     bug-gnu-chess@gnu.org
     cracraft@ai.mit.edu, cracraft@stanfordalumni.org, cracraft@earthlink.net
     lukas@debian.org
*/

#include <config.h>
#include "common.h"
#include <stdlib.h>
#include <unistd.h>

/*
 * Pondering has to check for input_status == INPUT_NONE regularly,
 * that is what Iterate() is supposed to do when the PONDER flag is
 * set.  Output should only be generated if the xboard flag is
 * set. (Otherwise stdout gets messed up.)
 */
void ponder(void)
{
   /* Save flags and clear the time control bit for pondering */
   const unsigned int saved_flags = flags;

   /* Save number of moves out of book */
   const int nmfb = nmovesfrombook;

   CLEAR(flags, TIMECTL);
   SET(flags, PONDER);

   dbg_printf("Pondering, GameCnt = %d\n", GameCnt);
   Iterate();
   dbg_printf("Pondering ended, GameCnt = %d\n", GameCnt);

   /* Restore flags, will also clear the PONDER flag */
   flags=saved_flags;

   nmovesfrombook = nmfb;
}
