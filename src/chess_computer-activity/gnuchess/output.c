/* GNU Chess 5.0 - output.c - output code
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
 *
 */

#include <config.h>
#include <stdio.h>
#include "common.h"

void ShowTime (void)
/**************************************************************************
 *
 *  Print out the time settings.
 *
 **************************************************************************/
{
}

void ShowMoveList (int ply)
/**************************************************************************
 *
 *  Print out the move list.  
 *
 **************************************************************************/
{
   leaf *node;
   int i = 0;
   
   for (node = TreePtr[ply]; node < TreePtr[ply+1]; node++)
   {
      SANMove (node->move, ply); 
      printf ("%5s %3d\t", SANmv, SwapOff(node->move));
      if (++i == 5)
      {
         printf ("\n"); 
         i = 0;
      }
   }
   printf ("\n");
} 


void ShowSmallBoard (void)     
/*****************************************************************************
 *
 *  Display the board.  Not only that but display some useful information
 *  like whether enpassant is legal and castling state.
 *
 *****************************************************************************/ 
{
   int r, c, sq;

   printf ("\n");
   if (board.side == white)
      printf ("white  ");
   else
      printf ("black  ");

   if (board.flag & WKINGCASTLE)
      printf ("K");
   if (board.flag & WQUEENCASTLE)
      printf ("Q");
   if (board.flag & BKINGCASTLE)
      printf ("k");
   if (board.flag & BQUEENCASTLE)
      printf ("q");

   if (board.ep > -1)
      printf ("  %s", algbr[board.ep]);

   printf ("\n");
   for (r = 56; r >= 0; r -= 8)
   {
      for (c = 0; c < 8; c++)
      {
         sq = r + c;
         if (board.b[white][pawn] & BitPosArray[sq])
            printf ("P ");
         else if (board.b[white][knight] & BitPosArray[sq])
            printf ("N ");
         else if (board.b[white][bishop] & BitPosArray[sq])
            printf ("B ");
         else if (board.b[white][rook]   & BitPosArray[sq])
            printf ("R ");
         else if (board.b[white][queen]  & BitPosArray[sq])
            printf ("Q ");
         else if (board.b[white][king]   & BitPosArray[sq])
            printf ("K ");
         else if (board.b[black][pawn]   & BitPosArray[sq])
            printf ("p ");
         else if (board.b[black][knight] & BitPosArray[sq])
            printf ("n ");
         else if (board.b[black][bishop] & BitPosArray[sq])
            printf ("b ");
         else if (board.b[black][rook]   & BitPosArray[sq])
            printf ("r ");
         else if (board.b[black][queen]  & BitPosArray[sq])
            printf ("q ");
         else if (board.b[black][king]   & BitPosArray[sq])
            printf ("k ");
         else
            printf (". ");
      }
      printf ("\n");
   }
   printf ("\n");
}
  


void ShowBitBoard (BitBoard *b)
/*****************************************************************************
 *
 * Just to print a lousy ascii board  
 *
 *****************************************************************************/
{
   int r, c;

   printf ("\n");
   for (r = 56; r >= 0; r -= 8)
   {
      for (c = 0; c < 8; c++)
      {
         if (*b & BitPosArray[r + c])
            printf ("1 ");
         else
            printf (". "); 
      }
      printf ("\n");
   }
   printf ("\n");
}


void ShowBoard (void)     
/*****************************************************************************
 *
 *  Display the board.  Not only that but display some useful information
 *  like whether enpassant is legal and castling state.
 *
 *****************************************************************************/ 
{
   int r, c, sq;

   fprintf (ofp, "\n");
   if (board.side == white)
      fprintf (ofp, "white  ");
   else
      fprintf (ofp, "black  ");

   if (board.flag & WKINGCASTLE)
      fprintf (ofp, "K");
   if (board.flag & WQUEENCASTLE)
      fprintf (ofp, "Q");
   if (board.flag & BKINGCASTLE)
      fprintf (ofp, "k");
   if (board.flag & BQUEENCASTLE)
      fprintf (ofp, "q");

   if (board.ep > -1)
      fprintf (ofp, "  %s", algbr[board.ep]);

   fprintf (ofp, "\n");
   for (r = 56; r >= 0; r -= 8)
   {
      for (c = 0; c < 8; c++)
      {
         sq = r + c;
         if (board.b[white][pawn]   & BitPosArray[sq])
            fprintf (ofp, "P ");
         else if (board.b[white][knight] & BitPosArray[sq])
            fprintf (ofp, "N ");
         else if (board.b[white][bishop] & BitPosArray[sq])
            fprintf (ofp, "B ");
         else if (board.b[white][rook]   & BitPosArray[sq])
            fprintf (ofp, "R ");
         else if (board.b[white][queen]  & BitPosArray[sq])
            fprintf (ofp, "Q ");
         else if (board.b[white][king]   & BitPosArray[sq])
            fprintf (ofp, "K ");
         else if (board.b[black][pawn]   & BitPosArray[sq])
            fprintf (ofp, "p ");
         else if (board.b[black][knight] & BitPosArray[sq])
            fprintf (ofp, "n ");
         else if (board.b[black][bishop] & BitPosArray[sq])
            fprintf (ofp, "b ");
         else if (board.b[black][rook]   & BitPosArray[sq])
            fprintf (ofp, "r ");
         else if (board.b[black][queen]  & BitPosArray[sq])
            fprintf (ofp, "q ");
         else if (board.b[black][king]   & BitPosArray[sq])
            fprintf (ofp, "k ");
         else
            fprintf (ofp, ". ");
      }
      fprintf (ofp, "\n");
   }
   fprintf (ofp, "\n");
}

void ShowCBoard (void)
/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
{
   int r, c;

   for (r = 56; r >= 0; r -= 8)
   {
      for (c = 0; c < 8; c++)
      {
         printf ("%2c ", cboard[r + c] ? notation[cboard[r+c]] : '.');
      }
      printf ("\n");
   }
   printf ("\n");
}


void ShowMvboard (void)
/*****************************************************************************
 *
 *  Print the Mvboard[] array.
 *
 *****************************************************************************/
{
   int r, c;

   for (r = 56; r >= 0; r -= 8)
   {
      for (c = 0; c < 8; c++)
      {
         printf ("%2d ", Mvboard[r + c]);
      }
      printf ("\n");
   }
   printf ("\n");
}
   
void ShowGame (void)
{
  int i;

/* *********************************************
   * We must handle the special case of an EPD *
   * game where the first move is by black     *
   ********************************************* */
  
  if ( GameCnt >= 0 )
  {
  
    printf ("      White   Black\n");
  
    if ( ( board.side == white && GameCnt % 2 == 1 ) ||
         ( board.side == black && GameCnt % 2 == 0 ))
    {
    
      for (i = 0; i <= GameCnt; i += 2)
        {
          printf ("%3d.  %-7s %-7s\n", i/2 + 1, Game[i].SANmv, 
    	      Game[i + 1].SANmv);
        }
    }
    else {
    
      printf ("  1.          %-7s\n", Game[0].SANmv);
  
      for (i = 1; i <= GameCnt; i += 2)
        {
          printf ("%3d.  %-7s %-7s\n", i/2 + 2, Game[i].SANmv, 
    	      Game[i + 1].SANmv);
        }
    }
    printf ("\n");
  }
}
