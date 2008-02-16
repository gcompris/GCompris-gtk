/* gcompris - py-mod-anim.h
 *
 * Copyright (C) 2003 Olivier Samyn <osamyn@ulb.ac.be>
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
#ifndef _PY_MOD_ANIM_H_
#define _PY_MOD_ANIM_H_

#include <Python.h>
#include "gcompris/anim.h"

void python_gcompris_anim_module_init();

typedef struct {
  PyObject_HEAD
  GcomprisAnimation *a;
} py_GcomprisAnimation;

typedef struct {
  PyObject_HEAD
  PyObject *anim;
  GcomprisAnimCanvasItem *item;
} py_GcomprisAnimCanvas;

#endif
