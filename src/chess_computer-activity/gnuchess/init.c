/* GNU Chess 5.0 - init.c - initialization of variables code
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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "common.h"
#include "version.h"
#include "lexpgn.h"

void Initialize (void)
/**************************************************************************
 *
 *  The main initialization driver.
 *
 **************************************************************************/
{
   InitLzArray ();
   InitBitPosArray ();
   InitMoveArray ();
   InitRay ();
   InitFromToRay (); 
   InitRankFileBit ();
   InitPassedPawnMask ();
   InitIsolaniMask ();
   InitSquarePawnMask ();
   InitBitCount ();
   InitRotAtak ();
   InitRandomMasks ();
   InitDistance ();
   InitVars ();
   InitHashCode ();
   InitHashTable ();
   CalcHashKey ();
   InitInput ();
}


void InitFICS (void)
{
   if (flags & XBOARD) {
     printf ("tellics shout Greetings from %s %s. Ready for a game.\n", PROGRAM, VERSION);
     printf ("tellics set 1 %s %s.\n",PROGRAM,VERSION);
   }

}

#define NBITS 16

void InitLzArray (void)
/***************************************************************************
 *
 *  The lzArray is created.  This array is used when the position
 *  of the leading non-zero bit is required.  The convention used
 *  is that the leftmost bit is considered as position 0 and the
 *  rightmost bit position 63.
 *
 ***************************************************************************/
{
   int i, j, s, n;

   s = n = 1;
   for (i = 0; i < NBITS; i++)
   {
      for (j = s; j < s + n; j++)
         lzArray[j] = NBITS - 1 - i;
      s += n;
      n += n;
   }
}


void InitBitPosArray (void)
/***************************************************************************
 *
 *  BitPosArray[i] returns the bitboard whose ith bit is set to 1 
 *  and every other bits 0.  This ought to be faster than doing
 *  shifting all the time (I think).  
 *  Also compute the NotBitPosArray = ~BitPosArray.
 *
 ***************************************************************************/
{
   BitBoard b;
   int i;

   b = (BitBoard) 1;  
   for (i = 63; i >= 0; i--, b <<= 1)
   {
      BitPosArray[i] = b;
      NotBitPosArray[i] = ~b;
   }
}
 


/*  Data used for generating MoveArray  */

static const int dir[8][8] =
{
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 9, 11, 0, 0, 0, 0, 0, 0 },
  { -21, -19, -12, -8, 8, 12, 19, 21 },
  { -11, -9, 9, 11, 0, 0, 0, 0 },
  { -10, -1, 1, 10, 0, 0, 0, 0 },
  { -11, -10, -9, -1, 1, 9, 10, 11 },
  { -11, -10, -9, -1, 1, 9, 10, 11 },
  { -9, -11, 0, 0, 0, 0, 0, 0 }
};
static const int ndir[8] = 
{ 0, 2, 8, 4, 4, 8, 8, 2 };

static const int map[120] =
{
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
  -1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
  -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
  -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
  -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
  -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
  -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 
};


void InitMoveArray (void)
/***************************************************************************
 * 
 *  Generate the move bitboards.  For e.g. the bitboard for all
 *  the moves of a knight on f3 is given by MoveArray[knight][21].
 *
 ***************************************************************************/
{
   int piece, fsq, tsq, f, t, n;
   BitBoard *b;

   for (piece = pawn; piece <= bpawn; piece++)
   {
      for (fsq = 0; fsq < 120; fsq++)
      {
         if ((f = map[fsq]) == -1) continue;
         b = &MoveArray[piece][f];
         *b = NULLBITBOARD;
         for (n = 0; n < ndir[piece]; n++)
         {
            tsq = fsq;
            do
            {
               tsq += dir[piece][n];
               if ((t = map[tsq]) != -1)
                  SETBIT (*b, t);
            } while (range[piece] && t != -1);
         }
      }
   }
}


void InitRay (void)
/**************************************************************************
 *
 *  For each square, there are 8 rays.  The first 4 rays are diagonals 
 *  for the bishops and the next 4  are file/rank for the rooks.  
 *  The queen uses all 8 rays.
 *  These rays are used for move generation rather than MoveArray[].
 *  Also initialize the directions[][] array.  directions[f][t] returns
 *  the index into Ray[f] array allow us to find the ray in that direction.
 *
 **************************************************************************/
{
   int piece, fsq, tsq, f, t, n, ray;
   BitBoard *b;

   memset (directions, -1, sizeof (directions));
   for (fsq = 0; fsq < 120; fsq++)
   {
      if ((f = map[fsq]) == -1) continue;
      ray = -1;
      for (piece = bishop; piece <= rook; piece++)
      {
         for (n = 0; n < ndir[piece]; n++)
         {
            b = &Ray[f][++ray];
            *b = NULLBITBOARD;
            tsq = fsq;
            do
            {
               tsq += dir[piece][n];
               if ((t = map[tsq]) != -1)
	       {
                  SETBIT (*b, t);
	          directions[f][t] = ray;
	       }
            } while (t != -1);
         }
      }
   }
}


void InitFromToRay (void)
/***************************************************************************
 *
 *  The FromToRay[b2][f6] gives the diagonal ray from c3 to f6;
 *  It also produces horizontal/vertical rays as well.   If no
 *  ray is possible, then a 0 is returned.
 *
 ***************************************************************************/
{
   int piece, fsq, tsq, f, t, n;
   BitBoard *b;

   memset (FromToRay, 0, sizeof (FromToRay));
   for (piece = bishop; piece <= rook; piece++)
   {
      for (fsq = 0; fsq < 120; fsq++)
      {
         if ((f = map[fsq]) == -1) continue;
         for (n = 0; n < ndir[piece]; n++)
         {
            tsq = fsq;
            t = map[tsq];
            do
            {
               b = &FromToRay[f][t];
               tsq += dir[piece][n];         
               if ((t = map[tsq]) != -1)
               {
                  SETBIT (FromToRay[f][t], t);
                  FromToRay[f][t] |= *b;
               }
            } while (t != -1);
         }
      }
   }
}


void InitRankFileBit (void)
/***************************************************************************
 *
 *  RankBit[2] has all the bits on the 3rd rank 1 and others 0.
 *  FileBit[2] has all the bits on the 3rd file 1 and others 0.
 *
 ***************************************************************************/
{
   BitBoard b;
   int i;

   i = 8;
   b = (BitBoard) 255;
   while (i--)
   {
      RankBit[i] = b;
      b <<= 8;
   }
   
   i = 8;
   b = ULL(0x0101010101010101);
   while (i--)
   {
      FileBit[i] = b;
      b <<= 1;
   }
}


void InitRandomMasks (void)
{
  mask_kr_trapped_w[0]=BitPosArray[H2];
  mask_kr_trapped_w[1]=BitPosArray[H1]|BitPosArray[H2];
  mask_kr_trapped_w[2]=BitPosArray[G1]|BitPosArray[H1]|BitPosArray[H2];
  mask_qr_trapped_w[0]=BitPosArray[A2];
  mask_qr_trapped_w[1]=BitPosArray[A1]|BitPosArray[A2];
  mask_qr_trapped_w[2]=BitPosArray[A1]|BitPosArray[B1]|BitPosArray[A2];
  mask_kr_trapped_b[0]=BitPosArray[H7];
  mask_kr_trapped_b[1]=BitPosArray[H8]|BitPosArray[H7];
  mask_kr_trapped_b[2]=BitPosArray[H8]|BitPosArray[G8]|BitPosArray[H7];
  mask_qr_trapped_b[0]=BitPosArray[A7];
  mask_qr_trapped_b[1]=BitPosArray[A8]|BitPosArray[A7];
  mask_qr_trapped_b[2]=BitPosArray[A8]|BitPosArray[B8]|BitPosArray[A7];
}

void InitPassedPawnMask (void)
/**************************************************************************
 *
 *  The PassedPawnMask variable is used to determine if a pawn is passed.
 *  This mask is basically all 1's from the square in front of the pawn to
 *  the promotion square, also duplicated on both files besides the pawn
 *  file.  Other bits will be set to zero.
 *  E.g. PassedPawnMask[white][b3] = 1's in a4-c4-c8-a8 rect, 0 otherwise.
 *
 **************************************************************************/
{
   unsigned int sq;

   memset (PassedPawnMask, 0, sizeof (PassedPawnMask));

   /*  Do for white pawns first */
   for (sq = 0; sq < 64; sq++)
   {
      PassedPawnMask[white][sq] = Ray[sq][7];
      if (ROW(sq) != 0)
         PassedPawnMask[white][sq] |= Ray[sq-1][7];
      if (ROW(sq) != 7)
         PassedPawnMask[white][sq] |= Ray[sq+1][7];
   }

   /*  Do for black pawns */
   for (sq = 0; sq < 64; sq++)
   {
      PassedPawnMask[black][sq] = Ray[sq][4];
      if (ROW(sq) != 0)
         PassedPawnMask[black][sq] |= Ray[sq-1][4];
      if (ROW(sq) != 7)
         PassedPawnMask[black][sq] |= Ray[sq+1][4];
   }
}


void InitIsolaniMask (void)
/**************************************************************************
 *
 *  The IsolaniMask variable is used to determine if a pawn is an isolani.
 *  This mask is basically all 1's on files beside the file the pawn is on.
 *  Other bits will be set to zero.
 *  E.g. IsolaniMask[d-file] = 1's in c-file & e-file, 0 otherwise.
 *  IMPORTANT:!!!!
 *  Make sure this routine is called AFTER InitRankFileBit().
 *
 **************************************************************************/
{
   int i;

   IsolaniMask[0] = FileBit[1];
   IsolaniMask[7] = FileBit[6];
   for (i = 1; i <= 6; i++)
      IsolaniMask[i] = FileBit[i-1] | FileBit[i+1];
      
}


void InitSquarePawnMask (void)
/**************************************************************************
 *
 *  The SquarePawnMask is used to determine if a king is in the square of
 *  the passed pawn and is able to prevent it from queening.  
 *  Caveat:  Pawns on 2nd rank have the same mask as pawns on the 3rd rank
 *  as they can advance 2 squares.
 *
 **************************************************************************/
{
   unsigned int sq;
   int len, i, j;

   memset (SquarePawnMask, 0, sizeof (PassedPawnMask));
   for (sq = 0; sq < 64; sq++)
   {
      /*  White mask  */
      len = 7 - RANK (sq);
      i = MAX (sq & 56, sq - len);
      j = MIN (sq | 7, sq + len);
      while (i <= j)
      {
         SquarePawnMask[white][sq] |= (BitPosArray[i] | FromToRay[i][i|56]);
         i++;
      }

      /*  Black mask  */
      len = RANK (sq);
      i = MAX (sq & 56, sq - len);
      j = MIN (sq | 7, sq + len);
      while (i <= j)
      {
         SquarePawnMask[black][sq] |= (BitPosArray[i] | FromToRay[i][i&7]);
         i++;
      }
   }

   /*  For pawns on 2nd rank, they have same mask as pawns on 3rd rank */
   for (sq = A2; sq <= H2; sq++)
      SquarePawnMask[white][sq] = SquarePawnMask[white][sq+8];
   for (sq = A7; sq <= H7; sq++)
      SquarePawnMask[black][sq] = SquarePawnMask[black][sq-8];
}

 
void InitBitCount (void)
/**************************************************************************
 *
 *  The BitCount array returns the no. of bits present in the 16 bit
 *  input argument.  This is use for counting the number of bits set
 *  in a BitBoard (e.g. for mobility count).
 *
 **************************************************************************/
{
   int i, j, n;
 
   BitCount[0] = 0;
   BitCount[1] = 1; 
   i = 1;
   for (n = 2; n <= 16; n++)
   {
      i <<= 1;
      for (j = i; j <= i + (i-1); j++)  
         BitCount[j] = 1 + BitCount[j - i]; 
   }
} 


void InitRotAtak (void)
/**************************************************************************
 *
 *  The attack tables for a normal chessboard and the rotated chess board
 *  are calculated here.
 *
 **************************************************************************/
{
   int sq, map, sq1, sq2;
   int cmap[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
   int rot1[8] = { A1, A2, A3, A4, A5, A6, A7, A8 };
   int rot2[8] = { A1, B2, C3, D4, E5, F6, G7, H8 };
   int rot3[8] = { A8, B7, C6, D5, E4, F3, G2, H1 };
   
   for (sq = A1; sq <= H1; sq++)
   {
      for (map = 0; map < 256; map++)
      {
	 Rook00Atak[sq][map] = 0;
	 Rook90Atak[rot1[sq]][map] = 0;
	 Bishop45Atak[rot2[sq]][map] = 0;
	 Bishop315Atak[rot3[sq]][map] = 0;
	    sq1 = sq2 = sq;
	    while (sq1 > 0)
	    {
	       if (cmap[--sq1] & map)
	          break;
	    }
	    while (sq2 < 7)
	    {
	       if (cmap[++sq2] & map)
	          break;
	    }
	    Rook00Atak[sq][map] = 
		FromToRay[sq][sq1] | FromToRay[sq][sq2];
	    Rook90Atak[rot1[sq]][map] =
		FromToRay[rot1[sq]][rot1[sq1]] | 
		FromToRay[rot1[sq]][rot1[sq2]];
	    Bishop45Atak[rot2[sq]][map] =
		FromToRay[rot2[sq]][rot2[sq1]] |
		FromToRay[rot2[sq]][rot2[sq2]];
	    Bishop315Atak[rot3[sq]][map] =
		FromToRay[rot3[sq]][rot3[sq1]] |
		FromToRay[rot3[sq]][rot3[sq2]];
      }
   } 

   for (map = 0; map < 256; map++)
   {
      for (sq = A2; sq <= H8; sq++)
      {
	 Rook00Atak[sq][map] = Rook00Atak[sq-8][map] >> 8;
      }

      for (sq1 = B_FILE; sq1 <= H_FILE; sq1++)
      {
	 for (sq2 = 0; sq2 < 64; sq2+=8)
	 {
	    sq = sq2 + sq1;
	    Rook90Atak[sq][map] = Rook90Atak[sq-1][map] >> 1;
	 }
      }

      for (sq1 = B1, sq2 = H7; sq1 <= H1; sq1++, sq2-=8)
      {
         for (sq = sq1; sq <= sq2; sq += 9)
	 {
	    Bishop45Atak[sq][map] = Bishop45Atak[sq+8][map] << 8;
	 }
      }
      for (sq1 = A2, sq2 = G8; sq1 <= A8; sq1+=8, sq2--)
      {
         for (sq = sq1; sq <= sq2; sq += 9)
	 {
	    Bishop45Atak[sq][map] = 
		(Bishop45Atak[sq+1][map] & NotBitPosArray[sq1-8]) << 1;
	 }
      }

      for (sq1 = H2, sq2 = B8; sq1 <= H8; sq1+=8, sq2++)
      {
         for (sq = sq1; sq <= sq2; sq += 7)
	 {
	    Bishop315Atak[sq][map] = Bishop315Atak[sq-8][map] >> 8;
	 }
      }
      for (sq1 = G1, sq2 = A7; sq1 >= A1; sq1--, sq2-=8)
      {
         for (sq = sq1; sq <= sq2; sq += 7)
	 {
	    Bishop315Atak[sq][map] = 
		(Bishop315Atak[sq+1][map] & NotBitPosArray[sq2+8]) << 1;
	 }
      }
   }
}


void InitDistance (void)
/**************************************************************************
 *
 *  There are two arrays dealing with distances.  The distance[s1][s2]
 *  array gives the minimum number of moves a king takes to get from s1
 *  to s2.  The taxicab[s1][s2] array is the taxicab distance.  Example:
 *  distance[a1][b3] = 2;  taxicab[a1[b3] = 3;
 *
 *************************************************************************/
{
   int f, t, j;
   int d1, d2;

   for (f = 0; f < 64; f++)
     for (t = 0; t < 8; t++)
       DistMap[f][t] = ULL(0);


   for (f = 0; f < 64; f++)
      for (t = f; t < 64; t++)
      {
         d1 = (t & 0x07) - (f & 0x07);
         if (d1 < 0) d1 = -d1;
         d2 = (t >> 3) - (f >> 3);
         if (d2 < 0) d2 = -d2;
         distance[f][t] = MAX (d1, d2);
         distance[t][f] = MAX (d1, d2);
         taxicab[f][t] = d1 + d2;
         taxicab[t][f] = d1 + d2;
      }

   for (f = 0; f < 64; f++)
     for (t = 0; t < 64; t++)
	 DistMap[f][distance[t][f]] |= BitPosArray[t];

   for (f = 0; f < 64; f++)
     for (t = 0; t < 8; t++)
       for (j = 0; j < t; j++)
	 DistMap[f][t] |= DistMap[f][j];
}


void InitVars (void)
/***************************************************************************
 *
 *  Initialize various variables, especially for new game.
 *
 ***************************************************************************/
{
   int i;

   /* Initialize board */
   memset (&board, 0, sizeof (board));
   for (i = 8; i < 16; i++)
      SETBIT (board.b[white][pawn], i);
   SETBIT (board.b[white][rook], 0);
   SETBIT (board.b[white][knight], 1);
   SETBIT (board.b[white][bishop], 2);
   SETBIT (board.b[white][queen], 3);
   SETBIT (board.b[white][king], 4);
   SETBIT (board.b[white][bishop], 5);
   SETBIT (board.b[white][knight], 6);
   SETBIT (board.b[white][rook], 7);
   for (i = 48; i < 56; i++)
      SETBIT (board.b[black][pawn], i);
   SETBIT (board.b[black][rook], 56);
   SETBIT (board.b[black][knight], 57);
   SETBIT (board.b[black][bishop], 58);
   SETBIT (board.b[black][queen], 59);
   SETBIT (board.b[black][king], 60);
   SETBIT (board.b[black][bishop], 61);
   SETBIT (board.b[black][knight], 62);
   SETBIT (board.b[black][rook], 63);

   SETBIT (stonewall[white], D4); /* SMC */
   SETBIT (stonewall[white], E3); /* SMC */
   SETBIT (stonewall[white], F4); /* SMC */

   SETBIT (stonewall[black], D5); /* SMC */
   SETBIT (stonewall[black], E6); /* SMC */
   SETBIT (stonewall[black], F5); /* SMC */

   rings[0] = ULL(0x0000001818000000);
   rings[1] = ULL(0x00003C24243C0000);
   rings[2] = ULL(0x007E424242427E00);
   rings[3] = ULL(0xFF818181818181FF);

   boxes[0] = ULL(0x00003C3C3C3C0000); /* rings[0] | rings[1] */
   boxes[1] = ULL(0x007E7E7E7E7E7E00); /* rings[0] | rings[1] | rings[2] */

   boardhalf[white] = RankBit[0]|RankBit[1]|RankBit[2]|RankBit[3];
   boardhalf[black] = RankBit[4]|RankBit[5]|RankBit[6]|RankBit[7];
   boardside[ks] = FileBit[4]|FileBit[5]|FileBit[6]|FileBit[7];
   boardside[qs] = FileBit[0]|FileBit[1]|FileBit[2]|FileBit[3];

   board.flag |= (WCASTLE | BCASTLE);
   RealSide = board.side = white;
   board.ep = -1;
   board.king[white] = E1;
   board.king[black] = E8;
   RealGameCnt = GameCnt = -1;
   Game50 = 0;
   computer = black;
   CalcHashKey ();
   Game[0].hashkey = HashKey;
   board.pmaterial[white] = board.pmaterial[black] =
      2*ValueR + 2*ValueN + 2*ValueB + ValueQ;
   board.material[white] = board.material[black] =
      board.pmaterial[white] + 8*ValueP;

   /* Initialize pgn values */
   initial_comments = NULL;
   /* Reset values; doing this again will cause a trivial memory leak
    * when reloading PGN files as games, but it's not worth fixing. */
   pgn_event = pgn_site =
   pgn_date = pgn_round = pgn_white = pgn_black =
   pgn_whiteELO = pgn_blackELO = pgn_result =
   pgn_othertags = NULL; 


   UpdateFriends ();
   UpdateCBoard ();
   UpdateMvboard ();

   for (i = A1; i <= H8; i++)
   {
      if (cboard[i])
      {
         SETBIT (board.blockerr90, r90[i]);
         SETBIT (board.blockerr45, r45[i]);
         SETBIT (board.blockerr315, r315[i]);
      }
   }

   /* TreePtr[0] is practically unused.  TreePtr[1] points to the
    * base of the tree.
    */
   TreePtr[0] = TreePtr[1] = Tree;

   /* Initialize some of the game flags */
   SET (flags, USEHASH);
   SET (flags, USENULL);
   SearchTime = 5;
   SearchDepth = 0;
   board.castled[white] = board.castled[black] = false;
   phase = PHASE;

/*  Calculate the ttable hashmask & pawntable hashmask */
   if ( HashSize == 0 )
     CalcHashSize(HashSize);
     
   signal (SIGINT, EndSearch);

   nmovesfrombook = 0;
}


void InitHashCode (void)
/***************************************************************************
 *
 *  The hash code for the various pieces standing on the various squares
 *  are initialized here.  When a particular piece is move from square A
 *  to square B, the current hashkey is XORed against the hash code for
 *  the A square and B square to give the new hashkey.
 *
 ***************************************************************************/
{
   int color, piece, sq;

   for (color = white; color <= black; color++)
   {
      for (piece = pawn; piece <= king; piece++)
      {
	 for (sq = 0; sq < 64; sq++)
	 {
	    hashcode[color][piece][sq] = Rand64 ();
	 }
      }
   }
   for (sq = 0; sq < 64; sq++)
      ephash[sq] = Rand64 ();
   WKCastlehash = Rand64 ();
   WQCastlehash = Rand64 ();
   BKCastlehash = Rand64 ();
   BQCastlehash = Rand64 ();
   Sidehash = Rand64 ();
}


void InitHashTable (void)
/****************************************************************************
  *
  *  Allocate memory for our transposition tables.  By default, the
  *  transposition table will have 16K entries; 8K for each color.
  *  Add code to allocated memory for pawn table as well.
  *
  ****************************************************************************/
{
    unsigned int size;
    unsigned int allocating = 1;
    /* One can not use realloc() here, it screws up error handling */
    do {
      free(HashTab[0]);
      free(HashTab[1]);
      HashTab[0] = malloc (HashSize * sizeof (HashSlot));
      HashTab[1] = malloc (HashSize * sizeof (HashSlot));
      if (HashTab[0] == NULL || HashTab[1] == NULL) {
         printf ("Not enough memory for transposition table, trying again.\n");
         if (HashSize == HASHSLOTS) {
           fprintf(stderr, "Memory exhausted, goodbye, my friend.\n");
           exit(EXIT_FAILURE);
         }
         CalcHashSize(HashSize>>1);
      }
      else
         allocating = 0;
    } while (allocating);
    size = (HashSize * 2 * sizeof (HashSlot)) >> 10;
    if (!(flags & XBOARD)) {
      printf ("Transposition table:  Entries=%dK Size=%dK\n",
              HashSize>>10, size);
    }
    /*
     * Here realloc() is O.K., though not very useful, because PAWNSLOTS
     * is constant anyway.
     */
    PawnTab[0] = (PawnSlot *) realloc (PawnTab[0], PAWNSLOTS * sizeof 
(PawnSlot));
    PawnTab[1] = (PawnSlot *) realloc (PawnTab[1], PAWNSLOTS * sizeof 
(PawnSlot));
    if (PawnTab[0] == NULL || PawnTab[1] == NULL) {
       printf ("Not enough memory for pawn table, goodbye.\n");
       exit(EXIT_FAILURE);
    } else {
       size = (PAWNSLOTS * 2 * sizeof (PawnSlot)) >> 10;
       if (!(flags & XBOARD))
         printf ("Pawn hash table: Entries=%dK Size=%dK\n",
                 PAWNSLOTS >> 10, size);
    }
}


void NewPosition (void)
/****************************************************************************
 *
 *  Reset some flags for a new game/position.
 *
 ****************************************************************************/
{
   CLEAR (flags, ENDED);
   Game50 = 0;
   RealGameCnt = GameCnt = -1;
   Game[0].hashkey = HashKey;
   TTClear ();
   PTClear ();
   nmovesfrombook = 0;
   ExchCnt[white] = ExchCnt[black] = 0;
}
