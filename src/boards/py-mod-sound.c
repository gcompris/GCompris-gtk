#include <Python.h>
#include <pygobject.h>
#include <stdarg.h>
#include "gcompris/gcompris.h"
#include "py-mod-sound.h"
#include "py-gcompris-board.h"

/* All functions provided by this python module
 * wraps a gcompris function. Each "py_*" function wraps the
 * "*" C function.
 */

/*void gcompris_play_ogg_list( GList* files ); */
static PyObject*
py_gcompris_play_ogg_list(PyObject* self, PyObject* args)
{
  GList* list;
  PyObject* pylist;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "O:gcompris_play_ogg_list", &pylist))
    return NULL;
  list = (GList*) pygobject_get(pylist);

  /* Call the corresponding C function */
  gcompris_play_ogg_list(list);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}



/* void gcompris_play_ogg(char *, ...);
   The method: gcompris_play_ogg_list( GList* ) is used
   to perform the core call. (Because there's no way to construct
   a variable argument function call.
*/

static PyObject*
py_gcompris_play_ogg(PyObject* self, PyObject* args)
{
  PyObject* item;
  GList* list = NULL;
  int i, length;

  /* Parse arguments */
  length = PyTuple_Size(args);
  if(length<1) return NULL;
  for(i=0; i<length; i++){
    item = PyTuple_GetItem(args, i);
    list = g_list_append(list, PyString_AsString(item));
  }
  
  /* Call the corresponding C function */
  gcompris_play_ogg_list(list);
  g_list_free(list);
    
  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}



static PyObject*
py_gcompris_reopen_sound(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.reopen_sound"))
    return NULL;

  /* Call the corresponding C function */
  gcompris_reopen_sound();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
py_gcompris_close_sound(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.close_sound"))
    return NULL;

  /* Call the corresponding C function */
  gcompris_close_sound();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


static PyMethodDef PythonGcomprisSoundModule[] = {
  { "play_ogg_list",  py_gcompris_play_ogg_list, METH_VARARGS, "gcompris_play_ogg_list" },
  { "play_ogg",  py_gcompris_play_ogg, METH_VARARGS, "gcompris_play_ogg" },
  { "reopen",  py_gcompris_reopen_sound, METH_VARARGS, "gcompris_reopen_sound" },
  { "close",  py_gcompris_close_sound, METH_VARARGS, "gcompris_close_sound" },
  { NULL, NULL, 0, NULL}
};

void python_gcompris_sound_module_init(void)
{
  PyObject* module;
  module = Py_InitModule("_gcompris_sound", PythonGcomprisSoundModule);
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
