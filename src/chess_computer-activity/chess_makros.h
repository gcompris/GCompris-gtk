/*
 * Copyright (C) 1999 Robert Wilhelm
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MAKROS_H_
#define _MAKROS_H_

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

/* Types */
typedef gushort Square;
typedef gchar   Piece;

/* Various empty defines */
#define NONE		0

/* Generated movelist */
#define AB_ZUGL		512	/*  Length of list generated moves	*/
#define AB_ZUG_S	256	/*  Beginning of generated capture moves*/

/* Colours */
#define WHITE		1	/*  White to Move			*/
#define BLACK		129	/*  Black to move			*/

/* Notation types */
#define	SAN		0	/*  Standard algebraic notation		*/
#define	COORD		1	/*  Coordinate notation			*/

/* Game results */
#define	GAME_NONE	0	/*  No game			        */
#define	GAME_PAUSED	1	/*  Game is paused			*/
#define	GAME_PROGRESS	2	/*  Game is in progress			*/
#define	GAME_ABORTED	3	/*  Game was aborted			*/
#define	GAME_WHITE	4	/*  White won the game			*/
#define	GAME_BLACK	5	/*  Black won the game			*/
#define	GAME_DRAW	6	/*  Draw			        */

/* Board window modes */
#define	COMPUTER_BLACK	1
#define	COMPUTER_WHITE	2
#define	AUTOPLAY	3
#define	ANALYSE		4
#define	HUMANS		5

#define  OL            9     /********************************************/
#define  OM           10     /*                                          */
#define  OR           11     /*            OL |  OM  | OR                */
#define  LI           -1     /*           ----+------+----               */
#define  RE            1     /*            LI | FELD | RE                */
#define  UL          -11     /*           ----+------+----               */
#define  UM          -10     /*            UL |  UM  | UR                */
#define  UR           -9     /********************************************/

/*************************************************************************/
/*                         Figuren      Pieces                           */
/*************************************************************************/
/*                                                                       */
/*      weiss   white  32 + x      x = 1  Bauer     Pawn                 */
/*      schwarz black  64 + x          2  Springer  Knight               */
/*      leer    empty  0               3  Laeufer   Bishop               */
/*      rand    border 16              4  Turm      Rook                 */
/*                                     5  Dame      Queen                */
/*                                     6  Koenig    King                 */
/*                                                                       */
/*************************************************************************/

#define WP		33	 /*  white pawn   */
#define WN		34	 /*  white knight */
#define WB		35	 /*  white bishop */
#define WR		36	 /*  white rook   */
#define WQ		37	 /*  white queen  */
#define WK		38	 /*  white king   */
#define BP		65	 /*  black pawn   */
#define BN		66	 /*  black knight */
#define BB		67	 /*  black bishop */
#define BR		68	 /*  black rook   */
#define BQ		69	 /*  black queen  */
#define BK		70	 /*  black king	  */
#define EMPTY     0              /*  empty	  */
#define BORDER    16             /*  border (illegal) */

#define WPIECE(c) ((c) & 32)   /* white piece? */
#define BPIECE(c) ((c) & 64)   /* black piece? */

/****************************************************************/
/*                      Squares                                 */
/****************************************************************/

#define A1 21
#define B1 22
#define C1 23
#define D1 24
#define E1 25
#define F1 26
#define G1 27
#define H1 28
#define A2 31
#define B2 32
#define C2 33
#define D2 34
#define E2 35
#define F2 36
#define G2 37
#define H2 38
#define A3 41
#define B3 42
#define C3 43
#define D3 44
#define E3 45
#define F3 46
#define G3 47
#define H3 48
#define A4 51
#define B4 52
#define C4 53
#define D4 54
#define E4 55
#define F4 56
#define G4 57
#define H4 58
#define A5 61
#define B5 62
#define C5 63
#define D5 64
#define E5 65
#define F5 66
#define G5 67
#define H5 68
#define A6 71
#define B6 72
#define C6 73
#define D6 74
#define E6 75
#define F6 76
#define G6 77
#define H6 78
#define A7 81
#define B7 82
#define C7 83
#define D7 84
#define E7 85
#define F7 86
#define G7 87
#define H7 88
#define A8 91
#define B8 92
#define C8 93
#define D8 94
#define E8 95
#define F8 96
#define G8 97
#define H8 98

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MAKROS_H_ */

