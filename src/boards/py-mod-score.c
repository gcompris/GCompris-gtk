#include <Python.h>
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-mod-utils.h"
#include "py-gcompris-board.h"

/* All functions provided by this python module
 * wraps a gcompris function. Each "py_*" function wraps the
 * "*" C function.
 */

/* void gcompris_score_start (ScoreStyleList style, guint x, guint y, guint max); */
static PyObject*
py_gcompris_score_start(PyObject* self, PyObject* args)
{
  int style;
  int x,y;
  guint max;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "iiii:gcompris_score_start", &style, &x, &y, &max))
    return NULL;
  if(max<0) return NULL;

  /* Call the corresponding C function */
  gcompris_score_start(style, x, y, max);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_score_end(); */
static PyObject*
py_gcompris_score_end(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_score_end"))
    return NULL;

  /* Call the corresponding C function */
  gcompris_score_end();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_score_set(guint value); */
static PyObject*
py_gcompris_score_set(PyObject* self, PyObject* args)
{
  guint value;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "i:gcompris_score_set", &value))
    return NULL;
  if(value<0) return NULL;

  /* Call the corresponding C function */
  gcompris_score_set(value);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


static PyMethodDef PythonGcomprisScoreModule[] = {
  { "start",  py_gcompris_score_start, METH_VARARGS, "gcompris_score_start" },
  { "end",  py_gcompris_score_end, METH_VARARGS, "gcompris_score_end" },
  { "set",  py_gcompris_score_set, METH_VARARGS, "gcompris_score_set" },
  { NULL, NULL, 0, NULL}
};

void python_gcompris_score_module_init(void)
{
  PyObject* module;
  module = Py_InitModule("gcompris.score", PythonGcomprisScoreModule);

  /* Misc constants */
  PyModule_AddIntConstant(module, "STYLE_NOTE", SCORESTYLE_NOTE ); 
  PyModule_AddIntConstant(module, "STYLE_LIFE", SCORESTYLE_LIFE ); 
  PyModule_AddIntConstant(module, "LAST", SCORE_LAST ); 
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
