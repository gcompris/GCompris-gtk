/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* move-List.h
 *
 * Copyright (C) 2001  Robert Wilhelm
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
 * Author: Robert Wilhelm
 */

#ifndef _MOVE_LIST_H_
#define _MOVE_LIST_H_

#include <gtk/gtk.h>
#include "chess_position.h"

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

#define MOVE_TYPE_LIST			(move_list_get_type ())
#define MOVE_LIST(obj)			(GTK_CHECK_CAST ((obj), MOVE_TYPE_LIST, MoveList))
#define MOVE_LIST_CLASS(klass)		(GTK_CHECK_CLASS_CAST ((klass), MOVE_TYPE_LIST, MoveListClass))
#define IS_MOVE_LIST(obj)		(GTK_CHECK_TYPE ((obj), MOVE_TYPE_LIST))
#define IS_MOVE_LIST_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((obj), MOVE_TYPE_LIST))


typedef struct _MoveList        MoveList;
typedef struct _MoveListPrivate MoveListPrivate;
typedef struct _MoveListClass   MoveListClass;

struct _MoveList {
	GtkVBox parent;

	MoveListPrivate *priv;
};

struct _MoveListClass {
	GtkVBoxClass parent_class;

	void (*move_selected) (MoveList *movelist, gint plynum);
};



GtkType    move_list_get_type (void);
GtkWidget *move_list_new      (void);

void      move_list_freeze               (MoveList *movelist);
void      move_list_thaw                 (MoveList *movelist);
Position* move_list_get_position         (MoveList *movelist,
					  gint      plynum);
Position* move_list_get_position_start   (MoveList *movelist);
Position* move_list_get_position_current (MoveList *movelist);
void      move_list_get_ply              (MoveList *movelist,
					  gint      plynum,
					  Square   *from,
					  Square   *to);
void      move_list_add                  (MoveList *movelist,
					  Square       from,
					  Square       to);
void      move_list_movetoply            (MoveList *movelist,
					  gint      plynum);
void      move_list_move_forward         (MoveList *movelist);
void      move_list_move_back            (MoveList *movelist);
void      move_list_move_start           (MoveList *movelist);
void      move_list_move_end             (MoveList *movelist);
int       move_list_startply             (MoveList *movelist);
int       move_list_currply              (MoveList *movelist);
int       move_list_maxply               (MoveList *movelist);
void      move_list_clear_initial        (MoveList *movelist,
					  gint      plynum,
					  Position *pos);
void      move_list_clear_from           (MoveList *movelist,
					  gint      plynum);
void      move_list_clear                (MoveList *movelist);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MOVE_LIST_H_ */
