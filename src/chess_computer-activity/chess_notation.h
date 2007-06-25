/* 
 * Copyright (C) 1999 Robert Wilhelm
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef _NOTATION_H_
#define _NOTATION_H_

#include "chess_makros.h"
#include "chess_position.h"

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

/* Convert moves to and from strings */
void piece_move_to_ascii (char *p, Piece piece, Square from, Square to);
char * move_to_ascii (char *p, Square from, Square to);
void ascii_to_move (Position *pos, char *p, Square *from, Square *to);
int  ascii_to_piece (char p);
char piece_to_ascii (int piece);
void square_to_ascii(char **move, Square square);

/* Standard Algebraic Notation Functions */
char * move_to_san (Position *pos, Square from, Square to);
int san_to_move (Position *pos, char *p, Square *from, Square *to);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NOTATION_H_ */
