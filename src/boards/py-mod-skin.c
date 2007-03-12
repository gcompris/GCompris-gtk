#include <Python.h>
#define NO_IMPORT_PYGOBJECT 1
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-mod-skin.h"
#include "py-gcompris-board.h"

/* All functions provided by this python module
 * wraps a gcompris function. Each "py_*" function wraps the
 * "*" C function.
 */


/* gchar *gc_skin_image_get(gchar *imagename); */
static PyObject*
py_gc_skin_image_get(PyObject* self, PyObject* args)
{
  gchar* imagename;
  gchar* result;
  PyObject* pyresult;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gc_skin_image_get", &imagename))
    return NULL;

  /* Call the corresponding C function */
  result = gc_skin_image_get(imagename);

  /* Create and return the result */
  pyresult = Py_BuildValue("s", result);

  g_free(result);

  return(pyresult);
}


/* GdkPixbuf *gc_skin_pixmap_load(char *pixmapfile); */
static PyObject*
py_gc_skin_pixmap_load(PyObject* self, PyObject* args)
{
  char* pixmapfile;
  GdkPixbuf* result;
  PyObject* pyresult;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gc_skin_pixmap_load", &pixmapfile))
    return NULL;

  /* Call the corresponding C function */
  result = gc_skin_pixmap_load(pixmapfile);

  /* Create and return the result */
  pyresult = (PyObject*) pygobject_new((GObject*) result);

  gdk_pixbuf_unref(result);

  return(pyresult);
}


/* guint32 gc_skin_get_color_default(gchar* id, guint32 def); */
static PyObject*
py_gc_skin_get_color_default(PyObject* self, PyObject* args)
{
  gchar* id;
  guint32 def;
  guint32 result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "sl:gc_skin_get_color_default", &id, &def))
    return NULL;

  /* Call the corresponding C function */
  result = gc_skin_get_color_default(id, def);

  /* Create and return the result */
  return PyLong_FromUnsignedLong(result);
}


/* gchar* gc_skin_get_font_default(gchar* id, gchar* def); */
static PyObject*
py_gc_skin_get_font_default(PyObject* self, PyObject* args)
{
  gchar* id;
  gchar* def;
  gchar* result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ss:gc_skin_get_font_default", &id, &def))
    return NULL;

  /* Call the corresponding C function */
  result = gc_skin_get_font_default(id, def);

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


/* guint32 gc_skin_get_color(gchar* id); */
static PyObject*
py_gc_skin_get_color(PyObject* self, PyObject* args)
{
  gchar* id;
  guint32 result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gc_skin_get_color", &id))
    return NULL;

  /* Call the corresponding C function */
  result = gc_skin_get_color(id);

  /* Create and return the result */
  return PyLong_FromUnsignedLong(result);
}


/* gchar* gc_skin_get_font(gchar* id); */
static PyObject*
py_gc_skin_get_font(PyObject* self, PyObject* args)
{
  gchar* id;
  gchar* result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gc_skin_get_font_default", &id))
    return NULL;

  /* Call the corresponding C function */
  result = gc_skin_get_font(id);

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


static PyMethodDef PythonGcomprisSkinModule[] = {
  { "image_to_skin",  py_gc_skin_image_get, METH_VARARGS, "gc_skin_image_get" },
  { "load_pixmap",  py_gc_skin_pixmap_load, METH_VARARGS, "gc_skin_pixmap_load" },
  { "get_color_default",  py_gc_skin_get_color_default, METH_VARARGS,
    "gc_skin_get_color_default" },
  { "get_font_default",  py_gc_skin_get_font_default, METH_VARARGS,
    "gc_skin_get_font_default" },
  { "get_color",  py_gc_skin_get_color, METH_VARARGS,
    "gc_skin_get_color" },
  { "get_font",  py_gc_skin_get_font, METH_VARARGS,
    "gc_skin_get_font" },
  { NULL, NULL, 0, NULL}
};


void python_gc_skin_module_init(void)
{
  PyObject* module;
  module = Py_InitModule("_gcompris_skin", PythonGcomprisSkinModule);
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
