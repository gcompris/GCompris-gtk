/* GNU Chess 5.0 - eval.h - evaluation symbolic definitions
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

#ifndef EVAL_H
#define EVAL_H

/****************************************************************************
 *
 *  The various evaluations constants.
 *
 ****************************************************************************/

#define OPENING	     (phase <= 2)
#define ENDING       (phase >= 6)
#define TRADEPIECE   4
#define TRADEPAWNS   8
#define HUNGPENALTY  -20
#define ROOKMOVED    -20
#define KINGMOVED    -20
#define NOTCASTLED   -8
#define PFACTOR      550
#define EARLYMINORREPEAT -7

/***********/
/*  Pawns  */
/***********/
#define EIGHT_PAWNS	-10
#define STONEWALL	-10
#define LOCKEDPAWNS	-10
#define EARLYWINGPAWNMOVE -6
#define EARLYCENTERPREPEAT -6
#define BACKWARDPAWN   -(8+phase)
#define DOUBLEDPAWN    -(8+phase)
#define PAWNBASEATAK   -18
#define BLOCKDEPAWN    -48 /* -12 */
#define CONNECTEDPP	50
#define PAWNNEARKING    40
#define ATAKWEAKPAWN    2
#define FIANCHETTO_TARGET -13


/*************/
/*  Knights  */
/*************/
#define KNIGHTONRIM	-13
#define	OUTPOSTKNIGHT    10
#define PINNEDKNIGHT    -30
#define KNIGHTTRAPPED   -250


/*************/
/*  Bishops  */
/*************/
#define DOUBLEDBISHOPS  18
#define	OUTPOSTBISHOP    8
#define FIANCHETTO	 8
#define GOODENDINGBISHOP 16
#define BISHOPTRAPPED   -250
#define PINNEDBISHOP	-30

/***********/
/*  Rooks  */
/***********/
#define	ROOK7RANK      30
#define ROOKS7RANK     30
#define ROOKHALFFILE   5
#define ROOKOPENFILE   6
#define ROOKBEHINDPP   6
#define ROOKINFRONTPP -10
#define PINNEDROOK    -50
#define ROOKTRAPPED   -10
#define ROOKLIBERATED 40

/***********/
/*  Queen  */
/***********/
#define EARLYQUEENMOVE  -40
#define QUEENNEARKING   12
#define PINNEDQUEEN    -90
#define QUEEN_NOT_PRESENT -25

/**********/
/*  King  */
/**********/
#define KINGOPENFILE  -10
#define KINGOPENFILE1 -6
#define ATAKKING       10
#define DEFENDKING     6
#define HOPEN	      -600
#define GOPEN	      -30
#define KING_DEFENDER_DEFICIT -50
#define KING_BACK_RANK_WEAK -40
#define RUPTURE -20

#endif /* !EVAL_H */
