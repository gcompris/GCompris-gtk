/* GNU Chess 5.0 - test.c - testing code
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
#include <sys/time.h>

#include "common.h"

void TestMoveGenSpeed (void)
/**************************************************************************
 *
 *   This routine benchmarks the speed of the bitmap move generation.
 *   The test case is BK.epd, the 24 positions from the Brat-Kopec test
 *   suite.
 *
 **************************************************************************/
{
   unsigned long i;
   struct timeval t1, t2;
   double et;
   short side, xside;

   GenCnt = 0;
   et = 0;
/*
   while (ReadEPDFile ("../test/wac.epd", 0))
   {
*/
      gettimeofday (&t1, NULL);
      side = board.side;
      xside = 1^side;
      for (i = 0; i < 2000000; i++)
      {
	 TreePtr[2] = TreePtr[1];
         GenMoves (1);
      }
      gettimeofday (&t2, NULL);
      et += (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1e6;
      printf ("Time = %f\n", et);
/*
   }
*/
   printf ("No. of moves generated = %lu\n", GenCnt);
   printf ("Time taken = %f secs\n", et);
   if (et > 0)
      printf ("Rate = %f moves/sec.\n", GenCnt / et);
}



void TestNonCaptureGenSpeed (void)
/**************************************************************************
 *
 *   This routine benchmarks the speed of the bitmap move generation
 *   for non capturing moves.
 *   The test case is BK.epd, the 24 positions from the Brat-Kopec test
 *   suite.
 *
 **************************************************************************/
{
   unsigned long i;
   struct timeval t1, t2;
   double et;

   GenCnt = 0;
   et = 0;
   while (ReadEPDFile ("../test/wac.epd", 0))
   {
      gettimeofday (&t1, NULL);
      for (i = 0; i < 100000; i++)
      {
	 TreePtr[2] = TreePtr[1];
         GenNonCaptures (1);
      }
      gettimeofday (&t2, NULL);
      et += (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1e6;
      printf ("Time = %f\n", et);
   }
   printf ("No. of moves generated = %lu\n", GenCnt);
   printf ("Time taken = %f\n", et);
   if (et > 0)
      printf ("Rate = %f\n", GenCnt / et);  
}


void TestCaptureGenSpeed (void)
/**************************************************************************
 *
 *   This routine benchmarks the speed of the bitmap move generation
 *   for captures.
 *   The test case is BK.epd, the 24 positions from the Brat-Kopec test
 *   suite.
 *
 **************************************************************************/
{
   unsigned long i;
   struct timeval t1, t2;
   double et;

   GenCnt = 0;
   et = 0;
   while (ReadEPDFile ("../test/wac.epd", 0))
   {
      gettimeofday (&t1, NULL);
      for (i = 0; i < 200000; i++)
      {
	 TreePtr[2] = TreePtr[1];
         GenCaptures (1);
      }
      gettimeofday (&t2, NULL);
      et += (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1e6;
      printf ("Time = %f\n", et);
   }
   printf ("No. of moves generated = %lu\n", GenCnt);
   printf ("Time taken = %f\n", et);
   if (et > 0)
      printf ("Rate = %f\n", GenCnt / et);  
}


void TestMoveList (void)
/****************************************************************************
 *  
 *  This routine reads in a *.epd file (EPD notation) and prints the legal
 *  moves for that position.  
 *
 ****************************************************************************/
{
   while (ReadEPDFile ("TEST/GMG1.epd", 0))
   {
      ShowBoard (); 
      GenCnt = 0;
      TreePtr[2] = TreePtr[1];
      GenMoves (1);
      FilterIllegalMoves (1);
      ShowMoveList (1); 
      printf ("No. of moves generated = %lu\n\n", GenCnt);
   }
}


void TestNonCaptureList (void)
/****************************************************************************
 *  
 *  This routine reads in a *.epd file (EPD notation) and prints the 
 *  non-capturing moves for that position.  
 *
 ****************************************************************************/
{
   while (ReadEPDFile ("TEST/GMG1.epd", 0))
   {
      ShowBoard (); 
      GenCnt = 0;
      TreePtr[2] = TreePtr[1];
      GenNonCaptures (1);
      FilterIllegalMoves (1);
      ShowMoveList (1); 
      printf ("No. of moves generated = %lu\n\n", GenCnt);
   }
}


void TestCaptureList (void)
/****************************************************************************
 *  
 *  This routine reads in a *.epd file (EPD notation) and prints the capture
 *  moves for that position.  
 *
 ****************************************************************************/
{
   while (ReadEPDFile ("TEST/GMG1.epd", 0))
   {
      ShowBoard (); 
      GenCnt = 0;
      TreePtr[2] = TreePtr[1];
      GenCaptures (1);
      FilterIllegalMoves (1);
      ShowMoveList (1); 
      printf ("No. of moves generated = %lu\n\n", GenCnt);
   }
}


#define NEVALS 30000

void TestEvalSpeed (void)
/***************************************************************************
 *
 *  This routine reads in the BK.epd and test the speed of the 
 *  evaluation routines.
 *
 ***************************************************************************/
{
   unsigned long i;
   struct timeval t1, t2;
   double et;

   et = 0;
   EvalCnt = 0;
   while (ReadEPDFile ("../test/wac.epd", 0))
   {
      gettimeofday (&t1, NULL);
      for (i = 0; i < NEVALS; i++)
      {
         (void) Evaluate (-INFINITY, INFINITY);
      }
      gettimeofday (&t2, NULL);
      et += (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec)/1e6;
      printf ("Time = %f\n", et);
   }
   printf ("No. of positions evaluated = %lu\n", EvalCnt);
   printf ("Time taken = %f\n", et);
   if (et > 0)
      printf ("Rate = %f\n", EvalCnt / et);  
}



void TestEval (void)
/**************************************************************************
 *
 *  To test the evaluation routines,  read from the BK.epd test file.
 *  Print out the score.  This can be improved by being more verbose
 *  and printing out salient features of the board, e.g. King safety,
 *  double bishops, rook on seventh rank, weak pawns, doubled pawns,
 *  bad bishops, passwd pawns, etc etc.
 *
 ***************************************************************************/
{
   int score;

   SET (flags, TESTT);
   while (ReadEPDFile ("../test/wac.epd", 0))
   {
      ShowBoard ();
      score = Evaluate (-INFINITY, INFINITY);
      printf (board.side == white ? "W : " : "B : ");
      printf ("score = %d\n\n", score);
   }
   CLEAR (flags, TESTT);
}

