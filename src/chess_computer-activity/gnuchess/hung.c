/* GNU Chess 5.0 - hung.c - hung piece evaluation code
   Copyright (c) 1999-2002 Free Software Foundation, Inc.

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
*/
/*
 *  Hung piece evaluation.
 */

#include <config.h>
#include <stdio.h>
#include "common.h"

int EvalHung(int side)
/****************************************************************************
 *
 *  Calculate the number of hung pieces for a side.
 *
 ****************************************************************************/
{
   BitBoard c, n, b, r, q;
   int xside;

   xside = 1 ^ side;
   hunged[side] = 0;

   /* Knight */
   n = (Ataks[xside][pawn] & board.b[side][knight]);
   n |= (Ataks[xside][0] & board.b[side][knight] & ~Ataks[side][0]);

   /* Bishop */
   b = (Ataks[xside][pawn] & board.b[side][bishop]);
   b |= (Ataks[xside][0] & board.b[side][bishop] & ~Ataks[side][0]);

   /* Rook */
   r = Ataks[xside][pawn] | Ataks[xside][knight] | Ataks[xside][bishop];
   r = (r & board.b[side][rook]);
   r |= (Ataks[xside][0] & board.b[side][rook] & ~Ataks[side][0]);

   /* Queen */
   q = Ataks[xside][pawn] | Ataks[xside][knight] | Ataks[xside][bishop] |
       Ataks[xside][rook];
   q = (q & board.b[side][queen]);
   q |= (Ataks[xside][0] & board.b[side][queen] & ~Ataks[side][0]);

   c = n | b | r | q ;

   if (c)
      hunged[side] += nbits (c);

   /* King */
   if (Ataks[xside][0] & board.b[side][king])
      hunged[side] ++;

   return (hunged[side]);
}
