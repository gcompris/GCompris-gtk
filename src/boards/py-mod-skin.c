#include <Python.h>
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-mod-skin.h"
#include "py-gcompris-board.h"

/* All functions provided by this python module
 * wraps a gcompris function. Each "py_*" function wraps the
 * "*" C function.
 */


/* gchar *gcompris_image_to_skin(gchar *imagename); */
static PyObject*
py_gcompris_image_to_skin(PyObject* self, PyObject* args)
{
  gchar* imagename;
  gchar* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gcompris_image_to_skin", &imagename))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_image_to_skin(imagename);

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


/* GdkPixbuf *gcompris_load_skin_pixmap(char *pixmapfile); */
static PyObject*
py_gcompris_load_skin_pixmap(PyObject* self, PyObject* args)
{
  char* pixmapfile;
  GdkPixbuf* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gcompris_load_skin_pixmap", &pixmapfile))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_load_skin_pixmap(pixmapfile);

  /* Create and return the result */
  return (PyObject*) pygobject_new((GObject*) result);
}


/* guint32 gcompris_skin_get_color_default(gchar* id, guint32 def); */
static PyObject*
py_gcompris_skin_get_color_default(PyObject* self, PyObject* args)
{
  gchar* id;
  guint32 def;
  guint32 result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "sl:gcompris_skin_get_color_default", &id, &def))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_skin_get_color_default(id, def);

  /* Create and return the result */
  return PyLong_FromUnsignedLong(result);
}


/* gchar* gcompris_skin_get_font_default(gchar* id, gchar* def); */
static PyObject*
py_gcompris_skin_get_font_default(PyObject* self, PyObject* args)
{
  gchar* id;
  gchar* def;
  gchar* result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ss:gcompris_skin_get_font_default", &id, &def))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_skin_get_font_default(id, def);

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


/* guint32 gcompris_skin_get_color(gchar* id); */
static PyObject*
py_gcompris_skin_get_color(PyObject* self, PyObject* args)
{
  gchar* id;
  guint32 result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gcompris_skin_get_color", &id))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_skin_get_color(id);

  /* Create and return the result */
  return PyLong_FromUnsignedLong(result);
}


/* gchar* gcompris_skin_get_font(gchar* id); */
static PyObject*
py_gcompris_skin_get_font(PyObject* self, PyObject* args)
{
  gchar* id;
  gchar* result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gcompris_skin_get_font_default", &id))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_skin_get_font(id);

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


static PyMethodDef PythonGcomprisSkinModule[] = {
  { "image_to_skin",  py_gcompris_image_to_skin, METH_VARARGS, "gcompris_image_to_skin" },
  { "load_pixmap",  py_gcompris_load_skin_pixmap, METH_VARARGS, "gcompris_load_skin_pixmap" },
  { "get_color_default",  py_gcompris_skin_get_color_default, METH_VARARGS, 
    "gcompris_skin_get_color_default" },
  { "get_font_default",  py_gcompris_skin_get_font_default, METH_VARARGS, 
    "gcompris_skin_get_font_default" },
  { "get_color",  py_gcompris_skin_get_color, METH_VARARGS, 
    "gcompris_skin_get_color" },
  { "get_font",  py_gcompris_skin_get_font, METH_VARARGS, 
    "gcompris_skin_get_font" },
  { NULL, NULL, 0, NULL}
};


void python_gcompris_skin_module_init(void)
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
