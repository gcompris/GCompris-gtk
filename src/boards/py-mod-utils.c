#include <Python.h>
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-mod-utils.h"
#include "py-gcompris-board.h"

/* All functions provided by this python module
 * wraps a gcompris function. Each "py_*" function wraps the
 * "*" C function.
 */


/* GdkPixbuf *gcompris_load_number_pixmap(char number); */
static PyObject*
py_gcompris_load_number_pixmap(PyObject* self, PyObject* args)
{
  char value;
  GdkPixbuf* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "b:gcompris_load_number_pixmap", &value))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_load_number_pixmap(value);

  /* Create and return the result */
  return(PyObject*)pygobject_new((GObject*) result);
}


/* GdkPixbuf *gcompris_load_pixmap(char *pixmapfile); */
static PyObject*
py_gcompris_load_pixmap(PyObject* self, PyObject* args)
{
  char* pixmapfile;
  GdkPixbuf* result;
  /* Parse arguments */

  if(!PyArg_ParseTuple(args, "s:gcompris_load_pixmap", &pixmapfile))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_load_pixmap(pixmapfile);

  /* Create and return the result */
  return (PyObject*) pygobject_new((GObject*) result);
}


/* void	gcompris_set_image_focus(GnomeCanvasItem *item, gboolean focus); */
static PyObject*
py_gcompris_set_image_focus(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  gint pyfocus;
  gboolean focus;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Oi:gcompris_set_image_focus", &pyitem, &pyfocus))
    return NULL;

  item = (GnomeCanvasItem*) pygobject_get(pyitem);
  if(pyfocus>0) focus = TRUE;
  else focus = FALSE;

  /* Call the corresponding C function */
  gcompris_set_image_focus(item, focus);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* gint	gcompris_item_event_focus(GnomeCanvasItem *item,
                                  GdkEvent *event,
                                  GnomeCanvasItem *dest_item);
*/
static PyObject*
py_gcompris_item_event_focus(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  PyObject* pyevent;
  GdkEvent* event;
  gint result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "OO:gcompris_item_event_focus", &pyitem, &pyevent))
    return NULL;
  item = (GnomeCanvasItem*) pygobject_get(pyitem);
  event = (GdkEvent*) pygobject_get(pyevent);

  /* Call the corresponding C function */
  result = gcompris_item_event_focus(item, event, NULL);

  /* Create and return the result */
  return Py_BuildValue("i", result);
}


/* void item_absolute_move(GnomeCanvasItem *item, int x, int y); */
static PyObject*
py_gcompris_item_absolute_move(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  int x, y;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Oii:gcompris_item_absolute_move", &pyitem, &x, &y))
    return NULL;
  item = (GnomeCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  item_absolute_move(item, x, y);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void item_rotate(GnomeCanvasItem *item, double angle); */
static PyObject*
py_gcompris_item_rotate(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  double angle;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Od:gcompris_item_rotate", &pyitem, &angle))
    return NULL;
  item = (GnomeCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  item_rotate(item, angle);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void item_rotate_relative(GnomeCanvasItem *item, double angle); */
static PyObject*
py_gcompris_item_rotate_relative(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  double angle;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Od:gcompris_item_rotate_relative", &pyitem, &angle))
    return NULL;
  item = (GnomeCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  item_rotate_relative(item, angle);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void item_rotate_with_center(GnomeCanvasItem *item, double angle, int x, int y); */
static PyObject*
py_gcompris_item_rotate_with_center(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  double angle;
  int x,y;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Odii:gcompris_item_rotate_with_center", &pyitem, &angle, &x, &y))
    return NULL;
  item = (GnomeCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  item_rotate_with_center(item, angle, x, y);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void item_rotate_relative_with_center(GnomeCanvasItem *item, double angle, int x, int y); */
static PyObject*
py_gcompris_item_rotate_relative_with_center(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  double angle;
  int x,y;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Odii:gcompris_item_rotate_relative_with_center", &pyitem, &angle, &x, &y))
    return NULL;
  item = (GnomeCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  item_rotate_relative_with_center(item, angle, x, y);

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


/* void	gcompris_dialog(gchar *str, DialogBoxCallBack dbcb); */
static PyObject*
py_gcompris_dialog(PyObject* self, PyObject* args)
{
  PyObject* pyCallback;
  gchar* str;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "sO:gcompris_dialog", &str, &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback)) return NULL;

  /* Call the corresponding C function */
  gcompris_dialog(str, pyDialogBoxCallBack);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* GdkPixbuf *gcompris_load_pixmap_asset(gchar *dataset, gchar* categories, */
/*                                       gchar* mimetype, gchar* name);     */
static PyObject*
py_gcompris_load_pixmap_asset(PyObject* self, PyObject* args)
{
  gchar* dataset;
  gchar* categories;
  gchar* mimetype;
  gchar* name;
  GdkPixbuf* result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ssss:gcompris_load_pixmap_asset", &dataset, &categories, &mimetype, &name))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_load_pixmap_asset(dataset, categories, mimetype, name);

  /* Create and return the result */
  return(PyObject*)pygobject_new((GObject*) result);
}


/* gchar *gcompris_get_asset_file(gchar *dataset, gchar* categories, */
/*                                gchar* mimetype, gchar* file);     */
static PyObject*
py_gcompris_get_asset_file(PyObject* self, PyObject* args)
{
  gchar* dataset;
  gchar* categories;
  gchar* mimetype;
  gchar* name;
  gchar* result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ssss:gcompris_get_asset_file", &dataset, &categories, &mimetype, &name))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_get_asset_file(dataset, categories, mimetype, name);

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


static PyMethodDef PythonGcomprisUtilsModule[] = {
  { "load_number_pixmap",  py_gcompris_load_number_pixmap, METH_VARARGS, "gcompris_load_number_pixmap" },
  { "load_pixmap",  py_gcompris_load_pixmap, METH_VARARGS, "gcompris_load_pixmap" },
  { "set_image_focus",  py_gcompris_set_image_focus, METH_VARARGS, "gcompris_set_image_focus" },
  { "item_event_focus",  py_gcompris_item_event_focus, METH_VARARGS, "gcompris_item_event_focus" },
  { "item_absolute_move",  py_gcompris_item_absolute_move, METH_VARARGS, "item_absolute_move" },
  { "item_rotate",  py_gcompris_item_rotate, METH_VARARGS, "item_rotate" },
  { "item_rotate_relative",  py_gcompris_item_rotate_relative, METH_VARARGS, "item_rotate_relative" },
  { "item_rotate_relative_with_center",  py_gcompris_item_rotate_relative_with_center, METH_VARARGS,
    "item_rotate_relative_with_center" },
  { "item_rotate_with_center",  py_gcompris_item_rotate_with_center, METH_VARARGS,
    "item_rotate_with_center" },
  { "dialog",  py_gcompris_dialog, METH_VARARGS, "gcompris_dialog" },
  { "load_pixmap_asset",  py_gcompris_load_pixmap_asset, METH_VARARGS, "gcompris_load_pixmap_asset" },
  { "get_asset_file",  py_gcompris_get_asset_file, METH_VARARGS, "gcompris_get_asset_file" },
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
