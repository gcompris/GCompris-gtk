#include <Python.h>
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-mod-utils.h"
#include "py-gcompris-board.h"

/* All functions provided by this python module
 * wraps a gcompris function. Each "py_*" function wraps the
 * "*" C function.
 */

/* typedef void (*GcomprisTimerEnd) (); */
static PyObject* pyTimerCallBackFunc = NULL;

void pyTimerCallBack(){
  PyObject* result;
  if(pyTimerCallBackFunc==NULL) return;

  /* Build arguments */
  result = PyObject_CallObject(pyTimerCallBackFunc, NULL);
  if(result==NULL){
    PyErr_Print();
  } else {
    Py_DECREF(result);
  }
}


/* void gc_timer_display
        (int x, int y, TimerList type, int second, GcomprisTimerEnd gcomprisTimerEnd); */
static PyObject*
py_gc_timer_display(PyObject* self, PyObject* args)
{
  int x,y;
  int type;
  int second;
  PyObject* pyCallback;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "iiiiO:gc_timer_display", &x, &y, &type, &second, &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback)) return NULL;
  pyTimerCallBackFunc = pyCallback;

  /* Call the corresponding C function */
  gc_timer_display(x, y, type, second, pyTimerCallBack );

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_timer_add(int second); */
static PyObject*
py_gc_timer_add(PyObject* self, PyObject* args)
{
  int second;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "i:gc_timer_add", &second))
    return NULL;

  /* Call the corresponding C function */
  gc_timer_add(second);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/*void gc_timer_end(void); */
static PyObject*
py_gc_timer_end(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_timer_end"))
    return NULL;

  /* Call the corresponding C function */
  gc_timer_end();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* guint gc_timer_get_remaining(); */
static PyObject*
py_gc_timer_get_remaining(PyObject* self, PyObject* args)
{
  guint result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_timer_get_remaining"))
    return NULL;

  /* Call the corresponding C function */
  result = gc_timer_get_remaining();

  /* Create and return the result */
  return Py_BuildValue("i", result);  
}


/* void	 gc_timer_pause(gboolean pause); */
static PyObject*
py_gc_timer_pause(PyObject* self, PyObject* args)
{
  gboolean pause;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "i:gc_timer_pause", &pause))
    return NULL;

  /* Call the corresponding C function */
  gc_timer_pause(pause);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


static PyMethodDef PythonGcomprisTimerModule[] = {
  { "display",  py_gc_timer_display, METH_VARARGS, "gc_timer_display" },
  { "add",  py_gc_timer_add, METH_VARARGS, "gc_timer_add" },
  { "end",  py_gc_timer_end, METH_VARARGS, "gc_timer_end" },
  { "get_remaining",  py_gc_timer_get_remaining, METH_VARARGS, "gc_timer_get_remaining" },
  { "pause",  py_gc_timer_pause, METH_VARARGS, "gc_timer_pause" },
  { NULL, NULL, 0, NULL}
};

void python_gc_timer_module_init(void)
{
  PyObject* module;
  module = Py_InitModule("_gcompris_timer", PythonGcomprisTimerModule);

  /* TimerList constants */
  PyModule_AddIntConstant(module, "TEXT", GCOMPRIS_TIMER_TEXT );
  PyModule_AddIntConstant(module, "SAND", GCOMPRIS_TIMER_SAND );
  PyModule_AddIntConstant(module, "BALLOON", GCOMPRIS_TIMER_BALLOON );
  PyModule_AddIntConstant(module, "CLOCK", GCOMPRIS_TIMER_CLOCK );
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
