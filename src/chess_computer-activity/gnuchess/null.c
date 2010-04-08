/* GNU Chess 5.0 - null.c - null move code
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

void MakeNullMove (int side)
/*****************************************************************************
 *
 *  Makes a null move on the board and update the various game information.
 *
 *****************************************************************************/
{
   GameRec *g;

   GameCnt++;
   g = &Game[GameCnt];
   g->epsq = board.ep;
   g->bflag = board.flag;
   g->hashkey = HashKey;
   if (board.ep > -1)
      HashKey ^= ephash[board.ep];
   HashKey ^= Sidehash; 
   board.ep = -1;
   board.side = 1^side;

   /* Update game record */
   g->move = NULLMOVE;
   return;
}


void UnmakeNullMove (int side)
/****************************************************************************
 *
 *  Undoing a null move.
 *
 ****************************************************************************/
{
   GameRec *g;

   side = 1^side;
   board.side = side;
   g = &Game[GameCnt];
   board.ep = g->epsq;
   board.flag = g->bflag;
   HashKey = g->hashkey;
   GameCnt--;
   return;
}
