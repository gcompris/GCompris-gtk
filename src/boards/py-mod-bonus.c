/* gcompris - py-mod-gcompris.c
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
#include <Python.h>
#define NO_IMPORT_PYGOBJECT 1
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-mod-bonus.h"
#include "py-gcompris-board.h"

/* All functions provided by this python module
 * wraps a gcompris function. Each "py_*" function wraps the
 * "*" C function.
 */

/* void	gc_bonus_display(int gamewon, int bonus_id); */
static PyObject*
py_gc_bonus_display(PyObject* self, PyObject* args)
{
  int gamewon;
  int bonus_id;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ii:gc_bonus_display", &gamewon, &bonus_id))
    return NULL;

  /* Call the corresponding C function */
  gc_bonus_display(gamewon, bonus_id);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_bonus_end_display(int type); */
static PyObject*
py_gcompris_gc_bonus_end_display(PyObject* self, PyObject* args)
{
  int type;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "i:gc_bonus_end_display", &type))
    return NULL;

  /* Call the corresponding C function */
  gc_bonus_end_display(type);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


static PyMethodDef PythonGcomprisBonusModule[] = {
  { "display",  py_gc_bonus_display, METH_VARARGS, "gc_bonus_display" },
  { "board_finished",  py_gcompris_gc_bonus_end_display, METH_VARARGS, "gc_bonus_end_display" },
  { NULL, NULL, 0, NULL}
};

void python_gcompris_bonus_module_init(void)
{
  PyObject* module;
  module = Py_InitModule("_gcompris_bonus", PythonGcomprisBonusModule);

  /* Misc constants */
  PyModule_AddIntConstant(module, "TIME_CLICK_TO", TIME_CLICK_TO_BONUS );

  /* GCBonusList constants */
  PyModule_AddIntConstant(module, "RANDOM", GC_BONUS_RANDOM );
  PyModule_AddIntConstant(module, "SMILEY", GC_BONUS_SMILEY );
  PyModule_AddIntConstant(module, "FLOWER", GC_BONUS_FLOWER );
  PyModule_AddIntConstant(module, "TUX",    GC_BONUS_TUX );
  PyModule_AddIntConstant(module, "GNU",    GC_BONUS_GNU );
  PyModule_AddIntConstant(module, "GNU",    GC_BONUS_LION );

  /* BonusFinishedList constants */
  PyModule_AddIntConstant(module, "FINISHED_RANDOM", GC_BOARD_FINISHED_RANDOM );
  PyModule_AddIntConstant(module, "FINISHED_TUXPLANE", GC_BOARD_FINISHED_TUXPLANE );
  PyModule_AddIntConstant(module, "FINISHED_TUXLOCO", GC_BOARD_FINISHED_TUXLOCO );
  PyModule_AddIntConstant(module, "FINISHED_TOOMANYERRORS", GC_BOARD_FINISHED_TOOMANYERRORS );

  /* GCBonusStatusList constants */
  PyModule_AddIntConstant(module, "LOOSE", GC_BOARD_LOOSE );
  PyModule_AddIntConstant(module, "WIN",   GC_BOARD_WIN );
  PyModule_AddIntConstant(module, "DRAW",  GC_BOARD_DRAW );
  PyModule_AddIntConstant(module, "COMPLETED", GC_BOARD_COMPLETED );
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

