/* GNU Chess 5.0 - atak.c - attack code
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
#include <string.h>
#include "common.h"


short SqAtakd (short sq, short side)
/**************************************************************************
 *
 *  To determine if sq is attacked by any pieces from side.
 *
 **************************************************************************/
{
   register BitBoard *a, b, *c, d, blocker;
   int t;
   
   a = board.b[side];

   /* Knights */
   if (a[knight] & MoveArray[knight][sq])
      return (true);

   /* Kings */
   if (a[king] & MoveArray[king][sq])
      return (true);

   /* Pawns */
   if (a[pawn] & MoveArray[ptype[1^side]][sq])
      return (true);
      
   c = FromToRay[sq];
   blocker = board.blocker;

   /* Bishops & Queen */
   b = (a[bishop] | a[queen]) & MoveArray[bishop][sq];
   d = ~b & blocker; 
   while (b)
   {
      t = leadz (b);
      if (!(c[t] & d))
         return (true);
      CLEARBIT (b, t); 
   }

   /* Rooks & Queen */
   b = (a[rook] | a[queen]) & MoveArray[rook][sq];
   d = ~b & blocker;
   while (b)
   {
      t = leadz (b);
      if (!(c[t] & d))
         return (true);
      CLEARBIT (b, t); 
   }
   return (false);
}

extern const short raybeg[];
extern const short rayend[];

void GenAtaks (void)
/*************************************************************************
 *
 *  To generate the attack table.
 *  Ataks[side][0] holds the total attack tables.
 *  Ataks[side][pawn] holds the BitBoard of squares attacked by all pawns.
 *
 *************************************************************************/
{
   int side; 
   int sq;
   register BitBoard *a, b, *t, *a0;

   memset (Ataks, 0, sizeof (Ataks)); 
   for (side = white; side <= black; side++)
   {
      a = board.b[side];

      /* Knight */
      t = &Ataks[side][knight];
      b = a[knight];
      while (b)
      {
         sq = leadz (b);
         CLEARBIT (b, sq);
         *t |= MoveArray[knight][sq];
      }

      /* Bishops */
      t = &Ataks[side][bishop];
      b = a[bishop];
      while (b)
      {
	 sq = leadz (b);
	 CLEARBIT (b, sq);
	 *t |= BishopAttack(sq);
      }

      /*  Rooks */
      t = &Ataks[side][rook];
      b = a[rook];
      while (b)
      {
	 sq = leadz (b);
	 CLEARBIT (b, sq);
	 *t |= RookAttack(sq);
      }

      /*  Queen  */
      t = &Ataks[side][queen];
      b = a[queen];
      while (b)
      {
	 sq = leadz (b);
	 CLEARBIT (b, sq);
	 *t |= QueenAttack(sq);
      }

      /* King */
      t = &Ataks[side][king];
      b = a[king];
      while (b)
      {
         sq = leadz (b);
         CLEARBIT (b, sq);
         *t |= MoveArray[king][sq];
      }

      /*  pawns  */
      t = &Ataks[side][pawn];
      if (side == white)
      {
         b = board.b[white][pawn] & ~FileBit[0];
         *t |= (b >> 7);
         b = board.b[white][pawn] & ~FileBit[7];
         *t |= (b >> 9);
      }
      else
      {
         b = board.b[black][pawn] & ~FileBit[0];
         *t |= (b << 9);
         b = board.b[black][pawn] & ~FileBit[7];
         *t |= (b << 7);
      }
      a0 = Ataks[side];
      a0[0] = a0[pawn] | a0[knight] | a0[bishop] | a0[rook] |
              a0[queen] | a0[king];
   }
}


BitBoard AttackTo (int sq, int side)
/***************************************************************************
 *
 *  Generate a bitboard of all squares with pieces belonging to side
 *  which attack sq.
 *
 ***************************************************************************/
{
   register BitBoard *a, b, *c, e, blocker;
   int t;
   
   a = board.b[side];

   /* Knights */
   e = (a[knight] & MoveArray[knight][sq]); 	

   /* Kings */
   e |= (a[king] & MoveArray[king][sq]); 	

   /* Pawns */
   e |= (a[pawn] & MoveArray[ptype[1^side]][sq]);
      
   c = FromToRay[sq];
   blocker = board.blocker;

   /* Bishops & Queen */
   b = (a[bishop] | a[queen]) & MoveArray[bishop][sq];
   while (b)
   {
      t = leadz (b);
      CLEARBIT (b, t); 
      if (!(c[t] & blocker & NotBitPosArray[t]))
	 e |= BitPosArray[t];
   }

   /* Rooks & Queen */
   b = (a[rook] | a[queen]) & MoveArray[rook][sq];
   while (b)
   {
      t = leadz (b);
      CLEARBIT (b, t); 
      if (!(c[t] & blocker & NotBitPosArray[t]))
	 e |= BitPosArray[t];
   }

   return (e);
}
 

BitBoard AttackXTo (int sq, int side)
/***************************************************************************
 *
 *  Generate a bitboard of all squares with pieces belonging to side
 *  which attack sq.  This routine is slightly different from AttackTo
 *  as it includes X-ray attacks as well and these can go through the
 *  opponents pieces as well (e.g. a white R will attack the squares 
 *  beyond the black R). 
 *
 ***************************************************************************/
{
   register BitBoard *a, b, *c, *d, e, blocker;
   int t;
   
   a = board.b[side];
   d = board.b[1^side];

   /* Knights */
   e = (a[knight] & MoveArray[knight][sq]); 	

   /* Kings */
   e |= (a[king] & MoveArray[king][sq]); 	

   c = FromToRay[sq];

   /* Bishops & Queen & Pawns */
   b = (a[pawn] & MoveArray[ptype[1^side]][sq]);
   blocker = board.blocker;
   blocker &= ~(a[bishop] | a[queen] | d[bishop] | d[queen] | b);
   b |= (a[bishop] | a[queen]) & MoveArray[bishop][sq];
   while (b)
   {
      t = leadz (b);
      CLEARBIT (b, t); 
      if (!(c[t] & blocker & NotBitPosArray[t]))
	 e |= BitPosArray[t];
   }

   /* Rooks & Queen */
   b = (a[rook] | a[queen]) & MoveArray[rook][sq];
   blocker = board.blocker;
   blocker &= ~(a[rook] | a[queen] | d[rook] | d[queen]);
   while (b)
   {
      t = leadz (b);
      CLEARBIT (b, t); 
      if (!(c[t] & blocker & NotBitPosArray[t]))
	 e |= BitPosArray[t];
   }

   return (e);
}


BitBoard AttackFrom (int sq, int piece, int side)
/***************************************************************************
 *
 *  Generate a bitboard of all squares attacked by a piece on sq.
 *
 ***************************************************************************/
{
   switch (piece)
   {
      case pawn :
         return (MoveArray[ptype[side]][sq]);
      case knight :
	 return (MoveArray[knight][sq]);
      case bishop :
	 return (BishopAttack(sq));
      case rook :
	 return (RookAttack(sq));
      case queen :
	 return (QueenAttack(sq));
      case king :
	 return (MoveArray[king][sq]);
   } 
   return (0);
}


BitBoard AttackXFrom (int sq, int side)
/***************************************************************************
 *
 *  Generate a bitboard of all squares attacked by a piece on sq.  This 
 *  routine is different from AttackFrom in that it includes Xray attacks.
 *  Caveat:  This routine does not take into account xrays through pawns.
 *
 ***************************************************************************/
{
   register BitBoard *a, b, c, blocker;
   int piece, dir, blocksq;

   a = board.b[side];
   piece = cboard[sq];
   blocker = board.blocker;
   b = 0;
   switch (piece)
   {
      case pawn :
         b = MoveArray[ptype[side]][sq];
         break;
      case knight :
	 b = MoveArray[knight][sq];
         break;
      case bishop : /* falls through as queens move diagnonally */
      case queen :
	 blocker &= ~(a[bishop] | a[queen]);
	 for (dir = raybeg[bishop]; dir < rayend[bishop]; dir++)
         {
            c = Ray[sq][dir] & blocker;
	    if (c == NULLBITBOARD)
	       c = Ray[sq][dir]; 
            else
            {
               blocksq = (BitPosArray[sq] > c ? leadz (c) : trailz (c));
               c = FromToRay[sq][blocksq];
            }
            b |= c;
         }
	 if (piece == bishop) /* Queen falls through as they move like rooks */
	    break;
         blocker = board.blocker;
      case rook :
	 blocker &= ~(a[rook] | a[queen]);
	 for (dir = raybeg[rook]; dir < rayend[rook]; dir++)
         {
            c = Ray[sq][dir] & blocker;
	    if (c == NULLBITBOARD)
	       c = Ray[sq][dir]; 
            else
            {
               blocksq = (BitPosArray[sq] > c ? leadz (c) : trailz (c));
               c = FromToRay[sq][blocksq];
            }
            b |= c;
	 }
	 break;
      case king :
	 b = MoveArray[king][sq];
         break;
   } 
   return (b);
}


int PinnedOnKing (int sq, int side)
/***************************************************************************
 *
 *  Determine if the piece on sq is pinned against the King.
 *  Side is the color of the piece.  
 *  Caveat: PinnedOnKing should only be called by GenCheckEscapes().
 *  The more generic FindPins() function should be used for evaluating
 *  pins against other pieces.
 *
 ***************************************************************************/
{
   int xside;
   int KingSq, dir, sq1;
   BitBoard b, blocker;

   KingSq = board.king[side];
   if ((dir = directions[KingSq][sq]) == -1)
      return (false);

   xside = 1 ^ side;
   blocker = board.blocker;
 
   /*  Path from piece to king is blocked, so no pin */
   if (FromToRay[KingSq][sq] & NotBitPosArray[sq] & blocker)
      return (false);
   b = (Ray[KingSq][dir] ^ FromToRay[KingSq][sq]) & blocker;
   if (b == NULLBITBOARD)
      return (false);
   sq1 = (sq > KingSq ? leadz (b) : trailz (b));

   /*  If diagonal  */
   if (dir <= 3 && 
	BitPosArray[sq1] & (board.b[xside][queen] | board.b[xside][bishop]))
      return (true);
   
   /*  Rank / file  */  
   if (dir >= 4 && 
	BitPosArray[sq1] & (board.b[xside][queen] | board.b[xside][rook]))
      return (true);

   return (false);
}


void FindPins (BitBoard *pin)
/***************************************************************************
 *
 *  This function creates a bitboard of all pieces which are pinned.
 *
 ***************************************************************************/
{
   int side, xside;
   int sq, sq1;
   BitBoard b, c, e, f, t, *p;
   
   *pin = NULLBITBOARD;
   t = board.friends[white] | board.friends[black];
   for (side = white; side <= black; side++)
   {
      xside = 1^side;
      p = board.b[xside];

      /*  Check if bishop is pinning anything */
      e = p[rook] | p[queen] | p[king];
      e |= (p[bishop] | p[knight]) & ~Ataks[xside][0];
      b = board.b[side][bishop];
      while (b)
      {
         sq = leadz (b);
         CLEARBIT (b, sq); 

	 c = MoveArray[bishop][sq] & e;
         while (c)
         {
	    sq1 = leadz (c);
	    CLEARBIT (c, sq1);
	    f = t & NotBitPosArray[sq] & FromToRay[sq1][sq];
            if ((board.friends[xside] & f) && nbits (f) == 1)
	       *pin |= f;
         }
      }

      /*  Check if rook is pinning anything */
      e = p[queen] | p[king];
      e |= (p[rook] | p[bishop] | p[knight]) & ~Ataks[xside][0];
      b = board.b[side][rook];
      while (b)
      {
         sq = leadz (b);
         CLEARBIT (b, sq); 

	 c = MoveArray[rook][sq] & e;
         while (c)
         {
	    sq1 = leadz (c);
	    CLEARBIT (c, sq1);
	    f = t & NotBitPosArray[sq] & FromToRay[sq1][sq];
            if ((board.friends[xside] & f) && nbits (f) == 1)
	       *pin |= f;
         }
      }

      /*  Check if queen is pinning anything */
      e = board.b[xside][king];
      e |= (p[queen] | p[rook] | p[bishop] | p[knight]) & ~Ataks[xside][0];
      b = board.b[side][queen];
      while (b)
      {
         sq = leadz (b);
         CLEARBIT (b, sq); 

	 c = MoveArray[queen][sq] & e;
         while (c)
         {
	    sq1 = leadz (c);
	    CLEARBIT (c, sq1);
	    f = t & NotBitPosArray[sq] & FromToRay[sq1][sq];
            if ((board.friends[xside] & f) && nbits (f) == 1)

	       *pin |= f;
         }
      }
   }
      
   return ;
}


int MateScan (int side)
/***************************************************************************
 *
 *  This routine scans the squares around the king to see if a Q + piece
 *  is attacking any of them.  If none, return 0, else return 1.
 *  If the square is defended, there is no threat.
 *  Limitations:  Assume only 1 Q present.
 *
 ***************************************************************************/
{
   int KingSq, QueenSq, sq;
   int xside;
   BitBoard b;

   xside = 1 ^ side;

   /*  Opponent has no queen, forget it  */
   if (board.b[xside][queen] == 0)
      return (0);

   KingSq = board.king[side];
   QueenSq = leadz (board.b[xside][queen]);
   b = QueenAttack(QueenSq) & MoveArray[king][KingSq];
   if (b == 0)
      return (0);

   while (b)
   {
      sq = leadz (b);
      if (AttackTo (sq, side) == board.b[side][king] &&
          AttackXTo (sq, xside) != board.b[xside][queen])
         return (1);
      CLEARBIT (b, sq);
   }
   
   return (0);
}
