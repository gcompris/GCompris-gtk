/* GNU Chess 5.0 - util.c - utility routine code
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
 * Endianness checks are now unnecessary
 */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "common.h"

/*
 * We actually do not have this configuration variable yet,
 * should be manually added when inlining is not possible.
 * If inlining is possible, these functions have now moved
 * to util.h which must be included by all callers of
 * leadz() and nbits().
 */

#ifdef NO_INLINE

unsigned char leadz (BitBoard b)
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

unsigned char nbits (BitBoard b)
/***************************************************************************
 *
 *  Count the number of bits in b.
 *
 ***************************************************************************/
{
  return BitCount[b>>48] + BitCount[(b>>32) & 0xffff]
    + BitCount[(b>>16) & 0xffff] + BitCount[b & 0xffff];
}

#endif /* NO_INLINE */

void UpdateFriends (void)
/***************************************************************************
 *
 *  Update friend and enemy bitboard.
 *
 ***************************************************************************/
{
   register BitBoard *w, *b;

   w = board.b[white];
   b = board.b[black];
   board.friends[white] = 
      w[pawn] | w[knight] | w[bishop] | w[rook] | w[queen] | w[king];
   board.friends[black] = 
      b[pawn] | b[knight] | b[bishop] | b[rook] | b[queen] | b[king];
   board.blocker = board.friends[white] | board.friends[black];
}
   

void UpdateCBoard (void)
/**************************************************************************
 *
 *  Updates cboard[].  cboard[i] returns the piece on square i.
 *
 **************************************************************************/
{
   BitBoard b;
   int piece, sq;

   memset (cboard, 0, sizeof (cboard));
   for (piece = pawn; piece <= king; piece++)
   {
      b = board.b[white][piece] | board.b[black][piece];
      while (b)
      {
         sq = leadz (b);
         CLEARBIT (b, sq);
         cboard[sq] = piece;
      }
   }
}


static const int OrigCboard[64] = 
{ rook,  knight, bishop, queen, king,  bishop, knight, rook,
  pawn,  pawn,   pawn,   pawn,  pawn,  pawn,   pawn,   pawn,
  empty, empty,  empty,  empty, empty, empty,  empty,  empty,
  empty, empty,  empty,  empty, empty, empty,  empty,  empty,
  empty, empty,  empty,  empty, empty, empty,  empty,  empty,
  empty, empty,  empty,  empty, empty, empty,  empty,  empty,
  pawn,  pawn,   pawn,   pawn,  pawn,  pawn,   pawn,   pawn,
  rook,  knight, bishop, queen, king,  bishop, knight, rook };

void UpdateMvboard (void)
/**************************************************************************
 *
 *  Updates Mvboard[].  Mvboard[i] returns the number of times the piece
 *  on square i have moved.  When loading from EPD, if a piece is not on
 *  its original square, set it to 1, otherwise set to 0.
 *
 **************************************************************************/
{
   int sq;
 
   for (sq = 0; sq < 64; sq++)
   {
      if (cboard[sq] == empty || cboard[sq] == OrigCboard[sq])
         Mvboard[sq] = 0;
      else
         Mvboard[sq] = 1;
   } 
}


void EndSearch (int sig __attribute__ ((unused)) )
/***************************************************************************
 *
 *  User has pressed Ctrl-C.  Just set flags TIMEOUT to be true.
 *
 ***************************************************************************/
{
   SET (flags, TIMEOUT);
   signal (SIGINT, EndSearch);
}


short ValidateBoard (void)
/***************************************************************************
 *
 *  Check the board to make sure that its valid.  Some things to check are
 *  a.  Both sides have only 1 king.
 *  b.  Side not on the move must not be in check.
 *  c.  If en passant square is set, check it is possible.
 *  d.  Check if castling status are all correct.
 *
 ***************************************************************************/
{
   int side, xside, sq;

   if (nbits (board.b[white][king]) != 1) 
      return (false);
   if (nbits (board.b[black][king]) != 1) 
      return (false);

   side = board.side;  
   xside = 1^side;
   if (SqAtakd (board.king[xside], side))
      return (false);

   if (board.ep > -1)
   {
      sq = board.ep + (xside == white ? 8 : -8);
      if (!(BitPosArray[sq] & board.b[xside][pawn]))
	 return (false);
   }

   if (board.flag & WKINGCASTLE)
   {
      if (!(BitPosArray[E1] & board.b[white][king]))
	 return (false);
      if (!(BitPosArray[H1] & board.b[white][rook]))
	 return (false);
   }
   if (board.flag & WQUEENCASTLE)
   {
      if (!(BitPosArray[E1] & board.b[white][king]))
	 return (false);
      if (!(BitPosArray[A1] & board.b[white][rook]))
	 return (false);
   }
   if (board.flag & BKINGCASTLE)
   {
      if (!(BitPosArray[E8] & board.b[black][king]))
	 return (false);
      if (!(BitPosArray[H8] & board.b[black][rook]))
	 return (false);
   }
   if (board.flag & BQUEENCASTLE)
   {
      if (!(BitPosArray[E8] & board.b[black][king]))
	 return (false);
      if (!(BitPosArray[A8] & board.b[black][rook]))
	 return (false);
   }

   return (true);
}
