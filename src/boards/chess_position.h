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

#ifndef _POSITION_H_
#define _POSITION_H_

#include <gtk/gtk.h>
#include "chess_makros.h"

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

#define TYPE_POSITION	       (position_get_type ())
#define POSITION(obj)          GTK_CHECK_CAST (obj, TYPE_POSITION, Position)
#define POSITION_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, TYPE_POSITION, PositionClass)
#define IS_POSITION(obj)       GTK_CHECK_TYPE (obj, TYPE_POSITION)

typedef struct _Position        Position;
typedef struct _PositionClass   PositionClass;
typedef struct _PositionPrivate PositionPrivate;

struct _PositionClass {
	GtkObjectClass parent_class;
};

struct _Position {
	GtkObject        object;

	/* 10x10 board - extra for move generation */
	Piece            square[120]; 

	PositionPrivate *priv;
};

GtkType    position_get_type             (void);    
GtkObject *position_new                   (void);
GtkObject *position_new_initial           (void);
Position  *position_copy                 (Position *pos);
void       position_set_initial          (Position *pos);
void       position_set_initial_partyend (Position *pos, int level);
void       position_set_initial_movelearn(Position *pos, int level);
void       position_move                 (Position *pos, Square from, Square to);
Square     position_move_normalize       (Position *pos, Square from, Square to);
void       position_move_reverse_white   (Position *pos, Square from, Square to);
void       position_move_reverse_black   (Position *pos, Square from, Square to);
void       position_move_reverse         (Position *pos, Square from, Square to);
gint       position_move_generator        (Position *pos, Square **index0, gshort *anz_s, gshort *anz_n);
gboolean   position_white_king_attack    (Position *pos);
gboolean   position_black_king_attack    (Position *pos);
gshort     position_legal_move           (Position *pos, Square **zl, gshort *as, gshort *an);

/* Misc. accessors */
void       position_set_white_king       (Position *pos, Square square);
void       position_set_black_king       (Position *pos, Square square);
void       position_set_color_to_move    (Position *pos, short color);
short      position_get_color_to_move    (Position *pos);
short      position_last_piece_captured  (Position *pos);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _POSITION_H_ */
