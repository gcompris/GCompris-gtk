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
  GnomeCanvasPixbuf *canvas;
  GcomprisAnimation *anim;
  GdkPixbufAnimationIter *iter;
  int state;
} GcomprisAnimCanvasItem;

GcomprisAnimation *gcompris_load_animation(char *filename);

/*! \fn GcomprisAnimation       *gcompris_load_animation_asset(gchar *dataset, gchar* categories, gchar* name)
    \brief Returns an animation from the assetml base.
    \warning If there is more than one answer, the first animation is returned

    \param dataset The assetml dataset
    \param categories The category filter
    \param mimetype The mimetype for the asset to search
    \param name The name filter
*/
GcomprisAnimation *gcompris_load_animation_asset(gchar *dataset,
                                                 gchar *categories,
                                                 gchar *mimetype,
                                                 gchar *name);

GcomprisAnimCanvasItem *gcompris_activate_animation(GnomeCanvasGroup *parent,
                                                    GcomprisAnimation*);
void gcompris_set_anim_state(GcomprisAnimCanvasItem*, int);
void gcompris_deactivate_animation(GcomprisAnimCanvasItem*);
void gcompris_free_animation(GcomprisAnimation*);

#endif
