/* gcompris - py-mod-utils.c
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
#include "py-mod-utils.h"
#include "py-gcompris-board.h"

/* All functions provided by this python module
 * wraps a gcompris function. Each "py_*" function wraps the
 * "*" C function.
 */


/* GdkPixbuf *gc_pixmap_load(char *pixmapfile); */
static PyObject*
py_gc_pixmap_load(PyObject* self, PyObject* args)
{
  char* pixmapfile;
  GdkPixbuf* result;
  PyObject* pyresult;

  /* Parse arguments */

  if(!PyArg_ParseTuple(args, "s:gc_pixmap_load", &pixmapfile))
    return NULL;

  /* Call the corresponding C function */
  result = gc_pixmap_load(pixmapfile);

  /* Create and return the result */
  pyresult = (PyObject*) pygobject_new((GObject*) result);

  gdk_pixbuf_unref(result);

  return(pyresult);

}

/* RsvgHandle *gc_svg_load(char *pixmapfile); */
static PyObject*
py_gc_svg_load(PyObject* self, PyObject* args)
{
  char* pixmapfile;
  RsvgHandle* result;
  PyObject* pyresult;

  /* Parse arguments */

  if(!PyArg_ParseTuple(args, "s:gc_svg_load", &pixmapfile))
    return NULL;

  /* Call the corresponding C function */
  result = gc_rsvg_load(pixmapfile);

  /* Create and return the result */
  pyresult = (PyObject*) pygobject_new((GObject*) result);

  rsvg_handle_free(result);

  return(pyresult);

}

/* gchar *gc_file_find_absolute(gchar *file); */
static PyObject*
py_gc_file_find_absolute(PyObject* self, PyObject* args)
{
  char* file;
  gchar* result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gc_db_get_filename", &file))
    return NULL;

  /* Call the corresponding C function */
  result = (gchar*)gc_file_find_absolute(file);

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


/* gint	gc_item_focus_init(GooCanvasItem *source_item,
                           GooCanvasItem *target_item);
*/
static PyObject*
py_gc_item_focus_init(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  PyObject* pytarget;
  GooCanvasItem* item;
  GooCanvasItem* target = NULL;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "OO:gc_item_focus_init",
		       &pyitem, &pytarget))
    return NULL;

  item = (GooCanvasItem*) pygobject_get(pyitem);
  if(pytarget != Py_None)
    target = (GooCanvasItem*) pygobject_get(pytarget);

  /* Call the corresponding C function */
  gc_item_focus_init(item, target);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* gint	gc_item_focus_remove(GooCanvasItem *source_item,
                           GooCanvasItem *target_item);
*/
static PyObject*
py_gc_item_focus_remove(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  PyObject* pytarget;
  GooCanvasItem* item;
  GooCanvasItem* target = NULL;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "OO:gc_item_focus_remove",
		       &pyitem, &pytarget))
    return NULL;

  item = (GooCanvasItem*) pygobject_get(pyitem);
  if(pytarget != Py_None)
    target = (GooCanvasItem*) pygobject_get(pytarget);

  /* Call the corresponding C function */
  gc_item_focus_remove(item, target);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_item_absolute_move(GooCanvasItem *item, int x, int y); */
static PyObject*
py_gc_item_absolute_move(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GooCanvasItem* item;
  int x, y;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Oii:gc_item_absolute_move", &pyitem, &x, &y))
    return NULL;
  item = (GooCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  gc_item_absolute_move(item, x, y);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_item_rotate(GooCanvasItem *item, double angle); */
static PyObject*
py_gc_item_rotate(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GooCanvasItem* item;
  double angle;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Od:gc_item_rotate_relative", &pyitem, &angle))
    return NULL;
  item = (GooCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  gc_item_rotate(item, angle);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_item_rotate_relative(GooCanvasItem *item, double angle); */
static PyObject*
py_gc_item_rotate_relative(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GooCanvasItem* item;
  double angle;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Od:gc_item_rotate_relative", &pyitem, &angle))
    return NULL;
  item = (GooCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  gc_item_rotate_relative(item, angle);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_item_rotate_with_center(GooCanvasItem *item, double angle, int x, int y); */
static PyObject*
py_gc_item_rotate_with_center(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GooCanvasItem* item;
  double angle;
  int x,y;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Odii:gc_item_rotate_with_center", &pyitem, &angle, &x, &y))
    return NULL;
  item = (GooCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  gc_item_rotate_with_center(item, angle, x, y);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_item_rotate_relative_with_center(GooCanvasItem *item, double angle, int x, int y); */
static PyObject*
py_gc_item_rotate_relative_with_center(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GooCanvasItem* item;
  double angle;
  int x,y;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Odii:gc_item_rotate_relative_with_center", &pyitem, &angle, &x, &y))
    return NULL;
  item = (GooCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  gc_item_rotate_relative_with_center(item, angle, x, y);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* Dialog callback wrapper */
static PyObject* pyDialogBoxCallBackFunc = NULL;

/* typedef void (*DialogBoxCallBack) (); */
void pyDialogBoxCallBack(){
  PyObject* result;
  if(pyDialogBoxCallBackFunc==NULL) return;

  /* Build arguments */
  result = PyObject_CallObject(pyDialogBoxCallBackFunc, NULL);
  if(result==NULL){
    PyErr_Print();
  } else {
    Py_DECREF(result);
  }
}


/* void	gc_dialog(gchar *str, DialogBoxCallBack dbcb); */
static PyObject*
py_gc_dialog(PyObject* self, PyObject* args)
{
  PyObject* pyCallback;
  gchar* str;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "sO:gc_dialog", &str, &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback)){
    gc_dialog(str,NULL);
  }
  else {
  /* Call the corresponding C function */
  pyDialogBoxCallBackFunc = pyCallback ;
  gc_dialog(str, pyDialogBoxCallBack);
  }
  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void	gc_dialog_close(); */
static PyObject*
py_gc_dialog_close(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_dialog"))
    return NULL;

  /* Call the corresponding C function */
  gc_dialog_close();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void filename_pass(PyObject *pyitem, PyObject *pystring) */
static PyObject*
py_gcompris_filename_pass(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GooCanvasItem *item;
  char *string;


  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Os:gcompris_filename_pass", &pyitem, &string))
    return NULL;

  /* pass parameter from python */
  item = (GooCanvasItem *) pygobject_get(pyitem);

  /* gcompris_filename_pass( item, string); */
  g_object_set_data( G_OBJECT(item), "filename", string);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;

}

/*
 * Set a property in a canvas object
 * ---------------------------------
 */
/* void canvas_set_property(GooCanvasItem *item, gchar *property, gchar* value) */
static PyObject*
py_gcompris_canvas_set_property(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GooCanvasItem *item;
  char *property;
  char *value;


  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Oss:gcompris_canvas_set_property",
		       &pyitem, &property, &value))
    return NULL;

  /* pass parameter from python */
  item = (GooCanvasItem *) pygobject_get(pyitem);

  /* gcompris_filename_pass( item, string); */
  g_object_set_data( G_OBJECT(item), property, value);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;

}

/*
 * Get a property in a canvas object
 * ---------------------------------
 */
/* gchar *canvas_get_property(GooCanvasItem *item, gchar *property) */
static PyObject*
py_gcompris_canvas_get_property(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GooCanvasItem *item;
  char  *property;
  gchar *result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Os:gcompris_canvas_get_property", &pyitem, &property))
    return NULL;

  /* pass parameter from python */
  item = (GooCanvasItem *) pygobject_get(pyitem);

  /* gcompris_filename_pass( item, string); */
  result = g_object_get_data( G_OBJECT(item), property);

  /* Create and return the result */
  if(result==NULL) {
    Py_INCREF(Py_None);
    return Py_None;
  } else {
    return Py_BuildValue("s", result);
  }

}


static PyMethodDef PythonGcomprisUtilsModule[] = {
  { "load_pixmap",  py_gc_pixmap_load, METH_VARARGS, "gc_pixmap_load" },
  { "load_svg",  py_gc_svg_load, METH_VARARGS, "gc_svg_load" },
  { "find_file_absolute",  py_gc_file_find_absolute, METH_VARARGS, "gc_file_find_absolute" },
  { "item_focus_init",  py_gc_item_focus_init, METH_VARARGS, "gc_item_focus_init" },
  { "item_focus_remove",  py_gc_item_focus_remove, METH_VARARGS, "gc_item_focus_remove" },
  { "item_absolute_move",  py_gc_item_absolute_move, METH_VARARGS, "gc_item_absolute_move" },
  { "item_rotate",  py_gc_item_rotate, METH_VARARGS, "gc_item_rotate" },
  { "item_rotate_relative",  py_gc_item_rotate_relative, METH_VARARGS, "gc_item_rotate_relative" },
  { "item_rotate__with_center",  py_gc_item_rotate_with_center, METH_VARARGS,
    "item_rotate_with_center" },
  { "item_rotate_relative_with_center",  py_gc_item_rotate_relative_with_center, METH_VARARGS,
    "item_rotate_relative_with_center" },
  { "dialog",  py_gc_dialog, METH_VARARGS, "gc_dialog" },
  { "dialog_close",  py_gc_dialog_close, METH_VARARGS, "gc_dialog_close" },
  { "filename_pass",  py_gcompris_filename_pass, METH_VARARGS, "gcompris_filename_pass" },
  { "canvas_set_property",  py_gcompris_canvas_set_property, METH_VARARGS, "gcompris_canvas_set_property" },
  { "canvas_get_property",  py_gcompris_canvas_get_property, METH_VARARGS, "gcompris_canvas_get_property" },
  { NULL, NULL, 0, NULL}
};


void python_gcompris_utils_module_init(void)
{
  PyObject* module;
  module = Py_InitModule("_gcompris_utils", PythonGcomprisUtilsModule);
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
