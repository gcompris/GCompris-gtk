#include <Python.h>
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-mod-bonus.h"
#include "py-gcompris-board.h"

/* All functions provided by this python module
 * wraps a gcompris function. Each "py_*" function wraps the
 * "*" C function.
 */

/* void	gcompris_display_bonus(int gamewon, int bonus_id); */
static PyObject*
py_gcompris_display_bonus(PyObject* self, PyObject* args)
{
  int gamewon;
  int bonus_id;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ii:gcompris_display_bonus", &gamewon, &bonus_id))
    return NULL;

  /* Call the corresponding C function */
  gcompris_display_bonus(gamewon, bonus_id);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void board_finished(int type); */
static PyObject*
py_gcompris_board_finished(PyObject* self, PyObject* args)
{
  int type;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "i:board_finished", &type))
    return NULL;

  /* Call the corresponding C function */
  board_finished(type);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


static PyMethodDef PythonGcomprisBonusModule[] = {
  { "display",  py_gcompris_display_bonus, METH_VARARGS, "gcompris_display_bonus" },
  { "board_finished",  py_gcompris_board_finished, METH_VARARGS, "board_finished" },
  { NULL, NULL, 0, NULL}
};

void python_gcompris_bonus_module_init(void)
{
  PyObject* module;
  module = Py_InitModule("gcompris.bonus", PythonGcomprisBonusModule);

  /* Misc constants */
  PyModule_AddIntConstant(module, "TIME_CLICK_TO", TIME_CLICK_TO_BONUS ); 

  /* BonusList constants */
  PyModule_AddIntConstant(module, "RANDOM", BONUS_RANDOM ); 
  PyModule_AddIntConstant(module, "SMILEY", BONUS_SMILEY ); 
  PyModule_AddIntConstant(module, "FLOWER", BONUS_FLOWER ); 
  PyModule_AddIntConstant(module, "LAST", BONUS_LAST ); 

  /* BonusFinishedList constants */
  PyModule_AddIntConstant(module, "FINISHED_RANDOM", BOARD_FINISHED_RANDOM ); 
  PyModule_AddIntConstant(module, "FINISHED_TUXPLANE", BOARD_FINISHED_TUXPLANE ); 
  PyModule_AddIntConstant(module, "FINISHED_TUXLOCO", BOARD_FINISHED_TUXLOCO ); 
  PyModule_AddIntConstant(module, "FINISHED_TOOMANYERRORS", BOARD_FINISHED_TOOMANYERRORS ); 
  PyModule_AddIntConstant(module, "FINISHED_LAST", BOARD_FINISHED_LAST ); 
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

