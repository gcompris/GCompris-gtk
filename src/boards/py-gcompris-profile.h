/* gcompris - py-gcompris-profile.h
 *
 * Copyright (C) 2003, 2008 Olivier Samyn <osamyn@ulb.ac.be>
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
#ifndef _PY_GCOMPRIS_PROFILE_H_
#define _PY_GCOMPRIS_PROFILE_H_

#include <Python.h>
#include "gcompris/gcompris.h"

PyObject* gcompris_new_pyGcomprisProfileObject(GcomprisProfile* profile);

typedef struct{
  PyObject_HEAD
  GcomprisProfile* cdata;
} pyGcomprisProfileObject;

PyObject* gcompris_new_pyGcomprisClassObject(GcomprisClass* class);

typedef struct{
  PyObject_HEAD
  GcomprisClass* cdata;
} pyGcomprisClassObject;

PyObject* gcompris_new_pyGcomprisGroupObject(GcomprisGroup* group);

typedef struct{
  PyObject_HEAD
  GcomprisGroup* cdata;
} pyGcomprisGroupObject;


PyObject* gcompris_new_pyGcomprisUserObject(GcomprisUser* user);

typedef struct{
  PyObject_HEAD
  GcomprisUser* cdata;
} pyGcomprisUserObject;

#endif
