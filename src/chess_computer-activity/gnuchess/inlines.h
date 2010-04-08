/* GNU Chess 5.0 - inlines.h - static inline functions

   This file constains static inline version of performance-critical
   functions. Is included by common.h unless NO_INLINE is defined.

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
     lukas@debian.org
*/

#ifndef INLINES_H
#define INLINES_H

static inline unsigned char leadz (BitBoard b)
/**************************************************************************
 *
 *  Returns the leading bit in a bitboard.  Leftmost bit is 0 and
 *  rightmost bit is 63.  Thanks to Robert Hyatt for this algorithm.
 *
 ***************************************************************************/
{
  if (b >> 48) return lzArray[b >> 48];
  if (b >> 32) return lzArray[b >> 32] + 16;
  if (b >> 16) return lzArray[b >> 16] + 32;
  return lzArray[b] + 48;
}


static inline unsigned char nbits (BitBoard b)
/***************************************************************************
 *
 *  Count the number of bits in b.
 *
 ***************************************************************************/
{
  return BitCount[b>>48] + BitCount[(b>>32) & 0xffff]
    + BitCount[(b>>16) & 0xffff] + BitCount[b & 0xffff];
}

#endif /* !INLINES_H */
