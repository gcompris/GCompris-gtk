/* gcompris - gcompris-edit.h
 *
 * Time-stamp: <2002/06/04 21:40:44 bruno>
 *
 * Copyright (C) 2000,2001,2002 Bruno Coudoin
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */



void		 gcompris_edit_display_description(GcomprisBoard *gcomprisBoard);
void		 gcompris_edit_display_iconlist();
void		 gcompris_ctree_selection_add(GcomprisBoard *gcomprisBoard, GtkCTreeNode *node);
void		 gcompris_ctree_selection_del(GcomprisBoard *gcomprisBoard, GtkCTreeNode *node);
GcomprisBoard	*gcompris_ctree_get_selected_board();
GtkCTreeNode	*gcompris_ctree_get_selected_node();
void		 gcompris_ctree_set_board_status(GcomprisBoard *gcomprisBoard,
						 GtkCTreeNode *node,
						 gboolean status);
void		 gcompris_ctree_update_status(gcomprisBoard, node);
