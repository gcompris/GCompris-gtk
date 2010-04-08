/* GNU Chess 5.0 - hash.c - hash function code
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

#include <config.h>
#include <stdio.h>
#include "common.h"


void CalcHashKey (void)
/***************************************************************************
 *
 *  Calculates the hashkey for the current board position.  We sometimes
 *  need to do this especially when loading an EPD  position.
 *  Note:  The hashkey is a 64 bit unsigned integer number.  
 *  Added in pawnhashkey calculation.
 *
 ***************************************************************************/
{
   int sq, piece, color;
   BitBoard b;

   PawnHashKey = HashKey = (HashType) 0;
   for (color = white; color <= black; color++)
   {
      for (piece = pawn; piece <= king; piece++)
      {
	 b = board.b[color][piece];
	 while (b)
	 {
	    sq = leadz (b);
  	    CLEARBIT (b, sq);
	    HashKey ^= hashcode[color][piece][sq];
	    if (piece == pawn)
	       PawnHashKey ^= hashcode[color][piece][sq];
	 }
      }
   }

   /* Take into account castling status & en passant */
   if (board.ep > -1)
      HashKey ^= ephash[board.ep];
   if (board.flag & WKINGCASTLE)
      HashKey ^= WKCastlehash;
   if (board.flag & WQUEENCASTLE)
      HashKey ^= WQCastlehash;
   if (board.flag & BKINGCASTLE)
      HashKey ^= BKCastlehash;
   if (board.flag & BQUEENCASTLE)
      HashKey ^= BQCastlehash;

   /* Take into account side to move */
   if (board.side == black)
      HashKey ^= Sidehash;

}


void ShowHashKey (HashType HashKey)
/***************************************************************************
 *
 *  Just a small utility routine to print out the hashkey (which is 64 bits).
 *
 ***************************************************************************/
{
   unsigned long a1, a2;

   a1 = HashKey & 0xFFFFFFFF;
   a2 = (HashKey >> 32);
   printf ("Hashkey = %lx%lx\n", a2, a1);
}

void CalcHashSize (int tablesize)
/***************************************************************************
 *
 *  Calculates the ttable hashtable size, ttable hashmask, and pawntable hashmask
 *
 ***************************************************************************/
{
   int i;

   i = (tablesize < HASHSLOTS ? HASHSLOTS : tablesize);

   if ( i > 1073741823 ){
	   printf("Max hashsize exceeded\n");
	   i = 107374183;
   }

   TTHashMask = 0;
   while ((i>>=1) > 0)
   {
      TTHashMask <<= 1;
      TTHashMask |= 1;
   }
   HashSize = TTHashMask + 1;
   printf ("Adjusting HashSize to %d slots\n", HashSize);

   i = PAWNSLOTS;
   PHashMask = 0;
   while ((i>>=1) > 0)
   {
      PHashMask <<= 1;
      PHashMask |= 1;
   }
}
