/* gcompris - anim.h
 *
 * Time-stamp: <2005/02/13 11:59:55 joe>
 *
 * Copyright (C) 2000 Joe Neeman
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef GCOMPRISANIM_H
#define GCOMPRISANIM_H

#include "gcompris.h"

/** Defines an animation object. This structure is opaque and should not be
 *  accessed directly.
 */
typedef struct {
  GdkPixbufAnimation **anim;
  int numstates;
} GcomprisAnimation;

typedef struct {
  GooCanvasItem *canvas;
  GcomprisAnimation *anim;
  GdkPixbufAnimationIter *iter;
  int state;
} GcomprisAnimCanvasItem;

GcomprisAnimation *gc_anim_load(char *filename);

GcomprisAnimCanvasItem *gc_anim_activate(GooCanvasItem *parent,
					 GcomprisAnimation*);
void gc_anim_set_state(GcomprisAnimCanvasItem*, int);
void gc_anim_deactivate(GcomprisAnimCanvasItem*);
void gc_anim_free(GcomprisAnimation*);
void gc_anim_swap(GcomprisAnimCanvasItem *item, GcomprisAnimation *new_anim);

#endif
