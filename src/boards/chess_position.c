/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* engine.h
 *
 * Copyright (C) 1999  Robert Wilhelm
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: Robert Wilhelm
 *          JP Rosevear
 */

#include <stdlib.h>
#include <string.h>
#include "chess_position.h"


struct _PositionPrivate {
	Square tomove;	        /* Color to move */
	gshort wr_a_move;	/* Flag if the white 'a' rook has moved */
	gshort wr_h_move;	/* Flag if the white 'h' rook has moved */
	Square wk_square;	/* The square the white king is on */
	gshort br_a_move;	/* Flag if the black 'a' rook has moved */
	gshort br_h_move;       /* Flag if the black 'h' rook has moved */
	Square bk_square;	/* The square the black king is on */
	gshort ep;	        /* En passent */
	Piece  captured;	/* Last piece captured */
};

/* Move generation variables */
static const int jump [] = { 8, 12,19, 21,-8,-12,-19,-21,     
			     9, 11,-9,-11, 1, 10,-10, -1,     
			     9, 11, 1, 10,-1,  1, 10, -1,
			     -9,-11, 1,-10,-1     };
static Square *nindex, *sindex;

/* Prototypes */
static void class_init (PositionClass *class);
static void init (Position *pos);

static void position_set_empty (Position *pos);

GtkType
position_get_type ()
{
	static guint position_type = 0;

	if (!position_type) {
		GtkTypeInfo position_info = {
			"Position",
			sizeof (Position),
			sizeof (PositionClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(gpointer) NULL,
			(gpointer) NULL,
			(GtkClassInitFunc) NULL
		};
		position_type = gtk_type_unique (gtk_object_get_type (),
						 &position_info);
	}

	return position_type;
}

static void
finalize (GtkObject *object)
{
	Position *pos = (Position *) object;

	g_free (pos->priv);

	pos->priv = NULL;
}

static void
class_init (PositionClass *class)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass*) class;

	//2	object_class->finalize = finalize;
}

static void
init (Position *pos)
{
	pos->priv = g_new0 (PositionPrivate, 1);

	position_set_empty (pos);
}

GtkObject *
position_new ()
{
	return GTK_OBJECT (gtk_type_new (position_get_type ()));
}

GtkObject *
position_new_initial ()
{
	Position *pos;

	pos = POSITION (position_new ());
	position_set_initial (pos);

	return GTK_OBJECT (pos);
}

Position *
position_copy (Position *pos)
{
	Position *cpPos;

	cpPos = POSITION (position_new ());

	memcpy (cpPos->priv, pos->priv, sizeof (PositionPrivate));
	memcpy (cpPos->square, pos->square, sizeof (gchar) * 120);

	return cpPos;
}

static void
position_set_empty (Position *pos)
{
	unsigned int a;            

	for (a = 0 ; a < 120 ; a++)
		pos->square [a] = EMPTY;

	for (a = 0; a < 10; a++) {
		pos->square [a]          = BORDER;
		pos->square [a + 10]     = BORDER;
		pos->square [a + 100]    = BORDER;
		pos->square [a + 110]    = BORDER;
		pos->square [a * 10]     = BORDER;
		pos->square [a * 10 + 9] = BORDER;
	}

	pos->priv->wk_square = 0;
	pos->priv->wr_h_move = 0;
	pos->priv->wr_a_move = 0;
	pos->priv->bk_square = 0;
	pos->priv->br_h_move = 0;
	pos->priv->br_a_move = 0;
	pos->priv->captured = EMPTY;

	pos->priv->tomove = NONE;  
}

void
position_set_initial (Position *pos)
{
	unsigned int a;

	/* The white pieces */
    	pos->square [A1] = WR;
	pos->square [B1] = WN;
	pos->square [C1] = WB;
	pos->square [D1] = WQ;
    	pos->square [E1] = WK;
	pos->square [F1] = WB;
	pos->square [G1] = WN;
	pos->square [H1] = WR;

	/* The black pieces */
	pos->square [A8] = BR;
	pos->square [B8] = BN;
	pos->square [C8] = BB;
	pos->square [D8] = BQ;
	pos->square [E8] = BK;
	pos->square [F8] = BB;
	pos->square [G8] = BN;
	pos->square [H8] = BR;

	/* Pawns on the 2nd and 7th ranks */
	for (a = A2; a <= H2 ;a++) 
		pos->square [a] = WP;
	for (a = A7; a <= H7 ;a++)
		pos->square [a] = BP;

	/* The rest is blank */
	for (a = A3; a <= H3 ;a++)
		pos->square [a] = EMPTY;
	for (a = A4; a <= H4 ;a++)
		pos->square [a] = EMPTY;
	for (a = A5; a <= H5 ;a++)
		pos->square [a] = EMPTY;
	for (a = A6; a <= H6 ;a++)
		pos->square [a] = EMPTY;

	/* White to move, white king on E1 and black king on E8 */
	pos->priv->wr_a_move = 0;
	pos->priv->wr_h_move = 0;
	pos->priv->wk_square = E1;
	pos->priv->br_a_move = 0;
	pos->priv->br_h_move = 0;
	pos->priv->bk_square = E8;
	pos->priv->captured = EMPTY;
	pos->priv->tomove = WHITE;
}

void
position_set_initial_partyend (Position *pos, int level)
{
	unsigned int a;
	register Square square;
	register gshort rank;

	for (rank = 1; rank <= 8; rank++) { 
		for (square = A1 + ((rank - 1) * 10); 
		     square <= H1 + ((rank - 1) * 10);
		     square++) {
		pos->square [square] = EMPTY;
		}
	}

	switch(level) {
	case 1:
	default:
		/* The white pieces */
		pos->square [A1] = WK;
		pos->square [G1] = WQ;
		pos->square [F1] = WQ;
		
		/* The black pieces */
		pos->square [E8] = BK;

		/* The kings */
		pos->priv->wk_square = A1;
		pos->priv->bk_square = E8;
		break;
	case 2:
		/* The white pieces */
		pos->square [E1] = WK;
		pos->square [F1] = WR;
		pos->square [G1] = WR;
		
		/* The black pieces */
		pos->square [A8] = BK;

		/* The kings */
		pos->priv->wk_square = E1;
		pos->priv->bk_square = A8;
		break;
	case 3:
		/* The white pieces */
		pos->square [E1] = WK;
		pos->square [B4] = WR;
		pos->square [B5] = WB;
		
		/* The black pieces */
		pos->square [A1] = BK;

		/* The kings */
		pos->priv->wk_square = E1;
		pos->priv->bk_square = A1;
		break;
	}

	/* White to move */
	pos->priv->wr_a_move = 0;
	pos->priv->wr_h_move = 0;
	pos->priv->br_a_move = 0;
	pos->priv->br_h_move = 0;
	pos->priv->captured = EMPTY;
	pos->priv->tomove = WHITE;
}

void
position_set_initial_movelearn (Position *pos, int level)
{
	unsigned int a;
	register Square square;
	register gshort rank;

	for (rank = 1; rank <= 8; rank++) { 
		for (square = A1 + ((rank - 1) * 10); 
		     square <= H1 + ((rank - 1) * 10);
		     square++) {
		pos->square [square] = EMPTY;
		}
	}

	switch(level) {
	case 1:
	default:
		/* The white pieces */
		pos->square [E1] = WK;
		pos->square [B4] = WR;
		
		/* The black pieces */
		pos->square [A1] = BK;
		pos->square [E7] = BP;

		/* The kings */
		pos->priv->wk_square = E1;
		pos->priv->bk_square = A1;
		break;
	case 2:
		/* The white pieces */
		pos->square [E1] = WK;
		pos->square [G1] = WQ;
		
		/* The black pieces */
		pos->square [A1] = BK;
		pos->square [E7] = BP;

		/* The kings */
		pos->priv->wk_square = E1;
		pos->priv->bk_square = A1;
		break;
	case 3:
		/* The white pieces */
		pos->square [E1] = WK;
		pos->square [C4] = WB;
		pos->square [D4] = WB;
		
		/* The black pieces */
		pos->square [A1] = BK;
		pos->square [E7] = BP;

		/* The kings */
		pos->priv->wk_square = E1;
		pos->priv->bk_square = A1;
		break;
	case 4:
		/* The white pieces */
		pos->square [E1] = WK;
		pos->square [B4] = WN;
		pos->square [B5] = WN;
		
		/* The black pieces */
		pos->square [A1] = BK;
		pos->square [E7] = BP;

		/* The kings */
		pos->priv->wk_square = E1;
		pos->priv->bk_square = A1;
		break;
	case 5:
		/* The white pieces */
		pos->square [E1] = WK;
		pos->square [D2] = WP;
		pos->square [E2] = WP;
		pos->square [F2] = WP;
		pos->square [G2] = WP;
		pos->square [H2] = WP;
		
		/* The black pieces */
		pos->square [A1] = BK;
		pos->square [E7] = BP;

		/* The kings */
		pos->priv->wk_square = E1;
		pos->priv->bk_square = A1;
		break;
	}

	/* White to move */
	pos->priv->wr_a_move = 0;
	pos->priv->wr_h_move = 0;
	pos->priv->br_a_move = 0;
	pos->priv->br_h_move = 0;
	pos->priv->captured = EMPTY;
	pos->priv->tomove = WHITE;
}

/* Debug purpose */
void
position_display (Position *pos)
{
  register Square square;
  register Piece piece;
  register gshort rank;

  for (rank = 8; rank >= 1; rank--) { 
    for (square = A1 + ((rank - 1) * 10); 
	 square <= H1 + ((rank - 1) * 10);
	 square++) {

	    printf("%c ", piece_to_ascii(pos->square [square]));
    }
    printf("\n");
  }
}
/* 
 * Move Functions
 *
 * A set of functions to make a move in the context of the position 
 * passed in.
 */
static void
position_move_white_castle_short (Position *pos)
{
        pos->square [E1] = pos->square [H1] = EMPTY;
        pos->square [F1] = WR;
        pos->square [G1] = WK;
}

static void
position_move_white_castle_long (Position *pos)
{
        pos->square [E1] = pos->square [A1] = EMPTY;
        pos->square [C1] = WK;
        pos->square [D1] = WR;
}

static void
position_move_black_castle_short (Position *pos)
{
        pos->square [E8] = pos->square [H8] = EMPTY;
        pos->square [F8] = BR;
        pos->square [G8] = BK;
 }

static void
position_move_black_castle_long (Position *pos)
{
        pos->square [E8] = pos->square [A8] = EMPTY;
        pos->square [C8] = BK;
        pos->square [D8] = BR;
}

static void
position_move_white (Position *pos, Square from, Square to)
{
	Piece piece;
	Square new_to;

	piece = pos->square [from];

	switch (piece) {
	case WP :
		/* If we are promoting a pawn */
		if (to & 128) {
			new_to = (to & 7) + A8;            
			piece = ((to & 127) >> 3 ) + WP - 1;

			pos->priv->captured = pos->square [new_to];
			pos->square [from]  = EMPTY;
			pos->square [new_to] = piece;
			pos->priv->ep = EMPTY;
			return;
		}

		/* If we are capturing en passent */
		if ((to - from) != 10) {
			if((to - from) != 20) {
				if(pos->square [to] == EMPTY) {
					pos->square [to - 10] = EMPTY;
					pos->priv->ep = EMPTY;
					pos->square [to] = piece;
					pos->square [from] = EMPTY;
					pos->priv->captured = EMPTY;
					return;
				}
			}
		}

		pos->priv->captured = pos->square [to];
		pos->square [to]   = piece;
		pos->square [from] = EMPTY;

		if ((to - from) == 20)
			pos->priv->ep = to;
		else  
			pos->priv->ep = EMPTY;
		return;

	case WK :
		pos->priv->ep = EMPTY;
		pos->priv->wk_square = to;
		pos->priv->wr_a_move += 1;
		pos->priv->wr_h_move += 1;

		/*  If we are not castling */
		if (from != E1 || abs (to - from) != 2) {
			pos->priv->captured = pos->square [to];
			pos->square [to]   = piece; 
			pos->square [from] = EMPTY;
			return;
		}

		/*  If we are castling */
		switch (to) {
		case G1 :
			position_move_white_castle_short (pos);
			break;  
		case C1 :
			position_move_white_castle_long (pos);
			break;  
		default :       
			abort ();
		}
		return;

	default :
		pos->priv->ep = EMPTY;
		pos->priv->captured = pos->square [to];
		pos->square [to] = piece;
		pos->square [from]  = EMPTY;

		/* Indicate if a rook has moved */
		if (piece == WR && from == A1)
			pos->priv->wr_a_move += 1;
		if (piece == WR && from == H1)
			pos->priv->wr_h_move += 1;
		return;
	}
}

static void
position_move_black (Position *pos, Square from, Square to)
{
	gshort piece, new_to;

	piece = pos->square [from];

	switch (piece) {
	case BP :
		/* If we are promoting a pawn */
		if (to & 128) {
			new_to = (to & 7) + A1;
			piece = ((to & 127) >> 3) + BP - 1;

			pos->priv->captured = pos->square [new_to];
			pos->square [from]  = EMPTY;
			pos->square [new_to] = piece;
			pos->priv->ep = EMPTY;
			return;
		}

		/* If we are capturing en passent */
		if ((from - to) != 10) {
			if ((from - to) != 20) {
				if (pos->square [to] == EMPTY) {
					pos->square [to + 10] = EMPTY;
					pos->priv->ep = EMPTY;
					pos->square [to] = piece;
					pos->square [from] = EMPTY;
					pos->priv->captured = EMPTY;
					return;
				}
			}
		}

		pos->priv->captured = pos->square [to];
		pos->square [to] = piece;
		pos->square [from] = EMPTY;

		if ((from - to) == 20)
			pos->priv->ep = to;
		else                   
			pos->priv->ep = EMPTY;	
		return;

	case BK :
		pos->priv->ep = EMPTY;
		pos->priv->bk_square = to;
		pos->priv->br_a_move += 1;
		pos->priv->br_h_move += 1;

		/*  If we are not castling */
		if (from != E8 || abs (to - from) != 2) {
			pos->priv->captured = pos->square [to];
			pos->square [to] = piece;
			pos->square [from]  = EMPTY;
			return;
		}

		/*  If we are castling */
		switch (to) {
		case G8 :       
			position_move_black_castle_short (pos);
			break; 
		case C8 :
			position_move_black_castle_long (pos);
			break; 
		default :
			abort();
		}
		return;

	default :
		pos->priv->ep = EMPTY;
		pos->priv->captured = pos->square [to];
		pos->square [to]   = piece;
		pos->square [from] = EMPTY;

		/* Indicate if a rook has moved */
		if (piece == BR && from == A8)
			pos->priv->br_a_move += 1;
		if (piece == BR && from == H8)
			pos->priv->br_h_move += 1;
		return;
	}
}

void
position_move (Position *pos, Square from, Square to)
{
	switch (pos->priv->tomove) {
		case WHITE:
			position_move_white (pos, from, to);
			pos->priv->tomove = BLACK;
			break;
		case BLACK:
			position_move_black (pos, from, to);
			pos->priv->tomove = WHITE;
			break;
        	default:
			g_assert_not_reached ();
	}
}

static Square
position_move_normalize_promotion (Position *pos, Square to, gshort n1,
				   gshort n2, gshort n3, gshort n4)
{
	Square help;
	Piece fi = EMPTY;
	gshort c;

        if  (pos->priv->tomove == WHITE)
		help = to - A8;
        else
		help = to - A1;

        if (help == (n1 & 7)) {
		/* Automatic queen promotion prefs_get_promotetoqueen */
		c = 0;

		if (pos->priv->tomove == WHITE) 
			switch (c) {
			case 0 : fi = WQ; break;
			case 1 : fi = WR; break;
			case 2 : fi = WB; break;
			case 3 : fi = WN; break;
			}
		else 
			switch (c) {
			case 0 : fi = BQ; break;
			case 1 : fi = BR; break;
			case 2 : fi = BB; break;
			case 3 : fi = BN; break;
			}

		if (pos->priv->tomove == WHITE)
			help = 128 + to - A8 + (fi - WP + 1) * 8;
		else
			help = 128 + to - A1 + (fi - BP + 1) * 8;

		if (help == n1)
			return n1;
		if (help == n2)
			return n2;
		if (help == n3)
			return n3;
		if (help == n4)
			return n4;
	  
        }
        return FALSE;
}

Square
position_move_normalize (Position *pos, Square from, Square to)
{
	gshort legal_moves, anz_n, anz_s;
	Square movelist [AB_ZUGL];
	Square *ap, *aq, ret;
	gshort i;

	ap =  movelist + AB_ZUG_S;
	legal_moves = position_legal_move (pos, &ap, &anz_s, &anz_n);

	for (aq = ap, i = 0; i < legal_moves; i++, aq += 2 ) {
		if (from == *aq) {
			if (to == *(aq + 1)) 
				return to;
			else if (*(aq + 1) & 128) { 
                                /* Promotion */
				ret = position_move_normalize_promotion (
					pos, to, *(aq + 1), *(aq + 3),
					*(aq + 5), *(aq + 7));
				if (ret)
					return ret;
				aq += 6; /* skip 3 moves */
			}
		}
	}

	return FALSE;
}

/* 
 * Move Reverse Functions
 *
 * A set of functions to reverse a previous move in the context of the position 
 * passed in.
 *
 */
static void
position_move_reverse_castling_white_short (Position *pos)
{
        pos->square [E1] = WK;
        pos->square [F1] = pos->square [G1] = EMPTY;
        pos->square [H1] = WR;
}

static void
position_move_reverse_castling_white_long (Position *pos)
{
        pos->square [A1] = WR;
        pos->square [C1] = pos->square [D1] = EMPTY;
        pos->square [E1] = WK;
}

static void
position_move_reverse_castling_black_short (Position *pos)
{
        pos->square [E8] = BK;
        pos->square [F8] = pos->square [G8] = EMPTY;
        pos->square [H8] = BR;
}

static void
position_move_reverse_castling_black_long (Position *pos)
{
        pos->square [A8] = BR;
        pos->square [D8] = pos->square [C8] = EMPTY;
        pos->square [E8] = BK;
}

static void
position_move_reverse_promotion_white (Position *pos, Square from, Square to)
{
	pos->square [from] = WP;
	pos->square [(to & 7) + A8] = pos->priv->captured;
}

static void
position_move_reverse_promotion_black (Position *pos, Square from, Square to)
{
	pos->square [from] = BP;
	pos->square [(to & 7) + A1] = pos->priv->captured;
}

void
position_move_reverse_white (Position *pos, Square from, Square to) 
{
	Piece fi;

	pos->priv->tomove = WHITE; /* change tomove */

	if (to & 128) {
                /* Promotion */
		position_move_reverse_promotion_white (pos, from, to);
		return;
	}

        fi = pos->square[to];

        /*    look for castling   */
        if (fi == WK) {
        	pos->priv->wk_square = from;
		pos->priv->wr_a_move -= 1;
		pos->priv->wr_h_move -= 1;
		
        	if (from != E1)  {  /*    no castling   */
               		pos->square[from]  = fi;
               		pos->square[to] = pos->priv->captured;
               		return;
        	}
        	if (abs (from - to) != 2)  {  /*  no castling  */
               		pos->square[from]  = fi;
               		pos->square[to] = pos->priv->captured;
               		return;
        	}

        	if (to == G1) {
			position_move_reverse_castling_white_short (pos);
			return;
		} else if (to == C1) {
			position_move_reverse_castling_white_long (pos);
			return;
		}
        	abort();
	}

	if (fi == WR && from == A1)
		pos->priv->wr_a_move -= 1;
	if (fi == WR && from == H1)
		pos->priv->wr_h_move -= 1;

        if (fi == WP) {
        	if ((to - from) != 10) {
          		if ((to - from) != 20) {
            			if (pos->priv->captured == EMPTY) {
                			pos->square [to - 10] = BP;
                			pos->square [to] = EMPTY;
                			pos->square [from] = WP;
          			        return;
        			}
			}
		}
        	pos->square [from] = fi;
        	pos->square [to] = pos->priv->captured;
		return;
	}
        pos->square [from]  = fi;
        pos->square [to] = pos->priv->captured;
}

void
position_move_reverse_black (Position *pos, Square from, Square to) 
{
	int fi;

	pos->priv->tomove = BLACK;      /* change tomove */

	if (to & 128) {
                /* Promotion */
		position_move_reverse_promotion_black (pos, from, to);
		return;
	}

        fi = pos->square[to];

        /* look for castling */
        if (fi == BK) {
        	pos->priv->bk_square = from;
		pos->priv->br_a_move -= 1;
		pos->priv->br_h_move -= 1;
		
        	if (from != E8)  {  /*    no castling   */
               		pos->square[from]  = fi;
               		pos->square[to] = pos->priv->captured;
               		return;
        	}
        	if (abs (from - to) != 2)  {  /*    no castling  */
               		pos->square[from]  = fi;
               		pos->square[to] = pos->priv->captured;
               		return;
        	}

        	if (to == G8) {
			position_move_reverse_castling_black_short (pos);
			return;
		} else if (to == C8) {
			position_move_reverse_castling_black_long (pos);
			return;
		}
        	abort ();
	}

	if (fi == BR && from == A8)
		pos->priv->br_a_move -= 1;
	if (fi == BR && from == H8)
		pos->priv->br_h_move -= 1;

        if (fi == BP) {
        	if ((from - to) != 10) {
          		if ((from - to) !=20) {
            			if (pos->priv->captured == EMPTY) {
                			pos->square[to+10] = WP;
                			pos->square[to] = EMPTY;
                			pos->square[from] = BP;
          			        return;
        			}
			}
		}
        	pos->square [from] = fi;
        	pos->square [to]= pos->priv->captured;
		return;
	}
	pos->square [from]  = fi;
        pos->square [to] = pos->priv->captured;
}

void
position_move_reverse (Position *pos, Square from, Square to)
{
	switch (pos->priv->tomove) {
		case WHITE:
			position_move_reverse_black (pos, from, to);
			break;
		case BLACK:
			position_move_reverse_white (pos, from, to);
			break;
        	default:
			g_assert_not_reached ();
	}
}

/* 
 * Move Generation Functions
 *
 * A set of functions to generate moves in the context of the position 
 * passed in.
 *
 */

static void inline
new_move (Square from, Square to)
{
	*nindex = from;
	*(nindex + 1) = to;
	nindex += 2;
}

static void 
new_capture_move (Square from, Square to)
{
	sindex -= 2;
	*sindex = from;
	*(sindex + 1) = to;
}

static void
wdouble (Position *pos, Square from, gshort a, gshort b)
{
	Square to, jp;
	Piece piece;


	for (; a < b; a++) {
		to = from;
		jp = jump[a];

		do {
			to += jp;
			piece  = pos->square[to];

			if (piece == EMPTY)
				new_move (from, to);
			else if (piece == BORDER)
				break;
			else if (BPIECE (piece)) {
				new_capture_move (from, to);
				break;
			} else {
				break;
			}
		} while (TRUE);
        }
}

static void
white_promotion (Square from, Square to)
{
	register Square new_to;
	register gshort i;

	for (i = 2; i < 6; i++) {
		new_to = 128 + 8 * i + to - A8;
		new_capture_move (from, new_to);
	}
}

static void
wpawn2 (Position *pos, Square from)
{
	register Square to;

	to = from + 10;    
	if (pos->square[to] == EMPTY ) {
		new_move (from, to);
	        to = from + 20;
		if (pos->square[to] == EMPTY )
			new_move (from, to);
	}

	if (BPIECE (pos->square[from + 9]))
		new_capture_move (from, from + 9);
	if (BPIECE (pos->square[from +11]))
		new_capture_move (from, from + 11);
}

static void
wpawn3 (Position *pos, Square from)
{
	register Square to;

	to = from + 10;
	if (pos->square[to] == EMPTY)
		new_move (from, to);

	if (BPIECE (pos->square[from + 9]))
		new_capture_move (from, from + 9);
	if (BPIECE (pos->square[from + 11]))
		new_capture_move (from, from + 11);
}

static void
wpawn5 (Position *pos, Square from)
{
	wpawn3 (pos, from);

	if ((from - 1) == pos->priv->ep)  
		new_capture_move (from, from + 9);
	else if ((from + 1) == pos->priv->ep)
		new_capture_move (from, from + 11);
}

static void
wpawn7 (Position *pos, Square from)
{
	register Square to;

	to = from + 10;
	if (pos->square[to] == EMPTY )
		white_promotion (from, to);

	if (BPIECE (pos->square[from + 9]))
		white_promotion (from, from + 9);
	if (BPIECE (pos->square[from + 11]))
		white_promotion (from, from + 11);
}

static void
wknight (Position *pos, gshort from)
{
	register Square to;
	register Piece piece;
	register gshort a;

	for (a = 0; a < 8; a++) {
		to = from + jump[a];
		piece = pos->square[to];

		switch (piece) {
			case EMPTY:
				new_move (from, to);
				break;
			case BORDER:  
				break;
			default:
				if (BPIECE (piece))
					new_capture_move (from, to);
		}
	}
}

static void
wbishop (Position *pos, Square from)
{
	gshort a = 8, b = 12;

	wdouble (pos, from, a, b);
}

static void
wrook (Position *pos, Square from)
{
	gshort a = 12, b = 16;

	wdouble (pos, from, a, b);
}

static void
wqueen (Position *pos, Square from)
{
	gshort a = 8, b = 16;

	wdouble (pos, from, a, b);
}

static void
wking (Position *pos, Square from)
{
	register Piece piece;
	register Square to;
	register gshort a;

	for (a = 8; a < 16 ; a++) {
		to = from + jump[a];
		piece = pos->square[to];

		switch (piece) {
			case EMPTY:
				new_move (from, to);
				break;
			case BORDER:
				break;
			default:
				if (BPIECE (piece))
					new_capture_move (from, to);
		}
	}
}

static void
w_ro_k (Position *pos)
{
	if (pos->square[F1] == EMPTY
	    && pos->square[G1] == EMPTY
	    && pos->square[H1] == WR)
		new_move (E1, G1);
}

static void
w_ro_l (Position *pos)
{
	if (pos->square[D1] == EMPTY &&
	    pos->square[C1] == EMPTY &&
	    pos->square[B1] == EMPTY &&
	    pos->square[A1] == WR) 
		new_move (E1, C1);
}

static void
wkingro (Position *pos, Square from)
{
	register Square to;
	register Piece piece;
	register gshort a;

	for (a = 8; a < 16 ; a++) {
		to = from + jump[a];
		piece = pos->square[to];

		switch (piece) {
		case EMPTY:
			new_move (from, to);
			break;
		case BORDER:
			break;
		default:
			if (BPIECE (piece))
				new_capture_move (from, to);
		}
	}

	if (pos->priv->wk_square != E1)
		return;
	if (!pos->priv->wr_h_move)
		w_ro_k (pos);
	if (!pos->priv->wr_a_move)
		w_ro_l (pos);
}

static void
bdouble (Position *pos, Square from, gshort a, gshort b)
{
	Square to, jp;
	Piece piece;

        for (; a < b; a++) {
		to = from;
		jp = jump[a];

		do {
			to += jp;
                        piece  = pos->square[to];

			if (piece == EMPTY)
				new_move (from, to);
			else if (piece == BORDER)
				break;
			else if (WPIECE (piece)) { 
				new_capture_move (from, to);
				break;
			} else {
				break;
			}
                } while (TRUE);
        }
}

static void
black_promotion (gshort from, gshort to)
{
	register Square new_to;
	register gshort i;

	for (i = 2; i < 6; i++) {
		new_to = 128 + 8 * i + to - A1;
		new_capture_move (from, new_to);
	}
}

static void
bpawn7 (Position *pos, Square from)
{
	register Square to;

	to = from - 10;                
	if (pos->square[to] == EMPTY) {
		new_move (from, to);
		to = from - 20;            
		if (pos->square[to] == EMPTY)
			new_move (from, to);
	}

	if (WPIECE (pos->square[from - 9]))
		new_capture_move (from, from - 9);
	if (WPIECE (pos->square[from - 11]))
		new_capture_move (from, from - 11);
}

static void
bpawn6 (Position *pos, Square from)
{
	register Square to;

	to = from - 10;
	if (pos->square[to] == EMPTY)
		new_move (from, to);

	if (WPIECE (pos->square[from - 9 ]))
		new_capture_move (from, from - 9);
	if (WPIECE (pos->square[from - 11]))
		new_capture_move (from, from - 11);
}

static void
bpawn4 (Position *pos, Square from)
{
	bpawn6 (pos, from);

	if ((from - 1) == pos->priv->ep)
		new_capture_move (from, from - 11);
	if ((from + 1) == pos->priv->ep)
		new_capture_move (from, from - 9);
}

static void
bpawn2 (Position *pos, Square from)
{
	register Square to;

	to = from - 10;
	if (pos->square[to] == EMPTY)
		black_promotion (from, to);

	if (WPIECE (pos->square[from - 9]))
		black_promotion (from, from - 9);
	if (WPIECE (pos->square[from - 11]))
		black_promotion (from, from - 11);
}

static void
bknight (Position *pos, Square from)
{
	register Square to;
	register Piece piece;
	register gshort a;

	for (a = 0 ; a < 8; a++) {
		to = from + jump[a];
		piece = pos->square[to];

		switch (piece) {
			case EMPTY:
				new_move (from, to);
				break;
			case BORDER:
				break;
			default:
				if (WPIECE (piece))
					new_capture_move (from, to);
		}
	}
}

static void
bbishop (Position *pos, Square from)
{
	gshort a = 8, b = 12;

	bdouble (pos, from, a, b);
}

static void
brook (Position *pos, Square from)
{
	gshort a = 12, b = 16;

	bdouble (pos, from, a, b);
}

static void
bqueen(Position *pos, Square from)
{
	gshort a = 8, b = 16;

	bdouble (pos, from, a, b);
}

static void
bking (Position *pos, Square from)
{
	register Piece piece;
	register Square to;
	register gshort a;

	for (a = 8; a < 16 ; a++) {
		to = from + jump[a];
		piece = pos->square[to];

		switch (piece) {
			case EMPTY:
				new_move (from, to);
				break;
			case BORDER:
				break;
			default:
				if (WPIECE (piece))
					new_capture_move (from, to);
		}
	}
}

static
void b_ro_k (Position *pos)
{
	if (pos->square[F8] == EMPTY &&
	    pos->square[G8] == EMPTY && 
	    pos->square[H8] == BR)
		new_move (E8, G8);

}

static
void b_ro_l (Position *pos)
{
	if (pos->square[D8] == EMPTY &&
	    pos->square[C8] == EMPTY &&
	    pos->square[B8] == EMPTY &&
	    pos->square[A8] == BR) 
		new_move(E8, C8);
}

static void
bkingro (Position *pos, Square from) 
{
	register Square to;
	register Piece piece;
	register gshort a;

	for (a = 8; a < 16 ; a++) {
		to = from + jump[a];
		piece = pos->square[to];

		switch (piece) {
		case EMPTY:
			new_move (from, to);
			break;
		case BORDER:
			break;
		default:
			if (WPIECE (piece))
				new_capture_move (from, to);
		}
	}

	if ( pos->priv->bk_square != E8)
		return;
	if (!pos->priv->br_h_move)
		b_ro_k (pos);  
	if (!pos->priv->br_a_move)
		b_ro_l (pos);
}

static int
position_move_generator_white (Position *pos, Square **index0, gshort *anz_s, gshort *anz_n)
{
	register Square square;
	register Piece piece;
	register gshort rank;

	nindex = sindex = *index0;

	for (rank = 1; rank <= 8; rank++) { 
		for (square = A1 + ((rank - 1) * 10); 
		     square <= H1 + ((rank - 1) * 10);
		     square++) {
			piece = pos->square[square];

			if (!WPIECE (piece))
				continue;

			switch (piece) {
			case WP: 
				switch (rank) {
				case 1:
				case 8:
					g_assert_not_reached ();
					break;
				case 2:
					wpawn2 (pos, square);
					break;
				case 3:
				case 4:
				case 6:
					wpawn3 (pos, square);
					break;
				case 5:
					wpawn5 (pos, square);
					break;
				case 7:
					wpawn7 (pos, square);
					break;
				}
				break;
			case WN:
				wknight (pos, square);
				break;
			case WB:
				wbishop (pos, square);
				break;
			case WR:
				wrook (pos, square);
				break;
			case WQ:
				wqueen (pos, square);
				break;
			case WK:
				if (rank == 1) 
					wkingro (pos, square);
				else
					wking (pos, square);
				break;
			}
		}
	}

	*anz_n = (gshort) ((nindex  - *index0) / 2);
	*anz_s = (gshort) ((*index0 - sindex) / 2 );
	*index0 = sindex;

	return *anz_n + *anz_s;
}

static int
position_move_generator_black (Position *pos, Square **index0, gshort *anz_s, gshort *anz_n)
{
	register Square square;
	register Piece piece;
	register gshort rank;

	nindex = sindex = *index0;

	for (rank = 1; rank <= 8; rank++) { 
		for (square = A1 + ((rank - 1) * 10);
		     square <= H1 + ((rank - 1) * 10);
		     square++ ) {
			piece = pos->square[square];
			if (!BPIECE (piece))
				continue;
			
			switch (piece)   {
			case BP: 
				switch (rank) {
				case 1:
				case 8:
					g_assert_not_reached ();
					break;
				case 2:
					bpawn2 (pos, square);
					break;
				case 3:
				case 5:
				case 6:
					bpawn6 (pos, square);
					break;
				case 4:
					bpawn4 (pos, square);
					break;
				case 7:
					bpawn7 (pos, square);
					break;
 				}
				break;
			case BN:
				bknight (pos, square);
				break;
			case BB:
				bbishop (pos, square);
				break;
			case BR:
				brook (pos, square);
				break;
			case BQ:
				bqueen (pos, square);
				break;
			case BK:
				if (rank == 8) 
					bkingro (pos, square);
				else
					bking (pos, square);
				break;
			}
		}
	}

	*anz_n = (gshort) ((nindex  - *index0) / 2);
	*anz_s = (gshort) ((*index0 - sindex) / 2);
	*index0 = sindex;
     
	return *anz_n + *anz_s;
}

gint
position_move_generator (Position *pos, Square **index0,
			 gshort *anz_s, gshort *anz_n)
{

	if (pos->priv->tomove == WHITE ) 
		return position_move_generator_white (pos, index0, anz_s, anz_n);
	else if (pos->priv->tomove == BLACK ) 
		return position_move_generator_black (pos, index0, anz_s, anz_n);
	else
		abort();
}

/* 
 * Position Characteristic Functions
 *
 * A set of functions to give information about the position
 * passed in.
 *
 */

#define CHECK(king_place,direction,piece1,piece2)\
{  int 	i= king_place; \
   do { i += direction; } \
   while(!pos->square[i]); \
   if (pos->square[i] == piece1) return piece1; \
   if (pos->square[i] == piece2) return piece2; \
}

static int long4 (Position *pos, int ort, int r1, int r2, int r3, int r4, int f1, int f2)
{

	CHECK (ort,r1,f1,f2);
	CHECK (ort,r2,f1,f2);
	CHECK (ort,r3,f1,f2);
	CHECK (ort,r4,f1,f2);

	return FALSE;
}   

#define KURZ_TEST(r)   if (pos->square[ort+r] == f1) return f1

static int
short8 (Position *pos, int ort, int r1, int r2, int r3,
	int r4, int r5, int r6, int r7, int r8, int f1)
{
	KURZ_TEST (r1);
	KURZ_TEST (r2);
	KURZ_TEST (r3);
	KURZ_TEST (r4);
	KURZ_TEST (r5);
	KURZ_TEST (r6);
	KURZ_TEST (r7);
	KURZ_TEST (r8);

	return FALSE;
}

gboolean
position_white_king_attack (Position *pos)
{
	Square k = pos->priv->wk_square;
	int ret;

	g_return_val_if_fail (pos != NULL, 0);
	g_return_val_if_fail (IS_POSITION (pos), 0);

	ret = long4 (pos, k, 9, 11, -9, -11, BQ, BB);
	if (ret)
		return ret;
   
	ret = long4 (pos, k, 1, 10, -10, -1, BQ, BR);
	if (ret)
		return ret;
   
	if (short8 (pos, k, 8, 12, 19, 21, -8, -12, -19, -21, BN))
		return BN;
	if (short8 (pos, k, 9, 11, -9, -11, 1, 10, -10, -1, BK))
		return BK;
   
	if (pos->square[k+OL] == BP)
		return BP;
	if (pos->square[k+OR] == BP)
		return BP;

	return FALSE;
}

gboolean
position_black_king_attack (Position *pos)
{
	Square k = pos->priv->bk_square;
	int ret;

	g_return_val_if_fail (pos != NULL, 0);
	g_return_val_if_fail (IS_POSITION (pos), 0);
	
	ret = long4 (pos, k, 9, 11, -9, -11, WQ, WB);
	if (ret)
		return ret;
   
	ret = long4 (pos, k, 1, 10, -10, -1, WQ, WR);
	if (ret)
		return ret;
   
	if (short8 (pos, k, 8, 12, 19, 21, -8, -12, -19, -21, WN))
		return WN;
	if (short8 (pos, k, 9, 11, -9, -11, 1, 10, -10, -1, WK))
		return WK;
   
	if (pos->square[k+UL] == WP)
	   	return WP;
	if (pos->square[k+UR] == WP)
		return WP;

	return FALSE;
}

gshort
position_legal_move (Position *pos, Square **zl, gshort *as, gshort *an)
{
	Position temp;
	Square tomove = pos->priv->tomove;
	gboolean check = FALSE;
	gshort gen_moves, a;
	Square *ap, *ap2;
	gshort anz_s, anz_n, legal_moves;
	Square zugl[AB_ZUGL];

	g_return_val_if_fail (pos != NULL, 0);
	g_return_val_if_fail (IS_POSITION (pos), 0);

        ap =  zugl + AB_ZUG_S;
        gen_moves = position_move_generator (pos, &ap, &anz_s, &anz_n);
        legal_moves = 0;
        ap2 = *zl;

        for (a = 0; a < gen_moves; a++) {
                temp = *pos;
                position_move (pos, *ap, *(ap+1));

                switch (tomove) {
                        case WHITE: 
				check = position_white_king_attack (pos);
				break;
                        case BLACK:
				check = position_black_king_attack (pos);
				break;
                        default:
				g_assert_not_reached ();
                }

                if (!check) {
                        *ap2++ = *ap++;
                        *ap2++ = *ap++;
                        legal_moves++;
                } else {
		        ap += 2;
		}
                switch (tomove) {
                        case WHITE:
                                position_move_reverse_white (pos, *(ap - 2), 
							     *(ap - 1));
                                break;
                        case BLACK:
                                position_move_reverse_black (pos, *(ap - 2), 
							     *(ap - 1));
                                break;
                }
                *pos = temp;
        }
        *as = legal_moves;
	*an = 0;

        return legal_moves;
}

short
position_last_piece_captured (Position *pos)
{
	g_return_val_if_fail (pos != NULL, EMPTY);
	g_return_val_if_fail (IS_POSITION (pos), EMPTY);

	return pos->priv->captured;
}

short
position_get_color_to_move (Position *pos)
{
	g_return_val_if_fail (pos != NULL, NONE);
	g_return_val_if_fail (IS_POSITION (pos), NONE);

	return pos->priv->tomove;
}

void
position_set_white_king (Position *pos, Square square)
{
	g_return_if_fail (pos != NULL);
	g_return_if_fail (IS_POSITION (pos));

	pos->priv->wk_square = square;
}

void
position_set_black_king (Position *pos, Square square)
{
	g_return_if_fail (pos != NULL);
	g_return_if_fail (IS_POSITION (pos));

	pos->priv->bk_square = square;
}

void
position_set_color_to_move (Position *pos, short color)
{
	g_return_if_fail (pos != NULL);
	g_return_if_fail (IS_POSITION (pos));

	pos->priv->tomove = color;
}

