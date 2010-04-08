/* GNU Chess 5.0 - solve.c - position solving code
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
#include <string.h>
#include "common.h"


void Solve (char *file)
/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
{
   int total, correct, found;
   long TotalNodes;
   char *p;

   total = correct = 0;
   TotalNodes = 0;
   SET (flags, SOLVE); 
   while (ReadEPDFile (file, 0))
   {
      NewPosition ();
      total++;
      ShowBoard (); 
      Iterate ();
      TotalNodes += NodeCnt + QuiesCnt;
      p = solution;
      found = false;
      while (*p != '\0')
      {
         if (!strncmp (p, SANmv, strlen(SANmv)))
         {
	    correct++;
	    found = true;
	    break;
         }
         while (*p != ' ' && *p != '\0') p++;
	 while (*p == ' ' && *p != '\0') p++;
      }
      printf ("%s : ", id);
      printf (found ? "Correct:  " : "Incorrect:  ");
      printf ("%s %s\n", SANmv, solution);
      printf ("Correct=%d Total=%d\n", correct, total);
   }
   printf ("\nTotal nodes = %ld\n", TotalNodes);
   CLEAR (flags, SOLVE);
}

