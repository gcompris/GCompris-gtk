/* GNU Chess 5.0 - random.c - random number code
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

uint32_t Rand32 (void)
/****************************************************************************
 *
 *  A 32 bit random number generator.  An implementation in C of the 
 *  algorithm given by Knuth, the art of computer programming, vol. 2,
 *  pp. 26-27.  We use e=32, so we have to evaluate y(n) = y(n-24) + y(n-55)
 *  mod 2^32, which is implicitly done by unsigned arithmetic. 
 *
 ****************************************************************************/
{
/* 
 *  Random numbers from Mathematica 2.0
 *  SeedRandom = 1;
 *  Table[Random[Integer, {0, 2^32 - 1}]
 */
   static uint32_t x[55] =  
   {
	1410651636UL,
	3012776752UL,
	3497475623UL,
	2892145026UL,
	1571949714UL,
	3253082284UL,
	3489895018UL,
	387949491UL,
	2597396737UL,
	1981903553UL,
	3160251843UL,
	129444464UL,
	1851443344UL,
	4156445905UL,
	224604922UL,
	1455067070UL,
	3953493484UL,
	1460937157UL,
	2528362617UL,
	317430674UL,
	3229354360UL,
	117491133UL,
	832845075UL,
	1961600170UL,
	1321557429UL,
	747750121UL,
	545747446UL,
	810476036UL,
	503334515UL,
	4088144633UL,
	2824216555UL,
	3738252341UL,
	3493754131UL,
	3672533954UL,
	29494241UL,
	1180928407UL,
	4213624418UL,
	33062851UL,
	3221315737UL,
	1145213552UL,
	2957984897UL,
	4078668503UL,
	2262661702UL,
	65478801UL,
	2527208841UL,
	1960622036UL,		
	315685891UL,
	1196037864UL,
	804614524UL,
	1421733266UL,
	2017105031UL,
	3882325900UL,
	810735053UL,
	384606609UL,
	2393861397UL
   };
   static int init = true;
   static uint32_t y[55];
   static int j, k;
   uint32_t ul;

   if (init)
   {
      int i;
      init = false;
      for (i = 0; i < 55; i++)
         y[i] = x[i];
      j = 24 - 1;
      k = 55 - 1;
   }
   ul = (y[k] += y[j]);
   if (--j < 0) j = 55 - 1;
   if (--k < 0) k = 55 - 1;
   return (ul);
}

HashType Rand64 (void)
{
  HashType b;

  b = Rand32();
  b = b << 32;
  b |= Rand32 ();
  return (b);
}
