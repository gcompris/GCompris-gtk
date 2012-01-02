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

#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
  gdk_pixbuf_unref(result);
#else
  g_object_unref(result);
#endif

  return(pyresult);
}


/* GdkPixbuf *gc_skin_rsvg_get(); */
static PyObject*
py_gc_skin_rsvg_get(PyObject* self, PyObject* args)
{
  RsvgHandle* result;
  PyObject* pyresult;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_skin_rsvg_get"))
    return NULL;

  /* Call the corresponding C function */
  result = gc_skin_rsvg_get();

  /* Create and return the result */
  pyresult = (PyObject*) pygobject_new((GObject*) result);

  // 'result' must not be freed, it is managed by gc_skin_rsvg_get()

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
  { "svg_get",  py_gc_skin_rsvg_get, METH_VARARGS, "gc_skin_rsvg_get" },
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
  Py_InitModule("_gcompris_skin", PythonGcomprisSkinModule);
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
