/* gcompris - py-mod-gcompris.c
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
#include <Python.h>
#define NO_IMPORT_PYGOBJECT 1
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-mod-utils.h"
#include "py-gcompris-board.h"

/* All functions provided by this python module
 * wraps a gcompris function. Each "py_*" function wraps the
 * "*" C function.
 */

/* void gc_score_start (ScoreStyleList style, guint x, guint y, guint max); */
static PyObject*
py_gc_score_start(PyObject* self, PyObject* args)
{
  int style;
  int x,y;
  guint max;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "iiii:gc_score_start", &style, &x, &y, &max))
    return NULL;
  if(max<0) return NULL;

  /* Call the corresponding C function */
  gc_score_start(style, x, y, max);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_score_end(); */
static PyObject*
py_gc_score_end(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_score_end"))
    return NULL;

  /* Call the corresponding C function */
  gc_score_end();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_score_set(guint value); */
static PyObject*
py_gc_score_set(PyObject* self, PyObject* args)
{
  guint value;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "i:gc_score_set", &value))
    return NULL;
  if(value<0) return NULL;

  /* Call the corresponding C function */
  gc_score_set(value);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


static PyMethodDef PythonGcomprisScoreModule[] = {
  { "start",  py_gc_score_start, METH_VARARGS, "gc_score_start" },
  { "end",  py_gc_score_end, METH_VARARGS, "gc_score_end" },
  { "set",  py_gc_score_set, METH_VARARGS, "gc_score_set" },
  { NULL, NULL, 0, NULL}
};

void python_gc_score_module_init(void)
{
  PyObject* module;
  module = Py_InitModule("_gcompris_score", PythonGcomprisScoreModule);

  /* Misc constants */
  PyModule_AddIntConstant(module, "STYLE_NOTE", SCORESTYLE_NOTE );
  PyModule_AddIntConstant(module, "STYLE_LIFE", SCORESTYLE_LIFE );
}

/* Some usefull code parts ... */
/*
static PyObject*
py_gcompris_(PyObject* self, PyObject* args)
{
*/  /* Parse arguments */
/*  if(!PyArg_ParseTuple(args, ":gcompris_"))
    return NULL;
*/
  /* Call the corresponding C function */
/*  gcompris_();
*/
  /* Create and return the result */
/*  Py_INCREF(Py_None);
  return Py_None;
}
*/
/*
  { "",  py_gcompris_, METH_VARARGS, "gcompris_" },
*/
