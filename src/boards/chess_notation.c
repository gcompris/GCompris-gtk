/* 
 * Copyright (C) 1999,2001 Robert Wilhelm
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "chess_position.h"
#include "chess_notation.h"

static char piece_to_ascii_t[]= {' ','N','B','R','Q','K'};

static int norm_piece (Piece piece);

static void
file_to_ascii (char **move, Square square)
{
	*(*move)++ = square - square / 10 * 10 + 96;	/* a - h */
}

static void
rank_to_ascii (char **move, Square square)
{
	*(*move)++ = square / 10 + 47 ;			/* 1 - 8 */
}

static void
square_to_ascii(char **move, Square square)
{
	file_to_ascii (move, square);
	rank_to_ascii (move, square);
}


static int
same_rank (Square square, Square square2)
{
	char *s1;
	char *s2;
	char same1;
	char same2;

	s1 = &same1;
	s2 = &same2;
	rank_to_ascii (&s1, square);
	rank_to_ascii (&s2, square2);

	if (same1 == same2)
		return 1;

	return 0;
}

static int
same_file (Square square, Square square2)
{
	char *s1;
	char *s2;
	char same1;
	char same2;

	s1 = &same1;
	s2 = &same2;
	file_to_ascii (&s1, square);
	file_to_ascii (&s2, square2);
	if (same1 == same2)
		return 1;

	return 0;
}


static void 
delete_x (char *str)
{
	char *p = strchr (str, 'x');

	if (p)
		while ((*p = *(p+1)))
			p++;
}

static void 
delete_plus (char *str)
{
	char *p = strchr (str, '+');

	if (p)
		while ((*p = *(p+1)))
			p++;
}

static void 
delete_ep (char *str)
{
	char *p = strstr (str, "ep");

	if (p)
		while ((*p = *(p+2)))
			p++;
}

static void 
delete_equal (char *str)
{
	char *p = strstr (str, "=");

	if (p)
		while ((*p = *(p+1)))
			p++;
}

static void 
delete_hash (char *str)
{
	char *p = strstr (str, "#");

	if (p)
		while ((*p = *(p+1)))
			p++;
}

char *
move_to_ascii (char *p, Square from, Square to)
{
	Square a;

	file_to_ascii (&p, from);
	rank_to_ascii (&p, from);

	if (to & 128) {
                /* promotion */
		a = to;

		if (from > E4) 
			a = (a & 7) + A8; /* white */
		else
			a = (a & 7) + A1; /* black */

		*p++    = a - a / 10 * 10 + 96;     /*  a - h       */
		*p++    = a / 10 + 47 ;             /*  1 - 8       */
		*p++    = '=';
		*p++ = piece_to_ascii_t[((to >> 3) & 7)-1];
	} else {
		file_to_ascii (&p, to);
		rank_to_ascii (&p, to);
	}

	*p = '\0';

	return p;
}

int
ascii_to_piece (char p)
{
	if (p == 'q')
		return WQ-WP;
	if (p == 'r')
		return WR-WP;
	if (p == 'b')
		return WB-WP;
	if (p == 'n')
		return WN-WP;
	if (p == 'Q')
		return WQ-WP;
	if (p == 'R')
		return WR-WP;
	if (p == 'B')
		return WB-WP;
	if (p == 'N')
		return WN-WP;

	g_assert_not_reached ();

	return -1;
}

char
piece_to_ascii (int piece)
{
  int i;

  i = norm_piece (piece);

  if(WPIECE(piece))
    return piece_to_ascii_t[i];
  else
    return tolower(piece_to_ascii_t[i]);

  g_assert_not_reached ();
  
  return -1;
}

void
ascii_to_move (Position *pos, char *p, Square *from, Square *to)
{
	delete_x (p);

	if (*p == 'o') {
		/* Castling */
		if (!strcmp (p, "o-o-o")) {
			if (position_get_color_to_move (pos) == WHITE) {
				*from = E1;
				*to   = C1;
			} else {
				*from = E8;
				*to   = C8;
			}
		} else {
			if (position_get_color_to_move (pos) == WHITE) {
				*from = E1;
				*to   = G1;
			} else {
				*from = E8;
				*to   = G8;
			}
		}
		return;
	}

	*from = (*p - 'a' + 1) + (*(p + 1) - '1' + 2 ) * 10;
	p += 2;
	*to = (*p - 'a' + 1) + (*(p + 1) - '1' + 2 ) * 10;
	p += 2;
	
	if (*p == 'q' || *p == 'r' || *p == 'b' || *p =='n' ||
	    *p =='Q' || *p =='R' || *p == 'B' || *p == 'N' ) { 
                /* Promotion */
		if (*to < A2)
			*to = 128 +  *to - A1 + (ascii_to_piece (*p) + 1) * 8;
		else if (*to > A7)
			*to = 128 +  *to - A8 + (ascii_to_piece (*p) + 1) * 8;
		else
			g_assert_not_reached ();
	}
}

int 
san_to_move (Position *pos, char *str, Square *from, Square *to)
{
	Square zugliste[AB_ZUGL];
	Square *ap, *aq;
	gshort anz, anz_n, anz_s;
	gshort i;
	gchar *p;
	gchar liste[100][10];

	delete_x(str);
	delete_plus(str);
	delete_ep(str);
	delete_equal(str);
	delete_hash(str);

	ap =  zugliste + AB_ZUG_S;
	anz = position_legal_move (pos, &ap, &anz_s, &anz_n);

	for (aq = ap, i = 0; i < anz; i++, aq += 2) {
		p = liste[i];
		piece_move_to_ascii (p, pos->square[*aq], *aq, *(aq + 1));

		if (*p == ' ') {
                        /* pawn move */

                        /* e.g. e2e4 */
			p++;
			if (!strcmp (p, str)) {
				*from = *aq;
				*to   = *(aq+1);
				return 0;
			}

                        /* e.g. ed5 */			
			p[1]=p[2];  
			p[2]=p[3];
			p[3]=p[4];
			p[4]=p[5];

                        /* not e.g. bb3 */
			if (p[0] != p[1])   
				if (!strcmp(p,str)) {
					*from = *aq;
					*to   = *(aq+1);
					return 0;
				}

                        /* e.g. d5 */
			p++;
			if (!strcmp(p,str)) {
				*from = *aq;
				*to   = *(aq+1);
				return 0;
			}

		} else {
			/* no pawn move */
			char  tmp;

                        /* e.g. Ng1f3 */
			if (!strcmp (p, str)) {
				*from = *aq;
				*to   = *(aq+1);
				return 0;
			}

                        /* Ngf3 */
			tmp =p[2];
			p[2]=p[3];        
			p[3]=p[4];
			p[4]=p[5];

			if (!strcmp(p,str)) {
				*from = *aq;
				*to   = *(aq+1);
				return 0;
			}

			/* N1f3 */
			p[1]=tmp;

			if (!strcmp(p,str)) {
				*from = *aq;
				*to   = *(aq+1);
				return 0;
			}

			/* Nf3 */
			p[1]=p[2];
			p[2]=p[3];
			p[3]=p[4];

			if (!strcmp(p,str)) {
				*from = *aq;
				*to   = *(aq+1);
				return 0;
			}
		}
	}

	return 1;
}

static int
norm_piece (Piece piece)
{
	if (WPIECE (piece))
		return piece - WP;
	if (BPIECE (piece))
		return piece - BP;

	return piece;
}

void
piece_move_to_ascii (char *p, Piece piece, Square from, Square to)
{
	int i;

        if ((piece == WK || piece == BK) && abs (from - to) == 2) {
		if (to % 10 == 3) {
			strcpy (p,"O-O-O");
			return;
		}
		if (to % 10 == 7) {
			strcpy (p,"O-O");
			return;
		}
		g_assert_not_reached ();
	}

        i = norm_piece (piece);
        *p++ = piece_to_ascii_t[i];
        move_to_ascii (p, from, to);
}

char *
move_to_san (Position *pos, Square from, Square to)
{
	Square checksquare, checkto;
	Piece piece, promote;
	int norm, desrank, desfile, inc;
	int i, tempdesfile, tempdesrank;
	char *san;
	char *temp;
	const int jump[]={ 8, 12,19, 21,-8,-12,-19,-21};

	san = g_new0 (char, 12);
	temp = san;

	desrank = desfile = promote = 0;

	/* Handle Promotion */
	if (to & 128) {
		promote = ((to >> 3) & 7)-1;
     		if ( from > E4) {
			to = (to & 7) + A8;
			piece = WP;
		} else {
			to = (to & 7) + A1;
			piece = BP;
		}
	} else {
		piece = pos->square[to];
	}

	/* Check if we have to designate the rank or file */
	switch (piece) {
	case WQ:
	case BQ:
		/* Check like rooks and bishops */
	case WR:
	case BR:
		/* Check for other rooks/queens */
		i = 0;
		while (1) {
			tempdesfile = tempdesrank = 0;
			if (i == 0) {
				checksquare = 20 + (to % 10);
				checkto =  90 + (to % 10);
				if (from / 10 > to / 10 )
					checkto = to - 10;
				else if (from / 10 < to /10)
					checksquare = to + 10;
				inc = 10;
			} else if (i == 1) {
				checksquare = 10 * (to / 10) + 1;
				checkto =  10 * (to / 10) + 8;
				if (from % 10 > to % 10)
					checkto = to - 1;
				else if (from % 10 < to % 10)
					checksquare = to + 1;
				inc = 1;
			} else {
				break;
			}

			while (checksquare <= checkto) {
				if (pos->square[checksquare] == piece && checksquare != to) {
					if (same_rank(from, checksquare))
						tempdesfile = 1;
					else if (same_file(from, checksquare))
						desrank = 1;
					else 
						tempdesfile = 1;
				} else if (pos->square[checksquare] != EMPTY && checksquare < to) {
					tempdesfile = tempdesrank = 0;	/* A piece is in the way */
				} else if (pos->square[checksquare] != EMPTY && checksquare > to) {
					break;			/* A piece is in the way */
				}
				checksquare += inc;
			}
			i++;
			if (tempdesfile == 1) desfile = 1;
			if (tempdesrank == 1) desrank = 1;
		}
		if (piece == WR || piece == BR) break;
	case WB:
	case BB:
		/* Check for other bishops/queens */
		i = 0;
		while (1) {
			tempdesfile = tempdesrank = 0;
			if (i == 0) {
				checksquare = to - (((to % 10) - 1) * 11);
				checkto =  to + ((9 - (to / 10)) * 11);
				if (from % 10 > to % 10  && from / 10 > to / 10)
					checkto = to - 11;
				else if (from % 10 < to % 10 && from / 10 < to / 10)
					checksquare = to + 11;
				inc = 11;
			} else if (i == 1) {
				checksquare = to - ((8 - (to % 10)) * 9);
				checkto =  to + ((9 - (to / 10)) * 9);
				if (from % 10 > to % 10 && from / 10 < to / 10)
					checksquare = to + 9;
				else if (from % 10 < to % 10  && from / 10 > to / 10)
					checkto = to - 9;
				inc = 9;
			} else {
				break;
			}

			while (checksquare <= checkto) {
				if (pos->square[checksquare] == piece && checksquare != to) {
					if (same_rank(from, checksquare))
						tempdesfile = 1;
					else if (same_file(from, checksquare))
						desrank = 1;
					else 
						tempdesfile = 1;
				} else if (pos->square[checksquare] != EMPTY && checksquare < to) {
					tempdesfile = tempdesrank = 0;	/* A piece is in the way */
				} else if (pos->square[checksquare] != EMPTY && checksquare > to) {
					break;			/* A piece is in the way */
				}
				checksquare += inc;
			}
			i++;
			if (tempdesfile == 1) desfile = 1;
			if (tempdesrank == 1) desrank = 1;
		}
		break;
	case WN:
	case BN:
		/* Check for other knights */
		for (i=0;i<8;i++) {
			if (pos->square[to+jump[i]] == piece && to+jump[i] >= 0) {
				if (same_rank(from, to+jump[i]))
					desfile = 1;
				else if (same_file(from, to+jump[i]))
					desrank = 1;
				else 
					desfile = 1;
			}
		}
		break;				

	}

	/* Handle Castling */
	if ((piece == WK || piece == BK) && abs(from-to) == 2) {
		if (to % 10 == 3)
			strcpy(temp,"O-O-O");
		if (to % 10 == 7)
			strcpy(temp,"O-O");
	} else {
		/* The piece letter */
	        norm = norm_piece(piece);
		if (norm > 0)
		        *temp++ = piece_to_ascii_t[norm];

		/* The rank/file designators */
		if (desfile)
			file_to_ascii(&temp, from);
		if (desrank)
			rank_to_ascii(&temp, from);

		/* If there was a capture */
		if (position_last_piece_captured (pos) != EMPTY) {
			if (piece == WP || piece == BP)
				file_to_ascii (&temp, from);
			*temp++ = 'x';
		}

		/* Destination square */
	        square_to_ascii (&temp, to);

		/* If there was promotion */
		if (promote) {
			*temp++ = '=';
			norm = norm_piece(promote);
			*temp++ = piece_to_ascii_t[norm];
		}

		*temp = '\0';
	}

	temp = san;
	san = g_strdup (temp);
	g_free (temp);
	return san;
}
